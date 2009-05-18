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

#include <iostream>
#include <atomicity-policy.hh>
#include <tspinlock.hh>
#include <pthread.h>

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

    template <typename M = more::tspinlock_half_recursive, typename C = nullCancel >
    class tstreambuf : public std::streambuf
    {
    public:

        tstreambuf(std::streambuf *out)
        : _M_out(out), _M_device(), _M_cancel_state()
        {}

    protected:

        virtual std::streamsize
        xsputn (const char *s, std::streamsize n)
        {
            _M_device.lock();
            C::cancel_disable(_M_cancel_state);
            return  _M_out->sputn(s,n);
        }

        virtual int_type
        overflow (int_type c)
        {
            _M_device.lock();
            C::cancel_disable(_M_cancel_state);
            if (c != EOF)
                return _M_out->sputc(c);
            return c;
        }

        int sync()
        { 
            int ret = _M_out->pubsync();
            C::cancel_restore(_M_cancel_state);
            _M_device.unlock();
            return ret == 0 ? 0 : -1; 
        }

    private:        

        std::streambuf *_M_out;
        M _M_device;
        int _M_cancel_state;
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

        static tspinlock_half_recursive * spin_alloc(std::ostream &out)
        {
            int index = std::ios_base::xalloc();
            tspinlock_half_recursive * &ret = reinterpret_cast<tspinlock_half_recursive * &>(out.iword(index));
            ret = new tspinlock_half_recursive;
            return ret;
        }

        static tspinlock_half_recursive * get_lock(std::ostream &out)
        {
            static tspinlock_half_recursive * ret = spin_alloc(out);
            return ret;
        }

        static inline std::ostream &_lock(std::ostream &out)
        {
            tspinlock_half_recursive * sl = get_lock(out);
            sl->lock();
            return out;
        }

        static inline std::ostream &_unlock(std::ostream &out)
        {
            tspinlock_half_recursive * sl = get_lock(out);
            sl->unlock();
            return out;
        }

        public:

        friend 
        std::ostream &operator<<(std::ostream &out, more::lock_stream::lock)
        {
            more::lock_stream::_lock(out);
            return out;
        }    

        friend 
        std::ostream &operator<<(std::ostream &out, more::lock_stream::unlock)
        {
            more::lock_stream::_unlock(out);
            return out;
        }
    };

} // namespace more

#endif /* _TSTREAMBUF_HH_ */
