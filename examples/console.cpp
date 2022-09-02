#include <opack/core.hpp>
#include <opack/module/adl.hpp>
#include <opack/module/fipa_acl.hpp>
#include <fmt/compile.h>
#include <iostream>
#include <windows.h>
#include <shellapi.h>

using namespace opack;

int main()
{
	size_t choice{ 0 };
	fmt::print("========== Menu =========\n");
	fmt::print("[1] Empty\n");
	fmt::print("[2] w\\ modules\n");
	//fmt::print("[2] SimpleSim\n");
	//fmt::print("[3] MedicalSim\n");
	fmt::print("Choose a simulation : ");
	std::cin >> choice;
	auto world = create_world();
	switch (choice)
	{
	    case 2:
		    fipa_acl::import(world);
		    adl::import(world);
			break;
		case 1:
		default:
			std::cout << "Running empty simulation...\n";
	}
	auto url = "https://www.flecs.dev/explorer/?remote=true";
	fmt::print("Explorer url : {}\n", url);
	ShellExecute(0, 0, url, 0, 0 , SW_SHOW );
    run_with_webapp(world);
}