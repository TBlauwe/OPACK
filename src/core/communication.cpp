#include <opack/core/communication.hpp>

namespace opack
{
	void impl::import_communication(flecs::world& world)
	{
		world.component<Sender>().add(flecs::Exclusive);
        world.component<Receiver>();
        world.component<ReaderLeft>();
        world.component<Channel>();
        world.component<Performative>().add(flecs::Exclusive);

        world.entity<Broadcast>().add<Channel>();
	    world.emplace<queries::Messages>(world);

        world.system("System_ConsumeMessageAfterRead")
            .term<const Timestamp>()
            .term(flecs::IsA).second<Message>()
            .term<ReaderLeft>(flecs::Wildcard).not_()
            .kind<Cycle::End>()
            .each([](Entity message)
                {
                    message.destruct();
                }
        );

	}

    MessageHandle& MessageHandle::sender(EntityView sender)
    {
        opack_assert(sender.is_valid(), "Sender is invalid.");
        add<Sender>(sender);
        return *this;
    }

    MessageHandle& MessageHandle::timeout(float time)
    {
        set<TimeTimeout>({ time });
        return *this;
    }

    MessageHandle& MessageHandle::timeout(size_t tick)
    {
        set<TickTimeout>({ tick });
        return *this;
    }

    MessageHandle& MessageHandle::receiver(EntityView receiver)
    {
        add<Receiver>(receiver);
        add<ReaderLeft>(receiver);
        return *this;
    }

    MessageHandle& MessageHandle::send()
    {
        opack_assert(has<Sender>(flecs::Wildcard), "Message {} has no sender.", path().c_str());
        opack_assert(has<Receiver>(flecs::Wildcard) || has<Channel>(flecs::Wildcard), "Message {} has no receiver.", path().c_str());
	    set<Timestamp>({ world().time() });
        return *this;
    }

	MessageHandle write(EntityView sender, EntityView prefab)
    {	
        opack_assert(sender.is_valid(), "Sender is not valid");
        opack_assert(prefab.is_valid(), "Prefab is not valid");
		auto message = MessageHandle(sender.world(), sender.world().entity().is_a(prefab));
		message.sender(sender);
        message.add<Conversation>(message);
        opack::internal::organize_entity<Message>(message);
        return message;
	}

    MessageHandle reply(EntityView message, EntityView prefab)
    {
        opack_assert(message.is_valid(), "Message is not valid");
        opack_assert(prefab.is_valid(), "Prefab is not valid");
		auto reply = MessageHandle(message.world(), message.world().entity().is_a(prefab));
        reply.receiver(message.target<Sender>());
        reply.add<Conversation>(message);
        opack::internal::organize_entity<Message>(reply);
        return reply;
    }

    EntityView performative(EntityView message)
    {
        opack_assert(message.is_valid(), "Message is not valid");
        opack_assert(message.has<Performative>(flecs::Wildcard), "Message {} has no performative set.", message.path().c_str());
        return message.target<Performative>();
    }

    EntityView sender(EntityView message)
    {
        opack_assert(message.is_valid(), "Message is not valid");
        opack_assert(message.has<Sender>(flecs::Wildcard), "Message {} has no sender set.", message.path().c_str());
        return message.target<Sender>();
    }

    EntityView conversation_id(EntityView message)
    {
        opack_assert(message.is_valid(), "Message is not valid");
        return message.target<Conversation>();
    }

    float timestamp(EntityView message)
    {
        opack_assert(message.is_valid(), "Message is not valid");
        opack_assert(message.has<Timestamp>(), "Message {} has no timestamp yet. Has it been send ?", message.path().c_str());
        return message.get<Timestamp>()->value;
    }

    bool has_receiver(EntityView message, EntityView receiver)
    {
        opack_assert(message.is_valid(), "Message is not valid");
        opack_assert(receiver.is_valid(), "Reader is not valid");
        return message.has<Receiver>(receiver);
    }

    bool has_been_read_by(EntityView message, EntityView reader)
    {
        opack_assert(message.is_valid(), "Message is not valid");
        opack_assert(reader.is_valid(), "Reader is not valid");
        return !message.has<ReaderLeft>(reader) && message.has<Receiver>(reader);
    }

    inbox::inbox(EntityView _agent)
        : agent(_agent), 
        query(*agent.world().get<queries::Messages>()),
        iter(query.rule.rule.iter())
    {
        opack_assert(agent.is_valid(), "Agent is not valid");
        iter.set_var(query.receiver_var, agent);
    }

    Entity inbox::first()
    {
        return iter.first();
    }

    size_t inbox::count()
    {
        return static_cast<size_t>(iter.count());
    }

    void inbox::clear()
    {
        iter.each([this](Entity message) {consume(agent, message); });
    }

    void inbox::each(std::function<void(Entity)> func)
    {
        std::unordered_set<flecs::entity_t> duplicates;
        iter.each(
            [this, &duplicates, &func](flecs::iter& it, size_t index)
            {
                auto message = it.entity(index);
                if (!duplicates.contains(message))
                {
                    duplicates.insert(message);
                    consume(agent, message);
                    func(message);
                }
            }
        );
    }

    MessageHandleView receive(EntityView agent)
    {
        opack_assert(agent.is_valid(), "Agent is not valid");
        auto in = inbox(agent);
        auto m = in.first();
        if (m.is_valid())
            consume(agent, m);
        return MessageHandleView(agent.world(), m);
    }

    void consume(EntityView reader, Entity message)
    {
        opack_assert(reader.is_valid(), "Reader is not valid");
        opack_assert(message.is_valid(), "Message is not valid");
        message.mut(reader).remove<ReaderLeft>(reader);
    }

    queries::Messages::Messages(opack::World& world)
        : rule
    {
        world.rule_builder<>()
        .term(flecs::IsA).second<Message>()
        .term<Performative>().second().var("Performative")
        .term<Sender>().second().var("Sender")
        .term<Receiver>().second().var("Receiver")
        .term<ReaderLeft>().second().var("Receiver")
        .term<Conversation>().second().var("Conversation")
        .build()
    },
    sender_var{ rule.rule.find_var("Sender") },
    receiver_var{ rule.rule.find_var("Receiver") },
    performative_var{ rule.rule.find_var("Performative") },
    conversation_var{ rule.rule.find_var("Conversation") }
    {};
}
