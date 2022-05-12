/*****************************************************************//**
 * \file   activity_dl.hpp
 * \brief  Module to manipulate and reason on a activity model using ACTIVITY-DL
 *
 * \author Tristan
 * \date   May 2022
 *********************************************************************/
#pragma once

#include <concepts>
#include <functional>

#include <flecs.h>

// TODO Maybe action should also be associated with a type. 
// The upside is the ability to refer to them from anywhere (among other upside)
// The downside is we would never be able to generate action from a json, but haven't already made this choice
// e.g sense, actuator, etc.
// To ponder

struct adl
{
	struct Order 
	{
		size_t value {0};
	};

	enum class LogicalConstructor { AND, OR };
	enum class TemporalConstructor { IND, SEQ_ORD, ORD, SEQ };

	struct ContextualCondition {};
	struct FavorableCondition {};
	struct NomologicalCondition {};
	struct RegulatoryCondition {};
	struct SatisfactionCondition {};

	struct Activity {};

	adl(flecs::world& world);

	template<std::derived_from<Activity> T>
	static bool is_task_in(flecs::entity task);

	template<std::derived_from<Activity> T>
	static flecs::entity task(flecs::world& world, const char* name, flecs::entity parent = flecs::entity::null());

	template<std::derived_from<Activity> T>
	static void parent(flecs::entity task, flecs::entity parent);

	template<std::derived_from<Activity> T>
	static bool is_parent_of(flecs::entity parent, flecs::entity task);

	template<std::derived_from<Activity> T>
	static bool is_child_of(flecs::entity child, flecs::entity task);

	template<std::derived_from<Activity> T>
	static flecs::entity parent_of(flecs::entity task);

	template<std::derived_from<Activity> T>
	static void children_of(flecs::entity task, std::function<void(flecs::entity)>&& func);

	template<std::derived_from<Activity> T>
	static size_t children_count(flecs::entity task);

	template<std::derived_from<Activity> T>
	static void remove(flecs::entity task);

	template<std::derived_from<Activity> T>
	static flecs::entity instantiate(flecs::world& world);
};

template<std::derived_from<adl::Activity> T>
bool adl::is_task_in(flecs::entity task)
{
	return task.has<T>() || task.has<T>(flecs::Wildcard);
}

template<std::derived_from<adl::Activity> T>
flecs::entity adl::parent_of(flecs::entity task)
{
	return task.get_object<T>();
}

template<std::derived_from<adl::Activity> T>
void adl::children_of(flecs::entity task, std::function<void(flecs::entity)>&& func)
{
	auto world = task.world();
	auto filter = world.filter_builder().term<T>().obj(task).build();
	filter.each([&func](flecs::iter& it, size_t index) {func(it.entity(index)); });
}

template<std::derived_from<adl::Activity> T>
void adl::parent(flecs::entity task, flecs::entity parent)
{
	ecs_assert(adl::is_task_in<T>(parent), ECS_INVALID_PARAMETER,
		"Parent task is not in the same activity tree. Make sure that you create subtask only in the same activity tree of the parent task.");
	task.remove<T>();
	task.add<T>(parent);
	parent.add<Order>(task);
}

template<std::derived_from<adl::Activity> T>
bool adl::is_parent_of(flecs::entity parent, flecs::entity task)
{
	return task.has<T>(parent);
}

template<std::derived_from<adl::Activity> T>
bool adl::is_child_of(flecs::entity child, flecs::entity task)
{
	return child.has<T>(task);
}

template<std::derived_from<adl::Activity> T>
flecs::entity adl::task(flecs::world& world, const char* name, flecs::entity parent)
{
	auto task = world.entity(name);
	if (parent == flecs::entity::null())
		task.add<T>();
	else
	{
		adl::parent<T>(task, parent);
	}
	return task;
}

template<std::derived_from<adl::Activity> T>
size_t adl::children_count(flecs::entity task)
{
	size_t count {0};
	adl::children_of<T>(task, [&count](flecs::entity) {count++; });
	return count;
}

template<std::derived_from<adl::Activity> T>
void adl::remove(flecs::entity task)
{
	auto world = task.world();
	world.defer_begin();
	task.remove<T>();
	task.remove<T>(flecs::Wildcard);
	adl::children_of<T>(task, [](flecs::entity subtask) {adl::remove<T>(subtask); });
	world.defer_end();
}

template<std::derived_from<adl::Activity> T>
flecs::entity adl::instantiate(flecs::world& world)
{
	flecs::entity root;
	auto filter = world.filter_builder().term<T>().build();
	filter.each([&root](flecs::iter& it, size_t index) {root = it.entity(index); });
	if(root)
		return root.clone();
	return root;
}
