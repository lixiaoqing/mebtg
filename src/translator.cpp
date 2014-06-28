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
			generate_kbest_for_span(span,start_pos);
		}
	}
}

void SentenceTranslator::generate_kbest_for_span(size_t span,size_t start_pos)
{
	priority_queue<Cand> merged_cands;
	for(size_t span_lhs=0;span_lhs<span;span_lhs++)
	{
		Cand cand = merge_two_subcands(candpq_matrix.at(start_pos).at(span_lhs).top(),candpq_matrix.at(start_pos+span_lhs+1).at(span-span_lhs-1).top());
		merged_cands.push(cand);
	}

	set<vector<int> > duplicate_set;
	duplicate_set.clear();
	for(size_t i=0;i<BEAM_SIZE+EXTRA_SIZE;i++)
	{
		if (merged_cands.empty()==true)
			break;
		Cand best_cand = merged_cands.pop();
		if (span == src_sen_len)
		{
		}
		candpq_matrix.at(start_pos).at(span).push(best_cand);
		vector<int> key = {best_cand->left_ith_best,best_cand->right_jth_best,best_cand->split_pos};
		if (duplicate_set.find(key) != duplicate_set.end())
		{
			add_neighbours_to_merged_cands();
			duplicate_set.insert(key);
		}
	}
}

Cand SentenceTranslator::merge_two_subcands(const Cand &cand_lhs, const Cand &cand_rhs)
{
}

void SentenceTranslator::add_neighbours_to_merged_cands()
{
}
