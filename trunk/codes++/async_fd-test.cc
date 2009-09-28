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
#include <async_fd.hh>

using namespace more;

int
main(int argc, char *argv[])
{    
    int fd = open("test.txt", O_RDWR|O_CREAT);
    if ( fd == -1 )
        throw more::syscall_error("open");

    async_fd<IO_blocking, IO_nonblocking> out(fd);

    {
        std::cout << "---------- write ----------" << std::endl;
        std::cout << "(" << write(out,"test\n",5) << ")" << std::endl;    
        out.flush();   
    }

    {
        std::cout << "---------- writev ----------" << std::endl;

        iovec vec[2] = { { const_cast<char *>("0123456789"), 10 },
            { const_cast<char *>("abcdefghilmnopqrstuvz"), 21 }};

        std::cout << "(" << out.writev(vec,2) << ")";
        out.dump_buffer(std::cout);
        std::cout << std::endl;

        std::cout << "flush..." << std::endl;
        out.flush();

        out.dump_buffer(std::cout);
        std::cout << std::endl;
    }

    {
        std::cout << "---------- poll ----------" << std::endl;
        short revent = out.poll<POLLOUT>();
        std::cout << "revent= " << std::hex << revent << " &POLLOUT -> " << static_cast<bool>(revent & POLLOUT) << std::endl; 
        std::cout << std::endl;
    }

    char read_buf[80];
   
    {     
        std::cout << "---------- read in 5 sec: type something and press enter... ----------" << std::endl;

        more::async_fd<IO_nonblocking, IO_nonblocking> i(0);

        if ( i.poll<POLLIN>(5000) & POLLIN ) {
            int n = i.read(read_buf,80);
            std::cout << "in.read()=" << std::dec << n << std::endl;
            if (n > 0) {
                read_buf[n]='\0';
                std::cout << "[" << read_buf << "]" << std::endl;
            }
        }
        else {
            std::cout << "nothing to read!" << std::endl;
        }
    }

    std::cout << "---------- type enter to terminate  ----------" << std::endl;
    {
        more::async_fd<IO_blocking, IO_blocking> abc(0);
        abc.read(read_buf,80);
    }

    return 0;
}
 
