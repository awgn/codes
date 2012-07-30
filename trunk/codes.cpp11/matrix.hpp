/* $Id$ */
/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <bonelli@antifork.org> wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return. Nicola Bonelli
 * ----------------------------------------------------------------------------
 */
 
#ifndef _MATRIX_HPP_
#define _MATRIX_HPP_ 

#include <iostream>
#include <array>
#include <vector>
#include <stdexcept>
#include <algorithm>
#include <functional>
#include <iterator>
#include <numeric>
#include <limits>
#include <type_traits>
#include <initializer_list>

using namespace std::placeholders;

namespace more { 

    struct scoped_assert
    {
        scoped_assert() = default;

        scoped_assert(bool value, const char *what)
        {
            if (!value)
                throw std::runtime_error(what);
        }
    };

    ///////////////////////////////////////////////////////////////////////////////
    // matrix iterators
    ///////////////////////////////////////////////////////////////////////////////
 
    template <typename Tp, typename Iter, typename ConstIter>
    struct _base_matrix_iterators
    {
        typedef Iter                    iterator;
        typedef ConstIter               const_iterator;

        struct _const_col_iterator;

        struct _col_iterator : public std::iterator<std::bidirectional_iterator_tag, Tp>
        {
            friend struct _const_col_iterator;

            _col_iterator(size_t c)
            : m_it(), m_col(c)
            {}
 
            _col_iterator(const _col_iterator &rhs)
            : m_it(rhs.m_it), m_col(rhs.m_col)
            {}

            _col_iterator
            operator=(const _col_iterator &rhs)
            {
                m_it = rhs.m_it;
                m_col = rhs.m_col;
            }
 
            explicit _col_iterator(iterator it, size_t c)
            : m_it(it), m_col(c)
            {}

            Tp &
            operator*()
            {
                return *m_it;
            }

            Tp *
            operator->()
            {
                return &(*m_it);
            }

            _col_iterator
            operator++()
            {
                std::advance(m_it, m_col);
                return *this;
            }

            _col_iterator
            operator++(int)
            {
                _col_iterator ret(*this);
                ++(*this);
                return ret;
            }

            _col_iterator
            operator--()
            {
                std::advance(m_it, -m_col);
                return *this;
            }

            _col_iterator
            operator--(int)
            {
                _col_iterator ret(*this);
                --(*this);
                return ret;
            }

            bool
            operator==(const _col_iterator &rhs)
            {
                return m_it == rhs.m_it;
            }
            bool
            operator!=(const _col_iterator &rhs)
            {
                return m_it != rhs.m_it;
            }

        private:
            iterator m_it;
            size_t   m_col;
        };
 
        struct _const_col_iterator : public std::iterator<std::bidirectional_iterator_tag, Tp>
        {
            _const_col_iterator(size_t c)
            : m_it(), m_col(c)
            {}
 
            _const_col_iterator(const _const_col_iterator &rhs)
            : m_it(rhs.m_it), m_col(rhs.m_col)
            {}

            _const_col_iterator(const _col_iterator &rhs)
            : m_it(rhs.m_it), m_col(rhs.m_col)
            {}
 
            _const_col_iterator
            operator=(const _const_col_iterator &rhs)
            {
                m_it = rhs.m_it;
                m_col = rhs.m_col;
            }
 
            explicit _const_col_iterator(const_iterator it, size_t c)
            : m_it(it), m_col(c)
            {}

            const Tp &
            operator*() const
            {
                return *m_it;
            }

            const Tp *
            operator->() const
            {
                return &(*m_it);
            }

            _const_col_iterator
            operator++()
            {
                std::advance(m_it, m_col);
                return *this;
            }

            _const_col_iterator
            operator++(int)
            {
                _const_col_iterator ret(*this);
                ++(*this);
                return ret;
            }

            _const_col_iterator
            operator--()
            {
                std::advance(m_it, -m_col);
                return *this;
            }

            _const_col_iterator
            operator--(int)
            {
                _const_col_iterator ret(*this);
                --(*this);
                return ret;
            }

