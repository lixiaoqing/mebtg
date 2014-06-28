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
					cand.tgt_word_id_list = {tgt_vocab->get_id("NULL")};
					cand.trans_prob_list.resize(PROB_NUM,LogP_PseudoZero);
					cand.score = 0;
					for (size_t i=0;i<PROB_NUM;i++)
					{
						cand.score += feature_weight.trans.at(i)*cand.trans_prob_list.at(i);
					}
					cand.phrase_num = 1;
					cand.lm_prob = lm_model->cal_lm_score(cand.tgt_word_id_list);
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
				cand.lm_prob = lm_model->cal_lm_score(cand.tgt_word_id_list);
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

string SentenceTranslator::generate_kbest_for_span(size_t span,size_t start_pos)
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
