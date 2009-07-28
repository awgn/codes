/* $Id$ */
/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <bonelli@antifork.org> wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return. Nicola Bonelli
 * ----------------------------------------------------------------------------
 */

#include <ext_buffer.hh>

#include <iostream>
#include <algorithm>
#include <iterator>

#include <tr1/array>

int
main(int argc, char *argv[])
{
    char hello[]="hello ";
    char world[]="world!";

    more::ext_buffer x_hello(hello, 6);
    more::ext_buffer x_world(world, 6);

    const more::ext_buffer y_hello = x_hello;

    const void * p_hello = y_hello.data();

    const struct iovec * iov_hello = &x_hello;

    struct iovec abc[2] = { more::ext_buffer(hello,6), more::ext_buffer(world,6) }; 

    std::cout << "iovec test with ext_buffer: ";
    std::cout.flush();
    writev(fileno(stdout), abc, 2);

    std::tr1::array<iovec,1> array_iovec_hello = {{ more::ext_buffer(hello,5) }};

    std::cout << std::endl;

    std::string str( x_hello.begin(), x_hello.end() );
    std::cout << "iterator test: -> " << str << std::endl; 
    std::string rstr( x_hello.rbegin(), x_hello.rend() );
    std::cout << "reverse iterator iterator test: -> " << rstr << std::endl; 

    more::ext_const_buffer const_hello (x_hello);
    more::ext_const_buffer::const_iterator it = const_hello.begin();

    more::ext_const_buffer copy_constructor_const_hello(const_hello);
    more::ext_const_buffer copy_constructor_const_world(x_world);

    //////////////
    // make test:

    std::cout << "enter 4 chars:" << std::flush;
    char buf[4];

    // more::ext_buffer b = more::ext_buffer::make( more::functor::read<>(0, buf, 4) );                          // simple read()
    more::ext_const_buffer b = more::ext_const_buffer::make( more::functor::read<more::atomic_io>(0, buf, 4) );        // atomic_io read()

    std::cout << "b.data()=" << b.data() << " b.size()=" << b.size() << std::endl;
    
    std::cout << "[";
    std::copy( b.begin(), b.end(), std::ostream_iterator<char>(std::cout, ""));
    std::cout << "]" << std::endl;

    return 0;
}
 
