/*
	HypoStack.h
	hypothesesջ����
	���ܣ�����һ��hypothesesջ�����Խ���pop��push����
	*/

#ifndef HYPOSTACK_H
#define HYPOSTACK_H

#include "StdAfx.h"
#include "Feature.h"
#include "SearchSpaceStock.h"
#include "Vocab.h"
class s_Hypotheses
{
public:

	bitset<SEN_LEN_MAX> bitCovered;					//��¼�ѷ����λ��
	//int ulLastTwoEnWord[2];				//�������Ӣ�Ĵʣ���������ģ�͸���
	vector<int> ngramLastWord;
	s_SearchSpace* pstLastPhrase;					//���һ������Ķ���
	s_Hypotheses* pstFatherHypo;					//��¼�����;
	int prestack, prenum;							//��¼�����;
	double dPro;                                    //ȫ�����ۣ�����δ������
	double dNowPro;                                 //��ǰ����
	Feature feats;									//�������	
	vector<s_Hypotheses> addArc;					//�ϲ��Ľ��
	
	bool operator==(const s_Hypotheses &right)const;	
};

class HypoStack
{
public:
	HypoStack():_ulSize(0),_ulPopNum(0),STACK_SIZE(300),RECOMBINE_HYPOTHESES(1)
	{
	};
	void Initialize(int stack, bool recombine, Vocab* vocab)
	{
		_pVocab = vocab;
		STACK_SIZE = stack;
		RECOMBINE_HYPOTHESES = recombine;
		_pstHypothesesArray = new s_Hypotheses[STACK_SIZE];
		_pstHypotheseslist = new s_Hypotheses*[STACK_SIZE];
	};
	~HypoStack()
	{
		delete []_pstHypothesesArray;
		delete []_pstHypotheseslist;
	};

	//��ջ���ã�δ�����κ�Ԫ�أ�
	void Reset()
	{
		_ulPopNum = 0;
	};								

	//��ջ���
	void Clear()
	{
		_ulSize = 0;
		_ulPopNum = 0;
	};
	//����ջ�Ĵ�С
	int size()
	{
		return _ulSize;
	};
	
	//����ջ����õ� hypotheses
	s_Hypotheses* pop();											

	void ShowHypotheses();


	//////////////////////////////////////////////////////////////////////////
	//���µ�hypotheses ����ջ��
	int push(s_Hypotheses &Hypotheses);

private:
	Vocab* _pVocab;
	int STACK_SIZE;
	bool RECOMBINE_HYPOTHESES;
	int _ulSize;									//ջ��hypotheses ������
	int _ulPopNum;								//��¼popʱջ����õ�hypotheses��λ��
	s_Hypotheses *_pstHypothesesArray;			//ջ���飨ֻ������ŵ�Ԫ�أ������� ��
	s_Hypotheses** _pstHypotheseslist;			//��ջ�����е�Ԫ�ذ�costֵ��С���򣬰ѵ�ַ���ڴ�������           
};

#endif //!define HYPOSTACK_H

