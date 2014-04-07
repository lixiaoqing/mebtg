/*
	algorithm.h
	�����㷨
*/

#ifndef ALGORITHM_H
#define ALGORITHM_H

#include "StdAfx.h"
#include "PhraseProTrie.h"
#include "PhrasePro.h"
#include "SearchSpaceStock.h"
#include "HypoStack.h"
#include "Vocab.h"
#include "Ngram.h"
#include "Feature.h"
#include "Candidate.h"
#include "Function.h"
#include "time.h"

#include "MePredict.h"
#include "LanguageModel.h"
#include "ff_klm.h"

#include <maxent/maxentmodel.hpp>
using namespace maxent;

class DecodingSpanThread
{
	private:
		pthread_t _handle;
	public:
		int _threadId;
		int _threadNum;
	public:
		DecodingSpanThread() {};

		virtual ~DecodingSpanThread() {};
		
		static void* Wrapper(void* ptr)
		{
			DecodingSpanThread *p = (DecodingSpanThread*)ptr;
			p->DecodeSpan();
			return 0;
		}

		virtual void DecodeSpan() {}

		void Start()
		{
			pthread_create(&_handle, 0, &DecodingSpanThread::Wrapper, static_cast<void*> (this));
		}

		void Join()
		{
			pthread_join(_handle, 0);
		}
};

//Ϊ���ȶ�����Ƶ�s_SearchSpace�ıȽϺ������żҿ�08��12��25�����
//����ͨ����һ����"<" ����������ȶ���
struct SearchSpaceComp: public binary_function<s_SearchSpace, s_SearchSpace, bool>
{
    bool operator()(s_SearchSpace a, s_SearchSpace b)
    {
	return a.dPro < b.dPro;
    }
};

class CNPhraseNode
{
public:
	int position;
	PhraseProTrie* pPhrase;
	double CNprob;
	double LMprob;
	int LastWord;
};


class Algorithm 
{
public:
	Algorithm(){};
	~Algorithm();
	bool TranslateFile(const char* test_file, const char* result_file, FileFormate filetype=TXT, SegFormate segtype=SEG);
	//���淭��һ�еĺ������żҿ�09��1��7����ӣ�һ�У�������ԭ�ģ��﷨���������ȼ�����
	bool TranslateLine(char* pcSenChi, char* pcSenEng, vector<string> &nBestList,	vector< vector<double> > &feats, SegFormate segtype=SEG);
	bool TranslateSentence(char* pcSenChi, char* pcSenEng, vector<string> &nBestList,	vector< vector<double> > &feats, SegFormate segtype=SEG);
	bool Initialize(Config parameter);
	
public:
	vector< pair<int,int> > m_spans;
	bool NodeKBest(int start, int end, vector<s_SearchSpace*> &hypo_collection);    //����cube prunning ����һ���ڵ��kbest��ѡ����
private:
	

	//��������
	//Vocab* _pVocabChi;                  //���ĵ��ʱ�
	//Vocab* _pVocabEng;					//Ӣ�ĵ��ʱ�
	LanguageModel* _pNgram;						//Ӣ������ģ��
	int NgramOrder;
	PhrasePro* _pstPhrasePro;			//phrase�������
	Lambda all_Lambda;					//������Ȩ��

	int BEAM_HIST;					//beam������֦
	double BEAM_PROB;                              //��ֵ��֦����

	int TRAN_TABLE_SIZE;				//ÿ��Phrase���ĺ�ѡ��
	bool RECOMBINE_HYPOTHESES;          //�Ƿ���м�������

	int NbestNumber;					//����Nbest�ĸ���
	bool train;                         //�Ƿ��ǽ���ѵ��
	int feats_num;

	int threadNum;

	bool printInfo;						//��ӡ������Ϣ
	bool printNbest;					//��ӡN��Best

	string nBestFileName;               //N-Best �ļ���
	string featsfilename;				//�����ļ���
	string candsfilename;				//N-best�����ļ���

	FileFormate _filetype;              //�ļ��ĸ�ʽ
	SegFormate _segmenttype;            //�ִʸ�ʽ
	ParseWordType _parsetype;           //�дʵ�����
	UnknownWord _transUnkownWord;       //��δ��¼�ʵĴ���
	//�������ģ�����żҿ�09��1��5�����
	ReorderModel* m_me_reorder_model;
	//������������ĵķ���ģ������С��14��3��29�����
	vector <MaxentModel*> m_context_based_translation_models;
	int m_reorder_window;                 //���򴰿ڵĴ�С
	int m_cube_flag;                    //�Ƿ�����cube prunning�㷨����

	//���뷽�򼰵������
	int m_chn2eng_flag;
	string g_Chinese_Punc_Set;
	string g_Chinese_Split_Puncs;


