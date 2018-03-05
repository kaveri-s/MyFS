#define FUSE_USE_VERSION 26

#include <fuse.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <errno.h>
#include <sys/time.h>
#include <mcheck.h>

// #include "mydef.h"

#include "try.h"
#include "mydef.h"

static void set_stat(struct myinode *node, struct stat *stbuf) {

  stbuf->st_mode   = node->st_mode;
  stbuf->st_nlink  = node->st_nlink;
  stbuf->st_size   = node->st_size;
  stbuf->st_blocks = node->st_blocks;
  stbuf->st_uid    = node->st_uid;
  stbuf->st_gid    = node->st_gid;
  stbuf->st_mtime  = node->st_mtim;
  stbuf->st_atime  = node->st_atim;
  stbuf->st_ctime  = node->st_ctim;

}

static void set_time(struct myinode *node, int param) {

  time_t now = time(0);
  if(param & AT) node->st_atim = now;
  if(param & CT) node->st_ctim = now;
  if(param & MT) node->st_mtim = now;

}

static int initstat(struct myinode *node, mode_t mode) {
  struct stat *stbuf = (struct stat*)malloc(sizeof(struct stat));
  memset(stbuf, 0, sizeof(struct stat));
  node->st_mode = mode;
  node->st_uid = getuid();
  node->st_gid = getgid();
  node->st_blocks = 0;
  node->st_nlink = 0;
  set_time(node, AT | MT | CT);
  free(stbuf);
  return 1;
}

static int inode_entry(const char *path, mode_t mode) {

  const int full=1, empty=0;
  
  struct myinode *node = (struct myinode *)malloc(sizeof(struct myinode));
  
  if(getnodebypath(path, root, node)) {
    errno = EEXIST;
    free(node);
    return -errno;
  }

  //Find parent
  char *dirpath = get_dirname(path);

  struct myinode *parent = (struct myinode *)malloc(sizeof(struct myinode)); 
  if(!getnodebypath(dirpath, root, parent)) {
    free(dirpath);
    free(parent);
    free(node);
    return -errno;
  }
  free(dirpath);

  //Check for free blocks
  int blk = 0;
  for(int b=3; b<BLOCK_NO; b++) {
    if(memcmp(fs+BLOCKSIZE+b*sizeof(int), &empty, sizeof(int))==0) {
      blk = b;
      break;
    }
  }

  if(blk == 0){
    errno = ENOMEM;
    free(node);
    free(parent);
    return -errno;
  }

  //Check for free inodes
  for(int i=1; i<INODE_NO; i++) {
    memcpy(node, (struct myinode *)(fs+i*INODE_SIZE), INODE_SIZE);
    if(node->type==FREE) {

      if(!initstat(node, mode)) {
        free(node);
        free(parent);
        return -errno;
      }

      // read_inode(parent->st_id);

      // Add entry in parent directory
      if(!dir_add_alloc(parent, get_basename(path), node)) {
        free(node);
        free(parent);
        return -errno;
      }

      else {
        if(S_ISDIR(node->st_mode)) {
          if(dir_add(parent->st_id, node->st_id, blk, get_basename(path))) {
            node->st_size=BLOCKSIZE;
            node->st_nlink=2;
            node->type = DIRECTORY;
            parent->st_nlink++;
            memcpy(fs+BLOCKSIZE+blk*sizeof(int), &full, sizeof(int));
          }
          else {
            free(node);
            free(parent);
            return -errno;
          }
        }
        else {
          node->st_nlink = 1;
          node->type = ORDINARY;
          memcpy(fs+BLOCKSIZE+blk*sizeof(int), &full, sizeof(int));
        }
        node->direct_blk[0]=blk;
        node->st_blocks=1;
        set_time(node, AT|CT|MT);
        set_time(parent, AT|MT);
        memcpy(fs+node->st_id*INODE_SIZE, node, INODE_SIZE);
        memcpy(fs+parent->st_id*INODE_SIZE, parent, INODE_SIZE);
        free(node);
        free(parent);
        return 0;
      }
    }
  }

  free(node);
  free(parent);
  
  return -ENOMEM;

}


// Fuse Operations

static int fs_flush(const char *path, struct fuse_file_info *fi) {
	return 0;
}

