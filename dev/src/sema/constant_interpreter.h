#pragma once

#include <cstdint>
#include <memory>
#include <vector>

#include "sema/constant_frames.h"
#include "sema/constant_values.h"
#include "sema/semantic_graph.h"
#include "support/memory/arena.h"
#include "support/pointer_map.h"
#include "syntax/ast.h"

namespace sema {

class Analyzer;
class ConstantMemo;
struct Conversion;

// The constant evaluator of semantics mode (5.19, 7.1.5): it
// interprets typed expressions and constexpr function and constructor
// bodies over their recorded facts (the conversions, selected functions
// and constructor plans the analysis made), computing typed values
// (sema/constant_values.h). One interpreter serves one evaluation (a
// template argument, a static_assert condition, a variable's initializer);
// its objects, frames and scratch values live in its own arena, released
// with it. Beyond C++11 it evaluates the C++14 forms later code expects
// (automatic variables, assignment, loops and branches in constexpr
// bodies). Evaluation that is not a constant expression fails softly with
// a location and reason; the caller decides whether that is an error.
class ConstantInterpreter
{
public:
	explicit ConstantInterpreter(Analyzer& analyzer);
	// Its steps are added to the graph's constant_steps.
	~ConstantInterpreter();

	// The prvalue of a constant expression (a glvalue read, an array or a
	// function decayed); with `converted`, after the conversion its context
	// recorded (a converted constant expression, 5.19p3, or a contextual
	// conversion to bool).
	bool evaluate(const syntax::Expr& expr, bool converted, ConstValue& out);
	// Its scalar value after the given conversion (a conversion function's
	// result before its second standard conversion, see check_narrowing).
	bool evaluate_after(const syntax::Expr& expr, const Conversion* conversion, ConstValue& out);
	// A variable's initialization as a constant expression: the complete
	// object's value (a reference's referent), published to the semantic
	// arena; with `transient`, a non-reference's integer value that is not a
	// value-initialization's zero stays in the evaluation's storage (valid
	// while the interpreter lives).
	// With static_zero, trivial default-initialization preserves the zero
	// value provided by static storage.
	bool initialize(const VariableEntity* variable, const syntax::Initializer* initializer, const ConstValue*& out,
		bool transient = false, bool static_zero = false);

	uint32_t failure_location() const { return failure_location_; }
	const char* failure() const { return failure_; }
	// Whether the evaluation took a base class subobject's provisional
	// offset in a class still being defined (see base_offset): its result
	// is not that class's layout's until it is complete.
	bool met_incomplete_class() const { return incomplete_class_; }

	using Stacks = ConstantStacks;

private:
	using Object = ConstObject;
	using Local = ConstLocal;
	using Result = ConstResult;
	using Frame = ConstFrame;
	using Argument = ConstArgument;
	using Swap = ConstSwap;
	using Region = ConstRegion;
	enum class Flow : uint8_t
	{
		Next,
		Break,
		Continue,
		Return
	};

	// ------------------------------------------------ failure and budget
	bool fail(uint32_t location, const char* why);
	bool step(uint32_t location);
	bool stack_left(uint32_t location);
	// A walk of an object's subobjects nested past kSubobjectDepth levels
	// (a chain of member classes, as deep as the program declares them)
	// that the stack budget no longer holds.
	static const uint32_t kSubobjectDepth = 256;
	bool subobjects_too_deep(uint32_t depth) const;

