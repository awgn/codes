/* ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <bonelli@antifork.org> wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return. Nicola Bonelli 
 * ----------------------------------------------------------------------------
 */

#ifndef _KEY_VALUE_HPP_
#define _KEY_VALUE_HPP_ 

#include <type_traits.hpp>      // more!
#include <typemap.hpp>          // more!
#ifdef LEXEME_DEBUG
#include <streamer.hpp>         // more!
#endif

#include <iostream>
#include <fstream>
#include <string>
#include <cstring>
#include <memory>
#include <functional>
#include <type_traits>
#include <tuple>
#include <limits>
#include <stdexcept>
#include <cassert>

#define MAP_KEY(t,k)  struct k { \
    typedef std::pair<k,t> type; \
    static const bool has_default = false; \
    static const char * str() \
    { return # k; } \
};

#define MAP_KEY_VALUE(t,k,v)  struct k { \
    typedef std::pair<k,t> type; \
    static const bool has_default = true; \
    static const char * str() \
    { return # k; } \
    static t default_value() \
    { return v; } \
};

//////////////////////////////////
//  key-value config file parser 

namespace more { 

    namespace details {

#ifdef LEXEME_DEBUG
        const char * const BOLD  = "\E[1m";
        const char * const RESET = "\E[0m";
        const char * const BLUE  = "\E[1;34m";
#endif

        template <class CharT, class Traits>
        inline
        std::basic_istream<CharT,Traits> &
        ignore_line(std::basic_istream<CharT,Traits> &in_)
        {
            in_.ignore(std::numeric_limits<std::streamsize>::max(), in_.widen('\n'));
            return in_;
        }
    
        class line_streambuf : public std::streambuf 
        {
            std::streambuf * m_in;
            int m_line;

        public:
            line_streambuf(std::streambuf *in)
            : m_in(in), m_line(1)
            {}

            virtual int_type underflow()
            {
                return  m_in->sgetc();
            }

            virtual int_type uflow()
            {
                int_type c = m_in->sbumpc();
                if (c == '\n')
                    m_line++;
                return c;
            }

            int line() const
            { return m_line; }
        };

        template <class CharT, class Traits>
        inline int line_number(std::basic_istream<CharT,Traits> &in)
        {
            line_streambuf * ln = dynamic_cast<line_streambuf *>(in.rdbuf());
            if (ln) {
                return ln->line();
            }
            return -1;
        }

        template <typename KEY, typename TYPE, bool has_default>
        struct get_default
        {
            static TYPE value()
            { return KEY::default_value(); }
        };
        template <typename KEY, typename TYPE>
        struct get_default<KEY, TYPE, false>
        {
            static TYPE value()
            { return TYPE(); }
        };

        // detail::tuple_helper<sizeof...(Ti)>::parse_lexeme(*this, tup) &&

        template <size_t N>
        struct tuple_helper
        {
            template <typename L, typename ...Ti>
            static bool parse_lexeme(L &lex,  std::tuple<Ti...> &tup)
            {
#ifdef LEXEME_DEBUG
            std::cout << __PRETTY_FUNCTION__ << std::endl;
#endif
                typename std::tuple_element<sizeof...(Ti)-N,
                                             typename std::tuple<Ti...>   
                                                >::type elem;

                bool ok = lex.parse_lexeme(elem);
                if (!ok)
                    return false;

                std::get<sizeof...(Ti) - N>(tup) = std::move(elem);
                return tuple_helper<N-1>::parse_lexeme(lex, tup);
            }
        };
        template <>
        struct tuple_helper<1>
        {
            template <typename L, typename ...Ti>
            static bool parse_lexeme(L &lex,  std::tuple<Ti...> &tup)
            {
#ifdef LEXEME_DEBUG
            std::cout << __PRETTY_FUNCTION__ << std::endl;
#endif
                typename std::tuple_element<sizeof...(Ti)-1,
                                             typename std::tuple<Ti...>   
                                                >::type elem;
                bool ok = lex.parse_lexeme(elem);
                if (ok)
                    std::get<sizeof...(Ti) - 1>(tup) = std::move(elem);
                return ok;
            }
        };

        // get a mutable version of a generic type
        // 

        template <typename T>
        struct mutable_type
        {
            typedef typename std::remove_const<T>::type type;
        };
        template <typename T, typename V> 
        struct mutable_type<std::pair<T,V>>
        {
            typedef std::pair<typename std::remove_const<T>::type,
                              typename std::remove_const<V>::type> type;
        };

        //////////////// insert helpers

