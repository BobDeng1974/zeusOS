
/* Copyright 2008 The Android Open Source Project
 */

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <linux/types.h>
#include <linux/ashmem.h>

#include <stdbool.h>
#include <string.h>

#include <private/android_filesystem_config.h>

#include "binder.h"
#include "test_server.h"




int svcmgr_publish(struct binder_state *bs, uint32_t target, const char *name, void *ptr)
{
    int status;
    unsigned iodata[512/4];
    struct binder_io msg, reply;

    bio_init(&msg, iodata, sizeof(iodata), 4);
    bio_put_uint32(&msg, 0);  // strict mode header
    bio_put_string16_x(&msg, SVC_MGR_NAME);
    bio_put_string16_x(&msg, name);
    bio_put_obj(&msg, ptr);

    if (binder_call(bs, &msg, &reply, target, SVC_MGR_ADD_SERVICE))
        return -1;

    status = bio_get_uint32(&reply);

    binder_done(bs, &msg, &reply);

    return status;
}

void sayhello(void)
{
	static int cnt = 0;
	fprintf(stderr, "say hello : %d\n", ++cnt);
}


int sayhello_to(char *name)
{
	static int cnt = 0;
	fprintf(stderr, "say hello to %s : %d\n", name, ++cnt);
	return cnt;
}

void saygoodbye(void)
{
	static int cnt = 0;
	fprintf(stderr, "say goodbye : %d\n", ++cnt);
}


int saygoodbye_to(char *name)
{
	static int cnt = 0;
	fprintf(stderr, "say goodbye to %s : %d\n", name, ++cnt);
	return cnt;
}


int allocIonMem(unsigned int size)
{
	int iFd;
	int iShareFd;
	int iRet;

	unsigned char *buffer;
	int i;
	
	iFd = open(ASHMEM_NAME_DEF, O_RDWR);

	iRet = ioctl(iFd, ASHMEM_SET_NAME, "test_server");
	if (iRet < 0) {
		printf("ioctl(iFd, ASHMEM_SET_NAME, test_server) error!\n");
		return -1;
	}
	iRet = ioctl(iFd, ASHMEM_SET_SIZE, 0x1000);
	if (iRet < 0) {
		printf("ioctl(iFd, ASHMEM_SET_SIZE, 0x1000) error!\n");
		return -1;
	}

	buffer =  (unsigned char *)mmap(NULL, 0x1000, PROT_READ | PROT_WRITE, MAP_SHARED, iFd, 0);	
	for (i = 0; i < 5; i++) {
		buffer[i] = (unsigned char)i;
	}
	
	return iFd;
}


int hello_service_handler(struct binder_state *bs,
                   struct binder_transaction_data *txn,
                   struct binder_io *msg,
                   struct binder_io *reply)
{
	/* 根据txn->code知道要调用哪一个函数
	 * 如果需要参数, 可以从msg取出
	 * 如果要返回结果, 可以把结果放入reply
	 */

	/* sayhello
	 * sayhello_to
	 */
	
    uint16_t *s;
	char name[512];
    size_t len;
    uint32_t handle;
    uint32_t strict_policy;
	int i;


    // Equivalent to Parcel::enforceInterface(), reading the RPC
    // header with the strict mode policy mask and the interface name.
    // Note that we ignore the strict_policy and don't propagate it
    // further (since we do no outbound RPCs anyway).
    strict_policy = bio_get_uint32(msg);


    switch(txn->code) {
    case HELLO_SVR_CMD_SAYHELLO:
		sayhello();
		bio_put_uint32(reply, 0); /* no exception */
        return 0;

    case HELLO_SVR_CMD_SAYHELLO_TO:
		/* 从msg里取出字符串 */
		s = bio_get_string16(msg, &len);  //"IHelloService"
		s = bio_get_string16(msg, &len);  // name
		if (s == NULL) {
			return -1;
		}
		for (i = 0; i < len; i++)
			name[i] = s[i];
		name[i] = '\0';

		/* 处理 */
		i = sayhello_to(name);

		/* 把结果放入reply */
		bio_put_uint32(reply, 0); /* no exception */
		bio_put_uint32(reply, i);
		
        break;

    default:
        fprintf(stderr, "unknown code %d\n", txn->code);
        return -1;
    }

    return 0;
}



