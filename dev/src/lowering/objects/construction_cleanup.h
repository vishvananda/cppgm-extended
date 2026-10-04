#ifndef CPPGM_LOWERING_CONSTRUCTION_CLEANUP_H
#define CPPGM_LOWERING_CONSTRUCTION_CLEANUP_H

#include "lowering/objects/cleanup_continuations.h"
#include "lowering/support/errors.h"
#include "lowering/support/identity_maps.h"
#include "lowering/support/sequences.h"

#include <vector>

namespace cppgm
{
namespace lowering
{

struct ConstructionCleanupStep
{
	std::uint32_t tail;
	std::uint32_t temporary_action;
	std::uint32_t allocation_node;
	semantic::TypeId type;
	semantic::BindingId destructor;
	ir::Operand destination;

	ConstructionCleanupStep(std::uint32_t previous, std::uint32_t temporary,
		semantic::TypeId object_type, semantic::BindingId destructor_binding,
		const ir::Operand& address, std::uint32_t allocation = semantic::kNoDumpEdge)
		: tail(previous), temporary_action(temporary), allocation_node(allocation), type(object_type),
		  destructor(destructor_binding), destination(address)
	{
	}
};

template <class Derived>
class ConstructionCleanupLowering
{
protected:
	ConstructionCleanupLowering() : construction_cleanup_root_(0),
		construction_cleanup_active_(false) {}

	void ResetConstructionFunctionState()
	{
		construction_cleanup_steps_.clear();
		ResetConstructionExpressionState();
	}

	void ResetConstructionExpressionState()
	{
		construction_cleanup_root_ = 0;
		construction_cleanup_active_ = false;
		construction_temporary_actions_.Clear();
	}

	bool HasConstructionCleanup() const
	{
		return construction_cleanup_active_;
	}

	std::uint32_t ConstructionCleanupRoot() const
	{
		return construction_cleanup_root_;
	}

	void RestoreConstructionCleanupRoot(std::uint32_t root)
	{
		construction_cleanup_root_ = root;
	}

	void MergeConstructionBranches(std::uint32_t previous,
		std::uint32_t first, std::uint32_t second, semantic::TypeId type,
		const ir::Operand& destination)
	{
		if (!construction_cleanup_active_) return;
		construction_cleanup_root_ = previous;
		semantic::BindingId destructor = semantic::kNoBinding;
		const std::uint32_t branches[] = { first, second };
		for (std::size_t arm = 0; arm != 2; ++arm)
		{
			std::vector<std::uint32_t> temporaries;
			for (std::uint32_t root = branches[arm]; root != previous;)
			{
				if (root == 0) ThrowLoweringInternal("construction branch boundary lost");
				const ConstructionCleanupStep step = construction_cleanup_steps_[root - 1];
				if (step.temporary_action != semantic::kNoDumpEdge)
					temporaries.push_back(step.temporary_action);
				else if (step.type == type && step.destination.kind == destination.kind &&
					step.destination.id == destination.id) destructor = step.destructor;
				root = step.tail;
			}
			// Arm temporaries already have evaluated-path lifetime guards. Both
			// sets survive the join; the selected set keeps its completion order.
			for (std::size_t i = temporaries.size(); i != 0; --i)
				PushConstructionTemporary(temporaries[i - 1]);
		}
		if (destructor != semantic::kNoBinding)
			PushConstructionCleanup(ConstructionCleanupStep(construction_cleanup_root_,
				semantic::kNoDumpEdge, type, destructor, destination));
	}

	void PushConstructionCleanup(const ConstructionCleanupStep& step)
	{
		if (construction_cleanup_steps_.size() >= UINT32_MAX)
			ThrowLoweringResourceLimit("construction cleanup identity overflow");
		construction_cleanup_steps_.push_back(step);
		construction_cleanup_root_ = static_cast<std::uint32_t>(
			construction_cleanup_steps_.size());
	}

	void PushConstructionTemporary(std::uint32_t action)
	{
		PushConstructionCleanup(ConstructionCleanupStep(construction_cleanup_root_,
			action, semantic::kNoType, semantic::kNoBinding, ir::Operand()));
	}

	bool BeginConstructionCleanup(const semantic::DumpNode& recipe, bool force = false)
	{
		Derived& derived = static_cast<Derived&>(*this);
		if (!force && !construction_cleanup_active_ && (!recipe.contains_construction_cleanup ||
			recipe.value_constructor != semantic::kNoDumpEdge))
			return false;
		const bool owns_expression = !derived.full_expression_cleanup_active_;
		if (owns_expression)
		{
			const support::NodeChildren empty;
			derived.BeginFullExpressionCleanup(empty, 0, true);
		}
		if (construction_cleanup_active_) return owns_expression;
		derived.PauseFullExpressionCleanupSegment();
		construction_cleanup_active_ = true;
		for (std::size_t i = derived.full_expression_cleanup_actions_.size(); i != 0; --i)
		{
			const std::uint32_t action = derived.full_expression_cleanup_actions_[i - 1];
			const std::uint32_t temporary = derived.arena_.nodes[action].lifetime_object;
			if (temporary != semantic::kNoDumpEdge) construction_temporary_actions_.Insert(temporary, action);
			if (temporary != semantic::kNoDumpEdge && derived.temporary_initialized_[temporary])
				PushConstructionTemporary(action);
		}
		return owns_expression;
	}

