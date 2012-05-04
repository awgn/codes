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
#include <vector>
#include <string>

#include <type_traits.hpp> // !more

#include <yats.hpp>
using namespace yats;


template <typename T>
struct false_container 
{
    typedef T value_type;
};

struct s {};

Context(more_typetraits_test)
{
    Test(is_class_or_union)
    {
        Assert(more::traits::is_class_or_union<int>::value , is_false());
        Assert(more::traits::is_class_or_union<s>::value , is_true());
    }

    Test(is_a_container)
    {
        Assert(more::traits::is_container<int>::value , is_false() );
        Assert(more::traits::is_container<false_container<int> >::value , is_false());
        Assert(more::traits::is_container<std::vector<int> >::value , is_true());
    }

    Test(has_value_type)
    {
        Assert(more::traits::has_value_type<false_container<int> >::value , is_true());
    }

    Test(has_iterator)
    {
        Assert(more::traits::has_iterator<false_container<int> >::value , is_false());
    }

    typedef std::tuple<long long int, long int, short int> mytuple;
    
    Test(is_a_tuple)
    {
        Assert( more::traits::is_tuple< int >::value , is_false());
        Assert( more::traits::is_tuple< mytuple >::value , is_true());
    }

    Test(is_a_pair)
    {
        Assert((more::traits::is_pair< std::pair<int,int> >::value) , is_true());
        Assert( more::traits::is_pair< int >::value , is_false() );
    }

}

int
main(int argc, char *argv[])
{
    return yats::run();
}
 
