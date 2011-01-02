/* $Id$ */
/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <bonelli@antifork.org> wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return. Nicola Bonelli
 * ----------------------------------------------------------------------------
 */

#ifndef _LOGSTREAMBUF_HPP_
#define _LOGSTREAMBUF_HPP_ 

#include <iostream>

namespace more { 

    class logstreambuf : public std::streambuf
    {
    public:

        enum { EMERG, ALERT, CRIT, ERR, 
               WARNING, NOTICE, INFO, DEBUG };

        friend std::ostream &priority(std::ostream &out, int n = EMERG)
        {
            logstreambuf * lb = dynamic_cast<logstreambuf *>(out.rdbuf());
            if(lb) 
                lb->priority(n);
            return out;
        }

        friend std::ostream &default_priority(std::ostream &out, int n = EMERG)
        {
            logstreambuf * lb = dynamic_cast<logstreambuf *>(out.rdbuf());
            if(lb) 
                lb->default_priority(n);
            return out;
        }

        logstreambuf(std::streambuf *out)
        : _M_priority(-1), _M_default_priority(EMERG), _M_loglevel(-1),  _M_out(out)
        {}

        void loglevel(int n)
        { _M_loglevel = n; }

        static int iword_index()
        { static int index = std::ios_base::xalloc();
            return index; }

    protected:

        virtual std::streamsize
        xsputn (const char *s, std::streamsize n)
        {
            if ( prio() <= _M_loglevel) {
                return _M_out->sputn(s,n);
            }
            return n; 
        }

        virtual int_type
        overflow (int_type c)
        {
            int p = prio();
            if (c == '\n')
                _M_priority = -1;

            if ( p <= _M_loglevel)  
               return _M_out->sputc(c);
             
            return c;
        }

        int sync()
        {
            if (prio() <= _M_loglevel) 
                return _M_out->pubsync();
            return 0; 
        }

        void priority(int n)
        { _M_priority = n; }

        void default_priority(int n)
        { _M_default_priority = n; }

    private:        
        
        int prio() const 
        { return _M_priority != -1 ? _M_priority : _M_default_priority; }

        int _M_priority;
        int _M_default_priority;

        int _M_loglevel;

        std::streambuf *_M_out;
    };

    std::ostream &priority(std::ostream &out, int n);
    std::ostream &default_priority(std::ostream &out, int n);

} // namespace more

#endif /* _LOGSTREAMBUF_HPP_ */
