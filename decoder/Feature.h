
#ifndef FEATURE_H
#define FEATURE_H

#include "StdAfx.h"

class Feature 
{
public:
	Feature();
	void Init(int FeatNum);
	Feature operator+(const Feature& right);
	Feature& operator=(const Feature& right);
	Feature operator-(const Feature& right);

	vector<double> featFunc;             //ÿ�������Ĵ��
};

#endif
