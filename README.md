![wip](https://img.shields.io/badge/-WIP-red)

# Opack

> :warning: Project not ready yet !

__Opack__ is a C++ library to simulate a world with a variety of agents, _i.e reactive to cognitive agents_.
Thanks to the underlying library __[Flecs](https://github.com/SanderMertens/flecs)__, agents, and the world,
are composable according to your needs.

__Opack__ is a meta-model that help you build __agent models__ tailored to your needs.
An agent model can be fixed for a whole simulation, _e.g a reactive agent model_. Or, an agent
model can be modulated during the simulation, _e.g switched from a reactive agent model when in combat 
to a cognitive agent when in conversation._

Here is a minimal example :

> If you are coming from __[Flecs](https://github.com/SanderMertens/flecs)__, you can import __Opack__ into
your world.


```cpp
#include <opack/core.hpp>					// Include core library features
#include <opack/module/simple_agent.hpp>	// For starter

int main()
{
	auto world = opack::create_world();		// A world is a container of all ECS data.
											// Here we return a world, with Opack already imported.
    // -- or --
    //flecs::world world;                   // If you already have a world,
    //opack::import(world)                  // you can import Opack like so.

	world.import<simple>()					// Import "simple" module into the world.
											// It will add necessary components / "concepts".

	// Spawn three entities that are "simple" agents.
	auto agent_1 = opack::spawn<simple::Agent>(world);  
	auto agent_2 = opack::spawn<simple::Agent>(world);  
	auto agent_3 = opack::spawn<simple::Agent>(world);  

    opack::run(world);
    // or 
    //world.app().run();
}
```

This example does nothing by itself. A __world__ needs to be populated with capabilities and concepts, _e.g, position, factions, ennemies, allies, etc.,_
that you would typically define when building your game. It will form an ontology on which your agent will be able to reason.

Here, we just spawned `simple::Agent`, which has one actuator `simple::Actuator` and one sense `simple::Sense`.

First, let's see what is an agent :

An agent is defined as an entity :
* with __characteristics__, _e.g stress, personality traits, mood, etc,
* with __knowledge__ about the world, _e.g other entities, activities, etc.

An agent can :
* __perceive__ the world through __senses__,

```cpp
// Let's say agent_1 perceive agent_3 with simple::Sense
opack::perceive<simple::Sense>(agent_1, agent_3);

auto p = opack::Perception(agent_1)
p.does_perceive(agent_3); //return true
p.does_perceive(agent_2); //return false
p.each<simple::Agent>([](flecs::entity subject){/*...*/}); 
```

* __reason__ on the world through __operations flows__,
* __act__ on the world through actuators. Only one action per actuator
```cpp
// Let's say w perceive agent_3 with simple::Sense
OPACK_ACTION(MyAction) // Here we defined an identifier for an action
                       // equivalent to `struct MyAction : opack::Action {};`

              
opack::init<MyAction>(world)        // Initialize `MyAction` so we can customize it,
    .require<simple::Actuator>()    // like by telling which actuator is necessary for it to be executed.
    .on_begin([](flecs::entity action){/*...*/})    // What to do when it first begins, ...
    .on_update([](flecs::entity action){/*...*/})   // ... when it updates (for continuous actions) ...
    .on_end([](flecs::entity action){/*...*/})      // ... when it stops ...
    .on_cancel([](flecs::entity action){/*...*/})   // ... and when it's cancelled.
    ;

auto action = opack::spawn<MyAction>(world)     // Let's spawn an action, if we need to customize it 
                                                // for this particular instance.

opack::act(agent_1, action)                     
world.progress()                // Here, since it's a ponctual action (no duration)
                                // on_begin, on_update and on_end are called.
action.is_valid()               // False, since the action is finished it's destroyed.
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
			"OPACK_BUILD_EXAMPLES       OFF"
			"OPACK_BUILD_TESTS          OFF"
			"OPACK_BUILD_BENCHMARKS     OFF"
			"OPACK_DEVELOPPER_WARNINGS  OFF"
			"OPACK_ORGANIZE             ON"
			"OPACK_ASSERTS              ON"
)
```


#### CMake options

| Options                    | Default  | Description                                              |
| -------------------------- | -------- | -------------------------------------------------------- |
| OPACK_BUILD_EXAMPLES       | OFF      | "Build examples." |
| OPACK_BUILD_TESTS          | OFF      | "Build tests using doctest." |
| OPACK_BUILD_BENCHMARKS     | OFF      | "Build benchmarks using google benchmarks." |
| OPACK_DEVELOPPER_WARNINGS  | OFF      | "Enable more warnings when compiling" |
| OPACK_ORGANIZE             | ON       | "Enable organisation of entities, different from C++ namespace (mainly organisation for explorer). Disabling it may lead to more performance." |
| OPACK_ASSERTS              | ON       | "Enable assertions. Disabling it may lead to more performance." |


### Prerequisites
Each target will automatically install dependencies via [CPM](https://github.com/cpm-cmake/).

However, some dependencies needs to be downloaded manually (for some targets) :

#### Docs

> Only if you are building target `OPACK_DOCS` !

Install [doxygen](https://www.doxygen.nl/download.html)

If you are seeing this error :

```
error : Problems running epstopdf. Check your TeX installation!
```

Install a TeX distribution on your systems, see : https://www.latex-project.org/get/ .

## Benchmarks

The library used for benchmarking is [Google benchmark](https://github.com/google/benchmark).
After building the target, you can find them in the folder `bin\benchmarks`.
To run a benchmark, simply launch the .exe (e.g on windows):
> bin/benchmarks/opack_benchmarks_{$config}_{$compiler}.exe

However, if you want to pass more options to tune the benchmarking, see 
[Google benchmark usage guide](https://github.com/google/benchmark/blob/main/docs/user_guide.md).

Alternitavely, you can use `bin\benchmarks\run_benchmarks.py` python script, to run benchmarks with a predefined set of options.

> py .\run_benchmarks.py .\opack_benchmarks_{$config}_{$compiler}.exe -n Release

This line will generate a json file with 'Release' in its name. It will also repeat benchmarks 10 times and compute the mean, median, variance, etc.

## Credits

* **[Doctest](https://github.com/doctest/doctest)**
* **[Doxygen-awesome](https://github.com/jothepro/doxygen-awesome-css)**
* **[Flecs](https://github.com/SanderMertens/flecs)**
* **[fmt](https://github.com/fmtlib/fmt)**
* **[Google Benchmark](https://github.com/google/benchmark)**