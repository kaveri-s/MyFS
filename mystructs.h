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
        struct list_head        s_inodes;
        struct list_head        s_dirty;
        struct hlist_head       s_anon;
        struct list_head        s_files;
        struct list_head        s_dentry_lru;
        struct block_device     *s_bdev;
        struct mtd_info         *s_mtd;
        struct hlist_node       s_instances;
        struct quota_info       s_dquot;
        char                    s_id[32];
        void                    *s_fs_info;
        fmode_t                 s_mode;
        u32                     s_time_gran;
        char                    *s_subtype;
        char                    *s_options;
 };

struct super_operations {
        struct inode *(*alloc_inode)(struct super_block *sb);
        void (*destroy_inode)(struct inode *);
        void (*read_inode) (struct inode *);
        void (*dirty_inode) (struct inode *);
        void (*write_inode) (struct inode *, int);
        void (*put_inode) (struct inode *);
        void (*drop_inode) (struct inode *);
        void (*delete_inode) (struct inode *);
        void (*put_super) (struct super_block *);
        void (*write_super) (struct super_block *);
        int (*sync_fs)(struct super_block *sb, int wait);
        void (*write_super_lockfs) (struct super_block *);
        void (*unlockfs) (struct super_block *);
        int (*statfs) (struct super_block *, struct statfs *);
        int (*remount_fs) (struct super_block *, int *, char *);
        void (*clear_inode) (struct inode *);
        void (*umount_begin) (struct super_block *);
        int (*show_options)(struct seq_file *, struct vfsmount *);
};

struct inode {
        struct hlist_node       i_hash;
        struct list_head        i_list;
        struct list_head        i_sb_list;
        struct list_head        i_dentry;
        unsigned long           i_ino;
        atomic_t                i_count;
        unsigned int            i_nlink;
        uid_t                   i_uid;
        gid_t                   i_gid;
        kdev_t                  i_rdev;
        u64                     i_version;
        loff_t                  i_size;
        seqcount_t              i_size_seqcount;
        struct timespec         i_atime;
        struct timespec         i_mtime;
        struct timespec         i_ctime;
        unsigned int            i_blkbits;
        blkcnt_t                i_blocks;
        unsigned short          i_bytes;
        umode_t                 i_mode;
        const struct inode_operations   *i_op;
        const struct file_operations    *i_fop;
        struct super_block      *i_sb;
        struct address_space    *i_mapping;
        struct address_space    i_data;
        struct dquot            *i_dqot[MAXQUOTAS];
        struct list_head        i_devices;
        union {
                struct pipe_inode_info  *i_pipe;
                struct block_device     *i_bdev;
                struct cdev             *i_cdev;
        };
        unsigned long           i_dnotify_mask;
        struct dnotify_struct   *i_dnotify;
        struct list_head        inotify_watches;
        unsigned long           i_state;
        unsigned long           dirtied_when;
        unsigned int            i_flags;
        atomic_t                i_writecount;
};

struct inode_operations {
        int (*create) (struct inode *, struct dentry *, int);
        struct dentry * (*lookup) (struct inode *, struct dentry *);
        int (*link) (struct dentry *, struct inode *, struct dentry *);
        int (*unlink) (struct inode *, struct dentry *);
        int (*symlink) (struct inode *, struct dentry *, const char *);
        int (*mkdir) (struct inode *, struct dentry *, int);
        int (*rmdir) (struct inode *, struct dentry *);
        int (*mknod) (struct inode *, struct dentry *, int, dev_t);
        int (*rename) (struct inode *, struct dentry *, struct inode *, struct dentry *);
        int (*readlink) (struct dentry *, char *,int);
        int (*follow_link) (struct dentry *, struct nameidata *);
        void (*truncate) (struct inode *);
        int (*permission) (struct inode *, int);
        int (*setattr) (struct dentry *, struct iattr *);
        int (*getattr) (struct vfsmount *mnt, struct dentry *, struct kstat *);
        int (*setxattr) (struct dentry *, const char *, const void *, size_t, int);
        ssize_t (*getxattr) (struct dentry *, const char *, void *, size_t);
        ssize_t (*listxattr) (struct dentry *, char *, size_t);
        int (*removexattr) (struct dentry *, const char *);
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