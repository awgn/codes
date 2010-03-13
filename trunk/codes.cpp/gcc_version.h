#ifndef _GCC_VERSION_HH_
#define _GCC_VERSION_HH_ 

#ifndef GCC_VERSION
#define GCC_VERSION (__GNUC__ * 10000 \
                     + __GNUC_MINOR__ * 100 \
                     + __GNUC_PATCHLEVEL__)
#endif

#endif /* _GCC_VERSION_HH_ */
