/*****************************************************************//**
 * \file   components.hpp
 * \brief  Some components defined for OPACK.
 * 
 * \author Tristan
 * \date   August 2022
 *********************************************************************/

#include <flecs.h>
#include <opack/core/api_types.hpp>

namespace opack
{
    /** Do not clean entities with this component. */
    struct DoNotClean {};

	/** Relation between an action and an entity.
	 * Actuator "X" is doing action "Y".
	 */
	struct Act {};

	/** Relation between an action and an entity.
	 * Action "X" is done "By" entity "Y".
	 */
	struct By {};

	/** Relation between an action and an entity.
	 * Action "X" is being done "On" entity "Y".
	 */
	struct On {};	

	/** Indicates the minimum and maximum of entities needed by an action. */
	struct Arity { size_t min{ 1 }; size_t max{ 1 };};

	/** Indicates how much time is left, before action is started. */
	struct Delay { float value{ 1 }; };

	/** Measure since how long it has been added. */
	struct Duration { float value{ 0.0 }; };

	/** Holds simulation time. */
	struct Timestamp
	{
		float value {0.0f};
	};

	struct Begin {};
	struct End {};
}
