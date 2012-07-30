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
#include <exprtempl.hpp>

#include <yats.hpp>
using namespace yats;
using namespace more::expr;

//////////////////// example ///////////////////////

// example: _false and _true integral type.
//

namespace 
{
    expr_integral<int, 0> __0 = expr_integral<int, 0>();
    expr_integral<int, 1> __1 = expr_integral<int, 1>();
    expr_integral<int, 2> __2 = expr_integral<int, 2>();
    expr_integral<int, 3> __3 = expr_integral<int, 3>();
    expr_integral<int, 4> __4 = expr_integral<int, 4>();
    expr_integral<int, 5> __5 = expr_integral<int, 5>();
    expr_integral<int, 6> __6 = expr_integral<int, 6>();
    expr_integral<int, 7> __7 = expr_integral<int, 7>();
    expr_integral<int, 8> __8 = expr_integral<int, 8>();
}                           

Context(expression_template_tests)
{
    
    Test(boolean)
    {
        Assert( eval( ( !_false ^ _true ) == _false ), is_true());
        Assert( eval( _false || _true ), is_true());
        Assert( eval( _true  ^ _true ), is_false());
        Assert( eval( _false ^ _false), is_false());
        Assert( eval( ~_false ), is_true());
    }

    Test(integer)
    {
        Assert( eval( (__1 + __2) * __2 == ((__8 / __2) + __2) ), is_true());
        Assert( eval( __1 < __2 ), is_true());
        Assert( eval( __1 >= __2 ), is_false());
    }
}

int
main(int argc, char *argv[])
{
    return yats::run(argc, argv);
}
