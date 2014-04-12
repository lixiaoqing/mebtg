/*
*Algorithm.cpp  - Algorithm class, the kernel of decoder

* Copyright (C) 2009 by Jiajun Zhang <jjzhang@nlpr.ia.ac.cn> 

National Laboratory of Pattern Recognition, IA, CAS

* Begin       : 10/10/2009
* Last Change : 10/10/2009

*/



#include "Algorithm.h"
#include "MePredict.h"

void MaxentModel::load(string &modelfile)
{
	gzFile f = gzopen(modelfile.c_str(), "rb");
	if (f == NULL)
	{
		cerr<<"cannot open model file!\n";
		return;
	}

	char buf[100];
	gzread(f, (void*)buf, 16);
	buf[16] = '\0';
	string s = buf;

	if (s.find("txt") != s.npos) 
	{
		load_txt(modelfile);
	}
       	else if (s.find("bin") != s.npos) 
	{
		load_bin(modelfile);
	} 
	else
	{
		cerr<<"unknown model type!\n";
		return;
	}
	gzclose(f);
}

void MaxentModel::load_bin(string &modelfile)
{
	gzFile f;
	f = gzopen(modelfile.c_str(), "rb");
	if (f == NULL)
	{
		cerr<<"cannot open model file!\n";
		return;
	}
	//skip header
	gzseek(f, 16, 0);

	char buf[4096]; // TODO: handle unsafe buffer
	// read features
	gzread(f, (void*)&feature_num, sizeof(feature_num));
	size_t len;
	for (size_t i = 0; i < feature_num; ++i) 
	{
		gzread(f, (void*)&len, sizeof(len));
		gzread(f, (void*)&buf, len);
		feature2id.insert(make_pair(string(buf,len),i));
	}

	// read tags
	gzread(f, (void*)&tag_num, sizeof(tag_num));
	for (size_t i = 0; i < tag_num; ++i) 
	{
		gzread(f, (void*)&len, sizeof(len));
		gzread(f, (void*)&buf, len);
		tag2id.insert(make_pair(string(buf,len),i));
	}

	// read paramaters
	size_t tagid;
	featureAddrVec.resize(feature_num+1,0);
	for (size_t i = 0; i < feature_num; ++i) 
	{
		gzread(f, (void*)&len, sizeof(len));
		featureAddrVec.at(i+1) = featureAddrVec.at(i)+len;
		for (size_t j = 0; j < len; ++j) 
		{
			gzread(f, (void*)&tagid, sizeof(tagid));
			lambda2tagVec.push_back(tagid);
		}
	}

	// load theta
	gzread(f, (void*)&lambda_num, sizeof(lambda_num));

	double lambda;
	for (size_t i = 0; i < lambda_num; ++i) 
	{
		gzread(f, (void*)&lambda, sizeof(lambda));
		lambdaVec.push_back(lambda);
	}
	gzclose(f);
}

void MaxentModel::load_txt(string &modelfile)
{
	ifstream fin;
	fin.open(modelfile.c_str());
	if(!fin.is_open())
	{
		cerr<<"open MaxEnt model error!"<<endl;
		return;
	}
	string s;
	getline(fin,s);
	getline(fin,s);
	feature_num = s2i(s);
	for (int i = 0;i<feature_num;i++)
	{
		getline(fin,s);
		TrimLine(s);
		feature2id.insert(pair<string,int>(s,i));
		featureVec.push_back(s);
	}
	getline(fin,s);
	tag_num = s2i(s);
	for (int i = 0; i<tag_num; i++)
	{
		getline(fin,s);
		TrimLine(s);
		tag2id.insert(pair<string,int>(s,i));
	}
	featureAddrVec.resize(feature_num+1,0);
	for (int i = 0; i<feature_num; i++)
	{
		getline(fin,s);
		TrimLine(s);
		vector <string> vs;
		Split(vs,s);
		featureAddrVec.at(i+1) = featureAddrVec.at(i)+s2i(vs.at(0));
		for(size_t j = 1;j<vs.size();j++)
			lambda2tagVec.push_back(s2i(vs.at(j)));
	}
	getline(fin,s);
	lambda_num = s2i(s);
	for (int i = 0; i<lambda_num;i++)
	{
		getline(fin,s);
		TrimLine(s);
		double lambda = s2d(s);
		lambdaVec.push_back(lambda);
	}
}

double MaxentModel::eval(vector<string> &context, string & outcome)
{
	int tagid = 0;
	map <string,size_t>::iterator it = tag2id.find(outcome);
	if (it != tag2id.end())
	{
		tagid = it->second;
	}
	else
	{
		return -99;
	}
	//cout<<"tgt tanslation found\n";  //4debug
	vector<double> probs;
	probs.resize(tag_num, 0.0);
	
	for (size_t i = 0; i < context.size(); i ++)
	{
		map<string,size_t>::iterator iter = feature2id.find(context.at(i));
		if (iter != feature2id.end())
		{
			int featureid = iter->second;
			int num1 = featureAddrVec.at(featureid);
			int num2 = featureAddrVec.at(featureid+1);
			for (int j = num1; j < num2; j ++)
			{
				probs.at(lambda2tagVec.at(j)) += lambdaVec.at(j);
			}
		}
		else
		{
			continue;
		}
	}
	
	double sum = 0.0;
	for (size_t j = 0; j < probs.size(); j ++)
	{
		probs.at(j) = exp(probs.at(j));
		sum += probs.at(j);
	}
	
	for (size_t k = 0; k < probs.size(); k ++)
	{
		probs.at(k) /= sum; 
	}
	return log10(probs.at(tagid));
}

/**********************************************************************************************/
//初始化参数
/**********************************************************************************************/
bool
Algorithm::Initialize(Config parameter)
{
	_ulSenLenChi=0;
	_ulSenLenEng=0;
	_pstSearchSpace=NULL;
	m_hypo_collection.clear();
	_pstHypoStack=NULL;
	_ulSenLenMax=0;
	//以下数据变量的初始化为张家俊08年12月25日添加
	m_syn_flag = NULL;
	m_order_flag = NULL;
	m_nbest_max = NULL;
	m_span_vector.clear();
	m_syn_prob_map.clear();
	m_nbest_cands = NULL;
	m_chart = NULL; //09.06.05添加
	m_spans.clear();
	m_search_space_queue = NULL;
	m_search_space_order_vector = NULL;
	m_split_best_vector = NULL;
	m_other_space_vector = NULL;

	//下面调序模型变量初始化由张家俊09年1月5日添加
	m_me_reorder_model = new ReorderModel();
	if(!m_me_reorder_model->ReadFile(parameter.reorder_model_filename.c_str()))
	{
	    cerr<<"Reading reordering model file Error!"<<endl;
	    exit(-1);
	}

	//parameters
	BEAM_HIST = parameter.beam_hist;
	BEAM_PROB = parameter.beam_prob;

	TRAN_TABLE_SIZE = parameter.ttable;
	RECOMBINE_HYPOTHESES = parameter.recombine;
	NbestNumber = parameter.nbest;

	threadNum = parameter.threadsNum;

	NgramOrder = parameter.lmorder;

	m_reorder_window = parameter.reorder_window;
	m_cube_flag = parameter.cube_flag;

	m_chn2eng_flag = parameter.chn2eng_flag;
	g_Chinese_Punc_Set = parameter.g_Chinese_Punc_Set;
	g_Chinese_Split_Puncs = parameter.g_Chinese_Split_Puncs;

	all_Lambda.trans = parameter.Lambda_trans;

	all_Lambda.lm = parameter.Lambda_lm;
	all_Lambda.sense = parameter.Lambda_sense;
	all_Lambda.len = parameter.Lambda_len;
	all_Lambda.reorderStraight = parameter.Lambda_reorderStraight;
	all_Lambda.reorderSwap = parameter.Lambda_reorderSwap;
	all_Lambda.dPhraseNum = parameter.Lambda_dPhraseNum;


	train = parameter.train;
	feats_num = parameter.feats_num;
	printInfo = parameter.printInfo;
	printNbest = parameter.printNbest;

	nBestFileName = parameter.nbestfilename;

	_filetype = parameter.filetype;
	_segmenttype = parameter.segmenttype;
	_transUnkownWord = parameter.transUnknownWord;



	_pVocabChi = new Vocab(parameter.sourcevocabfilename.c_str());
	_pVocabEng = new Vocab(parameter.targetvocabfilename.c_str());

	_pstPhrasePro = new PhrasePro(_pVocabChi, _pVocabEng, TRAN_TABLE_SIZE);
	
	//下面基于上下文的翻译模型变量初始化由李小青14年3月29日添加
	m_context_based_translation_models.resize(_pVocabChi->size(),NULL);
	ifstream fin_catalog(parameter.catalog_filename.c_str());
	if (!fin_catalog)
	{
		cout<<"cannot find catalog file!\n";
		exit(-1);
	}
	string context_model_filename;
	while(getline(fin_catalog,context_model_filename))
	{
		//cout<<"loading sense model "<<context_model_filename<<endl;  //4debug
		string model_file = "data/"+context_model_filename;
		MaxentModel* m = new MaxentModel;
		m->load(model_file);
		m_context_based_translation_models.at(atoi(context_model_filename.c_str())) = m;
	}

	//根据测试语料对短语对和语言模型进行削减
	if( parameter.reduceVoc )
	{
	}

	//read phrase table
	_pstPhrasePro->SetArgs(all_Lambda);
	if ( !_pstPhrasePro->ReadFile(parameter.phrasefilename.c_str(), parameter.reduceVoc, parameter.segmenttype) )
	{
		cerr<<"reading phrase file error!\n";
		return 0;
	}

	//为每个短语对读取词对齐信息，由李小青2014年4月4日添加
	if ( !_pstPhrasePro->ReadAlignment("data/phrase-table") )
	{
		cerr<<"reading alignment file error!\n";
		return 0;
	}

	_pNgram = NULL;
	//read LM file
	if( parameter.KenLM )
	{
		KLanguageModelFactory* klmFactory = new KLanguageModelFactory();
		_pNgram = klmFactory->Create(parameter.lmfilename, _pVocabEng);
	}
	else
	{
		_pNgram =  new Ngram(_pVocabEng, parameter.lmorder, parameter.lmfilename, parameter.reduceVoc);
	}
//////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////
	_pulSenChi = new int[SEN_LEN_MAX+2];
	if( !_pulSenChi )
	{
		cerr<<"Allocate Memory for Chinese Sentence Error!\n";
	}

	_pulSenEng= new int[3*SEN_LEN_MAX];
	if( !_pulSenEng )
	{
		cerr<<"Allocate Memory for English Sentence Error!\n"<<endl;
	}

	return 1;
}


