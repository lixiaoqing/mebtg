#ifndef VOCAB_H
#define VOCAB_H

#include "stdafx.h"

class Vocab
{
	public:
		Vocab(const string &dict_file) {load_dict(dict_file);};
		string get_word(int id){return word_list.at(id);};
		int get_id(const string &word);
	private:
		void load_dict(const string &dict_file);
	private:
		vector<string> word_list;
		map<string,int> word2id;
};

#endif
