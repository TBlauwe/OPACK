#include "opack/module/fipa_acl.hpp"
#include <unordered_set>

void fipa_acl::import(opack::World& world)
{
	auto scope = world.entity("::opack::modules::fipa_acl");
	auto prev = world.set_scope(scope);
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

	world.system<opack::Timestamp>("System_ConsumeMessageAfterRead")
		.term(flecs::IsA).second<Message>()
		.term<Read>().second(flecs::Wildcard)
		.kind(flecs::PreFrame)
		.iter([](flecs::iter& iter, opack::Timestamp*)
			{
				for (auto i : iter)
				{
					auto e = iter.entity(i);
					auto reader = iter.id(3);
					e.remove<Receiver>(reader);
				}
			}
	);

	world.system<opack::Timestamp>("System_CleanUp_Leftover_FIPA_ACL_Messages")
		.term(flecs::IsA).second<Message>()
		.term<Receiver>().second(flecs::Wildcard).not_()
		.kind(flecs::PostFrame)
		.each([](opack::Entity e, opack::Timestamp&)
			{
				e.destruct();
			}
	);
    world.set_scope(prev);
}

fipa_acl::MessageBuilder::MessageBuilder(opack::World& world)
	: message(world.entity().is_a<Message>())
{
	message.set<ConversationID>({static_cast<int>(message)});
	opack::_::organize_entity<Message>(message);
}

fipa_acl::MessageBuilder::MessageBuilder(opack::Entity entity)
	: message(entity.world().entity().is_a<Message>())
{
	message.set<ConversationID>({static_cast<int>(message)});
	opack::_::organize_entity<Message>(message);
}

fipa_acl::ReplyBuilder::ReplyBuilder(opack::Entity m)
	: MessageBuilder(m.world().entity().is_a<Message>())
{
	message.set<ConversationID>({static_cast<int>(m)});
	message.add<Receiver>(fipa_acl::sender(m));
}

fipa_acl::MessageBuilder& fipa_acl::MessageBuilder::performative(Performative performative)
{
	message.add(performative);
	return *this;
}

fipa_acl::MessageBuilder& fipa_acl::MessageBuilder::sender(opack::Entity sender)
{
	message.add<Sender>(sender);
	return *this;
}

fipa_acl::MessageBuilder& fipa_acl::MessageBuilder::timeout(float time)
{
	message.set<opack::TimeTimeout>({time});
	return *this;
}

fipa_acl::MessageBuilder& fipa_acl::MessageBuilder::timeout(size_t tick)
{
	message.set<opack::TickTimeout>({tick});
	return *this;
}

fipa_acl::MessageBuilder& fipa_acl::MessageBuilder::receiver(opack::Entity receiver)
{
	message.add<Receiver>(receiver);
	return *this;
}

fipa_acl::MessageBuilder& fipa_acl::MessageBuilder::conversation_id(int id)
{
	message.set<ConversationID>({ id });
	return *this;
}

opack::Entity& fipa_acl::performative(opack::Entity message, Performative performative)
{
	return message.set<Performative>({performative});
}

opack::Entity& fipa_acl::sender(opack::Entity message, opack::Entity sender)
{
	return message.add<Sender>(sender);
}

fipa_acl::Performative fipa_acl::performative(opack::Entity message)
{
	return *message.get<Performative>();
}

opack::Entity fipa_acl::sender(opack::Entity message)
{
	return message.target<Sender>();
}

int fipa_acl::conversation_id(opack::Entity message)
{
	return message.get<ConversationID>()->value;
}

float fipa_acl::timestamp(opack::Entity message)
{
	return message.get<opack::Timestamp>()->value;
}

bool fipa_acl::has_receiver(opack::Entity message, opack::Entity receiver)
{
	return message.has<Receiver>(receiver);
}

bool fipa_acl::has_been_read_by(opack::Entity message, opack::Entity reader)
{
	return message.has<Read>(reader);
}

fipa_acl::MessageBuilder fipa_acl::message(opack::Entity entity)
{
	return MessageBuilder(entity).sender(entity);
}

fipa_acl::ReplyBuilder fipa_acl::reply(opack::Entity message)
{
	return {message};
}

opack::Entity fipa_acl::MessageBuilder::build()
{
	return message;
}

opack::Entity fipa_acl::MessageBuilder::send()
{ 
	fipa_acl::send(message);
	return message;
}

void fipa_acl::send(opack::Entity message)
{
	//ecs_assert(message.has<Sender>(flecs::Wildcard), ECS_INVALID_PARAMETER, "message has no sender.");
	message.set<opack::Timestamp>({ message.world().time() });
}

fipa_acl::Inbox fipa_acl::inbox(opack::Entity entity, Performative performative)
{
	auto world = entity.world();
	auto query = world.get<queries::Messages>();
	Inbox inbox{ entity, query->rule.rule.iter().set_var(query->receiver_var, entity) };

	if(performative != Performative::None)
		inbox.iter.set_var(query->performative_var, world.id(performative));
	return inbox;
}

void fipa_acl::consume(opack::Entity message, opack::Entity reader)
{
	if(message.is_valid())
		message.add<Read>(reader);
}

opack::Entity fipa_acl::Inbox::first()
{
	auto m = iter.first();
	consume(m, entity);
	return m;
}

size_t fipa_acl::Inbox::count()
{
	return static_cast<size_t>(iter.count());
}

void fipa_acl::Inbox::clear()
{
	iter.each([this](opack::Entity message) {consume(message, entity); });
}

void fipa_acl::Inbox::each(std::function<void(opack::Entity)> func)
{
	std::unordered_set<flecs::entity_t> duplicates;
	iter.each(
		[this, &duplicates, &func](flecs::iter& it, size_t index)
		{
			auto message = it.entity(index);
			if (!duplicates.contains(message))
			{
				duplicates.insert(message);
				consume(message, entity);
				func(message);
			}
		}
	);
}

opack::Entity fipa_acl::receive(opack::Entity entity, Performative performative)
{
	auto rule = inbox(entity, performative);
	auto m = rule.first();
	if(m.is_valid())
		m.add<Read>(entity);
	return m;
}

fipa_acl::queries::Messages::Messages(opack::World& world)
	: rule
{
	world.rule_builder<>()
	.term(flecs::IsA).second<Message>()
	.term().first().var("Performative")
	.term<Sender>().second().var("Sender")
	.term<Receiver>().second().var("Receiver")
	.term<Read>().second().var("Receiver").not_()
	.build()
},
sender_var{ rule.rule.find_var("Sender") },
receiver_var{ rule.rule.find_var("Receiver") },
performative_var{ rule.rule.find_var("Performative") }
{};
