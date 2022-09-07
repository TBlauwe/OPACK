#include <doctest/doctest.h>
#include <opack/core.hpp>
#include <opack/module/fipa_acl.hpp>

OPACK_SUB_PREFAB(MyAgent, opack::Agent);

TEST_CASE("fipa-acl API")
{
    auto world = opack::create_world();
    fipa_acl::import(world);
	opack::init<MyAgent>(world);

	auto sender	  = opack::spawn<MyAgent>(world);
	auto receiver_1 = opack::spawn<MyAgent>(world);
	auto receiver_2 = opack::spawn<MyAgent>(world);
	auto receiver_3 = opack::spawn<MyAgent>(world);

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
	opack::step(world);
	CHECK(message.is_alive());

	fipa_acl::send(message);
	CHECK(message.has<opack::Timestamp>());

	opack::step(world);
	CHECK(message.is_alive());

	SUBCASE("Deletion")
	{
		message.remove<fipa_acl::Receiver>(receiver_1);
		message.remove<fipa_acl::Receiver>(receiver_2);
	    opack::step(world);
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

	    opack::step(world);
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

	    opack::step(world);
		CHECK(m.is_alive()); // Message is still alive, but it will be deleted at the beginning 
							 // of the next cycle
	    opack::step(world);
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

OPACK_SENSE(Vision);
struct A {};

TEST_CASE("fipa_acl in systems")
{
    auto world = opack::create_world();
    fipa_acl::import(world);
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
	opack::each<MyAgent>(world,
		[&receive_counter](flecs::entity e)
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
	world.observer()
		.term(flecs::IsA).second<fipa_acl::Message>()
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
