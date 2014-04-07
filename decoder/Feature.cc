#include "Feature.h"


Feature::Feature()
{
	featFunc.clear();
}
void
Feature::Init(int featNum)
{
	featFunc.clear();
	featFunc.resize(featNum);
	fill(featFunc.begin(),featFunc.end(),0.0);
}

Feature
Feature::operator+(const Feature &right)
{
	Feature feat;
	feat.Init(right.featFunc.size());
	for (int i=0; i<(int)featFunc.size(); i++)
	{
		feat.featFunc[i] = featFunc[i] + right.featFunc[i]; 
	}
	return feat;
}

Feature
Feature::operator-(const Feature &right)
{
	Feature feat;
	feat.Init(right.featFunc.size() );
	for( int i=0; i<(int)featFunc.size(); i++ )
	{
		feat.featFunc[i] = featFunc[i] - right.featFunc[i];
	}
	return feat;
}

Feature&
Feature::operator =(const Feature &right)
{
	featFunc.clear();
	featFunc = right.featFunc;
	return *this;
}



