#include <string>

#include <opack/core.hpp>
#include <opack/examples/all.hpp>

using namespace opack;

int main(int argc, char* argv[])
{
	size_t choice{ 0 };
	std::cout << "========== Menu =========\n";
	std::cout << "[1] Empty\n";
	std::cout << "[2] SimpleSim\n";
	std::cout << "[3] MedicalSim\n";
	std::cout << "Choose a simulation : ";
	std::cin >> choice;
	std::unique_ptr<Simulation> sim;
	switch (choice)
	{
		case 1:
			sim = std::make_unique<EmptySim>(argc, argv);
			break;
		case 2:
			sim = std::make_unique<SimpleSim>(argc, argv);
			break;
		case 3:
			sim = std::make_unique<MedicalSim>(argc, argv);
			break;
		default:
			std::cout << "Closing...\n";
	}

	std::string input;
	std::getline(std::cin, input); // Clear first line
	while (input != "Q")
	{
		std::cout << "\n========== Controls =========\n";
		std::cout << "[only ENTER] : step once\n";
		std::cout << "         [Q] : quit\n";
		std::cout << "Input : ";
		std::getline(std::cin, input);
		std::cout << "\n";
		if (input.empty())
		{
			sim->step();
		}
	}

}