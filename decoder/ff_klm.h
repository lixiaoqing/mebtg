#ifndef _KLM_FF_H_
#define _KLM_FF_H_

#include <vector>
#include <string>

#include "SearchSpaceStock.h"
#include "LanguageModel.h"
#include "Vocab.h"

template <class Model> struct KLanguageModelImpl;

// the supported template types are instantiated explicitly
// in ff_klm.cc.
template <class Model>
class KLanguageModel : public LanguageModel {
 public:
  // param = "filename.lm [-o n]"
  KLanguageModel(const std::string& param, Vocab* lm_vocab);
  ~KLanguageModel();
  virtual double FinalTraversalFeatures(s_SearchSpace* edge) ;
  static std::string usage(bool param,bool verbose);
  //Features features() const;
  void Init() {
	  cerr<<"KLanguage Model OK!\n";
  }
 protected:
  /*
  virtual void TraversalFeaturesImpl(const SentenceMetadata& smeta,
                                     const Hypergraph::Edge& edge,
                                     const std::vector<const void*>& ant_contexts,
                                     SparseVector<double>* features,
                                     SparseVector<double>* estimated_features,
                                     void* out_context) const;
									 */
  virtual double TraversalFeaturesImpl(s_SearchSpace* edge) ;
 private:
  KLanguageModelImpl<Model>* pimpl_;
};

//struct KLanguageModelFactory : public FactoryBase<FeatureFunction> {
class KLanguageModelFactory {
	public:
		//FP Create(std::string param) const;
		//template <class Model> KLanguageModel<Model>* Create(std::string &param) const;
		//void Create(std::string &param, LanguageModel* plm, Vocab* lm_vocab) const;
		LanguageModel* Create(std::string &param, Vocab* lm_vocab) const;
		std::string usage(bool params,bool verbose) const;
};

#endif