        template <typename T>
        bool insert_check(const std::pair<T,bool> &p)
        {
            return p.second;
        }
        template <typename T>
        bool insert_check(const T &)
        {
            return true;
        }

        // enabled for std::vector, std::dequeue, std::list
        // 
        template <typename C, typename V>
        typename std::enable_if<!more::traits::has_key_type<C>::value && 
                                !more::traits::has_container_type<C>::value,bool>::type 
        insert(C &cont, V &&value)
        {
            cont.push_back(std::forward<V>(value));
            return true;
        }

        // enabled for std::stack, std::queue, std::priority_queue
        // 
        template <typename C, typename V>
        typename std::enable_if<!more::traits::has_key_type<C>::value && 
                                more::traits::has_container_type<C>::value,bool>::type 
        insert(C &cont, V &&value)
        {
            cont.push(std::forward<V>(value));
            return true;
        }

        // enabled for std::set, std::multiset, std::unordered_set,
        // std::unordered_multiset
        // 
        template <typename C, typename K>
        typename std::enable_if<more::traits::has_key_type<C>::value ,bool>::type 
        insert(C &cont, K && value)
        {
            return insert_check(cont.insert(std::forward<K>(value)));
        }

        // enabled for std::map, std::multimap, std::unordered_map,
        // std::unordered_multimap
        // 
        template <typename C, typename T, typename V>
        typename std::enable_if<more::traits::has_key_type<C>::value ,bool>::type 
        insert(C &cont, std::pair<T,V> && value)
        {
            return insert_check(cont.insert(std::move(value)));
        }

    } // namespace details

    /////////////////////////////////////////////////////////////////////////////////////////
    
    template <typename ...T> struct key_value_pack;

    typedef std::tuple<bool,        // whether it is strict or non strict parsing 
                       char,        // assign separator -> default =
                       char,        // comment          -> defualt #
                       std::string> parser_options;

    template <typename CharT, typename Traits>
    class lexer 
    {
        std::basic_istream<CharT, Traits> & m_in;
        parser_options m_option;

    public:

        lexer(std::basic_istream<CharT, Traits> &in, const parser_options &opt)
        : m_in(in), m_option(opt)
        {}

        ~lexer()
        {}

        bool _(char c)
        {
            char _c;
            if(!(m_in >> std::ws))
                return false;
            _c = m_in.peek();
            if (!m_in)
                return false;
            if (c == _c) {
                m_in >> _c;
                assert(m_in);
                return true;
            }
            return false;
        }

        bool _(const char *s)
        {
            std::string _s; return (m_in >> _s && _s.compare(s) == 0) ? true : false;
        }
        
        // skip comment lines

        void skip_comments()
        {
            for(;;) {
                m_in >> std::ws;
                if (m_in.peek() == std::get<2>(m_option)) {
                    m_in >> details::ignore_line;
                    continue;
                }
                break;
            }
        }

        // very generic parser for types supporting operator>> ...
        //
        
        template <typename T>
        inline 
        typename std::enable_if<!more::traits::is_container<T>::value,bool>::type 
        parse_lexeme(T &lex)
        { 
#ifdef LEXEME_DEBUG
            std::cout << __PRETTY_FUNCTION__ << std::endl;
#endif 

#if __GNUC__ == 4 &&  __GNUC_MINOR__ > 4
            static_assert(more::traits::has_extraction_operator<T>::value, "parse_lexeme: *** T must have a valid extraction operator>>() ***");
#endif
            typename std::remove_const<T>::type e;

            if(m_in >> e)
                lex = std::move(e);

#ifdef LEXEME_DEBUG
            std::cout << details::BLUE << ":: lex<T>[" << lex << "]" << details::RESET << std::endl;
#endif
            return m_in;
        }        

        // parser for string literal:    
        //

        inline bool parse_lexeme(const char * &lex)
        {
#ifdef LEXEME_DEBUG
            std::cout << __PRETTY_FUNCTION__ << std::endl;
#endif
            std::string s;
            if (!parse_lexeme(s))
               return false;
            lex = strdup(s.c_str());    
            return true;
        }

        // parser for raw pointer: 
        //

        template <typename T>
        inline bool parse_lexeme(T * &ptr)
        {
#ifdef LEXEME_DEBUG
            std::cout << __PRETTY_FUNCTION__ << std::endl;
#endif
            if (!ptr)
                ptr = new T;
            if (!parse_lexeme(*ptr))
                return false;
            return true;
        }

        // parser for shared_ptr<T>:
        //

