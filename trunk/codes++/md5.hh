/* $Id$ */
/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <bonelli@antifork.org> wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return. Nicola Bonelli
 * ----------------------------------------------------------------------------
 */


#ifndef _MD5_HH_
#define _MD5_HH_ 

#include <tr1/array>
#include <tr1/functional>

#include <iostream>
#include <iomanip>
#include <sstream>
#include <cstring>
#include <stdexcept>
#include <algorithm>
#include <iterator>

#include <stdint.h>
#include <mtp.hh>

namespace more { 

    using std::tr1::array;
    using namespace std::tr1::placeholders;

    class md5 : public std::iterator<std::output_iterator_tag, void, void, void, void> 
    {
    public:
        typedef array<uint8_t,16>::const_iterator const_iterator; 

    private:
        array<uint32_t,2>   _M_total;
        array<uint32_t,4>   _M_state;
        array<uint8_t,64>   _M_buffer;

        array<uint8_t,16>   _M_digest;

        bool _M_initialized;

        void process(const uint8_t data[64]);
        void update (const uint8_t *input, uint32_t length );
        void finish (uint8_t digest[16]);

    public:

        md5()
        : _M_total(), _M_state(), _M_buffer(), _M_digest(), _M_initialized(false)
        { 
            this->init(); 
        }

        ~md5()
        {}

        /////////////////////////////////////////////////////////////////////////
        // cipher functor interface

        void 
        operator()(const char &c)
        {
            this->update(reinterpret_cast<const uint8_t *>(&c),1);
        }
        void 
        operator()(const unsigned char &c)
        {
            this->update(reinterpret_cast<const uint8_t *>(&c),1);
        }

        void 
        operator()(const uint8_t *buf, uint32_t buflen)
        {
            this->update(buf,buflen);
        }

        void
        operator()(const std::string &str)
        {
            this->update(reinterpret_cast<const uint8_t *>(str.c_str()), str.length());
        }

        template <typename I>
        void operator()(I first, I last)
        {
            std::for_each(first, last, 
                std::tr1::bind
                    (
                    static_cast<void (md5::*)(const typename std::iterator_traits<I>::value_type &)> (&md5::operator()), 
                    std::tr1::ref(*this), _1) 
                    );
        }

        template <typename T>
        void operator()(const T &cont)
        {
            this->operator()(cont.begin(), cont.end());
        }

        /////////////////////////////////////////////////////////////////////////
        //

        void init()
        {
            _M_total[0] = 0;
            _M_total[1] = 0;

            _M_state[0] = 0x67452301;
            _M_state[1] = 0xEFCDAB89;
            _M_state[2] = 0x98BADCFE;
            _M_state[3] = 0x10325476;

            _M_initialized = true;
        }

        void
        finish()
        { 
            this->finish(_M_digest.begin());
        }

        const_iterator 
        digest_begin() const
        { 
            return _M_digest.begin(); 
        } 
        
        const_iterator 
        digest_end() const
        { 
            return _M_digest.end(); 
        } 

        template <typename T, int W, char C = '0'>
        struct hex_ostream_iterator : public std::iterator<std::output_iterator_tag, void, void, void, void>
        {
            std::ostream &_M_out;
            const std::string _M_sep;

            hex_ostream_iterator(std::ostream &out, const std::string &sep)
            : _M_out(out),
              _M_sep(sep)
            {
                _M_out << std::hex;
            }
            
            ~hex_ostream_iterator()
            {
                _M_out << std::dec;
            }
            
            hex_ostream_iterator &
            operator=(T c)
            {
                _M_out << std::setw(W) << std::setfill(C) << std::right << c << _M_sep;
                return *this;
            }

            hex_ostream_iterator &
            operator *()
            { return *this; }

            hex_ostream_iterator &
            operator++()
            { return *this; }

            hex_ostream_iterator &
            operator++(int)
            { return *this; }
        };

        std::string
        digest_str(const std::string & sep = "" ) const
        {
            std::ostringstream digest;
            std::copy(_M_digest.begin(), _M_digest.end(), hex_ostream_iterator<uint32_t,2,'0'>(digest,sep));
            return digest.str();
        }

        /////////////////////////////////////////////////////////////////////////
        // md5 object is itself an output iterator:

        md5 & 
        operator=(unsigned char c)
        {
            this->operator()(&c, 1);
            return *this; 
        }

        md5 & 
        operator=(const std::string &str)
        {
            this->operator()(str);
            return *this; 
        }

        md5 & 
        operator *()
        { return *this; }

        md5 &
        operator++()
        { return *this; }

        md5 &
        operator++(int)
        { return *this; }

   };

    ////////////////////////////////////////////////////////////////////////////////////////
    // streambuf support: 
    //  it does provide the correct overloading for std::streambuf * by means of disable_if

    static inline 
    md5 &  
    operator<<(md5 & cipher, std::streambuf * sb)
    {
        // std::cout << __PRETTY_FUNCTION__ << std::endl;
        cipher = std::copy( std::istreambuf_iterator<char>(sb),
                            std::istreambuf_iterator<char>(),
                            cipher);
        return cipher;
    }

    template <typename T>
    static inline
    typename mtp::disable_if<
        std::tr1::is_base_of
            <
            std::streambuf, // to provide a better overloading for std::basic_stringbuf<char, std::char_traits<char>, std::allocator<char> > 
            typename std::tr1::remove_pointer<T>::type
            >,
        md5 &
    >::type
    operator<<(md5 & cipher, const T & object)
    {
        // std::cout << __PRETTY_FUNCTION__ << std::endl;
        std::ostringstream out;
        out << object;
        cipher(out.str());
        return cipher;
    } 
    

    //////////////////////////////////
    // md5_adaptor: md5 csum functor

    template <class T>
    struct md5_adaptor
    {
        md5_adaptor()
        {}

        ~md5_adaptor()
        {}

        template <typename I>
        T operator()(I first, I last)
        {
            T ret; md5 cipher;
            cipher(first, last);
            cipher.finish();
            std::copy(cipher.digest_begin(), cipher.digest_end(), std::back_inserter(ret)); 
            return ret;
        }

        template <typename C>
        T operator()(const C & cont)
        {
            T ret; md5 cipher;
            cipher(cont);
            cipher.finish();
            std::copy(cipher.digest_begin(), cipher.digest_end(), std::back_inserter(ret)); 
            return ret;
        }        

    };
    
} // namespace more

#endif /* _MD5_HH_ */