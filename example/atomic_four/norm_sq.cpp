/* --------------------------------------------------------------------------
CppAD: C++ Algorithmic Differentiation: Copyright (C) 2003-22 Bradley M. Bell

CppAD is distributed under the terms of the
             Eclipse Public License Version 2.0.

This Source Code may also be made available under the following
Secondary License when the conditions for such availability set forth
in the Eclipse Public License, Version 2.0 are satisfied:
      GNU General Public License, Version 2.0 or later.
---------------------------------------------------------------------------- */

/*
$begin atomic_four_norm_sq.cpp$$
$spell
    sq
    bool
    enum
$$

$section Atomic Euclidean Norm Squared: Example and Test$$

$head Function$$
This example demonstrates using $cref atomic_four$$
to define the operation
$latex g : \B{R}^n \rightarrow \B{R}$$ where
$latex \[
    g(x) =  x_0^2 + \cdots + x_{n-1}^2
\] $$

$head Purpose$$
This atomic function demonstrates the following cases:
$list number$$
an arbitrary number of arguments $icode n$$
$lnext
zero and first order forward mode.
$lnext
first order derivatives using reverse mode.
$lend

$head Define Atomic Function$$
$srcthisfile%0
    %// BEGIN_DEFINE_ATOMIC_FUNCTION%// END_DEFINE_ATOMIC_FUNCTION%
1%$$

$head Use Atomic Function$$
$srcthisfile%0
    %// BEGIN_USE_ATOMIC_FUNCTION%// END_USE_ATOMIC_FUNCTION%
1%$$

$end
*/
# include <cppad/cppad.hpp>

