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
};
