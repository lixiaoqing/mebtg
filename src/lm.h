#include "stdafx.h"
#include "lm/model.hh"
using namespace lm::ngram;

class LanguageModel
{
	public:
		LanguageModel(const string &lm_file)
		{
			kenlm = new Model(lm_file.c_str());
			cout<<"load language model file "<<lm_file<<" over\n";
		};
		double eval(const vector<string> &words);

	private:
		Model *kenlm;
};
