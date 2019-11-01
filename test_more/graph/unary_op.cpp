/* --------------------------------------------------------------------------
CppAD: C++ Algorithmic Differentiation: Copyright (C) 2003-19 Bradley M. Bell

CppAD is distributed under the terms of the
             Eclipse Public License Version 2.0.

This Source Code may also be made available under the following
Secondary License when the conditions for such availability set forth
in the Eclipse Public License, Version 2.0 are satisfied:
      GNU General Public License, Version 2.0 or later.
---------------------------------------------------------------------------- */
/*
$begin graph_unary_op.cpp$$
$spell
    sin
$$

$section Graph Unary Operator: Example and Test$$

$head Source Code$$
$srcfile%test_more/graph/unary_op.cpp%0%// BEGIN C++%// END C++%1%$$

$end
*/
// BEGIN C++
# include <cppad/cppad.hpp>

bool unary_op(void)
{   bool ok = true;
    using CppAD::vector;
    using CppAD::AD;
    using std::string;
    typedef CppAD::local::graph::graph_op_struct graph_op_struct;
    typedef CppAD::local::graph::cpp_graph       cpp_graph;
    double eps99 = 99.0 * std::numeric_limits<double>::epsilon();
    //
    // AD graph example
    // node_1 : p[0]
    // node_2 : x[0]
    // node_3 : c[0]
    // node_4 : sin(p[0])
    // node_5 : sin(x[0])
    // node_6 : sin(c[0])
    // node_7 : sin(p[0]) + sin(x[0]) + sin(c[0])
    // y[0]   = sin(p[0]) + sin(x[0]) + sin(c[0])
    //
    //
    // C++ graph object
    cpp_graph graph_obj;
    //
    //
    // structure corresponding to one operator
    graph_op_struct         op_usage;
    //
    // size_t value that is not used
    //
    // set scalars
    graph_obj.function_name_set("unary_op example");
    size_t n_dynamic_ind = 1;
    graph_obj.n_dynamic_ind_set(n_dynamic_ind);
    size_t n_independent = 1;
    graph_obj.n_independent_set(n_independent);
    graph_obj.constant_vec_push_back( -0.1 );
    //
    // node_4 : sin(p[0])
    op_usage.op_enum          = CppAD::local::graph::sin_graph_op;
    op_usage.start_arg        = graph_obj.operator_arg_size();
    graph_obj.operator_vec_push_back(op_usage);
    graph_obj.operator_arg_push_back(1);
    //
    // node_5 : sin(x[0])
    op_usage.start_arg        = graph_obj.operator_arg_size();
    graph_obj.operator_vec_push_back(op_usage);
    graph_obj.operator_arg_push_back(2);
    //
    // node_6 : sin(c[0])
    op_usage.start_arg        = graph_obj.operator_arg_size();
    graph_obj.operator_vec_push_back(op_usage);
    graph_obj.operator_arg_push_back(3);
    //
    // node_7 : sin(p[0]) + sin(x[0]) + sin(c[0])
    op_usage.op_enum          = CppAD::local::graph::sum_graph_op;
    op_usage.start_arg        = graph_obj.operator_arg_size();
    graph_obj.operator_vec_push_back(op_usage);
    graph_obj.operator_vec_push_back(op_usage);
    graph_obj.operator_arg_push_back(4);
    graph_obj.operator_arg_push_back(5);
    graph_obj.operator_arg_push_back(6);
    //
    // y[0]   = sin(p[0]) + sin(x[0]) + sin(c[0])
    graph_obj.dependent_vec_push_back(7);
    //
    // f(x, p) = sin(p_0) + sin(x_0) + sin(c_0)
    CppAD::ADFun<double> f;
    f.from_graph(graph_obj);
    ok &= f.Domain() == 1;
    ok &= f.Range() == 1;
    ok &= f.size_dyn_ind() == 1;
    //
    // value of constant in function
    vector<double> c(1);
    c[0] = -0.1;
    //
    // set independent variables and parameters
    vector<double> p(1), x(1);
    p[0] = 0.2;
    x[0] = 0.3;
    //
    // compute y = f(x, p)
    f.new_dynamic(p);
    vector<double> y = f.Forward(0, x);
    //
    // check result
    double check = std::sin(p[0]) + std::sin(x[0]) + std::sin(c[0]);
    ok &= CppAD::NearEqual(y[0], check, eps99, eps99);
    // -----------------------------------------------------------------------
    // Convert to Graph graph and back again
    f.to_graph(graph_obj);
    f.from_graph(graph_obj);
    // -----------------------------------------------------------------------
    //
    // compute y = f(x, p)
    f.new_dynamic(p);
    y = f.Forward(0, x);
    //
    // check result
    ok &= CppAD::NearEqual(y[0], check, eps99, eps99);
    //
    return ok;
}
// END C++
