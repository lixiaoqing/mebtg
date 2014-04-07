/*
	NgramTrie.h
	�洢����ģ�Ͳ��������ṹ��ÿһ��洢�ɸ��ڵ㵽��ǰ�ڵ�Ĵʹ��ɵ�����ģ�͵ĸ��ʺ�Backoff ֵ
*/

#ifndef NGRAMTRIE_H
#define NGRAMTRIE_H

#include "StdAfx.h"
#include "Vocab.h"

//���ݽڵ㣬�洢���ʺ�Backoff ֵ
typedef struct
{
	double bow;							 //backoff value;
	map<int, double> probs;	//���ʺ���������ֵ
}NgramNode;	

class NgramTrie
{
public:
	//���ؽڵ��ֵ
	NgramNode &value()
	{
		return _node;
	}

	//����һ���µ��ӽڵ�
	NgramTrie* InsertTrie(const int &wordIndex, NgramTrie &newTrie )                   
	{
		_subTrie.insert(map<int,NgramTrie>::value_type(wordIndex, newTrie));
		return &_subTrie[wordIndex];
	};

	//�ҵ�һ���ӽڵ�
	NgramTrie *findTrie( const int &wordIndex )                   
	{
		map<int, NgramTrie>::iterator it= _subTrie.find(wordIndex);
		if( it != _subTrie.end() )
			return &( (*it).second );
		else
			return NULL;
	};

	//�ҵ�һ������ֽڵ�,û������룻
	NgramTrie* findTrie(const int *words);                     

private:
	NgramNode _node;														//���ݽڵ�
	map<int, NgramTrie> _subTrie;                               //�ӽڵ㼯
};

#endif


