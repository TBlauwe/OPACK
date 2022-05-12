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

struct adl
{
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
	auto relation = world.pair<T>(flecs::Wildcard);
	task.each(world.id<T>(), task, [&func](flecs::id id)
		{
			func(id.second());
		}
	);
}

template<std::derived_from<adl::Activity> T>
void adl::parent(flecs::entity task, flecs::entity parent)
{
	ecs_assert(adl::is_task_in<T>(parent), ECS_INVALID_PARAMETER,
		"Parent task is not in the same activity tree. Make sure that you create subtask only in the same activity tree of the parent task.");
	task.remove<T>();
	task.add<T>(parent);
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



