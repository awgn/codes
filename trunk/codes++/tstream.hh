/* $Id$ */
/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <bonelli@antifork.org> wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return. Nicola Bonelli
 * ----------------------------------------------------------------------------
 */

#ifndef _TSTREAM_HH_
#define _TSTREAM_HH_ 

#include <iostream>
#include <tspinlock.hh>
#include <pthread.h>

namespace more { 

    /////////////////////////////////////////////////////////////////////////////////////////////////////////
    // tstreambuf is cancel-safe streambuf againts the pthread_cancel() by means of pthread_setcancelstate(). 
    // see http://www.codesourcery.com/archives/c++-pthreads/threads.html for more information 

    class tstreambuf : public std::streambuf
    {
    public:

        tstreambuf(std::streambuf *out)
        : _M_out(out)
        {}

    protected:

        virtual std::streamsize
        xsputn (const char *s, std::streamsize n)
        {
            int store;
            int ret;
            pthread_setcancelstate(PTHREAD_CANCEL_DISABLE,&store);
            ret = _M_out->sputn(s,n);
            pthread_setcancelstate(store,NULL);
            return ret; 
        }

        virtual int_type
        overflow (int_type c)
        {
            int store;
            int ret;
            pthread_setcancelstate(PTHREAD_CANCEL_DISABLE,&store);
            ret = _M_out->sputc(c);
            pthread_setcancelstate(store,NULL);
            return ret;
        }

        int sync()
        {
            int store;
            int ret;
            pthread_setcancelstate(PTHREAD_CANCEL_DISABLE,&store);
            ret = _M_out->pubsync();
            pthread_setcancelstate(store,NULL);
            return ret;
        }

    private:        

        std::streambuf *_M_out;
    };

    ///////////////////////////////////////////////////////////////////////////////////
    // This class makes atomic the access to the stream, by means of a ticket spinlock 
    // stored in the internal extensible array of the stream itself.
    // Constructor-on-the-first-use idiom ensures that the iword index is
    // unique across different compilation units.

    struct tspinlock_stream 
    {
        struct lock   {};
        struct unlock {};

        static int iword_index()
        {
            static int index = std::ios_base::xalloc();
            return index;
        }

        static inline std::ostream &_lock(std::ostream &out)
        {
            volatile int & sp = reinterpret_cast<int &>(out.iword( iword_index()));
            tspinlock::lock(sp);
            return out;
        }

        static inline std::ostream &_unlock(std::ostream &out)
        {
            volatile int & sp = reinterpret_cast<int &>(out.iword( iword_index())); 
            tspinlock::unlock(sp);
            return out;
        }
    };

    static inline
    std::ostream &operator<<(std::ostream &out, more::tspinlock_stream::lock)
    {
        more::tspinlock_stream::_lock(out);
        return out;
    }    
    
    static inline
    std::ostream &operator<<(std::ostream &out, more::tspinlock_stream::unlock)
    {
        more::tspinlock_stream::_unlock(out);
        return out;
    }

} // namespace more

#endif /* _TSTREAM_HH_ */