	// -------------------------------------------- objects and places
	ConstValue skeleton(const Type* type);
	bool new_object(const Type* type, ConstPlace& place, uint32_t location);
	bool locate(const ConstPlace& place, bool write, ConstValue*& out, uint32_t location);
	bool locate_variant(const ConstPlace& place, ConstValue* storage, const Type* type, bool write, ConstValue*& out,
		uint32_t location);
	bool root_storage(const ConstPlace& place, bool write, ConstValue*& storage, const Type*& type, uint32_t location);
	bool root_type(const ConstPlace& place, const Type*& type, uint32_t location);
	bool same_literal(const ConstValue& a, const ConstValue& b);
	bool holds(const Type* type, int64_t offset, const Type* wanted, uint32_t depth);
	const DataMember* member_at(const ClassEntity* cls, int64_t offset, int64_t& start);
	int64_t member_extent(const DataMember* member);
	const SpanTable& span_table(const ClassEntity* cls);
	const DataMember* empty_member_at(
		const ClassEntity* cls, int64_t offset, const Type* wanted, int64_t& start, uint32_t depth);
	bool ordered(const Type* type, int64_t a, int64_t b, uint32_t depth);
	uint64_t leaves(const Type* type, uint32_t depth);
	ConstValue* descend(
		ConstValue* value, const Type* type, int64_t offset, const Type* wanted, bool write, uint32_t depth);
	const ConstValue* literal_object(const syntax::LiteralExpr* literal, uint32_t location);
	bool read(const ConstPlace& place, ConstValue& out, uint32_t location);
	bool write(const ConstPlace& place, const ConstValue& value, uint32_t location);
	bool valid_pointer(const ConstValue& value, uint32_t location);
	bool member_place(const ConstPlace& object, const VariableEntity* member, ConstPlace& out, uint32_t location);
	bool base_offset(const ClassEntity* derived, const ClassEntity* base, int64_t& offset, uint32_t location);
	bool base_search(const ClassEntity* derived, const ClassEntity* base, int64_t& offset, uint32_t location);
	bool to_base(ConstPlace& place, const ClassEntity* base, uint32_t location);
	bool to_derived(ConstPlace& place, const ClassEntity* derived, const Type* type, uint32_t location);
	// Pointers to members: a conversion between related classes' (4.11p2,
	// 5.2.9p12 with `to_base`), and the object of `.*` or `->*` as the
	// subobject of the member's class the pointer designates (5.5).
	bool member_adjusted(ConstValue& value, const Type* to, bool to_base, uint32_t location);
	bool through_naming(ConstPlace& object, const ClassEntity* naming, uint32_t location);
	bool member_pointer_object(const syntax::BinaryExpr& expr, ConstValue& pointer, ConstPlace& out);
	bool element_bounds(const ConstPlace& place, int64_t& first, int64_t& count, uint32_t location);
	bool advance(ConstValue& pointer, int64_t count, uint32_t location);
	bool array_of(const ConstPlace& place, int64_t size, int64_t& first, int64_t& bound, uint32_t location);
	uint64_t size_of(const Type* type, uint32_t location);
	void copy_into(ConstValue& target, const ConstValue& source);
	void zero_into(ConstValue& target);
	bool complete(const ConstValue& value) const;
	const ConstValue* publish(const ConstValue& value, uint32_t location);
	bool publishable(const ConstValue& value) const;
	bool static_variable_place(const ConstPlace& place) const;
	const ConstValue* provisional_value(const VariableEntity* variable);
	bool extend_temporary(ConstPlace& place, uint32_t location);

