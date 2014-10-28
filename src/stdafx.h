#ifndef STDAFX_H
#define STDAFX_H

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <set>
#include <vector>
#include <map>
#include <unordered_map>

#include <algorithm>
#include <bitset>
#include <queue>
#include <functional>
#include <limits>


#include <zlib.h>
#include <stdio.h>
#include <math.h>
#include <time.h>
#include <assert.h>
#include <string.h>
#include <pthread.h>
#include <omp.h>


using namespace std;

const size_t LM_ORDER = 5;
const size_t PROB_NUM=4;
const size_t RULE_LEN_MAX=7;
const double LogP_PseudoZero = -99.0;
const double LogP_One = 0.0;

struct TuneInfo
{
	size_t sen_id;
	string translation;
	vector<double> feature_values;
	double total_score;
};

struct Filenames
{
	string input_file;
	string output_file;
	string nbest_file;
	string src_vocab_file;
	string tgt_vocab_file;
	string rule_table_file;
	string lm_file;
	string reorder_model_file;
	string wsd_model_file;
};

struct Parameter
{
	size_t BEAM_SIZE;					//优先级队列的大小限制
	size_t SEN_THREAD_NUM;				//句子级并行数
	size_t SPAN_THREAD_NUM;				//span级并行数
	size_t NBEST_NUM;
	size_t REORDER_WINDOW;     			//最大调序范围
	size_t RULE_NUM_LIMIT;		      	//源端相同的情况下最多能加载的规则数
	bool PRINT_NBEST;
	bool DUMP_RULE;						//是否输出所使用的规则
	bool LOAD_ALIGNMENT;				//加载短语表时是否加载短语内部的词对齐
};

struct Weight
{
	vector<double> trans;
	double lm;
	double reorder_mono;
	double reorder_swap;
	double len;							//译文的单词数
	double phrase_num;					//源端被切成的短语数
};

#endif
