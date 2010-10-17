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
#include <cassert>

#include <typelist.hpp>
#include <cxa_demangle.hpp>

using namespace more::type;

// unary metafunction example:
//

template <typename Tp>
struct metafunction
{
    typedef int type;
};


int main()
{
    typedef typelist<>          list0;
    typedef typelist<int>       list1;
    typedef typelist<int,char>  list2;

    assert( length<list0>::value == 0 );
    assert( length<list1>::value == 1 );
    assert( length<list2>::value == 2 );

    typedef append<list2, short>::type  list3;
    assert( length<list3>::value == 3 );

    typedef insert<list3, char>::type list4;
    assert( length<list4>::value == 4 );

    assert( (indexof<list0, int>::value == -1) );
    assert( (indexof<list1, int>::value ==  0) );
    assert( (indexof<list1, char>::value == -1) );

    assert( (is_same<typelist<>,      typelist<>>::value == true) );
    assert( (is_same<typelist<list0>, typelist<list0>>::value == true) );
    assert( (is_same<typelist<list1>, typelist<list1>>::value == true) );
    assert( (is_same<typelist<list2>, typelist<list2>>::value == true) );
    assert( (is_same<typelist<list3>, typelist<list3>>::value == true) );

    // or std::is_same is good enough...

    assert( (std::is_same<typelist<list3>, typelist<list3>>::value == true) );
    
    
    std::cout << "{*} at...\n";
    std::cout << "    " << 0 << ":" << more::cxa_demangle(typeid(at<list3,0>::type).name()) << std::endl;
    std::cout << "    " << 1 << ":" << more::cxa_demangle(typeid(at<list3,1>::type).name()) << std::endl;
    std::cout << "    " << 2 << ":" << more::cxa_demangle(typeid(at<list3,2>::type).name()) << std::endl;


    typedef for_each<list3, metafunction >::type list3_;

    std::cout << "{*} for_each...\n";
    std::cout << "    " << 0 << ":" << more::cxa_demangle(typeid(at<list3_,0>::type).name()) << std::endl;
    std::cout << "    " << 1 << ":" << more::cxa_demangle(typeid(at<list3_,1>::type).name()) << std::endl;
    std::cout << "    " << 2 << ":" << more::cxa_demangle(typeid(at<list3_,2>::type).name()) << std::endl;
    
}
