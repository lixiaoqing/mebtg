/*
	SearchSpace.h
	搜索候选项
*/

#ifndef SEARCHSPACESTOCK_H
#define SEARCHSPACESTOCK_H

#include "StdAfx.h"
#include "lm/left.hh"

#pragma pack(push)
#pragma pack(1)

struct BoundaryAnnotatedState {
  lm::ngram::ChartState state;
  bool seen_bos, seen_eos;
};

#pragma pack(pop)

//存储搜索候选项的结构
class s_SearchSpace	                
{
	public:

		s_SearchSpace()
		{
			ulFirstWord = -1;					
			ulLastWord = -1;					
			ulEnNum = 0;						
			dPro = 0;							
			dCN = 0;
			viEnPhrase.clear();
			m_en_str = "";
			m_phrase_num = 0;
			m_en_ngram_prob = 0;
			m_context_based_trans_prob = 0;
			m_straight_reorder_prob = 0;
			m_swap_reorder_prob = 0;
			m_syn_reorder_prob = 0;
			m_split_point=-1;

			//用于记录该假设由孩子节点的第几个候选翻译合并而来,09.06.06添加
			m_left_ith_best = 0;
			m_right_jth_best = 0;

			m_recombine_array.clear();

			left_ant = 0;
			right_ant = 0;

		}
		~s_SearchSpace()
		{
			ulFirstWord = -1;					
			ulLastWord = -1;					
			ulEnNum = 0;						
			dPro = 0;							
			dCN = 0;
			viEnPhrase.clear();
			m_en_str = "";
			m_phrase_num = 0;
			m_en_ngram_prob = 0;
			m_context_based_trans_prob = 0;
			m_straight_reorder_prob = 0;
			m_swap_reorder_prob = 0;
			m_syn_reorder_prob = 0;
			m_split_point=-1;

			//用于记录该假设由孩子节点的第几个候选翻译合并而来,09.06.06添加
			m_left_ith_best = 0;
			m_right_jth_best = 0;

			m_recombine_array.clear();

			left_ant = 0;
			right_ant = 0;
		}

		int ulFirstWord;					//翻译的中文第一个词的位置
		int ulLastWord;					//翻译的中文最后一个词的位置
		int ulEnNum;						//英文词的个数
		vector<int> viEnPhrase;             //英语短语
		string m_en_str;  //英语短语串
		double dPro;								//概率
		vector<double> eachTransPro;
		double dCN;
		double dCLM;
		//以下变量为张家俊08年12月25日添加
		int m_phrase_num;             //该翻译对应的短语个数
		double m_en_ngram_prob;       //该翻译对应的目标语言模型概率
		double m_straight_reorder_prob;         //该翻译对应的调序概率
		double m_swap_reorder_prob;         //该翻译对应的调序概率
		double m_syn_reorder_prob;     //该翻译对应的语法短语调序概率
		int m_split_point;             //记录当前推导由哪两个推导合并而来
		vector<s_SearchSpace>  m_recombine_array;  //记录重新合并时被该假设合并的其他假设

		//以下变量为李小青14年3月31日添加
		double m_context_based_trans_prob;        //记录基于上下文的翻译概率

		//用于记录该假设由孩子节点的第几个候选翻译合并而来,09.06.06添加
		int m_left_ith_best;
		int m_right_jth_best;

		//state
		BoundaryAnnotatedState cur_state;
		s_SearchSpace* left_ant;
		s_SearchSpace* right_ant;

};

class HypoHeap
{
	private:
		HEAPTYPE _heapType;
		int _heapSize;
		int _count;
		vector<s_SearchSpace*> _pHypoList;
	public:
		HypoHeap():_heapType(MINHEAP), _heapSize(0), _count(0)
		{}
		void Init(HEAPTYPE heapType, int heapSize);
		~HypoHeap();
		void Clear();
		bool Empty();
		bool Full();
		int Count();
		void Resize(int newHeapSize);
		void Update(s_SearchSpace* hypo);
		bool Compare(s_SearchSpace* oneHypo, s_SearchSpace* anotherHypo);
		void Push(s_SearchSpace* hypo); 
		s_SearchSpace* Pop();
		void Up(int hypoPos);
		void Down(int hypoPos);
};

/*
class SearchSpaceStock 
{
public:
	SearchSpaceStock():_ulSize(0),_ulPopNum(0),_bClose(false),TRAN_TABLE_SIZE(10)
	{
	};
	void Initialize(int ttable, int lmorder)
	{
		TRAN_TABLE_SIZE = ttable;
		m_lm_order=lmorder;

		_stSearchSpaceArray = new s_SearchSpace[TRAN_TABLE_SIZE];
		_stSearchSpaceList = new s_SearchSpace*[TRAN_TABLE_SIZE];
	};
	~SearchSpaceStock()
	{
		delete []_stSearchSpaceArray;
		delete []_stSearchSpaceList;
	};
	
	//将栈重置；默认从0开始出栈
	void Reset()
	{
		_ulPopNum = 0;
	};

	//将栈清空；
	void Clear()
	{
		_ulPopNum = 0;
		_ulSize = 0;
		_bClose = false;
	};

	//返回栈内元素的数量
	int Size()
	{
		return _ulSize;
	};
	
	//将栈内的元素弹出
	s_SearchSpace* pop()
	{
		if( _ulSize == 0 )
		{
			return NULL;
		}
		else if( _ulPopNum >= _ulSize )
		{
			_ulPopNum = 0;
			return NULL;
		}
		else
		{
			return _stSearchSpaceList[_ulPopNum++];
		}

	};

	//下面的函数由张家俊08年12月28日添加
	s_SearchSpace GetAt(int pos)
	{
	    if(_ulSize == 0)
	    {
		cerr<<"The pos to get member of searchspace Error!"<<endl;
		exit(-1);
	    }
	    else if( pos > _ulSize )
	    {
		cerr<<"The pos to get member of searchspace is beyond the size!"<<endl;
		exit(-1);
	    }
	    else
		return *_stSearchSpaceList[pos-1];
	}

	//将该栈锁定，不允许添加翻译候选项
	int CloseStack()
	{
		_bClose = true;
		return _ulSize;
	}

	//测试该栈是否锁定
	bool IsClose()
	{
		return _bClose;
	}

	//将新的元素入栈
	int push(s_SearchSpace &searchspace);
	//下面增加边的函数由张家俊08年12月25日添加
	int AddEdge(s_SearchSpace &searchspace);

private:
	int TRAN_TABLE_SIZE;
	bool _bClose;											//是否将该栈锁定，不允许添加翻译候选项
	int _ulSize;									//栈内元素的数量
	int _ulPopNum;								//出栈时元素的位置
	s_SearchSpace** _stSearchSpaceList;		//排序的元素数组，存储的是元素的指针
	s_SearchSpace* _stSearchSpaceArray;		//存储元素数据的数组
	//下面的语言模型阶数变量由张家俊08年12月25日添加
	int m_lm_order;
};
*/

#endif   //!define SEARCHSPACE_H

