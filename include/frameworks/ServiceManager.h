#ifndef __SERVICEMANAGER_H__
#define __SERVICEMANAGER_H__

#include <binder/Binder.h>
#include <binder/Parcel.h>
#include <binder/Interface.h>



struct BinderRef {
	int len;
	int sender_euid;
	int sender_pid;
	int allow_isolated;
	unsigned int handle;
};

class IServiceManager : public IInterface {
public:
	virtual int getService(const char *name) = 0;
	virtual int addService(const char *name, void *ptr) = 0;
	DECLARE_META_INTERFACE(ServiceManager);
};


class BnServiceManager : public BnBinder, public IServiceManager {
public:	
	virtual int getService(const char *name);
	virtual int addService(const char *name, void *ptr);
	
	virtual int onTransact(struct binder_transaction_data *txn, Parcel *msg, Parcel *reply);
	static BnServiceManager* get(void);
private:
	BnServiceManager();
	static BnServiceManager *mBnServiceManager;
	static pthread_mutex_t tMutex;
};




class BpServiceManager : public IServiceManager {
public:
	BpServiceManager() {}
	BpServiceManager(int iHandle)
	{		
		mTargetHandle = iHandle;
	}
	virtual int getService(const char *name) ;
	virtual int addService(const char *name, void *ptr);
	
};




#endif	//__SERVICEMANAGER_H__
