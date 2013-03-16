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

#include <tuple_ext.hpp>   // !more

namespace more
{
    /////////////////////////   std::put_time is missing in g++ up to 4.7.x
   
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
   
    /////////////////////////   more::logger

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
                log_ << make_timestamp_();
            fun(log_);
            done_++;
            cond_.notify_all();
        }

        std::string
        make_timestamp_()
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


    /////////////////////////   more::lazy_stream


    template <typename ...Ts>
    struct lazy_stream
    {
        struct stream_on
        {
            stream_on(std::ostream &out)
            : out_(out)
            {}
            
            template <typename T>
            void operator()(const T &ref)
            {
                out_ << ref;
            }

            std::ostream &out_;
        };

        lazy_stream(logger &l)
        : refs_()
        , run_(true)
        , log_(l)
        {}

        template <typename ... Tx, typename T>
        lazy_stream(lazy_stream<Tx...> const &l, const T &data)
        : refs_(std::tuple_cat(l.refs_, std::tie(data)))
        , run_(true)
        , log_(l.log_)
        {
            l.run_ = false;
        }

 
        ~lazy_stream()
        {
            if (run_)
            {
                log_.sync([this](std::ostream &o)
                {
                    tuple_for_each(refs_,stream_on(o));                           
                });
            }
        }

        std::tuple<Ts...> refs_;
        mutable bool run_;
        logger &log_;
    };

    // manipulator are function template (unresolved function types) which cannot be
    // deduced by the template machinery
    //
    
    typedef std::ostream& (manip_t)(std::ostream&);

    // lazy_stream<Ts...> << data
    //

    template <typename ...Ts>
    inline lazy_stream<Ts..., const manip_t &>
    operator<<(lazy_stream<Ts...> const &l, manip_t const &m)
    {
        return lazy_stream<Ts..., const manip_t &>(l, m);
    }

    template <typename ...Ts, typename T>
    inline lazy_stream<Ts..., const T &>
    operator<<(lazy_stream<Ts...> const &l, const T &data)
    {
        return lazy_stream<Ts..., const T &>(l, data);
    }

    // more::logger << data
    //

    inline lazy_stream<const manip_t &>
    operator<<(logger &l, manip_t const &m)
    {
        return lazy_stream<>(l) << m;
    }

    template <typename T>
    inline lazy_stream<const T &>
    operator<<(logger &l, const T &data)
    {
        return lazy_stream<>(l) << data;
    }
    
};


#endif /* _LOGGER_HPP_ */
