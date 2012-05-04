/* $Id$ */
/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <bonelli@antifork.org> wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return. Nicola Bonelli
 * ----------------------------------------------------------------------------
 */
 
#include <iostream>

#include <matrix.hpp>
#include <yats.hpp>

using namespace yats;
using namespace more;

Context(more_matrix_test)
{
    
   Test(generic)
   {
        more::matrix<int,2,2> a;
        more::matrix<int> b(2,2);
        more::matrix<int> c(a);

        c = { 1, 0, 0, 1 };
        a = { 1, 2, 3, 4 };

        b = a;
        Assert(b == a, is_true());

        more::matrix<int,3,3> d;
        // d = a;   // not allowed!

        AssertThrow( (d = b) );      // scoped assert
   }

    Test(matrix_stream)
    { 
        more::matrix<int,2,2> m;
        Assert( (m == mat<2,2>({0,0,0,0})), is_true() );
    }        

    Test(operations)
    {
        more::matrix<int, 2, 2> m;

        m(0,0) = 1;
        m(0,1) = 2;
        m(1,0) = 3;
        m(1,1) = 4;

        more::matrix<int> m2(m);

        m += m;
        Assert( (m == mat<2,2>({ 2, 4, 6, 8 })), is_true() );

        m -= m2;
        Assert( (m == mat<2,2>({ 1, 2, 3, 4 })), is_true() );

        m *= 2;
        Assert( (m == mat<2,2>({ 2, 4, 6, 8 })), is_true() );

        m /= 2;
        Assert( (m == mat<2,2>({ 1, 2, 3, 4 })), is_true() );
    }

    Test(operations_dyn)
    {
        more::matrix<int> m(2,2);
        m = {1, 2, 3, 4};

        more::matrix<int,2,2> m2(m);

        m += m;
        Assert( (m == mat<2,2>({ 2, 4, 6, 8 })), is_true() );

        m -= m2;
        Assert( (m == mat<2,2>({ 1, 2, 3, 4 })), is_true() );

        m *= 2;
        Assert( (m == mat<2,2>({ 2, 4, 6, 8 })), is_true() );

        m /= 2;
        Assert( (m == mat<2,2>({ 1, 2, 3, 4 })), is_true() );
    }

    Test(scalar_product_dyn)
    {
        more::matrix<int> m(2,2);
        m = {1, 2, 3, 4};

        Assert( (m * 2 == mat<2,2>({ 2, 4, 6, 8 })), is_true() );
        Assert( (2 * m == mat<2,2>({ 2, 4, 6, 8 })), is_true() );
    }
    
    Test(scalar_division)
    {
        more::matrix<double> m(2,2);
        m = {1.0, 2.0, 3.0, 4.0};

        Assert( (m/2 == mat<2,2>({ 0.5, 1.0, 1.5, 2.0 })), is_true() );
    }

    Test(sign)
    {
        more::matrix<int> m(2,2);
        m = {1, 2, 3, 4};

        m = -m;

        Assert( ( m == mat<2,2>({ -1, -2, -3, -4 })), is_true() );
    }

    Test(tr_dynamic)
    {
        more::matrix<int> m(2,2);
        m = {1, 2, 3, 4};

        Assert( ( tr(m) == mat<2,2>({ 1, 3, 2, 4 })), is_true() );
    }

    Test(tr_static)
    {
        more::matrix<int,2,3> m;
        m = {1, 2, 0, 3, 4, 0};

        Assert( ( tr(m) == mat<3,2>({ 1, 3, 2, 4, 0, 0 })), is_true() );
        Assert( ( tr(tr(m)) == mat<2,3>({ 1, 2, 0, 3, 4, 0 })), is_true() );
    }
    
    Test(is_simmetric)
    {
        more::matrix<double,2,2> m;

        m = {1, 2, 3, 4};
        Assert(is_simmetric(m), is_false());
        
        m = {1, 2, 2, 1};
        Assert(is_simmetric(m), is_true());
    }

    Test(is_simmetric_static)
    {
        more::matrix<double> m(2,2);
        m = {1, 2, 3, 4};

        m = {1, 2, 3, 4};
        Assert(is_simmetric(m), is_false());
        
        m = {1, 2, 2, 1};
        Assert(is_simmetric(m), is_true());
    }

    Test(op_equal)
    {
        more::matrix<double> m(2,3);
        m = {1, 2, 0, 3, 4, 0};
         
        Assert(m == m,      is_true());
        Assert(m != m,      is_false());
        Assert(m == tr(m),  is_false());
        Assert(m != tr(m),  is_true());
    }

    Test(matrix_traits)
    {
        Assert( static_cast<bool>(more::is_matrix<int>::value), is_false() );
        Assert( static_cast<bool>(more::is_matrix< more::matrix<int> >::value), is_true() );
    }

    Test(row_iterator)
    {        
        std::vector<int> r1 = {1, 2};
        std::vector<int> r2 = {3, 4};
 
        more::matrix<int,2,2> a = {1, 2, 3, 4};
        more::matrix<int> b(2,2); 
        b = {1, 2, 3, 4 };

        Assert( std::equal(a.row_begin(0), a.row_end(0), r1.begin()), is_true() );
        Assert( std::equal(a.row_begin(1), a.row_end(1), r2.begin()), is_true() );
        Assert( std::equal(b.row_begin(0), b.row_end(0), r1.begin()), is_true() );
        Assert( std::equal(b.row_begin(1), b.row_end(1), r2.begin()), is_true() );
    }

    Test(const_col_iterator)
    {        
        std::vector<int> r1 = {1, 3};
        std::vector<int> r2 = {2, 4};
 
        more::matrix<int,2,2> a = { 1, 2, 3, 4 };

        more::matrix<int,2,2>::const_col_iterator it1_beg = a.col_begin(0);
        more::matrix<int,2,2>::const_col_iterator it1_end = a.col_end(0);
        more::matrix<int,2,2>::const_col_iterator it2_beg = a.col_begin(1);
        more::matrix<int,2,2>::const_col_iterator it2_end = a.col_end(1);

        Assert( std::equal(it1_beg, it1_end, r1.begin()), is_true() );
        Assert( std::equal(it2_beg, it2_end, r2.begin()), is_true() );
    }

    Test(const_col_iterator_dyn)
    {        
        std::vector<int> r1 = {1, 3};
        std::vector<int> r2 = {2, 4};
 
        more::matrix<int> a(2,2);
        a = { 1, 2, 3, 4 };

        more::matrix<int>::const_col_iterator it1_beg = a.col_begin(0);
        more::matrix<int>::const_col_iterator it1_end = a.col_end(0);
        more::matrix<int>::const_col_iterator it2_beg = a.col_begin(1);
        more::matrix<int>::const_col_iterator it2_end = a.col_end(1);

        Assert( std::equal(it1_beg, it1_end, r1.begin()) , is_true());
        Assert( std::equal(it2_beg, it2_end, r2.begin()) , is_true());
    }
    
    Test(product_static_static)
    {
        more::matrix<int,2,3> a;
        a = { 1, 2, 3, 4, 5, 6 };

        more::matrix<int,3,2> b;
        b = { 1, 1, 1, 1, 1, 1 };

        more::matrix<int,2,2> p;
        p = { 6, 6, 15, 15 };

        Assert( a * b, is_equal_to(p));
    }

    Test(product_dyn_dyn)
    {
        more::matrix<int> a(2,3);
        a = { 1, 2, 3, 4, 5, 6 };

        more::matrix<int> b(3,2);
        b = { 1, 1, 1, 1, 1, 1 };

        more::matrix<int> p(2,2);

        p = { 6, 6, 15, 15 };

        Assert( a * b, is_equal_to(p));
    }

    Test(product_dyn_static)
    {
        more::matrix<int> a(2,3);
        a = { 1, 2, 3, 4, 5, 6 };

        more::matrix<int,3,2> b;
        b = { 1, 1, 1, 1, 1, 1 };

        more::matrix<int> p(2,2);

        p = { 6, 6, 15, 15 };

        Assert( a * b, is_equal_to(p));
    }
 
    Test(product_satatic_dyn)
    {
        more::matrix<int,2,3> a;
        a = { 1, 2, 3, 4, 5, 6 };

        more::matrix<int> b(3,2);
        b = { 1, 1, 1, 1, 1, 1 };

        more::matrix<int> p(2,2);

        p = { 6, 6, 15, 15 };

        Assert( a * b, is_equal_to(p));
    }

    Test(is_square)
    {
        more::matrix<int,2,3> a;
        more::matrix<int> b(2,2);

        Assert( is_square(a) , is_false());
        Assert( is_square(b) , is_true() );
    }

    Test(det)
    {
        more::matrix<double> a(3,3);
        a = {1, 7, 3, 2, 5, 2, 3, 8, 10};
        Assert( det(a), is_equal_to(-61) );
    }
}

int
main(int argc, char *argv[])
{
    return yats::run();
}