// BEGIN_DEFINE_ATOMIC_FUNCTION
// empty namespace
namespace {
    // atomic_norm_sq
    class atomic_norm_sq : public CppAD::atomic_four<double> {
    public:
        atomic_norm_sq(const std::string& name) :
        CppAD::atomic_four<double>(name)
        { }
    private:
        // for_type
        bool for_type(
            size_t                                     call_id     ,
            const CppAD::vector<CppAD::ad_type_enum>&  type_x      ,
            CppAD::vector<CppAD::ad_type_enum>&        type_y      ) override
        {   assert( call_id == 0 );       // default value
            assert(type_y.size() == 1 );  // m
            //
            // type_y
            size_t n     = type_x.size();
            type_y[0] = CppAD::constant_enum;
            for(size_t j = 0; j < n; ++j)
                type_y[0] = std::max(type_y[0], type_x[j]);
            return true;
        }
        // forward
        bool forward(
            size_t                             call_id     ,
            const CppAD::vector<bool>&         select_y    ,
            size_t                             order_low   ,
            size_t                             order_up    ,
            const CppAD::vector<double>&       tx          ,
            CppAD::vector<double>&             ty          ) override
        {
            size_t q = order_up + 1;
            size_t n = tx.size() / q;
    # ifndef NDEBUG
            size_t m = ty.size() / q;
            assert( call_id == 0 );
            assert( m == 1 );
            assert( m == select_y.size() );
    # endif
            // ok
            bool ok = order_up <= 1 && order_low <= order_up;
            if ( ! ok )
                return ok;
            //
            // sum = x_0^0 * x_0^0 + x_1^0 * x_1^0 + ...
            double sum = 0.0;
            for(size_t j = 0; j < n; ++j)
            {   double xj0 = tx[ j * q + 0];
                sum       += xj0 * xj0;
            }
            //
            // ty[0] = sum
            if( order_low <= 0 )
                ty[0] = sum;
            if( order_up < 1 )
                return ok;

            // sum = x_0^0 * x_0^1 + x_1^0 ^ x_1^1 + ...
            sum   = 0.0;
            for(size_t j = 0; j < n; ++j)
            {   double xj0 = tx[ j * q + 0];
                double xj1 = tx[ j * q + 1];
                sum       += xj0 * xj1;
            }
            // ty[1] = 2.0 * sum
            assert( order_up == 1 );
            ty[1] = 2.0 * sum;
            return ok;

            // Assume we are not using forward mode with order > 1
            assert( ! ok );
            return ok;
        }
        // reverse
        bool reverse(
            size_t                              call_id     ,
            const CppAD::vector<bool>&          select_x    ,
            size_t                              order_up    ,
            const CppAD::vector<double>&        tx          ,
            const CppAD::vector<double>&        ty          ,
            CppAD::vector<double>&              px          ,
            const CppAD::vector<double>&        py          ) override
        {
            size_t q = order_up + 1;
            size_t n = tx.size() / q;
    # ifndef NDEBUG
            size_t m = ty.size() / q;
            assert( call_id == 0 );
            assert( m == 1 );
            assert( px.size() == tx.size() );
            assert( py.size() == ty.size() );
            assert( n == select_x.size() );
    # endif
            // ok
            bool ok = order_up == 0;
            if ( ! ok )
                return ok;

            // first order reverse mode
            for(size_t j = 0; j < n; ++j)
            {   // x_0^0
                double xj0 = tx[ j * q + 0];
                //
                // H = G( F( { x_j^k } ) )
                double dF = 2.0 * xj0; // partial F w.r.t x_j^0
                double dG = py[0];     // partial of G w.r.t. y[0]
                double dH = dG * dF;   // partial of H w.r.t. x_j^0

                // px[j]
                px[j] = dH;
            }
            return ok;
        }
        // jac_sparsity
        bool jac_sparsity(
            size_t                                     call_id     ,
            bool                                       dependency  ,
            const CppAD::vector<bool>&                 select_x    ,
            const CppAD::vector<bool>&                 select_y    ,
            CppAD::sparse_rc< CppAD::vector<size_t> >& pattern_out ) override
        {   size_t n = select_x.size();
            size_t m = select_y.size();
# ifndef NDEBUG
            assert( call_id == 0 );
            assert( m == 1 );
# endif
            // nnz
            size_t nnz = 0;
            if( select_y[0] )
            {   for(size_t j = 0; j < n; ++j)
                {   if( select_x[j] )
                        ++nnz;
                }
            }
            // pattern_out
            pattern_out.resize(m, n, nnz);
            size_t k = 0;
            if( select_y[0] )
            {   for(size_t j = 0; j < n; ++j)
                {   if( select_x[j] )
                        pattern_out.set(k++, 0, j);
                }
            }
            assert( k == nnz );
            return true;
        }
        //
        // hes_sparsity
        bool hes_sparsity(
            size_t                                     call_id     ,
            const CppAD::vector<bool>&                 select_x    ,
            const CppAD::vector<bool>&                 select_y    ,
            CppAD::sparse_rc< CppAD::vector<size_t> >& pattern_out ) override
        {   size_t n = select_x.size();
# ifndef NDEBUG
            size_t m = select_y.size();
            assert( call_id == 0 );
            assert( m == 1 );
# endif
            // nnz
            size_t nnz = 0;
            if( select_y[0] )
            {   for(size_t j = 0; j < n; ++j)
                {   if( select_x[j] )
                        ++nnz;
                }
            }
            // pattern_out
            pattern_out.resize(n, n, nnz);
            size_t k = 0;
            if( select_y[0] )
            {   for(size_t j = 0; j < n; ++j)
                {   if( select_x[j] )
                        pattern_out.set(k++, j, j);
                }
            }
            return true;
        }
    };
}
// END_DEFINE_ATOMIC_FUNCTION

