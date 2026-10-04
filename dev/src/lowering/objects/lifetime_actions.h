#ifndef CPPGM_LOWERING_LIFETIME_ACTIONS_H
#define CPPGM_LOWERING_LIFETIME_ACTIONS_H

#include "lowering/support/identity_maps.h"
#include "lowering/support/errors.h"
#include "lowering/support/sequences.h"
#include "lowering/ir/model.h"
#include "semantic/model/graph.h"
#include "lowering/objects/cleanup_continuations.h"
#include "lowering/objects/construction_cleanup.h"

#include <cstdint>

namespace cppgm
{
namespace lowering
{

using namespace semantic;
using namespace semantic;
using namespace lowering::ir;
using namespace lowering::support;

const std::size_t kDestructorCleanupInlineLimit = 8;

template <class Derived>
class LifetimeActionLowering
{
protected:
	LifetimeActionLowering()
		: direct_return_slot_(kNoLowId), shared_return_slot_(kNoLowId), lexical_cleanup_terminate_(kNoLowId), lexical_body_unwind_target_(kNoLowId),
		  lexical_body_unwind_depth_(0) {}

	void ResetLifetimeFunctionState()
	{
		direct_return_slot_ = kNoLowId;
		shared_return_slot_ = kNoLowId;
		lexical_cleanup_terminate_ = kNoLowId;
		lexical_body_unwind_target_ = kNoLowId;
		lexical_body_unwind_depth_ = 0;
		return_cleanup_roots_.Clear();
		return_cleanup_counts_.clear();
	}

	std::size_t LexicalDestructorDepth(const DumpNode& action) const
	{
		const Derived& derived = static_cast<const Derived&>(*this);
		return action.lexical_cleanup_plan == 0 ? 0 :
			derived.arena_.lexical_cleanup_plans[action.lexical_cleanup_plan - 1].depth;
	}

	BlockId LexicalCleanupTerminateBlock()
	{
		if (lexical_cleanup_terminate_ == kNoLowId)
			lexical_cleanup_terminate_ = MakeCleanupTerminateBlock();
		return lexical_cleanup_terminate_;
	}

	BlockId MakeCleanupTerminateBlock()
	{
		Derived& derived = static_cast<Derived&>(*this);
		const BlockId original = derived.current_block_;
		const BlockId terminate = derived.AddBlock(derived.NewLabel("lexical_cleanup_terminate"));
		derived.SelectBlock(terminate);
		Instruction clause(Instruction::EH_CATCH_ALL);
		clause.first = Operand(derived.AllocateExceptionHandlerSelector(), LowI32());
		derived.Emit(clause);
		const Operand exception = derived.Temp(LowPtr());
		Instruction read(Instruction::EXCEPTION);
		read.dest = exception.id;
		read.type = LowPtr();
		derived.Emit(read);
		CallArguments arguments;
		arguments.Push(exception);
		(void)derived.EmitExceptionRuntimeCall(derived.output_.terminate_helper_symbol,
			LowVoid(), arguments);
		derived.Emit(Instruction(Instruction::UNREACHABLE));
		derived.SelectBlock(original);
		return terminate;
	}

	void LowerUnwindDestructorAction(const DumpNode& action)
	{
		Derived& derived = static_cast<Derived&>(*this);
		const bool may_throw = !derived.program_.bindings[action.binding].nonthrowing;
		if (may_throw) derived.EmitEhTarget(Instruction::EH_TRY, MakeCleanupTerminateBlock());
		derived.LowerDestructorAction(action, true);
		if (may_throw) derived.Emit(Instruction(Instruction::EH_END));
	}

	BlockId LexicalUnwindCleanup(std::uint32_t root, std::size_t depth)
	{
		Derived& derived = static_cast<Derived&>(*this);
		using namespace lowering::cleanup;
		const std::uint32_t context = derived.ExceptionCleanupContextAtDepth(depth);
		const BlockId original = derived.current_block_;
		std::vector<std::uint32_t> pending;
		BlockId tail = kNoLowId;
		while (tail == kNoLowId)
		{
			bool inserted = false;
			const std::uint32_t cache = derived.cleanup_continuations_.Intern(
				Key(root, kNoCleanupState, lexical_body_unwind_target_ + 1, context, LEXICAL_UNWIND_CACHE), &inserted);
			const State& known = derived.cleanup_continuations_.Get(cache);
			if (known.block_bound) { tail = known.block; break; }
			const BlockId block = derived.AddBlock(derived.NewLabel("lexical_unwind"));
			derived.cleanup_continuations_.BindBlock(cache, block);
			pending.push_back(cache);
			if (root == 0) break;
			const LexicalCleanupPlan& plan = derived.arena_.lexical_cleanup_plans[root - 1];
			if (plan.kind == LEXICAL_CLEANUP_TRY_EXIT) break;
			root = plan.tail;
		}
		for (std::size_t i = pending.size(); i != 0; --i)
		{
			const State state = derived.cleanup_continuations_.Get(pending[i - 1]);
			derived.SelectBlock(state.block);
			root = state.key.action;
			if (root == 0)
			{
				if (lexical_body_unwind_target_ != kNoLowId)
				{
					derived.Emit(Instruction(Instruction::EH_END));
					derived.EmitJump(lexical_body_unwind_target_);
				}
				else derived.EmitExceptionResume();
			}
			else
			{
				const LexicalCleanupPlan& plan = derived.arena_.lexical_cleanup_plans[root - 1];
				if (plan.kind == LEXICAL_CLEANUP_TRY_EXIT)
					derived.EmitLexicalCleanupTryExit(plan.node, depth);
				else
				{
					if (plan.kind == LEXICAL_CLEANUP_HANDLER_EXIT)
						derived.EmitLexicalCleanupRegionExit(plan.depth - 1);
					else
					{
						const DumpNode& action = derived.arena_.nodes[plan.node];
						const bool may_throw = !derived.program_.bindings[action.binding].nonthrowing;
						if (may_throw)
							derived.EmitEhTarget(Instruction::EH_TRY, LexicalCleanupTerminateBlock());
						derived.LowerDestructorAction(action);
						if (may_throw) derived.Emit(Instruction(Instruction::EH_END));
					}
					derived.EmitJump(tail);
				}
			}
			tail = state.block;
		}
		derived.SelectBlock(original);
		return tail;
	}