        template <typename T>
        inline bool parse_lexeme(std::shared_ptr<T> &ptr)
        {
#ifdef LEXEME_DEBUG
            std::cout << __PRETTY_FUNCTION__ << std::endl;
#endif
            if (!ptr)
                ptr.reset(new T);
            if (!parse_lexeme(*ptr))
                return false;
            return true;
        }

        // parser for std::string:
        //
        inline bool parse_lexeme(std::string &lex)
        {
#ifdef LEXEME_DEBUG
            std::cout << __PRETTY_FUNCTION__ << std::endl;
#endif
            m_in >> std::noskipws >> std::ws;

            std::string str;
            str.reserve(32);

            typedef std::string::traits_type traits_type;
            traits_type::char_type c, quote = traits_type::char_type('\0');

            c = m_in.peek();

            if ( traits_type::eq(c, '"') ||
                 traits_type::eq(c, '\'')) {
                quote = c;
                m_in.get();
                c = m_in.peek();
#ifdef LEXEME_DEBUG
            std::cout << details::BLUE << ":: quote -> " << quote << std::endl;
#endif
            }

            while(c != traits_type::eof() && (
                    (quote && (c != quote)) ||
                        (!quote && ( std::isalnum(c) || traits_type::eq(c, '_') || traits_type::eq(c, '-'))) ||
                            traits_type::eq(c, '\\')
                            )) {

                   c = m_in.get(); // get char
                   if (c == '\\')
                        c = m_in.get();

                   str.push_back(c);
                   c = m_in.peek();
            }
            

            if (quote) {
                c = m_in.get();
                if (!traits_type::eq(c, quote)) { 
                    std::clog << "parse: error at string '" << str << "': missing quotation mark?!\n"; 
                    return false;
                }
            }

            lex = std::move(str);
            m_in >> std::skipws;

#ifdef LEXEME_DEBUG
            std::cout << details::BLUE << ":: lex<string>[" << lex << "]" << details::RESET << std::endl;
#endif
            return true;
        }


        // parser for boolean
        //

        inline bool parse_lexeme(bool &lex)
        {
#ifdef LEXEME_DEBUG
            std::cout << __PRETTY_FUNCTION__ << std::endl;
#endif
            m_in >> std::noboolalpha;
            if (!(m_in >> lex)) {
                m_in.clear();
                m_in >> std::boolalpha >> lex;
            }
#ifdef LEXEME_DEBUG
            std::cout << details::BLUE << ":: lex<bool>[" << lex << "]" << details::RESET << std::endl;
#endif
            return m_in;
        }

        // parser for generic pairs
        // 

        template <typename T, typename V>
        inline bool parse_lexeme(std::pair<T,V> &lex)
        { 
#ifdef LEXEME_DEBUG
            std::cout << __PRETTY_FUNCTION__ << std::endl;
#endif
            T first; V second;

            bool ok;
            if (_('('))
            {
                ok =  parse_lexeme(first)     &&
                      parse_lexeme(second)    &&
                      _(')');  
            }
            else {
                ok = _('[') &&
                     parse_lexeme(first)        &&
                     _("->")                    &&
                     parse_lexeme(second)       &&
                     _(']');
            }

            if (ok)
                lex = std::make_pair(first,second);
#ifdef LEXEME_DEBUG
            std::cout << details::BLUE << ":: lex<pair>[" << lex << "]" << details::RESET << std::endl;
#endif
            return ok;
        }

        // parser for generic tuple
        // 

        template <typename ...Ti>
        bool parse_lexeme(std::tuple<Ti...> &lex)
        {
#ifdef LEXEME_DEBUG
            std::cout << __PRETTY_FUNCTION__ << std::endl;
#endif
            std::tuple<Ti...> tup;

            bool ok = _('(') &&
                        details::tuple_helper<sizeof...(Ti)>::parse_lexeme(*this, tup) &&
                      _(')');

            if (ok)
                lex = std::move(tup);          
#ifdef LEXEME_DEBUG
            std::cout << details::BLUE << ":: lex<tuple>[" << lex << "]" << details::RESET << std::endl;
#endif
            return ok;
        }

        // parser for generic containers:
        //

        template <typename C>
        inline typename std::enable_if<more::traits::is_container<C>::value,bool>::type 
        parse_lexeme(C & lex)
        {
#ifdef LEXEME_DEBUG
            std::cout << __PRETTY_FUNCTION__ << std::endl;
#endif
            bool ok = true;
            if (!_('[')) 
                return false;
            
            do {
                skip_comments();

                if (_(']')) 
                    break;
        
                typename details::mutable_type<
                    typename C::value_type>::type value;

                ok = parse_lexeme(value);
                if (ok) {
                    if (!details::insert(lex,std::move(value))) {
                        std::clog << std::get<3>(m_option) << 
                            ": insert error (dup value at line " << 
                            details::line_number(m_in) << ")" << std::endl;
                        return false;
                    }
                }
            }
            while(ok);
#ifdef LEXEME_DEBUG
            std::cout << details::BLUE << ":: lex<C>[" << lex << "]" << details::RESET << std::endl;
#endif
            return ok;
        }
        
