/*
PhraseProTrie.cpp
*/


#include "PhraseProTrie.h"

bool 
s_PhrasePro::operator <(const s_PhrasePro &right)const
{
	return dPro < right.dPro;
}

s_PhrasePro&
s_PhrasePro::operator =(const s_PhrasePro &right)
{
	ulEnNum = right.ulEnNum;					
	viEnPhrase = right.viEnPhrase;
	dPro = right.dPro;						
// 	dpce = right.dpce;
// 	dlce = right.dlce;
// 	dpec = right.dpec;
// 	dlec = right.dlec;
	eachTransPro = right.eachTransPro;
	ch_pos_to_en_pos_list = right.ch_pos_to_en_pos_list;

	return *this;
}

////////////////////////////////////////////////////////////////////////
// 1.��Ҫ���ܣ����Ҷ��ڵ㣨�������Ķ�����ң�ÿһ��洢һ�����Ĵʣ�
// 2.��ڲ��������Ķ���Ĵʺ�
// 3.���ڲ������ڵ��ָ��
// 4.�������ڣ�2006.04.10
// 5.�㷨��飺
//
PhraseProTrie* 
PhraseProTrie::findTrie(const int *Indices)
{
	int m_len= Vocab::length(Indices);
        //cout<<m_len<<endl;
	PhraseProTrie* subTrie=this;
	for(int i=0; i<m_len; i++)
	{
		PhraseProTrie* m_subTemp=subTrie->findTrie(Indices[i]);
		if( m_subTemp )
			subTrie = m_subTemp;
		else
		{
			PhraseProTrie m_triTemp;
			subTrie=subTrie->InsertTrie(Indices[i],m_triTemp);
		}
	}
	return subTrie;
}

