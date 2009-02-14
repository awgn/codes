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

int
main(int argc, char *argv[])
{
    more::exec abc("/bin/ls");

    abc.arg("/").arg("-la");

    int n;
    if ( !abc(more::exec::redirect_fd<1>(n)) ) {
        std::cout << "exec error!" << std::endl;
        exit(1);
    }

    int c;
    char buffer[1024+1];
    
    while ( (c = read(n,buffer, 1024)) > 0 ) {
        buffer[c]='\0';
        std::cout << buffer << std::endl;
    }

    abc.wait();

    if ( abc.is_exited()) {
        std::cout << "--> exit: " << abc.exit_status() << " pid: " << abc.pid() << std::endl;
    }

    return 0;
}
 
