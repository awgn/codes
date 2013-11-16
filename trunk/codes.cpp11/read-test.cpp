#include <read.hpp>
#include <show.hpp>

#include <vector>
#include <list>
#include <map>

int
main(int argc, char *argv[])
{
    auto a = read<int>("10");

    auto b = read<std::pair<int,std::string>>("( 10 10)");

    auto c = read<std::array<int, 3>>("[1 2 3]");

    auto d = read<std::tuple<int, std::string, char>>("(10 hello c)");

    auto e = read<std::vector<int>>("[ 1 2 3 ]");
    auto f = read<std::list<int>>  ("[ 4 5 6 ]");

    auto g = read<std::map<int, std::string>>("[ (1 hello) (2 world) ]");

    auto h = read<std::chrono::nanoseconds>  ("1_ns");
    auto i = read<std::chrono::system_clock::time_point>("1_h");

    auto j = read<bool>("true");

    std::cout << show(a) << std::endl;
    std::cout << show(b) << std::endl;
    std::cout << show(c) << std::endl;

    std::cout << show(d) << std::endl;
    std::cout << show(e) << std::endl;

    std::cout << show(f) << std::endl;

    std::cout << show(g) << std::endl;

    std::cout << show(h) << std::endl;
    std::cout << show(i) << std::endl;

    std::cout << show(j) << std::endl;
    return 0;
}

