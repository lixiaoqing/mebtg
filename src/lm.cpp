#include "lm.h"

double LanguageModel::eval(const vector<string> &words)
{
	State state(kenlm->BeginSentenceState()), out_state;
	const Vocabulary &vocab = kenlm->GetVocabulary();
	double sen_score = 0;
	double ngram_score = 0;
	for (const auto &word : words)
	{
		ngram_score = kenlm->Score(state, vocab.Index(word), out_state);
		sen_score += ngram_score;
		state = out_state;
	}
	return sen_score;
}
