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

#include <stdexcept>
#include <csignal>
#include <thread>
#include <atomic>
#include <map>
#include <initializer_list>

#include <iostream>

namespace more {

    // unix_signal exception...
    //
    
    class unix_signal : public std::runtime_error
    {
        int sig_;

    public:

        explicit 
        unix_signal(int sig)
        : std::runtime_error("unix signal " + std::to_string(sig))
        , sig_(sig)
        {}

        virtual ~unix_signal() noexcept
        {}

        int signum() const noexcept
        {
            return sig_;
        }
    };
    
    // signal handler...
    //
    
    class signal_handler {

    private:

        std::map<int, std::function<void()>> handler_;
        std::chrono::system_clock::duration timeout_;
        std::atomic_int sig_;
        sigset_t set_;

        signal_handler() 
        : handler_()
        , timeout_(std::chrono::seconds(1))
        , sig_()
        , set_()
        {
        }

        ~signal_handler()
        {
        }

        void init_(std::chrono::milliseconds t)
        {
            // setup timeout...
            //
            
            timeout_ = std::move(t);
            
            // block signals for the calling thread/process...
            //
            
            sigfillset(&set_); 

            // except for the following ones:
            //

            sigdelset(&set_, SIGBUS);
            sigdelset(&set_, SIGFPE);
            sigdelset(&set_, SIGILL);
            sigdelset(&set_, SIGSEGV);
            
            sigprocmask(SIG_BLOCK, &set_, nullptr);

            // start signal handler thread...
            //

            std::thread([this](){
       
                int sig;
                for(;;)
                {
                    sigwait(&set_, &sig);

                    auto it = handler_.find(sig);
                    if (it != std::end(handler_))
                    {
                        it->second();
                    }
                    else
                    {
                        int expected = 0, n = 0;
                        
                        auto start = std::chrono::system_clock::now();

                        while(!sig_.compare_exchange_weak(expected, sig))
                        {
                            n++;

                            std::this_thread::sleep_for(std::chrono::milliseconds(10));

                            if (std::chrono::system_clock::now() - start > timeout_)
                                throw std::runtime_error("pending unix signal " + std::to_string(expected) + " timeout!");

                            expected = 0;
                        }
                    }
                }
                        
            }).detach();
        }

    public:

        signal_handler(signal_handler const&  other) = delete;

        signal_handler& 
        operator=(signal_handler const&  other) = delete;

        static signal_handler& 
        instance()
        {
            static signal_handler one;
            return one;
        }

        static void
        init(std::chrono::milliseconds t = std::chrono::seconds(1))
        {
            instance().init_(t);
        }

        static void
        init(std::initializer_list<decltype(handler_)::value_type> hs,
             std::chrono::milliseconds t = std::chrono::seconds(1))
        {
            instance().handler_.insert(hs);
            instance().init_(t);
        }

        static void
        may_throw()
        {
            auto sig = instance().sig_.exchange(0);
            if (sig)
            {
                throw unix_signal(sig);
            }
        }

        static std::unique_ptr<int>
        get()
        {
            auto sig = instance().sig_.exchange(0);
            return sig ? std::unique_ptr<int>(new int(sig)) :
                         std::unique_ptr<int>();
        }
    };


} // namespace more
