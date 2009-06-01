/* $Id*/
/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <bonelli@antifork.org> wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return. Nicola Bonelli 
 * ----------------------------------------------------------------------------
 */

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*****************************************************************************************************************

 [*] --- All the things you have always wanted to know about writing a generic perfect forwarder --- [*]


 definition: A generic forwarder (wrapper) is a template function whose final objective is to forward the 
             passed arguments to a given target function or callable object, provided as extra argument. 

             A forwarder is perfect if its call is equivalent to passing its arguments directly to the target function: 
             the behavior is correct and sports the same performance, not involving any further copy constructor.

 hypothesis: The current C++ standard does not allow to implement a generic perfect forwarder. A perfect forwarder is 
             indeed enabled by means of C++0x rvalue references. Instead, a generic non-perfect but correct forwarded 
             is implementable with the help of tr1::reference_wrapper<>. 

 thesis:     The major challenge of writing a generic wrapper is to provide a mechanism for it to pass its 
             arguments to a target function that accepts them by non-const reference.

             The client of the wrapper (the callee) must be able to pass both lvalue and rvalue as arguments, 
             in accordance with the target signature. Obviously if a target parameter (a parameter of the target function) 
             is taken as non-const reference, the corresponding actual parameter must not be provided to the wrapper 
             as rvalue (unnamed object).

             In the study presented, the following free function has been chosen as target object (it could be an object
             providing the operator() const or any other callable object):

             int fun(arg a1, const arg & a2, arg & a3)
             {
                a3 = arg(1);
                ...
                return 0;
             }

             The first signature examined as candidate for the perfect wrapper is the following one:
  
             template <typename F, typename T1, typename T2, typename T3>
             int candidate_forwarder_1(F &f, T1 &t1, T2 &t2, T3 &t3)
             {
                     return f(t1,t2,t3);
             }

             with all the arguments passed by non-const reference.
              
             The candidate_forwarder_1 sports the theoretical performance in that no additional copy constructors 
             are called during the parameter passing (comparisons are provided by test #0 and #1).
         
             However such a wrapper has a limitation: it does not allow the callee to pass an rvalue as argument 
             even if this argument is intended to be passed to the target function by value or const reference. 
             
             This limitation is imposed by the standard that does not allow a non-const reference to bind an rvalue, 
             this for preventing the common error of modifyng an rvalue that does not make much sense.

             To overcome this limitation, a second candidate for the perfect forwarder is taken into account:

             template <typename F, typename T1, typename T2, typename T3>
             int candidate_forwarder_2(F f, T1 t1, T2 t2, T3 t3)
             {
                 return f(t1,t2,t3);
             }

             [ note: the template argument deduction dacay, as pointed out in: 
                     C++ Template: the complete guide (Vandevoorde/Josuttis) ]

             In this wrapper all the arguments are passed by value. At first glance it seems that the ban of passing rvalues 
             as argument is removed and indeed it is. But alas, arguments are copy-constructed and more importantly the type 
             deduction decay makes array and function types be deducted as pointer types and the top-level CV qualifiers 
             (const/volatile) be discarded.
             
             Consequently this second wrapper has the following problems: 

             *1) it's not a perfect forwarder due to the template argument deduction decay. The template arguments resolve 
                 to types that cannot be used to select a possible overloaded target function (feature not required 
                 in this study). 

             *2) parameters of the target function taken by reference are bound to the formal parameters of the forwarder 
                 (t3, in the example) and not to actual ones provided by the callee (a3 in the test), which is very bad!

             *3) parameters are passed by value and additional copy constructors are called.

             The test #2 shows the major problem (*2) that makes this wrapper incorrect: the argument a3 passed to the wrapper 
             is not updated by the target function as it would be if it were passed directly to the target.
            
             Fortunately a workaround to this problem does exist. The std::tr1::reference_wrapper<> is the helper class 
             the callee must use for those arguments intended to be passed as non-const reference to the target function.
          
             The common instantiation point will be the following one:

             candidate_forwarder_2(fun, a1, a2, std::tr1::ref(a3));

             Test #3 and #4 show the correctness of the forwarder function achieved by means of the tr1::reference_wrapper<>. 
            
             The comparison of the test #0 and #4 shows further copy constructors involved for passing a1 and a2 (*3). 
             This is price to pay to have a correct wrapper that can accept lvalue and rvalues where the target function does.

             Also note that test #3 does not involove additional copy constructors thanks to certain optimizations
             of temporary/unnamed objects that takes place during the argument passing:

                               copy constructors are optimized away
                                          /       /
             candidate_forwarder_2(fun, arg(), arg(), std::tr1::ref(a3) );

             Despite of the performance loss the test #4 points out, candidate_forwarder_2 function is preferable to 
             candidate_forwarder_1, in that the limitation of rvalue passed as argument is removed. Even if if it's not 
             perfect at all (*1), it's the most convenient implementation the current C++ standard allows with the help of TR1.

             The object adaptor std::tr1::bind, for instance, is written exactly this way.

             Instead, the perfect forwarder is enabled by means of C++0x rvalue reference and has the following form:

             template <typename F, typename T1, typename T2, typename T3>
             int the_perfect_forwarder(F f, T1 && t1, T2 && t2, T3 && t3)
             {
                 return f(std::forward<T1>(t1),
                          std::forward<T2>(t2),
                          std::forward<T3>(t3));
             }

             The limitation that a non-const reference cannot bind a rvalue is overcome by means of the rvalue reference [&&]
             which can binds to a rvalue and allows its modification.

             Bearing in mind the following reference collapsing rules introduced by C++0x:

             T&  -> &  -> T&
             T&  -> && -> T&
             T&& -> &  -> T&
             T&& -> && -> T&&

             This wrapper has the following properties:

             *) Argument deduction is done in the following way: 

                - if an lvalue of type A is passed, T resolves to A& and by the reference collapsing rules, 
                the argument type becomes A& (lvalue reference);

                - if an rvalue is passed, T resolves to A, and the argument type becomes A&&. 
                
                The std::forward<> helper is required for the wrapper to be perfect.

                namespace std {

                    template<class T> 
                    T&& forward(T&& a) 
                    {
                        return a;
                    } 
                }
                
                If an lvalue of type A is passed as argument, the corresponding type T of the wrapper resolves to A&. Then 
                by the collapsing rules, argument and return type of std::forward become A&. 
                
                If an rvaule of type A is passed as argument, the corresponding type T of the wrapper resolves to A. Then
                argument and return type of std::forward become A&&. 
                
                The return type of std::forward is an expression that is declared rvalue and has no-name, therefore 
                it is an rvalue. Without the use of std::forward the parameter couldn't be an rvalue because of its name (a). 

             *) Both lvalue and rvalue can be provided as argument to the wrapper, in accordance to the signature of the
                target function/object.

             *) Additional copy constructors are not involeved.

             Nicola -- 09/05/31