            bool
            operator==(const _const_col_iterator &rhs)
            {
                return m_it == rhs.m_it;
            }
            bool
            operator!=(const _const_col_iterator &rhs)
            {
                return m_it != rhs.m_it;
            }

        private:
            const_iterator m_it;
            size_t   m_col;
        };
 
    };

    ///////////////////////////////////////////////////////////////////////////////
    // static-size matrix
    ///////////////////////////////////////////////////////////////////////////////

    template <typename Tp, size_t R = 0 , size_t C = 0>
    class matrix : _base_matrix_iterators<Tp,typename std::array<Tp, R*C>::iterator, 
                                             typename std::array<Tp, R*C>::const_iterator>
    {
        static_assert(R > 0 && C > 0, "R,C must be > 0!");

    public:
        typedef Tp value_type;  
        typedef _base_matrix_iterators<Tp, 
                typename std::array<Tp, R*C>::iterator, 
                typename std::array<Tp, R*C>::const_iterator> base_type;

        typedef typename std::array<Tp, R*C>::iterator        iterator; 
        typedef typename std::array<Tp, R*C>::const_iterator  const_iterator; 
        typedef typename std::array<Tp, R*C>::iterator        row_iterator; 
        typedef typename std::array<Tp, R*C>::const_iterator  const_row_iterator; 
        typedef typename base_type::_col_iterator             col_iterator;
        typedef typename base_type::_const_col_iterator       const_col_iterator;

        static const int row_size = R;
        static const int col_size = C;

        matrix()
        : m_assert(), m_mat()
        {}

        matrix(std::initializer_list<Tp> lst)
        : m_assert(), m_mat()
        {
            scoped_assert(lst.size() == R * C, "matrix::op= size mismatch!");    
            std::copy(lst.begin(), lst.end(), m_mat.begin());
        }
        
        matrix(const matrix &rhs)
        : m_assert(), m_mat(rhs.m_mat)
        {}

        explicit matrix(const matrix<Tp,0,0> &rhs)
        : m_assert(rhs.row() == R && rhs.col() == C, "matrix: size mismatch!"), m_mat()
        {  
            // std::array is an aggregate type: therefore it does not support 
            // std::initializer_list constructor. Booooooooh!
            std::copy(rhs.begin(), rhs.end(), m_mat.begin());
        }

        matrix &
        operator=(const matrix &rhs)
        {
            std::copy(rhs.begin(), rhs.end(), m_mat.begin());
            return *this;
        }

        matrix &
        operator=(const matrix<Tp,0,0> &rhs)
        {
            scoped_assert(rhs.row() == R && rhs.col() == C, "matrix::op= size mismatch!");    
            std::copy(rhs.begin(), rhs.end(), m_mat.begin());
            return *this;
        }

        matrix & 
        operator=(std::initializer_list<Tp> lst)
        {
            scoped_assert( lst.size() == R * C, "matrix::op= size mismatch!");    
            std::copy(lst.begin(), lst.end(), m_mat.begin());
            return *this;
        }

        Tp &
        operator()(size_t r, size_t c)
        {
            scoped_assert( r < row() && c < col(), "matrix::op() bad index");
            return m_mat[r*C+c];
        }     

        const Tp &
        operator()(size_t r, size_t c) const
        {
            scoped_assert( r < row() && c < col(), "matrix::op() bad index");
            return m_mat[r*C+c];
        }

        // observers:
        //

        size_t row() const
        { return R; }

        size_t col() const
        { return C; }

        iterator 
        begin() 
        {
            return m_mat.begin();
        }
        const_iterator 
        begin() const
        {
            return m_mat.begin();
        }
        const_iterator 
        cbegin() const
        {
            return m_mat.begin();
        }

        iterator 
        end() 
        {
            return m_mat.end();
        }
        const_iterator 
        end() const
        {
            return m_mat.end();
        }
        const_iterator 
        cend() const
        {
            return m_mat.end();
        }
        
        //////// row_iterator:

        row_iterator 
        row_begin(size_t r) 
        {
            scoped_assert( r < row(), "matrix::row_iterator bad index");    
            return m_mat.begin() + (r * col());
        }
        
        const_row_iterator
        row_begin(size_t r) const       
        {
            scoped_assert( r < row(), "matrix::row_iterator bad index");    
            return m_mat.begin() + (r * col());
        }

        row_iterator 
        row_end(size_t r) 
        {
            scoped_assert( r < row(), "matrix::row_iterator bad index");    
            return m_mat.begin() + ((r+1) * col());
        }

        const_row_iterator
        row_end(size_t r) const
        {
            scoped_assert( r < row(), "matrix::row_iterator bad index");    
            return m_mat.begin() +((r+1) * col());
        }

        //////// col_iterator:

        col_iterator 
        col_begin(size_t c) 
        {
            scoped_assert( c < col(), "matrix::col_iterator bad index");    
            return col_iterator(m_mat.begin() + c, col());
        }
        
        const_col_iterator
        col_begin(size_t c) const
        {
            scoped_assert( c < col(), "matrix::col_iterator bad index");    
            return const_col_iterator(m_mat.begin() + c, col());
        }

        col_iterator 
        col_end(size_t c) 
        {
            scoped_assert( c < col(), "matrix::col_iterator bad index");    
            return col_iterator(m_mat.begin() + (c+R*C), col());
        }

        const_col_iterator
        col_end(size_t c) const
        {
            scoped_assert( c < col(), "matrix::col_iterator bad index");    
            return const_col_iterator(m_mat.begin() + (c+R*C), col());
        }
 
        ////////////////////////////////////////////

        bool 
        operator==(const matrix &rhs) const
        {
            return std::equal(m_mat.begin(), m_mat.end(), rhs.begin());   
        }
        bool 
        operator==(const matrix<Tp,0,0> &rhs) const
        {
            if (rhs.row() != R || rhs.col() != C)
                return false;
            return std::equal(m_mat.begin(), m_mat.end(), rhs.begin());   
        }

        bool 
        operator!=(const matrix &rhs) const
        {
            return !(*this == rhs);   
        }
        bool 
        operator!=(const matrix<Tp,0,0> &rhs) const
        {
            return !(*this == rhs);   
        }
 
        // +=

        matrix &
        operator+=(const matrix &rhs)
        {
            std::transform(m_mat.begin(), m_mat.end(), rhs.begin(), m_mat.begin(), std::plus<Tp>());
            return *this;
        }

        matrix &
        operator+=(const matrix<Tp,0,0> &rhs)
        {
            scoped_assert(rhs.row() == R && rhs.col() == C, "matrix::+= size mismatch!");    
            std::transform(m_mat.begin(), m_mat.end(), rhs.begin(), m_mat.begin(), std::plus<Tp>());
            return *this;
        }
 
        // -=

        matrix &
        operator-=(const matrix &rhs)
        {
            std::transform(m_mat.begin(), m_mat.end(), rhs.begin(), m_mat.begin(), std::minus<Tp>());
            return *this;
        }

        matrix &
        operator-=(const matrix<Tp,0,0> &rhs)
        {
            scoped_assert(rhs.row() == R && rhs.col() == C, "matrix::-= size mismatch!");    
            std::transform(m_mat.begin(), m_mat.end(), rhs.begin(), m_mat.begin(), std::minus<Tp>());
            return *this;
        }

        template <typename T>
        matrix &
        operator*=(T val)
        {
            std::transform(m_mat.begin(), m_mat.end(), m_mat.begin(), std::bind(std::multiplies<Tp>(),val,_1));
            return *this;
        }

        template <typename T>
        matrix &
        operator/=(T val)
        {
            std::transform(m_mat.begin(), m_mat.end(), m_mat.begin(), std::bind(std::divides<Tp>(),_1, val));
            return *this;
        }

        matrix &
        operator-()
        {
            return this->operator*=(-1);
        }        
 
    private:
        scoped_assert m_assert;
        std::array<Tp, R*C> m_mat; 
    };    
    
