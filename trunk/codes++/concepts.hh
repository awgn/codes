/* $Id$ */
/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <bonelli@antifork.org> wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return. Nicola Bonelli 
 * ----------------------------------------------------------------------------
 */

#ifndef _CONCEPTS_HH_
#define _CONCEPTS_HH_ 

// Yet another boost::concepts implementation...
//

#define FUNCTION_REQUIRES_CONCEPT(t,C)    \
    void (C<t>::*ptr_ ## C)() __attribute__((unused)) = &C<t>::constraints

#define CLASS_REQUIRES_CONCEPT(t,C)       \
    template <void (C<t>::*ptr_value)()> struct class_ ## C {}; \
    typedef class_ ## C< & C<t>::constraints> C ## _type

// some basic concepts...
//

namespace more { namespace basic_concepts {

    template <class T>
    struct DefaultConstructible
    {
        void constraints()
        {
            T a __attribute__((unused));
        };
    };

    template <class T>
    struct Assignable
    {
        T x;
        void constraints()
        {
            x = x;
        };
    };

    template <class T>
    struct CopyConstructible
    {
        T a;
        void constraints()
        {
            T b(a);
            T *c = &a;
        }
    };

    template <class T>
    struct PreIncrementable
    {
        T a;
        void constraints()
        {
            ++a;
        }
    };

    template <class T>
    struct PostIncrementable
    {
        T a;
        void constraints()
        {
            a++;
        }
    };

    template <class T>
    struct EqualityComparable
    {
        T a, b;
        void constraints()
        {
            a == b;
            a != b;
        }
    };

    template <class T>
    struct LessThanComparable
    {
        T a, b;
        void constraints()
        {
            a < b;
        }
    };

}}

#endif /* _CONCEPTS_HH_ */
