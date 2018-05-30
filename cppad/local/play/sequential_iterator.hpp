# ifndef CPPAD_LOCAL_PLAY_SEQUENTIAL_ITERATOR_HPP
# define CPPAD_LOCAL_PLAY_SEQUENTIAL_ITERATOR_HPP

/* --------------------------------------------------------------------------
CppAD: C++ Algorithmic Differentiation: Copyright (C) 2003-18 Bradley M. Bell

CppAD is distributed under multiple licenses. This distribution is under
the terms of the
                    Eclipse Public License Version 1.0.

A copy of this license is included in the COPYING file of this distribution.
Please visit http://www.coin-or.org/CppAD/ for information on other licenses.
-------------------------------------------------------------------------- */

// BEGIN_CPPAD_LOCAL_PLAY_NAMESPACE
namespace CppAD { namespace local { namespace play {

/*!
\file sequential_iterator.hpp
*/

/*!
Constant sequential iterator for a player object.

Except for constructor, the public API for this class is a subset of
the API for the random_iterator class.
*/
template <class Base>
class const_sequential_iterator {
private:
	/// pointer to the first operator in the player, BeginOp = *op_begin_
	const CPPAD_OP_CODE_TYPE* op_begin_;

	/// pointer one past last operator in the player, EndOp = *(op_end_ - 1)
	const CPPAD_OP_CODE_TYPE* op_end_;

	/// pointer to the first argument for the first operator
	const addr_t*             arg_begin_;

	/// pointer on past last argumemnt for last operator
	const addr_t*             arg_end_;

	/// pointer to current operator
	const CPPAD_OP_CODE_TYPE* op_cur_;

	/// pointer to first argument for current operator
	const addr_t*             arg_;

	/// index of last result for current operator
	size_t                    var_index_;

	/// number of variables in tape
	size_t                    num_var_;

