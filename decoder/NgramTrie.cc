/*
NgramTrie.cpp
*/

#include "NgramTrie.h"



////////////////////////////////////////////////////////////////////////
// 1.��Ҫ���ܣ��ҵ�һ�����ڵ㣬���û�У������
// 2.��ڲ�����һ���ʴ�
// 3.���ڲ������ڵ��ָ��
// 4.�������ڣ�2006.04.10
// 5.�㷨��飺
//
//
NgramTrie* 
NgramTrie::findTrie(const int *words)
{
	int m_len= Vocab::length(words);					//�ʴ�����
	NgramTrie* subTrie=this;
	//������
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

