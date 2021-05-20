simple-tdbg: history.o parse_elf.o myuntil.o simple-tdbg.c
	gcc parse_elf.o history.o myuntil.o simple-tdbg.c -o simple-tdbg

history.o: history.c
	gcc -o history.o -c history.c

parse_elf.o: parse_elf.c myuntil.o
	gcc -c myuntil.o parse_elf.c -o parse_elf.o

myuntil.o: myuntil.c
	gcc -c myuntil.c -o myuntil.o

clean:
	rm *.o
