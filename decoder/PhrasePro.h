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
	bool ReadFile(const char* PhraseProFileName, bool ReduceVoc=false, SegFormate segmenttype=NOSEG);												//读取参数文件
	bool ReadAlignment(const char* AlignmentFileName);

	int ParsePhrase(char** words, char ** phrase, int max);     //将短语切分出来
    
	//查找中文短语第一个词对应的翻译节点
	PhraseProTrie* findword(int &wordindex)
	{
		return contexts.findTrie(wordindex);
	};

protected:
	int trans_table_size;               //
	Vocab* _ChiVocab;					//中文词典
	Vocab* _EngVocab;					//英文词典
	PhraseProTrie contexts;				//翻译节点
	Lambda _lambda;
};

#endif

