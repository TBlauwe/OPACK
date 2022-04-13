#include <opack/core.hpp>
#include "simple_sim.hpp"
#include "empty_sim.hpp"

using namespace opack;



int main(int argc, char* argv[])
{
	size_t choice{ 0 };
	std::cout << "========== Menu =========\n";
	std::cout << "[1] Empty\n";
	std::cout << "[2] SimpleSim (TestBench)\n";
	std::cout << "Choose a simulation : ";
	std::cin >> choice;
	switch (choice)
	{
		case 1:
			EmptySim{argc, argv}.run();
			break;
		case 2:
			SimpleSim{argc, argv}.run();
			break;
		default:
			std::cout << "Closing...\n";
	}
}