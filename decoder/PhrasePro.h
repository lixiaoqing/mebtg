/*
PhrasePro.h

*/

#ifndef PHRASEPRO_H
#define PHRASEPRO_H
#include "StdAfx.h"
#include "PhraseProTrie.h"



class PhrasePro
{
public:
	PhrasePro(Vocab *chivocab, Vocab *engvocab, int ttable):_ChiVocab(chivocab),_EngVocab(engvocab),trans_table_size(ttable)
	{
	};
	bool SetArgs(Lambda all_Lambda)
	{
		_lambda = all_Lambda;
		return 1;
	};
	bool ReadFile(const char* PhraseProFileName, bool ReduceVoc=false, SegFormate segmenttype=NOSEG);												//��ȡ�����ļ�
	bool ReadAlignment(const char* AlignmentFileName);

	int ParsePhrase(char** words, char ** phrase, int max);     //�������зֳ���
    
	//�������Ķ����һ���ʶ�Ӧ�ķ���ڵ�
	PhraseProTrie* findword(int &wordindex)
	{
		return contexts.findTrie(wordindex);
	};

protected:
	int trans_table_size;               //
	Vocab* _ChiVocab;					//���Ĵʵ�
	Vocab* _EngVocab;					//Ӣ�Ĵʵ�
	PhraseProTrie contexts;				//����ڵ�
	Lambda _lambda;
};

#endif

