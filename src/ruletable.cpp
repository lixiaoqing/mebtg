#include "ruletable.h"

void RuleTable::load_rule_table(const string &rule_table_file)
{
	ifstream fin(rule_table_file.c_str(),ios::binary);
	if (!fin.is_open())
	{
		cerr<<"cannot open rule table file!\n";
		return;
	}
	short int src_rule_len=0;
	while(fin.read((char*)&src_rule_len,sizeof(short int)))
	{
		vector<int> src_word_id_list;
		src_word_id_list.resize(src_rule_len);
		fin.read((char*)&src_word_id_list[0],sizeof(int)*src_rule_len);

		short int tgt_rule_len=0;
		fin.read((char*)&tgt_rule_len,sizeof(short int));
		if (tgt_rule_len > RULE_LEN_MAX)
			continue;
		TgtRule tgt_rule;
		tgt_rule.word_num = tgt_rule_len;
		tgt_rule.word_id_list.resize(tgt_rule_len);
		fin.read((char*)&(tgt_rule.word_id_list[0]),sizeof(int)*tgt_rule_len);

		tgt_rule.prob_list.resize(PROB_NUM);
		fin.read((char*)&(tgt_rule.prob_list[0]),sizeof(double)*PROB_NUM);
		for(auto &e : tgt_rule.prob_list)
		{
			if( abs(e) <= numeric_limits<double>::epsilon() )
			{
				e = LogP_PseudoZero;
			}
			else
			{
				e = log10(e);
			}
		}

		if (LOAD_ALIGNMENT == true)
		{
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
		}


		tgt_rule.score = 0;
		if( tgt_rule.prob_list.size() != weight.trans.size() )
		{
			cout<<"number of probability in rule is wrong!"<<endl;
		}
		for( size_t i=0; i<weight.trans.size(); i++ )
		{
			tgt_rule.score += tgt_rule.prob_list[i]*weight.trans[i];
		}

		add_rule_to_trie(src_word_id_list,tgt_rule);
	}
	fin.close();
	cout<<"load rule table file "<<rule_table_file<<" over\n";
}

vector<vector<TgtRule>* > RuleTable::find_matched_rules_for_prefixes(const vector<int> &src_word_id_list,const size_t pos)
{
	vector<vector<TgtRule>* > matched_rules_for_prefixes;
	RuleTrieNode* current = root;
	for (size_t i=pos;i<src_word_id_list.size() && i-pos<RULE_LEN_MAX;i++)
	{
		auto it = current->id2chilren_map.find(src_word_id_list.at(i));
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

void RuleTable::add_rule_to_trie(const vector<int> &src_word_id_list, const TgtRule &tgt_rule)
{
	RuleTrieNode* current = root;
	for (const auto &word_id : src_word_id_list)
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
