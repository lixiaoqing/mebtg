#include "stdafx.h"
#include "lm/model.hh"
using namespace lm::ngram;

class LanguageModel
{
	public:
		LanguageModel(const string &lm_file){kenlm = new Model(lm_file.c_str());};
		double eval(const vector<string> &words);

	private:
		Model *kenlm;
};
