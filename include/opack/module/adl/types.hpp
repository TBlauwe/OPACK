/*****************************************************************//**
 * \file   api_types.hpp
 * \brief  API types for module Activity-DL.
 * 
 * \author Tristan
 * \date   September 2022
 *********************************************************************/
#pragma once

#include <opack/core.hpp>

/** Shorthand for creating an activity type.*/
#define ADL_ACTIVITY(name) OPACK_SUB_PREFAB(name, adl::Activity)

/**
 *Namespace "adl" is used for anything related to Activity-DL.
 *It's a domain language to represent an activity by a hierarchy of tasks.
 */
namespace adl
{
 	/** A task is any node in an activity tree, that is not a leaf, i.e an action. */
	struct Task {};

	/** Types used for explorer. */
    struct activities { struct prefabs {}; };

	/** An activity is a tree of tasks, with actions as leaf. */
	struct Activity : opack::internal::root<Activity>
	{
        using entities_folder_t = activities;
	    using prefabs_folder_t = activities::prefabs;
	};

	template<typename T>
	concept ActivityPrefab = opack::SubPrefab<T> && std::derived_from<T, Activity>;
}
