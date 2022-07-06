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

	auto message = fipa_acl::MessageBuilder(sim)
		.performative(fipa_acl::Performative::AcceptProposal)
		.sender(sender)
		.receiver(receiver_1)
		.receiver(receiver_2)
		.build();

	CHECK(message.has(fipa_acl::Performative::AcceptProposal));
	CHECK(message.has<fipa_acl::Sender>(sender));
	CHECK(message.has<fipa_acl::Receiver>(receiver_1));
	CHECK(message.has<fipa_acl::Receiver>(receiver_2));
	CHECK(!message.has<fipa_acl::Receiver>(receiver_3));
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
		CHECK(m.has(fipa_acl::Performative::AcceptProposal));
		CHECK(m.has<fipa_acl::Sender>(sender));
		CHECK(m.has<fipa_acl::Receiver>(receiver_1));
		CHECK(m.has<fipa_acl::Read>(receiver_1));
		CHECK(m.has<fipa_acl::Receiver>(receiver_2));
		CHECK(!m.has<fipa_acl::Receiver>(receiver_3));

		sim.step();
		CHECK(m.is_alive());

		m = fipa_acl::receive(receiver_1);
		CHECK(!m.is_valid());

		m = fipa_acl::receive(receiver_2);
		CHECK(m.has(fipa_acl::Performative::AcceptProposal));
		CHECK(m.has<fipa_acl::Sender>(sender));
		CHECK(!m.has<fipa_acl::Receiver>(receiver_1));
		CHECK(m.has<fipa_acl::Read>(receiver_1));
		CHECK(m.has<fipa_acl::Receiver>(receiver_2));
		CHECK(m.has<fipa_acl::Read>(receiver_2));
		CHECK(!m.has<fipa_acl::Receiver>(receiver_3));

		sim.step();
		CHECK(m.is_alive()); // Message is still alive, but it will be deleted at the beginning 
							 // of the next cycle
		sim.step();  
		CHECK(!m.is_alive());
	}
}
