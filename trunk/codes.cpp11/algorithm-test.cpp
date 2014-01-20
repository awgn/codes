/* $Id$ */
/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <bonelli@antifork.org> wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return. Nicola Bonelli
 * ----------------------------------------------------------------------------
 */


#include <algorithm.hpp>

#include <iostream>
#include <iterator>
#include <functional>


#include <yats.hpp>
using namespace yats;

Context(algorithm_test)
{
    std::string a("hello");
    std::string b("Hello");
    std::string c("helol");
    std::string d("helo");

    Test(levenshtein_distance)
    {
        Assert(more::levenshtein_distance(a.begin(), a.end(), b.begin(), b.end()), is_equal_to(1));
        Assert(more::levenshtein_distance(a.begin(), a.end(), d.begin(), d.end()), is_equal_to(1));
        Assert(more::levenshtein_distance(a,b), is_equal_to(1));
        Assert(more::levenshtein_distance(a,a), is_equal_to(0));
    }

    Test(levenshtein_distance_custom_predicate)
    {
        Assert(more::levenshtein_distance(a.begin(), a.end(), c.begin(), c.end(), std::not_equal_to<char>()), is_equal_to(2));
        Assert(more::levenshtein_distance(c.begin(), c.end(), a.begin(), a.end(), std::not_equal_to<char>()), is_equal_to(2));
        Assert(more::levenshtein_distance(a.begin(), a.end(), a.begin(), a.end(), std::not_equal_to<char>()), is_equal_to(0));
    }
}


int
main(int argc, char *argv[])
{
    return yats::run(argc, argv);
}

