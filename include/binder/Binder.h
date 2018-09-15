/* Copyright 2008 The Android Open Source Project
 */

#ifndef _BINDER_H_
#define _BINDER_H_

class Binder;


#include <binder/Parcel.h>
#include <pthread.h>
#include <unistd.h>



#define BINDER_IPC_32BIT

#define BIO_F_SHARED    0x01  /* needs to be buffer freed */
#define BIO_F_OVERFLOW  0x02  /* ran out of space */
#define BIO_F_IOERROR   0x04
#define BIO_F_MALLOCED  0x08  /* needs to be free()'d */



#define MY_IOC_NRBITS	8
#define MY_IOC_TYPEBITS	8

/*
 * Let any architecture override either of the following before
 * including this file.
 */

#ifndef MY_IOC_SIZEBITS
# define MY_IOC_SIZEBITS	14
#endif

#ifndef MY_IOC_DIRBITS
# define MY_IOC_DIRBITS	2
#endif

#define MY_IOC_NRMASK	((1 << MYMY_IOC_NRBITS)-1)
#define MY_IOC_TYPEMASK	((1 << MYMY_IOC_TYPEBITS)-1)
#define MY_IOC_SIZEMASK	((1 << MYMY_IOC_SIZEBITS)-1)
#define MY_IOC_DIRMASK	((1 << MY_IOC_DIRBITS)-1)

#define MY_IOC_NRSHIFT	0
#define MY_IOC_TYPESHIFT	(MY_IOC_NRSHIFT+MY_IOC_NRBITS)
#define MY_IOC_SIZESHIFT	(MY_IOC_TYPESHIFT+MY_IOC_TYPEBITS)
#define MY_IOC_DIRSHIFT	(MY_IOC_SIZESHIFT+MY_IOC_SIZEBITS)

/*
 * Direction bits, which any architecture can choose to override
 * before including this file.
 */

#ifndef MY_IOC_NONE
# define MY_IOC_NONE	0U
#endif

#ifndef MY_IOC_WRITE
# define MY_IOC_WRITE	1U
#endif

#ifndef MY_IOC_READ
# define MY_IOC_READ	2U
#endif

#define MY_IOC(dir,type,nr,size) \
	(((dir)  << MY_IOC_DIRSHIFT) | \
	 ((type) << MY_IOC_TYPESHIFT) | \
	 ((nr)   << MY_IOC_NRSHIFT) | \
	 ((size) << MY_IOC_SIZESHIFT))


#ifdef __KERNEL__
/* provoke compile error for invalid uses of size argument */
extern unsigned int __invalid_size_argument_forMY_IOC;
#define MY_IOC_TYPECHECK(t) \
	((sizeof(t) == sizeof(t[1]) && \
	  sizeof(t) < (1 << MY_IOC_SIZEBITS)) ? \
	  sizeof(t) : __invalid_size_argument_forMY_IOC)
#else
#define MY_IOC_TYPECHECK(t) (sizeof(t))
#endif

/* used to create numbers */
#define MY_IO(type,nr)		MY_IOC(MY_IOC_NONE,(type),(nr),0)
#define MY_IOR(type,nr,size)	MY_IOC(MY_IOC_READ,(type),(nr),(MY_IOC_TYPECHECK(size)))
#define MY_IOW(type,nr,size)	MY_IOC(MY_IOC_WRITE,(type),(nr),(MY_IOC_TYPECHECK(size)))
#define MY_IOWR(type,nr,size)	MY_IOC(MY_IOC_READ|MY_IOC_WRITE,(type),(nr),(MY_IOC_TYPECHECK(size)))
#define MY_IOR_BAD(type,nr,size)	MY_IOC(MY_IOC_READ,(type),(nr),sizeof(size))
#define MY_IOW_BAD(type,nr,size)	MY_IOC(MY_IOC_WRITE,(type),(nr),sizeof(size))
#define MY_IOWR_BAD(type,nr,size)	MY_IOC(MY_IOC_READ|MY_IOC_WRITE,(type),(nr),sizeof(size))

/* used to decode ioctl numbers.. */
#define MY_IOC_DIR(nr)		(((nr) >> MY_IOC_DIRSHIFT) & MY_IOC_DIRMASK)
#define MY_IOC_TYPE(nr)		(((nr) >> MY_IOC_TYPESHIFT) & MY_IOC_TYPEMASK)
#define MY_IOC_NR(nr)		(((nr) >> MY_IOC_NRSHIFT) & MY_IOC_NRMASK)
#define MY_IOC_SIZE(nr)		(((nr) >> MY_IOC_SIZESHIFT) & MY_IOC_SIZEMASK)

/* ...and for the drivers/sound files... */

#define MY_IOC_IN		(MY_IOC_WRITE << MY_IOC_DIRSHIFT)
#define MY_IOC_OUT		(MY_IOC_READ << MY_IOC_DIRSHIFT)
#define MY_IOC_INOUT	((MY_IOC_WRITE|MY_IOC_READ) << MY_IOC_DIRSHIFT)
#define MY_IOCSIZE_MASK	(MY_IOC_SIZEMASK << MY_IOC_SIZESHIFT)
#define MY_IOCSIZE_SHIFT	(MY_IOC_SIZESHIFT)





