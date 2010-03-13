/* $Id$ */
/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <bonelli@antifork.org> wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return. Nicola Bonelli 
 * ----------------------------------------------------------------------------
 */

//
// C++ flavor of inotify-code-example written by Robert Love and published 
// at http://www.linuxjournal.com/article/8478
//

#include "inotify.hh"

int main()
{
    Linux::Inotify<> test;

    int file __attribute__((unused)) = test.add_watch("./test", IN_MODIFY | IN_CREATE | IN_DELETE);
    int dir  __attribute__((unused)) = test.add_watch(".", IN_MODIFY | IN_CREATE | IN_DELETE);

    for(;;) {

        Linux::Inotify_list tmp = test.wait_events();

        for ( Linux::Inotify_list::iterator it = tmp.begin(); it != tmp.end() ; it++) {
            if (it->len && !strcmp( it->name.c_str(),"test") )
                std::cout << *it ; 
        }

    }     

    return 0;
}
