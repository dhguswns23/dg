INCLUDE=../../include
OPTS="-I $(INCLUDE)"

all: concolic

concolic: concolic.bc
	clang $(OPTS) -c concolic.c -O0 -g -o concolic

concolic.bc:
	clang $(OPTS) -emit-llvm -c concolic.c -O0 -g
	llvm-dis concolic.bc

clean:
	rm a.out *.bc *.o *.ll concolic

