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
    that_thread
    {
        typedef std::shared_ptr<volatile bool> interrupt_request_type;
        typedef std::map<std::thread::id, interrupt_request_type > map_type;
        typedef std::pair<map_type, std::mutex> mt_map_type;

        static interrupt_request_type
        interrupt_request()
        {
            return interrupt_request_type(new bool(false));
        }

        static
        void interrupt_request_store(std::thread::id h, interrupt_request_type p)
        {
            mt_map_type & __map = that_thread::get_int_map();
            std::lock_guard<std::mutex> lock(__map.second);
            __map.first.insert(std::make_pair(h,p));
        }

        static
        void interrupt(std::thread::id h)
        {
            mt_map_type & __map = that_thread::get_int_map();
            std::lock_guard<std::mutex> lock(__map.second);               
            
            map_type::iterator it = __map.first.find(h);
            if ( it == __map.first.end() )
                std::runtime_error("interrupt_request not found");
            *(it->second) = true;
            __map.first.erase(it);
        }
 
    private:
        static
        mt_map_type &
        get_int_map()
        {
            static mt_map_type one;
            return one;
        }
    };

    /////////////////////////////////////
    // factory for interruptible threads

    template <typename ...Types>
    std::thread
    make_interruptible_thread(Types... args)
    {    
        // create an interrupt request
        //

        that_thread::interrupt_request_type req = that_thread::interrupt_request();

        // run the thread, passing the interrupt request as last argument.
        //

        std::thread t(args..., req);

        // store the request for a later interruption...
        //
        that_thread::interrupt_request_store(t.get_id(), req);

        return std::move(t); 
    }

} // namespace more

#endif /* _THREAD_INTERRUPT_HPP_ */
