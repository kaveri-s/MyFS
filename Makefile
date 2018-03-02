all:myfs.o mydef.o try.o

myfs.o:myfs.c
	gcc -c -g -Wall -D_FILE_OFFSET_BITS=64 myfs.c -o myfs.o
mydef.o:mydef.c
	gcc -c -g -Wall -D_FILE_OFFSET_BITS=64 mydef.c -o mydef.o
try.o:try.c
	gcc -c -g -Wall -D_FILE_OFFSET_BITS=64 try.c -o try.o
myfs:myfs.o mydef.o try.o
	gcc myfs.o mydef.o try.o -lfuse -o myfs