/**********************************************************************************************/
//析构
/**********************************************************************************************/
Algorithm::~Algorithm()
{

	delete []_pulSenChi;
	_pulSenChi = NULL;
	delete []_pulSenEng;
	_pulSenEng = NULL;
	//以下修改由张家俊08年12月26日完成
	//delete []_pstHypoStack;
	//_pstHypoStack = NULL;

	//delete []_pdFutCostTable;
	//_pdFutCostTable = NULL;
	delete []_pstSearchSpace[0];
	_pstSearchSpace[0] = NULL;
	m_spans.clear();
	
	//删除基于上下文的翻译模型，由李小青2014年4月7日添加
	/*
	m_context_based_translation_models.clear();
	for(size_t i=1; i<m_context_based_translation_models.size() ; i++ )
	{
		if (m_context_based_translation_models.at(i) != NULL)
		{
			delete m_context_based_translation_models.at(i);
			m_context_based_translation_models.at(i) = NULL;
		}
	}
	*/
	
	for(int i=1; i<_ulSenLenMax+1 ; i++ )
	{
		delete []_pstSearchSpace[i];
		_pstSearchSpace[i] = NULL;
		delete []m_chart[i];
		m_chart[i] = NULL;
		delete []m_syn_flag[i];
		m_syn_flag[i] = NULL;
		if( printNbest )
		{
		    delete []m_nbest_cands[i];
		    m_nbest_cands[i] = NULL;
		    delete []m_search_space_queue[i];
		    m_search_space_queue[i] = NULL;
		    delete []m_search_space_order_vector[i];
		    m_search_space_order_vector[i] = NULL;
		    delete []m_split_best_vector[i];
		    m_split_best_vector[i] = NULL;
		    delete []m_other_space_vector[i];
		    m_other_space_vector[i] = NULL;
		}
	}
	delete []_pstSearchSpace;
	_pstSearchSpace = NULL;
	for( int k = 0; k < m_hypo_collection.size(); k++ )
		delete m_hypo_collection[k];
	m_hypo_collection.clear();
	delete []m_chart;
	m_chart = NULL;
	delete []m_syn_flag;
	m_syn_flag = NULL;
	m_syn_prob_map.clear();
	if( printNbest )
	{
	    delete []m_nbest_cands;
	    m_nbest_cands = NULL;
	    delete []m_search_space_queue;
	    m_search_space_queue = NULL;
	    delete []m_search_space_order_vector;
	    m_search_space_order_vector = NULL;
	    delete []m_split_best_vector;
	    m_split_best_vector = NULL;
	    delete []m_other_space_vector;
	    m_other_space_vector = NULL;
	}
}

////////////////////////////////////////////////////////////////////////
// 1.主要功能：将输入句子中给出的NP的翻译提取出来
// 2.入口参数：输入的句子；句子长度
// 3.出口参数：提取完NP翻译后的句子长度
// 4.生成日期：2006.04.10
// 5.算法简介：将句子中标记为<_NUMBER> 十 四 || fourteen <NUMBER> 这样的部分提取出来，
//				如果是训练语料中已经替换的，直接进行替换，如 <_NUMBER>,将翻译对放入到单独的一个表中
//				如果是没有进行替换的，用<_NORMAL> ..|| .. <NORMAL>标记的，将翻译对直接放到翻译候选项表中，
//				同一短语的翻译候选项限定在这个上，不允许有其他候选项
//
//
int Algorithm::NPParsing( char** words, int &SenLen )
{
	_bitClosed.reset();

	_mapNPTransTalbe.clear();	//将翻译表清空
	int i=0;
	int m_len=0;			//句子的实际长度
	char* m_sentence[SEN_LEN_MAX];
	string m_word;
	while( words[i] != 0 && words[i][0] != 0 )
	{
		m_word=words[i++];

		//数字
		if( m_word == g_Number_Start )
		{
		}
		//时间
		else if( m_word == g_Time_Start )
		{
		}
		//地名
		else if( m_word == g_Loc_Start )
		{
		}
		//人名
		else if( m_word == g_Person_Start )
		{
		}
		//组织机构名
		else if( m_word == g_Org_Start )
		{
		}
		//集外词
		else if( m_word == g_Namex_Start )
		{
		}
		//命名实体
		else if( m_word == g_NE_Start )
		{
		}
		//如果为一般短语，则将翻译对放到搜索候选项表中，同时不允许该短语产生其他的翻译选项
		else if( m_word == g_Normal_Start )
		{
		}
		else
		{
			m_sentence[m_len++] = words[i-1];
		}

	}


	for( i=0; i<m_len; i++)
	{
		words[i] = m_sentence[i];

	}

	if( i != m_len )
		cout<<"error\n";

	words[m_len][0] = 0;


	return 1;
}


