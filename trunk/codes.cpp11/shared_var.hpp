/* $Id$ */
/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <bonelli@antifork.org> wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return. Nicola Bonelli
 * ----------------------------------------------------------------------------
 */

#pragma once 

#include <atomic>
#include <memory>
#include <mutex>

namespace more {

    template <typename Tp>
    struct shared_var
    {
        shared_var()
        : current_(new Tp())
        {
        }

        shared_var(Tp value)
        : current_(new Tp(std::move(value)))
        {
        }

        ~shared_var()
        {
            delete current_.load(std::memory_order_relaxed);
        }

        shared_var(const shared_var &) = delete;
        shared_var& operator=(const shared_var &) = delete;

        shared_var(shared_var &&) = delete;
        shared_var& operator=(shared_var &&) = delete;


        /* multiple threads can access to the read-only shared object. 
         * promise: this reference is used for a short period ( < of that of the update frequency ) */

        Tp const &
        get() const 
        {
            return * current_.load(std::memory_order_acquire);
        }


        /* update the value of var: this method is supposed to be 
         * called with a reasonably low frequency. Note: the mutex is required
         * only in case of multiple calling threads. */

        template <typename ... Ts>
        void put(Ts && ... value)
        {
            auto nptr = new Tp(std::move(std::forward<Ts>(value)...));
            
            auto cur  = current_.exchange(nptr);

            std::lock_guard<std::mutex> lock(mutex_);
            
            garbage_.reset(cur);
        }
        

    private:
        std::atomic<Tp *> current_;
        std::unique_ptr<Tp> garbage_;
        std::mutex mutex_;
    };


} // namespace more

