#include "HashPhraseTable.h"

bool HashPhraseTable::ReadFile(const char* PhraseProFileName, bool ReduceVoc, SegFormate segmenttype)
{
        //ofstream log("log.bin");
	FILE* m_pfPhraseProFile = NULL;

	m_pfPhraseProFile = fopen(PhraseProFileName,"rb");
	if( !m_pfPhraseProFile )
	{
		cerr<<"Can not open the phrase probability file!\n";
		return 0;
	}

	cout<<"Reading Phrase Probability File:"<<PhraseProFileName<<endl;

	short int c_ch_word_counts=0;
	while( fread(&c_ch_word_counts,sizeof(short int),1,m_pfPhraseProFile ))
	{
            //cout<<c_ch_word_counts<<endl;
	    int *c_ch_index_array=new int[c_ch_word_counts+1];
	    fread(c_ch_index_array,sizeof(int),c_ch_word_counts,m_pfPhraseProFile);
            c_ch_index_array[c_ch_word_counts]=Vocab_None;
	    short int c_en_word_counts=0;
	    fread(&c_en_word_counts,sizeof(short int),1,m_pfPhraseProFile);
            //cout<<c_en_word_counts<<endl;
	    int *c_en_index_array=new int[c_en_word_counts+1];
	    fread(c_en_index_array,sizeof(int),c_en_word_counts,m_pfPhraseProFile);
            c_en_index_array[c_en_word_counts]=Vocab_None;
	    double c_trans_prob[4];
	    fread(c_trans_prob,sizeof(double),4,m_pfPhraseProFile);
            //for(size_t i=0;i<4;i++)
             //   cout<<c_trans_prob[i]<<endl;
	     //从短语表中读取词对齐信息，由李小青2014年4月16添加
	    short int c_alignment_num=0;
	    fread(&c_alignment_num,sizeof(short int),1,m_pfPhraseProFile);
	    int *c_alignment_array=new int[c_alignment_num];
	    fread(c_alignment_array,sizeof(int),c_alignment_num,m_pfPhraseProFile);

	    TgtPhrase tgt_phrase_tmp;
            tgt_phrase_tmp.ulEnNum=c_en_word_counts;
            if(tgt_phrase_tmp.ulEnNum > PHRASE_LEN_MAX)
                continue;

	    //将对齐信息添加到短语候选中去
	    tgt_phrase_tmp.ch_pos_to_en_pos_list.resize(c_ch_word_counts);
            for(size_t i=0;i<c_alignment_num/2;i++)
	    {
		    int ch_pos = c_alignment_array[2*i];
		    int en_pos = c_alignment_array[2*i+1];
		    tgt_phrase_tmp.ch_pos_to_en_pos_list[ch_pos].push_back(en_pos);
	    }

            for(size_t i=0;i<tgt_phrase_tmp.ulEnNum+1;i++)
                tgt_phrase_tmp.viEnPhrase.push_back(c_en_index_array[i]);

	    for(size_t i=0;i<4;i++)	
		if( c_trans_prob[i] == 0.0 )
		{
		    tgt_phrase_tmp.eachTransPro.push_back( LogP_PseudoZero );
		}
		else
		{
		    tgt_phrase_tmp.eachTransPro.push_back( log10(c_trans_prob[i]) );
		}

            //cout<<"here 1"<<endl;
	    tgt_phrase_tmp.dPro = 0;
	    if( tgt_phrase_tmp.eachTransPro.size() != _lambda.trans.size() )
	    {
		cout<<"the "<<" phrase pairs probability is error!"<<endl;
	    }
	    for( int i=0; i<(int)_lambda.trans.size(); i++ )
	    {
		tgt_phrase_tmp.dPro += tgt_phrase_tmp.eachTransPro[i]*_lambda.trans[i];
	    }
	    //插入对应的短语翻译
	    vector<int> src_phrase_tmp(c_ch_index_array,c_ch_index_array+c_ch_word_counts);
	    unordered_map<vector<int>,vector<TgtPhrase>,vechash >::iterator it = phrase_table.find(src_phrase_tmp);
	    if (it != phrase_table.end())
	    {
		    if( it->second.size() < trans_table_size )
		    {
			    it->second.push_back(tgt_phrase_tmp);
		    }
		    else
		    {
			    vector<TgtPhrase>::iterator iter = min_element(it->second.begin(), it->second.end());
			    if( *iter < tgt_phrase_tmp )
				    (*iter) = tgt_phrase_tmp;
		    }
	    }
	    else
	    {
		    vector<TgtPhrase> tgt_phrase_list_tmp;
		    tgt_phrase_list_tmp.push_back(tgt_phrase_tmp);
		    phrase_table.insert(make_pair(src_phrase_tmp,tgt_phrase_list_tmp));
	    }

	    delete c_ch_index_array;
	    delete c_en_index_array;
	    delete c_alignment_array;
	}

	
	fclose(m_pfPhraseProFile);

	return true;
}

vector<TgtPhrase>* HashPhraseTable::find_src_phrase(vector<int>& src_phrase_idx)
{
	unordered_map<vector<int>,vector<TgtPhrase>,vechash >::iterator it = phrase_table.find(src_phrase_idx);
	if (it != phrase_table.end())
		return &(it->second);
	else
		return nullptr;
}

