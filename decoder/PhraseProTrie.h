/*
	PhraseProTrie.h
	phrase������ʵĴ洢�ṹ
*/
#ifndef PHRASEPROTRIE_H
#define PHRASEPROTRIE_H
#include "StdAfx.h"
#include "Vocab.h"

//�洢Ӣ�Ķ���
class s_PhrasePro		         	        
{	
public:
	bool operator<(const s_PhrasePro &right) const;
	s_PhrasePro& operator=(const s_PhrasePro& right);
	int ulEnNum;						//Ӣ�Ĵʵĸ���
	vector<int> viEnPhrase;
	double dPro;								//����
	vector<double> eachTransPro;
	vector<vector<int> > ch_pos_to_en_pos_list;                //ÿ�����Ĵʶ�Ӧ��Ӣ�Ĵ�λ��
// 	double dpce;
// 	double dlce;
// 	double dpec;
// 	double dlec;
};


class PhraseProTrie
{
public:
	//���ؽڵ��ֵ
	vector<s_PhrasePro> &value()
	{
		return _phrasePro;
	};

	//����һ���ڵ�
	PhraseProTrie* InsertTrie(const int &Index, PhraseProTrie &newTrie)
	{
		_subPhraseTrie.insert(map<int, PhraseProTrie>::value_type(Index,newTrie) );
		return &_subPhraseTrie[Index];
	};
	
	//����һ���ڵ�;
	PhraseProTrie* findTrie(const int &Index)         
	{
		map<int ,PhraseProTrie>::iterator it= _subPhraseTrie.find(Index);
		if( _subPhraseTrie.end() != it )
			return &( (*it).second );
		else
			return NULL;
	};


	PhraseProTrie* findTrie(const int *Indices);      //����һ�����ڵ�,û�������
private:
	vector<s_PhrasePro> _phrasePro;						//�洢һ�����Ķ����Ӧ������Ӣ�Ķ���
	map<int, PhraseProTrie> _subPhraseTrie;	//�洢�ڵ�
};

#endif //!define PHRASEPRO_H

