#include "MePredict.h"

vector<string> tokenize( string input ) {
	vector< string > token;
	bool betweenWords = true;
	int start=0;
	int i=0;
	string word;
	for(; input[i] != '\0'; i++) 
	{
		bool isSpace = (input[i] == ' ' || input[i] == '\t');

		if (!isSpace && betweenWords) 
		{
			start = i;
			betweenWords = false;
		}
		else if (isSpace && !betweenWords) 
		{
			word=input.substr(start, i-start);
			token.push_back( word );
			betweenWords = true;
		}
	}
	if (!betweenWords)
	{   
		word = input.substr(start, i-start);
		token.push_back(word );
	}
	return token;
}

vector<double> ReorderModel::predict(vector< string > &context)
{
	if( 4 != context.size() && 8 != context.size() )
	{
		cerr<<"The input features are ERROR!"<<endl;
		exit(-1);
	}
	vector< string > newcontext;
	string feat1, feat2, feat3, feat4, feat5, feat6, feat7, feat8;

	feat1="c11=" + context[0];
	newcontext.push_back(feat1);

	feat2="c21=" + context[1];
	newcontext.push_back(feat2);

	feat3="e11=" + context[2];
	newcontext.push_back(feat3);

	feat4="e21=" + context[3];
	newcontext.push_back(feat4);

	if( 8 == context.size() )
	{
		feat5="c12=" + context[4];
		newcontext.push_back(feat5);

		feat6="c22=" + context[5];
		newcontext.push_back(feat6);

		feat7="e12=" + context[6];
		newcontext.push_back(feat7);

		feat8="e22=" + context[7];
		newcontext.push_back(feat8);
	}

	double normal = 0.0;
	vector< double > probs;
	for (int oid=0; oid<classcount; oid++) 
	{
		probs.push_back(0);
	}

	for (int i = 0; i < newcontext.size(); i++)
	{
		//cout<<newtest[i]<<endl;
		int featId;
		map< string, int >::iterator featit = featsMap.find(newcontext[i]);
		if( featit != featsMap.end() )
		{
			featId = featit->second;
			vector< pair<int, int> >& params = parameters[featId];
			for( int j = 0; j < params.size(); j++ )
			{
				probs[params[j].first] += weights[params[j].second];
			}
		}
	}

	double sum = 0.0;
	for( int i = 0; i < probs.size(); i++ )
	{
		probs[i] = exp(probs[i]);
		sum  += probs[i];
	}
	
	for( int i = 0; i < probs.size(); i++ )
		probs[i] /= sum;

	return probs;
}

bool ReorderModel::ReadFile(const char* reorderFileName) 
{
	ifstream infile(reorderFileName);
	if(!infile)cerr << "WARNING: " << " model file cannot be read.\n";
	int id = 0;
	string line,word;
	vector<string> tmpfeatgroup;

	cout<<"Reading Reorder File..."<<endl;

	//skip head comments
	getline(infile, line);
	while( line.empty() || '#' == line[0] )
		getline(infile, line);

	//load features
	featcount = atoi( line.c_str() );
	for( int i = 0; i < featcount; i++ )
	{
		getline(infile, line);
		featsMap.insert(make_pair(line, i));
	}

	//load outcomes
	getline(infile, line);
	classcount = atoi( line.c_str() );
	for( int i = 0; i < classcount; i++ )
	{
		getline(infile, line);
		classname.push_back(line);
	}
	if( "straight" == classname[0] )
		inOrder = 1;
	else
		inOrder = 0;

	//load parameters
	int weightId = 0;
	vector< pair<int, int> > params;
	for( int i = 0; i < featcount; i++ )
	{
		params.clear();
		getline(infile, line);
		vector<string> tmpParams = tokenize(line);
		for( int j = 1; j < tmpParams.size(); j++ )
		{
			int cid = atoi(tmpParams[j].c_str());
			params.push_back(make_pair(cid, weightId++));
		}
		parameters.push_back(params);
	}

	//load weights
	getline(infile, line);
	int weightcount = atoi(line.c_str());
	assert(weightId == weightcount);
	for( int i = 0; i < weightcount; i++ )
	{
		getline(infile, line);
		weights.push_back(atof(line.c_str()));
	}

	infile.close();

	return true;
}
