![wip](https://img.shields.io/badge/-WIP-red)

# OPACK

__OPACK__ is a C++ library to simulate a world with cognitive agents. The main goal is to allow quick construction of an agent model tailored to your needs. An agent model in OPACK is defined as follow :

* __O__ : set of operations manipulating __PACK__
* __PACK__ : set of data specific for an agent
* __P__ : set of current percepts
* __A__ : set of current actions
* __C__ : set of caracteristics (stable, e.g personality traits, or dynamic, e.g stress, emotion, etc.)
* __K__ : set of knowledge.

We provide tools to facilitate the construction of an OPACK that suit your needs. While a whole virtual environment can be created using this tool, here we suppose you already have one.

> :warning: Project not ready yet !

```cpp
#include <opack/core.hpp>

struct Vision : opack::Sense {};
struct Mesh { /*...*/ };

int main()
{
	// Step I : Initialize an empty simulation
	//========================================
	auto sim = opack::Simulation();       

	sim.register_sense<Vision>()	// Define a new sense 
	sim.perceive<Vision, Mesh>();	// Define what data can be perceived by this sense.

	// Step II : Populate your simulation
	//	- Can also be done when running the simulation
	//===================================
	auto artefact   = opack::artefact(sim, "An artefact");  // Create an artefact.
	artefact.add<Mesh>();

	auto arthur     = opack::agent(sim, "Arthur");          // Create an agent.
	arthur.add<Mesh>();


	// Step III : Loop
	//===================================

	//	- 1. Tell what the agent perceive
	//-----------------------------------
	opack::perceive<Vision>(sim, arthur, artefact); // Arthur is now seeing the artefact.

	//	- 2. Advance simulation
	//-------------------------
	sim.step(); // advance simulation by one step
	sim.step(1.0f); // advance simulation by one step as if 1 second has passed
	sim.step_n(100); // advance simulation by 100 steps
	sim.step_n(100, 1.0f); // advance simulation by 100 steps, as if 1 seconds passed between each steps

	//	- 3. Retrieve actions
	//-------------------------
	sim.actions();
}
```

## Installation

> :warning: needs to be tested !
 
To use the library, include the library in your build system :

### CMake

We recommend to use [CPM](https://github.com/cpm-cmake/CPM.cmake)
```cmake
CPMAddPackage(
        NAME opack 
        GITHUB_REPOSITORY TBlauwe/OPACK
        OPTIONS
            "OPK_BUILD_APPS OFF"
            "OPK_BUILD_BENCHMARKS OFF"
            "OPK_BUILD_DOCS OFF"
            "OPK_BUILD_TESTS OFF"
)
```

#### CMake options

| Options              | Default  | Description                                              |
| -------------------- | -------- | -------------------------------------------------------- |
| OPK_BUILD_SAMPLES    | OFF      | Build several applications showcasing the library        |
| OPK_BUILD_BENCHMARKS | OFF      | Build benchmarks                                         |
| OPK_BUILD_DOCS       | OFF      | Build documentation                                      |
| OPK_BUILD_TESTS      | OFF      | Build tests                                              |

### Prerequisites

Each target will automatically install dependencies via [CPM](https://github.com/cpm-cmake/).

However, some dependencies needs to be downloaded manually (for some targets) :

#### Docs

> Only if `BUILD_DOCS ON` !

Install [doxygen](https://www.doxygen.nl/download.html)

If you are seeing this error :

```
error : Problems running epstopdf. Check your TeX installation!
```

Install a TeX distribution on your systems, see : https://www.latex-project.org/get/ .

## Credits

* **[FLECS](https://github.com/SanderMertens/flecs)**
* **[Doxygen-awesome](https://github.com/jothepro/doxygen-awesome-css)**