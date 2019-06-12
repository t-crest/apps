# This is the trivial Hello World example, distributed over two .c files

all:
	patmos-clang -O2 -I../../patmos/c/libcorethread/ ../../patmos/c/libcorethread/*.c *.c lib/*.c -o vip4f.elf

clean:
	rm -rf vip4f.elf

simu:
	patemu vip4f.elf

run:
	patex vip4f.elf

