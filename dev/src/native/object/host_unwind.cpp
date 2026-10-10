#include "native/object/host_unwind.h"

namespace native {

namespace {

// DWARF register numbers (psABI 3.38) by x86 encoding.
const uint8_t kDwarfRegs[16] = { 0, 2, 1, 3, 7, 6, 4, 5, 8, 9, 10, 11, 12, 13, 14, 15 };
const uint8_t kDwarfRsp = 7;
const uint8_t kDwarfRbp = 6;
const uint8_t kDwarfReturn = 16;

enum : uint8_t
{
	DW_CFA_advance_loc = 0x40,
	DW_CFA_offset = 0x80,
	DW_CFA_restore = 0xc0,
	DW_CFA_nop = 0x00,
	DW_CFA_advance_loc1 = 0x02,
	DW_CFA_advance_loc2 = 0x03,
	DW_CFA_advance_loc4 = 0x04,
	DW_CFA_remember_state = 0x0a,
	DW_CFA_restore_state = 0x0b,
	DW_CFA_def_cfa = 0x0c,
	DW_CFA_def_cfa_register = 0x0d,
	DW_CFA_def_cfa_offset = 0x0e,
	DW_CFA_GNU_args_size = 0x2e
};

// Pointer encodings (LSB: DW_EH_PE_*).
const uint8_t kPcrelSdata4 = 0x1b;
const uint8_t kIndirectPcrelSdata4 = 0x9b;
const uint8_t kOmit = 0xff;
const uint8_t kUleb128 = 0x01;

void put_uleb(std::vector<uint8_t>& out, uint64_t value, unsigned bytes = 0)
{
	unsigned written = 0;
	do
	{
		uint8_t byte = value & 0x7f;
		value >>= 7;
		++written;
		if (value != 0 || written < bytes)
			byte |= 0x80;
		out.push_back(byte);
	} while (value != 0 || written < bytes);
}

// SLEB128, padded to `bytes` with sign continuation bytes when given.
void put_sleb(std::vector<uint8_t>& out, int64_t value, unsigned bytes = 0)
{
	unsigned written = 0;
	for (;;)
	{
		uint8_t byte = value & 0x7f;
		value >>= 7;
		++written;
		const bool done = (value == 0 && !(byte & 0x40)) || (value == -1 && (byte & 0x40));
		if (done && written >= bytes)
		{
			out.push_back(byte);
			return;
		}
		out.push_back(byte | 0x80);
	}
}

unsigned uleb_size(uint64_t value)
{
	unsigned size = 1;
	while (value >= 0x80)
	{
		value >>= 7;
		++size;
	}
	return size;
}

unsigned sleb_size(int64_t value)
{
	unsigned size = 1;
	while (value < -64 || value > 63)
	{
		value >>= 7;
		++size;
	}
	return size;
}

void put32(std::vector<uint8_t>& out, uint32_t value)
{
	for (unsigned k = 0; k < 4; ++k)
		out.push_back(static_cast<uint8_t>(value >> (8 * k)));
}

void patch32(std::vector<uint8_t>& out, size_t at, uint32_t value)
{
	for (unsigned k = 0; k < 4; ++k)
		out[at + k] = static_cast<uint8_t>(value >> (8 * k));
}

// Pads a CIE or FDE that starts at `start` to a multiple of 8 bytes and
// sets its length field.
void finish_entry(std::vector<uint8_t>& out, size_t start)
{
	while ((out.size() - start) % 8 != 0)
		out.push_back(DW_CFA_nop);
	patch32(out, start, static_cast<uint32_t>(out.size() - start - 4));
}

struct Cursor
{
	std::vector<uint8_t>& out;
	uint32_t at;

