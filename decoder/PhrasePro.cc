/*
Phrasepro.cpp
*/

#include "PhrasePro.h"

////////////////////////////////////////////////////////////////////////
// 1.主要功能：读取词对齐信息
// 2.入口参数：文件指针
// 3.出口参数：读文件是否成功
// 4.生成日期：2014.03.28
// 5.算法简介：
bool
PhrasePro::ReadAlignment(const char* AlignmentFileName)
{
	cout<<"reading alignment file "<<AlignmentFileName<<endl;
	ifstream fin;
	fin.open(AlignmentFileName);
	if (!fin.is_open())
	{
		cerr<<"Fail to open alignment file!\n";
		return 0;
	}
	string line;
	int num = 0;
	while(getline(fin,line))
	{
		vector <string> elements;
		string sep = "|||";
		Split(elements,line,sep);
		TrimLine(elements[0]);
		TrimLine(elements[1]);
		TrimLine(elements[3]);
		string ch_phrase = elements[0];
		string en_phrase = elements[1];
		vector <string> ch_word_vector;
		Split(ch_word_vector,ch_phrase);
		vector <string> en_word_vector;
		Split(en_word_vector,en_phrase);
		vector<vector<int> > ch_pos_to_en_pos_list(ch_word_vector.size());
		vector <string> alignments;
		Split(alignments,elements[3]);
		vector<string>::iterator iter;
		for (iter = alignments.begin(); iter != alignments.end();iter++)
		{
			vector <string> pos_pair;
			sep = "-";
			Split(pos_pair,*iter,sep);
			int ch_pos = s2i(pos_pair[0]);
			int en_pos = s2i(pos_pair[1]);
			ch_pos_to_en_pos_list[ch_pos].push_back(en_pos);
		}

		int *ch_index_vector = new int[ch_word_vector.size()+1];
		for (size_t i=0;i<ch_word_vector.size();i++)
		{
			ch_index_vector[i] = _ChiVocab->GetIndex(ch_word_vector[i].c_str());
		}
		ch_index_vector[ch_word_vector.size()]=Vocab_None;
		vector<int> en_index_vector(en_word_vector.size()+1);
		for (size_t i=0;i<en_word_vector.size();i++)
		{
			en_index_vector[i] = _EngVocab->GetIndex(en_word_vector[i].c_str());
		}
		en_index_vector[en_word_vector.size()]=Vocab_None;

		PhraseProTrie* m_phraseTrie;
		m_phraseTrie = contexts.findTrie(ch_index_vector);
		for (size_t i=0;i<m_phraseTrie->value().size();i++)
		{
			if (m_phraseTrie->value().at(i).viEnPhrase == en_index_vector)
			{
				num++;
				/*
				for (size_t j=0;j<ch_word_vector.size();j++)
					cout<<ch_word_vector.at(j)<<endl;
				for (size_t j=0;j<en_word_vector.size();j++)
					cout<<en_word_vector.at(j)<<endl;
				*/
				m_phraseTrie->value().at(i).ch_pos_to_en_pos_list = ch_pos_to_en_pos_list;
			}
		}
	}
	cout<<"added alignment infor items: "<<num<<endl;
	return true;
}


