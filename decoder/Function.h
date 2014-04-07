
#ifndef FUNCTION_H
#define FUNCTION_H

#include "StdAfx.h"


/************************************************************************/
/* the vector keeps n-best entries                                      */
/************************************************************************/
template<class tc>
void insert_to_vec(vector<tc> &v, int n, tc &t)
{
	if ((int)v.size() < n)
	{
		v.push_back(t);
	}
	else
	{	
		typename vector<tc>::iterator it = min_element(v.begin(), v.end());
		if( (*it) < t )
		{
			(*it) = t;
		}
	}
}

/************************************************************************/
/* pop max items                                                        */
/************************************************************************/
template<class tc>
tc popmax_from_vec(vector<tc> &v)
{
	tc t;
	typename vector<tc>::iterator it = max_element(v.begin(), v.end());
	t = (*it);
	v.erase(it);
	return t;
}


#endif
