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
         * promise: this reference is used for a short period ( < update period * GC ) */

        Tp const &
        get() const
        {
            if (auto r = current_.load(std::memory_order_acquire))
                return *r;
            throw std::runtime_error("shared_var::get: not engaged");
        }

        /* consume: a thread can get the object ptr and disengage its value atomically */

        Tp const *
        consume()
        {
            return current_.exchange(nullptr);
        }

        explicit operator bool() const
        {
            return current_.load(std::memory_order_relaxed);
        }

        /* update the value of var: this method is supposed to be
         * called with a reasonably low frequency: the garbage must stay alive
         * for a grace period time */

        void put(Tp const * nptr)
        {
            auto cur  = current_.exchange(nptr, std::memory_order_release);
            this->collect(cur);
        }

        template <typename ... Ts>
        void emplace(Ts && ... vs)
        {
            auto nptr = reinterpret_cast<Tp const *>(new Tp(std::forward<Ts>(vs)...));
            put(nptr);
        }


        /* perform a compare and exchange operation: required when multiple threads attempt to update the shared var.
         * As for put, the garbage must stay alive for a grace period time */

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
                return std::make_pair(false, cur);
            }
        }

        template <typename ... Ts>
        std::pair<bool, Tp const *>
        safe_emplace(Tp const *cur, Ts && ...vs)
        {
            auto nptr = reinterpret_cast<Tp const *>(new Tp(std::forward<Ts>(vs)...));
            return safe_put(cur, nptr);
        }

    private:

        void collect(Tp const *ptr)
        {
            std::lock_guard<std::mutex> lock(mutex_);
            garbage_.emplace_back(ptr);
            if (garbage_.size() > GC)
                garbage_.erase(std::begin(garbage_));
        }

        std::atomic<Tp const *> current_;
        std::vector<std::unique_ptr<Tp const>> garbage_;
        std::mutex mutex_;
    };


} // namespace more