	void LowerLexicalDestructorAction(const DumpNode& action, std::size_t* closed = 0,
		const ConstructionCleanupStep* returned = 0)
	{
		Derived& derived = static_cast<Derived&>(*this);
		if (action.lexical_cleanup_plan == 0 && !returned)
		{
			derived.LowerDestructorAction(action);
			return;
		}
		const std::uint32_t tail = action.lexical_cleanup_plan == 0 ? 0 :
			derived.arena_.lexical_cleanup_plans[action.lexical_cleanup_plan - 1].tail;
		const std::size_t depth = LexicalDestructorDepth(action);
		const std::size_t active = derived.ActiveExceptionRegionCount();
		if (depth > active) ThrowLoweringInternal("lexical destructor context exceeds active regions");
		if (closed)
		{
			derived.FinishExceptionControlExit(*closed, active - depth);
			*closed = active - depth;
		}
		if (derived.program_.bindings[action.binding].nonthrowing)
		{
			derived.LowerDestructorAction(action);
			return;
		}
		// A bare try boundary already dispatches this call. Only remaining
		// objects, handler exits, or a body cleanup need an additional tail.
		const bool body_tail = lexical_body_unwind_target_ != kNoLowId &&
			depth == lexical_body_unwind_depth_;
		if (!returned && !body_tail && (tail == 0 ||
			derived.arena_.lexical_cleanup_plans[tail - 1].kind ==
				LEXICAL_CLEANUP_TRY_EXIT))
		{
			derived.LowerDestructorAction(action);
			return;
		}
		const BlockId original = derived.current_block_;
		const BlockId unwind = LexicalUnwindCleanup(tail, depth);
		const BlockId dispatch = derived.AddBlock(derived.NewLabel("lexical_cleanup_dispatch"));
		derived.SelectBlock(dispatch);
		derived.EmitLexicalCleanupClauses(depth);
		derived.Emit(Instruction(Instruction::EH_CLEANUP));
		derived.Emit(Instruction(Instruction::EH_END));
		if (returned)
		{
			const bool may_throw = !derived.program_.bindings[returned->destructor].nonthrowing;
			if (may_throw) derived.EmitEhTarget(Instruction::EH_TRY, LexicalCleanupTerminateBlock());
			derived.LowerDestructorObject(returned->type, returned->destination,
				returned->destructor, false, true);
			if (may_throw) derived.Emit(Instruction(Instruction::EH_END));
		}
		derived.EmitJump(unwind);
		derived.SelectBlock(original);
		derived.EmitEhTarget(Instruction::EH_TRY, dispatch);
		derived.LowerDestructorAction(action);
		derived.Emit(Instruction(Instruction::EH_END));
	}

	std::size_t ReturnLexicalCleanupStart(const NodeChildren& children,
		bool has_value) const
	{
		const Derived& derived = static_cast<const Derived&>(*this);
		std::size_t first = has_value ? 1 : 0;
		while (first < children.size())
		{
			const DumpNode& action = derived.arena_.nodes[children[first]];
			if (action.kind != DUMP_DESTRUCTOR_ACTION ||
				!action.full_expression_staging) break;
			++first;
		}
		return first;
	}

	bool ReturnCleanupActionApplies(const NodeChildren& children,
		bool has_value, std::size_t index) const
	{
		const Derived& derived = static_cast<const Derived&>(*this);
		if (!has_value || !derived.arena_.nodes[children[0]].direct_return_slot)
			return true;
		return derived.arena_.nodes[children[index]].object_binding !=
			derived.arena_.nodes[children[0]].binding;
	}

