#include <unordered_map>
#include <boost/functional/hash.hpp>
#include "StdAfx.h"
#include "Vocab.h"
#include "myutils.h"
using namespace boost;

struct TgtPhrase
{
	bool operator<(const TgtPhrase &right) const {return dPro < right.dPro;};
	TgtPhrase& operator =(const TgtPhrase &right)
	{
		ulEnNum = right.ulEnNum;
		viEnPhrase = right.viEnPhrase;
		dPro = right.dPro;
		eachTransPro = right.eachTransPro;
		ch_pos_to_en_pos_list = right.ch_pos_to_en_pos_list;
		return *this;
	};
	int ulEnNum;						//英文词的个数
	vector<int> viEnPhrase;
	double dPro;								//概率
	vector<double> eachTransPro;
	vector<vector<int> > ch_pos_to_en_pos_list;                //每个中文词对应的英文词位置,由李小青2014年3月31日添加
};

struct vechash 
{
	size_t operator()(const vector<int>& v) const
	{
		return hash_range(v.begin(),v.end());
	}
};

class HashPhraseTable
{
	public:
		HashPhraseTable(Vocab *chivocab, Vocab *engvocab, int ttable):_ChiVocab(chivocab),_EngVocab(engvocab),trans_table_size(ttable){};
		bool SetArgs(Lambda all_Lambda)
		{
			_lambda = all_Lambda;
			return 1;
		};
		bool ReadFile(const char* PhraseProFileName, bool ReduceVoc=false, SegFormate segmenttype=NOSEG);												//读取参数文件
		vector<TgtPhrase>* find_src_phrase(vector<int>& src_phrase_idx);


	protected:
		int trans_table_size;               //
		Vocab* _ChiVocab;					//中文词典
		Vocab* _EngVocab;					//英文词典
		unordered_map<vector<int>,vector<TgtPhrase>,vechash> phrase_table;				//翻译节点
		Lambda _lambda;
};
