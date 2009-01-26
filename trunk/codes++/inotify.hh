/* $Id$ */
/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <bonelli@antifork.org> wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return. Nicola Bonelli 
 * ----------------------------------------------------------------------------
 */

//
// C++ flavor of inotify-code-example written by Robert Love and published 
// at http://www.linuxjournal.com/article/8478
//

#ifndef INOTIFY_HH
#define INOTIFY_HH

#include <stdexcept>
#include <iostream>
#include <string>
#include <list>
#include <cstring>

#include <sys/inotify.h>
#include <sys/select.h>
#include <errno.h>

#define ERR(x)      (std::string(x) + ": " + strerror(errno))
#define EVENT_SIZE  (sizeof (struct inotify_event))     /* size of the event structure, not counting name */
#define BUF_LEN     (1024 * (EVENT_SIZE + 16))          /* reasonable guess as to size of 1024 events */

namespace Linux {

    struct Inotify_event {
        int wd;               /* Watch descriptor.  */
        uint32_t mask;        /* Watch mask.  */
        uint32_t cookie;      /* Cookie to synchronize two events.  */
        uint32_t len;         /* Length (including NULs) of name.  */
        std::string name;     /* Name. */ 

        Inotify_event(int _wd, uint32_t _mask, uint32_t _cookie, uint32_t _len, const char *_name) 
            : wd(_wd), 
              mask(_mask), 
              cookie(_cookie), 
              len(_len), 
              name( _len ? std::string(_name) : std::string()) 
        {}
    }; 

    std::ostream &operator<<(std::ostream &o, const Inotify_event &i){

        o << "inotify_event:\n";
        o << "  wd     :" << std::dec << i.wd     << std::endl;
        o << "  mask   :" << std::hex << i.mask   << std::endl;
        o << "  cookie :" << std::hex << i.cookie << std::endl;
        o << "  len    :" << std::dec << i.len    << std::endl;
        if (i.len)
            o << "  name   :" << i.name << std::endl;   

        return o;
    }

    typedef std::list<Inotify_event> Inotify_list;

    template<int L=BUF_LEN>
        class Inotify {

            char _M_buf[L];
            int _M_fd;

            public:

            Inotify()
            :   _M_buf(),
                _M_fd(inotify_init())
            {
                if ( _M_fd < 0 )
                    throw std::runtime_error(ERR("inotify_init")); 
            }

            ~Inotify() { ::close(_M_fd); }

            int add_watch(const char *pathname, uint32_t mask) {
                return inotify_add_watch(_M_fd, pathname, mask); 
            }

            int rm_watch(uint32_t wd) {
                return inotify_rm_watch(_M_fd, wd);
            }

            Inotify_list wait_events() 
            {
                int len, i = 0;
                for (;;) {
                    len = read (_M_fd, _M_buf, L);
                    if (len < 0 && errno == EINTR )
                        continue;
                    if (len < 0)
                        throw std::runtime_error(ERR("wait_event"));

                    break;
                }

                Inotify_list ret;
                struct inotify_event *event;

                for (; i < len ; i += EVENT_SIZE + event->len) {

                    event = reinterpret_cast<struct inotify_event *> (&_M_buf[i]);

                    Inotify_event tmp (event->wd, event->mask, event->cookie, event->len, event->name);
                    ret.push_back(tmp);                 
                }

                return ret;
            }

        };

}

#undef ERR
#undef EVENT_SIZE 
#undef BUF_LEN

#endif /* INOTIFY_HH */

