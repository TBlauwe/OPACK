#include <iostream>

#include <opack/core.hpp>
#include <opack/module/simple_agent.hpp>
#include <opack/module/fipa_acl.hpp>
#include <opack/module/adl.hpp>
#include <opack/module/flows.hpp>

OPACK_FLOW(MyFlow);
OPACK_ACTION(MyAction);

int main()
{
	auto world = opack::create_world();

	world.import<simple>();
	world.import<adl>();

	world.system().iter([](flecs::iter& it){fmt::print("--------\n");});
	world.entity<simple::Agent>().add<MyFlow>();
	opack::init<MyAction>(world)
		.require<simple::Actuator>()
		.duration(0.0f)
		.on_action_begin<MyAction>([](flecs::entity action) { fmt::print("{} is beginning with duration {}\n", action.path().c_str(), action.get<opack::Duration>()->value);  })
		.on_action_end<MyAction>([](flecs::entity action) { fmt::print("{} is done\n", action.path().c_str()); });

	ActivityFlowBuilder<MyFlow, adl::Activity>(world).build();

	world.plecs_from_file("plecs/activity.flecs");

	flecs::log::set_level(1);
	opack::step_n(world, 10);
	flecs::log::set_level(0);
	//opack::run_with_webapp(world);
}
