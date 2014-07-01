#ifndef DATASTRUCT_H
#define DATASTRUCT_H
#include "stdafx.h"

struct Cand	                
{
	bool operator< (const Cand &cand_rhs) const
	{
		return (score < cand_rhs.score);
	}

	//info about src language
	int beg;			//begin position of covered src span
	int end;			//end position of covered src span
	int mid;			//split position of two subspan, which is the begin postion of the right span
	int phrase_num;			//number of phrases that the src span contains, calculated recursively

	//info about tgt language
	int tgt_word_num;		//the number of tgt words in the candidate translation 
	vector<int> tgt_wids;		//word ids of the candidate translation

	//score info
	double score;			//total score of the candidate
	vector<double> trans_probs;	//translation probabilities
	double lm_prob;
	double mono_reorder_prob;
	double swap_reorder_prob;
	double context_based_trans_prob;

	//merge info
	int rank_lhs;			//rank of the left subcand among all the cands with the same span
	int rank_rhs;			//rank of the right subcand among all the cands with the same span

	Cand ()
	{
		beg = 0;
		end = 0;
		mid = -1;
		phrase_num = 1;

		tgt_word_num = 1;

		score = 0.0;
		lm_prob = 0.0;
		mono_reorder_prob = 0.0;
		swap_reorder_prob = 0.0;
		context_based_trans_prob = 0.0;

		rank_lhs = 0;
		rank_rhs = 0;
	}
};

class Candpq
{
	public:
		void push(const Cand &cand)
		{ 
			for (Cand &e_cand : data)
			{
				if (is_bound_same(cand,e_cand))
				{
					if (cand.score < e_cand.score)
					{
						return;
					}
					if (cand.score > e_cand.score)
					{
						e_cand = cand;
						make_heap(data.begin(),data.end());
						return;
					}
				}
			}
			data.push_back(cand); 
			push_heap(data.begin(), data.end()); 
		}

		void pop()
		{
			pop_heap(data.begin(), data.end());
			data.pop_back();
		}

		Cand top() { return data.front(); }
		Cand at(size_t i) { return data.at(i); }
		int size() { return data.size();  }
		bool empty() { return data.empty(); }
	private:
		bool is_bound_same(const Cand &a, const Cand &b)
		{
			size_t len_a = a.tgt_wids.size();
			size_t len_b = b.tgt_wids.size();
			size_t bound_len_a = min(len_a, LM_ORDER-1);
			size_t bound_len_b = min(len_b, LM_ORDER-1);
			if (bound_len_a != bound_len_b)
				return false;
			if (bound_len_a < LM_ORDER && a.tgt_wids != b.tgt_wids)
				return false;
			for (size_t i=0;i<bound_len_a;i++)
			{
				if (a.tgt_wids.at(i) != b.tgt_wids.at(i) || a.tgt_wids.at(len_a-1-i) != b.tgt_wids.at(len_b-1-i))
					return false;
			}
			return true;
		}
	private:
		vector<Cand> data;
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
	size_t BEAM_SIZE;				//beam size threshold
	size_t EXTRA_BEAM_SIZE;
	size_t NBEST_NUM;				//nbest number
	size_t REORDER_WINDOW;       			//window size of reordering
	size_t RULE_NUM_LIMIT;			       	//number of tgt rules for each src side
	bool PRINT_NBEST;
};

struct Weight
{
	vector<double> trans;
	double lm;
	double reorder_mono;
	double reorder_swap;
	double len;					//word number in tgt translation
	double phrase_num;				//phrase number in src side
	//double sense;
};

#endif
