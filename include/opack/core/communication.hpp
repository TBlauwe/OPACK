/*****************************************************************//**
 * \file   communication.hpp
 * \brief  Communication API
 * 
 * \author Tristan
 * \date   October 2022
 *********************************************************************/
#pragma once

#include <concepts>

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

		template<typename T>
		MessageHandle& performative(T&& performative);

		MessageHandle& sender(opack::EntityView sender);
        /**
         * \brief After @c time, it will be deleted.
         * \param time in seconds.
         */
        MessageHandle& timeout(float time);
        /**
         * \brief After @c tick, it will be deleted.
         * \param tick simulation tick.
         */
		MessageHandle& timeout(size_t tick);
		MessageHandle& receiver(opack::EntityView receiver);
		MessageHandle& conversation_id(int id);

		/** @brief Build the message and returns it. */
		opack::Entity build();
		/** @brief Send the message and returns it. */
		opack::Entity send();
	};
}

namespace opack
{
	template<typename T>
	MessageHandle& MessageHandle::performative(T&& performative)
	{
		if constexpr (std::is_same_v<T, opack::Entity>)
			add<Performative>(performative);
		if constexpr (std::is_enum_v<T>)
			add<Performative>(world().to_entity(performative));
	}
}
