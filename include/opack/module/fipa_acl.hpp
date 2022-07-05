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

struct fipa
{
	fipa(flecs::world& world);

	// --------------------------------------------------------------------------- 
	// Message Structure
	// --------------------------------------------------------------------------- 
	struct Sender {};
	struct Receiver {};

	// --------------------------------------------------------------------------- 
	// Communicative Acts
	// --------------------------------------------------------------------------- 

	// The action of accepting a previously submitted proposal to perform an action.
	struct AcceptProposal {};

	// The action of agreeing to perform some action, possibly in the future.
	struct Agree {};

	struct Cancel {};
	struct CallForProposal {};
	struct Confirm {};
	struct Disconfirm {};
	struct Failure {};
	struct Inform {};
	struct InformIf {};
	struct InformRef {};
	struct NotUnderstood {};
	struct Propagate {};
	struct Propose {};
	struct Proxy {};
	struct QueryIf {};
	struct QueryRef {};
	struct Refuse {};
	struct RejectProposal {};
	struct Request {};
	struct RequestWhen {};
	struct RequestWhenever {};
	struct Subscribe {};

	struct MessageBuilder
	{

	};
};
