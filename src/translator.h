#include "stdafx.h"
#include "datastruct.h"
#include "vocab.h"
#include "ruletable.h"
#include "lm.h"
#include "maxent.h"
#include "myutils.h"

struct Models
{
	Vocab *src_vocab;
	Vocab *tgt_vocab;
	RuleTable *ruletable;
	MaxentModel *reorder_model;
	LanguageModel *lm_model;
};

class SentenceTranslator
{
	public:
		SentenceTranslator(const Models &i_models, const Parameter &i_para, const Weight &i_weight, const string &input_sen);
		~SentenceTranslator();
		string translate_sentence();
		vector<Tune_info> get_tune_info(size_t sen_id);
	private:
		void fill_matrix_with_matched_rules();
		double cal_increased_lm_score_for_sen_frag(const Cand *cand);
		double cal_increased_lm_score_for_whole_sen(const Cand *cand);
		pair<double,double> cal_reorder_score(const Cand *cand_lhs,const Cand *cand_rhs);
		void generate_kbest_for_span(const size_t beg,const size_t span);
		void merge_subcands_and_add_to_pq(const Cand *cand_lhs, const Cand *cand_rhs,int rank_lhs,int rank_rhs,Candpq &new_cands_by_mergence);
		void add_neighbours_to_pq(Cand *cur_cand, Candpq &new_cands_by_mergence);
		string words_to_str(vector<string> words);

	private:
		Vocab *src_vocab;
		Vocab *tgt_vocab;
		RuleTable *ruletable;
		MaxentModel *reorder_model;
		LanguageModel *lm_model;
		Parameter para;
		Weight feature_weight;

		vector<vector<Candli> > candli_matrix;		//存储解码过程中所有跨度对应的候选列表, 
													//candli_matrix[i][j]存储起始位置为i, 跨度为j的候选列表
		vector<int> src_wids;
		size_t src_sen_len;
};