////////////////////////////////////////////////////////////////////////
// 1.主要功能：生成搜索候选项
// 2.入口参数：无
// 3.出口参数：是否成功
// 4.生成日期：2009.10.10
// 5.算法简介：从第一个词开始，搜索所有可能的短语，如果在翻译概率表中含有这个短语，将其放入搜索候选项中
//
//
bool Algorithm::GenSearchSpace(vector< vector< pair<string,double> > > SenChiCN)
{
	PhraseProTrie *m_PhraseTrie;        //记录节点
	s_SearchSpace* m_SearchSpaceTemp;
	char m_pcSenChi[SEN_CHAR_MAX];
	char* words[SEN_LEN_MAX+1];
	for( int k=0; k<SEN_LEN_MAX+1; ++k )
	{
		words[k] = new char[WORD_LEN_MAX+1];
	}
	//normal text
	{
		int i=0, j=0;            //记录词的位置
		int len=0;               //短语的长度

		//for(size_t k=0;k<Vocab::length(_pulSenChi);k++)
		//   cout<<_pulSenChi[k]<<" ";
		//cout<<endl;
		for(i=1; i<= _ulSenLenChi; i++)		//起始位置,表示中文句子词号的数组从1开始
		{
			//bool m_bTemp = false;
			for(j=i, len = 0; j<=_ulSenLenChi && len<PHRASE_LEN_MAX;j++,len++ )	//当前词的位置
			{
				if( _bitClosed.test(j) )
				{
					break;
				}
				//cerr<<_pulSenChi[j]<<" "<<len<<endl;;
				if( len == 0 )
					m_PhraseTrie = _pstPhrasePro->findword(_pulSenChi[j]); 
				else
					m_PhraseTrie = m_PhraseTrie->findTrie(_pulSenChi[j]);

				//如果在翻译概率表中有这个短语，将所有的翻译候选项放入到搜索候选项栈中
				if( m_PhraseTrie )                                             //按照上面的插入方式，可能不存在么?
				{
					if( !m_PhraseTrie->value().size()  && len == 0 )       //未登录词，即该短语没有翻译候选，且短语长度为1
					{
						m_SearchSpaceTemp = new s_SearchSpace();
						m_hypo_collection.push_back(m_SearchSpaceTemp);
						m_SearchSpaceTemp->ulFirstWord = i;
						m_SearchSpaceTemp->ulLastWord = j;
						m_SearchSpaceTemp->ulEnNum = 1;

						m_SearchSpaceTemp->eachTransPro.resize( all_Lambda.trans.size() );
						fill(m_SearchSpaceTemp->eachTransPro.begin(), m_SearchSpaceTemp->eachTransPro.end(), LogP_PseudoZero);

						m_SearchSpaceTemp->dPro = 0;
						for (int m_i=0; m_i<(int)all_Lambda.trans.size(); m_i++ )
						{
							m_SearchSpaceTemp->dPro += all_Lambda.trans[m_i] * m_SearchSpaceTemp->eachTransPro[m_i];
						}

						if( _transUnkownWord == NULLWORD )
						{
							m_SearchSpaceTemp->viEnPhrase.clear();
							//下面的修改由张家俊08年12月31日完成
							m_SearchSpaceTemp->viEnPhrase.push_back( _pVocabEng->UnNull() );
							m_SearchSpaceTemp->m_en_str = g_Vocab_NULL;
						}
						else
						{
							m_SearchSpaceTemp->viEnPhrase.clear();
							m_SearchSpaceTemp->viEnPhrase.push_back( _pVocabEng->GetAndInsertIndex(_pVocabChi->GetWord( _pulSenChi[i] ).c_str()) );
							m_SearchSpaceTemp->m_en_str = _pVocabChi->GetWord(_pulSenChi[i]);
						}
						//下面几个参数计算由张家俊08年12月25日添加
						m_SearchSpaceTemp->m_phrase_num = 1;
						vector<int> wids = m_SearchSpaceTemp->viEnPhrase;
						//cerr<<"before language model computing 1:\n";
						m_SearchSpaceTemp->m_en_ngram_prob =  _pNgram->TraversalFeaturesImpl(m_SearchSpaceTemp) ;
						//cerr<<"after language model computing 1:\n";

						m_SearchSpaceTemp->dPro += all_Lambda.dPhraseNum*m_SearchSpaceTemp->m_phrase_num + all_Lambda.len*m_SearchSpaceTemp->ulEnNum + all_Lambda.lm*m_SearchSpaceTemp->m_en_ngram_prob;
						_pstSearchSpace[i][len].Update(m_SearchSpaceTemp);

					}
					//m_bTemp = true;

					else
					{
						vector<s_PhrasePro>::iterator itbegin=m_PhraseTrie->value().begin(), itend=m_PhraseTrie->value().end();
						for( ; itbegin!=itend; itbegin++)
						{
							m_SearchSpaceTemp = new s_SearchSpace();
							m_hypo_collection.push_back(m_SearchSpaceTemp);
							m_SearchSpaceTemp->ulFirstWord = i;
							m_SearchSpaceTemp->ulLastWord = j;
							m_SearchSpaceTemp->ulEnNum = (*itbegin).ulEnNum;
							m_SearchSpaceTemp->dPro = (*itbegin).dPro;

							m_SearchSpaceTemp->eachTransPro = (*itbegin).eachTransPro;

							m_SearchSpaceTemp->viEnPhrase.clear();
							if((*itbegin).viEnPhrase.size() > 1)
							{
								for( int i=0; i<(*itbegin).viEnPhrase.size()-1; i++ )
								{
									m_SearchSpaceTemp->viEnPhrase.push_back( (*itbegin).viEnPhrase[i] );
									m_SearchSpaceTemp->m_en_str += _pVocabEng->GetWord((*itbegin).viEnPhrase[i]);
									if( i != (*itbegin).viEnPhrase.size()-2 )
										m_SearchSpaceTemp->m_en_str += " ";
								}
							}
							else
							{
								m_SearchSpaceTemp->viEnPhrase.push_back( _pVocabEng->UnNull() );
								m_SearchSpaceTemp->m_en_str = g_Vocab_NULL;
							}
							//cerr<<"Error here!"<<endl;
							//下面几个参数计算由张家俊08年12月25日添加
							m_SearchSpaceTemp->m_phrase_num = 1;
							vector<int> wids = m_SearchSpaceTemp->viEnPhrase;
							//cerr<<"before language model computing 2:\n";
							if( NULL == _pNgram )
							{
								cerr<<"back new for ngram!\n";
								exit(1);
							}
							m_SearchSpaceTemp->m_en_ngram_prob =  _pNgram->TraversalFeaturesImpl(m_SearchSpaceTemp) ;
							//cerr<<"after language model computing 2:\n";
							//增加基于上下文的翻译概率，李小青于14年3月31日添加
							//此处中文端翻译单元为跨度为[i,j]的短语
							vector <int> &en_ids = (*itbegin).viEnPhrase;
							for (int k=i; k<=j; k++)
							{
								vector <int> en_pos_list = (*itbegin).ch_pos_to_en_pos_list.at(k-i);
								string tgt_translation;
								if (en_pos_list.size() == 0)
								{
									tgt_translation = "NULL";
								}
								else
								{
									tgt_translation = _pVocabEng->GetWord(en_ids[en_pos_list.at(0)]);
									for (size_t l=1; l<en_pos_list.size(); l++)
									{
										tgt_translation += "_" + _pVocabEng->GetWord(en_ids[en_pos_list.at(l)]);
									}
									/*
									for (size_t l=0; l<en_pos_list.size(); l++)  //4debug
									{
										cout<<en_pos_list.at(l)<<' '<<_pVocabEng->GetWord(en_ids[en_pos_list.at(l)])<<'\t';
									}
									*/
								}
								//cout<<"tgt translation:" << tgt_translation<<endl;  //4debug
								m_SearchSpaceTemp->m_context_based_trans_prob += GetContextBasedTranslationProb(k-1,tgt_translation);
								//cout<<"cand at each word, m_context_based_trans_prob: "<<m_SearchSpaceTemp->m_context_based_trans_prob<<endl;  //4debug
								//cout<<ch_word_vec.at(k-1)<<" ||| "<<tgt_translation<<'\t'<<m_SearchSpaceTemp->m_context_based_trans_prob<<endl;  //4debug
							}
							m_SearchSpaceTemp->dPro += all_Lambda.dPhraseNum*m_SearchSpaceTemp->m_phrase_num + all_Lambda.len*m_SearchSpaceTemp->ulEnNum + all_Lambda.lm*m_SearchSpaceTemp->m_en_ngram_prob + all_Lambda.sense*m_SearchSpaceTemp->m_context_based_trans_prob;
							if( m_SearchSpaceTemp->viEnPhrase[m_SearchSpaceTemp->viEnPhrase.size()-1] == -1 )
								cerr<<i<<" "<<len<<" "<<"Word Id Error!"<<endl;
							_pstSearchSpace[i][len].Update(m_SearchSpaceTemp);
						}

					}

				}
				else
				{
					break;
				}
			}
			if(  !_bitClosed.test(i) && len == 0)                                 //该词为未登录词，则翻译为NULL
			{
				m_SearchSpaceTemp = new s_SearchSpace();
				m_hypo_collection.push_back(m_SearchSpaceTemp);
				m_SearchSpaceTemp->ulFirstWord = i;
				m_SearchSpaceTemp->ulLastWord = i;
				m_SearchSpaceTemp->ulEnNum = 1;


				m_SearchSpaceTemp->eachTransPro.resize( all_Lambda.trans.size() );
				fill(m_SearchSpaceTemp->eachTransPro.begin(), m_SearchSpaceTemp->eachTransPro.end(), LogP_PseudoZero);

				m_SearchSpaceTemp->dPro = 0;
				for (int m_i=0; m_i<(int)all_Lambda.trans.size(); m_i++ )
				{
					m_SearchSpaceTemp->dPro += all_Lambda.trans[m_i] * m_SearchSpaceTemp->eachTransPro[m_i];
				}


				if( _transUnkownWord == NULLWORD )
				{
					m_SearchSpaceTemp->viEnPhrase.clear();
					//下面两行由张家俊08年12月31日修改
					m_SearchSpaceTemp->viEnPhrase.push_back( _pVocabEng->UnNull() );
					m_SearchSpaceTemp->m_en_str = g_Vocab_NULL;
				}
				else
				{
					m_SearchSpaceTemp->viEnPhrase.clear();
					m_SearchSpaceTemp->viEnPhrase.push_back( _pVocabEng->GetAndInsertIndex(_pVocabChi->GetWord( _pulSenChi[i] ).c_str()) );
					m_SearchSpaceTemp->m_en_str = _pVocabChi->GetWord(_pulSenChi[i]);
				}
				//下面几个参数计算由张家俊08年12月25日添加
				m_SearchSpaceTemp->m_phrase_num = 1;
				vector<int> wids = m_SearchSpaceTemp->viEnPhrase;
				//cerr<<"before language model computing 3:\n";
				m_SearchSpaceTemp->m_en_ngram_prob =  _pNgram->TraversalFeaturesImpl(m_SearchSpaceTemp) ;
				//cerr<<"after language model computing 3:\n";
				m_SearchSpaceTemp->dPro += all_Lambda.dPhraseNum*m_SearchSpaceTemp->m_phrase_num + all_Lambda.len*m_SearchSpaceTemp->ulEnNum + all_Lambda.lm*m_SearchSpaceTemp->m_en_ngram_prob;
				if( m_SearchSpaceTemp->viEnPhrase[m_SearchSpaceTemp->viEnPhrase.size()-1] == -1 )
					cerr<<i<<" "<<len<<" "<<"Word Id Error!"<<endl;
				_pstSearchSpace[i][len].Update(m_SearchSpaceTemp);


			}
		}

	}




	//
	//如果当前句子的长度小于前面最长的句子，则NULL->English的搜索候选项不用重新计算
	//
	if( m_SenLen > _ulSenLenMax )
	{
		m_PhraseTrie = _pstPhrasePro->findword( _pVocabChi->UnNull());
		if( m_PhraseTrie )
		{
			vector<s_PhrasePro>::iterator itbegin=m_PhraseTrie->value().begin(), itend=m_PhraseTrie->value().end();
			for( ; itbegin!=itend; itbegin++)
			{
				m_SearchSpaceTemp = new s_SearchSpace();
				m_hypo_collection.push_back(m_SearchSpaceTemp);
				m_SearchSpaceTemp->ulFirstWord = 0;
				m_SearchSpaceTemp->ulLastWord = 0;
				m_SearchSpaceTemp->ulEnNum = (*itbegin).ulEnNum;
				m_SearchSpaceTemp->dPro = (*itbegin).dPro;
				m_SearchSpaceTemp->viEnPhrase = (*itbegin).viEnPhrase;
				m_SearchSpaceTemp->m_en_str = g_Vocab_NULL;
				//下面几个参数计算由张家俊08年12月25日添加
				m_SearchSpaceTemp->m_phrase_num = 1;
				vector<int> wids = m_SearchSpaceTemp->viEnPhrase;
				//cerr<<"before language model computing 4:\n";
				m_SearchSpaceTemp->m_en_ngram_prob =  _pNgram->TraversalFeaturesImpl(m_SearchSpaceTemp) ;
				//cerr<<"before language model computing 4:\n";
				m_SearchSpaceTemp->dPro += all_Lambda.dPhraseNum*m_SearchSpaceTemp->m_phrase_num + all_Lambda.len*m_SearchSpaceTemp->ulEnNum + all_Lambda.lm*m_SearchSpaceTemp->m_en_ngram_prob;
				if( m_SearchSpaceTemp->viEnPhrase[m_SearchSpaceTemp->viEnPhrase.size()-1] == -1 )
					cerr<<0<<" "<<0<<" "<<"Word Id Error!"<<endl;
				_pstSearchSpace[0][0].Update(m_SearchSpaceTemp);
			}

		}

	}

	if( printInfo )
	{
		//not implemented now
	}

	for( int k=0; k<SEN_LEN_MAX+1; ++k )
	{
		delete[] words[k];
	}
	return true;
}

////////////////////////////////////////////////////////////////////////
// 1.主要功能：将词号转化为英文单词
// 2.入口参数：英文句子的指针
// 3.出口参数：是否成功
// 4.生成日期：2006.04.10
// 5.算法简介：


string 
Algorithm::GetEngStr( const vector<int> &sentenceID )
{
	string sentenceEng;
	if( sentenceID.size() == 0 )
		sentenceEng = "";
	else
	{
		sentenceEng = _pVocabEng->GetWord( sentenceID[0] );
		for( int i=1; i< (int)sentenceID.size(); i++ )
		{
			sentenceEng += " ";
			sentenceEng += _pVocabEng->GetWord( sentenceID[i] );
		}
	}

	return sentenceEng;
}

/**********************************************************************************************/
//MoveNullWord()
//去除翻译结果里的空词
//输入：翻译结果
//输出：成功与否标志
/**********************************************************************************************/
bool Algorithm::MoveNullWord(vector<int> &en_phrase)
{
    int c_nullword_index = _pVocabEng->UnNull();
    int c_senstart_index = _pVocabEng->SenStart();
    int c_senend_index = _pVocabEng->SenEnd();
    vector<int> c_tmp_phrase = en_phrase;
    en_phrase.clear();
    for(int i=0; i<c_tmp_phrase.size(); i++)
	if( (c_tmp_phrase[i] != c_nullword_index) && (c_tmp_phrase[i] != c_senstart_index) && (c_tmp_phrase[i] != c_senend_index) )
	    en_phrase.push_back(c_tmp_phrase[i]);
}

/***********************09.06.05添加 cube prunning*********************************************/
//判断两个目标翻译候选是否相同
bool IsTheSame(vector<int> &a, vector<int> &b)
{
    if(a.size() != b.size() )
	return false;
    for( int i = 0; i < a.size(); i++ )
	if( a[i] != b[i] )
	    return false;
    return true;
};

void DecodingSpan::DecodeSpan()
{
	DecodeSpan(_threadId, _threadNum);
}

void DecodingSpan::DecodeSpan(int threadId, int threadNum)
{
	int pairCount = _decoderPtr->m_spans.size();
	for( int i = pairCount*threadId/threadNum; i < pairCount*(threadId+1)/threadNum; i++ )
		_decoderPtr->NodeKBest(_decoderPtr->m_spans[i].first, _decoderPtr->m_spans[i].second, _hypoCollection);
}

