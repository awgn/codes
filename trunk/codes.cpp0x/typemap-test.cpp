/* $Id$ */
/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <bonelli@antifork.org> wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return. Nicola Bonelli 
 * ----------------------------------------------------------------------------
 */

#include <typemap.hpp>
#include <typeinfo>

TYPEMAP_KEY(std::string,name);
TYPEMAP_KEY(std::string,nickname);
TYPEMAP_KEY(int, age);
TYPEMAP_KEY(int, size);

int
main(int argc, char *argv[])
{
    typedef TYPEMAP(name, std::string, age, int) mymap;

    more::TM::get<name, mymap>::type me = "Nicola";
    more::TM::get<age , mymap>::type a __attribute__((unused)) = 36;

    typedef more::TM::null empty;

    typedef more::TM::append<size,int, empty >::type  map_1;
    typedef more::TM::append<age, int, map_1>::type  map_2;
    // typedef more::TM::append<nickname, int, map_2>::type map_3;

    more::TM::get<size, map_1>::type x __attribute__((unused))= 1;
    more::TM::get<size, map_2>::type c __attribute__((unused))= 2;
    more::TM::get<age,  map_2>::type d __attribute__((unused))= 3;

    std::cout << "size<map_1>::value  = " << more::TM::size<map_1>::value << std::endl;
    std::cout << "size<map_2>::value  = " << more::TM::size<map_2>::value << std::endl;

    std::cout << "index_of<size,map_2> = " << more::TM::index_of<size, map_2>::value << std::endl;
    std::cout << "index_of<age, map_2> = " << more::TM::index_of<age, map_2>::value << std::endl;
    std::cout << "index_of<name,map_2> = " << more::TM::index_of<name, map_2>::value << std::endl;

    std::cout << "get_key<0, map_2> = " << typeid(more::TM::get_key<0, map_2>::type).name() << std::endl; 
    std::cout << "get_key<1, map_2> = " << typeid(more::TM::get_key<1, map_2>::type).name() << std::endl; 
    std::cout << "get_key<2, map_2> = " << typeid(more::TM::get_key<2, map_2>::type).name() << std::endl; 

    std::cout << "key[0] = " << ( more::TM::get_key<0, map_2>::type::value() ? : "NULL" ) << std::endl;
    std::cout << "key[1] = " << ( more::TM::get_key<1, map_2>::type::value() ? : "NULL" ) << std::endl;
    std::cout << "key[2] = " << ( more::TM::get_key<2, map_2>::type::value() ? : "NULL" ) << std::endl;
    return 0;
}
 
