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
#include <singleton.hh>

struct s0: public more::singleton<s0, more::indestructible_singleton_type> 
{
    s0( base_type::tag )
    {
        std::cout << __PRETTY_FUNCTION__ << std::endl;
    }

    ~s0()
    {
        std::cout << __PRETTY_FUNCTION__ << std::endl;
    }
};

struct s1: public more::singleton<s1, volatile more::singleton_type, TYPELIST(int)> 
{
    int _M_value;

    // s1( more::singleton<s1, volatile more::singleton_type, TYPELIST(int)>::tag, int value)
    s1( base_type::tag, int value)
    :  _M_value(value)
    {
        std::cout << __PRETTY_FUNCTION__ << std::endl;
    }

    ~s1()
    {
        std::cout << __PRETTY_FUNCTION__ << std::endl;
    }
};

struct s2: public more::singleton<s2, const more::indestructible_singleton_type, TYPELIST(std::string, std::string) > {

    std::string _M_par1;
    std::string _M_par2;

    s2( base_type::tag, const std::string &p1, const std::string &p2)
    : _M_par1(p1),
      _M_par2(p2)
    {
        std::cout << __PRETTY_FUNCTION__ << std::endl;
    }

    ~s2() 
    {
        std::cout << __PRETTY_FUNCTION__ << std::endl;
    }

};

template <typename T>
struct s3: public more::singleton< s3<T>, more::singleton_type, TYPELIST(T) > 
{
    T _M_value;

    // s3( typename base_type::tag, T value )
    s3( typename more::singleton< s3<T>, more::singleton_type, TYPELIST(T) >::tag, T value )
    :  _M_value(value)
    {
        std::cout << __PRETTY_FUNCTION__ << std::endl;
    }

    ~s3()
    {
        std::cout << __PRETTY_FUNCTION__ << std::endl;
    }

};

#define TEST(n,...) \
struct test_ ## n : public more::singleton<test_ ## n, more::singleton_type, TYPELIST(__VA_ARGS__)> \
{ \
    test_ ## n ( base_type::tag, __VA_ARGS__) \
    {}\
}; 

#define CONST_TEST(n,...) \
struct const_test_ ## n : public more::singleton<const_test_ ## n, const more::singleton_type, TYPELIST(__VA_ARGS__)> \
{ \
    const_test_ ## n ( base_type::tag, __VA_ARGS__) \
    {}\
}; 

#define VOLATILE_TEST(n,...) \
struct volatile_test_ ## n : public more::singleton<volatile_test_ ## n, volatile more::singleton_type, TYPELIST(__VA_ARGS__)> \
{ \
    volatile_test_ ## n ( base_type::tag, __VA_ARGS__) \
    {}\
}; 

#define INDESTRUCTIBLE_TEST(n,...) \
struct ind_test_ ## n : public more::singleton<ind_test_ ## n, more::indestructible_singleton_type, TYPELIST(__VA_ARGS__)> \
{ \
    ind_test_ ## n ( base_type::tag, __VA_ARGS__) \
    {}\
}; 

#define INDESTRUCTIBLE_CONST_TEST(n,...) \
struct ind_const_test_ ## n : public more::singleton<ind_const_test_ ## n, const more::indestructible_singleton_type, TYPELIST(__VA_ARGS__)> \
{ \
    ind_const_test_ ## n ( base_type::tag, __VA_ARGS__) \
    {}\
}; 

#define INDESTRUCTIBLE_VOLATILE_TEST(n,...) \
struct ind_volatile_test_ ## n : public more::singleton<ind_volatile_test_ ## n, volatile more::indestructible_singleton_type, TYPELIST(__VA_ARGS__)> \
{ \
    ind_volatile_test_ ## n ( base_type::tag, __VA_ARGS__) \
    {}\
}; 

TEST(1, int);
TEST(2, int,int);
TEST(3, int,int,int);
TEST(4, int,int,int,int);
TEST(5, int,int,int,int,int);
TEST(6, int,int,int,int,int,int);
TEST(7, int,int,int,int,int,int,int);
TEST(8, int,int,int,int,int,int,int,int);
TEST(9, int,int,int,int,int,int,int,int,int);

CONST_TEST(1, int);
CONST_TEST(2, int,int);
CONST_TEST(3, int,int,int);
CONST_TEST(4, int,int,int,int);
CONST_TEST(5, int,int,int,int,int);
CONST_TEST(6, int,int,int,int,int,int);
CONST_TEST(7, int,int,int,int,int,int,int);
CONST_TEST(8, int,int,int,int,int,int,int,int);
CONST_TEST(9, int,int,int,int,int,int,int,int,int);

VOLATILE_TEST(1, int);
VOLATILE_TEST(2, int,int);
VOLATILE_TEST(3, int,int,int);
VOLATILE_TEST(4, int,int,int,int);
VOLATILE_TEST(5, int,int,int,int,int);
VOLATILE_TEST(6, int,int,int,int,int,int);
VOLATILE_TEST(7, int,int,int,int,int,int,int);
VOLATILE_TEST(8, int,int,int,int,int,int,int,int);
VOLATILE_TEST(9, int,int,int,int,int,int,int,int,int);

