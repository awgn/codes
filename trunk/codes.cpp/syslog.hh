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
#include <cstdio>
#include <stdexcept>

#include <syslog.h>

namespace more 
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
        : m_buffer(),
          m_priority(0),
          m_cursor(0),
          m_option(opt)
        {
            m_facility() = fac;
            m_level()    = lev;
        } 

        ~syslog()  
        {}

        syslog &
        open(const char *ident)
        {
            // std::cout << __PRETTY_FUNCTION__ << std::endl;
            if (ident)
                ::openlog(ident,m_option,m_facility());
            return *this;
        }

        syslog &
        setfacility(int fac)  
        {
            m_facility() = fac;
            m_priority   = 0;
            return *this;
        }

        syslog &
        setlevel(int lev)  
        {
            m_level()  = lev;
            m_priority = 0;
            return *this;
        }

        syslog &
        setlogmask(int mask)  
        {
            ::setlogmask(mask);
            return *this;
        }

        int 
        facility() const  
        { return const_cast<syslog *>(this)->m_facility(); }

        int 
        level() const  
        { return const_cast<syslog *>(this)->m_level(); }

        int 
        priority() const  
        { return m_priority ? : const_cast<syslog *>(this)->m_facility() | 
            const_cast<syslog *>(this)->m_level(); 
        }

        static inline
        syslog * ctrl(std::ostream &out)
        {
            syslog *ret = dynamic_cast<syslog *>(out.rdbuf());
            if ( ret == NULL )
                throw std::runtime_error("syslog::ctrl(): bad ostream");
            return ret;
        }

    private:
        static const int SIZE = 1024;
        char m_buffer[SIZE];

        int  m_priority;
        int  m_cursor;
        int  m_option;

        // central output functions...
        //

        virtual std::streamsize
        xsputn (const char *s, std::streamsize n)
        {
            int b(n);
            if ( m_cursor+b > SIZE-1 ) {
                fprintf(stderr, "syslog: message overflows in streambuf!\n");
                b = SIZE - 1 - m_cursor;
            }
            if (b > 0) {
                std::copy(s, s + b, m_buffer + m_cursor);
                m_cursor += b;
            }
            return n; 
        }

        virtual int_type
        overflow (int_type c)
        {
            if (m_cursor < SIZE) {
                m_buffer[m_cursor] = '\0';
                m_cursor = 0;
                ::syslog(priority(), "%s", m_buffer);
            }
            else {
                 m_cursor = 0;
            }

            return c;
        }

        int &m_facility() {
            static int fac;
            return fac; 
        }

        int &m_level() {
            static int lev;
            return lev; 
        }

        syslog (const syslog &);                    // noncopyable
        syslog & operator= (const syslog &);        // noncopyable

    };

}

namespace sys 
{
    extern std::ostream log;
    extern std::ostream plog;
    extern std::ostream err;
    extern std::ostream perr;
}

#endif /* SYSLOG_HH */
