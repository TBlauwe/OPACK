#include <opack/module/activity_dl.hpp>

adl::adl(flecs::world& world)
{
	world.module<adl>();

	world.component<adl::Activity>();

	world.component<adl::ContextualCondition>();
	world.component<adl::FavorableCondition>();
	world.component<adl::NomologicalCondition>();
	world.component<adl::RegulatoryCondition>();
	world.component<adl::SatisfactionCondition>();
}

flecs::entity adl::task(const char* name, flecs::entity parent)
{
	auto entity = parent.world().entity(name);
	entity.child_of(parent);
	return entity;
}

size_t adl::children_count(flecs::entity task)
{
	return opack::internal::children_count(task);
}

size_t adl::size(flecs::entity task)
{
	size_t count{ 0 };
	adl::traverse_dfs(task, [&count](flecs::entity) {count++; });
	return count;
}

flecs::entity adl::parent_of(flecs::entity task)
{
	return task.get_object(flecs::ChildOf);
}
