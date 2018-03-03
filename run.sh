mkdir temp
gcc -c -g -Wall -D_FILE_OFFSET_BITS=64 myfs.c -o myfs.o
gcc -c -g -Wall -D_FILE_OFFSET_BITS=64 mydef.c -o mydef.o
gcc -c -g -Wall -D_FILE_OFFSET_BITS=64 try.c -o try.o
gcc myfs.o mydef.o try.o -lfuse -o myfs
# valgrind --log-file="error.txt" --track-origins=yes --leak-check=full --show-leak-kinds=all ./myfs -s -d -f -o default_permissions -o auto_unmount temp
# ./myfs -d -f -s -o default_permissions -o auto_unmount temp && . mytest.sh
gdb -tui myfs
# gdb myfs
# -s -d -f -o default_permissions -o auto_unmount temp2