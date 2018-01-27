/* This file contains all the structures required to create a file system*/


#include <fuse.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/time.h>


struct file_system_type {
    const char *name;
    int fs_flags;
    struct super_block *get_sb;
    void (*kill_sb);
    struct module *owner;
    struct file_system_type *next;
    struct list_head fs_supers;
};

struct vfsmount {
        struct list_head mnt_hash;
        struct vfsmount *mnt_parent;    /* fs we are mounted on */
        struct dentry *mnt_mountpoint;  /* dentry of mountpoint */
        struct dentry *mnt_root;        /* root of the mounted tree */
        struct super_block *mnt_sb;     /* pointer to superblock */
        struct list_head mnt_mounts;    /* list of children, anchored here */
        struct list_head mnt_child;     /* and going through their mnt_child */
        atomic_t mnt_count;
        int mnt_flags;
        char *mnt_devname;              /* Name of device e.g. /dev/dsk/hda1 */
        struct list_head mnt_list;
};

struct super_block {
        struct list_head        s_list;
        dev_t                   s_dev;
        unsigned char           s_blocksize_bits;
        unsigned long           s_blocksize;
        unsigned long           s_old_blocksize;
        unsigned char           s_dirt;
        unsigned long long      s_maxbytes;
        struct file_system_type *s_type;
        const struct super_operations   *s_op;
        const struct dquot_operations   *dq_op;
        const struct quotactl_ops       *s_qcop;
        const struct export_operations *s_export_op;
        unsigned long           s_flags;
        unsigned long           s_magic;
        struct dentry           *s_root;
        int                     s_count;
        atomic_t                s_active;
        struct list_head        s_dirty;
        struct list_head        s_io;
        struct hlist_head       s_anon;
        struct list_head        s_files;
        struct block_device     *s_bdev;
        struct hlist_node       s_instances;
        struct quota_info       s_dquot;
        char                    s_id[32];
        void                    *s_fs_info;
 };


struct stat {
    unsigned int     st_dev;         /* ID of device containing file */
    unsigned int     st_ino;         /* inode number */
    unsigned int    st_mode;        /* protection */
    unsigned int   st_nlink;       /* number of hard links */
	unsigned int     st_uid;         /* user ID of owner */
	unsigned int     st_gid;         /* group ID of owner */
    unsigned int     st_rdev;        /* device ID (if special file) */
    unsigned int     st_size;        /* total size, in bytes */
    unsigned int st_blksize;     /* blocksize for filesystem I/O */
    unsigned int  st_blocks;      /* number of 512B blocks allocated */
    unsigned int st_atim;  /* time of last access */
    unsigned int st_mtim;  /* time of last modification */
	unsigned int st_ctim;  /* time of last status change */
};