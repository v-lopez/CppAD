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
$begin atomic_vector_sub_op.cpp$$

$section Atomic Vector Subtract Operator: Example Implementation$$

$head Forward Mode$$
see theory for forward mode
$cref/subtraction/ForwardTheory/Binary Operators/Subtraction/$$.

$head Example$$
The file $cref atomic_vector_sub.cpp$$ contains an example
and test for this operator.

$head Source$$
$srcthisfile%0%// BEGIN C++%// END C++%1%$$

$end
*/
// BEGIN C++
# include "atomic_vector.hpp"

// --------------------------------------------------------------------------
// forward_sub
void atomic_vector::forward_sub(
    size_t                                           m,
    size_t                                           p,
    size_t                                           q,
    const CppAD::vector<double>&                     tx,
    CppAD::vector<double>&                           ty)
{
    for(size_t k = p; k < q; ++k)
    {   for(size_t i = 0; i < m; ++i)
        {   size_t u_index  =       i * q + k;
            size_t v_index  = (m + i) * q + k;
            size_t y_index  =       i * q + k;
            // y_i^k = u_i^k - v_i^k
            ty[y_index]     = tx[u_index] - tx[v_index];
        }
    }
}
void atomic_vector::forward_sub(
    size_t                                           m,
    size_t                                           p,
    size_t                                           q,
    const CppAD::vector< CppAD::AD<double> >&        atx,
    CppAD::vector< CppAD::AD<double> >&              aty)
{
    size_t n = 2 * m;
    assert( atx.size() == n * q );
    assert( aty.size() == m * q );
    //
    // atu, atv
    ad_vector::const_iterator atu = atx.begin();
    ad_vector::const_iterator atv = atu + difference_type(m * q);
    //
    // ax
    ad_vector ax(n);
    ad_vector::iterator au = ax.begin();
    ad_vector::iterator av = au + difference_type(m);
    //
    // ay
    ad_vector ay(m);
    //
    for(size_t k = p; k < q; ++k)
    {   // au = u^k
        copy_mat_to_vec(m, q, k, atu, au);
        // av = v^k
        copy_mat_to_vec(m, q, k, atv, av);
        // ay = au - av
        (*this)(sub_enum, ax, ay); // atomic vector sub
        // y^k = ay
        copy_vec_to_mat(m, q, k, ay.begin(), aty.begin());
    }
}
// --------------------------------------------------------------------------
// reverse_sub
void atomic_vector::reverse_sub(
    size_t                                           m,
    size_t                                           q,
    const CppAD::vector<double>&                     tx,
    const CppAD::vector<double>&                     ty,
    CppAD::vector<double>&                           px,
    const CppAD::vector<double>&                     py)
{
    for(size_t k = 0; k < q; ++k)
    {   for(size_t i = 0; i < m; ++i)
        {   size_t u_index  =       i * q + k;
            size_t v_index  = (m + i) * q + k;
            size_t y_index  =       i * q + k;
            // y_i^k = u_i^k + v_i^k
            px[u_index] =   py[y_index];
            px[v_index] = - py[y_index];
        }
    }
}
void atomic_vector::reverse_sub(
    size_t                                           m,
    size_t                                           q,
    const CppAD::vector< CppAD::AD<double> >&        atx,
    const CppAD::vector< CppAD::AD<double> >&        aty,
    CppAD::vector< CppAD::AD<double> >&              apx,
    const CppAD::vector< CppAD::AD<double> >&        apy)
{
# ifndef NDEBUG
    size_t n = 2 * m;
    assert( atx.size() == n * q );
    assert( aty.size() == m * q );
    assert( apx.size() == n * q );
    assert( apy.size() == m * q );
# endif
    //
    // apu, apv
    ad_vector::iterator apu = apx.begin();
    ad_vector::iterator apv = apu + difference_type(m * q);
    //
    // ax
    ad_vector ax(m);
    ad_vector::iterator au = ax.begin();
    //
    // ay
    ad_vector ay(m);
    //
    for(size_t k = 0; k < q; ++k)
    {   // au = apy^k
        copy_mat_to_vec(m, q, k, apy.begin(), au);
        // apu^k = au
        copy_vec_to_mat(m, q, k, au, apu);
        // ay = - au
        (*this)(neg_enum, ax, ay); // atomic vector neg
        // apv^k = ay
        copy_vec_to_mat(m, q, k, ay.begin(), apv);
    }
}


// END C++