/**********************************************************************************************/
// 函数CubeParse
// 功能：利用cube prunning 算法高效地产生最终翻译结果
// 返回：成功与否标志
/**********************************************************************************************/
bool Algorithm::CubeParse(int m_SenLen, char* pcSenEng, vector<string> &nBestList, vector< vector<double> > &feats)
{
	//int threadNum = 10;

	nBestList.clear();
	feats.clear();

	//将span=1的候选翻译都导入到m_chart中
	for( int i = 1; i <= m_SenLen; i++ )
	{
		if( _pstSearchSpace[i][0].Count() )
		{
			s_SearchSpace* c_ptr_searchspace = NULL;
			//满足条件则循环插入,因为保存短语翻译表的假设栈已经排序，所以这里顺序插入即可
			while( (c_ptr_searchspace = _pstSearchSpace[i][0].Pop()) )
			{
				m_chart[i][0].push_back(c_ptr_searchspace);
			}
			//由于_pstSearchSpace是最小堆，所以需要进行逆序操作
			reverse(m_chart[i][0].begin(), m_chart[i][0].end());
		}
	}

	for( int c_span=2; c_span<=m_SenLen; c_span++ )
	{
		m_spans.clear(); //在解码所有长度相同的跨度时，都首先对存储跨度的向量清空
		for(int start=1; start<=m_SenLen-c_span+1;start++)
		{
			int end=start+c_span-1;

			if( m_chn2eng_flag )
			{
				//中间有逗号，则不翻译，以达到子句划分的目的
				int c_flag = 0;
				for(int i=start+1; i<end; i++)
				{
					if( g_Chinese_Split_Puncs.find(_pVocabChi->GetWord(_pulSenChi[i])) != string::npos )
					{
						c_flag = 1;
						break;
					}
				}
				if( c_flag )
					if( (start == 1 || g_Chinese_Punc_Set.find(_pVocabChi->GetWord(_pulSenChi[start-1])) != string::npos || g_Chinese_Punc_Set.find(_pVocabChi->GetWord(_pulSenChi[start])) != string::npos) && 
							(end == m_SenLen || g_Chinese_Punc_Set.find(_pVocabChi->GetWord(_pulSenChi[end])) != string::npos || g_Chinese_Punc_Set.find(_pVocabChi->GetWord(_pulSenChi[end+1])) != string::npos) )
						c_flag = 0;
				if( c_flag )
					continue;
			}
			else //英文或其他语种
			{
				int c_flag = 0;
				for( int i = start+1; i < end; i++ )
					if( g_other_punc_set.find(_pVocabChi->GetWord(_pulSenChi[i])) != string::npos )
					{
						c_flag = 1;
						break;
					}
				if( c_flag )
					if( (start == 1 || g_other_punc_set.find(_pVocabChi->GetWord(_pulSenChi[start-1])) != string::npos || g_other_punc_set.find(_pVocabChi->GetWord(_pulSenChi[start])) != string::npos) && 
							(end == m_SenLen || g_other_punc_set.find(_pVocabChi->GetWord(_pulSenChi[end])) != string::npos || g_other_punc_set.find(_pVocabChi->GetWord(_pulSenChi[end+1])) != string::npos) )
						c_flag = 0;

				if( c_flag )
					continue;
			}

			pair<int, int> c_span = make_pair(start, end);
			m_spans.push_back(c_span);
			//NodeKBest(start,end, m_hypo_collection);
		}

		//if( c_span > threadNum )
		if( c_span > 1 )
		{
			//cerr<<"new multiple threads..."<<endl;
			DecodingSpan** threads = new DecodingSpan*[threadNum];
			for( int i = 0; i < threadNum; i++ )
			{
				threads[i] = new DecodingSpan();
				threads[i]->_decoderPtr = this;
				threads[i]->_threadId = i;
				threads[i]->_threadNum = threadNum;
			}
			//cerr<<"start multiple threads..."<<endl;
			for( int i = 0; i < threadNum; i++ )
				threads[i]->Start();
			//cerr<<"mid multiple threads..."<<endl;
			for( int i = 0; i < threadNum; i++ )
				threads[i]->Join();
			for( int i = 0; i < threadNum; i++ )
				m_hypo_collection.insert(m_hypo_collection.end(), threads[i]->_hypoCollection.begin(), threads[i]->_hypoCollection.end());
			//cerr<<"end multiple threads..."<<endl;
			for( int i = 0; i < threadNum; i++)
				delete threads[i];
			delete []threads;
		}
		else
			for( int i = 0; i < m_spans.size(); i++ )
				NodeKBest(m_spans[i].first, m_spans[i].second, m_hypo_collection);
	}

	//cerr<<"translate end..."<<endl;
	//记录最优翻译
	if(!m_chart[1][m_SenLen-1].size())
	{
		cerr<<"Cannot find the best translation!"<<endl;
		return false;
	}
	s_SearchSpace* c_pTemp_searchspace = m_chart[1][m_SenLen-1][0];
	MoveNullWord(c_pTemp_searchspace->viEnPhrase);
	string c_best_trans_str = GetEngStr(c_pTemp_searchspace->viEnPhrase);
	//TrueCasing( c_best_trans_str ); //暂时不用大写手字母
	strcpy(pcSenEng, c_best_trans_str.c_str());
	//cout<<"the prob is "<<c_pTemp_searchspace->dPro<<endl;
	//cout<<"the size is "<<_pstSearchSpace[1][m_SenLen-1].size()<<endl;

	if( printNbest )
	{
		//记录nbest 列表以及特征值
		for(int i=0; i<m_chart[1][m_SenLen-1].size(); i++)
		{
			MoveNullWord(m_chart[1][m_SenLen-1][i]->viEnPhrase);
			//c_best_trans_str = GetEngStr(m_chart[1][m_SenLen-1][i]->viEnPhrase);
			c_best_trans_str = m_chart[1][m_SenLen-1][i]->m_en_str;
			//TrueCasing( c_best_trans_str ); //暂时不用大写手字母
			//直接利用cube prunning 产生根节点nbest，则不用判断翻译的重复
			/*
			   int flag = 0;
			   for(int k=nBestList.size()-1; k>=0; k--)
			   if(c_best_trans_str == nBestList[k])
			   {
			   flag = 1;
			   break;
			   }
			   if( flag )
			   continue;
			   */
			nBestList.push_back(c_best_trans_str);

			vector<double> c_feats;
			for(int j=0; j<m_chart[1][m_SenLen-1][i]->eachTransPro.size(); j++)
				c_feats.push_back(m_chart[1][m_SenLen-1][i]->eachTransPro[j]);
			c_feats.push_back(m_chart[1][m_SenLen-1][i]->ulEnNum);
			c_feats.push_back(m_chart[1][m_SenLen-1][i]->m_en_ngram_prob);
			c_feats.push_back(m_chart[1][m_SenLen-1][i]->m_straight_reorder_prob);
			c_feats.push_back(m_chart[1][m_SenLen-1][i]->m_swap_reorder_prob);
			c_feats.push_back(m_chart[1][m_SenLen-1][i]->m_phrase_num);
			c_feats.push_back(m_chart[1][m_SenLen-1][i]->m_context_based_trans_prob);  //增加基于上下文的翻译概率，供调参用
			//cout<<"whole sentence, m_context_based_trans_prob: "<<m_chart[1][m_SenLen-1][i]->m_context_based_trans_prob<<endl;  //4debug
			feats.push_back(c_feats);
		}
	}

	//清空数据结构内容
	for( int i=1; i<=_ulSenLenChi; i++)
	{
		int m_HypoLen=_ulSenLenChi-i+1;
		for(int j=0; j<m_HypoLen; j++)
		{
			_pstSearchSpace[i][j].Clear();
			m_syn_flag[i][j] = 0;
			m_chart[i][j].clear();
			if( printNbest )
			{
				m_order_flag[i][j] = 0;
				m_nbest_max[i][j] = 0;
				m_nbest_cands[i][j].clear();
				//清空排序表
				m_search_space_order_vector[i][j].clear();
				m_split_best_vector[i][j].clear();
				m_other_space_vector[i][j].clear();
				//只能用此方法清空优先队列
				while( !m_search_space_queue[i][j].empty() )
					m_search_space_queue[i][j].pop();
			}
		}
	}
	m_span_vector.clear();
	m_syn_prob_map.clear();
	_pstSearchSpace[0][0].Clear();
	for( int k = 0; k < m_hypo_collection.size(); k++ )
		delete m_hypo_collection[k];
	m_hypo_collection.clear();
	m_spans.clear();

	return true;
}

/****************************************************************************/
//函数 NodeKBest
//功能 cube prunning 的核心部分,对每个节点产生k个最好的翻译候选
//返回值：成功与否表示
/****************************************************************************/
bool Algorithm::NodeKBest(int start, int end, vector<s_SearchSpace*> &hypo_collection)
{
	//cerr<<"in NodeKBest ..."<<endl;
    int beam_hist = BEAM_HIST;

	//当前直接用cube prunning产生根节点的nbest，当n较小时合适，有待扩展RootKBest方法
	//权宜之计
	if( printNbest && (end - start) == m_SenLen-1 )
		beam_hist = NbestNumber;

    //double beam_prob = BEAM_PROB; 
	//dynamic beam thresholding
	double beam_prob = 0.0;
	if( train )
		beam_prob = BEAM_PROB*14;
	else
		beam_prob = BEAM_PROB*(double)(m_SenLen)/(end-start+1);

    int span = end-start;

    //先利用cube prunning产生该节点的k-best翻译候选
	//priority_queue<s_SearchSpace,vector<s_SearchSpace>,SearchSpaceComp> cands_pri_queue; //保留候选的优先队列
	HypoHeap candsQueue;
	candsQueue.Init(MAXHEAP, 2*beam_hist*beam_hist);

	//cerr<<"in NodeKBest 0 ..."<<endl;
	//首先加入该节点每条入边最好的翻译候选
	for(int i = start; i < end; i++)
	{
	    s_SearchSpace* left_searchspace;
		s_SearchSpace* right_searchspace;
	    if( m_chart[start][i-start].size() && m_chart[i+1][end-i-1].size() )
		{
			left_searchspace = m_chart[start][i-start][0];
			right_searchspace = m_chart[i+1][end-i-1][0];
			MergeIntoEdge(left_searchspace, right_searchspace, 1,1, candsQueue, hypo_collection);
		}
	}
	//cerr<<"in NodeKBest 1 ..."<<endl;

	//设置标志表示某条边是否已经由左孩子节点的ith候选和右孩子节点的jth候选扩展过
	//由于扩展涉及顺序和逆序，所以会出现重复扩展的现象

	set<int> duplicate_set;
	duplicate_set.clear();

	vector<s_SearchSpace*> buf;
	buf.clear();

	for( int i = 0; i < beam_hist+20; i++ )
	{
		if( candsQueue.Empty() )
			break;
		s_SearchSpace* best_cand = candsQueue.Pop();

		//cerr<<"cube pruning 0"<<endl;
		if( end - start == m_SenLen-1 ) //覆盖整个句子时，添加<s>，</s>计算语言模型概率
		{
			double added_lm_score = _pNgram->FinalTraversalFeatures(best_cand);
			best_cand->m_en_ngram_prob += added_lm_score;
			best_cand->dPro += all_Lambda.lm*added_lm_score; 
		}
		//cerr<<"cube pruning 1"<<endl;
		_pstSearchSpace[start][span].Update(best_cand);
		//cerr<<"cube pruning 2"<<endl;

		//加入临近的次优候选翻译
		int c_key = best_cand->m_left_ith_best*100000 + best_cand->m_right_jth_best*1000 + best_cand->m_split_point;
		//判断是否扩展过
		if( duplicate_set.end() == duplicate_set.find(c_key) )
		{
			PushSucc(best_cand, candsQueue, hypo_collection);
			duplicate_set.insert(c_key);
		}
		//cerr<<"cube pruning 3"<<endl;
	}
	//cerr<<"in NodeKBest 2 ..."<<endl;
	//如果_pstSearchSpace[start][span]中就有对应的翻译，则加入缓冲区中
	s_SearchSpace* c_ptr_searchspace = NULL; 
	while( c_ptr_searchspace = _pstSearchSpace[start][span].Pop() )
	{
		buf.push_back(c_ptr_searchspace);
	}
	//cerr<<"in NodeKBest 3 ..."<<endl;

	//由于_pstSearchSpace[start][span]是最小堆，所以需要对buf进行逆序
	reverse(buf.begin(), buf.end());
	//将缓冲区导入chart表中
	m_chart[start][end-start] = buf;

    return true;
}

