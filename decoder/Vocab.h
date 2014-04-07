/*
	Vocab.h
	存储单词表(source and target)
	按词号查找单词
	按单词查找词号
*/

#ifndef VOCAB_H
#define VOCAB_H

#include "StdAfx.h"

//char* const g_pcWordSeparators = " \t\r\n";     //词之间的分隔符


//char* const g_Vocab_NULL = "NULL";                //空
//char* const g_Vocab_Unknown = "<unk>";            //未知词
//char* const g_Vocab_SentStart = "<s>";            //句子的开始标志
//char* const	g_Vocab_SentEnd = "</s>";             //句子的结束标志
//char* const g_Vocab_NE = "<_NE>";                    //代表NE
//char* const g_Vocab_Time = "<_TIME>";                //时间
//char* const g_Vocab_Number="<_NUMBER>";              //数字

//const int Vocab_None = (int)-1;  //标志词典外的词



class Vocab
{
public:
	Vocab(const char *dict_filename):_VocabSize(0)
	{
                LoadDict(dict_filename);
		_ulNull = GetIndex(g_Vocab_NULL);
		_ulUnKnow = GetIndex(g_Vocab_Unknown);
		_ulSenStart = GetIndex(g_Vocab_SentStart);
		_ulSenEnd = GetIndex(g_Vocab_SentEnd);
		_ulNE = GetIndex(g_Vocab_NE);
		_ulTime = GetIndex(g_Vocab_Time);
		_ulNumber = GetIndex(g_Vocab_Number);
		_ulLoc = GetIndex( g_Vocab_Loc );
		_ulOrg = GetIndex( g_Vocab_Org );
		_ulPerson = GetIndex( g_Vocab_Person );
		_ulNamex = GetIndex( g_Vocab_Namex ); 

	}; //初始化；

	//load dictionary
	int LoadDict(const char *dict_filename)
	{
	    ifstream dict_file(dict_filename);
	    if(!dict_file)
	    {
		cerr<<"cannot open the dict file!"<<endl;
		return 0;
	    }

	    char char_line[1024];
	    while(dict_file.getline(char_line,1024))
	    {
		string str_line(char_line);
		istringstream buff(str_line);
		string word,index;
		if(!(buff>>word>>index))
		{
		    cerr<<"reading word and index error!"<<endl;
		    return 0;
		}
		int c_index=Insert(word);
		if(c_index != atoi(index.c_str()))
		{
		    cerr<<"the index error!"<<endl;
		    return 0;
		}
	    }
	    return 1;
	}
	//查找或加入词号；
	int GetAndInsertIndex(const char *word)                                  
	{

		map<string, int>::iterator it=index_data.find( word );
		if( it != index_data.end() )
			return (*it).second;
		else
			return Insert(word);
	} 

	int Convert(string str)
	{
		return GetAndInsertIndex(str.c_str());
	}

	string Convert(int  index)
	{
		return GetWord(index);
	}

	int GetIndex( const char *word )
	{
		map<string, int>::iterator it=index_data.find( word );
		if( it != index_data.end() )
			return (*it).second;
		else
			return _ulUnKnow;
	};

	//将词号转化为单词
	string GetWord(int index)
	{
		if( index<_VocabSize )
			return vocab_data[index].c_str();
		else
			return "NULL";
	};

	int size()
	{
		return _VocabSize;
	};

	int GetAndInsertWordsIndics(vector<string> words, int *wids, int max);		//将词转化为词号或插入
	int GetAndInsertWordsIndics(vector<string> words, vector<int> &wids, int max);
	int GetWordIndics(char** words, int *wids, int max, int &UnkownNum);	//将此转化为词号，并统计未知词的数量
	//static int ParseWords(char* sentence,char** words, int max, ParseWordType parsetype);			//将句子划分为词 
	static int ParseWords(char* sentence, vector<string> &words, int max, ParseWordType parsetype);			//将句子划分为词 
	static int ParseEngWords(char* sentence, char* * words, int max);
	static int *reverse(int *words);									//将词号逆转
	static int length(const int *words);								//计算句子的长度


	 int& UnNull(){ return _ulNull; };			//返回UnKnow Word的词号
	 int& UnKnow(){ return _ulUnKnow; };          //返回UnKnow Word的词号
	 int& SenStart(){ return _ulSenStart; };      //返回UnKnow Word的词号
	 int& SenEnd(){ return _ulSenEnd; };         //返回UnKnow Word的词号
	 int& NEIndex(){return _ulNE;	 };            //返回NE Word的词号
	 int& TimeIndex(){ return _ulTime; };			//返回TIME Word的词号
	 int& NumberIndex(){ return _ulNumber; };		//返回NUMBER Word的词号
	 int& LocIndex(){ return _ulLoc; };
	 int& PersonIndex(){ return _ulPerson; };
	 int& OrgIndex(){ return _ulOrg; };
	 int& NamexIndex(){ return _ulNamex; };

private:
	int Insert(string word)         //将单词插入到单词表中，并返回词号
	{
		vocab_data.push_back(word);
		index_data.insert(map<string, int>::value_type( word, _VocabSize ) );
		_VocabSize++;
		return _VocabSize-1;
	};

	int _VocabSize;					//单词表的大小
	vector<string> vocab_data;      //单词表，用于将词号转化为单词；
	map<string,int> index_data;		//索引表，用于将单词转化为词号；

	int _ulNull;              //g_Vocab_NULL = "NULL";的 index
	int _ulUnKnow;              //g_Vocab_Unknown = "<unk>";的 index
	int _ulSenStart;			//g_Vocab_SentStart = "<s>";的 index
	int _ulSenEnd;			//g_Vocab_SentEnd = "</s>";的 index
	int _ulNE;                //g_Vocab_NE = "<_NE>";的 index
	int _ulTime;				//g_Vocab_Time = "<_TIME>";的 index
	int _ulNumber;			//g_Vocab_Number="<_NUMBER>";的 index
	int _ulLoc;
	int _ulPerson;
	int _ulNamex;
	int _ulOrg;
};

static Vocab* _pVocabChi;                  //中文单词表
static Vocab* _pVocabEng;					//英文单词表

#endif