////////////////////////////////////////////////////////////////////////
// 1.主要功能：读取翻译概率参数文件
// 2.入口参数：文件指针
// 3.出口参数：读文件是否成功
// 4.生成日期：2006.04.10
// 5.算法简介：
bool
PhrasePro::ReadFile(const char* PhraseProFileName, bool ReduceVoc, SegFormate segmenttype)
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
	    int *c_ch_index_vector=new int[c_ch_word_counts+1];
	    fread(c_ch_index_vector,sizeof(int),c_ch_word_counts,m_pfPhraseProFile);
            c_ch_index_vector[c_ch_word_counts]=Vocab_None;
	    short int c_en_word_counts=0;
	    fread(&c_en_word_counts,sizeof(short int),1,m_pfPhraseProFile);
            //cout<<c_en_word_counts<<endl;
	    int *c_en_index_vector=new int[c_en_word_counts+1];
	    fread(c_en_index_vector,sizeof(int),c_en_word_counts,m_pfPhraseProFile);
            c_en_index_vector[c_en_word_counts]=Vocab_None;
	    double c_trans_prob[4];
	    fread(c_trans_prob,sizeof(double),4,m_pfPhraseProFile);
            //for(size_t i=0;i<4;i++)
             //   cout<<c_trans_prob[i]<<endl;
	     //从短语表中读取词对齐信息，由李小青2014年4月16添加
	    short int c_alignment_num=0;
	    fread(&c_alignment_num,sizeof(short int),1,m_pfPhraseProFile);
	    int *c_alignment_vector=new int[c_alignment_num];
	    fread(c_alignment_vector,sizeof(int),c_alignment_num,m_pfPhraseProFile);

	    s_PhrasePro m_PhraseTemp;
            m_PhraseTemp.ulEnNum=c_en_word_counts;
            if(m_PhraseTemp.ulEnNum > PHRASE_LEN_MAX)
                continue;

	    //将对齐信息添加到短语候选中去
	    m_PhraseTemp.ch_pos_to_en_pos_list.resize(c_ch_word_counts);
            for(size_t i=0;i<c_alignment_num/2;i++)
	    {
		    int ch_pos = c_alignment_vector[2*i];
		    int en_pos = c_alignment_vector[2*i+1];
		    m_PhraseTemp.ch_pos_to_en_pos_list[ch_pos].push_back(en_pos);
	    }

            for(size_t i=0;i<m_PhraseTemp.ulEnNum+1;i++)
                m_PhraseTemp.viEnPhrase.push_back(c_en_index_vector[i]);

	    for(size_t i=0;i<4;i++)	
		if( c_trans_prob[i] == 0.0 )
		{
		    m_PhraseTemp.eachTransPro.push_back( LogP_PseudoZero );
		}
		else
		{
		    m_PhraseTemp.eachTransPro.push_back( log10(c_trans_prob[i]) );
		}

            //cout<<"here 1"<<endl;
	    m_PhraseTemp.dPro = 0;
	    if( m_PhraseTemp.eachTransPro.size() != _lambda.trans.size() )
	    {
		cout<<"the "<<" phrase pairs probability is error!"<<endl;
	    }
	    for( int i=0; i<(int)_lambda.trans.size(); i++ )
	    {
		m_PhraseTemp.dPro += m_PhraseTemp.eachTransPro[i]*_lambda.trans[i];
	    }
            //cout<<"here 2"<<endl;
            
            //for(size_t i=0;i<Vocab::length(c_ch_index_vector);i++)
             //   log<<c_ch_index_vector[i]<<" ";
            //log<<endl;
            //for(size_t j=0;j<m_PhraseTemp.viEnPhrase.size();j++)
             //   log<<m_PhraseTemp.viEnPhrase[j]<<" ";
            //for(size_t k=0;k<m_PhraseTemp.eachTransPro.size();k++)
             //   log<<m_PhraseTemp.eachTransPro[k]<<" ";
            //log<<endl;
	    //插入对应的短语翻译
	    PhraseProTrie* m_phraseTrie;
            //cout<<"point here!"<<endl;
	    m_phraseTrie = contexts.findTrie(c_ch_index_vector);
            //for(size_t i=0;i<c_ch_word_counts;i++)
             //   cout<<c_ch_index_vector[i]<<" ";
            //cout<<endl;
            /*if(m_phraseTrie)
                cout<<"m_phraseTrie is not NULL!"<<endl;
            else
            {
                for(size_t i=0;i<c_ch_word_counts;i++)
                    cout<<c_ch_index_vector[i]<<" ";
                cout<<endl;
            }*/
	    if( (int)m_phraseTrie->value().size() < trans_table_size )
		m_phraseTrie->value().push_back( m_PhraseTemp );
	    else
	    {
		vector<s_PhrasePro>::iterator iter = min_element(m_phraseTrie->value().begin(), m_phraseTrie->value().end());
		if( *iter < m_PhraseTemp )
		    (*iter) = m_PhraseTemp;
	    }
            //cout<<"here 3"<<endl;

	    //contexts.findTrie(PhraseChiIndex)->value().push_back(m_PhraseTemp);	
	    delete c_ch_index_vector;
	    delete c_en_index_vector;
	}

	
	fclose(m_pfPhraseProFile);

	return true;
}


////////////////////////////////////////////////////////////////////////
// 1.主要功能：将短语翻译概率中的短语切分出来
// 2.入口参数：短语翻译概率的一行；短语单词；最大长度
// 3.出口参数：短语的长度
// 4.生成日期：2006.04.10
// 5.算法简介：按照短语之间的分隔符进行分割
int 
PhrasePro::ParsePhrase(char** words, char** phrase, int max)
{
	int len=0,i=0;
	while( strcmp(words[i],g_pcPhraseSeparators) && i<=max )
	{
		phrase[i] = words[i];
		i++;
	}
	if(i<=max)
	{
		phrase[i] = 0;
	}
	else
	{
		phrase[max] = 0;
	}
	return i;
}



