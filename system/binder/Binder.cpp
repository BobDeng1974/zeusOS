#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <linux/types.h>
#include<stdbool.h>
#include <string.h>
#include <pthread.h>

#include "binder.h"

#define MAX_BIO_SIZE (1 << 30)

#define TRACE 0

#if TRACE
#define ALOGI(x...) fprintf(stderr, "binder: " x)
#define ALOGE(x...) fprintf(stderr, "binder: " x)
#else
#define ALOGI(x...) 
#define ALOGE(x...) 
#endif



#if TRACE
void hexdump(void *_data, size_t len)
{
    unsigned char *data = _data;
    size_t count;

    for (count = 0; count < len; count++) {
        if ((count & 15) == 0)
            fprintf(stderr,"%04zu:", count);
        fprintf(stderr," %02x %c", *data,
                (*data < 32) || (*data > 126) ? '.' : *data);
        data++;
        if ((count & 15) == 15)
            fprintf(stderr,"\n");
    }
    if ((count & 15) != 0)
        fprintf(stderr,"\n");
}

void binder_dump_txn(struct binder_transaction_data *txn)
{
    struct flat_binder_object *obj;
    binder_size_t *offs = (binder_size_t *)(uintptr_t)txn->data.ptr.offsets;
    size_t count = txn->offsets_size / sizeof(binder_size_t);

    fprintf(stderr,"  target %016"PRIx64"  cookie %016"PRIx64"  code %08x  flags %08x\n",
            (uint64_t)txn->target.ptr, (uint64_t)txn->cookie, txn->code, txn->flags);
    fprintf(stderr,"  pid %8d  uid %8d  data %"PRIu64"  offs %"PRIu64"\n",
            txn->sender_pid, txn->sender_euid, (uint64_t)txn->data_size, (uint64_t)txn->offsets_size);
    hexdump((void *)(uintptr_t)txn->data.ptr.buffer, txn->data_size);
    while (count--) {
        obj = (struct flat_binder_object *) (((char*)(uintptr_t)txn->data.ptr.buffer) + *offs++);
        fprintf(stderr,"  - type %08x  flags %08x  ptr %016"PRIx64"  cookie %016"PRIx64"\n",
                obj->type, obj->flags, (uint64_t)obj->binder, (uint64_t)obj->cookie);
    }
}

#define NAME(n) case n: return #n
const char *cmd_name(uint32_t cmd)
{
    switch(cmd) {
        NAME(BR_NOOP);
        NAME(BR_TRANSACTION_COMPLETE);
        NAME(BR_INCREFS);
        NAME(BR_ACQUIRE);
        NAME(BR_RELEASE);
        NAME(BR_DECREFS);
        NAME(BR_TRANSACTION);
        NAME(BR_REPLY);
        NAME(BR_FAILED_REPLY);
        NAME(BR_DEAD_REPLY);
        NAME(BR_DEAD_BINDER);		
    default: return "???";
    }
}
#else
#define hexdump(a,b) do{} while (0)
#define binder_dump_txn(txn)  do{} while (0)
#endif

#define BIO_F_SHARED    0x01  /* needs to be buffer freed */
#define BIO_F_OVERFLOW  0x02  /* ran out of space */
#define BIO_F_IOERROR   0x04
#define BIO_F_MALLOCED  0x08  /* needs to be free()'d */


void binder_thread_loop(BinderState *bs, Binder *binder)
{
    int res;
    struct binder_write_read bwr;
    uint32_t readbuf[32];

    bwr.write_size = 0;
    bwr.write_consumed = 0;
    bwr.write_buffer = 0;

    readbuf[0] = BC_REGISTER_LOOPER;
    binder->binderWrite(readbuf, sizeof(uint32_t));

    for (;;) {
        bwr.read_size = sizeof(readbuf);
        bwr.read_consumed = 0;
        bwr.read_buffer = (uintptr_t) readbuf;

        res = ioctl(bs->fd, BINDER_WRITE_READ, &bwr);

        if (res < 0) {
            ALOGE("binder_loop: ioctl failed (%s)\n", strerror(errno));
            break;
        }

        res = binder->binderParse(NULL, (uintptr_t) readbuf, bwr.read_consumed);
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


static void * binder_thread_routine(BinderThreadDesc *btd)
{
	binder_thread_loop(btd->bs, btd->binder);
	return NULL;
}



Binder::Binder()
{
	binderOpen(128*1024);
}
Binder::~Binder()
{
	binderClose();
}

BinderState *Binder::binderOpen(unsigned int mapsize)
{
	struct binder_version vers;

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
		uint32_t cmd;
		struct binder_transaction_data txn;
	} __attribute__((packed)) writebuf;
	unsigned readbuf[32];

	if (msg.mBio.flags & BIO_F_OVERFLOW) {
		fprintf(stderr,"binder: txn buffer overflow\n");
		goto fail;
	}

	writebuf.cmd = BC_TRANSACTION;
	writebuf.txn.target.handle = target;
	writebuf.txn.code = code;
	writebuf.txn.flags = flags;
	writebuf.txn.data_size = msg.mBio.data - msg.mBio.data0;
	writebuf.txn.offsets_size = ((char*) msg.mBio.offs) - ((char*) msg.mBio.offs0);
	writebuf.txn.data.ptr.buffer = (uintptr_t)msg.mBio.data0;
	writebuf.txn.data.ptr.offsets = (uintptr_t)msg.mBio.offs0;

	bwr.write_size = sizeof(writebuf);
	bwr.write_consumed = 0;
	bwr.write_buffer = (uintptr_t) &writebuf;

	hexdump(msg.mBio.data0, msg.mBio.data - msg.mBio.data0);
	for (;;) {
		bwr.read_size = sizeof(readbuf);
		bwr.read_consumed = 0;
		bwr.read_buffer = (uintptr_t) readbuf;

		res = ioctl(mBinderState->fd, BINDER_WRITE_READ, &bwr);

		if (res < 0) {
			fprintf(stderr,"binder: ioctl failed (%s)\n", strerror(errno));
			goto fail;
		}

		res = binderParse(&reply, (uintptr_t) readbuf, bwr.read_consumed);
		if (res == 0) return 0;
		if (res < 0) goto fail;
	}

fail:
	memset(&reply.mBio, 0, sizeof(reply.mBio));
	reply.mBio.flags |= BIO_F_IOERROR;
	return -1;

}

int Binder::binderWrite(void *data, size_t len)
{
    struct binder_write_read bwr;
    int res;

    bwr.write_size = len;
    bwr.write_consumed = 0;
    bwr.write_buffer = (uintptr_t) data;
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
        uint32_t cmd;
        uintptr_t buffer;
    } __attribute__((packed)) data;

    if (reply.mBio.flags & BIO_F_SHARED) {
        data.cmd = BC_FREE_BUFFER;
        data.buffer = (uintptr_t) reply.mBio.data0;
        binderWrite(&data, sizeof(data));
        reply.mBio.flags = 0;
    }
}

