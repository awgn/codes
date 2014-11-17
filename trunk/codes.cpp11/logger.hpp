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
#include <vector>

#include <exception>
#include <thread>
#include <atomic>
#include <mutex>
#include <condition_variable>
#include <ctime>
#include <cerrno>
#include <system_error>
#include <initializer_list>


namespace more
{
    //////////// std::put_time is missing in g++ up to 4.7.x

    namespace decorator
    {
        static inline std::string
        put_time(const struct tm *tmb, const char *fmt)
        {
            char buf [64];
            if (!std::strftime(buf, 63, fmt, tmb))
                throw std::runtime_error("put_time: strftime");
            return buf;
        }

        static inline std::string
        timestamp()
        {
            auto now_c = std::chrono::system_clock::to_time_t
                         (
                            std::chrono::system_clock::now()
                         );

            struct tm tm_c;
            return put_time(localtime_r(&now_c, &tm_c), "[%F %T]");
        }

        static inline std::string
        thread_id()
        {
            std::ostringstream out;
            out << '[' << std::this_thread::get_id() << ']';
            return out.str();
        }
    }

    //////////// rotate_file function

    static inline void
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


    /////////////////////////   more::null_mutex

    struct null_mutex
    {
        void lock()
        { }

        bool try_lock()
        {
            return true;
        }

        void unlock()
        { }
    };

    /////////////////////////   more::logger

    class mini_logger
    {
    public:

        mini_logger(std::streambuf *pb)
        : fname_()
        , fout_ (new std::ostream(pb))
        , deco_ ()
        { }

        mini_logger(const char *name, std::ios_base::openmode mode)
        : fname_(name)
        , fout_ ()
        , deco_ ()
        {
            auto ofs = new std::ofstream(name, mode);
            if (!*ofs)
                throw std::runtime_error("logger: could not open " + std::string(name) + " file");
            fout_ = std::unique_ptr<std::ostream>(ofs);
        }

        mini_logger(mini_logger &&) = default;
        mini_logger& operator=(mini_logger &&) = default;

        //// specify decorators

        void decorators(std::initializer_list<std::function<std::string()>> init)
        {
            deco_ = decltype(deco_)(init);
        }

        //// log message

        template <typename Fun>
        void sync(Fun const &fun)
        {
            try
            {
                for(auto & f : deco_)
                    *fout_ << f() << ' ';

                fun(*fout_);
            }
            catch(std::exception &e)
            {
                *fout_ << "Exception: " << e.what() << std::endl;
            }
        }

        //// get file name

        std::string
        name() const
        {
            return fname_;
        }

        //// open/reopen log

        void
        open(std::string filename, std::ios_base::openmode mode)
        {
            auto ofs = new std::ofstream(filename, mode | std::ios_base::out);
            if (!*ofs)
                throw std::runtime_error("logger: could not open " + std::string(filename) + " file");

            fout_.reset(ofs);
            fname_ = std::move(filename);
        }

        //// close log

        void
        close()
        {
            rdbuf(nullptr);
        }

        //// rdbuf utility functions

        void
        rdbuf(std::streambuf *sb)
        {
            fout_.reset(new std::ostream(sb));
            fname_.clear();
        }

        std::streambuf *
        rdbuf() const
        {
            if (fout_)
                return fout_->rdbuf();
            return nullptr;
        }

        //// return the size of the log file

        size_t
        size() const
        {
            return size_();
        }

        //// rotate the log file

        void rotate(int depth = 3, size_t max_size = 0)
        {
            if (fname_.empty())
                return;
            if (this->size_() > max_size)
                rotate_(depth);
        }

    private:

        size_t
        size_() const
        {
            auto fb = dynamic_cast<std::filebuf *>(fout_->rdbuf());
            if (fb == nullptr)
                return 0;

            return static_cast<size_t>(fb->pubseekoff(0, std::ios_base::cur));
        }

