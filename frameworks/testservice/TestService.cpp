#include <frameworks/TestService.h>
#include <stdio.h>




BnTestService *BnTestService::mBnTestService = NULL;
pthread_mutex_t BnTestService::tMutex  = PTHREAD_MUTEX_INITIALIZER;


BnTestService* BnTestService::get(void)
{
	if (NULL == mBnTestService) {		
		pthread_mutex_lock(&tMutex);
		if (NULL == mBnTestService) {
			mBnTestService = new BnTestService();
		}
		pthread_mutex_unlock(&tMutex);
	}

	return mBnTestService;
}


BnTestService::BnTestService()
{

}


int BnTestService::hello(const char *str)
{
	for (int i = 0; i < 5; i++) {
		printf("hello %s\n", str);		
	}

	return 0;
}


int BnTestService::onTransact(struct binder_transaction_data *txn, Parcel *msg, Parcel *reply)
{
    char *str;
    unsigned int len;
    unsigned int handle;
    unsigned int strict_policy;
	unsigned int iRet = -1;
	int i;

	strict_policy = msg->getUint32();
	if (strict_policy != 0) {
		printf("BnTestService : strict_policy error!\n");
		reply->putUint32(-1);
		return -1;
	}


    switch(txn->code) {
    case HELLO_SVR_CMD_SAYHELLO_TO:
		str = msg->getString8(&len);

		iRet = hello(str);

		reply->putUint32(iRet);
		
        break;

    default:
        fprintf(stderr, "unknown code %d\n", txn->code);
        return -1;
    }

    return 0;

}


int BpTestService::hello(const   char *str)
{
    int status;
	Parcel msg, reply;

	msg.putUint32(0);
	msg.putString8(str);

	printf("BpTestService::mTargetHandle = %d\n", mTargetHandle);
	printf("BpTestService::str = %s\n", str);
	if (Binder::getBinder()->binderCall(msg, reply, mTargetHandle, HELLO_SVR_CMD_SAYHELLO_TO, 0))
		return -1;

	status = reply.getUint32();

    Binder::getBinder()->binderDone(msg, reply);

	return status;
}


IMPLEMENT_META_INTERFACE(TestService, "TestService");


