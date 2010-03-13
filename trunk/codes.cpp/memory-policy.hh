/* $Id$ */
/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <bonelli@antifork.org> wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return. Nicola Bonelli
 * ----------------------------------------------------------------------------
 */

#ifndef MEMORY_POLICY_HH
#define MEMORY_POLICY_HH

#include <cstdlib>
#include <stdexcept>

// memory policies
//
namespace memory {

    template <class T, class E = int>
        struct New 
        {
            static T *
            alloc() 
            { return new T; }

            static T *
            alloc(const E &e) 
            { return new T(e); }

            static void 
            dealloc(T *obj) 
            { delete obj; }
        };

    template <class T, class E = int>
        struct Static 
        {
            static T *
            alloc() 
            {
                static T ret;
                return &ret;
            }
            
            static T *
            alloc(const E &e) 
            {
                static T ret(e);
                return &ret;
            }

            static void 
            dealloc(T *) 
            {}
        };

    template <class T, class E = int>
        struct Malloc 
        {
            static T *
            alloc() 
            {
                void * place = malloc(sizeof(T));
                if (place == NULL)
                    throw std::runtime_error("malloc");
                T * r = new (place)T();
                return r;
            }

            static T *
            alloc(const E &e) 
            {
                void * place = malloc(sizeof(T));
                if (place == NULL)
                    throw std::runtime_error("malloc");
                T * r = new (place)T(e);
                return r;
            }

            static void dealloc(T *obj) 
            {
                obj->~T();
                free(obj);
            }
        };

}

#endif /* MEMORY_POLICY_HH */