/**********************************************************************************************/
//函数 PushSucc
//功能：产生当前候选的临近候选翻译
//返回值：成功与否标志
/**********************************************************************************************/
bool Algorithm::PushSucc(s_SearchSpace* cur_cand, HypoHeap &candsQueue, vector<s_SearchSpace*> &hypo_collection)
{
    int start = cur_cand->ulFirstWord;
    int end = cur_cand->ulLastWord;
    int split_point = cur_cand->m_split_point;

    //扩展相邻的两个次优候选翻译
    //首先扩展左节点的候选
    int left_kth_best = cur_cand->m_left_ith_best+1; 
    int right_kth_best = cur_cand->m_right_jth_best;
	//cerr<<"push succ left"<<endl;
    if( m_chart[start][split_point-start].size() >= left_kth_best )
		MergeIntoEdge(m_chart[start][split_point-start][left_kth_best-1], m_chart[split_point+1][end-split_point-1][right_kth_best-1], left_kth_best, right_kth_best, candsQueue, hypo_collection);

    //其次扩展右节点的候选
    left_kth_best = cur_cand->m_left_ith_best;
    right_kth_best = cur_cand->m_right_jth_best+1;
	//cerr<<"push succ right"<<endl;
    if( m_chart[split_point+1][end-split_point-1].size() >= right_kth_best )
	MergeIntoEdge(m_chart[start][split_point-start][left_kth_best-1], m_chart[split_point+1][end-split_point-1][right_kth_best-1], left_kth_best, right_kth_best, candsQueue, hypo_collection);
	//cerr<<"push succ finish"<<endl;

    return true;
}

/*********************************************************************************************************************************/
//函数MergeTwoVector
//将右边的vector合并到左边的vector
/*********************************************************************************************************************************/
void Algorithm::MergeTwoVector(vector<int> &left_vector, vector<int> &right_vector)
{
	for( int i = 0; i < right_vector.size(); i++ )
		left_vector.push_back(right_vector[i]);
}

string List2String(vector<int> &list, int flag)
{
	ostringstream buff;
	for( int i = 0; i < list.size(); i++ )
		buff<<list[i]<<" ";
	buff<<flag;
	return buff.str();
}

/*********************************************************************************************************************************/
//函数ComputeMergedLM
//计算两个目标串合并所增加的语言模型概率
/*********************************************************************************************************************************/
double Algorithm::ComputeMergedLM(Ngram* c_ngram_trie, int c_ngram_order, vector<int> &left_en_wids, vector<int> &right_en_wids)
{
	vector<int> wids = left_en_wids;
	MergeTwoVector(wids, right_en_wids);
    vector<int> c_left_right_wids,c_right_left_wids,c_straight_mid_wids;
    int c_left_size = left_en_wids.size();
    int c_right_size = right_en_wids.size();
    int n1 = c_ngram_order-1 < c_left_size ? c_ngram_order-1 : c_left_size;
    int n2 = c_ngram_order-1 < c_right_size ? c_ngram_order-1 : c_right_size;
	for(int k=c_left_size-n1; k<c_left_size; k++)
		c_left_right_wids.push_back(left_en_wids[k]);
    c_straight_mid_wids = c_left_right_wids;
    for(int k=0; k<n2; k++)
	{
		c_right_left_wids.push_back(right_en_wids[k]);
		c_straight_mid_wids.push_back(right_en_wids[k]);
	}
    double c_ngram_prob = c_ngram_trie->ReadSentenceNgram(c_straight_mid_wids,1) - c_ngram_trie->ReadSentenceNgram(c_left_right_wids,1) - c_ngram_trie->ReadSentenceNgram(c_right_left_wids,1);
	return c_ngram_prob;
}

/*********************************************************************************************************************************/
//函数GetContextBasedTranslationProb
//计算基于上下文的中文词到英文短语的翻译概率
//入口参数：中文词的位置，该中文词对应的英文翻译
//出口参数：概率
/*********************************************************************************************************************************/
double Algorithm::GetContextBasedTranslationProb(int pos, string &tgt_translation)
{
	int cur_word_id = _pulSenChi[pos+1];
	if (m_context_based_translation_models.at(cur_word_id) == NULL)
		return -99;
	//assert (ch_word_vec.size() == _ulSenLenChi)
	//cout<<"sense model found!\n";  //4debug
	int left_bound = max(pos-10,0);
	int right_bound = min(pos+10,_ulSenLenChi-1);
	vector <string> context;
	for (int i=left_bound; i<=right_bound; i++)
	{
		stringstream ss;
		string s;
		ss<<(i-pos);
		ss>>s;
		//cout<<s+"/"+ch_word_vec.at(i)<<endl; //4debug
		context.push_back(s+"/"+ch_word_vec.at(i));
	}
	return m_context_based_translation_models.at(cur_word_id)->eval(context,tgt_translation);
}

