#include <binder/Binder.h>
#include <frameworks/ServiceManager.h>
#include <sys/ioctl.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <stdbool.h>
#include <string.h>

#define MAX_BIO_SIZE (1 << 30)

#define TRACE 0

#if TRACE
#define ALOGI(x...) fprintf(stderr, "binder: " x)
#define ALOGE(x...) fprintf(stderr, "binder: " x)
#else
#define ALOGI(x...) 
#define ALOGE(x...) 
#endif


void binder_thread_loop(BinderState *bs, Binder *binder)
{
    int res;
    struct binder_write_read bwr;
    unsigned int readbuf[32];

    bwr.write_size = 0;
    bwr.write_consumed = 0;
    bwr.write_buffer = 0;

    readbuf[0] = BC_REGISTER_LOOPER;
    binder->binderWrite(readbuf, sizeof(unsigned int));

    for (;;) {
        bwr.read_size = sizeof(readbuf);
        bwr.read_consumed = 0;
        bwr.read_buffer = (binder_uintptr_t) readbuf;

        res = ioctl(bs->fd, BINDER_WRITE_READ, &bwr);

        if (res < 0) {
            ALOGE("binder_loop: ioctl failed (%s)\n", strerror(errno));
            break;
        }

        res = binder->binderParse(NULL, (unsigned char *)readbuf, bwr.read_consumed);
        if (res == 0) {
            ALOGE("binder_loop: unexpected reply?!\n");
            break;
        }
        if (res < 0) {
            ALOGE("binder_loop: io error %d %s\n", res, strerror(errno));
            break;
        }
    }
}


static void * binder_thread_routine(void *ptr)
{
	BinderThreadDesc *btd = (BinderThreadDesc *)ptr;

	binder_thread_loop(btd->bs, btd->binder);
	return NULL;
}


Binder *Binder::mBinder = NULL;
pthread_mutex_t Binder::tMutex  = PTHREAD_MUTEX_INITIALIZER;


Binder* Binder::getBinder(void)
{
	if (NULL == mBinder) {		
		pthread_mutex_lock(&tMutex);
		if (NULL == mBinder) {
			mBinder = new Binder();
		}
		pthread_mutex_unlock(&tMutex);
	}

	return mBinder;
}


Binder::Binder()
{
	mBninder = NULL;

	binderOpen();
}
Binder::~Binder()
{
	binderClose();
}

BinderState *Binder::binderOpen(void)
{
	struct binder_version vers;
	unsigned int mapsize = 128 * 1024;

	mBinderState = new BinderState();
	if (!mBinderState) {
		errno = ENOMEM;
		return NULL;
	}

	mBinderState->fd = open("/dev/binder", O_RDWR);
	if (mBinderState->fd < 0) {
		fprintf(stderr,"binder: cannot open device (%s)\n",
				strerror(errno));
		goto fail_open;
	}

	if ((ioctl(mBinderState->fd, BINDER_VERSION, &vers) == -1) ||
		(vers.protocol_version != BINDER_CURRENT_PROTOCOL_VERSION)) {
		fprintf(stderr, "binder: driver version differs from user space\n");
		goto fail_open;
	}

	mBinderState->mapsize = mapsize;
	mBinderState->mapped = mmap(NULL, mapsize, PROT_READ, MAP_PRIVATE, mBinderState->fd, 0);
	if (mBinderState->mapped == MAP_FAILED) {
		fprintf(stderr,"binder: cannot map device (%s)\n",
				strerror(errno));
		goto fail_map;
	}

	return mBinderState;

fail_map:
	close(mBinderState->fd);
fail_open:
	delete mBinderState;
	return NULL;
}

void Binder::binderClose(void)
{
    munmap(mBinderState->mapped, mBinderState->mapsize);
    close(mBinderState->fd);
    delete mBinderState;
}


int Binder::binderCall(Parcel& msg, Parcel& reply, unsigned int target, unsigned int code, unsigned int flags)
{
	int res;
	struct binder_write_read bwr;
	struct {
		unsigned int cmd;
		struct binder_transaction_data txn;
	} __attribute__((packed)) writebuf;
	unsigned readbuf[32];

	if (msg.mBio->flags & BIO_F_OVERFLOW) {
		fprintf(stderr,"binder: txn buffer overflow\n");
		goto fail;
	}

	writebuf.cmd = BC_TRANSACTION;
	writebuf.txn.target.handle = target;
	writebuf.txn.code = code;
	writebuf.txn.flags = flags;
	writebuf.txn.data_size = msg.mBio->data - msg.mBio->data0;
	writebuf.txn.offsets_size = ((char*) msg.mBio->offs) - ((char*) msg.mBio->offs0);
	writebuf.txn.data.ptr.buffer = (binder_uintptr_t)msg.mBio->data0;
	writebuf.txn.data.ptr.offsets = (binder_uintptr_t)msg.mBio->offs0;

	bwr.write_size = sizeof(writebuf);
	bwr.write_consumed = 0;
	bwr.write_buffer = (binder_uintptr_t) &writebuf;

	for (;;) {
		bwr.read_size = sizeof(readbuf);
		bwr.read_consumed = 0;
		bwr.read_buffer = (binder_uintptr_t) readbuf;

		res = ioctl(mBinderState->fd, BINDER_WRITE_READ, &bwr);

		if (res < 0) {
			fprintf(stderr,"binder: ioctl failed (%s)\n", strerror(errno));
			goto fail;
		}
		
		res = binderParse(&reply, (unsigned char *)readbuf, bwr.read_consumed);
		if (res == 0) return 0;
		if (res < 0) goto fail;
	}

fail:
	memset(reply.mBio, 0, sizeof(struct BinderIO));
	reply.mBio->flags |= BIO_F_IOERROR;
	return -1;

}

