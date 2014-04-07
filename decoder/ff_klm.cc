#include "ff_klm.h"

#include <cstring>
#include <iostream>

//#include <boost/scoped_ptr.hpp>

//#include "filelib.h"
//#include "stringlib.h"
//#include "hg.h"
//#include "tdict.h"
#include "lm/model.hh"
#include "lm/enumerate_vocab.hh"

#include "lm/left.hh"

using namespace std;

// -x : rules include <s> and </s>
// -n NAME : feature id is NAME
bool ParseLMArgs(string const& in, string* filename, string* mapfile, bool* explicit_markers, string* featname) {
  *explicit_markers = false;
  *featname="LanguageModel";
  *mapfile = "";
  
  if( "" == in )
	  return false;
  else
	 *filename = in;
}

template <class Model>
string KLanguageModel<Model>::usage(bool /*param*/,bool /*verbose*/) {
  return "KLanguageModel";
}

namespace {

struct VMapper : public lm::EnumerateVocab {
  VMapper(vector<lm::WordIndex>* out, Vocab* vocab) : out_(out), kLM_UNKNOWN_TOKEN(0),lm_vocab(vocab) { out_->clear(); }
  void Add(lm::WordIndex index, const StringPiece &str) {
    const int dec_id = lm_vocab->Convert(str.as_string());
    if (dec_id >= out_->size())
      out_->resize(dec_id + 1, kLM_UNKNOWN_TOKEN);
    (*out_)[dec_id] = index;
  }
  vector<lm::WordIndex>* out_;
  const lm::WordIndex kLM_UNKNOWN_TOKEN;
  Vocab* lm_vocab;
};

/*
#pragma pack(push)
#pragma pack(1)

struct BoundaryAnnotatedState {
  lm::ngram::ChartState state;
  bool seen_bos, seen_eos;
};

#pragma pack(pop)
*/

template <class Model> class BoundaryRuleScore {
  public:
    BoundaryRuleScore(const Model &m, BoundaryAnnotatedState &state) : 
        back_(m, state.state),
        bos_(state.seen_bos),
        eos_(state.seen_eos),
        penalty_(0.0),
        end_sentence_(m.GetVocabulary().EndSentence()) {
      bos_ = false;
      eos_ = false;
    }

    void BeginSentence() {
      back_.BeginSentence();
      bos_ = true;
    }

    void BeginNonTerminal(const BoundaryAnnotatedState &sub) {
      back_.BeginNonTerminal(sub.state, 0.0f);
      bos_ = sub.seen_bos;
      eos_ = sub.seen_eos;
    }

    void NonTerminal(const BoundaryAnnotatedState &sub) {
      back_.NonTerminal(sub.state, 0.0f);
      // cdec only calls this if there's content.  
      if (sub.seen_bos) {
        bos_ = true;
        penalty_ -= 100.0f;
      }
      if (eos_) penalty_ -= 100.0f;
      eos_ |= sub.seen_eos;
    }

    void Terminal(lm::WordIndex word) {
      back_.Terminal(word);
      if (eos_) penalty_ -= 100.0f;
      if (word == end_sentence_) eos_ = true;
    }

    float Finish() {
      return penalty_ + back_.Finish();
    }

  private:
    lm::ngram::RuleScore<Model> back_;
    bool &bos_, &eos_;

    float penalty_;

    lm::WordIndex end_sentence_;
};

} // namespace

template <class Model>
class KLanguageModelImpl {
 public:
  double LookupWords(s_SearchSpace* rule, double* oovs) {
    *oovs = 0;
	//BoundaryAnnotatedState* remnant = new BoundaryAnnotatedState();
	BoundaryAnnotatedState* remnant = &(rule->cur_state);
    BoundaryRuleScore<Model> ruleScore(*ngram_, *remnant);
	if( rule->m_split_point >= 0 ) { //using merging rules
		ruleScore.BeginNonTerminal((rule->left_ant)->cur_state);
		ruleScore.NonTerminal((rule->right_ant)->cur_state);
	}
	else { //using phrasal rules
		vector<int> e = rule->viEnPhrase;
		for( int i = 0; i < e.size(); i++ ) {
			const lm::WordIndex cur_word = MapWord(e[i]);
			ruleScore.Terminal(cur_word);
		}
	}
    double ret = ruleScore.Finish();
    remnant->state.ZeroRemaining();
	//rule->cur_state = remnant;
    return ret;
  }

