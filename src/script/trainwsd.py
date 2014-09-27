#!/usr/bin/python
from maxent import MaxentModel
from collections import Counter
from multiprocessing import Pool
import os.path

#get frequency and id for each word type
def get_freq_and_id_for_each_word_type():
	global w2freq,w2id
	w2id = dict([e.split() for e in open('vocab.ch').readlines()])
	w2freq = Counter(open(ch_file).read().split())

#get token info list for each word type, i.e. [(sen_sn,word_sn,translation),...]
def get_token_info_list_for_each_word_type():
	global type2token_info_list,lines
	sen_sn = -1
	for sch,sen,sal in zip(open(ch_file),open(en_file),open(align_file)):
		sen_sn += 1
		#build ch_sn to en_sns dict
		d = {}
		for e in sal.split():
			ch_sn,en_sn = [int(x) for x in e.split('-')]
			if ch_sn not in d:
				d[ch_sn] = [en_sn]
			else:
				d[ch_sn].append(en_sn)
	
		sch,sen = sch.split(),sen.split()
		lines.append(sch)
		for word_sn,w in enumerate(sch):
			if 10<=w2freq[w]<=20000:
				en_sns = d.get(word_sn,[])
				if len(en_sns) == 0:
					trans = 'NULL'
				else:
					trans = '_'.join(sen[en_sn] for en_sn in en_sns)
				if w not in type2token_info_list:
					type2token_info_list[w] = [(sen_sn,word_sn,trans)]
				else:
					type2token_info_list[w].append((sen_sn,word_sn,trans))

#train ME model for each word type
def train_model_for_one_word_type(kvp):
	wch,token_info_list = kvp
	if wch not in w2id:
		return
	if len(set(zip(*token_info_list)[2])) == 1:
		return
	m = MaxentModel()
	m.begin_add_event()
	for sen_sn,word_sn,trans in token_info_list:
		s = [str(i-word_sn)+'/'+w for i,w in enumerate(lines[sen_sn])]
		lbound = max(0,word_sn-10)
		rbound = word_sn+11
		context = s[lbound:rbound]
		m.add_event(context,trans,1)
	m.end_add_event()
	m.train(100,'lbfgs',1)
	m.save('model/{}'.format(w2id[wch]),True)

ch_file = 'fbis.ch'
en_file = 'fbis.en'
align_file = 'aligned.grow-diag-final-and'
w2freq = {}
w2id = {}
get_freq_and_id_for_each_word_type()
lines = []
type2token_info_list = {}
get_token_info_list_for_each_word_type()
pool = Pool(20)
pool.map( train_model_for_one_word_type,type2token_info_list.items() )
pool.close()
pool.join()
