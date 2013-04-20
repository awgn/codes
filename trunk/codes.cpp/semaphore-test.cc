/* $Id$ */
/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <bonelli@antifork.org> wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return. Nicola Bonelli
 * ----------------------------------------------------------------------------
 */

#include <pthread++.hh>
#include <semaphore.hh>

#include <iostream>

using namespace more;

posix::semaphore sem(0);

struct Wait : public posix::thread 
{
    void *operator()()
    {
        std::cout << "[WAIT] waiting..." << std::endl;
        sem.wait();
        std::cout << "[WAIT] done." << std::endl;
        return 0;
    } 

};

struct Post : public posix::thread
{
    void *operator()()
    {
        std::cout << "[POST] post in 2 sec..." << std::endl;
        sleep(3);
        std::cout << "[POST] post()" << std::endl;
        sem.post();
        std::cout << "[POST] done." << std::endl;
        return 0;
    } 
};

int main(int, char *[])
{
    posix::concrete_thread<Post> p;
    posix::concrete_thread<Wait> w;

    w.start();
    p.start();

    w.join();
    p.join();
}
