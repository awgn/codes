/* $Id$ */
/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <bonelli@antifork.org> wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return. Nicola Bonelli
 * ----------------------------------------------------------------------------
 */

#ifndef _THREAD_INTERRUPT_HPP_
#define _THREAD_INTERRUPT_HPP_

#include <memory>
#include <thread>
#include <mutex>
#include <map>

namespace more {

    struct
    thread_interrupt
    {
        typedef std::shared_ptr<volatile bool> request_type;

        typedef std::map<std::thread::id, request_type > map_type;

        struct hook
        {
            hook()
            : m_req(interrupt_request())
            {
                interrupt_request_store(std::this_thread::get_id(), m_req);
            }

            hook(request_type hook)
            : m_req(hook)
            {
                interrupt_request_store(std::this_thread::get_id(), m_req);
            }

            ~hook()
            {}

            bool
            operator()() const
            {
                return *m_req;
            }

            request_type m_req;
        };

        static
        void interrupt(std::thread::id h)
        {
            auto & map_ = thread_interrupt::get_map();
            std::lock_guard<std::mutex> lock(map_.second);

            auto it = map_.first.find(h);
            if (it == map_.first.end())
                throw std::runtime_error("interrupt_request not found");

            *(it->second) = true;
            map_.first.erase(it);
        }

   private:

        template <typename ...Types>
        friend std::thread
        make_interruptible_thread(Types&&... args);

        static request_type
        interrupt_request()
        {
            return request_type(new bool(false));
        }

        static
        void interrupt_request_store(std::thread::id h, request_type p)
        {
            auto & map_ = thread_interrupt::get_map();
            std::lock_guard<std::mutex> lock(map_.second);
            map_.first.insert(std::make_pair(h,p));
        }

        static
        std::pair<map_type, std::mutex> &
        get_map()
        {
            static std::pair<map_type, std::mutex> s_map;
            return s_map;
        }
    };

    /////////////////////////////////////
    // factory for interruptible threads

    template <typename ...Types>
    inline std::thread
    make_interruptible_thread(Types&&... args)
    {
        // create an interrupt request
        //
        thread_interrupt::request_type req = thread_interrupt::interrupt_request();

        // the thread is passed the interrupt request as last argument.
        //
        std::thread th(std::forward<Types>(args)..., req);

        // store the request for a later interruption...
        //
        thread_interrupt::interrupt_request_store(th.get_id(), req);

        return th;
    }

} // namespace more

#endif /* _THREAD_INTERRUPT_HPP_ */
