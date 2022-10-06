#include <flecs.h>
#include <fmt/core.h>
#include <nlohmann/json.hpp>
#include <utility>

struct Message
{
    const char* value;
};

namespace NS
{
	struct AnotherMessage
	{
		const char* value;
	};

    enum class Status {FREE, OCCUPIED};
}

using json = nlohmann::json;

auto to_json(flecs::entity_view entity)
{
	flecs::entity_to_json_desc_t desc;
    desc.serialize_path = false;
    desc.serialize_label = false;
    desc.serialize_values = true;
    desc.serialize_type_info = false;

    return json::parse(std::string(entity.to_json(&desc)));
}

int8_t find_key_index(const json& array, std::string pattern)
{
    int8_t idx {0};
    for(const auto& id : array)
    {
        if (id[0].get<std::string>() == pattern)
            return idx;
        idx++;
    }
	return -1;
}

std::pair<std::string, std::string> parse(std::string pattern)
{
	if (const std::size_t pos = pattern.rfind('.'); pos != std::string::npos)
        return{ pattern.substr(0, pos), pattern.substr(pos + 1) };
    return {"", ""};
}

std::string value_from(flecs::entity_view e, std::string pattern)
{
	auto object = to_json(e);
	auto key = find_key_index(object.at("ids"), pattern); // If enum / relation
	if(key >= 0)
		return object.at("values")[key].get<std::string>();

	if(auto [type_str, value_str] = parse(std::move(pattern)); !type_str.empty() && !value_str.empty())
    {
        key = find_key_index(object.at("ids"), type_str);
		if(key >= 0)
			return object.at("values")[key].at(value_str);
    }
    return "";
}

int main(int, char* [])
{
    flecs::world ecs;
    ecs.component<Message>()
		.member(flecs::String, "value");

    ecs.component<NS::AnotherMessage>()
		.member(flecs::String, "value");

    ecs.component<NS::Status>()
        .constant("FREE", static_cast<int32_t>(NS::Status::FREE))
        .constant("OCCUPIED", static_cast<int32_t>(NS::Status::OCCUPIED));

    ecs.plecs_from_str("test", "Prefab test_entity { - Message{\"Hello world !\"} - NS.AnotherMessage{\"Super Hello world !\"} - NS.Status {OCCUPIED} } e : test_entity");
    auto entity = ecs.lookup("e");
	flecs::entity_to_json_desc_t desc;
    desc.serialize_path = false;
    desc.serialize_label = false;
    desc.serialize_base = true;
    desc.serialize_values = true;
    desc.serialize_type_info = false;

    auto value = json::parse(std::string(entity.to_json(&desc)));
    fmt::print("JSON : {}\n", value.dump()); // Prints : 
	fmt::print("ids : {}\n", value.at("ids").dump());
    for(auto& id : value.at("ids"))
    {
        fmt::print("- id : {}\n", id[0].get<std::string>());
    }

    auto pattern = "NS.AnotherMessage.value";
    fmt::print("Value of entity {} from pattern {} equals {}\n", entity.path(), pattern, value_from(entity, pattern));

    pattern = "NS.Status";
    fmt::print("Value of entity {} from pattern {} equals {}\n", entity.path(), pattern, value_from(entity, pattern));
}