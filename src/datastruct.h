#include "stdafx.h"


struct Cand	                
{
	bool operator< (const Cand &cand_rhs) const
	{
		return (score < cand_rhs.score);
	}

	int first_src_word_pos;
	int last_src_word_pos;
	int tgt_word_num;
	vector<int> tgt_word_id_list;
	string tgt_word_str;
	int phrase_num;
	double score;
	vector<double> trans_prob_list;
	double lm_prob;
	double straight_reorder_prob;
	double swap_reorder_prob;
	double context_based_trans_prob;
	//double dCN;
	//double dCLM;
	//double m_syn_reorder_prob;
	//vector<Cand>  m_recombine_array;

	//用于记录该假设由孩子节点的第几个候选翻译合并而来,09.06.06添加
	int split_pos;
	int left_ith_best;
	int right_jth_best;

	//BoundaryAnnotatedState cur_state;
	Cand* left_ant;
	Cand* right_ant;
};

struct Config
{
	string lmfilename;						//language model probability file
	string phrasefilename;					//phrase translation probability file
	string testfilename;					//test file 
	string resultfilename;					//result file
	string nbestfilename;					//n_best result file
	string sourcelmfilename;                //source language model
	//调序模型文件由张家俊09年1月5日添加
	string reorder_model_filename;
	//基于上下文的翻译模型文件由李小青于14年3月29日添加,此处为记录所有模型文件名的目录文件
	string catalog_filename;

	string candsfilename;
	string featsfilename;
	string initfile;

        string sourcevocabfilename;
        string targetvocabfilename;


	int beam_hist;				//beam size threshold
	double beam_prob;                          //probability threshold of beam search

	int ttable;				//translation table size
	int nbest;				//nbest number
	int lmorder;              //language model order

	int reorder_window;       //window size of reordering
	int cube_flag;            //whether decode using cube prunniing


	double Lambda_lm;					//weight of language model 
	double Lambda_sense;					//weight of context based translation model
	double Lambda_reorderStraight;					//weight of reordering model 
	double Lambda_reorderSwap;					//weight of reordering model 
	vector<double> Lambda_trans;         //all the weights of phrase pairs
	double Lambda_len;					//weight of length
	double Lambda_dPhraseNum;
	bool recombine;						//whether recombine or not

	bool KenLM;

	bool reduceVoc;						//reduce the phrase and LM table depending on the testfile
	bool train;							//train for parameters

	int feats_num;
	bool printInfo;
	bool printNbest;

	int threadsNum;


	SegFormate segmenttype;
	FileFormate filetype;
	UnknownWord transUnknownWord;

	//语言对选项，09年06月23日添加
	int chn2eng_flag;          //表示中文到英文的翻译
	string g_Chinese_Punc_Set;  //不进行调序的中文标点约束
	string g_Chinese_Split_Puncs; //控制子句分割的中文标点约束

};


struct Weight
{
	vector<double> trans;
	double lm;
	double sense;
	double len;
	double furt;
	double reorder_straight;
	double reorder_swap;
	double phrase_num;
};

