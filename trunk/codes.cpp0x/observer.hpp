/* $Id$ */
/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <bonelli@antifork.org> wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return. Nicola Bonelli
 * ----------------------------------------------------------------------------
 */

#ifndef _OBSERVER_HPP_
#define _OBSERVER_HPP_ 

#include <algorithm>
#include <vector>
#include <cassert>
#include <stdexcept>
#include <functional>
#include <type_traits>
#include <memory>
#include <mutex>

namespace more { 

    ////////////////////////// observer
    
    template <typename ...Ti>
    class observer
    { 
    public:
        observer()
        {}

        virtual ~observer()
        {}

        template <typename ...Ts>
        void updatex(Ts&& ...args)
        {
            this->update(std::forward<Ts>(args)...);
        }

    protected:
        virtual void update(Ti ...args) = 0; 
    };

    ////////////////////////// subject

    template <typename ...Ti>
    class subject 
    { 
        struct null_deleter
        {
            void operator()(void *) {}
        };

    public:
        subject()
        : m_observers()
        {}

        ~subject()
        {}

        void notify(Ti&& ...args)
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            for( auto it = m_observers.begin(), it_e = m_observers.end(); it != it_e; ++it)
            {
                (*it)->updatex(std::forward<Ti>(args)...);
            }
        }

        //////////////////////////////////////////////////////

        void attach(const std::shared_ptr<observer<Ti...>> &sp)
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            if (std::find(m_observers.begin(), m_observers.end(), sp) == m_observers.end())
                m_observers.push_back(sp);
        }
        void attach(observer<Ti...> *rp)
        {
            attach(std::shared_ptr<observer<Ti...>>(rp, null_deleter()));
        }

        void detach(const std::shared_ptr<observer<Ti...>> &sp)
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            auto it = std::find(m_observers.begin(), m_observers.end(), sp);
            if (it != m_observers.end())
                m_observers.erase(it);
        }
        void detach(observer<Ti...> *rp)
        {
            detach(std::shared_ptr<observer<Ti...>>(rp, null_deleter()));
        }

    private:  
        std::vector<std::shared_ptr<observer<Ti...>>>  m_observers;
        std::mutex m_mutex;
    };

} // namespace more

#endif /* _OBSERVER_HH_ */
