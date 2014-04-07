/*
Ngram.cpp
读文件的部分主要源于SRILM软件包
*/

#include "Ngram.h"


bool
Ngram::ReadFile(const char* NgramFileName, bool ReduceVoc)
{
        //ofstream log("lm_log.bin");
	FILE *NgramFile;
	NgramFile = fopen(NgramFileName,"rb");
	if( !NgramFile )
	{
		cerr<<"Can not open the Language File: "<<NgramFileName<<endl;
		return 0;
	}
	cout<<"Reading Ngram File..."<<endl;

	int m_MaxOrder = 0;						 // maximal n-gram order in this model 
	int numNgrams[MAX_NGRAM_ORDER + 1];        // the number of n-grams for each order 

	for (int i = 0; i <= MAX_NGRAM_ORDER; i++) 
	{
		numNgrams[i] = 0;
	}

	fread(&m_MaxOrder,sizeof(int),1,NgramFile);
	if(m_MaxOrder<=0 || m_MaxOrder>MAX_NGRAM_ORDER)
	{
	    cerr<<"reading the lm order wrong!"<<endl;
	    return false;
	}
	fread(numNgrams,sizeof(int),m_MaxOrder,NgramFile);
	for(size_t i=0;i<m_MaxOrder;i++)
	{
	    for(size_t k=0;k<numNgrams[i];k++)
	    {
		double c_prob=0;
		double c_back=0;
	
		int* c_index_vector=new int[m_MaxOrder+1];
		for(size_t j=0;j<m_MaxOrder+1;j++)
		    c_index_vector[j]=0;	    

		fread(&c_prob,sizeof(double),1,NgramFile);
		fread(c_index_vector,sizeof(int),i+1,NgramFile);
                c_index_vector[i+1]=Vocab_None;
		if(i+1 != m_MaxOrder)
		    fread(&c_back,sizeof(double),1,NgramFile);

		Vocab::reverse(c_index_vector);
		if(i+1 < m_MaxOrder && c_back != 0.0) 
		{
		    InsertBOW(c_index_vector,c_back);
		}
                
                //for(size_t i=0;i<Vocab::length(c_index_vector);i++)
                 //   log<<c_index_vector[i]<<" ";
                //log<<c_prob<<" "<<c_back<<endl;
		//Insert Prob
		NgramTrie *m_NgramTrie = _ngramcontext.findTrie(&c_index_vector[1]);
		if( !m_NgramTrie )
		{
		    cerr<<"warning: no bow for prefix of ngram \""
			<< &c_index_vector[1] << "\"\n"; 
		}
		else
		{
                    //cout<<"insert"<<endl;
		    m_NgramTrie->value().probs.insert(map<int, double>::value_type(c_index_vector[0],c_prob) );
		}
		delete c_index_vector;
	    }
	}

	//cout<<reducenum<<endl;
	return true;
}


bool
Ngram::ParseLogP(const char *str, double &result)
{
	const unsigned maxDigits = 8;	// number of decimals in an integer

	const char *cp = str;
	const char *cp0;
	bool minus = false;

	/*
	* Log probabilties are typically negative values of magnitude > 0.0001,
	* and thus are usually formatted without exponential notation.
	* We parse this type of format using integer arithmetic for speed,
	* and fall back onto scanf() in all other cases.
	* We also use scanf() when there are too many digits to handle with
	* integers.
	* Finally, we also parse +/- infinity values as they are printed by 
	* printf().  These are "[Ii]nf" or "[Ii]nfinity".
	*/

	/*
	* Parse optional sign
	*/
	if (*cp == '-') {
		minus = true;
		cp++;
	} else if (*cp == '+') {
		cp++;
	}
	cp0 = cp;

	unsigned digits = 0;		// total value of parsed digits
	unsigned decimals = 1;		// scaling factor from decimal point
	unsigned precision = 0;		// total number of parsed digits

	/*
	* Parse digits before decimal point
	*/
	while (isdigit(*cp)) {
		digits = digits * 10 + (*(cp++) - '0');
		precision ++;
	}

	if (*cp == '.') {
		cp++;

		/*
		* Parse digits after decimal point
		*/
		while (isdigit(*cp)) {
			digits = digits * 10 + (*(cp++) - '0');
			precision ++;
			decimals *= 10;
		}
	}

	/*
	* If we're at the end of the string then we're done.
	* Otherwise there was either an error or some format we can't
	* handle, so fall back on scanf(), after checking for infinity
	* values.
	*/
	if (*cp == '\0' && precision <= maxDigits) {
		result = (minus ? - (double)digits : (double)digits) / (double)decimals;
		return true;
	} else if ((*cp0 == 'i' || *cp0 == 'I') &&
		(strncmp(cp0, "Inf", 3) == 0 || strncmp(cp0, "inf", 3) == 0))
	{
		result = (minus ? LogP_Zero : LogP_Inf);
		return true;
	} else {
		return (sscanf(str, "%lf", &result) == 1);
	}
}
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
// 1.主要功能：计算一个句子的语言模型概率，从0开始，如果是句首，0位置为开始标志符，最后一个为“Vocab_None”标志，用于计算句子长度
// 2.入口参数：词号表示的句子（顺序表示，不用逆转）；开始的位置，即从几元模型开始计算
// 3.出口参数：语言模型的概率
// 4.生成日期：2004.04.10
// 5.算法简介：
//
//

