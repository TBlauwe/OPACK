#include <opack/core.hpp> // Core header to use the library

// 1. Identify an actuator with a type.
OPACK_ACTUATOR(Hand);
// ---- An actuator allows an agent to realize actions that needs
//		this actuator. Only one action can be done at the same time
//		by an actuator.

// 2. Identify an action with a type.
OPACK_ACTION(HandAction);				// An action that will be used as a prefab.
OPACK_SUB_ACTION(Launch, HandAction);	// A subprefab of HandAction,
OPACK_SUB_ACTION(Take, HandAction);		// and another subprefab of HandAction.
										// They will inherit configuration of HandAction

// A relation that will be used for our action.
struct On {};

int main()
{
	auto world = opack::create_world();

	// 3. Each "concept" as usual must be initialized.
	// 3.1 ---- Our actuator. First let's start with a blank actuator.
	opack::init<Hand>(world);

	// 3.2 ---- Our base action concept.
	opack::init<HandAction>(world)
		.require<Hand>()	// A required actuator must be specified.
		.on_action_begin<HandAction>([](opack::Entity action){ fmt::print("Action \"{}\" is beginning.\n", action.path().c_str()); })
		.on_action_update<HandAction>([](opack::Entity action, float dt){ fmt::print("Action \"{}\" is updating with dt of {}.\n", action.path().c_str(), dt); })
		.on_action_end<HandAction>([](opack::Entity action){ fmt::print("Action \"{}\" is ending.\n", action.path().c_str()); })
		//.on_action_cancel<HandAction>([](opack::Entity action){})
		//.on_action_paused<HandAction>([](opack::Entity action){})
		//.on_action_resume<HandAction>([](opack::Entity action){})
		;

	// 3.3 ---- Our launch action takes some time.
	opack::init<Launch>(world)
		.duration(20.0f)		// A default duration (in seconds) can be specified.
		;

	// 3.3 ---- Our Take action does not.
	opack::init<Take>(world);


	// 4. Add actuator to an agent type.
	opack::add_actuator<Hand, opack::Agent>(world);

	// 5. Let's spawn some entities :
	auto agent = opack::spawn<opack::Agent>(world);
	auto artefact = opack::spawn<opack::Agent>(world);

	// 6. Spawn an instance of action Take so we can modify its parameters.
	auto take_action = opack::spawn<Take>(world);
	take_action.add<On>(artefact);

	// 7.1 Agent will no do the action instance.
	opack::act(agent, take_action);
	// 7.2 Agent current action can be retrieved
	opack::current_action<Hand>(agent);
	opack::step(world);

	// 8. Shorthand to create an action and also parametrize it.
	opack::act<Launch>(agent).add<On>(artefact);

	// Run world with web app activated so we can inspect it here :
	// https://www.flecs.dev/explorer/?remote=true.
	opack::run_with_webapp(world);
}
