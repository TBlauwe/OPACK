#include <opack/module/fipa_acl.hpp>
#include <unordered_set>

fipa_acl::fipa_acl(opack::World& world)
{
	world.component<Performative>()
		.constant("AcceptProposal", static_cast<int32_t>(Performative::AcceptProposal))
		.constant("Agree", static_cast<int32_t>(Performative::Agree))
		.constant("Cancel", static_cast<int32_t>(Performative::Cancel))
		.constant("CallForProposal", static_cast<int32_t>(Performative::CallForProposal))
		.constant("Confirm", static_cast<int32_t>(Performative::Confirm))
		.constant("Disconfirm", static_cast<int32_t>(Performative::Disconfirm))
		.constant("Failure", static_cast<int32_t>(Performative::Failure))
		.constant("Inform", static_cast<int32_t>(Performative::Inform))
		.constant("InformIf", static_cast<int32_t>(Performative::InformIf))
		.constant("InformRef", static_cast<int32_t>(Performative::InformRef))
		.constant("NotUnderstood", static_cast<int32_t>(Performative::NotUnderstood))
		.constant("Propagate", static_cast<int32_t>(Performative::Propagate))
		.constant("Propose", static_cast<int32_t>(Performative::Propose))
		.constant("Proxy", static_cast<int32_t>(Performative::Proxy))
		.constant("QueryIf", static_cast<int32_t>(Performative::QueryIf))
		.constant("QueryRef", static_cast<int32_t>(Performative::QueryRef))
		.constant("Refuse", static_cast<int32_t>(Performative::Refuse))
		.constant("RejectProposal", static_cast<int32_t>(Performative::RejectProposal))
		.constant("Request", static_cast<int32_t>(Performative::Request))
		.constant("RequestWhen", static_cast<int32_t>(Performative::RequestWhen))
		.constant("RequestWhenever", static_cast<int32_t>(Performative::RequestWhenever))
		.constant("Subscribe", static_cast<int32_t>(Performative::Subscribe))
		;
}
