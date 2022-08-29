/*****************************************************************//**
 * \file   simulation.hpp
 * \brief  API to controls the simulation.
 * 
 * \author Tristan
 * \date   August 2022
 *********************************************************************/
#pragma once

#include <opack/core/api_types.hpp>

/**
@brief Shorthand for OPACK_SUB_PREFAB(name, opack::Agent)
*/
#define OPACK_AGENT(name) OPACK_SUB_PREFAB(name, opack::Agent)

/**
@brief Identical to OPACK_SUB_PREFAB(name, base)
*/
#define OPACK_SUB_AGENT(name, base) OPACK_SUB_PREFAB(name, base)

/**
@brief Shorthand for OPACK_SUB_PREFAB(name, opack::Artefact)
*/
#define OPACK_ARTEFACT(name) OPACK_SUB_PREFAB(name, opack::Artefact)

/**
@brief Identical to OPACK_SUB_PREFAB(name, base)
*/
#define OPACK_SUB_ARTEFACT(name, base) OPACK_SUB_PREFAB(name, base)

namespace opack {


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
    float time_scale(const World& world);

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
