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
#include <cassert>
#include <system_error>

#include <tuple_ext.hpp>   // !more

namespace more
{
    namespace 
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
        
        void rotate_file(const std::string &name, int level)
        {
            auto ext = [](int n) -> std::string 
            { 
                return n > 0 ? ('.' + std::to_string(n)) : ""; 
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
    }

    //// the ofstream with a name (used for log rotation)

    template <typename CharT, typename Traits = std::char_traits<CharT> >
    struct named_ofstream : public std::basic_ofstream<CharT, Traits>
    {
        named_ofstream()
        : std::basic_ofstream<CharT, Traits>()
        , filename_()
        {}

        explicit
        named_ofstream(const char *s,
                       std::ios_base::openmode mode = std::ios_base::out|std::ios_base::trunc)
        : std::basic_ofstream<CharT, Traits>(s, mode)
        , filename_(s)
        {}

        explicit
        named_ofstream(const std::string &s,
                       std::ios_base::openmode mode = std::ios_base::out|std::ios_base::trunc)
        : std::basic_ofstream<CharT, Traits>(s, mode)
        , filename_(s)
        {}

        std::string
        name() const
        {
            return filename_;
        }

        void open(const char *filename, std::ios_base::openmode mode = std::ios_base::out)
        {
            std::basic_ofstream<CharT, Traits>::open(filename, mode);
            filename_.assign(filename);
        }

        void open(const std::string &filename, std::ios_base::openmode mode = std::ios_base::out)
        {
            std::basic_ofstream<CharT, Traits>::open(filename, mode);
            filename_.assign(filename);
        }

        void close()
        {
            std::basic_ofstream<CharT, Traits>::close();
            filename_.clear();
        }

    private:
        std::string filename_;
    };


    using nofstream  = named_ofstream<char>;
    using nwofstream = named_ofstream<wchar_t>;


    /////////////////////////   more::logger

    class logger
    {
    public:

        logger(bool timestamp = true)
        : file_()
        , log_(std::cout.rdbuf())
        , mutex_()
        , timestamp_(timestamp)
        , ticket_()
        , done_()
        {
        }

        explicit
        logger(std::streambuf *sb, bool timestamp = true)
        : file_()
        , log_(sb)
        , mutex_()
        , timestamp_(timestamp)
        , ticket_(0)
        , done_()
        {
        }
        
        explicit
        logger(const char *filename, bool timestamp = true)
        : file_(filename)
        , log_(file_.rdbuf())
        , mutex_()
        , timestamp_(timestamp)
        , ticket_(0)
        , done_()
        {
        }
        
        ~logger() = default;

        std::string
        name() const
        {
            return file_.name();
        }
        
        void
        name(std::string filename, std::ios_base::openmode mode)  
        {
            std::lock_guard<std::mutex> lock(mutex_);
            file_.close();
            file_.open(filename, mode);
            log_.rdbuf(file_.rdbuf());
        }
        
        void
        rdbuf(std::streambuf *sb) 
        {
            std::lock_guard<std::mutex> lock(mutex_);
            file_.close();
            log_.rdbuf(sb);
        }

        std::streambuf *
        rdbuf() const
        {
            return log_.rdbuf();
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
                std::thread([this, fun]() 
                { 
                    auto t = this->ticket_++;
                    sync_(t, fun); 
                
                }).detach();
            }
            catch(...)
            {
                sync([fun](std::ostream &out) 
                {
                    out << "exception: log thread could not be started!" << std::endl;
                    fun(out);
                });
            }
        }

        //// log message: sync
        
        template <typename Fun>
        void sync(Fun const &fun)
        {
            auto t = ticket_++;
            sync_(t, fun);
        }

        //// log message: sync_ratelimit

        template <size_t N, typename Fun>
        void sync_ratelimit(std::chrono::system_clock::time_point now, Fun const &fun)
        {
            auto r = ratelimit_<N>(now);
            if (r.first)
            {
                sync(fun);
            }
            else if (r.second)
            {
                sync([=](std::ostream &out)
                { 
                    out << r.second << " message suppressed." << std::endl; 
                });
            }
        }
        
        //// log message: sync_ratelimit

        template <size_t N, typename Fun>
        void async_ratelimit(std::chrono::system_clock::time_point now, Fun const &fun)
        {
            auto r = ratelimit_<N>(now);
            if (r.first)
            {
                async(fun);
            }
            else if (r.second)
            {
                async([=](std::ostream &out)
                { 
                    out << r.second << " message suppressed." << std::endl; 
                });
            }
        }

        //// return the size of the log file
        
        size_t
        size() const
        {
            std::lock_guard<std::mutex> lock(mutex_);

            auto fb = dynamic_cast<std::filebuf *>(log_.rdbuf());
            if (fb == nullptr) 
                return 0;

            auto n = fb->pubseekoff(0, std::ios_base::cur);
            assert(n >= 0);

            return static_cast<size_t>(n);
        }
        
        //// rotate the log file 

        void rotate(int level = 3)
        {
            auto name = file_.name();
            
            if (name.empty())
                return;

            std::thread([=]() {
                
                std::lock_guard<std::mutex> lock(mutex_);

                std::filebuf * fb = dynamic_cast<std::filebuf *>(log_.rdbuf());
                if (fb == nullptr) 
                    return; 
                
                fb->close();

                rotate_file(name, level);

                if (!fb->open(name, std::ios::out))
                    throw std::runtime_error( "logger: rotate " + name);

            }).detach();
        }

    private:
        
        template <int N>
        std::pair<bool, int> ratelimit_(std::chrono::system_clock::time_point now)
        {
            struct ratelimit
            {
                std::chrono::system_clock::time_point tp;
                int32_t rate;
            };

            static __thread ratelimit *rt;

            if (!rt) 
                rt = new ratelimit{ std::chrono::system_clock::time_point(), 0 };

            int delta = 0;

            if (std::chrono::system_clock::to_time_t(rt->tp) !=
                std::chrono::system_clock::to_time_t(now))
            {
                rt->tp = now;
                delta = (rt->rate > 2*N ? (rt->rate - N) : (rt->rate > N ? N : rt->rate));
                rt->rate -= delta;
            }
            else 
            {
                rt->rate++;
            }

            return std::make_pair(rt->rate < N, delta);
        }


        template <typename Fun>
        void sync_(unsigned long t, Fun const &fun)
        {
            std::unique_lock<std::mutex> lock(mutex_);
            cond_.wait(lock, [&]() -> bool { return t == done_; });
            try
            {
                if (timestamp_)
                    log_ << make_timestamp_();

                fun(log_);
            }
            catch(std::exception &e)
            {
                log_ << "Exception: " << e.what() << std::endl;
            }

            done_++;
            cond_.notify_all();
        }

        static std::string
        make_timestamp_()
        {
            auto now_c = std::chrono::system_clock::to_time_t
                         (
                            std::chrono::system_clock::now()
                         );

            struct tm tm_c;
            return put_time(localtime_r(&now_c, &tm_c), "[ %F %T ] ");                    
        }   

        nofstream file_;

        std::ostream log_;
        mutable std::mutex mutex_;
        std::condition_variable cond_;

        bool timestamp_;
        
        std::atomic_ulong ticket_;
        unsigned long done_;
    };

    //// more::lazy_stream a temporary stream that log synchronously at its
    //// descrution point.


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
    inline lazy_stream<Ts..., manip_t &>
    operator<<(lazy_stream<Ts...> const &l, manip_t & m)
    {
        return lazy_stream<Ts..., manip_t &>(l, m);
    }

    template <typename ...Ts, typename T>
    inline lazy_stream<Ts..., const T &>
    operator<<(lazy_stream<Ts...> const &l, const T &data)
    {
        return lazy_stream<Ts..., const T &>(l, data);
    }

    // more::logger << data
    //

    inline lazy_stream<manip_t &>
    operator<<(logger &l, manip_t &m)
    {
        return lazy_stream<>(l) << m;
    }

    template <typename T>
    inline lazy_stream<const T &>
    operator<<(logger &l, const T &data)
    {
        return lazy_stream<>(l) << data;
    }
    
}

#endif /* _LOGGER_HPP_ */