    ///////////////////////////////////////////////////////////////////////////////
    // dynamic-size matrix
    ///////////////////////////////////////////////////////////////////////////////
 
    template <typename Tp>
    class matrix<Tp,0,0> : _base_matrix_iterators<Tp,typename std::vector<Tp>::iterator, 
                                                     typename std::vector<Tp>::const_iterator>
    {
    public:            
        typedef Tp value_type;          
        typedef _base_matrix_iterators<Tp, 
                typename std::vector<Tp>::iterator, 
                typename std::vector<Tp>::const_iterator>       base_type;

        typedef typename std::vector<Tp>::iterator              iterator; 
        typedef typename std::vector<Tp>::const_iterator        const_iterator; 
        typedef typename std::vector<Tp>::iterator              row_iterator; 
        typedef typename std::vector<Tp>::const_iterator        const_row_iterator; 
        typedef typename base_type::_col_iterator               col_iterator;
        typedef typename base_type::_const_col_iterator         const_col_iterator;
 
        static const int row_size = 0;
        static const int col_size = 0;

        matrix(size_t r, size_t c)
        : m_assert(), m_r(r), m_c(c), m_mat(r*c)
        {}

        matrix(const matrix &rhs)
        : m_assert(), m_r(rhs.row()), m_c(rhs.col()), m_mat(rhs.m_mat)
        {}

        template <size_t R, size_t C>
        explicit matrix(const matrix<Tp,R,C> &rhs)
        : m_assert(), m_r(R), m_c(C), m_mat(rhs.begin(), rhs.end())
        {} 

        template <size_t R, size_t C>
        matrix &
        operator=(const matrix<Tp,R,C> &rhs)
        {
            scoped_assert(row() == R && col() == C, "matrix::op= size mismatch!");    
            std::copy(rhs.begin(), rhs.end(), m_mat.begin());
            return *this;
        }

        matrix &
        operator=(const matrix &rhs)
        {
            scoped_assert(rhs.row() == row() && rhs.col() == col(), "matrix::op= size mismatch!");    
            std::copy(rhs.begin(), rhs.end(), m_mat.begin());
            return *this;
        }

        matrix & 
        operator=(std::initializer_list<Tp> lst)
        {
            scoped_assert( lst.size() == row() * col(), "matrix::op= size mismatch!");    
            m_mat.assign(lst.begin(), lst.end());
            return *this;
        }
 
        Tp &
        operator()(size_t r, size_t c)
        {
            scoped_assert(r < row() && c < col(), "matrix::op() bad index");
            return m_mat[r*col()+c];
        }     

        const Tp &
        operator()(size_t r, size_t c) const
        {
            scoped_assert(r < row() && c < col(), "matrix::op() bad index");
            return m_mat[r*col()+c];
        }

        // observers:
        //

        size_t row() const
        { return m_r; }

        size_t col() const
        { return m_c; }

        iterator 
        begin() 
        {
            return m_mat.begin();
        }
        const_iterator 
        begin() const
        {
            return m_mat.begin();
        }
        const_iterator 
        cbegin() const
        {
            return m_mat.begin();
        }

        iterator 
        end() 
        {
            return m_mat.end();
        }
        const_iterator 
        end() const
        {
            return m_mat.end();
        }
        const_iterator 
        cend() const
        {
            return m_mat.end();
        }
 
        //////// row_iterator:

        row_iterator 
        row_begin(size_t r) 
        {
            scoped_assert( r < row(), "matrix::row_iterator bad index");    
            return m_mat.begin() + static_cast<ptrdiff_t>(r * col());
        }
        
        const_row_iterator
        row_begin(size_t r) const
        {
            scoped_assert( r < row(), "matrix::row_iterator bad index");    
            return m_mat.begin() + static_cast<ptrdiff_t>(r * col());
        }

        row_iterator 
        row_end(size_t r) 
        {
            scoped_assert( r < row(), "matrix::row_iterator bad index");    
            return m_mat.begin() + static_cast<ptrdiff_t>((r+1) * col());
        }

        const_row_iterator
        row_end(size_t r) const
        {
            scoped_assert( r < row(), "matrix::row_iterator bad index");    
            return m_mat.begin() + static_cast<ptrdiff_t>((r+1) * col());
        }

        //////// col_iterator:

        col_iterator 
        col_begin(size_t c) 
        {
            scoped_assert( c < col(), "matrix::col_iterator bad index");    
            return col_iterator(m_mat.begin() + static_cast<ptrdiff_t>(c), col());
        }
        
        const_col_iterator
        col_begin(size_t c) const
        {
            scoped_assert( c < col(), "matrix::col_iterator bad index");    
            return const_col_iterator(m_mat.begin() + static_cast<ptrdiff_t>(c), col());
        }

        col_iterator 
        col_end(size_t c) 
        {
            scoped_assert( c < col(), "matrix::col_iterator bad index");    
            return col_iterator(m_mat.begin() + static_cast<ptrdiff_t>(c+row()*col()), col());
        }

        const_col_iterator
        col_end(size_t c) const
        {
            scoped_assert( c < col(), "matrix::col_iterator bad index");    
            return const_col_iterator(m_mat.begin() + static_cast<ptrdiff_t>(c+row()*col()), col());
        }
 
        ////////////////////////////////////
 
        template <size_t R, size_t C>
        bool 
        operator==(const matrix<Tp,R,C> &rhs) const
        { 
            if (row() != R || col() != C)
                return false;
            return std::equal(m_mat.begin(), m_mat.end(), rhs.begin());   
        }
        bool 
        operator==(const matrix &rhs) const
        {
            if (rhs.row() != row() || rhs.col() != col())
                return false;
            return std::equal(m_mat.begin(), m_mat.end(), rhs.begin());   
        }

        template <size_t R, size_t C>
        bool 
        operator!=(const matrix<Tp,R,C> &rhs) const
        {
            return !(*this == rhs);   
        }
        bool 
        operator!=(const matrix &rhs) const
        {
            return !(*this == rhs);   
        }
        
        // +=

        template <size_t R, size_t C>
        matrix &
        operator+=(const matrix<Tp,R,C> &rhs)
        {
            scoped_assert(row() == R && col() == C, "matrix::+= size mismatch!");    
            std::transform(m_mat.begin(), m_mat.end(), rhs.begin(), m_mat.begin(), std::plus<Tp>());
            return *this;
        }

        matrix &
        operator+=(const matrix &rhs)
        {
            scoped_assert(rhs.row() == row() && rhs.col() == col(), "matrix::+= size mismatch!");    
            std::transform(m_mat.begin(), m_mat.end(), rhs.begin(), m_mat.begin(), std::plus<Tp>());
            return *this;
        }
 
        // -=

        template <size_t R, size_t C>
        matrix &
        operator-=(const matrix<Tp,R,C> &rhs)
        {
            scoped_assert(row() == R && col() == C, "matrix::-= size mismatch!");    
            std::transform(m_mat.begin(), m_mat.end(), rhs.begin(), m_mat.begin(), std::minus<Tp>());
            return *this;
        }

        matrix &
        operator-=(const matrix &rhs)
        {
            scoped_assert(rhs.row() == row() && rhs.col() == col(), "matrix::-= size mismatch!");    
            std::transform(m_mat.begin(), m_mat.end(), rhs.begin(), m_mat.begin(), std::minus<Tp>());
            return *this;
        }

        template <typename T>
        matrix &
        operator*=(T val)
        {
            std::transform(m_mat.begin(), m_mat.end(), m_mat.begin(), std::bind(std::multiplies<Tp>(),val,_1));
            return *this;
        }

        template <typename T>
        matrix &
        operator/=(T val)
        {
            std::transform(m_mat.begin(), m_mat.end(), m_mat.begin(), std::bind(std::divides<Tp>(),_1, val));
            return *this;
        }

        matrix &
        operator-()
        {
            return this->operator*=(-1);
        }        

    private:
        scoped_assert m_assert;

        size_t m_r;
        size_t m_c;
        std::vector<Tp> m_mat;        
    };

