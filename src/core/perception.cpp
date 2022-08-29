#include <opack/core/perception.hpp>

opack::queries::perception::Entity::Entity(flecs::world& world)
	: internal::Rule
{ 

	world.rule_builder()
    .term(flecs::IsA).second<opack::Tangible>()
    .term().first().var("This").src().var("Sensor")
    .term(flecs::IsA).src().var("Observer").second<opack::Tangible>()
    .term().first().var("Sense").src().var("Observer").second().var("Sensor")
	.build()
},
observer_var{ rule.find_var("Observer") },
sense_var{ rule.find_var("Sense") }
{};

opack::queries::perception::Component::Component(flecs::world& world)
	: internal::Rule
{
	world.rule_builder()
	.expr("$Sense($Observer, $Subject), $Predicate($Subject)")
	.term<opack::Sense>().src().var("Sense").second().var("Predicate")
	.build()
},
observer_var{ rule.find_var("Observer") },
sense_var{ rule.find_var("Sense") },
subject_var{ rule.find_var("Subject") },
predicate_var{ rule.find_var("Predicate") }
{};

opack::queries::perception::Relation::Relation(flecs::world& world)
	: internal::Rule
{
	world.rule_builder()
	.expr("$Sense($Observer, $Subject), $Predicate($Subject, $Object)")
	.term<opack::Sense>().src().var("Sense").second().var("Predicate")
	.build()
},
observer_var{ rule.find_var("Observer") },
sense_var{ rule.find_var("Sense") },
subject_var{ rule.find_var("Subject") },
predicate_var{ rule.find_var("Predicate") },
object_var{ rule.find_var("Object") }
{};

