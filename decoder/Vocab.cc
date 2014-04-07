/*

	Vocab.cpp

*/
#include "Vocab.h"

////////////////////////////////////////////////////////////////////////
// 1.主要功能：将句子中的词（之间以空格分开）切分开
// 2.入口参数：句子指针；词的指针；最大长度；
// 3.出口参数：句子长度
// 4.生成日期：2006.04.10
// 5.算法简介：查找词之间的分隔符，将每一个词分开，在最后添加一个0
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
// 1.主要功能：将词转化为词号,如果词典中没有则插入新词
// 2.入口参数：划分为词的句子；词号表示的句子；最大长度
// 3.出口参数：词的个数
// 4.生成日期：2004.04.10
// 5.算法简介：
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
// 1.主要功能：将词转化为词号，并统计未知词的个数
// 2.入口参数：划分为词的句子；词号表示的句子；最大长度
// 3.出口参数：词的个数
// 4.生成日期：2004.04.10
// 5.算法简介：
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
// 1.主要功能：将词号表示的句子逆转
// 2.入口参数：词号表示的句子
// 3.出口参数：逆转后的句子
// 4.生成日期：2006.04.10
// 5.算法简介：
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
// 1.主要功能：计算句子长度
// 2.入口参数：词号表示的句子
// 3.出口参数：句子长度
// 4.生成日期：2006.04.10
// 5.算法简介：
//
int
Vocab::length(const int *words)
{
	unsigned int len = 0;

	while (words[len] != Vocab_None) len++;
	return len;
}


