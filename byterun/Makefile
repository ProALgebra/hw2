all: byterun interpreter bytecode_freq

.PHONY: compare_old
compare_old:
	diff -u interpretatorv1_0old.cpp interpretatorv1_0.cpp || true

interpreter: interpretatorv1_0.cpp bytecode_verify.o byterun_nomain.o
	g++ -std=c++17 -m32 -fpermissive interpretatorv1_0.cpp bytecode_verify.o byterun_nomain.o ../runtime32/runtime.a -o interpreter

interpreter_old: interpretatorv1_0old.cpp
	g++ -std=c++17 -m32 -fpermissive interpretatorv1_0old.cpp ../runtime32/runtime.a -o interpreter_old

byterun: byterun.o
	$(CC) -m32 -g -o byterun byterun.o ../runtime32/runtime.a

byterun_nomain.o: byterun.c
	$(CC) -g -fstack-protector-all -m32 -DNO_MAIN -c byterun.c -o byterun_nomain.o

bytecode_verify.o: bytecode_verify.cpp
	g++ -std=c++17 -m32 -DVERIFY_NO_MAIN -c bytecode_verify.cpp -o bytecode_verify.o

byterun.o: byterun.c
	$(CC) -g -fstack-protector-all -m32 -c byterun.c

bytecode_freq: bytecode_freq.cpp byterun_nomain.o
	g++ -std=c++17 -m32 bytecode_freq.cpp byterun_nomain.o -o bytecode_freq

perf_test: lama all performance
	lamac -b ../performance/Sort.lama
	`which time` -f "Sort time\t%U" ./interpreter Sort.bc

perf_compare: lama interpreter interpreter_old performance
	lamac -b ../performance/Sort.lama
	@echo "new interpreter:"
	`which time` -f "Sort time\t%U" ./interpreter Sort.bc
	@echo "old interpreter:"
	`which time` -f "Sort time\t%U" ./interpreter_old Sort.bc

.PHONY: perf_avg
perf_avg: lama interpreter interpreter_old performance
	lamac -b ../performance/Sort.lama
	@echo "new interpreter average (5 runs):"
	@{ for i in 1 2 3 4 5; do `which time` -f "%U" ./interpreter Sort.bc >/dev/null; done; } 2>&1 | awk '{s+=$$1} END {printf("Sort time\t%.3f\n", s/NR)}'
	@echo "old interpreter average (5 runs):"
	@{ for i in 1 2 3 4 5; do `which time` -f "%U" ./interpreter_old Sort.bc >/dev/null; done; } 2>&1 | awk '{s+=$$1} END {printf("Sort time\t%.3f\n", s/NR)}'

lama:
	$(MAKE) -C ..

performance:
	$(MAKE) -C ../performance

clean:
	$(RM) *.a *.o *~ bytecode_freq interpreter interpreter_old byterun
