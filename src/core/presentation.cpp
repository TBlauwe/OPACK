#include <opack/core.hpp>

using namespace opack;

int main()
{

auto sim = Simulation();
sim.step(); // Progress by one cycle
sim.step_n(10); // Progress 10 times
sim.step(1.0f); // Progress once as if 1 second has passed.

struct Stress : {float value; };
struct Introvert : {};
auto agent = agent(sim);
agent.add<Stress>();
agent.add<Introvert>();

struct MySense : opack::Sense{};
struct X : {/*...*/};   struct Y : {/*...*/}; struct Act : {/*...*/};   
opack::reg<MySense>(sim); opack::perceive<MySense, X, Y, Act>(sim);
/* Cas 1 */ opack::perceive<MySense>(sim, a, b);
opack::does_perceive<Sense, X>(sim, a, b) // return true
opack::does_perceive<Sense, Y>(sim, a, b) // return false

struct Towards {};
struct Walk : opack::Action {};  struct UseLegs : opack::Actuator {};
opack::reg<Help, UseLegs>(sim);
auto walk_inst = opack::action<Walk>(sim).add<Towards>(b);
opack::act<UseLegs>(a, walk_inst);

}