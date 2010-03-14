/* $Id$ */
/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <bonelli@antifork.org> wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return. Nicola Bonelli
 * ----------------------------------------------------------------------------
 */

#ifndef _TQUEUE_HH_
#define _TQUEUE_HH_ 

#include <pthread++.hh>     // more!
#include <queue>
#include <deque>

namespace more { 

    template<typename Tp, typename Seq = std::deque<Tp> >
    class tqueue
    {

    public:
        typedef typename std::queue<Tp, Seq>::value_type      value_type;
        typedef typename std::queue<Tp, Seq>::reference       reference;
        typedef typename std::queue<Tp, Seq>::const_reference const_reference;
        typedef typename std::queue<Tp, Seq>::size_type       size_type;
        typedef typename std::queue<Tp, Seq>                  container_type; 

    private:
        std::queue<Tp, Seq>     _M_queue;
        mutable posix::mutex    _M_mutex;
        posix::cond             _M_cond;

    public:

        tqueue()
        : _M_queue(),
          _M_mutex(PTHREAD_MUTEX_RECURSIVE),
          _M_cond()
        {}

        ~tqueue()
        {}

        bool 
        empty() const
        {
            posix::scoped_lock<posix::mutex> lock(_M_mutex);
            return _M_queue.empty();
        }

        size_type 
        size() const
        { 
            posix::scoped_lock<posix::mutex> lock(_M_mutex);
            return _M_queue.size();
        }

        // simple push and pop...
        //

        void 
        push(const Tp & data)
        {
            {
                posix::scoped_lock<posix::mutex> lock(_M_mutex);
                _M_queue.push(data);
            } // lock.unlock()

            _M_cond.signal();
        }

        void 
        pop(Tp& value)
        {
            posix::scoped_lock<posix::mutex> lock(_M_mutex);
            while(_M_queue.empty())
            {
                _M_cond.wait(lock);
            }

            value=_M_queue.front();
            _M_queue.pop();
        }

        bool 
        try_pop(Tp& value)
        {
            posix::scoped_lock<posix::mutex> lock(_M_mutex);
            if(_M_queue.empty())
                return false;

            value=_M_queue.front();
            _M_queue.pop();

            return true;
        }

        // advanced push and pop...
        //

        template <typename Fn>
        bool push_enabled_if(const Fn predicate, const Tp & value)
        {
            {
                posix::scoped_lock<posix::mutex> lock(_M_mutex);
                if (!predicate()) 
                    return false;
                _M_queue.push(value);
            } // lock.unlock()

            _M_cond.signal();
            return true;
        }

        template <typename Fn>
        void pop_and_exec(const Fn action, Tp & value)
        {
            posix::scoped_lock<posix::mutex> lock(_M_mutex);
            while(_M_queue.empty())
            {
                _M_cond.wait(lock);
            }

            value=_M_queue.front();
            _M_queue.pop();

            // invoke the callable object:
            action(); 
        }

        template <typename Fn>
        bool try_pop_and_exec(const Fn action, Tp & value)
        {
            posix::scoped_lock<posix::mutex> lock(_M_mutex);
            if(_M_queue.empty())
                return false;

            value=_M_queue.front();
            _M_queue.pop();

            // invoke the callable object:
            action();

            return true;
        }

    };

} // namespace more
#endif /* _TQUEUE_HH_ */