#define B_PACK_CHARS(c1, c2, c3, c4)   ((((c1)<<24)) | (((c2)<<16)) | (((c3)<<8)) | (c4))
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
#define B_TYPE_LARGE 0x85
enum {
 BINDER_TYPE_BINDER = B_PACK_CHARS('s', 'b', '*', B_TYPE_LARGE),
 BINDER_TYPE_WEAK_BINDER = B_PACK_CHARS('w', 'b', '*', B_TYPE_LARGE),
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
 BINDER_TYPE_HANDLE = B_PACK_CHARS('s', 'h', '*', B_TYPE_LARGE),
 BINDER_TYPE_WEAK_HANDLE = B_PACK_CHARS('w', 'h', '*', B_TYPE_LARGE),
 BINDER_TYPE_FD = B_PACK_CHARS('f', 'd', '*', B_TYPE_LARGE),
};
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
enum {
 FLAT_BINDER_FLAG_PRIORITY_MASK = 0xff,
 FLAT_BINDER_FLAG_ACCEPTS_FDS = 0x100,
};
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
#ifdef BINDER_IPC_32BIT
typedef unsigned int binder_size_t;
typedef unsigned int *binder_uintptr_t;
#else
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
typedef unsigned long long binder_size_t;
typedef unsigned long long *binder_uintptr_t;
#endif
struct flat_binder_object {
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
 unsigned int type;
 unsigned int flags;
 union {
 binder_uintptr_t binder;
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
 unsigned int handle;
 };
 binder_uintptr_t cookie;
};
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
struct binder_write_read {
 binder_size_t write_size;
 binder_size_t write_consumed;
 binder_uintptr_t write_buffer;
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
 binder_size_t read_size;
 binder_size_t read_consumed;
 binder_uintptr_t read_buffer;
};
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
struct binder_version {
 int protocol_version;
};
#ifdef BINDER_IPC_32BIT
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
#define BINDER_CURRENT_PROTOCOL_VERSION 7
#else
#define BINDER_CURRENT_PROTOCOL_VERSION 8
#endif
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
#define BINDER_WRITE_READ MY_IOWR('b', 1, struct binder_write_read)
#define BINDER_SET_IDLE_TIMEOUT MY_IOW('b', 3, __s64)
#define BINDER_SET_MAX_THREADS MY_IOW('b', 5, unsigned int)
#define BINDER_SET_IDLE_PRIORITY MY_IOW('b', 6, int)
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
#define BINDER_SET_CONTEXT_MGR MY_IOW('b', 7, int)
#define BINDER_THREAD_EXIT MY_IOW('b', 8, int)
#define BINDER_VERSION MY_IOWR('b', 9, struct binder_version)
enum transaction_flags {
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
 TF_ONE_WAY = 0x01,
 TF_ROOT_OBJECT = 0x04,
 TF_STATUS_CODE = 0x08,
 TF_ACCEPT_FDS = 0x10,
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
};
struct binder_transaction_data {
 union {
 unsigned int handle;
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
 binder_uintptr_t ptr;
 } target;
 binder_uintptr_t cookie;
 unsigned int code;
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
 unsigned int flags;
 pid_t sender_pid;
 uid_t sender_euid;
 binder_size_t data_size;
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
 binder_size_t offsets_size;
 union {
 struct {
 binder_uintptr_t buffer;
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
 binder_uintptr_t offsets;
 } ptr;
 unsigned char buf[8];
 } data;
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
};
struct binder_ptr_cookie {
 binder_uintptr_t ptr;
 binder_uintptr_t cookie;
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
};
struct binder_handle_cookie {
 unsigned int handle;
 binder_uintptr_t cookie;
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
} __attribute__((packed));
struct binder_pri_desc {
 int priority;
 unsigned int desc;
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
};
struct binder_pri_ptr_cookie {
 int priority;
 binder_uintptr_t ptr;
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
 binder_uintptr_t cookie;
};
// binder_driver_return_protocol 
#define BR_ERROR MY_IOR('r', 0, int)
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
#define BR_OK MY_IO('r', 1)
#define BR_TRANSACTION MY_IOR('r', 2, struct binder_transaction_data)
#define BR_REPLY MY_IOR('r', 3, struct binder_transaction_data)
#define BR_ACQUIRE_RESULT MY_IOR('r', 4, int)
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
#define BR_DEAD_REPLY MY_IO('r', 5)
#define BR_TRANSACTION_COMPLETE MY_IO('r', 6)
#define BR_INCREFS MY_IOR('r', 7, struct binder_ptr_cookie)
#define BR_ACQUIRE MY_IOR('r', 8, struct binder_ptr_cookie)
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
#define BR_RELEASE MY_IOR('r', 9, struct binder_ptr_cookie)
#define BR_DECREFS MY_IOR('r', 10, struct binder_ptr_cookie)
#define BR_ATTEMPT_ACQUIRE MY_IOR('r', 11, struct binder_pri_ptr_cookie)
#define BR_NOOP MY_IO('r', 12)
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
#define BR_SPAWN_LOOPER MY_IO('r', 13)
#define BR_FINISHED MY_IO('r', 14)
#define BR_DEAD_BINDER MY_IOR('r', 15, binder_uintptr_t)
#define BR_CLEAR_DEATH_NOTIFICATION_DONE MY_IOR('r', 16, binder_uintptr_t)
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
#define BR_FAILED_REPLY MY_IO('r', 17)


