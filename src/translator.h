#include "stdafx.h"
#include "datastruct.h"
#include "vocab.h"
#include "ruletable.h"

class FileTranslator
{
	public:
		void translate_file(const string &input_file, const string &output_file);

	private:
		Vocab *src_vocab;
		Vocab *tgt_vocab;
		RuleTable *ruletable;
		MaxentModel *reorder_model;
		LanguageModel *lm_model;
};

class SentenceTranslator
{
	public:
		string translate_sentence();
	private:
		void fill_matrix_with_matched_rules();

	private:
		Vocab *src_vocab;
		Vocab *tgt_vocab;
		RuleTable *ruletable;
		MaxentModel *reorder_model;
		LanguageModel *lm_model;

	vector<vector<priority_queue<Cand> > > candpq_matrix;  //NOTE: maybe store pointer in the matrix is better
	vector<int> src_word_id_list;
	size_t src_sen_len;
};
