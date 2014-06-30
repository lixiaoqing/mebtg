#include "translator.h"

FileTranslator::FileTranslator(const Models &i_models, const Parameter &i_para, const Weight &i_weight)
{
	models = i_models;
	para = i_para;
	feature_weight = i_weight;
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
		line.erase(0,line.find_first_not_of(" \t\r\n"));
		line.erase(line.find_last_not_of(" \t\r\n")+1);
		if (line.size()==0)
		{
			fout<<endl;
		}
		else
		{
			SentenceTranslator sen_translator(models,para,feature_weight,line);
			fout<<sen_translator.translate_sentence()<<endl;
		}
	}
}

SentenceTranslator::SentenceTranslator(const Models &i_models, const Parameter &i_para, const Weight &i_weight, const string &input_sen)
{
	src_vocab = i_models.src_vocab;
	tgt_vocab = i_models.tgt_vocab;
	ruletable = i_models.ruletable;
	reorder_model = i_models.reorder_model;
	lm_model = i_models.lm_model;
	para = i_para;
	feature_weight = i_weight;

	stringstream ss(input_sen);
	string word;
	while(ss>>word)
	{
		src_wids.push_back(src_vocab->get_id(word));
	}

	src_sen_len = src_wids.size();
	candpq_matrix.resize(src_sen_len);
	for (size_t span=0;span<src_sen_len;span++)  //this span is pseudo, there is ONE word in span with size 0, etc.
	{
		candpq_matrix.at(span).resize(src_sen_len-span);
	}

	fill_matrix_with_matched_rules();
}

void SentenceTranslator::fill_matrix_with_matched_rules()
{
	for (size_t beg=0;beg<src_sen_len;beg++)
	{
		vector<vector<TgtRule>* > matched_rules_for_prefixes = ruletable->find_matched_rules_for_prefixes(src_wids,beg);
		for (size_t span=0;span<matched_rules_for_prefixes.size();span++)
		{
			if (matched_rules_for_prefixes.at(span) == NULL)
			{
				if (span == 0)
				{
					Cand cand;
					cand.beg = beg;
					cand.end = beg+span;
					cand.tgt_wids.push_back(tgt_vocab->get_id("<unk>"));
					cand.score = 0;
					cand.trans_probs.resize(PROB_NUM,LogP_PseudoZero);
					for (size_t i=0;i<PROB_NUM;i++)
					{
						cand.score += feature_weight.trans.at(i)*cand.trans_probs.at(i);
					}
					cand.lm_prob = cal_increased_lm_score_for_sen_frag(cand);
					cand.score += feature_weight.phrase_num*cand.phrase_num + feature_weight.len*cand.tgt_word_num + feature_weight.lm*cand.lm_prob;
					candpq_matrix.at(beg).at(span).push(cand);
				}
				continue;
			}
			for (const auto &tgt_rule : *matched_rules_for_prefixes.at(span))
			{
				Cand cand;
				cand.beg = beg;
				cand.end = beg+span;
				cand.tgt_word_num = tgt_rule.word_num;
				cand.tgt_wids = tgt_rule.word_id_list;
				cand.trans_probs = tgt_rule.prob_list;
				cand.score = tgt_rule.score;
				cand.lm_prob = cal_increased_lm_score_for_sen_frag(cand);
				cand.score += feature_weight.phrase_num*cand.phrase_num + feature_weight.len*cand.tgt_word_num + feature_weight.lm*cand.lm_prob;
				candpq_matrix.at(beg).at(span).push(cand);
			}
		}
	}
}

pair<double> SentenceTranslator::cal_reorder_score(const Cand &cand_lhs,const Cand &cand_rhs)
{}