        // recursive parser for key_value_pack
        //
        template <typename ...Ti>
        bool parse_lexeme(key_value_pack<Ti...> &elem, bool bracket = true)
        {
#ifdef LEXEME_DEBUG
            std::cout << __PRETTY_FUNCTION__ << std::endl;
#endif
            m_in.unsetf(std::ios::dec);
            m_in.unsetf(std::ios::hex);
            m_in.unsetf(std::ios::oct);

            if (bracket &&  ! _('{')) {
                std::clog << std::get<3>(m_option) << 
                    ": parse error: missing open bracket (line " << 
                        details::line_number(m_in) << ")" << std::endl;
                return false;
            }

            key_value_pack<Ti...> tmp;
            
            while(m_in) {
                
                m_in >> std::noskipws >> std::ws;

                std::string key;
                key.reserve(16);

                // parse the key 
                //
                char c = '\0';

                while ((m_in >> c) && !isspace(c) && c != std::get<1>(m_option) ) {
                    key.push_back(c);
                }
                
                // skip comments/empty lines
                //
                if (key.empty() || key[0] == std::get<2>(m_option)) {
                    if (c != '\n') 
                        m_in >> details::ignore_line;
                    continue;    
                }

                if (bracket && !key.compare("}")) {
                    bracket = false;
                    break;
                }
#ifdef LEXEME_DEBUG
                std::cout << details::BOLD << ":: key[" << key << "]" << details::RESET << std::endl;
#endif
                m_in >> std::skipws;

                // parse separator ('=')
                //
                if (c != std::get<1>(m_option)) {
                    m_in >> c;
                    if (c != std::get<1>(m_option)) {
                        std::clog << std::get<3>(m_option) << ": parse error: key[" << key << "] missing separator '" 
                        << std::get<1>(m_option) << "' (line "<< details::line_number(m_in) << ")" << std::endl;
                        return false;
                    }
                }
                m_in >> std::ws;

                // parse value...
                // 
                if (!tmp.parse(m_in, key, m_option, *this)) 
                    return false;
            }

            if (bracket) { 
                std::clog << std::get<3>(m_option) << ": parse error: missing close bracket (line "<< details::line_number(m_in) << ")" << std::endl;
                return false;
            }

            // parsing correct...

            elem  = tmp;
            return true;
        }
    };

    template <typename CharT, typename Traits>
    lexer<CharT, Traits> 
    make_lexer(std::basic_istream<CharT, Traits> &in, const parser_options &opt)
    {
        return lexer<CharT, Traits>(in, opt);
    }
    
    //////////////////////////////////////////////////////////////////////////
    //   options 
    namespace key_value_opt {

        struct options
        {
            parser_options m_opt;

            options()
            : m_opt(false,'=', '#', std::string())
            {}

            options &
            strict() { std::get<0>(m_opt) = true; return *this; }

            options &
            non_strict() { std::get<0>(m_opt) = false; return *this; }
 
            options &
            separator(char c) { std::get<1>(m_opt) = c; return *this; }
            
            options &
            comment(char c) { std::get<2>(m_opt) = c; return *this; }

            operator parser_options()
            {
                return m_opt;
            }
        };

        static inline options
        strict() { return options().strict(); }

        static inline options
        non_strict() { return options().non_strict(); }

        static inline options
        separator(char c) { return options().separator(c); }
        
        static inline options
        comment(char c) { return options().comment(c); }
    }

    //////////////////////////////////////////////////////////////////////////
    //   parser: key_value_pack

    template <typename ...T> struct key_value_pack;

    template <typename T0, typename ...Ti>
    struct key_value_pack<T0, Ti...>
    {
    public:
        typedef more::type::typemap<typename T0::type, typename Ti::type...>    map_type;
        typedef typename T0::type::first_type                                   key_type;
        typedef typename T0::type::second_type                                  value_type;

        typedef key_value_pack<Ti...>                                           parser_type;

        parser_type  m_parser;
        key_type     m_key;
        value_type   m_value;

        key_value_pack()
        : m_parser(),
          m_key(), m_value(details::get_default<key_type, value_type, key_type::has_default>::value()) 
        {}
                          
