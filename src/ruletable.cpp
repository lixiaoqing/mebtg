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
		vector<int> src_wids;
		src_wids.resize(src_rule_len);
		fin.read((char*)&src_wids[0],sizeof(int)*src_rule_len);

		short int tgt_rule_len=0;
		fin.read((char*)&tgt_rule_len,sizeof(short int));
		if (tgt_rule_len > RULE_LEN_MAX)
			continue;
		TgtRule tgt_rule;
		tgt_rule.word_num = tgt_rule_len;
		tgt_rule.wids.resize(tgt_rule_len);
		fin.read((char*)&(tgt_rule.wids[0]),sizeof(int)*tgt_rule_len);

		tgt_rule.probs.resize(PROB_NUM);
		fin.read((char*)&(tgt_rule.probs[0]),sizeof(double)*PROB_NUM);
		for(auto &e : tgt_rule.probs)
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

			tgt_rule.s2t_pos_map.resize(src_rule_len);
			for(size_t i=0;i<alignment_num/2;i++)
			{
				int ch_pos = alignment_array[2*i];
				int en_pos = alignment_array[2*i+1];
				tgt_rule.s2t_pos_map[ch_pos].push_back(en_pos);
			}
		}


		tgt_rule.score = 0;
		if( tgt_rule.probs.size() != weight.trans.size() )
		{
			cout<<"number of probability in rule is wrong!"<<endl;
		}
		for( size_t i=0; i<weight.trans.size(); i++ )
		{
			tgt_rule.score += tgt_rule.probs[i]*weight.trans[i];
		}
		tgt_rule.src_sense_ids = src_wids;

		add_rule_to_trie(src_wids,tgt_rule);
	}
	fin.close();
	cout<<"load rule table file "<<rule_table_file<<" over\n";
}

/**************************************************************************************
 1. 函数功能: 为以pos为起始位置的所有前缀寻找匹配的翻译规则
 2. 入口参数: 源语言句子的词义矩阵; 起始位置
 3. 出口参数: 每个span匹配到的所有规则
 4. 算法简介: 1) 从前往后生成所有能够匹配的词义组合
              2) 取出每个匹配的词义组合对应的翻译规则
************************************************************************************* */
vector<vector<TgtRule*> > RuleTable::find_matched_rules_for_prefixes(const vector<vector<int> > &src_sense_id_matrix,const size_t pos)
{
	vector<vector<TgtRule*> > matched_rules_for_prefixes;
	vector<vector<RuleTrieNode*> > matched_trienodes_for_prefixes;                                //记录每个前缀匹配到的所有Trie节点
	matched_trienodes_for_prefixes.push_back({root});
	for (size_t i=pos;i<src_sense_id_matrix.size() && i-pos<RULE_LEN_MAX;i++)
	{
		auto &pre_trienodes = matched_trienodes_for_prefixes.at(i-pos);
		vector<RuleTrieNode*> cur_trienodes;
		auto &cur_sense_id_vec = src_sense_id_matrix.at(i);
		for (size_t j=0; j<pre_trienodes.size(); j++)                                             //遍历前一层匹配上的Trie节点
		{	
			for (size_t k=0; k<cur_sense_id_vec.size(); k++)                                      //遍历当前词的所有词义
			{
				auto &subtrie_map = pre_trienodes.at(j)->id2subtrie_map; 
				auto it = subtrie_map.find( cur_sense_id_vec.at(k) );
				if ( it != subtrie_map.end() )
				{
					cur_trienodes.push_back(it->second);
				}
				else
					continue;
			}
		}
		if ( !cur_trienodes.empty() )
		{
			matched_trienodes_for_prefixes.push_back(cur_trienodes);
		}
		else
			break;
	}

	for (size_t i=1; i<matched_trienodes_for_prefixes.size(); i++)                                //跳过i=0(root节点)
	{
		auto &trienodes_for_cur_span = matched_trienodes_for_prefixes.at(i);
		vector<TgtRule*> rules_for_cur_span;
		for (size_t j=0; j<trienodes_for_cur_span.size(); j++)                                    //收集当前span所有匹配上的Trie节点包含的规则
		{
			for (size_t k=0; k<trienodes_for_cur_span.at(j)->tgt_rules.size(); k++)
			{
				rules_for_cur_span.push_back( &trienodes_for_cur_span.at(j)->tgt_rules.at(k) );
			}
		}
		if ( !rules_for_cur_span.empty() )
		{
			matched_rules_for_prefixes.push_back(rules_for_cur_span);
		}
		else
		{
			matched_rules_for_prefixes.push_back({NULL});
		}
	}
	return matched_rules_for_prefixes;
}

void RuleTable::add_rule_to_trie(const vector<int> &src_wids, const TgtRule &tgt_rule)
{
	RuleTrieNode* current = root;
	for (const auto &wid : src_wids)
	{        
		auto it = current->id2subtrie_map.find(wid);
		if ( it != current->id2subtrie_map.end() )
		{
			current = it->second;
		}
		else
		{
			RuleTrieNode* tmp = new RuleTrieNode();
			current->id2subtrie_map.insert(make_pair(wid,tmp));
			current = tmp;
		}
	}
	if (current->tgt_rules.size() < RULE_NUM_LIMIT)
	{
		current->tgt_rules.push_back(tgt_rule);
	}
	else
	{
		auto it = min_element(current->tgt_rules.begin(), current->tgt_rules.end());
		if( it->score < tgt_rule.score )
		{
			(*it) = tgt_rule;
		}
	}
}