	void PlanLexicalReturnCleanup(std::uint32_t node,
		const NodeChildren& children, std::uint32_t syntax_context)
	{
		Derived& derived = static_cast<Derived&>(*this);
		using namespace lowering::cleanup;
		const bool has_value = !children.empty() &&
			derived.arena_.nodes[children[0]].kind != DUMP_DESTRUCTOR_ACTION;
		const std::size_t first = ReturnLexicalCleanupStart(children, has_value);
		bool inserted = false;
		std::uint32_t tail = derived.cleanup_continuations_.Intern(Key(
			kNoCleanupState, kNoCleanupState, has_value ? 1 : 0,
			syntax_context, LEXICAL_RETURN_CENSUS), &inserted);
		std::size_t actions = 0;
		for (std::size_t i = children.size(); i != first; --i)
		{
			const std::size_t index = i - 1;
			if (derived.arena_.nodes[children[index]].kind !=
				DUMP_DESTRUCTOR_ACTION)
				ThrowLoweringInternal("invalid planned return cleanup action");
			if (!ReturnCleanupActionApplies(children, has_value, index)) continue;
			const ActionKey key = MakeActionKey(
				derived.arena_.nodes[children[index]]);
			const std::uint32_t action = derived.cleanup_continuations_.InternAction(
				key, children[index], &inserted);
			tail = derived.cleanup_continuations_.Intern(Key(action, tail, 0,
				syntax_context, LEXICAL_RETURN_CENSUS), &inserted);
			++actions;
		}
		if (actions == 0) return;
		return_cleanup_roots_.Insert(node, tail);
		if (tail >= return_cleanup_counts_.size())
			return_cleanup_counts_.resize(static_cast<std::size_t>(tail) + 1, 0);
		++return_cleanup_counts_[tail];
	}

	void FinalizeLexicalReturnCleanupPlan()
	{
		Derived& derived = static_cast<Derived&>(*this);
		if (derived.current_indirect_result_ ||
			derived.current_result_.kind == LOW_VOID ||
			derived.current_result_.kind == LOW_OBJECT) return;
		for (std::size_t i = 0; i < return_cleanup_counts_.size(); ++i)
			if (return_cleanup_counts_[i] > 1)
			{
				shared_return_slot_ = derived.CreateGeneratedSlot(
					"cleanup_return", derived.current_result_);
				return;
			}
	}

	bool ShouldShareLexicalReturn(std::uint32_t node) const
	{
		std::uint32_t root = kNoLowId;
		return return_cleanup_roots_.Find(node, &root) &&
			root < return_cleanup_counts_.size() &&
			return_cleanup_counts_[root] > 1;
	}

	SlotId EnsureDirectReturnSlot(std::uint32_t node)
	{
		Derived& derived = static_cast<Derived&>(*this);
		if (direct_return_slot_ == kNoLowId)
			direct_return_slot_ = derived.EnsureGeneratedSlot(
				node, "retobj", derived.current_result_);
		else if (derived.generated_slots_[node] == kNoLowId)
		{
			derived.generated_slots_[node] = direct_return_slot_;
			derived.generated_slot_nodes_.push_back(node);
		}
		return direct_return_slot_;
	}

	Operand ZeroDirectReturnObject(std::uint32_t node, const LowType& type)
	{
		Derived& derived = static_cast<Derived&>(*this);
		const Operand slot(EnsureDirectReturnSlot(node), type);
		Instruction zero(Instruction::ZERO_OBJECT);
		zero.type = type;
		zero.first = slot;
		derived.Emit(zero);
		return slot;
	}

	void LowerConstructorReturn(std::uint32_t node, Operand* result_value)
	{
		Derived& derived = static_cast<Derived&>(*this);
		const DumpNode& action = derived.arena_.nodes[node];
		if (derived.current_indirect_result_)
		{
			const Operand destination(static_cast<ParameterId>(0), LowPtr());
			if (action.value_initialization)
				derived.EmitZeroInitialization(action.operand_type, destination);
			derived.LowerConstructorAction(node, destination);
			return;
		}
		if (derived.current_result_.kind != LOW_OBJECT)
			ThrowLoweringInternal(
				"class construction return has a non-object boundary");
		const Operand slot(EnsureDirectReturnSlot(node), derived.current_result_);
		const Operand destination = derived.AddressOfStorage(slot);
		if (action.value_initialization)
			derived.EmitZeroInitialization(action.operand_type, destination);
		derived.LowerConstructorAction(node, destination);
		*result_value = slot;
	}

	std::uint32_t InternLexicalCleanupState(
		const lowering::cleanup::Key& key, const char* label,
		std::vector<std::uint32_t>* pending, bool* inserted)
	{
		Derived& derived = static_cast<Derived&>(*this);
		if (derived.stats_) ++derived.stats_->cleanup_state_probes;
		const std::uint32_t state =
			derived.cleanup_continuations_.Intern(key, inserted);
		if (!*inserted)
		{
			if (derived.stats_) ++derived.stats_->cleanup_state_hits;
			return state;
		}
		const BlockId block = derived.AddBlock(derived.NewLabel(label));
		derived.cleanup_continuations_.BindBlock(state, block);
		pending->push_back(state);
		if (derived.stats_)
		{
			++derived.stats_->cleanup_unique_states;
			++derived.stats_->cleanup_blocks_emitted;
		}
		return state;
	}

	BlockId LexicalCleanupBlock(std::uint32_t state) const
	{
		const Derived& derived = static_cast<const Derived&>(*this);
		const lowering::cleanup::State& record =
			derived.cleanup_continuations_.Get(state);
		if (!record.block_bound)
			ThrowLoweringInternal("lexical cleanup continuation has no block");
		return record.block;
	}

