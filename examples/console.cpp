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
	switch (choice)
	{
		case 1:
			EmptySim{argc, argv}.run_with_webapp();
			break;
		case 2:
			SimpleSim{argc, argv}.run_with_webapp();
			break;
		case 3:
			MedicalSim{argc, argv}.run_with_webapp();
			break;
		default:
			std::cout << "Closing...\n";
	}
}