	// ---------------------------------------------------- expressions
	bool decay_reference(ConstPlace place, const Type* type, ConstValue& out, uint32_t location);
	bool rvalue(const syntax::Expr& expr, ConstValue& out);
	bool operand(const syntax::Expr& expr, ConstValue& out);
	bool converted(const syntax::Expr& expr, const Conversion* conversion, ConstValue& out);
	bool apply_standard(const syntax::Expr& expr, const Conversion* conversion, ConstValue& value);
	bool convert_scalar(ConstValue& value, const Type* to, uint32_t location);
	bool truth(const syntax::Expr& expr, bool& out);
	bool truth_of(const ConstValue& value, bool& out, uint32_t location);
	bool glvalue(const syntax::Expr& expr, ConstPlace& out);
	bool object_place(const syntax::Expr& expr, ConstPlace& out);
	bool discard(const syntax::Expr& expr);
	bool entity_place(const Entity* entity, const Type* type, ConstPlace& out, uint32_t location);
	bool this_place(ConstPlace& out, uint32_t location);
	bool literal(const syntax::LiteralExpr& expr, ConstValue& out);
	bool binary(const syntax::BinaryExpr& expr, ConstValue& out);
	bool arithmetic(const syntax::BinaryExpr& expr, const ConstValue& left, ConstValue& out);
	bool pointer_arithmetic(const syntax::BinaryExpr& expr, const ConstValue* left, ConstValue& out);
	bool comparison(const syntax::BinaryExpr& expr, const ConstValue& left, ConstValue& out);
	bool logical(const syntax::BinaryExpr& expr, ConstValue& out);
	bool assignment(const syntax::BinaryExpr& expr, ConstPlace& out);
	bool compound_assignment(const syntax::BinaryExpr& expr, ConstPlace& out);
	bool unary(const syntax::UnaryExpr& expr, ConstValue& out);
	bool increment(const syntax::UnaryExpr& expr, bool postfix, ConstPlace& place, ConstValue& old);
	bool conditional_value(const syntax::ConditionalExpr& expr, ConstValue& out);
	// A builtin the analysis expanded, of constant operands.
	bool intrinsic_value(const syntax::CallExpr& call_expr, ConstValue& out);
	// GCC's __builtin_bswap16/32/64 and clz/ctz/popcount[l|ll] of a
	// constant operand.
	bool bit_builtin(const syntax::CallExpr& expr, ConstValue& out);
	bool elementwise_minmax(const syntax::CallExpr& expr, ConstValue& out);
	// GCC's __builtin_add/sub/mul_overflow of constant operands, the
	// result stored through the third.
	bool overflow_builtin(const syntax::CallExpr& expr, ConstValue& out);
	// GCC's folded library builtins: strcmp and strncmp of constant
	// strings, copysign, fmax and fmin of constants; false for another
	// function (true with a failure for a call of one that is no constant).
	bool library_comparison(
		const FunctionEntity* function, const Argument* arguments, uint32_t count, Result& result, uint32_t location);
	bool library_floating(
		const FunctionEntity* function, const Argument* arguments, uint32_t count, Result& result, uint32_t location);
	// GCC's __builtin_FILE, __builtin_LINE and __builtin_FUNCTION, Clang's
	// __builtin_COLUMN (see Intrinsic::SourceFile).
	bool source_location(const syntax::CallExpr& expr, ConstValue& out);
	bool cast(const syntax::Expr& expr, const syntax::Expr& operand, ConstValue& out);
	bool subscript(const syntax::SubscriptExpr& expr, ConstPlace& out);
	bool member(const syntax::MemberExpr& expr, ConstPlace& out);
	bool member_object(const syntax::MemberExpr& expr);
	bool reference_cast(const syntax::Expr& expr, const syntax::Expr& operand, ConstPlace& out);
	bool bind(const syntax::Expr& expr, const Conversion* conversion, const Type* reference, ConstPlace& out);
	static bool binds_temporary(const syntax::Expr& expr, const Conversion* conversion);
	bool floating_operation(
		TokenKind op, const Type* type, long double left, long double right, long double& out, uint32_t location);
	// GNU complex values (sema/constant_complex.cpp): an Array value of the
	// complex type holding its parts in the evaluation's storage; one of a
	// list of the parts, converted, true, combined, compared, negated or
	// conjugated, stepped; a part of a prvalue or glvalue.
	bool complex_value(const Type* type, const ConstValue& real, const ConstValue& imaginary, ConstValue& out);
	bool part_list(const syntax::Slice<syntax::Expr*>& parts, const Type* type, ConstValue& out, uint32_t location);
	// GNU vector values (LaneVector): an Array value of the lanes, made of
	// given lanes and a fill; one of a lane list (each lane a constant); one
	// reinterpreted as another vector type of its size.
	bool vector_value(
		const Type* type, const ConstValue* lanes, uint32_t count, const ConstValue& fill, ConstValue& out);
	bool bit_cast(const syntax::BuiltinTraitExpr& expr, ConstValue& out);
	bool object_representation(const Type* type, ConstValue& value, uint8_t* bytes, uint8_t* known, uint64_t capacity,
		bool encode, uint32_t location, uint32_t depth);
	bool bit_field_representation(const DataMember& field, ConstValue& value, uint8_t* bytes, uint8_t* known,
		uint64_t capacity, bool encode, uint32_t location);
	bool vector_reinterpretation(ConstValue& value, const Type* to, uint32_t location);
	bool vector_lane(const syntax::SubscriptExpr& expr, ConstPlace* place, ConstValue* value);
	bool lane_list(const syntax::Slice<syntax::Expr*>& lanes, const Type* type, ConstValue& out, uint32_t location);
	bool complex_convert(ConstValue& value, const Type* to, uint32_t location);
	bool complex_truth(const ConstValue& value, bool& out, uint32_t location);
	bool part_operation(TokenKind op, const Type* part, ConstValue a, ConstValue b, ConstValue& out, uint32_t location);
	bool part_negation(const Type* part, ConstValue value, ConstValue& out, uint32_t location);
	bool complex_operation(TokenKind op, const Type* type, const ConstValue& left, const ConstValue& right,
		ConstValue& out, uint32_t location);
	bool complex_equal(bool equal, const ConstValue& left, const ConstValue& right, bool& out, uint32_t location);
	bool complex_unary(TokenKind op, const Type* type, const ConstValue& value, ConstValue& out, uint32_t location);
	bool complex_step(const Type* type, const ConstValue& old, bool decrement, ConstValue& out, uint32_t location);
	bool complex_part(const syntax::BuiltinTraitExpr& trait, ConstValue& out);
	bool complex_part_place(const syntax::BuiltinTraitExpr& trait, ConstPlace& out);

