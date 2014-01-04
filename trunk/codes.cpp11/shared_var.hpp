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
#include <vector>

namespace more {

    template <typename Tp, size_t GC = 16>
    struct shared_var
    {
        shared_var()
        : current_(nullptr)
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

        Tp const *
        get() const 
        {
            return current_.load(std::memory_order_acquire);
        }


        /* update the value of var: this method is supposed to be 
         * called with a reasonably low frequency: the garbage must stay alive
         * for a grace period time */ 

        template <typename ... Ts>
        void put(Ts && ... value)
        {
            auto nptr = reinterpret_cast<Tp const *>(new Tp(std::move(std::forward<Ts>(value)...)));
            auto cur  = current_.exchange(nptr, std::memory_order_release);
            this->collect(cur);
        }                               
        
        void put(Tp const * nptr)
        {
            auto cur  = current_.exchange(nptr, std::memory_order_release);
            this->collect(cur);
        }                               

        /* perform a compare and exchange operation: required when multiple threads attempt to update the shared var.
         * As for put, the garbage must stay alive for a grace period time */

        template <typename ... Ts>
        std::pair<bool, Tp const *>
        safe_put(Tp const *cur, Ts && ...value)
        {
            auto nptr = reinterpret_cast<Tp const *>(new Tp(std::move(std::forward<Ts>(value)...)));

            auto ret = current_.compare_exchange_strong(cur, nptr, std::memory_order_release,
                                                                   std::memory_order_relaxed);

            if (ret)
            {
                this->collect(cur);
                return std::make_pair(true, nptr);
            }
            else
            {
                delete nptr;
                return std::make_pair(false, cur);
            }
        }


        std::pair<bool, Tp const *>
        safe_put(Tp const *cur, Tp const *nptr)
        {
            auto ret = current_.compare_exchange_strong(cur, nptr, std::memory_order_release,
                                                                   std::memory_order_relaxed);

            if (ret)
            {
                this->collect(cur);
                return std::make_pair(true, nptr);
            }
            else
            {
                delete nptr;
                return std::make_pair(false, cur);
            }
        }

    private:
        
        void collect(Tp const *ptr)
        {
            std::lock_guard<std::mutex> lock(mutex_);
            garbage_.emplace_back(ptr);
            if (garbage_.size() > GC)
            {
                garbage_.erase(std::begin(garbage_));
            }
        }

        std::atomic<Tp const *> current_;
        std::vector<std::unique_ptr<Tp const>> garbage_;
        std::mutex mutex_;
    };


} // namespace more

