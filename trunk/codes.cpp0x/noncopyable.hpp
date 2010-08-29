/* $Id$ */
/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <bonelli@antifork.org> wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return. Nicola Bonelli
 * ----------------------------------------------------------------------------
 */

#ifndef __MORE_NONCOPYABLE_HPP__
#define __MORE_NONCOPYABLE_HPP__

// yet another tribute to boost library: noncopyable tool.
//

namespace more {

    class noncopyable
    {
    protected:
        noncopyable() = default;
        ~noncopyable() = default;

    private:
        noncopyable(const noncopyable &) = deleted;
        const noncopyable & operator=(const noncopyable &) = deleted;
    };

} // namespace more

#endif /* __MORE_NONCOPYABLE_HPP__ */
