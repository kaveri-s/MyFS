
#include <errno.h>
#include <stdlib.h>
#include <string.h>

#include "try.c"

// Node Functions

int getnodebypath(const char *path, struct myinode *parent, struct myinode *node) {
  if(!S_ISDIR(parent->st_mode)) {
    errno = ENOTDIR;
    return 0;
  }

  if(path[1] == '\0') {
    node = parent;
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
  struct myinode *child;
  if(!dir_find(parent, name, len, child)) {
    errno = ENOENT;
    return 0;
  }

  if(*end == '\0') {
    // Last node in path
    memcpy(node, fs[dirent->direct_blk[0]*INODE_SIZE]);
    return 1;
  } else {
    // Not the last node in path (or a trailing slash)
    return getnodebypath(end, dirent->node, node);
  }
}


//Directory Functions
int dir_add(struct myinode *dirnode, struct mydirent *entry) {
  struct mydirent *dir = (struct mydirent *) fs[parent->direct_blk[0]];
  struct myinode *existing_entry;

  if(dir_find(dirnode, entry->name, strlen(entry->name), existing_entry)) {
    errno = EEXIST;
    return 0;
  }

  if(*dir == NULL) {
    *dir = entry;
    entry->next = NULL;
  } else {
    entry->next = *dir;
    *dir = entry;
  }

  entry->node->mystat.st_nlink++;

  if(S_ISDIR(entry->node->mystat.st_mode)) {
    dirnode->mystat.st_nlink++;
  }

  return 1;
}

int dir_add_alloc(struct myinode *parent, const char *name, struct myinode *child) {
  
  if(!dir_add(parent, entry)) {
    free(entry);
    return 0;
  }
  
  return 1;
}

int dir_remove(struct myinode *dirnode, const char *name) {
  struct mydirent **dir = (struct mydirent **) &dirnode->data;

  struct mydirent *ent = *dir;
  struct mydirent **ptr = dir;

  while(ent != NULL) {
    if(strcmp(ent->name, name) == 0) {
      *ptr = ent->next;

      if(S_ISDIR(ent->node->mystat.st_mode)) {
        dirnode->mystat.st_nlink--;
      }

      free(ent);

      return 1;
    }

    ptr = &ent->next;
    ent = ent->next;
  }

  errno = ENOENT;

  return 0;
}

int dir_find(struct myinode *parent, const char *name, int namelen, struct myinode *child) {
  struct mydirent *ent = (struct mydirent *) fs[parent->direct_blk[0]]; //if it is a directory inode it should point to a dirent structure

  for(int i=2;i<59;i++) {
    if(ent->sub_id[i]!=-1){
      if(len(ent->subs[i]==namelen){
        if(strncmp(ent->subs[i], name, namelen)) {
          memcpy(child, fs[ent->sub_id[i]*INODE_SIZE], INODE_SIZE);
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

