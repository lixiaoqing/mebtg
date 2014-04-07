
#ifndef CANDIDATE_H
#define CANDIDATE_H

#include "StdAfx.h"
#include "Feature.h"
#include "HypoStack.h"



class CandTrans
{
public:
	s_Hypotheses* preHypo;
	vector<int> sentence;			//�ʺű�ʾ�ľ���
	Feature feats;					//�������
	double prob;					//�ܴ��

	bool operator<(const CandTrans &right)const;
	bool operator>(const CandTrans &right)const;
	CandTrans& operator=(const CandTrans &right);

};


#endif

