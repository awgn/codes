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
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>

#include <iostream>
#include <cstring>
#include <cerrno>

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
        : m_fd(),
          m_status(false)
        {
            m_fd = open(filelock, O_RDONLY|O_CREAT);
            if (m_fd == -1) {
                std::clog << "open: " << std::strerror(errno) << std::endl;
                return;
            }
            if (flock(m_fd, flock_operation<MODE>::value) < 0 ) {
                std::clog << "flock: " << std::strerror(errno) << std::endl;
                return;
            }
            m_status = true;
        }

        ~scoped_flock()
        {
            if (flock(m_fd, LOCK_UN) < 0 ) {
                std::clog << "flock(...,LOCK_UN: " << std::strerror(errno) << std::endl;
                return;
            }
            close(m_fd);
        }

        bool is_open() const
        { return m_fd >=0; }

        bool is_locked() const 
        { return m_status; }

    private:
        scoped_flock(const scoped_flock &);
        scoped_flock &operator=(const scoped_flock &);

        int  m_fd;
        bool m_status;
    };

} // namespace more

#endif /* FLOCK_HH */
