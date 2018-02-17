
#include <libgen.h>
#include <linux/limits.h>
#include <sys/stat.h>

struct filehandle {
  struct myinode *node;
  int o_flags;
};

#ifndef _NODE_H
#define _NODE_H


struct myinode {
  struct stat   mystat;
  void         *data;
};

int getnodebypath(const char *path, struct myinode *root, struct myinode **node);

#endif

#ifndef _DIR_H
#define _DIR_H

struct mydirent {
  char             name[PATH_MAX];
  struct myinode     *node;
  struct mydirent *next;
};

int dir_add(struct myinode *dir, struct mydirent *entry);

int dir_add_alloc(struct myinode *dir, const char *name, struct myinode *node);

int dir_remove(struct myinode *dir, const char *name);

int dir_find(struct myinode *dir, const char *name, int namelen, struct mydirent **entry);

#endif

// Utility Functions
char * get_dirname(const char *msg);
char * get_basename(const char *msg);




