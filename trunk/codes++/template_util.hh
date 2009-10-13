/* $Id$ */
/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <bonelli@antifork.org> wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return. Nicola Bonelli 
 * ----------------------------------------------------------------------------
 */

#include <iostream>

namespace more {

    namespace {
        
        struct null_type {};

        /////////////////////// tag 

        template <int n>
        struct tag
        {
            enum { value = n };
        };

        /////////////////////// count_param

        // -- count_param_1

        template <typename T1> 
        struct count_param_1 
        { 
            enum { value = 1 }; 
        };
        template <> 
        struct count_param_1<null_type> 
        { 
            enum { value = 0 }; 
        };

        // -- count_param_2

        template <typename T1, typename T2> 
        struct count_param_2 
        { 
            enum { value = 2 }; 
        };
        template <typename T1> 
        struct count_param_2<T1, null_type> 
        { 
            enum { value = 1 }; 
        };
        template <> 
        struct count_param_2<null_type, null_type> 
        { 
            enum { value = 0 }; 
        };

        // -- count_param_3

        template <typename T1, typename T2, typename T3> 
        struct count_param_3 
        { 
            enum { value = 3 }; 
        };
        template <typename T1, typename T2> 
        struct count_param_3<T1, T2, null_type> 
        { 
            enum { value = 2 }; 
        };
        template <typename T1> 
        struct count_param_3<T1, null_type, null_type> 
        { 
            enum { value = 1 }; 
        };
        template <> 
        struct count_param_3<null_type, null_type, null_type> 
        { 
            enum { value = 0 }; 
        };

        // -- count_param_4

        template <typename T1, typename T2, typename T3, typename T4> 
        struct count_param_4 
        { 
            enum { value = 4 }; 
        };        
        template <typename T1, typename T2, typename T3> 
        struct count_param_4<T1, T2, T3, null_type> 
        { 
            enum { value = 3 }; 
        };
        template <typename T1, typename T2> 
        struct count_param_4<T1, T2, null_type, null_type> 
        { 
            enum { value = 2 }; 
        };
        template <typename T1> 
        struct count_param_4<T1, null_type, null_type, null_type> 
        { 
            enum { value = 1 }; 
        };
        template <> 
        struct count_param_4<null_type, null_type, null_type, null_type> 
        { 
            enum { value = 0 }; 
        };

        // -- count_param_5

        template <typename T1, typename T2, typename T3, typename T4, typename T5> 
        struct count_param_5 
        { 
            enum { value = 5 }; 
        };          
        template <typename T1, typename T2, typename T3, typename T4> 
        struct count_param_5<T1, T2, T3, T4, null_type> 
        { 
            enum { value = 4 }; 
        };        
        template <typename T1, typename T2, typename T3> 
        struct count_param_5<T1, T2, T3, null_type, null_type> 
        { 
            enum { value = 3 }; 
        };
        template <typename T1, typename T2> 
        struct count_param_5<T1, T2, null_type, null_type, null_type> 
        { 
            enum { value = 2 }; 
        };
        template <typename T1> 
        struct count_param_5<T1, null_type, null_type, null_type, null_type> 
        { 
            enum { value = 1 }; 
        };
        template <> 
        struct count_param_5<null_type, null_type, null_type, null_type, null_type> 
        { 
            enum { value = 0 }; 
        };

        // -- count_param_6

        template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6> 
        struct count_param_6 
        { 
            enum { value = 6 }; 
        };          
        template <typename T1, typename T2, typename T3, typename T4, typename T5> 
        struct count_param_6<T1, T2, T3, T4, T5, null_type> 
        { 
            enum { value = 5 }; 
        };        
        template <typename T1, typename T2, typename T3, typename T4> 
        struct count_param_6<T1, T2, T3, T4, null_type, null_type> 
        { 
            enum { value = 4 }; 
        };        
        template <typename T1, typename T2, typename T3> 
        struct count_param_6<T1, T2, T3, null_type, null_type, null_type> 
        { 
            enum { value = 3 }; 
        };
        template <typename T1, typename T2> 
        struct count_param_6<T1, T2, null_type, null_type, null_type, null_type> 
        { 
            enum { value = 2 }; 
        };
        template <typename T1> 
        struct count_param_6<T1, null_type, null_type, null_type, null_type, null_type> 
        { 
            enum { value = 1 }; 
        };
        template <> 
        struct count_param_6<null_type, null_type, null_type, null_type, null_type, null_type> 
        { 
            enum { value = 0 }; 
        };

    }
} 
