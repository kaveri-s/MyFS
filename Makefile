all:myfs.o mydef.o

myfs.o:myfs.c
	gcc -c -g -Wall -D_FILE_OFFSET_BITS=64 myfs.c -o myfs.o
mydef.o:mydef.c
	gcc -c -g -Wall -D_FILE_OFFSET_BITS=64 mydef.c -o mydef.o
myfs:myfs.o mydef.o
	gcc myfs.o mydef.o -lfuse -o myfs
