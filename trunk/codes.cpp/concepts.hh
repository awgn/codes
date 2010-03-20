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

#ifndef __GXX_EXPERIMENTAL_CXX0X__
#include <tr1/type_traits>
namespace std { using namespace std::tr1; }
#else
#include <tr1/type_traits>
#include <type_traits>
#endif

// Yet another boost::concepts implementation...
//

#define FUNCTION_REQUIRES_CONCEPT(t,C)    \
    void (C<t>::*ptr_ ## C)() __attribute__((unused)) = &C<t>::constraints

#define FUNCTION_REQUIRES_BINARY_CONCEPT(t1,t2,C)    \
    void (C<t1,t2>::*ptr_ ## C)() __attribute__((unused)) = &C<t1,t2>::constraints

#define CLASS_REQUIRES_CONCEPT(t,C)       \
    template <void (C<t>::*ptr_value)()> struct class_ ## C {}; \
    typedef class_ ## C< & C<t>::constraints> C ## _type

#ifndef CLASS_REQUIRES_BINARY_CONCEPT
#define CLASS_REQUIRES_BINARY_CONCEPT(t1,t2,C)       \
    template <void (C<t1,t2>::*ptr_value)()> struct class_ ## C {}; \
    typedef class_ ## C< & C<t1,t2>::constraints> C ## _type
#endif

// some basic concepts...
//

namespace more { namespace basic_concepts {

    template <bool> struct static_assert__;
    template <>
    struct static_assert__<true>
    {
        enum { value = true };
    };

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

    template <class T>
    struct LessOrEqualThanComparable
    {
        T a, b;
        void constraints()
        {
            a <= b;
        }
    };

    template <class T>
    struct GreaterThanComparable
    {
        T a, b;
        void constraints()
        {
            a > b;
        }
    };

    template <class T>
    struct GreaterOrEqualThanComparable
    {
        T a, b;
        void constraints()
        {
            a >= b;
        }
    };

    /////////////////// tr1::type_traits concepts /////////////////// 

#define TR1_TYPE_TRAITS_CONCEPT(type, trait) \
    template <typename Ty>  \
    struct type \
    {   \
        void constraints() \
        {   \
            static_assert__<std::trait<Ty>::value>();  \
        }\
    };
#define TR1_TYPE_TRAITS_BINARY_CONCEPT(type, trait) \
    template <typename T1, typename T2>  \
    struct type \
    {   \
        void constraints() \
        {   \
            static_assert__<std::trait<T1,T2>::value>();  \
        }\
    };

    TR1_TYPE_TRAITS_CONCEPT(IsVoidConcept, is_void);  
    TR1_TYPE_TRAITS_CONCEPT(IsIntegralConcept, is_integral);  
    TR1_TYPE_TRAITS_CONCEPT(IsFloatingPointConcept, is_floating_point);  
    TR1_TYPE_TRAITS_CONCEPT(IsArrayConcept, is_array);  
    TR1_TYPE_TRAITS_CONCEPT(IsPointerConcept, is_pointer);  
    TR1_TYPE_TRAITS_CONCEPT(IsReferenceConcept, is_reference);  
    TR1_TYPE_TRAITS_CONCEPT(IsMemberObjectPointerConcept, is_member_object_pointer);  
    TR1_TYPE_TRAITS_CONCEPT(IsMemberFunctionPointerConcept, is_member_function_pointer);  
    TR1_TYPE_TRAITS_CONCEPT(IsEnumConcept, is_enum);  
    TR1_TYPE_TRAITS_CONCEPT(IsUnionConcept, is_union);  
    TR1_TYPE_TRAITS_CONCEPT(IsClassConcept, is_class);  
    TR1_TYPE_TRAITS_CONCEPT(IsFunctionConcept, is_function);  

    TR1_TYPE_TRAITS_CONCEPT(IsArithmeticConcept, is_arithmetic);  
    TR1_TYPE_TRAITS_CONCEPT(IsFundamentalConcept, is_fundamental);  
    TR1_TYPE_TRAITS_CONCEPT(IsObjectConcept, is_object);  
    TR1_TYPE_TRAITS_CONCEPT(IsScalarConcept, is_scalar);  
    TR1_TYPE_TRAITS_CONCEPT(IsCompoundConcept, is_compound);  
    TR1_TYPE_TRAITS_CONCEPT(IsMemberPointerConcept, is_member_pointer);  

    TR1_TYPE_TRAITS_CONCEPT(IsConstConcept, is_const);  
    TR1_TYPE_TRAITS_CONCEPT(IsVolatileConcept, is_volatile);  
    TR1_TYPE_TRAITS_CONCEPT(IsPodConcept, is_pod);  
    TR1_TYPE_TRAITS_CONCEPT(IsEmptyConcept, is_empty);  
    TR1_TYPE_TRAITS_CONCEPT(IsPolymorphicConcept, is_polymorphic);  
    TR1_TYPE_TRAITS_CONCEPT(IsAbstractConcept, is_abstract);  
    TR1_TYPE_TRAITS_CONCEPT(HasTrivialContructorConcept, tr1::has_trivial_constructor);  
    TR1_TYPE_TRAITS_CONCEPT(HasTrivialCopyConcept, tr1::has_trivial_copy);  
    TR1_TYPE_TRAITS_CONCEPT(HasTrivialAssignConcept, has_trivial_assign);  
    TR1_TYPE_TRAITS_CONCEPT(HasTrivialDestructorConcept, has_trivial_destructor);  
    TR1_TYPE_TRAITS_CONCEPT(HasNothrowContructorConcept, tr1::has_nothrow_constructor);  
    TR1_TYPE_TRAITS_CONCEPT(HasNothrowCopyConcept, tr1::has_nothrow_copy);  
    TR1_TYPE_TRAITS_CONCEPT(HasNothrowAssignConcept, has_nothrow_assign);  
    TR1_TYPE_TRAITS_CONCEPT(HasVirtualDestructorConcept, has_virtual_destructor);  
    TR1_TYPE_TRAITS_CONCEPT(IsSignedConcept, is_signed);  
    TR1_TYPE_TRAITS_CONCEPT(IsUnsignedConcept, is_unsigned);  

    TR1_TYPE_TRAITS_BINARY_CONCEPT(IsSameConcept, is_same);
    TR1_TYPE_TRAITS_BINARY_CONCEPT(IsConvertibleConcept, is_convertible);
    TR1_TYPE_TRAITS_BINARY_CONCEPT(IsBaseOfConcept, is_base_of);

}}

#endif /* _CONCEPTS_HH_ */
