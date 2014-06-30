#include "stdafx.h"
#include "datastruct.h"
#include "vocab.h"
#include "ruletable.h"
#include "lm.h"
#include "maxent.h"

struct Models
{
	Vocab *src_vocab;
	Vocab *tgt_vocab;
	RuleTable *ruletable;
	MaxentModel *reorder_model;
	LanguageModel *lm_model;
};

class FileTranslator
{
	public:
		FileTranslator(const Models &i_models, const Parameter &i_para, const Weight &i_weight);
		void translate_file(const string &input_file, const string &output_file);

	private:
		Models models;
		Parameter para;
		Weight feature_weight;
};

class SentenceTranslator
{
	public:
		SentenceTranslator(const Models &i_models, const Parameter &i_para, const Weight &i_weight, const string &input_sen);
		string translate_sentence();
	private:
		void fill_matrix_with_matched_rules();
		double cal_increased_lm_score_for_sen_frag(const Cand &cand);
		double cal_increased_lm_score_for_whole_sen(const Cand &cand);
		pair<double> cal_reorder_score(const Cand &cand_lhs,const Cand &cand_rhs);
		void generate_kbest_for_span(size_t beg,size_t span);
		void merge_subcands_and_add_to_pq(const Cand &cand_lhs, const Cand &cand_rhs,int rank_lhs,int rank_rhs,Candpq &new_cands_by_mergence);
		void add_neighbours_to_pq(Cand &cur_cand, Candpq &new_cands_by_mergence);

	private:
		Vocab *src_vocab;
		Vocab *tgt_vocab;
		RuleTable *ruletable;
		MaxentModel *reorder_model;
		LanguageModel *lm_model;
		Parameter para;
		Weight feature_weight;

		vector<vector<Candpq > > candpq_matrix;  //NOTE: maybe store pointer in the matrix is better
		vector<int> src_wids;
		size_t src_sen_len;
};
