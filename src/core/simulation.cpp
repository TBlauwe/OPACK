#include <opack/core/simulation.hpp>
#include <opack/core/perception.hpp>
#include <iostream>

opack::Simulation::Simulation(int argc, char* argv[])
	: world{ argc, argv }
{
	world.entity("::opack").add(flecs::Module);

	auto action = world.prefab<Action>()
		.add<Action>()
		.add<Arity>()
		;
	
	world.component<By>();
	world.component<On>();

	world.prefab<Actuator>()
		.add<Actuator>()
		.add(flecs::Exclusive)
		.add(flecs::OneOf, action)
		;

	world.prefab<Agent>().add<Agent>();
	world.prefab<Artefact>().add<Artefact>();
	world.prefab<Sense>().add<Sense>();

	world.emplace<Query::Perception::Component>(world);
	world.emplace<Query::Perception::Relation>(world);

	// If no one is performing an action, delete it.
	world.system<Action>("System_CleanupActionWithoutInitiator")
		.term<By>().obj(flecs::Wildcard).oper(flecs::Not)
		.iter([](flecs::iter& iter)
			{
				for (auto i : iter)
				{
					iter.entity(i).destruct();
				}
			}
	);

	world.system<Delay>("System_DelayUpdate")
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

opack::Simulation::~Simulation()
{
	stop();
}

bool opack::Simulation::step(float elapsed_time) {
	bool should_continue = world.progress(elapsed_time);

	//executor.wait_for_all();
	//size_t size = commands_queue.size();    // Since size can be updated between for loop (async),
	//                                        // we must check only once, not at every loop !
	//for (int i = 0; i<size; i++)
	//{
	//    auto command = commands_queue.pop();
	//    if(command && command.value()) // BUG : Somehow some commands are empty
	//        command.value()(_world);
	//}

	return should_continue;
}

void opack::Simulation::step_n(size_t n, float elapsed_time) {
	bool should_continue = true;
	for (size_t i = 0; i < n && should_continue; i++) {
		should_continue = step(elapsed_time);
	}
}

void opack::Simulation::rest_app()
{
	std::cout << "See web explorer on : https://www.flecs.dev/explorer/?remote=true\n";

	world.set<flecs::rest::Rest>({});
	while (step());
	world.remove<flecs::rest::Rest>();
}

void opack::Simulation::stop()
{
	world.quit();
}

float opack::Simulation::target_fps() const
{
	return world.get_target_fps();
}

void opack::Simulation::target_fps(float value)
{
	world.set_target_fps(value);
}

float opack::Simulation::time_scale() const
{
	return world.get_time_scale();
}

void opack::Simulation::time_scale(float value)
{
	world.set_time_scale(value);
}

size_t opack::Simulation::tick() const
{
	return static_cast<size_t>(world.tick());
}

float opack::Simulation::delta_time() const
{
	return world.delta_time();
}

float opack::Simulation::time() const
{
	return world.time();
}
