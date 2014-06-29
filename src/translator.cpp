#include "translator.h"

FileTranslator::FileTranslator(Vocab *i_src_vocab, Vocab *i_tgt_vocab, RuleTable *i_ruletable, MaxentModel *i_reorder_model, LanguageModel *i_lm_model)
{
	src_vocab = i_src_vocab;
	tgt_vocab = i_tgt_vocab;
	ruletable = i_ruletable;
	reorder_model = i_reorder_model;
	lm_model = i_lm_model;
}

void FileTranslator::translate_file(const string &input_file, const string &output_file)
{
	ifstream fin(input_file.c_str());
	if (!fin.is_open())
	{
		cerr<<"cannot open input file!\n";
		return;
	}
	ofstream fout(output_file.c_str());
	if (!fout.is_open())
	{
		cerr<<"cannot open output file!\n";
		return;
	}
	string line;
	while(getline(fin,line))
	{
		TrimLine(line);
		if (line.size()==0)
		{
			fout<<endl;
		}
		else
		{
			SentenceTranslator sen_translator(line);
			fout<<sen_translator.translate_sentence()<<endl;
		}
	}
}

SentenceTranslator::SentenceTranslator(Vocab *i_src_vocab, Vocab *i_tgt_vocab, RuleTable *i_ruletable, MaxentModel *i_reorder_model, LanguageModel *i_lm_model, const string &input_sen)
{
	src_vocab = i_src_vocab;
	tgt_vocab = i_tgt_vocab;
	ruletable = i_ruletable;
	reorder_model = i_reorder_model;
	lm_model = i_lm_model;

	stringstream ss(input_sen);
	string word;
	while(ss>>word)
	{
		src_word_id_list.push_back(src_vocab->get_id(word));
	}

	src_sen_len = src_word_id_list.size();
	candpq_matrix.resize(src_sen_len);
	for (size_t span=0;span<src_sen_len;span++)
	{
		candpq_matrix.at(span).resize(src_sen_len-span);
	}

	fill_matrix_with_matched_rules();
}

double SentenceTranslator::cal_increased_lm_score_for_sen_frag(const Cand &cand)
{
	vector<int> &wids = cand.tgt_word_id_list;
	if (cand.split_pos == -1)
	{
		return lm_model->eval(wids);
	}
	else
	{
		//NOTE:split_pos shoud equal the size of the left ant
		size_t rbound_in_left = cand.split_pos - (LM_ORDER_IN_MT-1)<cand.split_pos?(LM_ORDER_IN_MT-1):cand.split_pos;  //NOTE:minus one needed?
		size_t lbound_in_right = (LM_ORDER_IN_MT-1)<(wids.size()-cand.split_pos)?(LM_ORDER_IN_MT-1):(wids.size()-cand.split_pos);
		auto begit = wids.begin();
		vector<int> rbound_words_in_left(begit+rbound_in_left,begit+cand.split_pos);
		vector<int> lbound_words_in_right(begit+cand.split_pos,begit+lbound_in_right);
		vector<int> combined_bound_words(begit+rbound_in_left,begit+lbound_in_right);
		return lm_model->eval_word_id_list(combined_bound_words) - lm_model->eval_word_id_list(rbound_words_in_left) - lm_model->eval_word_id_list(lbound_words_in_right);
	}
}

double SentenceTranslator::cal_increased_lm_score_for_whole_sen(const Cand &cand)
{
	vector<int> &wids = cand.tgt_word_id_list;
	size_t len = wids.size();
	size_t bound = (LM_ORDER_IN_MT-1)<len?(LM_ORDER_IN_MT-1):len;
	vector<int> words_in_left_bound(wids.begin(),wids.begin()+bound);
	vector<int> words_in_right_bound(wids.end()-bound,wids.end());

	vector<int> sen_begin_vec, sen_end_vec;
	sen_begin_vec.push_back(tgt_vocab->get_id("<s>"));
	sen_begin_vec.insert(sen_begin_vec.end(), left_boundary_wids.begin(), left_boundary_wids.end());
	sen_end_vec = right_boundary_wids;
	sen_end_vec.push_back(tgt_vocab->get_id("</s>"));
	return lm_model->eval_word_id_list(sen_begin_vec) - lm_model->eval_word_id_list(words_in_left_bound) + lm_model->eval_word_id_list(sen_end_vec) - lm_model->eval_word_id_list(words_in_right_bound);
}

