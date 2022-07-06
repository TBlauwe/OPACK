/*********************************************************************
 * \file   fipa_acl.hpp
 * \brief  Module to add fipa acl, i.e agent communication language. 
 * See http://www.fipa.org/specs/fipa00037/SC00037J.html.
 * 
 * \author Tristan
 * \date   July 2022
 *********************************************************************/
#pragma once

#include <flecs.h>
#include <opack/core/types.hpp>
#include <opack/utils/flecs_helper.hpp>

struct fipa_acl
{
	fipa_acl(flecs::world& world);

	// --------------------------------------------------------------------------- 
	// Message Structure
	// --------------------------------------------------------------------------- 
	struct Message : opack::Message {};
	struct Read {};
	struct Sender {};
	struct Receiver {};

	// --------------------------------------------------------------------------- 
	// Communicative Acts
	// --------------------------------------------------------------------------- 

	// The action of accepting a previously submitted proposal to perform an action.
	// The action of agreeing to perform some action, possibly in the future.
	enum class Performative
	{
		None = 0,
		AcceptProposal,
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

	/// <summary>
	/// Builder class to construct a fipa-acl message. At any moment, the message can be sent. But if there is no receivers, 
	/// then the message will be discared at the end of the cycle.
	/// </summary>
	class MessageBuilder
	{
	public:
		MessageBuilder(flecs::world& world);
		MessageBuilder(flecs::entity entity);

		MessageBuilder& performative(Performative performative);
		MessageBuilder& sender(flecs::entity sender);
		MessageBuilder& receiver(flecs::entity receiver);
		flecs::entity build();

	private:
		flecs::entity message;
	};

	static void send(flecs::entity sender);
	static flecs::entity receive(flecs::entity entity, fipa_acl::Performative performative = fipa_acl::Performative::None);

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
			Messages(flecs::world& world);
		};
	};
};
