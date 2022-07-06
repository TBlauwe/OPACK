#include <opack/module/fipa_acl.hpp>

fipa_acl::fipa_acl(flecs::world& world)
{
	world.module<fipa_acl>();
	world.component<Message>().is_a<opack::Message>();
	world.component<Sender>().add(flecs::Exclusive);
	world.component<Receiver>();
	world.component<Read>();
	world.component<Performative>()
		.constant("Accept proposal", static_cast<int>(Performative::AcceptProposal))
		.constant("Agree", static_cast<int>(Performative::Agree))
		.constant("Cancel", static_cast<int>(Performative::Cancel))
		.constant("CallForProposal", static_cast<int>(Performative::CallForProposal))
		.constant("Confirm", static_cast<int>(Performative::Confirm))
		.constant("Disconfirm", static_cast<int>(Performative::Disconfirm))
		.constant("Failure", static_cast<int>(Performative::Failure))
		.constant("Inform", static_cast<int>(Performative::Inform))
		.constant("InformIf", static_cast<int>(Performative::InformIf))
		.constant("InformRef", static_cast<int>(Performative::InformRef))
		.constant("NotUnderstood", static_cast<int>(Performative::NotUnderstood))
		.constant("Propagate", static_cast<int>(Performative::Propagate))
		.constant("Propose", static_cast<int>(Performative::Propose))
		.constant("Proxy", static_cast<int>(Performative::Proxy))
		.constant("QueryIf", static_cast<int>(Performative::QueryIf))
		.constant("QueryRef", static_cast<int>(Performative::QueryRef))
		.constant("Refuse", static_cast<int>(Performative::Refuse))
		.constant("RejectProposal", static_cast<int>(Performative::RejectProposal))
		.constant("Request", static_cast<int>(Performative::Request))
		.constant("RequestWhen", static_cast<int>(Performative::RequestWhen))
		.constant("RequestWhenever", static_cast<int>(Performative::RequestWhenever))
		.constant("Subscribe", static_cast<int>(Performative::Subscribe))
		;

	world.emplace<queries::Messages>(world);

	world.system<Message, opack::Timestamp>("System_ConsumeMessageAfterRead")
		.term<Read>().obj(flecs::Wildcard)
		.kind(flecs::PostUpdate)
		.iter([](flecs::iter& iter, Message*, opack::Timestamp*)
			{
				for (auto i : iter)
				{
					auto e = iter.entity(i);
					auto reader = iter.id(3);
					e.remove<Receiver>(reader);
				}
			}
	);

	world.system<Message, opack::Timestamp>("System_CleanUp_Leftover_FIPA_ACL_Messages")
		.term<Receiver>().obj(flecs::Wildcard).oper(flecs::Not)
		.kind(flecs::PreUpdate)
		.each([](flecs::entity e, Message, opack::Timestamp)
			{
				e.destruct();
			}
	);

}

fipa_acl::MessageBuilder::MessageBuilder(flecs::world& world)
	: message(world.entity())
{
	message.add<Message>();
	opack::internal::organize<opack::world::Messages>(message);
}

fipa_acl::MessageBuilder::MessageBuilder(flecs::entity entity)
	: message(entity.world().entity())
{
	message.add<Message>();
	opack::internal::organize<opack::world::Messages>(message);
}

fipa_acl::MessageBuilder& fipa_acl::MessageBuilder::performative(fipa_acl::Performative performative)
{
	message.add(performative);
	return *this;
}

fipa_acl::MessageBuilder& fipa_acl::MessageBuilder::sender(flecs::entity sender)
{
	message.add<Sender>(sender);
	return *this;
}

fipa_acl::MessageBuilder& fipa_acl::MessageBuilder::receiver(flecs::entity receiver)
{
	message.add<Receiver>(receiver);
	return *this;
}

flecs::entity fipa_acl::MessageBuilder::build()
{
	return message;
}

void fipa_acl::send(flecs::entity message)
{
	message.set<opack::Timestamp>({ message.world().time() });
}

flecs::entity fipa_acl::receive(flecs::entity entity, fipa_acl::Performative performative)
{
	auto world = entity.world();
	auto query = world.get<fipa_acl::queries::Messages>();
	auto rule = query->rule.rule.iter()
		.set_var(query->receiver_var, entity);

	if(performative != fipa_acl::Performative::None)
		rule.set_var(query->performative_var, world.id(performative));
	;

	auto m = rule.first();
	if(m.is_valid())
		m.add<Read>(entity);
	return m;
}

fipa_acl::queries::Messages::Messages(flecs::world& world)
	: rule
{
	world.rule_builder<Message>()
	.expr("$Performative(This)")
	.term<Sender>().obj().var("Sender")
	.term<Receiver>().obj().var("Receiver")
	.term<Read>().obj().var("Receiver").oper(flecs::Not)
	.build()
},
sender_var{ rule.rule.find_var("Sender") },
receiver_var{ rule.rule.find_var("Receiver") },
performative_var{ rule.rule.find_var("Performative") }
{};
