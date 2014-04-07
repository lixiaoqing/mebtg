#!/bin/bash
#This is just an example compilation.  You should integrate these files into your build system.  I can provide boost jam if you want.  
#If your code uses ICU, edit util/string_piece.hh and uncomment #define USE_ICU
#I use zlib by default.  If you don't want to depend on zlib, remove #define USE_ZLIB from util/file_piece.hh

#don't need to use if compiling with moses Makefiles already

set -e

for i in util/{bit_packing,ersatz_progress,exception,file_piece,murmur_hash,file,mmap} lm/{bhiksha,binary_format,config,lm_exception,model,quantize,read_arpa,search_hashed,search_trie,trie,trie_sort,virtual_interface,vocab} decoder/{Algorithm,HypoStack,ff_klm,Ngram,NgramTrie,PhrasePro,PhraseProTrie,SearchSpaceStock,Translator,Vocab,Candidate,Feature,MePredict}; do
  g++ -I. -O3 -DNDEBUG $CXXFLAGS -c $i.cc -o $i.o -l pthread -lz -lmaxent -lgfortran -w
done
g++ -I. -O3 -static {decoder,lm,util}/*.o -lz -lmaxent -lgfortran -o mebtgDecoderMultiThreads.debug -l pthread -w
g++ -I. -O3 -DNDEBUG $CXXFLAGS lm/build_binary.cc {lm,util}/*.o -lz -lmaxent -lgfortran -o build_binary -w
g++ -I. -O3 -DNDEBUG $CXXFLAGS lm/ngram_query.cc {lm,util}/*.o -lz -lmaxent -lgfortran -o query -w
