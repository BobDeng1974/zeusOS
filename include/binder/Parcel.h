#ifndef __PARCEL_H__
#define __PARCEL_H__
class Parcel;


#include <binder/Binder.h>


class Parcel {
public:
	Parcel();
	virtual ~Parcel();
	void bioInitFromTxn(struct binder_transaction_data *txn);
	void *bioAlloc(unsigned int size);
	struct flat_binder_object *bioAllocObj(void);
	void putUint32(unsigned int iVal);
	void putFd(unsigned int iFd);
	void putObj(void *ptr);
	void putRef(unsigned int iHandle);
	void putString8(const char *str);
	void putString16(const unsigned short *str);
	void putString16_X(const unsigned short *_str);
	unsigned int getUint32(void);
	char *getString8(unsigned int *size);
	unsigned short *getString16(unsigned int *size);
	unsigned int getFd(void);
	unsigned int getRef(void);
	
	

	struct BinderIO *mBio;
private:
	void *get(unsigned int size);
	struct flat_binder_object *getObj(void);
	
	unsigned char *mIoData;
};


#endif	//__PARCEL_H__