	// --------------------------------------------------- construction
	bool materialize(const syntax::Expr& expr, const ConstPlace& target);
	bool temporary(const syntax::Expr& expr, ConstPlace& out);
	bool initialize_from(const syntax::Expr& expr, const Conversion* conversion, const ConstPlace& target);
	bool construct(const Type* type, const ConstPlace& target, const syntax::Initializer* init,
		const FunctionEntity* constructor, uint32_t location);
	bool value_initialize(
		const Type* type, const ConstPlace& target, const FunctionEntity* constructor, uint32_t location);
	bool initialize_object(
		const Type* type, const ConstPlace& target, const syntax::Initializer& init, uint32_t location);
	bool initialize_scalar(
		const Type* type, const ConstPlace& target, const syntax::Expr* single, uint32_t location, bool extend = false);
	bool aggregate(const Type* type, const ConstPlace& target, const syntax::BracedInitList& list, uint32_t location);
	bool list_object(
		const ClassEntity* entity, const ConstPlace& target, const syntax::BracedInitList& list, uint32_t location);
	bool aggregate_members(const ClassEntity* entity, const ConstPlace& target, const syntax::BracedInitList& list,
		uint32_t& next, uint32_t location);
	bool aggregate_element(const Type* type, const ConstPlace& target, const syntax::BracedInitList& list,
		uint32_t& next, uint32_t location);
	bool string_array(const Type* type, const ConstPlace& target, const syntax::Expr& literal, uint32_t location);
	bool constructor_call(const FunctionEntity* constructor, const ConstPlace& target,
		const syntax::Slice<syntax::Expr*>& written, uint32_t location);
	bool run_constructor(const FunctionEntity* constructor, const ConstPlace& target, const Argument* arguments,
		uint32_t count, uint32_t location);
	bool argument_place(const Argument& argument, const Type* parameter, ConstPlace& out, uint32_t location);
	bool copy_subobjects(
		const FunctionEntity* function, const ConstPlace& target, const ConstPlace& source, uint32_t location);
	bool subobjects(const FunctionEntity* constructor, const ConstPlace& self, uint32_t location);
	bool member_initialization(const SubobjectInit& init, const ConstPlace& self, uint32_t location);
	bool member_slot(const ConstPlace& object, const VariableEntity* member, ConstPlace& out, uint32_t location);

