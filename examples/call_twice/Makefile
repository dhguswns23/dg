CC=clang
CFLAGS= -O0 -g


all: fnptr

fnptr: fnptr.bc
	$(CC) $(CFLAGS) fnptr.c -o fnptr

fnptr.bc: fnptr.c
	# $(CC) $(CFLAGS) -emit-llvm -c fnptr.c -fsanitize=address
	$(CC) $(CFLAGS) -emit-llvm -c fnptr.c
	llvm-dis fnptr.bc

clean:
	rm -f a.out *.bc *.o *.ll fnptr

clean-klee:
	rm -rf klee-*
