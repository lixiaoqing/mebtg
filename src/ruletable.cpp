#include "ruletable.h"

void RuleTable::load_rule_table(const string &rule_table_file)
{
	ifstream fin(rule_table_file.c_str(),ios::binary);
	if (!fin.is_open())
	{
		cerr<<"cannot open rule table file!\n";
		return;
	}

	int phrase_num,src_wids_len,tgt_wids_len;
	fin.read((char*)&phrase_num,sizeof(int));
	fin.read((char*)&src_wids_len,sizeof(int));
	fin.read((char*)&tgt_wids_len,sizeof(int));
	vector<int> src_phrase_lens(phrase_num,0);
	vector<int> src_wids(src_wids_len,0);
	vector<int> tgt_phrase_lens(phrase_num,0);
	vector<int> tgt_wids(tgt_wids_len,0);
	vector<double> probs(phrase_num*4,0.0);
	fin.read((char*)&src_phrase_lens[0],sizeof(int)*phrase_num);
	fin.read((char*)&src_wids[0],sizeof(int)*src_wids_len);
	fin.read((char*)&tgt_phrase_lens[0],sizeof(int)*phrase_num);
	fin.read((char*)&tgt_wids[0],sizeof(int)*tgt_wids_len);
	fin.read((char*)&probs[0],sizeof(double)*phrase_num*PROB_NUM);

	vector<int> alignment_nums;
	vector<int> src_alignments;
	vector<int> tgt_alignments;
	if (LOAD_ALIGNMENT == true)
	{
		alignment_nums.resize(phrase_num,0);
		fin.read((char*)&alignment_nums[0],sizeof(int)*phrase_num);
		int alignments_len;
		fin.read((char*)&alignments_len,sizeof(int));
		src_alignments.resize(alignments_len,0);
		tgt_alignments.resize(alignments_len,0);
		fin.read((char*)&src_alignments[0],sizeof(int)*alignments_len);
		fin.read((char*)&tgt_alignments[0],sizeof(int)*alignments_len);
	}
	size_t src_begin=0,tgt_begin=0,align_begin=0;
	for (size_t i=0;i<phrase_num;i++)
	{
		vector<int> t_src_wids(src_wids.begin()+src_begin,src_wids.begin()+src_begin+src_phrase_lens.at(i));
		vector<int> t_tgt_wids(tgt_wids.begin()+tgt_begin,tgt_wids.begin()+tgt_begin+tgt_phrase_lens.at(i));
		src_begin += src_phrase_lens.at(i);
		tgt_begin += tgt_phrase_lens.at(i);
		TgtRule t_tgt_rule;
		t_tgt_rule.word_num = tgt_phrase_lens.at(i);
		t_tgt_rule.wids = t_tgt_wids;
		t_tgt_rule.prob_list.resize(PROB_NUM);
		for (size_t j=0;j<PROB_NUM;j++)
		{
			t_tgt_rule.prob_list.at(j) = (abs(probs.at(i*PROB_NUM+j))<=numeric_limits<double>::epsilon()?LogP_PseudoZero:log10(probs.at(i*PROB_NUM+j)));
		}
		if (LOAD_ALIGNMENT == true)
		{
			for(size_t j=0;j<alignment_nums.at(i);j++)
			{
				t_tgt_rule.ch_pos_to_en_pos_list[src_alignments.at(align_begin+j)].push_back(tgt_alignments.at(align_begin+j));
			}
			align_begin += alignment_nums.at(i);
		}
		t_tgt_rule.score = 0;
		for( size_t j=0; j<weight.trans.size(); j++ )
		{
			t_tgt_rule.score += t_tgt_rule.prob_list.at(j)*weight.trans.at(j);
		}

		add_rule_to_trie(t_src_wids,t_tgt_rule);
	}

	fin.close();
	cout<<"load rule table file "<<rule_table_file<<" over\n";
}

vector<vector<TgtRule>* > RuleTable::find_matched_rules_for_prefixes(const vector<int> &src_wids,const size_t pos)
{
	vector<vector<TgtRule>* > matched_rules_for_prefixes;
	RuleTrieNode* current = root;
	for (size_t i=pos;i<src_wids.size() && i-pos<RULE_LEN_MAX;i++)
	{
		auto it = current->id2chilren_map.find(src_wids.at(i));
		if (it != current->id2chilren_map.end())
		{
			current = it->second;
			if (current->tgt_rule_list.size() == 0)
			{
				matched_rules_for_prefixes.push_back(NULL);
			}
			else
			{
				matched_rules_for_prefixes.push_back(&(current->tgt_rule_list));
			}
		}
		else
		{
			matched_rules_for_prefixes.push_back(NULL);
			return matched_rules_for_prefixes;
		}
	}
	return matched_rules_for_prefixes;
}

void RuleTable::add_rule_to_trie(const vector<int> &src_wids, const TgtRule &tgt_rule)
{
	RuleTrieNode* current = root;
	for (const auto &word_id : src_wids)
	{        
		auto it = current->id2chilren_map.find(word_id);
		if ( it != current->id2chilren_map.end() )
		{
			current = it->second;
		}
		else
		{
			RuleTrieNode* tmp = new RuleTrieNode();
			current->id2chilren_map.insert(make_pair(word_id,tmp));
			current = tmp;
		}
	}
	if (current->tgt_rule_list.size() < RULE_NUM_LIMIT)
	{
		current->tgt_rule_list.push_back(tgt_rule);
	}
	else
	{
		auto it = min_element(current->tgt_rule_list.begin(), current->tgt_rule_list.end());
		if( it->score < tgt_rule.score )
		{
			(*it) = tgt_rule;
		}
	}
}