    ///////////////////////////////////////////////////////////////////////////////
    // free functions:
    ///////////////////////////////////////////////////////////////////////////////
 
    // type trait: 
    //

    template <typename Tp>
    struct is_matrix 
    {
        enum { value = false };
    };

    template <typename Tp, size_t R, size_t C>
    struct is_matrix< more::matrix<Tp, R, C> >
    {
        enum { value = true };
    }; 
    
    // streaming:
    //
    
    template <typename CharT, typename Traits, typename Tp, size_t R, size_t C>
    inline typename std::basic_ostream<CharT, Traits> &
    operator<<(std::basic_ostream<CharT,Traits> &out, const matrix<Tp,R,C> & rhs)
    {   
        auto it = rhs.cbegin();
        auto it_end = rhs.cend();

        out << *it++ << ' ';
        for(int col = rhs.col(); it != it_end; ++it)
        {
            std::cout << *it << ' ';
            if (! ((std::distance(it_end, it)+1) % col) )
                std::cout << std::endl;     
        }
        return out;
    }

    ////////////////////////////////////////////////////////////////

    template <typename Tp, size_t R, size_t C, size_t R_, size_t C_>
    inline matrix<Tp,R,C>
    operator+(const matrix<Tp,R,C> &lhs, const matrix<Tp,R_,C_> &rhs)
    {
        return matrix<Tp,R,C>(lhs)+=rhs;
    }

