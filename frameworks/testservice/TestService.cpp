#include <frameworks/TestService.h>
#include <stdio.h>


int BnTestService::hello(const unsigned char *str)
{
	for (int i = 0; i < 3; i++) {
		printf("hello %s\n", str);		
	}

	return 0;
}

void BnTestService::binderDeath(void *ptr)
{
	printf("binderDeath !!!\n");
}

int BnTestService::onTransact(struct binder_transaction_data *txn, Parcel *msg, Parcel *reply)
{
    unsigned char *s;
	unsigned char name[512];
    unsigned int len;
    unsigned int handle;
    unsigned int strict_policy;
	unsigned int iRet;
	int i;

	strict_policy = msg->getUint32();
	if (strict_policy != 0) {
		printf("strict_policy error!\n");
		reply->putUint32(-1);
		return -1;
	}


    switch(txn->code) {
    case HELLO_SVR_CMD_SAYHELLO_TO:
		s = msg->getString8(&len);

		if (s == NULL) {
			return -1;
		}
		for (i = 0; i < len; i++)
			name[i] = s[i];
		name[i] = '\0';

		iRet = hello(name);

		reply->putUint32(iRet);
		
        break;

    default:
        fprintf(stderr, "unknown code %d\n", txn->code);
        return -1;
    }

    return 0;

}


int BpTestService::hello(const unsigned char *str)
{
    int status;
	Parcel msg, reply;

	msg.putUint32(0);
	msg.putString8(str);


	if (binderCall(msg, reply, mTargetHandle, HELLO_SVR_CMD_SAYHELLO_TO, 0))
		return -1;

	status = reply.getUint32();

    binderDone(msg, reply);

	return status;

}


IMPLEMENT_META_INTERFACE(TestService, "testservice");


