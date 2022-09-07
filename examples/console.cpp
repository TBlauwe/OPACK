#include <opack/core.hpp>
#include <opack/module/adl.hpp>
#include <opack/module/fipa_acl.hpp>
#include <iostream>
#include <windows.h>
#include <shellapi.h>

using namespace opack;

int main()
{
	size_t choice{ 0 };
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
	ShellExecute(0, 0, url, 0, 0 , SW_SHOW );
    run_with_webapp(world);
}