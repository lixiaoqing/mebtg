/*
	algorithm.h
	搜索算法
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

//为优先队列设计的s_SearchSpace的比较函数是张家俊08年12月25日添加
//跟普通排序不一样，"<" 产生大根优先队列
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
	//下面翻译一行的函数由张家俊09年1月7日添加，一行＝待翻译原文＋语法短语跳序跨度及概率
	bool TranslateLine(char* pcSenChi, char* pcSenEng, vector<string> &nBestList,	vector< vector<double> > &feats, SegFormate segtype=SEG);
	bool TranslateSentence(char* pcSenChi, char* pcSenEng, vector<string> &nBestList,	vector< vector<double> > &feats, SegFormate segtype=SEG);
	bool Initialize(Config parameter);
	
public:
	vector< pair<int,int> > m_spans;
	bool NodeKBest(int start, int end, vector<s_SearchSpace*> &hypo_collection);    //利用cube prunning 产生一个节点的kbest候选翻译
private:
	

	//参数数据
	//Vocab* _pVocabChi;                  //中文单词表
	//Vocab* _pVocabEng;					//英文单词表
	LanguageModel* _pNgram;						//英文语言模型
	int NgramOrder;
	PhrasePro* _pstPhrasePro;			//phrase翻译概率
	Lambda all_Lambda;					//特征的权重

	int BEAM_HIST;					//beam容量剪枝
	double BEAM_PROB;                              //阈值剪枝参数

	int TRAN_TABLE_SIZE;				//每个Phrase最大的候选项
	bool RECOMBINE_HYPOTHESES;          //是否进行假设重组

	int NbestNumber;					//生成Nbest的个数
	bool train;                         //是否是进行训练
	int feats_num;

	int threadNum;

	bool printInfo;						//打印搜索信息
	bool printNbest;					//打印N－Best

	string nBestFileName;               //N-Best 文件名
	string featsfilename;				//特征文件名
	string candsfilename;				//N-best个数文件名

	FileFormate _filetype;              //文件的格式
	SegFormate _segmenttype;            //分词格式
	ParseWordType _parsetype;           //切词的种类
	UnknownWord _transUnkownWord;       //对未登录词的处理
	//下面调序模型由张家俊09年1月5日添加
	ReorderModel* m_me_reorder_model;
	//下面基于上下文的翻译模型由李小青14年3月29日添加
	vector <MaxentModel*> m_context_based_translation_models;
	int m_reorder_window;                 //调序窗口的大小
	int m_cube_flag;                    //是否利用cube prunning算法解码

	//翻译方向及调序控制
	int m_chn2eng_flag;
	string g_Chinese_Punc_Set;
	string g_Chinese_Split_Puncs;


	//有关句子的数据
	int* _pulSenChi;                            //中文句子词号序列
	int* _pulSenEng;							//英文句子词号序列
	int _ulSenLenChi;							//中文句子长度
	vector <string> ch_word_vec;                                            //中文词序列
	int m_SenLen;								//将NP 翻译提取出来之前的句子长度
	int _ulSenLenMax;							//到目前为止最长的句子;
	int _ulSenLenEng;							//英文句子长度
	HypoStack*  _pstHypoStack;                  //Hypotheses栈
	//SearchSpaceStock** _pstSearchSpace;			//搜索候选项
	HypoHeap** _pstSearchSpace;			//搜索候选项
	vector<s_SearchSpace*> m_hypo_collection;   //存储解码一句话时申请的所有空间
	bitset<SEN_LEN_MAX> _bitClosed;             //该位是否有确定的翻译候选项
	map<int, int> _mapNPTransTalbe;				//给定短语翻译候选项的表,前面为在中文句子中的位置，后面为英文翻译的词号(如果是多个词，也看成是一个词)
	//以下有关句子的数据变量为张家俊添加于08年12月24日
	int** m_syn_flag;                    //目前的短语是否是语法短语	
	int** m_order_flag;                  //各个跨度的假设是否排序
	int** m_nbest_max;                   //记录某个跨度最多能产生的nbest个数
	vector<string> m_span_vector;        //存储语法短语调序的跨度
	map<string,double> m_syn_prob_map;    //存储语法短语间的调序概率
	vector<s_SearchSpace>** m_nbest_cands;     //每个推导的nbest候选
	vector<s_SearchSpace*>** m_search_space_order_vector;  //每个推导保存所有分割点处的假设顺序
	vector<s_SearchSpace*>** m_split_best_vector;
	vector<s_SearchSpace*>** m_other_space_vector;
	priority_queue<s_SearchSpace,vector<s_SearchSpace>,SearchSpaceComp>** m_search_space_queue; //每个推导保留的优先队列

	bool MoveNullWord(vector<int> &en_phrase);          //去除NULL

	//cube prunning 的解码版本,09年6月5日添加
	//数据结构
	vector<s_SearchSpace*>** m_chart;
	//函数
	bool CubeParse(int m_SenLen, char* pcSenEng, vector<string> &nbestList, vector< vector<double> > &feats);  //推导整个句子
	bool PushSucc(s_SearchSpace* cur_cand, HypoHeap &candsQueue, vector<s_SearchSpace*> &hypo_collection);       //将当前候选翻译的临近候选压入优先队列中
	bool MergeIntoEdge(s_SearchSpace* left_searchspace, s_SearchSpace* right_searchspace, int left_kth_best, int right_kth_best, HypoHeap &candsQueue, vector<s_SearchSpace*> &hypo_collection);  //由tail nodes形成候选翻译压入优先队列
	//待扩展的kbest parsing产生根节点的nbest 列表
	bool RootKNBest(){};

	//函数	

	int NPParsing(char** words, int &SenLen);							//将已知的候选翻译分离出来
	void NPRevert(s_Hypotheses *pstHypo, vector<int> &sentence);		//将结果中的NP还原
	bool GenSearchSpace(vector< vector< pair<string,double> > > SenChiCN);       //生成搜索候选项
	string GetEngStr( const vector<int> &sentenceID );					//将词号还原为词

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

		//重写DecodingSpanThread中的DecodeSpan函数
		void DecodeSpan();
		void DecodeSpan(int threadId, int threadNum);
};

#endif // !define ALGORITHM_H

