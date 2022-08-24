/*********************************************************************
 * \file  simulation.hpp
 * \brief API to control the simulation.
 *
 * \author Tristan
 * \date   April 2022
 *********************************************************************/
#pragma once

#include <opack/core/api_types.hpp>

namespace opack {

	/**
	@brief Count number of entities matching the pattern.
	@param world explicit.
	@return Numbers of entities having @c T.

    WARNING ! Do not works with prefabs, e.g. : @c count<Agent>(world).
	Use @ref count(World& world, Entity rel, Entity obj).
	*/
	template<typename T>
	size_t count(const World& world)
	{
		return static_cast<size_t>(world.count<T>());
	}

	/**
	@brief Count number of entities matching the pattern.
	@param world explicit.
	@param obj Match following pattern : entity--T-->obj.
	@return Numbers of matching pattern : entity--T-->obj.
	*/
	template<typename T>
	size_t count(const World& world, const Entity obj)
	{
		return static_cast<size_t>(world.count<T>(obj));
	}

	/**
	@brief Count number of entities matching the pattern.
	@param world explicit.
	@param rel Relation entity.
	@param obj Object entity.
	@return Numbers of matching pattern : entity--rel-->obj.
	*/
	size_t count(const World& world, const Entity rel, const Entity obj);

    /**
    @brief Return target fps.
    */
    float target_fps(const World& world);

    /**
    @brief Set target fps.
    */
    void target_fps(World& world, float value);

    /**
    @brief Return time scale.
    */
    float time_scale(World& world);

    /**
    @brief Set time scale.
    */
    void time_scale(World& world, float value);

    /** Return number of elapsed ticks. */
    int32_t tick(const World& world);

    /** Returns last reported delta time (elapsed time between two cycles). Affected by time scale. */
    float delta_time(const World& world);

    /** Returns total elapsed simulation time. */
    float time(const World& world);

    /** Imports a module @c T and return corresponding entity. */
    template<typename T>
    Entity import(World& world)
    {
        return world.import<T>();
    }


    /**
    @brief Advance simulation by one-step and specify elapsed time.
    @param elapsed_time time elapsed. If 0 (default), then it is automatically measured. Affected by time scale.
    @return False, if application should stop.
    */
    bool step(World& world, float elapsed_time = 0.0f);

    /**
    @brief Advance simulation by @c n step and specify elapsed time between each step.
    @param n number of steps
    @param elapsed_time time elapsed. If 0 (default), then it is automatically measured. Affected by time scale. 
    */
    void step_n(World& world, size_t n, float elapsed_time = 0.0f);

    /**
     * Run the simulation with a rest app enabled.
     * You can inspect the world with following url :
     * https://www.flecs.dev/explorer/?remote=true.
     */
    void run_with_webapp(World& world);

    /** Run the simulation. */
    void run(World& world);

	/** Stop simulation after current cycle. */
	void stop(World& world);

} // namespace opack