	void RetainConstructionTemporaries(std::uint32_t previous)
	{
		// Successful construction transfers its subobjects and allocation to
		// the result; argument temporaries keep their full-expression lifetime.
		std::vector<std::uint32_t> retained;
		for (std::uint32_t root = construction_cleanup_root_; root != previous;)
		{
			if (root == 0) ThrowLoweringInternal("construction cleanup boundary lost");
			const ConstructionCleanupStep& step = construction_cleanup_steps_[root - 1];
			if (step.temporary_action != semantic::kNoDumpEdge) retained.push_back(step.temporary_action);
			root = step.tail;
		}
		construction_cleanup_root_ = previous;
		for (std::size_t i = retained.size(); i != 0; --i) PushConstructionTemporary(retained[i - 1]);
	}

	void CompleteConstructionObject(std::uint32_t previous,
		semantic::TypeId type, semantic::BindingId destructor,
		const ir::Operand& destination)
	{
		if (!construction_cleanup_active_ || destructor == semantic::kNoBinding) return;
		Derived& derived = static_cast<Derived&>(*this);
		derived.PauseFullExpressionCleanupSegment();
		RetainConstructionTemporaries(previous);
		PushConstructionCleanup(ConstructionCleanupStep(construction_cleanup_root_,
			semantic::kNoDumpEdge, type, destructor, destination));
		derived.full_expression_cleanup_ready_ = true;
	}

	bool TransitionConstructionTemporary(std::uint32_t temporary)
	{
		if (!construction_cleanup_active_) return false;
		Derived& derived = static_cast<Derived&>(*this);
		derived.PauseFullExpressionCleanupSegment();
		std::uint32_t action = semantic::kNoDumpEdge;
		if (construction_temporary_actions_.Find(temporary, &action))
		{
			if (construction_cleanup_root_ != 0)
			{
				const ConstructionCleanupStep& latest = construction_cleanup_steps_[construction_cleanup_root_ - 1];
				const ir::Operand& address = derived.temporary_addresses_[temporary];
				if (latest.temporary_action == semantic::kNoDumpEdge &&
					latest.type == derived.arena_.nodes[temporary].type &&
					latest.destination.kind == address.kind && latest.destination.id == address.id)
					construction_cleanup_root_ = latest.tail;
			}
			PushConstructionTemporary(action);
			derived.full_expression_cleanup_ready_ = true;
			if (derived.full_expression_uses_linked_dispatch_ && derived.full_expression_linked_action_cursor_ != 0)
				--derived.full_expression_linked_action_cursor_;
		}
		return true;
	}

	void RetireConstructionTemporary(std::uint32_t action)
	{
		if (!construction_cleanup_active_) return;
		std::vector<ConstructionCleanupStep> retained;
		std::uint32_t root = construction_cleanup_root_;
		while (root != 0)
		{
			const ConstructionCleanupStep step = construction_cleanup_steps_[root - 1];
			root = step.tail;
			if (step.temporary_action == action)
			{
				construction_cleanup_root_ = root;
				for (std::size_t i = retained.size(); i != 0; --i)
				{
					ConstructionCleanupStep replacement = retained[i - 1];
					replacement.tail = construction_cleanup_root_;
					PushConstructionCleanup(replacement);
				}
				return;
			}
			retained.push_back(step);
		}
	}

	void LowerConstructionNormalTemporary(std::uint32_t action)
	{
		Derived& derived = static_cast<Derived&>(*this);
		if (construction_cleanup_active_)
		{
			derived.PauseFullExpressionCleanupSegment();
			RetireConstructionTemporary(action);
			derived.EnsureFullExpressionCleanupSegment();
		}
		derived.LowerFullExpressionDestructorAction(action);
		if (construction_cleanup_active_) derived.PauseFullExpressionCleanupSegment();
	}

	std::uint32_t BuildConstructionCleanupTail(std::uint32_t tail, std::uint32_t context)
	{
		Derived& derived = static_cast<Derived&>(*this);
		std::uint32_t root = construction_cleanup_root_;
		std::uint32_t result = tail;
		while (root != 0)
		{
			bool inserted = false;
			const std::uint32_t state = derived.InternContinuation(cleanup::Key(root,
				cleanup::kNoCleanupState, tail, context, cleanup::CONSTRUCTION_PREFIX),
				"construction_cleanup", &inserted);
			if (root == construction_cleanup_root_) result = state;
			if (!inserted) break;
			root = construction_cleanup_steps_[root - 1].tail;
		}
		return result;
	}

