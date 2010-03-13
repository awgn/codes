/* $Id$ */
/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <bonelli@antifork.org> wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return. Nicola Bonelli
 * ----------------------------------------------------------------------------
 */

#ifndef _DEQUE_UTILS_HH_
#define _DEQUE_UTILS_HH_ 

#include <sys/uio.h>

#include <deque>
#include <vector>

namespace more { 

    template <typename T>
    std::vector<iovec>
    deque_iovec(const std::deque<T> &cont)
    {
        std::vector<iovec> ret;

        typename std::deque<T>::const_iterator it = cont.begin();
        typename std::deque<T>::const_iterator it_end = cont.end();

        if ( it == it_end )
            return std::vector<iovec>();

        const T * base = & *it; size_t len = 0;

        for(const T * p = base; it != it_end; ++it, ++p, len += sizeof(T) )
        {
            if ( p != & *it)
            {
                iovec iov = { static_cast<void *>(const_cast<T *>(base)), len };
                ret.push_back(iov);
                base = p = & *it;
                len  = 0;
            }
        }
       
        iovec iov = { static_cast<void *>(const_cast<T *>(base)), len };
        ret.push_back(iov);
        return ret;
    }    

} // nnamespace more

#endif /* _DEQUE_UTILS_HH_ */
