/* $Id$ */
/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <bonelli@antifork.org> wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return. Nicola Bonelli 
 * ----------------------------------------------------------------------------
 */

#ifndef CACHE_HH
#define CACHE_HH

#include <sys/time.h>
#include <time.h>

#include <iostream>
#include <stdexcept>
#include <string>
#include <map>

namespace more 
{
    template <class KEY, class VALUE>
    class cache {
    public:
        template <typename T>
        class proxy {

        public:
            typedef T value_type;

            proxy(const T &r = T(), int t=0)
            : m_data(r),
              m_timeout(t),
              m_timestamp()
            { this->ts_update(); }

            proxy(const proxy &rhs)
            {
                m_data = rhs.m_data;
                m_timeout = rhs.m_timeout;
                this->ts_update();
            }

            proxy &
            operator=(const proxy &rhs)
            {
                if ( &m_data == &rhs )
                    return *this;
                this->m_data = rhs.m_data;
                this->m_timeout = rhs.m_timeout;
                this->ts_update();
                return *this;
            }

            proxy &
            operator=(const T &rhs)
            {
                m_data = rhs;
                this->ts_update();
                return *this;
            }

            T *
            operator&()
            { return &m_data; }

            const T *
            operator&() const
            { return &m_data; }

            operator T &()
            { return m_data; }

            operator const T &() const
            { return m_data; }

            void
            ts_update(int timeo = -1)
            {
                struct timeval now;
                gettimeofday(&now,NULL);
                m_timestamp = now.tv_sec;
                m_timeout = (timeo == -1 ? m_timeout : timeo);
            }

            bool
            is_expired() const
            {
                if (m_timeout == 0)
                    return false;   // never expires
                struct timeval now;
                gettimeofday(&now,NULL);
                return static_cast<unsigned int>(now.tv_sec) > (m_timestamp + m_timeout);
            }

        private:
            T m_data;
            unsigned int m_timeout;
            unsigned int m_timestamp;
        };

        struct expired {
            VALUE * pimp;
            expired(VALUE &t)
            : pimp(&t)
            {}
        };

        typedef std::map<KEY, proxy<VALUE> > cache_type;
        typedef typename std::map<KEY,proxy<VALUE> >::iterator iterator;
        typedef typename std::map<KEY,proxy<VALUE> >::const_iterator const_iterator;

        cache() 
        : m_db()
        {}

        ~cache(){}

        VALUE *
        insert(KEY k, VALUE v, int t = 0) 
        {
            typename cache_type::iterator it = m_db.find(k);
            if ( it != m_db.end() && !it->second.is_expired() ) 
                throw std::runtime_error("key already in use!");
            m_db[k] = proxy<VALUE>(v, t);
            return & m_db[k];
        }

        VALUE *
        update(KEY k, VALUE v, int t = 0) 
        {
            m_db[k] = proxy<VALUE>(v, t);
            m_db[k].ts_update();
            return & m_db[k];
        }

        VALUE *
        update(KEY k, int t = 0) 
        {
            typename cache_type::iterator it = m_db.find(k);
            if ( it == m_db.end() )
                throw std::runtime_error("key not found!");
            it->second.ts_update(t);
            return & m_db[k]; 
        }

        VALUE *
        find(KEY k, bool update = false) 
        {
            typename cache_type::iterator it = m_db.find(k);
            if( it == m_db.end()) 
                throw std::runtime_error("key not found!");
            if (it->second.is_expired()) {
                throw expired(it->second);
            }
            if (update)
                it->second.ts_update();
            return & it->second;
        }

        bool 
        has_key(KEY k, bool update = false) 
        {
            typename cache_type::iterator it = m_db.find(k);
            if ( it == m_db.end())
                return false;
            if (it->second.is_expired())
                return false;
            if (update)
                it->second.ts_update();
            return true;
        }

        int 
        erase(KEY k) 
        { return m_db.erase(k); }

        // iterators...
        //

        typename cache_type::iterator 
        begin() 
        { return  m_db.begin(); }

        typename cache_type::iterator 
        end() 
        { return m_db.end(); }

        typename cache_type::const_iterator 
        begin() const 
        { return  m_db.begin(); }

        typename cache_type::const_iterator 
        end() const 
        { return m_db.end(); }

    private:
        cache_type m_db;
    };

} // namespace more

#endif /* CACHE_HH */
