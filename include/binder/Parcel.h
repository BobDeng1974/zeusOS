#ifndef __PARCEL_H__
#define __PARCEL_H__

#include <linux/types.h>
#include <binder/Binder.h>
#include <string>

class Parcel {
public:
	Parcel();
	virtual ~Parcel();
	void Parcel::bioInitFromTxn(struct binder_transaction_data *txn);
	void *bioAlloc(unsigned int size);
	struct flat_binder_object *bioAllocObj(void);
	void putUint32(unsigned int iVal);
	void putFd(unsigned int iFd);
	void putObj(void *ptr);
	void putRef(unsigned int iHandle);
	void putString16(const unsigned short *str);
	void putString16_X(const unsigned short *str);
	unsigned int getUint32(void);
	unsigned short *getString16(unsigned int *size);
	unsigned int getFd(void);
	unsigned int getRef(void);
	
	

	BinderIO mBio;
private:
	void *get(unsigned int size);
	struct flat_binder_object *getObj(void);
	
	unsigned char *mIoData;
};


#endif	//__PARCEL_H__