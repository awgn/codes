/* $Id$ */
/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <bonelli@antifork.org> wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return. Nicola Bonelli 
 * ----------------------------------------------------------------------------
 */

#ifndef _TIMEVAL_HPP_
#define _TIMEVAL_HPP_

#include <sys/time.h>

#include <iostream>
#include <algorithm>

namespace more { 

    class Timeval
    {
    public:

        typedef void(Timeval::*bool_type)();

        Timeval()
        : m_tv()
        {}

        Timeval(time_t sec, suseconds_t usec)
        {
            m_tv.tv_sec = sec;
            m_tv.tv_usec = usec;    
        }

        ~Timeval()
        {}

        Timeval(const Timeval& rhs)
        {
            m_tv.tv_sec = rhs.tv_sec();
            m_tv.tv_usec = rhs.tv_usec();
        }

        explicit Timeval(const struct timeval& rhs)
        {
            m_tv.tv_sec = rhs.tv_sec;
            m_tv.tv_usec = rhs.tv_usec;
        }

        explicit Timeval(const struct timespec& rhs)
        {
            m_tv.tv_sec = rhs.tv_sec;
            m_tv.tv_usec = rhs.tv_nsec/1000;
        }
        
        void 
        swap(Timeval &rhs) 
        {   
            std::swap(m_tv.tv_sec, rhs.m_tv.tv_sec);
            std::swap(m_tv.tv_usec, rhs.m_tv.tv_usec);
        }

        Timeval & operator=(const Timeval &rhs)
        { 
            m_tv.tv_sec = rhs.tv_sec();
            m_tv.tv_usec = rhs.tv_usec();
            return *this;
        }

        Timeval & operator=(const timeval &rhs)
        { 
            m_tv.tv_sec = rhs.tv_sec;
            m_tv.tv_usec = rhs.tv_usec;
            return *this;
        }

        Timeval & operator=(const timespec &rhs)
        { 
            m_tv.tv_sec = rhs.tv_sec;
            m_tv.tv_usec = rhs.tv_nsec/1000;
            return *this;
        }

        Timeval & operator+=(const Timeval &rhs)
        {   
            m_tv.tv_sec +=  rhs.tv_sec();
            m_tv.tv_usec += rhs.tv_usec();

            if (m_tv.tv_usec >= 1000000) {
                ++m_tv.tv_sec;
                m_tv.tv_usec -= 1000000;
            }
            return *this;
        }

        friend const Timeval operator+(Timeval lhs, const Timeval &rhs)
        { return lhs+=rhs; }

        Timeval & operator-=(const Timeval &rhs)
        {   
            m_tv.tv_sec -=  rhs.tv_sec();
            m_tv.tv_usec -= rhs.tv_usec();

            if (m_tv.tv_usec < 0) {
                --m_tv.tv_sec;
                m_tv.tv_usec += 1000000;
            }
            return *this;
        }

        friend const Timeval operator-(Timeval lhs, const Timeval &rhs)
        { return lhs-=rhs; }

        time_t
        tv_sec() const
        {
            return m_tv.tv_sec;
        }
    
        suseconds_t
        tv_usec() const
        {
            return m_tv.tv_usec;
        }

        long long int 
        to_sec() const 
        { 
            return static_cast<long long int>(m_tv.tv_sec); 
        }
    
        long long int 
        to_msec() const 
        { 
            return static_cast<long long int>(m_tv.tv_sec)*1000 + static_cast<long long int>(m_tv.tv_usec)/1000; 
        } 
    
        long long int 
        to_usec() const 
        { 
            return static_cast<long long int>(m_tv.tv_sec)*1000000 + static_cast<long long int>(m_tv.tv_usec); 
        }
        
        void 
        update()
        {
            ::gettimeofday(&m_tv,0);
        }

        /////////// conversion

        operator bool_type() const
        {
            return (m_tv.tv_sec || m_tv.tv_usec) ? &Timeval::update : 0;
        }

        operator const timeval () const
        {
            return m_tv;
        }

        operator const timespec () const
        {
            timespec ret;
            ret.tv_sec  = m_tv.tv_sec;
            ret.tv_nsec = m_tv.tv_usec * 1000; 
            return ret;
        }

        /////////// static 

        static Timeval now()
        {
            struct timeval now; ::gettimeofday(&now, 0);
            return Timeval(now);
        }

    private:
        struct timeval m_tv;
    };

    static inline 
    bool operator==(const Timeval &lhs, const Timeval &rhs)
    {
        return lhs.tv_sec() == rhs.tv_sec() && lhs.tv_usec() == rhs.tv_usec();
    }
    static inline
    bool operator!=(const Timeval &lhs, const Timeval &rhs)
    {
        return !(lhs == rhs);
    }
    static inline
    bool operator<(const Timeval &lhs, const Timeval &rhs)
    {
        return lhs.tv_sec() < rhs.tv_sec() || ( lhs.tv_sec() == rhs.tv_sec() && lhs.tv_usec() < rhs.tv_usec());
    }    
    static inline
    bool operator>(const Timeval &lhs, const Timeval &rhs)
    {
        return lhs.tv_sec() > rhs.tv_sec() || ( lhs.tv_sec() == rhs.tv_sec() && lhs.tv_usec() > rhs.tv_usec());
    }    
    static inline
    bool operator<=(const Timeval &lhs, const Timeval &rhs)
    {
        return !(lhs > rhs); 
    }    
    static inline
    bool operator>=(const Timeval &lhs, const Timeval &rhs)
    {
        return !(lhs < rhs); 
    }    

    // global swap()
    static inline 
    void swap (Timeval& x, Timeval& y) 
    { x.swap(y); }

    template <typename CharT, typename Traits>
    inline std::basic_ostream<CharT,Traits> &
    operator<<(std::basic_ostream<CharT,Traits> &out, const more::Timeval &value)
    {
        out << "{ .sec=" << value.tv_sec() << 
        " .usec=" << value.tv_usec() << " }";
        return out;
    }

} // namespace more

#endif /* _TIMEVAL_HPP_ */