*****************************************************************************************************************/
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <tr1/functional>
#include <iostream>
#include <colorful.hh>

using namespace more;

typedef colorful< TYPELIST(ecma::bold, ecma::fg_red) > red;
typedef colorful< TYPELIST(ecma::bold, ecma::fg_blue) > blue;
typedef colorful< TYPELIST(ecma::reset) > rst;
typedef colorful< TYPELIST(ecma::bold) > bold;

struct arg 
{
    arg(int n = 0)
    : _M_value(n)
    { std::cout << "    " << __PRETTY_FUNCTION__ << std::endl; }

    ~arg()
    { std::cout << "    " << __PRETTY_FUNCTION__ << std::endl; }
    
    arg(const arg &rhs)
    : _M_value(rhs._M_value)
    { 
        std::cout << "    " << __PRETTY_FUNCTION__ << std::endl; 
    }

    arg &operator=(const arg &rhs)
    { 
        _M_value = rhs._M_value;
        std::cout << "    " << __PRETTY_FUNCTION__ << std::endl; 
    }
    
    int get() const
    { return _M_value; }

    int _M_value;
};


/////////////////////////////////////////////////////////////
// the target function:

int fun(arg a1, const arg & a2, arg & a3)
{
    a3 = arg(1);       
    std::cout << "fun: a1 = " << a1.get() << std::endl;
    std::cout << "fun: a2 = " << a2.get() << std::endl;
    std::cout << "fun: a3 = " << a3.get() << std::endl;
    return 0;
}

