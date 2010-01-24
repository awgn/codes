/* $Id$ */
/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <bonelli@antifork.org> wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return. Nicola Bonelli
 * ----------------------------------------------------------------------------
 */

// this classes require cpufreq library (debian package: libcpufreq-dev)
//

#ifndef ARCH_CPU_HH 
#define ARCH_CPU_HH 

#include <cstring>
#include <cstdio>
#include <string>
#include <stdexcept>
#include <list>

#include <cpufreq.hh>       // more + libcpufreq
#include <tr1/memory>

// asm policies are taken from the linux kernel 2.6/include/arch-.../
//

#if !defined(ULLONG_MAX) && ( defined(ARCH_i386) || defined(ARCH_X86_64) || defined(ARCH_IA64) )    
#define ULLONG_MAX 18446744073709551615ULL
#endif

#if !defined(ARCH_i386) && !defined(ARCH_X86_64) && !defined(ARCH_IA64) && !defined(ARCH_PARISC)
#error "No arch defined (ie: -DARCH_i386, -DARCH_X86_64...)"
#endif

namespace more { namespace arch {

    enum { 
        ia386,
        x86_64,
        ia64,
        parisc
    };

    template <int n> struct cpu_traits;

    template <>
    struct cpu_traits<ia386> 
    {
#ifdef ARCH_i386
        typedef unsigned long long cycles_type;
        static const cycles_type max = ULLONG_MAX;

#define rdtscll(val) \
        __asm__ __volatile__("rdtsc" : "=A" (val))
        static cycles_type get_cycles () {
            cycles_type ret;
            rdtscll(ret);
            return ret;
        }
#endif
    };

    template <>
    struct cpu_traits<x86_64> 
    {
#ifdef ARCH_X86_64
        typedef unsigned long long cycles_type;
        static const cycles_type max = ULLONG_MAX;

#define rdtscll(val) do { \
    unsigned int __a,__d; \
    asm volatile("rdtsc" : "=a" (__a), "=d" (__d)); \
    (val) = ((unsigned long)__a) | (((unsigned long)__d)<<32); \
} while(0)

        static const cycles_type get_cycles () 
        {
            cycles_type ret;
            rdtscll(ret);
            return ret;
        }
#endif
    };

    template <>
    struct cpu_traits<ia64>
    {
    #ifdef ARCH_IA64
        typedef unsigned long long cycles_type;
        static const cycles_type max = ULLONG_MAX;

#define ia64_getreg(num) ({ \
        asm volatile ("mov %o=ar%1" : "=r" (ia64_res) : "i" (num)); \
        ia64_res; \
        })

        static inline cycles_type get_cycles ()
        {
            cycles_type ret;
            ret = ia64_getreg( 44 /* AR_ITC */);
            return ret;
        }
#endif
    };

    template <>
    struct cpu_traits<parisc>
    {
    #ifdef ARCH_PARISC
        typedef unsigned long cycles_type;
        static const cycles_type max = ULONG_MAX;

#define mfctl(reg) ({ \
        unsigned long cr;  \
        __asm__ __volatile__( \
        "mfctl " #reg ",%0" : \
        "=r" (cr) \
        ); \
        cr; \
        })
        static inline cycles_type get_cycles ()
        {
            return mfctl(16);
        }
#endif
    };

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

   public:

        /* 
         *  static methods 
         */

        typedef arch::cpu_traits<Ty>::cycles_type cycles_type;

        static 
        arch::cpu_traits<Ty>::cycles_type 
        get_cycles()
        {
            return arch::cpu_traits<Ty>::get_cycles();
        }

        static int wait_until(const arch::cpu_traits<Ty>::cycles_type &t)
        {
            if (arch::cpu_traits<Ty>::get_cycles() >= t)
                return -1;

            while (arch::cpu_traits<Ty>::get_cycles() < t);
            return 0;
        }

        static int type() 
        {
            return Ty;
        }        

        static
        const arch::cpu_traits<Ty>::cycles_type &
        Hz()
        {
            static arch::cpu_traits<Ty>::cycles_type hz = 
                static_cast<arch::cpu_traits<Ty>::cycles_type>(more::cpufreq(0).freq_hardware()) * 1000;

            return hz;
        }

        // update the Hz() value with that of the nth cpu.
        //

        static
        const arch::cpu_traits<Ty>::cycles_type & 
        Hz(int n)
        {
            arch::cpu_traits<Ty>::cycles_type hz = 
                static_cast<arch::cpu_traits<Ty>::cycles_type>(more::cpufreq(n).freq_hardware()) * 1000;
            std::swap(hz, const_cast< arch::cpu_traits<Ty>::cycles_type & >(Hz()));
            return Hz();
        }

    };


} // namespace arch
} // namespace more

#endif /* ARCH_CPU_HH */

