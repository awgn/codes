/* $Id$ */
/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <bonelli@antifork.org> wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return. Nicola Bonelli 
 * ----------------------------------------------------------------------------
 */

#include <shared_ptr.hh>
#include <iostream>

#include <tr1/memory>

////////////////////////////////////////////////
// more::shared_ptr<> traits usage:

template <typename T>
void shared_ptr_test(T)
{
    std::cout << std::boolalpha;
    std::cout << "is_tr1::value   = " << T::is_tr1::value << std::endl;    
    std::cout << "is_boost::value = " << T::is_boost::value << std::endl;    
    std::cout << "is_qt::value    = " << T::is_qt::value << std::endl;    
};

void shared_ptr_std_test(more::std_type)
{
    std::cout << "more::shared_ptr<> is a std_type" << std::endl;
}

void shared_ptr_std_test(more::qt_type)
{

    std::cout << "more::shared_ptr<> is a qt_type" << std::endl;
}

//////////////////////////////////////////////////
// cross-library shared_pointer example: 

more::shared_ptr<int>::type
make_zero()
{
    return more::shared_ptr<int>::type(new int(0));
}


//////////////////////////////////////////////////
// cross-library test

int
main(int, char *[])
{
    shared_ptr_test( more::shared_ptr<int>() );
    shared_ptr_std_test( more::shared_ptr<int>::shared_ptr_type() );

#if defined(MORE_USE_BOOST_SHARED_PTR)
    boost::shared_ptr<int> zero = make_zero();
#elif defined(MORE_USE_QT_SHARED_PTR)
    QSharedPointer<int> zero = make_zero();
#else
    std::tr1::shared_ptr<int> zero = make_zero();
#endif

    std::cout << "zero is " << *zero << std::endl;

    return 0;
}
 
