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
#include <tr1/array>

#include <iostream>
#include <sstream>
#include <vector>
#include <string>
#include <algorithm>
#include <iterator>
#include <cstdio>
#include <set>

#include <sys/types.h>
#include <sys/wait.h>

#include <unistd.h>
#include <signal.h>
#include <error.hh>

#include <string-utils.hh>  // more!

namespace more {

    class exec
    {
        friend class exec_group;

    public:
        enum fdnum { STDIN, STDOUT, STDERR };
        
        typedef std::tr1::function<int(const char *, char * const[])> exec_type;
        typedef std::pair<enum fdnum, std::tr1::reference_wrapper<int> > redirect_type;

    public:
        exec(const std::string &arg = std::string(), exec_type ex = ::execv /* ::execvp */)
        : _M_arg(),
          _M_redir(),
          _M_pipe(),
          _M_status(-1),
          _M_delay(0),
          _M_wait(false),
          _M_pid(getpid()),
          _M_exec(ex)
        {             
            if (!arg.empty()) {
                std::stringstream tmp(arg);
                std::copy(std::istream_iterator<std::string>(tmp),
                          std::istream_iterator<std::string>(),
                          std::back_inserter(_M_arg));
            }
        }

        // build exec array by means of iterators
        //

        struct simple_trim 
        {
            std::string operator()(const std::string &str) const
            {
                return more::trim_copy(str);
            }
        };

        template <typename Iter>
        exec(Iter beg, Iter end, exec_type ex = ::execv /* ::execvp */)
        : _M_arg(),
          _M_redir(),
          _M_pipe(),
          _M_status(-1),
          _M_delay(0),
          _M_wait(false),
          _M_pid(getpid()),
          _M_exec(ex)
        {
            // apply simple_trim while copying the strings from the range
            //
            std::transform(beg, end, std::back_inserter(_M_arg), simple_trim());
        }

        ~exec()
        {
            for(unsigned int i = 0; i < _M_redir.size(); i++)
            {
                int fd = _M_redir[i].first;
                if ( _M_pipe.at(fd)[!fd] > fileno(stderr) )
                    ::close(_M_pipe.at(fd)[!fd]);
            }

            if (_M_wait)
                this->wait();
        }

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

        exec &
        arg(const std::string &arg)
        { 
            _M_arg.push_back(arg); 
            return *this; 
        } 

        // return -1 in case of execve() failure!
        //

        int operator()()
        {
            _M_wait = true;
    
            // create pipes...
            //
            for(unsigned int i = 0; i < _M_redir.size(); i++)
            {
                int fd = _M_redir[i].first;
                if ( ::pipe( _M_pipe.at(fd) ) < 0 )
                    throw std::runtime_error(std::string("pipe: ").append(pretty_strerror(errno)));
            }
            
            // rationale: vfork() shares memory between parent and child.
            //            exec_ret is used to vehicle the return value of execve() in case of
            //            error.

            int exec_ret = 0; 

            _M_pid = ::vfork();
            if (_M_pid == -1) 
            {
                // close open pipes...
                // 
                for(unsigned int i=0; i < _M_redir.size(); i++)
                {
                    int fd = _M_redir[i].first;
                    ::close(_M_pipe.at(fd)[0]);
                    ::close(_M_pipe.at(fd)[1]);
                    _M_pipe.at(fd)[0] = 0;
                    _M_pipe.at(fd)[1] = 0;
                }
                
                throw std::runtime_error(std::string("fork: ").append(pretty_strerror(errno)));
            }

            if (_M_pid == 0) { // child

                for(unsigned int i=0; i < _M_redir.size(); i++)
                {
                    int fd = _M_redir[i].first;
                    ::close(_M_pipe.at(fd)[!fd]);
                    ::close(fd);
                    ::dup2(_M_pipe.at(fd)[!!fd], fd); 
                }

                if ( _M_run() < 0 ) {
                    exec_ret = -1;
                    exit(1);
                }
            }

            // parent...
            
            for(unsigned int i = 0; i < _M_redir.size(); i++)
            {
                int fd = _M_redir[i].first;
                ::close(_M_pipe.at(fd)[!!fd]);
                _M_redir[i].second.get() = _M_pipe.at(fd)[!fd];
            }

            return exec_ret;
        }

