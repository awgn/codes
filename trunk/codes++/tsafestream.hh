/* $Id$ */
/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <bonelli@antifork.org> wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return. Nicola Bonelli
 * ----------------------------------------------------------------------------
 */

#ifndef _TSAFESTREAM_HH_
#define _TSAFESTREAM_HH_ 

#include <iostream>
#include <sched.h>
#include <tspinlock.hh>

// This class makes a stream treadsafe by means of a ticket spinlock 
// which is stored in the internal extensible array of the stream.
// Constructor-on-the-first-use idiom ensures that the iword index is
// unique across different compilation units.

namespace more { 

    struct tsafestream 
    {
        struct lock   {};
        struct unlock {};

        static int iword_index()
        {
            static int index = std::ios_base::xalloc();
            return index;
        }

        static inline std::ostream &spinlock(std::ostream &out)
        {
            volatile int & sp = reinterpret_cast<int &>(out.iword( iword_index()));
            tspinlock::lock(sp);
            return out;
        }

        static inline std::ostream &spinunlock(std::ostream &out)
        {
            volatile int & sp = reinterpret_cast<int &>(out.iword( iword_index())); 
            tspinlock::unlock(sp);
            return out;
        }
    };

} // namespace more

namespace std
{
    static inline
    std::ostream &operator<<(std::ostream &out, more::tsafestream::lock)
    {
        more::tsafestream::spinlock(out);
        return out;
    }    
    
    static inline
    std::ostream &operator<<(std::ostream &out, more::tsafestream::unlock)
    {
        more::tsafestream::spinunlock(out);
        return out;
    }
}

#endif /* _TSAFESTREAM_HH_ */