int goodbye_service_handler(struct binder_state *bs,
                   struct binder_transaction_data *txn,
                   struct binder_io *msg,
                   struct binder_io *reply)
{
	/* 根据txn->code知道要调用哪一个函数
	 * 如果需要参数, 可以从msg取出
	 * 如果要返回结果, 可以把结果放入reply
	 */

	/* sayhello
	 * sayhello_to
	 */
	
    uint16_t *s;
	char name[512];
    size_t len;
    uint32_t handle;
    uint32_t strict_policy;
	int i;


    // Equivalent to Parcel::enforceInterface(), reading the RPC
    // header with the strict mode policy mask and the interface name.
    // Note that we ignore the strict_policy and don't propagate it
    // further (since we do no outbound RPCs anyway).
    strict_policy = bio_get_uint32(msg);


    switch(txn->code) {
    case GOODBYE_SVR_CMD_SAYGOODBYE:
		saygoodbye();
		bio_put_uint32(reply, 0); /* no exception */
        return 0;

    case GOODBYE_SVR_CMD_SAYGOODBYE_TO:
		/* 从msg里取出字符串 */
		s = bio_get_string16(msg, &len);  //"IGoodbyeService"
		s = bio_get_string16(msg, &len);  // name
		if (s == NULL) {
			return -1;
		}
		for (i = 0; i < len; i++)
			name[i] = s[i];
		name[i] = '\0';

		/* 处理 */
		i = saygoodbye_to(name);

		/* 把结果放入reply */
		bio_put_uint32(reply, 0); /* no exception */
		bio_put_uint32(reply, i);
		
        break;

    default:
        fprintf(stderr, "unknown code %d\n", txn->code);
        return -1;
    }

    return 0;
}


int mem_service_handler(struct binder_state *bs,
                   struct binder_transaction_data *txn,
                   struct binder_io *msg,
                   struct binder_io *reply)
{
	/* 根据txn->code知道要调用哪一个函数
	 * 如果需要参数, 可以从msg取出
	 * 如果要返回结果, 可以把结果放入reply
	 */

	/* sayhello
	 * sayhello_to
	 */
	
    uint16_t *s;
	char name[512];
    size_t len;
    uint32_t handle;
    uint32_t strict_policy;
	int i;
	int iFd;


    // Equivalent to Parcel::enforceInterface(), reading the RPC
    // header with the strict mode policy mask and the interface name.
    // Note that we ignore the strict_policy and don't propagate it
    // further (since we do no outbound RPCs anyway).
    strict_policy = bio_get_uint32(msg);


    switch(txn->code) {
    case GET_MEM_FD:
		iFd = allocIonMem(0x1000);
		if (iFd < 0) {
			bio_put_uint32(reply, iFd); /* no exception */			
		}
		else {
			bio_put_uint32(reply, 0); /* no exception */
			bio_put_fd(reply, iFd);
		}

        return 0;

    default:
        fprintf(stderr, "unknown code %d\n", txn->code);
        return -1;
    }

    return 0;
}


int test_server_handler(struct binder_state *bs,
                   struct binder_transaction_data *txn,
                   struct binder_io *msg,
                   struct binder_io *reply)
{
	int (*handler)(struct binder_state *bs,
                   struct binder_transaction_data *txn,
                   struct binder_io *msg,
                   struct binder_io *reply);

	handler = (int (*)(struct binder_state *bs,
                   struct binder_transaction_data *txn,
                   struct binder_io *msg,
                   struct binder_io *reply))txn->target.ptr;
	
	return handler(bs, txn, msg, reply);
}

int main(int argc, char **argv)
{
    int fd;
    struct binder_state *bs;
    uint32_t svcmgr = BINDER_SERVICE_MANAGER;
    uint32_t handle;
	int ret;

    bs = binder_open(128*1024);
    if (!bs) {
        fprintf(stderr, "failed to open binder driver\n");
        return -1;
    }

	/* add service */
	ret = svcmgr_publish(bs, svcmgr, "hello", hello_service_handler);
    if (ret) {
        fprintf(stderr, "failed to publish hello service\n");
        return -1;
    }
	ret = svcmgr_publish(bs, svcmgr, "goodbye", goodbye_service_handler);
    if (ret) {
        fprintf(stderr, "failed to publish goodbye service\n");
    }

	ret = svcmgr_publish(bs, svcmgr, "mem", mem_service_handler);
    if (ret) {
        fprintf(stderr, "failed to publish goodbye service\n");
    }


	binder_set_maxthreads(bs, 10);

    binder_loop(bs, test_server_handler);

    return 0;
}
