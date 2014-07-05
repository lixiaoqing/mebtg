#include "stdafx.h"

struct NgramTrieNode
{
	double bow;
	map<int,double> probs;
	map<int,NgramTrieNode*> id2chilren_map;
};

class LanguageModel
{
	public:
		LanguageModel(const string &lm_file){root=new NgramTrieNode;load_lm(lm_file);};
		void load_lm(const string &lm_file);
		double eval(const vector<int> &wids);
	private:
		NgramTrieNode* search_or_create_path_in_trie(const vector<int> &wids);
		void add_bow_to_trie(const vector<int> &wids,double bow);
		void add_prob_to_trie(const vector<int> &wids,double prob);

	private:
		NgramTrieNode *root;
};
