/* $Id$ */
/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <bonelli@antifork.org> wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return. Nicola Bonelli 
 * ----------------------------------------------------------------------------
 */

#include <exec.hh>

using namespace more;

int
main(int argc, char *argv[])
{
    // 1)
    // more::exec abc;
    // abc.arg("/bin/ls").arg("/").arg("-la");
    //
    // 2)
    // abc.cmdline("/bin/ls -la /");

    // 3) by means of iterator...
    //

    const char * args[] = { "/bin/ls" , "/" , "-la" };
    more::exec abc(args, args+3); 

    std::cout << "running: [" << abc.cmdline() << "]" << std::endl;

    int out;
    abc.redirect( std::make_pair(more::exec::STDOUT, std::tr1::ref(out)) );
    
    // int err;
    // abc.redirect( exec::child_fd(more::exec::STDERR, std::tr1::ref(err) ) );

    abc(); // run

    char buffer[1024+1];
    
    int c;
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

    // group.wait_all();

    //
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
 