	//�йؾ��ӵ�����
	int* _pulSenChi;                            //���ľ��Ӵʺ�����
	int* _pulSenEng;							//Ӣ�ľ��Ӵʺ�����
	int _ulSenLenChi;							//���ľ��ӳ���
	vector <string> ch_word_vec;                                            //���Ĵ�����
	int m_SenLen;								//��NP ������ȡ����֮ǰ�ľ��ӳ���
	int _ulSenLenMax;							//��ĿǰΪֹ��ľ���;
	int _ulSenLenEng;							//Ӣ�ľ��ӳ���
	HypoStack*  _pstHypoStack;                  //Hypothesesջ
	//SearchSpaceStock** _pstSearchSpace;			//������ѡ��
	HypoHeap** _pstSearchSpace;			//������ѡ��
	vector<s_SearchSpace*> m_hypo_collection;   //�洢����һ�仰ʱ��������пռ�
	bitset<SEN_LEN_MAX> _bitClosed;             //��λ�Ƿ���ȷ���ķ����ѡ��
	map<int, int> _mapNPTransTalbe;				//�������﷭���ѡ��ı�,ǰ��Ϊ�����ľ����е�λ�ã�����ΪӢ�ķ���Ĵʺ�(����Ƕ���ʣ�Ҳ������һ����)
	//�����йؾ��ӵ����ݱ���Ϊ�żҿ������08��12��24��
	int** m_syn_flag;                    //Ŀǰ�Ķ����Ƿ����﷨����	
	int** m_order_flag;                  //������ȵļ����Ƿ�����
	int** m_nbest_max;                   //��¼ĳ���������ܲ�����nbest����
	vector<string> m_span_vector;        //�洢�﷨�������Ŀ��
	map<string,double> m_syn_prob_map;    //�洢�﷨�����ĵ������
	vector<s_SearchSpace>** m_nbest_cands;     //ÿ���Ƶ���nbest��ѡ
	vector<s_SearchSpace*>** m_search_space_order_vector;  //ÿ���Ƶ��������зָ�㴦�ļ���˳��
	vector<s_SearchSpace*>** m_split_best_vector;
	vector<s_SearchSpace*>** m_other_space_vector;
	priority_queue<s_SearchSpace,vector<s_SearchSpace>,SearchSpaceComp>** m_search_space_queue; //ÿ���Ƶ����������ȶ���

	bool MoveNullWord(vector<int> &en_phrase);          //ȥ��NULL

	//cube prunning �Ľ���汾,09��6��5�����
	//���ݽṹ
	vector<s_SearchSpace*>** m_chart;
	//����
	bool CubeParse(int m_SenLen, char* pcSenEng, vector<string> &nbestList, vector< vector<double> > &feats);  //�Ƶ���������
	bool PushSucc(s_SearchSpace* cur_cand, HypoHeap &candsQueue, vector<s_SearchSpace*> &hypo_collection);       //����ǰ��ѡ������ٽ���ѡѹ�����ȶ�����
	bool MergeIntoEdge(s_SearchSpace* left_searchspace, s_SearchSpace* right_searchspace, int left_kth_best, int right_kth_best, HypoHeap &candsQueue, vector<s_SearchSpace*> &hypo_collection);  //��tail nodes�γɺ�ѡ����ѹ�����ȶ���
	//����չ��kbest parsing�������ڵ��nbest �б�
	bool RootKNBest(){};

	//����	

	int NPParsing(char** words, int &SenLen);							//����֪�ĺ�ѡ����������
	void NPRevert(s_Hypotheses *pstHypo, vector<int> &sentence);		//������е�NP��ԭ
	bool GenSearchSpace(vector< vector< pair<string,double> > > SenChiCN);       //����������ѡ��
	string GetEngStr( const vector<int> &sentenceID );					//���ʺŻ�ԭΪ��

	void TrueCasing(char* sentence);
	void TrueCasing(string &sentence);

	void MergeTwoVector(vector<int> &left_vector, vector<int> &right_vector);
	double ComputeMergedLM(Ngram* c_ngram_trie, int c_ngram_order, vector<int> &left_en_wids, vector<int> &right_en_wids);

	double GetContextBasedTranslationProb(int pos, string &tgt_translation);

};

class DecodingSpan : public DecodingSpanThread
{
	public:
		Algorithm* _decoderPtr;
		vector<s_SearchSpace*> _hypoCollection;

		DecodingSpan() {};
		~DecodingSpan() {};

		//��дDecodingSpanThread�е�DecodeSpan����
		void DecodeSpan();
		void DecodeSpan(int threadId, int threadNum);
};

#endif // !define ALGORITHM_H

