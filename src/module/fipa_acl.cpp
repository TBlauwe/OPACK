#include <opack/module/fipa_acl.hpp>
#include <unordered_set>

fipa_acl::fipa_acl(flecs::world& world)
{
	world.module<fipa_acl>();
	world.component<Message>().is_a<opack::Message>();
	world.component<Sender>().add(flecs::Exclusive);
	world.component<Receiver>();
	world.component<Read>();
	world.component<ConversationID>();
	world.component<Topic>();
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
		.kind(flecs::PreFrame)
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
		.kind(flecs::PostFrame)
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
	message.set<ConversationID>({static_cast<int>(message)});
	opack::internal::organize<opack::world::Messages>(message);
}

fipa_acl::MessageBuilder::MessageBuilder(flecs::entity entity)
	: message(entity.world().entity())
{
	message.add<Message>();
	message.set<ConversationID>({static_cast<int>(message)});
	opack::internal::organize<opack::world::Messages>(message);
}

fipa_acl::ReplyBuilder::ReplyBuilder(flecs::entity m)
	: MessageBuilder(m.world().entity())
{
	message.set<ConversationID>({static_cast<int>(m)});
	message.add<Receiver>(fipa_acl::sender(m));
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

fipa_acl::MessageBuilder& fipa_acl::MessageBuilder::conversation_id(int id)
{
	message.set<ConversationID>({ id });
	return *this;
}

flecs::entity& fipa_acl::performative(flecs::entity message, fipa_acl::Performative performative)
{
	return message.set<fipa_acl::Performative>({performative});
}

flecs::entity& fipa_acl::sender(flecs::entity message, flecs::entity sender)
{
	return message.add<Sender>(sender);
}

fipa_acl::Performative fipa_acl::performative(flecs::entity message)
{
	return *message.get<fipa_acl::Performative>();
}

flecs::entity fipa_acl::sender(flecs::entity message)
{
	return message.get_object<Sender>();
}

int fipa_acl::conversation_id(flecs::entity message)
{
	return message.get<ConversationID>()->value;
}

float fipa_acl::timestamp(flecs::entity message)
{
	return message.get<opack::Timestamp>()->value;
}

bool fipa_acl::has_receiver(flecs::entity message, flecs::entity receiver)
{
	return message.has<fipa_acl::Receiver>(receiver);
}

bool fipa_acl::has_been_read_by(flecs::entity message, flecs::entity reader)
{
	return message.has<fipa_acl::Read>(reader);
}

fipa_acl::MessageBuilder fipa_acl::message(flecs::entity entity)
{
	return fipa_acl::MessageBuilder(entity).sender(entity);
}

fipa_acl::ReplyBuilder fipa_acl::reply(flecs::entity message)
{
	return fipa_acl::ReplyBuilder(message);
}

flecs::entity fipa_acl::MessageBuilder::build()
{
	return message;
}

flecs::entity fipa_acl::MessageBuilder::send()
{ 
	fipa_acl::send(message);
	return message;
}

void fipa_acl::send(flecs::entity message)
{
	//ecs_assert(message.has<Sender>(flecs::Wildcard), ECS_INVALID_PARAMETER, "message has no sender.");
	message.set<opack::Timestamp>({ message.world().time() });
}

fipa_acl::Inbox fipa_acl::inbox(flecs::entity entity, fipa_acl::Performative performative)
{
	auto world = entity.world();
	auto query = world.get<fipa_acl::queries::Messages>();
	Inbox inbox{ entity, query->rule.rule.iter().set_var(query->receiver_var, entity) };

	if(performative != fipa_acl::Performative::None)
		inbox.iter.set_var(query->performative_var, world.id(performative));
	return inbox;
}

void fipa_acl::consume(flecs::entity message, flecs::entity reader)
{
	if(message.is_valid())
		message.add<fipa_acl::Read>(reader);
}

flecs::entity fipa_acl::Inbox::first()
{
	auto m = iter.first();
	fipa_acl::consume(m, entity);
	return m;
}

size_t fipa_acl::Inbox::count()
{
	return iter.count();
}

void fipa_acl::Inbox::clear()
{
	iter.each([this](flecs::entity message) {fipa_acl::consume(message, entity); });
}

void fipa_acl::Inbox::each(std::function<void(flecs::entity)> func)
{
	std::unordered_set<flecs::entity_t> duplicates;
	iter.each(
		[this, &duplicates, &func](flecs::iter& it, size_t index)
		{
			auto message = it.entity(index);
			if (!duplicates.contains(message))
			{
				duplicates.insert(message);
				fipa_acl::consume(message, entity);
				func(message);
			}
		}
	);
}

flecs::entity fipa_acl::receive(flecs::entity entity, fipa_acl::Performative performative)
{
	auto rule = fipa_acl::inbox(entity, performative);
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
