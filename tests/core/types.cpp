#include <doctest/doctest.h>
#include <opack/core.hpp>

TEST_CASE("Types API")
{
	OPACK_TAG(V);            
	OPACK_PREFAB(A);        
    OPACK_SUB_PREFAB(B, A); 
    OPACK_SUB_PREFAB(C, B); 

    CHECK(opack::NotSubPrefab<V>);
    CHECK(opack::NotSubPrefab<A>);
    CHECK(opack::SubPrefab<B>);
    CHECK(opack::SubPrefab<C>);
}
