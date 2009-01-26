/* $Id$ */
/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <bonelli@antifork.org> wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return. Nicola Bonelli 
 * ----------------------------------------------------------------------------
 */
#ifndef PROBE_HH
#define PROBE_HH

#include <iostream>

namespace generic 
{

#define probe_dump(x) std::cout << x << " :" << this << ": " << __PRETTY_FUNCTION__ << '\n'

    struct probe 
    {
        probe() 
        { 
            probe_dump("[C]"); 
        }
        probe(const probe &) 
        { 
            probe_dump("[!]"); 
        } 
        probe &operator=(const probe &) 
        { 
            probe_dump("[!]");
            return *this; 
        } 

        template <typename T1>
        probe(const T1 &) 
        { 
            probe_dump("[c]"); 
        } 
        template <typename T1, typename T2>
        probe(const T1 &, const T2 &) 
        {
            probe_dump("[c]"); 
        }
        template <typename T1, typename T2, typename T3>
        probe(const T1 &, const T2 &, const T3 &) 
        { 
            probe_dump("[c]"); 
        }
        template <typename T1, typename T2, typename T3,
                  typename T4>
        probe(const T1 &, const T2 &, const T3 &,
              const T4 &) 
        { 
            probe_dump("[c]"); 
        }
        template <typename T1, typename T2, typename T3,
                  typename T4, typename T5>
        probe(const T1 &, const T2 &, const T3 &,
              const T4 &, const T5 &) 
        { 
            probe_dump("[c]"); 
        }
        template <typename T1, typename T2, typename T3,
                  typename T4, typename T5, typename T6>
        probe(const T1 &, const T2 &, const T3 &,
              const T4 &, const T5 &, const T6 &) 
        { 
            probe_dump("[c]"); 
        }

        ~probe() 
        { 
            probe_dump("[d]"); 
        }

        friend std::ostream &
        operator<<(std::ostream &o, const probe &that) 
        {
            o << "<<probe:" << &that << ">>";
            return o;
        } 

    };

} // namespace generic

#endif /* PROBE_HH */

