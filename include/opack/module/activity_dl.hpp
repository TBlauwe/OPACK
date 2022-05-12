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

#include <opack/utils/flecs_helper.hpp>
#include <opack/utils/type_name.hpp>

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
	static flecs::entity activity(flecs::world& world);

	template<std::derived_from<Activity> T>
	static flecs::entity instantiate(flecs::world& world);

	template<std::derived_from<Activity> T>
	static flecs::entity compose(flecs::entity parent);

	static flecs::entity task(const char* name, flecs::entity parent);

	static size_t children_count(flecs::entity task);
	static size_t size(flecs::entity task);

	static flecs::entity parent_of(flecs::entity task);

	/**
	 * Iterate an activity using dfs.
	 * Function @c func signature is @c void<flecs::entity>.
	 */
	template<typename Func>
	static void traverse_dfs(flecs::entity task, Func&& func);
};

template<std::derived_from<adl::Activity> T>
flecs::entity adl::activity(flecs::world& world)
{
	 return world.prefab<T>();
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
	return instance;
}
