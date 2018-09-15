#ifndef __INTERFACE_H__
#define __INTERFACE_H__

#include <binder/Binder.h>



class IInterface {
public:
	IInterface() {}
	IInterface(Binder *binder, int iHandle)
	{		
		mBinder = binder;
		mTargetHandle = iHandle;
	}
	Binder *mBinder;
	int mTargetHandle;
};


template<typename INTERFACE>
inline INTERFACE *interface_cast(Binder *binder, int iHandle)
{
    return INTERFACE::asInterface(binder, iHandle);
}


#define DECLARE_META_INTERFACE(INTERFACE)                               \
    static I##INTERFACE *asInterface(Binder *binder, int iHandle);      \
	static const char *serviceName;                                     \


#define IMPLEMENT_META_INTERFACE(INTERFACE, NAME)                       \
    I##INTERFACE *I##INTERFACE::asInterface(Binder *binder, int iHandle)\
    {                                                                   \
        I##INTERFACE *intr;                                             \
        intr = new Bp##INTERFACE(binder, iHandle);                      \
        return intr;                                                    \
    }                                                                   \
    const char *I##INTERFACE::serviceName = NAME;                       \

#endif	//__INTERFACE_H__