	std::uint32_t BuildLexicalReturnCleanup(const NodeChildren& children,
		bool has_value, bool returns_value, std::size_t first_cleanup,
		std::vector<std::uint32_t>* pending)
	{
		Derived& derived = static_cast<Derived&>(*this);
		using namespace lowering::cleanup;
		const std::uint32_t context = derived.ExceptionCleanupContext();
		const std::uint32_t terminal = returns_value ?
			static_cast<std::uint32_t>(shared_return_slot_) + 1 : 0;
		bool inserted = false;
		std::uint32_t tail = InternLexicalCleanupState(Key(kNoCleanupState,
			kNoCleanupState, terminal, context, LEXICAL_RETURN_TERMINAL),
			"return_cleanup_terminal", pending, &inserted);
		std::size_t depth = 0;
		for (std::size_t i = children.size(); i != first_cleanup; --i)
		{
			const std::size_t index = i - 1;
			if (!ReturnCleanupActionApplies(children, has_value, index)) continue;
			const std::size_t target = LexicalDestructorDepth(derived.arena_.nodes[children[index]]);
			while (depth < target)
			{
				tail = InternLexicalCleanupState(Key(static_cast<std::uint32_t>(depth++),
					tail, terminal, context, LEXICAL_RETURN_REGION_EXIT),
					"return_cleanup_exit", pending, &inserted);
			}
			const ActionKey action_key = MakeActionKey(
				derived.arena_.nodes[children[index]]);
			const std::uint32_t action = derived.cleanup_continuations_.InternAction(
				action_key, children[index], &inserted);
			tail = InternLexicalCleanupState(Key(action, tail, terminal, context,
				LEXICAL_RETURN_ACTION), "return_cleanup_action", pending, &inserted);
			if (!inserted && derived.stats_)
				++derived.stats_->cleanup_destructor_actions_avoided;
		}
		while (depth < derived.ActiveExceptionRegionCount())
			tail = InternLexicalCleanupState(Key(static_cast<std::uint32_t>(depth++),
				tail, terminal, context, LEXICAL_RETURN_REGION_EXIT),
				"return_cleanup_exit", pending, &inserted);
		return tail;
	}

	void MaterializeLexicalReturnCleanup(
		const std::vector<std::uint32_t>& pending,
		std::size_t closed_exception_handlers,
		std::size_t exception_regions, bool returns_value)
	{
		(void)closed_exception_handlers;
		Derived& derived = static_cast<Derived&>(*this);
		using namespace lowering::cleanup;
		const BlockId original = derived.current_block_;
		for (std::size_t i = 0; i < pending.size(); ++i)
		{
			const State state = derived.cleanup_continuations_.Get(pending[i]);
			derived.SelectBlock(state.block);
			if (state.key.mode == LEXICAL_RETURN_ACTION)
			{
				LowerLexicalDestructorAction(
					derived.arena_.nodes[derived.cleanup_continuations_.GetAction(
						state.key.action).representative_node]);
				derived.EmitJump(LexicalCleanupBlock(state.key.tail));
			}
			else if (state.key.mode == LEXICAL_RETURN_REGION_EXIT)
			{
				derived.EmitLexicalCleanupRegionExit(state.key.action);
				derived.EmitJump(LexicalCleanupBlock(state.key.tail));
			}
			else if (state.key.mode == LEXICAL_RETURN_TERMINAL)
			{
				derived.FinishExceptionControlExit(exception_regions,
					exception_regions);
				if (derived.constructor_body_cleanup_active_)
					derived.Emit(Instruction(Instruction::EH_END));
				derived.FinishFunctionExceptionBoundaryNormalExit();
				if (!returns_value)
					derived.Emit(Instruction(Instruction::RETURN_VOID));
				else
				{
					Instruction instruction(Instruction::RETURN_VALUE);
					instruction.type = derived.current_result_;
					instruction.first = Operand(shared_return_slot_,
						derived.current_result_);
					derived.Emit(instruction);
				}
			}
			else ThrowLoweringInternal(
				"invalid lexical return cleanup continuation mode");
		}
		derived.SelectBlock(original);
	}

	Operand LowerScalarReturnValue(std::uint32_t node)
	{
		Derived& derived = static_cast<Derived&>(*this);
		const bool boolean_conversion =
			derived.arena_.nodes[node].boolean_conversion;
		const Operand value = boolean_conversion ?
			derived.LowerConvertedValue(node,
				derived.current_result_, false) :
			derived.LowerValue(node,
				derived.current_result_.kind == LOW_PTR ?
				derived.current_result_ : LowType());
		const bool preserve_unsigned_conversion =
			value.kind == Operand::INTEGER && IsInteger(value.type) &&
			IsInteger(derived.current_result_) && value.type.is_signed &&
			!derived.current_result_.is_signed &&
			value.type.width < derived.current_result_.width;
		return boolean_conversion ? value : derived.Convert(
			value, derived.current_result_,
			!preserve_unsigned_conversion);
	}

