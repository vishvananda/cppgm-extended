#pragma once

#include <vector>

#include "sema/analyzer.h"

// The analysis state an instantiation sets aside (sema/class_templates.cpp
// and the other instantiating files use it).
namespace sema {

// The scratch of the analysis that an instantiation interrupts and that
// its users clear rather than extend from its size.
struct Analyzer::InstantiationScratch
{
	std::vector<Candidate> candidates;
	std::vector<FunctionEntity*> overload_set;
	std::vector<Conversion> sequences;
	std::vector<Conversion> argument_scratch;
	std::vector<FunctionEntity*> operator_set;
	std::vector<const Type*> operand_options;
	std::vector<const FunctionEntity*> converter_scratch;
	std::vector<FunctionEntity*> adl_functions;
	std::vector<Scope*> adl_namespaces;
	std::vector<ClassEntity*> adl_enclosing;
	std::vector<ClassEntity*> adl_pending;
	std::vector<ClassEntity*> adl_classes;
	std::vector<const syntax::BinaryExpr*> binary_spine;
};

// Instantiation runs in the middle of other analysis (a class completed for
// an expression, a declaration or an overload candidate): the contextual
// state and the non-reentrant scratch of the analysis around it are set
// aside, and restored however the instantiation ends. The instantiation
// analyzes with its nesting level's scratch from the analyzer's pool.
class Analyzer::InstantiationContext
{
public:
	// `depth`: the nesting of the instantiation entered (checked by
	// enter_instantiation before any state changes). `substitution`: part
	// of the substitution under way (a placeholder's expression, a default
	// template argument), whose failure, where it is a result, stays one
	// and is left recorded for the substitution.
	// `limit`: the declaration ordinal of the definition context of what
	// it instantiates, whose later namespace-scope names its lookups do not
	// see (14.6.4.2); none by default.
	InstantiationContext(
		Analyzer& analyzer, uint32_t depth, bool substitution = false, uint32_t limit = NameLookup::kNoLimit);
	~InstantiationContext();

private:
	void swap_scratch();

	Analyzer& a_;
	uint32_t limit_;
	uint64_t dependent_uses_;
	uint32_t instantiation_depth_;
	BodyContext* body_;
	uint32_t class_depth_;
	uint32_t template_depth_;
	const syntax::TemplateParameterList* template_list_;
	Scope* template_scope_;
	ClassEntity* template_owner_;
	bool owner_specifiers_;
	Scope* placeholder_home_;
	const Scope* namespace_friend_class_;
	const Scope* access_point_;
	const Scope* access_scope_;
	bool suppress_implicit_;
	TokenKind current_access_;
	Scope* member_scope_;
	Scope* expression_scope_;
	const ClassEntity* naming_class_;
	NamePrefix last_prefix_;
	Scope* declaration_class_;
	Scope* access_deferral_;
	const ClassEntity* base_object_;
	bool c_language_;
	bool linkage_extern_;
	bool linkage_specified_;
	PendingPrototype pending_prototype_;
	FunctionEntity* demanding_;
	uint32_t current_epoch_;
	const syntax::Initializer* initializing_;
	uint32_t unevaluated_;
	uint32_t deferring_;
	uint32_t deferring_reserved_;
	uint32_t template_arguments_;
	bool callee_name_;
	uint32_t soft_substitution_;
	SubstitutionFailure failure_;
	bool substitution_;
	const ClassEntity* direct_class_;
	bool standard_only_;
	bool explicit_specialization_;
	bool lexical_limit_;
	InstantiationScratch* scratch_;
};

// A substitution whose failure is a result (see soft_substitution_): a
// failure recorded while it runs is its own, consumed when it ends; one
// recorded around it before is kept.
class Analyzer::SoftSubstitution
{
public:
	explicit SoftSubstitution(Analyzer& analyzer) : a_(analyzer), outer_(analyzer.failure_)
	{
		++a_.soft_substitution_;
		a_.failure_ = SubstitutionFailure();
	}
	~SoftSubstitution()
	{
		if (a_.failed())
			++a_.graph_.substitution_failures;
		--a_.soft_substitution_;
		a_.failure_ = outer_;
	}
	bool failed() const { return a_.failed(); }

private:
	Analyzer& a_;
	SubstitutionFailure outer_;
};

} // namespace sema
