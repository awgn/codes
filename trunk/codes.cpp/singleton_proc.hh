/* $Id$ */
/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <bonelli@antifork.org> wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return. Nicola Bonelli
 * ----------------------------------------------------------------------------
 */

#ifndef SINGLETON_PROC_HH
#define SINGLETON_PROC_HH

#include <sys/file.h>
#include <errno.h>
#include <err.h>

#include <iostream>

extern char *__progname;

namespace more 
{
    class singleton_proc 
    {
        int _M_fd;
    public:
        singleton_proc(const char *filelock) 
        : _M_fd(open(filelock, O_RDONLY|O_CREAT))
        {
            if (_M_fd == -1)
                errx(1, "couldn't open lockfile");

            if (flock(_M_fd, LOCK_EX|LOCK_NB) < 0 ) {
                if (errno == EWOULDBLOCK)
                    errx(1, "a session of '%s' is already running", __progname);
                else
                    err(1, "flock");
           }
        }

        ~singleton_proc() 
        {
            if (flock(_M_fd, LOCK_UN) < 0 ) {
                warn("flock(...,LOCK_UN)");
                return;
            }
            close(_M_fd);
        }
    
    };

}

#endif /* SINGLETON_PROC_HH */

