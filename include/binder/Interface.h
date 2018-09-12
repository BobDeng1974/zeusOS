#ifndef __INTERFACE_H__
#define __INTERFACE_H__

#include <binder/Binder.h>



template<typename INTERFACE>
inline INTERFACE *interface_cast(const Binder *obj)
{
    return INTERFACE::asInterface(obj);
}


#define DECLARE_META_INTERFACE(INTERFACE)                               \
    static I##INTERFACE *asInterface(const Binder *obj);                \
    I##INTERFACE();                                                     \
    virtual ~I##INTERFACE();                                            \


#define IMPLEMENT_META_INTERFACE(INTERFACE, NAME)                       \
    I##INTERFACE *I##INTERFACE::asInterface(const Binder *obj)          \
    {                                                                   \
        I##INTERFACE *intr;                                             \
        if (obj != NULL) {                                              \
            intr = new Bp##INTERFACE(obj);                              \
        }                                                               \
        return intr;                                                    \
    }                                                                   \
    I##INTERFACE::I##INTERFACE() { }                                    \
    I##INTERFACE::~I##INTERFACE() { }                                   \


#endif	//__INTERFACE_H__
