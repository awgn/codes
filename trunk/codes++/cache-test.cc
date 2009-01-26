/* $Id$ */
/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <bonelli@antifork.org> wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return. Nicola Bonelli
 * ----------------------------------------------------------------------------
 */

#include <cache.hh>

#include <iostream>
#include <stdexcept>
#include <iomanip>

struct my_entry {
    int val;
};

generic::cache<std::string,my_entry> cc;

my_entry *cc_find(const std::string &key) 
{
    my_entry * f;

    try {
        f = cc.find(key);
    }
    catch(generic::cache<std::string,my_entry>::expired &e) {
        std::cout << "   entry expired@" << e.pimp << ": {" << e.pimp->val << "} (cache<>::expired has been thrown)" << std::endl;
        return e.pimp;
    }
    catch(std::runtime_error &r) {
        std::cout << "  " << r.what() << std::endl;
        return NULL;
    }

    std::cout << "   entry found@" << std::hex << f << std::dec << ": {" << f->val << "}" << std::endl;
    return f;
}


int 
main(int argc, char *argv[])
{
    my_entry r = { 1 };
    my_entry t = { 2 };

    my_entry *p , *q;

    std::cout << "create the entry: {" << r.val << "} ...\n";
    std::cout << "create the entry: {" << t.val << "} ...\n";

    p = cc.insert("ephemeral", r, 1);
    q = cc.insert("everlasting", t, 1000);

    std::cout << "   entry@" << std::hex << p << std::dec << std::endl;
    std::cout << "   entry@" << std::hex << q << std::dec << std::endl;

    std::cout << "search the entry 'ephemeral'\n";
    p = cc_find("ephemeral");

    std::cout << "sleep 2 sec...\n";
    sleep(2);

    std::cout << "research the entry 'ephemeral' ...\n";
    q = cc_find("ephemeral");

    std::cout << "update the timeout for the current entry...\n";
    cc.update("ephemeral", 1);

    std::cout << "search the entry again...\n";
    q = cc_find("ephemeral");

    std::cout << "search with an invalid key...\n";
    q = cc_find("unknown");

    sleep(2);

    std::cout << std::boolalpha << "dump the content of the cache...\n";
    generic::cache<std::string, my_entry>::iterator it = cc.begin();
    for(; it != cc.end(); ++it) 
    {
        std::cout << "   key:" << (*it).first <<  
                     " value:" << static_cast<my_entry>((*it).second).val << " expired:"  << (*it).second.is_expired() << std::endl;
    }

    return 0;
}
