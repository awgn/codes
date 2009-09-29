#include <iostream>
#include <string>

std::string helloworld()
{
    std::string ret("Hello World!");
    return std::move(ret); 
}

int
main(int argc, char *argv[])
{
    std::cout << helloworld() << std::endl;
    return 0;
}
 
