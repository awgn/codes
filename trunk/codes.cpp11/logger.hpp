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
#include <sstream>

#include <exception>
#include <thread>
#include <atomic>
#include <mutex>
#include <memory>
#include <condition_variable>
#include <ctime>
#include <cerrno>
#include <cassert>
#include <system_error>

#include <tuple_ext.hpp>   // !more

namespace more
{
    namespace details 
    { 
        //////////// std::put_time is missing in g++ up to 4.7.x
        
        std::string 
        put_time(const struct tm *tmb, const char *fmt)
        {
            char buf [64];
            if (!std::strftime(buf, 63, fmt, tmb))
                throw std::runtime_error("put_time: strftime");
            return buf;
        }
        
        //////////// rotate_file function
        
        void rotate_file(const std::string &name, int depth)
        {
            auto ext = [](int n) -> std::string 
            { 
                return n > 0 ? ('.' + std::to_string(n)) : ""; 
            };

            for(int i = depth-1; i >= 0; i--)
            {
                if(std::rename((name + ext(i)).c_str(), (name + ext(i+1)).c_str()) != 0)
                    if (errno != ENOENT)
                    {
                        throw std::system_error(errno, std::generic_category(), "std::rename");        
                    }
            }
        }
    }

    /////////////////////////   more::logger


    class logger
    {
    public:

        logger(bool timestamp = true)
        : fname_() 
        , fbuf_()
        , out_(new std::ostream(std::cout.rdbuf()))
        , mutex_()
        , timestamp_(timestamp)
        , ticket_(0)
        , done_()
        {
        }

        explicit
        logger(std::streambuf *sb, bool timestamp = true)
        : fname_()
        , fbuf_()
        , out_(new std::ostream(sb))
        , mutex_()
        , timestamp_(timestamp)
        , ticket_(0)
        , done_()
        {
        }
        
        explicit
        logger(const char *filename, bool timestamp = true)
        : fname_(filename)
        , fbuf_(new std::filebuf())
        , out_()
        , mutex_()
        , timestamp_(timestamp)
        , ticket_(0)
        , done_()
        {
            if(!fbuf_->open(filename, std::ios_base::out|std::ios_base::trunc))
                throw std::system_error(errno, std::generic_category(), "filebuf: open");        
                
            out_->rdbuf(fbuf_.get());
        }
        
        ~logger() = default;
        
        logger(logger &&) = default;
        logger& operator=(logger &&) = default;


        std::string const &
        name() const
        {
            return fname_;
        }
        
        void
        open(std::string filename, std::ios_base::openmode mode = std::ios_base::out|std::ios_base::trunc)  
        {
            std::lock_guard<std::mutex> lock(mutex_);

            fbuf_->close();

            if (!fbuf_->open(filename, mode))
                throw std::system_error(errno, std::generic_category(), "filebuf: open");        

            fname_ = std::move(filename);
            out_->rdbuf(fbuf_.get());
        }
        
        void
        close()
        {
            std::lock_guard<std::mutex> lock(mutex_);
            fbuf_->close();
            fname_.clear();
            out_->rdbuf(nullptr);
        }

        void
        rdbuf(std::streambuf *sb) 
        {
            std::lock_guard<std::mutex> lock(mutex_);
            fbuf_->close();
            fname_.clear();
            out_->rdbuf(sb);
        }

        std::streambuf *
        rdbuf() const
        {
            return out_->rdbuf();
        }

        void timestamp(bool value)
        {
            timestamp_ = value;
        }
        
        bool timestamp() const
        {
            return timestamp_;
        }

        //// log message: async

        template <typename Fun>
        void async(Fun const &fun)
        {
            try
            {
                auto t = this->ticket_++;
                std::thread([this, fun, t]() 
                {    
                    sync_(std::make_pair(true, t), fun); 
                
                }).detach();
            }
            catch(...)
            {
                sync([](std::ostream &out) 
                {
                    out << "exception: could not start log thread!" << std::endl;
                });
                
                sync(fun);
            }
        }

        //// log message: sync
        
        template <typename Fun>
        void sync(Fun const &fun)
        {
            sync_(std::make_pair(false, 0), fun);
        }

        //// return the size of the log file
        
        size_t
        size() const
        {
            std::lock_guard<std::mutex> lock(mutex_);
            return size_();
        }
        
        //// rotate the log file 

        void rotate(int depth = 3, size_t max_size = 0)
        {
            std::lock_guard<std::mutex> lock(mutex_);
            
            if (fname_.empty())
                return;
            
            if (size_() > max_size)
                rotate_(depth);
        }

        void rotate_async(int depth = 3, size_t max_size = 0)
        {
            std::lock_guard<std::mutex> lock(mutex_);
            
            if (fname_.empty())
                return;
            
            if (size_() > max_size)
            {
                std::thread([this, depth]() 
                {
                    std::lock_guard<std::mutex> lock(mutex_);
                    this->rotate_(depth);

                }).detach();
            }
        }

    private:
        