double 
Ngram::ReadSentenceNgram(int *wids,int pos)
{
	int len = Vocab::length( wids );
	double m_SumPro = 0;
	NgramTrie* m_Trie;
	NgramTrie* m_next;
	
	for( int i=pos-1; i<len; i++ )
	{
		int m_pos = i;
		int m_word = wids[i];
		double m_WordPro = LogP_Zero;
		double m_dPro = LogP_Zero;
		double m_dBow = LogP_One;

		m_Trie = &_ngramcontext;
		do
		{
			//查找语言模型的概率
			map<int,double>::iterator it=m_Trie->value().probs.find(m_word);
			if( m_Trie->value().probs.end() != it )
			{
				m_dPro = (*it).second;
				m_dBow = LogP_One;
			}
// 			else if( m_pos == i )  //unknown word
// 			{
// 				;
// 			}

			//如果已经达到语言模型的次数，或者到达了句子的开始位置，则退出
			if( i-m_pos >= _ulOrder-1 || m_pos < 1 )
			{
				break;
			}
			
			//查找Backoff 值

			m_next = m_Trie->findTrie( wids[m_pos-1] );
			if( m_next )
			{
				m_dBow += m_next->value().bow;
				double m_d = m_next->value().bow;
				m_Trie = m_next;
				m_pos--;
			}
			else
			{
				break;
			}

		}while(1);

		m_WordPro = m_dPro+m_dBow;
		if(  m_WordPro != LogP_Zero )
		{
			m_SumPro += m_WordPro;
		}
	}

	return m_SumPro;
}


double 
Ngram::ReadSentenceNgram(vector<int> wids,int pos)
{
	int len = wids.size();
	double m_SumPro = 0;
	NgramTrie* m_Trie;
	NgramTrie* m_next;

	for( int i=pos-1; i<len; i++ )
	{
		int m_pos = i;
		int m_word = wids[i];
		double m_WordPro = LogP_Zero;
		double m_dPro = LogP_Zero;
		double m_dBow = LogP_One;

		m_Trie = &_ngramcontext;
		do
		{
			//查找语言模型的概率
			map<int,double>::iterator it=m_Trie->value().probs.find(m_word);
			if( m_Trie->value().probs.end() != it )
			{
				m_dPro = (*it).second;
				m_dBow = LogP_One;
			}
			// 			else if( m_pos == i )  //unknown word
			// 			{
			// 				;
			// 			}

			//如果已经达到语言模型的次数，或者到达了句子的开始位置，则退出
			if( i-m_pos >= _ulOrder-1 || m_pos < 1 )
			{
				break;
			}

			//查找Backoff 值

			m_next = m_Trie->findTrie( wids[m_pos-1] );
			if( m_next )
			{
				m_dBow += m_next->value().bow;
				double m_d = m_next->value().bow;
				m_Trie = m_next;
				m_pos--;
			}
			else
			{
				break;
			}

		}while(1);

		m_WordPro = m_dPro+m_dBow;
		if(  m_WordPro != LogP_Zero )
		{
			m_SumPro += m_WordPro;
		}
	}

	return m_SumPro;
}

double Ngram::FinalTraversalFeatures(s_SearchSpace* edge) {

	vector<int> wids = edge->viEnPhrase;
	int en_size = wids.size();
	//get the boundary of the translation
	vector<int> left_boundary_wids, right_boundary_wids;
	int boundary_size = _ulOrder-1<en_size?_ulOrder:en_size;
	for( int k = 0; k < boundary_size; k++ ) {
		left_boundary_wids.push_back(wids[k]);
	}
	for( int k = en_size-boundary_size; k < en_size; k++ ) {
		right_boundary_wids.push_back(wids[k]);
	}
	//record the senence begin marker and sentence end marker
	vector<int> sen_begin_vec, sen_end_vec;
	sen_begin_vec.push_back(_vocab->SenStart());
	sen_begin_vec.insert(sen_begin_vec.end(), left_boundary_wids.begin(), left_boundary_wids.end());
	sen_end_vec = right_boundary_wids;
	sen_end_vec.push_back(_vocab->SenEnd());
	return this->ReadSentenceNgram(sen_begin_vec,1)-this->ReadSentenceNgram(left_boundary_wids,1)+this->ReadSentenceNgram(sen_end_vec, sen_end_vec.size());
}

double Ngram::TraversalFeaturesImpl(s_SearchSpace* edge) {
	if( edge->m_split_point >= 0 ) { //using merging rules
		vector<int> left_wids = (edge->left_ant)->viEnPhrase;
		vector<int> right_wids = (edge->right_ant)->viEnPhrase;
		return ComputeMergedLM(left_wids, right_wids);
	}
	else { //using phrasal rule
		return this->ReadSentenceNgram(edge->viEnPhrase,1);
	}
}

double Ngram::ComputeMergedLM(vector<int> &left_wids, vector<int> &right_wids) {
	vector<int> wids = left_wids;
	wids.insert(wids.end(), right_wids.begin(), right_wids.end());
	vector<int> left_right_wids, right_left_wids, straight_mid_wids;
	int left_size = left_wids.size();
	int right_size = right_wids.size();
	int n1 = _ulOrder-1<left_size?_ulOrder-1:left_size;	
	int n2 = _ulOrder-1<right_size?_ulOrder-1:right_size;
	for( int k = left_size-n1; k < left_size; k++ ) {
		left_right_wids.push_back(left_wids[k]);
	}
	straight_mid_wids = left_right_wids;
	for( int k = 0; k < n2; k++ ) {
		right_left_wids.push_back(right_wids[k]);
		straight_mid_wids.push_back(right_wids[k]);
	}
	return this->ReadSentenceNgram(straight_mid_wids,1)-this->ReadSentenceNgram(left_right_wids,1)-this->ReadSentenceNgram(right_left_wids,1);
}

void Ngram::Init() const {
	cerr<<"SRILM OK!\n";
}
