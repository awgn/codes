/* $Id$ */
/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <bonelli@antifork.org> wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return. Nicola Bonelli
 * ----------------------------------------------------------------------------
 */
 
#ifndef _YATS_HPP_
#define _YATS_HPP_ 

#include <cstdlib>
#include <iostream>
#include <sstream>
#include <functional>
#include <algorithm>
#include <stdexcept>
#include <typeinfo>
#include <vector>
#include <memory>

#ifdef __linux__
#include <cxxabi.h>
#endif

#define Context(ctx) \
namespace ctx { static const char __context[] = #ctx; } \
namespace ctx
 
#define Test(type) \
void test_ ## type(const char *); \
yats::test_register hook_ ## type(test_ ##type, #type); \
void test_ ## type(const char *__name)

#define Assert(x,pred)          _Assert(x, pred, __context, __name, __LINE__)
#define Assert_nothrow(x)       _Assert_nothrow(x, __LINE__)
#define Assert_throw(x)         _Assert_throw(x, __LINE__)
#define Assert_throw_type(x,t)  _Assert_throw_type(x, t, __LINE__)

#define _Assert_nothrow(x,line) \
try \
{ \
    x; \
} \
catch(std::exception &e) \
{           \
    std::ostringstream err; \
    err << std::boolalpha << "Test " << __context << "::" << __name  \
                        << " -> exception not expected. Got " \
                        << yats::type_name(e) << "(\"" << e.what() << "\")" \
                        << " error at line " << line; \
    throw std::runtime_error(err.str()); \
} \
catch(...) \
{           \
    std::ostringstream err; \
    err << std::boolalpha << "Test " << __context << "::" << __name  \
                        << " -> exception not expected. Got unknown exception error at line " << line; \
    throw std::runtime_error(err.str()); \
} 

#define _Assert_throw(x,line) \
{ \
    bool thrown = false; \
    try \
    { \
        x; \
    } \
    catch(...) \
    {                \
        thrown = true; \
    }           \
    if (!thrown) \
    {  \
        std::ostringstream e; \
        e << std::boolalpha << "Test " << __context << "::" << __name  \
                            << " -> exception expected. Error at line " << line; \
        throw std::runtime_error(e.str()); \
    }  \
}

#define _Assert_throw_type(x, type, line) \
{ \
    bool thrown = false; \
    try \
    { \
        x; \
    } \
    catch(std::exception &e) \
    { \
        if (typeid(e).name() != typeid(type).name()) { \
            std::ostringstream err; \
            err << std::boolalpha << "Test " << __context << "::" << __name  \
                            << " -> exception " << yats::type_name<type>()  \
                            <<  " expected. Got " << yats::type_name(e) << " error at line " << line; \
            throw std::runtime_error(err.str()); \
        } \
        thrown = true; \
    }  \
    if (!thrown) \
    {  \
        std::ostringstream err; \
        err << std::boolalpha << "Test " << __context << "::" << __name  \
                            << " -> exception " << yats::type_name<type>() << " expected. Error at line " << line; \
        throw std::runtime_error(err.str()); \
    }  \
}

using namespace std::placeholders;

namespace yats 
{

#ifdef __linux__
    static inline
    std::string
    cxa_demangle(const char *name)
    {
        int status;
        std::shared_ptr<char> ret(abi::__cxa_demangle(name,0,0, &status), ::free);
        if (status < 0) 
            throw std::runtime_error("__cxa_demangle");
        return std::string(ret.get());
    }

#else
    static inline
    std::string
    cxa_demangle(const char *name)
    {
        return std::string(name);
    }
#endif

    template <typename Tp>
    std::string
    type_name()
    {
        return cxa_demangle(typeid(Tp).name());
    }
    template <typename Tp>
    std::string
    type_name(const Tp &t)
    {
        return cxa_demangle(typeid(t).name());
    }
    
    typedef std::function<void()> test_fun;

    struct test
    {
        static std::vector<std::pair<test_fun,std::string>> &
        list()
        {
            static std::vector<std::pair<test_fun,std::string>> ret;
            return ret; 
        }

        static int run()
        {
            unsigned int n = 0;
            std::cout << "Running " << list().size() << " tests." << std::endl;
            
            for(auto it = list().begin(), it_e = list().end(); it != it_e; ++it)
            {
                try
                {
                    it->first.operator()();
                    n++;  
                }   
                catch(std::exception &e)
                {
                    std::cerr << e.what() << std::endl;
                }
            }

            std::cerr << list().size() - n << " tests failed." << std::endl;
            return n == list().size() ? EXIT_SUCCESS : EXIT_FAILURE;
        }
    };

    struct test_register
    {
        test_register(void(*f)(const char *), const char *name)
        {
            test::list().push_back(std::make_pair(std::bind(f, name), name));        
        }
    };

    template <typename T>
    struct predicate
    {
        T value;
        const char * descr;
        std::function<bool(T&&)> fun;

        predicate(const char * _descr, std::function<bool(T&&)> _fun, T _value)
        : value(_value), descr(_descr), fun(_fun)
        {}

        bool operator()(T &&value) const
        {
            return fun(std::forward<T>(value));
        }
    };

    template <typename T>
    void _Assert(T &&__value, const predicate<T> &pred, const char *__ctx, const char *__name, int line)
    {
        if (!pred(std::forward<T>(__value))) {
            std::ostringstream e;
            e << std::boolalpha << "Test " << __ctx << "::" << __name 
                                << " -> predicate " << pred.descr << "(" << pred.value 
                                << ") failed: got (" << __value <<  "). Error at line " << line;
            throw std::runtime_error(e.str());
        }
    }

    /// standard predicates...

#define std_functional(__name__) \
    template <typename T> \
    predicate<T>           \
    is_ ## __name__ (T &&value)  \
    {                        \
        return predicate<T>("is_" #__name__,  \
                            std::function<bool(T&&)>( \
                                std::bind(std::__name__<T>(), _1, std::forward<T>(value))), \
                                std::forward<T>(value)); \
    }

    std_functional(greater)
    std_functional(greater_equal)
    std_functional(less)
    std_functional(less_equal)
    std_functional(equal_to)
    std_functional(not_equal_to)

    /// boolean...

    predicate<bool>
    is_true()
    {
        return predicate<bool>("is_boolean",  
                            std::function<bool(bool)>(
                                std::bind(std::equal_to<bool>(), _1, true)), true); 
    }
    
    predicate<bool>
    is_false()
    {
        return predicate<bool>("is_boolean",  
                            std::function<bool(bool)>(
                                std::bind(std::equal_to<bool>(), _1, false)), false); 
    }
}

#endif /* _YATS_HPP_ */
