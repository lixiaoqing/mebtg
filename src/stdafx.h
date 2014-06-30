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

const int LM_ORDER = 5;
const int END_ID=0;
const int RULE_LEN_MAX=7;
const int PROB_NUM=4;
const int WORD_LEN_MAX=100;
const int SEN_LEN_MAX = 500;			  //���ӵ���󳤶�(������)
const int SEN_CHAR_MAX = 5000;         //���ӵ�����ַ���
const int PHRASE_LEN_MAX = 40;         //phrase max length
//const int FEAT_NUM=8;                   //�����ĸ���

const int g_DefaultNgramOrder = 3;      //Ĭ�ϵ�ngram order
const double LogP_PseudoZero = -99.0;			// non-inf value used for log 0
const double LogP_One = 0.0;			    // log(1) 
//const int Vocab_None = (int)-1;  //��־�ʵ���Ĵ�

const int LINE_LEN_MAX = 256;                  //������ģ��ʱһ�е�����ַ���
const int MAX_NGRAM_ORDER = 9;               //����ģ�͵���� Order

#endif
