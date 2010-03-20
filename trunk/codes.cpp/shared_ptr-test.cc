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
#include <cassert>
#include <cstdlib>
#include <vector>

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

void my_free(char *x)
{
    std::cout << "costom deleter ok!" << std::endl;
    free(x);
}

//////////////////////////////////////////////////
// cross-library shared_pointer example: 

more::shared_ptr<int>::type
make_zero()
{
    more::shared_ptr<int>::type null;
    assert(!null);

    if (!null) {
        std::cout << "conversion-to-bool test passed! (null-test)" << std::endl;
    }

    more::shared_ptr<int>::type ret(new int(1));
    assert(ret);

    ret.reset(new int(0));
    assert(*ret == 0);

    // copy

    more::shared_ptr<int>::type copy(ret);
    assert(ret.get() == copy.get());

    // custom deleter test:

    more::shared_ptr<char>::type cd((char *)malloc(1024), my_free);
    std::cout << "zero is not null but " << *ret << " @" << (void *)ret.get() << std::endl;

    // const_cast:

    more::shared_ptr<const char>::type q = more::const_pointer_cast<const char>(cd);
    assert(q.get() == cd.get());

    // assignment:

    more::shared_ptr<char>::type xyz;
    xyz = more::static_pointer_cast<char>(cd);
    assert(xyz.get() == cd.get());

    return ret;
}

std::vector< more::shared_ptr<int>::native_type >
factory()
{
    std::vector< more::shared_ptr<int>::type > vec;

    vec.push_back( more::shared_ptr<int>::type( new int(1) ) );
    vec.push_back( more::shared_ptr<int>::type( new int(2) ) );
    vec.push_back( more::shared_ptr<int>::type( new int(3) ) );

    return more::native_shared_ptr_container_cast(vec); 
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

    std::vector<boost::shared_ptr<int> > vec = factory();
    std::vector<boost::shared_ptr<int> >::iterator it = vec.begin();
    std::vector<boost::shared_ptr<int> >::iterator it_end = vec.end();

#elif defined(MORE_USE_QT_SHARED_PTR)
    QSharedPointer<int> zero = make_zero();

    std::vector<QSharedPointer<int> > vec = factory();
    std::vector<QSharedPointer<int> >::iterator it = vec.begin();
    std::vector<QSharedPointer<int> >::iterator it_end = vec.end();

#else
    std::shared_ptr<int> zero = make_zero();    
    
    std::vector<std::shared_ptr<int> > vec = factory();
    std::vector<std::shared_ptr<int> >::iterator it = vec.begin();
    std::vector<std::shared_ptr<int> >::iterator it_end = vec.end();

#endif

    for(int n=0; it != it_end; ++it, ++n)
    {
        std::cout << "vec[" << n << "]-> " << *(*it) << std::endl;
    }

    std::cout << "done!" << std::endl;
    return 0;
}
 
