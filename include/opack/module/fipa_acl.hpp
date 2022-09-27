/*****************************************************************//**
 * \file   fipa_acl.hpp
 * \brief  Module adding fipa-acl like messages, i.e agent communication language. 
 * See http://www.fipa.org/specs/fipa00037/SC00037J.html.
 * 
 * \author Tristan 
 * \date   August 2022
 *********************************************************************/
#pragma once

#include <flecs.h>
#include <opack/core.hpp>
#include <opack/utils/flecs_helper.hpp>

struct fipa_acl
{
	fipa_acl(opack::World& world);

	// --------------------------------------------------------------------------- 
	// Message Structure
	// --------------------------------------------------------------------------- 
	OPACK_SUB_PREFAB(Message, opack::Message);

	struct Read {};
	struct Sender {};
	struct Receiver {};
	struct Channel {};
	struct ConversationID 
	{
		int value {0};
	};
	struct Topic {};

	// --------------------------------------------------------------------------- 
	// Communicative Acts
	// --------------------------------------------------------------------------- 

	/**  A performative indicates the intention behind a message. */
	enum class Performative
	{
		None = 0,
	    // Accepts a previously submitted proposal to perform an action.
		AcceptProposal,
	    // Agrees to perform some action, possibly in the future.
		Agree,
		Cancel,
		CallForProposal,
		Confirm,
		Disconfirm,
		Failure,
		Inform,
		InformIf,
		InformRef,
		NotUnderstood,
		Propagate,
		Propose,
		Proxy,
		QueryIf,
		QueryRef,
		Refuse,
		RejectProposal,
		Request,
		RequestWhen,
		RequestWhenever,
		Subscribe
	};

	/**
	 *@brief Builder class to construct a fipa-acl message. At any moment, the message can be sent. But if there is no receivers,
	 *then the message will be discarded at the end of the cycle.
	 */
	class MessageBuilder
	{
	public:
		MessageBuilder(opack::World& world);
		MessageBuilder(opack::Entity entity);

		MessageBuilder& performative(Performative performative);
		MessageBuilder& sender(opack::Entity sender);
        /**
         * \brief After @c time, it will be deleted.
         * \param time in seconds.
         */
        MessageBuilder& timeout(float time);
        /**
         * \brief After @c tick, it will be deleted.
         * \param tick simulation tick.
         */
		MessageBuilder& timeout(size_t tick);
		MessageBuilder& receiver(opack::Entity receiver);
		MessageBuilder& conversation_id(int id);

		/** @brief Build the message and returns it. */
		opack::Entity build();
		/** @brief Send the message and returns it. */
		opack::Entity send();

	protected:
		opack::Entity message;
	};

	class ReplyBuilder : public MessageBuilder
	{
	public:
		ReplyBuilder(opack::Entity message);
	};

	static void consume(opack::Entity message, opack::Entity reader);

	/**
	 * @brief Allows to retrieve messages from a template.
	 * You should not create it manually and use @c fipa_acl::inbox() instead.
	 */
	struct Inbox
	{
		opack::Entity entity;
		flecs::iter_iterable<> iter;

		opack::Entity first();
		size_t count();
		void each(std::function<void(opack::Entity)> func);
		void clear();
	};

	/**
	 *@brief Returns a message builder with the sender already initialized to entity.
	 *@param sender Automatically set as the sender.
	 */
	static MessageBuilder message(opack::Entity sender);

	/**
	 *@brief Sends the messages, so that other can read it.
	 *@param sender Automatically set as the sender.
	 */
	static void send(opack::Entity sender);


    /**
     *@brief Return an iterable to retrieve messages matching template. More performant if you need to have multiple calls.
     */
	static Inbox inbox(opack::Entity entity, Performative performative = Performative::None);

	/**
	 *@brief Retrieve the first message addressed to this entity, matching the template.
	 *It's a shortcut to using the inbox. However, if you need to receive multiple messages, prefer to use @ref inbox.
	 */
	static opack::Entity receive(opack::Entity entity, Performative performative = Performative::None);

	/**
	 *@brief Create a reply from given message.
	 *@param message Reply to this message.
	 */
	static ReplyBuilder reply(opack::Entity message);

	// Setter
	static opack::Entity& performative(opack::Entity message, fipa_acl::Performative performative);
	static opack::Entity& sender(opack::Entity message, opack::Entity sender);

	// Getter
	static Performative performative(opack::Entity message);
	static opack::Entity sender(opack::Entity message);
	static int conversation_id(opack::Entity message);
	static float timestamp(opack::Entity message);
	static bool has_receiver(opack::Entity message, opack::Entity receiver);
	static bool has_been_read_by(opack::Entity message, opack::Entity reader);

	struct queries
	{
		/**
		 * Query :
		 * @code
			fipa_acl.Sender(This, $Sender), fipa_acl.Receiver(This, $Receiver), $Performative(This), !fipa_acl.Read(This, $Receiver)
		 * @endcode.
		 */
		struct Messages
		{
			opack::internal::Rule rule;
			int32_t sender_var;
			int32_t receiver_var;
			int32_t performative_var;
			Messages(opack::World& world);
		};
	};
};