/////////////////////////////////////////////////////////////
// candidates:

template <typename F, typename T1, typename T2, typename T3>
int candidate_forwarder_1(F &f, T1 &t1, T2 &t2, T3 &t3)
{
    return f(t1,t2,t3);
}   

template <typename F, typename T1, typename T2, typename T3>
int candidate_forwarder_2(F f, T1 t1, T2 t2, T3 t3)
{
    return f(t1,t2,t3);
} 

#ifdef __GXX_EXPERIMENTAL_CXX0X__
template <typename F, typename T1, typename T2, typename T3>
int the_perfect_forwarder(F f, T1 && t1, T2 && t2, T3 && t3)
{
    return f(std::forward<T1>(t1),
             std::forward<T2>(t2),
             std::forward<T2>(t3));
} 
#endif

/////////////////////////////////////////////////////////////
// test case:

int
main(int argc, char *argv[])
{    
    
    std::cout << red() << "\n[#0] direct call" << rst() << std::endl;
    {
        arg a1;
        arg a2;
        arg a3;

        std::cout << "--- prolog:" << std::endl;
        fun(a1, a2, a3);  // rvalue are not allowed
        std::cout << "--- epilog:" << std::endl;

        std::cout << "    arg & a3 = " << a3.get() << " <== should be 1! [" << bold() << "OK" << rst() << "]" << std::endl;
    }


    std::cout << red() << "\n[#1] candidate_forwarder_1" << rst() << std::endl;
    {
        arg a1;
        arg a2;
        arg a3;

        std::cout << "--- prolog:" << std::endl;
        candidate_forwarder_1(fun, a1, a2, a3);  // rvalue are not allowed
        std::cout << "--- epilog:" << std::endl;

        std::cout << "    arg & a3 = " << a3.get() << " <== should be 1! [" << bold() << "OK" << rst() << "]" << std::endl;
    }

    std::cout << red() << "\n[#2] candidate_forwarder_2" << rst() << std::endl;
    {
        arg a3;

        std::cout << "--- prolog:" << std::endl;
        candidate_forwarder_2(fun, arg(), arg(), a3 );  // rvalue are not allowed
        std::cout << "--- epilog:" << std::endl;

        std::cout << "    arg & a3 = " << a3.get() << " <== should be 1! [" << red() << "ERROR" << rst() << "]" << std::endl;
    }

    std::cout << red() << "\n[#3] candidate_forwarder_2 [with std::tr1::reference_wrapper<>]" << rst() << std::endl;
    {
        arg a3;

        std::cout << "--- prolog:" << std::endl;
        candidate_forwarder_2(fun, arg(), arg(), std::tr1::ref(a3) );  // rvalue are not allowed
        std::cout << "--- epilog:" << std::endl;

        std::cout << "    arg & a3 = " << a3.get() << " <== should be 1! [" << bold() << "OK" << rst() << "]" << std::endl;
    }

    std::cout << red() << "\n[#4] candidate_forwarder_2 [with std::tr1::reference_wrapper<>]" << rst() << std::endl;
    {
        arg a1;
        arg a2;
        arg a3;

        std::cout << "--- prolog:" << std::endl;
        candidate_forwarder_2(fun, a1, a2, std::tr1::ref(a3) );  // rvalue are not allowed
        std::cout << "--- epilog:" << std::endl;

        std::cout << "    arg & a3 = " << a3.get() << " <== should be 1! [" << bold() << "OK" << rst() << "]" << std::endl;
    }

#ifdef __GXX_EXPERIMENTAL_CXX0X__    
    std::cout << blue() << "\n[#5] the_perfect_forwarder [c++0x right reference]" << rst() << std::endl;
    {
        arg a1;
        arg a2;
        arg a3;

        std::cout << "--- prolog:" << std::endl;
        the_perfect_forwarder(fun, a1, a2, a3 );  // rvalue are not allowed
        std::cout << "--- epilog:" << std::endl;

        std::cout << "    arg & a3 = " << a3.get() << " <== should be 1! [" << bold() << "OK" << rst() << "]" << std::endl;
    }

#endif
    return 0;
}

