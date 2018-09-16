#ifndef __INPUTMANAGER_H__
#define __INPUTMANAGER_H__

#include <binder/Binder.h>
#include <binder/Parcel.h>
#include <binder/Interface.h>
#include <list>
#include <sys/time.h>
#include <ThreadPool.h>


#define TOUCHSCREEN		(1 << 0)
#define NETWORKINPUT	(1 << 1)
#define ADDFILE			(1 << 2)
#define DELFILE			(1 << 3)
#define ADDDIR			(1 << 4)
#define DELDIR			(1 << 5)
#define ADDDEVICE		(1 << 6)
#define DELDEVICE		(1 << 7)
#define KEYEVENT		(1 << 8)

#define INPUTMANAGER_ADD_LISTENER	1
#define INPUTMANAGER_DEL_LISTENER	2


struct ListenerDesc {
	unsigned int iType;	//关心那些输入事件
	unsigned int iUid;	//PID & fd[0]
	int iFd;	//fd[1]
	char *pcName;
	int iHandle;
	struct BinderDeath *death;
};


struct CookEvent {
	struct timeval tTime;
	char *filePath;
	int iType;
	int iX;
	int iY;
	int iKey;
	int iPressure;
	int iTrackID;		//多点触控，轨迹ID
	int iTouchMajor;	//手指在TP上的面积
};


class IInputManager : public IInterface {
public:
	virtual int addListener(struct ListenerDesc *ptListenerDesc) = 0;
	virtual int delListener(unsigned int iUid) = 0;
	DECLARE_META_INTERFACE(InputManager);
};


class BnInputManager : public BnBinder, public IInputManager {
public:
	~BnInputManager();
	virtual int addListener(struct ListenerDesc *ptListenerDesc);
	virtual int delListener(unsigned int iUid);
	void dispatchEvent(struct CookEvent *cookEvent);
	
	virtual int onTransact(struct binder_transaction_data *txn, Parcel *msg, Parcel *reply);
	
	std::list<struct ListenerDesc*> mListenerDescList;
	static BnInputManager* get(void);
	ThreadPool *mInputReaderthread;
	ThreadPool *mDispatchthread;

private:
	BnInputManager();
	static BnInputManager *mBnInputManager;
	static pthread_mutex_t tMutex;
};




class BpInputManager : public IInputManager {
public:
	BpInputManager(){}
	BpInputManager(int iHandle)
	{		
		mTargetHandle = iHandle;
	}

	virtual int addListener(struct ListenerDesc *ptListenerDesc);
	virtual int delListener(unsigned int iUid);	
};


#endif	//__INPUTMANAGER_H__
