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
#include <fstream>
#include <exception>
#include <thread>
#include <atomic>
#include <mutex>
#include <condition_variable>
#include <ctime>
#include <cerrno>
#include <system_error>

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

        logger(std::streambuf *sb, bool timestamp = true)
        : log_(sb)
        , mutex_()
        , timestamp_(timestamp)
        , ticket_(0)
        , done_()
        {
        }

    
        ~logger()
        {
        }


        void rdbuf(std::streambuf *sb)
        {
            log_.rdbuf(sb);
        }


        std::streambuf *
        rdbuf() const
        {
            return log_.rdbuf();
        }

        
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

        
        void rotate(const std::string &filename, int maxsize, int level = 3)
        {
            std::thread([=]() {

                std::lock_guard<std::mutex> lock(mutex_);

                std::filebuf * fout = dynamic_cast<std::filebuf *>(log_.rdbuf());
                if (fout == nullptr) 
                    return; 
                
                // get the size of the out streambuf...
                
                auto size = log_.rdbuf()->pubseekoff(0, std::ios_base::cur);
                if (size > maxsize) {  

                    fout->close();

                    rotate_(filename, level);

                    if (!fout->open(filename, std::ios::out))
                        throw std::runtime_error( "logger: rotate " + filename);
                }

            }).detach();
        }


    private:

        static void rotate_(const std::string &name, int level)
        {
            auto ext = [](int n) -> std::string 
            { 
                return n > 0 ? ("." + std::to_string(n)) : ""; 
            };

            for(int i = level-1; i >= 0; i--)
            {
                if(std::rename((name + ext(i)).c_str(), (name + ext(i+1)).c_str()) != 0)
                    if (errno != ENOENT)
                    {
                        throw std::system_error(errno, std::generic_category(), "std::rename");        
                    }
            }
        }

        template <typename Fun>
        void sync_(unsigned long t, Fun const &fun)
        {
            std::unique_lock<std::mutex> lock(mutex_);
            cond_.wait(lock, [&]() -> bool { return t == done_; });
            if (timestamp_)
                log_ << make_timestamp();
            fun(log_);
            done_++;
            cond_.notify_all();
        }

        std::string
        make_timestamp()
        {
            auto now_c = std::chrono::system_clock::to_time_t(
                            std::chrono::system_clock::now()
                         );
            struct tm tm_c;
            return put_time(localtime_r(&now_c, &tm_c), "[ %F %T ] ");                    
        }

        std::ostream log_;
        
        std::mutex mutex_;
        std::condition_variable cond_;

        bool timestamp_;
        
        std::atomic_ulong ticket_;
        unsigned long done_;
    };

};


#endif /* _LOGGER_HPP_ */
