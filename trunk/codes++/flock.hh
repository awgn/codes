/* $Id$ */
/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <bonelli@antifork.org> wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return. Nicola Bonelli 
 * ----------------------------------------------------------------------------
 */

#ifndef FLOCK_HH
#define FLOCK_HH

#include <sys/file.h>
#include <errno.h>

#include <iostream>
#include <cstring>

namespace more 
{
    template <int> struct flock_operation;
    template <>    struct flock_operation<LOCK_EX>         { enum { value = LOCK_EX }; }; 
    template <>    struct flock_operation<LOCK_SH>         { enum { value = LOCK_SH }; }; 
    template <>    struct flock_operation<LOCK_EX|LOCK_NB> { enum { value = LOCK_EX|LOCK_NB }; }; 
    template <>    struct flock_operation<LOCK_SH|LOCK_NB> { enum { value = LOCK_SH|LOCK_NB }; }; 

    template <int MODE=LOCK_EX>
    class scoped_flock
    {
    public:
        explicit scoped_flock(const char *filelock) 
        : _M_fd(),
          _M_status(false)
        {
            _M_fd = open(filelock, O_RDONLY|O_CREAT);
            if (_M_fd == -1) {
                std::clog << "open: " << strerror(errno) << std::endl;
                return;
            }
            if (flock(_M_fd, flock_operation<MODE>::value) < 0 ) {
                std::clog << "flock: " << strerror(errno) << std::endl;
                return;
            }
            _M_status = true;
        }

        ~scoped_flock()
        {
            if (flock(_M_fd, LOCK_UN) < 0 ) {
                std::clog << "flock(...,LOCK_UN: " << strerror(errno) << std::endl;
                return;
            }
            close(_M_fd);
        }

        bool is_open() const
        { return _M_fd >=0; }

        bool is_locked() const 
        { return _M_status; }

    private:
        scoped_flock(const scoped_flock &);
        scoped_flock &operator=(const scoped_flock &);

        int  _M_fd;
        bool _M_status;
    };

} // namespace more

#endif /* FLOCK_HH */
