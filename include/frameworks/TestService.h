#ifndef __TESTSERVICE_H__
#define __TESTSERVICE_H__

#include <frameworks/ServiceManager.h>


#define HELLO_SVR_CMD_SAYHELLO_TO 1

class ITestService : public IInterface {
public:
	
	virtual int hello(const char *str) = 0;
	DECLARE_META_INTERFACE(TestService);
};

class BnTestService : public BnBinder, public ITestService {
public:
	virtual int hello(const char *str);
	virtual int onTransact(struct binder_transaction_data *txn, Parcel *msg, Parcel *reply);
	static BnTestService* get(void);
private:
	BnTestService();
	static BnTestService *mBnTestService;
	static pthread_mutex_t tMutex;
};




class BpTestService : public ITestService {
public:
	BpTestService(){}
	BpTestService(int iHandle)
	{		
		mTargetHandle = iHandle;
	}
	virtual int hello(const char *str) ;
private:
};



#endif //__TESTSERVICE_H__