        void redirect(const redirect_type &x)
        {
            _M_redir.push_back(x);
        }

        int kill(int sig)
        { 
            if ( ::getpid() == _M_pid ) {
                throw std::runtime_error("exec::kill: (pid unitialized!)");
            } 
            
            if (_M_pid > 0) // this kill is not meant to kill the world!
                return ::kill(_M_pid,sig); 

            return -1; 
        }

        bool wait()
        {
            _M_wait = false;
            if (::getpid() == _M_pid) {
                throw std::runtime_error("exec::wait: (pid unitialized!)");
            }

            if (::waitpid(_M_pid,&_M_status,0) < 0 ) {
                std::clog << "exec::waitpid: " << pretty_strerror(errno) << std::endl;
                return false;
            }   
            return true;
        }

        void detach()
        {
            _M_wait = false;
        }

        void delay(int value_msec)
        { _M_delay = value_msec; }

        // is_exited() -> exit_status()
        //

        bool is_exited() const 
        { return WIFEXITED(_M_status); }

        int exit_status() const
        { 
            if (!WIFEXITED(_M_status))
                throw std::runtime_error("exec::exit_status: !EXITED");
            return WEXITSTATUS(_M_status); 
        }

        // is_signaled() -> term_signal()
        //

        int is_signaled() const
        { return WIFSIGNALED(_M_status); }

        int term_signal() const
        { 
            if (!WIFSIGNALED(_M_status))
                throw std::runtime_error("exec::term_signal: !SIGNALED");
            return WTERMSIG(_M_status); 
        }

        // is_stopped() -> stop_signal()
        //

        int is_stopped() const
        { return WIFSTOPPED(_M_status); }

        int stop_signal() const
        { 
            if(!WIFSTOPPED(_M_status)) 
                throw std::runtime_error("exec::stop_signal: !STOPPED");
            return WSTOPSIG(_M_status); 
        }

        pid_t
        pid() const 
        { return _M_pid; }

    private:
        std::vector<std::string>    _M_arg;
        std::vector<redirect_type>  _M_redir;
        std::tr1::array< int[2], 3> _M_pipe;

        int     _M_status;
        int     _M_delay;
        bool    _M_wait;
        pid_t   _M_pid;

        exec_type _M_exec;

        int _M_run()
        {
            int n = _M_arg.size();
            const char *argv[n+1];
            argv[n] = 0;
            
            for(int i=0; i < n;i++) 
                argv[i]=_M_arg[i].c_str();

            if (_M_delay)
                usleep(_M_delay*1000);

            if ( _M_exec(argv[0], const_cast<char * const *>(argv)) == -1 ) {
                std::clog << "exec::exec: " << pretty_strerror(errno) << std::endl;
                return -1;
            }

            return 0;
        }
    };

    // group of processes
    //

    class exec_group 
    {
    public:
        typedef std::set<exec *>::iterator          iterator;
        typedef std::set<exec *>::const_iterator    const_iterator;

        exec_group()
        : _M_group()
        {}

        ~exec_group()
        {}

        // iterators..

        iterator
        begin()
        { return _M_group.begin(); }

        iterator
        end()
        { return _M_group.end(); }

        const_iterator
        begin() const
        { return _M_group.begin(); }

        const_iterator
        end() const
        { return _M_group.end(); }

        bool add(exec *ptr)
        {
            return _M_group.insert(ptr).second;
        }

        void remove(exec *ptr)
        {
            _M_group.erase(ptr);
        }

        void run()
        {
            std::for_each(_M_group.begin(), _M_group.end(), std::tr1::mem_fn(&exec::operator()));
        }

        void
        wait_all()
        {
            std::for_each(_M_group.begin(), _M_group.end(), std::tr1::mem_fn(&exec::wait));
        }

        exec *
        wait()
        {
            int status;
            pid_t p = waitpid(0, &status, 0);
            if (p < 0)
                return NULL;

            std::set<exec *>::iterator it = _M_group.begin();
            for(; it != _M_group.end(); ++it) 
            {
                if ( p == (*it)->pid() ) { // found!
                    (*it)->_M_wait = false;
                    (*it)->_M_status = status;
                    return *it;
                }
            }    
            return NULL;
        }

    private:
        std::set<exec *> _M_group;
    };

} // namespace more

#endif /* _EXEC_HH_  */

 
