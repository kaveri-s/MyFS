/* This file contains all the structures required to create a file system


#include <fuse.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/time.h>


struct stat {
    unsigned int     st_dev;         /* ID of device containing file */
    unsigned int     st_ino;         /* inode number */
    unsigned int    st_mode;        /* protection */
    unsigned int   st_nlink;       /* number of hard links */
	unsigned int     st_uid;         /* user ID of owner */
	unsigned int     st_gid;         /* group ID of owner */
    unsigned int     st_rdev;        /* device ID (if special file) */
    unsigned int     st_size;        /* total size, in bytes */
    unsigned int st_blksize;     /* blocksize for filesystem I/O */
    unsigned int  st_blocks;      /* number of 512B blocks allocated */
    unsigned int st_atim;  /* time of last access */
    unsigned int st_mtim;  /* time of last modification */
	unsigned int st_ctim;  /* time of last status change */
};