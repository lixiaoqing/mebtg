#include "lm.h"

void LanguageModel::load_lm(const string &lm_file)
{
	ifstream fin(lm_file.c_str(),ios::binary);
	if (!fin.is_open())
	{
		cerr<<"cannot open language model file!\n";
		return;
	}

	int max_order;
	fin.read((char*)&max_order,sizeof(int));
	vector<int> num_for_each_ngram;
	num_for_each_ngram.resize(max_order);
	fin.read((char*)&num_for_each_ngram[0],sizeof(int)*max_order);
	for(size_t i=0;i<max_order;i++)
	{
		for(size_t j=0;j<num_for_each_ngram.at(i);j++)
		{
			double prob,bow;
			size_t order= i+1;
			vector<int> wids;
			wids.resize(order);
			fin.read((char*)&prob,sizeof(double));
			fin.read((char*)&wids[0],sizeof(int)*(order));
			reverse(wids.begin(),wids.end());
			//append VocabNone to the end of wids??
			if(order != max_order)
			{
				fin.read((char*)&bow,sizeof(double));
			}
			//if(order<max_order && bow != 0.0)
			{
				add_bow_to_trie(wids,bow);
			}
			add_prob_to_trie(wids,prob);
		}
	}
	cout<<"load lm model file "<<lm_file<<" over\n";
}

NgramTrieNode* LanguageModel::search_or_create_path_in_trie(const vector<int> &wids)
{
	NgramTrieNode* cur_node = root;
	for (const auto &word_id : wids)
	{        
		auto it = cur_node->id2chilren_map.find(word_id);
		if ( it != cur_node->id2chilren_map.end() )
		{
			cur_node = it->second;
		}
		else
		{
			NgramTrieNode* tmp = new NgramTrieNode();
			cur_node->id2chilren_map.insert(make_pair(word_id,tmp));
			cur_node = tmp;
		}
	}
	return cur_node;
}

void LanguageModel::add_bow_to_trie(const vector<int> &wids,double bow)
{
	NgramTrieNode* node=search_or_create_path_in_trie(wids);
	node->bow=bow;
}

void LanguageModel::add_prob_to_trie(const vector<int> &wids,double prob)
{
	vector<int> history(wids.begin()+1,wids.end());
	NgramTrieNode* node=search_or_create_path_in_trie(history);
	node->probs.insert(make_pair(wids.at(0),prob));
}

double LanguageModel::eval(const vector<int> &wids)
{
	int len = wids.size();
	double sen_lm_score = 0;
	NgramTrieNode* cur_node;

	for( int i=0; i<len; i++ )
	{
		int pos = i;
		double prob = LogP_PseudoZero;
		double bow = 0;

		cur_node = root;
		while(1)
		{
			//查找语言模型的概率
			auto it=cur_node->probs.find(wids.at(i));
			if( cur_node->probs.end() != it )
			{
				prob = it->second;
				bow = 0;
			}
			//如果已经达到语言模型的次数，或者到达了句子的开始位置，则退出
			if( i-pos >= LM_ORDER-1 || pos < 1 )
			{
				break;
			}

			//查找Backoff 值
			auto it_child = cur_node->id2chilren_map.find( wids.at(pos-1) );
			if( it_child != cur_node->id2chilren_map.end())
			{
				cur_node = it_child->second;;
				bow += cur_node->bow;
				pos--;
			}
			else
			{
				break;
			}
		}

		sen_lm_score += prob+bow;
	}

	return sen_lm_score;
}
