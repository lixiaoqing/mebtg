#include "stdafx.h"

struct TgtRule
{
	bool operator<(const TgtRule &rhs) const{return score<rhs.score;};
	int word_num;
	vector<int> word_id_list;
	double score;
	vector<double> prob_list;
	vector<vector<int> > ch_pos_to_en_pos_list;
};

struct TrieNode 
{
	vector<TgtRule> tgt_rule_list;
	map <int, TrieNode*> id2chilren_map;
};

class RuleTable
{
	public:
		RuleTable(int size_limit,Weight weight){m_size_limit=size_limit;m_weight=weight;root=new TrieNode;};
		void load_rule_table(const string &rule_table_file);
		vector<vector<TgtRule>* > find_matched_rules_for_spans(vector<int> &src_rule,size_t pos);

	private:
		void add_rule_to_trie(const vector<int> &src_rule, const TgtRule &tgt_rule);

	private:
		int m_size_limit;
		TrieNode *root;
		Weight m_weight;
};