        key_value_pack(const char *name, 
            const parser_options &mode = std::make_tuple(false, '=', '#', "pack")) 
        : m_parser(),
          m_key(), m_value(details::get_default<key_type, value_type, key_type::has_default>::value()) 
        {
            if(!this->open(name, mode))
                throw std::runtime_error("key_value_pack");
        }

        virtual ~key_value_pack()
        {}

        //////////////////////////////////////////////////////////////////////////
        // get method

        template <typename Key>
        typename std::add_lvalue_reference<typename more::type::get<map_type, Key>::type>::type
        get() 
        { return get_<Key>(std::integral_constant<int, more::type::index_of<map_type, Key>::value>()); }
        template <typename Key>

        typename std::add_lvalue_reference<
            typename std::add_const<
                typename more::type::get<map_type, Key>::type>::type>::type
        get() const
        { return const_cast<key_value_pack *>(this)->get_<Key>
            (std::integral_constant<int, more::type::index_of<map_type, Key>::value>()); }
        
        template <typename Key, int N>
        typename std::add_lvalue_reference<typename more::type::get<map_type, Key>::type>::type
        get_(std::integral_constant<int,N>) 
        { return m_parser.get_<Key>(std::integral_constant<int, N-1>()); }

        template <typename Key>
        typename std::add_lvalue_reference<value_type>::type
        get_(std::integral_constant<int,0>) 
        { return m_value; } 

    public:
        //////////////////////////////////////////////////////////////////////////
        // run-time parser 

        template <typename CharT, typename Traits>
        bool parse(std::basic_istream<CharT, Traits> &in, const std::string &key, 
                         const parser_options &mode, lexer<CharT, Traits> &lex)
        { return parse_(in, key, *this, mode, lex); }

        template <typename CharT, typename Traits, typename _T0, typename ..._Ti>
        static bool parse_(std::basic_istream<CharT, Traits> &in, const std::string &key, 
                                  key_value_pack<_T0, _Ti...> &that, const parser_options &mode, lexer<CharT, Traits> &lex)
        {
            if (key == _T0::type::first_type::str()) {
                
                if (!lex.parse_lexeme(that.m_value) || in.fail() ) {
                    std::clog << std::get<3>(mode) << ": parse error: key[" << 
                    _T0::type::first_type::str() << "] unexpected argument (line " << 
                    details::line_number(in) << ")" << std::endl;
                    return false;
                }
                return true;
            }
            return parse_(in, key, that.m_parser, mode, lex);
        }

        template <typename CharT, typename Traits>
        static bool parse_(std::basic_istream<CharT, Traits> &in, const std::string &key, 
                                  key_value_pack<> &, const parser_options &mode, lexer<CharT, Traits>&)
        {
            // unknown key-value...
            if (std::get<0>(mode)) {   // strict mode: dump-error 
                std::clog << std::get<3>(mode) << ": parse error: key[" << key << "] unknown (line " << 
                details::line_number(in) << ")" << std::endl;
                return false;
            }
            // non-strict mode: skip this line
            in >> details::ignore_line;
            return true;
        }

    public:
        bool 
        open(const char *name, parser_options mode = std::make_tuple(false, '=', '#')) 
        {
            std::ifstream sc(name);
            std::get<3>(mode) = std::string(name);
            
            if (!sc) {
                std::clog << name << ": parse error: no such file" << std::endl;
                return false;
            }

            details::line_streambuf sb(sc.rdbuf());
            std::istream in(&sb);    
            return open(in, mode);
        }

        template <typename CharT, typename Traits>
        bool open(std::basic_istream<CharT, Traits> &in, parser_options mode = std::make_tuple(false, '=', '#', "unnamed")) 
        {
            auto lex = make_lexer(in, mode);
            return lex.parse_lexeme(*this, false);
        }
    };

    template <>
    class key_value_pack<> {};

    template<typename T, typename ...Ti>
    inline 
    typename std::add_lvalue_reference<
        typename more::type::get<typename key_value_pack<Ti...>::map_type, T>::type>::type 
    get(key_value_pack<Ti...> &p) 
    {                                           
        return p.get<T>();
    }

    template<typename T, typename ...Ti>
    inline 
    typename std::add_lvalue_reference<
        typename std::add_const<
            typename more::type::get<typename key_value_pack<Ti...>::map_type, T>::type>::type>::type 
    get(const key_value_pack<Ti...> &p) 
    {       
        return p.get<T>();
    }

} // namespace more

#endif /* _KEYVALUE_HPP_ */
