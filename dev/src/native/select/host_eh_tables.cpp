#include "native/select/function_selector.h"
#include "native/select/host_eh.h"

#include <algorithm>

// The host EH tables of a function, from the states host_eh.cpp found:
// - The type table: one index per catch type (type information, or none
//   for a catch-all). The clauses' own selectors when each selector of the
//   function names one type (the frontend numbers a try statement's
//   handlers so), else indexes by first appearance (terminate guards
//   number their catch-all on their own).
// - Each landing's action chain of type indexes: its leading clauses (a
//   bare eh_cleanup as a cleanup) followed by the chains of the inactive
//   records below it, each index once and nothing after a catch-all; a
//   landing without clauses is a cleanup, as is a cleanup record's landing
//   whose clauses have no catch-all, and a landing whose chain has no
//   catch-all where an exception it does not catch meets work (the
//   frontend runs the destructors of objects declared before a try on its
//   landing's no-match path).
// - An exception specification's filter clause (eh_filter): a negative
//   chain entry naming its list of type indexes (EhTables::specs); the
//   function's types then take indexes by first appearance.
// - Each landing's selector translation, from the filter the personality
//   delivers (the first chain entry that matches) to the selector the
//   course model's clauses leave: the selector of the first leading clause
//   with that index, 0 for any other filter. A landing whose clauses map to
//   themselves (a filter clause whose selector is its negative filter: the
//   frontend's guard) and whose chain holds no other type index needs none
//   (a specification's filter below passes through: the guard it resumes
//   to tests it), unless the function has mid-landing clauses (their
//   `selector == 0` test needs the exact value): its pad stores the filter
//   as the selector. When any pad translates, every pad keeps the raw
//   filter (slot + 16) and a landing a resume of the frame enters has a
//   selector block too: the resumed landing's selector comes from the raw
//   filter, not from the selector the resuming landing's pad left (a
//   function-try-block's handler entered from a member's cleanup landing).
namespace native {

using lowir::Instruction;
using lowir::kNoId;
using lowir::Opcode;
using lowir::TypeKind;
using lowir::ValueKind;

namespace {

// A catch-all's type key.
const uint32_t kCatchAllType = kNoId - 1;

} // namespace

// The type index of catch clause `inst`.
uint32_t FunctionSelector::eh_clause_index(const Instruction& inst)
{
	if (eh_identity_)
		return static_cast<uint32_t>(inst.immediate);
	const uint32_t type = inst.op == Opcode::EhCatch ? inst.ops[0].index : kCatchAllType;
	if (const uint32_t* known = eh_type_index_.find(type))
		return *known;
	out_->eh.types.push_back(type == kCatchAllType ? kNoId : type);
	const uint32_t index = static_cast<uint32_t>(out_->eh.types.size());
	eh_type_index_.insert(type, index);
	return index;
}

// The negative filter of exception specification clause `inst`: its list
// of type indexes appended to the specification lists once.
int32_t FunctionSelector::eh_filter_value(lowir::InstId id, const Instruction& inst)
{
	for (const std::pair<uint32_t, int32_t>& known : eh_filter_values_)
	{
		if (known.first == id)
			return known.second;
	}
	std::vector<uint8_t>& specs = out_->eh.specs;
	const int32_t value = -1 - static_cast<int32_t>(specs.size());
	for (uint32_t k = 0; k < inst.extra_count; ++k)
	{
		const uint32_t type = body_->operands[inst.extra_first + k].index;
		uint32_t index = 0;
		if (const uint32_t* known = eh_type_index_.find(type))
			index = *known;
		else
		{
			out_->eh.types.push_back(type);
			index = static_cast<uint32_t>(out_->eh.types.size());
			eh_type_index_.insert(type, index);
		}
		for (uint32_t rest = index;;)
		{
			const uint8_t byte = static_cast<uint8_t>(rest & 0x7f);
			rest >>= 7;
			specs.push_back(static_cast<uint8_t>(byte | (rest != 0 ? 0x80 : 0)));
			if (rest == 0)
				break;
		}
	}
	specs.push_back(0);
	eh_filter_values_.push_back(std::make_pair(id, value));
	return value;
}

// The leading clauses of landing `block` as chain entries (type indexes, 0
// for a cleanup clause, a specification's negative filter); whether one is
// a catch-all.
void FunctionSelector::eh_landing_clauses(uint32_t block, std::vector<int32_t>& filters, bool& catch_all)
{
	const lowir::FunctionBody& body = *body_;
	filters.clear();
	catch_all = false;
	for (lowir::InstId i = body.blocks[blocks_[block].id].first; i != kNoId; i = body.instructions[i].next)
	{
		const Instruction& inst = body.instructions[i];
		if (!host_eh::clause(inst))
			break;
		if (inst.op == Opcode::EhFilter)
			filters.push_back(eh_filter_value(i, inst));
		else if (inst.op == Opcode::EhCleanup)
			filters.push_back(0);
		else
			filters.push_back(static_cast<int32_t>(eh_clause_index(inst)));
		catch_all = catch_all || inst.op == Opcode::EhCatchAll;
	}
}

// Whether the clause selectors can be the type indexes: one type per
// selector of the landings whose clauses chains hold (those with a table
// entry and the inactive records below them), in a table not much larger
// than the clauses.
void FunctionSelector::eh_choose_indexes()
{
	const lowir::FunctionBody& body = *body_;
	std::vector<uint8_t>& chained = eh_chained_;
	chained.assign(blocks_.size(), 0);
	for (uint32_t b = 0; b < blocks_.size(); ++b)
		if (eh_landing_index_[b] != kNoId)
			for (uint32_t k = eh_landing_node_[b]; k != 0 && !chained[eh_nodes_[k].landing];
				k = eh_nodes_[eh_nodes_[k].parent].protect)
				chained[eh_nodes_[k].landing] = 1;
	std::vector<uint32_t>& by_selector = eh_seen_;
	by_selector.clear();
	uint32_t clauses = 0;
	bool filtered = false;
	eh_identity_ = true;
	eh_filter_values_.clear();
	for (uint32_t b = 0; b < blocks_.size(); ++b)
	{
		if (!chained[b])
			continue;
		for (lowir::InstId i = body.blocks[blocks_[b].id].first; i != kNoId; i = body.instructions[i].next)
		{
			const Instruction& inst = body.instructions[i];
			if (!host_eh::clause(inst))
				break;
			// A specification's types take indexes of their own.
			if (inst.op == Opcode::EhFilter)
				filtered = true;
			if (inst.op != Opcode::EhCatch && inst.op != Opcode::EhCatchAll)
				continue;
			if (inst.immediate < 1 || inst.immediate > host_eh::kMaxSelector)
				eh_fail("a catch selector outside 1..65536");
			const uint32_t selector = static_cast<uint32_t>(inst.immediate);
			const uint32_t type = inst.op == Opcode::EhCatch ? inst.ops[0].index : kCatchAllType;
			++clauses;
			if (selector >= by_selector.size())
				by_selector.resize(selector + 1, kNoId);
			if (by_selector[selector] == kNoId)
				by_selector[selector] = type;
			else if (by_selector[selector] != type)
				eh_identity_ = false;
		}
	}
	if (by_selector.size() > 4 * clauses + 17 || filtered)
		eh_identity_ = false;
	std::vector<uint32_t>& types = out_->eh.types;
	if (!eh_identity_)
	{
		eh_type_index_.clear(clauses);
		return;
	}
	types.assign(by_selector.empty() ? 0 : by_selector.size() - 1, kNoId);
	for (size_t s = 1; s < by_selector.size(); ++s)
		if (by_selector[s] != kNoId && by_selector[s] != kCatchAllType)
			types[s - 1] = by_selector[s];
}

// The chain of inactive node `n` (memoized), after the chains of the
// inactive records below it.
void FunctionSelector::eh_build_chain(uint32_t n)
{
	std::vector<uint32_t>& pending = eh_pending_;
	pending.clear();
	for (uint32_t k = n; k != 0 && eh_nodes_[k].chain_count == kNoId; k = eh_nodes_[eh_nodes_[k].parent].protect)
		pending.push_back(k);
	std::vector<int32_t>& own = eh_own_;
	for (size_t p = pending.size(); p-- > 0;)
	{
		const uint32_t k = pending[p];
		bool catch_all = false;
		eh_landing_clauses(eh_nodes_[k].landing, own, catch_all);
		if (own.empty() || (eh_nodes_[k].cleanup && !catch_all))
			own.push_back(0);
		// Each index once, one cleanup, nothing after a catch-all.
		eh_stamps_.resize(out_->eh.types.size() + 1, 0);
		const uint32_t stamp = ++eh_stamp_;
		bool cleanup = false;
		const uint32_t first = static_cast<uint32_t>(eh_chains_.size());
		auto add = [&](int32_t filter) {
			if (filter == 0)
			{
				if (!cleanup)
					eh_chains_.push_back(0);
				cleanup = true;
				return;
			}
			if (filter < 0)
			{
				if (std::find(eh_chains_.begin() + first, eh_chains_.end(), filter) == eh_chains_.end())
					eh_chains_.push_back(filter);
				return;
			}
			if (eh_stamps_[filter] == stamp)
				return;
			eh_stamps_[filter] = stamp;
			eh_chains_.push_back(filter);
		};
		for (size_t c = 0; c < own.size(); ++c)
			add(own[c]);
		const uint32_t below = eh_nodes_[eh_nodes_[k].parent].protect;
		bool ended = catch_all;
		if (below != 0 && !catch_all)
		{
			const EhNode& next = eh_nodes_[below];
			for (uint32_t c = 0; c < next.chain_count; ++c)
			{
				const int32_t filter = eh_chains_[next.chain + c];
				add(filter);
				ended = ended || (filter > 0 && out_->eh.types[filter - 1] == kNoId);
			}
		}
		// A landing the personality may pass by (no catch-all) is still
		// entered when an exception it does not catch has work there.
		if (!cleanup && !ended && eh_unmatched_work(eh_nodes_[k].landing))
			add(0);
		eh_nodes_[k].chain = first;
		eh_nodes_[k].chain_count = static_cast<uint32_t>(eh_chains_.size()) - first;
	}
}

// Whether an exception no clause of the landing at `block` catches (its
// selector 0) meets work before it leaves the frame: a call, a store or
// another effect, a return, a mid-landing clause (a terminate forwarding)
// on the paths from the landing, following a resume into a landing of the
// frame and only the no-match edges of selector compares. Memoized per
// block (eh_work_: 0 unknown, 1 visiting, 2 none, 3 work).
bool FunctionSelector::eh_unmatched_work(uint32_t block)
{
	const lowir::FunctionBody& body = *body_;
	uint8_t& state = eh_work_[block];
	if (state != 0)
		return state == 3;
	state = 1;
	bool work = false;
	uint32_t selector = kNoId;
	bool leading = eh_landing_node_[block] != kNoId;
	for (lowir::InstId i = body.blocks[blocks_[block].id].first; i != kNoId && !work; i = body.instructions[i].next)
	{
		const Instruction& inst = body.instructions[i];
		if (!host_eh::clause(inst))
			leading = false;
		switch (inst.op)
		{
		case Opcode::Call:
		case Opcode::Store:
		case Opcode::AtomicStore:
		case Opcode::AtomicAddFetch:
		case Opcode::AtomicExchange:
		case Opcode::AtomicCompareExchange:
		case Opcode::CopyObject:
		case Opcode::ZeroInit:
		case Opcode::VaStart:
		case Opcode::Return:
		case Opcode::Throw:
			work = true;
			break;
		case Opcode::EhCatchAll:
			work = !leading;
			break;
		case Opcode::ExceptionSelector:
			selector = inst.result;
			break;
		case Opcode::Resume:
			if (eh_sites_[i] != 0)
				work = eh_unmatched_work(eh_sites_[i] - 1);
			break;
		case Opcode::Jump:
			if (eh_sites_[i] == kResumeTerminal)
				break;
			if (eh_sites_[i] != 0)
			{
				work = eh_unmatched_work(eh_sites_[i] - 1);
				break;
			}
			// fall through
		case Opcode::Switch:
			for (uint32_t k = succ_first_[block]; k < succ_first_[block + 1] && !work; ++k)
				work = eh_unmatched_work(succs_[k]);
			break;
		case Opcode::Branch:
		{
			uint64_t yes = 1;
			uint64_t no = 1;
			eh_refine(inst, selector, yes, no);
			work = (yes != 0 && eh_unmatched_work(layout_of_[inst.ops[1].index])) ||
				(no != 0 && eh_unmatched_work(layout_of_[inst.ops[2].index]));
			break;
		}
		default:
			break;
		}
	}
	eh_work_[block] = work ? 3 : 2;
	return work;
}

// Landing `index` (block `b`): its chain and selector translation.
void FunctionSelector::eh_build_landing(uint32_t b, uint32_t index)
{
	const lowir::FunctionBody& body = *body_;
	const uint32_t node = eh_landing_node_[b];
	eh_build_chain(node);
	EhTables& eh = out_->eh;
	EhLanding& landing = eh.landings[index];
	landing.pad = kNoId;
	landing.select = kNoId;
	landing.first = static_cast<uint32_t>(eh.filters.size());
	landing.count = eh_nodes_[node].chain_count;
	for (uint32_t c = 0; c < landing.count; ++c)
		eh.filters.push_back(eh_chains_[eh_nodes_[node].chain + c]);
	// (filter, selector) pairs of the leading catch clauses, the first per
	// filter.
	landing.map_first = static_cast<uint32_t>(eh.map.size());
	eh_stamps_.resize(eh.types.size() + 1, 0);
	const uint32_t stamp = ++eh_stamp_;
	bool translate = eh_mid_clauses_;
	for (lowir::InstId i = body.blocks[blocks_[b].id].first; i != kNoId; i = body.instructions[i].next)
	{
		const Instruction& inst = body.instructions[i];
		if (!host_eh::clause(inst))
			break;
		if (inst.op == Opcode::EhFilter)
		{
			// A specification's violation leaves its clause's selector: the
			// filter itself when the clause names it (the frontend's guard).
			const int32_t filter = eh_filter_value(i, inst);
			eh.map.push_back(filter);
			eh.map.push_back(static_cast<int32_t>(inst.immediate));
			translate = translate || filter != inst.immediate;
			continue;
		}
		if (inst.op != Opcode::EhCatch && inst.op != Opcode::EhCatchAll)
			continue;
		const uint32_t filter = eh_clause_index(inst);
		if (eh_stamps_[filter] == stamp)
			continue;
		eh_stamps_[filter] = stamp;
		eh.map.push_back(static_cast<int32_t>(filter));
		eh.map.push_back(static_cast<int32_t>(inst.immediate));
		translate = translate || static_cast<int64_t>(filter) != inst.immediate;
	}
	landing.map_count = (static_cast<uint32_t>(eh.map.size()) - landing.map_first) / 2;
	for (uint32_t c = 0; c < landing.count && !translate; ++c)
		translate = eh.filters[landing.first + c] > 0 && eh_stamps_[eh.filters[landing.first + c]] != stamp;
	eh_translate_[index] = translate;
}

void FunctionSelector::eh_build_tables(uint32_t landings)
{
	const uint32_t n = static_cast<uint32_t>(blocks_.size());
	// An edge into a landing whose clauses would match again: only a
	// catch-all first, whose selector the jump or the conditional edge's
	// own block sets (eh_entry_selector, eh_entry_edge).
	for (uint32_t b = 0; b < n; ++b)
		if (blocks_[b].reachable && eh_landing_node_[b] != kNoId && eh_entry_selector(b) == 0 &&
			(eh_landing_mask(b) & ~static_cast<uint64_t>(1)) != 0)
			for (uint32_t k = preds_first_[b]; k < preds_first_[b + 1]; ++k)
				if (eh_in_[preds_[k]] != kNoId)
					eh_fail("a jump into a landing with catch clauses");
	eh_chains_.clear();
	eh_work_.assign(n, 0);
	eh_choose_indexes();
	eh_stamps_.assign(out_->eh.types.size() + 1, 0);
	eh_stamp_ = 0;
	eh_translate_.assign(landings, 0);
	out_->eh.landings.resize(landings);
	for (uint32_t b = 0; b < n; ++b)
		if (eh_landing_index_[b] != kNoId)
			eh_build_landing(b, eh_landing_index_[b]);
	// A translating pad leaves a selector other than the filter: a resume
	// into another landing of the frame then recomputes that landing's from
	// the raw filter every pad keeps.
	eh_raw_ = false;
	for (uint32_t l = 0; l < landings; ++l)
		eh_raw_ = eh_raw_ || eh_translate_[l] != 0;
	out_->eh.slot = frame_alloc(eh_raw_ ? 24 : 16, 8);
}

// Whether landing block `b` (landing `index`) has a selector block: it
// translates, or a resume enters it after another pad translated.
bool FunctionSelector::eh_selects(uint32_t b, uint32_t index) const
{
	return eh_translate_[index] != 0 || (eh_raw_ && eh_resumed_[b] != 0);
}

// A landing's pad (the exception object; the filter as the selector, the
// raw filter, or both) when a call names it, and its selector block, ahead
// of its block.
void FunctionSelector::emit_landing_pad(uint32_t b)
{
	location_ = 0;
	const uint32_t index = eh_landing_index_[b];
	const int32_t slot = out_->eh.slot;
	const bool translate = eh_translate_[index] != 0;
	if (pad_mir_[b] != kNoId)
	{
		begin_mir_block(lowir::own_name(body_->blocks[blocks_[b].id]), kNoId, kNoId);
		out_->blocks.back().landing = true;
		Inst& pad = emit2(
			Op::Landing, TypeKind::Void, Operand::make_frame(slot), Operand::make_frame(slot + (eh_raw_ ? 16 : 8)));
		if (eh_raw_ && !translate)
		{
			pad.ops[2] = Operand::make_frame(slot + 8);
			pad.nops = 3;
		}
		emit1(Op::Jmp, TypeKind::Void, Operand::make_label(translate ? select_mir_[b] : blocks_[b].mir));
		end_mir_block();
	}
	if (select_mir_[b] == kNoId)
		return;
	begin_mir_block(lowir::own_name(body_->blocks[blocks_[b].id]), kNoId, kNoId);
	out_->blocks.back().landing = true;
	Inst& select = emit2(Op::EhSelect, TypeKind::Void, Operand::make_frame(slot + 8), Operand::make_frame(slot + 16));
	select.aux = index;
	emit1(Op::Jmp, TypeKind::Void, Operand::make_label(blocks_[b].mir));
	end_mir_block();
}

} // namespace native
