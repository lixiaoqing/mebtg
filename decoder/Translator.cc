/*
*Translator.cpp  - Phrase based decoder

* Copyright (C) 2006 by Chunguang Chai <cgchai@nlpr.ia.ac.cn> baidu

National Laboratory of Pattern Recognition, IA, CAS

* Begin       : 05/06/2007
* Last Change : 05/06/2007

*/


#include "StdAfx.h"
#include "Algorithm.h"


bool ParseArgs(int argc, char *argv[], Config &parameter);
void help();
bool ReadConfigFile(char* configfile, Config &parameter);

int main( int argc, char *argv[] )
{
	Config parameter;
	if( !ParseArgs(argc, argv, parameter) )
	{
		cerr<<"read parameters error!\n";
		return 0;
	}
	
	Algorithm* m_pAlgorithm;
	m_pAlgorithm = new Algorithm();
	if( !( m_pAlgorithm->Initialize(parameter) ) )
	{
		exit(0);
	}

 
	clock_t m_tBegin, m_tEnd;
	m_tBegin = clock();

	if( !(m_pAlgorithm->TranslateFile(parameter.testfilename.c_str(), parameter.resultfilename.c_str(), parameter.filetype, parameter.segmenttype) ) )
	{
		cout<<"translate error!"<<endl;
		exit(0);
	}
	
	m_tEnd = clock();
	cout<<"Total Time: "<<(double)( (m_tEnd-m_tBegin)/CLOCKS_PER_SEC) <<endl;

	delete m_pAlgorithm;
	
}

bool ParseArgs(int argc, char *argv[], Config &parameter)
{
	if( 1 == argc )
	{
		help();
		return false;
	}
	for( int i=1; i<argc; i++ )
	{
		string t( argv[i] );
		if( "-conf" == t )
		{
			if( !ReadConfigFile(argv[++i], parameter) )
			{
				return 0;
			}
		}
		else if( "-beam-hist" == t )
		{
			parameter.beam_hist = atoi( argv[++i] );
		}
		else if( "-beam-prob" == t )
		{
		    	parameter.beam_prob = atof( argv[++i] );
		}
		else if( "-ttable-limit" == t)
		{
			parameter.ttable = atoi( argv[++i] );
		}
		else if( "-nbest-list" == t )
		{
			parameter.nbest = atoi( argv[++i] );
		}		
		else if( "-test-file" == t )
		{
			parameter.nbestfilename = argv[++i];
		}
		else if( "-result-file" == t )
		{
			parameter.resultfilename = argv[++i];
		}
		else if( "-nbest-file" == t )
		{
			parameter.nbestfilename = argv[++i];
		}
		else if( "-trian" == t )
		{
			parameter.train = 1;
		}
		else if( "-help" == t)
		{
			help();
			return false;
		}

		else
		{
			cerr<<"Unknown option \""<<argv[i]<<"\""<<endl;
			cerr<<"\"search -help\" for information "<<endl;
			return 0;
		}

	}
	return 1;

}

void help()
{
	cerr << "Decoder v1.0 Command Line Help: \n\n";
	cerr << "-conf : specifies the configure file\n\n";
	cerr << "-stack: maximum size of the beam (default 300)\n\n";
	cerr << "-ttable-limit: number of phrase translations for each foreign phrase (default 10)\n\n";
	cerr << "-nbest-list: number of candidate translations for each input sentence (default 0)\n\n";
	cerr << "-test-file: specifies test file\n\n";
	cerr << "-result-file: specifies result file\n\n";
	cerr << "-nbest-file: specifies nbest-list file (if nbest-list is more than 1, nbest-list will be dumped to this file)\n\n";
			
}


