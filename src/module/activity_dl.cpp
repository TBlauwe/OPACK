#include <opack/module/adl.hpp>

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
			opack::prefab<Task>(world)
				.add<LogicalConstructor>()
				.add<TemporalConstructor>();
            opack::prefab<Activity>(world).is_a<Task>();

            world.component<Order>()
                .member<size_t>("value");
			world.component<LogicalConstructor>()
				.constant("AND", static_cast<int>(LogicalConstructor::AND))
				.constant("OR", static_cast<int>(LogicalConstructor::OR))
                .add(flecs::Union);
			world.component<TemporalConstructor>()
				.constant("IND", static_cast<int>(TemporalConstructor::IND))
				.constant("SEQ_ORD", static_cast<int>(TemporalConstructor::SEQ_ORD))
				.constant("ORD", static_cast<int>(TemporalConstructor::ORD))
				.constant("SEQ", static_cast<int>(TemporalConstructor::SEQ))
				.constant("PAR", static_cast<int>(TemporalConstructor::PAR))
                .add(flecs::Union);
			world.component<Constructor>()
				.member<LogicalConstructor>("logical")
				.member<TemporalConstructor>("temporal");

            world.component<Contextual>();
            world.component<Favorable>();
            world.component<Nomological>();
            world.component<Regulatory>();
            world.component<Satisfaction>();
        }
	}
	world.set_scope(prev);
}

opack::Entity adl::task(const char* name, opack::Entity parent, LogicalConstructor logical, TemporalConstructor temporal, size_t arity_max, size_t arity_min)
{
	auto entity = parent.world().prefab(name).is_a<Task>();
	entity.child_of(parent);
	entity.set<Order>({children_count(parent)});
	entity.set<Constructor>({ logical, temporal });
	entity.set<opack::Arity>({arity_min, arity_max});
	return entity;
}

bool adl::has_children(opack::Entity task)
{
	return children_count(task) > 0;
}

bool adl::is_finished(opack::Entity task)
{
	bool result{ true };
	if(adl::has_children(task))
	{
        ecs_assert(task.has<Constructor>(), ECS_INVALID_PARAMETER, "Task doesn't have a logical constructor component.");
        // False if children are not finished 
		task.children([&result](opack::Entity e) {result &= is_finished(e); }); 
	    switch (task.get<Constructor>()->logical)
		{
			case LogicalConstructor::AND:
                // But true if one is and is not satisfied
				task.children([&result](opack::Entity e) {result |= is_finished(e) && !is_satisfied(e); });
				break;
			case LogicalConstructor::OR:
                // True if one child is finished
				task.children([&result](opack::Entity e) {result |= is_finished(e) && is_satisfied(e); });
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
		task.children([&result](opack::Entity e) {result |= has_started(e); });
	}
	else
	{
		result = task.has<opack::Begin, opack::Timestamp>();
	}
	return result;
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
	traverse_dfs(task, [&count](opack::Entity) {count++; });
	return count;
}

opack::Entity adl::parent_of(opack::Entity task)
{
	auto parent = task.parent();
	if (opack::is_a<Task>(parent))
		return parent;
	return opack::Entity::null();
}

bool adl::is_root(opack::Entity task)
{
	return !parent_of(task);
}

bool adl::is_satisfied(opack::Entity task)
{
	bool result{ true };
	if(has_children(task))
	{
		ecs_assert(task.has<Constructor>(), ECS_INVALID_PARAMETER, "Task doesn't have a logical constructor.");
		switch (task.get<Constructor>()->logical)
		{
			case LogicalConstructor::AND:
                // False if one child is not satisfied
				task.children([&result](opack::Entity e) {result &= is_satisfied(e); });
				break;
			case LogicalConstructor::OR:
				result = false;  // True if one child is satisfied
				task.children([&result](opack::Entity e) {result |= is_satisfied(e); });
				break;
		}
	}
	else
	{
		result = check_condition<Satisfaction>(task);
	}
	return result;
}

bool adl::is_potential(opack::Entity task)
{
	if (is_satisfied(task) || in_progress(task))
		return false;
    return check_condition<Contextual>(task);
}

bool adl::in_progress(opack::Entity task)
{
	if(has_children(task))
	{
		bool result{ false };
		task.children([&result](opack::Entity e) {result |= in_progress(e); }); // False if one child is not satisfied
		return result;
	}
	return has_started(task) && !is_finished(task);
}

opack::Entity adl::initiator(opack::Entity action, size_t n)
{
	return action.target<opack::By>(static_cast<int>(n));
}
