#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <linux/types.h>
#include <stdbool.h>

#include <private/android_filesystem_config.h>

#include <frameworks/ServiceManager.h>


#if 1
#define ALOGI(x...) fprintf(stderr, "svcmgr: " x)
#define ALOGE(x...) fprintf(stderr, "svcmgr: " x)
#else
#define LOG_TAG "ServiceManager"
#include <cutils/log.h>
#endif



struct svcinfo
{
    struct svcinfo *next;
    uint32_t handle;
    BinderDeath death;
    int allow_isolated;
    size_t len;
    unsigned char name[0];
};

struct svcinfo *svclist = NULL;

struct svcinfo *find_svc(const unsigned char *name, size_t len)
{
    struct svcinfo *si;

    for (si = svclist; si; si = si->next) {
        if ((len == si->len) &&
            !memcmp(name, si->name, len)) {
            return si;
        }
    }
    return NULL;
}

unsigned char svcmgr_id[] = {
    'a','n','d','r','o','i','d','.','o','s','.',
    'I','S','e','r','v','i','c','e','M','a','n','a','g','e','r'
};


int BnServiceManager::getService(const unsigned char *name)
{
    struct svcinfo *si;
	int len;

	len = strlen((char *)name);
    si = find_svc(name, len);

    if (si && si->handle) {
        return si->handle;
    } else {
        return 0;
    }

}

int BnServiceManager::addService(const unsigned char *name, void *ptr)
{
    struct svcinfo *si;
	struct BinderRef *ptBinderRef = (struct BinderRef *)ptr;


    if (!ptBinderRef->handle || (ptBinderRef->len == 0) || (ptBinderRef->len > 127))
        return -1;


    si = find_svc(name, ptBinderRef->len);
    if (si) {
        if (si->handle) {
            ALOGE("add_service('%s',%x) uid=%d - ALREADY REGISTERED, OVERRIDE\n", name, ptBinderRef->handle, ptBinderRef->sender_euid);
            binderDeath((void *)si);
        }
        si->handle = ptBinderRef->handle;
    } else {
        si = (struct svcinfo *)malloc(sizeof(*si) + (ptBinderRef->len + 1) * sizeof(unsigned short));
        if (!si) {
            ALOGE("add_service('%s',%x) uid=%d - OUT OF MEMORY\n", name, ptBinderRef->handle, ptBinderRef->sender_euid);
            return -1;
        }
        si->handle = ptBinderRef->handle;
        si->len = ptBinderRef->len;
        memcpy(si->name, name, (ptBinderRef->len + 1));
        si->name[ptBinderRef->len] = '\0';
		si->death.binder = this;
        si->death.ptr = si;
        si->allow_isolated = ptBinderRef->allow_isolated;
        si->next = svclist;
        svclist = si;
    }

    ALOGI("add_service('%s'), handle = %d\n", name, ptBinderRef->handle);

    binderAcquire(ptBinderRef->handle);
    binderLinkToDeath(ptBinderRef->handle, &si->death);
    return 0;

}

void BnServiceManager::binderDeath(void *ptr)
{
    struct svcinfo *si = (struct svcinfo* ) ptr;

    ALOGI("service '%s' died\n", si->name);
    if (si->handle) {
        binderRelease(si->handle);
		
        si->handle = 0;
    }

}


int BnServiceManager::onTransact(struct binder_transaction_data *txn, Parcel *msg, Parcel *reply)
{
	struct BinderRef tBinderRef;
    unsigned char *name;
    unsigned int len;
    unsigned int handle;
    unsigned int strict_policy;
    int allow_isolated;


    if (txn->target.handle != BINDER_SERVICE_MANAGER)
        return -1;

    if (txn->code == PING_TRANSACTION)
        return 0;


    strict_policy = msg->getUint32();



    switch(txn->code) {
    case SVC_MGR_GET_SERVICE:
    case SVC_MGR_CHECK_SERVICE:
		name = msg->getString8(&len);
        if (name == NULL) {
            return -1;
        }

		tBinderRef.len = len;
		tBinderRef.sender_euid = txn->sender_euid;
		tBinderRef.sender_pid = txn->sender_pid;
        handle = getService(name);
        if (!handle)
            break;
		reply->putRef(handle);
        return 0;

    case SVC_MGR_ADD_SERVICE:
		name = msg->getString8(&len);
        if (name == NULL) {
            return -1;
        }
		handle = msg->getRef();
		allow_isolated = msg->getUint32() ? 1 : 0;

		tBinderRef.len = len;
		tBinderRef.sender_euid = txn->sender_euid;
		tBinderRef.sender_pid = txn->sender_pid;
		tBinderRef.allow_isolated = allow_isolated;
		tBinderRef.handle = handle;
			
        if (addService(name, (void *)&tBinderRef))
            return -1;
        break;

    default:
        ALOGE("unknown code %d\n", txn->code);
        return -1;
    }

	reply->putUint32(0);
    return 0;

}


int BpServiceManager::getService(const unsigned char *name)
{
    unsigned int handle;
	Parcel msg, reply;

	msg.putUint32(0);
	msg.putString8(name);


    if (mBinder->binderCall(msg, reply, mTargetHandle, SVC_MGR_CHECK_SERVICE, 0))
        return -1;

	handle = reply.getRef();

    if (handle)
        mBinder->binderAcquire(handle);

    mBinder->binderDone(msg, reply);

    return handle;

}

int BpServiceManager::addService(const unsigned char *name, void *ptr)
{
    int status;
	Parcel msg, reply;

	msg.putUint32(0);
	msg.putString8(name);
	msg.putObj(ptr);


	if (mBinder->binderCall(msg, reply, mTargetHandle, SVC_MGR_ADD_SERVICE, 0))
		return -1;

	status = reply.getUint32();

    mBinder->binderDone(msg, reply);

	return status;
}


IMPLEMENT_META_INTERFACE(ServiceManager, "servicemanager");