INDESTRUCTIBLE_TEST(1, int);
INDESTRUCTIBLE_TEST(2, int,int);
INDESTRUCTIBLE_TEST(3, int,int,int);
INDESTRUCTIBLE_TEST(4, int,int,int,int);
INDESTRUCTIBLE_TEST(5, int,int,int,int,int);
INDESTRUCTIBLE_TEST(6, int,int,int,int,int,int);
INDESTRUCTIBLE_TEST(7, int,int,int,int,int,int,int);
INDESTRUCTIBLE_TEST(8, int,int,int,int,int,int,int,int);
INDESTRUCTIBLE_TEST(9, int,int,int,int,int,int,int,int,int);

INDESTRUCTIBLE_CONST_TEST(1, int);
INDESTRUCTIBLE_CONST_TEST(2, int,int);
INDESTRUCTIBLE_CONST_TEST(3, int,int,int);
INDESTRUCTIBLE_CONST_TEST(4, int,int,int,int);
INDESTRUCTIBLE_CONST_TEST(5, int,int,int,int,int);
INDESTRUCTIBLE_CONST_TEST(6, int,int,int,int,int,int);
INDESTRUCTIBLE_CONST_TEST(7, int,int,int,int,int,int,int);
INDESTRUCTIBLE_CONST_TEST(8, int,int,int,int,int,int,int,int);
INDESTRUCTIBLE_CONST_TEST(9, int,int,int,int,int,int,int,int,int);

INDESTRUCTIBLE_VOLATILE_TEST(1, int);
INDESTRUCTIBLE_VOLATILE_TEST(2, int,int);
INDESTRUCTIBLE_VOLATILE_TEST(3, int,int,int);
INDESTRUCTIBLE_VOLATILE_TEST(4, int,int,int,int);
INDESTRUCTIBLE_VOLATILE_TEST(5, int,int,int,int,int);
INDESTRUCTIBLE_VOLATILE_TEST(6, int,int,int,int,int,int);
INDESTRUCTIBLE_VOLATILE_TEST(7, int,int,int,int,int,int,int);
INDESTRUCTIBLE_VOLATILE_TEST(8, int,int,int,int,int,int,int,int);
INDESTRUCTIBLE_VOLATILE_TEST(9, int,int,int,int,int,int,int,int,int);

