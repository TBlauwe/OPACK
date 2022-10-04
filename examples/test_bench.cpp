#include <flecs.h>
#include <iostream>
#include <nlohmann/json.hpp>

struct Message
{
    const char* value;
};

struct AnotherMessage
{
    const char* value;
};

using json = nlohmann::json;

int main(int, char* [])
{
    flecs::world ecs;
    ecs.component<Message>()
		.member(flecs::String, "value");

    ecs.component<AnotherMessage>()
		.member(flecs::String, "value");

    ecs.plecs_from_str("test", "test_entity { - Message{\"Hello world !\"} - AnotherMessage{\"Super Hello world !\"}} ");
    auto entity = ecs.lookup("test_entity");
	flecs::entity_to_json_desc_t desc;
    desc.serialize_path = true;
    desc.serialize_label = false;
    desc.serialize_values = true;
    desc.serialize_type_info = false;
    auto value = json::parse(std::string(entity.to_json(&desc)));
    value["values"]["value"];
    std::cout <<  value.dump() << "\n"; // Prints : 
    std::cout << entity.get<Message>()->value << "\n"; // Prints : 
    ecs.app().enable_rest().run();
}