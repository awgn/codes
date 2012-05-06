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

#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <signal.h>

#include <system_error.hh>      // more!
#include <string-utils.hh>      // more!

#include <tr1/type_traits>   
#include <tr1/functional>    
#include <tr1/array>         
#include <tr1/memory>        

#include <iostream>
#include <sstream>
#include <vector>
#include <string>
#include <algorithm>
#include <iterator>
#include <cstdio>
#include <set>
#include <utility>
#include <functional>

using namespace std::tr1::placeholders;

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
        : m_arg(),
          m_redir(),
          m_pipe(),
          m_status(-1),
          m_delay(0),
          m_wait(false),
          m_pid(getpid()),
          m_exec(ex)
        {             
            if (!arg.empty()) {
                std::stringstream tmp(arg);
                std::copy(std::istream_iterator<std::string>(tmp),
                          std::istream_iterator<std::string>(),
                          std::back_inserter(m_arg));
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

        // a prolog is called in the child context between the vfork and the execve system call.
        // this is a dummy, nullprolog 
        //

        struct nullprolog : public std::unary_function<void, void> 
        {
            void operator()() const
            {}
        };

        template <typename Iter>
        exec(Iter beg, Iter end, exec_type ex = ::execv /* ::execvp */)
        : m_arg(),
          m_redir(),
          m_pipe(),
          m_status(-1),
          m_delay(0),
          m_wait(false),
          m_pid(getpid()),
          m_exec(ex)
        {
            // apply simple_trim while copying the strings from the range
            //
            std::transform(beg, end, std::back_inserter(m_arg), simple_trim());
        }

        ~exec()
        {
            for(unsigned int i = 0; i < m_redir.size(); i++)
            {
                int fd = m_redir[i].first;
                if ( m_pipe.at(fd)[!fd] > fileno(stderr) )
                    ::close(m_pipe.at(fd)[!fd]);
            }

            if (m_wait)
                this->wait();
        }

        exec &
        cmdline(const std::string &cmd)
        {
            m_arg.clear();
            std::stringstream tmp(cmd);
            std::copy(std::istream_iterator<std::string>(tmp),
                      std::istream_iterator<std::string>(),
                      std::back_inserter(m_arg));
            return *this;
        }

        std::string
        cmdline() const 
        {
            std::stringstream tmp;
            std::copy(m_arg.begin(), m_arg.end(), std::ostream_iterator<std::string>(tmp," "));
            return tmp.str();
        }

        exec &
        arg(const std::string &arg)
        { 
            m_arg.push_back(arg); 
            return *this; 
        } 

        // return -1 in case of execve() failure!
        //

        int operator()(std::tr1::function<void()> prolog = nullprolog())
        {
            m_wait = true;
    
            // create pipes...
            //
            for(unsigned int i = 0; i < m_redir.size(); i++)
            {
                int fd = m_redir[i].first;
                if ( ::pipe( m_pipe.at(fd) ) < 0 )
                    throw std::runtime_error(std::string("pipe: ").append(more::strerror(errno)));
            }
            
            // rationale: vfork() shares memory between parent and child.
            //            exec_ret is used to vehicle the return value of execve() in case of
            //            error.

            int exec_ret = 0; 

            m_pid = ::vfork();
            if (m_pid == -1) 
            {
                // close open pipes...
                // 
                for(unsigned int i=0; i < m_redir.size(); i++)
                {
                    int fd = m_redir[i].first;
                    ::close(m_pipe.at(fd)[0]);
                    ::close(m_pipe.at(fd)[1]);
                    m_pipe.at(fd)[0] = 0;
                    m_pipe.at(fd)[1] = 0;
                }
                
                throw std::runtime_error(std::string("fork: ").append(more::strerror(errno)));
            }

            if (m_pid == 0) { // child

                // invoke a user-defined prolog in the child.
                // note: only a limited number of operations can be done here.
                // Please refer to man 3 vfork.
                //

                prolog();

                for(unsigned int i=0; i < m_redir.size(); i++)
                {
                    int fd = m_redir[i].first;
                    ::close(m_pipe.at(fd)[!fd]);
                    ::close(fd);
                    ::dup2(m_pipe.at(fd)[!!fd], fd); 
                }

                if ( m_run() < 0 ) {
                    exec_ret = -1;
                    _exit(1);
                }
            }

            // parent...
            
            for(unsigned int i = 0; i < m_redir.size(); i++)
            {
                int fd = m_redir[i].first;
                ::close(m_pipe.at(fd)[!!fd]);
                m_redir[i].second.get() = m_pipe.at(fd)[!fd];
            }

            return exec_ret;
        }

        void redirect(const redirect_type &x)
        {
            m_redir.push_back(x);
        }

        int kill(int sig)
        { 
            if ( ::getpid() == m_pid ) {
                throw std::runtime_error("exec::kill: (pid unitialized!)");
            } 
            
            if (m_pid > 0) // this kill is not meant to kill the world!
                return ::kill(m_pid,sig); 

            return -1; 
        }

        bool wait()
        {
            m_wait = false;
            if (::getpid() == m_pid) {
                throw std::runtime_error("exec::wait: (pid unitialized!)");
            }

            if (::waitpid(m_pid,&m_status,0) < 0 ) {
                std::clog << "exec::waitpid: " << more::strerror(errno) << std::endl;
                return false;
            }   
            return true;
        }

        void detach()
        {
            m_wait = false;
        }

        void delay(int value_msec)
        { m_delay = value_msec; }

        // is_exited() -> exit_status()
        //

        bool is_exited() const 
        { return WIFEXITED(m_status); }

        int exit_status() const
        { 
            if (!WIFEXITED(m_status))
                throw std::runtime_error("exec::exit_status: !EXITED");
            return WEXITSTATUS(m_status); 
        }

        // is_signaled() -> term_signal()
        //

        int is_signaled() const
        { return WIFSIGNALED(m_status); }

        int term_signal() const
        { 
            if (!WIFSIGNALED(m_status))
                throw std::runtime_error("exec::term_signal: !SIGNALED");
            return WTERMSIG(m_status); 
        }

        // is_stopped() -> stop_signal()
        //

        int is_stopped() const
        { return WIFSTOPPED(m_status); }

        int stop_signal() const
        { 
            if(!WIFSTOPPED(m_status)) 
                throw std::runtime_error("exec::stop_signal: !STOPPED");
            return WSTOPSIG(m_status); 
        }

        pid_t
        pid() const 
        { return m_pid; }

    private:
        std::vector<std::string>    m_arg;
        std::vector<redirect_type>  m_redir;
        std::tr1::array< int[2], 3> m_pipe;

        int     m_status;
        int     m_delay;
        bool    m_wait;
        pid_t   m_pid;

        exec_type m_exec;

        int m_run()
        {
            int n = m_arg.size();
            const char *argv[n+1];
            argv[n] = 0;
            
            for(int i=0; i < n;i++) 
                argv[i]=m_arg[i].c_str();

            if (m_delay)
                usleep(m_delay*1000);

            if ( m_exec(argv[0], const_cast<char * const *>(argv)) == -1 ) {
                std::clog << "exec::exec: " << more::strerror(errno) << std::endl;
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
        : m_group()
        {}

        ~exec_group()
        {}

        // iterators..

        iterator
        begin()
        { return m_group.begin(); }

        iterator
        end()
        { return m_group.end(); }

        const_iterator
        begin() const
        { return m_group.begin(); }

        const_iterator
        end() const
        { return m_group.end(); }

        bool add(exec *ptr)
        {
            return m_group.insert(ptr).second;
        }

        void remove(exec *ptr)
        {
            m_group.erase(ptr);
        }

        void run()
        {
            std::for_each(m_group.begin(), m_group.end(), std::tr1::bind(&exec::operator(), _1, exec::nullprolog() /* default argument */ ));
        }

        template <typename T>
        void run(T prolog)
        {
            std::for_each(m_group.begin(), m_group.end(), std::tr1::bind(&exec::operator(), _1, prolog /* default argument */ ));
        }


        void
        wait_all()
        {
            std::for_each(m_group.begin(), m_group.end(), std::tr1::mem_fn(&exec::wait));
        }

        exec *
        wait()
        {
            int status;
            pid_t p = waitpid(0, &status, 0);
            if (p < 0)
                return NULL;

            std::set<exec *>::iterator it = m_group.begin();
            for(; it != m_group.end(); ++it) 
            {
                if ( p == (*it)->pid() ) { // found!
                    (*it)->m_wait = false;
                    (*it)->m_status = status;
                    return *it;
                }
            }    
            return NULL;
        }

    private:
        std::set<exec *> m_group;
    };

} // namespace more

#endif /* _EXEC_HH_  */

 