int 
main(int argc, char *argv[])
{

#define RED     "\E[0;31;1m"
#define RESET   "\E[0m"

    s0 & r0 = s0::instance();
    volatile s1 & r1 = s1::instance(97);
    const s2 & r2 = s2::instance("hello","world");   // initialize the singleton

    std::cout << RED << "\nsingleton instance (constructor on first-use):" << RESET << std::endl;

    std::cout << "r0 -> singleton instance @" << std::hex << &r0  << '\n';
    std::cout << "r1 -> singleton instance @" << std::hex << &r1  << '\n';
    std::cout << "r2 -> singleton instance @" << std::hex << &r2  << '\n';

    volatile s1 & q1 = s1::instance();
    const s2 & q2 = s2::instance();

    std::cout << RED << "\nsingleton instances (access): " << RESET << std::endl;

    std::cout << "q1: slot @" << std::hex << &q1 << " value:" << std::dec << q1._M_value << '\n';
    std::cout << "q2: slot @" << std::hex << &q2 << " value:" << q2._M_par1 << ' ' << q2._M_par2 << '\n';

    std::cout << RED << "\ntemplate singleton<T> instance:" << RESET << std::endl;

    s3<double> & t3 = s3<double>::instance(11.2);
    s3<float>  & t4 = s3<float>::instance(10.2);

    std::cout << "t3: slot @" << std::hex << &t3 << " value:" << std::dec << t3._M_value << '\n';
    std::cout << "t4: slot @" << std::hex << &t4 << " value:" << std::dec << t4._M_value << '\n';

    std::cout << "t3: slot @" << std::hex << & s3<double>::instance() << " value:" << std::dec << s3<double>::instance()._M_value << '\n';
    std::cout << "t4: slot @" << std::hex << & s3<float>::instance() << " value:" << std::dec << s3<float>::instance()._M_value << '\n';

    test_1 & ref_1 __attribute__((unused)) = test_1::instance();
    test_2 & ref_2 __attribute__((unused)) = test_2::instance();
    test_3 & ref_3 __attribute__((unused)) = test_3::instance();
    test_4 & ref_4 __attribute__((unused)) = test_4::instance();
    test_5 & ref_5 __attribute__((unused)) = test_5::instance();
    test_6 & ref_6 __attribute__((unused)) = test_6::instance();
    test_7 & ref_7 __attribute__((unused)) = test_7::instance();
    test_8 & ref_8 __attribute__((unused)) = test_8::instance();
    test_9 & ref_9 __attribute__((unused)) = test_9::instance();

    const const_test_1 & const_ref_1 __attribute__((unused)) = const_test_1::instance();
    const const_test_2 & const_ref_2 __attribute__((unused)) = const_test_2::instance();
    const const_test_3 & const_ref_3 __attribute__((unused)) = const_test_3::instance();
    const const_test_4 & const_ref_4 __attribute__((unused)) = const_test_4::instance();
    const const_test_5 & const_ref_5 __attribute__((unused)) = const_test_5::instance();
    const const_test_6 & const_ref_6 __attribute__((unused)) = const_test_6::instance();
    const const_test_7 & const_ref_7 __attribute__((unused)) = const_test_7::instance();
    const const_test_8 & const_ref_8 __attribute__((unused)) = const_test_8::instance();
    const const_test_9 & const_ref_9 __attribute__((unused)) = const_test_9::instance();

    volatile volatile_test_1 & volatile_ref_1 __attribute__((unused)) = volatile_test_1::instance();
    volatile volatile_test_2 & volatile_ref_2 __attribute__((unused)) = volatile_test_2::instance();
    volatile volatile_test_3 & volatile_ref_3 __attribute__((unused)) = volatile_test_3::instance();
    volatile volatile_test_4 & volatile_ref_4 __attribute__((unused)) = volatile_test_4::instance();
    volatile volatile_test_5 & volatile_ref_5 __attribute__((unused)) = volatile_test_5::instance();
    volatile volatile_test_6 & volatile_ref_6 __attribute__((unused)) = volatile_test_6::instance();
    volatile volatile_test_7 & volatile_ref_7 __attribute__((unused)) = volatile_test_7::instance();
    volatile volatile_test_8 & volatile_ref_8 __attribute__((unused)) = volatile_test_8::instance();
    volatile volatile_test_9 & volatile_ref_9 __attribute__((unused)) = volatile_test_9::instance();

    ind_test_1 & ind_ref_1 __attribute__((unused)) = ind_test_1::instance();
    ind_test_2 & ind_ref_2 __attribute__((unused)) = ind_test_2::instance();
    ind_test_3 & ind_ref_3 __attribute__((unused)) = ind_test_3::instance();
    ind_test_4 & ind_ref_4 __attribute__((unused)) = ind_test_4::instance();
    ind_test_5 & ind_ref_5 __attribute__((unused)) = ind_test_5::instance();
    ind_test_6 & ind_ref_6 __attribute__((unused)) = ind_test_6::instance();
    ind_test_7 & ind_ref_7 __attribute__((unused)) = ind_test_7::instance();
    ind_test_8 & ind_ref_8 __attribute__((unused)) = ind_test_8::instance();
    ind_test_9 & ind_ref_9 __attribute__((unused)) = ind_test_9::instance();

    const ind_const_test_1 & ind_const_ref_1 __attribute__((unused)) = ind_const_test_1::instance();
    const ind_const_test_2 & ind_const_ref_2 __attribute__((unused)) = ind_const_test_2::instance();
    const ind_const_test_3 & ind_const_ref_3 __attribute__((unused)) = ind_const_test_3::instance();
    const ind_const_test_4 & ind_const_ref_4 __attribute__((unused)) = ind_const_test_4::instance();
    const ind_const_test_5 & ind_const_ref_5 __attribute__((unused)) = ind_const_test_5::instance();
    const ind_const_test_6 & ind_const_ref_6 __attribute__((unused)) = ind_const_test_6::instance();
    const ind_const_test_7 & ind_const_ref_7 __attribute__((unused)) = ind_const_test_7::instance();
    const ind_const_test_8 & ind_const_ref_8 __attribute__((unused)) = ind_const_test_8::instance();
    const ind_const_test_9 & ind_const_ref_9 __attribute__((unused)) = ind_const_test_9::instance();

    volatile ind_volatile_test_1 & ind_volatile_ref_1 __attribute__((unused)) = ind_volatile_test_1::instance();
    volatile ind_volatile_test_2 & ind_volatile_ref_2 __attribute__((unused)) = ind_volatile_test_2::instance();
    volatile ind_volatile_test_3 & ind_volatile_ref_3 __attribute__((unused)) = ind_volatile_test_3::instance();
    volatile ind_volatile_test_4 & ind_volatile_ref_4 __attribute__((unused)) = ind_volatile_test_4::instance();
    volatile ind_volatile_test_5 & ind_volatile_ref_5 __attribute__((unused)) = ind_volatile_test_5::instance();
    volatile ind_volatile_test_6 & ind_volatile_ref_6 __attribute__((unused)) = ind_volatile_test_6::instance();
    volatile ind_volatile_test_7 & ind_volatile_ref_7 __attribute__((unused)) = ind_volatile_test_7::instance();
    volatile ind_volatile_test_8 & ind_volatile_ref_8 __attribute__((unused)) = ind_volatile_test_8::instance();
    volatile ind_volatile_test_9 & ind_volatile_ref_9 __attribute__((unused)) = ind_volatile_test_9::instance();

#ifdef ERR_0
    s1 a;  // <- instances of singleton are not allowed 
#endif
#ifdef ERR_1
    s1 b ( const_cast<const s1 &>(s1::instance()) ); // <- instances are not copyable
#endif
#ifdef ERR_2
    s1 & r5 = s1::instance(97); // cv-correctness
#endif
    return 0;
}
