/* $Id$ */
/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <bonelli@antifork.org> wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return. Nicola Bonelli
 * ----------------------------------------------------------------------------
 */

#ifndef _DOUBLE_CHECKED_SINGLETON_HPP_
#define _DOUBLE_CHECKED_SINGLETON_HPP_ 

#include <atomic>
#include <mutex>

namespace more { 

    // This is a simple implementation of the famous double-checked lock singleton.
    // Race fixed by John Calsbeek, see http://stackoverflow.com/questions/6086912/double-checked-lock-singleton-in-c11
    // 
    // It should be race free. Nicola

    namespace double_checked
    {
        
        template <typename Tp>
        class singleton
        {
        public:

            template <typename ...Ti>
            static Tp &
            instance(Ti && ...arg)
            {
                // thread_local improves the performance of about a factor 10 
                // (load with acquire semantic is indeed called just one time per thread). 
                // Though not yet supported, g++ has its non-standard keyword __thread. Nicola

                static __thread Tp *instance;

                if (!instance && 
                    !(instance = m_instance.load(std::memory_order_acquire)))
                {
                    std::lock_guard<std::mutex> lock(m_mutex);
                    if (!m_instance.load(std::memory_order_relaxed))
                    {
                        instance = new Tp(std::forward<Ti>(arg)...);
                        m_instance.store(instance, std::memory_order_release);    
                    }    
                }

                return * instance;
            }

        private:
            singleton()
            {}

            ~singleton()
            {}

            singleton(const singleton&) = delete;
            singleton& operator=(const singleton&) = delete;

            static std::atomic<Tp *> m_instance;
            static std::mutex m_mutex;            
        };    

        template <typename Tp>
        std::atomic<Tp *> singleton<Tp>::m_instance;

        template <typename Tp>
        std::mutex singleton<Tp>::m_mutex;

    } // namespace double_checked

} // namespace more

#endif /* _DOUBLE_CHECKED_SINGLETON_HPP_ */