	/// value of current operator; i.e. op_ = *op_cur_
	OpCode                    op_;
public:
	/// assignment operator
	void operator=(const const_sequential_iterator& rhs)
	{
		op_begin_  = rhs.op_begin_;
		op_end_    = rhs.op_end_;
		arg_begin_ = rhs.arg_begin_;
		arg_end_   = rhs.arg_end_;
		op_cur_    = rhs.op_cur_;
		arg_       = rhs.arg_;
		var_index_ = rhs.var_index_;
		num_var_   = rhs.num_var_;
		op_        = rhs.op_;
		return;
	}
	/*!
	Create a sequential iterator starting either at beginning or end of tape

	\param num_var
	is the number of variables in the tape.

	\param op_vec
	is the vector of operators on the tape.

	\param arg_vec
	is the vector of arguments for all the operators

	\param op_index
	is the operator index that iterator will start at.
	It must be zero or op_vec_->size() - 1.

	\par Assumptions
	- OpCode(op_vec_[0]) == BeginOp
	- OpCode(op_vec_[op_vec_->size() - 1]) == EndOp
	*/
	const_sequential_iterator(
		size_t                                num_var    ,
		const pod_vector<CPPAD_OP_CODE_TYPE>* op_vec     ,
		const pod_vector<addr_t>*             arg_vec    ,
		size_t                                op_index   )
	:
	op_begin_   ( op_vec->data() )                   ,
	op_end_     ( op_vec->data() + op_vec->size() )  ,
	arg_begin_  ( arg_vec->data() )                  ,
	arg_end_    ( arg_vec->data() + arg_vec->size() ),
	num_var_    ( num_var )
	{	if( op_index == 0 )
		{
			// index of last result for BeginOp
			var_index_ = 0;
			//
			// first argument to BeginOp
			arg_       = arg_vec->data();
			//
			// BeginOp
			op_cur_    = op_begin_;
			op_        = OpCode( *op_cur_ );
			CPPAD_ASSERT_UNKNOWN( op_ == BeginOp );
			CPPAD_ASSERT_NARG_NRES(op_, 1, 1);
		}
		else
		{	CPPAD_ASSERT_UNKNOWN(op_index == op_vec->size()-1);
			//
			// index of last result for EndOp
			var_index_ = num_var - 1;
			//
			// first argument to EndOp (has no arguments)
			arg_ = arg_vec->data() + arg_vec->size();
			//
			// EndOp
			op_cur_    = op_end_ - 1;
			op_        = OpCode( *op_cur_ );
			CPPAD_ASSERT_UNKNOWN( op_ == EndOp );
			CPPAD_ASSERT_NARG_NRES(op_, 0, 0);
		}
	}
	/*!
	Advance iterator to next operator
	*/
	const_sequential_iterator<Base>& operator++(void)
	{
		// first argument for next operator
		arg_ += NumArg(op_);
		//
		// next operator
		++op_cur_;
		op_ = OpCode( *op_cur_ );
		//
		// last result for next operator
		var_index_ += NumRes(op_);
		//
		return *this;
	}
	/*!
	Correction applied before ++ operation when current operator
	is CSumOp or CSkipOp.
	*/
	void correct_before_increment(void)
	{	// number of arguments for this operator depends on argument data
		CPPAD_ASSERT_UNKNOWN( NumArg(op_) == 0 );
		const addr_t* arg = arg_;
		//
		// CSumOp
		if( op_ == CSumOp )
		{	//
			CPPAD_ASSERT_UNKNOWN( arg + 1 < arg_end_ );
			addr_t n_var      = arg[0] + arg[1];
			CPPAD_ASSERT_UNKNOWN( n_var == arg[3 + n_var] );
			//
			// add actual number of arguments to arg_
			arg_ += 4 + n_var;
		}
		//
		// CSkip
		else
		{	CPPAD_ASSERT_UNKNOWN( op_ == CSkipOp );
			//
			CPPAD_ASSERT_UNKNOWN( arg + 5 < arg_end_ );
			addr_t n_skip     = arg[4] + arg[5];
			CPPAD_ASSERT_UNKNOWN( n_skip == arg[6 + n_skip] );
			//
			// add actual number of arguments to arg_
			arg_ += 7 + n_skip;
		}
		return;
	}
	/*!
	Backup iterator to previous operator
	*/
	const_sequential_iterator<Base>& operator--(void)
	{	//
		// last result for next operator
		var_index_ -= NumRes(op_);
		//
		// next operator
		--op_cur_;
		op_ = OpCode( *op_cur_ );
		//
		// first argument for next operator
		arg_ -= NumArg(op_);
		//
		return *this;
	}
	/*!
	Correction applied after -- operation when current operator
	is CSumOp or CSkipOp.

	\param arg [out]
	corrected point to arguments for this operation.
	*/
	void correct_after_decrement(const addr_t*& arg)
	{	// number of arguments for this operator depends on argument data
		CPPAD_ASSERT_UNKNOWN( NumArg(op_) == 0 );
		//
		// infromation for number of arguments is stored in arg_ - 1
		CPPAD_ASSERT_UNKNOWN( arg_begin_ < arg_ );
		//
		// CSumOp
		if( op_ == CSumOp )
		{	// number of variables is stored in last argument
			addr_t n_var = *(arg_ - 1);
			//
			// corrected index of first argument to this operator
			arg = arg_ -= 4 + n_var;
			//
			CPPAD_ASSERT_UNKNOWN( arg[0] + arg[1] == n_var );
		}
		//
		// CSkip
		else
		{	CPPAD_ASSERT_UNKNOWN( op_ == CSkipOp );
			//
			// number to possibly skip is stored in last argument
			addr_t n_skip = *(arg_ - 1);
			//
			// corrected index of frist argument to this operator
			arg = arg_ -= 7 + n_skip;
			//
			CPPAD_ASSERT_UNKNOWN( arg[4] + arg[5] == n_skip );
		}
		CPPAD_ASSERT_UNKNOWN( arg_begin_ <= arg );
		CPPAD_ASSERT_UNKNOWN( arg + NumArg(op_) <= arg_end_ );
	}
	/*!
	\brief
	Get information corresponding to current operator.

	\param op [out]
	op code for this operator.

	\param arg [out]
	pointer to the first arguement to this operator.

	\param var_index [out]
	index of the last variable (primary variable) for this operator.
	If there is no primary variable for this operator, var_index
	is not sepcified and could have any value.
	*/
	void op_info(
		OpCode&        op         ,
		const addr_t*& arg        ,
		size_t&        var_index  ) const
	{	// op
		CPPAD_ASSERT_UNKNOWN( op_begin_ <= op_cur_ && op_cur_ < op_end_ )
		op        = op_;
		//
		// arg
		arg = arg_;
		CPPAD_ASSERT_UNKNOWN( arg_begin_ <= arg );
		CPPAD_ASSERT_UNKNOWN( arg + NumArg(op) <= arg_end_ );
		//
		// var_index
		CPPAD_ASSERT_UNKNOWN( var_index_ < num_var_ || NumRes(op) == 0 );
		var_index = var_index_;
	}
	/// current operator index
	size_t op_index(void)
	{	return op_cur_ - op_begin_; }
};

} } } // BEGIN_CPPAD_LOCAL_PLAY_NAMESPACE

# endif
