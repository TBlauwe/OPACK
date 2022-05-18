/*********************************************************************
 * \file   activity_dl.hpp
 * \brief  Module to manipulate and reason on a activity model using ACTIVITY-DL
 *
 * \author Tristan
 * \date   May 2022
 *********************************************************************/
#pragma once

#include <concepts>
#include <functional>
#include <map>

#include <flecs.h>

#include <opack/core/types.hpp>
#include <opack/core/action.hpp>
#include <opack/utils/flecs_helper.hpp>
#include <opack/utils/type_name.hpp>

struct adl
{
	struct Order
	{
		size_t value{ 0 };
	};

	enum class LogicalConstructor { AND, OR };
	enum class TemporalConstructor { IND, SEQ_ORD, ORD, SEQ, PAR };

	// TODO When enum relations are fixed, maybe use them instead of a specif components ? (Check perf)
	struct Constructor
	{
		LogicalConstructor logical{ LogicalConstructor::OR };
		TemporalConstructor temporal{ TemporalConstructor::IND };
	};

	struct ContextualCondition {};
	struct FavorableCondition {};
	struct NomologicalCondition {};
	struct RegulatoryCondition {};
	struct SatisfactionCondition {};

	struct Satisfied {};

	struct Activity {};

	adl(flecs::world& world);

	/**
	 *  Create an activity model refered as @c T.
	 */
	template<std::derived_from<Activity> T>
	static flecs::entity activity(
		flecs::world& world,
		LogicalConstructor logical = LogicalConstructor::AND,
		TemporalConstructor temporal = TemporalConstructor::SEQ_ORD,
		size_t arity_max = 1,
		size_t arity_min = 1
	);

	/**
	 * Instantiate a copy of the activity model @c T.
	 */
	template<std::derived_from<Activity> T>
	static flecs::entity instantiate(flecs::world& world);

	/**
	 * Add activity @c T as a children of @c parent.
	 */
	template<std::derived_from<Activity> T>
	static flecs::entity compose(flecs::entity parent);

	/**
	 * Shortant to create an action from prefab @c T and set it as a child of @c parent.
	 */
	template<std::derived_from<opack::Action> T>
	static flecs::entity action(flecs::entity parent);

	/**
	 * Create a task named @c name, with @c parent as parent.
	 */
	static flecs::entity task(const char* name,
		flecs::entity parent,
		LogicalConstructor logical = LogicalConstructor::AND,
		TemporalConstructor temporal = TemporalConstructor::SEQ_ORD,
		size_t arity_max = 1,
		size_t arity_min = 1
	);

	/**
	 * A task is considered to be a task only if it has children.
	 */
	static bool has_children(flecs::entity task);

	static bool is_finished(flecs::entity task);
	static bool has_started(flecs::entity task);
	static bool in_progress(flecs::entity task);

	static size_t order(flecs::entity task);

	static size_t children_count(flecs::entity task);
	static size_t size(flecs::entity task);

	static flecs::entity parent_of(flecs::entity task);

	static bool is_satisfied(flecs::entity task);
	static bool check_satisfaction(flecs::entity task);
	static bool is_potential(flecs::entity task);

	template<typename OutputIterator>
	static bool potential_actions(flecs::entity task, OutputIterator out);

	static bool has_task_in_progress(flecs::entity task);

	/**
	 * Iterate an activity using dfs.
	 * Function @c func signature is @c void<flecs::entity>.
	 */
	template<typename Func>
	static void traverse_dfs(flecs::entity task, Func&& func);
};

template<std::derived_from<adl::Activity> T>
flecs::entity adl::activity(flecs::world& world, LogicalConstructor logical, TemporalConstructor temporal, size_t arity_max, size_t arity_min)
{
	return world.prefab<T>()
		.template set<Constructor>({ logical, temporal })
		.template set<opack::Arity>({ arity_min, arity_max });
}

template<typename Func>
void adl::traverse_dfs(flecs::entity task, Func&& func)
{
	func(task);
	task.children([&func](flecs::entity e) {traverse_dfs(e, std::forward<Func>(func)); });
}

template<std::derived_from<adl::Activity> T>
flecs::entity adl::instantiate(flecs::world& world)
{
	return world.entity().is_a<T>().set_doc_name(type_name_cstr<T>());
}

template<std::derived_from<adl::Activity> T>
flecs::entity adl::compose(flecs::entity parent)
{
	auto world = parent.world();
	auto instance = adl::instantiate<T>(world);
	instance.child_of(parent);
	instance.template set<Order>({ adl::children_count(parent) });
	return instance;
}

template<std::derived_from<opack::Action> T>
flecs::entity adl::action(flecs::entity parent)
{
	auto world = parent.world();
	auto entity = opack::action<T>(world).child_of(parent);
	entity.template set<Order>({ adl::children_count(parent) });
	return entity;
}

template<typename OutputIterator>
bool adl::potential_actions(flecs::entity task, OutputIterator out)
{
	if (adl::is_finished(task))
		return adl::is_satisfied(task);

	if (!adl::has_children(task))
	{
		ecs_assert(task.has<opack::Action>(), ECS_INVALID_PARAMETER, "Leaf task is not an action.");
		if (!adl::has_started(task))
			*out++ = task;
	}
	else
	{
		ecs_assert(task.has<Constructor>(), ECS_INVALID_PARAMETER, "Task doesn't have a constructor component.");

		// 1. Retrieve all children and sort them by their orders.
		std::map<size_t, flecs::entity> subtasks{};
		task.children
		(
			[&subtasks](flecs::entity e)
			{
				subtasks.emplace(adl::order(e), e);
			}
		);

		// 2. Check if a task is in progress
		bool has_active_task{ false };
		for (auto [order, subtask] : subtasks)
		{
			has_active_task |= adl::has_started(subtask) && !adl::is_finished(subtask);
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
				if (!has_active_task && !adl::is_satisfied(subtask))
				{
					succeeded |= potential_actions(subtask, out);
				}
			}
			break;
		case TemporalConstructor::SEQ_ORD: // First non satisfied task is next, if last one is finished.
			for (auto [order, subtask] : subtasks)
			{
				if (!has_active_task && first &&  (!adl::is_satisfied(subtask) && !adl::is_finished(subtask)))
				{
					succeeded |= potential_actions(subtask, out);
					first = false;
				}
			}
			break;
		case TemporalConstructor::ORD: // First non satisfied task is next, even if last one isn't finished.
			for (auto [order, subtask] : subtasks)
			{
				if (first && (!adl::is_satisfied(subtask) && !adl::has_started(subtask)))
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
