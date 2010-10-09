/* $Id$ */
 
/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <bonelli@antifork.org> wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return. Nicola Bonelli
 * ----------------------------------------------------------------------------
 */

#ifndef _CPU_CYCLES_HPP_
#define _CPU_CYCLES_HPP_

#ifndef __GNUC__
#error compiler not supported!
#endif

namespace more { namespace this_cpu { 

    // assembly policies are taken from the linux kernel 2.6/include/arch-.../
    //
 
    namespace detail 
    {
#if defined(__i386__) && !defined(__LP64__)    
            typedef unsigned long long cycles_t;

            static cycles_t get_cycles() 
            {
                cycles_t val;
                __asm__ __volatile__("rdtsc" : "=A" (val));
                return val;
            }
#endif

#if defined(__LP64__)    
            typedef unsigned long long cycles_t;
        
            static const cycles_t get_cycles() 
            {
                cycles_t val;
                unsigned long __a,__d;
                __asm__ __volatile__("rdtsc" : "=a" (__a), "=d" (__d));
                val = ((unsigned long)__a) | (((unsigned long long)__d)<<32);
                return val;
            }
#endif

#if defined(__ia64__)
            typedef unsigned long long cycles_t;

            static inline cycles_t get_cycles ()
            {
                cycles_t val;
                __asm__ __volatile__ ("mov %o=ar%1" : "=r" (val) : "i" (44));  
                return val;
            }
#endif
    } // detail

        typedef detail::cycles_t cycles_type;

        static inline cycles_type 
        get_cycles()
        {
            return detail::get_cycles();
        }

        static inline
        bool busywait_until(const cycles_type &t)
        {
            if (detail::get_cycles() >= t)
                return false;
            while (detail::get_cycles() < t)
            {}
            return true;
        }
        
        static inline
        bool busywait_for(const cycles_type &d)
        {
            return busywait_until(detail::get_cycles() + d);
        } 

} // namespace this_cpu
} // namespace more

#endif /* _CPU_CYCLES_HPP_ */
