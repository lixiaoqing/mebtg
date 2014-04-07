/* 
 stdafx.h
 常量声明
 */

#ifndef STDAFX_H
#define STDAFX_H

# define _LARGEFILE64_SOURCE
# define _FILE_OFFSET_BITS 64

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <set>
#include <vector>
#include <map>

#include <algorithm>
#include <bitset>
//以下两个头文件为张家俊08年12月25日添加
#include <queue>
#include <functional>


using namespace std;

#include <stdio.h>
#include <math.h>
#include <time.h>
#include <assert.h>
#include <string.h>
#include <pthread.h>


const int WORD_LEN_MAX=100;
const int SEN_LEN_MAX = 500;			  //句子的最大长度(单词数)
const int SEN_CHAR_MAX = 5000;         //句子的最大字符数
const int PHRASE_LEN_MAX = 40;         //phrase max length
//const int FEAT_NUM=8;                   //特征的个数



//#define HUGE_VAL 1000

char* const g_pcPhraseSeparators= "|||";
            
char* const g_pcWordSeparators = " \t\r\n";			 //词之间的分隔符
char* const g_Vocab_NULL = "NULL";					 //空
char* const g_Vocab_Unknown = "<unk>";				 //未知词
char* const g_Vocab_SentStart = "<s>";				  //句子的开始标志
char* const     g_Vocab_SentEnd = "</s>";				  //句子的结束标志
            
char* const g_Vocab_NE = "_NE";					  //代表NE
char* const g_Vocab_Org = "_ORG";					//组织机构名
char* const g_Vocab_Time = "_TIMEP" ;                //代表时间
char* const g_Vocab_Number= "_DIGITP" ;              //代表数字
char* const g_Vocab_Loc = "_LOCATION";				//地名
char* const g_Vocab_Person = "_PERSON";				//人名
char* const g_Vocab_Namex = "_NAMEX";				//集外词
            
char* const g_NP_Separator="|||";						//翻译候选项的分隔符
char* const g_Number_Start="<NUMBER>";					//给定短语翻译候选项的数字起始标志
char* const g_Number_End = "</NUMBER>";					//给定短语翻译候选项的数字结束标志
char* const g_Time_Start = "<TIME>";					//给定短语翻译候选项的时间起始标志
char* const g_Time_End = "</TIME>";						//给定短语翻译候选项的时间结束标志
char* const g_Loc_Start = "<LOC>";
char* const g_Loc_End = "</LOC>";
char* const g_Person_Start = "<PERSON>";
char* const g_Person_End = "</PERSON>";
char* const g_Org_Start = "<ORG>";
char* const g_Org_End = "</ORG>";
char* const g_Namex_Start = "<NAMEX>";					//
char* const g_Namex_End = "</NAMEX>";
char* const g_NE_Start = "<NE>";						//给定短语翻译候选项的NE起始标志
char* const g_NE_End = "</NE>";							//给定短语翻译候选项的NE结束标志
char* const g_Normal_Start = "<NORMAL>";				//给定短语翻译候选项的一般短语起始标志
char* const g_Normal_End = "</NORMAL>";					//给定短语翻译候选项的一般短语结束标志


//Confusion network 标志
char* const g_Epsilon_Word = "*DELETE*";
char* const g_Start_CN = "!SENT_START";
char* const g_END_CN = "!SENT_END";

//中文标点符号
//const string g_Chinese_Punc_Set = "，、；：“”。！？（）《》［］";
const string g_other_punc_set = ". , ; : \" ! ? ( ) < > [ ]";



const int g_DefaultNgramOrder = 3;      //默认的ngram order
const double LogP_PseudoZero = -99.0;			// non-inf value used for log 0
const double LogP_Zero = -HUGE_VAL;		    //log(0) 
const double LogP_Inf = HUGE_VAL;			// log(Inf) 
const double LogP_One = 0.0;			    // log(1) 
const int Vocab_None = (int)-1;  //标志词典外的词


const int LINE_LEN_MAX = 256;                  //读语言模型时一行的最大字符数
const int MAX_NGRAM_ORDER = 9;               //语言模型的最大 Order

typedef enum{XML, TXT, CN}FileFormate;
typedef enum{NOSEG, SEG, CHARACTER}SegFormate;
typedef enum{WORDS, SPACE}ParseWordType;
typedef enum{NULLWORD, SOURCEWORD}UnknownWord;
enum HEAPTYPE {MINHEAP, MAXHEAP};

typedef struct{
	string lmfilename;						//language model probability file
	string phrasefilename;					//phrase translation probability file
	string testfilename;					//test file 
	string resultfilename;					//result file
	string nbestfilename;					//n_best result file
	string sourcelmfilename;                //source language model
	//调序模型文件由张家俊09年1月5日添加
	string reorder_model_filename;
	//基于上下文的翻译模型文件由李小青于14年3月29日添加,此处为记录所有模型文件名的目录文件
	string catalog_filename;

	string candsfilename;
	string featsfilename;
	string initfile;

        string sourcevocabfilename;
        string targetvocabfilename;


	int beam_hist;				//beam size threshold
	double beam_prob;                          //probability threshold of beam search

	int ttable;				//translation table size
	int nbest;				//nbest number
	int lmorder;              //language model order

	int reorder_window;       //window size of reordering
	int cube_flag;            //whether decode using cube prunniing


	double Lambda_lm;					//weight of language model 
	double Lambda_sense;					//weight of context based translation model
	double Lambda_reorderStraight;					//weight of reordering model 
	double Lambda_reorderSwap;					//weight of reordering model 
	vector<double> Lambda_trans;         //all the weights of phrase pairs
	double Lambda_len;					//weight of length
	double Lambda_dPhraseNum;
	bool recombine;						//whether recombine or not

	bool KenLM;

	bool reduceVoc;						//reduce the phrase and LM table depending on the testfile
	bool train;							//train for parameters

	int feats_num;
	bool printInfo;
	bool printNbest;

	int threadsNum;


	SegFormate segmenttype;
	FileFormate filetype;
	UnknownWord transUnknownWord;

	//语言对选项，09年06月23日添加
	int chn2eng_flag;          //表示中文到英文的翻译
	string g_Chinese_Punc_Set;  //不进行调序的中文标点约束
	string g_Chinese_Split_Puncs; //控制子句分割的中文标点约束

}Config;


typedef struct{
	vector<double> trans;
	double lm;
	double sense;
	double len;
	double furt;
	double reorderStraight;
	double reorderSwap;
	double dPhraseNum;
}Lambda;



#endif // !define STDAFX_H

