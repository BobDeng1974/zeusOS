#include <binder/Parcel.h>
#include <string.h>




#define MAX_BIO_SIZE (1 << 30)


Parcel::Parcel()
{
	int iMax = 1024;
	int iMaxOffs = 4;
	int n = iMaxOffs * sizeof(int);

	mIoData = (unsigned char *)new char[iMax];
	mBio = new BinderIO();

    if (n > iMax) {
        mBio->flags = BIO_F_OVERFLOW;
        mBio->data_avail = 0;
        mBio->offs_avail = 0;
        return;
    }

    mBio->data = mBio->data0 = (char *) mIoData + n;
    mBio->offs = mBio->offs0 = (binder_size_t *)mIoData;
    mBio->data_avail = iMax - n;
    mBio->offs_avail = iMaxOffs;
    mBio->flags = 0;

	
}

Parcel::~Parcel()
{
	delete mIoData;
	delete mBio;
}

void Parcel::bioInitFromTxn(struct binder_transaction_data *txn)
{
    mBio->data = mBio->data0 = (char *)(intptr_t)txn->data.ptr.buffer;
    mBio->offs = mBio->offs0 = (binder_size_t *)(intptr_t)txn->data.ptr.offsets;
    mBio->data_avail = txn->data_size;
    mBio->offs_avail = txn->offsets_size / sizeof(size_t);
    mBio->flags = BIO_F_SHARED;
}


void *Parcel::bioAlloc(unsigned int size)
{
    size = (size + 3) & (~3);
    if ((int)size > mBio->data_avail) {
        mBio->flags |= BIO_F_OVERFLOW;
        return NULL;
    } else {
        void *ptr = mBio->data;
        mBio->data += size;
        mBio->data_avail -= size;
        return ptr;
    }
}


struct flat_binder_object *Parcel::bioAllocObj(void)
{
    struct flat_binder_object *obj;

    obj = (struct flat_binder_object *)bioAlloc(sizeof(*obj));

    if (obj && mBio->offs_avail) {
        mBio->offs_avail--;
        *mBio->offs++ = ((char*) obj) - ((char*) mBio->data0);
        return obj;
    }

    mBio->flags |= BIO_F_OVERFLOW;
    return NULL;
}


void Parcel::putUint32(unsigned int iVal)
{
    unsigned int *ptr = (unsigned int *)bioAlloc(sizeof(iVal));
    if (ptr)
        *ptr = iVal;
}

void Parcel::putFd(unsigned int iFd)
{
    struct flat_binder_object *obj;

    obj = bioAllocObj();
    if (!obj)
        return;

    obj->flags = 0x7f | FLAT_BINDER_FLAG_ACCEPTS_FDS;
    obj->type = BINDER_TYPE_FD;
    obj->handle = iFd;
    obj->cookie = 0;
}
void Parcel::putObj(void *ptr)
{
    struct flat_binder_object *obj;

    obj = bioAllocObj();
    if (!obj)
        return;

    obj->flags = 0x7f | FLAT_BINDER_FLAG_ACCEPTS_FDS;
    obj->type = BINDER_TYPE_BINDER;
    obj->binder = (binder_uintptr_t)ptr;
    obj->cookie = 0;
}

void Parcel::putRef(unsigned int iHandle)
{
    struct flat_binder_object *obj;

    if (iHandle)
        obj = bioAllocObj();
    else
        obj = (struct flat_binder_object *)bioAlloc(sizeof(*obj));

    if (!obj)
        return;

    obj->flags = 0x7f | FLAT_BINDER_FLAG_ACCEPTS_FDS;
    obj->type = BINDER_TYPE_HANDLE;
    obj->handle = iHandle;
    obj->cookie = 0;
}

void Parcel::putString8(const char *str)
{
    unsigned int len;
    char *ptr;

    if (!str) {
        putUint32(0xffffffff);
        return;
    }

    len = 0;
    while (str[len]) len++;

    if (len >= (MAX_BIO_SIZE / sizeof(char))) {
        putUint32(0xffffffff);
        return;
    }

    /* Note: The payload will carry 32bit size instead of size_t */
    putUint32(len);
    len = (len + 1) * sizeof(char);
    ptr = (char *)bioAlloc(len);
    if (ptr)
        memcpy(ptr, str, len);

}


void Parcel::putString16(const unsigned short *str)
{
    unsigned int len;
    unsigned short *ptr;

    if (!str) {
        putUint32(0xffffffff);
        return;
    }

    len = 0;
    while (str[len]) len++;

    if (len >= (MAX_BIO_SIZE / sizeof(unsigned short))) {
        putUint32(0xffffffff);
        return;
    }

    /* Note: The payload will carry 32bit size instead of size_t */
    putUint32(len);
    len = (len + 1) * sizeof(unsigned short);
    ptr = (unsigned short *)bioAlloc(len);
    if (ptr)
        memcpy(ptr, str, len);

}

void Parcel::putString16_X(const unsigned short *_str)
{
    unsigned char *str = (unsigned char*) _str;
    unsigned int len;
    unsigned short *ptr;

    if (!str) {
        putUint32(0xffffffff);
        return;
    }

    len = strlen((char *)str);

    if (len >= (MAX_BIO_SIZE / sizeof(unsigned short))) {
        putUint32(0xffffffff);
        return;
    }

    /* Note: The payload will carry 32bit size instead of size_t */
    putUint32(len);
    ptr = (unsigned short *)bioAlloc((len + 1) * sizeof(unsigned short));
    if (!ptr)
        return;

    while (*str)
        *ptr++ = *str++;
    *ptr++ = 0;

}



void *Parcel::get(unsigned int size)
{
    size = (size + 3) & (~3);

    if (mBio->data_avail < size){
        mBio->data_avail = 0;
        mBio->flags |= BIO_F_OVERFLOW;
        return NULL;
    }  else {
        void *ptr = mBio->data;
        mBio->data += size;
        mBio->data_avail -= size;
        return ptr;
    }
}

unsigned int Parcel::getUint32(void)
{
    unsigned int *ptr = (unsigned int *)get(sizeof(*ptr));
    return ptr ? *ptr : 0;
}

char *Parcel::getString8(unsigned int *size)
{
    unsigned int len;

    /* Note: The payload will carry 32bit size instead of size_t */
    len = getUint32();
    if (size)
        *size = len;
    return (char *)get((len + 1) * sizeof(char));
}


unsigned short *Parcel::getString16(unsigned int *size)
{
    unsigned int len;

    /* Note: The payload will carry 32bit size instead of size_t */
    len = getUint32();
    if (size)
        *size = len;
    return (unsigned short *)get((len + 1) * sizeof(unsigned short));
}


struct flat_binder_object *Parcel::getObj(void)
{
    int n;
    int off = mBio->data - mBio->data0;

    /* TODO: be smarter about this? */
    for (n = 0; n < mBio->offs_avail; n++) {
        if (mBio->offs[n] == off)
            return (struct flat_binder_object *)get(sizeof(struct flat_binder_object));
    }

    mBio->data_avail = 0;
    mBio->flags |= BIO_F_OVERFLOW;
    return NULL;
}

unsigned int Parcel::getFd(void)
{
    struct flat_binder_object *obj;

    obj = getObj();
    if (!obj)
        return 0;

    if (obj->type == BINDER_TYPE_FD)
        return obj->handle;

    return 0;
}

unsigned int Parcel::getRef(void)
{
    struct flat_binder_object *obj;

    obj = getObj();
    if (!obj)
        return 0;

    if (obj->type == BINDER_TYPE_HANDLE)
        return obj->handle;

    return 0;
}

