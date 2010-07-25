/* $Id$ */
/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <bonelli@antifork.org> wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return. Nicola Bonelli
 * ----------------------------------------------------------------------------
 */
 
#include <cassert>
#include <iostream>

#include <matrix.hpp>

  int
main(int argc, char *argv[])
{
    more::matrix<int,2,2> a;
    more::matrix<int> b(2,2);
    more::matrix<int> c(a);

    c = { 1, 0, 0, 1 };
    a = { 1, 2, 3, 4 };

    try 
    {
        more::matrix<int,3,3> d(c);
        assert(0);
    }
    catch(std::exception &e)
    {
        std::cout << "exception: " << e.what() << " (ok)" << std::endl;
    }

    // op=
    b = a;
    assert(b == a);

    try 
    {
        more::matrix<int,3,3> d;
        // d = a;   // not allowed!
        d = b;      // scoped assert
        assert(0);
    }
    catch(std::exception &e)
    {
        std::cout << "exception: " << e.what() << " (ok)" << std::endl;
    }

    // op()
    { 
        more::matrix<int,3,5> m;
        std::cout << m;
    }        
    /////////////////////////

    {
        more::matrix<int, 2, 2> m;

        m(0,0) = 1;
        m(0,1) = 2;
        m(1,0) = 3;
        m(1,1) = 4;

        more::matrix<int> m2(m);

        std::cout << "----" << std::endl;

        m += m;
        std::cout << m;

        std::cout << "----" << std::endl;

        m -= m2;
        std::cout << m;

        std::cout << "----" << std::endl;
        m *= 2;
        std::cout << m;

        std::cout << "----" << std::endl;
        m /= 2;
        std::cout << m;
    }

    {
        more::matrix<int> m(2,2);
        m = {1, 2, 3, 4};

        more::matrix<int,2,2> m2(m);

        std::cout << "----" << std::endl;

        m += m;
        std::cout << m;

        std::cout << "----" << std::endl;

        m -= m2;
        std::cout << m;

        std::cout << "----" << std::endl;
        m *= 2;
        std::cout << m;

        std::cout << "----" << std::endl;
        m /= 2;
        std::cout << m;
    }

    {
        more::matrix<int,2,2> m;
        m = {1, 2, 3, 4};

        std::cout << "----" << std::endl;
        std::cout << (m + m);
    }
    {
        more::matrix<int> m(2,2);
        m = {1, 2, 3, 4};

        std::cout << "----" << std::endl;
        std::cout << (m - m);
    }
    {
        more::matrix<int> m(2,2);
        m = {1, 2, 3, 4};

        std::cout << "----" << std::endl;
        std::cout << 2 * m;
        std::cout << m * 2;
    }
    {
        more::matrix<double> m(2,2);
        m = {1.0, 2.0, 3.0, 4.0};

        std::cout << "----" << std::endl;
        std::cout << m/2;
    }

    {
        more::matrix<double> m(2,2);
        m = {1, 2, 3, 4};

        std::cout << "----" << std::endl;
        std::cout << -m;
    }

    {
        more::matrix<double> m(2,2);
        m = {1, 2, 3, 4};

        std::cout << "----" << std::endl;
        std::cout << tr(m);
    }

    {
        more::matrix<double,2,3> m;
        m = {1, 2, 0, 3, 4, 0};

        std::cout << "----" << std::endl;
        std::cout << tr(m);
        std::cout << "----" << std::endl;
        std::cout << tr(tr(m));
    }

    {
        more::matrix<double,2,2> m;
        m = {1, 2, 3, 4};

        std::cout << std::boolalpha;
        std::cout << is_simmetric(m) << std::endl;
        assert(!is_simmetric(m));
    }

    {
        more::matrix<double,2,2> m;
        m = {1, 2, 2, 1};

        std::cout << std::boolalpha;
        std::cout << is_simmetric(m) << std::endl;
        assert(is_simmetric(m));
    }

    {
        more::matrix<double> m(2,2);
        m = {1, 2, 3, 4};

        std::cout << std::boolalpha;
        std::cout << is_simmetric(m) << std::endl;
        assert(!is_simmetric(m));
    }

    {
        more::matrix<double> m(2,2);
        m = {1, 2, 2, 1};

        std::cout << std::boolalpha;
        std::cout << is_simmetric(m) << std::endl;
        assert(is_simmetric(m));
    }

    {
        more::matrix<double> m(2,3);
        m = {1, 2, 0, 3, 4, 0};
         
        assert(m == m);
        assert(!(m!=m));
        assert(!(m == tr(m)));
        assert((m != tr(m)));

        std::cout << (m == m) << std::endl;
        std::cout << (m != m) << std::endl;
        std::cout << (m == tr(m)) << std::endl;
        std::cout << (m != tr(m)) << std::endl;
    }

    assert( !more::is_matrix<int>::value );
    assert(  more::is_matrix< more::matrix<int> >::value );

    {        
        std::vector<int> r1 = {1, 2};
        std::vector<int> r2 = {3, 4};
 
        more::matrix<int,2,2> a;
        a = { 1, 2, 3, 4 };

        more::matrix<int> b(2,2);
        b = { 1, 2, 3, 4 };

        assert( std::equal(a.row_begin(0), a.row_end(0), r1.begin()) );
        assert( std::equal(a.row_begin(1), a.row_end(1), r2.begin()) );
        assert( std::equal(b.row_begin(0), b.row_end(0), r1.begin()) );
        assert( std::equal(b.row_begin(1), b.row_end(1), r2.begin()) );
    }

    {        
        std::vector<int> r1 = {1, 3};
        std::vector<int> r2 = {2, 4};
 
        more::matrix<int,2,2> a;
        a = { 1, 2, 3, 4 };

        more::matrix<int,2,2>::const_col_iterator it1_beg = a.col_begin(0);
        more::matrix<int,2,2>::const_col_iterator it1_end = a.col_end(0);
        more::matrix<int,2,2>::const_col_iterator it2_beg = a.col_begin(1);
        more::matrix<int,2,2>::const_col_iterator it2_end = a.col_end(1);

        assert( std::equal(it1_beg, it1_end, r1.begin()) );
        assert( std::equal(it2_beg, it2_end, r2.begin()) );
    }

    {        
        std::vector<int> r1 = {1, 3};
        std::vector<int> r2 = {2, 4};
 
        more::matrix<int> a(2,2);
        a = { 1, 2, 3, 4 };

        more::matrix<int>::const_col_iterator it1_beg = a.col_begin(0);
        more::matrix<int>::const_col_iterator it1_end = a.col_end(0);
        more::matrix<int>::const_col_iterator it2_beg = a.col_begin(1);
        more::matrix<int>::const_col_iterator it2_end = a.col_end(1);

        assert( std::equal(it1_beg, it1_end, r1.begin()) );
        assert( std::equal(it2_beg, it2_end, r2.begin()) );
    }
    
    // product: static * static
    {
        more::matrix<int,2,3> a;
        a = { 1, 2, 3, 4, 5, 6 };

        more::matrix<int,3,2> b;
        b = { 1, 1, 1, 1, 1, 1 };

        more::matrix<int,2,2> p;

        p = { 6, 6, 15, 15 };

        std::cout << (a*b);
        assert( (a * b) == p);
    }

    // product: dynamic * dynamic
    {
        more::matrix<int> a(2,3);
        a = { 1, 2, 3, 4, 5, 6 };

        more::matrix<int> b(3,2);
        b = { 1, 1, 1, 1, 1, 1 };

        more::matrix<int> p(2,2);

        p = { 6, 6, 15, 15 };

        std::cout << (a*b);
        assert((a * b) == p);
    }

    // product: dynamic * static
    {
        more::matrix<int> a(2,3);
        a = { 1, 2, 3, 4, 5, 6 };

        more::matrix<int,3,2> b;
        b = { 1, 1, 1, 1, 1, 1 };

        more::matrix<int> p(2,2);

        p = { 6, 6, 15, 15 };

        std::cout << (a*b);
        assert((a * b) == p);
    }
 
    // product: static * dynamic 
    {
        more::matrix<int,2,3> a;
        a = { 1, 2, 3, 4, 5, 6 };

        more::matrix<int> b(3,2);
        b = { 1, 1, 1, 1, 1, 1 };

        more::matrix<int> p(2,2);

        p = { 6, 6, 15, 15 };

        std::cout << (a*b);
        assert( (a * b) == p);
    }

    // is_square:
    {
        more::matrix<int,2,3> a;
        more::matrix<int> b(2,2);

        assert( is_square(a) == false );
        assert( is_square(b) == true  );
    }

    {
        more::matrix<double> a(3,3);
        a = {1, 7, 3, 2, 5, 2, 3, 8, 10};
        assert( det(a) == -61 );
    }

    return 0;
}

