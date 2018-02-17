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
  set_stat(node, stbuf);
  memset(stbuf, 0, sizeof(struct stat));
  stbuf->st_mode  = mode;
  stbuf->st_nlink = 0;
  set_time(node, AT | MT | CT);
  return 1;
}

static int inode_entry(const char *path, mode_t mode, struct myinode *newnode) {
  
  //Find parent
  char *dirpath = get_dirname(path);

  struct myinode *parent;
  if(!getnodebypath(dirpath, root, parent)) {
    free(dirpath);
    return -errno;
  }
  free(dirpath);

  struct myinode *node = (struct myinode *)malloc(sizeof(struct myinode));

  //Check for free inodes
  for(int i=1; i< BLOCK_NO; i++) {
    memcpy(node, fs[i*INODE_SIZE], INODE_SIZE);
    if(node->type==FREE) {
      node->st_uid = getuid();
      node->st_gid = getgid();
      node->type = ORDINARY;

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
        memcpy(newnode, node, INODE_SIZE);
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
  if(!getnodebypath(path, root, &node)) {
    return -errno;
  }

  stbuf->st_mode   = node->st_mode;
  stbuf->st_nlink  = node->st_nlink;
  stbuf->st_size   = node->st_size;
  stbuf->st_blocks = node->st_blocks;
  stbuf->st_uid    = node->st_uid;
  stbuf->st_gid    = node->st_gid;
  stbuf->st_mtime  = node->st_mtim;
  stbuf->st_atime  = node->st_atim;
  stbuf->st_ctime  = node->st_ctim;

  // Directories contain the implicit hardlink '.'
  if(S_ISDIR(node->st_mode)) {
    stbuf->st_nlink++;
  }

  return 0;
}

static int fs_readdir(const char *path, void *buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi) {
  struct myinode *dir;

  
  if(!getnodebypath(path, root, dir)) {
    return -errno;
  }

  filler(buf, ".",  dir->st, 0);
  if(dir == root) {
    filler(buf, "..", NULL, 0);
  } else {
    char *parent_path = get_dirname(path);
    struct myinode *parent;
    getnodebypath(parent_path, root, &parent);
    free(parent_path);
    filler(buf, "..", &parent->st, 0);
  }

  struct mydirent *entry = dir->data;
  while(entry != NULL) {
    if(filler(buf, entry->name, &entry->node->st, 0))
      break;
    entry = entry->next;
  }

  return 0;
}

static int fs_mknod(const char *path, mode_t mode, dev_t rdev) {
  struct myinode *node;
  int res = inode_entry(path, mode, node);
  if(res) return res;

  node->st_blocks = 0;

  return 0;
}

static int fs_mkdir(const char *path, mode_t mode) {
  struct myinode *node;
  int res = inode_entry(path, S_IFDIR | mode, &node);
  if(res) return res;

  node->data = NULL;

  return 0;
}

static int fs_unlink(const char *path) {
  char *dirpath, *name;
  struct myinode *dir, *node;

  if(!getnodebypath(path, root, &node)) {
    return -errno;
  }

  dirpath = get_dirname(path);
  if(!getnodebypath(dirpath, root, &dir)) {
    free(dirpath);
    return -errno;
  }
  free(dirpath);

  name = get_basename(path);

  if(!dir_remove(dir, name)) {
    free(name);
    return -errno;
  }
  free(name);

  if(node->st_nlink == 0) {
    if(node->data) free(node->data);
    free(node);
  }

  return 0;
}

static int fs_rmdir(const char *path) {
  char *dirpath, *name;
  struct myinode *dir, *node;

  if(!getnodebypath(path, root, &node)) {
    return -errno;
  }

  dirpath = get_dirname(path);

  if(!getnodebypath(dirpath, root, &dir)) {
    free(dirpath);
    return -errno;
  }
  free(dirpath);

  name = get_basename(path);

  if(!dir_remove(dir, name)) {
    free(name);
    return -errno;
  }
  free(name);

  free(node);

  return 0;
}

static int fs_open(const char *path, struct fuse_file_info *fi) {
  struct myinode *node;
  if(!getnodebypath(path, root, &node)) {
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