	Operand LowerReferenceReturnValue(std::uint32_t node)
	{
		Derived& derived = static_cast<Derived&>(*this);
		const DumpNode& returned = derived.arena_.nodes[node];
		if (returned.category != VALUE_PRVALUE)
			return derived.AddressOfStorage(
				derived.LowerStorage(node));
		else
		{
			const LowType type =
				derived.LowerExpressionType(returned.type);
			const Operand slot(derived.EnsureGeneratedSlot(
				node, "retref", type), type);
			Instruction store(Instruction::STORE);
			store.type = type;
			store.first = derived.LowerValue(node, type);
			store.second = slot;
			derived.Emit(store);
			return derived.AddressOfStorage(slot);
		}
	}

	void LowerConditionalReturn(std::uint32_t node, Operand* result_value)
	{
		Derived& derived = static_cast<Derived&>(*this);
		if (derived.current_indirect_result_)
		{
			const Operand destination(
				static_cast<ParameterId>(0), LowPtr());
			derived.LowerClassConditionalResult(
				node, destination);
		}
		else if (derived.current_result_.kind == LOW_OBJECT)
		{
			const Operand slot(EnsureDirectReturnSlot(node),
				derived.current_result_);
			derived.LowerClassConditionalResult(node,
				derived.AddressOfStorage(slot));
			*result_value = slot;
		}
		else ThrowLoweringInternal(
			"class conditional return has a non-object boundary");
	}

	ConstructionCleanupStep RetainReturnedObject(std::uint32_t return_node,
		std::uint32_t value_node, const Operand& value)
	{
		Derived& derived = static_cast<Derived&>(*this);
		const TypeId type = derived.arena_.nodes[value_node].type;
		const BindingId destructor = derived.arena_.nodes[return_node].selected_binding;
		const Operand address = derived.current_indirect_result_ ?
			Operand(static_cast<ParameterId>(0), LowPtr()) : derived.AddressOfStorage(value);
		derived.CompleteConstructionObject(0, type, destructor, address);
		return ConstructionCleanupStep(0, kNoDumpEdge, type, destructor, address);
	}