static int fs_utimens(const char *path, const struct timespec tv[2]) {
  struct myinode *node = (struct myinode *)malloc(sizeof(struct myinode));
  memset(node, 0, INODE_SIZE);
  if(!getnodebypath(path, root, node)) {
    return -errno;
  }

  set_time(node, AT|CT|MT);

	return 0;
}

static int fs_getattr(const char *path, struct stat *stbuf) {
  struct myinode *node = (struct myinode *)malloc(sizeof(struct myinode));
  memset(node, 0, INODE_SIZE);
  if(!getnodebypath(path, root, node)) {
    return -errno;
  }

  set_stat(node, stbuf);

  // read_inode(node->st_id);

  return 0;
}

static int fs_readdir(const char *path, void *buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi) {
  struct myinode *current = (struct myinode *)malloc(sizeof(struct myinode));

  if(!getnodebypath(path, root, current)) {
    return -errno;
  }

  struct stat *cst = (struct stat *)malloc(sizeof(struct stat));
  set_stat(current, cst);

  filler(buf, ".",  cst, 0);

  if(current == root) {
    filler(buf, "..", cst, 0);
  } else {
    char *parent_path = get_dirname(path);
    struct myinode *parent = (struct myinode *)malloc(sizeof(struct myinode)); ;
    getnodebypath(parent_path, root, parent);
    // read_inode(parent->st_id);
    struct stat *pst = (struct stat *)malloc(sizeof(struct stat));
    set_stat(parent, pst);
    free(parent_path);
    filler(buf, "..", pst, 0);
  }

  struct mydirent *entry = (struct mydirent *) (fs+current->direct_blk[0]*BLOCKSIZE);
  for(int i=2; i<SUB_NO; i++) {
    if(entry->sub_id[i] != -1) {
      struct myinode *child = (struct myinode *)malloc(sizeof(struct myinode));
      memset(child, 0, INODE_SIZE);
      memcpy(child, fs+(entry->sub_id[i])*INODE_SIZE, INODE_SIZE);
      struct stat *stbuf = (struct stat *)malloc(sizeof(struct stat));
      memset(stbuf, 0, sizeof(struct stat));
      set_stat(child, stbuf);
      filler(buf, entry->subs[i], stbuf, 0);
    }
  }

  return 0;
}

static int fs_mknod(const char *path, mode_t mode, dev_t rdev) {
  // struct myinode *node;
  int res = inode_entry(path, mode);
  if(res) return res;

  return 0;
}

static int fs_mkdir(const char *path, mode_t mode) {
  // struct myinode *node;
  int res = inode_entry(path, S_IFDIR | mode);
  if(res) return res;

  return 0;
}

static int fs_unlink(const char *path) {
  char *parent_path, *name;
  struct myinode *parent = (struct myinode *)malloc(sizeof(struct myinode)); 
  struct myinode *child = (struct myinode *)malloc(sizeof(struct myinode)); 

  if(!getnodebypath(path, root, child)) {
    return -errno;
  }

  parent_path = get_dirname(path);
  if(!getnodebypath(parent_path, root, parent)) {
    free(parent_path);
    return -errno;
  }
  free(parent_path);

  name = get_basename(path);

  if(!dir_remove(parent, child, name)) {
    free(name);
    return -errno;
  }

  child->type = FREE;
  child->st_blocks = 0;
  child->st_size = 0;
  child->st_nlink = 0;
  parent->st_nlink--;

  memcpy(fs+parent->st_id*INODE_SIZE, parent, INODE_SIZE);
  memcpy(fs+child->st_id*INODE_SIZE, child, INODE_SIZE);

  free(name);

  return 0;
}

static int fs_rmdir(const char *path) {
  char *parent_path, *name;
  struct myinode *parent = (struct myinode *)malloc(sizeof(struct myinode)); 
  struct myinode *child = (struct myinode *)malloc(sizeof(struct myinode));

  if(!getnodebypath(path, root, child)) {
    return -errno;
  }

  parent_path = get_dirname(path);

  if(!getnodebypath(parent_path, root, parent)) {
    free(parent_path);
    return -errno;
  }
  free(parent_path);

  name = get_basename(path);

  if(!dir_remove(parent, child, name)) {
    free(name);
    return -errno;
  }

  child->type = FREE;
  child->st_blocks = 0;
  child->st_size = 0;
  child->st_nlink = 0;
  parent->st_nlink--;

  memcpy(fs+child->st_id*INODE_SIZE, child, INODE_SIZE);
  memcpy(fs+parent->st_id*INODE_SIZE, parent, INODE_SIZE);

  free(name);

  free(parent);
  free(child);

  return 0;
}

