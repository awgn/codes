/* $Id$ */
/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <bonelli@antifork.org> wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return. Nicola Bonelli 
 * ----------------------------------------------------------------------------
 */

#include <exec.hpp>

using namespace more;

int
main(int, char *[])
{
    // 1)
    // abc.cmdline("/bin/ls -la /");

    // 2) by means of iterator...
    //

    const char * args[] = { "/bin/ls" , "/" , "-la" };
    // more::exec abc(args, args+3); 

    more::exec abc {"/bin/ls", "/", "-la" };

    std::cout << "running: [" << abc.cmdline() << "]" << std::endl;

    int out;
    abc.redirect( exec::redirect_type( more::exec::STDOUT, std::ref(out)) );
    
    // int err;
    // abc.redirect( exec::redirect_type(more::exec::STDERR, std::ref(err) ) );

    abc(); // run
    
    // abc(exec::nullprolog());

    char buffer[1024+1];
    
    ssize_t c;
    while ( (c = read(out,buffer, 1024)) > 0 ) {
        buffer[c]='\0';
        std::cout << buffer;
    }

    abc.wait();

    if ( abc.is_exited() ) {
        std::cout << "-> exit: [" << abc.exit_status() << " pid: " << abc.pid() << "]" << std::endl;
    }

    std::cout << "running group: [" << abc.cmdline() << "] 2 processes..." << std::endl;
   
    more::exec abc1(args, args+3);
    more::exec abc2(args, args+3);

    more::exec_group group;

    group.add(&abc1);
    group.add(&abc2);

    group.run();

    // you can also provide a prolog for each child process...
    // group.run(more::exec::nullprolog());

    // group.wait_all();

    // if ( abc1.is_exited()) {
    //     std::cout << "-> exit: [" << abc1.exit_status() << " pid: " << abc1.pid() << "]" << std::endl;
    // }
    // if ( abc2.is_exited()) {
    //     std::cout << "-> exit: [" << abc2.exit_status() << " pid: " << abc2.pid() << "]" << std::endl;
    // }

    more::exec * p;
    while ( (p = group.wait()) )
    {
        if ( p->is_exited() )
            std::cout << "group -> exit: [" << p->exit_status() << " pid: " << p->pid() << "]" << std::endl;
    }

    return 0;
}
 
