#include <stdio.h>
#include <stdint.h>
#include <libgen.h>
#include <linux/limits.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/time.h>
#include <sys/mman.h>

#define BLOCKSIZE 4096
#define BLOCK_NO 100
#define INODE_SIZE 88
#define INODE_NO 45
#define SUB_NO 59
#define MAX_NAME_LEN 64
#define MAX_MAP 3

#define AT (1 << 0)
#define CT (1 << 1)
#define MT (1 << 2)

#ifndef _NODE_H
#define _NODE_H
typedef enum {
	ORDINARY,
	DIRECTORY,
	FREE
} file_type;

//Stores fs info
void *fs;



//Allocating max 3 blocks for a file
struct myinode {   
    ino_t st_id;            //inode id
    mode_t st_mode;         //permissions
    nlink_t st_nlink;       //number of hard links
    off_t st_size;          //size
    blkcnt_t st_blocks;     //number of blocks referred to
    uid_t st_uid;           //user id
    gid_t st_gid;           //group id
    time_t st_mtim;         //modify time
    time_t st_atim;         //access time
    time_t st_ctim;         //creation time

    int direct_blk[MAX_MAP];      //blocks mapped to
    file_type type;  //0-free    1-file      2-directory

};

struct myinode *root;

//Reducing limit to 60 files in a directory and 64 characters in the name
struct mydirent {
    char name[MAX_NAME_LEN];      //name of dir
    char subs[SUB_NO][MAX_NAME_LEN];  //names of contents
    int sub_id[SUB_NO];     //inode ids of contents
    char extra[20];
};

struct filehandle {
  struct myinode *node;
  int o_flags;
};


void openfile();
void testfs();
int make_rootnode(ino_t st_id, file_type type, int blk, mode_t mode);
int make_rootdir(char* name);
int init_fs();

#endif