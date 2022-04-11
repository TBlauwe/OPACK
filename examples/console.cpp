#include <opack/core.hpp>
#include "simple_sim.hpp"

using namespace opack;

int main(int argc, char* argv[])
{
	SimpleSim sim{argc, argv};
	sim.run();
}