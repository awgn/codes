/* $Id$ */
/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <bonelli@antifork.org> wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return. Nicola Bonelli
 * ----------------------------------------------------------------------------
 */

#ifndef _LNISTREAMBUF_HH_
#define _LNISTREAMBUF_HH_ 

#include <iostream>

namespace more { 

    class lnistreambuf : public std::streambuf {

        std::streambuf * _M_in;
        int _M_line;

    public:
        lnistreambuf(std::streambuf *in)
        : _M_in(in),
          _M_line(1)
        {}

        virtual int_type underflow()
        {
            return  _M_in->sgetc();
        }

        virtual int_type uflow()
        {
            int_type c = _M_in->sbumpc();
            if ( c == '\n' )
                _M_line++;
            return c;
        }

        int line() const
        { return _M_line; }
    };


    template <class charT, class Traits>
    inline int lineno(std::basic_istream<charT,Traits> &in)
    {
        lnistreambuf * ln = dynamic_cast<lnistreambuf *>(in.rdbuf());
        if (ln) {
            return ln->line(); 
        }   
        return -1;
    }

} // namespace more


#endif /* _LNISTREAMBUF_HH_ */
