/*****************************************************************//**
 * \file   adl.hpp
 * \brief  Defines a module for using ACTIVITY-DL.
 * 
 * \author Tristan 
 * \date   September 2022
 *********************************************************************/
#pragma once

#include <map>
#include <opack/core.hpp>
#define ADL_TRACE

/** Shorthand for creating an activity type.*/
#define ADL_ACTIVITY(name) OPACK_SUB_PREFAB(name, adl::Activity)

// TODO Should improve potential actions call to tell whether or not an instance is finished or not.
// TODO Should automatically add default condition to actions prefab
/**
 * Module for anything related to Activity-DL.
 * It's a domain language to represent an activity by a hierarchy of tasks.
 */
struct adl
{
 	/** A task is any node in an activity tree, that is not a leaf, i.e an action. */
	struct Task {};

	/** Types used for explorer. */
    struct activities {};

	/** An activity is a tree of tasks, with actions as leaf. */
	struct Activity : opack::internal::root<Activity>
	{
        using entities_folder_t = activities;
	};

	OPACK_ACTION(Action);


	/** Contains order of a task in regards to its parent. */
	struct Order
	{
		std::size_t value{ 0 };
	};

	enum class LogicalConstructor { AND, OR, XOR };
	enum class TemporalConstructor { IND, SEQ_ORD, ORD, SEQ, PAR };

	struct Constructor
	{
		LogicalConstructor logical;
		TemporalConstructor temporal;
	};

	/** An optional task does not fail satisfaction of parent task. */
	struct Optional {};

	using cond_func_t = std::function<bool(opack::EntityView)>;
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

    /** Import Activity-DL module in your world. */
    adl(opack::World& world);

	/** Create a task named @c name, with @c parent as parent. */
	static opack::Entity task(const char* name,
		opack::Entity parent,
		LogicalConstructor logical = LogicalConstructor::AND,
		TemporalConstructor temporal = TemporalConstructor::SEQ_ORD,
		std::size_t arity_max = 1,
		std::size_t arity_min = 1
	);

	/** True if @c task has any children, i.e sub-tasks or actions, false otherwise.*/
	static bool has_children(opack::EntityView task);

	/**
	 *@brief True if @c task is finished.
	 *
	 *A task is finished if :
	 *	- it has no child, i.e an action, and it has been done.
	 *	- it has child and there are finished.
	 */
	static bool is_finished(opack::EntityView task);

	/** True if one child or itself has started. */
	static bool has_started(opack::EntityView task);

	/** True if one child or itself is in progress (started but not finished). */
	static bool in_progress(opack::EntityView task);

	/** Set logical constructor of task. */
	static void logical_constructor(opack::Entity task, LogicalConstructor constructor);

	/** Returns logical constructor of task. */
	static LogicalConstructor logical_constructor(opack::EntityView task);

	/** Set temporal constructor of task. */
	static void temporal_constructor(opack::Entity task, TemporalConstructor constructor);

	/** Returns temporal constructor of task. */
	static TemporalConstructor temporal_constructor(opack::EntityView task);

	/** Returns order of @c task in regards to its parent. */
	static std::size_t order(opack::EntityView task);

	/** Returns number of direct children of @c task. */
	static std::size_t children_count(opack::EntityView task);

	/** Returns number of direct and indirect children of @c task. */
	static std::size_t size(opack::EntityView task);

	/** Returns parent task of @c task, null entity otherwise. */
	static opack::Entity parent_of(opack::EntityView task);

	/** Returns root task of @c task, null entity otherwise. */
	static opack::Entity get_root(opack::EntityView task);

	/** Returns depth of task in activity tree (0 -> root). */
	static std::size_t get_depth(opack::EntityView task);

	/** True if @c task has no parent task. */
	static bool is_root(opack::EntityView task);

	/** True if @c entity is a task (has children and a constructor{logical, temporal}. */
	static bool is_leaf(opack::EntityView task);

	/** Returns true if @c task is satisfied. True by default or
	 *depending on the satisfaction condition
	 */
	static bool is_satisfied(opack::EntityView task);

	/** Retrieve pointer to value @c T, stored in context
	 *(either from current task or parent tasks).
	 */
	template<typename T>
	static const T* ctx_value(opack::EntityView task)
	{
		const T* result = task.get<T>();
		if(!result)
		{
			if(auto parent = parent_of(task))
				result = ctx_value<T>(parent);
		}
		return result;
	}

    /** Store value @c T in context (first into parent task or itself otherwise). */
	template<typename T>
	static void ctx_value(opack::Entity task, T&& value)
	{
		if(auto parent = parent_of(task))
			ctx_value<T>(parent, std::forward<T>(value));
		else
			task.set<T>({ value });
	}

	/** Retrieve entity from context using relation @c (R, *). */
	template<typename R>
	static opack::Entity ctx_entity(opack::EntityView task)
	{
		auto result = task.target<R>();
		if(!result)
		{
			if(auto parent = parent_of(task))
				result = ctx_entity<R>(parent);
		}
		return result;
	}

	/** Store entity in context using relation @c (R, *). */
	template<typename R>
	static void ctx_entity(opack::Entity task, opack::Entity entity)
	{
		if(auto parent = parent_of(task))
			ctx_entity<R>(parent, entity);
		else
			task.add<R>(entity);
	}

	/** Set condition @c T of @c task. */
	template<std::derived_from<Condition> T>
	static void condition(opack::Entity task, cond_func_t func)
	{
		task.set<T>({ func });
	}

