/* $Id$ */
/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <bonelli@antifork.org> wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return. Nicola Bonelli 
 * ----------------------------------------------------------------------------
 */

#include "typelist.hh"
#include <iostream>

using namespace mtp;

// example of unary function...
//

namespace example 
{
    template <int n>
    struct int_
    {
        enum { value = n };
    };

    // unary function..
    //
    template <typename T>
    struct sizeof_
    {
        typedef int_<sizeof(T)> type;
    };

    struct sizeof_f
    {
        template <typename T>
        struct apply : sizeof_<T> {};
    };

    // binary function..
    //
    template <typename T1, typename T2>
    struct same_size
    {
        typedef int_<sizeof(T1) == sizeof(T2)> type;
    };

    struct same_size_f 
    {
        template <typename T1, typename T2>
        struct apply : same_size<T1,T2> {}; 
    };

}

int main()
{
    typedef TYPELIST(int) z_list; 
    std::cout << "{*} length...\n";
    std::cout << "    len:" << TL::length<z_list>::value << std::endl;

    std::cout << "{*} at...\n";
    TL::at<z_list,0>::type x1 = 1;
    std::cout << "    int:" << x1 << std::endl;

    // append...
    typedef TL::null a_list;

    std::cout << "{*} append...\n";
    typedef TL::append<a_list, bool>::type b_list;

    TL::at<b_list,0>::type x2 = true;
    std::cout << "    bool:" << x2 << std::endl;

    typedef TL::append<b_list,int>::type c_list;
    TL::at<c_list,1>::type x3 = 2;
    std::cout << "    int:" << x3 << std::endl;

    typedef TL::append<c_list, c_list>::type d_list;
    std::cout << "    len:" << TL::length<d_list>::value << std::endl;

    typedef TYPELIST(TL::null) e_list;
    std::cout << "{*} insert...\n";
    typedef TL::insert<e_list, int>::type f_list;

    TL::at<f_list,0>::type x4 = 1;
    std::cout << "    int:" << x4 << std::endl;

    typedef TL::insert<f_list, std::string>::type g_list;
    TL::at<g_list, 0>::type h ("hello world");

    std::cout << "    string:" << h << std::endl;

    // indexof
    std::cout << "{*} indexof...\n";
    std::cout << "    double:" << TL::indexof<g_list, double>::value << std::endl;
    std::cout << "    string:" << TL::indexof<g_list, std::string>::value << std::endl;
    std::cout << "    int   :" << TL::indexof<g_list, int>::value << std::endl;

    typedef TL::apply1<g_list, example::sizeof_f>::type h_list;

    std::cout << "{*} apply1...\n";
    std::cout << "    [0] sizeof= " << TL::at<h_list,0>::type::value << std::endl;
    std::cout << "    [1] sizeof= " << TL::at<h_list,1>::type::value << std::endl;
    std::cout << "    [2] sizeof= " << TL::at<h_list,2>::type::value << std::endl;

    // is_same...
    std::cout << "{*} is_same...\n";
    std::cout << "    is_same = " << TL::is_same<TL::null, TL::null>::value << std::endl;

    // tranform...
    typedef TL::transform<h_list,h_list, example::same_size_f>::type i_list;

    std::cout << "{*} transform...\n";
    std::cout << "    [0] value= " << TL::at<i_list,0>::type::value << std::endl;
    std::cout << "    [1] value= " << TL::at<i_list,1>::type::value << std::endl;
    std::cout << "    [2] value= " << TL::at<i_list,2>::type::value << std::endl;
 
}
