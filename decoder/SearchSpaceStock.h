/*
	SearchSpace.h
	������ѡ��
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

//�洢������ѡ��Ľṹ
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

			//���ڼ�¼�ü����ɺ��ӽڵ�ĵڼ�����ѡ����ϲ�����,09.06.06���
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

			//���ڼ�¼�ü����ɺ��ӽڵ�ĵڼ�����ѡ����ϲ�����,09.06.06���
			m_left_ith_best = 0;
			m_right_jth_best = 0;

			m_recombine_array.clear();

			left_ant = 0;
			right_ant = 0;
		}

		int ulFirstWord;					//��������ĵ�һ���ʵ�λ��
		int ulLastWord;					//������������һ���ʵ�λ��
		int ulEnNum;						//Ӣ�Ĵʵĸ���
		vector<int> viEnPhrase;             //Ӣ�����
		string m_en_str;  //Ӣ����ﴮ
		double dPro;								//����
		vector<double> eachTransPro;
		double dCN;
		double dCLM;
		//���±���Ϊ�żҿ�08��12��25�����
		int m_phrase_num;             //�÷����Ӧ�Ķ������
		double m_en_ngram_prob;       //�÷����Ӧ��Ŀ������ģ�͸���
		double m_straight_reorder_prob;         //�÷����Ӧ�ĵ������
		double m_swap_reorder_prob;         //�÷����Ӧ�ĵ������
		double m_syn_reorder_prob;     //�÷����Ӧ���﷨����������
		int m_split_point;             //��¼��ǰ�Ƶ����������Ƶ��ϲ�����
		vector<s_SearchSpace>  m_recombine_array;  //��¼���ºϲ�ʱ���ü���ϲ�����������

		//���±���Ϊ��С��14��3��31�����
		double m_context_based_trans_prob;        //��¼���������ĵķ������

		//���ڼ�¼�ü����ɺ��ӽڵ�ĵڼ�����ѡ����ϲ�����,09.06.06���
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
	
	//��ջ���ã�Ĭ�ϴ�0��ʼ��ջ
	void Reset()
	{
		_ulPopNum = 0;
	};

	//��ջ��գ�
	void Clear()
	{
		_ulPopNum = 0;
		_ulSize = 0;
		_bClose = false;
	};

	//����ջ��Ԫ�ص�����
	int Size()
	{
		return _ulSize;
	};
	
	//��ջ�ڵ�Ԫ�ص���
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

	//����ĺ������żҿ�08��12��28�����
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

	//����ջ��������������ӷ����ѡ��
	int CloseStack()
	{
		_bClose = true;
		return _ulSize;
	}

	//���Ը�ջ�Ƿ�����
	bool IsClose()
	{
		return _bClose;
	}

	//���µ�Ԫ����ջ
	int push(s_SearchSpace &searchspace);
	//�������ӱߵĺ������żҿ�08��12��25�����
	int AddEdge(s_SearchSpace &searchspace);

private:
	int TRAN_TABLE_SIZE;
	bool _bClose;											//�Ƿ񽫸�ջ��������������ӷ����ѡ��
	int _ulSize;									//ջ��Ԫ�ص�����
	int _ulPopNum;								//��ջʱԪ�ص�λ��
	s_SearchSpace** _stSearchSpaceList;		//�����Ԫ�����飬�洢����Ԫ�ص�ָ��
	s_SearchSpace* _stSearchSpaceArray;		//�洢Ԫ�����ݵ�����
	//���������ģ�ͽ����������żҿ�08��12��25�����
	int m_lm_order;
};
*/

#endif   //!define SEARCHSPACE_H

