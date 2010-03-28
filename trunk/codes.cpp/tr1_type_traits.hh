#ifndef _TR1_TYPE_TRAITS_HH_
#define _TR1_TYPE_TRAITS_HH_ 

#if defined(MORE_USE_BOOST) || defined(__INTEL_COMPILER) 

#include <boost/tr1/type_traits.hpp>
namespace std { using namespace std::tr1; }

#elif defined(__GNUC__)

# if defined(__GXX_EXPERIMENTAL_CXX0X__)
# include <tr1/type_traits>
# include <type_traits>
# else
# include <tr1/type_traits>
namespace std { using namespace std::tr1; }
# endif

#endif

#endif /* _TR1_TYPE_TRAITS_HH_ */
