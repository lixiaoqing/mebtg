#include "ruletable.h"

void RuleTable::load_rule_table(const string &rule_table_file)
{
	ifstream fin(rule_table_file.c_str(),ios::binary);
	if (!fin.is_open())
	{
		cerr<<"cannot open rule table file!\n";
		return;
	}
	cout<<"loading rule table file: "<<rule_table_file<<endl;
	short int src_rule_len=0;
	while(fin.read((char*)&src_rule_len,sizeof(short int)))
	{
		vector<int> src_rule;
		src_rule.resize(src_rule_len+1);
		fin.read((char*)&src_rule[0],sizeof(int)*src_rule_len);
		src_rule[src_rule_len] = END_ID;

		short int tgt_rule_len=0;
		fin.read((char*)&tgt_rule_len,sizeof(short int));
		if (tgt_rule_len > RULE_LEN_MAX)
			continue;
		TgtRule tgt_rule;
		tgt_rule.word_num = tgt_rule_len;
		tgt_rule.word_id_list.resize(tgt_rule_len+1);
		fin.read((char*)&(tgt_rule.word_id_list[0]),sizeof(int)*tgt_rule_len);
		tgt_rule.word_id_list[tgt_rule_len] = END_ID;

		tgt_rule.prob_list.resize(PROB_NUM);
		fin.read((char*)&(tgt_rule.prob_list[0]),sizeof(double)*PROB_NUM);
		for(auto &e : tgt_rule.prob_list)
		{
			if( e == 0.0 )
			{
				e = LogP_PseudoZero;
			}
			else
			{
				e = log10(e);
			}
		}

		short int alignment_num=0;
		fin.read((char*)&alignment_num,sizeof(short int));
		int *alignment_array = new int[alignment_num];
		fin.read((char*)alignment_array,sizeof(int)*alignment_num);

		tgt_rule.ch_pos_to_en_pos_list.resize(src_rule_len);
		for(size_t i=0;i<alignment_num/2;i++)
		{
			int ch_pos = alignment_array[2*i];
			int en_pos = alignment_array[2*i+1];
			tgt_rule.ch_pos_to_en_pos_list[ch_pos].push_back(en_pos);
		}


		tgt_rule.score = 0;
		if( tgt_rule.prob_list.size() != m_weight.trans.size() )
		{
			cout<<"number of probability in rule is wrong!"<<endl;
		}
		for( size_t i=0; i<m_weight.trans.size(); i++ )
		{
			tgt_rule.score += tgt_rule.prob_list[i]*m_weight.trans[i];
		}

		add_rule_to_trie(src_rule,tgt_rule);
	}
	fin.close();
}

vector<vector<TgtRule>* > RuleTable::find_matched_rules_for_spans(vector<int> &src_rule,size_t pos)
{
	vector<vector<TgtRule>* > matched_rules_for_spans;
	TrieNode* current = root;
	for (size_t i=pos;i<src_rule.size() && i-pos<RULE_LEN_MAX;i++)
	{
		auto it = current->id2chilren_map.find(src_rule.at(i));
		if (it != current->id2chilren_map.end())
		{
			current = it->second;
			if (current->tgt_rule_list.size() == 0)
			{
				matched_rules_for_spans.push_back(NULL);
			}
			else
			{
				matched_rules_for_spans.push_back(&(current->tgt_rule_list));
			}
		}
		else
		{
			return matched_rules_for_spans;
		}
	}
	return matched_rules_for_spans;
}

void RuleTable::add_rule_to_trie(const vector<int> &src_rule, const TgtRule &tgt_rule)
{
	TrieNode* current = root;
	for (const auto &word_id : src_rule)
	{        
		auto it = current->id2chilren_map.find(word_id);
		if ( it != current->id2chilren_map.end() )
		{
			current = it->second;
		}
		else
		{
			TrieNode* tmp = new TrieNode();
			current->id2chilren_map.insert(make_pair(word_id,tmp));
			current = tmp;
		}
	}
	if (current->tgt_rule_list.size() < m_size_limit)
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
