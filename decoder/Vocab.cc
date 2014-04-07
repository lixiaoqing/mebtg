/*

	Vocab.cpp

*/
#include "Vocab.h"

////////////////////////////////////////////////////////////////////////
// 1.��Ҫ���ܣ��������еĴʣ�֮���Կո�ֿ����зֿ�
// 2.��ڲ���������ָ�룻�ʵ�ָ�룻��󳤶ȣ�
// 3.���ڲ��������ӳ���
// 4.�������ڣ�2006.04.10
// 5.�㷨��飺���Ҵ�֮��ķָ�������ÿһ���ʷֿ�����������һ��0
//

int
Vocab::ParseEngWords(char* sentence, char** words, int max)
{
	char *word;
	int i = 0;

	for (word = strtok(sentence, g_pcWordSeparators);
		i <= max && word != 0;
		i++, word = strtok(0, g_pcWordSeparators))
	{
		words[i] = word;
	}
	if (i <= max) 
	{
		words[i] = 0;

	}
	else
	{
		words[max]=0;
	}
	return i;

}



int
Vocab::ParseWords(char* sentence, vector<string> &words, int max, ParseWordType parsetype)
{
	int i=0;
	if( parsetype == WORDS )
	{
		istringstream buf(sentence);
		string word = "";
		while( buf>>word && i <= max )
		{
			words.push_back(word);
			i++;
		}
		/*
		//char *sentence=line;	
		while( *sentence && i<=max )
		{
		int pos=0;
		if( *sentence<0 )
		{
		words[i][0] = *(sentence++);
		words[i][1] = *(sentence++);
		words[i][2] = '\0';
		++i;
		continue;
		}
		else if( *sentence == ' ')
		{
		sentence++;
		continue;
		}
		else
		{
		while( *sentence != ' ' && *sentence >0 )
		{
		if( pos > WORD_LEN_MAX -2 )
		break;
		words[i][pos++] = *(sentence++);
		}
		words[i][pos] = '\0';
		++i;
		}
		}
		*/

	}
	else
	{
		istringstream buf(sentence);
		string word = "";
		while( buf>>word && i <= max )
		{
			words.push_back(word);
			i++;
		}

		/*
		   char *word;
		   for (word = strtok(sentence, g_pcWordSeparators);
		   i <= max && word != 0;
		   i++, word = strtok(0, g_pcWordSeparators))
		   {
		//words[i] = word;
		strcpy( words[i], word);
		}
		*/
	}
	return i;
}

////////////////////////////////////////////////////////////////////////
// 1.��Ҫ���ܣ�����ת��Ϊ�ʺ�,����ʵ���û��������´�
// 2.��ڲ���������Ϊ�ʵľ��ӣ��ʺű�ʾ�ľ��ӣ���󳤶�
// 3.���ڲ������ʵĸ���
// 4.�������ڣ�2004.04.10
// 5.�㷨��飺
//
int 
Vocab::GetAndInsertWordsIndics(vector<string> words, int* wids, int max)
{
	int i;

	//for (i = 0; i < max && words[i]!=0 && words[i][0] != 0; i++) 
	for (i = 0; i < words.size(); i++) 
	{
		wids[i] = GetAndInsertIndex(words[i].c_str());
	}
	if( i< max )
	{
		wids[i]= Vocab_None ;
	}
	else
		wids[max-1] = Vocab_None;
	return i;
}

int 
Vocab::GetAndInsertWordsIndics(vector<string> words, vector<int> &wids, int max)
{
	int i;
	//for (i = 0; i < max && words[i]!=0 && words[i][0] != 0; i++) 
	for (i = 0; i < words.size(); i++) 
	{
		wids.push_back( GetAndInsertIndex(words[i].c_str()) );
	}
	wids.push_back( Vocab_None );
	return i;
}

////////////////////////////////////////////////////////////////////////
// 1.��Ҫ���ܣ�����ת��Ϊ�ʺţ���ͳ��δ֪�ʵĸ���
// 2.��ڲ���������Ϊ�ʵľ��ӣ��ʺű�ʾ�ľ��ӣ���󳤶�
// 3.���ڲ������ʵĸ���
// 4.�������ڣ�2004.04.10
// 5.�㷨��飺
//
int 
Vocab::GetWordIndics(char** words, int *wids, int max, int &UnkownNum)
{
	int i;
	UnkownNum = 0;

	for (i = 0; i < max && words[i]!=0 && words[i][0] != 0; i++) 
	{
		wids[i] = GetIndex(words[i]);
		if( _ulUnKnow == wids[i] )
			UnkownNum++;
	}
	if( i< max )
	{
		wids[i]= Vocab_None ;
	}
	else
		wids[max-1] = Vocab_None;
	return i;

}

////////////////////////////////////////////////////////////////////////
// 1.��Ҫ���ܣ����ʺű�ʾ�ľ�����ת
// 2.��ڲ������ʺű�ʾ�ľ���
// 3.���ڲ�������ת��ľ���
// 4.�������ڣ�2006.04.10
// 5.�㷨��飺
//
int *
Vocab::reverse(int *words)
{
	int i, j;	/* j can get negative ! */

	for (i = 0, j = length(words) - 1;	i < j;	i++, j--)
	{
		int x = words[i];
		words[i] = words[j];
		words[j] = x;
	}
	return words;
}



////////////////////////////////////////////////////////////////////////
// 1.��Ҫ���ܣ�������ӳ���
// 2.��ڲ������ʺű�ʾ�ľ���
// 3.���ڲ��������ӳ���
// 4.�������ڣ�2006.04.10
// 5.�㷨��飺
//
int
Vocab::length(const int *words)
{
	unsigned int len = 0;

	while (words[len] != Vocab_None) len++;
	return len;
}


