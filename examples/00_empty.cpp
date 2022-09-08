#include <opack/core.hpp>

int main()
{
	auto world = opack::create_world();
	opack::run_with_webapp(world);
}