/**********************************************************************************************/
//函数 MergeIntoEdge
//功能：将两个孩子节点的候选合并为当前节点的候选翻译
//返回值：成功与否标志
/**********************************************************************************************/
bool Algorithm::MergeIntoEdge(s_SearchSpace* left_searchspace, s_SearchSpace* right_searchspace, int left_kth_best, int right_kth_best, HypoHeap &candsQueue, vector<s_SearchSpace*> &hypo_collection)
{
	//cerr<<"in merge edges 0"<<endl;
	//cerr<<"in merge edges 0.2"<<endl;
    s_SearchSpace* c_tmp_searchspace = new s_SearchSpace();
	//cerr<<"in merge edges 0.5"<<endl;
	hypo_collection.push_back(c_tmp_searchspace);
	//cerr<<"in merge edges 1"<<endl;
    //合并操作
    //记录span的开始，结束与分割点
    int start = left_searchspace->ulFirstWord;
    int end = right_searchspace->ulLastWord;
    int split_point = left_searchspace->ulLastWord;

	int c_left_size = left_searchspace->viEnPhrase.size();
	int c_right_size = right_searchspace->viEnPhrase.size();
    //顺序合并

    //计算调序模型概率
    double c_straight_prob = 0;
    double c_invert_prob = 0;
    if( right_searchspace->ulLastWord - left_searchspace->ulFirstWord < m_reorder_window )
	{
		int c11 = left_searchspace->ulFirstWord;
		int c12 = left_searchspace->ulLastWord;
		int c21 = right_searchspace->ulFirstWord;
		int c22 = right_searchspace->ulLastWord;
		int e11 = left_searchspace->viEnPhrase[0];
		int e12 = left_searchspace->viEnPhrase[c_left_size-1];
		int e21 = right_searchspace->viEnPhrase[0];
		int e22 = right_searchspace->viEnPhrase[c_right_size-1];
		string c_feat_c11 = _pVocabChi->GetWord(_pulSenChi[c11]);
		string c_feat_c12 = _pVocabChi->GetWord(_pulSenChi[c12]);
		string c_feat_c21 = _pVocabChi->GetWord(_pulSenChi[c21]); 
		string c_feat_c22 = _pVocabChi->GetWord(_pulSenChi[c22]); 
		string c_feat_e11 = _pVocabEng->GetWord(e11);
		string c_feat_e12 = _pVocabEng->GetWord(e12);
		string c_feat_e21 = _pVocabEng->GetWord(e21);
		string c_feat_e22 = _pVocabEng->GetWord(e22);
		vector<string> c_feats_vector;
		c_feats_vector.clear();
		c_feats_vector.push_back(c_feat_c11);
		c_feats_vector.push_back(c_feat_c21);
		c_feats_vector.push_back(c_feat_e11);
		c_feats_vector.push_back(c_feat_e21);
		c_feats_vector.push_back(c_feat_c12);
		c_feats_vector.push_back(c_feat_c22);
		c_feats_vector.push_back(c_feat_e12);
		c_feats_vector.push_back(c_feat_e22);
		vector<double> c_reorder_prob_vector = m_me_reorder_model->predict(c_feats_vector);
		if( c_reorder_prob_vector.size() != 2 )
		{
			cerr<<"Computing reorder prob Error!"<<endl;
			exit(-1);
		}
		if( m_me_reorder_model->inOrder )
		{
			c_straight_prob = log10(c_reorder_prob_vector[0]);
			c_invert_prob = log10(c_reorder_prob_vector[1]);
		}
		else
		{
			c_straight_prob = log10(c_reorder_prob_vector[1]);
			c_invert_prob = log10(c_reorder_prob_vector[0]);
		}
	}

    c_tmp_searchspace->ulFirstWord = left_searchspace->ulFirstWord;
    c_tmp_searchspace->ulLastWord = right_searchspace->ulLastWord;
    c_tmp_searchspace->ulEnNum = left_searchspace->ulEnNum + right_searchspace->ulEnNum;
    c_tmp_searchspace->m_phrase_num = left_searchspace->m_phrase_num + right_searchspace->m_phrase_num;
    c_tmp_searchspace->m_straight_reorder_prob = left_searchspace->m_straight_reorder_prob + right_searchspace->m_straight_reorder_prob + c_straight_prob;
    c_tmp_searchspace->m_swap_reorder_prob = left_searchspace->m_swap_reorder_prob + right_searchspace->m_swap_reorder_prob;
    c_tmp_searchspace->m_split_point = split_point;
    //增加该翻译由左右孩子节点的第几个候选翻译合并而来,09.06.06添加
    c_tmp_searchspace->m_left_ith_best = left_kth_best;
    c_tmp_searchspace->m_right_jth_best = right_kth_best;

    c_tmp_searchspace->viEnPhrase = left_searchspace->viEnPhrase;
    MergeTwoVector(c_tmp_searchspace->viEnPhrase, right_searchspace->viEnPhrase);
	c_tmp_searchspace->m_en_str = left_searchspace->m_en_str + " " + right_searchspace->m_en_str;
    //cout<<"before get trans prob..."<<(left_searchspace->eachTransPro).size()<<" "<<(right_searchspace->eachTransPro).size()<<endl;
    for(int k=0; k<(left_searchspace->eachTransPro).size(); k++)
		c_tmp_searchspace->eachTransPro.push_back((left_searchspace->eachTransPro)[k] + (right_searchspace->eachTransPro)[k]);
	c_tmp_searchspace->left_ant = left_searchspace;
	c_tmp_searchspace->right_ant = right_searchspace;
    //cout<<"after get trans prob..."<<endl;
    //计算语言模型的增量
	double c_ngram_prob = _pNgram->TraversalFeaturesImpl(c_tmp_searchspace);
    c_tmp_searchspace->m_en_ngram_prob = left_searchspace->m_en_ngram_prob + right_searchspace->m_en_ngram_prob + c_ngram_prob;
    //计算基于上下文的翻译模型的打分
    c_tmp_searchspace->m_context_based_trans_prob = left_searchspace->m_context_based_trans_prob + right_searchspace->m_context_based_trans_prob;
    //cout<<"mono merge, m_context_based_trans_prob:"<<c_tmp_searchspace->m_context_based_trans_prob<<endl;  //4debug
	c_tmp_searchspace->dPro = left_searchspace->dPro + right_searchspace->dPro + all_Lambda.lm*c_ngram_prob + all_Lambda.reorderStraight*c_straight_prob;
	//cerr<<"in merge edges 2"<<endl;
    //将此边压栈
    candsQueue.Push(c_tmp_searchspace);
	//cerr<<"in merge edges 3"<<endl;

    //逆序合并
    //设定重排序窗口
    if(right_searchspace->ulLastWord - left_searchspace->ulFirstWord > m_reorder_window-1)
		return true;
	if( m_chn2eng_flag )
	{
		int break_flag = 0;
		for(int k=left_searchspace->ulFirstWord; k<=right_searchspace->ulLastWord; k++)
		{
			string::size_type  c_pos = g_Chinese_Punc_Set.find(_pVocabChi->GetWord(_pulSenChi[k]));
			if( c_pos != string::npos )
			{
				break_flag = 1;
				break;
			}
		}		
		if( break_flag )
			return true;
	}

    c_tmp_searchspace = new s_SearchSpace();
	hypo_collection.push_back(c_tmp_searchspace);
	//cerr<<"in merge edges 4"<<endl;
    //构造新边
	c_tmp_searchspace->ulFirstWord = left_searchspace->ulFirstWord;
    c_tmp_searchspace->ulLastWord = right_searchspace->ulLastWord;
    c_tmp_searchspace->ulEnNum = left_searchspace->ulEnNum + right_searchspace->ulEnNum;
    c_tmp_searchspace->m_phrase_num = left_searchspace->m_phrase_num + right_searchspace->m_phrase_num;
    c_tmp_searchspace->m_split_point = split_point;
    //增加该翻译由左右孩子节点的第几个候选翻译合并而来,09.06.06添加
    c_tmp_searchspace->m_left_ith_best = left_kth_best;
    c_tmp_searchspace->m_right_jth_best = right_kth_best;

    //cout<<"before get trans prob..."<<(left_searchspace->eachTransPro).size()<<" "<<(right_searchspace->eachTransPro).size()<<endl;
    for(int k=0; k<(left_searchspace->eachTransPro).size(); k++)
		c_tmp_searchspace->eachTransPro.push_back((left_searchspace->eachTransPro)[k] + (right_searchspace->eachTransPro)[k]);
	c_tmp_searchspace->left_ant = right_searchspace;
	c_tmp_searchspace->right_ant = left_searchspace;
    //cout<<"after get trans prob..."<<endl;
    //计算语言模型的增量

    c_tmp_searchspace->m_straight_reorder_prob = left_searchspace->m_straight_reorder_prob + right_searchspace->m_straight_reorder_prob;
    c_tmp_searchspace->m_swap_reorder_prob = left_searchspace->m_swap_reorder_prob + right_searchspace->m_swap_reorder_prob + c_invert_prob;
    c_tmp_searchspace->viEnPhrase = right_searchspace->viEnPhrase;
    MergeTwoVector(c_tmp_searchspace->viEnPhrase, left_searchspace->viEnPhrase);
	c_tmp_searchspace->m_en_str = right_searchspace->m_en_str + " " + left_searchspace->m_en_str;
    //计算语言模型增量
	c_ngram_prob = _pNgram->TraversalFeaturesImpl(c_tmp_searchspace);
    c_tmp_searchspace->m_en_ngram_prob = left_searchspace->m_en_ngram_prob + right_searchspace->m_en_ngram_prob + c_ngram_prob;
    //计算基于上下文的翻译模型的打分
    c_tmp_searchspace->m_context_based_trans_prob = left_searchspace->m_context_based_trans_prob + right_searchspace->m_context_based_trans_prob;
    //cout<<"swap merge, m_context_based_trans_prob:"<<c_tmp_searchspace->m_context_based_trans_prob<<endl;  //4debug
    c_tmp_searchspace->dPro = left_searchspace->dPro + right_searchspace->dPro + all_Lambda.lm*c_ngram_prob + all_Lambda.reorderSwap*c_invert_prob;
    //将此边压栈
	//cerr<<"in merge edges 5"<<endl;
    candsQueue.Push(c_tmp_searchspace);
	//cerr<<"in merge edges 6"<<endl;

    return true;

}

/***********************cube prunning 结束*****************************************************/

/**********************************************************************************************/
//翻译一行：一行＝待翻译原文＋语法调序跨度及概率
//输入：一行句子
//输出：英文句子，Nbest,特征打分
/**********************************************************************************************/
bool Algorithm::TranslateLine(char* pcSenChi,  char* pcSenEng, vector<string> &nBestList, vector< vector<double> > &feats, SegFormate segtype)
{
    //将待翻译原文与语法调序跨度及概率分开
    string c_line = pcSenChi;
    string::size_type c_pos = c_line.find("#*#");
    if( c_pos == string::npos )
    {
	cerr<<"The input is Error!"<<endl;
	return false;
    }
    string c_chi_str = c_line.substr(0,c_pos);
    //记录原文
    strcpy(pcSenChi,c_chi_str.c_str());
    cout<<pcSenChi<<endl;
    
    //存储语法跨度及其概率
    c_line.erase(0,c_pos+3);
    if( c_line.find("NULL") == string::npos )
    {
	string::size_type c_span_pos = c_line.find("|||");
	while( c_span_pos != string::npos )
	{
	    m_span_vector.push_back(c_line.substr(0,c_span_pos));
	    c_line.erase(0,c_span_pos+3);
	    c_span_pos = c_line.find("|||");
	}
	//可能是出问题的地方
	m_span_vector.push_back(c_line);
    }
    //cout<<m_span_vector.size()<<endl;
    //翻译原文
    TranslateSentence(pcSenChi,pcSenEng,nBestList,feats,segtype);
    
    return true;
}

