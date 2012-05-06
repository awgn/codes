/* $Id$ */
/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <bonelli@antifork.org> wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return. Nicola Bonelli
 * ----------------------------------------------------------------------------
 */

#ifndef _TSTREAMBUF_HH_
#define _TSTREAMBUF_HH_ 

#include <pthread.h>

#include <atomicity-policy.hh>  // more!
#include <spinlock.hh>          // more!

#include <iostream>
#include <cstdio>

namespace more { 

    /////////////////////////////////////////////////////////////////////////////////////////////////
    // tstreambuf is policy-based spinklocked and cancel-safe streambuf (againts pthread_cancel() 
    // by means of pthread_setcancelstate()). 
    // see http://www.codesourcery.com/archives/c++-pthreads/threads.html for more information 

    struct nullCancel 
    {
        static void cancel_disable(int &store)
        {}
        static void cancel_restore(int &store)
        {}
    };

    struct cancelSafe 
    {
        static void cancel_disable(int &store)
        {
            pthread_setcancelstate(PTHREAD_CANCEL_DISABLE,&store);
        }
        static void cancel_restore(int &store)
        {
            pthread_setcancelstate(store,NULL);
        }
    };

    template <typename M = more::spinlock_open_recursive, typename C = nullCancel >
    class tstreambuf : public std::streambuf
    {
    public:

        tstreambuf(std::streambuf *out)
        : m_out(out), m_device() 
        {}

    protected:

        virtual std::streamsize
        xsputn (const char *s, std::streamsize n)
        {
            m_device.lock();
            int store;
            C::cancel_disable(store);
            int r = m_out->sputn(s,n);
            C::cancel_restore(store);
            return r;
        }

        virtual int_type
        overflow (int_type c)
        {
            m_device.lock();
            if (c != EOF) {
                int store; 
                C::cancel_disable(store);
                int r = m_out->sputc(c);
                C::cancel_restore(store);
                return r;
            }
            return c;
        }

        int sync()
        { 
            m_device.lock();
            int store;
            C::cancel_disable(store);
            int r = m_out->pubsync();
            C::cancel_restore(store);
            m_device.unlock();
            return r == 0 ? 0 : -1; 
        }

    private:        

        std::streambuf *m_out;
        M m_device;
    };

    ///////////////////////////////////////////////////////////////////////////////////
    // This class makes atomic the access to the stream, by means of a ticket spinlock 
    // stored in the internal extensible array of the stream itself.
    // Constructor-on-the-first-use idiom ensures that the iword index (the pointer to the spinlock) 
    // is unique across different compilation units.

    struct lock_stream 
    {
        struct lock   {};
        struct unlock {};

        protected:

        static spinlock_open_recursive * spin_alloc(std::ostream &out)
        {
            int index = std::ios_base::xalloc();
            spinlock_open_recursive * &ret = reinterpret_cast<spinlock_open_recursive * &>(out.iword(index));
            ret = new spinlock_open_recursive;
            return ret;
        }

        static spinlock_open_recursive * get_lock(std::ostream &out)
        {
            static spinlock_open_recursive * ret = spin_alloc(out);
            return ret;
        }

        static inline std::ostream &_lock(std::ostream &out)
        {
            spinlock_open_recursive * sl = get_lock(out);
            sl->lock();
            return out;
        }

        static inline std::ostream &_unlock(std::ostream &out)
        {
            spinlock_open_recursive * sl = get_lock(out);
            sl->unlock();
            return out;
        }

        public:

        template <typename CharT, typename Traits>
        friend inline std::basic_ostream<CharT,Traits> &
        operator<<(std::basic_ostream<CharT, Traits> &out, more::lock_stream::lock)
        {
            more::lock_stream::_lock(out);
            return out;
        }    

        template <typename CharT, typename Traits>
        friend inline std::basic_ostream<CharT,Traits> &
        operator<<(std::basic_ostream<CharT, Traits> &out, more::lock_stream::unlock)
        {
            more::lock_stream::_unlock(out);
            return out;
        }
    };

} // namespace more

#endif /* _TSTREAMBUF_HH_ */