    template <typename Tp, size_t R, size_t C, size_t R_, size_t C_>
    inline matrix<Tp,R,C>
    operator-(const matrix<Tp,R,C> &lhs, const matrix<Tp,R_,C_> &rhs)
    {
        return matrix<Tp,R,C>(lhs)-=rhs;
    }
 
    template <typename Tp, size_t R, size_t C, typename A>
    inline typename __gnu_cxx::__enable_if<!is_matrix<A>::value, typename more::matrix<Tp,R,C>>::__type
    operator*(const matrix<Tp,R,C> &lhs, A rhs)
    {
        return matrix<Tp,R,C>(lhs)*=rhs;
    }
    template <typename Tp, size_t R, size_t C, typename A>
    inline typename __gnu_cxx::__enable_if<!is_matrix<A>::value,typename more::matrix<Tp,R,C>>::__type
    operator*(A lhs, const matrix<Tp,R,C> &rhs)
    {
        return matrix<Tp,R,C>(rhs)*=lhs;
    }
 
    template <typename Tp, size_t R, size_t C, typename A>
    inline typename __gnu_cxx::__enable_if<!is_matrix<A>::value,typename more::matrix<Tp,R,C>>::__type
    operator/(const matrix<Tp,R,C> &lhs, A rhs)
    {
        return matrix<Tp,R,C>(lhs)/=rhs;
    }
 