	void advance(uint32_t to)
	{
		const uint32_t delta = to - at;
		if (delta == 0)
			return;
		if (delta < 0x40)
			out.push_back(static_cast<uint8_t>(DW_CFA_advance_loc | delta));
		else if (delta < 0x100)
		{
			out.push_back(DW_CFA_advance_loc1);
			out.push_back(static_cast<uint8_t>(delta));
		}
		else if (delta < 0x10000)
		{
			out.push_back(DW_CFA_advance_loc2);
			out.push_back(static_cast<uint8_t>(delta));
			out.push_back(static_cast<uint8_t>(delta >> 8));
		}
		else
		{
			out.push_back(DW_CFA_advance_loc4);
			put32(out, delta);
		}
		at = to;
	}
};

} // namespace

uint32_t encode_cie(bool personality, std::vector<uint8_t>& out)
{
	const size_t start = out.size();
	put32(out, 0);
	put32(out, 0); // CIE id
	out.push_back(1);
	const char* augmentation = personality ? "zPLR" : "zR";
	for (const char* c = augmentation; *c != 0; ++c)
		out.push_back(static_cast<uint8_t>(*c));
	out.push_back(0);
	put_uleb(out, 1);
	put_sleb(out, -8);
	put_uleb(out, kDwarfReturn);
	uint32_t field = 0;
	if (personality)
	{
		put_uleb(out, 7);
		out.push_back(kIndirectPcrelSdata4);
		field = static_cast<uint32_t>(out.size());
		put32(out, 0);
		out.push_back(kPcrelSdata4);
		out.push_back(kPcrelSdata4);
	}
	else
	{
		put_uleb(out, 1);
		out.push_back(kPcrelSdata4);
	}
	out.push_back(DW_CFA_def_cfa);
	put_uleb(out, kDwarfRsp);
	put_uleb(out, 8);
	out.push_back(DW_CFA_offset | kDwarfReturn);
	put_uleb(out, 1);
	finish_entry(out, start);
	return field;
}

// Without a frame pointer (UnwindFacts::frameless) the CFA stays RSP +
// the bytes below the return address: `reserve`, then each push; each
// epilogue pops them back and releases the reserve.
void encode_frameless_instructions(const UnwindFacts& facts, std::vector<uint8_t>& out)
{
	Cursor cursor = { out, 0 };
	uint32_t below = 8;
	// Each prologue: the reservation, then the saves' pushes. A
	// shrink-wrapped function's code before its first has the entry's
	// rules; a later one starts from them again (the one before it ended
	// with the body's).
	std::vector<UnwindFacts::Prologue> prologues = facts.prologues;
	if (prologues.empty())
	{
		UnwindFacts::Prologue only = { 0, facts.reserve_end, {} };
		for (size_t k = 0; k < facts.saves.size() && k < 6; ++k)
			only.save_end[k] = facts.saves[k].end;
		prologues.push_back(only);
	}
	for (size_t p = 0; p < prologues.size(); ++p)
	{
		if (p != 0)
		{
			cursor.advance(prologues[p].start);
			out.push_back(DW_CFA_def_cfa_offset);
			put_uleb(out, 8);
			for (const UnwindFacts::Save& save : facts.saves)
				out.push_back(static_cast<uint8_t>(DW_CFA_restore | kDwarfRegs[save.reg]));
		}
		below = 8 + facts.reserve;
		if (facts.reserve != 0)
		{
			cursor.advance(prologues[p].reserve_end);
			out.push_back(DW_CFA_def_cfa_offset);
			put_uleb(out, below);
		}
		// [virtual rbp + offset] is CFA - 16 + offset.
		for (size_t k = 0; k < facts.saves.size() && k < 6; ++k)
		{
			const UnwindFacts::Save& save = facts.saves[k];
			cursor.advance(prologues[p].save_end[k]);
			below += 8;
			out.push_back(DW_CFA_def_cfa_offset);
			put_uleb(out, below);
			out.push_back(static_cast<uint8_t>(DW_CFA_offset | kDwarfRegs[save.reg]));
			put_uleb(out, static_cast<uint32_t>(16 - save.offset) / 8);
		}
	}
	const uint32_t body = below;
	// The body's rules hold up to each epilogue (the previous one's code
	// ends with its restore): they are remembered where the cursor is.
	auto epilogue = [&](const uint32_t* pop_end, uint32_t released_end, uint32_t ret_end, bool more) {
		if (body == 8)
			return;
		if (more)
			out.push_back(DW_CFA_remember_state);
		uint32_t at = body;
		for (size_t k = facts.saves.size(); k-- > 0;)
		{
			const size_t popped = facts.saves.size() - 1 - k; // pops run rbx first: the last save first
			cursor.advance(pop_end[popped]);
			at -= 8;
			out.push_back(DW_CFA_def_cfa_offset);
			put_uleb(out, at);
			out.push_back(static_cast<uint8_t>(DW_CFA_restore | kDwarfRegs[facts.saves[k].reg]));
		}
		if (facts.reserve != 0)
		{
			cursor.advance(released_end);
			out.push_back(DW_CFA_def_cfa_offset);
			put_uleb(out, 8);
		}
		if (more)
		{
			cursor.advance(ret_end);
			out.push_back(DW_CFA_restore_state);
		}
	};
	for (const UnwindFacts::Epilogue& ret : facts.returns)
		epilogue(ret.pop_end, ret.leave_end, ret.ret_end, true);
	epilogue(facts.pop_end, facts.leave_end, facts.ret_end, facts.size > facts.ret_end);
}

void encode_frame_instructions(const UnwindFacts& facts, bool lsda, std::vector<uint8_t>& out)
{
	if (facts.frameless)
	{
		encode_frameless_instructions(facts, out);
		return;
	}
	Cursor cursor = { out, 0 };
	if (facts.frame_end == 0)
		return;
	cursor.advance(facts.push_end);
	out.push_back(DW_CFA_def_cfa_offset);
	put_uleb(out, 16);
	out.push_back(DW_CFA_offset | kDwarfRbp);
	put_uleb(out, 2);
	cursor.advance(facts.frame_end);
	out.push_back(DW_CFA_def_cfa_register);
	put_uleb(out, kDwarfRbp);
	// [rbp + offset] is CFA - 16 + offset.
	for (const UnwindFacts::Save& save : facts.saves)
	{
		cursor.advance(save.end);
		out.push_back(static_cast<uint8_t>(DW_CFA_offset | kDwarfRegs[save.reg]));
		put_uleb(out, static_cast<uint32_t>(16 - save.offset) / 8);
	}
	// Each epilogue: after its `leave` the CFA is RSP + 8 and the saved
	// registers hold their callers' values; code after its `ret` (another
	// block, the resume terminal) has the body's rules again.
	auto epilogue = [&](uint32_t leave_end, uint32_t ret_end, bool more) {
		cursor.advance(leave_end);
		if (more)
			out.push_back(DW_CFA_remember_state);
		out.push_back(DW_CFA_def_cfa);
		put_uleb(out, kDwarfRsp);
		put_uleb(out, 8);
		out.push_back(DW_CFA_restore | kDwarfRbp);
		for (const UnwindFacts::Save& save : facts.saves)
			out.push_back(static_cast<uint8_t>(DW_CFA_restore | kDwarfRegs[save.reg]));
		if (more)
		{
			cursor.advance(ret_end);
			out.push_back(DW_CFA_restore_state);
		}
	};
	// The duplicated epilogues and the calls' argument sizes, in address
	// order (the cursor only advances).
	uint32_t args = 0;
	size_t next_return = 0;
	for (const UnwindFacts::CallSite& call : facts.calls)
	{
		while (next_return < facts.returns.size() && facts.returns[next_return].leave_end <= call.begin)
		{
			epilogue(facts.returns[next_return].leave_end, facts.returns[next_return].ret_end, true);
			++next_return;
		}
		if (!lsda || call.landing == 0 || call.args == args || call.begin >= facts.ret_end)
			continue;
		cursor.advance(call.begin);
		out.push_back(DW_CFA_GNU_args_size);
		put_uleb(out, call.args);
		args = call.args;
	}
	for (; next_return < facts.returns.size(); ++next_return)
		epilogue(facts.returns[next_return].leave_end, facts.returns[next_return].ret_end, true);
	epilogue(facts.leave_end, facts.ret_end, facts.size > facts.ret_end);
}

void encode_fde(uint32_t cie, uint32_t code_bytes, bool lsda, const uint8_t* instructions, size_t count,
	std::vector<uint8_t>& out, uint32_t& pc_field, uint32_t& lsda_field)
{
	const size_t start = out.size();
	put32(out, 0);
	put32(out, static_cast<uint32_t>(out.size() - cie));
	pc_field = static_cast<uint32_t>(out.size());
	put32(out, 0);
	put32(out, code_bytes);
	put_uleb(out, lsda ? 4 : 0);
	lsda_field = 0;
	if (lsda)
	{
		lsda_field = static_cast<uint32_t>(out.size());
		put32(out, 0);
	}
	out.insert(out.end(), instructions, instructions + count);
	finish_entry(out, start);
}

bool encode_lsda(const UnwindFacts& facts, const Function& function, std::vector<uint8_t>& out,
	std::vector<LsdaTypeField>& types, LsdaScratch& scratch)
{
	std::vector<uint8_t>& used = scratch.used;
	used.assign(function.eh.landings.size(), 0);
	bool landed = false;
	for (const UnwindFacts::CallSite& call : facts.calls)
		if (call.landing != 0)
		{
			used[call.landing - 1] = 1;
			landed = true;
		}
	if (!landed)
		return false;
	// Action records interned by (filter, next record + 1), each chain
	// built from its end; a chain that is only a cleanup is action 0.
	typedef LsdaScratch::Record Record;
	std::vector<Record>& records = scratch.records;
	records.clear();
	scratch.index.clear(function.eh.filters.size());
	FlatMap<uint64_t, uint32_t, LsdaScratch::KeyHash>& index = scratch.index;
	std::vector<uint32_t>& first = scratch.first;
	first.assign(function.eh.landings.size(), 0);
	bool typed = false;
	for (size_t l = 0; l < function.eh.landings.size(); ++l)
	{
		const EhLanding& landing = function.eh.landings[l];
		if (!used[l] || landing.count == 0 || (landing.count == 1 && function.eh.filters[landing.first] == 0))
			continue;
		uint32_t next = 0;
		for (uint32_t c = landing.count; c-- > 0;)
		{
			const int32_t filter = function.eh.filters[landing.first + c];
			typed = typed || filter != 0;
			const uint64_t key = (static_cast<uint64_t>(static_cast<uint32_t>(filter)) << 32) | next;
			if (const uint32_t* known = index.find(key))
				next = *known;
			else
			{
				records.push_back(Record{ filter, next });
				next = static_cast<uint32_t>(records.size());
				index.insert(key, next);
			}
		}
		first[l] = next;
	}
	// Record positions: a record's displacement to the next one depends on
	// the sizes before it; sizes only grow until they fit (a shorter value
	// is padded).
	std::vector<uint32_t>& position = scratch.position;
	std::vector<uint32_t>& size = scratch.size;
	position.assign(records.size(), 0);
	size.assign(records.size(), 0);
	for (size_t r = 0; r < records.size(); ++r)
		size[r] = sleb_size(records[r].filter) + 1;
	auto displacement = [&](size_t r) {
		return records[r].next == 0
			? 0
			: static_cast<int64_t>(position[records[r].next - 1]) - (position[r] + sleb_size(records[r].filter));
	};
	for (bool changed = true; changed;)
	{
		changed = false;
		uint32_t at = 0;
		for (size_t r = 0; r < records.size(); ++r)
		{
			position[r] = at;
			at += size[r];
		}
		for (size_t r = 0; r < records.size(); ++r)
		{
			const uint32_t needed = sleb_size(records[r].filter) + sleb_size(displacement(r));
			if (needed > size[r])
			{
				size[r] = needed;
				changed = true;
			}
		}
	}
	std::vector<uint8_t>& rest = scratch.rest;
	std::vector<uint8_t>& sites = scratch.sites;
	rest.clear();
	sites.clear();
	const UnwindFacts::CallSite* open = nullptr;
	uint32_t open_end = 0;
	auto flush = [&]() {
		if (open == nullptr)
			return;
		put_uleb(sites, open->begin);
		put_uleb(sites, open_end - open->begin);
		put_uleb(sites, open->landing == 0 ? 0 : facts.pads[open->landing - 1]);
		const uint32_t record = open->landing == 0 ? 0 : first[open->landing - 1];
		put_uleb(sites, record == 0 ? 0 : position[record - 1] + 1);
	};
	for (const UnwindFacts::CallSite& call : facts.calls)
	{
		// Landings with the same pad have the same chain.
		if (open != nullptr && open->landing == call.landing)
		{
			open_end = call.end;
			continue;
		}
		flush();
		open = &call;
		open_end = call.end;
	}
	flush();
	rest.push_back(kUleb128);
	put_uleb(rest, sites.size());
	rest.insert(rest.end(), sites.begin(), sites.end());
	for (size_t r = 0; r < records.size(); ++r)
	{
		put_sleb(rest, records[r].filter);
		put_sleb(rest, displacement(r), size[r] - sleb_size(records[r].filter));
	}
	const uint32_t entries = typed ? static_cast<uint32_t>(function.eh.types.size()) : 0;
	const size_t start = out.size();
	out.push_back(kOmit);
	if (entries == 0 && (!typed || function.eh.specs.empty()))
	{
		out.push_back(kOmit);
		out.insert(out.end(), rest.begin(), rest.end());
		return true;
	}
	// The type table ends 4-aligned from the LSDA's start; TTBase's own
	// ULEB128 size affects that, so it may be padded.
	unsigned base_bytes = 1;
	uint32_t padding = 0;
	uint64_t base = 0;
	for (;;)
	{
		const uint64_t before_table = 2 + base_bytes + rest.size();
		padding = static_cast<uint32_t>((4 - before_table % 4) % 4);
		base = rest.size() + padding + 4ull * entries;
		if (uleb_size(base) <= base_bytes)
			break;
		base_bytes = uleb_size(base);
	}
	out.push_back(kIndirectPcrelSdata4);
	put_uleb(out, base, base_bytes);
	out.insert(out.end(), rest.begin(), rest.end());
	out.insert(out.end(), padding, 0);
	const size_t table = out.size();
	out.insert(out.end(), 4ull * entries, 0);
	// The exception specifications' lists after TTBase.
	out.insert(out.end(), function.eh.specs.begin(), function.eh.specs.end());
	// Entry k (from 1) is the k-th 4-byte field below TTBase.
	for (uint32_t k = 1; k <= entries; ++k)
	{
		const uint32_t type = function.eh.types[k - 1];
		if (type != lowir::kNoId)
			types.push_back(LsdaTypeField{ static_cast<uint32_t>(table + 4ull * (entries - k) - start), type });
	}
	return true;
}

} // namespace native
