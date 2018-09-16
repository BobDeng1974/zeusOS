#ifndef __INTERFACE_H__
#define __INTERFACE_H__

#include <binder/Binder.h>



class IInterface {
public:
	IInterface()
	{
		mTargetHandle = -1;
	}
	int mTargetHandle;
};


template<typename INTERFACE>
inline INTERFACE *interface_cast(int iHandle)
{
    return INTERFACE::asInterface(iHandle);
}


#define DECLARE_META_INTERFACE(INTERFACE)                               \
    static I##INTERFACE *asInterface(int iHandle);                      \
	static const char *serviceName;                                     \


#define IMPLEMENT_META_INTERFACE(INTERFACE, NAME)                       \
    I##INTERFACE *I##INTERFACE::asInterface(int iHandle)\
    {                                                                   \
        I##INTERFACE *intr;                                             \
        intr = new Bp##INTERFACE(iHandle);                              \
        return intr;                                                    \
    }                                                                   \
    const char *I##INTERFACE::serviceName = NAME;                       \

#endif	//__INTERFACE_H__
