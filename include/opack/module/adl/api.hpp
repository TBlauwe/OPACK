/*****************************************************************//**
 * \file   api.hpp
 * \brief  API of module Activity-DL.
 * 
 * \author Tristan
 * \date   September 2022
 *********************************************************************/
#pragma once

#include <opack/core.hpp>

#include "types.hpp"
#include "components.hpp"

namespace adl
{
	/** Create a task named @c name, with @c parent as parent. */
	opack::Entity task(const char* name,
		opack::Entity parent,
		LogicalConstructor logical = LogicalConstructor::AND,
		TemporalConstructor temporal = TemporalConstructor::SEQ_ORD,
		size_t arity_max = 1,
		size_t arity_min = 1
	);

	/** True if @c task has any children, i.e sub-tasks or actions, false otherwise.*/
	bool has_children(opack::Entity task);

	/**
	 *@brief True if @c task is finished.
	 *
	 *A task is finished if :
	 *	- it has no child, i.e an action, and it has been done.
	 *	- it has child and there are finished.
	 */
	bool is_finished(opack::Entity task);

	/** True if one child or itself has started. */
	bool has_started(opack::Entity task);

	/** True if one child or itself is in progress (started but not finished). */
	bool in_progress(opack::Entity task);

	/** Returns order of @c task in regards to its parent. */
	size_t order(opack::Entity task);

	/** Returns number of direct children of @c task. */
	size_t children_count(opack::Entity task);

	/** Returns number of direct and indirect children of @c task. */
	size_t size(opack::Entity task);

	/** Returns parent task of @c task, null entity otherwise. */
	opack::Entity parent_of(opack::Entity task);

	/** True if @c task has no parent task. */
	bool is_root(opack::Entity task);

	/** Returns true if @c task is satisfied. True by default or
	 *depending on the satisfaction condition
	 */
	bool is_satisfied(opack::Entity task);

	/** Returns true if @c task is a suitable task for action. */
	bool is_potential(opack::Entity task);

	/** Returns the @c n -th agent doing this @c task. */
	opack::Entity initiator(opack::Entity task, size_t n = 0);

	/** Retrieve pointer to value @c T, stored in context
	 *(either from current task or parent tasks).
	 */
	template<typename T>
	const T* ctx_value(opack::Entity task)
	{
		const T* result = task.get<T>();
		if(!result)
		{
			auto parent = task.parent();
			if (opack::is_a<Task>(parent))
				result = ctx_value<T>(parent);
		}
		return result;
	}

    /** Store value @c T in context (first into parent task or itself otherwise). */
	template<typename T>
	void ctx_value(opack::Entity task, T&& value)
	{
        auto parent = task.parent();
		if (opack::is_a<Task>(parent))
			ctx_value<T>(parent, std::forward<T>(value));
		else
			task.set<T>({ value });
	}

	/** Retrieve entity from context using relation @c (R, *). */
	template<typename R>
	opack::Entity ctx_entity(opack::Entity task)
	{
		auto result = task.target<R>();
		if(!result)
		{
			auto parent = task.parent();
			if (opack::is_a<Task>(parent))
				result = ctx_entity<R>(parent);

		}
		return result;
	}

	/** Store entity in context using relation @c (R, *). */
	template<typename R>
	void ctx_entity(opack::Entity task, opack::Entity entity)
	{
        auto parent = task.parent();
		if (opack::is_a<Task>(parent))
			ctx_entity<R>(parent, entity);
		else
			task.add<R>(entity);
	}

	/** Set condition @c T of @c task. */
	template<std::derived_from<Condition> T>
	void condition(opack::Entity& task, cond_func_t func)
	{
		task.emplace<T>(func);
	}

	/** Check if condition @c T of @c task is true or false. */
	template<std::derived_from<Condition> T>
	bool check_condition(opack::Entity& task)
	{
		return !task.has<T>() || (task.has<T>() && task.get<T>()->func(task));
	}

