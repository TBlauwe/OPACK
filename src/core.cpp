#include <opack/core.hpp>
#include <opack/core/world.hpp>
#include <opack/core/perception.hpp>

opack::World opack::create_world()
{
	opack::World world;
	opack::import_opack(world);
	return world;
}

namespace opack
{
	void define_action_systems(World& world);
}

void opack::import_opack(World& world)
{
	world.import<flecs::units>();

	world.component<DoNotClean>();
	world.component<Token>();

	// -------------------------------------------------------
	// Folders
	// -------------------------------------------------------
	world.entity("::opack").add(flecs::Module);
	world.entity("::opack::world").add(flecs::Module);
	world.entity<world::behaviours>().add(flecs::Module);
	world.entity<world::dynamics>().add(flecs::Module);
	world.entity<world::rules>().add(flecs::Module);


	// -------------------------------------------------------
	// API Types
	// -------------------------------------------------------
	internal::create_module_entity<Agent>(world);
	internal::create_module_entity<Artefact>(world);
	internal::create_module_entity<Action>(world);
	internal::create_module_entity<Message>(world);
	internal::create_module_entity<Actuator>(world);
	internal::create_module_entity<Sense>(world);
	internal::create_module_entity<Flow>(world);
	internal::create_module_entity<Operation>(world);
	internal::create_module_entity<Behaviour>(world);
	internal::create_module_entity<Message>(world);


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
		.override<Token>()
		.add<Arity>()
		.add(ActionStatus::waiting)
		;
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

	world.component<ActionStatus>()
		.constant("waiting", static_cast<int32_t>(ActionStatus::waiting))
		.constant("starting", static_cast<int32_t>(ActionStatus::starting))
		.constant("running", static_cast<int32_t>(ActionStatus::running))
		.constant("suspended", static_cast<int32_t>(ActionStatus::suspended))
		.constant("resumed", static_cast<int32_t>(ActionStatus::resumed))
		.constant("aborted", static_cast<int32_t>(ActionStatus::aborted))
		.constant("finished", static_cast<int32_t>(ActionStatus::finished))
		;

	world.component<Doing>()
		.add(flecs::Exclusive)
		;

	world.component<RequiredActuator>();

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
	world.entity<Cycle::Begin>().add(flecs::Phase).depends_on(flecs::OnUpdate);
	world.entity<Perceive::PreUpdate>().add(flecs::Phase).depends_on<Cycle::Begin>();
	world.entity<Perceive::Update>().add(flecs::Phase).depends_on<Perceive::PreUpdate>();
	world.entity<Perceive::PostUpdate>().add(flecs::Phase).depends_on<Perceive::Update>();
	world.entity<Reason::PreUpdate>().add(flecs::Phase).depends_on<Perceive::PostUpdate>();
	world.entity<Reason::Update>().add(flecs::Phase).depends_on<Reason::PreUpdate>();
	world.entity<Reason::PostUpdate>().add(flecs::Phase).depends_on<Reason::Update>();
	world.entity<Act::PreUpdate>().add(flecs::Phase).depends_on<Reason::PostUpdate>();
	world.entity<Act::Update>().add(flecs::Phase).depends_on<Act::PreUpdate>();
	world.entity<Act::PostUpdate>().add(flecs::Phase).depends_on<Act::Update>();
	world.entity<Cycle::End>().add(flecs::Phase).depends_on<Act::PostUpdate>();

	world.component<Begin>();
	world.component<End>();

	define_action_systems(world);

	world.system<LastActionPrefabs>("System_RememberLastActionsPrefabs")
		.kind<Act::Update>()
		.term<Doing>(flecs::Wildcard)
		.term(flecs::IsA).second<opack::Actuator>()
		.each([](flecs::entity actuator, LastActionPrefabs& last_actions)
			{
				if(auto prefab = actuator.target<Doing>().target(flecs::IsA) ; prefab != last_actions.peek())
					last_actions.previous_prefabs_done.push(prefab);
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

void opack::define_action_systems(opack::World& world)
{
	world.system("System_BeginAction")
		.kind<Act::Update>()
		.term(flecs::IsA).second<opack::Action>()
		.term<By>(flecs::Wildcard)
		.term(ActionStatus::starting)
		.term(ActionStatus::running).write()
		.term<Begin, Timestamp>().not_().write()
		.each([](flecs::entity action)
			{
		        action.set<Begin, Timestamp>({action.world().time()});
				action.add(ActionStatus::running);
			}
	).child_of<opack::world::dynamics>();

	world.system<Duration>("System_UpdateActionDuration")
		.kind<Act::Update>()
		.term_at(1).self()
		.term(flecs::IsA).second<opack::Action>()
		.term(ActionStatus::running)
		.each([](flecs::iter& it, size_t index, Duration& duration)
			{
				duration.value -= it.delta_system_time();
				if(duration.value <= 0.0f)
		            it.entity(index).remove<Duration>();
			}
	).child_of<opack::world::dynamics>();

	world.system("System_EndAction")
		.kind<Act::PostUpdate>()
		.term(flecs::IsA).second<opack::Action>()
		.term<By>(flecs::Wildcard)
		.term<Duration>().self().not_()
		.term(ActionStatus::running)
		.term(ActionStatus::finished).write()
		.term<End, Timestamp>().not_().write()
		.each([](flecs::entity action)
			{
		        action.set<End, Timestamp>({action.world().time()});
				action.add(ActionStatus::finished);
			}
	).child_of<opack::world::dynamics>();

	world.system("System_CleanAction")
		.kind<Cycle::End>()
		.term<DoNotClean>().optional()
		.term(flecs::IsA).second<opack::Action>()
		.term<By>(flecs::Wildcard)
		.term(ActionStatus::finished).or_()
		.term(ActionStatus::aborted).or_()
		.each([](flecs::iter& it, size_t index)
			{
				if(auto action = it.entity(index) ; it.is_set(1))
					action.remove<Token>();
				else
					action.destruct();
			}
	).child_of<opack::world::dynamics>();

}
