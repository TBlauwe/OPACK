/*****************************************************************//**
 * \file   activity_dl.hpp
 * \brief  Module to manipulate and reason on a activity model using ACTIVITY-DL
 * 
 * \author Tristan
 * \date   August 2022
 *********************************************************************/
#pragma once

#include <functional>
#include <map>

#include <flecs.h>

#include <opack/core/api_types.hpp>
#include <opack/core/components.hpp>
#include <opack/core/action.hpp>
#include <opack/utils/flecs_helper.hpp>
#include <opack/utils/type_name.hpp>

/** Shorthand for creating a type.*/
#define ADL_ACTIVITY(name) OPACK_SUB_PREFAB(name, adl::Activity)
/** Shorthand for creating a static variable containing the constructor.*/
#define ADL_LOGICAL_CONSTRUCTOR(value) static constexpr adl::LogicalConstructor logical_constructor {value}
/** Shorthand for creating a static variable containing the constructor.*/
#define ADL_TEMPORAL_CONSTRUCTOR(value) static constexpr adl::TemporalConstructor temporal_constructor {value};

/** Namespace for API related to Activity-DL. */
namespace adl
{

    struct activities { struct prefabs {}; };
	struct Activity : public opack::_::root<Activity>
	{
        using entities_folder_t = activities;
	    using prefabs_folder_t = activities::prefabs;
	};
    struct Context {};
	struct Task {};
	template<typename T>
	concept ActivityPrefab = opack::SubPrefab<T> && std::derived_from<T, Activity>;

	/** Component to hold order of a task in regards to its parent. */
	struct Order
	{
		size_t value{ 0 };
	};

	enum class LogicalConstructor { AND, OR };
	enum class TemporalConstructor { IND, SEQ_ORD, ORD, SEQ, PAR };

	struct Constructor
	{
		LogicalConstructor logical_constructor {LogicalConstructor::AND };
		TemporalConstructor temporal_constructor {TemporalConstructor::SEQ_ORD };
	};

	struct Condition
	{
		Condition() = default;
        Condition(std::function<bool(opack::Entity task, opack::Entity entity)>&& f) : func(std::move(f)) { }
		std::function<bool(opack::Entity task, opack::Entity entity)> func;
	};

	struct ContextualCondition : Condition { using Condition::Condition; };
	struct FavorableCondition : Condition { using Condition::Condition; };
	struct NomologicalCondition : Condition { using Condition::Condition; };
	struct RegulatoryCondition : Condition { using Condition::Condition; };
	struct SatisfactionCondition : Condition { using Condition::Condition; };

	struct Satisfied {};

	/** Import Activity-DL in your world. */
	void import(opack::World& world);

	/** Create a task named @c name, with @c parent as parent. */
	opack::Entity task(const char* name,
		opack::Entity parent,
		LogicalConstructor logical = LogicalConstructor::AND,
		TemporalConstructor temporal = TemporalConstructor::SEQ_ORD,
		size_t arity_max = 1,
		size_t arity_min = 1
	);

	/** A task is considered to be a task only if it has children. */
	bool has_children(opack::Entity task);

	bool is_finished(opack::Entity task, opack::Entity agent = opack::Entity::null());
	bool has_started(opack::Entity task);
	bool in_progress(opack::Entity task, opack::Entity agent = opack::Entity::null());

	size_t order(opack::Entity task);

	size_t children_count(opack::Entity task);
	size_t size(opack::Entity task);

	opack::Entity parent_of(opack::Entity task);

	bool is_satisfied(opack::Entity task, opack::Entity agent = opack::Entity::null());
	bool check_satisfaction(opack::Entity task, opack::Entity agent = opack::Entity::null());
	bool is_potential(opack::Entity task, opack::Entity agent = opack::Entity::null());
	bool has_task_in_progress(opack::Entity task, opack::Entity agent = opack::Entity::null());

	/**
	 *@brief Retrieve pointer to context value @c T, from current task or parent task.
	 *
	 */
	template<typename T>
	const T* context_value(opack::Entity task)
	{
		const T* result = task.get<T>();
		if(!result)
		{
			auto parent = task.parent();
			if (opack::is_a<Task>(parent))
				result = context_value<T>(parent);

		}
		return result;
	}

    /** Set context value @c T, into parent task or itself if no parent. */
	template<typename T>
	void context_value(opack::Entity task, T&& value)
	{
        auto parent = task.parent();
		if (opack::is_a<Task>(parent))
			context_value<T>(parent, std::forward<T>(value));
		else
			task.set<T>({ value });
	}

