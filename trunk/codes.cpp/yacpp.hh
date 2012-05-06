/* $Id$ */
/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <bonelli@antifork.org> wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return. Nicola Bonelli 
 * ----------------------------------------------------------------------------
 */

#ifndef _YACPP_HH_ 
#define _YACPP_HH_

// Yet another simple C/C++ parser
// 

#include <iomanip.hh>           // more!

#include <tr1/type_traits>   
#include <iostream>
#include <fstream>
#include <iterator>
#include <string>
#include <algorithm>
#include <functional>

namespace more { namespace yacpp {

    // basic types..
    //

    struct line_type {};  // generic code line

    struct code_type : public line_type {};
    struct cpp_type  : public code_type {};
    struct lit_type  : public code_type {};

    struct comment_type {};

    // predicate that return true if pos is inside a literal
    //

    static inline
    bool is_literal(const std::string &__that, std::string::size_type __pos)
    {
        if (__pos >= __that.size())
            return false;

        const char *__data = __that.data();
        bool ret = false;
        for(std::string::size_type n = 0; n <= __pos; ++n)
        {
            if (__data[n] == '"' &&  
                ( n == 0 || __data[n-1] != '\\' )) 
                ret ^= true; 
        }
        return ret;
    }

    // find the substring in source code (and not in literals)
    //

    static inline
    std::string::size_type
    c_find(const std::string &__str, const char *__s, std::string::size_type __pos = std::string::npos)
    {
        do {
            __pos = __str.find(__s, __pos+1);
        }
        while( __pos != std::string::npos && is_literal(__s,__pos) );
        return __pos;
    }

    // functional to separate c/c++ comments from source lines 
    //

    struct cpp_decanter : std::unary_function<std::string &, bool>
    {
    private:        
        int m_status;

    public:
        enum { status_code,
            status_comment,
            status_literal };

        cpp_decanter()
        : m_status(status_code)
        {}

        ~cpp_decanter()
        {}

        void 
        operator()(std::string &line, const std::string &com = std::string()) 
        {
            // comments are discarded if the caller is not interested in them... 
            std::string & __comment = const_cast<std::string &>(com);  

            __comment.clear();

            for(;;) { 

                switch(m_status) 
                {
                case status_code:
                    {
                        std::string::size_type beg0 = c_find(line, "//");
                        std::string::size_type beg  = c_find(line, "/*");
                        std::string::size_type end  = c_find(line, "*/");
                
                        // detect a C++ comment...
                        if ( beg0 != std::string::npos ) {
                            m_move(line, beg0, line.size(), __comment);
                            continue;
                        }

                        // detect a C comment (begin)
                        if ( beg == std::string::npos )
                            return;

                        // detect a C comment (end)
                        if ( end == std::string::npos ) {
                            m_move(line, beg, line.size(), __comment);
                            m_status = status_comment;
                            return; 
                        }

                        m_move(line, beg, end+2, __comment);
                        continue;
                    } break;

                case status_comment: 
                    {
                        std::string::size_type end = c_find(line, "*/");

                        if (end == std::string::npos) {
                            __comment.append(line);
                            line.clear();
                            return; 
                        }

                        m_move(line, 0, end+2, __comment);
                        m_status = status_code;

                    } break;
                }
            }
        }

    private:        
        void
        m_move(std::string &from, std::string::size_type beg, std::string::size_type end, 
                std::string &to)
        {
            std::copy(from.begin()+beg, from.begin()+end, std::back_inserter(to));
            from.erase(beg,end-beg);
        }

    };

    // basic_iterator for c/c++ source code lines
    //

    template <typename Target> 
    class basic_iterator : 
        public std::iterator<std::input_iterator_tag, std::string>
    {
    public:
        // end of input
        basic_iterator()
        : m_stream(0), m_decanter(), m_line(), m_ok(false)
        {}

        basic_iterator(std::istream &s)
        : m_stream(&s), m_decanter(), m_line(), m_ok(false)
        { m_read(); } 

        basic_iterator(const basic_iterator &rhs)
        : m_stream(rhs.m_stream), m_decanter(rhs.m_decanter),
          m_line(rhs.m_line), m_ok(rhs.m_ok)
        {}

    private:
        std::istream       *m_stream;
        cpp_decanter        m_decanter;
        more::string_line   m_line;
        bool                m_ok;

        void m_read()
        {
            std::string comment;
            m_ok = (m_stream && *m_stream) ? true : false;
            if (m_ok) {
                do { 
                    if (!(*m_stream >> m_line)) {
                        m_ok = false;
                        break;
                    }
                    // split codeline from comments...
                    m_decanter( static_cast<std::string &>(m_line), comment);
                    more::trim(comment);
                    if ( std::tr1::is_same<Target, comment_type>::value )
                        std::swap(comment, static_cast<std::string &>(m_line)); 
                }
                while( !more::trim(static_cast<std::string &>(m_line)).size() || 
                       ( std::tr1::is_same<Target, cpp_type>::value && m_line.str()[0] != '#' )
                     );
            }
        }

    public:
        const std::string &
        operator *() const
        { return m_line; }

        const std::string *
        operator->() const { return &(operator*()); }

        basic_iterator &
        operator++()
        {
            m_read();
            return *this; 
        }

        basic_iterator 
        operator++(int)
        {  
            basic_iterator __tmp = *this; 
            m_read();         
            return __tmp;
        }

        bool m_equal(const basic_iterator &rhs) const
        {
            return (m_ok == rhs.m_ok) && (!m_ok || m_stream == rhs.m_stream);
        }
    };

    typedef basic_iterator<line_type>     line_iterator;    // line iterator
    typedef basic_iterator<cpp_type>      cpp_iterator;     // c/c++ preprocessor iterator 
    typedef basic_iterator<comment_type>  comment_iterator; // c/c++ comment iterator 

    template <typename Tp> 
    inline bool
    operator==(const basic_iterator<Tp> &lhs, const basic_iterator<Tp> &rhs)
    {
        return lhs.m_equal(rhs);
    }

    template <typename Tp> 
    inline bool
    operator!=(const basic_iterator<Tp> &lhs, const basic_iterator<Tp> &rhs)
    {
        return !lhs.m_equal(rhs);
    }

}}

#endif /* _YACPP_HH_ */