double SentenceTranslator::cal_increased_lm_score_for_sen_frag(const Cand &cand)
{
	const vector<int> &wids = cand.tgt_wids;
	if (cand.mid == -1)
	{
		return lm_model->eval(wids);
	}
	else
	{
		size_t size_lhs = cand.mid - cand.beg;
		size_t size_rhs = cand.end - cand.mid + 1;
		size_t bound_size_lhs = min(size_lhs, LM_ORDER-1);
		size_t bound_size_rhs = min(size_rhs, LM_ORDER-1);
		auto it_split = wids.begin() + cand.mid;
		vector<int> bound_words_lhs(it_split-bound_size_lhs,it_split);
		vector<int> bound_words_rhs(it_split,it_split+bound_size_rhs);
		vector<int> combined_bound_words(it_split-bound_size_lhs,it_split+bound_size_rhs);
		return lm_model->eval(combined_bound_words) - lm_model->eval(bound_words_lhs) - lm_model->eval(bound_words_rhs);
	}
}

double SentenceTranslator::cal_increased_lm_score_for_whole_sen(const Cand &cand)
{
	const vector<int> &wids = cand.tgt_wids;
	size_t len = wids.size();
	size_t bound = min(len,LM_ORDER-1);
	vector<int> sen_beg_words(wids.begin(),wids.begin()+bound);
	vector<int> sen_end_words(wids.end()-bound,wids.end());

	vector<int> sen_beg_words_ext, sen_end_words_ext;
	sen_beg_words_ext.push_back(tgt_vocab->get_id("<s>"));
	sen_beg_words_ext.insert(sen_beg_words_ext.end(), sen_beg_words.begin(), sen_beg_words.end());
	sen_end_words_ext = sen_end_words;
	sen_end_words_ext.push_back(tgt_vocab->get_id("</s>"));
	return lm_model->eval(sen_beg_words_ext) - lm_model->eval(sen_beg_words) + lm_model->eval(sen_end_words_ext) - lm_model->eval(sen_end_words);
}

string SentenceTranslator::translate_sentence()
{
	for (size_t span=1;span<src_sen_len;span++)
	{
		for(size_t beg=0;beg<src_sen_len-span;beg++)
		{
			generate_kbest_for_span(beg,span);
		}
	}
}

void SentenceTranslator::generate_kbest_for_span(size_t beg,size_t span)
{
	Candpq candpq_merge;
	for(size_t span_lhs=0;span_lhs<span;span_lhs++)
	{
		const Cand &best_cand_lhs = candpq_matrix.at(beg).at(span_lhs).top();
		const Cand &best_cand_rhs = candpq_matrix.at(beg+span_lhs+1).at(span-span_lhs-1).top();
		merge_subcands_and_add_to_pq(best_cand_lhs,best_cand_rhs,1,1,candpq_merge);
	}

	set<vector<int> > duplicate_set;
	duplicate_set.clear();
	for(size_t i=0;i<para.BEAM_SIZE+para.EXTRA_SIZE;i++)
	{
		if (candpq_merge.empty()==true)
			break;
		Cand best_cand = candpq_merge.top();
		candpq_merge.pop();
		if (span == src_sen_len-1)
		{
			double increased_lm_prob = cal_increased_lm_score_for_whole_sen(best_cand);
			best_cand.lm_prob += increased_lm_prob;
			best_cand.score += feature_weight.lm*increased_lm_prob;
		}
		candpq_matrix.at(beg).at(span).push(best_cand);
		vector<int> key = {best_cand.rank_lhs,best_cand.rank_rhs,best_cand.mid};
		if (duplicate_set.find(key) == duplicate_set.end())
		{
			add_neighbours_to_pq(best_cand,candpq_merge);
			duplicate_set.insert(key);
		}
	}
}

