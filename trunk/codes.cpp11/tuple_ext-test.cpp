#include <iostream>
#include <tuple_ext.hpp>

#include <yats.hpp>

using namespace yats;


Context(tuple_extended)
{

    Test(call)
    {
        auto somma = [](int a, int b) { return a+b; };

        std::tuple<int, int> tup{40,2};

        Assert( more::explode_call(somma, tup) == 42);
    }


}


int
main(int argc, char *argv[])
{
    return yats::run(argc, argv);
}
 

