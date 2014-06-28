#include "stdafx.h"

class MaxentModel
{
	public:
		void load(string &modelfile);
		double eval(vector<string> &context, string &tag);		
		void eval_all(vector<double> &me_scores, vector<string> &context);		
		int get_tagid(string tag);
	private:
		void load_bin(string &modelfile);
		void load_txt(string &modelfile);
		size_t feature_num;
		size_t tag_num;
		size_t lambda_num;
		map <string,size_t> feature2id;
		map <string,size_t> tag2id;
		vector <string> featureVec;
		vector <double> lambdaVec;
		vector <size_t> lambda2tagVec;
		vector <size_t> featureAddrVec;				
};


