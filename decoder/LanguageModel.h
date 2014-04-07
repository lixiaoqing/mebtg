#ifndef LANGUAGEMODEL_H
#define LANGUAGEMODEL_H

#include <iostream>
#include <string>
#include "SearchSpaceStock.h"

class LanguageModel {
	public:
		LanguageModel() {}
		~LanguageModel() {}
		virtual double FinalTraversalFeatures(s_SearchSpace* edge) {}
		virtual double TraversalFeaturesImpl(s_SearchSpace* edge) {}
		virtual void Init() const {}
};

#endif
