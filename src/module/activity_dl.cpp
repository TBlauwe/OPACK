#include <opack/module/activity_dl.hpp>

adl::adl(flecs::world& world)
{
	world.module<adl>("::modules::Activity-DL");

	world.entity<adl_::world::prefab::Activities>("::world::prefab::Activities").add(flecs::Module);
	world.entity<adl_::world::Activities>("::world::Activities").add(flecs::Module);

	world.component<adl::Activity>();
	world.component<adl::Satisfied>();
	world.component<adl::Order>();
	world.component<adl::Constructor>();

	world.component<adl::ContextualCondition>();
	world.component<adl::FavorableCondition>();
	world.component<adl::NomologicalCondition>();
	world.component<adl::RegulatoryCondition>();
	world.component<adl::SatisfactionCondition>();
}

flecs::entity adl::task(const char* name, flecs::entity parent, LogicalConstructor logical, TemporalConstructor temporal, size_t arity_max, size_t arity_min)
{
	auto entity = parent.world().prefab(name);
	entity.child_of(parent);
	entity.set<Order>({adl::children_count(parent)});
	entity.set<Constructor>({logical, temporal});
	entity.set<opack::Arity>({arity_min, arity_max});
	return entity;
}

bool adl::has_children(flecs::entity task)
{
	return adl::children_count(task) > 0;
}

bool adl::is_finished(flecs::entity task, flecs::entity agent)
{
	bool result{ true };
	if(adl::has_children(task))
	{
		ecs_assert(task.has<Constructor>(), ECS_INVALID_PARAMETER, "Task doesn't have a constructor component");
		task.children([&agent, &result](flecs::entity e) {result &= is_finished(e, agent); }); // False if children are not finished 
		switch (task.get<Constructor>()->logical)
		{
			case LogicalConstructor::AND:
				task.children([&agent, &result](flecs::entity e) {result |= is_finished(e, agent) && !is_satisfied(e, agent); }); // But true if one is and is not satisfied
				break;
			case LogicalConstructor::OR:
				task.children([&agent, &result](flecs::entity e) {result |= is_finished(e, agent) && is_satisfied(e, agent); }); // True if one child is finished
				break;
		}
	}
	else
	{
		result = task.has<opack::End, opack::Timestamp>();
	}
	return result;
}

bool adl::has_started(flecs::entity task)
{
	bool result{ false };
	if(adl::has_children(task))
	{
		ecs_assert(task.has<Constructor>(), ECS_INVALID_PARAMETER, "Task doesn't have a constructor component");
		task.children([&result](flecs::entity e) {result |= has_started(e); }); // False if children are not finished 
	}
	else
	{
		result = task.has<opack::Begin, opack::Timestamp>();
	}
	return result;
}

bool adl::in_progress(flecs::entity task, flecs::entity agent)
{
	return adl::has_started(task) && !adl::is_finished(task, agent);
}

size_t adl::order(flecs::entity task)
{
	return task.get<Order>()->value;
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

bool adl::check_satisfaction(flecs::entity task, flecs::entity agent)
{
	return (task.has<Satisfied>() && !task.has<SatisfactionCondition>()) || (task.has<SatisfactionCondition>() && task.get<SatisfactionCondition>()->func(task, agent));
}

bool adl::is_satisfied(flecs::entity task, flecs::entity agent)
{
	bool result{ true };
	if(adl::has_children(task))
	{
		ecs_assert(task.has<Constructor>(), ECS_INVALID_PARAMETER, "Task doesn't have a constructor component");
		switch (task.get<Constructor>()->logical)
		{
			case LogicalConstructor::AND:
				task.children([&agent, &result](flecs::entity e) {result &= is_satisfied(e, agent); }); // False if one child is not satisfied
				break;
			case LogicalConstructor::OR:
				result = false; 
				task.children([&agent, &result](flecs::entity e) {result |= is_satisfied(e, agent); }); // True if one child is satisfied
				break;
		}
	}
	else
	{
		result = adl::check_satisfaction(task, agent);
	}
	return result;
}

bool adl::is_potential(flecs::entity task, flecs::entity agent)
{
	if (adl::is_satisfied(task, agent) || adl::in_progress(task, agent))
		return false;
	else
		if (task.has<ContextualCondition>())
			return task.get<ContextualCondition>()->func(task, agent);
		else
			return true;
}

bool adl::has_task_in_progress(flecs::entity task, flecs::entity agent)
{
	if(adl::has_children(task))
	{
		bool result{ false };
		task.children([&agent, &result](flecs::entity e) {result |= has_task_in_progress(e, agent); }); // False if one child is not satisfied
		return result;
	}
	else
	{
		return adl::in_progress(task, agent);
	}
}