void SentenceTranslator::fill_matrix_with_matched_rules()
{
	for (size_t start_pos=0;start_pos<src_sen_len;start_pos++)
	{
		vector<vector<TgtRule>* > matched_rules_for_prefixes = ruletable->find_matched_rules_for_prefixes(src_word_id_list,start_pos);
		for (size_t span=0;span<matched_rules_for_prefixes.size();span++)
		{
			if (matched_rules_for_prefixes.at(span) == NULL)
			{
				if (span == 0)
				{
					Cand cand;
					cand.first_src_word_pos = start_pos;
					cand.last_src_word_pos = start_pos+span;
					cand.tgt_word_num = 1;
					cand.tgt_word_id_list = {tgt_vocab->get_id("<unk>")};
					cand.trans_prob_list.resize(PROB_NUM,LogP_PseudoZero);
					cand.score = 0;
					for (size_t i=0;i<PROB_NUM;i++)
					{
						cand.score += feature_weight.trans.at(i)*cand.trans_prob_list.at(i);
					}
					cand.phrase_num = 1;
					cand.lm_prob = cal_increased_lm_score_for_sen_frag(cand.tgt_word_id_list);
					cand.score += feature_weight.phrase_num*cand.phrase_num + feature_weight.len*cand.tgt_word_num + feature_weight.lm*cand.lm_prob;
					candpq_matrix.at(start_pos).at(span).push(cand);
				}
				continue;
			}
			for (const auto &tgt_rule : *matched_rules_for_prefixes.at(span))
			{
				Cand cand;
				cand.first_src_word_pos = start_pos;
				cand.last_src_word_pos = start_pos+span;
				cand.tgt_word_num = tgt_rule.word_num;
				cand.tgt_word_id_list = tgt_rule.word_id_list;
				cand.trans_prob_list = tgt_rule.prob_list;
				cand.score = tgt_rule.score;
				cand.phrase_num = 1;
				cand.lm_prob = cal_increased_lm_score_for_sen_frag(cand.tgt_word_id_list);
				cand.score += feature_weight.phrase_num*cand.phrase_num + feature_weight.len*cand.tgt_word_num + feature_weight.lm*cand.lm_prob;
				candpq_matrix.at(start_pos).at(span).push(cand);
			}
		}
	}
}

string SentenceTranslator::translate_sentence()
{
	for (size_t span=1;span<src_sen_len;span++)
	{
		for(size_t start_pos=0;start_pos<src_sen_len-span;start_pos++)
		{
			generate_kbest_for_span(start_pos,span);
		}
	}
}

void SentenceTranslator::generate_kbest_for_span(size_t start_pos,size_t span)
{
	priority_queue<Cand> new_cands_from_mergence;
	for(size_t span_lhs=0;span_lhs<span;span_lhs++)
	{
		merge_subcands_and_add_to_new(candpq_matrix.at(start_pos).at(span_lhs).top(),candpq_matrix.at(start_pos+span_lhs+1).at(span-span_lhs-1).top(),1,1,new_cands_from_mergence);
	}

	set<vector<int> > duplicate_set;
	duplicate_set.clear();
	for(size_t i=0;i<BEAM_SIZE+EXTRA_SIZE;i++)
	{
		if (new_cands_from_mergence.empty()==true)
			break;
		Cand best_cand = new_cands_from_mergence.pop();
		if (span == src_sen_len)
		{
		}
		candpq_matrix.at(start_pos).at(span).push(best_cand);
		vector<int> key = {best_cand->rank_in_left,best_cand->rank_in_right,best_cand->split_pos};
		if (duplicate_set.find(key) == duplicate_set.end())
		{
			add_neighbours_to_merged_cands(cur_cand,new_cands_from_mergence);
			duplicate_set.insert(key);
		}
	}
}

