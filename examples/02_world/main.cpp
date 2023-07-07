#include <opack/core.hpp>	// Core header to use the library

// 5. An entity can be associated to a manual identifier
struct MyId {};

int main()
{
	// 1. Create an empty world.
	auto world = opack::create_world();

	// 2. Create an empty entity.
	{
		auto entity = world.entity();

		// 3. Each entity is associated to an unique identifier
		fmt::print("Entity ID : {}\n", entity.id());
		fmt::print("Entity path : {}\n", entity.path().c_str());
	}

	fmt::print("---\n");
	// 4. Create an empty named entity.
	{
		auto entity = world.entity("my_entity");
		fmt::print("Entity ID : {}\n", entity.id());
		fmt::print("Entity path : {}\n", entity.path().c_str());
	}

	fmt::print("---\n");
	// 5. Create an empty entity associated to a type
	{
		auto entity = opack::entity<MyId>(world);

		// 3. Each entity is associated to an unique identifier
		fmt::print("Entity ID : {}\n", entity.id());
		fmt::print("Entity path : {}\n", entity.path().c_str());
	}
}
