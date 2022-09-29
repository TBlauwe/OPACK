#include <flecs.h>
#include <fmt/core.h>

enum class Status
{
	FREE,
    TAKEN
};

int main(int, char* [])
{
    flecs::world ecs;
    ecs.component<Status>()
        .constant("FREE", static_cast<int32_t>(Status::FREE))
        .constant("TAKEN", static_cast<int32_t>(Status::TAKEN))
        ;
    ecs.entity().add(Status::FREE);
    fmt::print("{}", ecs.to_entity(Status::FREE).name());
    ecs.app().enable_rest().run();
}