static int fs_creat(const char *path, mode_t mode, struct fuse_file_info *fi) {

  int res = inode_entry(path, mode);

  struct myinode *node = (struct myinode *)malloc(sizeof(struct myinode));
  memset(node, 0, INODE_SIZE);

  if(!getnodebypath(path, root, node)) {
    return -errno;
  }

  //Store filehandle information in fuse
  struct filehandle *fh = malloc(sizeof(struct filehandle));
  fh->node    = node;
  fh->o_flags = fi->flags;
  fi->fh = (uint64_t) fh;

  if(res) return res;

  return 0;
}

static int fs_open(const char *path, struct fuse_file_info *fi) {
  struct myinode *node = (struct myinode *)malloc(sizeof(struct myinode));
  memset(node, 0, INODE_SIZE);

  if(!getnodebypath(path, root, node)) {
    return -errno;
  }

  set_time(node, AT);

  //Store filehandle information in fuse
  struct filehandle *fh = malloc(sizeof(struct filehandle));
  fh->node    = node;
  fh->o_flags = fi->flags;
  fi->fh = (uint64_t) fh;

  return 0;
}

static int fs_truncate(const char *path, off_t size) {
  int full=1, empty=0;

  struct myinode *node = (struct myinode *)malloc(sizeof(struct myinode));
  memset(node, 0, INODE_SIZE);

  if(!getnodebypath(path, root, node)) {
    return -errno;
  }

  // Calculate new block count
  blkcnt_t req_blocks = (size + BLOCKSIZE - 1) / BLOCKSIZE;

  if(req_blocks>MAX_MAP || MAX_MAP*BLOCKSIZE < size) { //last byte of file should be used to store NULL
    errno = EFBIG;
    return -errno;
  }

  blkcnt_t oldblkcnt = node->st_blocks;

  if(node->st_blocks < req_blocks) {

    blkcnt_t extra = req_blocks - node->st_blocks;
    int blks[extra];
    for(int i=0; i<extra; i++){
      int blk=0;
      for(int b=3;b<BLOCK_NO; b++) {
        if(memcmp(fs+BLOCKSIZE+b*sizeof(int), &empty, sizeof(int))==0)
          blk = b;
      }
      if(blk == 0){
        errno = ENOMEM;
        return -errno;
      }
      blks[i]=blk;
    }
    for(int i=0;i<extra;i++) {
      node->direct_blk[node->st_blocks+i]=blks[i];
      memcpy(fs+BLOCKSIZE+blks[i]*sizeof(int), &full, sizeof(int));
    }
  } else if(oldblkcnt > req_blocks) {

    for(int i=req_blocks;i<oldblkcnt;i++) {
      memcpy(fs+BLOCKSIZE+node->direct_blk[i]*sizeof(int), &empty, sizeof(int));
      memset(fs+node->direct_blk[i]*BLOCKSIZE, 0, BLOCKSIZE);
      node->direct_blk[i]=0;
    }
  }

  // Update file size
  node->st_size = size;
  node->st_blocks = req_blocks;

  set_time(node, CT | MT);

  memcpy(fs+node->st_id*INODE_SIZE, node, INODE_SIZE);


  return 0;
}

static int fs_read(const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info *fi) {
  struct filehandle *fh = (struct filehandle *) fi->fh;

  off_t filesize = fh->node->st_size;

  if(offset >= filesize) {
    return 0;
  }

  size_t avail = filesize - offset;
  size_t n = (size < avail) ? size : avail;

  int bblk = (int)offset/BLOCKSIZE;
  int eblk = (int)(offset+size)/BLOCKSIZE;
  int end=0;

  for(int i=bblk; i<=eblk; i++) {
    int roff = (i==bblk)?offset-bblk*BLOCKSIZE:0;
    int rsize = (i==eblk)?((offset+size)-eblk*BLOCKSIZE):BLOCKSIZE;
    memcpy(buf+end, (char *)(fs+fh->node->direct_blk[i]*BLOCKSIZE) + roff, rsize);
    end=rsize;
  }
  set_time(fh->node, AT);

  return n;
}

