/* $Id$ */
/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <bonelli@antifork.org> wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return. Nicola Bonelli
 * ----------------------------------------------------------------------------
 */
 
#ifndef _LOGGER_HPP_
#define _LOGGER_HPP_

#include <iostream>
#include <exception>
#include <thread>
#include <atomic>
#include <mutex>
#include <condition_variable>
#include <ctime>

namespace more
{
    // std::put_time is missing in g++ up to 4.7.x
    namespace 
    { 
        std::string 
        put_time(const struct tm *tmb, const char *fmt)
        {
            char buf [64];
            if (!std::strftime(buf, 63, fmt, tmb))
                throw std::runtime_error("put_time: strftime");
            return buf;
        }
    }

    class logger
    {
    
    public:
        logger(std::ostream &log, bool timestamp = true)
        : log_(log)
        , mutex_()
        , timestamp_(timestamp)
        , ticket_(0)
        , done_()
        {}

        ~logger()
        {}

        template <typename Fun>
        void async(Fun const &fun)
        {
            auto t = ticket_++;
            
            std::thread([this, fun, t]() 
            { 
                sync_(t, fun); 
            
            }).detach();
        }

        template <typename Fun>
        void sync(Fun const &fun)
        {
            auto t = ticket_++;
            sync_(t, fun);
        }

    private:

        template <typename Fun>
        void sync_(unsigned long t, Fun const &fun)
        {
            std::unique_lock<std::mutex> lock(mutex_);
            cond_.wait(lock, [&]() -> bool { return t == done_; });
            if (timestamp_)
                put_timestamp_();
            fun(log_);
            done_++;
            cond_.notify_all();
        }

        void put_timestamp_()
        {
            auto now_c = std::chrono::system_clock::to_time_t(
                            std::chrono::system_clock::now()
                         );
            struct tm tm_c;
            log_ << put_time(localtime_r(&now_c, &tm_c), "[ %F %T ] ");                    
        }

        std::ostream& log_;
        std::mutex mutex_;
        std::condition_variable cond_;

        bool timestamp_;
        
        std::atomic_ulong ticket_;
        unsigned long done_;
    };

};


#endif /* _LOGGER_HPP_ */
