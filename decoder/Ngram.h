/*
	Ngram.h
	存储Ngram概率信息
	提供读取Ngram文件，读取Ngram的操作
	*/

#ifndef NGRAM_H
#define NGRAM_H

#include "StdAfx.h"
#include "NgramTrie.h"
#include "Vocab.h"
#include "LanguageModel.h"


class Ngram : public LanguageModel
{
public:

	Ngram( Vocab *vocab, int Order, string filename, bool reduceVoc)
	{
		_ulOrder = Order;
		_vocab = vocab;
		if(_ulOrder<1)
			_ulOrder=1;

		if( !ReadFile( filename.c_str(), reduceVoc) )
		{
			cerr<<"reading Language file error!\n";
			exit(-1);
		}
	}; 

	virtual double FinalTraversalFeatures(s_SearchSpace* edge);
	virtual double TraversalFeaturesImpl(s_SearchSpace* edge);
	double ComputeMergedLM(vector<int> &left_wids, vector<int> &right_wids);
	virtual void Init() const;

	//将概率值分离出来
	bool ParseLogP(const char *str, double &result);

	//读取语言模型文件
	bool ReadFile(const char* NgramFileName, bool ReduceVoc=false); 

	//计算句子的语言模型概率
	double ReadSentenceNgram(int *wids, int pos);
	double ReadSentenceNgram(vector<int> wids, int pos);
	
	//插入Backoff值
	bool InsertBOW(int * words, double &bow)
	{
		_ngramcontext.findTrie(words)->value().bow = bow;
		return true;
	};          

	
private:
	Vocab *_vocab;					//单词表，转化为词号
	NgramTrie _ngramcontext;		//存储语言模型参数的树
	int _ulOrder;			//语言模型的次数（默认三元语言模型）
	
};

#endif