static int fs_write(const char *path, const char *buf, size_t size, off_t offset, struct fuse_file_info *fi) {

  const int full=1, empty=0;

  struct filehandle *fh = (struct filehandle *) fi->fh;

  struct myinode *node = (struct myinode *)fh->node;

  blkcnt_t req_blocks = (offset + size + BLOCKSIZE - 1) / BLOCKSIZE;

  if(req_blocks>3 || MAX_MAP*BLOCKSIZE < offset+size) { //last byte of file should be used to store NULL
    errno = EFBIG;
    return -errno;
  }

  if(node->st_blocks < req_blocks) {

    blkcnt_t extra = req_blocks - node->st_blocks;
    int blks[extra];
    for(int i=0; i<extra; i++){
      int blk=0;
      for(int b=3;b<BLOCK_NO; b++) {
        if(memcmp(fs+BLOCKSIZE+b*sizeof(int), &empty, sizeof(int))==0)
          blk = b;
      }
      if(blk == 0){
        errno = ENOMEM;
        return -errno;
      }
      blks[i]=blk;
    }
    for(int i=0;i<extra;i++) {
      node->direct_blk[node->st_blocks+i]=blks[i];
      memcpy(fs+BLOCKSIZE+blks[i]*sizeof(int), &full, sizeof(int));
    }
    node->st_blocks+=extra;
  }


  int bblk = (int)offset/BLOCKSIZE;
  int eblk = (int)(offset+size)/BLOCKSIZE;
  int end=0;

  for(int i=bblk; i<=eblk; i++) {
    int roff = (i==bblk)?offset-bblk*BLOCKSIZE:0;
    int rsize = (i==eblk)?((offset+size)-eblk*BLOCKSIZE):BLOCKSIZE;
    memcpy(fs + (node->direct_blk[i]*BLOCKSIZE) + roff, buf+end, rsize);
    end=rsize;
  }

  off_t minsize = offset + size;
  if(minsize > node->st_size) {
    node->st_size = minsize;
  }

  set_time(node, CT | MT);

  // for(int i=0;i<BLOCKSIZE*BLOCK_NO;i++) {
  //   printf("%c", fs[i]);
  // }

  memcpy(fs+node->st_id*INODE_SIZE, node, INODE_SIZE);

  return size;
}

static int fs_release(const char *path, struct fuse_file_info *fi) {
  struct filehandle *fh = (struct filehandle *) fi->fh;

  // If the file was deleted but we could not free it due to open file descriptors,
  // free the node and its data after all file descriptors have been closed.
  free(fh->node);

  // Free "file handle"
  free(fh);

  return 0;
}

static struct fuse_operations fs_oper = {
  .getattr      = fs_getattr,
  .readdir      = fs_readdir,
  .mknod        = fs_mknod,
  .mkdir        = fs_mkdir,
  .unlink       = fs_unlink,
  .rmdir        = fs_rmdir,
  .create        = fs_creat,
  .open         = fs_open,
  .read         = fs_read,
  .write        = fs_write,
  .flush        = fs_flush,
  .utimens      = fs_utimens,
  .truncate     = fs_truncate,
  .release      = fs_release
};


//
// Application entry point
//

int main(int argc, char *argv[]) {
  // mtrace();

  fs = malloc(BLOCK_NO*BLOCKSIZE);
  memset(fs, 0, BLOCK_NO*BLOCKSIZE);

  // Initialize root directory
  root = (struct myinode *)malloc(sizeof(struct myinode));
  memset(root, 0, INODE_SIZE);

  openfile();

  set_time(root, AT|CT|MT);
  // free_blocks();
  // read_inode(root->st_id);
  // read_dirent(root->direct_blk[0]);
  // printf("Done.");
  // const char *path = "/hello";
  // printf("%d", fs_mkdir(path, S_IFDIR|0755));
  // root->st_uid = getuid();
  // root->st_gid = getgid();

  // No entries
  umask(0);

  return fuse_main(argc, argv, &fs_oper, NULL);
  
  // return 1;
}

