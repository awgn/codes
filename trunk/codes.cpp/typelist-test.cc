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

#include <typelist.hh>
#include <cxa_demangle.hh>

using namespace more;

// example of unary function...
//

namespace example 
{
    template <int n>
    struct integral
    {
        enum { value = n };
    };

    // unary function..
    //
    template <typename T>
    struct sizeof_
    {
        typedef integral<sizeof(T)> type;
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
        typedef integral<sizeof(T1) == sizeof(T2)> type;
    };

    struct same_size_f 
    {
        template <typename T1, typename T2>
        struct apply : same_size<T1,T2> {}; 
    };

}

int main(int, char *[])
{
    typedef TYPELIST(int) list0;
    typedef TL::append<list0, bool>::type list1;
    typedef TL::append<list1, std::string>::type list2;

    std::cout << "{*} length...\n";
    std::cout << "    len:" << TL::length<list2>::value << std::endl;

    std::cout << "{*} at...\n";

    std::cout << "    " << 0 << ":" << more::cxa_demangle(typeid(TL::at<list2,0>::type).name()) << std::endl;
    std::cout << "    " << 1 << ":" << more::cxa_demangle(typeid(TL::at<list2,1>::type).name()) << std::endl;
    std::cout << "    " << 2 << ":" << more::cxa_demangle(typeid(TL::at<list2,2>::type).name()) << std::endl;


    // append...
    typedef TL::null a_list;

    std::cout << "{*} append...\n";

    typedef TL::append<list2, short>::type list3;

    std::cout << "    " << 0 << ":" << more::cxa_demangle(typeid(TL::at<list3,0>::type).name()) << std::endl;
    std::cout << "    " << 1 << ":" << more::cxa_demangle(typeid(TL::at<list3,1>::type).name()) << std::endl;
    std::cout << "    " << 2 << ":" << more::cxa_demangle(typeid(TL::at<list3,2>::type).name()) << std::endl;
    std::cout << "    " << 3 << ":" << more::cxa_demangle(typeid(TL::at<list3,3>::type).name()) << std::endl;

    std::cout << "{*} insert...\n";
    typedef TL::insert<list3, double>::type list4;

    std::cout << "    " << 0 << ":" << more::cxa_demangle(typeid(TL::at<list4,0>::type).name()) << std::endl;
    std::cout << "    " << 1 << ":" << more::cxa_demangle(typeid(TL::at<list4,1>::type).name()) << std::endl;
    std::cout << "    " << 2 << ":" << more::cxa_demangle(typeid(TL::at<list4,2>::type).name()) << std::endl;
    std::cout << "    " << 3 << ":" << more::cxa_demangle(typeid(TL::at<list4,3>::type).name()) << std::endl;
    std::cout << "    " << 4 << ":" << more::cxa_demangle(typeid(TL::at<list4,4>::type).name()) << std::endl;

    // index_of
    std::cout << "{*} index_of...\n";
    std::cout << "    double:" << TL::index_of<list4, double>::value << std::endl;
    std::cout << "    string:" << TL::index_of<list4, std::string>::value << std::endl;
    std::cout << "    int   :" << TL::index_of<list4, int>::value << std::endl;

    typedef TL::apply1<list4, example::sizeof_f>::type size_list;

    std::cout << "{*} apply1...\n";
    std::cout << "    [0] sizeof= " << TL::at<size_list,0>::type::value << std::endl;
    std::cout << "    [1] sizeof= " << TL::at<size_list,1>::type::value << std::endl;
    std::cout << "    [2] sizeof= " << TL::at<size_list,2>::type::value << std::endl;
    std::cout << "    [3] sizeof= " << TL::at<size_list,3>::type::value << std::endl;
    std::cout << "    [4] sizeof= " << TL::at<size_list,4>::type::value << std::endl;

    // is_same...
    std::cout << "{*} is_same...\n";
    std::cout << "    is_same = " << TL::is_same<TL::null, TL::null>::value << std::endl;

    // tranform...
    typedef TL::transform<list4, list4, example::same_size_f>::type samesize_list;

    std::cout << "{*} transform...\n";
    std::cout << "    [0] value= " << TL::at<samesize_list,0>::type::value << std::endl;
    std::cout << "    [1] value= " << TL::at<samesize_list,1>::type::value << std::endl;
    std::cout << "    [2] value= " << TL::at<samesize_list,2>::type::value << std::endl;
    std::cout << "    [3] value= " << TL::at<samesize_list,3>::type::value << std::endl;
    std::cout << "    [4] value= " << TL::at<samesize_list,4>::type::value << std::endl;
 
}
