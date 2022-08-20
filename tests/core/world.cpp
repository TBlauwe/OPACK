#include <doctest/doctest.h>
#include <opack/core.hpp>

TEST_CASE("Environment API")
{
	opack::World world;

	struct V {};            // A tag
	struct A {};            // An identifier
    OPACK_SUB_PREFAB(B, A); // A subprefab of A
    OPACK_SUB_PREFAB(C, B); // A subprefab of B

    auto a = opack::entity<A>(world);     // Create an entity associated to type A.
    auto b = opack::entity<B>(world);     // Create an entity associated to type A.
    auto c = opack::entity<C>(world);     // Create an entity associated to type A.

    SUBCASE("Prefab")
    {
        // Not yet a prefab.
        auto prefab = opack::entity<A>(world);     // Create an entity associated to type A.
        CHECK(opack::entity<A>(world) == prefab);	
        CHECK(!prefab.has(flecs::Prefab));	

        // Now it is a prefab.
        CHECK(opack::prefab<A>(world) == prefab);	// Should return same entity ...
        CHECK(prefab.has(flecs::Prefab));		    // ... should be now a prefab ...
        CHECK(prefab.has<flecs::Component>());	    // ... with "Component" component.

        // Create an instance
        auto inst = opack::spawn<A>(world);
        CHECK(inst != prefab);                      // Should be different entities.
        CHECK(opack::is_a<A>(inst);                 // Should be different entities.
        inst
    }

    world.app().enable_rest().run();
}