int Binder::binderWrite(void *data, int len)
{
    struct binder_write_read bwr;
    int res;

    bwr.write_size = len;
    bwr.write_consumed = 0;
    bwr.write_buffer = (binder_uintptr_t) data;
    bwr.read_size = 0;
    bwr.read_consumed = 0;
    bwr.read_buffer = 0;
    res = ioctl(mBinderState->fd, BINDER_WRITE_READ, &bwr);
    if (res < 0) {
        fprintf(stderr,"binder_write: ioctl failed (%s)\n",
                strerror(errno));
    }
    return res;
}


void Binder::binderDone(Parcel& msg, Parcel& reply)
{
    struct {
        unsigned int cmd;
        unsigned int * buffer;
    } __attribute__((packed)) data;

    if (reply.mBio->flags & BIO_F_SHARED) {
        data.cmd = BC_FREE_BUFFER;
        data.buffer = (unsigned int *) reply.mBio->data0;
        binderWrite(&data, sizeof(data));
        reply.mBio->flags = 0;
    }
}

/* manipulate strong references */
void Binder::binderAcquire(unsigned int target)
{
    unsigned int cmd[2];
    cmd[0] = BC_ACQUIRE;
    cmd[1] = target;
    binderWrite(cmd, sizeof(cmd));

}

void Binder::binderRelease(unsigned int target)
{
    unsigned int cmd[2];
    cmd[0] = BC_RELEASE;
    cmd[1] = target;
    binderWrite(cmd, sizeof(cmd));
}


#define NAME(n) case n: return #n
const char *cmd_name(unsigned int cmd)
{
    switch(cmd) {
		NAME(BR_FAILED_REPLY);
		NAME(BR_CLEAR_DEATH_NOTIFICATION_DONE);
		NAME(BR_DEAD_BINDER);
		NAME(BR_FINISHED);
		NAME(BR_SPAWN_LOOPER);
		NAME(BR_NOOP);
		NAME(BR_ATTEMPT_ACQUIRE);
		NAME(BR_DECREFS);
		NAME(BR_RELEASE);
		NAME(BR_ACQUIRE);
		NAME(BR_INCREFS);
		NAME(BR_TRANSACTION_COMPLETE);
		NAME(BR_DEAD_REPLY);
		NAME(BR_ACQUIRE_RESULT);
		NAME(BR_REPLY);
		NAME(BR_TRANSACTION);
		NAME(BR_OK);
		NAME(BR_ERROR);
    default: return "???";
    }
}


