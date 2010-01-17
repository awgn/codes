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

#include <tr1/type_traits>
#include <iostream>
#include <fstream>
#include <iterator>
#include <string>

#include <iomanip.hh>

namespace more { namespace yacpp {

    // basic types..
    //

    struct line_type {};  // generic code line

    struct code_type : public line_type {};
    struct cpp_type  : public code_type {};
    struct lit_type  : public code_type {};

    struct comment_type {};

    // return true is pos is inside a literal string
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

    // find a substring in source code (not in literals)
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
        int _M_status;

    public:
        enum { status_code,
            status_comment,
            status_literal };

        cpp_decanter()
        : _M_status(status_code)
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

                switch(_M_status) 
                {
                case status_code:
                    {
                        std::string::size_type beg0 = c_find(line, "//");
                        std::string::size_type beg  = c_find(line, "/*");
                        std::string::size_type end  = c_find(line, "*/");
                
                        // detect a C++ comment...
                        if ( beg0 != std::string::npos ) {
                            _M_move(line, beg0, line.size(), __comment);
                            continue;
                        }

                        // detect a C comment (begin)
                        if ( beg == std::string::npos )
                            return;

                        // detect a C comment (end)
                        if ( end == std::string::npos ) {
                            _M_move(line, beg, line.size(), __comment);
                            _M_status = status_comment;
                            return; 
                        }

                        _M_move(line, beg, end+2, __comment);
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

                        _M_move(line, 0, end+2, __comment);
                        _M_status = status_code;

                    } break;
                }
            }
        }

    private:        
        void
        _M_move(std::string &from, std::string::size_type beg, std::string::size_type end, 
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
        : _M_stream(0), _M_decanter(), _M_line(), _M_ok(false)
        {}

        basic_iterator(std::istream &s)
        : _M_stream(&s), _M_decanter(), _M_line(), _M_ok(false)
        { _M_read(); } 

        basic_iterator(const basic_iterator &rhs)
        : _M_stream(rhs._M_stream),_M_decanter(rhs._M_decanter),
        _M_line(rhs._M_line), _M_ok(rhs._M_ok)
        {}

    private:
        std::istream       *_M_stream;
        cpp_decanter        _M_decanter;
        more::string_line   _M_line;
        bool                _M_ok;

        void _M_read()
        {
            std::string comment;
            _M_ok = (_M_stream && *_M_stream) ? true : false;
            if (_M_ok) {
                do { 
                    if (!(*_M_stream >> _M_line)) {
                        _M_ok = false;
                        break;
                    }
                    // split codeline from comments...
                    _M_decanter( static_cast<std::string &>(_M_line), comment);
                    more::trim(comment);
                    if ( std::tr1::is_same<Target, comment_type>::value )
                        std::swap(comment, static_cast<std::string &>(_M_line)); 
                }
                while( !more::trim(static_cast<std::string &>(_M_line)).size() || 
                       ( std::tr1::is_same<Target, cpp_type>::value && _M_line.str()[0] != '#' )
                     );
            }
        }

    public:
        const std::string &
        operator *() const
        { return _M_line; }

        const std::string *
        operator->() const { return &(operator*()); }

        basic_iterator &
        operator++()
        {
            _M_read();
            return *this; 
        }

        basic_iterator 
        operator++(int)
        {  
            basic_iterator __tmp = *this; 
            _M_read();         
            return __tmp;
        }

        bool _M_equal(const basic_iterator &rhs) const
        {
            return (_M_ok == rhs._M_ok) && (!_M_ok || _M_stream == rhs._M_stream);
        }
    };

    typedef basic_iterator<line_type>     line_iterator;    // line iterator
    typedef basic_iterator<cpp_type>      cpp_iterator;     // c/c++ preprocessor iterator 
    typedef basic_iterator<comment_type>  comment_iterator; // c/c++ comment iterator 

    template <typename Tp> 
    inline bool
    operator==(const basic_iterator<Tp> &lhs, const basic_iterator<Tp> &rhs)
    {
        return lhs._M_equal(rhs);
    }

    template <typename Tp> 
    inline bool
    operator!=(const basic_iterator<Tp> &lhs, const basic_iterator<Tp> &rhs)
    {
        return !lhs._M_equal(rhs);
    }

}}

#endif /* _YACPP_HH_ */
