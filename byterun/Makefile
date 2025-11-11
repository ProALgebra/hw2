all: byterun interpreter

interpreter: interpretatorv1_0.cpp
	g++ -std=c++17 -m32 -fpermissive interpretatorv1_0.cpp ../runtime32/runtime.a -o interpreter

byterun: byterun.o
	$(CC) -m32 -g -o byterun byterun.o ../runtime32/runtime.a
	

byterun.o: byterun.c
	$(CC) -g -fstack-protector-all -m32 -c byterun.c

perf_test: lama all performance
	lamac -b ../performance/Sort.lama
	`which time` -f "Sort time\t%U" ./interpreter Sort.bc

lama:
	$(MAKE) -C ..

performance:
	$(MAKE) -C ../performance

clean:
	$(RM) *.a *.o *~
