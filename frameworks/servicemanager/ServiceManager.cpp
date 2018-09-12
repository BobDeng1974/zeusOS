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

uint32_t svcmgr_handle;

const char *str8(const uint16_t *x, size_t x_len)
{
    static char buf[128];
    size_t max = 127;
    char *p = buf;

    if (x_len < max) {
        max = x_len;
    }

    if (x) {
        while ((max > 0) && (*x != '\0')) {
            *p++ = *x++;
            max--;
        }
    }
    *p++ = 0;
    return buf;
}

int str16eq(const uint16_t *a, const char *b)
{
    while (*a && *b)
        if (*a++ != *b++) return 0;
    if (*a || *b)
        return 0;
    return 1;
}

static char *service_manager_context;

static bool check_mac_perms(pid_t spid, const char *tctx, const char *perm, const char *name)
{
    return true;
}

static bool check_mac_perms_from_getcon(pid_t spid, const char *perm)
{
    return true;
}

static bool check_mac_perms_from_lookup(pid_t spid, const char *perm, const char *name)
{
    return true;
}

static int svc_can_register(const uint16_t *name, size_t name_len, pid_t spid)
{
    const char *perm = "add";
    return check_mac_perms_from_lookup(spid, perm, str8(name, name_len)) ? 1 : 0;
}

static int svc_can_list(pid_t spid)
{
    const char *perm = "list";
    return check_mac_perms_from_getcon(spid, perm) ? 1 : 0;
}

static int svc_can_find(const uint16_t *name, size_t name_len, pid_t spid)
{
    const char *perm = "find";
    return check_mac_perms_from_lookup(spid, perm, str8(name, name_len)) ? 1 : 0;
}

struct svcinfo
{
    struct svcinfo *next;
    uint32_t handle;
    struct binder_death death;
    int allow_isolated;
    size_t len;
    uint16_t name[0];
};

struct svcinfo *svclist = NULL;

struct svcinfo *find_svc(const uint16_t *s16, size_t len)
{
    struct svcinfo *si;

    for (si = svclist; si; si = si->next) {
        if ((len == si->len) &&
            !memcmp(s16, si->name, len * sizeof(uint16_t))) {
            return si;
        }
    }
    return NULL;
}

void BnServiceManager::svcinfoDeath(void *ptr)
{
    struct svcinfo *si = (struct svcinfo* ) ptr;

    ALOGI("service '%s' died\n", str8(si->name, si->len));
    if (si->handle) {
        binderRelease(si->handle);
		
        si->handle = 0;
    }
}

uint16_t svcmgr_id[] = {
    'a','n','d','r','o','i','d','.','o','s','.',
    'I','S','e','r','v','i','c','e','M','a','n','a','g','e','r'
};


int BnServiceManager::getService(const unsigned short *name)
{
    struct svcinfo *si;
	int len;

	len = strlen(name);
    si = find_svc(name, len);

    if (si && si->handle) {
        return si->handle;
    } else {
        return 0;
    }

}

int BnServiceManager::addService(const unsigned short *name, void *ptr)
{
    struct svcinfo *si;
	BinderRef *ptBinderRef = (BinderRef *)ptr;


    if (!ptBinderRef->handle || (ptBinderRef->len == 0) || (ptBinderRef->len > 127))
        return -1;


    si = find_svc(name, ptBinderRef->len);
    if (si) {
        if (si->handle) {
            ALOGE("add_service('%s',%x) uid=%d - ALREADY REGISTERED, OVERRIDE\n", str8(name, ptBinderRef->len), ptBinderRef->handle, ptBinderRef->sender_euid);
            svcinfoDeath((void *)si);
        }
        si->handle = ptBinderRef->handle;
    } else {
        si = malloc(sizeof(*si) + (ptBinderRef->len + 1) * sizeof(unsigned short));
        if (!si) {
            ALOGE("add_service('%s',%x) uid=%d - OUT OF MEMORY\n", str8(name, ptBinderRef->len), ptBinderRef->handle, ptBinderRef->sender_euid);
            return -1;
        }
        si->handle = ptBinderRef->handle;
        si->len = ptBinderRef->len;
        memcpy(si->name, name, (ptBinderRef->len + 1) * sizeof(unsigned short));
        si->name[ptBinderRef->len] = '\0';
        si->death.func = (void*) svcinfo_death;
        si->death.ptr = si;
        si->allow_isolated = ptBinderRef->allow_isolated;
        si->next = svclist;
        svclist = si;
    }

    ALOGI("add_service('%s'), handle = %d\n", str8(name, ptBinderRef->len), ptBinderRef->handle);

    binderAcquire(ptBinderRef->handle);
    binderLinkToDeath(ptBinderRef->handle, &si->death);
    return 0;

}

