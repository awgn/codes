/* $Id$ */
/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <bonelli@antifork.org> wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return. Nicola Bonelli
 * ----------------------------------------------------------------------------
 */

#include <stream_container.hh>
#include <vector>
#include <list>
#include <map>

#include <iostream>

int 
main(int argc, char *argv[])
{
    std::cout << "string                        :" << std::string("hello world") << std::endl; 
    std::vector<int> v1;

    v1.push_back(0);
    v1.push_back(1);
    v1.push_back(2);

    more::stream_container::sep(std::cout, " - ");   // set the separator

    std::cout << "vector<int>                   :" << v1 << std::endl;

    std::vector<char> v2;

    v2.push_back('a');
    v2.push_back('b');
    v2.push_back('c');
    v2.push_back('\n');
    v2.push_back('\0');

    more::stream_container::sep(std::cout, "|");   // set the separator
    std::cout << "vector<char>                  :" << v2 << std::endl;

    std::list<std::pair<int,int> > l1;

    l1.push_back( std::make_pair(0,0) );
    l1.push_back( std::make_pair(1,1) );
    l1.push_back( std::make_pair(2,2) );

    more::stream_container::sep(std::cout);    // remove separator

    std::cout << "list< std::pair<int,int> >    :" << l1 << std::endl;

    std::map<std::string,int> m;

    m["hello"]=0;
    m["world"]=1;

    more::stream_container::sep(std::cout, "|");   // set the separator
    std::cout << "map<std::string,int>          :" << m << std::endl;

    return 0;
}
