/* $Id$ */
/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <bonelli@antifork.org> wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return. Nicola Bonelli
 * ----------------------------------------------------------------------------
 */
 
#ifndef _TOKEN_HPP_
#define _TOKEN_HPP_ 

#include <cctype>
#include <string>

namespace more { 

    template <typename Pred>
    struct basic_token
    {
        operator const std::string &() const
        {
            return _M_str;
        }

        template <typename CharT, typename Traits>
        friend inline std::basic_istream<CharT, Traits> & 
        operator>>(std::basic_istream<CharT,Traits> &in, basic_token &rhs)
        {   
            typedef typename std::basic_istream<CharT,Traits>::int_type int_type;
            typedef typename std::basic_istream<CharT,Traits>::ios_base ios_base;

            const int_type eof = Traits::eof();
            typename ios_base::iostate err = ios_base::goodbit; 

            rhs._M_str.erase();

            Pred is_token;

            // skip delimiters:
            int_type c = in.rdbuf()->sgetc();
            while( !Traits::eq_int_type(c, eof) && !is_token(c) )
            {
                c = in.rdbuf()->snextc();
            } 

            if ( Traits::eq_int_type(c, eof) ) {
                err |= ios_base::failbit;
                in.setstate(err);
                return in;
            } 

            rhs._M_str.append(1,Traits::to_char_type(c));
            while ( !(c = in.rdbuf()->snextc(), Traits::eq_int_type(c,eof))  && 
                    is_token(c) )
            {
                rhs._M_str.append(1,Traits::to_char_type(c));
            }

            if ( Traits::eq_int_type(c,eof) ) {
                err |= ios_base::eofbit;
                in.width(0);
                in.setstate(err);            
            }
            return in;
        }

    private:
        std::string _M_str;    
    };

} // namespace more

#endif /* _TOKEN_HPP_ */