  // this assumes no target words on final unary -> goal rule.  is that ok?
  // for <s> (n-1 left words) and (n-1 right words) </s>
  double FinalTraversalCost(s_SearchSpace* edge, double* oovs) {
    const BoundaryAnnotatedState &annotated = edge->cur_state;
    if (add_sos_eos_) {  // rules do not produce <s> </s>, so do it here
      assert(!annotated.seen_bos);
      assert(!annotated.seen_eos);
      lm::ngram::ChartState cstate;
      lm::ngram::RuleScore<Model> ruleScore(*ngram_, cstate);
      ruleScore.BeginSentence();
      ruleScore.NonTerminal(annotated.state, 0.0f);
      ruleScore.Terminal(kEOS_);
      return ruleScore.Finish();
    } else {  // rules DO produce <s> ... </s>
      double ret = 0.0;
      if (!annotated.seen_bos) ret -= 100.0;
      if (!annotated.seen_eos) ret -= 100.0;
      return ret;
    }
  }


  // converts to cdec word id's to KenLM's id space, OOVs and <unk> end up at 0
  lm::WordIndex MapWord(int w) const {
    if (w >= dec2klm_map_.size())
      return 0;
    else
      return dec2klm_map_[w];
  }

 public:
  KLanguageModelImpl(const string& filename, const string& mapfile, bool explicit_markers, Vocab* lm_vocab) :
      //kCDEC_UNK(_pVocabEng->Convert("<unk>")) ,
      //kCDEC_SOS(_pVocabEng->Convert("<s>")) ,
      add_sos_eos_(!explicit_markers) {
		  kCDEC_UNK = lm_vocab->Convert("<unk>");
		  kCDEC_SOS = lm_vocab->Convert("<s>");
    {
      VMapper vm(&dec2klm_map_, lm_vocab);
      lm::ngram::Config conf;
      conf.enumerate_vocab = &vm;
      ngram_ = new Model(filename.c_str(), conf);
    }
    order_ = ngram_->Order();
    cerr << "Loaded " << order_ << "-gram KLM from " << filename << " (MapSize=" << dec2klm_map_.size() << ")\n";

    // special handling of beginning / ending sentence markers
    kSOS_ = MapWord(kCDEC_SOS);
    assert(kSOS_ > 0);
    kEOS_ = MapWord(lm_vocab->Convert("</s>"));
    assert(kEOS_ > 0);
    assert(MapWord(kCDEC_UNK) == 0); // KenLM invariant

  }

  ~KLanguageModelImpl() {
    delete ngram_;
  }

  int ReserveStateSize() const { return sizeof(BoundaryAnnotatedState); }

 private:
  int kCDEC_UNK;
  int kCDEC_SOS;
  lm::WordIndex kSOS_;  // <s> - requires special handling.
  lm::WordIndex kEOS_;  // </s>
  Model* ngram_;
  const bool add_sos_eos_; // flag indicating whether the hypergraph produces <s> and </s>
                     // if this is true, FinalTransitionFeatures will "add" <s> and </s>
                     // if false, FinalTransitionFeatures will score anything with the
                     // markers in the right place (i.e., the beginning and end of
                     // the sentence) with 0, and anything else with -100

  int order_;
  vector<lm::WordIndex> dec2klm_map_;
};

template <class Model>
KLanguageModel<Model>::KLanguageModel(const string& param, Vocab* lm_vocab) {
  string filename, mapfile, featname;
  bool explicit_markers;
  explicit_markers = false;
  featname="LanguageModel";
  mapfile = "";
  filename = param;
  cerr<<"in parsing para ..."<<endl;
  /*
  if (!ParseLMArgs(param, &filename, &mapfile, &explicit_markers, &featname)) {
    abort();
  }
  */
  try {
	cerr<<"new klanguage model ..."<<endl;
    pimpl_ = new KLanguageModelImpl<Model>(filename, mapfile, explicit_markers, lm_vocab);
	cerr<<"after klanguage model ..."<<endl;
  } catch (std::exception &e) {
    std::cerr << e.what() << std::endl;
    abort();
  }
//  SetStateSize(pimpl_->ReserveStateSize());
}

