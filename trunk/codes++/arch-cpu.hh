/* $Id$ */
/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <bonelli@antifork.org> wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return. Nicola Bonelli
 * ----------------------------------------------------------------------------
 */

#ifndef ARCH_CPU_HH 
#define ARCH_CPU_HH 

#include <cstring>
#include <cstdio>
#include <string>
#include <stdexcept>

#include <cpuinfo.hh>       // more
#include <timeval.hh>       // more
#include <lexical_cast.hh>  // more

// arch policies taken from the linux kernel 2.6/include/arch-.../
//

#if !defined(ULLONG_MAX) && ( defined(ARCH_i386) || defined(ARCH_X86_64) || defined(ARCH_IA64) )    
#define ULLONG_MAX 18446744073709551615ULL
#endif

#if !defined(ARCH_i386) && !defined(ARCH_X86_64) && !defined(ARCH_IA64) && !defined(ARCH_PARISC)
#error "No arch defined (ie: -DARCH_i386)"
#endif

namespace arch
{
    enum { 
        ia386,
        x86_64,
        ia64,
        parisc
    };

    template <int n> struct type;

    template <>
    struct type<ia386> 
    {
#ifdef ARCH_i386
        typedef unsigned long long cycles_t;
        static const cycles_t max = ULLONG_MAX;

#define rdtscll(val) \
        __asm__ __volatile__("rdtsc" : "=A" (val))
        static cycles_t get_cycles () {
            cycles_t ret;
            rdtscll(ret);
            return ret;
        }
#endif
    };

    template <>
    struct type<x86_64> 
    {
#ifdef ARCH_X86_64
        typedef unsigned long long cycles_t;
        static const cycles_t max = ULLONG_MAX;

#define rdtscll(val) do { \
    unsigned int __a,__d; \
    asm volatile("rdtsc" : "=a" (__a), "=d" (__d)); \
    (val) = ((unsigned long)__a) | (((unsigned long)__d)<<32); \
} while(0)

static const cycles_t get_cycles () 
{
    cycles_t ret;
    rdtscll(ret);
    return ret;
}
#endif
};

template <>
struct type<ia64>
{
#ifdef ARCH_IA64
    typedef unsigned long long cycles_t;
    static const cycles_t max = ULLONG_MAX;

#define ia64_getreg(num) ({ \
    cycles_t ia64_res; \
    asm volatile ("mov %o=ar%1" : "=r" (ia64_res) : "i" (num)); \
    ia64_res; \
    })
    static inline cycles_t get_cycles ()
    {
        cycles_t ret;
        ret = ia64_getreg( 44 /* AR_ITC */);
        return ret;
    }
#endif
};

template <>
struct type<parisc>
{
#ifdef ARCH_PARISC
    typedef unsigned long cycles_t;
    static const cycles_t max = ULONG_MAX;

#define mfctl(reg) ({ \
    unsigned long cr;  \
    __asm__ __volatile__( \
            "mfctl " #reg ",%0" : \
            "=r" (cr) \
            ); \
    cr; \
    })
    static inline cycles_t get_cycles ()
    {
        return mfctl(16);
    }
#endif
};

} // namespace arch

namespace arch 
{
    class cpu 
    {

#if   defined(ARCH_i386)
        static const int Ty = arch::ia386; 
#elif defined(ARCH_X86_64)
        static const int Ty = arch::x86_64;
#elif defined(ARCH_IA64)
        static const int Ty = arch::ia64;
#elif defined(ARCH_PARISC)
        static const int Ty = arch::parisc;
#endif
        arch::type<Ty>::cycles_t _M_hz;

        cpu(int sec=0) :
        _M_hz()
        {
            more::proc::cpuinfo cpu;
            long double mhz = more::lexical_cast<long double>(cpu(0,"cpu MHz"));

            _M_hz = static_cast<arch::type<Ty>::cycles_t>(mhz*1000000);
            if ( sec == 0 )  // no clock estimation, use the one read from proc
                return;

            // clock estimation (to detect speedstep and cool 'n quiet)  

            more::Timeval g1 = more::Timeval::now();
            arch::type<Ty>::cycles_t c1 = arch::type<Ty>::get_cycles();

            sleep(sec);

            more::Timeval g2 = more::Timeval::now();
            arch::type<Ty>::cycles_t c2 = arch::type<Ty>::get_cycles();

            unsigned long long usec = (g2-g1).to_usec();

            long double ehz  = (long double)(c2-c1)/(long double)(usec)*1000000;

            if ( static_cast<int>(ehz/1000000) == static_cast<int>(_M_hz/1000000) ) 
                return;

            std::clog << "cpu scalable-clock (speedstep/cool 'n quiet) detected!" << std::endl;
        }

    public:
        typedef arch::type<Ty>::cycles_t cycles_t;

        static 
        arch::type<Ty>::cycles_t Hz(int n=0) 
        {   
            static cpu mycpu(n);
            return mycpu._M_hz; 
        }

        static 
        arch::type<Ty>::cycles_t get_cycles()
        {
            return arch::type<Ty>::get_cycles();
        }

        static int type() 
        {
            return Ty;
        }        

        static int wait_until(const arch::type<Ty>::cycles_t &t)
        {
            if (arch::type<Ty>::get_cycles() >= t)
                return -1;

            while (arch::type<Ty>::get_cycles() < t);
            return 0;
        }

    };

}

#endif /* ARCH_CPU_HH */

