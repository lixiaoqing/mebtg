/*
	PhraseProTrie.h
	phrase翻译概率的存储结构
*/
#ifndef PHRASEPROTRIE_H
#define PHRASEPROTRIE_H
#include "StdAfx.h"
#include "Vocab.h"

//存储英文短语
class s_PhrasePro		         	        
{	
public:
	bool operator<(const s_PhrasePro &right) const;
	s_PhrasePro& operator=(const s_PhrasePro& right);
	int ulEnNum;						//英文词的个数
	vector<int> viEnPhrase;
	double dPro;								//概率
	vector<double> eachTransPro;
	vector<vector<int> > ch_pos_to_en_pos_list;                //每个中文词对应的英文词位置
// 	double dpce;
// 	double dlce;
// 	double dpec;
// 	double dlec;
};


class PhraseProTrie
{
public:
	//返回节点的值
	vector<s_PhrasePro> &value()
	{
		return _phrasePro;
	};

	//插入一个节点
	PhraseProTrie* InsertTrie(const int &Index, PhraseProTrie &newTrie)
	{
		_subPhraseTrie.insert(map<int, PhraseProTrie>::value_type(Index,newTrie) );
		return &_subPhraseTrie[Index];
	};
	
	//查找一个节点;
	PhraseProTrie* findTrie(const int &Index)         
	{
		map<int ,PhraseProTrie>::iterator it= _subPhraseTrie.find(Index);
		if( _subPhraseTrie.end() != it )
			return &( (*it).second );
		else
			return NULL;
	};


	PhraseProTrie* findTrie(const int *Indices);      //查找一个多层节点,没有则插入
private:
	vector<s_PhrasePro> _phrasePro;						//存储一个中文短语对应的所有英文短语
	map<int, PhraseProTrie> _subPhraseTrie;	//存储节点
};

#endif //!define PHRASEPRO_H

