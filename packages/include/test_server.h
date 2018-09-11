

#ifndef _TEST_SERVER_H
#define _TEST_SERVER_H
#include <linux/ashmem.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>

#define HELLO_SVR_CMD_SAYHELLO     1
#define HELLO_SVR_CMD_SAYHELLO_TO  2

#define GOODBYE_SVR_CMD_SAYGOODBYE     1
#define GOODBYE_SVR_CMD_SAYGOODBYE_TO  2

#define GET_MEM_FD	1

#endif // _TEST_SERVER_H

