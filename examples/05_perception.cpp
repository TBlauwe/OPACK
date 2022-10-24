#include <opack/core.hpp> // Core header to use the library

// 0. Illustrative components that will be added to entities
struct Appearance
{
	int value {0};
};

struct Voice
{
	int value {0};
};

// 1. Identify sense with a type.
OPACK_SENSE(Vision);

int main()
{
	auto world = opack::create_world();

	// Not relevant here
	{
		// A component must be reflected to inspect it in inspector
		// and manipulate it through .flecs file.
		world.component<Voice>()
			.member<int>("value");

		world.component<Appearance>()
			.member<int>("value");
	}

	// 2. Initialize a concept.
	// ---- Analyse provided type to initialize it correctly.
	auto vision = opack::init<Vision>(world);

	// 3. Add sense to a type of agent
	opack::add_sense<Vision, opack::Agent>(world);

	// 4. Indicates capabilities of sense.
	opack::perceive<Vision, Appearance>(world);	// Appearance is perceivable
													// through Vision.

	opack::load(world, "plecs/perception.flecs"); // Load our descriptive file

	// 4. Retrieve agent by name
	auto alain = world.lookup("Alain");
	auto beatrice = world.lookup("Beatrice");

	// 5. Get perception API for our agent "A"
	auto p_a = opack::perception(alain);
	auto p_b = opack::perception(beatrice);

	// 6. True only if a --Vision--> b
	fmt::print("Does Alain perceive Beatrice ? {}\n", p_a.perceive<Vision>(beatrice));

	// 7. A value is obtainable only if it perceivable
	if(auto appearance = p_a.value<Vision, Appearance>(beatrice))
		fmt::print("Alain perceive Beatrice appearance with value {}.\n", appearance->value);
	else
		fmt::print("Alain does not perceive Beatrice appearance.\n");

	// 7. A value is obtainable only if it perceivable
	if(auto voice = p_a.value<Vision, Voice>(beatrice))
		fmt::print("Alain perceive Beatrice voice with value {}.\n", voice->value);
	else
		fmt::print("Alain does not perceive Beatrice voice.\n");

	// 8. We can also retrieved each perceivable value.
	p_a.each<Appearance>([](opack::Entity subject, const Appearance * appearance){
		fmt::print("Alain perceive {} with an appearance of value {}.\n", subject.path().c_str(), appearance->value);
	});
	
	// 9. or instance !
	p_a.each<opack::Agent>([](opack::Entity subject, const opack::Agent * appearance){
		fmt::print("Alain perceive agent {}\n", subject.path().c_str());
	});

	// As usual, let's run the world to inspect it here :
	// https://www.flecs.dev/explorer/?remote=true.
	opack::run_with_webapp(world);
}
