CXX=g++
CXXFLAGS=-std=c++11 -O3 -fopenmp -lz -I. -DKENLM_MAX_ORDER=6
objs=lm/*.o util/*.o util/double-conversion/*.o
translator: main.o translator.o lm.o ruletable.o maxent.o vocab.o datastruct.o myutils.o $(objs)
	$(CXX) -o mebtg main.o translator.o lm.o ruletable.o maxent.o vocab.o myutils.o datastruct.o $(objs) $(CXXFLAGS)

main.o:main.cpp
	        $(CXX) $(CXXFLAGS) -c main.cpp
translator.o:main.cpp
	        $(CXX) $(CXXFLAGS) -c translator.cpp
lm.o:main.cpp
	        $(CXX) $(CXXFLAGS) -c lm.cpp
ruletable.o:main.cpp
	        $(CXX) $(CXXFLAGS) -c ruletable.cpp
maxent.o:main.cpp
	        $(CXX) $(CXXFLAGS) -c maxent.cpp
vocab.o:main.cpp
	        $(CXX) $(CXXFLAGS) -c vocab.cpp
datastruct.o:main.cpp
	        $(CXX) $(CXXFLAGS) -c datastruct.cpp
myutils.o:main.cpp
	        $(CXX) $(CXXFLAGS) -c myutils.cpp

clean:
	rm *.o