	void LowerReturn(std::uint32_t return_node, const NodeChildren& children)
	{
		Derived& derived = static_cast<Derived&>(*this);
		const bool has_value = !children.empty() &&
			derived.arena_.nodes[children[0]].kind != DUMP_DESTRUCTOR_ACTION;
		const bool protect_return = has_value &&
			derived.arena_.nodes[return_node].selected_binding != kNoBinding &&
			!derived.arena_.nodes[children[0]].direct_return_slot;
		const std::size_t first_cleanup = has_value ? 1 : 0;
		std::size_t full_expression_cleanup_end = first_cleanup;
		NodeChildren full_expression_actions;
		bool conditional_full_expression = false;
		bool explicitly_managed_full_expression = false;
		bool lexical_unwind = false;
		while (full_expression_cleanup_end < children.size())
		{
			const DumpNode& action =
				derived.arena_.nodes[children[full_expression_cleanup_end]];
			if (action.kind != DUMP_DESTRUCTOR_ACTION ||
				!action.full_expression_staging)
				break;
			full_expression_actions.Push(children[full_expression_cleanup_end]);
			if (action.lifetime_object != kNoDumpEdge &&
				derived.arena_.nodes[action.lifetime_object].conditionally_constructed)
				conditional_full_expression = true;
			if (action.managed_full_expression_cleanup)
				explicitly_managed_full_expression = true;
			lexical_unwind = lexical_unwind || action.unwind_only;
			++full_expression_cleanup_end;
		}
		const bool managed_full_expression = conditional_full_expression ||
			explicitly_managed_full_expression || lexical_unwind || protect_return;
		if (managed_full_expression)
			derived.BeginFullExpressionCleanup(full_expression_actions, 0,
				has_value && derived.arena_.nodes[children[0]].kind ==
					DUMP_CONDITIONAL_EXPRESSION);
		if (protect_return)
			derived.BeginConstructionCleanup(derived.arena_.nodes[return_node], true);
		Operand result_value;
		if (has_value)
		{
			if (derived.arena_.nodes[children[0]].direct_return_slot)
			{
				if (!derived.current_indirect_result_)
					ThrowLoweringInternal(
						"direct return slot has a direct result boundary");
			}
			else if (derived.arena_.nodes[children[0]].kind ==
				DUMP_BRACED_INIT_LIST &&
				derived.IsClassObjectType(derived.arena_.nodes[children[0]].type))
			{
				if (derived.current_indirect_result_)
				{
					const Operand destination(
						static_cast<ParameterId>(0), LowPtr());
					derived.LowerRuntimeObjectValue(
						derived.arena_.nodes[children[0]].type,
						children[0], destination);
				}
				else if (derived.current_result_.kind == LOW_OBJECT)
				{
					const Operand slot(EnsureDirectReturnSlot(children[0]),
						derived.current_result_);
					derived.LowerRuntimeObjectValue(
						derived.arena_.nodes[children[0]].type,
						children[0], derived.AddressOfStorage(slot));
					result_value = slot;
				}
				else ThrowLoweringInternal(
					"class aggregate return has a non-object boundary");
			}
			else if (derived.arena_.nodes[children[0]].kind ==
				DUMP_CONDITIONAL_EXPRESSION &&
				!derived.current_result_reference_ &&
				derived.IsClassObjectType(derived.arena_.nodes[children[0]].type))
				LowerConditionalReturn(children[0], &result_value);
			else if (derived.arena_.nodes[children[0]].kind ==
				DUMP_CLASS_VALUE_TRANSFER)
			{
				if (derived.current_indirect_result_)
				{
					const Operand destination(
						static_cast<ParameterId>(0), LowPtr());
					derived.LowerClassValueTransfer(children[0], destination);
				}
				else if (derived.current_result_.kind != LOW_OBJECT)
					ThrowLoweringInternal(
						"class-value return has a non-object boundary");
				else
				{
					const Operand slot(EnsureDirectReturnSlot(children[0]),
						derived.current_result_);
					derived.LowerClassValueTransfer(children[0],
						derived.AddressOfStorage(slot));
					result_value = slot;
				}
			}
			else if (derived.arena_.nodes[children[0]].kind ==
				DUMP_AGGREGATE_CONSTRUCTION_ACTION)
			{
				const bool protect =
					derived.AggregateConstructionOwnsNontrivialParameters(children[0]);
				const BlockId dispatch = protect ? derived.AddBlock(
					derived.NewLabel("call_unwind_dispatch")) : BlockId(kNoLowId);
				const BlockId end = protect ? derived.AddBlock(
					derived.NewLabel("call_unwind_end")) : BlockId(kNoLowId);
				if (protect)
					derived.EmitEhTarget(Instruction::EH_TRY, dispatch);
				if (derived.current_indirect_result_)
				{
					const Operand destination(
						static_cast<ParameterId>(0), LowPtr());
					derived.LowerAggregateConstructionAction(
						children[0], destination);
				}
				else if (derived.current_result_.kind == LOW_OBJECT)
				{
					const Operand slot(EnsureDirectReturnSlot(children[0]),
						derived.current_result_);
					derived.LowerAggregateConstructionAction(children[0],
						derived.AddressOfStorage(slot));
					result_value = slot;
				}
				else ThrowLoweringInternal(
					"aggregate construction return has a non-object boundary");
				if (protect)
				{
					derived.Emit(Instruction(Instruction::EH_END));
					derived.EmitJump(end);
					derived.SelectBlock(dispatch);
					derived.EmitExceptionResume();
					derived.SelectBlock(end);
				}
			}
			else if (derived.arena_.nodes[children[0]].kind ==
				DUMP_CONSTRUCTOR_ACTION)
				LowerConstructorReturn(children[0], &result_value);
			else if (derived.current_indirect_result_ &&
				derived.LowerIndirectComplexResult(children[0],
					Operand(static_cast<ParameterId>(0), LowPtr()))) {}
			else if (derived.current_result_.kind == LOW_VOID)
				(void)derived.LowerValue(children[0]);
			else if (derived.current_result_reference_)
			{
				result_value = LowerReferenceReturnValue(children[0]);
			}
			else
			{
				result_value = LowerScalarReturnValue(children[0]);
			}
		}
		if (derived.CurrentBlock().terminated) return;
		const ConstructionCleanupStep returned = protect_return ?
			RetainReturnedObject(return_node, children[0], result_value) :
			ConstructionCleanupStep(0, kNoDumpEdge, kNoType, kNoBinding, Operand());
		const bool returns_value =
			derived.current_result_.kind != LOW_VOID;
		const bool share_lexical_cleanup =
			!derived.destructor_return_routes_to_epilogue_ &&
			!derived.current_indirect_result_ &&
			derived.current_result_.kind != LOW_OBJECT &&
			(!returns_value || shared_return_slot_ != kNoLowId) &&
			ShouldShareLexicalReturn(return_node);
		if (share_lexical_cleanup && returns_value)
		{
			Instruction store(Instruction::STORE);
			store.type = derived.current_result_;
			store.first = result_value;
			store.second = Operand(shared_return_slot_, derived.current_result_);
			derived.Emit(store);
		}
		if (managed_full_expression)
			derived.CompleteFullExpressionCleanup();
		const std::size_t exception_regions =
			derived.ActiveExceptionRegionCount();
		std::size_t closed_exception_handlers =
			derived.BeginExceptionControlExit(exception_regions);
		const std::size_t remaining_cleanup = managed_full_expression ?
			full_expression_cleanup_end : first_cleanup;
		if (share_lexical_cleanup)
		{
			std::vector<std::uint32_t> pending;
			const std::uint32_t root = BuildLexicalReturnCleanup(children,
				has_value, returns_value, remaining_cleanup, &pending);
			derived.EmitJump(LexicalCleanupBlock(root));
			MaterializeLexicalReturnCleanup(pending,
				closed_exception_handlers, exception_regions, returns_value);
			return;
		}
		for (std::size_t i = remaining_cleanup; i < children.size(); ++i)
		{
			if (derived.arena_.nodes[children[i]].kind != DUMP_DESTRUCTOR_ACTION)
				ThrowLoweringInternal("invalid return cleanup action");
			if (has_value &&
				derived.arena_.nodes[children[0]].direct_return_slot &&
				derived.arena_.nodes[children[i]].object_binding ==
					derived.arena_.nodes[children[0]].binding)
				continue;
			LowerLexicalDestructorAction(derived.arena_.nodes[children[i]],
				&closed_exception_handlers, protect_return ? &returned : 0);
		}
		derived.FinishExceptionControlExit(
			closed_exception_handlers, exception_regions);
		if (derived.constructor_body_cleanup_active_)
			derived.Emit(Instruction(Instruction::EH_END));
		if (derived.destructor_return_routes_to_epilogue_)
		{
			if (derived.destructor_return_target_ == kNoLowId)
				derived.destructor_return_target_ = derived.AddBlock(
					derived.NewLabel("destructor_return_epilogue"));
			derived.Emit(Instruction(Instruction::EH_END));
			derived.EmitJump(derived.destructor_return_target_);
			return;
		}
		derived.FinishFunctionExceptionBoundaryNormalExit();
		if (derived.current_result_.kind == LOW_VOID)
			derived.Emit(Instruction(Instruction::RETURN_VOID));
		else
		{
			if (!has_value)
				ThrowLoweringInternal("non-void return has no value");
			Instruction instruction(Instruction::RETURN_VALUE);
			instruction.type = derived.current_result_;
			instruction.first = result_value;
			derived.Emit(instruction);
		}
	}

