all:mebtg build_binary query
.PHONY:all

util/bit_packing.o:util/bit_packing.cc
	g++ -I. -O0 -DNDEBUG  -c util/bit_packing.cc -o util/bit_packing.o -l pthread -w
util/ersatz_progress.o:util/ersatz_progress.cc
	g++ -I. -O0 -DNDEBUG  -c util/ersatz_progress.cc -o util/ersatz_progress.o -l pthread -w
util/exception.o:util/exception.cc
	g++ -I. -O0 -DNDEBUG  -c util/exception.cc -o util/exception.o -l pthread -w
util/file_piece.o:util/file_piece.cc
	g++ -I. -O0 -DNDEBUG  -c util/file_piece.cc -o util/file_piece.o -l pthread -w
util/murmur_hash.o:util/murmur_hash.cc
	g++ -I. -O0 -DNDEBUG  -c util/murmur_hash.cc -o util/murmur_hash.o -l pthread -w
util/file.o:util/file.cc
	g++ -I. -O0 -DNDEBUG  -c util/file.cc -o util/file.o -l pthread -w
util/mmap.o:util/mmap.cc
	g++ -I. -O0 -DNDEBUG  -c util/mmap.cc -o util/mmap.o -l pthread -w
lm/bhiksha.o:lm/bhiksha.cc
	g++ -I. -O0 -DNDEBUG  -c lm/bhiksha.cc -o lm/bhiksha.o -l pthread -w
lm/binary_format.o:lm/binary_format.cc
	g++ -I. -O0 -DNDEBUG  -c lm/binary_format.cc -o lm/binary_format.o -l pthread -w
lm/config.o:lm/config.cc
	g++ -I. -O0 -DNDEBUG  -c lm/config.cc -o lm/config.o -l pthread -w
lm/lm_exception.o:lm/lm_exception.cc
	g++ -I. -O0 -DNDEBUG  -c lm/lm_exception.cc -o lm/lm_exception.o -l pthread -w
lm/model.o:lm/model.cc
	g++ -I. -O0 -DNDEBUG  -c lm/model.cc -o lm/model.o -l pthread -w
lm/quantize.o:lm/quantize.cc
	g++ -I. -O0 -DNDEBUG  -c lm/quantize.cc -o lm/quantize.o -l pthread -w
lm/read_arpa.o:lm/read_arpa.cc
	g++ -I. -O0 -DNDEBUG  -c lm/read_arpa.cc -o lm/read_arpa.o -l pthread -w
lm/search_hashed.o:lm/search_hashed.cc
	g++ -I. -O0 -DNDEBUG  -c lm/search_hashed.cc -o lm/search_hashed.o -l pthread -w
lm/search_trie.o:lm/search_trie.cc
	g++ -I. -O0 -DNDEBUG  -c lm/search_trie.cc -o lm/search_trie.o -l pthread -w
lm/trie.o:lm/trie.cc
	g++ -I. -O0 -DNDEBUG  -c lm/trie.cc -o lm/trie.o -l pthread -w
lm/trie_sort.o:lm/trie_sort.cc
	g++ -I. -O0 -DNDEBUG  -c lm/trie_sort.cc -o lm/trie_sort.o -l pthread -w
lm/virtual_interface.o:lm/virtual_interface.cc
	g++ -I. -O0 -DNDEBUG  -c lm/virtual_interface.cc -o lm/virtual_interface.o -l pthread -w
lm/vocab.o:lm/vocab.cc
	g++ -I. -O0 -DNDEBUG  -c lm/vocab.cc -o lm/vocab.o -l pthread -w
decoder/Algorithm.o:decoder/Algorithm.cc
	g++ -I. -O0 -DNDEBUG  -c decoder/Algorithm.cc -o decoder/Algorithm.o -l pthread -w
decoder/HypoStack.o:decoder/HypoStack.cc
	g++ -I. -O0 -DNDEBUG  -c decoder/HypoStack.cc -o decoder/HypoStack.o -l pthread -w
