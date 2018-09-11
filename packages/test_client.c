/* Copyright 2008 The Android Open Source Project
 */

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <linux/types.h>
#include<stdbool.h>
#include <string.h>

#include <private/android_filesystem_config.h>

#include "binder.h"
#include "test_server.h"

#include <unistd.h>


uint32_t svcmgr_lookup(struct binder_state *bs, uint32_t target, const char *name)
{
    uint32_t handle;
    unsigned iodata[512/4];
    struct binder_io msg, reply;

    bio_init(&msg, iodata, sizeof(iodata), 4);
    bio_put_uint32(&msg, 0);  // strict mode header
    bio_put_string16_x(&msg, SVC_MGR_NAME);
    bio_put_string16_x(&msg, name);

    if (binder_call(bs, &msg, &reply, target, SVC_MGR_CHECK_SERVICE))
        return 0;

    handle = bio_get_ref(&reply);

    if (handle)
        binder_acquire(bs, handle);

    binder_done(bs, &msg, &reply);

    return handle;
}


struct binder_state *g_bs;
uint32_t g_hello_handle;
uint32_t g_goodbye_handle;

uint32_t g_memservice_handle;

void sayhello(void)
{
    unsigned iodata[512/4];
    struct binder_io msg, reply;

	/* 构造binder_io */
    bio_init(&msg, iodata, sizeof(iodata), 4);
    bio_put_uint32(&msg, 0);  // strict mode header
    bio_put_string16_x(&msg, "IHelloService");

	/* 放入参数 */

	/* 调用binder_call */
    if (binder_call(g_bs, &msg, &reply, g_hello_handle, HELLO_SVR_CMD_SAYHELLO))
        return ;
	
	/* 从reply中解析出返回值 */

    binder_done(g_bs, &msg, &reply);
	
}

int sayhello_to(char *name)
{
	unsigned iodata[512/4];
	struct binder_io msg, reply;
	int ret;
	int exception;

	/* 构造binder_io */
	bio_init(&msg, iodata, sizeof(iodata), 4);
	bio_put_uint32(&msg, 0);  // strict mode header
    bio_put_string16_x(&msg, "IHelloService");

	/* 放入参数 */
    bio_put_string16_x(&msg, name);

	/* 调用binder_call */
	if (binder_call(g_bs, &msg, &reply, g_hello_handle, HELLO_SVR_CMD_SAYHELLO_TO))
		return 0;
	
	/* 从reply中解析出返回值 */
	exception = bio_get_uint32(&reply);
	if (exception)
		ret = -1;
	else
		ret = bio_get_uint32(&reply);

	binder_done(g_bs, &msg, &reply);

	return ret;
	
}


void saygoodbye(void)
{
    unsigned iodata[512/4];
    struct binder_io msg, reply;

	/* 构造binder_io */
    bio_init(&msg, iodata, sizeof(iodata), 4);
    bio_put_uint32(&msg, 0);  // strict mode header
    bio_put_string16_x(&msg, "IGoodbyeService");

	/* 放入参数 */

	/* 调用binder_call */
    if (binder_call(g_bs, &msg, &reply, g_goodbye_handle, GOODBYE_SVR_CMD_SAYGOODBYE))
        return ;
	
	/* 从reply中解析出返回值 */

    binder_done(g_bs, &msg, &reply);
	
}

int saygoodbye_to(char *name)
{
	unsigned iodata[512/4];
	struct binder_io msg, reply;
	int ret;
	int exception;

	/* 构造binder_io */
	bio_init(&msg, iodata, sizeof(iodata), 4);
	bio_put_uint32(&msg, 0);  // strict mode header
    bio_put_string16_x(&msg, "IGoodbyeService");

	/* 放入参数 */
    bio_put_string16_x(&msg, name);

	/* 调用binder_call */
	if (binder_call(g_bs, &msg, &reply, g_goodbye_handle, GOODBYE_SVR_CMD_SAYGOODBYE_TO))
		return 0;
	
	/* 从reply中解析出返回值 */
	exception = bio_get_uint32(&reply);
	if (exception)
		ret = -1;
	else
		ret = bio_get_uint32(&reply);

	binder_done(g_bs, &msg, &reply);

	return ret;
	
}


