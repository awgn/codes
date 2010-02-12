/* $Id$ */
/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <bonelli@antifork.org> wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return. Nicola Bonelli 
 * ----------------------------------------------------------------------------
 */

#ifndef _VARIANT_HH_
#define _VARIANT_HH_ 

#include <typeinfo>
#include <iostream>
#include <stdexcept>

#include <typelist.hh>
#include <mtp.hh>

// Yet another boost tribute: the class variant.
//

namespace more { 

    using namespace std;

    namespace variat_helper {

        struct none {};    

        template <typename T1, typename T2, bool value = (sizeof(T1) > sizeof(T2)) >
        struct size_max
        {
            typedef T1 value_type;
        };
        template <typename T1, typename T2>
        struct size_max<T1,T2,false>
        {
            typedef T2 value_type;
        };

        ///////////////////////////////////////////// type index 
        // 

        template <typename T, typename T0, typename T1, typename T2, typename T3, typename T4, typename T5>
        struct type_index
        {
            typedef TYPELIST(T0,T1,T2,T3,T4,T5) list_type;
            enum { value = more::TL::index_of<list_type, T>::value };
        };

        ///////////////////////////////////////////// store strategy 
        // 

        template <typename T, typename T0, typename T1, typename T2, typename T3, typename T4, typename T5>
        struct strategy_store
        {            
            static void set(T *that, const T0 &elem)
            {
                that->store(elem);
                that->_M_type = 0;
            }            
            static void set(T *that, const T1 &elem)
            {
                that->store(elem);
                that->_M_type = 1; 
            }
            static void set(T *that, const T2 &elem)
            {
                that->store(elem);
                that->_M_type = 2;
            }
            static void set(T *that, const T3 &elem)
            {
                that->store(elem);
                that->_M_type = 3;
            }
            static void set(T *that, const T4 &elem)
            {
                that->store(elem);
                that->_M_type = 4;
            }
            static void set(T *that, const T5 &elem)
            {
                that->store(elem);
                that->_M_type = 5;
            }
 
        };

        template <typename T, typename T0>
        struct strategy_store<T, T0, none, none, none, none, none>
        {
            static void set(T *that, const T0 &elem)
            {
                that->store(elem);
                that->_M_type = 0;
            }
        };

        template <typename T, typename T0, typename T1>
        struct strategy_store<T, T0, T1, none, none, none, none>
        {
            static void set(T *that, const T0 &elem)
            {
                that->store(elem);
                that->_M_type = 0;
            }            
            static void set(T *that, const T1 &elem)
            {
                that->store(elem);
                that->_M_type = 1;
            }
        };

        template <typename T, typename T0, typename T1, typename T2>
        struct strategy_store<T, T0, T1, T2, none, none, none>
        {
            static void set(T *that, const T0 &elem)
            {
                that->store(elem);
                that->_M_type = 0;
            }            
            static void set(T *that, const T1 &elem)
            {
                that->store(elem);
                that->_M_type = 1;
            }
            static void set(T *that, const T2 &elem)
            {
                that->store(elem);
                that->_M_type = 2;
            }
        };

        template <typename T, typename T0, typename T1, typename T2, typename T3>
        struct strategy_store<T, T0, T1, T2, T3, none, none>
        {
            static void set(T *that, const T0 &elem)
            {
                that->store(elem);
                that->_M_type = 0;
            }            
            static void set(T *that, const T1 &elem)
            {
                that->store(elem);
                that->_M_type = 1;
            }
            static void set(T *that, const T2 &elem)
            {
                that->store(elem);
                that->_M_type = 2;
            }            
            static void set(T *that, const T3 &elem)
            {
                that->store(elem);
                that->_M_type = 3;
            }
        };

        template <typename T, typename T0, typename T1, typename T2, typename T3, typename T4>
        struct strategy_store<T, T0, T1, T2, T3, T4, none>
        {
            static void set(T *that, const T0 &elem)
            {
                that->store(elem);
                that->_M_type = 0;
            }            
            static void set(T *that, const T1 &elem)
            {
                that->store(elem);
                that->_M_type = 1;
            }
            static void set(T *that, const T2 &elem)
            {
                that->store(elem);
                that->_M_type = 2;
            }            
            static void set(T *that, const T3 &elem)
            {
                that->store(elem);
                that->_M_type = 3;
            }            
            static void set(T *that, const T4 &elem)
            {
                that->store(elem);
                that->_M_type = 4;
            }

        };

    }   // namespace variant_helper

    template <typename T0, typename T1 = variat_helper::none, 
                           typename T2 = variat_helper::none,
                           typename T3 = variat_helper::none,
                           typename T4 = variat_helper::none,
                           typename T5 = variat_helper::none
                           >
    class variant {

