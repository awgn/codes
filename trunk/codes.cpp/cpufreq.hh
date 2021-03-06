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

#ifndef _CPUFREQ_HH_ 
#define _CPUFREQ_HH_ 

#include <cpufreq.h>        // libcpufreq

#include <tr1/memory>    
#include <cstring>
#include <cstdio>
#include <string>
#include <stdexcept>
#include <utility>
#include <list>

namespace more {

    class cpufreq 
    {
   public:

        /************************************** 
                 cpufreq library wrapper 
         **************************************/       

        cpufreq(int n)
        : m_cpu(n)
        {
            if (cpufreq_cpu_exists(m_cpu))
                throw std::runtime_error("cpu don't exist!");
        }

        ~cpufreq()
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
            return cpufreq_get_freq_kernel(m_cpu);
        }

        unsigned long
        freq_hardware() const
        {
            return cpufreq_get_freq_hardware(m_cpu);
        }

        /* determine CPU transition latency
         *
         * returns 0 on failure, else transition latency in 10^(-9) s = nanoseconds
         */

        // unsigned long
        // transition_latency() const
        // {
        //     return cpufreq_get_transition_latency(m_cpu);
        // }

        /* determine hardware CPU frequency limits
         *
         * These may be limited further by thermal, energy or other
         * considerations by cpufreq policy notifiers in the kernel.
         */

        std::pair<unsigned long, unsigned long>
        freq_hardware_limits() const
        {
            unsigned long min, max;
            cpufreq_get_hardware_limits(m_cpu, &min, &max);
            return std::make_pair(min,max);
        }

        /* determine CPUfreq driver used
         *
         */

        std::string
        get_driver() const
        {
            std::tr1::shared_ptr<char> d( cpufreq_get_driver(m_cpu), cpufreq_put_driver );
            return std::string(d.get()); 
        }

        /* determine CPUfreq policy currently used
         *
         */

        std::tr1::shared_ptr<const cpufreq_policy> 
        policy() const
        {
            return std::tr1::shared_ptr<const cpufreq_policy>(cpufreq_get_policy(m_cpu), cpufreq_put_policy);
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
                gov( cpufreq_get_available_governors(m_cpu), cpufreq_put_available_governors);

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
                q ( cpufreq_get_available_frequencies(m_cpu), cpufreq_put_available_frequencies);
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
            return std::tr1::shared_ptr<const cpufreq_stats>(cpufreq_get_stats(m_cpu, total_time), cpufreq_put_stats);
        }

        unsigned long
        get_transition() const
        {
            return cpufreq_get_transitions(m_cpu);
        }
     
        /* modify a policy by only changing min/max freq or governor 
         *
         * Does not check whether result is what was intended.
         */

        void
        set_policy_min_freq(unsigned long value)
        {
            if( cpufreq_modify_policy_min(m_cpu, value) != 0)
                throw std::runtime_error("cpufreq_modify_policy_min");

        }
        void
        set_policy_mx_freq(unsigned long value)
        {
            if( cpufreq_modify_policy_max(m_cpu, value) != 0)
                throw std::runtime_error("cpufreq_modify_policy_max");

        }

        void
        set_policy_governor(const std::string &value)
        {
            if( cpufreq_modify_policy_governor(m_cpu, const_cast<char *>(value.c_str())) != 0)
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
            if(cpufreq_set_frequency(m_cpu,target_frequency) != 0)
               throw std::runtime_error("cpufreq_set_frequency"); 
        }

    private:
        int m_cpu;
    
    };

} // namespace more


#endif /* _CPUFREQ_HH_ */
