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

class IServiceManager : public Binder{
public:
	virtual int getService(const unsigned char *name) = 0;
	virtual int addService(const unsigned char *name, void *ptr) = 0;
	DECLARE_META_INTERFACE(ServiceManager);

};


class BnServiceManager : public IServiceManager {
public:
	virtual int getService(const unsigned char *name);
	virtual int addService(const unsigned char *name, void *ptr);
	
	virtual void binderDeath(void *ptr);

	virtual int onTransact(struct binder_transaction_data *txn, Parcel *msg, Parcel *reply);
};




class BpServiceManager : public IServiceManager {
public:
	BpServiceManager(int iHandle){ mTargetHandle = iHandle; }
	virtual int getService(const unsigned char *name) ;
	virtual int addService(const unsigned char *name, void *ptr);
	virtual int onTransact(struct binder_transaction_data *txn, Parcel *msg, Parcel *reply) {return 0;}
private:
	BpServiceManager(){}
};




#endif	//__SERVICEMANAGER_H__
