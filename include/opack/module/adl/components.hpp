/*****************************************************************//**
 * \file   components.hpp
 * \brief  API components introduced by module Activity-DL.
 *
 * \author Tristan
 * \date   September 2022
 *********************************************************************/
#pragma once

#include <opack/core.hpp>

namespace adl
{
	/** Contains order of a task in regards to its parent. */
	struct Order
	{
		size_t value{ 0 };
	};

	enum class LogicalConstructor { AND, OR };
	enum class TemporalConstructor { IND, SEQ_ORD, ORD, SEQ, PAR };

	struct Constructor
	{
		LogicalConstructor logical;
		TemporalConstructor temporal;
	};

	using cond_func_t = std::function<bool(opack::Entity)>;
	struct Condition
	{
		cond_func_t func;
		Condition() = default;
		Condition(cond_func_t f) : func(std::move(f)){}
	};

	struct Contextual : Condition { using Condition::Condition; };
	struct Favorable : Condition { using Condition::Condition; };
	struct Nomological : Condition { using Condition::Condition; };
	struct Regulatory : Condition { using Condition::Condition; };
	struct Satisfaction : Condition { using Condition::Condition; };
}
