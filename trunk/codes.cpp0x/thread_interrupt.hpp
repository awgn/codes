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

        struct interrupt_hook
        {
            interrupt_hook()
            : _M_req(interrupt_request())
            {   
                interrupt_request_store(std::this_thread::get_id(), _M_req);
            }

            ~interrupt_hook()
            {}
            
            operator bool()
            {
                return *_M_req;
            }
            
            interrupt_request_type _M_req;
        };

        static
        void interrupt(std::thread::id h)
        {
            mt_map_type & __map = that_thread::get_int_map();
            std::lock_guard<std::mutex> lock(__map.second);               

            map_type::iterator it = __map.first.find(h);
            if ( it == __map.first.end() )
                throw std::runtime_error("interrupt_request not found");
            
            *(it->second) = true;
            __map.first.erase(it);
        }
 
    private:          
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
        mt_map_type &
        get_int_map()
        {
            static mt_map_type one;
            return one;
        }
    };


} // namespace more

#endif /* _THREAD_INTERRUPT_HPP_ */
