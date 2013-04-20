/* $Id$ */
/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <bonelli@antifork.org> wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return. Nicola Bonelli
 * ----------------------------------------------------------------------------
 */

#include <cstdlib>
#include <getopt.hh>

class opt_classic : public posix::getopt {

    public:
        opt_classic(const char *o) 
        : posix::getopt(o) 
        {} 

        int parseopt(int i, char *arg) { 
            std::cout << "opt: '" << char(i) << "' -> " << arg << std::endl; 
            return 0;
        }
};

class opt_long : public posix::getopt_long {

    public:
        opt_long(const char *o, const struct option *lo, int *li) 
        : posix::getopt_long(o,lo,li) 
        {} 

        int parseopt(int i, char *arg) { 
            std::cout << "opt: '" << char(i) << "' -> " << arg << std::endl; 
            return 0;
        }
};

static const struct option long_options[] = {
    {"aaa", required_argument, NULL, 'a' },
    {"bbb", required_argument, NULL, 'b' },
    {"ccc", required_argument, NULL, 'c' },
    {0, 0, 0, 0}
};

int main(int argc, char *argv[])
{
    opt_long dummy(":a:b:c:", long_options, NULL);
    // opt_classic         Y(":a:b:c:");

    dummy.parse(argc,argv);
}
