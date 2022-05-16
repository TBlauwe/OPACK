#include <opack/core.hpp>
#include <opack/examples/empty_sim.hpp>
#include <opack/examples/simple_sim.hpp>

using namespace opack;

int main(int argc, char* argv[])
{
	size_t choice{ 0 };
	std::cout << "========== Menu =========\n";
	std::cout << "[1] Empty\n";
	std::cout << "[2] SimpleSim\n";
	std::cout << "Choose a simulation : ";
	std::cin >> choice;
	switch (choice)
	{
		case 1:
			EmptySim{argc, argv}.run_with_webapp();
			break;
		case 2:
			SimpleSim{argc, argv}.run_with_webapp();
			break;
		default:
			std::cout << "Closing...\n";
	}
}