int getMemFd(void)
{
    unsigned iodata[512/4];
    struct binder_io msg, reply;
	int ret;
	int iFdOld;
	int iFdNew;

	/* 构造binder_io */
    bio_init(&msg, iodata, sizeof(iodata), 4);
    bio_put_uint32(&msg, 0);  // strict mode header
    bio_put_string16_x(&msg, "IHelloService");

	/* 放入参数 */

	/* 调用binder_call */
    if (binder_call(g_bs, &msg, &reply, g_memservice_handle, GET_MEM_FD))
        return ;
	
	/* 从reply中解析出返回值 */
	ret = bio_get_uint32(&reply);
	if (ret) {
		return -1;
	}
	printf("bio_get_fd\n");

	iFdOld = bio_get_fd(&reply);
	printf("iFdOld = %d\n", iFdOld);
	iFdNew = dup(iFdOld);

	printf("binder_done\n");
    binder_done(g_bs, &msg, &reply);

	return iFdNew;
}



/* ./test_client hello
 * ./test_client hello <name>
 */

int main(int argc, char **argv)
{
    int fd;
    struct binder_state *bs;
    uint32_t svcmgr = BINDER_SERVICE_MANAGER;
    uint32_t handle;
	int ret;

	if (argc < 2){
        fprintf(stderr, "Usage:\n");
        fprintf(stderr, "%s <hello|goodbye|mem>\n", argv[0]);
        fprintf(stderr, "%s <hello|goodbye> <name>\n", argv[0]);
        return -1;
	}

    bs = binder_open(128*1024);
    if (!bs) {
        fprintf(stderr, "failed to open binder driver\n");
        return -1;
    }
	g_bs = bs;


	/* get service */
	handle = svcmgr_lookup(bs, svcmgr, "goodbye");
	if (!handle) {
        fprintf(stderr, "failed to get goodbye service\n");
        return -1;
	}
	g_goodbye_handle = handle;
	fprintf(stderr, "Handle for goodbye service = %d\n", g_goodbye_handle);

	handle = svcmgr_lookup(bs, svcmgr, "hello");
	if (!handle) {
        fprintf(stderr, "failed to get hello service\n");
        return -1;
	}
	g_hello_handle = handle;
	fprintf(stderr, "Handle for hello service = %d\n", g_hello_handle);


	handle = svcmgr_lookup(bs, svcmgr, "mem");
	if (!handle) {
        fprintf(stderr, "failed to get hello service\n");
        return -1;
	}
	g_memservice_handle = handle;
	fprintf(stderr, "Handle for mem service = %d\n", g_memservice_handle);



	/* send data to server */
	if (!strcmp(argv[1], "hello"))
	{
		if (argc == 2) {
			sayhello();
		} else if (argc == 3) {
			ret = sayhello_to(argv[2]);
	        fprintf(stderr, "get ret of sayhello_to = %d\n", ret);		
		}
	} else if (!strcmp(argv[1], "goodbye"))
	{
		if (argc == 2) {
			saygoodbye();
		} else if (argc == 3) {
			ret = saygoodbye_to(argv[2]);
	        fprintf(stderr, "get ret of sayhello_to = %d\n", ret);		
		}
	} else if (!strcmp(argv[1], "mem")){
		unsigned char *buffer;
		int iRet;
		int iFd;
		int i;
		
		iFd = getMemFd();		
		printf("mmap start!\n");
		buffer =  (unsigned char *)mmap(NULL, 0x1000, PROT_READ | PROT_WRITE, MAP_SHARED, iFd, 0);
		printf("mmap end!\n");

		for (i = 0; i < 5; i++) {
			printf("buffer[%d] = 0x%x\n", i, buffer[i]);
		}
		
	}

	binder_release(bs, handle);

    return 0;
}
