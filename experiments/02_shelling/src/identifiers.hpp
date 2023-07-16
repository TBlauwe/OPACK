#pragma once
#include <vector>
#include <opack/core.hpp>

OPACK_AGENT(Agent);

// ----- Tags
struct Happy {};

// ----- Components
struct GlobalStats
{
	float percent_similar{ 0.0f };
	float percent_unhappy{ 0.0f };
};

struct SimilarWanted 
{
	float value{ 0.0f };
};

struct LocalStats
{
	uint8_t similar_nearby{ 0 };
	uint8_t other_nearby{ 0 };
	uint8_t total_nearby{ 0 };
};

// ----- Relations
enum class Team
{
	Red,
	Blue
};

bool same_team(const flecs::entity a, const flecs::entity b)
{
	return a.get<Team>() == b.get<Team>();
}
