/*****************************************************************//**
 * \file   communication.hpp
 * \brief  Communication API
 * 
 * \author Tristan
 * \date   October 2022
 *********************************************************************/
#pragma once

#include <flecs.h>

#include <opack/core/api_types.hpp>
#include <opack/core/components.hpp>

 /**
 @brief Shorthand for OPACK_SUB_PREFAB(name, opack::Message)
 */
#define OPACK_MESSAGE(name) OPACK_SUB_PREFAB(name, opack::Message)

 /**
 @brief Identical to OPACK_SUB_PREFAB(name, base)
 */
#define OPACK_SUB_MESSAGE(name, base) OPACK_SUB_PREFAB(name, base)

namespace opack
{

	/** Component relation used to indicate who send the message. */
	struct Sender{};

	/** Component relation used to indicate who should receive the message. */
	struct Receiver{};

	/** Component relation used to indicate a distribution channel. */
	struct Channel{};

	/**  Default channel */
	struct Broadcast{};

	/** Component relation used to indicate a performative. */
	struct Performative{};

	struct MessageHandleView : HandleView
	{
		using HandleView::HandleView;
	};

	struct MessageHandle : Handle
	{
		using Handle::Handle;
	};
}