        void
        rotate_(int depth = 3)
        {
            std::filebuf * fb = dynamic_cast<std::filebuf *>(fout_->rdbuf());
            if (fb == nullptr)
                return;

            fb->close();

            bool rot = true;
            try
            {
                rotate_file(fname_, depth);
            }
            catch(...)
            {
                rot = false;
            }

            if (!fb->open(fname_, std::ios_base::out |std::ios_base::app))
                throw std::system_error(errno, std::generic_category(), "filebuf: open");

            if (!rot)
            {
                sync([=](std::ostream &out)
                {
                    out << "rotate: error while rotating files!" << std::endl;
                });
            }
        }

        std::string                                 fname_;
        std::unique_ptr<std::ostream>               fout_;
        std::vector<std::function<std::string()>>   deco_;
    };



    template <typename Mutex = safe_mutex>
    class logger
    {
        struct logger_data
        {
            logger_data() : ticket(0), done(0) { }

            mutable Mutex                 mutex;
            std::condition_variable_any   cond;
            std::atomic_ulong             ticket;
            unsigned long                 done;
        };

    public:

        logger()
        : logs_  ()
        , data_  (std::unique_ptr<logger_data>(new logger_data))
        { }

        explicit
        logger(std::streambuf *sb)
        : logs_  ()
        , data_  (std::unique_ptr<logger_data>(new logger_data))
        {
            logs_.push_back(std::unique_ptr<mini_logger>(new mini_logger(sb)));
        }

        explicit
        logger(const char *filename, std::ios_base::openmode mode = std::ios_base::out)
        : logs_  ()
        , data_  (std::unique_ptr<logger_data>(new logger_data))
        {
            logs_.push_back(std::unique_ptr<mini_logger>(new mini_logger(filename, mode)));
        }

        logger(logger&&) = default;

        ~logger() = default;

        //// log message asynchronously

        template <typename Fun>
        void sync(Fun const &fun)
        {
            this->sync_(std::make_pair(false, 0), fun);
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
                    this->sync_(std::make_pair(true, t), fun);

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

        //// mini logger forwarders...

        size_t
        open(std::string filename, std::ios_base::openmode mode = std::ios_base::out)
        {
            std::lock_guard<Mutex> lock(data_->mutex);
            logs_.push_back(std::unique_ptr<mini_logger>(new mini_logger(filename.c_str(), mode)));
            return logs_.size() - 1;
        }

        void
        open_at(size_t n, std::string filename, std::ios_base::openmode mode = std::ios_base::out)
        {
            std::lock_guard<Mutex> lock(data_->mutex);
            logs_.at(n)->open(filename.c_str(), mode);
        }

        std::streambuf *
        rdbuf_at(size_t n) const
        {
            std::lock_guard<Mutex> lock(data_->mutex);
            return logs_.at(n)->rdbuf();
        }

        size_t
        rdbuf(std::streambuf *sb)
        {
            std::lock_guard<Mutex> lock(data_->mutex);
            logs_.push_back(std::unique_ptr<mini_logger>(new mini_logger(sb)));
            return logs_.size() - 1;
        }

        void
        rdbuf_at(size_t n, std::streambuf *sb)
        {
            std::lock_guard<Mutex> lock(data_->mutex);
            logs_.at(n)->rdbuf(sb);
        }

        void
        close()
        {
            std::lock_guard<Mutex> lock(data_->mutex);
            if (logs_.empty())
                throw std::logic_error("logger: close");
            logs_.pop_back();
        }

        std::string
        name_at(size_t n) const
        {
            std::lock_guard<Mutex> lock(data_->mutex);
            return logs_.at(n)->name();
        }

        size_t
        size_at(size_t n) const
        {
            std::lock_guard<Mutex> lock(data_->mutex);
            return logs_.at(n)->size();
        }

        void rotate(int depth = 3, size_t max_size = 0)
        {
            std::lock_guard<Mutex> lock(data_->mutex);
            for(auto &log : logs_)
                log->rotate(depth, max_size);
        }

        void rotate_at(size_t n, int depth = 3, size_t max_size = 0)
        {
            std::lock_guard<Mutex> lock(data_->mutex);
            logs_.at(n)->rotate(depth, max_size);
        }

        void decorators(std::initializer_list<std::function<std::string()>> init)
        {
            std::lock_guard<Mutex> lock(data_->mutex);
            for(auto &log : logs_)
                log->decorators(init);
        }

        void
        decorators_at(size_t n, std::initializer_list<std::function<std::string()>> init)
        {
            std::lock_guard<Mutex> lock(data_->mutex);
            logs_.at(n)->decorators(init);
        }

    private:

        template <typename Fun>
        void sync_(std::pair<bool, unsigned long> ticket, Fun const &fun)
        {
            std::unique_lock<Mutex> lock(data_->mutex);

            if (ticket.first)
            {
                data_->cond.wait(lock, [&]() { return ticket.second == data_->done; });
            }

            for(auto &log : logs_)
            {
                log->sync(fun);
            }

            if (ticket.first)
            {
                data_->done++;
                data_->cond.notify_all();
            }
        }

        std::vector<std::unique_ptr<mini_logger>> logs_;
        std::unique_ptr<logger_data>  data_;
    };


