/* $Id*/
/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <bonelli@antifork.org> wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return. Nicola Bonelli 
 * ----------------------------------------------------------------------------
 */

#include <tr1/functional>
#include <iostream>
#include <colorful.hh>

using namespace more;

typedef colorful< TYPELIST(ecma::bold, ecma::fg_red) > red;
typedef colorful< TYPELIST(ecma::bold, ecma::fg_blue) > blue;
typedef colorful< TYPELIST(ecma::reset) > rst;
typedef colorful< TYPELIST(ecma::bold) > bold;

struct arg 
{
    arg(int n = 0)
    : _M_value(n)
    { std::cout << "    " << __PRETTY_FUNCTION__ << std::endl; }

    ~arg()
    { std::cout << "    " << __PRETTY_FUNCTION__ << std::endl; }
    
    arg(const arg &rhs)
    : _M_value(rhs._M_value)
    { 
        std::cout << "    " << __PRETTY_FUNCTION__ << std::endl; 
    }

    arg &operator=(const arg &rhs)
    { 
        _M_value = rhs._M_value;
        std::cout << "    " << __PRETTY_FUNCTION__ << std::endl; 
    }
    
    int get() const
    { return _M_value; }

    int _M_value;
};


/////////////////////////////////////////////////////////////
// the target function:

int fun(arg a1, const arg & a2, arg & a3)
{
    a3 = arg(1);       
    std::cout << "fun: a1 = " << a1.get() << std::endl;
    std::cout << "fun: a2 = " << a2.get() << std::endl;
    std::cout << "fun: a3 = " << a3.get() << std::endl;
    return 0;
}

/////////////////////////////////////////////////////////////
// candidates:

template <typename F, typename T1, typename T2, typename T3>
int candidate_forwarder_1(F &f, T1 &t1, T2 &t2, T3 &t3)
{
    return f(t1,t2,t3);
}   

template <typename F, typename T1, typename T2, typename T3>
int candidate_forwarder_2(F f, T1 t1, T2 t2, T3 t3)
{
    return f(t1,t2,t3);
} 

#ifdef __GXX_EXPERIMENTAL_CXX0X__
template <typename F, typename T1, typename T2, typename T3>
int the_perfect_forwarder(F f, T1 && t1, T2 && t2, T3 && t3)
{
    return f(std::forward<T1>(t1),
             std::forward<T2>(t2),
             std::forward<T2>(t3));
} 
#endif

/////////////////////////////////////////////////////////////
// test case:

int
main(int argc, char *argv[])
{    
    
    std::cout << red() << "\n[#0] direct call" << rst() << std::endl;
    {
        arg a1;
        arg a2;
        arg a3;

        std::cout << "--- prolog:" << std::endl;
        fun(a1, a2, a3);  // rvalue are not allowed
        std::cout << "--- epilog:" << std::endl;

        std::cout << "    arg & a3 = " << a3.get() << " <== should be 1! [" << bold() << "OK" << rst() << "]" << std::endl;
    }


    std::cout << red() << "\n[#1] candidate_forwarder_1" << rst() << std::endl;
    {
        arg a1;
        arg a2;
        arg a3;

        std::cout << "--- prolog:" << std::endl;
        candidate_forwarder_1(fun, a1, a2, a3);  // rvalue are not allowed
        std::cout << "--- epilog:" << std::endl;

        std::cout << "    arg & a3 = " << a3.get() << " <== should be 1! [" << bold() << "OK" << rst() << "]" << std::endl;
    }

    std::cout << red() << "\n[#2] candidate_forwarder_2" << rst() << std::endl;
    {
        arg a3;

        std::cout << "--- prolog:" << std::endl;
        candidate_forwarder_2(fun, arg(), arg(), a3 );  // rvalue are not allowed
        std::cout << "--- epilog:" << std::endl;

        std::cout << "    arg & a3 = " << a3.get() << " <== should be 1! [" << red() << "ERROR" << rst() << "]" << std::endl;
    }

    std::cout << red() << "\n[#3] candidate_forwarder_2 [with std::tr1::reference_wrapper<>]" << rst() << std::endl;
    {
        arg a3;

        std::cout << "--- prolog:" << std::endl;
        candidate_forwarder_2(fun, arg(), arg(), std::tr1::ref(a3) );  // rvalue are not allowed
        std::cout << "--- epilog:" << std::endl;

        std::cout << "    arg & a3 = " << a3.get() << " <== should be 1! [" << bold() << "OK" << rst() << "]" << std::endl;
    }

    std::cout << red() << "\n[#4] candidate_forwarder_2 [with std::tr1::reference_wrapper<>]" << rst() << std::endl;
    {
        arg a1;
        arg a2;
        arg a3;

        std::cout << "--- prolog:" << std::endl;
        candidate_forwarder_2(fun, a1, a2, std::tr1::ref(a3) );  // rvalue are not allowed
        std::cout << "--- epilog:" << std::endl;

        std::cout << "    arg & a3 = " << a3.get() << " <== should be 1! [" << bold() << "OK" << rst() << "]" << std::endl;
    }

#ifdef __GXX_EXPERIMENTAL_CXX0X__    
    std::cout << blue() << "\n[#5] the_perfect_forwarder [c++0x right reference]" << rst() << std::endl;
    {
        arg a1;
        arg a2;
        arg a3;

        std::cout << "--- prolog:" << std::endl;
        the_perfect_forwarder(fun, a1, a2, a3 );  // rvalue are not allowed
        std::cout << "--- epilog:" << std::endl;

        std::cout << "    arg & a3 = " << a3.get() << " <== should be 1! [" << bold() << "OK" << rst() << "]" << std::endl;
    }

#endif
    return 0;
}

