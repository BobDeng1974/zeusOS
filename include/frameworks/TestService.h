#ifndef __TESTSERVICE_H__
#define __TESTSERVICE_H__

#include <frameworks/ServiceManager.h>


#define HELLO_SVR_CMD_SAYHELLO_TO 1

class ITestService : public IInterface {
public:
	virtual int hello(const unsigned char *str) = 0;
	DECLARE_META_INTERFACE(TestService);
};


class BnTestService : public ITestService, public Binder {
public:
	virtual int hello(const unsigned char *str);
	
	virtual void binderDeath(void *ptr);

	virtual int onTransact(struct binder_transaction_data *txn, Parcel *msg, Parcel *reply);
};




class BpTestService : public ITestService {
public:
	BpTestService(){}
	BpTestService(Binder *binder, int iHandle)
	{		
		mBinder = binder;
		mTargetHandle = iHandle;
	}
	virtual int hello(const unsigned char *str) ;
private:
};



#endif //__TESTSERVICE_H__