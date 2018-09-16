#include <frameworks/InputManager.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>


static void inputBinderDeath(void *ptr)
{
	struct ListenerDesc *ptListenerDesc = (struct ListenerDesc *)ptr;

	printf("input Listener '%s' died\n", ptListenerDesc->pcName);
    if (ptListenerDesc->iHandle) {
		BnInputManager::get()->mListenerDescList.remove(ptListenerDesc);
		Binder::getBinder()->binderRelease(ptListenerDesc->iHandle);
		free(ptListenerDesc->death);
		free(ptListenerDesc);
    }
}



BnInputManager *BnInputManager::mBnInputManager = NULL;
pthread_mutex_t BnInputManager::tMutex  = PTHREAD_MUTEX_INITIALIZER;


BnInputManager* BnInputManager::get(void)
{
	if (NULL == mBnInputManager) {		
		pthread_mutex_lock(&tMutex);
		if (NULL == mBnInputManager) {
			mBnInputManager = new BnInputManager();
		}
		pthread_mutex_unlock(&tMutex);
	}

	return mBnInputManager;
}


BnInputManager::BnInputManager()
{
	mInputReaderthread = new ThreadPool();
	mDispatchthread = new ThreadPool();

	mDispatchthread->set_pool_size(1);	
    mInputReaderthread->set_pool_size(1);	
}

BnInputManager::~BnInputManager()
{
	delete mInputReaderthread;
	delete mDispatchthread;
}


void BnInputManager::dispatchEvent(struct CookEvent *cookEvent)
{
	for(std::list<struct ListenerDesc*>::iterator listenerDescList_iter = mListenerDescList.begin(); listenerDescList_iter != mListenerDescList.end(); ++listenerDescList_iter) {
		if (cookEvent->iType & (*(listenerDescList_iter))->iType) {
			write((*(listenerDescList_iter))->iFd, cookEvent, sizeof(struct CookEvent));
		}
	}
	
}



int BnInputManager::addListener(struct ListenerDesc *ptListenerDesc)
{
	printf("BnInputManager::addListener\n");

	if (ptListenerDesc->iHandle) {
	    Binder::getBinder()->binderAcquire(ptListenerDesc->iHandle);
    	Binder::getBinder()->binderLinkToDeath(ptListenerDesc->iHandle, ptListenerDesc->death);
	}

	mListenerDescList.push_back(ptListenerDesc);
	return 0;
}

int BnInputManager::delListener(unsigned int iUid)
{
	printf("BnInputManager::delListener\n");

	for(std::list<struct ListenerDesc*>::iterator listenerDescList_iter = mListenerDescList.begin(); listenerDescList_iter != mListenerDescList.end(); ++listenerDescList_iter) {
		if (iUid == (*(listenerDescList_iter))->iUid) {
			mListenerDescList.remove((*(listenerDescList_iter)));
			Binder::getBinder()->binderRelease((*(listenerDescList_iter))->iHandle);
			free((*(listenerDescList_iter))->death);
			free((*(listenerDescList_iter)));
			return 0;
		}
	}

	return -1;
}


int BnInputManager::onTransact(struct binder_transaction_data *txn, Parcel *msg, Parcel *reply)
{
    char *s;
	char name[512];
    unsigned int len;
    unsigned int handle;
    unsigned int strict_policy;
	unsigned int iRet;
	int i;

	strict_policy = msg->getUint32();
	if (strict_policy != 0) {
		printf("strict_policy error!\n");
		reply->putUint32(-1);
		return -1;
	}


    switch(txn->code) {
    case INPUTMANAGER_ADD_LISTENER:
    {
		struct ListenerDesc *ptListenerDesc = (struct ListenerDesc *)malloc(sizeof(struct ListenerDesc));
		
		ptListenerDesc->iType = msg->getUint32();
		ptListenerDesc->iUid = msg->getUint32();
		ptListenerDesc->iFd = dup(msg->getFd());
		ptListenerDesc->pcName = msg->getString8(&len);
		ptListenerDesc->iHandle = msg->getRef();

		ptListenerDesc->death = (struct BinderDeath *)malloc(sizeof(struct BinderDeath));
		ptListenerDesc->death->ptr = ptListenerDesc;
		ptListenerDesc->death->binderDeath = inputBinderDeath;

		iRet = addListener(ptListenerDesc);
		
		reply->putUint32(iRet);
		
		break;
	}

    case INPUTMANAGER_DEL_LISTENER:
    {
		unsigned int iUid;
		iUid = msg->getUint32();
		
		iRet = delListener(iUid);
		
		reply->putUint32(iRet);
		
		break;
	}

    default:
        fprintf(stderr, "unknown code %d\n", txn->code);
        return -1;
    }

    return 0;

}


int BpInputManager::addListener(struct ListenerDesc *ptListenerDesc)
{
    int status;
	Parcel msg, reply;

	msg.putUint32(0);
	msg.putUint32(ptListenerDesc->iType);
	msg.putUint32(ptListenerDesc->iUid);
	msg.putFd(ptListenerDesc->iFd);
	msg.putString8(ptListenerDesc->pcName);
	msg.putObj(this);

	if (Binder::getBinder()->binderCall(msg, reply, mTargetHandle, INPUTMANAGER_ADD_LISTENER, TF_ACCEPT_FDS))
		return -1;

	status = reply.getUint32();

    Binder::getBinder()->binderDone(msg, reply);

	return status;

}

int BpInputManager::delListener(unsigned int iUid)
{
    int status;
	Parcel msg, reply;

	msg.putUint32(0);
	msg.putUint32(iUid);

	if (Binder::getBinder()->binderCall(msg, reply, mTargetHandle, INPUTMANAGER_DEL_LISTENER, 0))
		return -1;

	status = reply.getUint32();

    Binder::getBinder()->binderDone(msg, reply);

	return status;
}



IMPLEMENT_META_INTERFACE(InputManager, "InputManager");



