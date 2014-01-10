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
        : m_priority(-1), m_default_priority(EMERG), m_loglevel(-1),  m_out(out)
        {}

        void loglevel(int n)
        { m_loglevel = n; }

        static int iword_index()
        { static int index = std::ios_base::xalloc();
            return index; }

    protected:

        virtual std::streamsize
        xsputn (const char *s, std::streamsize n)
        {
            if ( prio() <= m_loglevel) {
                return m_out->sputn(s,n);
            }
            return n; 
        }

        virtual int_type
        overflow (int_type c)
        {
            int p = prio();
            if (c == '\n')
                m_priority = -1;

            if ( p <= m_loglevel)  
               return m_out->sputc(static_cast<char_type>(c));
             
            return c;
        }

        int sync()
        {
            if (prio() <= m_loglevel) 
                return m_out->pubsync();
            return 0; 
        }

        void priority(int n)
        { m_priority = n; }

        void default_priority(int n)
        { m_default_priority = n; }

    private:        
        
        int prio() const 
        { return m_priority != -1 ? m_priority : m_default_priority; }

        int m_priority;
        int m_default_priority;

        int m_loglevel;

        std::streambuf *m_out;
    };

} // namespace more

#endif /* _LOGSTREAMBUF_HPP_ */
