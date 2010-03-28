#ifndef _TR1_REGEX_HH_
#define _TR1_REGEX_HH_ 

#if defined(MORE_USE_BOOST) || defined(__INTEL_COMPILER) 

#include <boost/tr1/regex.hpp>
namespace std { using namespace std::tr1; }

#elif defined(__GNUC__)

# if defined(__GXX_EXPERIMENTAL_CXX0X__)
# include <regex>
# else
# include <tr1/regex>
namespace std { using namespace std::tr1; }
# endif

#endif

#endif /* _TR1_REGEX_HH_ */