    namespace
    {
        //// lazy_logger is a temporary logger that accumulates and log messages at its descrution point.

        struct log_async_t {} log_async = log_async_t {};

        template <typename Mutex>
        struct lazy_logger
        {
            lazy_logger(logger<Mutex> &log, bool as = false)
            : accum_ ()
            , enable_(true)
            , async_ (as)
            , log_   (log)
            {}

            lazy_logger(lazy_logger && other)
            : accum_ (std::move(other.accum_))
            , enable_(true)
            , async_ (other.async_)
            , log_   (other.log_)
            {
                out_.flags(other.out_.flags());
                other.enable_ = false;
            }

            template <typename T>
            lazy_logger(lazy_logger && other, const T &data)
            : accum_ ()
            , enable_(true)
            , async_ (other.async_)
            , log_   (other.log_)
            {
                out_.flags(other.out_.flags());
                out_ << data;

                accum_ = std::move(other.accum_) + out_.str();
                other.enable_ = false;
            }

            lazy_logger(const lazy_logger &) = delete;
            lazy_logger&operator=(const lazy_logger &) = delete;
            lazy_logger&operator=(lazy_logger &&) = delete;

            ~lazy_logger()
            {
                if (enable_)
                {
                    if (async_)
                    {
                        auto str = std::move(accum_);
                        log_.async([str](std::ostream &o)
                                   {
                                      o << str << std::flush;
                                   });
                    }
                    else
                    {
                        log_.sync([this](std::ostream &o)
                                  {
                                      o << accum_ << std::flush;
                                  });
                    }
                }
            }

            std::string accum_;

            mutable bool enable_;
            mutable bool async_;
            logger<Mutex> &log_;

            std::ostringstream out_;
        };

        // manipulator are function template (unresolved function types) which cannot be
        // deduced by the template machinery
        //

        typedef std::ostream& (manip_t)(std::ostream&);

        // lazy_logger<Ts...> << data
        //

        template <typename Mut>
        inline lazy_logger<Mut>
        operator<<(lazy_logger<Mut> &&l, manip_t & m)
        {
            return lazy_logger<Mut>(std::move(l), m);
        }

        template <typename Mut>
        inline lazy_logger<Mut>
        operator<<(lazy_logger<Mut> &&l, const log_async_t &)
        {
            return l.async_ = true, lazy_logger<Mut>(std::move(l));
        }

        template <typename Mut, typename T>
        inline lazy_logger<Mut>
        operator<<(lazy_logger<Mut> &&l, const T &data)
        {
            return lazy_logger<Mut>(std::move(l), data);
        }

    } // unnamed namespace

    // more::logger << data
    //

    template <typename Mut>
    inline lazy_logger<Mut>
    operator<<(logger<Mut> &l, manip_t &m)
    {
        return lazy_logger<Mut>(l) << m;
    }

    template <typename Mut, typename T>
    inline lazy_logger<Mut>
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
