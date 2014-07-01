CXXFLAGS =-std=c++11 -O2 -static
translator: main.o translator.o lm.o ruletable.o maxent.o vocab.o myutils.o
	g++ -o mebtg main.o translator.o lm.o ruletable.o maxent.o vocab.o myutils.o -std=c++11 -lz -O2

main.o: translator.h stdafx.h datastruct.h vocab.h ruletable.h lm.h maxent.h myutils.h
translator.o: translator.h stdafx.h datastruct.h vocab.h ruletable.h lm.h maxent.h myutils.h
lm.o: lm.h stdafx.h
ruletable.o: ruletable.h stdafx.h datastruct.h
maxent.o: maxent.h stdafx.h myutils.h
vocab.o: vocab.h stdafx.h
myutils.o: myutils.h stdafx.h

clean:
	rm *.o