/* manipulate strong references */
void Binder::binderAcquire(unsigned int target)
{
    uint32_t cmd[2];
    cmd[0] = BC_ACQUIRE;
    cmd[1] = target;
    binderWrite(cmd, sizeof(cmd));

}

void Binder::binderRelease(unsigned int target)
{
    uint32_t cmd[2];
    cmd[0] = BC_RELEASE;
    cmd[1] = target;
    binderWrite(cmd, sizeof(cmd));
}

int Binder::binderParse(Parcel *data, uintptr_t ptr, size_t size)
{
	int r = 1;
	uintptr_t end = ptr + (uintptr_t) size;
	BinderIO *bio = NULL;

	if (data) {
		bio = &data->mBio;
	}

	while (ptr < end) {
		uint32_t cmd = *(uint32_t *) ptr;
		ptr += sizeof(uint32_t);
#if TRACE
		fprintf(stderr,"%s:\n", cmd_name(cmd));
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
			if ((end - ptr) < sizeof(*txn)) {
				ALOGE("parse: txn too small!\n");
				return -1;
			}
			binder_dump_txn(txn);

			Parcel msg;
			Parcel reply;
			int res;

			msg.bioInitFromTxn(txn);
			res = onTransact(txn, &msg, &reply);
			binderSendReply(reply, txn->data.ptr.buffer, res);

			ptr += sizeof(*txn);
			break;
		}
		case BR_REPLY: {
			struct binder_transaction_data *txn = (struct binder_transaction_data *) ptr;
			if ((end - ptr) < sizeof(*txn)) {
				ALOGE("parse: reply too small!\n");
				return -1;
			}
			binder_dump_txn(txn);
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
			struct BinderDeath *death = (struct BinderDeath *)(uintptr_t) *(binder_uintptr_t *)ptr;
			ptr += sizeof(binder_uintptr_t);
			death->func(mBinderState, death->ptr);
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
        uint32_t cmd;
        struct binder_handle_cookie payload;
    } __attribute__((packed)) data;

    data.cmd = BC_REQUEST_DEATH_NOTIFICATION;
    data.payload.handle = target;
    data.payload.cookie = (uintptr_t) death;
    binderWrite(&data, sizeof(data));
}

void Binder::binderLoop(void)
{
    int res;
    struct binder_write_read bwr;
    uint32_t readbuf[32];

    bwr.write_size = 0;
    bwr.write_consumed = 0;
    bwr.write_buffer = 0;

    readbuf[0] = BC_ENTER_LOOPER;
    binderWrite(readbuf, sizeof(uint32_t));

    for (;;) {
        bwr.read_size = sizeof(readbuf);
        bwr.read_consumed = 0;
        bwr.read_buffer = (uintptr_t) readbuf;

        res = ioctl(mBinderState->fd, BINDER_WRITE_READ, &bwr);

        if (res < 0) {
            ALOGE("binder_loop: ioctl failed (%s)\n", strerror(errno));
            break;
        }

        res = binderParse(NULL, (uintptr_t) readbuf, bwr.read_consumed);
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


void Binder::binderSendReply(Parcel& reply, binder_uintptr_t buffer_to_free, int status)
{
    struct {
        uint32_t cmd_free;
        binder_uintptr_t buffer;
        uint32_t cmd_reply;
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
        data.txn.data.ptr.buffer = (uintptr_t)&status;
        data.txn.data.ptr.offsets = 0;
    } else {
        data.txn.flags = 0;
        data.txn.data_size = reply.mBio.data - reply.mBio.data0;
        data.txn.offsets_size = ((char*) reply.mBio.offs) - ((char*) reply.mBio.offs0);
        data.txn.data.ptr.buffer = (uintptr_t)reply.mBio.data0;
        data.txn.data.ptr.offsets = (uintptr_t)reply.mBio.offs0;
    }
    binderWrite(&data, sizeof(data));
}