void BnServiceManager::onTransact(struct binder_transaction_data *txn, Parcel *msg, Parcel *reply)
{
    struct svcinfo *si;
    uint16_t *s;
    size_t len;
    uint32_t handle;
    uint32_t strict_policy;
    int allow_isolated;


    if (txn->target.handle != BINDER_SERVICE_MANAGER)
        return -1;

    if (txn->code == PING_TRANSACTION)
        return 0;


    strict_policy = msg.getUint32();
	s = msg.getString16(&len);
    if (s == NULL) {
        return -1;
    }

    if ((len != (sizeof(svcmgr_id) / 2)) ||
        memcmp(svcmgr_id, s, sizeof(svcmgr_id))) {
        fprintf(stderr,"invalid id %s\n", str8(s, len));
        return -1;
    }


    switch(txn->code) {
    case SVC_MGR_GET_SERVICE:
    case SVC_MGR_CHECK_SERVICE:
		s = msg.getString16(&len);
        if (s == NULL) {
            return -1;
        }
		BinderRef tBinderRef;
		tBinderRef.len = len;
		tBinderRef.sender_euid = txn->sender_euid;
		tBinderRef.sender_pid = txn->sender_pid;
        handle = getService(s);
        if (!handle)
            break;
        bio_put_ref(reply, handle);
        return 0;

    case SVC_MGR_ADD_SERVICE:
		s = msg.getString16(&len);
        if (s == NULL) {
            return -1;
        }
		handle = msg.getRef();
		allow_isolated = msg.getUint32() ? 1 : 0;

		BinderRef tBinderRef;
		tBinderRef.len = len;
		tBinderRef.sender_euid = txn->sender_euid;
		tBinderRef.sender_pid = txn->sender_pid;
		tBinderRef.allow_isolated = allow_isolated;
		tBinderRef.handle = handle;
			
        if (addService(s, (void *)&tBinderRef))
            return -1;
        break;

    default:
        ALOGE("unknown code %d\n", txn->code);
        return -1;
    }

    bio_put_uint32(reply, 0);
    return 0;

}


int BpServiceManager::getService(const unsigned short *name)
{
    uint32_t handle;
	Parcel msg, reply;

	msg.putUint32(0);
	msg.putString16_X(SVC_MGR_NAME);
	msg.putString16_X(name);


    if (binderCall(&msg, &reply, BINDER_SERVICE_MANAGER, SVC_MGR_CHECK_SERVICE, 0))
        return 0;

	handle = reply.getRef();

    if (handle)
        binderAcquire(handle);

    binderDone(&msg, &reply);

    return handle;

}

int BpServiceManager::addService(const unsigned short *name, void *ptr)
{
    int status;
	Parcel msg, reply;

	msg.putUint32(0);
	msg.putString16_X(SVC_MGR_NAME);
	msg.putString16_X(name);
	msg.putObj(ptr);


	if (binderCall(&msg, &reply, BINDER_SERVICE_MANAGER, SVC_MGR_ADD_SERVICE, 0))
		return -1;

	status = reply.getUint32();

    binderDone(&msg, &reply);

	return status;
}


