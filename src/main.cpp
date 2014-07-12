#include "translator.h"

void read_config(Filenames &fns,Parameter &para, Weight &weight, const string &config_file)
{
	ifstream fin;
	fin.open(config_file.c_str());
	if (!fin.is_open())
	{
		cerr<<"fail to open config file\n";
		return;
	}
	string line;
	while(getline(fin,line))
	{
		TrimLine(line);
		if (line == "[input-file]")
		{
			getline(fin,line);
			fns.input_file = line;
		}
		else if (line == "[output-file]")
		{
			getline(fin,line);
			fns.output_file = line;
		}
		else if (line == "[nbest-file]")
		{
			getline(fin,line);
			fns.nbest_file = line;
		}
		else if (line == "[src-vocab-file]")
		{
			getline(fin,line);
			fns.src_vocab_file = line;
		}
		else if (line == "[tgt-vocab-file]")
		{
			getline(fin,line);
			fns.tgt_vocab_file = line;
		}
		else if (line == "[rule-table-file]")
		{
			getline(fin,line);
			fns.rule_table_file = line;
		}
		else if (line == "[lm-file]")
		{
			getline(fin,line);
			fns.lm_file = line;
		}
		else if (line == "[reorder-model-file]")
		{
			getline(fin,line);
			fns.reorder_model_file = line;
		}
		else if (line == "[BEAM-SIZE]")
		{
			getline(fin,line);
			para.BEAM_SIZE = stoi(line);
		}
		else if (line == "[SEN-THREAD-NUM]")
		{
			getline(fin,line);
			para.SEN_THREAD_NUM = stoi(line);
		}
		else if (line == "[SPAN-THREAD-NUM]")
		{
			getline(fin,line);
			para.SPAN_THREAD_NUM = stoi(line);
		}
		else if (line == "[NBEST-NUM]")
		{
			getline(fin,line);
			para.NBEST_NUM = stoi(line);
		}
		else if (line == "[REORDER-WINDOW]")
		{
			getline(fin,line);
			para.REORDER_WINDOW = stoi(line);
		}
		else if (line == "[RULE-NUM-LIMIT]")
		{
			getline(fin,line);
			para.RULE_NUM_LIMIT = stoi(line);
		}
		else if (line == "[PRINT-NBEST]")
		{
			getline(fin,line);
			para.PRINT_NBEST = stoi(line);
		}
		else if (line == "[LOAD-ALIGNMENT]")
		{
			getline(fin,line);
			para.LOAD_ALIGNMENT = stoi(line);
		}
		else if (line == "[weight]")
		{
			while(getline(fin,line))
			{
				if (line == "")
					break;
				stringstream ss(line);
				string feature;
				ss >> feature;
				if (feature.find("trans") != string::npos)
				{
					double w;
					ss>>w;
					weight.trans.push_back(w);
				}
				else if(feature == "len")
				{
					ss>>weight.len;
				}
				else if(feature == "lm")
				{
					ss>>weight.lm;
				}
				else if(feature == "reorder-mono")
				{
					ss>>weight.reorder_mono;
				}
				else if(feature == "reorder-swap")
				{
					ss>>weight.reorder_swap;
				}
				else if(feature == "phrase-num")
				{
					ss>>weight.phrase_num;
				}
			}
		}
	}
}

void translate_file(const Models &models, const Parameter &para, const Weight &weight, const string &input_file, const string &output_file)
{
	ifstream fin(input_file.c_str());
	if (!fin.is_open())
	{
		cerr<<"cannot open input file!\n";
		return;
	}
	ofstream fout(output_file.c_str());
	if (!fout.is_open())
	{
		cerr<<"cannot open output file!\n";
		return;
	}
	vector<string> input_sen;
	vector<string> output_sen;
	string line;
	while(getline(fin,line))
	{
		TrimLine(line);
		input_sen.push_back(line);
	}
	output_sen.resize(input_sen.size());
#pragma omp parallel for num_threads(para.SEN_THREAD_NUM)
	for (size_t i=0;i<input_sen.size();i++)
	{
		SentenceTranslator sen_translator(models,para,weight,input_sen.at(i));
		output_sen.at(i) = sen_translator.translate_sentence();
	}
	for (const string &sen : output_sen)
	{
		fout<<sen<<endl;
	}
}

int main()
{
	clock_t a,b;
	a = clock();

	omp_set_nested(1);
	Filenames fns;
	Parameter para;
	Weight weight;
	read_config(fns,para,weight,"config.ini");

	Vocab *src_vocab = new Vocab(fns.src_vocab_file);
	Vocab *tgt_vocab = new Vocab(fns.tgt_vocab_file);
	RuleTable *ruletable = new RuleTable(para.RULE_NUM_LIMIT,para.LOAD_ALIGNMENT,weight,fns.rule_table_file);
	MaxentModel *reorder_model = new MaxentModel(fns.reorder_model_file);
	LanguageModel *lm_model = new LanguageModel(fns.lm_file);

	b = clock();
	cout<<"loading time: "<<double(b-a)/CLOCKS_PER_SEC<<endl;

	Models models = {src_vocab,tgt_vocab,ruletable,reorder_model,lm_model};
	translate_file(models,para,weight,fns.input_file,fns.output_file);
	b = clock();
	cout<<"time cost: "<<double(b-a)/CLOCKS_PER_SEC<<endl;
	return 0;
}
