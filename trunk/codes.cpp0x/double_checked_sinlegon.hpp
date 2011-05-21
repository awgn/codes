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

    // this is a simple implementation of the famous double-checked singleton.
    // it should be race free. Nicola

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
                if (!m_instance.load(std::memory_order_relaxed))
                {
                    std::lock_guard<std::mutex> lock(m_mutex);
                    if (!m_instance.load(std::memory_order_acquire))
                    {
                        Tp * i = new Tp(std::forward<Ti>(arg)...);
                        m_instance.store(i, std::memory_order_release);    
                    }    
                }

                return * m_instance.load(std::memory_order_relaxed);
            }

        private:
            singleton()
            {}

            ~singleton()
            {}

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
