#include <doctest/doctest.h>
#include <opack/core.hpp>
#include <opack/algorithm/influence_graph.hpp>
#include <iostream>

TEST_SUITE_BEGIN("Algorithm : influence graph");

template<typename U, typename V>
void print(opack::IPGraph<U, V>& ig)
{
	fmt::print(" --------- IG graph ----------\n");
	for (auto& [u_idx, v_idx] : ig.positive_influences())
	{
		fmt::print("({}) --- + ---> ({})\n", ig.u_at(u_idx), ig.v_at(v_idx));
	}
	for (auto& [u_idx, v_idx] : ig.negative_influences())
	{
		fmt::print("({}) --- - ---> ({})\n", ig.u_at(u_idx), ig.v_at(v_idx));
	}
	for (auto& [v_idx, score] : ig.scores())
	{
		fmt::print("({}) has {}\n", ig.v_at(v_idx), score);
	}
}

TEST_CASE("With int")
{
	opack::IPGraph<int, int> ig{ };

	std::array U{ 0,1,2,3 };
	std::array V{ 0,1,2,3,4 };


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

TEST_CASE("With entity")
{
	auto world = opack::create_world();
	opack::IPGraph<flecs::entity, flecs::entity> ig{ };

	std::array U{ world.entity(), world.entity(), world.entity(), world.entity() };
	std::array V{ world.entity(), world.entity(), world.entity(), world.entity(), world.entity() };

	ig.scope(U[0]).positive_influence(V[0]);
	CHECK(ig.score(V[0]) == 1);
	CHECK(V[0] == ig.compute());

	ig.scope(U[0]).negative_influence(V[1]);
	CHECK(ig.score(V[0]) == 1);
	CHECK(ig.score(V[1]) == -1);
	CHECK(V[0] == ig.compute());

	ig.scope(U[1]).positive_influence(V[0]);
	ig.scope(U[1]).positive_influence(V[0]);
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
