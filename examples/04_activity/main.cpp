#include <iostream>

#include <opack/core.hpp>
#include <opack/module/simple_agent.hpp>
#include <opack/module/fipa_acl.hpp>
#include <opack/module/adl.hpp>
#include <opack/module/flows.hpp>

// 1. Create an identifier to refer to our flow.
OPACK_FLOW(MyFlow);

// 2. Create a second identifier to refer to our action.
OPACK_ACTION(MyAction);

int main()
{
	// 3. Create an empty world.
	auto world = opack::create_world();

	// 4. Loads our "simple" module for a simple agent.
	opack::import<simple>(world);

	// 4. Loads our "adl" module to add ACTIVITY-DL capabilities
	opack::import<adl>(world);

	// 5. [OPTIONAL] - Here we define a simple system to separate each cycle/tick by a dash line.
	world.system().iter([](flecs::iter& it){fmt::print("--------\n");});

	// 6.1 First, let's retrieve our "simple::Agent" prefab
	opack::prefab<simple::Agent>(world)
		// 6.2 Second, let's add our "MyFlow" identifier, to tell that it will use it.
		.add<MyFlow>();

	// 7.1 Let's define our action concept "MyAction"
	opack::init<MyAction>(world)
		// 7.2 Define which actuator is needed
		.require<simple::Actuator>()
		// 7.3 How long should it run
		.duration(0.0f)
		// 7.4 What happens when action is beginning
		.on_action_begin<MyAction>([](flecs::entity action) { fmt::print("{} is beginning with duration {}\n", action.path().c_str(), action.has<opack::Duration>() ? action.get<opack::Duration>()->value : 0);  })
		// 7.5 What happens when action is updating
		.on_action_update<MyAction>([](flecs::entity action, float dt) { fmt::print("{} is updating with a delta-time of {}\n", action.path().c_str(), dt);  })
		// 7.6 What happens when action is ending
		.on_action_end<MyAction>([](flecs::entity action) { fmt::print("{} is done\n", action.path().c_str()); });

	// 8. Create a flow used to reason over an activity tree.
	// "MyFlow" is our flow identifier.
	// "adl::Activity" is our relation used to retrieved activity that will be processed.
	ActivityFlowBuilder<MyFlow, adl::Activity>(world).build();

	// 9. Load activity from file.
	opack::load(world, "plecs/activity.flecs");

	// 10. As usual, let's run the world to inspect it here :
	// https://www.flecs.dev/explorer/?remote=true.
	opack::run_with_webapp(world);
}
