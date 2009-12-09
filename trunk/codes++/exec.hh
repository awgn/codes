/* $Id$ */
/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <bonelli@antifork.org> wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return. Nicola Bonelli 
 * ----------------------------------------------------------------------------
 */

#ifndef _EXEC_HH_
#define _EXEC_HH_

#include <tr1/functional>
#include <iostream>
#include <sstream>
#include <algorithm>
#include <vector>
#include <string>
#include <cstring>
#include <cstdio>
#include <iterator>
#include <cassert>

#include <sys/types.h>
#include <sys/wait.h>

#include <unistd.h>
#include <signal.h>

#include <error.hh>

namespace more {

    class exec
    {
    public:
        typedef std::tr1::function<int(const char *, char * const[])> exec_type;

        exec(const std::string &arg0 = std::string(), exec_type ex = ::execv)
        : _M_arg(),
          _M_status(-1),
          _M_pipe(),
          _M_delay(0),
          _M_wait(false),
          _M_pid(getpid()),
          _M_exec(ex)
        { 
            if (!arg0.empty())
                _M_arg.push_back(arg0); 
        }

        ~exec()
        {
            if (_M_pipe[0] > fileno(stderr)) {
                close(_M_pipe[0]);
            }
            if (_M_wait)
                this->wait();
        }

        exec &
        arg(const std::string &arg)
        { _M_arg.push_back(arg); return *this; }

        exec &
        cmdline(const std::string &cmd)
        {
            _M_arg.clear();
            std::stringstream tmp(cmd);
            std::copy(std::istream_iterator<std::string>(tmp),
                      std::istream_iterator<std::string>(),
                      std::back_inserter(_M_arg));
            return *this;
        }

        std::string
        cmdline() const 
        {
            std::stringstream tmp;
            std::copy(_M_arg.begin(), _M_arg.end(), std::ostream_iterator<std::string>(tmp," "));
            return tmp.str();
        }

        template <int fd>
        struct redirect_fd 
        {
            enum { value = fd };

            redirect_fd(int &nfd)
            : _M_nfd(nfd)
            {}

            void set(int n)
            { _M_nfd = n; }

        private:
            int & _M_nfd;
        };

        bool operator()()
        {
            _M_wait = true;
            _M_pid = fork();
            if (_M_pid == -1) {
                std::clog << "exec::fork: " << pretty_strerror(errno) << std::endl;
                return false;
            }

            if (_M_pid == 0) { // child
                run();
            }
           
            return true; 
        }

        template <int fd>
        bool operator()( redirect_fd<fd> nf )
        {            
            _M_wait = true;
            if ( pipe(_M_pipe) < 0 ) {
                std::clog << "exec::pipe: " << pretty_strerror(errno) << std::endl;
                return false;
            }

            _M_pid = fork();
            if (_M_pid == -1) {
                std::clog << "exec::fork: " << pretty_strerror(errno) << std::endl;
                close(_M_pipe[0]);
                close(_M_pipe[1]);
                _M_pipe[0] = 0;
                _M_pipe[1] = 0;
                return false;
            }

            if (_M_pid == 0) { // child

                close(_M_pipe[0]);
                close(fd);

                dup2(_M_pipe[1], fd);
                run();
            }

            close(_M_pipe[1]);
            nf.set(_M_pipe[0]);

            return true; 
        }

        int kill(int sig)
        { 
            if ( getpid() == _M_pid ) {
                std::clog << "exec::kill: INTERNAL ERROR (pid unitialized!)" << std::endl;
                return -1; 
            } 
            
            if (_M_pid > 0) // this kill is not meant to kill the world!
                return ::kill(_M_pid,sig); 

            return -1; 
        }

        bool wait()
        {
            _M_wait = false;
            if ( getpid() == _M_pid) {
                std::clog << "exec::kill: INTERNAL ERROR (pid unitialized!)" << std::endl;
                return -1; 
            }

            if ( waitpid(_M_pid,&_M_status,0) < 0 ) {
                std::clog << "exec::waitpid: " << pretty_strerror(errno) << std::endl;
                return false;
            }   
            return true;
        }

        void delay(int value_msec)
        { _M_delay = value_msec; }

        // is_exited() -> exit_status()
        //

        bool is_exited() const 
        { return WIFEXITED(_M_status); }

        int exit_status() const
        { assert(WIFEXITED(_M_status));
            return WEXITSTATUS(_M_status); }

        // is_signaled() -> term_signal()
        //

        int is_signaled() const
        { return WIFSIGNALED(_M_status); }

        int term_signal() const
        { assert(WIFSIGNALED(_M_status));
            return WTERMSIG(_M_status); }

        // is_stopped() -> stop_signal()
        //

        int is_stopped() const
        { return WIFSTOPPED(_M_status); }

        int stop_signal() const
        { assert(WIFSTOPPED(_M_status)); 
            return WSTOPSIG(_M_status); }

        pid_t
        pid() const 
        { return _M_pid; }

    private:

        std::vector<std::string> _M_arg;

        int     _M_status;
        int     _M_pipe[2];
        int     _M_delay;
        bool    _M_wait;
        pid_t   _M_pid;

        exec_type _M_exec;

        void run()
        {
            int n = _M_arg.size();
            const char *argv[n+1];

            for(int i=0; i < n;i++) 
                argv[i]=_M_arg[i].c_str();
            argv[n] = 0;

            if (_M_delay)
                usleep(_M_delay*1000);

            if ( _M_exec(argv[0], const_cast<char * const *>(argv)) == -1 ) {
                std::clog << "exec::exec: " << pretty_strerror(errno) << std::endl;
                raise(SIGABRT);            
            }
        }
    };

} // namespace more

#endif /* _EXEC_HH_  */

 
