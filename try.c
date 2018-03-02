#include "try.h"


int read_inode(ino_t i) {
    struct myinode *ino = (struct myinode *)malloc(sizeof(struct myinode));
    memcpy(ino, fs, INODE_SIZE);
    printf("\nId: %d", ino->st_id);
    printf("\nMode: %d", ino->st_mode);
    printf("\nHard Links: %d", ino->st_nlink);
    printf("\nSize: %d", ino->st_size);
    printf("\nBlocks: %d", ino->st_blocks);
    printf("\nUID: %d", ino->st_uid);
    printf("\nGID: %d", ino->st_gid);
    printf("\nAT: %d", ino->st_atim);
    printf("\nMT: %d", ino->st_mtim);
    printf("\nBT: %d\n", ino->st_ctim);
    for(int i=0; i<ino->st_blocks;i++)
        printf("Block no %d mapped to: %d\n", i, ino->direct_blk[i]);

}

int rootdir(int blk) {
    struct mydirent *dir = (struct mydirent *)malloc(sizeof(struct mydirent));
    memcpy(dir, fs+blk*BLOCKSIZE, BLOCKSIZE);
    printf("Name %s\n", dir->name);
    for(int i=0; i<SUB_NO; i++) {
        printf("Entry: %s\t", dir->subs[i]);
        printf("Inode number: %d\n", dir->sub_id[i]);
    }
}

void openfile() {
    const char *filepath = "/home/kaveri/Desktop/myfs";

    int fd = open(filepath, O_RDWR | O_CREAT | O_TRUNC, (mode_t)0600);
    if (fd == -1) {
        perror("Error opening file for writing");
        exit(EXIT_FAILURE);
    }

    // Stretch the file size to the size of the (mmapped) array of char

    size_t fssize = BLOCK_NO*BLOCKSIZE+1; // + \0 null character

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

    fs = (char *)mmap(0, fssize, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (fs == MAP_FAILED) {
	    close(fd);
	    perror("Error mmapping the file");
	    exit(EXIT_FAILURE);
    }
    printf("fs created");
}

// void testfs() {
//     printf("\nFS contents: \n");
//     for(int i=BLOCKSIZE;i<BLOCKSIZE+INODE_SIZE;i++) {
//         printf("*%d*", fs[i]);
//     }
//     printf("\n");
// }

int make_rootnode(ino_t st_id, file_type type, int blk, mode_t mode){
    // printf("Made it here!!");
    root->st_mode=mode;
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
    strcpy(dir->name, name);
    strcpy(dir->subs[0],".");
    dir->sub_id[0]=root->direct_blk[0];
    for(int i=1; i<SUB_NO; i++)
        dir->sub_id[i]=-1;
    memcpy(fs+root->direct_blk[0]*BLOCKSIZE, dir, BLOCKSIZE);
    return 1;
}

int init_fs() {
    // printf("Made it here");
    make_rootnode(0, DIRECTORY, 2, S_IFDIR | 0755);

    make_rootdir("/");

    printf("Initialised Root");

    struct myinode *ino = (struct myinode *)malloc(sizeof(struct myinode));
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
    memcpy(fs+BLOCKSIZE+1, &occupied, sizeof(int)); //occupied by free block list
    memcpy(fs+BLOCKSIZE+2, &occupied, sizeof(int)); //occupied by root

    // printf("\nInitialised free block list");
    return 1;
}

void main(){
    printf("Size of myinode: %d", sizeof(struct myinode));
    printf("\nSize of mydirent: %d", sizeof(struct mydirent));
    printf("\nNo. of inodes: %f", 4000.0/sizeof(struct myinode));
    printf("\nIt only prints");

    // openfile();

    fs = malloc(BLOCK_NO*BLOCKSIZE);
    root = (struct myinode *)malloc(sizeof(struct myinode));
    // printf("Made it here");
    init_fs(root);
    read_inode(0);
    // testfs();
    rootdir(2);
    free(root);
}
