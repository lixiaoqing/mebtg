/*
	Ngram.h
	�洢Ngram������Ϣ
	�ṩ��ȡNgram�ļ�����ȡNgram�Ĳ���
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

	//������ֵ�������
	bool ParseLogP(const char *str, double &result);

	//��ȡ����ģ���ļ�
	bool ReadFile(const char* NgramFileName, bool ReduceVoc=false); 

	//������ӵ�����ģ�͸���
	double ReadSentenceNgram(int *wids, int pos);
	double ReadSentenceNgram(vector<int> wids, int pos);
	
	//����Backoffֵ
	bool InsertBOW(int * words, double &bow)
	{
		_ngramcontext.findTrie(words)->value().bow = bow;
		return true;
	};          

	
private:
	Vocab *_vocab;					//���ʱ�ת��Ϊ�ʺ�
	NgramTrie _ngramcontext;		//�洢����ģ�Ͳ�������
	int _ulOrder;			//����ģ�͵Ĵ�����Ĭ����Ԫ����ģ�ͣ�
	
};

#endif

