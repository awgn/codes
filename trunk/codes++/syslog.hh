/* $Id$ */
/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <bonelli@antifork.org> wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return. Nicola Bonelli
 * ----------------------------------------------------------------------------
 */

#ifndef SYSLOG_HH
#define SYSLOG_HH

#include <iostream>
#include <cassert>
#include <stdexcept>
#include <syslog.h>

namespace sys 
{
    class syslog : public std::streambuf 
    {

    public:
        // option: 	
        //      LOG_CONS	Write directly to system console
        //		LOG_NDELAY	Open the connection immediately
        //		LOG_NOWAIT	Don't wait for child processes
        //		LOG_ODELAY	Connection  is  delayed	
        //		LOG_PERROR	Print to stderr
        //		LOG_PID		Include PID

        // facility:	
        //      LOG_AUTH	sec/authorization messages (deprecated)
        //		LOG_AUTHPRIV	sec/authorization messages
        // 		LOG_CRON	clock daemon
        //		LOG_DAEMON	system daemons
        // 		LOG_FTP		ftp daemon
        //		LOG_KERN	kernel messages
        //		LOG_LOCAL0 through LOG_LOCAL7
        //		LOG_LPR		line printer subsystem
        //		LOG_MAIL	mail subsystem
        //		LOG_NEWS	USENET news subsystem
        //		LOG_SYSLOG	message by syslog
        //		LOG_USER 	generic (default)
        //		LOG_UUCP	UUCP subsystem

        // level:       
        //      LOG_EMERG	system is unusable
        //		LOG_ALERT 	action must be taken immediately
        //		LOG_CRIT 	critical conditions
        //		LOG_ERR 	error conditions
        //		LOG_WARNING 	warning conditions
        //		LOG_NOTICE 	normal, but significant, condition
        //		LOG_INFO 	informational message
        //		LOG_DEBUG 	debug-level message

        // priority:    facility|level

        explicit syslog(int opt, int fac = LOG_USER, int lev = LOG_NOTICE )  
        : _M_priority(0),
          _M_cursor(0),
          _M_option(opt)
        {
            _M_facility() = fac;
            _M_level()    = lev;
        } 

        ~syslog()  
        {}

        syslog &
        open(const char *ident)
        {
            // std::cout << __PRETTY_FUNCTION__ << std::endl;
            if (ident)
                ::openlog(ident,_M_option,_M_facility());
            return *this;
        }

        syslog &
        setfacility(int fac)  
        {
            _M_facility() = fac;
            _M_priority   = 0;
            return *this;
        }

        syslog &
        setlevel(int lev)  
        {
            _M_level()  = lev;
            _M_priority = 0;
            return *this;
        }

        syslog &
        setlogmask(int mask)  
        {
            ::setlogmask(mask);
            return *this;
        }

        const int 
        facility() const  
        { return const_cast<syslog *>(this)->_M_facility(); }

        const int 
        level() const  
        { return const_cast<syslog *>(this)->_M_level(); }

        const int 
        priority() const  
        { return _M_priority ? : const_cast<syslog *>(this)->_M_facility() | 
            const_cast<syslog *>(this)->_M_level(); }

    private:
        static const int SIZE = 1024;
        char _M_buffer[SIZE];

        int  _M_priority;
        int  _M_cursor;
        int  _M_option;

        // central output functions...
        //

        virtual std::streamsize
        xsputn (const char *s, std::streamsize n)
        {
            int b(n);
            if ( _M_cursor+b > SIZE-1 ) {
                std::clog << "syslog: message overflows streambuf!\n" << std::endl;
                b = SIZE - 1 - _M_cursor;
            }
            if (b > 0) {
                std::copy(s, s + b, _M_buffer + _M_cursor);
                _M_cursor += b;
            }
            return n; 
        }

        virtual int_type
        overflow (int_type c)
        {
            if (_M_cursor < SIZE) {
                _M_buffer[_M_cursor] = '\0';
                _M_cursor = 0;
                ::syslog(priority(), _M_buffer);
            }
            else {
                 _M_cursor = 0;
            }

            return c;
        }

        int &_M_facility() {
            static int fac;
            return fac; 
        }

        int &_M_level() {
            static int lev;
            return lev; 
        }

        syslog (const syslog &);                    // noncopyable
        syslog & operator= (const syslog &);        // noncopyable

    };

    static inline
    syslog * ctrl(std::ostream &out)
    {
        syslog *ret = dynamic_cast<syslog *>(out.rdbuf());
        if ( ret == NULL )
            throw std::runtime_error("sys::ctrl(): bad ostream");
        return ret;
    }

    extern std::ostream log;
    extern std::ostream plog;
    extern std::ostream err;
    extern std::ostream perr;
}

#endif /* SYSLOG_HH */
