all: div

div: div.bc
	clang -c div.c -O0 -g -o div

div.bc: div.c
	clang -emit-llvm -c div.c -O0 -g
	llvm-dis div.bc

clean:
	rm a.out *.bc *.o *.ll div

