#include <algorithm>

#include "lowir/builder.h"
#include "opt/loops.h"
#include "opt/passes.h"

namespace opt {

using namespace lowir;

namespace {

// The most fields one slot splits into (bounded growth: every copy or
// zeroing of the slot expands to one access per field).
const uint32_t kMaxFields = 16;

// A temporary's address: a slot plus a constant byte offset.
struct SlotAddress
{
	SlotId slot;
	int64_t offset;
};

struct Field
{
	uint64_t offset;
	Type type;
	bool packed; // an eightbyte's integer: its loads take their bits from it
};

struct AccessRange
{
	uint64_t first, end;
	bool indexed;
};

struct SlotFacts
{
	bool candidate = true;
	bool partial = false;                               // fixed fields split; untiled bytes keep their original storage
	bool call_written = false;                          // a call returns its object into the slot
	bool copied_out = false;                            // a copy reads the slot whole
	std::vector<Field> fields;                          // typed accesses
	std::vector<uint64_t> recurrent;                    // fields written on every pass of a loop
	std::vector<std::pair<uint64_t, uint64_t>> indexed; // ranges accessed through bounded variable indices
	std::vector<std::pair<uint64_t, uint64_t>> spans;   // copied or zeroed [offset, end)
	std::vector<std::pair<uint64_t, uint64_t>> stores;  // stored fields [offset, end)
	std::vector<std::pair<uint64_t, uint64_t>> reads;   // copied out whole [offset, end)
	std::vector<std::pair<uint64_t, uint64_t>> writes;  // stored, copied into, zeroed or a call's result [offset, end)
	std::vector<std::pair<uint64_t, bool>> wholes;      // i64 accesses at eightbyte starts (true: loads)
	uint64_t packed = 0;                                // eightbytes that move only whole (bit per eightbyte)
	uint64_t whole = 0;                                 // eightbytes that are one integer of their width
	uint64_t loose = 0;                                 // eightbytes that pack when a copy partner's is whole
	uint64_t forced = 0;                                // eightbytes an i64 access reads or writes whole
};

// A copy between two slots of this function: each side's typed fields
// suggest the other's fillers (a copy expanded in chunks wider than the
// fields its source was just written by reads it back across them: a
// store-forwarding stall per copy).
struct SlotCopy
{
	SlotAddress from, to;
	uint64_t bytes;
};

// An integer type of `bytes` (1, 2, 4 or 8).
Type filler_type(uint64_t bytes)
{
	return Type::scalar(bytes == 1 ? TypeKind::U8
			: bytes == 2           ? TypeKind::U16
			: bytes == 4           ? TypeKind::U32
								   : TypeKind::I64);
}

// A short zero write has the same byte coverage as a scalar store.
bool scalar_zero(const Instruction& inst)
{
	return inst.op == Opcode::ZeroInit && !(inst.flags & IF_VOLATILE) &&
		(inst.immediate == 1 || inst.immediate == 2 || inst.immediate == 4 || inst.immediate == 8);
}

class AggregateSlots
{
public:
	explicit AggregateSlots(FunctionContext& context) : c_(context), body_(context.body) {}

	bool run();

private:
	bool objects_through_slots();
	bool address_of(Value v, SlotAddress& out) const;
	uint64_t address_extent(Value v) const { return !v.is(ValueKind::Temp) || extent_.empty() ? 0 : extent_[v.index]; }
	void escape(Value v);
	void analyze();
	bool scalarize_partial_copies();
	void mark_recurrent_fields();
	void access(Value storage, Type type, bool store);
	void span(Value storage, uint64_t bytes, bool read = false);
	bool settle(SlotFacts& facts, uint64_t size, SlotId slot);
	uint64_t packable(SlotFacts& facts, uint64_t size) const;
	void agree_packing(const std::vector<uint8_t>& settled);
	void pack(SlotFacts& facts) const;
	uint32_t packed_field(SlotId slot, uint64_t offset) const;
	void load_packed(BlockId block, InstId& prev, InstId id, SlotId slot, uint64_t offset);
	void store_packed(BlockId block, InstId& prev, InstId id, SlotId slot, uint64_t offset);
	Type hinted_filler(SlotId slot, uint64_t gap, uint64_t end) const;
	Value field_slot(SlotId slot, uint64_t offset) const;
	void rewrite();
	bool compatible_copies();
	bool share_floats(const SlotAddress& x, const SlotAddress& y, uint64_t bytes);
	bool same_fields(const SlotAddress& x, const SlotAddress& y, uint64_t bytes) const;
	void expand_copy(BlockId block, InstId prev, const Instruction& inst, InstId& last);
	InstId emit(BlockId block, InstId after, Instruction inst, Type result);
	Value pointer_at(BlockId block, InstId& after, Value base, int64_t offset, uint32_t location);

