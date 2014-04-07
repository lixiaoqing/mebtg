/*
	NgramTrie.h
	存储语言模型参数的树结构，每一层存储由根节点到当前节点的词构成的语言模型的概率和Backoff 值
*/

#ifndef NGRAMTRIE_H
#define NGRAMTRIE_H

#include "StdAfx.h"
#include "Vocab.h"

//数据节点，存储概率和Backoff 值
typedef struct
{
	double bow;							 //backoff value;
	map<int, double> probs;	//按词号索引概率值
}NgramNode;	

class NgramTrie
{
public:
	//返回节点的值
	NgramNode &value()
	{
		return _node;
	}

	//插入一个新的子节点
	NgramTrie* InsertTrie(const int &wordIndex, NgramTrie &newTrie )                   
	{
		_subTrie.insert(map<int,NgramTrie>::value_type(wordIndex, newTrie));
		return &_subTrie[wordIndex];
	};

	//找到一个子节点
	NgramTrie *findTrie( const int &wordIndex )                   
	{
		map<int, NgramTrie>::iterator it= _subTrie.find(wordIndex);
		if( it != _subTrie.end() )
			return &( (*it).second );
		else
			return NULL;
	};

	//找到一个多层字节点,没有则插入；
	NgramTrie* findTrie(const int *words);                     

private:
	NgramNode _node;														//数据节点
	map<int, NgramTrie> _subTrie;                               //子节点集
};

#endif


