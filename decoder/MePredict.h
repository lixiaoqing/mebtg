#ifndef MEPREDICT_H
#define MEPREDICT_H

#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <sstream>
#include <iostream>
//#include <strstream>

#include <fstream>
#include <map>
#include <assert.h>
#include <ctype.h>
#include <string>
#include <algorithm>
#include <cstdlib>
#include <ctype.h>
#include <cstddef>
#include <vector>
#include <ctime>
#include <set>
#include <math.h>

#include <functional>

using namespace std;
vector< string > tokenize( string input );

class ReorderModel{
public:
	double correctionConstant;//修正常数
	double correctionParam;
	double faval;
	int classcount;
	vector< string > classname;
	map< string, int > featsMap;
	vector< vector< pair<int, int> > > parameters;
	vector< double > weights;
    int featcount;
    map< string,int > chVocab;
	int inOrder; //1:straight reverted; 0:reverted straight
	~ReorderModel()
	{
		classname.clear();
		featsMap.clear();
		parameters.clear();
		weights.clear();
	}
	vector< double > predict(vector< string > &context);
	bool ReadFile(const char* reorderFileName); 
	string Int2Str(int a)
	{
		ostringstream s;
		s << a;

		return s.str();	//将int转化为string
	}
};

#endif
