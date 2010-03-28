#ifndef _TR1_RANDOM_HH_
#define _TR1_RANDOM_HH_ 

#if defined(MORE_USE_BOOST) || defined(__INTEL_COMPILER) 

#include <boost/tr1/random.hpp>
namespace std { using namespace std::tr1; }

#elif defined(__GNUC__)

# if defined(__GXX_EXPERIMENTAL_CXX0X__)
# include <random>
# else
# include <tr1/random>
namespace std { using namespace std::tr1; }
# endif

#endif

#endif /* _TR1_RANDOM_HH_ */
