/* $Id$ */
/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <bonelli@antifork.org> wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return. Nicola Bonelli
 * ----------------------------------------------------------------------------
 */

#include <iostream>
#include <exprtempl.hh>

using namespace more::expr;

//////////////////// example ///////////////////////

// example: _false and _true integral type.
//

namespace {

    static integral<int, 0> _0 __attribute__((unused));
    static integral<int, 1> _1 __attribute__((unused));
    static integral<int, 2> _2 __attribute__((unused));
    static integral<int, 3> _3 __attribute__((unused));
    static integral<int, 4> _4 __attribute__((unused));
    static integral<int, 5> _5 __attribute__((unused));
    static integral<int, 6> _6 __attribute__((unused));
    static integral<int, 7> _7 __attribute__((unused));
    static integral<int, 8> _8 __attribute__((unused));

}

int
main(int argc, char *argv[])
{
    std::cout << std::boolalpha;
    std::cout << (( !_false ^ _true ) == _false) << " -> " << eval( ( !_false ^ _true ) == _false ) << std::endl;

    std::cout << (_false | _true) << " -> " << eval(_false | _true) << std::endl;

    std::cout << (~_false) << " -> " << eval(~_false) << std::endl;

    std::cout << ( (_1 + _2) * _2 == ((_8 / _2) + _2) ) << " -> " << eval( (_1 + _2) * _2 == ((_8 / _2) + _2) ) << std::endl;

    std::cout << ( _1 < _2 ) << " -> " << eval( _1 < _2 ) << std::endl;
    std::cout << ( _1 >= _2 ) << " -> " << eval( _1 >= _2 ) << std::endl;

    return 0;
}