	void EmitDestructorActionRange(const NodeChildren& children,
		std::size_t first)
	{
		Derived& derived = static_cast<Derived&>(*this);
		for (std::size_t i = first; i < children.size(); ++i)
		{
			if (derived.arena_.nodes[children[i]].kind != DUMP_DESTRUCTOR_ACTION)
				ThrowLoweringInternal("invalid destructor suffix action");
			if (i + 1 == children.size())
			{
				derived.LowerDestructorAction(derived.arena_.nodes[children[i]]);
				continue;
			}
			const BlockId cleanup = derived.AddBlock(
				derived.NewLabel("destructor_suffix_cleanup"));
			const BlockId next = derived.AddBlock(
				derived.NewLabel("destructor_suffix_next"));
			derived.EmitEhTarget(Instruction::EH_CLEANUP, cleanup);
			derived.LowerDestructorAction(derived.arena_.nodes[children[i]]);
			derived.Emit(Instruction(Instruction::EH_END));
			derived.EmitJump(next);
			derived.SelectBlock(cleanup);
			for (std::size_t j = i + 1; j < children.size(); ++j)
				LowerUnwindDestructorAction(derived.arena_.nodes[children[j]]);
			derived.Emit(Instruction(Instruction::EH_END));
			derived.EmitExceptionResume();
			derived.SelectBlock(next);
		}
	}

	void LowerDestructorBody(std::uint32_t body)
	{
		Derived& derived = static_cast<Derived&>(*this);
		const NodeChildren children = derived.Children(body);
		std::size_t first_action = children.size();
		for (std::size_t i = 0; i < children.size(); ++i)
			if (derived.arena_.nodes[children[i]].kind == DUMP_DESTRUCTOR_ACTION)
			{
				first_action = i;
				break;
			}
		if (first_action == children.size())
		{
			derived.LowerStatement(body);
			return;
		}
		if (children.size() - first_action > kDestructorCleanupInlineLimit)
		{
			LowerCompactDestructorBody(body, children, first_action);
			return;
		}
		const BlockId cleanup = derived.AddBlock(
			derived.NewLabel("destructor_cleanup"));
		const bool detached = derived.arena_.nodes[body].lexical_body_cleanup ||
			derived.arena_.nodes[body].body_contains_source_try;
		const BlockId cleanup_entry = detached ? derived.AddBlock(
			derived.NewLabel("destructor_cleanup_entry")) : cleanup;
		const BlockId end = derived.AddBlock(
			derived.NewLabel("destructor_end"));
		derived.EmitEhTarget(Instruction::EH_CLEANUP, cleanup);
		derived.destructor_return_target_ = kNoLowId;
		derived.destructor_return_routes_to_epilogue_ = true;
		lexical_body_unwind_target_ = detached ? cleanup_entry : BlockId(kNoLowId);
		lexical_body_unwind_depth_ = derived.ActiveExceptionRegionCount();
		for (std::size_t i = 0; i < first_action; ++i)
			derived.LowerStatement(children[i]);
		derived.destructor_return_routes_to_epilogue_ = false;
		lexical_body_unwind_target_ = kNoLowId;
		if (derived.destructor_return_target_ != kNoLowId)
		{
			if (!derived.CurrentBlock().terminated)
			{
				derived.Emit(Instruction(Instruction::EH_END));
				derived.EmitJump(derived.destructor_return_target_);
			}
			derived.SelectBlock(derived.destructor_return_target_);
		}
		else
		{
			if (derived.CurrentBlock().terminated) return;
			derived.Emit(Instruction(Instruction::EH_END));
		}
		derived.destructor_return_target_ = kNoLowId;
		EmitDestructorActionRange(children, first_action);
		derived.EmitJump(end);
		derived.SelectBlock(cleanup);
		if (detached)
		{
			derived.EmitEnclosingTryHandlerClauses();
			derived.Emit(Instruction(Instruction::EH_END));
			derived.EmitJump(cleanup_entry);
			derived.SelectBlock(cleanup_entry);
		}
		for (std::size_t i = first_action; i < children.size(); ++i)
			LowerUnwindDestructorAction(derived.arena_.nodes[children[i]]);
		if (!detached) derived.Emit(Instruction(Instruction::EH_END));
		derived.FinishExceptionCleanupDispatch(detached &&
			derived.EnclosingTryRegion() != 0, false);
		derived.SelectBlock(end);
	}