// BEGIN_USE_ATOMIC_FUNCTION
bool norm_sq(void)
{   // ok, eps
    bool ok    = true;
    double eps = 10. * CppAD::numeric_limits<double>::epsilon();
    //
    // atom_norm_sq
    atomic_norm_sq afun("atomic_norm_sq");
    //
    // n, m
    size_t n = 2;
    size_t m = 1;
    //
    // x
    CPPAD_TESTVECTOR(double) x(n);
    for(size_t j = 0; j < n; ++j)
        x[j] = 1.0 / (double(j) + 1.0);
    //
    // ax
    CPPAD_TESTVECTOR( CppAD::AD<double> ) ax(n);
    for(size_t j = 0; j < n; ++j)
        ax[j] = x[j];
    CppAD::Independent(ax);
    //
    // ay
    CPPAD_TESTVECTOR( CppAD::AD<double> ) ay(m);
    afun(ax, ay);
    //
    // f
    CppAD::ADFun<double> f;
    f.Dependent (ax, ay);
    //
    // check
    double check = 0.0;
    for(size_t j = 0; j < n; ++j)
        check += x[j] * x[j];
    //
    // ok
    // check ay[0]
    ok &= CppAD::NearEqual( Value(ay[0]) , check,  eps, eps);
    //
    // ok
    // check zero order forward mode
    CPPAD_TESTVECTOR(double) y(m);
    y   = f.Forward(0, x);
    ok &= CppAD::NearEqual(y[0] , check,  eps, eps);
    //
    // n2, check
    size_t n2  = n / 2;
    check      = 2.0 * x[n2];
    //
    // ok
    // check first order forward mode partial w.r.t. x[n2]
    CPPAD_TESTVECTOR(double) x1(n), y1(m);
    for(size_t j = 0; j < n; ++j)
            x1[j] = 0.0;
    x1[n2] = 1.0;
    y1     = f.Forward(1, x1);
    ok    &= CppAD::NearEqual(y1[0] , check,  eps, eps);
    //
    // ok
    // first order reverse mode
    size_t q = 1;
    CPPAD_TESTVECTOR(double)  w(m), dw(n * q);
    w[0]  = 1.;
    dw    = f.Reverse(q, w);
    for(size_t j = 0; j < n; ++j)
    {   check = 2.0 * x[j];
        ok &= CppAD::NearEqual(dw[j] , check,  eps, eps);
    }
    //
    // pattern_out
    // reverse mode Jacobian sparstiy pattern
    CppAD::sparse_rc< CPPAD_TESTVECTOR(size_t) > pattern_in, pattern_out;
    pattern_in.resize(m, m, m);
    for(size_t i = 0; i < m; ++i)
        pattern_in.set(i, i, i);
    bool transpose     = false;
    bool dependency    = false;
    bool internal_bool = false;
    f.rev_jac_sparsity(
        pattern_in, transpose, dependency, internal_bool, pattern_out
    );
    //
    // ok
    ok &= pattern_out.nnz() == n;
    CPPAD_TESTVECTOR(size_t) row_major  = pattern_out.row_major();
    for(size_t j = 0; j < n; ++j)
    {   size_t r = pattern_out.row()[ row_major[j] ];
        size_t c = pattern_out.col()[ row_major[j] ];
        ok      &= r == 0 && c == j;
    }
    //
    // pattern_out
    // forward mode Hessian sparsity pattern
    CPPAD_TESTVECTOR(bool) select_x(n), select_y(m);
    for(size_t j = 0; j < n; ++j)
        select_x[j] = true;
    for(size_t i = 0; i < m; ++i)
        select_y[i] = true;
    internal_bool = false;
    f.for_hes_sparsity(
        select_x, select_y, internal_bool, pattern_out
    );
    //
    // ok
    ok &= pattern_out.nnz() == n;
    row_major  = pattern_out.row_major();
    for(size_t j = 0; j < n; ++j)
    {   size_t r   = pattern_out.row()[ row_major[j] ];
        size_t c   = pattern_out.col()[ row_major[j] ];
        ok        &= r == j && c == j;
    }
    //
    return ok;
}
// END_USE_ATOMIC_FUNCTION