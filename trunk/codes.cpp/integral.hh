/* $Id$ */
/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <bonelli@antifork.org> wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return. Nicola Bonelli 
 * ----------------------------------------------------------------------------
 */

#ifndef _INTEGRAL_HH_
#define _INTEGRAL_HH_ 

namespace more { 

    // TR1 helper classes [4.3]

    template <typename Tp, Tp _v>
    struct integral_constant
    {
        typedef Tp  value_type;
        typedef integral_constant<Tp,_v> type;

        static const Tp     value = _v;
    }; 

    typedef integral_constant<bool, true > true_type;
    typedef integral_constant<bool, false> false_type;

    // library traits:

    struct std_type {}; 
    struct qt_type {};

    struct tr1_type   : public std_type {};
    struct boost_type : public std_type {};

}

#endif /* _INTEGRAL_HH_ */
