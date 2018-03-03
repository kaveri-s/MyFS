#include "try.h"
#include<sys/time.h>


// int read_inode(ino_t i) {
//     struct myinode *ino = (struct myinode *)malloc(sizeof(struct myinode));
//     memcpy(ino, fs, INODE_SIZE);
//     printf("\nId: %d", ino->st_id);
//     printf("\nMode: %d", ino->st_mode);
//     printf("\nHard Links: %d", ino->st_nlink);
//     printf("\nSize: %d", ino->st_size);
//     printf("\nBlocks: %d", ino->st_blocks);
//     printf("\nUID: %d", ino->st_uid);
//     printf("\nGID: %d", ino->st_gid);
//     printf("\nAT: %d", ino->st_atim);
//     printf("\nMT: %d", ino->st_mtim);
//     printf("\nBT: %d\n", ino->st_ctim);
//     for(int i=0; i<ino->st_blocks;i++)
//         printf("Block no %d mapped to: %d\n", i, ino->direct_blk[i]);
//     free(ino);
//     return 1;
// }

int read_dirent(int blk) {
    struct mydirent *dir = (struct mydirent *)malloc(sizeof(struct mydirent));
    memcpy(dir, fs+blk*BLOCKSIZE, BLOCKSIZE);
    printf("Name %s\n", dir->name);
    for(int i=0; i<SUB_NO; i++) {
        printf("Entry: %s\t", dir->subs[i]);
        printf("Inode number: %d\n", dir->sub_id[i]);
    }
    free(dir);
    return 1;
}

int read_file(struct myinode *node) {
    char data[BLOCKSIZE];
    for(int i; i < node->st_blocks; i++) {
        memcpy(data, fs+node->direct_blk[i]*BLOCKSIZE, BLOCKSIZE);
        puts(data);
    }
    return 1;
}

int free_blocks() {
    int blk=-1;
    printf("\nFree Block List: ");
    for(int i=0; i<BLOCK_NO; i++) {
        memcpy(&blk, fs+BLOCKSIZE+i*sizeof(int), sizeof(int));
        printf("%d\t", blk);
    }
    return 1;
}

void openfile() {
    const char *filepath = "/home/kaveri/Desktop/myfs";

    int fd;

    size_t fssize = BLOCK_NO*BLOCKSIZE;

    if((fd = open(filepath, O_RDWR, (mode_t)0600))==-1){
        fd = open(filepath, O_RDWR | O_CREAT | O_TRUNC, (mode_t)0600);
        if (fd == -1) {
            perror("Error opening file for writing");
            exit(EXIT_FAILURE);
        }

        // Stretch the file size to the size of the (mmapped) array of char
        int result = lseek(fd, fssize-1, SEEK_SET);
        if (result == -1) {
	        close(fd);
	        perror("Error calling lseek() to 'stretch' the file");
	        exit(EXIT_FAILURE);
        }

        result = write(fd, "", 1);
        if (result != 1) {
	        close(fd);
	        perror("Error writing last byte of the file");
	        exit(EXIT_FAILURE);
        }

        fs = mmap(0, fssize, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
        if (fs == MAP_FAILED) {
	        close(fd);
	        perror("Error mmapping the file");
	        exit(EXIT_FAILURE);
        }

        init_fs();

        return;

    }

    

    fs = mmap(0, fssize, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (fs == MAP_FAILED) {
	    close(fd);
	    perror("Error mmapping the file");
	    exit(EXIT_FAILURE);
    }

    memcpy(root, fs, INODE_SIZE);

    printf("FS Synced");
}

int make_rootnode(ino_t st_id, file_type type, int blk, mode_t mode){
    // printf("Made it here!!");
    root->st_mode=mode;
    root->st_nlink=2;
    root->st_id=st_id;
    root->type=type;
    root->st_size=BLOCKSIZE;
    root->st_blocks=1;
    root->direct_blk[0]=blk;
    root->st_uid = getuid();
    root->st_gid = getgid();
    memcpy(fs, root, INODE_SIZE);
    return 1;
}

int make_rootdir(char* name) {
    struct mydirent *dir = (struct mydirent *)malloc(sizeof(struct mydirent));
    memset(dir, 0, BLOCKSIZE);
    strcpy(dir->name, name);
    strcpy(dir->subs[0],".");
    dir->sub_id[0]=root->st_id;
    strcpy(dir->subs[1],"..");
    dir->sub_id[1]=root->st_id;
    for(int i=2; i<SUB_NO; i++)
        dir->sub_id[i]=-1;
    memcpy(fs+root->direct_blk[0]*BLOCKSIZE, dir, BLOCKSIZE);
    free(dir);
    return 1;
}

int init_fs() {
    // printf("Made it here");
    make_rootnode(0, DIRECTORY, 2, S_IFDIR | 0755);

    // read_inode(root->st_id);

    make_rootdir("/");

    // read_dirent(root->direct_blk[0]);

    printf("Initialised Root");

    struct myinode *ino = (struct myinode *)malloc(sizeof(struct myinode));
    memset(ino, 0, INODE_SIZE);
    for(int i=1; i<INODE_NO; i++) {
        ino->st_id=i;
        ino->type=FREE;
        ino->st_size=0;
        ino->st_mode=0755;
        ino->st_blocks=0;
        memcpy(fs+(i*INODE_SIZE), ino, INODE_SIZE);
    }

    // printf("\nInitialised rest of the inodes");

    int occupied = 1;

    memcpy(fs+BLOCKSIZE, &occupied, sizeof(int)); //occupied by inode list
    memcpy(fs+BLOCKSIZE+sizeof(int), &occupied, sizeof(int)); //occupied by free block list
    memcpy(fs+BLOCKSIZE+2*sizeof(int), &occupied, sizeof(int)); //occupied by root

    free(ino);
    // printf("\nInitialised free block list");
    return 1;
}

// void main(){
//     printf("Size of myinode: %d", sizeof(struct myinode));
//     printf("\nSize of mydirent: %d", sizeof(struct mydirent));
//     printf("\nNo. of inodes: %f", 4000.0/sizeof(struct myinode));
//     printf("\nIt only prints");

//     // openfile();

//     fs = malloc(BLOCK_NO*BLOCKSIZE);
//     root = (struct myinode *)malloc(sizeof(struct myinode));
//     // printf("Made it here");
//     init_fs();
//     read_inode(0);
//     // testfs();
//     read_dirent(2);
//     free_blocks();
//     free(root);
// }
