#include <opack/module/core.hpp>
#include <opack/core/perception.hpp>

opack::concepts::concepts(flecs::world& world)
{
	world.module<concepts>();
	world.import<flecs::units>();

	// Organisation
	// ------------
	world.entity("opack::world").add(flecs::Module);
	world.entity("::world::prefab").add(flecs::Module);
	_::create_module_entity<Agent>(world);
	_::create_module_entity<Artefact>(world);
	_::create_module_entity<Action>(world);
	world.entity<world::Artefacts>("::world::Artefacts").add(flecs::Module);
	world.entity<world::Messages>("::world::Messages").add(flecs::Module);
	world.entity<world::Flows>("::world::Flows").add(flecs::Module);
	world.entity<world::Operations>("::world::Operations").add(flecs::Module);
	world.entity<world::Behaviours>("::world::Behaviours").add(flecs::Module);
	world.entity<world::Dynamics>("::world::Dynamics").add(flecs::Module);

	world.entity("::opack::queries").add(flecs::Module);
	world.entity("::modules").add(flecs::Module);


	// Operation
	// ---------
	world.component<Flow>();
	world.component<Operation>();
	world.component<Behaviour>();
	world.component<Active>();

	// MAS
	// ---
	opack::prefab<Agent>(world)
		.child_of<Agent::prefabs_folder_t>();
	opack::prefab<Artefact>(world)
		.child_of<Artefact::prefabs_folder_t>();
	world.component<Actuator>();
	world.component<Message>();
	world.component<Sense>();

	// Action
	// ------
	auto action = opack::prefab<Action>(world)
		.add<Arity>()
		.child_of<Action::prefabs_folder_t>();

	world.component<Arity>()
		.member<size_t>("min")
		.member<size_t>("max")
		;
	world.component<By>();
	world.component<On>();
	world.component<Delay>()
		.member<float, flecs::units::duration::Seconds>("value")
		;

	opack::prefab<Actuator>(world)
		.add(flecs::Exclusive)
		.add(flecs::OneOf, action)
		;

	// Perception
	// ----------
	opack::prefab<Sense>(world);

	world.entity("::opack::queries::perception").add(flecs::Module);
	world.emplace<queries::perception::Component>(world);
	world.emplace<queries::perception::Relation>(world);

	// Knowledge
	// ----------
	world.component<Knowledge>();

	// Misc
	// ----
	world.component<Timestamp>()
		.member<float>("value");
	world.component<Begin>();
	world.component<End>();
}

opack::dynamics::dynamics(flecs::world& world)
{
	world.module<dynamics>();
	world.import<concepts>();

	world.system<Action>("CleanActions")
		.term<By>().second(flecs::Wildcard).oper(flecs::Not)
		.term<Knowledge>().oper(flecs::Not)
		.iter([](flecs::iter& iter)
			{
				for (auto i : iter)
				{
					iter.entity(i).destruct();
				}
			}
	);

	world.system<Delay>("UpdateDelay")
		.iter([](flecs::iter& iter, Delay* delays)
			{
				for (auto i : iter)
				{
					delays[i].value -= iter.delta_system_time();
					if (delays[i].value <= 0)
						iter.entity(i).remove<Delay>();
				}
			}
	);
}
