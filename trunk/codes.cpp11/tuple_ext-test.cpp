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

        Assert( more::tuple_call(somma, tup), is_equal_to(42));
    }


    Test(for_each)
    {
        std::tuple<int,int,int> v {1,2,3};
        int sum = 0;

        more::tuple_for_each(v, [&](int n) {
                              sum += n;
                             });

        Assert(sum, is_equal_to(6));
    }
}


int
main(int argc, char *argv[])
{
    return yats::run(argc, argv);
}
 