    public:
        friend class variat_helper::strategy_store<variant,T0,T1,T2,T3,T4,T5>;

        template <typename CharT, typename Traits>
        friend std::basic_ostream<CharT,Traits> &
        operator<<(std::basic_ostream<CharT,Traits> &out, const more::variant<T0,T1,T2,T3,T4,T5> & var)
        {
            var.printon(out);
            return out; 
        }

        variant()
        : _M_type(0)
        {
            this->default_ctor<T0>(_M_storage);
        }

        template <typename T>
        variant(const T &value)
        : _M_type(0)
        {
            this->default_ctor<T0>(_M_storage);
            try {
                variat_helper::strategy_store<variant,T0,T1,T2,T3,T4,T5>::set(this,value);
            }
            catch(...)
            {
                this->dtor(_M_storage, 0);
                throw;
            }
        }

        ~variant()
        {
            this->dtor(_M_storage, _M_type);
        }

        variant(const variant & rhs)
        : _M_type(0)
        {
            this->default_ctor<T0>(_M_storage);
            try {
                switch(rhs._M_type) 
                {
                case 0: variat_helper::strategy_store<variant,T0,T1,T2,T3,T4,T5>::set(this, rhs.get<T0>()); break;
                case 1: variat_helper::strategy_store<variant,T0,T1,T2,T3,T4,T5>::set(this, 
                                       rhs.get<typename  mtp::if_< std::tr1::is_same<T1, variat_helper::none>::value, T0, T1>::type>()); break;
                case 2: variat_helper::strategy_store<variant,T0,T1,T2,T3,T4,T5>::set(this, 
                                       rhs.get<typename  mtp::if_< std::tr1::is_same<T2, variat_helper::none>::value, T0, T2>::type>()); break;
                case 3: variat_helper::strategy_store<variant,T0,T1,T2,T3,T4,T5>::set(this, 
                                       rhs.get<typename  mtp::if_< std::tr1::is_same<T3, variat_helper::none>::value, T0, T3>::type>()); break;
                case 4: variat_helper::strategy_store<variant,T0,T1,T2,T3,T4,T5>::set(this, 
                                       rhs.get<typename  mtp::if_< std::tr1::is_same<T4, variat_helper::none>::value, T0, T4>::type>()); break;
                case 5: variat_helper::strategy_store<variant,T0,T1,T2,T3,T4,T5>::set(this, 
                                       rhs.get<typename  mtp::if_< std::tr1::is_same<T5, variat_helper::none>::value, T0, T5>::type>()); break;
                }
            }
            catch(...)
            {
                this->dtor(_M_storage, 0);
                throw;
            }

            this->_M_type = rhs._M_type;
        }

        template <typename T>
        variant &operator=(const T& rhs)
        {
            variat_helper::strategy_store<variant, T0,T1,T2,T3,T4,T5>::set(this, rhs);
            return *this;
        }

        variant &operator=(const variant<T0,T1,T2> &rhs)
        {
            switch(rhs._M_type) 
            {
                case 0: this->store( rhs.get<T0>() ); break;
                case 1: this->store( rhs.get<T1>() ); break;
                case 2: this->store( rhs.get<T2>() ); break;
                case 3: this->store( rhs.get<T3>() ); break;
                case 4: this->store( rhs.get<T4>() ); break;
                case 5: this->store( rhs.get<T5>() ); break;
            }

            _M_type = rhs._M_type;
            return *this;
        }

        bool 
        empty() const 
        {
            return false;   // never empty
        }

        int
        which() const
        { return _M_type; }

        int
        storage_size() const
        {
            return sizeof(_M_storage)/sizeof(_M_storage[0]);
        }

        const std::type_info &
        type() const
        {
            switch(_M_type)
            {
                case 0: return typeid(T0);
                case 1: return typeid(T1);
                case 2: return typeid(T2);
                case 3: return typeid(T3);
                case 4: return typeid(T4);
                case 5: return typeid(T5);
            }

            return typeid(int); // unreachable
        }

        template <typename T>
        T & get()
        {
            if ( variat_helper::type_index<T,T0,T1,T2,T3,T4,T5>::value != _M_type )
                throw std::bad_cast();

            return *reinterpret_cast<T *>(_M_storage);
        }

        template <typename T>
        const T & get() const
        {
            if ( variat_helper::type_index<T,T0,T1,T2,T3,T4,T5>::value != _M_type )
                throw std::bad_cast();

            return *reinterpret_cast<const T *>(_M_storage);
        }

