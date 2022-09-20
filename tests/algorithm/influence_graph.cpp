#include <doctest/doctest.h>
#include <opack/core.hpp>
#include <opack/algorithm/influence_graph.hpp>
#include <iostream>

TEST_SUITE_BEGIN("Algorithm : influence graph");

TEST_CASE("Initialization")
{
	opack::InfluenceGraph<int, int> ig { };

	std::array U{ 0,1,2,3 };
	std::array V{ 0,1,2,3,4};

	ig.positive_influence(U[0], V[0]);
	CHECK(ig.score(V[0]) == 1);
	CHECK(V[0] == ig.compute());

	ig.negative_influence(U[0], V[1]);
	CHECK(ig.score(V[0]) == 1);
	CHECK(ig.score(V[1]) == -1);
	CHECK(V[0] == ig.compute());

	ig.positive_influence(U[1], V[0]);
	ig.positive_influence(U[2], V[0]);
	CHECK(ig.score(V[0]) == 3);
	CHECK(ig.score(V[1]) == -1);
	CHECK(V[0] == ig.compute());

	ig.positive_influence(U[0], V[2]);
	ig.positive_influence(U[1], V[2]);
	ig.positive_influence(U[2], V[2]);
	ig.positive_influence(U[3], V[2]);
	CHECK(ig.score(V[0]) == 3);
	CHECK(ig.score(V[1]) == -1);
	CHECK(ig.score(V[2]) == 4);
	CHECK(V[2] == ig.compute());
}
