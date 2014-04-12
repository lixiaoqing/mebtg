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
//��ʼ������
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
	//�������ݱ����ĳ�ʼ��Ϊ�żҿ�08��12��25�����
	m_syn_flag = NULL;
	m_order_flag = NULL;
	m_nbest_max = NULL;
	m_span_vector.clear();
	m_syn_prob_map.clear();
	m_nbest_cands = NULL;
	m_chart = NULL; //09.06.05���
	m_spans.clear();
	m_search_space_queue = NULL;
	m_search_space_order_vector = NULL;
	m_split_best_vector = NULL;
	m_other_space_vector = NULL;

	//�������ģ�ͱ�����ʼ�����żҿ�09��1��5�����
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
	
	//������������ĵķ���ģ�ͱ�����ʼ������С��14��3��29�����
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

	//���ݲ������϶Զ���Ժ�����ģ�ͽ�������
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

	//Ϊÿ������Զ�ȡ�ʶ�����Ϣ������С��2014��4��4�����
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
//����
/**********************************************************************************************/
Algorithm::~Algorithm()
{

	delete []_pulSenChi;
	_pulSenChi = NULL;
	delete []_pulSenEng;
	_pulSenEng = NULL;
	//�����޸����żҿ�08��12��26�����
	//delete []_pstHypoStack;
	//_pstHypoStack = NULL;

	//delete []_pdFutCostTable;
	//_pdFutCostTable = NULL;
	delete []_pstSearchSpace[0];
	_pstSearchSpace[0] = NULL;
	m_spans.clear();
	
	//ɾ�����������ĵķ���ģ�ͣ�����С��2014��4��7�����
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
// 1.��Ҫ���ܣ�����������и�����NP�ķ�����ȡ����
// 2.��ڲ���������ľ��ӣ����ӳ���
// 3.���ڲ�������ȡ��NP�����ľ��ӳ���
// 4.�������ڣ�2006.04.10
// 5.�㷨��飺�������б��Ϊ<_NUMBER> ʮ �� || fourteen <NUMBER> �����Ĳ�����ȡ������
//				�����ѵ���������Ѿ��滻�ģ�ֱ�ӽ����滻���� <_NUMBER>,������Է��뵽������һ������
//				�����û�н����滻�ģ���<_NORMAL> ..|| .. <NORMAL>��ǵģ��������ֱ�ӷŵ������ѡ����У�
//				ͬһ����ķ����ѡ���޶�������ϣ���������������ѡ��
//
//
int Algorithm::NPParsing( char** words, int &SenLen )
{
	_bitClosed.reset();

	_mapNPTransTalbe.clear();	//����������
	int i=0;
	int m_len=0;			//���ӵ�ʵ�ʳ���
	char* m_sentence[SEN_LEN_MAX];
	string m_word;
	while( words[i] != 0 && words[i][0] != 0 )
	{
		m_word=words[i++];

		//����
		if( m_word == g_Number_Start )
		{
		}
		//ʱ��
		else if( m_word == g_Time_Start )
		{
		}
		//����
		else if( m_word == g_Loc_Start )
		{
		}
		//����
		else if( m_word == g_Person_Start )
		{
		}
		//��֯������
		else if( m_word == g_Org_Start )
		{
		}
		//�����
		else if( m_word == g_Namex_Start )
		{
		}
		//����ʵ��
		else if( m_word == g_NE_Start )
		{
		}
		//���Ϊһ�����򽫷���Էŵ�������ѡ����У�ͬʱ������ö�����������ķ���ѡ��
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
// 1.��Ҫ���ܣ�����������ѡ��
// 2.��ڲ�������
// 3.���ڲ������Ƿ�ɹ�
// 4.�������ڣ�2009.10.10
// 5.�㷨��飺�ӵ�һ���ʿ�ʼ���������п��ܵĶ������ڷ�����ʱ��к����������������������ѡ����
//
//
bool Algorithm::GenSearchSpace(vector< vector< pair<string,double> > > SenChiCN)
{
	PhraseProTrie *m_PhraseTrie;        //��¼�ڵ�
	s_SearchSpace* m_SearchSpaceTemp;
	char m_pcSenChi[SEN_CHAR_MAX];
	char* words[SEN_LEN_MAX+1];
	for( int k=0; k<SEN_LEN_MAX+1; ++k )
	{
		words[k] = new char[WORD_LEN_MAX+1];
	}
	//normal text
	{
		int i=0, j=0;            //��¼�ʵ�λ��
		int len=0;               //����ĳ���

		//for(size_t k=0;k<Vocab::length(_pulSenChi);k++)
		//   cout<<_pulSenChi[k]<<" ";
		//cout<<endl;
		for(i=1; i<= _ulSenLenChi; i++)		//��ʼλ��,��ʾ���ľ��Ӵʺŵ������1��ʼ
		{
			//bool m_bTemp = false;
			for(j=i, len = 0; j<=_ulSenLenChi && len<PHRASE_LEN_MAX;j++,len++ )	//��ǰ�ʵ�λ��
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

				//����ڷ�����ʱ����������������еķ����ѡ����뵽������ѡ��ջ��
				if( m_PhraseTrie )                                             //��������Ĳ��뷽ʽ�����ܲ�����ô?
				{
					if( !m_PhraseTrie->value().size()  && len == 0 )       //δ��¼�ʣ����ö���û�з����ѡ���Ҷ��ﳤ��Ϊ1
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
							//������޸����żҿ�08��12��31�����
							m_SearchSpaceTemp->viEnPhrase.push_back( _pVocabEng->UnNull() );
							m_SearchSpaceTemp->m_en_str = g_Vocab_NULL;
						}
						else
						{
							m_SearchSpaceTemp->viEnPhrase.clear();
							m_SearchSpaceTemp->viEnPhrase.push_back( _pVocabEng->GetAndInsertIndex(_pVocabChi->GetWord( _pulSenChi[i] ).c_str()) );
							m_SearchSpaceTemp->m_en_str = _pVocabChi->GetWord(_pulSenChi[i]);
						}
						//���漸�������������żҿ�08��12��25�����
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
							//���漸�������������żҿ�08��12��25�����
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
							//���ӻ��������ĵķ�����ʣ���С����14��3��31�����
							//�˴����Ķ˷��뵥ԪΪ���Ϊ[i,j]�Ķ���
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
			if(  !_bitClosed.test(i) && len == 0)                                 //�ô�Ϊδ��¼�ʣ�����ΪNULL
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
					//�����������żҿ�08��12��31���޸�
					m_SearchSpaceTemp->viEnPhrase.push_back( _pVocabEng->UnNull() );
					m_SearchSpaceTemp->m_en_str = g_Vocab_NULL;
				}
				else
				{
					m_SearchSpaceTemp->viEnPhrase.clear();
					m_SearchSpaceTemp->viEnPhrase.push_back( _pVocabEng->GetAndInsertIndex(_pVocabChi->GetWord( _pulSenChi[i] ).c_str()) );
					m_SearchSpaceTemp->m_en_str = _pVocabChi->GetWord(_pulSenChi[i]);
				}
				//���漸�������������żҿ�08��12��25�����
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
	//�����ǰ���ӵĳ���С��ǰ����ľ��ӣ���NULL->English��������ѡ������¼���
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
				//���漸�������������żҿ�08��12��25�����
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
// 1.��Ҫ���ܣ����ʺ�ת��ΪӢ�ĵ���
// 2.��ڲ�����Ӣ�ľ��ӵ�ָ��
// 3.���ڲ������Ƿ�ɹ�
// 4.�������ڣ�2006.04.10
// 5.�㷨��飺


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
//ȥ����������Ŀմ�
//���룺������
//������ɹ�����־
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

/***********************09.06.05��� cube prunning*********************************************/
//�ж�����Ŀ�귭���ѡ�Ƿ���ͬ
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
// ����CubeParse
// ���ܣ�����cube prunning �㷨��Ч�ز������շ�����
// ���أ��ɹ�����־
/**********************************************************************************************/
bool Algorithm::CubeParse(int m_SenLen, char* pcSenEng, vector<string> &nBestList, vector< vector<double> > &feats)
{
	//int threadNum = 10;

	nBestList.clear();
	feats.clear();

	//��span=1�ĺ�ѡ���붼���뵽m_chart��
	for( int i = 1; i <= m_SenLen; i++ )
	{
		if( _pstSearchSpace[i][0].Count() )
		{
			s_SearchSpace* c_ptr_searchspace = NULL;
			//����������ѭ������,��Ϊ������﷭���ļ���ջ�Ѿ�������������˳����뼴��
			while( (c_ptr_searchspace = _pstSearchSpace[i][0].Pop()) )
			{
				m_chart[i][0].push_back(c_ptr_searchspace);
			}
			//����_pstSearchSpace����С�ѣ�������Ҫ�����������
			reverse(m_chart[i][0].begin(), m_chart[i][0].end());
		}
	}

	for( int c_span=2; c_span<=m_SenLen; c_span++ )
	{
		m_spans.clear(); //�ڽ������г�����ͬ�Ŀ��ʱ�������ȶԴ洢��ȵ��������
		for(int start=1; start<=m_SenLen-c_span+1;start++)
		{
			int end=start+c_span-1;

			if( m_chn2eng_flag )
			{
				//�м��ж��ţ��򲻷��룬�Դﵽ�Ӿ仮�ֵ�Ŀ��
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
			else //Ӣ�Ļ���������
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
	//��¼���ŷ���
	if(!m_chart[1][m_SenLen-1].size())
	{
		cerr<<"Cannot find the best translation!"<<endl;
		return false;
	}
	s_SearchSpace* c_pTemp_searchspace = m_chart[1][m_SenLen-1][0];
	MoveNullWord(c_pTemp_searchspace->viEnPhrase);
	string c_best_trans_str = GetEngStr(c_pTemp_searchspace->viEnPhrase);
	//TrueCasing( c_best_trans_str ); //��ʱ���ô�д����ĸ
	strcpy(pcSenEng, c_best_trans_str.c_str());
	//cout<<"the prob is "<<c_pTemp_searchspace->dPro<<endl;
	//cout<<"the size is "<<_pstSearchSpace[1][m_SenLen-1].size()<<endl;

	if( printNbest )
	{
		//��¼nbest �б��Լ�����ֵ
		for(int i=0; i<m_chart[1][m_SenLen-1].size(); i++)
		{
			MoveNullWord(m_chart[1][m_SenLen-1][i]->viEnPhrase);
			//c_best_trans_str = GetEngStr(m_chart[1][m_SenLen-1][i]->viEnPhrase);
			c_best_trans_str = m_chart[1][m_SenLen-1][i]->m_en_str;
			//TrueCasing( c_best_trans_str ); //��ʱ���ô�д����ĸ
			//ֱ������cube prunning �������ڵ�nbest�������жϷ�����ظ�
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
			c_feats.push_back(m_chart[1][m_SenLen-1][i]->m_context_based_trans_prob);  //���ӻ��������ĵķ�����ʣ���������
			//cout<<"whole sentence, m_context_based_trans_prob: "<<m_chart[1][m_SenLen-1][i]->m_context_based_trans_prob<<endl;  //4debug
			feats.push_back(c_feats);
		}
	}

	//������ݽṹ����
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
				//��������
				m_search_space_order_vector[i][j].clear();
				m_split_best_vector[i][j].clear();
				m_other_space_vector[i][j].clear();
				//ֻ���ô˷���������ȶ���
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
//���� NodeKBest
//���� cube prunning �ĺ��Ĳ���,��ÿ���ڵ����k����õķ����ѡ
//����ֵ���ɹ�����ʾ
/****************************************************************************/
bool Algorithm::NodeKBest(int start, int end, vector<s_SearchSpace*> &hypo_collection)
{
	//cerr<<"in NodeKBest ..."<<endl;
    int beam_hist = BEAM_HIST;

	//��ǰֱ����cube prunning�������ڵ��nbest����n��Сʱ���ʣ��д���չRootKBest����
	//Ȩ��֮��
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

    //������cube prunning�����ýڵ��k-best�����ѡ
	//priority_queue<s_SearchSpace,vector<s_SearchSpace>,SearchSpaceComp> cands_pri_queue; //������ѡ�����ȶ���
	HypoHeap candsQueue;
	candsQueue.Init(MAXHEAP, 2*beam_hist*beam_hist);

	//cerr<<"in NodeKBest 0 ..."<<endl;
	//���ȼ���ýڵ�ÿ�������õķ����ѡ
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

	//���ñ�־��ʾĳ�����Ƿ��Ѿ������ӽڵ��ith��ѡ���Һ��ӽڵ��jth��ѡ��չ��
	//������չ�漰˳����������Ի�����ظ���չ������

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
		if( end - start == m_SenLen-1 ) //������������ʱ�����<s>��</s>��������ģ�͸���
		{
			double added_lm_score = _pNgram->FinalTraversalFeatures(best_cand);
			best_cand->m_en_ngram_prob += added_lm_score;
			best_cand->dPro += all_Lambda.lm*added_lm_score; 
		}
		//cerr<<"cube pruning 1"<<endl;
		_pstSearchSpace[start][span].Update(best_cand);
		//cerr<<"cube pruning 2"<<endl;

		//�����ٽ��Ĵ��ź�ѡ����
		int c_key = best_cand->m_left_ith_best*100000 + best_cand->m_right_jth_best*1000 + best_cand->m_split_point;
		//�ж��Ƿ���չ��
		if( duplicate_set.end() == duplicate_set.find(c_key) )
		{
			PushSucc(best_cand, candsQueue, hypo_collection);
			duplicate_set.insert(c_key);
		}
		//cerr<<"cube pruning 3"<<endl;
	}
	//cerr<<"in NodeKBest 2 ..."<<endl;
	//���_pstSearchSpace[start][span]�о��ж�Ӧ�ķ��룬����뻺������
	s_SearchSpace* c_ptr_searchspace = NULL; 
	while( c_ptr_searchspace = _pstSearchSpace[start][span].Pop() )
	{
		buf.push_back(c_ptr_searchspace);
	}
	//cerr<<"in NodeKBest 3 ..."<<endl;

	//����_pstSearchSpace[start][span]����С�ѣ�������Ҫ��buf��������
	reverse(buf.begin(), buf.end());
	//������������chart����
	m_chart[start][end-start] = buf;

    return true;
}

/**********************************************************************************************/
//���� PushSucc
//���ܣ�������ǰ��ѡ���ٽ���ѡ����
//����ֵ���ɹ�����־
/**********************************************************************************************/
bool Algorithm::PushSucc(s_SearchSpace* cur_cand, HypoHeap &candsQueue, vector<s_SearchSpace*> &hypo_collection)
{
    int start = cur_cand->ulFirstWord;
    int end = cur_cand->ulLastWord;
    int split_point = cur_cand->m_split_point;

    //��չ���ڵ��������ź�ѡ����
    //������չ��ڵ�ĺ�ѡ
    int left_kth_best = cur_cand->m_left_ith_best+1; 
    int right_kth_best = cur_cand->m_right_jth_best;
	//cerr<<"push succ left"<<endl;
    if( m_chart[start][split_point-start].size() >= left_kth_best )
		MergeIntoEdge(m_chart[start][split_point-start][left_kth_best-1], m_chart[split_point+1][end-split_point-1][right_kth_best-1], left_kth_best, right_kth_best, candsQueue, hypo_collection);

    //�����չ�ҽڵ�ĺ�ѡ
    left_kth_best = cur_cand->m_left_ith_best;
    right_kth_best = cur_cand->m_right_jth_best+1;
	//cerr<<"push succ right"<<endl;
    if( m_chart[split_point+1][end-split_point-1].size() >= right_kth_best )
	MergeIntoEdge(m_chart[start][split_point-start][left_kth_best-1], m_chart[split_point+1][end-split_point-1][right_kth_best-1], left_kth_best, right_kth_best, candsQueue, hypo_collection);
	//cerr<<"push succ finish"<<endl;

    return true;
}

/*********************************************************************************************************************************/
//����MergeTwoVector
//���ұߵ�vector�ϲ�����ߵ�vector
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
//����ComputeMergedLM
//��������Ŀ�괮�ϲ������ӵ�����ģ�͸���
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
//����GetContextBasedTranslationProb
//������������ĵ����Ĵʵ�Ӣ�Ķ���ķ������
//��ڲ��������Ĵʵ�λ�ã������Ĵʶ�Ӧ��Ӣ�ķ���
//���ڲ���������
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
//���� MergeIntoEdge
//���ܣ����������ӽڵ�ĺ�ѡ�ϲ�Ϊ��ǰ�ڵ�ĺ�ѡ����
//����ֵ���ɹ�����־
/**********************************************************************************************/
bool Algorithm::MergeIntoEdge(s_SearchSpace* left_searchspace, s_SearchSpace* right_searchspace, int left_kth_best, int right_kth_best, HypoHeap &candsQueue, vector<s_SearchSpace*> &hypo_collection)
{
	//cerr<<"in merge edges 0"<<endl;
	//cerr<<"in merge edges 0.2"<<endl;
    s_SearchSpace* c_tmp_searchspace = new s_SearchSpace();
	//cerr<<"in merge edges 0.5"<<endl;
	hypo_collection.push_back(c_tmp_searchspace);
	//cerr<<"in merge edges 1"<<endl;
    //�ϲ�����
    //��¼span�Ŀ�ʼ��������ָ��
    int start = left_searchspace->ulFirstWord;
    int end = right_searchspace->ulLastWord;
    int split_point = left_searchspace->ulLastWord;

	int c_left_size = left_searchspace->viEnPhrase.size();
	int c_right_size = right_searchspace->viEnPhrase.size();
    //˳��ϲ�

    //�������ģ�͸���
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
    //���Ӹ÷��������Һ��ӽڵ�ĵڼ�����ѡ����ϲ�����,09.06.06���
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
    //��������ģ�͵�����
	double c_ngram_prob = _pNgram->TraversalFeaturesImpl(c_tmp_searchspace);
    c_tmp_searchspace->m_en_ngram_prob = left_searchspace->m_en_ngram_prob + right_searchspace->m_en_ngram_prob + c_ngram_prob;
    //������������ĵķ���ģ�͵Ĵ��
    c_tmp_searchspace->m_context_based_trans_prob = left_searchspace->m_context_based_trans_prob + right_searchspace->m_context_based_trans_prob;
    //cout<<"mono merge, m_context_based_trans_prob:"<<c_tmp_searchspace->m_context_based_trans_prob<<endl;  //4debug
	c_tmp_searchspace->dPro = left_searchspace->dPro + right_searchspace->dPro + all_Lambda.lm*c_ngram_prob + all_Lambda.reorderStraight*c_straight_prob;
	//cerr<<"in merge edges 2"<<endl;
    //���˱�ѹջ
    candsQueue.Push(c_tmp_searchspace);
	//cerr<<"in merge edges 3"<<endl;

    //����ϲ�
    //�趨�����򴰿�
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
    //�����±�
	c_tmp_searchspace->ulFirstWord = left_searchspace->ulFirstWord;
    c_tmp_searchspace->ulLastWord = right_searchspace->ulLastWord;
    c_tmp_searchspace->ulEnNum = left_searchspace->ulEnNum + right_searchspace->ulEnNum;
    c_tmp_searchspace->m_phrase_num = left_searchspace->m_phrase_num + right_searchspace->m_phrase_num;
    c_tmp_searchspace->m_split_point = split_point;
    //���Ӹ÷��������Һ��ӽڵ�ĵڼ�����ѡ����ϲ�����,09.06.06���
    c_tmp_searchspace->m_left_ith_best = left_kth_best;
    c_tmp_searchspace->m_right_jth_best = right_kth_best;

    //cout<<"before get trans prob..."<<(left_searchspace->eachTransPro).size()<<" "<<(right_searchspace->eachTransPro).size()<<endl;
    for(int k=0; k<(left_searchspace->eachTransPro).size(); k++)
		c_tmp_searchspace->eachTransPro.push_back((left_searchspace->eachTransPro)[k] + (right_searchspace->eachTransPro)[k]);
	c_tmp_searchspace->left_ant = right_searchspace;
	c_tmp_searchspace->right_ant = left_searchspace;
    //cout<<"after get trans prob..."<<endl;
    //��������ģ�͵�����

    c_tmp_searchspace->m_straight_reorder_prob = left_searchspace->m_straight_reorder_prob + right_searchspace->m_straight_reorder_prob;
    c_tmp_searchspace->m_swap_reorder_prob = left_searchspace->m_swap_reorder_prob + right_searchspace->m_swap_reorder_prob + c_invert_prob;
    c_tmp_searchspace->viEnPhrase = right_searchspace->viEnPhrase;
    MergeTwoVector(c_tmp_searchspace->viEnPhrase, left_searchspace->viEnPhrase);
	c_tmp_searchspace->m_en_str = right_searchspace->m_en_str + " " + left_searchspace->m_en_str;
    //��������ģ������
	c_ngram_prob = _pNgram->TraversalFeaturesImpl(c_tmp_searchspace);
    c_tmp_searchspace->m_en_ngram_prob = left_searchspace->m_en_ngram_prob + right_searchspace->m_en_ngram_prob + c_ngram_prob;
    //������������ĵķ���ģ�͵Ĵ��
    c_tmp_searchspace->m_context_based_trans_prob = left_searchspace->m_context_based_trans_prob + right_searchspace->m_context_based_trans_prob;
    //cout<<"swap merge, m_context_based_trans_prob:"<<c_tmp_searchspace->m_context_based_trans_prob<<endl;  //4debug
    c_tmp_searchspace->dPro = left_searchspace->dPro + right_searchspace->dPro + all_Lambda.lm*c_ngram_prob + all_Lambda.reorderSwap*c_invert_prob;
    //���˱�ѹջ
	//cerr<<"in merge edges 5"<<endl;
    candsQueue.Push(c_tmp_searchspace);
	//cerr<<"in merge edges 6"<<endl;

    return true;

}

/***********************cube prunning ����*****************************************************/

/**********************************************************************************************/
//����һ�У�һ�У�������ԭ�ģ��﷨�����ȼ�����
//���룺һ�о���
//�����Ӣ�ľ��ӣ�Nbest,�������
/**********************************************************************************************/
bool Algorithm::TranslateLine(char* pcSenChi,  char* pcSenEng, vector<string> &nBestList, vector< vector<double> > &feats, SegFormate segtype)
{
    //��������ԭ�����﷨�����ȼ����ʷֿ�
    string c_line = pcSenChi;
    string::size_type c_pos = c_line.find("#*#");
    if( c_pos == string::npos )
    {
	cerr<<"The input is Error!"<<endl;
	return false;
    }
    string c_chi_str = c_line.substr(0,c_pos);
    //��¼ԭ��
    strcpy(pcSenChi,c_chi_str.c_str());
    cout<<pcSenChi<<endl;
    
    //�洢�﷨��ȼ������
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
	//�����ǳ�����ĵط�
	m_span_vector.push_back(c_line);
    }
    //cout<<m_span_vector.size()<<endl;
    //����ԭ��
    TranslateSentence(pcSenChi,pcSenEng,nBestList,feats,segtype);
    
    return true;
}

/**********************************************************************************************/
//�������
//����: ���ľ���,�ִʸ�ʽ
//���: Ӣ�ľ��ӣ�Nbest, �������
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

	    //���·ִ�
// 			if( segtype == NOSEG )
// 			{
// 				reseg(ChiSen);
// 			}

	    //�����ӷָ�Ϊ��,����������һ��"0"
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


	    //�����ǰ���ӵĳ��ȴ���ǰ��һ�����ӵĳ���ʱ,���·����ڴ�
	    if( _ulSenLenMax < m_SenLen )  
	    {
		int m_i = 0;
		int m_j = 0;



		//�Ƚ����е��ڴ��ͷŵ�
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
			//��������Ϊ�żҿ�08��12��25�ո�д
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
		    delete []_pstSearchSpace;				 //������ѡ��
		    _pstSearchSpace = NULL;
			for( int k = 0; k < m_hypo_collection.size(); k++ )
				delete m_hypo_collection[k];
			m_hypo_collection.clear();
		    delete []m_chart;
		    m_chart = NULL;
		    //��������Ϊ�żҿ�08��12��25�ո�д
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

		    //delete []_pstHypoStack;                  //hypotheses ջ
		    //_pstHypoStack = NULL;
		}

		//ÿ��Phrase�ĺ�ѡ�����ŵ�һ��Search Space Stock �У�
		//��ͬһ���ʿ�ʼ��Search Space Stock ��ŵ�һ��������
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
		//�����ڴ�����Ϊ�żҿ�08��12��25�����
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
		//��һ��Stack���ڴ洢NULL->English
		//
		_pstSearchSpace[0] = new HypoHeap[1];             
		if( !_pstSearchSpace[0] )
		{
		    cerr<<"Allocate Memory for Search Space Error!\n";   
		}
		_pstSearchSpace[0][0].Init(MINHEAP, TRAN_TABLE_SIZE);
		//��ʼ����һ��Ԫ�أ�Ϊ�żҿ�08��12��25�����
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
		//ÿ��������Stack������������Phrase����󳤶ȣ����Ҳ��ᳬ����ǰλ�õ����ӽ�β�ĳ���
		//
		for(  m_i = 1; m_i<m_SenLen+1; m_i++ )
		{
		    //	int m_stacklen =  (m_SenLen-m_i+1) > PHRASE_LEN_MAX ? PHRASE_LEN_MAX :(m_SenLen-m_i+1);
		    //	CKY��ʼ��chart��ͼ�Ĵ�С,Ϊ�żҿ�08��12��25�����
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
		    //���¿ռ�����Ϊ�żҿ�08��12��25�����
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
	    //�����и�����NP������ȡ����
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
	    //�����ôʺű�ʾʱ���洢λ�ô�1��ʼ��0������ӵĿ�ʼ��<s>
	    //
	    //cerr<<"get word index"<<endl;
	    _ulSenLenChi =	_pVocabChi->GetAndInsertWordsIndics( ch_word_vec, &_pulSenChi[1],  SEN_LEN_MAX );   //�����ôʺű�ʾʱ
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


	    //����������ѡ��,Ҳ�����CKY��ʼ���Ĺ��ܣ��żҿ�08��12��25�����
	    vector< vector< pair<string,double> > > SenChiCN;
	    //cerr<<"before gensearchspace"<<endl;
	    m_bTemp = GenSearchSpace(SenChiCN);
	    //cerr<<"after gensearchspace"<<endl;
	    if( !m_bTemp)
		{
			cerr<<"Generate Search Space Error!\n";
			break;
		}

	    //������ӣ����ֽ��뷽����һ������cube prunning + beam������ֱ������beam
		//��һ���ٶȿ죬�ڶ��ֵ������������һ��
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
	    _ulSenLenMax = m_SenLen;                  //�ڷ����������Ȼ������_ulSenLenMax,����NULL->English���ɵ�������ѡ���
	}
	if( !m_bTemp )
	{
	    return false;
	}
	return true;

}


/**********************************************************************************************/
//�����ļ�
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
	//�ı���ʽ
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

			//���淭��һ�еĺ������żҿ�09��1��8�����
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
					c_prob += all_Lambda.sense*feats[m_i][c_size+5];	//���ӻ��������ĵķ������
					nbestfile<<"||| "<<c_prob;
					nbestfile<<endl;
				}
			}
		}
	}
	//863��ʽ
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
				//�����������żҿ�08��12��31�����
				//line = "<s> " + line;
				//line = line + " </s>";

				strcpy(ChiSen, line.c_str() );


				num++;
				cout<<"Translating The Sentence "<<num<<endl;
				cout<<ChiSen<<endl;
				
				//���淭��һ�еĺ������żҿ�09��1��8���޸�
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
						c_prob += all_Lambda.sense*feats[m_i][c_size+5];	//���ӻ��������ĵķ������
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
//��NP��ԭ
/**********************************************************************************************/
void
Algorithm::NPRevert(s_Hypotheses *m_pHypotheses, vector<int> &sentence)
{

	int m_wordNULLIndex = _pVocabEng->UnNull();			//"NULL"�Ĵʺ�
	int m_wordNumberIndex = _pVocabEng->NumberIndex();	//"<_NUMBER>"�Ĵʺ�
	int m_wordTimeIndex = _pVocabEng->TimeIndex();		//"<_TIME>"�Ĵʺ�
	int m_wordNEIndex = _pVocabEng->NEIndex();			//"<_NE>"�Ĵʺ�
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
