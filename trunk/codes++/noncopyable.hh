/* $Id$ */
/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <bonelli@antifork.org> wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return. Nicola Bonelli
 * ----------------------------------------------------------------------------
 */

#ifndef __NONCOPYABLE_HH__
#define __NONCOPYABLE_HH__

// yet another tribute to boost library: noncopyable tool.
//

namespace more {

    class noncopyable
    {
    protected:
        noncopyable()
        {}

        ~noncopyable()
        {}

    private:
        noncopyable(const noncopyable &);
        const noncopyable & operator=(const noncopyable &);
    };

} // namespace more

#endif /* __NONCOPYABLE_HH__ */
