#include <opack/core.hpp>
#include <opack/core/world.hpp>
#include <opack/core/perception.hpp>

opack::World opack::create_world()
{
	opack::World world;
	opack::import_opack(world);
	return world;
}

void opack::import_opack(World& world)
{
	world.import<flecs::units>();

	// -------------------------------------------------------
	// Folders
	// -------------------------------------------------------
	world.entity("::opack").add(flecs::Module);
	world.entity("::opack::world").add(flecs::Module);
	world.entity<world::prefabs>().add(flecs::Module);
	world.entity<world::behaviours>().add(flecs::Module);
	world.entity<world::dynamics>().add(flecs::Module);
	world.entity<world::rules>().add(flecs::Module);
	world.entity<world::modules>().add(flecs::Module);


	// -------------------------------------------------------
	// API Types
	// -------------------------------------------------------
	_::create_module_entity<Agent>(world);
	_::create_module_entity<Artefact>(world);
	_::create_module_entity<Action>(world);
	_::create_module_entity<Message>(world);
	_::create_module_entity<Actuator>(world);
	_::create_module_entity<Sense>(world);
	_::create_module_entity<Flow>(world);
	_::create_module_entity<Operation>(world);
	_::create_module_entity<Message>(world);


	// -------------------------------------------------------
	// Prefabs 
	// -------------------------------------------------------
	opack::prefab<Agent>(world);
    opack::prefab<Artefact>(world);
	opack::prefab<Action>(world)
		.add<Arity>();
	opack::prefab<Actuator>(world)
		.add(flecs::Exclusive)
		.add(flecs::OneOf, entity<Action>(world))
		;
	opack::prefab<Sense>(world);


	// -------------------------------------------------------
	// Operation
	// -------------------------------------------------------
	world.component<Behaviour>();
	world.component<Active>();

	world.component<Arity>()
		.member<size_t>("min")
		.member<size_t>("max")
		;
	world.component<By>();
	world.component<On>();
	world.component<Delay>()
		.member<float, flecs::units::duration::Seconds>("value")
		;

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
	).child_of<opack::world::dynamics>();

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
	).child_of<opack::world::dynamics>();
}
