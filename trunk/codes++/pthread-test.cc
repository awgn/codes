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

using namespace more;
using namespace more::posix;

mutex global_mutex;
cond  global_cond;
rw_mutex global_mutex_rw;

class Hello : public posix::thread {

public:
    void *operator()()
    {
        sleep(1);

        for(int j=0;j<2;j++) {
            this_thread::testcancel();
            scoped_lock<mutex> lock(global_mutex);
            for(int i=0;i<3;i++) {
                std::cout << "    [" << std::hex << this_thread::get_id() << "] hello\n"; 
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

};

class World : public posix::thread {

public:
    void *operator()()
    {
        sleep(1);

        for(int j=0;j<2;j++) {
            this_thread::testcancel();
            scoped_lock<mutex> lock(global_mutex);
            for(int i=0;i<3;i++) {
                std::cout << "    [" << std::hex << this_thread::get_id() << "] world\n"; 
                usleep(500000);
            }
        }
        return NULL;
    }
};


class Reader : public posix::thread {

public:
    void *operator()()
    {
        {
            scoped_lock<read_mutex> lock(global_mutex_rw);

            for(int i=0;i<5;i++) {
                std::cout << "    [" << std::hex << this_thread::get_id() << "] reader!\n"; 
                usleep(500000);
            }
        }

        return NULL;
    }
};

class Writer : public posix::thread {

public:
    void *operator()()
    {
        scoped_lock<write_mutex> lock(global_mutex_rw);

        for(int i=0;i<3;i++) {
            std::cout << "    [" << std::hex << this_thread::get_id() << "] writer!\n"; 
            usleep(500000);
        }
        return NULL;
    }

};


struct WaitCond : public posix::thread {

    void *operator()()
    {
        scoped_lock<mutex> lock(global_mutex);

        std::cout << "    [" << std::hex << this_thread::get_id() << "] waiting on conditon...\n";  

        global_cond.wait(lock);

        std::cout << "    [" << std::hex << this_thread::get_id() << "] signaled...\n"; 
        usleep(500000);
        return NULL;
    } 
};


struct Detached : public posix::thread
{
    void *operator()()
    {
        sleep(1);
        return NULL;
    }

    Detached()
    {
        std::cout << "    [" << std::hex << this_thread::get_id() << "] started...\n"; 
    }

    ~Detached()
    {
        std::cout << "    [" << std::hex << this_thread::get_id() << "] finished!\n"; 
    }
};

struct Restartable : public posix::thread
{
    Restartable(int n)
    : _M_init(n), _M_value(n)
    {
        std::cout << "    " << __PRETTY_FUNCTION__ << std::endl;
    }

    void restart_impl()
    {
        new (this) concrete_thread<Restartable>(_M_init);
    }

    void *operator()()
    {
        for(int i=0; i<5; i++) {
            std::cout << "    [" << std::hex << this_thread::get_id() << "] " << _M_value++ << std::endl; 
            sleep(1);
        }
        return NULL;
    }

    int _M_init;
    int _M_value;
};


struct join_functor
{
    void operator()(more::posix::thread *t) const
    {
        std::cout << __PRETTY_FUNCTION__ << " thread@ " << t << " finished! [ thread_id = " << t->get_id() << "]" << std::endl;
    }
};


#define RED     "\E[0;31;1m"
#define RESET   "\E[0m"

int main(int argc, char *argv[])
{

    std::cout << "\n[*]" RED " scoped thread..." RESET "\n";
    {
        concrete_thread<Hello> test;
        test.start();
        sleep(2);
    } // terminate as soon as possible

    std::cout << "\n[*]" RED " scoped thread (custom attr)..." RESET "\n";
    {
        std::tr1::shared_ptr<posix::thread_attr> a(new posix::thread_attr);
        a->setstacksize(1000000);

        concrete_thread<Hello> test(a);
        test.start();
        sleep(2);
    } // terminate as soon as possible


    std::cout << "\n[*] "RED"basic mutex..."RESET"\n";
    {
        Hello *hello = new concrete_thread<Hello>;
        World *world = new concrete_thread<World>;

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

        Reader *one = new concrete_thread<Reader>;
        Reader *two = new concrete_thread<Reader>;

        one->start();
        two->start();

        one->join();
        two->join();

        delete one;
        delete two;
    }

    std::cout << "\n[*] "RED"reader/writer mutex..."RESET"\n";
    {
        Reader *one = new concrete_thread<Reader>;
        Writer *two = new concrete_thread<Writer>;

        one->start();
        two->start();

        one->join();
        two->join();

        delete one;
        delete two;
    }

    std::cout << "\n[*] "RED"stopping a thread in a critical section..."RESET"\n";
    {
        Reader *one = new concrete_thread<Reader>;
        one->start();
        sleep(1);
        delete one;
    }

    std::cout << "\n[*] "RED"stopping a detached thread in a critical section..."RESET"\n";
    {
        Reader *one = new concrete_thread<Reader>;
        one->start();
        one->detach();
        delete one;
    }


    std::cout << "\n[*] "RED"wait cond thread..."RESET"\n";
    {   
        WaitCond * wc = new concrete_thread<WaitCond>;
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

    std::cout << "\n[*] "RED "terminate on cond thread..."RESET"\n";
    {   
        WaitCond * wc = new concrete_thread<WaitCond>;
        wc->start();

        std::cout << "    -> waiting for the thread to wait for conditions to be signaled...\n";
        sleep(1);

        wc->detach();
        delete wc;
    }

    std::cout << "\n[*] "RED "detached thread in the heap..."RESET"\n";
    {
        thread::start_detached_in_heap<Detached>();
        sleep(3);
    }

    std::cout << "\n[*] "RED "restartable thread..."RESET"\n";
    {
        posix::thread * t = new concrete_thread<Restartable>(1);
        t->start();
        t->join();
        std::cout << "\n[*] "RED "-----"RESET"\n";
        t->restart();
        t->join();
    }

    std::cout << "\n[*] "RED "thread_group..."RESET"\n";
    {   
        thread_group grp;

        thread *t1  = new concrete_thread<Reader>;
        thread *t2  = new concrete_thread<Reader>;
        thread *t3  = new concrete_thread<Reader>;
        thread *t4  = new concrete_thread<Reader>;
        thread *t5  = new concrete_thread<Reader>;

        grp.add(t1);
        grp.add(t2);
        grp.add(t3);
        grp.add(t4);
        grp.add(t5);

        grp.start_all();
        
        grp.join_all ( join_functor() );

        delete t1;
        delete t2;
        delete t3;
        delete t4;
        delete t5;
        
    }

    std::cout << "done." << std::endl;

    return 0;
}