	FunctionContext& c_;
	FunctionBody& body_;
	std::vector<SlotAddress> address_; // per temp (slot kNoId: none)
	std::vector<uint64_t> extent_;     // maximum additional offset of a bounded address
	std::vector<SlotFacts> facts_;     // per slot
	std::vector<SlotCopy> copies_;
	bool partial_copy_ = false;
	std::vector<uint32_t> first_field_; // per slot: first new slot of its fields (kNoId: kept)
};

bool AggregateSlots::address_of(Value v, SlotAddress& out) const
{
	if (v.is(ValueKind::Slot))
	{
		out = SlotAddress{ v.index, 0 };
		return true;
	}
	if (v.is(ValueKind::Temp) && v.index < address_.size() && address_[v.index].slot != kNoId)
	{
		out = address_[v.index];
		return true;
	}
	return false;
}

// A use the analysis does not model: the slot behind `v` keeps its storage.
void AggregateSlots::escape(Value v)
{
	SlotAddress a;
	if (address_of(v, a))
		facts_[a.slot].candidate = false;
	else if (v.is(ValueKind::Slot))
		facts_[v.index].candidate = false;
}

void AggregateSlots::access(Value storage, Type type, bool store)
{
	SlotAddress a;
	if (storage.is(ValueKind::Slot))
	{
		a.slot = storage.index;
		a.offset = 0;
	}
	else if (!address_of(storage, a))
		return;
	if (type.is_object() || a.offset < 0)
	{
		facts_[a.slot].candidate = false;
		return;
	}
	const uint64_t extent = storage.is(ValueKind::Temp) ? address_extent(storage) : 0;
	if (extent != 0)
	{
		const uint64_t end = static_cast<uint64_t>(a.offset) + extent + storage_bytes(type);
		if (end > storage_bytes(body_.slots[a.slot].type))
			facts_[a.slot].candidate = false;
		else
			facts_[a.slot].indexed.push_back(std::make_pair(static_cast<uint64_t>(a.offset), end));
		return;
	}
	facts_[a.slot].fields.push_back(Field{ static_cast<uint64_t>(a.offset), type, false });
	if (type.kind() == TypeKind::I64 && a.offset % 8 == 0)
		facts_[a.slot].wholes.push_back(std::make_pair(static_cast<uint64_t>(a.offset), !store));
	if (store)
	{
		facts_[a.slot].stores.push_back(
			std::make_pair(static_cast<uint64_t>(a.offset), a.offset + storage_bytes(type)));
		facts_[a.slot].writes.push_back(facts_[a.slot].stores.back());
	}
}

void AggregateSlots::span(Value storage, uint64_t bytes, bool read)
{
	SlotAddress a;
	if (storage.is(ValueKind::Slot))
	{
		a.slot = storage.index;
		a.offset = 0;
	}
	else if (!address_of(storage, a))
		return;
	if (a.offset < 0 || (storage.is(ValueKind::Temp) && address_extent(storage) != 0))
	{
		facts_[a.slot].candidate = false;
		return;
	}
	facts_[a.slot].spans.push_back(std::make_pair(static_cast<uint64_t>(a.offset), a.offset + bytes));
	(read ? facts_[a.slot].reads : facts_[a.slot].writes).push_back(facts_[a.slot].spans.back());
}

void AggregateSlots::analyze()
{
	address_.assign(body_.temps.size(), SlotAddress{ kNoId, 0 });
	extent_.clear();
	facts_.assign(body_.slots.size(), SlotFacts());
	copies_.clear();
	partial_copy_ = false;
	for (BlockId b : body_.layout)
		for (InstId i = body_.blocks[b].first; i != kNoId; i = body_.instructions[i].next)
		{
			const Instruction& inst = body_.instructions[i];
			SlotAddress a;
			switch (inst.op)
			{
			case Opcode::Addr:
				if (inst.ops[0].is(ValueKind::Slot))
					address_[inst.result] = SlotAddress{ inst.ops[0].index, 0 };
				continue;
			case Opcode::Copy:
				if (inst.type.is_pointer() && address_of(inst.ops[0], a))
				{
					address_[inst.result] = a;
					if (!extent_.empty())
						extent_[inst.result] = address_extent(inst.ops[0]);
					continue;
				}
				break;
			case Opcode::Index:
				if (address_of(inst.ops[0], a))
				{
					const uint64_t element = storage_bytes(inst.type);
					uint64_t extent = address_extent(inst.ops[0]);
					if (inst.ops[1].is(ValueKind::Integer))
					{
						const Literal bits = body_.literals[inst.ops[1].index];
						const __int128 offset = a.offset +
							static_cast<__int128>(static_cast<int64_t>(bits.low)) * element;
						if (offset < 0 || offset > static_cast<__int128>(storage_bytes(body_.slots[a.slot].type)))
							break;
						a.offset = static_cast<int64_t>(offset);
					}
					else
					{
						const uint64_t bound = index_upper_bound(body_, inst.ops[1]);
						if (bound >= (uint64_t(1) << 32) || element >= (uint64_t(1) << 16))
							break;
						extent += bound * element;
						if (a.offset < 0 ||
							static_cast<uint64_t>(a.offset) + extent > storage_bytes(body_.slots[a.slot].type))
							break;
					}
					address_[inst.result] = a;
					if (extent != 0)
					{
						if (extent_.empty())
							extent_.assign(body_.temps.size(), 0);
						extent_[inst.result] = extent;
					}
					continue;
				}
				break;
			case Opcode::Load:
				if ((inst.flags & IF_VOLATILE) != 0)
					break;
				access(inst.ops[0], inst.type, false);
				continue;
			case Opcode::Store:
				if ((inst.flags & IF_VOLATILE) != 0)
					break;
				escape(inst.ops[0]);
				access(inst.ops[1], inst.type, true);
				continue;
			case Opcode::CopyObject:
				// A source that is an object value: a call's result copies
				// into a split destination through a temporary of its own
				// (expand_copy); a parameter's keeps the destination (the
				// object arrived whole in registers and usually leaves
				// whole: a field-wise copy read back whole stalls).
				if (inst.ops[0].is(ValueKind::Temp) && body_.temps[inst.ops[0].index].type.is_object())
				{
					if (inst.ops[0].index < body_.parameter_count)
						escape(inst.ops[1]);
					else
						span(inst.ops[1], static_cast<uint64_t>(inst.immediate));
					continue;
				}
				span(inst.ops[0], static_cast<uint64_t>(inst.immediate), true);
				span(inst.ops[1], static_cast<uint64_t>(inst.immediate));
				{
					SlotAddress whole;
					if (inst.ops[0].is(ValueKind::Slot))
						facts_[inst.ops[0].index].copied_out = true;
					else if (address_of(inst.ops[0], whole))
						facts_[whole.slot].copied_out = true;
					SlotCopy copy = { { kNoId, 0 }, { kNoId, 0 }, static_cast<uint64_t>(inst.immediate) };
					if (inst.ops[0].is(ValueKind::Slot))
						copy.from = SlotAddress{ inst.ops[0].index, 0 };
					else
						address_of(inst.ops[0], copy.from);
					if (inst.ops[1].is(ValueKind::Slot))
						copy.to = SlotAddress{ inst.ops[1].index, 0 };
					else
						address_of(inst.ops[1], copy.to);
					// Reuse the addresses already found for copy compatibility.
					// The length can be a whole word while either endpoint cuts
					// a carrier (for example eight bytes copied at offset four).
					const auto partial = [&](const SlotAddress& at) {
						return at.slot != kNoId && at.offset >= 0 &&
							((static_cast<uint64_t>(at.offset) | copy.bytes) % 8 != 0);
					};
					partial_copy_ = partial_copy_ ||
						(!(inst.flags & IF_VOLATILE) && inst.immediate > 0 && copy.bytes <= 16 &&
							(partial(copy.from) || partial(copy.to)));
					if (copy.from.slot != kNoId && copy.to.slot != kNoId && copy.from.slot != copy.to.slot)
						copies_.push_back(copy);
				}
				continue;
			case Opcode::ZeroInit:
				if (scalar_zero(inst))
					access(inst.ops[0], filler_type(static_cast<uint64_t>(inst.immediate)), true);
				else
					span(inst.ops[0], static_cast<uint64_t>(inst.immediate));
				continue;
			case Opcode::Phi:
				continue; // after every address is known (an operand may be defined later in layout)
			case Opcode::Call:
			{
				// A slot a call returns its object into (indirect_result)
				// is written whole there: its fields read it back after
				// the call (rewrite), as a copy from an object value.
				const Signature* sig = c_.program.call_boundary(inst);
				const Parameter* params = sig != nullptr ? c_.program.parameters_of(*sig) : nullptr;
				for (unsigned k = 0; k < 3; ++k)
					escape(inst.ops[k]);
				for (uint32_t k = 0; k < inst.extra_count; ++k)
				{
					const Value v = body_.operands[inst.extra_first + k];
					SlotAddress r = { kNoId, 0 };
					const bool result = params != nullptr && k < sig->parameter_count &&
						params[k].pass == ir_model::PPM_INDIRECT_RESULT && !v.is(ValueKind::Slot) && address_of(v, r) &&
						r.offset == 0 && body_.slots[r.slot].type.is_object();
					if (result)
					{
						span(v, storage_bytes(body_.slots[r.slot].type));
						facts_[r.slot].call_written = true;
					}
					else
						escape(v);
				}
				continue;
			}
			default:
				break;
			}
			for_each_operand(body_, const_cast<Instruction&>(inst), [&](Value& v) { escape(v); });
		}
	for (BlockId b : body_.layout)
		for (InstId i = body_.blocks[b].first; i != kNoId && body_.instructions[i].op == Opcode::Phi;
			i = body_.instructions[i].next)
			for_each_operand(body_, body_.instructions[i], [&](Value& v) { escape(v); });
}

// Splitting every field of a large copied object can turn cold state into
// loop-carried register pressure. Split only fixed fields updated on every
// pass; the other bytes continue to use the original object's bulk transfers.
void AggregateSlots::mark_recurrent_fields()
{
	bool any = false;
	for (SlotId s = 0; s < facts_.size(); ++s)
		any = any ||
			(facts_[s].candidate && !facts_[s].spans.empty() && storage_bytes(body_.slots[s].type) > 8 * kMaxFields);
	if (!any)
		return;
	Cfg& cfg = c_.ensure_cfg();
	LoopForest forest;
	find_loops(body_, cfg, forest);
	for (const Loop& loop : forest.loops)
	{
		if (loop.latches.size() != 1)
			continue;
		for (BlockId b : loop.blocks)
		{
			if (!cfg.dominates(b, loop.latches[0]))
				continue;
			for (InstId i = body_.blocks[b].first; i != kNoId; i = body_.instructions[i].next)
			{
				const Instruction& inst = body_.instructions[i];
				if (inst.op != Opcode::Store || (inst.flags & IF_VOLATILE) != 0)
					continue;
				SlotAddress at = { kNoId, 0 };
				if (inst.ops[1].is(ValueKind::Slot))
					at.slot = inst.ops[1].index;
				else if (!address_of(inst.ops[1], at) || address_extent(inst.ops[1]) != 0)
					continue;
				SlotFacts& facts = facts_[at.slot];
				if (facts.candidate && !facts.spans.empty() && at.offset >= 0 &&
					storage_bytes(body_.slots[at.slot].type) > 8 * kMaxFields)
				{
					const uint64_t offset = static_cast<uint64_t>(at.offset);
					if (std::find(facts.recurrent.begin(), facts.recurrent.end(), offset) == facts.recurrent.end())
					{
						if (facts.recurrent.size() == kMaxFields)
							facts.candidate = false;
						else
							facts.recurrent.push_back(offset);
					}
				}
			}
		}
	}
}

// Fields: the typed accesses, which must agree where they meet, tiled
// with integer fillers across every copied or zeroed span.
// A filler at `gap` (below `end`) in `slot`: a typed field that a copy
// partner has there (its type, so copies between the two match field for
// field), else void.
Type AggregateSlots::hinted_filler(SlotId slot, uint64_t gap, uint64_t end) const
{
	for (const SlotCopy& copy : copies_)
	{
		const bool from = copy.from.slot == slot;
		if (!from && copy.to.slot != slot)
			continue;
		const SlotAddress& self = from ? copy.from : copy.to;
		const SlotAddress& other = from ? copy.to : copy.from;
		if (static_cast<int64_t>(gap) < self.offset ||
			static_cast<int64_t>(gap) >= self.offset + static_cast<int64_t>(copy.bytes))
			continue;
		const int64_t at = other.offset + static_cast<int64_t>(gap) - self.offset;
		for (const Field& f : facts_[other.slot].fields)
		{
			const uint64_t bytes = storage_bytes(f.type);
			if (static_cast<int64_t>(f.offset) == at && (bytes <= 8 || f.type.is_vector()) &&
				(bytes & (bytes - 1)) == 0 && gap + bytes <= end && gap % bytes == 0 &&
				f.type.kind() != TypeKind::F80 && !f.type.is_object())
				return f.type;
		}
	}
	return Type::scalar(TypeKind::Void);
}

// Overlapping accesses share one integer carrier per eightbyte: integers,
// pointers, and floating values as their bits (a value that a short write
// also reaches). This is representation traffic, so the original loads and
// stores become extracts and inserts before ordinary scalar promotion.
// Copies crossing a carrier's edge, vector and wider fields and indexed
// storage retain their memory.
bool widen_integer_fields(SlotFacts& facts, uint64_t size)
{
	uint64_t needed = 0;
	for (size_t i = 0; i < facts.fields.size(); ++i)
	{
		const Field& f = facts.fields[i];
		const uint64_t end = f.offset + storage_bytes(f.type);
		for (size_t j = i + 1; j < facts.fields.size() && facts.fields[j].offset < end; ++j)
		{
			const Field& g = facts.fields[j];
			if (f.offset < g.offset + storage_bytes(g.type) &&
				!(f.offset == g.offset &&
					(f.type == g.type || float_twins(f.type, g.type) || integer_twins(f.type, g.type))))
			{
				if (f.offset / 8 >= 64 || g.offset / 8 >= 64)
					return false;
				needed |= (uint64_t(1) << (f.offset / 8)) | (uint64_t(1) << (g.offset / 8));
			}
		}
	}
	if (needed == 0)
		return true;
	for (uint32_t chunk = 0; chunk < 64; ++chunk)
	{
		if (((needed >> chunk) & 1) == 0)
			continue;
		const uint64_t start = uint64_t(chunk) * 8, end = std::min(start + 8, size), width = end - start;
		if (start >= size || (width != 2 && width != 4 && width != 8))
			return false;
		for (const Field& f : facts.fields)
			if (f.offset < end && f.offset + storage_bytes(f.type) > start &&
				(!(f.type.is_integer() || f.type.is_pointer() || f.type.kind() == TypeKind::F32 ||
					 f.type.kind() == TypeKind::F64) ||
					f.offset < start || f.offset + storage_bytes(f.type) > end))
				return false;
		for (const auto& span : facts.spans)
			if (span.first < end && span.second > start && !(span.first <= start && span.second >= end))
				return false;
	}
	std::vector<Field> fields;
	for (const Field& f : facts.fields)
		if (f.offset / 8 >= 64 || ((needed >> (f.offset / 8)) & 1) == 0)
			fields.push_back(f);
	for (uint32_t chunk = 0; chunk < 64; ++chunk)
		if (((needed >> chunk) & 1) != 0)
		{
			const uint64_t start = uint64_t(chunk) * 8;
			fields.push_back(Field{ start, filler_type(std::min(uint64_t(8), size - start)), true });
		}
	facts.fields.swap(fields);
	facts.forced |= needed;
	return true;
}

bool AggregateSlots::settle(SlotFacts& facts, uint64_t size, SlotId slot)
{
	std::vector<Field>& fields = facts.fields;
	// Access frequency does not change the storage shape. Collapse repeated
	// descriptors before testing overlap, and inspect only intersecting ranges.
	// Scalar widths and the finite type vocabulary bound each overlap window.
	std::sort(fields.begin(), fields.end(), [](const Field& x, const Field& y) {
		return x.offset < y.offset || (x.offset == y.offset && x.type.kind() < y.type.kind());
	});
	fields.erase(std::unique(fields.begin(), fields.end(),
					 [](const Field& x, const Field& y) { return x.offset == y.offset && x.type == y.type; }),
		fields.end());
	facts.partial = !facts.spans.empty() && size > 8 * kMaxFields;
	if (facts.partial && facts.recurrent.empty())
		return false;
	if (!facts.partial && !facts.spans.empty() && !facts.indexed.empty())
		return false;
	if (!facts.indexed.empty())
	{
		// Indexed regions keep their original storage. Whole-object copies
		// move those residual bytes separately from disjoint scalar fields.
		// Close overlaps with fixed accesses too: a wide fixed access
		// straddling an indexed region must keep all of its bytes coherent.
		std::vector<AccessRange> ranges;
		for (const auto& range : facts.indexed)
			ranges.push_back(AccessRange{ range.first, range.second, true });
		for (const Field& field : fields)
		{
			const uint64_t end = field.offset + storage_bytes(field.type);
			if (end > size || end < field.offset)
				return false;
			ranges.push_back(AccessRange{ field.offset, end, false });
		}
		std::sort(
			ranges.begin(), ranges.end(), [](const AccessRange& x, const AccessRange& y) { return x.first < y.first; });
		uint32_t count = 0;
		for (const AccessRange range : ranges)
			if (count != 0 && range.first < ranges[count - 1].end)
			{
				ranges[count - 1].end = std::max(ranges[count - 1].end, range.end);
				ranges[count - 1].indexed = ranges[count - 1].indexed || range.indexed;
			}
			else
				ranges[count++] = range;
		facts.indexed.clear();
		for (uint32_t k = 0; k < count; ++k)
			if (ranges[k].indexed)
				facts.indexed.push_back(std::make_pair(ranges[k].first, ranges[k].end));
		std::vector<Field> kept;
		for (const Field& field : fields)
		{
			const auto at = std::upper_bound(facts.indexed.begin(), facts.indexed.end(), field.offset,
				[](uint64_t offset, const std::pair<uint64_t, uint64_t>& range) { return offset < range.second; });
			const bool overlap = at != facts.indexed.end() && at->first < field.offset + storage_bytes(field.type);
			if (!overlap)
				kept.push_back(field);
		}
		fields.swap(kept);
		facts.wholes.clear();
	}
	// An i64 access of an eightbyte other fields share (a copy simplified
	// to a load and a store) moves it whole: a copy's span, if the
	// eightbyte packs (pack_forced).
	for (const std::pair<uint64_t, bool>& w : facts.wholes)
	{
		if (facts.partial)
			break;
		bool shared = false;
		for (const Field& f : fields)
			shared = shared ||
				(f.offset < w.first + 8 && f.offset + storage_bytes(f.type) > w.first &&
					!(f.offset == w.first && (f.type.kind() == TypeKind::I64 || f.type.kind() == TypeKind::F64)));
		if (!shared || w.first / 8 >= 64 || w.first + 8 > size)
			continue;
		facts.forced |= static_cast<uint64_t>(1) << (w.first / 8);
	}
	if (facts.forced != 0)
	{
		auto forced = [&](uint64_t offset) { return offset / 8 < 64 && ((facts.forced >> (offset / 8)) & 1) != 0; };
		std::vector<Field> kept;
		for (const Field& f : fields)
			if (!(f.type.kind() == TypeKind::I64 && f.offset % 8 == 0 && forced(f.offset)))
				kept.push_back(f);
		fields.swap(kept);
		std::vector<std::pair<uint64_t, uint64_t>> stores;
		for (const std::pair<uint64_t, uint64_t>& w : facts.stores)
			if (!(w.second - w.first == 8 && w.first % 8 == 0 && forced(w.first)))
				stores.push_back(w);
		facts.stores.swap(stores);
		for (const std::pair<uint64_t, bool>& w : facts.wholes)
			if (forced(w.first))
			{
				facts.spans.push_back(std::make_pair(w.first, w.first + 8));
				if (w.second)
					facts.reads.push_back(std::make_pair(w.first, w.first + 8));
			}
	}
	if (!widen_integer_fields(facts, size))
		return false;
	std::sort(fields.begin(), fields.end(), [](const Field& x, const Field& y) { return x.offset < y.offset; });
	std::vector<Field> unique;
	for (const Field& f : fields)
	{
		const uint64_t end = f.offset + storage_bytes(f.type);
		if (end > size || storage_bytes(f.type) == 0)
			return false;
		if (!unique.empty() && unique.back().offset == f.offset)
		{
			// A signedness-only view needs no wider carrier. Slot promotion
			// retypes each access while keeping independent fields separate.
			if (integer_twins(unique.back().type, f.type))
				continue;
			// (An integer carrying a floating field's bits: the field stays
			// floating, and promotion retypes the integer accesses.)
			if (float_twins(unique.back().type, f.type))
			{
				if (f.type.is_float())
					unique.back().type = f.type;
				continue;
			}
			if (unique.back().type != f.type)
				return false;
			continue;
		}
		if (!unique.empty() && unique.back().offset + storage_bytes(unique.back().type) > f.offset)
			return false;
		unique.push_back(f);
	}
	if (facts.partial)
		unique.erase(std::remove_if(unique.begin(), unique.end(),
						 [&](const Field& f) {
							 return std::find(facts.recurrent.begin(), facts.recurrent.end(), f.offset) ==
								 facts.recurrent.end();
						 }),
			unique.end());
	for (const std::pair<uint64_t, uint64_t>& s : facts.spans)
	{
		if (s.second > size)
			return false;
		// An f80 field's value is 10 of its 16 bytes: a copy or zeroing
		// expanded through it would lose the rest.
		for (const Field& f : unique)
			if (f.type.kind() == TypeKind::F80 && f.offset < s.second && f.offset + 16 > s.first)
				return false;
		if (facts.partial)
		{
			for (const Field& f : unique)
				if (f.offset < s.second && f.offset + storage_bytes(f.type) > s.first &&
					(f.offset < s.first || f.offset + storage_bytes(f.type) > s.second))
					return false;
			continue;
		}
		// Fillers for the bytes of [first, second) no field covers.
		uint64_t at = s.first;
		std::vector<Field> fill;
		for (const Field& f : unique)
		{
			const uint64_t end = f.offset + storage_bytes(f.type);
			if (end <= at || f.offset >= s.second)
				continue;
			if (f.offset < at || end > s.second)
				return false; // a field straddles the span's edge
			for (uint64_t gap = at; gap < f.offset;)
			{
				const Type hint = hinted_filler(slot, gap, f.offset);
				uint64_t bytes = hint.is_void() ? 8 : storage_bytes(hint);
				while (bytes > 1 && (gap % bytes != 0 || gap + bytes > f.offset))
					bytes /= 2;
				fill.push_back(Field{ gap, hint.is_void() ? filler_type(bytes) : hint, false });
				gap += bytes;
				if (unique.size() + fill.size() > kMaxFields)
					return false; // checked as the fillers grow: a span may be huge
			}
			at = end;
		}
		for (uint64_t gap = at; gap < s.second;)
		{
			const Type hint = hinted_filler(slot, gap, s.second);
			uint64_t bytes = hint.is_void() ? 8 : storage_bytes(hint);
			while (bytes > 1 && (gap % bytes != 0 || gap + bytes > s.second))
				bytes /= 2;
			fill.push_back(Field{ gap, hint.is_void() ? filler_type(bytes) : hint, false });
			gap += bytes;
			if (unique.size() + fill.size() > kMaxFields)
				return false;
		}
		unique.insert(unique.end(), fill.begin(), fill.end());
		std::sort(unique.begin(), unique.end(), [](const Field& x, const Field& y) { return x.offset < y.offset; });
	}
	if (unique.empty() || unique.size() > kMaxFields)
		return false;
	fields.swap(unique);
	return true;
}

// Eightbytes of a slot that move whole: written by copies and zeroing
// alone (no field is stored there), or copied out whole and in at least
// three pieces; their fields integers. Each becomes one integer field:
// its fields' loads take their bits from it (a shift and a truncation)
// and their stores put them in (a mask, a shift and an or), so a copy is
// one load and one store per eightbyte instead of one per field
// (lowir::Value's u8, u8, u16 and u32), and narrow fields stored one by
// one never meet a wider read of them (a store-forwarding stall). Two
// fields stored apart and copied in halves (SourceChar's code point and
// offset) stay apart: their copies cost what the inserts would. Fields
// of up to 64 eightbytes; at least two in the eightbyte. Also notes the
// eightbytes that are whole, and the loose ones: integers and pointers
// that stay apart by these rules alone (agree_packing).
uint64_t AggregateSlots::packable(SlotFacts& facts, uint64_t size) const
{
	uint64_t mask = 0;
	for (uint64_t start = 0; start < size && start / 8 < 64; start += 8)
	{
		const uint64_t end = std::min(start + 8, size), width = end - start;
		if (width != 2 && width != 4 && width != 8)
			continue;
		uint32_t inside = 0;
		bool ok = true, integers = true;
		for (const Field& f : facts.fields)
		{
			const uint64_t f_end = f.offset + storage_bytes(f.type);
			if (f_end <= start || f.offset >= end)
				continue;
			const TypeKind k = f.type.kind();
			ok = ok && f.offset >= start && f_end <= end &&
				((k >= TypeKind::I1 && k <= TypeKind::I64) || f.type.is_pointer() || k == TypeKind::F32 ||
					k == TypeKind::F64);
			integers = integers && k != TypeKind::F32 && k != TypeKind::F64;
			++inside;
		}
		bool loose = ok && integers && inside >= 1;
		bool stored = false, read = false;
		for (const std::pair<uint64_t, uint64_t>& w : facts.stores)
			stored = stored || (w.second > start && w.first < end);
		for (const std::pair<uint64_t, uint64_t>& w : facts.reads)
			read = read || (w.first <= start && w.second >= end);
		const bool forced = ((facts.forced >> (start / 8)) & 1) != 0;
		ok = ok && (forced || !stored || (read && inside >= 3));
		for (const std::pair<uint64_t, uint64_t>& w : facts.spans)
		{
			const bool apart = w.second <= start || w.first >= end || (w.first <= start && w.second >= end);
			ok = ok && apart;
			loose = loose && apart;
		}
		// (One field of the eightbyte's own integer type already is its
		// packed form, however it is written: a copy partner packing it
		// agrees.)
		bool whole = false;
		for (const Field& f : facts.fields)
			whole = whole || (f.offset == start && f.type.kind() == filler_type(width).kind());
		const uint64_t bit = static_cast<uint64_t>(1) << (start / 8);
		if (inside == 1 && whole)
		{
			mask |= bit;
			facts.whole |= bit;
		}
		else if (ok && (inside >= 2 || (forced && inside >= 1)))
			mask |= bit;
		else if (loose)
			facts.loose |= bit;
	}
	return mask;
}

// A copy between two split slots moves field to field, so the two pack
// alike: an eightbyte one side packs where the other does not, or across
// the other's eightbyte boundary, unpacks on both. An eightbyte one side
// holds whole has no fields to unpack into, so the other's loose
// eightbyte packs with it (a by-value argument stored a field at a time,
// then read as one integer through the callee's copy), and is whole to
// its own copy partners from then. Repeats until stable: an eightbyte
// packs this way once, and unpacks for good.
void AggregateSlots::agree_packing(const std::vector<uint8_t>& settled)
{
	auto bit = [](uint64_t offset) { return offset / 8 < 64 ? static_cast<uint64_t>(1) << (offset / 8) : 0; };
	for (bool changed = true; changed;)
	{
		changed = false;
		for (const SlotCopy& copy : copies_)
		{
			// A source kept in memory and stored a field at a time: its
			// destination reads it back a field at a time too.
			if (!settled[copy.from.slot] && settled[copy.to.slot])
			{
				const uint64_t before = facts_[copy.to.slot].packed;
				for (const std::pair<uint64_t, uint64_t>& w : facts_[copy.from.slot].stores)
					for (uint64_t a = std::max(w.first, static_cast<uint64_t>(copy.from.offset));
						a < w.second && a < copy.from.offset + copy.bytes; ++a)
						facts_[copy.to.slot].packed &= ~bit(a - copy.from.offset + copy.to.offset);
				changed = changed || facts_[copy.to.slot].packed != before;
				continue;
			}
			if (!settled[copy.from.slot] || !settled[copy.to.slot])
				continue;
			SlotFacts& x = facts_[copy.from.slot];
			SlotFacts& y = facts_[copy.to.slot];
			for (uint64_t rel = 0; rel < copy.bytes; ++rel)
			{
				const uint64_t a = copy.from.offset + rel, b = copy.to.offset + rel;
				const bool px = (x.packed & bit(a)) != 0, py = (y.packed & bit(b)) != 0;
				if (!(px || py) || (px == py && a % 8 == b % 8))
					continue;
				SlotFacts& open = px ? y : x;
				const uint64_t held = px ? x.whole & bit(a) : y.whole & bit(b);
				const uint64_t at = bit(px ? b : a);
				if (px != py && a % 8 == b % 8 && held != 0 && (open.loose & at) != 0)
				{
					open.packed |= at;
					open.whole |= at;
					open.loose &= ~at;
				}
				else
				{
					x.packed &= ~bit(a);
					y.packed &= ~bit(b);
					x.loose &= ~bit(a);
					y.loose &= ~bit(b);
				}
				changed = true;
			}
		}
	}
}

// The packed eightbytes' fields become one integer each.
void AggregateSlots::pack(SlotFacts& facts) const
{
	if (facts.packed == 0)
		return;
	std::vector<Field> fields;
	for (const Field& f : facts.fields)
	{
		const uint64_t start = f.offset / 8 * 8;
		if (start / 8 >= 64 || (facts.packed & (static_cast<uint64_t>(1) << (start / 8))) == 0)
		{
			fields.push_back(f);
			continue;
		}
		if (!fields.empty() && fields.back().packed && fields.back().offset == start)
			continue;
		uint64_t end = start;
		for (const Field& g : facts.fields)
			if (g.offset / 8 * 8 == start)
				end = std::max(end, g.offset + storage_bytes(g.type));
		// The eightbyte's width (packable: 2, 4 or 8 bytes up to the
		// slot's end; the fields reach it when copies cover it).
		uint64_t width = 2;
		while (width < end - start)
			width *= 2;
		fields.push_back(Field{ start, filler_type(width), true });
	}
	facts.fields.swap(fields);
}

// A load of `offset` in a packed field of `slot`: the field's load, shifted
// down to the bytes read and truncated to the load's type (in place of
// instruction `id`, which keeps its result).
// The packed field holding byte `offset` of `slot`.
uint32_t AggregateSlots::packed_field(SlotId slot, uint64_t offset) const
{
	const std::vector<Field>& fields = facts_[slot].fields;
	uint32_t k = 0;
	while (!(fields[k].offset <= offset && offset < fields[k].offset + storage_bytes(fields[k].type)))
		++k;
	return k;
}

void AggregateSlots::load_packed(BlockId block, InstId& prev, InstId id, SlotId slot, uint64_t offset)
{
	const std::vector<Field>& fields = facts_[slot].fields;
	const uint32_t k = packed_field(slot, offset);
	const Instruction original = body_.instructions[id];
	const Type wide = fields[k].type;
	Instruction load = make_instruction(Opcode::Load, wide);
	load.ops[0] = Value::slot(first_field_[slot] + k);
	load.location = original.location;
	prev = emit(block, prev, load, wide);
	Value bits = Value::temp(body_.instructions[prev].result);
	if (offset > fields[k].offset)
	{
		Instruction shift = make_instruction(Opcode::Binary, wide);
		shift.sub = static_cast<uint8_t>(BinaryOp::UShr);
		shift.ops[0] = bits;
		shift.ops[1] = integer_value(body_, integer_literal(static_cast<int64_t>(8 * (offset - fields[k].offset))));
		shift.location = original.location;
		prev = emit(block, prev, shift, wide);
		bits = Value::temp(body_.instructions[prev].result);
	}
	const bool bit_view = original.type.is_pointer() || original.type.kind() == TypeKind::F32 ||
		original.type.kind() == TypeKind::F64;
	if (bit_view && storage_bytes(original.type) < storage_bytes(wide))
	{
		const Type word = filler_type(storage_bytes(original.type));
		Instruction trunc = make_instruction(Opcode::Convert, word);
		trunc.sub = static_cast<uint8_t>(ConvertOp::Trunc);
		trunc.ops[0] = bits;
		set_convert_source_type(trunc, wide);
		trunc.location = original.location;
		prev = emit(block, prev, trunc, word);
		bits = Value::temp(body_.instructions[prev].result);
	}
	Instruction narrow = make_instruction(Opcode::Convert, original.type);
	narrow.sub = static_cast<uint8_t>(ConvertOp::Trunc);
	narrow.ops[0] = bits;
	set_convert_source_type(narrow, wide);
	if (bit_view || storage_bytes(original.type) == storage_bytes(wide))
	{
		narrow.op = Opcode::Copy;
		narrow.sub = 0;
		narrow.immediate = 0;
	}
	narrow.result = original.result;
	narrow.next = original.next;
	narrow.location = original.location;
	body_.instructions[id] = narrow;
}

// A store to `offset` in a packed field of `slot`: the field's bits there
// cleared and the value's put in (in place of instruction `id`, which
// becomes the field's store). A literal goes in shifted. Bytes no write of
// the slot reaches keep their first value, zero (slot promotion's for a
// read before any store), and stay out of the merge: a loop's local
// whose padding the merge kept would carry it around the loop in a phi.
void AggregateSlots::store_packed(BlockId block, InstId& prev, InstId id, SlotId slot, uint64_t offset)
{
	const std::vector<Field>& fields = facts_[slot].fields;
	const uint32_t k = packed_field(slot, offset);
	const Instruction original = body_.instructions[id];
	const Type wide = fields[k].type;
	const unsigned shift = static_cast<unsigned>(8 * (offset - fields[k].offset));
	const unsigned bits = static_cast<unsigned>(8 * storage_bytes(original.type));
	const uint64_t mask = (bits >= 64 ? ~static_cast<uint64_t>(0) : (static_cast<uint64_t>(1) << bits) - 1) << shift;
	const uint64_t start = fields[k].offset, end = start + storage_bytes(wide);
	uint64_t written = 0;
	for (const std::pair<uint64_t, uint64_t>& w : facts_[slot].writes)
		for (uint64_t at = std::max(w.first, start); at < std::min(w.second, end); ++at)
			written |= static_cast<uint64_t>(0xff) << (8 * (at - start));
	const uint64_t keep = ~mask & written;
	const Value field = Value::slot(first_field_[slot] + k);
	auto binary = [&](BinaryOp op, Value a, Value b) {
		Instruction inst = make_instruction(Opcode::Binary, wide);
		inst.sub = static_cast<uint8_t>(op);
		inst.ops[0] = a;
		inst.ops[1] = b;
		inst.location = original.location;
		prev = emit(block, prev, inst, wide);
		return Value::temp(body_.instructions[prev].result);
	};
	Value merged = Value::none();
	if (keep != 0)
	{
		Instruction load = make_instruction(Opcode::Load, wide);
		load.ops[0] = field;
		load.location = original.location;
		prev = emit(block, prev, load, wide);
		merged = binary(BinaryOp::And, Value::temp(body_.instructions[prev].result),
			integer_value(body_, integer_literal(static_cast<int64_t>(keep))));
	}
	const Value v = original.ops[0];
	if (v.is(ValueKind::Integer))
	{
		const uint64_t put = (body_.literals[v.index].low << shift) & mask;
		if (merged.is(ValueKind::None))
			merged = integer_value(body_, integer_literal(static_cast<int64_t>(put)));
		else if (put != 0)
			merged = binary(BinaryOp::Or, merged, integer_value(body_, integer_literal(static_cast<int64_t>(put))));
	}
	else
	{
		Value bits_value = v;
		Type bits_type = original.type;
		if (original.type.is_pointer() || original.type.kind() == TypeKind::F32 ||
			original.type.kind() == TypeKind::F64)
		{
			bits_type = filler_type(storage_bytes(original.type));
			Instruction bits_copy = make_instruction(Opcode::Copy, bits_type);
			bits_copy.ops[0] = v;
			bits_copy.location = original.location;
			prev = emit(block, prev, bits_copy, bits_type);
			bits_value = Value::temp(body_.instructions[prev].result);
		}
		Instruction widen = make_instruction(Opcode::Convert, wide);
		widen.sub = static_cast<uint8_t>(ConvertOp::ZExt);
		widen.ops[0] = bits_value;
		set_convert_source_type(widen, bits_type);
		if (storage_bytes(original.type) == storage_bytes(wide))
		{
			widen.op = Opcode::Copy;
			widen.sub = 0;
			widen.immediate = 0;
		}
		widen.location = original.location;
		prev = emit(block, prev, widen, wide);
		Value placed = Value::temp(body_.instructions[prev].result);
		if (shift != 0)
			placed = binary(BinaryOp::Shl, placed, integer_value(body_, integer_literal(static_cast<int64_t>(shift))));
		merged = merged.is(ValueKind::None) ? placed : binary(BinaryOp::Or, merged, placed);
	}
	Instruction store = original;
	store.type = wide;
	store.flags &= ~IF_TYPED; // the eightbyte's store, no longer the field's
	store.ops[0] = merged;
	store.ops[1] = field;
	body_.instructions[id] = store;
}

Value AggregateSlots::field_slot(SlotId slot, uint64_t offset) const
{
	const std::vector<Field>& fields = facts_[slot].fields;
	for (uint32_t k = 0; k < fields.size(); ++k)
		if (fields[k].offset == offset)
			return Value::slot(first_field_[slot] + k);
	return Value::none();
}

InstId AggregateSlots::emit(BlockId block, InstId after, Instruction inst, Type result)
{
	const InstId id = append_instruction(body_, inst);
	if (!result.is_void())
		body_.instructions[id].result = append_temp(body_, result, id);
	insert_after(body_, block, after, id);
	return id;
}

// A pointer to `offset` bytes past `base` (a pointer value, or a slot whose
// address is taken here).
Value AggregateSlots::pointer_at(BlockId block, InstId& after, Value base, int64_t offset, uint32_t location)
{
	if (base.is(ValueKind::Slot))
	{
		Instruction addr = make_instruction(Opcode::Addr, body_.pointer_type());
		addr.ops[0] = base;
		addr.location = location;
		after = emit(block, after, addr, body_.pointer_type());
		base = Value::temp(body_.instructions[after].result);
	}
	if (offset == 0)
		return base;
	Instruction index = make_instruction(Opcode::Index, Type::scalar(TypeKind::I8));
	index.sub = ir_model::IPK_FIELD;
	index.ops[0] = base;
	index.ops[1] = integer_value(body_, integer_literal(offset));
	index.location = location;
	after = emit(block, after, index, body_.pointer_type());
	return Value::temp(body_.instructions[after].result);
}

// copyobj or zeroinit touching a split slot: one load and store (or one
// store of zero) per field in the span.
void AggregateSlots::expand_copy(BlockId block, InstId prev, const Instruction& inst, InstId& last)
{
	InstId at = prev;
	const uint64_t bytes = static_cast<uint64_t>(inst.immediate);
	SlotAddress src = { kNoId, 0 }, dst = { kNoId, 0 };
	auto split = [&](Value v, SlotAddress& a) {
		if (v.is(ValueKind::Slot))
			a = SlotAddress{ v.index, 0 };
		else if (!address_of(v, a))
			return false;
		return first_field_[a.slot] != kNoId;
	};
	const bool zero = inst.op == Opcode::ZeroInit;
	const Value dst_value = zero ? inst.ops[0] : inst.ops[1];
	Value source = zero ? Value::none() : inst.ops[0];
	if (!zero && source.is(ValueKind::Temp) && body_.temps[source.index].type.is_object())
	{
		// An object value: into a temporary of its own, read field by field.
		const SlotId temporary = static_cast<SlotId>(body_.slots.size());
		body_.slots.push_back(Slot{ 0, SlotKind::Aggregate, 0, 0, body_.temps[source.index].type });
		Instruction addr = make_instruction(Opcode::Addr, body_.pointer_type());
		addr.ops[0] = Value::slot(temporary);
		addr.location = inst.location;
		at = emit(block, at, addr, body_.pointer_type());
		Instruction copy = inst;
		copy.next = kNoId;
		copy.ops[1] = Value::temp(body_.instructions[at].result);
		at = emit(block, at, copy, Type::scalar(TypeKind::Void));
		source = copy.ops[1];
	}
	const bool src_split = !zero && split(source, src);
	const bool dst_split = split(dst_value, dst);
	// Walk the split side's fields (the destination's when both split).
	const SlotAddress& lead = dst_split ? dst : src;
	uint64_t covered = 0;
	const auto residual = [&](uint64_t end) {
		if (end <= covered)
			return;
		Instruction copy = inst;
		copy.next = kNoId;
		copy.immediate = static_cast<int64_t>(end - covered);
		// Byte alignment is valid for every residual partition. Do not
		// inherit a whole object's stronger alignment after an odd offset.
		copy.sub = 0;
		const Value to = pointer_at(block, at, dst_value, static_cast<int64_t>(covered), inst.location);
		copy.ops[0] = zero ? to : pointer_at(block, at, source, static_cast<int64_t>(covered), inst.location);
		if (!zero)
			copy.ops[1] = to;
		at = emit(block, at, copy, Type::scalar(TypeKind::Void));
	};
	for (const Field& f : facts_[lead.slot].fields)
	{
		if (f.offset < static_cast<uint64_t>(lead.offset) || f.offset + storage_bytes(f.type) > lead.offset + bytes)
			continue;
		const int64_t rel = static_cast<int64_t>(f.offset) - lead.offset;
		residual(static_cast<uint64_t>(rel));
		Value value;
		if (zero && f.type.is_vector())
		{
			Instruction clear = make_instruction(Opcode::Vector, f.type);
			clear.sub = static_cast<uint8_t>(VectorOp::Pzero);
			clear.location = inst.location;
			at = emit(block, at, clear, f.type);
			value = Value::temp(body_.instructions[at].result);
		}
		else if (zero)
			value = f.type.is_float() ? literal_value(body_, ValueKind::Float, f.type.kind(), Literal{ 0, 0 })
									  : integer_value(body_, Literal{ 0, 0 });
		else
		{
			Instruction load = make_instruction(Opcode::Load, f.type);
			load.location = inst.location;
			const Value field = src_split ? field_slot(src.slot, src.offset + rel) : Value::none();
			load.ops[0] = !field.is(ValueKind::None) ? field : pointer_at(block, at, source, rel, inst.location);
			at = emit(block, at, load, f.type);
			value = Value::temp(body_.instructions[at].result);
		}
		Instruction store = make_instruction(Opcode::Store, f.type);
		store.location = inst.location;
		store.ops[0] = value;
		const Value field = dst_split ? field_slot(dst.slot, dst.offset + rel) : Value::none();
		store.ops[1] = !field.is(ValueKind::None) ? field : pointer_at(block, at, dst_value, rel, inst.location);
		at = emit(block, at, store, Type::scalar(TypeKind::Void));
		covered = static_cast<uint64_t>(rel) + storage_bytes(f.type);
	}
	residual(bytes);
	last = at;
}

void AggregateSlots::rewrite()
{
	for (BlockId b : body_.layout)
	{
		InstId prev = kNoId;
		for (InstId i = body_.blocks[b].first; i != kNoId;)
		{
			Instruction inst = body_.instructions[i];
			const InstId next = inst.next;
			const bool clear = scalar_zero(inst);
			if (clear)
			{
				inst.type = filler_type(static_cast<uint64_t>(inst.immediate));
				inst.op = Opcode::Store;
				inst.ops[1] = inst.ops[0];
				inst.ops[0] = Value::none();
				inst.immediate = 0;
				inst.sub = 0;
			}
			SlotAddress a;
			const bool load = inst.op == Opcode::Load, store = inst.op == Opcode::Store;
			if (load || store)
			{
				const Value storage = inst.ops[load ? 0 : 1];
				if (storage.is(ValueKind::Slot))
					a = SlotAddress{ storage.index, 0 };
				else if (!address_of(storage, a))
					a.slot = kNoId;
				if (a.slot != kNoId && first_field_[a.slot] != kNoId &&
					(!storage.is(ValueKind::Temp) || address_extent(storage) == 0))
				{
					if (clear)
					{
						inst.ops[0] = integer_value(body_, Literal{ 0, 0 });
						body_.instructions[i] = inst;
					}
					const uint64_t offset = static_cast<uint64_t>(a.offset);
					bool packed = false;
					for (const Field& f : facts_[a.slot].fields)
						packed = packed ||
							(f.packed && f.offset <= offset && offset < f.offset + storage_bytes(f.type) &&
								!(f.offset == offset && f.type == inst.type));
					if (packed)
					{
						if (load)
							load_packed(b, prev, i, a.slot, offset);
						else
							store_packed(b, prev, i, a.slot, offset);
						prev = i;
						i = next;
						continue;
					}
					const Value field = field_slot(a.slot, offset);
					if (!field.is(ValueKind::None))
						body_.instructions[i].ops[load ? 0 : 1] = field;
				}
			}
			else if (inst.op == Opcode::Call)
			{
				const Signature* sig = c_.program.call_boundary(inst);
				const Parameter* params = sig != nullptr ? c_.program.parameters_of(*sig) : nullptr;
				InstId at = i;
				for (uint32_t k = 0; params != nullptr && k < inst.extra_count && k < sig->parameter_count; ++k)
				{
					const Value v = body_.operands[inst.extra_first + k];
					SlotAddress r = { kNoId, 0 };
					if (params[k].pass != ir_model::PPM_INDIRECT_RESULT || v.is(ValueKind::Slot) || !address_of(v, r) ||
						first_field_[r.slot] == kNoId)
						continue;
					// The call fills a temporary; the fields copy from it.
					const Type type = body_.slots[r.slot].type;
					const SlotId temporary = static_cast<SlotId>(body_.slots.size());
					body_.slots.push_back(Slot{ 0, SlotKind::Aggregate, 0, 0, type });
					Instruction addr = make_instruction(Opcode::Addr, body_.pointer_type());
					addr.ops[0] = Value::slot(temporary);
					addr.location = inst.location;
					const InstId a_id = emit(b, prev, addr, body_.pointer_type());
					prev = a_id;
					body_.operands[body_.instructions[i].extra_first + k] = Value::temp(
						body_.instructions[a_id].result);
					Instruction copy = make_instruction(Opcode::CopyObject, Type::scalar(TypeKind::Void));
					copy.ops[0] = Value::temp(body_.instructions[a_id].result);
					copy.ops[1] = v;
					copy.immediate = static_cast<int64_t>(storage_bytes(type));
					copy.location = inst.location;
					InstId last = at;
					expand_copy(b, at, copy, last);
					at = last;
				}
				if (at != i)
				{
					prev = at;
					i = body_.instructions[at].next;
					continue;
				}
			}
			else if (inst.op == Opcode::CopyObject || inst.op == Opcode::ZeroInit)
			{
				SlotAddress x = { kNoId, 0 }, y = { kNoId, 0 };
				const bool touches = (inst.ops[0].is(ValueKind::Slot) && first_field_[inst.ops[0].index] != kNoId) ||
					(address_of(inst.ops[0], x) && first_field_[x.slot] != kNoId) ||
					(inst.op == Opcode::CopyObject &&
						((inst.ops[1].is(ValueKind::Slot) && first_field_[inst.ops[1].index] != kNoId) ||
							(address_of(inst.ops[1], y) && first_field_[y.slot] != kNoId)));
				if (touches)
				{
					InstId last = prev;
					expand_copy(b, prev, inst, last);
					unlink_after(body_, b, last, i);
					prev = last;
					i = next;
					continue;
				}
			}
			prev = i;
			i = next;
		}
	}
}

// A whole-object load whose value only stores read becomes a copy into a
// temporary slot of its own, and a store of a call's result or a parameter
// a copy from the value: the forms the splitting below reads field by field.
// (Complex values move this way, as whole objects from slot to slot.)
// Object values are immutable snapshots. Their value copies are aliases;
// copies from storage are loads. Whole-object transfers accept copyobj's
// object values directly below, without cycling them through store and back.
// Include copies the inliner introduced; no frontend spelling is privileged.
bool canonicalize_object_values(FunctionBody& body)
{
	std::vector<Value> alias;
	bool changed = false;
	for (BlockId b : body.layout)
		for (InstId i = body.blocks[b].first; i != kNoId; i = body.instructions[i].next)
		{
			Instruction& inst = body.instructions[i];
			if (inst.op == Opcode::Copy && inst.type.is_object())
			{
				const Value from = inst.ops[0];
				if (from.is(ValueKind::Temp) && body.temps[from.index].type == inst.type)
				{
					if (alias.empty())
						alias.assign(body.temps.size(), Value::none());
					alias[inst.result] = from;
				}
				else
					inst.op = Opcode::Load;
				changed = true;
			}
		}
	if (alias.empty())
		return changed;
	const auto resolve = [&](Value value) {
		Value root = value;
		while (root.is(ValueKind::Temp) && !alias[root.index].is(ValueKind::None))
			root = alias[root.index];
		while (value.is(ValueKind::Temp) && !alias[value.index].is(ValueKind::None))
		{
			const Value next = alias[value.index];
			alias[value.index] = root;
			value = next;
		}
		return root;
	};
	for (BlockId b : body.layout)
	{
		InstId prev = kNoId;
		for (InstId i = body.blocks[b].first; i != kNoId;)
		{
			Instruction& inst = body.instructions[i];
			const InstId next = inst.next;
			for_each_operand(body, inst, [&](Value& value) { value = resolve(value); });
			if (inst.result != kNoId && !alias[inst.result].is(ValueKind::None))
				unlink_after(body, b, prev, i);
			else
				prev = i;
			i = next;
		}
	}
	return true;
}

bool AggregateSlots::objects_through_slots()
{
	const auto whole = [](const Instruction& inst) {
		return (inst.op == Opcode::Load || inst.op == Opcode::Store) && inst.type.is_object() &&
			(inst.flags & IF_VOLATILE) == 0;
	};
	const auto object_copy = [&](const Instruction& inst) {
		return inst.op == Opcode::CopyObject && inst.ops[0].is(ValueKind::Temp) &&
			body_.temps[inst.ops[0].index].type.is_object();
	};
	// Per temp: 1 a whole load's value only whole stores have read, 2 not.
	std::vector<uint8_t> stored;
	for (BlockId b : body_.layout)
		for (InstId i = body_.blocks[b].first; i != kNoId; i = body_.instructions[i].next)
		{
			const Instruction& inst = body_.instructions[i];
			if (!whole(inst))
				continue;
			if (stored.empty())
				stored.assign(body_.temps.size(), 0);
			if (inst.op == Opcode::Load && inst.result != kNoId)
				stored[inst.result] = 1;
		}
	if (stored.empty())
		return false;
	for (BlockId b : body_.layout)
		for (InstId i = body_.blocks[b].first; i != kNoId; i = body_.instructions[i].next)
		{
			const Instruction& inst = body_.instructions[i];
			if ((whole(inst) && inst.op == Opcode::Store) || object_copy(inst))
				continue;
			for_each_operand(body_, inst, [&](const Value& v) {
				if (v.is(ValueKind::Temp) && stored[v.index] == 1)
					stored[v.index] = 2;
			});
		}
	bool rewritten = false;
	const auto copy = [&](Instruction& inst, Value from, Value to) {
		const Type type = object_copy(inst) ? body_.temps[inst.ops[0].index].type : inst.type;
		inst.op = Opcode::CopyObject;
		inst.ops[0] = from;
		inst.ops[1] = to;
		inst.immediate = static_cast<int64_t>(storage_bytes(type));
		inst.sub = static_cast<uint8_t>(type.align_log2);
		inst.type = Type::scalar(TypeKind::Void);
		rewritten = true;
	};
	std::vector<SlotId> home(body_.temps.size(), kNoId);
	for (BlockId b : body_.layout)
		for (InstId i = body_.blocks[b].first; i != kNoId; i = body_.instructions[i].next)
		{
			Instruction& inst = body_.instructions[i];
			if (!whole(inst) && !object_copy(inst))
				continue;
			if (inst.op == Opcode::Load)
			{
				if (inst.result == kNoId || stored[inst.result] != 1)
					continue;
				const SlotId slot = static_cast<SlotId>(body_.slots.size());
				body_.slots.push_back(Slot{ 0, SlotKind::Aggregate, 0, 0, inst.type });
				home[inst.result] = slot;
				inst.result = kNoId;
				copy(inst, inst.ops[0], Value::slot(slot));
				continue;
			}
			const Value value = inst.ops[0];
			if (!value.is(ValueKind::Temp))
				continue;
			const Value to = inst.ops[1];
			if (home[value.index] != kNoId)
				copy(inst, Value::slot(home[value.index]), to);
			else if (!object_copy(inst) &&
				(value.index < body_.parameter_count ||
					body_.instructions[body_.temps[value.index].def].op == Opcode::Call))
				copy(inst, value, to);
		}
	return rewritten;
}

// A short copy may end within a word that another access transfers whole.
// Describe its exact bytes as scalar accesses so packed-field rewriting can
// preserve the untouched bytes instead of rejecting the entire carrier.
bool AggregateSlots::scalarize_partial_copies()
{
	if (!partial_copy_)
		return false;
	std::vector<uint64_t> words(facts_.size(), 0);
	bool any = false;
	for (SlotId s = 0; s < facts_.size(); ++s)
	{
		if (!facts_[s].candidate)
			continue;
		for (const auto& w : facts_[s].wholes)
			if (w.first / 8 < 64)
			{
				words[s] |= uint64_t(1) << (w.first / 8);
				any = true;
			}
	}
	if (!any)
		return false;
	const auto address = [&](Value v, SlotAddress& a) {
		if (v.is(ValueKind::Slot))
		{
			a = SlotAddress{ v.index, 0 };
			return true;
		}
		return address_of(v, a) && address_extent(v) == 0;
	};
	const auto straddles = [&](Value v, uint64_t bytes) {
		SlotAddress a;
		if (!address(v, a) || a.offset < 0 || words[a.slot] == 0)
			return false;
		const uint64_t first = static_cast<uint64_t>(a.offset), end = first + bytes;
		return (first % 8 != 0 && first / 8 < 64 && ((words[a.slot] >> (first / 8)) & 1)) ||
			(end % 8 != 0 && end / 8 < 64 && ((words[a.slot] >> (end / 8)) & 1));
	};
	bool changed = false;
	for (BlockId b : body_.layout)
	{
		InstId prev = kNoId;
		for (InstId i = body_.blocks[b].first; i != kNoId;)
		{
			const Instruction inst = body_.instructions[i];
			const InstId next = inst.next;
			const uint64_t bytes = static_cast<uint64_t>(inst.immediate);
			const bool object = inst.ops[0].is(ValueKind::Temp) && body_.temps[inst.ops[0].index].type.is_object();
			if (inst.op != Opcode::CopyObject || (inst.flags & IF_VOLATILE) || inst.immediate <= 0 || bytes > 16 ||
				object || (!straddles(inst.ops[0], bytes) && !straddles(inst.ops[1], bytes)))
			{
				prev = i;
				i = next;
				continue;
			}
			struct Chunk
			{
				uint64_t offset;
				Type type;
				Value value;
			};
			std::vector<Chunk> chunks;
			for (uint64_t offset = 0; offset < bytes;)
			{
				uint64_t width = 8;
				while (width > bytes - offset)
					width /= 2;
				for (unsigned side = 0; side < 2; ++side)
				{
					SlotAddress a;
					if (address(inst.ops[side], a) && a.offset >= 0)
						while (width > 1 && ((static_cast<uint64_t>(a.offset) + offset) % 8) + width > 8)
							width /= 2;
				}
				Instruction load = make_instruction(Opcode::Load, filler_type(width));
				load.ops[0] = pointer_at(b, prev, inst.ops[0], static_cast<int64_t>(offset), inst.location);
				load.location = inst.location;
				prev = emit(b, prev, load, load.type);
				chunks.push_back(Chunk{ offset, load.type, Value::temp(body_.instructions[prev].result) });
				offset += width;
			}
			// Read every part before writing: overlapping copies retain the
			// source bytes, including when the two operands name one slot.
			for (const Chunk& chunk : chunks)
			{
				Instruction store = make_instruction(Opcode::Store, chunk.type);
				store.ops[0] = chunk.value;
				store.ops[1] = pointer_at(b, prev, inst.ops[1], static_cast<int64_t>(chunk.offset), inst.location);
				store.location = inst.location;
				prev = emit(b, prev, store, Type::scalar(TypeKind::Void));
			}
			unlink_after(body_, b, prev, i);
			changed = true;
			i = next;
		}
	}
	return changed;
}

bool AggregateSlots::run()
{
	const bool object_values = std::any_of(
		body_.temps.begin(), body_.temps.end(), [](const Temp& temp) { return temp.type.is_object(); });
	if (body_.slots.empty() && !object_values)
		return false;
	// Scalar-only SSA cannot contain an object alias or a loaded object value.
	// Use the dense type table before walking the linked instructions twice.
	bool rewritten = false;
	if (object_values)
	{
		rewritten = canonicalize_object_values(body_);
		rewritten = objects_through_slots() || rewritten;
	}
	analyze();
	if (scalarize_partial_copies())
	{
		rewritten = true;
		analyze();
	}
	mark_recurrent_fields();
	first_field_.assign(body_.slots.size(), kNoId);
	bool any = false;
	const uint32_t slots = static_cast<uint32_t>(body_.slots.size());
	std::vector<uint8_t> settled(slots, 0);
	for (SlotId s = 0; s < slots; ++s)
	{
		SlotFacts& facts = facts_[s];
		const Type type = body_.slots[s].type;
		// A call's returned object copied on whole stays in memory: its
		// fields would be loaded after the call and stored again.
		if (facts.call_written && facts.copied_out)
			continue;
		if (!facts.candidate || !settle(facts, storage_bytes(type), s))
			continue;
		settled[s] = 1;
		if ((type.is_object() || facts.forced != 0) && !facts.partial)
			facts.packed = packable(facts, storage_bytes(type));
	}
	agree_packing(settled);
	for (SlotId s = 0; s < slots; ++s)
	{
		if (!settled[s])
			continue;
		SlotFacts& facts = facts_[s];
		// An eightbyte read or written whole by an i64 access has no
		// fields to give it apart from its integer.
		if ((facts.forced & ~facts.packed) != 0)
			continue;
		pack(facts);
		first_field_[s] = static_cast<uint32_t>(body_.slots.size());
		for (const Field& f : facts.fields)
			body_.slots.push_back(Slot{ 0, SlotKind::Field, 0, 0, f.type });
		any = true;
	}
	if (!any || !compatible_copies())
		return rewritten;
	rewrite();
	return true;
}

// A copy between two split slots moves field to field: their fields must
// agree across the span, or both keep their storage. Repeats until stable
// (each round unsplits at least one slot).
bool AggregateSlots::compatible_copies()
{
	for (bool changed = true; changed;)
	{
		changed = false;
		for (BlockId b : body_.layout)
			for (InstId i = body_.blocks[b].first; i != kNoId; i = body_.instructions[i].next)
			{
				const Instruction& inst = body_.instructions[i];
				if (inst.op != Opcode::CopyObject)
					continue;
				SlotAddress x, y;
				if (inst.ops[0].is(ValueKind::Slot))
					x = SlotAddress{ inst.ops[0].index, 0 };
				else if (!address_of(inst.ops[0], x))
					continue;
				if (inst.ops[1].is(ValueKind::Slot))
					y = SlotAddress{ inst.ops[1].index, 0 };
				else if (!address_of(inst.ops[1], y))
					continue;
				if (first_field_[x.slot] == kNoId || first_field_[y.slot] == kNoId)
					continue;
				changed = share_floats(x, y, static_cast<uint64_t>(inst.immediate)) || changed;
				if (x.slot == y.slot || !same_fields(x, y, static_cast<uint64_t>(inst.immediate)))
				{
					first_field_[x.slot] = first_field_[y.slot] = kNoId;
					changed = true;
				}
			}
	}
	for (SlotId s = 0; s < first_field_.size(); ++s)
		if (first_field_[s] != kNoId)
			return true;
	return false;
}

// An integer field a copy moves to or from a floating field of its width is
// that field's carrier: it floats too, so the value stays in one register file.
bool AggregateSlots::share_floats(const SlotAddress& x, const SlotAddress& y, uint64_t bytes)
{
	bool changed = false;
	std::vector<Field>& from = facts_[x.slot].fields;
	std::vector<Field>& to = facts_[y.slot].fields;
	for (size_t i = 0; i < from.size(); ++i)
	{
		if (from[i].offset < static_cast<uint64_t>(x.offset) ||
			from[i].offset + storage_bytes(from[i].type) > x.offset + bytes)
			continue;
		const uint64_t at = from[i].offset - x.offset + y.offset;
		for (size_t k = 0; k < to.size(); ++k)
			if (to[k].offset == at && float_twins(from[i].type, to[k].type))
			{
				Field& integer = from[i].type.is_float() ? to[k] : from[i];
				const Type floating = from[i].type.is_float() ? from[i].type : to[k].type;
				const SlotId field = first_field_[from[i].type.is_float() ? y.slot : x.slot] +
					static_cast<uint32_t>(from[i].type.is_float() ? k : i);
				integer.type = floating;
				body_.slots[field].type = floating;
				changed = true;
			}
	}
	return changed;
}

bool AggregateSlots::same_fields(const SlotAddress& x, const SlotAddress& y, uint64_t bytes) const
{
	auto in_span = [&](const SlotAddress& a, const Field& f) {
		return f.offset >= static_cast<uint64_t>(a.offset) && f.offset + storage_bytes(f.type) <= a.offset + bytes;
	};
	uint32_t count_x = 0, count_y = 0;
	for (const Field& f : facts_[x.slot].fields)
		count_x += in_span(x, f) ? 1 : 0;
	for (const Field& f : facts_[y.slot].fields)
	{
		if (!in_span(y, f))
			continue;
		++count_y;
		const uint64_t at = f.offset - y.offset + x.offset;
		bool found = false;
		for (const Field& g : facts_[x.slot].fields)
			found = found || (g.offset == at && (g.type == f.type || integer_twins(g.type, f.type)));
		if (!found)
			return false;
	}
	return count_x == count_y;
}

} // namespace

bool split_aggregate_slots(FunctionContext& context)
{
	AggregateSlots pass(context);
	return pass.run();
}

} // namespace opt
