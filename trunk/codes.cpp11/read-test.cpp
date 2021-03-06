#include <variant.hpp>
#include <read.hpp>
#include <show.hpp>

#include <vector>
#include <list>
#include <map>

namespace my
{
    struct test
    {
        int value;
    };

    inline std::string
    show(test const&  value)
    {
        return std::to_string(value.value);
    }

    template <typename CharT, typename Traits>
    void
    read(test &ret, std::basic_istream<CharT,Traits>&in)
    {
        if (!(in >> ret.value))
            throw std::runtime_error("read<test>: parse error");
    }
}


int
main(int, char *[])
{
    auto a = read<int>("10");

    auto b  = read<std::pair<int,std::string>>("( 10 10)");
    auto b2 = read<std::pair<int,std::string>>("( 10 10)");
    auto b3 = read<std::pair<int,std::string>>("( 10 10)");

    auto c = read<std::array<int, 3>>("[1 2 3]");

    auto d = read<std::tuple<int, std::string, char>>("(10 hello c)");

    auto e = read<std::vector<int>>("[ 1 2 3 ]");
    auto f = read<std::list<int>>  ("[ 4 5 6 ]");

    auto g = read<std::map<int, std::string>>("[ (1 hello) (2 world) ]");

    auto h = read<std::chrono::nanoseconds>  ("1_ns");
    auto i = read<std::chrono::system_clock::time_point>("1_h");

    auto k = read<my::test>("42");

    auto u = read<std::unique_ptr<int>>("11");

    auto j = read<bool>("true");

    std::cout << show(a) << std::endl;
    std::cout << show(b) << std::endl;
    std::cout << show(c) << std::endl;

    std::cout << show(d) << std::endl;
    std::cout << show(e) << std::endl;

    std::cout << show(f) << std::endl;

    std::cout << show(g) << std::endl;

    std::cout << show(h) << std::endl;
    std::cout << show(k) << std::endl;
    std::cout << show(i) << std::endl;

    std::cout << show(j) << std::endl;

    std::cout << show(u) << std::endl;

    auto z = read<more::variant<int, std::string>>("world");
    std::cout << show(z) <<  std::endl;

    // {
    //     std::istringstream in(std::string("10"));
    //     std::cout << try_read<std::chrono::seconds>(in) << std::endl;
    //     std::cout << try_read<int>(in) << std::endl;
    //     std::cout << try_read<double>(in) << std::endl;
    //     std::cout << try_read<std::string>(in) << std::endl;
    // }

    return 0;
}

