all: hellokvm

clean:
	-@rm *.o *.h *.bin hellokvm 2> /dev/null || true

hellokvm: hellokvm.o
	gcc -g hellokvm.o -o hellokvm

hellokvm.o: hellokvm.c code.h
	gcc -g -o hellokvm.o -c hellokvm.c

code.h: code.bin
	BINCODE="$(shell hexdump -v -e '"\\""x" 1/1 "%02x" ""' code.bin)" ; echo "const char guest_code[] = \"$$BINCODE\";" > code.h

code.bin: code.o
	ld -m elf_i386 --oformat binary -N -e _start -Ttext 0x10000 -o code.bin code.o

code.o: code.S
	as -32 code.S -o code.o
	
