#ifndef _TR1_UTILITY_HH_
#define _TR1_UTILITY_HH_ 

#if defined(MORE_USE_BOOST) || defined(__INTEL_COMPILER) 

#include <boost/tr1/utility.hpp>
namespace std { using namespace std::tr1; }

#elif defined(__GNUC__)

# if defined(__GXX_EXPERIMENTAL_CXX0X__)
# include <utility>
# else
# include <tr1/utility>
namespace std { using namespace std::tr1; }
# endif

#endif

#endif /* _TR1_UTILITY_HH_ */
