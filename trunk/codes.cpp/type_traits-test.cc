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
#include <cassert>

#include "type_traits.hh"

#define RED    "\e[0;31;1m"
#define BLACK  "\e[0;29;1m"
#define RESET  "\e[0m"

std::string verdict( std::false_type )
{ return "[" RED "false" RESET "]"; }

std::string verdict( std::true_type )
{ return "[" BLACK "true" RESET "]"; }

template <typename T>
struct identity 
{
    typedef T type;
};

template <int N>
struct num_identity 
{
    enum {value = N};
};

template <typename T>
struct false_container 
{
    typedef T value_type;
};

struct s {};

int
main(int argc, char *argv[])
{
    std::cout << "int is a class on union? " << verdict( more::traits::is_class_or_union<int>())  << std::endl; assert(more::traits::is_class_or_union<int>::value == false);
    std::cout << "struct is a class or union? " << verdict(more::traits::is_class_or_union<s>())  << std::endl; assert(more::traits::is_class_or_union<s>::value == true);

    std::cout << "int is a container ? " << verdict( more::traits::is_container<int>())  << std::endl; assert(more::traits::is_container<int>::value == false );
    std::cout << "false_container<int> is a container ? " << verdict( more::traits::is_container<false_container<int> >())  << std::endl; assert( more::traits::is_container<false_container<int> >::value == false);
    std::cout << "false_container<int> has value_type ? " << verdict( more::traits::has_value_type<false_container<int> >())  << std::endl; assert( more::traits::has_value_type<false_container<int> >::value == true);
    std::cout << "false_container<int> has interator ? " << verdict( more::traits::has_iterator<false_container<int> >())  << std::endl; assert(more::traits::has_iterator<false_container<int> >::value == false);

    std::cout << "std::vector<int> is a container ? " << verdict( more::traits::is_container< std::vector<int> >())  << std::endl; assert(more::traits::is_container<std::vector<int> >::value == true);

    typedef std::tuple<long long int, long int, short int> mytuple;

    std::cout << "int is a tuple ? " << verdict( more::traits::is_tuple< int >())  << std::endl; assert( more::traits::is_tuple< int >::value == false);
    std::cout << "mytuple is a tuple ? " << verdict( more::traits::is_tuple< mytuple >())  << std::endl; assert ( more::traits::is_tuple< mytuple >::value == true);
    std::cout << "int is a metafunction ? " << verdict( more::traits::is_metafunction< int >())  << std::endl; assert(more::traits::is_metafunction< int >::value == false);
    std::cout << "identity is a metafunction ? " << verdict( more::traits::is_metafunction< identity<int> >())  << std::endl; assert( more::traits::is_metafunction< identity<int> >::value == true);
    std::cout << "num_identity is a metafunction ? " << verdict( more::traits::is_metafunction< num_identity<0> >())  << std::endl; assert( more::traits::is_metafunction< num_identity<0> >::value == true);

    std::cout << "is_pair? " << verdict( more::traits::is_pair< std::pair<int,int> >()) << std::endl; assert((more::traits::is_pair< std::pair<int,int> >::value) == true);
    std::cout << "is_pair? " << verdict( more::traits::is_pair< int >()) << std::endl; assert( more::traits::is_pair< int >::value == false );
    return 0;
}