        template <typename F>
        void apply_visitor(F cw)
        {            
            switch(_M_type)
            {
                case 0: cw(this->get<T0>()); break;
                case 1: cw(this->get< typename  mtp::if_< std::tr1::is_same<T1, variat_helper::none>::value, T0, T1>::type >()); break;
                case 2: cw(this->get< typename  mtp::if_< std::tr1::is_same<T2, variat_helper::none>::value, T0, T2>::type >()); break;
                case 3: cw(this->get< typename  mtp::if_< std::tr1::is_same<T3, variat_helper::none>::value, T0, T3>::type >()); break;
                case 4: cw(this->get< typename  mtp::if_< std::tr1::is_same<T4, variat_helper::none>::value, T0, T4>::type >()); break;
                case 5: cw(this->get< typename  mtp::if_< std::tr1::is_same<T5, variat_helper::none>::value, T0, T5>::type >()); break;
            }    
        }

    private:

        template <typename V>
        void store(const V &value)
        {    
            char storage_tmp[_S_storage_size];

            // copy construct the _M_storage content in storage_tmp:
            this->copy(_M_storage, storage_tmp, _M_type);

            // destroy the object in the _M_storage (as _M_type) ...
            this->dtor(_M_storage, _M_type);

            try 
            {
                // try to construct the new value in _M_storage
                new (_M_storage) V(value);
            }
            catch(...)
            {
                // try to copy construct the storage_tmp back in the _M_storage:
                // since an excpetion could be thrown it is not possibile to provide strong guarantee
                // of exception safety. 

                try 
                {
                    this->copy(storage_tmp, _M_storage, _M_type);
                }
                catch(...) 
                {
                    throw std::logic_error("exception-safety: basic guarantee!");
                }

                // destroy storage_tmp...
                this->dtor(storage_tmp, _M_type);

                throw;  // rethrow the exception 
            }

            // destroy storage_tmp...
            this->dtor(storage_tmp, _M_type); 
        }

        template <typename T> 
        void default_ctor(char *storage)
        {
            new(storage)T;
        }

        void default_ctor(char *storage, int type)
        {
            switch(type)
            {
                case 0: new(storage)T0; return;
                case 1: new(storage)T1; return;
                case 2: new(storage)T2; return; 
                case 3: new(storage)T3; return;
                case 4: new(storage)T4; return;
                case 5: new(storage)T5; return;
            }    
        }

        void dtor(char *storage, int type)
        {
            switch(type)
            {
                case 0: reinterpret_cast<T0 *>(storage)->~T0(); break;
                case 1: reinterpret_cast<T1 *>(storage)->~T1(); break;
                case 2: reinterpret_cast<T2 *>(storage)->~T2(); break;
                case 3: reinterpret_cast<T3 *>(storage)->~T3(); break;
                case 4: reinterpret_cast<T4 *>(storage)->~T4(); break;
                case 5: reinterpret_cast<T5 *>(storage)->~T5(); break;
            }    
        }

        void copy(char *from, char *to, int type)
        {
            switch(type)
            {
                case 0: new (to) T0( *reinterpret_cast<T0 *>(from) ); break;
                case 1: new (to) T1( *reinterpret_cast<T1 *>(from) ); break;
                case 2: new (to) T2( *reinterpret_cast<T2 *>(from) ); break;
                case 3: new (to) T3( *reinterpret_cast<T3 *>(from) ); break;
                case 4: new (to) T4( *reinterpret_cast<T4 *>(from) ); break;
                case 5: new (to) T5( *reinterpret_cast<T5 *>(from) ); break;
            }    
        }

        void printon(std::ostream &out) const
        {
             switch(_M_type)
            {
                case 0:  out << this->get<T0>(); break;
                case 1:  out << this->get< typename  mtp::if_< std::tr1::is_same<T1, variat_helper::none>::value, T0, T1>::type >(); break;
                case 2:  out << this->get< typename  mtp::if_< std::tr1::is_same<T2, variat_helper::none>::value, T0, T2>::type >(); break;
                case 3:  out << this->get< typename  mtp::if_< std::tr1::is_same<T3, variat_helper::none>::value, T0, T3>::type >(); break;
                case 4:  out << this->get< typename  mtp::if_< std::tr1::is_same<T4, variat_helper::none>::value, T0, T4>::type >(); break;
                case 5:  out << this->get< typename  mtp::if_< std::tr1::is_same<T5, variat_helper::none>::value, T0, T5>::type >(); break;
            }    
        }

        static const int _S_storage_size = 
            sizeof( typename variat_helper::size_max< 
                        T0, 
                        typename variat_helper::size_max<
                            T1,
                            typename variat_helper::size_max<
                                T2,
                                typename variat_helper::size_max<
                                    T3,
                                    typename variat_helper::size_max<
                                        T4,
                                        T5
                                    >::value_type
                                >::value_type
                            >::value_type
                        >::value_type 
                     >::value_type
                  );

        int  _M_type;
        char _M_storage[_S_storage_size];
    };

} // namespace more


#endif /* _VARIANT_HH_ */
