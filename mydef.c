
#include <errno.h>
#include <stdlib.h>
#include <string.h>

#include "mydef.h"
#include "try.h"

// Node Functions

int getnodebypath(const char *path, struct myinode *parent, struct myinode *child) {
  if(!S_ISDIR(parent->st_mode)) {
    errno = ENOTDIR;
    return 0;
  }

  if(path[1] == '\0') {
    memcpy(child, parent, INODE_SIZE);
    return 1;
  }

  // Extract name from path
  const char *name = path + 1;
  int len = 0;
  const char *end = name;
  while(*end != '\0' && *end != '/') {
    end++;
    len++;
  }

  // Search directory
  struct myinode *node = (struct myinode *)malloc(sizeof(struct myinode));
  if(!dir_find(parent, name, len, node)) {
    errno = ENOENT;
    return 0;
  }

  if(*end == '\0') {
    // Last node in path
    memcpy(child, node, INODE_SIZE);
    free(node);
    return 1;
  } else {
    // Not the last node in path (or a trailing slash)
    return getnodebypath(end, node, child);
  }
}


//Directory Functions
int dir_add(ino_t pi_id, ino_t ci_id, int blk, char *name) {
  
  struct mydirent *dir = (struct mydirent *)malloc(sizeof(struct mydirent));

  strcpy(dir->name, name);

  //set self and parent inodes
  strncpy(dir->subs[0], ".", 1);
  dir->sub_id[0] = pi_id;
  
  strncpy(dir->subs[1], "..", 2);
  dir->sub_id[1] = ci_id;

  //sub_id of -1 means that the directory can map a file name and id at that index
  for(int i=2;i<SUB_NO; i++) {
    dir->sub_id[i] = -1;
  }

  memcpy(fs+blk*BLOCKSIZE, dir, BLOCKSIZE);
  free(dir);

  return 1;
}

int dir_add_alloc(struct myinode *parent, const char *name, struct myinode *child) {
  
  struct mydirent *pdir = (struct mydirent *) fs+parent->direct_blk[0]*BLOCKSIZE;

  if(strlen(name)>MAX_NAME_LEN) {
    errno = ENAMETOOLONG;
    return 0;
  }

  for(int i=2;i<59;i++) { //search all subs from 2 (0 and 1 are for . and ..)
    if(pdir->sub_id[i]==-1) { //if the directory can hold more files/dirs
      strcpy(pdir->subs[i], name); //copy the name of the file/dir
      pdir->sub_id[i]=child->st_id; //map the name to inode of file/dir
      return 1;
    }
  }

  errno = EMLINK; //directory cannot hold more files/dirs

  return 0;

}

int dir_remove(struct myinode *parent, struct myinode *child, const char *name) {
  struct mydirent *pdir = (struct mydirent *) (fs+parent->direct_blk[0]);

  if(S_ISDIR(child->st_mode)) {
    struct mydirent *cdir = (struct mydirent *) (fs+child->direct_blk[0]);

    for(int i=2; i<SUB_NO; i++) { //check if directory is empty
      if(cdir->sub_id[i]!=-1) {
        errno = ENOTEMPTY;
        return 0;
      }
    }
  }

  for(int i=0; i<child->st_blocks; i++) {
    memset(fs+child->direct_blk[i]*BLOCKSIZE, 0, BLOCKSIZE); //clear the child's directory entry/file contents
  }

  for(int i=2;i<SUB_NO;i++) { //reset child's name and inode id in the parent dirent
    if(strcmp(name, pdir->subs[i])==0) {
      memset(pdir->subs[i], 0, MAX_NAME_LEN);
      pdir->sub_id[i]=-1;
      return 0;
    }
  }

  errno = ENOENT;

  return 0;
}

int dir_find(struct myinode *parent, const char *name, int namelen, struct myinode *child) {
  struct mydirent *pdir = (struct mydirent *) (fs+parent->direct_blk[0]); //get dirent of parent

  for(int i=2;i<59;i++) { //search all subs from 2 (0 and 1 are for . and ..)
    if(pdir->sub_id[i]!=-1) { //if it maps to a valid inode
      if(strlen(pdir->subs[i])==namelen) { //if the length of the sub_name matches namelen
        if(strncmp(pdir->subs[i], name, namelen)==0) { //if sub_name matches name
          int idchild = pdir->sub_id[i]; //get inode_id at that index
          memcpy(child, fs+idchild*INODE_SIZE, INODE_SIZE); //retrieve child's inode information from fs
          return 1;
        }
      }
    }
  }

  errno = ENOENT;

  return 0;
}

// Utility functions

char * get_dirname(const char *msg) {
  char *buf = strdup(msg);
  char *dir = dirname(buf);
  char *res = strdup(dir);
  free(buf);
  return res;
}

char * get_basename(const char *msg) {
  char *buf = strdup(msg);
  char *nam = basename(buf);
  char *res = strdup(nam);
  free(buf);
  return res;
}