// binder_driver_command_protocol 
#define BC_TRANSACTION MY_IOW('c', 0, struct binder_transaction_data)
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
#define BC_REPLY MY_IOW('c', 1, struct binder_transaction_data)
#define BC_ACQUIRE_RESULT MY_IOW('c', 2, int)
#define BC_FREE_BUFFER MY_IOW('c', 3, binder_uintptr_t)
#define BC_INCREFS MY_IOW('c', 4, unsigned int)
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
#define BC_ACQUIRE MY_IOW('c', 5, unsigned int)
#define BC_RELEASE MY_IOW('c', 6, unsigned int)
#define BC_DECREFS MY_IOW('c', 7, unsigned int)
#define BC_INCREFS_DONE MY_IOW('c', 8, struct binder_ptr_cookie)
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
#define BC_ACQUIRE_DONE MY_IOW('c', 9, struct binder_ptr_cookie)
#define BC_ATTEMPT_ACQUIRE MY_IOW('c', 10, struct binder_pri_desc)
#define BC_REGISTER_LOOPER MY_IO('c', 11)
#define BC_ENTER_LOOPER MY_IO('c', 12)
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
#define BC_EXIT_LOOPER MY_IO('c', 13)
#define BC_REQUEST_DEATH_NOTIFICATION MY_IOW('c', 14, struct binder_handle_cookie)
#define BC_CLEAR_DEATH_NOTIFICATION MY_IOW('c', 15, struct binder_handle_cookie)
#define BC_DEAD_BINDER_DONE MY_IOW('c', 16, binder_uintptr_t)
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */




struct BinderState
{
    int fd;
    void *mapped;
    unsigned int mapsize;
};



struct BinderIO
{
    char *data;            /* pointer to read/write from */
    binder_size_t *offs;   /* array of offsets */
    int data_avail;     /* bytes available in data buffer */
    int offs_avail;     /* entries available in offsets array */

    char *data0;           /* start of data buffer */
    binder_size_t *offs0;  /* start of offsets buffer */
    unsigned int flags;
    unsigned int unused;
};

struct BinderDeath {
	void *binder;
    void *ptr;
};

/* the one magic handle */
#define BINDER_SERVICE_MANAGER  0U

#define SVC_MGR_NAME "android.os.IServiceManager"

enum {
    /* Must match definitions in IBinder.h and IServiceManager.h */
    PING_TRANSACTION  = B_PACK_CHARS('_','P','N','G'),
    SVC_MGR_GET_SERVICE = 1,
    SVC_MGR_CHECK_SERVICE,
    SVC_MGR_ADD_SERVICE,
    SVC_MGR_LIST_SERVICES,
};

typedef int (*binderHandler)(struct BinderState *bs,
                              struct binder_transaction_data *txn,
                              struct BinderIO *msg,
                              struct BinderIO *reply);

struct BinderThreadDesc {
    BinderState *bs;
	Binder *binder;
};


class Binder {
public:
	Binder();
	virtual ~Binder();
	BinderState *binderOpen(unsigned int mapsize);
	void binderClose(void);

	int binderWrite(void *data, int len);
	
	/* initiate a blocking binder call
	 * - returns zero on success
	 */
	int binderCall(Parcel& msg, Parcel& reply, unsigned int target, unsigned int code, unsigned int flags);
	
	/* release any state associate with the binder_io
	 * - call once any necessary data has been extracted from the
	 *	 binder_io after binder_call() returns
	 * - can safely be called even if binder_call() fails
	 */
	void binderDone(Parcel& msg, Parcel& reply);

	void binderSendReply(Parcel& reply, binder_uintptr_t buffer_to_free, int status);
	
	/* manipulate strong references */
	void binderAcquire(unsigned int target);
	void binderRelease(unsigned int target);

	int binderParse(Parcel *data, unsigned char *ptr, int size);
	virtual void binderDeath(void *ptr);
	void binderLinkToDeath(unsigned int target, struct BinderDeath *death);
	
	virtual void binderLoop(void);
	
	int binderBecomeContextManager(void);
	
	void binderSetMaxthreads(int threads);

	virtual int onTransact(struct binder_transaction_data *txn, Parcel *msg, Parcel *reply) = 0;

	
private:
	BinderState *mBinderState;
};


#endif