decoder/ff_klm.o:decoder/ff_klm.cc
	g++ -I. -O0 -DNDEBUG  -c decoder/ff_klm.cc -o decoder/ff_klm.o -l pthread -w
decoder/Ngram.o:decoder/Ngram.cc
	g++ -I. -O0 -DNDEBUG  -c decoder/Ngram.cc -o decoder/Ngram.o -l pthread -w
decoder/NgramTrie.o:decoder/NgramTrie.cc
	g++ -I. -O0 -DNDEBUG  -c decoder/NgramTrie.cc -o decoder/NgramTrie.o -l pthread -w
decoder/PhrasePro.o:decoder/PhrasePro.cc
	g++ -I. -O0 -DNDEBUG  -c decoder/PhrasePro.cc -o decoder/PhrasePro.o -l pthread -w
decoder/PhraseProTrie.o:decoder/PhraseProTrie.cc
	g++ -I. -O0 -DNDEBUG  -c decoder/PhraseProTrie.cc -o decoder/PhraseProTrie.o -l pthread -w
decoder/SearchSpaceStock.o:decoder/SearchSpaceStock.cc
	g++ -I. -O0 -DNDEBUG  -c decoder/SearchSpaceStock.cc -o decoder/SearchSpaceStock.o -l pthread -w
decoder/Translator.o:decoder/Translator.cc
	g++ -I. -O0 -DNDEBUG  -c decoder/Translator.cc -o decoder/Translator.o -l pthread -w
decoder/Vocab.o:decoder/Vocab.cc
	g++ -I. -O0 -DNDEBUG  -c decoder/Vocab.cc -o decoder/Vocab.o -l pthread -w
decoder/Candidate.o:decoder/Candidate.cc
	g++ -I. -O0 -DNDEBUG  -c decoder/Candidate.cc -o decoder/Candidate.o -l pthread -w
decoder/Feature.o:decoder/Feature.cc
	g++ -I. -O0 -DNDEBUG  -c decoder/Feature.cc -o decoder/Feature.o -l pthread -w
decoder/MePredict.o:decoder/MePredict.cc
	g++ -I. -O0 -DNDEBUG  -c decoder/MePredict.cc -o decoder/MePredict.o -l pthread -w
objs = util/bit_packing.o util/ersatz_progress.o util/exception.o util/file_piece.o util/murmur_hash.o util/file.o util/mmap.o lm/bhiksha.o lm/binary_format.o lm/config.o lm/lm_exception.o lm/model.o lm/quantize.o lm/read_arpa.o lm/search_hashed.o lm/search_trie.o lm/trie.o lm/trie_sort.o lm/virtual_interface.o lm/vocab.o decoder/Algorithm.o decoder/HypoStack.o decoder/ff_klm.o decoder/Ngram.o decoder/NgramTrie.o decoder/PhrasePro.o decoder/PhraseProTrie.o decoder/SearchSpaceStock.o decoder/Translator.o decoder/Vocab.o decoder/Candidate.o decoder/Feature.o decoder/MePredict.o
objs1 = util/bit_packing.o util/ersatz_progress.o util/exception.o util/file_piece.o util/murmur_hash.o util/file.o util/mmap.o lm/bhiksha.o lm/binary_format.o lm/config.o lm/lm_exception.o lm/model.o lm/quantize.o lm/read_arpa.o lm/search_hashed.o lm/search_trie.o lm/trie.o lm/trie_sort.o lm/virtual_interface.o lm/vocab.o
mebtg:$(objs)
	g++ -I. -O0 -static $(objs) -lz -lmaxent -lgfortran -o mebtgDecoderMultiThreads.debug -l pthread -w
build_binary:$(objs1)
	g++ -I. -O0 -DNDEBUG lm/build_binary.cc $(objs1) -lz -lmaxent -lgfortran -o build_binary -w
query:$(objs1)
	g++ -I. -O0 -DNDEBUG lm/ngram_query.cc $(objs1) -lz -lmaxent -lgfortran -o query -w

.PHONY:clean
clean:
	rm decoder/*.o lm/*.o util/*.o

