#include "stdafx.h"
#include "datastruct.h"

struct TgtRule
{
	bool operator<(const TgtRule &rhs) const{return score<rhs.score;};
	int word_num;
	vector<int> word_id_list;
	double score;
	vector<double> prob_list;
	vector<vector<int> > ch_pos_to_en_pos_list;
};

struct RuleTrieNode 
{
	vector<TgtRule> tgt_rule_list;
	map <int, RuleTrieNode*> id2chilren_map;
};

class RuleTable
{
	public:
		RuleTable(int size_limit,Weight weight,const string &rule_table_file)
		{
			SIZE_LIMIT=size_limit;
			m_weight=weight;
			root=new RuleTrieNode;
			load_rule_table(rule_table_file);
		};
		vector<vector<TgtRule>* > find_matched_rules_for_prefixes(vector<int> &src_word_id_list,size_t pos);

	private:
		void load_rule_table(const string &rule_table_file);
		void add_rule_to_trie(const vector<int> &src_word_id_list, const TgtRule &tgt_rule);

	private:
		int SIZE_LIMIT;
		RuleTrieNode *root;
		Weight m_weight;
};
