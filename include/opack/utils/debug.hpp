/*****************************************************************//**
 * \file   debug.hpp
 * \brief  Utility header for debugging. Defines one assert function :
 * * assert(condition, message, ...) : If @c condition is evaluated to @ false, programs stop with formatted @c message.
 * And some log function :
 * 1. trace(condition, message, ...) : If @c condition is evaluated to @ false, a warning is emitted with formatted @c message.
 * 2. warn(condition, message, ...) : If @c condition is evaluated to @ false, a warning is emitted with formatted @c message.
 * 
 * \author Tristan
 * \date   September 2022
 *********************************************************************/
#pragma once

#include <flecs.h>		// For consistency, we use flecs underlying assert.
#include <fmt/core.h>	// For more readable messages, we need some string computation.
#include <fmt/color.h>	

/** Runtime assert only when @c OPACK_RUNTIME_CHECK is true.
 *	@param condition Assert if @c condition is false
 *	@param message A fmt string literal, followed by your arguments
 *
 *	Usage:
 *	@code{.cpp}
 *	opack_assert(false, "This will assert and this {} will be formatted", "argument");
 *	@endcode
 */
#ifdef OPACK_RUNTIME_CHECK
#define opack_assert(condition, message, ...)\
	ecs_assert(condition, ECS_INVALID_PARAMETER, fmt::format(fmt::runtime(message) __VA_OPT__(,) __VA_ARGS__).c_str())
#else
#define opack_assert(condition, message, ...) ((void)0)
#endif

/** Runtime warning emitted only when @c OPACK_RUNTIME_CHECK is true.
 *	@param condition Emits a warning if @c condition is false
 *	@param message A fmt string literal, followed by your arguments
 *
 *	Usage:
 *	@code{.cpp}
 *	opack_warn_if(false, "This will emit a warning and this {} will be formatted", "argument");
 *	@endcode
 */
#ifdef OPACK_LOG
#define opack_warn_if(condition, message, ...)\
	if(!(condition))\
	{\
		fmt::print(fmt::fg(fmt::color::coral), "[ WARN] : ");\
		fmt::print(fmt::fg(fmt::color::dim_gray), message __VA_OPT__(,) __VA_ARGS__);\
		fmt::print("\n");\
	}
#else
#define opack_warn_if(condition, message, ...) ((void)0)
#endif

/** Runtime warning emitted only when @c OPACK_RUNTIME_CHECK is true.
 *	@param message A fmt string literal, followed by your arguments
 *
 *	Usage:
 *	@code{.cpp}
 *	opack_warn("This will emit a warning and this {} will be formatted", "argument");
 *	@endcode
 */
#ifdef OPACK_LOG
#define opack_warn(message, ...)\
	opack_warn_if(false, message __VA_OPT__(,) __VA_ARGS__)
#else
#define opack_warn(false, message, ...) ((void)0)
#endif


/** Runtime trace emitted only when @c OPACK_RUNTIME_CHECK is true.
 *	@param condition Emits a trace if @c condition is false
 *	@param message A fmt string literal, followed by your arguments
 *
 *	Usage:
 *	@code{.cpp}
 *	opack_trace_if(false, "This will emit a trace and this {} will be formatted", "argument");
 *	@endcode
 */
#ifdef OPACK_LOG
#define opack_trace_if(condition, message, ...)\
	if(!(condition))\
	{\
		fmt::print(fmt::fg(fmt::color::light_steel_blue), "[TRACE] : ");\
		fmt::print(fmt::fg(fmt::color::dim_gray), message __VA_OPT__(,) __VA_ARGS__);\
		fmt::print("\n");\
	}
#else
#define opack_trace_if(condition, message, ...) ((void)0)
#endif

/** Runtime trace emitted only when @c OPACK_RUNTIME_CHECK is true.
 *	@param message A fmt string literal, followed by your arguments
 *
 *	Usage:
 *	@code{.cpp}
 *	opack_trace("This will emit a trace and this {} will be formatted", "argument");
 *	@endcode
 */
#ifdef OPACK_LOG
#define opack_trace(message, ...)\
	opack_trace_if(false, message __VA_OPT__(,) __VA_ARGS__)
#else
#define opack_trace(false, message, ...) ((void)0)
#endif
