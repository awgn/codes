/* $Id$ */
/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <bonelli@antifork.org> wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return. Nicola Bonelli
 * ----------------------------------------------------------------------------
 */


#ifndef _NULLSTREAMBUF_HH_
#define _NULLSTREAMBUF_HH_ 

#include <iostream>

namespace more { 
    
    class nullstreambuf : public std::streambuf
    {
    protected:

        std::streamsize
        xsputn(const char *s, std::streamsize n)
        {
            return n;
        }

        int_type
        overflow(int_type c)
        {
            return c;
        }

        int sync()
        {
            return 0;
        }
    };

} // namespace more

#endif /* _NULLSTREAMBUF_HH_ */