	/**
	 *@brief Retrieve entity from context target @c (R, target), from current task or parent task.
	 *
	 */
	template<typename R>
	opack::Entity context_target(opack::Entity task)
	{
		auto result = task.target<R>();
		if(!result)
		{
			auto parent = task.parent();
			if (opack::is_a<Task>(parent))
				result = context_target<R>(parent);

		}
		return result;
	}

    /** Set context target @c (R,target), into parent task or itself if no parent. */
	template<typename R>
	void context_target(opack::Entity task, opack::Entity target)
	{
        auto parent = task.parent();
		if (opack::is_a<Task>(parent))
			context_target<R>(parent, target);
		else
			task.add<R>(target);
	}


	/**
	 *  Create an activity model refered as @c T.
	 */
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

	/**
	 * Instantiate a copy of the activity model @c T.
	 */
	template<std::derived_from<Activity> T>
	opack::Entity instantiate(opack::World& world)
	{
		return opack::spawn<T>(world);
	}

	/**
	 * Add activity @c T as a children of @c parent.
	 */
	template<std::derived_from<Activity> T>
	opack::Entity compose(opack::Entity parent)
	{
		auto world = parent.world();
		auto instance = adl::instantiate<T>(world);
		instance.child_of(parent);
		instance.template set<Order>({ adl::children_count(parent) });
		return instance;
	}


	/**
	 * Shorthand to create an action from prefab @c T and set it as a child of @c parent.
	 */
	template<std::derived_from<opack::Action> T>
	opack::Entity action(opack::Entity parent)
	{
		auto world = parent.world();
		return opack::spawn<T>(world)
	        .child_of(parent)
	        .template add<opack::DoNotClean>()
		    .template set<Order>({ children_count(parent) });
	}


	template<typename OutputIterator>
	bool potential_actions(opack::Entity task, OutputIterator out, opack::Entity agent = opack::Entity::null())
	{
		if (adl::is_finished(task))
			return adl::is_satisfied(task, agent);

		if (!adl::has_children(task))
		{
			ecs_assert(opack::is_a<opack::Action>(task), ECS_INVALID_PARAMETER, "Leaf task is not an action.");
			if (adl::is_potential(task, agent))
				*out++ = task;
		}
		else
		{
			ecs_assert(task.has<Constructor>(), ECS_INVALID_PARAMETER, "Task doesn't have a logical and temporal constructor component.");

			// 1. Retrieve all children and sort them by their orders.
			std::map<size_t, opack::Entity> subtasks{};
			task.children
			(
				[&subtasks](opack::Entity e)
				{
					subtasks.emplace(adl::order(e), e);
				}
			);

			// 2. Check if a task is in progress
			bool has_active_task{ false };
			for (auto [order, subtask] : subtasks)
			{
				has_active_task |= adl::has_started(subtask) && !adl::is_finished(subtask, agent);
			}

			// 3. Determine, based on temporal constructor, which potential actions are added.
			bool first{ true };
			bool succeeded{ false };
			switch (task.get<Constructor>()->temporal_constructor)
			{
			case TemporalConstructor::PAR: // TODO
			case TemporalConstructor::IND: // Every potential actions are added, even if another isn't finished.
				for (auto [order, subtask] : subtasks)
				{
					succeeded |= potential_actions(subtask, out, agent);
				}
				break;
			case TemporalConstructor::SEQ: // Every potential actions are added, if last one is finished
				for (auto [order, subtask] : subtasks)
				{
					if (!has_active_task && !adl::is_satisfied(subtask, agent))
					{
						succeeded |= potential_actions(subtask, out, agent);
					}
				}
				break;
			case TemporalConstructor::SEQ_ORD: // First non satisfied task is next, if last one is finished.
				for (auto [order, subtask] : subtasks)
				{
					if (!has_active_task && first && (!adl::is_satisfied(subtask, agent) && !adl::is_finished(subtask, agent)))
					{
						succeeded |= potential_actions(subtask, out, agent);
						first = false;
					}
				}
				break;
			case TemporalConstructor::ORD: // First non satisfied task is next, even if last one isn't finished.
				for (auto [order, subtask] : subtasks)
				{
					if (first && (!adl::is_satisfied(subtask, agent) && !adl::has_started(subtask)))
					{
						succeeded |= potential_actions(subtask, out, agent);
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
};
