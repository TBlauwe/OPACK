#pragma once
#include <vector>
#include <opack/core.hpp>

struct Agent {};
struct Happy {};

struct GlobalStats
{
	float percent_similar{ 0.0f };
	float percent_unhappy{ 0.0f };
};

struct LocalStats
{
	int similar_nearby{ 0 };
	int other_nearby{ 0 };
	int total_nearby{ 0 };
};

enum class Team
{
	Red,
	Blue
};

bool same_team(const flecs::entity a, const flecs::entity b)
{
	return a.get<Team>() == b.get<Team>();
}
