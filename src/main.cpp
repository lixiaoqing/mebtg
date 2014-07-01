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
		else if (line == "[EXTRA-BEAM-SIZE]")
		{
			getline(fin,line);
			para.EXTRA_BEAM_SIZE = stoi(line);
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

int main()
{
	clock_t a,b;
	a = clock();

	Filenames fns;
	Parameter para;
	Weight weight;
	read_config(fns,para,weight,"config.ini");

	Vocab *src_vocab = new Vocab(fns.src_vocab_file);
	Vocab *tgt_vocab = new Vocab(fns.tgt_vocab_file);
	RuleTable *ruletable = new RuleTable(para.RULE_NUM_LIMIT,weight,fns.rule_table_file);
	MaxentModel *reorder_model = new MaxentModel(fns.reorder_model_file);
	LanguageModel *lm_model = new LanguageModel(fns.lm_file);

	b = clock();
	cout<<"loading time: "<<double(b-a)/CLOCKS_PER_SEC<<endl;

	Models models = {src_vocab,tgt_vocab,ruletable,reorder_model,lm_model};
	FileTranslator translator(models,para,weight);
	translator.translate_file(fns.input_file,fns.output_file);
	b = clock();
	cout<<"time cost: "<<double(b-a)/CLOCKS_PER_SEC<<endl;
	return 0;
}