/**********************************************************************************************/
//翻译句子
//输入: 中文句子,分词格式
//输出: 英文句子，Nbest, 特征打分
/**********************************************************************************************/
bool 
Algorithm::TranslateSentence(char* pcSenChi,  char* pcSenEng, vector<string> &nBestList, vector< vector<double> > &feats, SegFormate segtype)
{

        //cout<<"in trans 1"<<endl;

	if( segtype == CHARACTER )
	{
	    _parsetype = WORDS;
	    cerr<<"Sorry, now the input type is not supported!"<<endl;
	    return false;
	}
	else
	    _parsetype = SPACE;

	char m_pcSenChi[SEN_CHAR_MAX];
	bool m_bTemp=false;

	while(1)
	{
	    strcpy(m_pcSenChi,pcSenChi);
	    m_SenLen = 0;
		/*
	    char* words[SEN_LEN_MAX+1];
	    for( int i=0; i<SEN_LEN_MAX+1; ++i )
		{
			words[i] = new char[WORD_LEN_MAX+1];
		}
		*/
		//vector<string> words;
		ch_word_vec.clear();

	    //重新分词
// 			if( segtype == NOSEG )
// 			{
// 				reseg(ChiSen);
// 			}

	    //将句子分割为词,在最后添加了一个"0"
	    m_SenLen = Vocab::ParseWords(m_pcSenChi, ch_word_vec, SEN_LEN_MAX, _parsetype);
	    if( m_SenLen > SEN_LEN_MAX )
		{
			m_SenLen = SEN_LEN_MAX;
		}

	    if( !m_SenLen )
	    {
		cerr<<"Sentence Parsing Error!"<<endl;
		break;
	    }


	    //如果当前句子的长度大于前面一个句子的长度时,重新分配内存
	    if( _ulSenLenMax < m_SenLen )  
	    {
		int m_i = 0;
		int m_j = 0;



		//先将所有的内存释放掉
		if( _ulSenLenMax != 0)
		{
		    delete []_pstSearchSpace[0];
		    _pstSearchSpace[0] = NULL;
		    for( m_i = 1; m_i<_ulSenLenMax+1; m_i++ )
		    {
			delete []_pstSearchSpace[m_i]; 
			_pstSearchSpace[m_i] = NULL;
			delete []m_chart[m_i];
			m_chart[m_i] = NULL;
			//以下内容为张家俊08年12月25日改写
			delete []m_syn_flag[m_i];
			m_syn_flag[m_i] = NULL;
			if( printNbest )
			{
			    delete []m_order_flag[m_i];
			    m_order_flag[m_i] = NULL;
			    delete []m_nbest_max[m_i];
			    m_nbest_max[m_i] = NULL;
			    delete []m_nbest_cands[m_i];
			    m_nbest_cands[m_i] = NULL;
			    delete []m_search_space_queue[m_i];
			    m_search_space_queue[m_i] = NULL;
			    delete []m_search_space_order_vector[m_i];
			    m_search_space_order_vector[m_i] = NULL;
			    delete []m_split_best_vector[m_i];
			    m_split_best_vector[m_i] = NULL;
			    delete []m_other_space_vector[m_i];
			    m_other_space_vector[m_i] = NULL;
			}
		    }
		    delete []_pstSearchSpace;				 //搜索候选项
		    _pstSearchSpace = NULL;
			for( int k = 0; k < m_hypo_collection.size(); k++ )
				delete m_hypo_collection[k];
			m_hypo_collection.clear();
		    delete []m_chart;
		    m_chart = NULL;
		    //以下内容为张家俊08年12月25日改写
		    delete []m_syn_flag;
		    m_syn_flag = NULL;
		    if( printNbest )
		    {
			delete []m_order_flag;
			m_order_flag = NULL;
			delete []m_nbest_max;
			m_nbest_max = NULL;
			delete []m_nbest_cands;
			m_nbest_cands = NULL;
			delete []m_search_space_queue;
			m_search_space_queue = NULL;
			delete []m_search_space_order_vector;
			m_search_space_order_vector = NULL;
			delete []m_split_best_vector;
			m_split_best_vector = NULL;
			delete []m_other_space_vector;
			m_other_space_vector = NULL;
		    }
		    m_syn_prob_map.clear();

		    //delete []_pstHypoStack;                  //hypotheses 栈
		    //_pstHypoStack = NULL;
		}

		//每个Phrase的候选翻译存放到一个Search Space Stock 中，
		//由同一个词开始的Search Space Stock 存放到一个数组中
		//cerr<<"new space ..."<<endl;
		_pstSearchSpace = new HypoHeap*[m_SenLen+1];
		if( !_pstSearchSpace )
		{
		    cerr<<"Allocate Memory for Search Space Error!\n";
		}
		m_chart = new vector<s_SearchSpace*>*[m_SenLen+1];
		if( !m_chart )
		{
		    cerr<<"Allocate Memory for parsing chart Error!\n";
		}
		//以下内存申请为张家俊08年12月25日添加
		m_syn_flag = new int*[m_SenLen+1];
		if( !m_syn_flag )
		{
		    cerr<<"Allocate Memory for syntax phrase flag Error!"<<endl;
		}
		if( printNbest )
		{
		    m_order_flag = new int* [m_SenLen+1];
		    m_nbest_max = new int* [m_SenLen+1];
		    m_search_space_order_vector = new vector<s_SearchSpace*>*[m_SenLen+1];
		    m_split_best_vector = new vector<s_SearchSpace*>*[m_SenLen+1];
		    m_other_space_vector = new vector<s_SearchSpace*>*[m_SenLen+1];
		    m_nbest_cands = new vector<s_SearchSpace>*[m_SenLen+1];
		    m_search_space_queue = new priority_queue<s_SearchSpace, vector<s_SearchSpace>, SearchSpaceComp>*[m_SenLen+1];
		    if( !m_order_flag || !m_nbest_max || !m_nbest_cands || !m_search_space_queue || !m_search_space_order_vector || !m_split_best_vector || !m_other_space_vector )
		    {
			cerr<<"Allocate Memory for m_order_flag  m_search_space_order_vector, m_nbest_cands or m_search_space_queue Error!"<<endl;
		    }
		}

		//
		//第一个Stack用于存储NULL->English
		//
		_pstSearchSpace[0] = new HypoHeap[1];             
		if( !_pstSearchSpace[0] )
		{
		    cerr<<"Allocate Memory for Search Space Error!\n";   
		}
		_pstSearchSpace[0][0].Init(MINHEAP, TRAN_TABLE_SIZE);
		//初始化第一个元素，为张家俊08年12月25日添加
		m_syn_flag[0] = NULL;
		m_chart[0] = NULL;
		if( printNbest )
		{
		    m_order_flag[0] = NULL;
		    m_nbest_max[0] = NULL;
		    m_search_space_order_vector[0] = NULL;
		    m_split_best_vector[0] = NULL;
		    m_other_space_vector[0] = NULL;
		    m_nbest_cands[0] = NULL;
		    m_search_space_queue[0] = NULL;
		}
		//
		//每个数组里Stack的数量不超过Phrase的最大长度，并且不会超过当前位置到句子结尾的长度
		//
		for(  m_i = 1; m_i<m_SenLen+1; m_i++ )
		{
		    //	int m_stacklen =  (m_SenLen-m_i+1) > PHRASE_LEN_MAX ? PHRASE_LEN_MAX :(m_SenLen-m_i+1);
		    //	CKY初始化chart线图的大小,为张家俊08年12月25日添加
		    int m_stacklen = m_SenLen-m_i+1;
		    _pstSearchSpace[m_i] = new HypoHeap[m_stacklen];
		    if( !_pstSearchSpace[m_i] )
		    {
			cerr<<"Allocate Memory for Search Space Error!\n";
		    }
		    for(m_j=0; m_j<m_stacklen; m_j++)
		    {
			_pstSearchSpace[m_i][m_j].Init(MINHEAP, TRAN_TABLE_SIZE);
		    }
		    //以下空间申请为张家俊08年12月25日添加
		    m_syn_flag[m_i] = new int[m_stacklen];
		    m_chart[m_i] = new vector<s_SearchSpace*>[m_stacklen];
		    if( !m_syn_flag[m_i] || !m_chart[m_i] )
		    {
			cerr<<"Allocate Memory for syntax phrase flag m_syn_flag[m_i] or m_chart[m_i] in second dimension Error!"<<endl;
		    }
		    for(int k=0; k<m_stacklen; k++)
		    {
			m_syn_flag[m_i][k] = 0;
			m_chart[m_i][k].clear();
		    }
				
		    if( printNbest )
		    {
			m_order_flag[m_i] = new int[m_stacklen];
			m_nbest_max[m_i] = new int[m_stacklen];
			m_search_space_order_vector[m_i] = new vector<s_SearchSpace*>[m_stacklen];
			m_split_best_vector[m_i] = new vector<s_SearchSpace*>[m_stacklen];
			m_other_space_vector[m_i] = new vector<s_SearchSpace*>[m_stacklen];
			m_nbest_cands[m_i] = new vector<s_SearchSpace>[m_stacklen];
			m_search_space_queue[m_i] = new priority_queue<s_SearchSpace,vector<s_SearchSpace>,SearchSpaceComp>[m_stacklen];
			if( !m_order_flag[m_i] || !m_nbest_max || !m_search_space_order_vector[m_i] || !m_split_best_vector[m_i] || !m_other_space_vector[m_i] || !m_nbest_cands[m_i] || !m_search_space_queue[m_i] )
			{
			    cerr<<"Allocate Memory for m_search_space[m_i], m_nbest_cands[m_i] or m_search_space_queue[m_i] in second dimension Error!"<<endl;
			}
			for(int j=0; j<m_stacklen; j++)
			{
			    m_order_flag[m_i][j] = 0;
			    m_nbest_max[m_i][j] = 0;
			    m_nbest_cands[m_i][j].clear();
			    //m_search_space_queue[m_i][j].clear();
			    m_search_space_order_vector[m_i][j].clear();
			    m_split_best_vector[m_i][j].clear();
			    m_other_space_vector[m_i][j].clear();
			}
		    }

		}
	    }

		//cerr<<"before NP parsing"<<endl;
	    //将其中给出的NP翻译提取出来
	    //NPParsing( words,m_SenLen );
	    //cerr<<"Sentence Length: "<<m_SenLen<<endl;
	    if( !m_SenLen )
		{
			cerr<<"NP Parsing Error!"<<endl;
			break;
		}

	    if( m_SenLen > SEN_LEN_MAX )
			cerr<<"The Sentence is too long"<<endl;


	    //
	    //句子用词号表示时，存储位置从1开始，0代表句子的开始符<s>
	    //
	    //cerr<<"get word index"<<endl;
	    _ulSenLenChi =	_pVocabChi->GetAndInsertWordsIndics( ch_word_vec, &_pulSenChi[1],  SEN_LEN_MAX );   //句子用词号表示时
	    //cerr<<"word index complete!"<<endl;
	    //for(size_t i=1;i<Vocab::length(_pulSenChi);i++)
		//	cerr<<_pulSenChi[i]<<" ";
	    //cerr<<endl;
	    //cerr<<Vocab::length(_pulSenChi)<<endl;
	    //cerr<<_ulSenLenChi<<endl;


	    if( !_ulSenLenChi )
		{
			cerr<<"Get Chinese Word Index Error!"<<endl;
			break;
		}

		//cerr<<"delete ..."<<SEN_LEN_MAX<<endl;
		/*
	    for( int i=0; i<SEN_LEN_MAX+1; ++i)
		{
			delete[] words[i];
		}
		*/
		//cerr<<"after deletion ..."<<endl;


	    //生成搜索候选项,也即完成CKY初始化的功能，张家俊08年12月25日添加
	    vector< vector< pair<string,double> > > SenChiCN;
	    //cerr<<"before gensearchspace"<<endl;
	    m_bTemp = GenSearchSpace(SenChiCN);
	    //cerr<<"after gensearchspace"<<endl;
	    if( !m_bTemp)
		{
			cerr<<"Generate Search Space Error!\n";
			break;
		}

	    //翻译句子，两种解码方法：一是利用cube prunning + beam；二是直接利用beam
		//第一种速度快，第二种的搜索错误更少一点
		if( m_cube_flag )
			m_bTemp = CubeParse(_ulSenLenChi,pcSenEng,nBestList,feats);
		else
		{
			cerr<<"ERROR! It is now only cube pruning supported!"<<endl;
			exit(-1);
		}
	    if( !m_bTemp)
	    {
		cerr<<"CKY parse Error!"<<endl;
		break;
	    }

	    break;
	}

	if( _ulSenLenMax < _ulSenLenChi )
	{
	    _ulSenLenMax = m_SenLen;                  //在翻译过程中仍然可以用_ulSenLenMax,这样NULL->English生成的搜索候选项不变
	}
	if( !m_bTemp )
	{
	    return false;
	}
	return true;

}


