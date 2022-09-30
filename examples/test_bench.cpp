#include <flecs.h>
#include <fmt/core.h>

enum class Status
{
	FREE,
    TAKEN
};

struct R{};

inline void print(flecs::entity_view entity)
{
	fmt::print("Entity path {}\n", entity.path());
}

int main(int, char* [])
{
    flecs::world ecs;
    ecs.component<Status>()
        .constant("FREE", static_cast<int32_t>(Status::FREE))
        .constant("TAKEN", static_cast<int32_t>(Status::TAKEN))
        ;
    auto prefab = ecs.prefab("TestEntity").add(Status::FREE);
    auto entity = ecs.entity("TestEntity2").is_a(prefab);
    fmt::print("{}", entity.has(Status::FREE));
    ecs.system().term(Status::FREE).each(print);
    ecs.app().enable_rest().run();
}