        size_t
        size_() const
        {
            auto fb = dynamic_cast<std::filebuf *>(out_->rdbuf());
            if (fb == nullptr) 
                return 0;

            return static_cast<size_t>(fb->pubseekoff(0, std::ios_base::cur));
        }

        void 
        rotate_(int depth = 3)
        {
            std::filebuf * fb = dynamic_cast<std::filebuf *>(out_->rdbuf());
            if (fb == nullptr) 
                return; 
            
            fb->close();

            bool rot = true;
            try
            {
                details::rotate_file(fname_, depth);
            }
            catch(...)
            {
                rot = false;
            }

            if (!fb->open(fname_, std::ios::out))
            {
                throw std::system_error(errno, std::generic_category(), "filebuf: open");      
            }

            if (!rot)
            {
                sync([=](std::ostream &out)
                { 
                    out << "error while rotating file!" << std::endl; 
                });
            }
        }

        template <typename Fun>
        void sync_(std::pair<bool, unsigned long> ticket, Fun const &fun)
        {
            std::unique_lock<std::mutex> lock(mutex_);

            if (ticket.first)
            {
                cond_.wait(lock, [&]() { return ticket.second == done_; });
            }

            try
            {
                if (timestamp_)
                    *out_ << make_timestamp_();

                fun(*out_);
            }
            catch(std::exception &e)
            {
                *out_ << "Exception: " << e.what() << std::endl;
            }

            if (ticket.first)
            {
                done_++;
                cond_.notify_all();
            }
        }

        static std::string
        make_timestamp_()
        {
            auto now_c = std::chrono::system_clock::to_time_t
                         (
                            std::chrono::system_clock::now()
                         );

            struct tm tm_c;
            return details::put_time(localtime_r(&now_c, &tm_c), "[ %F %T ] ");                    
        }   

        std::string                     fname_;
        std::unique_ptr<std::filebuf>   fbuf_;
        std::unique_ptr<std::ostream>   out_;

        mutable std::mutex mutex_;
        std::condition_variable cond_;

        bool timestamp_;
        
        std::atomic_ulong ticket_;
        unsigned long done_;
    };


    //// more::lazy_ostream a temporary stream that logs at its
    //// descrution point.

    struct log_async_t {} log_async = log_async_t {};

    template <typename ...Ts>
    struct lazy_ostream
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

        lazy_ostream(logger &l, bool as = false)
        : refs_ ()
        , run_  (true)
        , async_(as)
        , log_  (l)
        {}

        lazy_ostream(const lazy_ostream &other)
        : refs_ (other.refs_)
        , run_  (true)
        , async_(other.async_)
        , log_  (other.log_)
        {
            other.run_ = false;
        }

        template <typename ... Tx, typename T>
        lazy_ostream(lazy_ostream<Tx...> const &l, const T &data)
        : refs_ (std::tuple_cat(l.refs_, std::tie(data)))
        , run_  (true)
        , async_(l.async_)
        , log_  (l.log_)
        {
            l.run_ = false;
        }

        ~lazy_ostream()
        {
            if (run_)
            {
                if (async_)
                {
                    std::ostringstream out;
                    tuple_for_each(refs_,stream_on(out));                           
                    auto const & str = out.str();

                    log_.async([str](std::ostream &o) 
                               { 
                                  o << str; 
                               });
                }
                else
                {
                    log_.sync([this](std::ostream &o) 
                              { 
                                  tuple_for_each(refs_, stream_on(o));
                              });
                }
            }
        }

        std::tuple<Ts...> refs_;
        mutable bool run_;
        mutable bool async_;
        logger &log_;
    };

    // manipulator are function template (unresolved function types) which cannot be
    // deduced by the template machinery
    //
    
    typedef std::ostream& (manip_t)(std::ostream&);

    // lazy_ostream<Ts...> << data
    //

    template <typename ...Ts>
    inline lazy_ostream<Ts..., manip_t &>
    operator<<(lazy_ostream<Ts...> const &l, manip_t & m)
    {
        return lazy_ostream<Ts..., manip_t &>(l, m);
    }

    template <typename ...Ts>
    inline lazy_ostream<Ts...> 
    operator<<(lazy_ostream<Ts...> const &l, const log_async_t &)
    {
        return l.async_ = true, lazy_ostream<Ts...>(l);
    }

    template <typename ...Ts, typename T>
    inline lazy_ostream<Ts..., const T &>
    operator<<(lazy_ostream<Ts...> const &l, const T &data)
    {
        return lazy_ostream<Ts..., const T &>(l, data);
    }

    // more::logger << data
    //

    inline lazy_ostream<manip_t &>
    operator<<(logger &l, manip_t &m)
    {
        return lazy_ostream<>(l) << m;
    }

    template <typename T>
    inline lazy_ostream<const T &>
    operator<<(logger &l, const T &data)
    {
        return lazy_ostream<>(l) << data;
    }
    
    inline lazy_ostream<> 
    operator<<(logger &l, const log_async_t &)
    {
        return lazy_ostream<>(l, true);
    }
    
}

#endif /* _LOGGER_HPP_ */
