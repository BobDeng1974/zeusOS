#ifndef __SERVICEMANAGER_H__
#define __SERVICEMANAGER_H__

#include <binder/Binder.h>
#include <binder/Parcel.h>



struct BinderRef {
	int len;
	int sender_euid;
	int sender_pid;
	int allow_isolated;
	unsigned int handle;
};

class IServiceManager : public Binder{
public:
	virtual int getService(const unsigned short *str) = 0;
	virtual void addService(const unsigned short *str, void *ptr) = 0;
	
};


class BnServiceManager : public IServiceManager {
public:
	virtual int getService(const unsigned short *name);
	virtual int addService(const unsigned short *name, void *ptr);
	
	virtual void binderDeath(void *ptr);

	virtual int onTransact(struct binder_transaction_data *txn, Parcel *msg, Parcel *reply);

private:
};




class BpServiceManager : public IServiceManager {
public:
	virtual int getService(const unsigned short *name) ;
	virtual int addService(const unsigned short *name, void *ptr);

	virtual int onTransact(struct binder_transaction_data *txn, Parcel *msg, Parcel *reply){/* nothing */}

private:
	
};

#endif	//__SERVICEMANAGER_H__