bool ReadConfigFile(char* configfile, Config &parameter)
{
	ifstream inputfile( configfile );
	if( !inputfile )
	{
		cerr<<"Can not open the config file!\n";
		return 0;
	}
	string line,str1,str2;

	parameter.feats_num = 0;
	parameter.Lambda_trans.clear();
	while( getline(inputfile, line) )
	{
		int pos = line.find_first_of("#",0);
		if( pos != string::npos )
		{
			line.erase(pos, line.size()-pos);
		}
		

		istringstream buff(line);
		if( !(buff>>str1) )
		{
			continue;
		}
		
		if( 0 == str1.find("beam_hist=") )
		{
			parameter.beam_hist = atoi(str1.substr(10, -1).c_str());
		}		
		else if( 0 == str1.find("beam_prob=") ) 
		{
			parameter.beam_prob = atof(str1.substr(10, -1).c_str());
		}
		else if( 0 == str1.find("ttable_limit=") )
		{
			parameter.ttable = atoi(str1.substr(13, -1).c_str());
		}
		else if( 0 == str1.find("nbest_list=") )
		{
			parameter.nbest = atoi( str1.substr(11,-1).c_str() );
		}
		else if( 0 == str1.find("lm_file=") )
		{
			parameter.lmfilename = str1.substr(8,-1);
		}
		else if( 0 == str1.find("phrase_file=") )
		{
			parameter.phrasefilename = str1.substr(12, -1);
		}
		else if( 0 == str1.find("test_file=") )
		{
			parameter.testfilename = str1.substr(10, -1);
		}
		else if( 0 == str1.find("result_file=") )
		{
			parameter.resultfilename = str1.substr(12, -1);
		}
		else if( 0 == str1.find("nbest_file=") )
		{
			parameter.nbestfilename = str1.substr(11, -1);
		}
		else if( 0 == str1.find("reorder_model_file=") )
		{
			parameter.reorder_model_filename = str1.substr(19, -1);
		}
		else if( 0 == str1.find("context_model_list=") )
		{
			parameter.catalog_filename = str1.substr(19, -1);
		}
		else if( 0 == str1.find("source_vocab_file=") )
		{
			parameter.sourcevocabfilename = str1.substr(18, -1);
		}
		else if( 0 == str1.find("target_vocab_file=") )
		{
			parameter.targetvocabfilename = str1.substr(18, -1);
		}
		/*
		else if( 0 == str1.find("string_tag_file=") )
		{
			parameter.stringtagfilename = str1.substr(16, -1);
		}
		*/
		else if( 0 == str1.find("file_format=") )
		{
			str2 = str1.substr(12, -1);
			if( "XML" == str2 )
				parameter.filetype = XML;
			else 
				parameter.filetype = TXT;
		}
		else if( 0 == str1.find("segmentation_format=") )
		{
			str2 = str1.substr(20, -1);
			if( "NOSEG" ==  str2 )
				parameter.segmenttype = NOSEG;
			else if( "SEG" == str2 )
				parameter.segmenttype = SEG;
			else 
				parameter.segmenttype = CHARACTER;
		}
		else if( 0 == str1.find("trans_unknown_word=") )
		{
			str2 = str1.substr(19, -1);
			if( "NULLWORD" == str2  )
			{
				parameter.transUnknownWord = NULLWORD;
			}
			else 
			{
				parameter.transUnknownWord = SOURCEWORD;
			}
		}
		else if( 0 == str1.find("recombine=") )
		{
			str2 = str1.substr(10, -1);
			if( atoi( str2.c_str() ) )
				parameter.recombine = true;
			else
				parameter.recombine = false;
		}
		else if( 0 == str1.find("reduce_voc=") )
		{
			str2 = str1.substr(11, -1);
			if( atoi(str2.c_str() ) )
				parameter.reduceVoc = true;
			else 
				parameter.reduceVoc = false;
		}
		else if( 0 == str1.find("lm_order=") )
		{
			parameter.lmorder = atoi( str1.substr(9, -1).c_str() );
		}		
		else if( 0 == str1.find("threads_num=") )
		{
			parameter.threadsNum = atoi( str1.substr(12, -1).c_str() );
		}		
		else if( 0 == str1.find("reorder_window=") )
		{
			parameter.reorder_window = atoi( str1.substr(15, -1).c_str() );
		}		
		else if( 0 == str1.find("cube_flag=") )
		{
			parameter.cube_flag = atoi( str1.substr(10, -1).c_str() );
		}
		else if( 0 == str1.find("chn2eng=") )
		{
			parameter.chn2eng_flag = atoi( str1.substr(8, -1).c_str() );
		}
		else if( 0 == str1.find("chinese-disreorder-puncs=") )
		{
			parameter.g_Chinese_Punc_Set = str1.substr(25, -1);
		}
		else if( 0 == str1.find("chinese-split-puncs=") )
		{
			parameter.g_Chinese_Split_Puncs = str1.substr(20, -1);
		}
		else if( 0 == str1.find("train=") )
		{
			parameter.train = atoi( str1.substr(6, -1).c_str() );
		}
		else if( 0 == str1.find("kenLM=") )
		{
			int kenFlag = atoi(str1.substr(6,-1).c_str());
			if( 1 == kenFlag )
				parameter.KenLM = true;
			else 
				parameter.KenLM = false;
		}
		else if( "transWeight1" == str1 )
		{
			buff>>str2;
			parameter.Lambda_trans.push_back(atof(str2.c_str()));
			parameter.feats_num += 1; 
		}
		else if( "transWeight2" == str1 )
		{
			buff>>str2;
			parameter.Lambda_trans.push_back(atof(str2.c_str()));
			parameter.feats_num += 1; 
		}
		else if( "transWeight3" == str1 )
		{
			buff>>str2;
			parameter.Lambda_trans.push_back(atof(str2.c_str()));
			parameter.feats_num += 1; 
		}
		else if( "transWeight4" == str1 )
		{
			buff>>str2;
			parameter.Lambda_trans.push_back(atof(str2.c_str()));
			parameter.feats_num += 1; 
		}
		else if( "len" == str1 )
		{					
			buff>>str2;
			parameter.Lambda_len = atof(str2.c_str());
			parameter.feats_num += 1;
		}
		else if( "phraseNum" == str1 )
		{					
			buff>>str2;
			parameter.Lambda_dPhraseNum = atof(str2.c_str());
			parameter.feats_num += 1;
		}
		else if( "lm" == str1 )
		{		
			buff>>str2;
			parameter.Lambda_lm = atof(str2.c_str());
			parameter.feats_num += 1;
		}
		else if( "sense" == str1 )
		{		
			buff>>str2;
			parameter.Lambda_sense = atof(str2.c_str());
			parameter.feats_num += 1;
		}
		else if( "reorderStraight" == str1 )
		{
			buff>>str2;
			parameter.Lambda_reorderStraight = atof(str2.c_str());
			parameter.feats_num += 1;
		}
		else if( "reorderSwap" == str1 )
		{
			buff>>str2;
			parameter.Lambda_reorderSwap = atof(str2.c_str());
			parameter.feats_num += 1;
		}
		else if( 0 == str1.find("print_info=") )
		{
			if( atoi(str1.substr(11, -1).c_str()) )
				parameter.printInfo = true;
			else
				parameter.printInfo = false;
		}
		else if( 0 == str1.find("print_nbest=") )
		{
			if( 1 == atoi( str1.substr(12, -1).c_str()) )
				parameter.printNbest = true;
			else 
				parameter.printNbest = false;
		}
	}

	cerr<<parameter.feats_num<<endl;
	inputfile.close();

	return 1;
}

