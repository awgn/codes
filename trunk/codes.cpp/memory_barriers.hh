/* $Id$ */
/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <bonelli@antifork.org> wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return. Nicola Bonelli 
 * ----------------------------------------------------------------------------
 */

#ifndef _MEMORY_BARRIERS_HH_
#define _MEMORY_BARRIERS_HH_ 

#ifdef __GNUC__
#  ifdef _GLIBCXX_READ_MEM_BARRIER
#  define MORE_MEMORY_READ_BARRIER  _GLIBCXX_READ_MEM_BARRIER
#  define MORE_MEMORY_WRITE_BARRIER _GLIBCXX_WRITE_MEM_BARRIER
#  else
#  define MORE_MEMORY_READ_BARRIER  __asm __volatile ("":::"memory") 
#  define MORE_MEMORY_WRITE_BARRIER __asm __volatile ("":::"memory") 
#  endif
#elif defined(_MSC_VER) && (_MSC_VER >= 1310)
#  define MORE_MEMORY_READ_BARRIER  _ReadWriteBarrier()
#  define MORE_MEMORY_WRITE_BARRIER _ReadWriteBarrier()
#else
#  error memory barrier not available
#endif

namespace more { 
    
    // memory barrier helper functions...
    //

    static inline void memory_barrier()
    {
        MORE_MEMORY_WRITE_BARRIER;
    }
    static inline void memory_read_barrier()
    {
        MORE_MEMORY_READ_BARRIER;
    }
    static inline void memory_write_barrier()
    {
        MORE_MEMORY_WRITE_BARRIER;
    }

} // namespace more

#endif /* _MEMORY_BARRIERS_HH_ */