    ///////// A * B: stc * stc size matrix:

    template <typename Tp, size_t R, size_t X, size_t C>
    inline typename __gnu_cxx::__enable_if< R != 0 && C != 0, typename more::matrix<Tp, R, C>>::__type
    operator*(const matrix<Tp,R,X> &lhs, const matrix<Tp, X, C> &rhs)
    {
        matrix<Tp, R, C> mat;
        for(unsigned int i = 0; i < R; i++)
            for(unsigned int j = 0; j < C; j++)
                mat(i,j) = std::inner_product(lhs.row_begin(i), lhs.row_end(i), rhs.col_begin(j), Tp()); 

        return mat;
    }

    ///////// A * B: [dyn * dyn, dyn * stc, stc * dyn]-size matrix:
    
    template <typename Tp, size_t R1, size_t C1, size_t R2, size_t C2>
    inline typename __gnu_cxx::__enable_if< R1 == 0 || C2 == 0, typename more::matrix<Tp, 0, 0>>::__type
    operator*(const matrix<Tp,R1,C1> &lhs, const matrix<Tp,R2,C2> &rhs)
    {
        scoped_assert(lhs.col() == rhs.row(), "matrix::* size mismatch!");    

        size_t row = lhs.row();
        size_t col = rhs.col();

        matrix<Tp> mat(row,col);

        for(unsigned int i = 0; i < row; i++)
            for(unsigned int j = 0; j < col; j++)
                mat(i,j) = std::inner_product(lhs.row_begin(i), lhs.row_end(i), rhs.col_begin(j), Tp()); 

        return mat;
    } 

