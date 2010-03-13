/* $Id$ */
/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <bonelli@antifork.org> wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return. Nicola Bonelli
 * ----------------------------------------------------------------------------
 */

#ifndef OSTREAM_DIVERTER_H
#define OSTREAM_DIVERTER_H

#include <iostream>

namespace more {

    // ostream diverter class -  
    //

    template <typename T>
    struct osd
    {
        static std::ostream cout;
        static std::ostream cerr;
        static std::ostream clog;

        static inline void
        cout_divert(std::ostream &o)
        {
            std::streambuf * r = o.rdbuf();
            cout.rdbuf(r);
        }
        static inline void
        cerr_divert(std::ostream &o)
        {
            std::streambuf * r = o.rdbuf();
            cerr.rdbuf(r);
        }
        static inline void
        clog_divert(std::ostream &o)
        {
            std::streambuf * r = o.rdbuf();
            clog.rdbuf(r);
        }
    };

    template <typename T>
    std::ostream osd<T>::cout(std::cout.rdbuf());

    template <typename T>
    std::ostream osd<T>::cerr(std::cerr.rdbuf());

    template <typename T>
    std::ostream osd<T>::clog(std::clog.rdbuf());

} // namespace more


#endif /* OSTREAM_DIVERTER_H */
