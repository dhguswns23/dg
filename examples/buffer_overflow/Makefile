CC=clang
CFLAGS= -O0 -g


all: buf

buf: buf.bc
	$(CC) $(CFLAGS) buf.c -o buf

buf.bc: buf.c
	# $(CC) $(CFLAGS) -emit-llvm -c buf.c -fsanitize=address
	$(CC) $(CFLAGS) -emit-llvm -c buf.c
	llvm-dis buf.bc

clean:
	rm -f a.out *.bc *.o *.ll buf

clean-klee:
	rm -rf klee-*
