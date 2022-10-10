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

	/** Component relation used to track a conversation. */
	struct Conversation{};

	/** Component relation used to indicate that the message has not yet been read by target. */
	struct ReaderLeft{};

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

		MessageHandle& sender(EntityView sender);

		MessageHandle& receiver(EntityView receiver);

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

		/** @brief Send the message and returns it. */
		MessageHandle& send();
	};

	/** Create or retrieve a channel identified by @c T. */
	template<typename T>
	Entity channel(World& world);

	/** Write a message from prefab @c prefab. */
	MessageHandle write(EntityView sender, EntityView prefab);

	/** Write a message from prefab @c T with @c sender as sender. */
	template<std::derived_from<Message> T = Message>
	MessageHandle write(EntityView sender);

	/** Reply to @c message using prefab @c prefab with a receiver set to previous sender. */
	MessageHandle reply(EntityView message, EntityView prefab);

	/** Reply to @c message using prefab @c T with a receiver set to previous sender. */
	template<std::derived_from<Message> T = Message>
	MessageHandle reply(EntityView message);

	/** Return a received message, if any (null entity returned otherwise), for @c agent. */
	MessageHandleView receive(EntityView agent);

	/** From @c reader pov, @c message doesn't matter anymore. If every receiver of a @c message
	 * has consumed the message, it will be discarded. */
	void consume(EntityView reader, Entity message);

	// ~~~ Getters ~~~
	EntityView performative(EntityView message);
	EntityView sender(EntityView message);
	EntityView conversation_id(EntityView message);
	float timestamp(EntityView message);
	bool has_receiver(EntityView message, EntityView receiver);
	bool has_been_read_by(EntityView message, EntityView reader);

	namespace queries
	{
		/**
		 * Query :
		 * @code
			opack.Sender(This, $Sender), opack.Receiver(This, $Receiver), opack.Performative(This, $Performative), opack.ReaderLeft(This, $Receiver)
		 * @endcode.
		 */
		struct Messages
		{
			opack::internal::Rule rule;
			int32_t sender_var;
			int32_t receiver_var;
			int32_t performative_var;
			int32_t conversation_var;
			Messages(opack::World& world);
		};
	};

	/**
	 * @brief Allows to retrieve messages from a template.
	 * You should not create it manually and use @c fipa_acl::inbox() instead.
	 */
	struct inbox
	{
		inbox(EntityView agent);
		Entity first();
		size_t count();
		void each(std::function<void(opack::Entity)> func);
		void clear();

		EntityView agent;
		const queries::Messages& query;
		flecs::iter_iterable<> iter;
	};

}

namespace opack
{
	namespace impl
	{
        void import_communication(flecs::world& world);
	}


	template<typename T>
	MessageHandle& MessageHandle::performative(T&& performative)
	{
		if constexpr (std::is_same_v<T, opack::Entity>)
			add<Performative>(performative);
		else if constexpr (std::is_enum_v<T>)
			add<Performative>(world().to_entity(performative));
		else 
			opack_assert(true, "Type {} is not an entity or a enum type (either must be used as a performative).", type_name_cstr<T>());
        return *this;
	}

	template<typename T>
	Entity channel(World& world)
	{
		return world.entity<T>().add<Channel>();
	}

	template<std::derived_from<Message> T>
	MessageHandle write(EntityView sender)
	{
		opack_assert(sender.is_valid(), "Sender is invalid.");
		return write(sender, sender.world().entity<T>());
	}

	template<std::derived_from<Message> T>
	MessageHandle reply(EntityView message)
	{
		opack_assert(message.is_valid(), "Message is invalid.");
		return reply(message, message.world().entity<T>());
	}
}