	void LowerCompactDestructorBody(std::uint32_t body,
		const NodeChildren& children, std::size_t first_action)
	{
		Derived& derived = static_cast<Derived&>(*this);
		const std::size_t action_count = children.size() - first_action;
		const Operand progress(derived.EnsureGeneratedSlot(
			body, "destructor_progress", LowI64()), LowI64());
		const BlockId cleanup = derived.AddBlock(
			derived.NewLabel("destructor_cleanup"));
		const bool detached = derived.arena_.nodes[body].lexical_body_cleanup ||
			derived.arena_.nodes[body].body_contains_source_try;
		const BlockId cleanup_entry = detached ? derived.AddBlock(
			derived.NewLabel("destructor_cleanup_entry")) : cleanup;
		const BlockId end = derived.AddBlock(
			derived.NewLabel("destructor_end"));
		Instruction initial_progress(Instruction::STORE);
		initial_progress.type = LowI64();
		initial_progress.first = Operand(0, LowI64());
		initial_progress.second = progress;
		derived.Emit(initial_progress);
		derived.EmitEhTarget(Instruction::EH_CLEANUP, cleanup);
		derived.destructor_return_target_ = kNoLowId;
		derived.destructor_return_routes_to_epilogue_ = true;
		lexical_body_unwind_target_ = detached ? cleanup_entry : BlockId(kNoLowId);
		lexical_body_unwind_depth_ = derived.ActiveExceptionRegionCount();
		for (std::size_t i = 0; i < first_action; ++i)
			derived.LowerStatement(children[i]);
		derived.destructor_return_routes_to_epilogue_ = false;
		lexical_body_unwind_target_ = kNoLowId;
		if (derived.destructor_return_target_ != kNoLowId)
		{
			if (!derived.CurrentBlock().terminated)
			{
				derived.Emit(Instruction(Instruction::EH_END));
				derived.EmitJump(derived.destructor_return_target_);
			}
			derived.SelectBlock(derived.destructor_return_target_);
		}
		else
		{
			if (derived.CurrentBlock().terminated) return;
			derived.Emit(Instruction(Instruction::EH_END));
		}
		derived.destructor_return_target_ = kNoLowId;
		for (std::size_t i = 0; i < action_count; ++i)
		{
			if (i + 1 < action_count)
			{
				Instruction next_progress(Instruction::STORE);
				next_progress.type = LowI64();
				next_progress.first = Operand(
					static_cast<std::int64_t>(i + 1), LowI64());
				next_progress.second = progress;
				derived.Emit(next_progress);
				derived.EmitEhTarget(Instruction::EH_CLEANUP, cleanup);
			}
			derived.LowerDestructorAction(
				derived.arena_.nodes[children[first_action + i]]);
			if (i + 1 < action_count)
				derived.Emit(Instruction(Instruction::EH_END));
		}
		derived.EmitJump(end);

		SmallSequence<BlockId, 8> cleanup_blocks;
		for (std::size_t i = 0; i < action_count; ++i)
			cleanup_blocks.Push(derived.AddBlock(
				derived.NewLabel("destructor_suffix_cleanup")));
		derived.SelectBlock(cleanup);
		if (detached)
		{
			derived.EmitEnclosingTryHandlerClauses();
			derived.Emit(Instruction(Instruction::EH_END));
			derived.EmitJump(cleanup_entry);
			derived.SelectBlock(cleanup_entry);
		}
		const Operand selected = derived.LoadStorage(progress, LowI64());
		Instruction dispatch(Instruction::SWITCH);
		dispatch.first = selected;
		dispatch.target = cleanup_blocks[0];
		SmallSequence<std::int64_t, 8> values;
		for (std::size_t i = 0; i < action_count; ++i)
			values.Push(static_cast<std::int64_t>(i));
		derived.AttachSwitchCases(&dispatch, values, cleanup_blocks);
		derived.Emit(dispatch);
		derived.RecordBlockIncoming(dispatch.target);
		for (std::size_t i = 0; i < cleanup_blocks.size(); ++i)
			derived.RecordBlockIncoming(cleanup_blocks[i]);
		for (std::size_t i = 0; i < action_count; ++i)
		{
			derived.SelectBlock(cleanup_blocks[i]);
			LowerUnwindDestructorAction(
				derived.arena_.nodes[children[first_action + i]]);
			if (i + 1 < action_count)
				derived.EmitJump(cleanup_blocks[i + 1]);
			else
			{
				if (!detached) derived.Emit(Instruction(Instruction::EH_END));
				derived.FinishExceptionCleanupDispatch(detached &&
					derived.EnclosingTryRegion() != 0, false);
			}
		}
		derived.SelectBlock(end);
	}

	SlotId direct_return_slot_, shared_return_slot_;
	BlockId lexical_cleanup_terminate_, lexical_body_unwind_target_;
	std::size_t lexical_body_unwind_depth_;
	FlatIdMap return_cleanup_roots_;
	std::vector<std::uint32_t> return_cleanup_counts_;
};

}
}

#endif