/*
template <class Model>
Features KLanguageModel<Model>::features() const {
  return single_feature(fid_);
}
*/

template <class Model>
KLanguageModel<Model>::~KLanguageModel() {
  delete pimpl_;
}

template <class Model>
double KLanguageModel<Model>::TraversalFeaturesImpl(s_SearchSpace* edge) {
  double oovs = 0;
  return pimpl_->LookupWords(edge, &oovs);
}

template <class Model>
double KLanguageModel<Model>::FinalTraversalFeatures(s_SearchSpace* edge) {
  double oovs = 0;
  double lm = pimpl_->FinalTraversalCost(edge, &oovs);
  return lm;
}

//template <class Model> boost::shared_ptr<FeatureFunction> CreateModel(const std::string &param) {
//template <class Model> KLanguageModel<Model>* CreateModel(const std::string &param) {
//template <class Model> void CreateModel(const std::string &param, LanguageModel* plm, Vocab* lm_vocab) {
template <class Model> LanguageModel* CreateModel(const std::string &param, Vocab* lm_vocab) {
  //KLanguageModel<Model> *ret = new KLanguageModel<Model>(param);
  cerr<<"before loading language model!\n";
  //plm = new KLanguageModel<Model>(param, lm_vocab);
  KLanguageModel<Model>* plm = new KLanguageModel<Model>(param, lm_vocab);
  cerr<<"after loading language model!\n";
  plm->Init();
  //return boost::shared_ptr<FeatureFunction>(ret);
  return (LanguageModel*)plm;
}

//boost::shared_ptr<FeatureFunction> KLanguageModelFactory::Create(std::string param) const {
//template <class Model> KLanguageModel<Model>* KLanguageModelFactory::Create(std::string &param) const {
LanguageModel* KLanguageModelFactory::Create(std::string &param, Vocab* lm_vocab) const {
  using namespace lm::ngram;
  std::string filename, ignored_map;
  bool ignored_markers;
  std::string ignored_featname;
  ParseLMArgs(param, &filename, &ignored_map, &ignored_markers, &ignored_featname);
  ModelType m;
  if (!RecognizeBinary(filename.c_str(), m)) m = HASH_PROBING;

  switch (m) {
    case HASH_PROBING:
		cerr<<"HASH_PROBING\n";
		//return CreateModel<ProbingModel>(param,plm, lm_vocab);
		return CreateModel<ProbingModel>(param, lm_vocab);
    case TRIE_SORTED:
		cerr<<"TRIE_SORTED\n";
		//return CreateModel<TrieModel>(param,plm, lm_vocab);
		return CreateModel<TrieModel>(param, lm_vocab);
	case ARRAY_TRIE_SORTED:
		cerr<<"ARRAY_TRIE_SORTED\n";
		//return CreateModel<ArrayTrieModel>(param, plm,lm_vocab);
		return CreateModel<ArrayTrieModel>(param,lm_vocab);
	case QUANT_TRIE_SORTED:
		cerr<<"QUANT_TRIE_SORTED\n";
		//return CreateModel<QuantTrieModel>(param,plm,lm_vocab);
		return CreateModel<QuantTrieModel>(param,lm_vocab);
    case QUANT_ARRAY_TRIE_SORTED:
		cerr<<"QUANT_ARRAY_TRIE_SORTED\n";
		//return CreateModel<QuantArrayTrieModel>(param,plm,lm_vocab);
		return CreateModel<QuantArrayTrieModel>(param,lm_vocab);
    default:
		cerr<<"ERROR HERE\n";
		UTIL_THROW(util::Exception, "Unrecognized kenlm binary file type " << (unsigned)m);
  }
}

std::string  KLanguageModelFactory::usage(bool params,bool verbose) const {
  return KLanguageModel<lm::ngram::Model>::usage(params, verbose);
}

