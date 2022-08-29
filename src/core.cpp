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
	opack::prefab<Tangible>(world);
	opack::prefab<Agent>(world).is_a<Tangible>();
    opack::prefab<Artefact>(world).is_a<Tangible>();
	opack::prefab<Action>(world)
		.add<Arity>();
	opack::prefab<Actuator>(world)
		;
	opack::prefab<Sense>(world)
		.add(flecs::Transitive)
        ;


	// -------------------------------------------------------
	// Operation
	// -------------------------------------------------------
	world.component<Behaviour>();
	world.component<Active>();

	world.component<Arity>()
		.member<size_t>("min")
		.member<size_t>("max")
		;

	world.component<Act>()
		.add(flecs::Exclusive)
		.add(flecs::OneOf, entity<Action::entities_folder_t>(world))
		;
	world.component<By>(); 
	world.component<On>();
	world.component<Delay>()
		.member<float, flecs::units::duration::Seconds>("value")
		;
	world.component<Delay>()
		.member<float, flecs::units::duration::Seconds>("value")
		;

	world.entity("::opack::queries::perception").add(flecs::Module);
	world.emplace<queries::perception::Entity>(world);
	world.emplace<queries::perception::Component>(world);
	world.emplace<queries::perception::Relation>(world);

	// Knowledge
	// ----------
	world.component<Knowledge>();

	// Misc
	// ----
	world.component<Timestamp>()
		.member<float, flecs::units::duration::Seconds>("value");

	world.component<Begin>();
	world.component<End>();

	world.system("StartActions")
		.kind(flecs::PostUpdate)
		.term(flecs::IsA).second<opack::Action>()
        .term<Begin, Timestamp>().not_().out()
        .term<Delay>().not_()
        .term<Begin>().not_().out()
		.each([](flecs::entity e)
			{
		        e.set<Begin, Timestamp>({e.world().time()});
		        e.add<Duration>();
		        e.add<Begin>();
			}
	).child_of<opack::world::dynamics>();

	world.system<Begin>("RemoveBeginTag")
		.kind(flecs::PostFrame)
		.each([](flecs::entity e, Begin)
			{
		        e.remove<Begin>();
			}
	).child_of<opack::world::dynamics>();

	world.system("CleanActions")
		.kind(flecs::PostFrame)
		.term(flecs::IsA).second<opack::Action>()
		.term<DoNotClean>().not_()
		.term<By>().second(flecs::Wildcard).not_().or_()
        .term<Duration>().not_()
		.each([](flecs::entity e)
			{
		        e.destruct();
			}
	).child_of<opack::world::dynamics>();

	world.system<Duration>("UpdateDuration")
		.each([](flecs::iter& iter, size_t, Duration& duration)
			{
					duration.value += iter.delta_system_time();
			}
	).child_of<opack::world::dynamics>();

	world.system<Delay>("UpdateDelay")
		.each([](flecs::iter& iter, size_t i, Delay& delay)
			{
					delay.value -= iter.delta_system_time();
					if (delay.value <= 0)
						iter.entity(i).remove<Delay>();
			}
	).child_of<opack::world::dynamics>();

	world.system<TickTimeout>("UpdateTickTimeout")
		.each([](flecs::iter& iter, size_t i, TickTimeout& timeout)
			{
					timeout.value--;
					if (timeout.value <= 0)
						iter.entity(i).destruct();
			}
	).child_of<opack::world::dynamics>();

	world.system<TimeTimeout>("UpdateTimeTimeout")
		.each([](flecs::iter& iter, size_t i, TimeTimeout& timeout)
			{
					timeout.value -= iter.delta_system_time();
					if (timeout.value <= 0)
						iter.entity(i).destruct();
			}
	).child_of<opack::world::dynamics>();
}
