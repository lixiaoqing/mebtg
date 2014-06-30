#include "stdafx.h"

struct NgramTrieNode  //word list are stored in reversed order
{
	double bow;  //bow for the word list from root to current node
	map<int,double> probs;  //p(last word|history),where history is the above word list
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
		//Vocab *tgt_vocab;
		NgramTrieNode *root;
};
