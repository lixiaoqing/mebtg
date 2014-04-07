#include "Candidate.h"


bool 
CandTrans::operator <(const CandTrans &right) const
{
	return prob<right.prob;
}

bool 
CandTrans::operator >(const CandTrans &right) const
{
	return prob>right.prob;
}

CandTrans&
CandTrans::operator =(const CandTrans &right)
{
	preHypo = right.preHypo;
	feats = right.feats;
	sentence = right.sentence;
	prob = right.prob;
	return *this;
}

