#include <opack/module/adl.hpp>

#include "opack/core/world.hpp"

adl::adl(opack::World& world)
{
	world.entity<Activity::entities_folder_t>().add(flecs::Module);

	world.component<Order>()
		.member<std::size_t>("value");
	world.component<LogicalConstructor>()
		.constant("AND", static_cast<int32_t>(LogicalConstructor::AND))
		.constant("OR", static_cast<int32_t>(LogicalConstructor::OR))
		;
	world.component<TemporalConstructor>()
		.constant("IND", static_cast<int32_t>(TemporalConstructor::IND))
		.constant("SEQ_ORD", static_cast<int32_t>(TemporalConstructor::SEQ_ORD))
		.constant("ORD", static_cast<int32_t>(TemporalConstructor::ORD))
		.constant("SEQ", static_cast<int32_t>(TemporalConstructor::SEQ))
		.constant("PAR", static_cast<int32_t>(TemporalConstructor::PAR))
		;
	world.component<Constructor>()
		.member<LogicalConstructor>("logical")
		.member<TemporalConstructor>("temporal");

	world.component<Contextual>();
	world.component<Favorable>();
	world.component<Nomological>();
	world.component<Regulatory>();
	world.component<Satisfaction>();

	// So we do not have to specify the order when it's omitted (e.g. in plecs file)
	world.observer("Observer_IfMissing_AddOrder")
		.event(flecs::OnAdd)
		.term<Constructor>().parent()
		.term<Order>().not_()
		.each(
		[](opack::Entity e)
		{
			e.set<Order>({ children_count(e.parent()) - 1 });
		}
	).child_of<opack::world::dynamics>();

	// So we do not have to specify a satisfaction condition when it's omitted (e.g. in plecs file)
	world.observer("Observer_IfMissing_AddSatisfaction")
		.event(flecs::OnAdd)
		.term<Constructor>().parent()
		.term<Satisfaction>().not_()
		.each(
		[](opack::Entity task)
		{
			condition<Satisfaction>(task, is_finished);
		}
	).child_of<opack::world::dynamics>();

	// So we do not have to specify DoNotClean when it's omitted (e.g. in plecs file)
	world.observer("Observer_IfMissing_AddDoNotClean")
		.event(flecs::OnAdd)
		.term<Constructor>().parent()
		.term<opack::DoNotClean>().not_()
		.each(
		[](opack::Entity task)
		{
				task.add<opack::DoNotClean>();
		}
	).child_of<opack::world::dynamics>();

	auto task = opack::prefab<Task>(world);
	condition<Satisfaction>(task, is_finished);
	opack::prefab<Activity>(world).is_a<Task>();
	auto action = opack::init<Action>(world).add<opack::DoNotClean>();
	condition<Satisfaction>(action, is_finished);
}

opack::Entity adl::task(const char* name, opack::Entity parent, LogicalConstructor logical, TemporalConstructor temporal, std::size_t arity_max, std::size_t arity_min)
{
	auto entity = parent.world().prefab(name).is_a<Task>();
	entity.child_of(parent);
	entity.set<Order>({ children_count(parent) });
	entity.set<Constructor>({ logical, temporal });
	entity.set<opack::Arity>({ arity_min, arity_max });
	return entity;
}

bool adl::has_children(opack::Entity task)
{
	return children_count(task) > 0;
}

bool adl::is_finished(opack::Entity task)
{
	bool result{ true };
	if (has_children(task))
	{
		opack_assert(task.has<Constructor>(), "Task {} doesn't have a logical constructor component.", task.path().c_str());
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
	if (has_children(task))
	{
		task.children([&result](opack::Entity e) {result |= has_started(e); });
	}
	else
	{
		result = task.has<opack::Begin, opack::Timestamp>();
	}
	return result;
}

std::size_t adl::order(opack::Entity task)
{
	opack_assert(task.has<Order>(), "Somehow task {} does not have an order. It should never happen. File an issue.", task.path().c_str());
	return task.get<Order>()->value;
}

std::size_t adl::children_count(opack::Entity task)
{
	return opack::internal::children_count(task);
}

std::size_t adl::size(opack::Entity task)
{
	std::size_t count{ 0 };
	traverse_dfs(task, [&count](opack::Entity) {count++; });
	return count;
}

opack::Entity adl::parent_of(opack::Entity task)
{
	auto parent = task.parent();
	if (parent && opack::is_a<Task>(parent))
		return parent;
	return opack::Entity::null();
}

bool adl::is_root(opack::Entity task)
{
	return !parent_of(task);
}

opack::Entity get_root(opack::Entity task)
{
	auto root = task;
	while (!adl::is_root(task) || !root.is_valid())
	{
		root = adl::parent_of(task);
	}
	return root;
}

bool adl::is_satisfied(opack::Entity task)
{
	bool result{ true };
	if (has_children(task))
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

std::map<std::size_t, opack::Entity> adl::children(opack::Entity task)
{
	std::map<std::size_t, opack::Entity> subtasks{};
	task.children
	(
		[&subtasks](opack::Entity e)
		{
			subtasks.emplace(order(e), e);
		}
	);
	return subtasks;
}

bool adl::in_progress(opack::Entity task)
{
	if (has_children(task))
	{
		bool result{ false };
		task.children([&result](opack::Entity e) {result |= in_progress(e); }); // False if one child is not satisfied
		return result;
	}
	return has_started(task) && !is_finished(task);
}

opack::Entity adl::initiator(opack::Entity action, std::size_t n)
{
	return action.target<opack::By>(static_cast<int>(n));
}
