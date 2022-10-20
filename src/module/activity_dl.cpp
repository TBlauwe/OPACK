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
		.constant("XOR", static_cast<int32_t>(LogicalConstructor::XOR))
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
		.term<Constructor>().self().not_()
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

	opack::prefab<Task>(world);
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

bool adl::has_children(opack::EntityView task)
{
	return children_count(task) > 0;
}

bool adl::is_finished(opack::EntityView task)
{
	if (!is_leaf(task))
	{
		bool result {true} ; 
		opack_assert(task.has<Constructor>(), "Task {} doesn't have a logical constructor component.", task.path().c_str());
		task.children([&result](opack::Entity e) {result &= is_finished(e); });
		if(result)
			return result; //Early return if all children are finished.

		switch (task.get<Constructor>()->logical)
		{
		case LogicalConstructor::AND:
			// Still true if one is finished but not satisfied.
			task.children([&result](opack::Entity e) {result |= is_finished(e) && !is_satisfied(e); });
			break;
		case LogicalConstructor::XOR:
			// Still true if one child is finished and satisfied.
			task.children([&result](opack::Entity e) {result |= is_finished(e) && is_satisfied(e); });
			break;
		case LogicalConstructor::OR:
			break;
		}
		return result;
	}
	return opack::is_finished(task);
}

bool adl::has_started(opack::EntityView task)
{
	if (!is_leaf(task))
	{
		bool result {false} ; 
		task.children([&result](opack::Entity e) {result |= has_started(e); });
		return result;
	}
	return opack::has_started(task);
}

std::size_t adl::order(opack::EntityView task)
{
	opack_assert(task.has<Order>(), "Somehow task {} does not have an order. It should never happen. File an issue.", task.path().c_str());
	return task.get<Order>()->value;
}

std::size_t adl::children_count(opack::EntityView task)
{
	return opack::internal::children_count(task);
}

std::size_t adl::size(opack::EntityView task)
{
	std::size_t count{ 0 };
	traverse_dfs(task, [&count](opack::EntityView) {count++; });
	return count;
}

opack::Entity adl::parent_of(opack::EntityView task)
{
	auto parent = task.parent();
	if (parent && opack::is_a<Task>(parent))
		return parent;
	return opack::Entity::null();
}

bool adl::is_root(opack::EntityView task)
{
	opack_assert(task.is_valid(), "Task is invalid");
	return !parent_of(task);
}

bool adl::is_leaf(opack::EntityView entity)
{
	opack_assert(entity.is_valid(), "Entity is invalid");
	return !has_children(entity);
}

opack::Entity adl::get_root(opack::EntityView task)
{
	opack::Entity root = task.mut(task);
	while (!is_root(root) && root.is_valid())
	{
		root = parent_of(root);
	}
	return root;
}

bool adl::is_satisfied(opack::EntityView task)
{
	bool result {true};
	if (!is_leaf(task))
	{
		ecs_assert(task.has<Constructor>(), ECS_INVALID_PARAMETER, "Task doesn't have a logical constructor.");
		switch (task.get<Constructor>()->logical)
		{
		case LogicalConstructor::AND:
			// False if one child is not satisfied
			task.children([&result](opack::Entity e) {result &= is_satisfied(e); });
			break;
		case LogicalConstructor::XOR: 
			[[fallthrough]];
		case LogicalConstructor::OR:
			result = false;  // True if one child is satisfied
			task.children([&result](opack::Entity e) {result |= is_satisfied(e); });
			break;
		}
	}
	return result && check_condition<Satisfaction>(task);
}

std::map<std::size_t, opack::Entity> adl::children(opack::EntityView task)
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

bool adl::in_progress(opack::EntityView task)
{
	if (!is_leaf(task))
	{
		bool result{ false };
		task.children([&result](opack::Entity e) {result |= in_progress(e); }); // True if one child is in progress. 
		return result;
	}
	return opack::is_in_progress(task);
}

void adl::temporal_constructor(opack::Entity task, TemporalConstructor constructor)
{
	opack_assert(task.is_valid(), "Task is invalid");
	task.get_mut<Constructor>()->temporal = constructor;
}

void adl::logical_constructor(opack::Entity task, LogicalConstructor constructor)
{
	opack_assert(task.is_valid(), "Task is invalid");
	task.get_mut<Constructor>()->logical = constructor;
}

adl::LogicalConstructor adl::logical_constructor(opack::EntityView task)
{
	opack_assert(task.is_valid(), "Task is invalid");
	opack_assert(task.has<Constructor>(), "Task {} has no constructor", task.name().c_str());
	return task.get<Constructor>()->logical;
}

adl::TemporalConstructor adl::temporal_constructor(opack::EntityView task)
{
	opack_assert(task.is_valid(), "Task is invalid");
	opack_assert(task.has<Constructor>(), "Task {} has no constructor", task.name().c_str());
	return task.get<Constructor>()->temporal;
}

std::size_t adl::get_depth(opack::EntityView task)
{
	std::size_t depth {0};
	auto root = task;
	while (!is_root(root) && root.is_valid())
	{
		root = parent_of(root);
		depth++;
	}
	return depth;
}
