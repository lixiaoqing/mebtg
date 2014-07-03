#ifndef DATASTRUCT_H
#define DATASTRUCT_H
#include "stdafx.h"

//存储翻译候选
struct Cand	                
{
	//源端信息
	int beg;			//当前候选在源语言中的起始位置
	int end;			//当前候选在源语言中的终止位置
	int phrase_num;			//当前候选包含的短语数

	//目标端信息
	int tgt_word_num;		//当前候选目标端的单词数
	vector<int> tgt_wids;		//当前候选目标端的id序列

	//打分信息
	double score;			//当前候选的总得分
	vector<double> trans_probs;	//翻译概率
	double lm_prob;
	double mono_reorder_prob;
	double swap_reorder_prob;

	//合并信息,记录当前候选是由两个子候选合并得来时的相关信息
	int mid;			//记录两个子候选在源语言中的交界位置
	int tgt_mid;			//记录两个子候选在目标语言中的交界位置,即第一个子候选目标端的长度
	int rank_lhs;			//记录第一个子候选在优先级队列中的排名
	int rank_rhs;			//记录第二个子候选在优先级队列中的排名

	Cand ()
	{
		beg = 0;
		end = 0;
		phrase_num = 1;

		tgt_word_num = 1;
		tgt_wids.clear();

		score = 0.0;
		trans_probs.clear();
		lm_prob = 0.0;
		mono_reorder_prob = 0.0;
		swap_reorder_prob = 0.0;

		mid = -1;
		tgt_mid = -1;
		rank_lhs = 0;
		rank_rhs = 0;
	}
};

bool cmp(const Cand *pl, const Cand *pr);

//将跨度相同的候选组织到优先级队列中
class Candpq
{
	public:
		void push(Cand *cand_ptr);

		void pop();

		Cand* top() { return data.front(); }
		Cand* at(size_t i) { return data.at(i); }
		int size() { return data.size();  }
		bool empty() { return data.empty(); }
	private:
		bool is_bound_same(const Cand *a, const Cand *b);
	private:
		vector<Cand*> data;
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
};

struct Parameter
{
	size_t BEAM_SIZE;				//优先级队列的大小限制
	size_t EXTRA_BEAM_SIZE;
	size_t NBEST_NUM;
	size_t REORDER_WINDOW;       			//最大调序范围
	size_t RULE_NUM_LIMIT;			       	//源端相同的情况下最多能加载的规则数
	bool PRINT_NBEST;
};

struct Weight
{
	vector<double> trans;
	double lm;
	double reorder_mono;
	double reorder_swap;
	double len;					//译文的单词数
	double phrase_num;				//源端被切成的短语数
};

#endif
