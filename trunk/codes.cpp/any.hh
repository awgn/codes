/* $Id$ */
/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <bonelli@antifork.org> wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return. Nicola Bonelli
 * ----------------------------------------------------------------------------
 */

#ifndef _ANY_HH_
#define _ANY_HH_ 

#include <tr1_type_traits.hh>   // more!

#include <string>
#include <typeinfo>
#include <algorithm> 

// Yet another boost tribute: the class any.
//

namespace more { 

    class bad_any_cast : public std::bad_cast
    {
        public:
            virtual const char * what() const throw()
            {
                return "more::any_cast<> error";
            }
    };

    class any
    {
    public:

        any()
        : _M_base(0)
        {}

        template <typename T>
        any(const T &value)
        : _M_base( new storage<T>(value) )
        {} 

        any(const any &rhs)
        : _M_base( rhs._M_base ? rhs._M_base->clone() : 0 )
        {}

        ~any()
        { delete _M_base; }

    public:

        bool empty() const
        { 
            return _M_base == 0; 
        }

        const std::type_info &
        type() const
        {
            return _M_base ? _M_base->type() : typeid(void);
        }

        any & swap(any & rhs)
        {
            std::swap(_M_base, rhs._M_base);
            return *this;
        }

        template <typename T>
        any & operator=(const T &rhs)
        {
            any(rhs).swap(*this);
            return *this;
        }

        any & operator=(any rhs)
        {
            rhs.swap(*this);
            return *this;
        }

    private:

        template <typename T>
        friend T * any_cast(any *);

        struct base
        {
            virtual ~base()
            {}

            virtual base * clone() const = 0;
            virtual const std::type_info & type() const = 0;

        };

        template <typename T>
        struct storage : public base
        {
            storage(const T &value)
            : _M_value(value)
            {}

            storage *
            clone() const
            {
               return new storage(_M_value); 
            }

            const std::type_info &
            type() const
            {
                return typeid(T);
            }

            T _M_value;
        };

        base * _M_base;

    };

    template <typename T>
    inline T * any_cast(any *rhs_p)
    {
        return ( rhs_p ?  
                    (dynamic_cast< any::storage<T> *>(rhs_p->_M_base) ? 
                        & static_cast<any::storage<T> *>(rhs_p->_M_base)->_M_value 
                        : 
                        0) 
                     : 
                     0 
               );
    }
    template <typename T>
    inline const T * any_cast(const any *rhs_p)
    {
        return any_cast<T>( const_cast<any *>(rhs_p) );
    }

    template <typename T>
    inline T any_cast(any &rhs)
    {
        typedef typename std::remove_const< typename std::remove_reference<T>::type >::type Type;

        Type * p = any_cast<Type>(&rhs);
        if (!p)
            throw bad_any_cast();

        return *p;
    }
    template <typename T>
    inline T any_cast(const any &rhs)
    {
        typedef typename std::remove_reference<T>::type Type;
        return any_cast<const Type &>(const_cast<any &>(rhs));
    }

    // ... and a tribute to Bjorn Karlsson [see the book: Beyond the C++ Standard Library]
    //

    class any_out
    {
    public:

        any_out()
        : _M_value(), _M_streamer(0)
        {}

        template <typename T>
        any_out(const T &value)
        : _M_value(value),
          _M_streamer( new streamer_impl<T> )
        {}

        any_out(const any_out &rhs)
        :  _M_value( rhs._M_value ),
           _M_streamer( rhs._M_streamer ? rhs._M_streamer->clone() : 0 )
        {}

        template <typename T>
        any_out& operator=(const T & rhs)
        {
            any_out(rhs).swap(*this);
            return *this;
        }

        any_out& operator=(any_out rhs)
        {
            rhs.swap(*this);
            return *this;    
        }

        ~any_out()
        {
            delete _M_streamer;
        }

        any_out & swap(any_out &rhs)
        {
            std::swap(_M_value, rhs._M_value);
            std::swap(_M_streamer, rhs._M_streamer);
            return *this;
        }

        friend std::ostream & operator<<(std::ostream &out, const any_out &value)
        {
            if (value._M_streamer) {
                value._M_streamer->printon(out,value._M_value);
            }
            return out;
        }

        any &
        get() 
        { return _M_value; }

        const any &
        get() const
        { return _M_value; }

        private:

        struct streamer
        {
            virtual void printon(std::ostream &out, const any &arg)=0;
            virtual streamer * clone()=0;
            virtual ~streamer() {}
        };

        template <typename T> struct streamer_impl : public streamer {
            
            virtual void printon(std::ostream &out, const any &arg)
            {
                out << * any_cast<T>(&arg);
            }

            virtual streamer * clone() {
                return new streamer_impl<T>();
            }
        };

    private:

        any _M_value;
        streamer * _M_streamer;
    };

} // namespace more

#endif /* _ANY_HH_ */
