/* This file contains all the structures required to create a file system*/


#include <fuse.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/time.h>


// struct file_system_type {
//     const char *name;
//     int fs_flags;
//     struct super_block *get_sb;
//     void (*kill_sb);
//     struct module *owner;
//     struct file_system_type *next;
//     struct list_head fs_supers;
// };

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
        void (*delete_inode) (struct inode *);
        void (*write_super) (struct super_block *);
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
        //int (*symlink) (struct inode *, struct dentry *, const char *);
        int (*mkdir) (struct inode *, struct dentry *, int);
        int (*rmdir) (struct inode *, struct dentry *);
        int (*mknod) (struct inode *, struct dentry *, int, dev_t);
        int (*rename) (struct inode *, struct dentry *, struct inode *, struct dentry *);
        int (*readlink) (struct dentry *, char *,int);
        //int (*follow_link) (struct dentry *, struct nameidata *);
        void (*truncate) (struct inode *);
        int (*permission) (struct inode *, int);
        int (*setattr) (struct dentry *, struct iattr *);
        int (*getattr) (struct vfsmount *mnt, struct dentry *, struct kstat *);
};

struct dentry {
        atomic_t        d_count;
        unsigned int    d_flags;
        int             d_mounted;
        struct inode    *d_inode;
        struct hlist_node   d_hash;
        struct dentry *d_parent;        /* parent directory */
        struct qstr d_name;
        struct list_head d_lru;         /* LRU list */
        union {
            struct list_head     d_child;      /* list of dentries within */ 
            struct rcu_head      d_rcu;        /* RCU locking */
        } d_u;         
        struct list_head d_subdirs;     /* our children */
        struct hlist_node d_alias;      /* inode alias list */
        unsigned long d_time;           /* used by d_revalidate */
        const struct dentry_operations *d_op;
        struct super_block *d_sb;       /* The root of the dentry tree */
        void *d_fsdata;                 /* fs-specific data */
        unsigned char d_iname[DNAME_INLINE_LEN];        /* small names */
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
    union {
        struct list_head   fu_list;       /* list of file objects */ 
        struct rcu_head    fu_rcuhead;    /* RCU list after freeing */
    } f_u; 
    struct path            f_path;        /* contains the dentry */ 
    struct file_operations *f_op;         /* file operations table */ 
    atomic_t               f_count;       /* file object’s usage count */ 
    unsigned int           f_flags;       /* flags specified on open */ 
    mode_t                 f_mode;        /* file access mode */ 
    loff_t                 f_pos;         /* file offset (file pointer) */ 
    struct fown_struct     f_owner;       /* owner data for signals */ 
    const struct cred      *f_cred;       /* file credentials */ 
    struct file_ra_state   f_ra; 
    /* read-ahead state */ 
    u64                    f_version;     /* version number */ 
    void                   *f_security;   /* security module */ 
    void                   *private_data; /* tty driver hook */
    struct list_head       f_ep_links;    /* list of epoll links */
    struct address_space   *f_mapping;    /* page cache mapping */ 
    unsigned long          f_mnt_write_state; /* debugging state */
};

struct qstr {
        const unsigned char *name;
        unsigned int len;
        unsigned int hash;
};

struct file_operations {
        struct module *owner;
        loff_t (*llseek) (struct file *, loff_t, int);
        ssize_t (*read) (struct file *, char *, size_t, loff_t *);
        //ssize_t (*aio_read) (struct kiocb *, char *, size_t, loff_t);
        ssize_t (*write) (struct file *, const char *, size_t, loff_t *);
        //ssize_t (*aio_write) (struct kiocb *, const char *, size_t, loff_t);
        int (*readdir) (struct file *, void *, filldir_t);
        //unsigned int (*poll) (struct file *, struct poll_table_struct *);
        //int (*ioctl) (struct inode *, struct file *, unsigned int, unsigned long);
        //int (*mmap) (struct file *, struct vm_area_struct *);
        int (*open) (struct inode *, struct file *);
        //int (*flush) (struct file *);
        int (*release) (struct inode *, struct file *);
        //int (*fsync) (struct file *, struct dentry *, int datasync);
        //int (*aio_fsync) (struct kiocb *, int datasync);
        //int (*fasync) (int, struct file *, int);
        //int (*lock) (struct file *, int, struct file_lock *);
        //ssize_t (*readv) (struct file *, const struct iovec *, unsigned long, loff_t *);
        //ssize_t (*writev) (struct file *, const struct iovec *, unsigned long, loff_t *);
        //ssize_t (*sendfile) (struct file *, loff_t *, size_t, read_actor_t, void *);
        //ssize_t (*sendpage) (struct file *, struct page *, int, size_t, loff_t *, int);
        //unsigned long (*get_unmapped_area)(struct file *, unsigned long, unsigned long, unsigned long, unsigned long);
};

struct files_struct { 
    atomic_t               count;              /* usage count */ 
    struct fdtable         *fdt;               /* pointer to other fd table */ 
    struct fdtable         fdtab;              /* base fd table */ 
    spinlock_t             file_lock;          /* per-file lock */ 
    int                     next_fd; 
    /* cache of next available fd */ 
    struct embedded_fd_set close_on_exec_init; /* list of close-on-exec fds */ 
    struct embedded_fd_set open_fds_init       /* list of open fds */ 
    struct file            *fd_array[NR_OPEN_DEFAULT]; /* base files array */
};

struct fs_struct { 
    int         users;    /* user count */ 
    rwlock_t    lock;     /* per-structure lock */ 
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

struct mnt_namespace { 
    atomic_t            count; /* usage count */ 
    struct vfsmount     *root; /* root directory */
    struct list_head    list;  /* list of mount points */ 
    wait_queue_head_t   poll;  /* polling waitqueue */ 
    int                 event; /* event count */
};  

// struct stat {
//     unsigned int     st_dev;         /* ID of device containing file */
//     unsigned int     st_ino;         /* inode number */
//     unsigned int    st_mode;        /* protection */
//     unsigned int   st_nlink;       /* number of hard links */
// 	unsigned int     st_uid;         /* user ID of owner */
// 	unsigned int     st_gid;         /* group ID of owner */
//     unsigned int     st_rdev;        /* device ID (if special file) */
//     unsigned int     st_size;        /* total size, in bytes */
//     unsigned int st_blksize;     /* blocksize for filesystem I/O */
//     unsigned int  st_blocks;      /* number of 512B blocks allocated */
//     unsigned int st_atim;  /* time of last access */
//     unsigned int st_mtim;  /* time of last modification */
// 	unsigned int st_ctim;  /* time of last status change */
// };