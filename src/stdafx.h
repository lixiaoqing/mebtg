#ifndef STDAFX_H
#define STDAFX_H

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <set>
#include <vector>
#include <map>

#include <algorithm>
#include <bitset>
//#include <queue>
#include <functional>


#include <zlib.h>
#include <stdio.h>
#include <math.h>
#include <time.h>
#include <assert.h>
#include <string.h>
#include <pthread.h>


using namespace std;

const size_t LM_ORDER = 5;
//const size_t START_ORDER_FOR_EVAL = 1;
//const size_t END_ID=0;
const size_t RULE_LEN_MAX=7;
const size_t PROB_NUM=4;
const size_t WORD_LEN_MAX=100;
const size_t SEN_LEN_MAX = 500;			  //句子的最大长度(单词数)
const size_t SEN_CHAR_MAX = 5000;         //句子的最大字符数
const size_t PHRASE_LEN_MAX = 40;         //phrase max length
//const size_t FEAT_NUM=8;                   //特征的个数

const size_t g_DefaultNgramOrder = 3;      //默认的ngram order
const double LogP_PseudoZero = -99.0;			// non-inf value used for log 0
const double LogP_One = 0.0;			    // log(1) 
//const int Vocab_None = (int)-1;  //标志词典外的词

const size_t LINE_LEN_MAX = 256;                  //读语言模型时一行的最大字符数
const size_t MAX_NGRAM_ORDER = 9;               //语言模型的最大 Order

#endif
