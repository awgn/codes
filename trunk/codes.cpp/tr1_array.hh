#ifndef _TR1_ARRAY_HH_
#define _TR1_ARRAY_HH_ 

#if defined(MORE_USE_BOOST) || defined(__INTEL_COMPILER) 

#include <boost/tr1/array.hpp>
namespace std { using namespace std::tr1; }

#elif defined(__GNUC__)

# if defined(__GXX_EXPERIMENTAL_CXX0X__)
# include <array>
# else
# include <tr1/array>
namespace std { using namespace std::tr1; }
# endif

#endif

#endif /* _TR1_ARRAY_HH_ */
