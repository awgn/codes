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
#include <string>

#include <variant.hh>
#include <oracle.hh>

struct hello : public more::oracle<hello> {};
struct world : public more::oracle<world> {};

oracle_std_swap(hello);
oracle_std_swap(world);

struct visitor 
{
    void operator()(const hello &) const
    {
        std::cout << "visitor: variant<...hello...> " << std::endl;
    }
    void operator()(const world &) const
    {
        std::cout << "visitor: variant<...world...> " << std::endl;
    }
    
    void operator()(const std::string &s) const
    {
        std::cout << "visitor: variant<...std::string...> = " << s << std::endl;
    }

    void operator()(const int &n) const
    {
        std::cout << "visitor: variant<...int...> = " << n << std::endl;
    }

};

int
main(int, char *[])
{
    more::oracle_trace<hello>::enable();
    more::oracle_trace<world>::enable();

    {
        more::variant<hello,int,std::string> abc;
        std::cout << "storage size: " << abc.storage_size() << std::endl;

        std::cout << "which:" << abc.which() << " typeid:" << abc.type().name() << std::endl;

        std::cout << "... store an integer ..." << std::endl;
        abc = 10;
        std::cout << "which:" << abc.which() << " typeid:" << abc.type().name() << std::endl;

        std::cout << "... store a string ..." << std::endl;
        abc = std::string("hello");
        std::cout << "which:" << abc.which() << " typeid:" << abc.type().name() << std::endl;

    }

    //  get<>
    {   
        more::variant<int,char> abc;
        abc = 10;

        std::cout << "int:" << abc.get<int>() << std::endl;
        abc = 'a';

        try {
            std::cout << "int:" << abc.get<int>() << std::endl;
        }
        catch(std::bad_cast) {
            std::cout << "char:" << abc.get<char>() << std::endl;
        }
    }   

    // stream
    {
        more::variant<int,double,std::string> abc;

        abc = 10;
        std::cout << "std::cout << " << abc << " << std::endl;" << std::endl;

        abc = 1.2;
        std::cout << "std::cout << " << abc << " << std::endl;" << std::endl;

        abc = "hello world!";
        std::cout << "std::cout << " << abc << " << std::endl;" << std::endl;
    }

    // visitor 
    {
        more::variant<int,std::string> abc;

        abc = 10;
        abc.apply_visitor(visitor());

        abc = "hello world!";
        abc.apply_visitor(visitor());
    }

    // copy constructor | operator= 
    {
        more::variant<hello, world> a((hello()));
        more::variant<hello, world> b((world()));

        a = b;

        more::variant<hello, world> copy(a);

        copy.apply_visitor(visitor());
    }
    return 0;
}
 