	void FinishConstructionArrayCleanup(bool routes_to_try = false)
	{
		Derived& derived = static_cast<Derived&>(*this);
		// Resume continues at a caller frame. An inner array landing must
		// explicitly enter the remaining cleanup in this constructor first.
		const ir::BlockId remaining = derived.full_expression_cleanup_dispatch_ != ir::kNoLowId ?
			derived.full_expression_cleanup_dispatch_ : derived.constructor_body_cleanup_target_;
		if (remaining == ir::kNoLowId)
		{
			if (routes_to_try) derived.FinishExceptionCleanupDispatch(true);
			else derived.EmitExceptionResume();
		}
		else
		{
			// A backing-array landing enters a still-protected expression
			// continuation, which retires its own frame after cleanup.
			derived.Emit(ir::Instruction(ir::Instruction::EH_END));
			if (!routes_to_try ||
				remaining != derived.full_expression_cleanup_dispatch_)
				derived.Emit(ir::Instruction(ir::Instruction::EH_END));
			derived.EmitJump(remaining);
		}
	}

	void LowerConstructionArrayPrefix(semantic::TypeId element,
		const ir::Operand& destination, semantic::BindingId destructor,
		const ir::Operand& progress)
	{
		Derived& derived = static_cast<Derived&>(*this);
		const ir::BlockId condition = derived.AddBlock(derived.NewLabel("construction_array_cleanup"));
		const ir::BlockId body = derived.AddBlock(derived.NewLabel("construction_array_cleanup_body"));
		const ir::BlockId end = derived.AddBlock(derived.NewLabel("construction_array_cleanup_end"));
		const bool may_throw = !derived.program_.bindings[destructor].nonthrowing;
		if (may_throw) derived.EmitEhTarget(ir::Instruction::EH_TRY, derived.MakeCleanupTerminateBlock());
		derived.EmitJump(condition);
		derived.SelectBlock(condition);
		const ir::Operand remaining = derived.LoadStorage(progress, ir::LowI64());
		const ir::Operand any = derived.Temp(ir::LowI64());
		ir::Instruction compare(ir::Instruction::CMP);
		compare.dest = any.id;
		compare.op = ir::LOW_OP_NE;
		compare.type = ir::LowI64();
		compare.first = remaining;
		compare.second = ir::Operand(0, ir::LowI64());
		derived.Emit(compare);
		derived.EmitBranch(any, body, end);
		derived.SelectBlock(body);
		const ir::Operand previous = derived.DecrementDestructorArrayProgress(progress, remaining);
		derived.EmitDestructorCall(destructor, derived.ArrayElementAddress(element, destination, previous));
		derived.EmitJump(condition);
		derived.SelectBlock(end);
		if (may_throw) derived.Emit(ir::Instruction(ir::Instruction::EH_END));
	}

	void LowerConstructionCleanupState(const cleanup::State& state)
	{
		Derived& derived = static_cast<Derived&>(*this);
		const ConstructionCleanupStep step = construction_cleanup_steps_[state.key.action - 1];
		const bool temporary = step.temporary_action != semantic::kNoDumpEdge;
		const semantic::BindingId destructor = temporary ?
			derived.arena_.nodes[step.temporary_action].binding : step.destructor;
		const bool may_throw = !derived.program_.bindings[destructor].nonthrowing;
		if (may_throw) derived.EmitEhTarget(ir::Instruction::EH_TRY, derived.LexicalCleanupTerminateBlock());
		if (temporary) derived.LowerFullExpressionDestructorAction(step.temporary_action);
		else if (step.allocation_node != semantic::kNoDumpEdge)
			derived.EmitSelectedDeallocation(derived.arena_.nodes[step.allocation_node],
				step.destination, step.destructor);
		else derived.LowerDestructorObject(step.type, step.destination, step.destructor, false, true);
		if (may_throw) derived.Emit(ir::Instruction(ir::Instruction::EH_END));
		std::uint32_t tail = state.key.terminal;
		if (step.tail != 0)
		{
			bool inserted = false;
			tail = derived.cleanup_continuations_.Intern(cleanup::Key(step.tail,
				cleanup::kNoCleanupState, state.key.terminal, state.key.context,
				cleanup::CONSTRUCTION_PREFIX), &inserted);
			if (inserted) ThrowLoweringInternal("construction cleanup suffix was not interned");
		}
		derived.EmitJump(derived.ContinuationBlock(tail));
	}

private:
	std::vector<ConstructionCleanupStep> construction_cleanup_steps_;
	support::FlatIdMap construction_temporary_actions_;
	std::uint32_t construction_cleanup_root_;
	bool construction_cleanup_active_;
};

}  // namespace lowering
}  // namespace cppgm

#endif
