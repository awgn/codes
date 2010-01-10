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
#include <cstdio>

#include <tr1/array>

int
main(int argc, char *argv[])
{
    char hello[]="hello ";
    char world[]="world!";

    more::ext_buffer x_hello(hello, 6);
    more::ext_buffer x_world(world, 6);

    const more::ext_buffer y_hello = x_hello;

    const void * p_hello __attribute__((unused)) = y_hello.data();

    const struct iovec * iov_hello __attribute__((unused))= &x_hello;

    struct iovec abc[2] = { more::ext_buffer(hello,6), more::ext_buffer(world,6) }; 

    std::cout << "iovec test with ext_buffer: ";
    std::cout.flush();
    writev(fileno(stdout), abc, 2);

    std::tr1::array<iovec,1> array_iovec_hello __attribute__((unused)) = {{ more::ext_buffer(hello,5) }};

    std::cout << std::endl;

    std::string str( x_hello.begin(), x_hello.end() );
    std::cout << "iterator test: -> " << str << std::endl; 
    std::string rstr( x_hello.rbegin(), x_hello.rend() );
    std::cout << "reverse iterator iterator test: -> " << rstr << std::endl; 

    more::ext_const_buffer const_hello (x_hello);
    more::ext_const_buffer::const_iterator it __attribute__((unused)) = const_hello.begin();

    more::ext_const_buffer copy_constructor_const_hello(const_hello);
    more::ext_const_buffer copy_constructor_const_world(x_world);

    //////////////
    // make test:

    std::cout << "enter 3 chars (+return):" << std::flush;
    char buf[8];


    // more::ext_buffer b = more::ext_buffer::make( more::functor::read<>(0, buf, 4) );                                      // simple read()
    more::ext_const_buffer b = more::ext_const_buffer( more::functor::read<more::atomic_io>(0, buf, 4) );        // atomic_io read()

    std::cout << "raw buf@ " << (void *)buf << std::endl;
    std::cout << "b.data()=" << b.data() << " b.size()=" << b.size() << std::endl;
    
    // std::cout << "[";
    // std::copy( b.begin(), b.end(), std::ostream_iterator<char>(std::cout, ""));
    // std::cout << "]" << std::endl;

    // b.at(0) = 'c';

    // std::cout << "[";
    // std::cout << b[0] << b[1] << b.at(2) << b.at(3);
    // std::cout << "]" << std::endl;

    // b.commit(4);

    // b.discard(4);
    // std::cout << "b.data()=" << b.data() << " b.size()=" << b.size() << std::endl;

    // b.commit(4);
    // std::cout << "b.data()=" << b.data() << " b.size()=" << b.size() << std::endl;

    std::cout << "enter 7 chars (+return):" << std::flush;
        
    char buf1[8];

    more::ext_buffer b1 = more::ext_buffer(buf1, 8, more::functor::read<more::atomic_io>(0, buf1, 8) );
    // more::ext_buffer b1 = more::ext_buffer(tmp, 8, more::functor::read<>(0, tmp, 8 ) );

    std::cout << "raw buf1@ " << (void *)buf1 << std::endl;
    std::cout << "b1.data()=" << b1.data() << " b1.size()=" << b1.size() << std::endl;

    std::cout << "enter 7 chars (+return):" << std::flush;

    more::ext_buffer b2 = more::ext_buffer(b1, more::functor::read<more::atomic_io>(0, b1.data(), b1.size() ));
    std::cout << "b2.data()=" << b2.data() << " b2.size()=" << b1.size() << std::endl;

    return 0;
}
 
