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
		LanguageModel(int lm_order_in_mt,const string &lm_file){LM_ORDER_IN_MT=lm_order_in_mt;root=new NgramTrieNode;load_lm(lm_file);};
		void load_lm(const string &lm_file);
		double eval_word_id_list(const vector<int> &word_id_list);
	private:
		NgramTrieNode* search_matched_path(const vector<int> &word_id_list);
		void add_bow_to_trie(const vector<int> &word_id_list,double bow);
		void add_prob_to_trie(const vector<int> &word_id_list,double prob);

	private:
		//Vocab *tgt_vocab;
		NgramTrieNode *root;
		int LM_ORDER_IN_MT;
		int START_ORDER_FOR_EVAL;
};
