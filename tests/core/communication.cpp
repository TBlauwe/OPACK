#include <doctest/doctest.h>
#include <opack/core.hpp>
#include <opack/module/fipa_acl.hpp>

TEST_CASE("Communication API")
{
	OPACK_AGENT(MyAgent);
    auto world = opack::create_world();
	opack::init<MyAgent>(world);

	auto sender		= opack::spawn<MyAgent>(world);
	auto receiver_1 = opack::spawn<MyAgent>(world);
	auto receiver_2 = opack::spawn<MyAgent>(world);
	auto receiver_3 = opack::spawn<MyAgent>(world);

	auto message = opack::write(sender)
		.performative(fipa_acl::Performative::AcceptProposal)
		.receiver(receiver_1)
		.receiver(receiver_2)
		;

	CHECK(opack::performative(message) == world.to_entity(fipa_acl::Performative::AcceptProposal));
	CHECK(opack::sender(message) == sender);
	CHECK(opack::conversation_id(message) == message);
	CHECK(opack::has_receiver(message, receiver_1));
	CHECK(opack::has_receiver(message, receiver_2));
	CHECK(!opack::has_receiver(message, receiver_3));
	opack::step(world);
	CHECK(message.is_alive());

	message.send();
	CHECK(message.has<opack::Timestamp>());

	opack::step(world);
	CHECK(message.is_alive());

	SUBCASE("Deletion")
	{
		opack::consume(receiver_1, message);
		opack::consume(receiver_2, message);
	    opack::step(world);
		CHECK(!message.is_alive());
	}

	SUBCASE("Reception")
	{
		auto m = opack::receive(receiver_1);
		CHECK(opack::performative(m) == world.to_entity(fipa_acl::Performative::AcceptProposal));
		CHECK(opack::conversation_id(m) == message);
		CHECK(opack::sender(m) == sender);
		CHECK(opack::has_receiver(m, receiver_1));
		CHECK(opack::has_receiver(m, receiver_2));
		CHECK(!opack::has_receiver(m, receiver_3));
		CHECK(opack::has_been_read_by(m, receiver_1));
		CHECK(!opack::has_been_read_by(m, receiver_2));

	    opack::step(world);
		CHECK(m.is_alive()); // Still one receiver left.

		m = opack::receive(receiver_1);
		CHECK(!m.is_valid());

		m = opack::receive(receiver_2);
		CHECK(opack::performative(m) == world.to_entity(fipa_acl::Performative::AcceptProposal));
		CHECK(opack::conversation_id(m) == message);
		CHECK(opack::sender(m) == sender);
		CHECK(opack::has_receiver(m, receiver_1));
		CHECK(opack::has_been_read_by(m, receiver_1));
		CHECK(opack::has_receiver(m, receiver_2));
		CHECK(opack::has_been_read_by(m, receiver_2));
		CHECK(!opack::has_receiver(m, receiver_3));
		CHECK(!opack::has_been_read_by(m, receiver_3));

	    opack::step(world);
		CHECK(!m.is_alive()); 
	}

	SUBCASE("Reply")
	{
		auto m = opack::receive(receiver_1);
		auto response = opack::reply(m)
			.sender(receiver_1)
			.performative(fipa_acl::Performative::Refuse)
			.send();

		CHECK(opack::sender(response) == receiver_1);
		CHECK(opack::performative(response) == world.to_entity(fipa_acl::Performative::Refuse));
		CHECK(opack::conversation_id(response) == message);
		CHECK(opack::has_receiver(response, sender));
		CHECK(!opack::has_been_read_by(response, sender));
		CHECK(!opack::has_receiver(response, receiver_2));
		CHECK(!opack::has_been_read_by(response, receiver_2));
		CHECK(!opack::has_receiver(response, receiver_3));
		CHECK(!opack::has_been_read_by(response, receiver_3));

		auto ack = opack::receive(sender);
		CHECK(opack::sender(ack) == receiver_1);
		CHECK(opack::performative(ack) == world.to_entity(fipa_acl::Performative::Refuse));
		CHECK(opack::conversation_id(ack) == message);
		CHECK(opack::has_receiver(response, sender));
		CHECK(opack::has_been_read_by(response, sender));
		CHECK(!opack::has_receiver(response, receiver_2));
		CHECK(!opack::has_been_read_by(response, receiver_2));
		CHECK(!opack::has_receiver(response, receiver_3));
		CHECK(!opack::has_been_read_by(response, receiver_3));
	}
}


TEST_CASE("Communication in systems")
{
	OPACK_SENSE(Vision);
	struct A {};
	OPACK_AGENT(MyAgent);

    auto world = opack::create_world();
    world.import<fipa_acl>();
	opack::init<Vision>(world);
	opack::init<MyAgent>(world);
	opack::add_sense<Vision, MyAgent>(world);

	auto a1 = opack::spawn<MyAgent>(world, "a1");
	auto a2 = opack::spawn<MyAgent>(world, "a2");
	auto a3 = opack::spawn<MyAgent>(world, "a3");
	auto a4 = opack::spawn<MyAgent>(world, "a4");
	opack::perceive<Vision>(a1, a2);
	opack::perceive<Vision>(a1, a3);
	opack::perceive<Vision>(a2, a3);
	opack::perceive<Vision>(a3, a4);

	int nb_agents = opack::count<opack::Agent>(world);
	int initial_counter = 10;
	int counter = initial_counter * nb_agents;

	opack::each<MyAgent>(world,
		[&counter](flecs::entity e)
			{
				if (counter)
				{
					opack::perception(e).each<MyAgent>(
						[&e](flecs::entity subject)
						{
							opack::write(e)
								.performative(fipa_acl::Performative::AcceptProposal)
								.receiver(subject)
								.send();
						}
						);
					counter--;
				}
			}
	);

	int receive_counter{ 0 };
	opack::each<MyAgent>(world,
		[&receive_counter](flecs::entity e)
			{
				auto inbox = opack::inbox(e);
				inbox.each(
					[&receive_counter](flecs::entity message)
					{
						receive_counter++;
					}
				);
			}
	);

	int send_counter{ 0 };
	world.observer()
		.term(flecs::IsA).second<opack::Message>()
		.event(flecs::OnAdd)
		.each([&send_counter](flecs::entity e) 
			{
				send_counter++;
			}
	);

	opack::step_n(world, initial_counter + 1); // Because message are received during next tick.
	int expected = initial_counter * nb_agents;
	CHECK(send_counter == expected);
	CHECK(receive_counter == send_counter);
}
