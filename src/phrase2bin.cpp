#include "myutils.h"

void phrase2bin(string phrase_filename,string mode)
{
	unordered_map <string,int> ch_vocab;
	unordered_map <string,int> en_vocab;
	vector<string> ch_vocab_vec;
	vector<string> en_vocab_vec;
	int ch_word_id = 0;
	int en_word_id = 0;
	ifstream fin(phrase_filename.c_str());
	if (!fin.is_open())
	{
		cout<<"fail to open "<<phrase_filename<<endl;
		return;
	}
	ofstream fout;
	if (mode == "0")
		fout.open("prob.bin-without-alignment",ios::binary);
	else
		fout.open("prob.bin-with-alignment",ios::binary);
	if (!fout.is_open())
	{
		cout<<"fail open model file to write!\n";
		return;
	}

	vector<int> ch_phrase_lens;
	vector<int> ch_wids;
	vector<int> en_phrase_lens;
	vector<int> en_wids;
	vector<double> probs;
	vector<int> alignment_nums;
	vector<int> src_alignments;
	vector<int> tgt_alignments;
	int phrase_num,ch_wids_len,en_wids_len,alignments_len;
	string line;
	while(getline(fin,line))
	{
		vector <string> elements;
		string sep = "|||";
		Split(elements,line,sep);
		for (auto &e : elements)
		{
			TrimLine(e);
		}
		vector <string> ch_words;
		Split(ch_words,elements[0]);
		ch_phrase_lens.push_back(ch_words.size());
		for (const auto &ch_word : ch_words)
		{
			auto it = ch_vocab.find(ch_word);
			if (it != ch_vocab.end())
			{
				ch_wids.push_back(it->second);
			}
			else
			{
				ch_wids.push_back(ch_word_id);
				ch_vocab.insert(make_pair(ch_word,ch_word_id));
				ch_vocab_vec.push_back(ch_word);
				ch_word_id++;
			}
		}

		vector <string> en_words;
		Split(en_words,elements[1]);
		en_phrase_lens.push_back(en_words.size());
		for (const auto &en_word : en_words)
		{
			auto it = en_vocab.find(en_word);
			if (it != en_vocab.end())
			{
				en_wids.push_back(it->second);
			}
			else
			{
				en_wids.push_back(en_word_id);
				en_vocab.insert(make_pair(en_word,en_word_id));
				en_vocab_vec.push_back(en_word);
				en_word_id++;
			}
		}

		vector <string> prob_strs;
		Split(prob_strs,elements[2]);
		for (const auto &prob_str : prob_strs)
		{
			probs.push_back(stod(prob_str));
		}

		vector <string> alignment_strs;
		sep = "-";
		if (mode == "1")
		{
			Split(alignment_strs,elements[3]);
			alignment_nums.push_back(alignment_strs.size());
			for (auto &align_str : alignment_strs)
			{
				vector <string> pos_pair;
				Split(pos_pair,align_str,sep);
				src_alignments.push_back(stoi(pos_pair[0]));
				tgt_alignments.push_back(stoi(pos_pair[1]));
			}
		}
	}
	phrase_num = ch_phrase_lens.size();
	ch_wids_len = ch_wids.size();
	en_wids_len = en_wids.size();
	fout.write((char*)&phrase_num,sizeof(int));
	fout.write((char*)&ch_wids_len,sizeof(int));
	fout.write((char*)&en_wids_len,sizeof(int));
	fout.write((char*)&ch_phrase_lens[0],sizeof(int)*phrase_num);
	fout.write((char*)&ch_wids[0],sizeof(int)*ch_wids_len);
	fout.write((char*)&en_phrase_lens[0],sizeof(int)*phrase_num);
	fout.write((char*)&en_wids[0],sizeof(int)*en_wids_len);
	fout.write((char*)&probs[0],sizeof(double)*phrase_num*4);
	if (mode == "1")
	{
		fout.write((char*)&alignment_nums[0],sizeof(int)*phrase_num);
		alignments_len = src_alignments.size();
		fout.write((char*)&alignments_len,sizeof(int));
		fout.write((char*)&src_alignments[0],sizeof(int)*alignments_len);
		fout.write((char*)&tgt_alignments[0],sizeof(int)*alignments_len);
	}
	fout.close();

	ofstream f_ch_vocab("vocab.ch");
	if (!f_ch_vocab.is_open())
	{
		cout<<"fail open ch vocab file to write!\n";
		return;
	}
	for(size_t i=0;i<ch_vocab_vec.size();i++)
	{
		f_ch_vocab<<ch_vocab_vec.at(i)+" "+to_string(i)+"\n";
	}
	f_ch_vocab.close();

	ofstream f_en_vocab("vocab.en");
	if (!f_en_vocab.is_open())
	{
		cout<<"fail open en vocab file to write!\n";
		return;
	}
	for(size_t i=0;i<en_vocab_vec.size();i++)
	{
		f_en_vocab<<en_vocab_vec.at(i)+" "+to_string(i)+"\n";
	}
	f_en_vocab.close();
}

int main(int argc,char* argv[])
{
    if(argc == 1)
    {
		cout<<"usage: ./phrase2bin phrase_filename mode\ndon't write alignment if mode==0, write if mode==1\n";
		return 0;
    }
    string phrase_filename(argv[1]);
	string mode(argv[2]);
    phrase2bin(phrase_filename,mode);
	return 0;
}
