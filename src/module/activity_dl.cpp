#include <opack/module/activity_dl.hpp>

#include "opack/core/world.hpp"

void adl::import(opack::World& world)
{
	auto scope = world.entity("::opack::modules");
	auto prev = world.set_scope(scope);
    world.entity<Activity::prefabs_folder_t>().add(flecs::Module);
    world.entity<Activity::entities_folder_t>().add(flecs::Module);
	{
        scope = world.entity("::opack::modules::adl").add(flecs::Module);
        world.set_scope(scope);
        {
            opack::prefab<Activity>(world);

            world.component<Satisfied>();
            world.component<Order>()
                .member<size_t>("value");
			world.component<LogicalConstructor>()
				.constant("AND", static_cast<int>(LogicalConstructor::AND))
				.constant("OR", static_cast<int>(LogicalConstructor::OR));
			world.component<TemporalConstructor>()
				.constant("IND", static_cast<int>(TemporalConstructor::IND))
				.constant("SEQ_ORD", static_cast<int>(TemporalConstructor::SEQ_ORD))
				.constant("ORD", static_cast<int>(TemporalConstructor::ORD))
				.constant("SEQ", static_cast<int>(TemporalConstructor::SEQ))
				.constant("PAR", static_cast<int>(TemporalConstructor::PAR));
			world.component<Constructor>()
                .member<LogicalConstructor>("logical_constructor")
                .member<TemporalConstructor>("temporal_constructor");

            world.component<ContextualCondition>();
            world.component<FavorableCondition>();
            world.component<NomologicalCondition>();
            world.component<RegulatoryCondition>();
            world.component<SatisfactionCondition>();
        }
	}
	world.set_scope(prev);
}

opack::Entity adl::task(const char* name, opack::Entity parent, LogicalConstructor logical, TemporalConstructor temporal, size_t arity_max, size_t arity_min)
{
	auto entity = parent.world().prefab(name);
	entity.child_of(parent);
	entity.set<Order>({adl::children_count(parent)});
	entity.add(logical);
	entity.add(temporal);
	entity.set<opack::Arity>({arity_min, arity_max});
	return entity;
}

bool adl::has_children(opack::Entity task)
{
	return adl::children_count(task) > 0;
}

bool adl::is_finished(opack::Entity task, opack::Entity agent)
{
	bool result{ true };
	if(adl::has_children(task))
	{
		ecs_assert(task.has<Constructor>(), ECS_INVALID_PARAMETER, "Task doesn't have a temporal and logical constructor.");
        // False if children are not finished 
		task.children([&agent, &result](opack::Entity e) {result &= is_finished(e, agent); }); 
		switch (task.get<Constructor>()->logical_constructor)
		{
			case LogicalConstructor::AND:
                // But true if one is and is not satisfied
				task.children([&agent, &result](opack::Entity e) {result |= is_finished(e, agent) && !is_satisfied(e, agent); });
				break;
			case LogicalConstructor::OR:
                // True if one child is finished
				task.children([&agent, &result](opack::Entity e) {result |= is_finished(e, agent) && is_satisfied(e, agent); });
				break;
		}
	}
	else
	{
		result = task.has<opack::End, opack::Timestamp>();
	}
	return result;
}

bool adl::has_started(opack::Entity task)
{
	bool result{ false };
	if(has_children(task))
	{
		ecs_assert(task.has<Constructor>(), ECS_INVALID_PARAMETER, "Task doesn't have a logical constructor.");
		task.children([&result](opack::Entity e) {result |= has_started(e); });
	}
	else
	{
		result = task.has<opack::Begin, opack::Timestamp>();
	}
	return result;
}

bool adl::in_progress(opack::Entity task, opack::Entity agent)
{
	return has_started(task) && !is_finished(task, agent);
}

size_t adl::order(opack::Entity task)
{
	return task.get<Order>()->value;
}

size_t adl::children_count(opack::Entity task)
{
	return opack::internal::children_count(task);
}

size_t adl::size(opack::Entity task)
{
	size_t count{ 0 };
	adl::traverse_dfs(task, [&count](opack::Entity) {count++; });
	return count;
}

opack::Entity adl::parent_of(opack::Entity task)
{
	return task.target(flecs::ChildOf);
}

bool adl::check_satisfaction(opack::Entity task, opack::Entity agent)
{
	return (task.has<Satisfied>() && !task.has<SatisfactionCondition>()) || (task.has<SatisfactionCondition>() && task.get<SatisfactionCondition>()->func(task, agent));
}

bool adl::is_satisfied(opack::Entity task, opack::Entity agent)
{
	bool result{ true };
	if(has_children(task))
	{
		ecs_assert(task.has<Constructor>(), ECS_INVALID_PARAMETER, "Task doesn't have a temporal and logical constructor.");
		switch (task.get<Constructor>()->logical_constructor)
		{
			case LogicalConstructor::AND:
                // False if one child is not satisfied
				task.children([&agent, &result](opack::Entity e) {result &= is_satisfied(e, agent); });
				break;
			case LogicalConstructor::OR:
				result = false;  // True if one child is satisfied
				task.children([&agent, &result](opack::Entity e) {result |= is_satisfied(e, agent); });
				break;
		}
	}
	else
	{
		result = check_satisfaction(task, agent);
	}
	return result;
}

bool adl::is_potential(opack::Entity task, opack::Entity agent)
{
	if (is_satisfied(task, agent) || in_progress(task, agent))
		return false;
    if (task.has<ContextualCondition>())
        return task.get<ContextualCondition>()->func(task, agent);
    return true;
}

bool adl::has_task_in_progress(opack::Entity task, opack::Entity agent)
{
	if(has_children(task))
	{
		bool result{ false };
		task.children([&agent, &result](opack::Entity e) {result |= has_task_in_progress(e, agent); }); // False if one child is not satisfied
		return result;
	}
    return in_progress(task, agent);
}
