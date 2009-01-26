/* $Id$ */
/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <bonelli@antifork.org> wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return. Nicola Bonelli
 * ----------------------------------------------------------------------------
 */

#include "pthread++.hh"

using namespace posix;

mutex global_mutex;
cond  global_cond;
rw_mutex global_mutex_rw;

class Hello : public posix::thread {

public:
    void *operator()()
    {
        sleep(1);

        for(int j=0;j<2;j++) {
            this->testcancel();
            scoped_lock<mutex> lock(global_mutex);
            for(int i=0;i<3;i++) {
                std::cout << "    [" << std::hex << self() << "] hello\n"; 
                usleep(500000);
            }
        }
        return NULL;
    }

    Hello()
    : posix::thread()
    {}

    Hello(std::tr1::shared_ptr<thread_attr> a)
    : posix::thread(a)
    {}

    ~Hello()
    {
        this->cancel();        
    }
};

class World : public posix::thread {

public:
    void *operator()()
    {
        sleep(1);

        for(int j=0;j<2;j++) {
            this->testcancel();
            scoped_lock<mutex> lock(global_mutex);
            for(int i=0;i<3;i++) {
                std::cout << "    [" << std::hex << self() << "] world\n"; 
                usleep(500000);
            }
        }
        return NULL;
    }

    ~World()
    {
        this->cancel();
    }
};


class Reader : public posix::thread {

public:
    void *operator()()
    {
        {
            scoped_lock<rw_mutex, base_lock::reader> lock(global_mutex_rw);

            for(int i=0;i<3;i++) {
                std::cout << "    [" << std::hex << self() << "] reader!\n"; sleep(1);
            }
        }

        return NULL;
    }

    ~Reader()
    {
        this->cancel();
    }
};

class Writer : public posix::thread {

public:
    void *operator()()
    {
        scoped_lock<rw_mutex, base_lock::writer> lock(global_mutex_rw);

        for(int i=0;i<3;i++) {
            std::cout << "    [" << std::hex << self() << "] writer!\n"; sleep(1);
        }
        return NULL;
    }

    ~Writer()
    {
        this->cancel();
    }

};


struct WaitCond : public posix::thread {

    void *operator()()
    {
        scoped_lock<mutex> lock(global_mutex);

        std::cout << "    [" << std::hex << self() << "] waiting on cond... (" << 
        std::boolalpha << (bool)global_cond << ")\n"; 

        global_cond.wait(global_mutex);

        std::cout << "    [" << std::hex << self() << "] signaled...\n"; sleep(1);
        return NULL;
    } 

    ~WaitCond()
    {
        this->cancel();
    }
};


int main(int argc, char *argv[])
{

#define RED     "\E[0;31;1m"
#define RESET   "\E[0m"

    std::cout << "\n[*]" RED " scoped thread..." RESET "\n";
    {
        Hello test;
        test.start();
        sleep(2);
    } // terminate as soon as possible

    std::cout << "\n[*]" RED " scoped thread (custom attr)..." RESET "\n";
    {
        std::tr1::shared_ptr<posix::thread_attr> a(new posix::thread_attr);
        a->setstacksize(1000000);

        Hello test(a);
        test.start();
        sleep(2);
    } // terminate as soon as possible


    std::cout << "\n[*] "RED"basic mutex..."RESET"\n";
    {
        Hello *hello = new Hello;
        World *world = new World;

        hello->start();
        world->start();

        std::cout << "    hello.is_running(): " << std::boolalpha << hello->is_running() << std::endl;
        std::cout << "    world.is_running(): " << std::boolalpha << world->is_running() << std::endl;

        hello->join();
        world->join();

        std::cout << "    hello.is_running(): " << std::boolalpha << hello->is_running() << std::endl;
        std::cout << "    world.is_running(): " << std::boolalpha << world->is_running() << std::endl;

        delete hello;
        delete world;
    }

    std::cout << "\n[*] " RED "reader mutex..." RESET "\n";
    {

        Reader *one = new Reader;
        Reader *two = new Reader;

        one->start();
        two->start();

        one->join();
        two->join();

        delete one;
        delete two;
    }

    std::cout << "\n[*] "RED"reader/writer mutex..."RESET"\n";
    {
        Reader *one = new Reader;
        Writer *two = new Writer;

        one->start();
        two->start();

        one->join();
        two->join();

        delete one;
        delete two;
    }

    std::cout << "\n[*] "RED"stopping a thread in a critical section..."RESET"\n";
    {
        Reader *one = new Reader;
        one->start();
        delete one;
    }
    std::cout << "\n[*] "RED"stopping a detached thread in a critical section..."RESET"\n";
    {
        Reader *one = new Reader;
        one->start();
        one->detach();
        delete one;
    }


    std::cout << "\n[*] "RED"wait cond thread..."RESET"\n";
    {   
        WaitCond * wc = new WaitCond;
        wc->start();

        std::cout << "    -> waiting for the thread to wait for conditions to be signaled...\n";
        sleep(1);

        {
            scoped_lock<mutex> lock(global_mutex);
            // global_mutex.lock();  

            std::cout << "    -> signaling...\n";
            global_cond.signal();

            // global_mutex.unlock();
        }

        std::cout << "    -> joining the thread...\n";
        wc->join();
        delete wc;
    }

    return 0;
}
