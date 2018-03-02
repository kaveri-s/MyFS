
#include <libgen.h>
#include <linux/limits.h>
#include <sys/stat.h>
#include "try.h"

int getnodebypath(const char *path, struct myinode *parent, struct myinode *child);

int dir_add(ino_t pi_id, ino_t ci_id, int blk, char *name);

int dir_add_alloc(struct myinode *parent, const char *name, struct myinode *child);

int dir_remove(struct myinode *parent, struct myinode *child, const char *name);

int dir_find(struct myinode *parent, const char *name, int namelen, struct myinode *child);

// Utility Functions
char * get_dirname(const char *msg);
char * get_basename(const char *msg);