	// ------------------------------------------------ calls and bodies
	bool call(const syntax::CallExpr& expr, Result& result);
	bool operator_call(const syntax::Expr& expr, Result& result);
	bool user_literal(const syntax::LiteralExpr& expr, Result& result);
	bool conversion_call(
		const syntax::Expr& expr, const ExprFacts* facts, const FunctionEntity* function, Result& result);
	bool call_value(const syntax::Expr& expr, ConstValue& out);
	bool call_place(const syntax::Expr& expr, ConstPlace& out);
	bool invoke(const FunctionEntity* function, const ConstPlace* object, const Argument* arguments, uint32_t count,
		Result& result, uint32_t location);
	bool builtin(
		const FunctionEntity* function, const Argument* arguments, uint32_t count, Result& result, uint32_t location);
	bool bind_parameters(const FunctionEntity* function, const Argument* arguments, uint32_t count, uint32_t location);
	bool evaluable(const FunctionEntity* function, uint32_t location);
	bool body(const FunctionEntity* function, Result& result, uint32_t location);
	bool invoke_body(const FunctionEntity* function, const ConstPlace* object, size_t first_local, Result& result,
		uint32_t location);
	bool reuse_call(uint64_t steps, size_t depth, uint32_t location);
	size_t suspend_swaps();
	void resume_swaps(size_t active);
	void end_parameters(const FunctionEntity* function, size_t first);
	bool execute(const syntax::Stmt& stmt, Flow& flow);
	bool compound(const syntax::CompoundStmt& block, Flow& flow);
	bool declaration(const syntax::Decl& decl);
	bool local_variable(const VariableEntity* variable, const syntax::Initializer* initializer, uint32_t location);
	bool condition(const syntax::Condition& condition, bool& out, uint32_t location);
	bool loop_body(const syntax::Stmt& body, Flow& flow, bool& leave);
	bool range_for(const syntax::RangeForStmt& stmt, Flow& flow);
	bool switch_statement(const syntax::ConditionalLoopStmt& stmt, Flow& flow);
	bool returned(const syntax::ValueStmt& stmt);
	bool local(const Entity* entity, ConstPlace& out) const;
	Region region() const;
	void release(const Region& region);

	Analyzer& analyzer_;
	// The evaluation's values (Analyzer::constant_scratch_ from `mark_`,
	// rewound as regions end and when the evaluation ends; results are
	// published).
	Arena& arena_;
	Arena::Mark mark_;
	// The analyzer's stacks (Analyzer::constant_stacks_) from where this
	// evaluation began: an object's slot is its index.
	Stacks& stacks_;
	std::vector<Object>& objects_;
	std::vector<Local>& locals_;
	std::vector<Frame>& frames_;
	const size_t first_object_;
	const size_t first_local_;
	const size_t first_frame_;
	// The swaps an enclosing evaluation had applied (resumed at the end).
	const size_t enclosing_swaps_;
	uint64_t steps_;
	std::unique_ptr<ConstantMemo> memo_;
	uint32_t failure_location_;
	const char* failure_;
	// The variable whose initialization is being evaluated, and the value
	// its object is given (it designates itself while being initialized).
	const VariableEntity* constructing_;
	ConstValue* constructing_value_;
	// Switch statements seeking their label: the label to resume at.
	const syntax::Stmt* seeking_;
	// The outermost default argument (or constructor's member
	// initializer) being evaluated: the location the source location
	// builtins report (kNoSourceSite: their own).
	static const uint32_t kNoSourceSite = 0xffffffffu;
	uint32_t source_site_;
	// Sets the site for its lifetime unless one is set (or `site` is
	// kNoSourceSite).
	struct SourceSite
	{
		SourceSite(uint32_t& current, uint32_t site) : current_(current), saved_(current)
		{
			if (current == kNoSourceSite)
				current = site;
		}
		~SourceSite() { current_ = saved_; }
		uint32_t& current_;
		const uint32_t saved_;
	};
	bool incomplete_class_;
	// GNU constant_p conditionals permit foldable reference casts in the
	// selected expression. A called function starts its own strict context.
	bool folding_ = false;
	struct Folding
	{
		Folding(bool& current, bool enabled) : current_(current), saved_(current) { current_ = enabled; }
		~Folding() { current_ = saved_; }
		bool& current_;
		const bool saved_;
	};
	// base_search's classes found not to lead to the base sought (per
	// query).
	PointerMap base_misses_;
};

} // namespace sema
