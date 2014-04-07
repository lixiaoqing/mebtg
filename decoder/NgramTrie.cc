/*
NgramTrie.cpp
*/

#include "NgramTrie.h"



////////////////////////////////////////////////////////////////////////
// 1.主要功能：找到一个多层节点，如果没有，则插入
// 2.入口参数：一个词串
// 3.出口参数：节点的指针
// 4.生成日期：2006.04.10
// 5.算法简介：
//
//
NgramTrie* 
NgramTrie::findTrie(const int *words)
{
	int m_len= Vocab::length(words);					//词串长度
	NgramTrie* subTrie=this;
	//逐层查找
	for(int i=0; i<m_len; i++)
	{
		NgramTrie* m_subTemp=subTrie->findTrie(words[i]);
		if( m_subTemp )
			subTrie = m_subTemp;
		else
		{
			NgramTrie m_triTemp;
			subTrie=subTrie->InsertTrie(words[i],m_triTemp);
		}
	}
	return subTrie;
}

