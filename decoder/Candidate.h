
#ifndef CANDIDATE_H
#define CANDIDATE_H

#include "StdAfx.h"
#include "Feature.h"
#include "HypoStack.h"



class CandTrans
{
public:
	s_Hypotheses* preHypo;
	vector<int> sentence;			//词号表示的句子
	Feature feats;					//特征打分
	double prob;					//总打分

	bool operator<(const CandTrans &right)const;
	bool operator>(const CandTrans &right)const;
	CandTrans& operator=(const CandTrans &right);

};


#endif

