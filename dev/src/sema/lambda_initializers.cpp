#include "sema/analyzer.h"

namespace sema {

void Analyzer::initialized_capture(LambdaFrame& frame, const syntax::LambdaCapture& capture, Scope* outer, Scope* inner)
{
	const uint32_t location = capture.initializer->location;
	if (!cxx14_)
		error(location, "initialized lambda captures require C++14");
	if (capture.pack)
		error(location, "an initialized capture pack requires a later language dialect");
	// Initializers name the enclosing scope, independently of the new names
	// introduced for the body. In particular, [x = x] reads the outer x.
	const Type* saved_deduced = auto_deduced_;
	auto_deduced_ = nullptr;
	Specifiers specifiers = Specifiers();
	specifiers.type = auto_placeholder_;
	specifiers.auto_type = true;
	const Type* declared = capture.by_reference ? types_.reference(auto_placeholder_, false) : auto_placeholder_;
	const uint32_t throwing = potentially_throwing_;
	syntax::Initializer deduction = *capture.initializer;
	if (deduction.kind == syntax::InitializerKind::Braced)
	{
		const syntax::BracedInitList& list = static_cast<const syntax::BracedInitList&>(*deduction.expression);
		if (list.elements.size != 1)
			error(location, "a directly initialized capture requires one expression");
		deduction.kind = syntax::InitializerKind::Assign;
		deduction.expression = list.elements[0];
	}
	const Type* type = deduced_type(specifiers, declared, &deduction, outer, location);
	auto_deduced_ = saved_deduced;
	VariableEntity* variable = new_entity<VariableEntity>(EntityKind::Variable, capture.name, inner, type);
	variable->defined = true;
	variable->init_capture = true;
	variable->member_initializer = capture.initializer;
	if (semantic())
		initialize_variable(variable, type, capture.initializer, outer, location);
	else
		initializer_names(capture.initializer, outer, ClassOf(type) != nullptr);
	const syntax::Expr* expression = capture.initializer->expression;
	variable->throwing_initializer = potentially_throwing_ != throwing ||
		(expression != nullptr && may_throw(*expression, 0));
	declare_in(inner, new_binding(BindingKind::Variable, capture.name, variable, type));
	add_capture(frame, variable, true, !capture.by_reference, location);
}

} // namespace sema
