/* $Id$ */
/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <bonelli@antifork.org> wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return. Nicola Bonelli
 * ----------------------------------------------------------------------------
 */

#ifndef _LOGSTREAM_HH_
#define _LOGSTREAM_HH_ 

#include <iostream>

#ifdef __LOG_FUNCTION__
#undef __LOG_FUNCTION__
#endif
#define LOGSTR(n)           #n
#define XLOGSTR(n)          LOGSTR(n)
#ifdef NDEBUG
#define __LOG_FUNCTION__    __func__
#else
#define __LOG_FUNCTION__    (__FILE__ "|" XLOGSTR(__LINE__) "|") << __func__
#endif

namespace more { 

    class logstreambuf : public std::streambuf
    {
    public:

        enum { EMERG, ALERT, CRIT, ERR, 
               WARNING, NOTICE, INFO, DEBUG };

        logstreambuf(std::streambuf *out)
        : _M_priority(EMERG), _M_leveldefault(EMERG), _M_level(-1),  _M_out(out)
        {}

        void priority(int n)
        { _M_priority = n; }

        void loglevel_default(int n)
        { _M_leveldefault = n; }

        void loglevel(int n)
        { _M_level = n; }

        static int iword_index()
        { static int index = std::ios_base::xalloc();
            return index; }

    protected:

        virtual std::streamsize
        xsputn (const char *s, std::streamsize n)
        {
            if ( currentLevel() <= _M_priority) {
                return _M_out->sputn(s,n);
            }
            return n; 
        }

        virtual int_type
        overflow (int_type c)
        {
            if (currentLevel() <= _M_priority) {
                if (c == '\n')
                    _M_level = -1;
                return _M_out->sputc(c);
            }
            return c;
        }

        int sync()
        {
            if (currentLevel() <= _M_priority)
                return  _M_out->pubsync();
            return 0;
        }

    private:        
        
        int currentLevel() const 
        { return _M_level != -1 ? _M_level : _M_leveldefault; }

        int _M_priority;

        int _M_leveldefault;
        int _M_level;

        std::streambuf *_M_out;
    };

    static inline std::ostream &loglevel(std::ostream &out, int n = logstreambuf::EMERG)
    {
        logstreambuf * lb = dynamic_cast<logstreambuf *>(out.rdbuf());
        if(lb) 
            lb->loglevel(n);
        return out;
    }
    static inline std::ostream &loglevel_default(std::ostream &out, int n = logstreambuf::EMERG)
    {
        logstreambuf * lb = dynamic_cast<logstreambuf *>(out.rdbuf());
        if(lb) 
            lb->loglevel_default(n);
        return out;
    }


} // namespace more

#endif /* _LOGSTREAM_HH_ */
