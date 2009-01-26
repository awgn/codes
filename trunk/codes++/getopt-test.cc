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
#include <singleton.hh>


class opt_long : public posix::getopt_long {

    public:
        opt_long(const char *o, const struct option *lo, int *li) : posix::getopt_long(o,lo,li) { } 

        int parseopt(int i, char *arg) { 
            std::cout << char(i) << " " << arg << std::endl; 
            return 0;
        }

};

class opt_classic : public posix::getopt {

    public:
        opt_classic(const char *o) : posix::getopt(o) { } 

        int parseopt(int i, char *arg) { 
            std::cout << char(i) << " " << arg << std::endl; 
            return 0;
        }

};

// options deriving from posix::getopt as singleton
//
struct opt_singleton : public posix::getopt, public generic::singleton<opt_singleton> {

    int a;
    int b;
    int c;

    SINGLETON_CTOR(opt_singleton), a(0), b(0), c(0) { }

    int parseopt(int i, char *arg) { 
        switch(i) {
            case 'a': a = atoi(arg); break;
            case 'b': b = atoi(arg); break;
            case 'c': c = atoi(arg); break;
        }
        return 0;
    }

};

static const struct option long_options[] = {
    {"aaa", required_argument, NULL, 'a' },
    {"bbb", required_argument, NULL, 'b' },
    {"ccc", required_argument, NULL, 'c' },
    {0, 0, 0, 0}
};


void posteriori() {

    opt_singleton &opts = const_cast<opt_singleton &>(opt_singleton::instance());

    std::cout << "a=" << opts.a << std::endl;
    std::cout << "b=" << opts.b << std::endl;
    std::cout << "c=" << opts.c << std::endl;
}


int main(int argc, char **argv)
{
    opt_long            X(":a:b:c:", long_options, NULL);
    opt_classic         Y(":a:b:c:");

    //
    // const_cast<> is required to loose the volatile attribute.
    // It's just like saying: "Yes sir, I know this code is *not* multithread!"
    //

    opt_singleton &Z = const_cast<opt_singleton &>(opt_singleton::instance());

    Z.init(":a:b:c:");
    Z.parse(argc, argv);

    posteriori();
}
