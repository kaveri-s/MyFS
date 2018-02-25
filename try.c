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

//Stores 38 inodes
char fs[BLOCK_NO*BLOCKSIZE];

struct myinode *root;

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

int free_blks[100];

#endif

int read_inode() {
    struct myinode *ino = (struct myinode *)malloc(sizeof(struct myinode));
    memcpy(ino, fs, INODE_SIZE);
    printf("\nInode info: %d", ino->direct_blk[0]);

}

int make_rootnode(ino_t st_id, file_type type, int blk, mode_t mode){
    printf("Made it here!!");
    root->st_mode=mode;
    root->st_id=st_id;
    root->type=type;
    root->st_blocks=1;
    root->direct_blk[0]=blk;
    memcpy((char *)fs, root, INODE_SIZE);
}

int make_rootdir(char* name) {
    struct mydirent *dir = (struct mydirent *)malloc(sizeof(struct mydirent));
    strcpy(dir->name, name);
    strcpy(dir->subs[0],".");
    dir->sub_id[0]=root->direct_blk[0];
    memcpy((char *)fs+root->direct_blk[0]*BLOCKSIZE, dir, BLOCKSIZE);
}

static void init_fs() {
    // printf("Made it here");
    make_rootnode(0, DIRECTORY, 2, S_IFDIR | 0755);

    make_rootdir("/");

    printf("Initialised Root");

    struct myinode *ino = (struct myinode *)malloc(sizeof(struct myinode));
    for(int i=1; i<INODE_NO; i++) {
        ino->st_id=i;
        ino->type=FREE;
        ino->st_blocks=0;
        memcpy(fs+(i*INODE_SIZE), ino, INODE_SIZE);
    }

    printf("\nInitialised rest of the inodes");

    free_blks[0]=1; //occupied by inode list
    free_blks[1]=1; //occupied by free block list

    printf("\nInitialised free block list");
}

void main(){
    printf("Size of myinode: %d", sizeof(struct myinode));
    printf("\nSize of mydirent: %d", sizeof(struct mydirent));
    printf("\nNo. of inodes: %f", 4000.0/sizeof(struct myinode));
    printf("\nSize of free block list: %d", sizeof(free_blks));
    printf("\nIt only prints");

    root = (struct myinode *)malloc(sizeof(struct myinode));
    // printf("Made it here");
    init_fs(root);
    read_inode();
    free(root);
}
