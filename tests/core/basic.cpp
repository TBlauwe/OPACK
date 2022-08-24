#include <doctest/doctest.h>
#include <opack/core.hpp>

OPACK_TAG(V);            
OPACK_PREFAB(A);        
OPACK_SUB_PREFAB(B, A); 
OPACK_SUB_PREFAB(C, B); 

TEST_CASE("World & Entity API")
{
	opack::World world = opack::create_world();

    auto a = opack::entity<A>(world);
    auto b = opack::entity<B>(world);
    auto c = opack::entity<C>(world);


    REQUIRE(a != b);
    REQUIRE(a != c);
    REQUIRE(b != c);

    opack::add<V>(a);

    REQUIRE(a.has<V>());
    REQUIRE(!b.has<V>());
    REQUIRE(!c.has<V>());

    
    CHECK(opack::prefab<A>(world) == a);	// Should return same entity ...
    CHECK(a.has(flecs::Prefab));		    // ... should be now a prefab ...
    CHECK(a.has<flecs::Component>());	    // ... with "Component" component ...
    CHECK(a.has<V>());		                // ... and "V".

    // Create an instance
    SUBCASE("Prefab")
    {
        auto inst = opack::spawn<A>(world);
        CHECK(inst != a);                       // Should be different entities.
        CHECK(opack::is_a<A>(inst));            // Should be recognized as an instance of "A" ...
        CHECK(inst.has<V>());                   // ..., so with "V".
    }

    SUBCASE("SubPrefab")
    {
        CHECK(opack::prefab<B>(world) == b);	
        CHECK(opack::prefab<C>(world) == c);	

        opack::init<B>(world);
        opack::init<C>(world);

        SUBCASE("Level 2")
        {
            auto inst = opack::spawn<B>(world);
            CHECK(opack::is_a<A>(inst));             
            CHECK(opack::is_a<B>(inst));          
            CHECK(!opack::is_a<C>(inst));           
            CHECK(inst.has<V>());                 
        }

        SUBCASE("Level 3")
        {
            auto inst = opack::spawn<C>(world);
            CHECK(opack::is_a<A>(inst));             
            CHECK(opack::is_a<B>(inst));            
            CHECK(opack::is_a<C>(inst));           
            CHECK(inst.has<V>());                   
        }
    }
}

