#include <opack/core/perception.hpp>

opack::queries::perception::Component::Component(flecs::world& world)
	: rule
{
	world.rule_builder()
	.expr("$Sense($Observer, $Subject), $Predicate($Subject)")
	.term<opack::Sense>().subj().var("Sense").obj().var("Predicate")
	.build()
},
observer_var{ rule.rule.find_var("Observer") },
sense_var{ rule.rule.find_var("Sense") },
subject_var{ rule.rule.find_var("Subject") },
predicate_var{ rule.rule.find_var("Predicate") }
{};

opack::queries::perception::Relation::Relation(flecs::world& world)
	: rule
{
	world.rule_builder()
	.expr("$Sense($Observer, $Subject), $Predicate($Subject, $Object)")
	.term<opack::Sense>().subj().var("Sense").obj().var("Predicate")
	.build()
},
observer_var{ rule.rule.find_var("Observer") },
sense_var{ rule.rule.find_var("Sense") },
subject_var{ rule.rule.find_var("Subject") },
predicate_var{ rule.rule.find_var("Predicate") },
object_var{ rule.rule.find_var("Object") }
{};

