/*
	Vocab.h
	�洢���ʱ�(source and target)
	���ʺŲ��ҵ���
	�����ʲ��Ҵʺ�
*/

#ifndef VOCAB_H
#define VOCAB_H

#include "StdAfx.h"

//char* const g_pcWordSeparators = " \t\r\n";     //��֮��ķָ���


//char* const g_Vocab_NULL = "NULL";                //��
//char* const g_Vocab_Unknown = "<unk>";            //δ֪��
//char* const g_Vocab_SentStart = "<s>";            //���ӵĿ�ʼ��־
//char* const	g_Vocab_SentEnd = "</s>";             //���ӵĽ�����־
//char* const g_Vocab_NE = "<_NE>";                    //����NE
//char* const g_Vocab_Time = "<_TIME>";                //ʱ��
//char* const g_Vocab_Number="<_NUMBER>";              //����

//const int Vocab_None = (int)-1;  //��־�ʵ���Ĵ�



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

	}; //��ʼ����

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
	//���һ����ʺţ�
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

	//���ʺ�ת��Ϊ����
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

	int GetAndInsertWordsIndics(vector<string> words, int *wids, int max);		//����ת��Ϊ�ʺŻ����
	int GetAndInsertWordsIndics(vector<string> words, vector<int> &wids, int max);
	int GetWordIndics(char** words, int *wids, int max, int &UnkownNum);	//����ת��Ϊ�ʺţ���ͳ��δ֪�ʵ�����
	//static int ParseWords(char* sentence,char** words, int max, ParseWordType parsetype);			//�����ӻ���Ϊ�� 
	static int ParseWords(char* sentence, vector<string> &words, int max, ParseWordType parsetype);			//�����ӻ���Ϊ�� 
	static int ParseEngWords(char* sentence, char* * words, int max);
	static int *reverse(int *words);									//���ʺ���ת
	static int length(const int *words);								//������ӵĳ���


	 int& UnNull(){ return _ulNull; };			//����UnKnow Word�Ĵʺ�
	 int& UnKnow(){ return _ulUnKnow; };          //����UnKnow Word�Ĵʺ�
	 int& SenStart(){ return _ulSenStart; };      //����UnKnow Word�Ĵʺ�
	 int& SenEnd(){ return _ulSenEnd; };         //����UnKnow Word�Ĵʺ�
	 int& NEIndex(){return _ulNE;	 };            //����NE Word�Ĵʺ�
	 int& TimeIndex(){ return _ulTime; };			//����TIME Word�Ĵʺ�
	 int& NumberIndex(){ return _ulNumber; };		//����NUMBER Word�Ĵʺ�
	 int& LocIndex(){ return _ulLoc; };
	 int& PersonIndex(){ return _ulPerson; };
	 int& OrgIndex(){ return _ulOrg; };
	 int& NamexIndex(){ return _ulNamex; };

private:
	int Insert(string word)         //�����ʲ��뵽���ʱ��У������شʺ�
	{
		vocab_data.push_back(word);
		index_data.insert(map<string, int>::value_type( word, _VocabSize ) );
		_VocabSize++;
		return _VocabSize-1;
	};

	int _VocabSize;					//���ʱ�Ĵ�С
	vector<string> vocab_data;      //���ʱ����ڽ��ʺ�ת��Ϊ���ʣ�
	map<string,int> index_data;		//���������ڽ�����ת��Ϊ�ʺţ�

	int _ulNull;              //g_Vocab_NULL = "NULL";�� index
	int _ulUnKnow;              //g_Vocab_Unknown = "<unk>";�� index
	int _ulSenStart;			//g_Vocab_SentStart = "<s>";�� index
	int _ulSenEnd;			//g_Vocab_SentEnd = "</s>";�� index
	int _ulNE;                //g_Vocab_NE = "<_NE>";�� index
	int _ulTime;				//g_Vocab_Time = "<_TIME>";�� index
	int _ulNumber;			//g_Vocab_Number="<_NUMBER>";�� index
	int _ulLoc;
	int _ulPerson;
	int _ulNamex;
	int _ulOrg;
};

static Vocab* _pVocabChi;                  //���ĵ��ʱ�
static Vocab* _pVocabEng;					//Ӣ�ĵ��ʱ�

#endif


