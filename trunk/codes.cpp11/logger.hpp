/* $Id$ */
/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <bonelli@antifork.org> wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return. Nicola Bonelli
 * ----------------------------------------------------------------------------
 */

#ifndef _MORE_LOGGER_HPP_
#define _MORE_LOGGER_HPP_

#include <iostream>
#include <fstream>
#include <sstream>
#include <memory>

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
    //////////// std::put_time is missing in g++ up to 4.7.x

    inline std::string
    put_time(const struct tm *tmb, const char *fmt)
    {
        char buf [64];
        if (!std::strftime(buf, 63, fmt, tmb))
            throw std::runtime_error("put_time: strftime");
        return buf;
    }

    //////////// rotate_file function

    inline void
    rotate_file(const std::string &name, int depth)
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

    /////////////////////////   more::safe_mutex: detect dead-locks at runtime.

    struct safe_mutex
    {
        void lock()
        {
            if (!mutex_.try_lock())
            {
                if (id_.load(std::memory_order_relaxed) == std::this_thread::get_id())
                    throw std::runtime_error("safe_mutex: deadlock detected");
                mutex_.lock();
            }
            id_.store(std::this_thread::get_id(), std::memory_order_relaxed);
            return;
        }

        bool try_lock()
        {
            if (!mutex_.try_lock())
            {
                if (id_.load(std::memory_order_relaxed) == std::this_thread::get_id())
                    throw std::runtime_error("safe_mutex: deadlock detected");
                return false;
            }
            else
            {
                id_.store(std::this_thread::get_id(), std::memory_order_relaxed);
                return true;
            }
        }

        void unlock()
        {
            id_.store(std::thread::id(), std::memory_order_relaxed);
            mutex_.unlock();
        }

    private:
        std::mutex      mutex_;
        std::atomic<std::thread::id> id_;
    };

    /////////////////////////   more::logger

    template <typename Mutex = safe_mutex>
    class logger
    {
        // delegating constructors are missing in g++-4.6.x
        //

        struct data_base
        {
            data_base(std::streambuf *fb, bool timestamp)
            : fname ()
            , fbuf  ()
            , out   (fb)
            , mutex ()
            , cond  ()
            , tstamp(timestamp)
            , ticket(0)
            , done  (0)
            {}

            data_base(const char *name, bool timestamp)
            : fname (name)
            , fbuf  (new std::filebuf())
            , out   (nullptr)
            , mutex ()
            , cond  ()
            , tstamp(timestamp)
            , ticket(0)
            , done  (0)
            {
                if(!fbuf->open(name, std::ios_base::out|std::ios_base::app))
                    throw std::system_error(errno, std::generic_category(), "filebuf: open");

                out.rdbuf(fbuf.get());
            }

            std::string                     fname;
            std::unique_ptr<std::filebuf>   fbuf;
            std::ostream                    out;

            mutable Mutex mutex;
            std::condition_variable_any cond;

            bool tstamp;

            std::atomic_ulong ticket;
            unsigned long done;
        };

        struct data : data_base
        {
            data(bool timestamp)
            : data_base(std::cout.rdbuf(), timestamp)
            {}

            data(std::streambuf *fb, bool timestamp)
            : data_base(fb, timestamp)
            {}

            data(const char *name, bool timestamp)
            : data_base(name,timestamp)
            {}
        };

    public:

        logger(bool timestamp = true)
        : data_(new data(timestamp))
        { }

        explicit
        logger(std::streambuf *sb, bool timestamp = true)
        : data_(new data(sb, timestamp))
        { }

        explicit
        logger(const char *filename, bool timestamp = true)
        : data_(new data(filename, timestamp))
        { }

        logger(logger&&) = default;

        ~logger() = default;

        std::string
        name() const
        {
            return data_->fname;
        }

        void
        open(std::string filename, std::ios_base::openmode mode = std::ios_base::out|std::ios_base::trunc)
        {
            std::lock_guard<Mutex> lock(data_->mutex);

            data_->fbuf.reset(new std::filebuf());

            if (!data_->fbuf->open(filename, std::ios_base::out | mode))
                throw std::system_error(errno, std::generic_category(), "filebuf: could not open " + filename);

            data_->fname = std::move(filename);

            data_->out.rdbuf(data_->fbuf.get());
        }

        void
        close()
        {
            rdbuf(nullptr);
        }

        void
        rdbuf(std::streambuf *sb)
        {
            std::lock_guard<Mutex> lock(data_->mutex);
            data_->fbuf.reset(nullptr);
            data_->fname.clear();
            data_->out.rdbuf(sb);
        }

        std::streambuf *
        rdbuf() const
        {
            return data_->out.rdbuf();
        }

        void timestamp(bool value)
        {
            data_->tstamp = value;
        }

        bool timestamp() const
        {
            return data_->tstamp;
        }

        //// log message synchronously

        template <typename Fun>
        void sync(Fun const &fun)
        {
            sync_(std::make_pair(false, 0), fun);
        }

        //// log message asynchronously

        template <typename Fun>
        void async(Fun const &fun)
        {
            try
            {
                auto t = data_->ticket++;
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

        //// return the size of the log file

        size_t
        size() const
        {
            std::lock_guard<Mutex> lock(data_->mutex);
            return size_();
        }

        //// rotate the log file

        void rotate(int depth = 3, size_t max_size = 0)
        {
            std::lock_guard<Mutex> lock(data_->mutex);

            if (data_->fname.empty())
                return;

            if (size_() > max_size)
                rotate_(depth);
        }

        //// rotate the log file asynchronously

        void rotate_async(int depth = 3, size_t max_size = 0)
        {
            std::lock_guard<Mutex> lock(data_->mutex);

            if (data_->fname.empty())
                return;

            if (size_() > max_size)
            {
                std::thread([this, depth]()
                {
                    std::lock_guard<Mutex> lock(data_->mutex);
                    this->rotate_(depth);

                }).detach();
            }
        }

    private:

        size_t
        size_() const
        {
            auto fb = dynamic_cast<std::filebuf *>(data_->out.rdbuf());
            if (fb == nullptr)
                return 0;

            return static_cast<size_t>(fb->pubseekoff(0, std::ios_base::cur));
        }

        void
        rotate_(int depth = 3)
        {
            std::filebuf * fb = dynamic_cast<std::filebuf *>(data_->out.rdbuf());
            if (fb == nullptr)
                return;

            fb->close();

            bool rot = true;
            try
            {
                rotate_file(data_->fname, depth);
            }
            catch(...)
            {
                rot = false;
            }

            if (!fb->open(data_->fname, std::ios_base::out |std::ios_base::app))
                throw std::system_error(errno, std::generic_category(), "filebuf: open");

            if (!rot)
            {
                sync([=](std::ostream &out)
                {
                    out << "rotate: error while rotating files!" << std::endl;
                });
            }
        }

        template <typename Fun>
        void sync_(std::pair<bool, unsigned long> ticket, Fun const &fun)
        {
            std::unique_lock<Mutex> lock(data_->mutex);

            if (ticket.first)
            {
                data_->cond.wait(lock, [&]() { return ticket.second == data_->done; });
            }

            try
            {
                if (data_->tstamp)
                    data_->out << make_timestamp_();

                fun(data_->out);
            }
            catch(std::exception &e)
            {
                data_->out << "Exception: " << e.what() << std::endl;
            }

            if (ticket.first)
            {
                data_->done++;
                data_->cond.notify_all();
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
            return put_time(localtime_r(&now_c, &tm_c), "[ %F %T ] ");
        }

        std::unique_ptr<data>   data_;

    };


    //// more::lazy_logger a temporary stream that logs at its
    //// descrution point.

    namespace
    {
        struct log_async_t {} log_async = log_async_t {};
    }

    template <typename Mutex, typename ...Ts>
    struct lazy_logger
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

        lazy_logger(logger<Mutex> &l, bool as = false)
        : refs_  ()
        , enable_(true)
        , async_ (as)
        , log_   (l)
        {}

        lazy_logger(const lazy_logger &other)
        : refs_  (other.refs_)
        , enable_(true)
        , async_ (other.async_)
        , log_   (other.log_)
        {
            other.enable_ = false;
        }

        template <typename ... Tx, typename T>
        lazy_logger(lazy_logger<Mutex, Tx...> const &log, const T &data)
        : refs_  (std::tuple_cat(log.refs_, std::tie(data)))
        , enable_(true)
        , async_ (log.async_)
        , log_   (log.log_)
        {
            log.enable_ = false;
        }

        ~lazy_logger()
        {
            if (enable_)
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
        mutable bool enable_;
        mutable bool async_;
        logger<Mutex> &log_;
    };

    // manipulator are function template (unresolved function types) which cannot be
    // deduced by the template machinery
    //

    typedef std::ostream& (manip_t)(std::ostream&);

    // lazy_logger<Ts...> << data
    //

    template <typename Mut, typename ...Ts>
    inline lazy_logger<Mut, Ts..., manip_t &>
    operator<<(lazy_logger<Mut, Ts...> const &l, manip_t & m)
    {
        return lazy_logger<Mut, Ts..., manip_t &>(l, m);
    }

    template <typename Mut, typename ...Ts>
    inline lazy_logger<Mut, Ts...>
    operator<<(lazy_logger<Mut, Ts...> const &l, const log_async_t &)
    {
        return l.async_ = true, lazy_logger<Mut, Ts...>(l);
    }

    template <typename Mut, typename ...Ts, typename T>
    inline lazy_logger<Mut, Ts..., const T &>
    operator<<(lazy_logger<Mut, Ts...> const &l, const T &data)
    {
        return lazy_logger<Mut, Ts..., const T &>(l, data);
    }

    // more::logger << data
    //

    template <typename Mut>
    inline lazy_logger<Mut,manip_t &>
    operator<<(logger<Mut> &l, manip_t &m)
    {
        return lazy_logger<Mut>(l) << m;
    }

    template <typename Mut, typename T>
    inline lazy_logger<Mut,const T &>
    operator<<(logger<Mut> &l, const T &data)
    {
        return lazy_logger<Mut>(l) << data;
    }

    template <typename Mut>
    inline lazy_logger<Mut>
    operator<<(logger<Mut> &l, const log_async_t &)
    {
        return lazy_logger<Mut>(l, true);
    }

}

#endif /* _MORE_LOGGER_HPP_ */
