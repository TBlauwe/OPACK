#include <doctest/doctest.h>
#include <opack/core.hpp>
#include <opack/module/fipa_acl.hpp>
#include <iostream>

TEST_SUITE_BEGIN("Module : FIPA-ACL");

TEST_CASE("Initialization")
{
	auto sim = opack::Simulation();
	auto module = sim.import<fipa_acl>();
	CHECK(sim.world.has<fipa_acl>());
}

TEST_CASE("Basics")
{
	auto sim = opack::Simulation();
	sim.import<fipa_acl>();

	auto sender = opack::agent(sim);
	auto receiver_1 = opack::agent(sim);
	auto receiver_2 = opack::agent(sim);
	auto receiver_3 = opack::agent(sim);

	auto message = fipa_acl::message(sender)
		.performative(fipa_acl::Performative::AcceptProposal)
		.receiver(receiver_1)
		.receiver(receiver_2)
		.build();

	CHECK(fipa_acl::performative(message) == fipa_acl::Performative::AcceptProposal);
	CHECK(fipa_acl::sender(message) == sender);
	CHECK(fipa_acl::conversation_id(message) == message);
	CHECK(fipa_acl::has_receiver(message, receiver_1));
	CHECK(fipa_acl::has_receiver(message, receiver_2));
	CHECK(!fipa_acl::has_receiver(message, receiver_3));
	sim.step();
	CHECK(message.is_alive());

	fipa_acl::send(message);
	CHECK(message.has<opack::Timestamp>());

	sim.step();
	CHECK(message.is_alive());

	SUBCASE("Deletion")
	{
		message.remove<fipa_acl::Receiver>(receiver_1);
		message.remove<fipa_acl::Receiver>(receiver_2);
		sim.step();
		CHECK(!message.is_alive());
	}

	SUBCASE("Reception")
	{
		auto m = fipa_acl::receive(receiver_1);
		CHECK(fipa_acl::performative(m) == fipa_acl::Performative::AcceptProposal);
		CHECK(fipa_acl::conversation_id(m) == message);
		CHECK(fipa_acl::sender(m) == sender);
		CHECK(fipa_acl::has_receiver(m, receiver_1));
		CHECK(fipa_acl::has_receiver(m, receiver_2));
		CHECK(!fipa_acl::has_receiver(m, receiver_3));
		CHECK(fipa_acl::has_been_read_by(m, receiver_1));
		CHECK(!fipa_acl::has_been_read_by(m, receiver_2));

		sim.step();
		CHECK(m.is_alive()); // Still one receiver left.

		m = fipa_acl::receive(receiver_1);
		CHECK(!m.is_valid());

		m = fipa_acl::receive(receiver_2);
		CHECK(fipa_acl::performative(m) == fipa_acl::Performative::AcceptProposal);
		CHECK(fipa_acl::conversation_id(m) == message);
		CHECK(fipa_acl::sender(m) == sender);
		CHECK(!fipa_acl::has_receiver(m, receiver_1)); // Has been consumed since we step once and receiver_1 has read the message.
		CHECK(fipa_acl::has_been_read_by(m, receiver_1));
		CHECK(fipa_acl::has_receiver(m, receiver_2));
		CHECK(fipa_acl::has_been_read_by(m, receiver_2));
		CHECK(!fipa_acl::has_receiver(m, receiver_3));
		CHECK(!fipa_acl::has_been_read_by(m, receiver_3));

		sim.step();
		CHECK(m.is_alive()); // Message is still alive, but it will be deleted at the beginning 
							 // of the next cycle
		sim.step();  
		CHECK(!m.is_alive());
	}

	SUBCASE("Reception")
	{
		auto m = fipa_acl::receive(receiver_1);
		auto response = fipa_acl::reply(m)
			.sender(receiver_1)
			.performative(fipa_acl::Performative::Refuse)
			.send();

		CHECK(fipa_acl::sender(response) == receiver_1);
		CHECK(fipa_acl::performative(response) == fipa_acl::Performative::Refuse);
		CHECK(fipa_acl::conversation_id(response) == message);
		CHECK(fipa_acl::has_receiver(response, sender));
		CHECK(!fipa_acl::has_been_read_by(response, sender));
		CHECK(!fipa_acl::has_receiver(response, receiver_2));
		CHECK(!fipa_acl::has_been_read_by(response, receiver_2));
		CHECK(!fipa_acl::has_receiver(response, receiver_3));
		CHECK(!fipa_acl::has_been_read_by(response, receiver_3));

		auto ack = fipa_acl::receive(sender);
		CHECK(fipa_acl::sender(ack) == receiver_1);
		CHECK(fipa_acl::performative(ack) == fipa_acl::Performative::Refuse);
		CHECK(fipa_acl::conversation_id(ack) == message);
		CHECK(fipa_acl::has_receiver(response, sender));
		CHECK(fipa_acl::has_been_read_by(response, sender));
		CHECK(!fipa_acl::has_receiver(response, receiver_2));
		CHECK(!fipa_acl::has_been_read_by(response, receiver_2));
		CHECK(!fipa_acl::has_receiver(response, receiver_3));
		CHECK(!fipa_acl::has_been_read_by(response, receiver_3));
	}
}

TEST_CASE("In systems")
{

	auto sim = opack::Simulation();
	sim.import<fipa_acl>();

	struct Vision : opack::Sense {};
	struct A {};
	opack::reg<Vision>(sim);
	opack::perceive<Vision, opack::Agent>(sim);

	auto a1 = opack::agent(sim, "a1");
	auto a2 = opack::agent(sim, "a2");
	auto a3 = opack::agent(sim, "a3");
	auto a4 = opack::agent(sim, "a4");
	opack::perceive<Vision>(a1, a2);
	opack::perceive<Vision>(a1, a3);
	opack::perceive<Vision>(a2, a3);
	opack::perceive<Vision>(a3, a4);

	int nb_agents = opack::count<opack::Agent>(sim);
	int initial_counter = 10;
	int counter = initial_counter * nb_agents;

	sim.world.system<opack::Agent>()
		.each([&counter](flecs::entity e, opack::Agent)
			{
				if (counter)
				{
					opack::each_perceived<opack::Agent>(e,
						[&e](flecs::entity subject)
						{
							auto m = fipa_acl::message(e)
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
	sim.world.system<opack::Agent>()
		.each([&receive_counter](flecs::entity e, opack::Agent)
			{
				auto inbox = fipa_acl::inbox(e);
				inbox.each(
					[&receive_counter](flecs::entity message)
					{
						receive_counter++;
					}
				);
			}
	);

	int send_counter{ 0 };
	sim.world.observer<fipa_acl::Message>()
		.event(flecs::OnAdd)
		.each([&send_counter](flecs::entity e, fipa_acl::Message) 
			{
				send_counter++;
			}
	);

	//sim.run_with_webapp();
	sim.step_n(initial_counter + 1); // Because message are received during next tick.
	int expected = initial_counter * nb_agents;
	CHECK(send_counter == expected);
	CHECK(receive_counter == send_counter);
}