int Binder::binderParse(Parcel *data, unsigned char *ptr, int size)
{
	int r = 1;
	unsigned char * end = ptr + size;
	struct BinderIO *bio = NULL;

	if (data) {
		bio = data->mBio;
	}

	while (ptr < end) {
		unsigned int cmd = *(unsigned int *) ptr;
		ptr += sizeof(unsigned int);
#if TRACE
		fprintf(stderr,"%s:%d\n", cmd_name(cmd), cmd);
#endif
		switch(cmd) {
		case BR_NOOP:
			break;
		case BR_TRANSACTION_COMPLETE:
			break;
		case BR_INCREFS:
		case BR_ACQUIRE:
		case BR_RELEASE:
		case BR_DECREFS:
#if TRACE
			fprintf(stderr,"  %p, %p\n", (void *)ptr, (void *)(ptr + sizeof(void *)));
#endif
			ptr += sizeof(struct binder_ptr_cookie);
			break;
		case BR_SPAWN_LOOPER: {
			pthread_t thread;
			BinderThreadDesc btd;

			btd.bs = mBinderState;
			btd.binder = this;
			
			pthread_create(&thread, NULL, binder_thread_routine, &btd);

			/* in new thread: ioctl(BC_ENTER_LOOPER), enter binder_looper */

			break;
		}
		case BR_TRANSACTION: {
			struct binder_transaction_data *txn = (struct binder_transaction_data *) ptr;
			if ((end - ptr) < (int)sizeof(*txn)) {
				ALOGE("parse: txn too small!\n");
				return -1;
			}


			Parcel msg;
			Parcel reply;
			int res;
			
			msg.bioInitFromTxn(txn);

			BnBinder *BBinder;

			if (txn->target.ptr) {
				BBinder = (BnBinder *)txn->target.ptr;
				res = BBinder->onTransact(txn, &msg, &reply);
			}
			else {
				res = BnServiceManager::get()->onTransact(txn, &msg, &reply);
			}
			
			binderSendReply(reply, txn->data.ptr.buffer, res);
			ptr += sizeof(*txn);
			break;
		}
		case BR_REPLY: {
			struct binder_transaction_data *txn = (struct binder_transaction_data *) ptr;
			if ((end - ptr) < (int)sizeof(*txn)) {
				ALOGE("parse: reply too small!\n");
				return -1;
			}
			
			if (bio) {
				data->bioInitFromTxn(txn);
				bio = 0;
			} else {
				/* todo FREE BUFFER */
			}
			ptr += sizeof(*txn);
			r = 0;
			break;
		}
		case BR_DEAD_BINDER: {
			struct BinderDeath *death = (struct BinderDeath *)(unsigned int *) *(binder_uintptr_t *)ptr;
			ptr += sizeof(binder_uintptr_t);

			death->binderDeath(death->ptr);
			break;
		}
		case BR_FAILED_REPLY:
			r = -1;
			break;
		case BR_DEAD_REPLY:
			r = -1;
			break;
		default:
			ALOGE("parse: OOPS %d\n", cmd);
			return -1;
		}
	}

	return r;
}


void Binder::binderLinkToDeath(unsigned int target, struct BinderDeath *death)
{
    struct {
        unsigned int cmd;
        struct binder_handle_cookie payload;
    } __attribute__((packed)) data;

    data.cmd = BC_REQUEST_DEATH_NOTIFICATION;
    data.payload.handle = target;
    data.payload.cookie = (binder_uintptr_t) death;
    binderWrite(&data, sizeof(data));
}

void Binder::binderLoop(void)
{
    int res;
    struct binder_write_read bwr;
    unsigned int readbuf[32];

    bwr.write_size = 0;
    bwr.write_consumed = 0;
    bwr.write_buffer = 0;

    readbuf[0] = BC_ENTER_LOOPER;
    binderWrite(readbuf, sizeof(unsigned int));

    for (;;) {
        bwr.read_size = sizeof(readbuf);
        bwr.read_consumed = 0;
        bwr.read_buffer = (binder_uintptr_t) readbuf;

        res = ioctl(mBinderState->fd, BINDER_WRITE_READ, &bwr);

        if (res < 0) {
            ALOGE("binder_loop: ioctl failed (%s)\n", strerror(errno));
            break;
        }

        res = binderParse(NULL, (unsigned char *) readbuf, bwr.read_consumed);
        if (res == 0) {
            ALOGE("binder_loop: unexpected reply?!\n");
            break;
        }
        if (res < 0) {
            ALOGE("binder_loop: io error %d %s\n", res, strerror(errno));
            break;
        }
    }

}


int Binder::binderBecomeContextManager(void)
{
    return ioctl(mBinderState->fd, BINDER_SET_CONTEXT_MGR, 0);
}

void Binder::binderSetMaxthreads(int threads)
{
	ioctl(mBinderState->fd, BINDER_SET_MAX_THREADS, &threads);
}

/* servicemanager 才需要这个 */
void Binder::setBnBinder(BnBinder *bninder)
{
	mBninder = bninder;
}


void Binder::binderSendReply(Parcel& reply, binder_uintptr_t buffer_to_free, int status)
{
    struct {
        unsigned int cmd_free;
        binder_uintptr_t buffer;
        unsigned int cmd_reply;
        struct binder_transaction_data txn;
    } __attribute__((packed)) data;

    data.cmd_free = BC_FREE_BUFFER;
    data.buffer = buffer_to_free;
    data.cmd_reply = BC_REPLY;
    data.txn.target.ptr = 0;
    data.txn.cookie = 0;
    data.txn.code = 0;
    if (status) {
        data.txn.flags = TF_STATUS_CODE;
        data.txn.data_size = sizeof(int);
        data.txn.offsets_size = 0;
        data.txn.data.ptr.buffer = (binder_uintptr_t)&status;
        data.txn.data.ptr.offsets = 0;
    } else {
        data.txn.flags = 0;
        data.txn.data_size = reply.mBio->data - reply.mBio->data0;
        data.txn.offsets_size = ((char*) reply.mBio->offs) - ((char*) reply.mBio->offs0);
        data.txn.data.ptr.buffer = (binder_uintptr_t)reply.mBio->data0;
        data.txn.data.ptr.offsets = (binder_uintptr_t)reply.mBio->offs0;
    }
    binderWrite(&data, sizeof(data));
}