/**********************************************************************************************/
//翻译文件
/**********************************************************************************************/
bool 
Algorithm::TranslateFile(const char* test_file, const char* sense_file, const char* result_file, FileFormate filetype, SegFormate segtype)
{
	
	ifstream inputfile(test_file);
	ofstream outputfile(result_file);
	ofstream nbestfile;

	if( !inputfile )
	{
		cerr<<"can not open test file!"<<endl;
		return 0;
	}

	if( !outputfile )
	{
		cerr<<"can not open result file!"<<endl;
		return 0;
	}

	if( printNbest )
	{
		nbestfile.open( nBestFileName.c_str() );
		if( !nbestfile )
		{
			cerr<<"can not open nbest file!\n";
			return 0;
		}
	}
	int num=0;
	char ChiSen[SEN_CHAR_MAX];
	char EngSen[SEN_CHAR_MAX];
	//文本格式
	if( filetype == TXT )
	{
		num=0;
		while( inputfile.getline(ChiSen, SEN_CHAR_MAX) )
		{
			if( strlen(ChiSen) < 1 )
				continue;
			
			vector<string> nBestlist;
			vector< vector<double> > feats;
			num++;
			cout<<"Translating The Sentence "<<num<<endl;
			cout<<ChiSen<<endl;

			//下面翻译一行的函数由张家俊09年1月8日添加
			TranslateSentence(ChiSen, EngSen, nBestlist, feats, segtype);

			cout<<EngSen<<endl<<endl;
			outputfile<<EngSen<<endl;

			if( printNbest  )
			{
				if( !nbestfile )
				{
					cout<<"can not write to Nbest file!\n";
					return 0;
				}
				for( unsigned int m_i=0; m_i<nBestlist.size(); m_i++ )
				{
					nbestfile<<num-1<<" ||| "<<nBestlist[m_i].c_str()<<" ||| ";
					for( unsigned int feat_j = 0; feat_j < feats[m_i].size(); feat_j++ )
						nbestfile<<feats[m_i][feat_j]<<" ";
					double c_prob = 0;
					int c_size = all_Lambda.trans.size();
					for( unsigned int i = 0; i < c_size; i++ )
						c_prob += all_Lambda.trans[i]*feats[m_i][i];
					c_prob += all_Lambda.len*feats[m_i][c_size];
					c_prob += all_Lambda.lm*feats[m_i][c_size+1];	
					c_prob += all_Lambda.reorderStraight*feats[m_i][c_size+2];	
					c_prob += all_Lambda.reorderSwap*feats[m_i][c_size+3];	
					c_prob += all_Lambda.dPhraseNum*feats[m_i][c_size+4];
					c_prob += all_Lambda.sense*feats[m_i][c_size+5];	//增加基于上下文的翻译概率
					nbestfile<<"||| "<<c_prob;
					nbestfile<<endl;
				}
			}
		}
	}
	//863格式
	else if( filetype == XML )
	{
		while( inputfile.getline(ChiSen,SEN_CHAR_MAX) )
		{

			string line(ChiSen);
			int pos;
			pos=line.find("<?xml");
			if( pos != string::npos )
			{				
				outputfile<<line.c_str()<<endl;
				continue;
			}

			pos = line.find("<srcset");
			if( pos != string::npos )
			{
				line.replace(0,7,"<tstset");
				int pos_tmp = line.find_last_of(">",line.size()-1);
				line.erase(pos_tmp, line.size()-pos_tmp);
				//line.erase(line.size()-1,1);
				line += " tgtlang=\"en\">";

				outputfile<<line.c_str()<<endl;
				continue;
			}

			
			if( line.find("</doc>") != string::npos || line.find("<p>") != string::npos || line.find("</p>") != string::npos )
			{
				outputfile<<line.c_str()<<endl;
				continue;
			}

			if( line.find("</srcset>") != string::npos )
			{
				outputfile<<"</tstset>"<<endl;
				break;
			}	

			pos = line.find("<doc ");
			if( pos != string::npos )
			{
				//line.erase(line.size()-1,1);
				int pos_tmp = line.find_last_of(">",line.size()-1);
				line.erase(pos_tmp, line.size()-pos_tmp);

				line += " site=\"nlpr\">";
				outputfile<<line<<endl;
				continue;
			}

			pos = line.find("<s id=");
			if( pos != string::npos )
			{
				vector<string> nBestlist;
				vector< vector<double> > feats;
				string prepos;

				pos = line.find_first_of(">",0);
				prepos = line.substr(0,pos+1);
				line.erase(0,pos+1);

				pos = line.find_last_of("<",line.size()-1);
				line.erase(pos,line.size()-pos);
				//下面两行由张家俊08年12月31日添加
				//line = "<s> " + line;
				//line = line + " </s>";

				strcpy(ChiSen, line.c_str() );


				num++;
				cout<<"Translating The Sentence "<<num<<endl;
				cout<<ChiSen<<endl;
				
				//下面翻译一行的函数由张家俊09年1月8日修改
				TranslateSentence(ChiSen, EngSen, nBestlist, feats, segtype);

				cout<<EngSen<<endl<<endl;;	
				outputfile<<prepos<<EngSen<<"</s>"<<endl;

				if( printNbest)
				{
					for( unsigned int m_i=0; m_i<nBestlist.size(); m_i++ )
					{
						nbestfile<<num-1<<" ||| "<<nBestlist[m_i].c_str()<<" ||| ";
						for( unsigned int feat_j = 0; feat_j < feats[m_i].size(); feat_j++ )
							nbestfile<<feats[m_i][feat_j]<<" ";
						double c_prob = 0;
						int c_size = all_Lambda.trans.size();
						for( unsigned int i = 0; i < c_size; i++ )
							c_prob += all_Lambda.trans[i]*feats[m_i][i];
						c_prob += all_Lambda.len*feats[m_i][c_size];
						c_prob += all_Lambda.lm*feats[m_i][c_size+1];	
						c_prob += all_Lambda.reorderStraight*feats[m_i][c_size+2];	
						c_prob += all_Lambda.reorderSwap*feats[m_i][c_size+3];	
						c_prob += all_Lambda.dPhraseNum*feats[m_i][c_size+4];
						c_prob += all_Lambda.sense*feats[m_i][c_size+5];	//增加基于上下文的翻译概率
						nbestfile<<"||| "<<c_prob;
						nbestfile<<endl;
					}
				}
			}		
		}
	}

	inputfile.close();
	outputfile.close();
	if( printNbest )
	{
		nbestfile.close();
	}
	return 1;
}

/**********************************************************************************************/
//将NP还原
/**********************************************************************************************/
void
Algorithm::NPRevert(s_Hypotheses *m_pHypotheses, vector<int> &sentence)
{

	int m_wordNULLIndex = _pVocabEng->UnNull();			//"NULL"的词号
	int m_wordNumberIndex = _pVocabEng->NumberIndex();	//"<_NUMBER>"的词号
	int m_wordTimeIndex = _pVocabEng->TimeIndex();		//"<_TIME>"的词号
	int m_wordNEIndex = _pVocabEng->NEIndex();			//"<_NE>"的词号
	int m_wordLOCIndex = _pVocabEng->LocIndex();		
	int m_wordPERSONIndex = _pVocabEng->PersonIndex();	
	int m_wordNAMEXIndex = _pVocabEng->NamexIndex();	
	int m_wordORGIndex = _pVocabEng->OrgIndex();

	int m_wordPosition = -1;
	int m_PhraseLen = m_pHypotheses->pstLastPhrase->ulEnNum;

	for( int i=m_PhraseLen-1; i>=0; i-- )
	{
		int m_wordIndex = m_pHypotheses->pstLastPhrase->viEnPhrase[i];
		if( m_wordIndex == m_wordNULLIndex )
		{
			continue;
		}
		//Number
		else if( m_wordIndex == m_wordNumberIndex )
		{
			m_wordPosition = 0;
			for( int j=m_pHypotheses->pstLastPhrase->ulFirstWord; j<=m_pHypotheses->pstLastPhrase->ulLastWord; j++ )
			{
				if( _pulSenChi[j] == m_wordNumberIndex )
				{
					m_wordPosition = j;
					break;
				}
			}
			if( m_wordPosition )
			{
				if( _mapNPTransTalbe.count( m_wordPosition ) )
				{
					sentence.push_back( _mapNPTransTalbe[m_wordPosition] );
				}
			}
		}
		//Time
		else if( m_wordIndex == m_wordTimeIndex )
		{
			m_wordPosition = 0;
			for( int j=m_pHypotheses->pstLastPhrase->ulFirstWord; j<=m_pHypotheses->pstLastPhrase->ulLastWord; j++ )
			{
				if( _pulSenChi[j] == m_wordTimeIndex )
				{
					m_wordPosition = j;
					break;
				}
			}
			if( m_wordPosition )
			{
				if( _mapNPTransTalbe.count( m_wordPosition ) )
				{
					sentence.push_back( _mapNPTransTalbe[m_wordPosition] );
				}
			}
		}
		//loc
		else if( m_wordIndex == m_wordLOCIndex )
		{
			m_wordPosition = 0;
			for( int j=m_pHypotheses->pstLastPhrase->ulFirstWord; j<=m_pHypotheses->pstLastPhrase->ulLastWord; j++ )
			{
				if( _pulSenChi[j] == m_wordLOCIndex )
				{
					m_wordPosition = j;
					break;
				}
			}
			if( m_wordPosition )
			{
				if( _mapNPTransTalbe.count( m_wordPosition ) )
				{
					sentence.push_back( _mapNPTransTalbe[m_wordPosition] );
				}
			}
		}
		//person
		else if( m_wordIndex == m_wordPERSONIndex )
		{
			m_wordPosition = 0;
			for( int j=m_pHypotheses->pstLastPhrase->ulFirstWord; j<=m_pHypotheses->pstLastPhrase->ulLastWord; j++ )
			{
				if( _pulSenChi[j] == m_wordPERSONIndex )
				{
					m_wordPosition = j;
					break;
				}
			}
			if( m_wordPosition )
			{
				if( _mapNPTransTalbe.count( m_wordPosition ) )
				{
					sentence.push_back( _mapNPTransTalbe[m_wordPosition] );
				}
			}
		}
		//namex
		else if( m_wordIndex == m_wordNAMEXIndex )
		{
			m_wordPosition = 0;
			for( int j=m_pHypotheses->pstLastPhrase->ulFirstWord; j<=m_pHypotheses->pstLastPhrase->ulLastWord; j++ )
			{
				if( _pulSenChi[j] == m_wordNAMEXIndex )
				{
					m_wordPosition = j;
					break;
				}
			}
			if( m_wordPosition )
			{
				if( _mapNPTransTalbe.count( m_wordPosition ) )
				{
					sentence.push_back( _mapNPTransTalbe[m_wordPosition] );
				}
			}
		}
		//org
		else if( m_wordIndex == m_wordORGIndex )
		{
			m_wordPosition = 0;
			for( int j=m_pHypotheses->pstLastPhrase->ulFirstWord; j<=m_pHypotheses->pstLastPhrase->ulLastWord; j++ )
			{
				if( _pulSenChi[j] == m_wordORGIndex )
				{
					m_wordPosition = j;
					break;
				}
			}
			if( m_wordPosition )
			{
				if( _mapNPTransTalbe.count( m_wordPosition ) )
				{
					sentence.push_back( _mapNPTransTalbe[m_wordPosition] );
				}
			}
		}
		//NE
		else if( m_wordIndex == m_wordNEIndex )
		{
			m_wordPosition = 0;
			for( int j=m_pHypotheses->pstLastPhrase->ulFirstWord; j<=m_pHypotheses->pstLastPhrase->ulLastWord; j++ )
			{
				if( _pulSenChi[j] == m_wordNEIndex )
				{
					m_wordPosition = j;
					break;
				}
			}
			if( m_wordPosition )
			{
				if( _mapNPTransTalbe.count( m_wordPosition ) )
				{
					sentence.push_back( _mapNPTransTalbe[m_wordPosition] );
				}
			}
		}
		else
		{
			sentence.push_back( m_wordIndex  );
		}
	}
}

void 
Algorithm::TrueCasing(char* sentence)
{
	if( sentence[0] >= 0x61 && sentence[0] <= 0x7A )
	{
		sentence[0] = sentence[0] - 0x20;
	}
}

void
Algorithm::TrueCasing(string &sentence)
{
	if( sentence[0] >= 0x61 && sentence[0] <= 0x7A )
	{
		sentence[0] = sentence[0] - 0x20;
	}
}