void SentenceTranslator::merge_subcands_and_add_to_pq(const Cand &cand_lhs, const Cand &cand_rhs,int rank_lhs,int rank_rhs,Candpq &candpq_merge)
{
	double mono_reorder_prob = 0;
	double swap_reorder_prob = 0;
	if (cand_rhs.end - cand_lhs.beg < para.REORDER_WINDOW)
	{
		pair<double> reorder_probs = cal_reorder_score(cand_lhs,cand_rhs);
		mono_reorder_prob = reorder_probs.first;
		swap_reorder_prob = reorder_probs.second;
	}
	
	Cand cand_mono;
	cand_mono.beg = cand_lhs.beg;
	cand_mono.end = cand_rhs.end;
	cand_mono.tgt_word_num = cand_lhs.tgt_word_num + cand_rhs.tgt_word_num;
	cand_mono.phrase_num = cand_lhs.phrase_num + cand_rhs.phrase_num;
	cand_mono.mono_reorder_prob = cand_lhs.mono_reorder_prob + cand_rhs.mono_reorder_prob + mono_reorder_prob;
	cand_mono.swap_reorder_prob = cand_lhs.swap_reorder_prob + cand_rhs.swap_reorder_prob + swap_reorder_prob;
	cand_mono.mid = cand_rhs.beg;
	cand_mono.rank_lhs = rank_lhs;
	cand_mono.rank_rhs = rank_rhs;
	cand_mono.tgt_wids = cand_lhs.tgt_wids;
	cand_mono.tgt_wids.insert(cand_mono.tgt_wids.end(),cand_rhs.tgt_wids.begin(),cand_rhs.tgt_wids.end());
	//cand_mono.tgt_str = cand_lhs.tgt_str + " " + cand_rhs.tgt_str;
	for (size_t i=0;i<PROB_NUM;i++)
	{
		cand_mono.trans_probs.push_back(cand_lhs.trans_probs.at(i)+cand_rhs.trans_probs.at(i));
	}
	//cand_mono.left_ant = &cand_lhs;
	//cand_mono.right_ant = &cand_rhs;
	double increased_lm_prob = cal_increased_lm_score_for_sen_frag(cand_mono);
	cand_mono.lm_prob = cand_lhs.lm_prob + cand_rhs.lm_prob + increased_lm_prob;
	//NOTE: add context based translation score here
	cand_mono.score = cand_lhs.score + cand_rhs.score + feature_weight.lm*increased_lm_prob + feature_weight.reorder_mono*mono_reorder_prob;
	candpq_merge.push(cand_mono);

	if (cand_rhs.end - cand_lhs.beg >= para.REORDER_WINDOW)
		return;
	Cand cand_swap = cand_mono;
	cand_swap.tgt_wids = cand_rhs.tgt_wids;
	cand_swap.tgt_wids.insert(cand_swap.tgt_wids.end(),cand_lhs.tgt_wids.begin(),cand_lhs.tgt_wids.end());
	//cand_swap.tgt_str = cand_rhs.tgt_str + " " + cand_lhs.tgt_str;
	increased_lm_prob = cal_increased_lm_score_for_sen_frag(cand_swap);
	cand_swap.lm_prob = cand_lhs.lm_prob + cand_rhs.lm_prob + increased_lm_prob;
	//NOTE: add context based translation score here
	cand_swap.score = cand_lhs.score + cand_rhs.score + feature_weight.lm*increased_lm_prob + feature_weight.reorder_mono*swap_reorder_prob;
	candpq_merge.push(cand_swap);
}

void SentenceTranslator::add_neighbours_to_pq(Cand &cur_cand, Candpq &candpq_merge)
{
	int beg = cur_cand.beg;
	int end = cur_cand.end;
	int mid = cur_cand.mid;
	int span_lhs = mid - beg - 1;
	int span_rhs = end - mid;

	int rank_lhs = cur_cand.rank_lhs + 1;
	int rank_rhs = cur_cand.rank_rhs;
	if(candpq_matrix.at(beg).at(span_lhs).size() >= rank_lhs)
	{
		const Cand &cand_lhs = candpq_matrix.at(beg).at(span_lhs).at(rank_lhs-1);
		const Cand &cand_rhs = candpq_matrix.at(mid).at(span_rhs).at(rank_rhs-1);
		merge_subcands_and_add_to_pq(cand_lhs,cand_rhs,rank_lhs,rank_rhs,candpq_merge);
	}

	rank_lhs = cur_cand.rank_lhs;
	rank_rhs = cur_cand.rank_rhs + 1;
	if(candpq_matrix.at(mid).at(span_rhs).size() >= rank_rhs)
	{
		const Cand &cand_lhs = candpq_matrix.at(beg).at(span_lhs).at(rank_lhs-1);
		const Cand &cand_rhs = candpq_matrix.at(mid).at(span_rhs).at(rank_rhs-1);
		merge_subcands_and_add_to_pq(cand_lhs,cand_rhs,rank_lhs,rank_rhs,candpq_merge);
	}
}
