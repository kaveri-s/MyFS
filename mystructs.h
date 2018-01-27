/* This file contains all the structures required to create a file system*/


#include <fuse.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/time.h>

struct stat {
    unsigned int    st_dev;         /* ID of device containing file */
    unsigned int    st_ino;         /* inode number */
    unsigned int    st_mode;        /* protection */
	unsigned int    st_uid;         /* user ID of owner */
	unsigned int    st_gid;         /* group ID of owner */
    unsigned int    st_rdev;        /* device ID (if special file) */
    unsigned int    st_size;        /* total size, in bytes */
    unsigned int    st_blksize;     /* blocksize for filesystem I/O */
    unsigned int    st_blocks;      /* number of 512B blocks allocated */
    unsigned int    st_atim;  /* time of last access */
    unsigned int    st_mtim;  /* time of last modification */
	unsigned int    st_ctim;  /* time of last status change */
};

struct file_system_type {
        const char *name;
        int fs_flags;
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

struct qstr {
        const unsigned char *name;
        unsigned int len;
};

struct super_block {
        dev_t s_dev;
        unsigned long           s_blocksize;
        struct file_system_type *s_type;
        struct dentry           *s_root;
        struct list_head        s_inodes;
        //fmode_t                 s_mode;
        //char                    *s_options;
 };

struct super_operations {
        struct inode *(*alloc_inode)(struct super_block *sb);
        void (*destroy_inode)(struct inode *);
        void (*read_inode) (struct inode *);
        void (*write_inode) (struct inode *, int);
        void (*delete_inode) (struct inode *);
        void (*write_super) (struct super_block *);
        int (*statfs) (struct super_block *, struct statfs *);
        int (*remount_fs) (struct super_block *, int *, char *);
        void (*clear_inode) (struct inode *);
};

struct inode {
        unsigned long i_ino;
        umode_t i_mode;
        uid_t i_uid;
        gid_t i_gid;
        kdev_t i_rdev;
        loff_t i_size;
        char date[8] i_atime;
        char date[8] i_ctime;
        char date[8] i_mtime;
        struct super_block *i_sb;
        struct address_space *i_mapping;
        struct list_head i_dentry;
}

struct inode_operations {
        int (*create) (struct inode *, struct dentry *, int);
        struct dentry * (*lookup) (struct inode *, struct dentry *);
        int (*link) (struct dentry *, struct inode *, struct dentry *);
        int (*unlink) (struct inode *, struct dentry *);
        //int (*symlink) (struct inode *, struct dentry *, const char *);
        int (*mkdir) (struct inode *, struct dentry *, int);
        int (*rmdir) (struct inode *, struct dentry *);
        int (*mknod) (struct inode *, struct dentry *, int, dev_t);
        //int (*rename) (struct inode *, struct dentry *, struct inode *, struct dentry *);
        int (*readlink) (struct dentry *, char *,int);
        //int (*follow_link) (struct dentry *, struct nameidata *);
        void (*truncate) (struct inode *);
        int (*permission) (struct inode *, int);
        int (*setattr) (struct dentry *, struct iattr *);
        int (*getattr) (struct vfsmount *mnt, struct dentry *, struct kstat *);
};

struct dentry {
        struct inode *d_inode;
        struct dentry *d_parent;
        struct qstr d_name;
        struct super_block *d_sb;
        struct list_head d_subdirs;
};


struct dentry_operations {
        int (*d_revalidate)(struct dentry *, int);
        int (*d_hash) (struct dentry *, struct qstr *);
        int (*d_compare) (struct dentry *, struct qstr *, struct qstr *);
        int (*d_delete)(struct dentry *);
        void (*d_release)(struct dentry *);
        void (*d_iput)(struct dentry *, struct inode *);
};

struct file {
        struct dentry *f_dentry;
        struct vfsmount *f_vfsmnt;
        mode_t f_mode;
        loff_t f_pos;
        unsigned int f_uid, f_gid;
        unsigned long f_version;
};

struct file_operations {
        struct module *owner;
        loff_t (*llseek) (struct file *, loff_t, int);
        ssize_t (*read) (struct file *, char *, size_t, loff_t *);
        ssize_t (*write) (struct file *, const char *, size_t, loff_t *);
        int (*readdir) (struct file *, void *, filldir_t);
        //int (*mmap) (struct file *, struct vm_area_struct *);
        int (*open) (struct inode *, struct file *);
        int (*release) (struct inode *, struct file *);
};

struct fs_struct { 
    int         users;    /* user count */ 
    int         umask;    /* umask */ 
    int         in_exec;  /* currently executing a file */ 
    struct path root;     /* root directory */ 
    struct path pwd;      /* current working directory */
};

struct nameidata {
        struct dentry *dentry;
        struct vfsmount *mnt;
        struct qstr last;
        unsigned int flags;
        int last_type;
};  

