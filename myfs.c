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

// #include "mydef.h"

#include "try.c"
#include "mydef.c"

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
  set_stat(node, stbuf);
  stbuf->st_mode  = mode;
  stbuf->st_nlink = 0;
  set_time(node, AT | MT | CT);
  return 1;
}

static int inode_entry(const char *path, mode_t mode) {
  
  struct myinode *node = (struct myinode *)malloc(sizeof(struct myinode));
  
  if(getnodebypath(path, root, node)) {
    errno = EEXIST;
    return -errno;
  }

  //Find parent
  char *dirpath = get_dirname(path);

  struct myinode *parent;
  if(!getnodebypath(dirpath, root, parent)) {
    free(dirpath);
    return -errno;
  }
  free(dirpath);

  //Check for free blocks
  int blk = 0;
  for(int b=3;b<BLOCK_NO; b++) {
    if(fs[BLOCKSIZE+b]==0)
      blk = b;
  }

  if(blk == 0){
    errno = ENOMEM;
    return -errno;
  }

  //Check for free inodes
  for(int i=1; i<INODE_NO; i++) {
    memcpy(node, fs[i*INODE_SIZE], INODE_SIZE);
    if(node->type==FREE) {
      node->st_uid = getuid();
      node->st_gid = getgid();
      node->type = ORDINARY;
      node->st_blocks = 0;


      if(!initstat(node, mode)) {
        free(node);
        return -errno;
      }

      // Add entry in parent directory
      if(!dir_add_alloc(parent, get_basename(path), node)) {
        free(node);
        free(parent);
        return -errno;
      }

      else {
        if(S_ISDIR(node->st_mode)) {
          if(!dir_add(parent->st_id, node->st_id, blk, get_basename(path))) {
            fs[BLOCKSIZE+blk]=1;
            free(node);
            free(parent);
            return -errno;
          }
          else {
            node->st_nlink=2;
            node->st_blocks=1;
            parent->st_nlink++;
          }
        }
        else {
          fs[BLOCKSIZE+blk]=1;
          node->direct_blk[0]=blk;
          node->st_blocks=1;
        }
        memcpy(fs[(node->st_id)*INODE_SIZE], node, INODE_SIZE);
        memcpy(fs[(parent->st_id)*INODE_SIZE], parent, INODE_SIZE);
        free(node);
        free(parent);
        return 1;
      }
    }
  }
  
  return -ENOMEM;

}


// Fuse Operations

static int fs_getattr(const char *path, struct stat *stbuf) {
  struct myinode *node;
  if(!getnodebypath(path, root, node)) {
    return -errno;
  }

  set_stat(node, stbuf);

  return 0;
}

static int fs_readdir(const char *path, void *buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi) {
  struct myinode *current;

  
  if(!getnodebypath(path, root, current)) {
    return -errno;
  }

  struct stat *cst = (struct stat *)malloc(sizeof(struct stat));
  set_stat(current, cst);

  filler(buf, ".",  cst, 0);

  if(current == root) {
    filler(buf, "..", NULL, 0);
  } else {
    char *parent_path = get_dirname(path);
    struct myinode *parent;
    getnodebypath(parent_path, root, parent);
    struct stat *pst = (struct stat *)malloc(sizeof(struct stat));
    set_stat(parent, pst);
    free(parent_path);
    filler(buf, "..", pst, 0);
  }

  struct mydirent *entry = (struct mydirent *) fs[current->direct_blk[0]];
  for(int i=2;i<SUB_NO;i++) {
    if(entry->sub_id[i] != -1) {
      struct myinode *child;
      memcpy(child, fs[(entry->sub_id[i])*INODE_SIZE], INODE_SIZE);
      struct stat *stbuf;
      set_stat(child, stbuf);
      if(filler(buf, entry->subs[i], stbuf, 0))
        break;
    }
  }

  return 0;
}

static int fs_mknod(const char *path, mode_t mode, dev_t rdev) {
  struct myinode *node;
  int res = inode_entry(path, mode);
  if(res) return res;

  return 0;
}

static int fs_mkdir(const char *path, mode_t mode) {
  struct myinode *node;
  int res = inode_entry(path, S_IFDIR | mode);
  if(res) return res;

  return 0;
}

static int fs_unlink(const char *path) {
  char *parent_path, *name;
  struct myinode *parent, *child;

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
  free(name);

  return 0;
}

static int fs_rmdir(const char *path) {
  char *parent_path, *name;
  struct myinode *parent, *child;

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
  free(name);

  free(parent);
  free(child);

  return 0;
}

static int fs_creat(const char *path, mode_t mode, struct fuse_file_info *fi) {

  struct myinode *node;
  if(!getnodebypath(path, root, node)) {
    return -errno;
  }

  int res = inode_entry(path, mode);
  if(res) return res;

  return 0;
}

static int fs_open(const char *path, struct fuse_file_info *fi) {
  struct myinode *node;
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

static int fs_read(const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info *fi) {
  struct filehandle *fh = (struct filehandle *) fi->fh;

  off_t filesize = fh->node->st_size;

  if(offset >= filesize) {
    return 0;
  }

  size_t avail = filesize - offset;
  size_t n = (size < avail) ? size : avail;

  memcpy(buf, fh->node->data + offset, n);

  set_time(fh->node, AT);

  return n;
}

static int fs_write(const char *path, const char *buf, size_t size, off_t offset, struct fuse_file_info *fi) {
  struct filehandle *fh = (struct filehandle *) fi->fh;

  struct myinode *node = fh->node;

  blkcnt_t req_blocks = (offset + size + BLOCKSIZE - 1) / BLOCKSIZE;

  if(node->st_blocks < req_blocks) {
    void *newdata = malloc(req_blocks * BLOCKSIZE);
    if(node->data != NULL) {      
      memcpy(newdata, node->data, node->st_size);
      free(node->data);
    }
    node->data = newdata;
    node->st_blocks = req_blocks;
  }

  memcpy(((char *) node->data) + offset, buf, size);

  off_t minsize = offset + size;
  if(minsize > node->st_size) {
    node->st_size = minsize;
  }

  set_time(node, CT | MT);

  return size;
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
  .write        = fs_write
};


//
// Application entry point
//

int main(int argc, char *argv[]) {

  // Initialize root directory
  root = (struct myinode *)malloc(sizeof(struct myinode));

  init_fs();
  root->st_uid = getuid();
  root->st_gid = getgid();

  // No entries
  umask(0);

  return fuse_main(argc, argv, &fs_oper, NULL);
}