void SentenceTranslator::merge_subcands_and_add_to_new(const Cand &cand_lhs, const Cand &cand_rhs,int rank_in_left,int rank_in_right,priority_queue<Cand> &new_cands_from_mergence)
{
	double straight_reorder_prob = 0;
	double swap_reorder_prob = 0;
	if (cand_rhs.last_src_word_pos - cand_lhs.first_src_word_pos < REORDER_WINDOW)
	{
		cal_reorder_score(straight_reorder_prob,swap_reorder_prob,cand_lhs,cand_rhs);
	}
	//NOTE: otherwise?
	
	Cand cand_straight;
	cand_straight.first_src_word_pos = cand_lhs.first_src_word_pos;
	cand_straight.last_src_word_pos = cand_rhs.last_src_word_pos;
	cand_straight.tgt_word_num = cand_lhs.tgt_word_num + cand_rhs.tgt_word_num;
	cand_straight.phrase_num = cand_lhs.phrase_num + cand_rhs.phrase_num;
	cand_straight.straight_reorder_prob = cand_lhs.straight_reorder_prob + cand_rhs.straight_reorder_prob + straight_reorder_prob;
	cand_straight.swap_reorder_prob = cand_lhs.swap_reorder_prob + cand_rhs.swap_reorder_prob + swap_reorder_prob;
	cand_straight.split_pos = cand_lhs.last_src_word_pos + 1; //NOTE:maybe split_pos should be relative pos
	cand_straight.rank_in_left = rank_in_left;
	cand_straight.rank_in_right = rank_in_right;
	cand_straight.tgt_word_id_list = cand_lhs.tgt_word_id_list;
	cand_straight.tgt_word_id_list.insert(cand_straight.tgt_word_id_list.end(),cand_rhs.tgt_word_id_list.begin(),cand_rhs.tgt_word_id_list.end());
	cand_straight.tgt_str = cand_lhs.tgt_str + " " + cand_rhs.tgt_str;
	for (size_t i=0;i<PROB_NUM;i++)
	{
		cand_straight.trans_prob_list.push_back(cand_lhs.trans_prob_list.at(i)+cand_rhs.trans_prob_list.at(i));
	}
	cand_straight.left_ant = &cand_lhs;
	cand_straight.right_ant = &cand_rhs;
	double increased_lm_prob = cal_increased_lm_score_for_sen_frag(cand_straight);
	cand_straight.lm_prob = cand_lhs.lm_prob + cand_rhs.lm_prob + increased_lm_prob;
	//NOTE: add context based translation score here
	cand_straight.score = cand_lhs.score + cand_rhs.score + feature_weight.lm*increased_lm_prob + feature_weight.reorder_straight*straight_reorder_prob;
	new_cands_from_mergence.push(cand_straight);

	if (cand_rhs.last_src_word_pos - cand_lhs.first_src_word_pos >= REORDER_WINDOW)
		return;
	Cand cand_swap = cand_straight;
	cand_swap.first_src_word_pos = cand_rhs.first_src_word_pos;
	cand_swap.last_src_word_pos = cand_lhs.last_src_word_pos;
	cand_swap.split_pos = cand_rhs.last_src_word_pos + 1;
	cand_swap.tgt_word_id_list = cand_rhs.tgt_word_id_list;
	cand_swap.tgt_word_id_list.insert(cand_swap.tgt_word_id_list.end(),cand_lhs.tgt_word_id_list.begin(),cand_lhs.tgt_word_id_list.end());
	cand_swap.tgt_str = cand_rhs.tgt_str + " " + cand_lhs.tgt_str;
	increased_lm_prob = cal_increased_lm_score_for_sen_frag(cand_swap);
	cand_swap.lm_prob = cand_lhs.lm_prob + cand_rhs.lm_prob + increased_lm_prob;
	//NOTE: add context based translation score here
	cand_swap.score = cand_lhs.score + cand_rhs.score + feature_weight.lm*increased_lm_prob + feature_weight.reorder_straight*swap_reorder_prob;
	new_cands_from_mergence.push(cand_swap);
}

void SentenceTranslator::add_neighbours_to_merged_cands(Cand &cur_cand, priority_queue<Cand> &new_cands_from_mergence)
{
	int start = cur_cand.first_src_word_pos;
	int end = cur_cand.last_src_word_pos;
	int split_pos = cur_cand.split_pos;

	int rank_in_left = cur_cand.rank_in_left + 1;
	int rank_in_right = cur_cand.rank_in_right;
	if(candpq_matrix.at(start).at(split_pos-start).size() >= rank_in_left)
	{
		merge_subcands_and_add_to_new(candpq_matrix.at(start).at(split_pos-start).at(rank_in_left-1),candpq_matrix.at(split_pos).at(end-split_pos).at(rank_in_right-1),rank_in_left,rank_in_right,new_cands_from_mergence);
	}

	rank_in_left = cur_cand.rank_in_left;
	rank_in_right = cur_cand.rank_in_right + 1;
	if(candpq_matrix.at(split_pos).at(end-split_pos).size() >= rank_in_right)
	{
		merge_subcands_and_add_to_new(candpq_matrix.at(start).at(split_pos-start).at(rank_in_left-1),candpq_matrix.at(split_pos).at(end-split_pos).at(rank_in_right-1),rank_in_left,rank_in_right,new_cands_from_mergence);
	}
}