    /////////

    template <typename Tp, size_t R, size_t C>
    inline matrix<Tp, C, R>
    tr(const matrix<Tp, R, C> &mat)
    {
        matrix<Tp, C, R> ret;
        for(unsigned int i = 0; i < mat.row(); i++)
            for(unsigned int j = 0; j < mat.col(); j++)
                ret(j,i) = mat(i,j);
        return ret;
    }

    template <typename Tp>
    inline matrix<Tp,0,0>
    tr(const matrix<Tp, 0, 0> &mat)
    {
        matrix<Tp> ret(mat.col(), mat.row());
        for(unsigned int i = 0; i < mat.row(); i++)
            for(unsigned int j = 0; j < mat.col(); j++)
                ret(j,i) = mat(i,j);
        return ret;
    }

    template <typename Tp, size_t R>
    inline bool
    is_simmetric(const matrix<Tp, R, R> &mat)
    {
        return mat == tr(mat);
    }

    template <typename Tp>
    inline bool
    is_simmetric(const matrix<Tp, 0, 0> &mat)
    {
        if ( mat.row() != mat.col())
            return false;
        return mat == tr(mat);
    }
 
    template <typename Tp, size_t R, size_t C>
    inline bool
    is_square(const matrix<Tp, R, C> &mat)
    {
        return mat.row() == mat.col();
    }

    template <typename Tp>
    inline typename __gnu_cxx::__enable_if< std::is_integral<Tp>::value, Tp>::__type  
    _divide(Tp a, Tp b)
    {
        double safe = static_cast<double>(a)/static_cast<double>(b);
        Tp r = a/b;

        if ( safe != static_cast<double>(r) )
        {
            throw std::runtime_error("matrix::__divide: underflow error!");
        }
        return r;
    }

    template <typename Tp>
    inline typename __gnu_cxx::__enable_if< !std::is_integral<Tp>::value, Tp>::__type  
    _divide(Tp a, Tp b)
    {
        return a/b;
    }
    
    // determinant: see http://cboard.cprogramming.com/cplusplus-programming/30001-determinant-calculation.html
    //              for further details.

    template <typename Tp, size_t R>
    Tp det(matrix<Tp, R, R> mat)
    {
        scoped_assert( is_square(mat), "matrix::det non a square matrix!");    
        const unsigned int order = mat.row();
        Tp _det = static_cast<Tp>(1);

        unsigned int j;
        for(unsigned int k = 0; k < order; ++k)
        {
            if (mat(k,k) == 0) {
                bool ok = false;
                for(j = k; j < order; ++j)
                {
                    if (mat(j,k) != Tp())
                        ok = true;
                }
                if (!ok)
                    return Tp();

                for(unsigned int i = k; i < order; ++i)
                    std::swap(mat(i,j), mat(i,k));

                _det = - _det;
            } 
            _det *= mat(k,k);
            if ( k+1 < order )
            {
                for (unsigned int i = k+1; i < order; ++i)
                    for (unsigned int j = k+1; j < order; ++j)
                        mat(i,j)-= _divide(mat(i,k)*mat(k,j), mat(k,k));
            }
        }
        return _det;
    }

    template <size_t R, size_t C, typename Tp>
    inline matrix<Tp, R, C>
    mat(std::initializer_list<Tp> l)
    {
        return matrix<Tp, R, C>(l);
    }

} // namespace more


#endif /* _MATRIX_HPP_ */
