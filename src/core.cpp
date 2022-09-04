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
	_::create_module_entity<Behaviour>(world);
	_::create_module_entity<Message>(world);


	// -------------------------------------------------------
	// Prefabs 
	// -------------------------------------------a------------
	opack::prefab<Tangible>(world);
	opack::prefab<Agent>(world)
        .is_a<Tangible>()
		.override<DefaultBehaviour>()
    ;
    opack::prefab<Artefact>(world).is_a<Tangible>();
	opack::prefab<Action>(world)
		.add<Arity>();
	opack::prefab<Actuator>(world)
		;
	opack::prefab<Sense>(world)
		.add(flecs::Transitive)
        ;
	opack::prefab<Flow>(world)
        ;

	// -------------------------------------------------------
	// Operation
	// -------------------------------------------------------
	world.component<Behaviour>();
	world.component<HasBehaviour>();

	world.component<Arity>()
		.member<size_t>("min")
		.member<size_t>("max")
		;

	world.component<Doing>()
		.add(flecs::Exclusive)
		.add(flecs::OneOf, entity<Action::entities_folder_t>(world))
		;
	world.component<By>(); 
	world.component<On>();
	world.component<Delay>()
		.member<float, flecs::units::duration::Seconds>("value")
		;
	world.component<Duration>()
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
	world.component<Timer>()
		.member<float, flecs::units::duration::Seconds>("value");

	// Phases
	// --------
	world.entity<Perceive::PreUpdate>().add(flecs::Phase).depends_on(flecs::OnUpdate);
	world.entity<Perceive::Update>().add(flecs::Phase).depends_on<Perceive::PreUpdate>();
	world.entity<Perceive::PostUpdate>().add(flecs::Phase).depends_on<Perceive::Update>();
	world.entity<Reason::PreUpdate>().add(flecs::Phase).depends_on<Perceive::PostUpdate>();
	world.entity<Reason::Update>().add(flecs::Phase).depends_on<Reason::PreUpdate>();
	world.entity<Reason::PostUpdate>().add(flecs::Phase).depends_on<Reason::Update>();
	world.entity<Act::PreUpdate>().add(flecs::Phase).depends_on<Reason::PostUpdate>();
	world.entity<Act::Update>().add(flecs::Phase).depends_on<Act::PreUpdate>();
	world.entity<Act::PostUpdate>().add(flecs::Phase).depends_on<Act::Update>();

	world.component<EventCallable>();
	world.component<Begin>();
	world.component<Cancel>();
	world.component<End>();

	world.system("CleanCancelledActions")
		.kind<Act::PreUpdate>()
		.term<OnCancel>().optional()
        .term<DoNotClean>().optional()
		.term<Cancel>()
		.term<Begin, Timestamp>()
		.term(flecs::IsA).second<opack::Action>()
		.each([](flecs::iter& it, size_t index)
			{
				auto e = it.entity(index);
				if (it.is_set(1))
				{
					it.field<OnCancel>(1)->func(e);
				}
				if (!it.is_set(2))
		            e.destruct();
				else 
					e.set<End, Timestamp>({ it.world().time()});
			}
	).child_of<opack::world::dynamics>();

	world.system<Timer>("TimerFinishedActions")
		.kind<Act::PreUpdate>()
		.term<Begin, Timestamp>()
		.term(flecs::IsA).second<opack::Action>()
		.each([](flecs::entity e, Timer& timer)
			{
				if(timer.value == 0.0f)
		            e.remove<Timer>();
			}
	).child_of<opack::world::dynamics>();

	world.system("StartActions")
		.kind<Act::PreUpdate>()
		.term<OnBegin>().optional()
		.term(flecs::IsA).second<opack::Action>()
		.term<By>(flecs::Wildcard)
		.term<Begin, Timestamp>().not_()
        .term<Delay>().not_()
		.each([](flecs::iter& it, size_t index)
			{
				auto e = it.entity(index);
		        e.set<Begin, Timestamp>({e.world().time()});
		        e.add<Duration>();
				if (it.is_set(1))
					it.field<OnBegin>(1)->func(e);
			}
	).child_of<opack::world::dynamics>();

	world.system<OnUpdate>("UpdateAction")
		.kind<Act::Update>()
		.term(flecs::IsA).second<opack::Action>()
        .term<Delay>().not_()
		.term<Begin, Timestamp>()
        .term<End, Timestamp>().not_()
		.each([](flecs::iter& it, size_t index, OnUpdate& callable)
			{
		        callable.func(it.entity(index), it.delta_time());
			}
	).child_of<opack::world::dynamics>();

	world.system<OnEnd>("EndAction")
		.kind<Act::PostUpdate>()
		.term<DoNotClean>().optional()
		.term(flecs::IsA).second<opack::Action>()
		.term<Timer>().not_()
		.term<Delay>().not_()
		.term<Begin, Timestamp>()
        .term<End, Timestamp>().not_()
		.each([](flecs::iter& it, size_t index, EventCallable& callable)
			{
				auto entity = it.entity(index);
		        callable.func(entity);
				if(it.is_set(2))
				    entity.set<End, Timestamp>({ it.world().time()});
				else
				    entity.destruct();
			}
	).child_of<opack::world::dynamics>();

	world.system<Duration>("UpdateDuration")
		.each([](flecs::iter& iter, size_t, Duration& duration)
			{
					duration.value += iter.delta_system_time();
			}
	).child_of<opack::world::dynamics>();

	world.system<Delay>("UpdateDelay")
		.term<Delay>().write()
		.each([](flecs::iter& iter, size_t i, Delay& delay)
			{
					delay.value -= iter.delta_system_time();
					if (delay.value <= 0)
						iter.entity(i).remove<Delay>();
			}
	).child_of<opack::world::dynamics>();

	world.system<Timer>("UpdateTimer")
		.each([](flecs::iter& iter, size_t, Timer& timer)
			{
					timer.value -= iter.delta_system_time();
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
