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

#include <thread>
#include <mutex>
#include <map>

namespace more { 

    struct 
    that_thread
    {
        typedef std::map<std::thread::id, bool> map_type;
        typedef std::pair<map_type, std::mutex> mt_map_type;

        static
        void interrupt(std::thread::id h)
        {
            mt_map_type & __map = that_thread::get_int_map();
            std::lock_guard<std::mutex> lock(__map.second);
            __map.first.insert(std::make_pair(h,true));
        }
 
        static bool
        interruption_requested(std::thread::id h = std::this_thread::get_id())
        {
            mt_map_type & __map = that_thread::get_int_map();
            std::lock_guard<std::mutex> lock(__map.second);
            map_type::iterator it = __map.first.find(h);
            if ( it == __map.first.end() )
                return false;
            return it->second;
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


} // namespace more

#endif /* _THREAD_INTERRUPT_HPP_ */