	/** Check if condition @c T of @c task is true or false. */
	template<std::derived_from<Condition> T>
	static bool check_condition(opack::EntityView task)
	{
		return !task.has<T>() || task.get<T>()->func(task);
	}

	/** Create an activity model referred as @c T. */
	template<typename T>
	requires opack::SubPrefab<T> && std::derived_from<T, adl::Activity>
	static opack::Entity activity(
		opack::World& world,
		LogicalConstructor logical = LogicalConstructor::AND,
		TemporalConstructor temporal = TemporalConstructor::SEQ_ORD,
		std::size_t arity_max = 1,
		std::size_t arity_min = 1
	)
	{
		return opack::init<T>(world)
			.template set<Constructor>({ logical, temporal })
			.template set<opack::Arity>({ arity_min, arity_max })
			;
	}

	/** Add activity @c T as a children of @c parent. */
	template<std::derived_from<Activity> T>
	static opack::Entity compose(opack::Entity parent)
	{
		auto world = parent.world();
		auto instance = opack::spawn<T>(world);
		instance.child_of(parent);
		instance.template set<Order>({ adl::children_count(parent) });
		return instance;
	}

	/** Shorthand to create an action from prefab @c T and set it as a child of @c parent. */
	template<std::derived_from<opack::Action> T>
	static opack::Entity action(opack::Entity parent)
	{
		auto world = parent.world();
		return world.prefab().is_a<T>()
	        .child_of(parent)
	        .template add<opack::DoNotClean>()
		    .template set<Order>({ children_count(parent) });
	}

	/** Returns an ordered map of @c task children. */
	static std::map<std::size_t, opack::Entity> children(opack::EntityView task);

	/** Add potential actions to output iterator @c out and returns true if task is satisfied.*/
	template<typename OutputIterator>
	static void compute_potential_actions(opack::Entity task, OutputIterator out, std::function<bool(opack::EntityView)> should_add = opack::always)
	{
#ifdef ADL_TRACE
		opack_trace("{0:-^{1}} Evaluating task {2}", "", 2*adl::get_depth(task), task.path().c_str());
		opack_trace("{0:-^{1}}-> is leaf ? {2}", "", 2*adl::get_depth(task), is_leaf(task));
		opack_trace("{0:-^{1}}-> in progress ? {2}", "", 2*adl::get_depth(task), in_progress(task));
		opack_trace("{0:-^{1}}-> is finished ? {2}", "", 2*adl::get_depth(task), is_finished(task));
		opack_trace("{0:-^{1}}-> is satisfied ? {2}", "", 2*adl::get_depth(task), is_satisfied(task));
		opack_trace("{0:-^{1}}-> is contextual ? {2}", "", 2*adl::get_depth(task), check_condition<Contextual>(task));
#endif

		if (is_leaf(task))
		{
			opack_assert(opack::is_a<opack::Action>(task), "Leaf task {} is not an action. Does it inherit from opack::Action or adl::Action ?", task.path().c_str());
			if (!in_progress(task) && !is_finished(task) && !is_satisfied(task) && check_condition<Contextual>(task) && should_add(task))
				*out++ = task;
			return ;
		}

		opack_assert(task.has<Constructor>(), "Task {} doesn't have a temporal constructor component.", task.path().c_str());
		switch (task.get<Constructor>()->temporal)
		{
		case TemporalConstructor::PAR: // TODO
		case TemporalConstructor::IND: // Every potential actions are added, even if another isn't finished.
			for (auto [order, subtask] : children(task))
			{
				compute_potential_actions(subtask, out, should_add);
			}
			break;
		case TemporalConstructor::SEQ: // Every potential actions are added, if last one is finished
			if(!in_progress(task))
			{
				for (auto [order, subtask] : children(task))
				{
					compute_potential_actions(subtask, out, should_add);
				}
			}
			break;
		case TemporalConstructor::SEQ_ORD: // First non satisfied task is next, if last one is finished.
			if(!in_progress(task))
			{
				bool first {true};
				auto subtasks {children(task)};
				for (auto it {subtasks.begin()} ; it != subtasks.end() && first ; ++it)
				{
					if (auto subtask = it->second; !is_finished(subtask) && !is_satisfied(subtask))
					{
						compute_potential_actions(subtask, out, should_add);
						first = false;
					}
				}
			}
			break;
		case TemporalConstructor::ORD: // First non satisfied task is next, even if last one isn't finished.
			{
				bool first {true};
				auto subtasks {children(task)};
				for (auto it {subtasks.begin()} ; it != subtasks.end() && first ; ++it)
				{
					if (auto subtask = it->second; !is_finished(subtask) && !in_progress(subtask) && !is_satisfied(subtask))
					{
						compute_potential_actions(subtask, out, should_add);
						first = false;
					}
				}
			}
			break;
		}
	}

	/**
	 * Iterate an activity using dfs.
	 * Function @c func signature is @c void<opack::Entity>.
	 */
	template<typename Func>
	static void traverse_dfs(opack::EntityView task, Func&& func)
	{
		func(task);
		task.children([&func](opack::Entity e) {traverse_dfs(e, std::forward<Func>(func)); });
	}

	/**
	 * Iterate an activity using dfs.
	 * Function @c func signature is @c void<opack::Entity>.
	 */
	template<typename Func>
	static void traverse_dfs(opack::Entity task, Func&& func)
	{
		func(task);
		task.children([&func](opack::Entity e) {traverse_dfs(e, std::forward<Func>(func)); });
	}
};
