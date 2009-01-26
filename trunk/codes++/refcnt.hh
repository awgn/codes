/* $Id$ */
/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <bonelli@antifork.org> wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return. Nicola Bonelli 
 * ----------------------------------------------------------------------------
 */

#ifndef REFCNT_HH
#define REFCNT_HH

//
// this refcnt class leverages __gnu_cxx::atomicity
// provided by g++ 3.4 or higher.
//

#if   __GNUC__ >= 4
#include <tr1/memory>
#elif (__GNUC__ == 3) && (__GNUC_MINOR__ == 4)
#include <bits/atomicity.h>
#else
#error "g++ compiler not supported"
#endif

namespace more 
{
    namespace refcnt_policy 
    {
        struct SingleThread 
        {
            static _Atomic_word ref(int i)
            {
                static volatile _Atomic_word _ref;
                _Atomic_word ret = _ref; _ref += i;
                return ret;
            }   
        };

        struct MultiThread 
        {
            static _Atomic_word ref(int i)
            {
                static _Atomic_word _ref;
                return __gnu_cxx::__exchange_and_add(&_ref,i);
            }   
        };

    } // namespace refcnt_policy

    template <typename T, typename PolicyThread = refcnt_policy::MultiThread>
    class refcnt 
    {
    public:
        refcnt()  
        { PolicyThread::ref(1); }

        virtual ~refcnt() 
        { PolicyThread::ref(-1); }

        static _Atomic_word 
        counter() 
        { 
            return PolicyThread::ref(0); 
        }
    };
}

#endif /* REFCNT_HH */

