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

#include <cpufreq.h>        // libcpufreq
#include <timeval.hh>       // more
#include <lexical_cast.hh>  // more

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

    template <int n> struct asm_policy;

    template <>
    struct asm_policy<ia386> 
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
    struct asm_policy<x86_64> 
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
    struct asm_policy<ia64>
    {
    #ifdef ARCH_IA64
        typedef unsigned long long cycles_t;
        static const cycles_t max = ULLONG_MAX;

#define ia64_getreg(num) ({ \
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
    struct asm_policy<parisc>
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

    // libcpufreq wrapper...
    //

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

        typedef arch::asm_policy<Ty>::cycles_t cycles_t;

        static 
        arch::asm_policy<Ty>::cycles_t 
        get_cycles()
        {
            return arch::asm_policy<Ty>::get_cycles();
        }

        static int wait_until(const arch::asm_policy<Ty>::cycles_t &t)
        {
            if (arch::asm_policy<Ty>::get_cycles() >= t)
                return -1;

            while (arch::asm_policy<Ty>::get_cycles() < t);
            return 0;
        }

        static int type() 
        {
            return Ty;
        }        

        /************************************** 
                 cpufreq library wrapper 
         **************************************/       

        cpu(int n)
        : _M_cpu(n)
        {
            if (cpufreq_cpu_exists(_M_cpu))
                throw std::runtime_error("cpu don't exist!");
        }

        ~cpu()
        {}

        /* determine current CPU frequency
         * - _kernel variant means kernel's opinion of CPU frequency
         * - _hardware variant means actual hardware CPU frequency,
         *    which is only available to root.
         *
         * returns 0 on failure, else frequency in kHz.
         */

        unsigned long
        freq_kernel() const
        {
            return cpufreq_get_freq_kernel(_M_cpu);
        }

        unsigned long
        freq_hardware() const
        {
            return cpufreq_get_freq_hardware(_M_cpu);
        }

        /* determine CPU transition latency
         *
         * returns 0 on failure, else transition latency in 10^(-9) s = nanoseconds
         */

        unsigned long
        transition_latency() const
        {
            return cpufreq_get_transition_latency(_M_cpu);
        }

        /* determine hardware CPU frequency limits
         *
         * These may be limited further by thermal, energy or other
         * considerations by cpufreq policy notifiers in the kernel.
         */

        std::pair<unsigned long, unsigned long>
        freq_hardware_limits() const
        {
            unsigned long min, max;
            cpufreq_get_hardware_limits(_M_cpu, &min, &max);
            return std::make_pair(min,max);
        }

        /* determine CPUfreq driver used
         *
         */

        std::string
        get_driver() const
        {
            std::tr1::shared_ptr<char> d( cpufreq_get_driver(_M_cpu), cpufreq_put_driver );
            return std::string(d.get()); 
        }

        /* determine CPUfreq policy currently used
         *
         */

        std::tr1::shared_ptr<const cpufreq_policy> 
        policy() const
        {
            return std::tr1::shared_ptr<const cpufreq_policy>(cpufreq_get_policy(_M_cpu), cpufreq_put_policy);
        } 

        /* determine CPUfreq governors currently available
         *
         * may be modified by modprobe'ing or rmmod'ing other governors. 
         */

        const std::list<std::string>
        available_governors() const
        {
            std::list<std::string> ret;
            std::tr1::shared_ptr<cpufreq_available_governors> 
                gov( cpufreq_get_available_governors(_M_cpu), cpufreq_put_available_governors);

            for( cpufreq_available_governors * p = gov.get() ;p != NULL; p=p->next)
                ret.push_back(p->governor);
            
            return ret; 
        }

        /* determine CPU frequency states available
         *
         * only present on _some_ ->target() cpufreq drivers. For information purposes
         * only. 
         */

        const std::list<unsigned long>
        available_frequencies() const
        {
            std::list<unsigned long> ret;
            std::tr1::shared_ptr<cpufreq_available_frequencies> 
                q ( cpufreq_get_available_frequencies(_M_cpu), cpufreq_put_available_frequencies);
            for( cpufreq_available_frequencies * p = q.get() ;p != NULL; p=p->next)
            {
                ret.push_back(p->frequency);
            }
            return ret; 
        }

        /* determine stats for cpufreq subsystem
         *
         * This is not available in all kernel versions or configurations.
         */

        std::tr1::shared_ptr<const cpufreq_stats>
        get_stats(unsigned long long *total_time) const
        {
            return std::tr1::shared_ptr<const cpufreq_stats>(cpufreq_get_stats(_M_cpu, total_time), cpufreq_put_stats);
        }

        unsigned long
        get_transition() const
        {
            return cpufreq_get_transitions(_M_cpu);
        }
     
        /* modify a policy by only changing min/max freq or governor 
         *
         * Does not check whether result is what was intended.
         */

        void
        set_policy_min_freq(unsigned long value)
        {
            if( cpufreq_modify_policy_min(_M_cpu, value) != 0)
                throw std::runtime_error("cpufreq_modify_policy_min");

        }
        void
        set_policy_mx_freq(unsigned long value)
        {
            if( cpufreq_modify_policy_max(_M_cpu, value) != 0)
                throw std::runtime_error("cpufreq_modify_policy_max");

        }

        void
        set_policy_governor(const std::string &value)
        {
            if( cpufreq_modify_policy_governor(_M_cpu, const_cast<char *>(value.c_str())) != 0)
                throw std::runtime_error("cpufreq_modify_policy_governor");
        }

        /* set a specific frequency
         *
         * Does only work if userspace governor can be used and no external
         * interference (other calls to this function or to set/modify_policy) 
         * occurs. Also does not work on ->range() cpufreq drivers.
         */

        void 
        set_frequency(unsigned long target_frequency)
        {
            if(cpufreq_set_frequency(_M_cpu,target_frequency) != 0)
               throw std::runtime_error("cpufreq_set_frequency"); 
        }

    private:
        int _M_cpu;
    
    };


} // namespace arch
} // namespace more

#endif /* ARCH_CPU_HH */

