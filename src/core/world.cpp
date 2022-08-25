#include <opack/core/world.hpp>

size_t opack::count(const World& world, const Entity rel, const Entity obj)
{
    return static_cast<size_t>(world.count(rel, obj));
}
