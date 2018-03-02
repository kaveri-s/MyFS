gcc -c -g -Wall -D_FILE_OFFSET_BITS=64 myfs.c -o myfs.o
gcc -c -g -Wall -D_FILE_OFFSET_BITS=64 mydef.c -o mydef.o
gcc -c -g -Wall -D_FILE_OFFSET_BITS=64 try.c -o try.o
gcc myfs.o mydef.o try.o -lfuse -o myfs
./myfs -s -o default_permissions -o auto_unmount temp
