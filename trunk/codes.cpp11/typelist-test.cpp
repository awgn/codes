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
#include <cxxabi.hpp>
#include <yats.hpp>

using namespace yats;
using namespace more::type;

// unary metafunction example:
//

template <typename Tp>
struct metafunction
{
    typedef int type;
};

Context(more_typelist_test)
{
    typedef typelist<>          list0;
    typedef typelist<int>       list1;
    typedef typelist<int,char>  list2;

    Test(length)
    {
        Assert( static_cast<int>(length<list0>::value), is_equal_to(0) );
        Assert( static_cast<int>(length<list1>::value), is_equal_to(1) );
        Assert( static_cast<int>(length<list2>::value), is_equal_to(2) );
    }

    typedef append<list2, short>::type  list3;
    
    Test(append)
    {
        Assert( static_cast<int>(length<list3>::value), is_equal_to(3) );
    }

    typedef insert<list3, char>::type list4;
    Test(insert)
    {
        Assert( static_cast<int>(length<list4>::value), is_equal_to(4) );
    }
    
    Test(index_of)
    {
        Assert( static_cast<int>(indexof<list0, int>::value) , is_equal_to(-1) );
        Assert( static_cast<int>(indexof<list1, int>::value) , is_equal_to(0)  );
        Assert( static_cast<int>(indexof<list1, char>::value), is_equal_to(-1) );
    }

    Test(is_same)
    {
        Assert( static_cast<int>(is_same<typelist<>,      typelist<>>::value)      , is_true() );
        Assert( static_cast<int>(is_same<typelist<list0>, typelist<list0>>::value) , is_true() );
        Assert( static_cast<int>(is_same<typelist<list1>, typelist<list1>>::value) , is_true() );
        Assert( static_cast<int>(is_same<typelist<list2>, typelist<list2>>::value) , is_true() );
        Assert( static_cast<int>(is_same<typelist<list3>, typelist<list3>>::value) , is_true() );

        // or std::is_same is good enough...
        Assert( (std::is_same<typelist<list3>, typelist<list3>>::value), is_true() );
    }

    Test(output)
    { 
        std::cout << "{*} at...\n";
        std::cout << "    " << 0 << ":" << demangle(typeid(at<list3,0>::type).name()) << std::endl;
        std::cout << "    " << 1 << ":" << demangle(typeid(at<list3,1>::type).name()) << std::endl;
        std::cout << "    " << 2 << ":" << demangle(typeid(at<list3,2>::type).name()) << std::endl;
    }

    typedef for_each<list3, metafunction >::type list3_;

    Test(for_each)
    {
        std::cout << "{*} for_each...\n";
        std::cout << "    " << 0 << ":" << demangle(typeid(at<list3_,0>::type).name()) << std::endl;
        std::cout << "    " << 1 << ":" << demangle(typeid(at<list3_,1>::type).name()) << std::endl;
        std::cout << "    " << 2 << ":" << demangle(typeid(at<list3_,2>::type).name()) << std::endl;
    }    
}

int
main(int argc, char *argv[])
{
    return yats::run(argc, argv);
}