	/** Create an activity model referred as @c T. */
	template<ActivityPrefab T>
	opack::Entity activity(
		opack::World& world,
		LogicalConstructor logical = LogicalConstructor::AND,
		TemporalConstructor temporal = TemporalConstructor::SEQ_ORD,
		size_t arity_max = 1,
		size_t arity_min = 1
	)
	{
		return opack::init<T>(world)
			.template set<Constructor>({ logical, temporal })
			.template set<opack::Arity>({ arity_min, arity_max })
			.template child_of<Activity::prefabs_folder_t>();
	}

	/** Add activity @c T as a children of @c parent. */
	template<std::derived_from<Activity> T>
	opack::Entity compose(opack::Entity parent)
	{
		auto world = parent.world();
		auto instance = opack::spawn<T>(world);
		instance.child_of(parent);
		instance.template set<Order>({ adl::children_count(parent) });
		return instance;
	}

	/** Shorthand to create an action from prefab @c T and set it as a child of @c parent. */
	template<std::derived_from<opack::Action> T>
	opack::Entity action(opack::Entity parent)
	{
		auto world = parent.world();
		return world.prefab().is_a<T>()
	        .child_of(parent)
	        .template add<opack::DoNotClean>()
		    .template set<Order>({ children_count(parent) });
	}

	template<typename OutputIterator>
	bool potential_actions(opack::Entity task, OutputIterator out)
	{
		if (is_finished(task))
			return is_satisfied(task);

		if (!has_children(task))
		{
			ecs_assert(opack::is_a<opack::Action>(task), ECS_INVALID_PARAMETER, "Leaf task is not an action.");
			if (is_potential(task))
				*out++ = task;
		}
		else
		{
			ecs_assert(task.has<Constructor>(), ECS_INVALID_PARAMETER, "Task doesn't have a temporal constructor component.");

			// 1. Retrieve all children and sort them by their orders.
			std::map<size_t, opack::Entity> subtasks{};
			task.children
			(
				[&subtasks](opack::Entity e)
				{
					subtasks.emplace(order(e), e);
				}
			);

			// 2. Check if a task is in progress
			bool has_active_task{ false };
			for (auto [order, subtask] : subtasks)
			{
				has_active_task |= in_progress(subtask);
			}

			// 3. Determine, based on temporal constructor, which potential actions are added.
			bool first{ true };
			bool succeeded{ false };
			switch (task.get<Constructor>()->temporal)
			{
			case TemporalConstructor::PAR: // TODO
			case TemporalConstructor::IND: // Every potential actions are added, even if another isn't finished.
				for (auto [order, subtask] : subtasks)
				{
					succeeded |= potential_actions(subtask, out);
				}
				break;
			case TemporalConstructor::SEQ: // Every potential actions are added, if last one is finished
				for (auto [order, subtask] : subtasks)
				{
					if (!has_active_task && !is_satisfied(subtask))
					{
						succeeded |= potential_actions(subtask, out);
					}
				}
				break;
			case TemporalConstructor::SEQ_ORD: // First non satisfied task is next, if last one is finished.
				for (auto [order, subtask] : subtasks)
				{
					if (!has_active_task && first && (!is_satisfied(subtask) && !is_finished(subtask)))
					{
						succeeded |= potential_actions(subtask, out);
						first = false;
					}
				}
				break;
			case TemporalConstructor::ORD: // First non satisfied task is next, even if last one isn't finished.
				for (auto [order, subtask] : subtasks)
				{
					if (first && (!is_satisfied(subtask) && !has_started(subtask)))
					{
						succeeded |= potential_actions(subtask, out);
						first = false;
					}
				}
				break;
			}
			return succeeded;
		}
		return false;
	}

	/**
	 * Iterate an activity using dfs.
	 * Function @c func signature is @c void<opack::Entity>.
	 */
	template<typename Func>
	void traverse_dfs(opack::Entity task, Func&& func)
	{
		func(task);
		task.children([&func](opack::Entity e) {traverse_dfs(e, std::forward<Func>(func)); });
	}
}
