#!/usr/bin/python
from subprocess import call
print '''all:mebtg build_binary query
.PHONY:all
'''

objs = []
objs1 = []
for e in 'bit_packing,ersatz_progress,exception,file_piece,murmur_hash,file,mmap'.split(','):
	o = 'util/'+e+'.o'
	c = 'util/'+e+'.cc'
	objs.append(o)
	objs1.append(o)
	print o+':'+c
	print '\tg++ -I. -O3 -DNDEBUG  -c {} -o {} -l pthread -w'.format(c,o)
for e in 'bhiksha,binary_format,config,lm_exception,model,quantize,read_arpa,search_hashed,search_trie,trie,trie_sort,virtual_interface,vocab'.split(','):
	o = 'lm/'+e+'.o'
	c = 'lm/'+e+'.cc'
	objs.append(o)
	objs1.append(o)
	print o+':'+c
	print '\tg++ -I. -O3 -DNDEBUG  -c {} -o {} -l pthread -w'.format(c,o)
for e in 'Algorithm,HypoStack,ff_klm,Ngram,NgramTrie,PhrasePro,PhraseProTrie,SearchSpaceStock,Translator,Vocab,Candidate,Feature,MePredict'.split(','):
	o = 'decoder/'+e+'.o'
	c = 'decoder/'+e+'.cc'
	objs.append(o)
	print o+':'+c
	print '\tg++ -I. -O3 -DNDEBUG  -c {} -o {} -l pthread -w'.format(c,o)

print '''objs = {}
objs1 = {}
mebtg:$(objs)
\tg++ -I. -O3 -static $(objs) -lz -lmaxent -lgfortran -o mebtgDecoderMultiThreads.debug -l pthread -w
build_binary:$(objs1)
\tg++ -I. -O3 -DNDEBUG lm/build_binary.cc $(objs1) -lz -lmaxent -lgfortran -o build_binary -w
query:$(objs1)
\tg++ -I. -O3 -DNDEBUG lm/ngram_query.cc $(objs1) -lz -lmaxent -lgfortran -o query -w

.PHONY:clean
clean:
	rm decoder/*.o lm/*.o util/*.o
'''.format(' '.join(objs),' '.join(objs1))
