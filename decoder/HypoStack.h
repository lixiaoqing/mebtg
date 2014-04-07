/*
	HypoStack.h
	hypotheses栈的类
	功能，生成一个hypotheses栈，可以进行pop和push操作
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

	bitset<SEN_LEN_MAX> bitCovered;					//记录已翻译的位置
	//int ulLastTwoEnWord[2];				//最后两个英文词，计算语言模型概率
	vector<int> ngramLastWord;
	s_SearchSpace* pstLastPhrase;					//最后一个翻译的短语
	s_Hypotheses* pstFatherHypo;					//记录父结点;
	int prestack, prenum;							//记录父结点;
	double dPro;                                    //全部代价，包括未来代价
	double dNowPro;                                 //当前代价
	Feature feats;									//特征打分	
	vector<s_Hypotheses> addArc;					//合并的结点
	
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

	//将栈重置，未弹出任何元素；
	void Reset()
	{
		_ulPopNum = 0;
	};								

	//将栈清空
	void Clear()
	{
		_ulSize = 0;
		_ulPopNum = 0;
	};
	//返回栈的大小
	int size()
	{
		return _ulSize;
	};
	
	//弹出栈内最好的 hypotheses
	s_Hypotheses* pop();											

	void ShowHypotheses();


	//////////////////////////////////////////////////////////////////////////
	//将新的hypotheses 放入栈内
	int push(s_Hypotheses &Hypotheses);

private:
	Vocab* _pVocab;
	int STACK_SIZE;
	bool RECOMBINE_HYPOTHESES;
	int _ulSize;									//栈内hypotheses 的数量
	int _ulPopNum;								//记录pop时栈内最好的hypotheses的位置
	s_Hypotheses *_pstHypothesesArray;			//栈数组（只存放最优的元素，不排序 ）
	s_Hypotheses** _pstHypotheseslist;			//将栈数组中的元素按cost值大小排序，把地址存在此数组中           
};

#endif //!define HYPOSTACK_H

