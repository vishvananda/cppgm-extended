#include "native/link/linker.h"

#include <algorithm>
#include <chrono>
#include <cstring>

#include "native/image/executable_file.h"
#include "native/link/runtime_library.h"
#include "support/flat_map.h"

namespace native {

using object::Binding;
using object::ObjectFile;
using object::Relocation;
using object::Section;
using object::SectionKind;
using object::Symbol;

namespace {

const uint32_t kNone = 0xffffffffu;
const uint64_t kNoSlot = ~static_cast<uint64_t>(0);
// Bounds on inputs: section alignment (larger ones pad with real bytes)
// and the zero-filled storage (the user half of the address space).
const uint64_t kMaxAlign = 1ull << 24;
const uint64_t kMaxSegment = 1ull << 46;

double milliseconds_since(std::chrono::steady_clock::time_point start)
{
	return std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - start).count();
}

uint64_t align_up(uint64_t value, uint64_t align)
{
	return (value + align - 1) & ~(align - 1);
}

// A symbol name in some input's string table.
struct NameKey
{
	const char* text;
	uint32_t length;

	bool operator==(const NameKey& other) const
	{
		return length == other.length && std::memcmp(text, other.text, length) == 0;
	}
};

struct NameHash
{
	size_t operator()(const NameKey& key) const
	{
		uint64_t hash = 0xcbf29ce484222325ull;
		for (uint32_t k = 0; k < key.length; ++k)
			hash = (hash ^ static_cast<uint8_t>(key.text[k])) * 0x100000001b3ull;
		return static_cast<size_t>(hash);
	}
};

struct WordHash
{
	size_t operator()(uint64_t key) const { return static_cast<size_t>(key); }
};

NameKey key_of(const char* text)
{
	return NameKey{ text, static_cast<uint32_t>(std::strlen(text)) };
}

enum class State : uint8_t
{
	Undefined,
	Common,
	Weak,
	Strong,
	Linker // defined by the linker: a startup array bound or the GOT
};

// What State::Linker symbols stand for (GlobalSymbol::offset).
enum LinkerSymbol : uint8_t
{
	kInitStart,
	kInitEnd,
	kFiniStart,
	kFiniEnd,
	kGotBase
};

struct GlobalSymbol
{
	NameKey name;
	State state;
	bool strong_reference;
	uint32_t object; // the definition (Weak, Strong)
	uint32_t symbol;
	uint32_t referenced_from; // the first input with a strong reference
	uint64_t common_size;
	uint64_t common_align;
	uint64_t offset; // Common: in zero-filled storage; Linker: a LinkerSymbol
	uint64_t address;
	uint64_t got; // GOT slot offset in the data, or kNoSlot
};

enum Segment : uint8_t
{
	kNoSegment,
	kText,
	kRodata,
	kData,
	kBss
};

struct Location
{
	uint8_t segment;
	uint64_t offset;
};

class Linker
{
public:
	Linker(std::vector<ObjectFile>& objects, LinkStats& stats, std::string& error)
		: objects_(objects), stats_(stats), error_(error)
	{}

	bool link(const std::string& output);

private:
	bool fail(const std::string& message)
	{
		error_ = message;
		return false;
	}
	bool add_object(uint32_t index);
	bool define(GlobalSymbol& global, State state, uint32_t object, uint32_t symbol);
	bool add_runtime();
	bool check_undefined();
	bool layout();
	bool place(uint32_t object, uint32_t section, std::vector<uint8_t>* buffer, uint8_t segment, uint8_t fill);
	void assign_got();
	void assign_addresses();
	bool relocate();
	bool apply(uint32_t object, const Relocation& relocation, uint8_t* field, uint64_t place, uint64_t room);
	bool target_address(uint32_t object, uint32_t symbol, uint64_t& address);
	uint64_t segment_address(uint8_t segment) const;
	std::vector<uint8_t>* segment_buffer(uint8_t segment);
	bool got_relocation(uint32_t type) const
	{
		return type == object::R_X86_64_GOTPCREL || type == object::R_X86_64_GOTPCRELX ||
			type == object::R_X86_64_REX_GOTPCRELX;
	}
	const char* group_signature(const ObjectFile& file, const object::Group& group) const;

	std::vector<ObjectFile>& objects_;
	LinkStats& stats_;
	std::string& error_;
	FlatMap<NameKey, uint32_t, NameHash> names_;
	FlatMap<NameKey, uint32_t, NameHash> groups_;
	std::vector<GlobalSymbol> globals_;
	std::vector<std::vector<uint32_t>> global_of_;    // per input symbol: its global, or kNone
	std::vector<std::vector<uint8_t>> kept_;          // per input section: loaded (not discarded)
	std::vector<std::vector<Location>> location_;     // per input section: kNoSegment if not loaded
	FlatMap<uint64_t, uint64_t, WordHash> local_got_; // (input << 32 | symbol) -> GOT slot
	std::vector<uint8_t> text_;
	std::vector<uint8_t> rodata_;
	std::vector<uint8_t> data_;
	uint64_t bss_ = 0;
	uint64_t array_bounds_[4] = { 0, 0, 0, 0 };     // init start/end, fini start/end in constants
	uint64_t got_base_ = 0;                         // the GOT's offset in the data
	uint64_t segment_align_[5] = { 1, 1, 1, 1, 1 }; // per Segment: its sections' largest alignment
	ExecutableLayout layout_;
};

bool Linker::define(GlobalSymbol& global, State state, uint32_t object, uint32_t symbol)
{
	if (state == State::Strong && global.state == State::Strong)
		return fail("duplicate symbol: " + std::string(global.name.text, global.name.length) + " (defined in " +
			objects_[global.object].path + " and " + objects_[object].path + ")");
	// gABI: a common symbol takes precedence over weak definitions.
	if (state == State::Strong || global.state == State::Undefined)
	{
		global.state = state;
		global.object = object;
		global.symbol = symbol;
	}
	return true;
}

bool Linker::add_object(uint32_t index)
{
	ObjectFile& file = objects_[index];
	location_[index].assign(file.sections.size(), Location{ kNoSegment, 0 });
	std::vector<uint8_t>& kept = kept_[index];
	kept.assign(file.sections.size(), 0);
	for (size_t s = 1; s < file.sections.size(); ++s)
		kept[s] = file.sections[s].kind != SectionKind::Null && file.sections[s].kind != SectionKind::Metadata &&
			file.sections[s].kind != SectionKind::MetadataStrings;
	for (const object::Group& group : file.groups)
	{
		const NameKey key = key_of(group_signature(file, group));
		if (groups_.find(key) == nullptr)
		{
			groups_.insert(key, index);
			continue;
		}
		// A discarded member's contents are never needed.
		++stats_.discarded_groups;
		for (const uint32_t member : group.sections)
		{
			kept[member] = 0;
			std::vector<uint8_t>().swap(file.sections[member].bytes);
			std::vector<Relocation>().swap(file.sections[member].relocations);
		}
	}
	std::vector<uint32_t>& global_of = global_of_[index];
	global_of.assign(file.symbols.size(), kNone);
	for (uint32_t s = 1; s < file.symbols.size(); ++s)
	{
		const Symbol& symbol = file.symbols[s];
		if (symbol.binding == Binding::Local)
			continue;
		const NameKey key = key_of(file.symbol_name(s));
		uint32_t* found = names_.find(key);
		if (found == nullptr)
		{
			GlobalSymbol global = GlobalSymbol();
			global.name = key;
			global.object = kNone;
			global.referenced_from = kNone;
			global.got = kNoSlot;
			globals_.push_back(global);
			names_.insert(key, static_cast<uint32_t>(globals_.size() - 1));
			found = names_.find(key);
		}
		const uint32_t g = *found;
		global_of[s] = g;
		GlobalSymbol& global = globals_[g];
		const bool regular = symbol.section != object::kUndefinedSection &&
			symbol.section != object::kAbsoluteSection && symbol.section != object::kCommonSection;
		if (symbol.section == object::kCommonSection)
		{
			if (global.state == State::Undefined || global.state == State::Common || global.state == State::Weak)
			{
				global.state = State::Common;
				if (symbol.size > global.common_size)
					global.common_size = symbol.size;
				if (symbol.value > global.common_align)
					global.common_align = symbol.value;
			}
			continue;
		}
		if (symbol.section == object::kAbsoluteSection || (regular && kept[symbol.section]))
		{
			if (!define(global, symbol.binding == Binding::Weak ? State::Weak : State::Strong, index, s))
				return false;
		}
	}
	// A global symbol a loaded section's relocation uses must be defined
	// (an unused undefined symbol, or one only discarded code uses, need
	// not be).
	for (uint32_t s = 1; s < file.sections.size(); ++s)
	{
		if (!kept[s])
			continue;
		for (const Relocation& relocation : file.sections[s].relocations)
		{
			if (relocation.symbol >= global_of.size() || global_of[relocation.symbol] == kNone ||
				file.symbols[relocation.symbol].binding != Binding::Global)
				continue;
			GlobalSymbol& global = globals_[global_of[relocation.symbol]];
			global.strong_reference = true;
			if (global.referenced_from == kNone)
				global.referenced_from = index;
		}
	}
	return true;
}

const char* Linker::group_signature(const ObjectFile& file, const object::Group& group) const
{
	// A section symbol (no name) signs its section's group by the
	// section's name, as GNU as writes a signature equal to that name.
	const Symbol& symbol = file.symbols[group.signature];
	if (symbol.type == object::SymbolType::Section && symbol.name == 0 && symbol.section < file.sections.size())
		return file.string(file.sections[symbol.section].name);
	return file.symbol_name(group.signature);
}

bool Linker::add_runtime()
{
	// The runtime library defines what is still undefined, the process
	// entry first among them.
	const std::chrono::steady_clock::time_point start = std::chrono::steady_clock::now();
	std::vector<std::string> wanted;
	for (const GlobalSymbol& global : globals_)
	{
		if (global.state != State::Undefined)
			continue;
		const std::string name(global.name.text, global.name.length);
		if (runtime_provides(name.c_str()))
			wanted.push_back(name);
	}
	const uint32_t first = static_cast<uint32_t>(objects_.size());
	if (!build_runtime(wanted, objects_, error_))
		return false;
	stats_.runtime_members = wanted.size() + 1;
	global_of_.resize(objects_.size());
	kept_.resize(objects_.size());
	location_.resize(objects_.size());
	for (uint32_t o = first; o < objects_.size(); ++o)
	{
		if (!add_object(o))
			return false;
	}
	// psABI: the link editor defines _GLOBAL_OFFSET_TABLE_.
	const char* const bounds[5] = { kInitArrayStart, kInitArrayEnd, kFiniArrayStart, kFiniArrayEnd,
		"_GLOBAL_OFFSET_TABLE_" };
	for (unsigned k = 0; k < 5; ++k)
	{
		uint32_t* found = names_.find(key_of(bounds[k]));
		if (found != nullptr && globals_[*found].state == State::Undefined)
		{
			globals_[*found].state = State::Linker;
			globals_[*found].offset = k;
		}
	}
	stats_.runtime_ms = milliseconds_since(start);
	return true;
}

bool Linker::check_undefined()
{
	std::string missing;
	unsigned count = 0;
	bool host_eh = false;
	for (const GlobalSymbol& global : globals_)
	{
		if (global.state != State::Undefined || !global.strong_reference)
			continue;
		const std::string name(global.name.text, global.name.length);
		host_eh = host_eh || name == "__gxx_personality_v0" || name == "_Unwind_Resume";
		if (++count > 8)
			continue;
		missing += missing.empty() ? "" : ", ";
		missing.append(global.name.text, global.name.length);
		if (global.referenced_from != kNone)
			missing += " (referenced from " + objects_[global.referenced_from].path + ")";
	}
	if (count == 0)
		return true;
	if (count > 8)
		missing += ", ...";
	// Host EH tables need the host C++ runtime, which link mode does not
	// provide: cppgm++ -c writes course objects for *.obj outputs.
	if (host_eh)
		missing += "; an object with host EH tables needs the host C++ toolchain's link (for link mode, compile "
				   "the source with cppgm++ -c to a *.obj course object)";
	return fail("undefined symbol" + std::string(count > 1 ? "s: " : ": ") + missing);
}

// The init (or fini) array position of a startup section: .preinit_array
// first, then .init_array.N / .fini_array.N by priority N (GNU ld's
// SORT_BY_INIT_PRIORITY), then the unprioritized sections.
uint32_t startup_order(const char* name)
{
	if (std::strcmp(name, ".preinit_array") == 0)
		return 0;
	const char* dot = std::strrchr(name, '.');
	if (dot == nullptr || dot == name || dot[1] == '\0')
		return 0x20000;
	uint32_t priority = 0;
	for (const char* c = dot + 1; *c != '\0'; ++c)
	{
		if (*c < '0' || *c > '9' || priority > 0xffff)
			return 0x20000;
		priority = priority * 10 + static_cast<uint32_t>(*c - '0');
	}
	return 1 + (priority > 0xffff ? 0xffff : priority);
}

bool Linker::place(uint32_t object, uint32_t section, std::vector<uint8_t>* buffer, uint8_t segment, uint8_t fill)
{
	Section& from = objects_[object].sections[section];
	if (from.align > kMaxAlign)
		return fail(objects_[object].path + ": section alignment beyond " + std::to_string(kMaxAlign));
	if (from.align > segment_align_[segment])
		segment_align_[segment] = from.align;
	uint64_t offset;
	if (buffer == nullptr)
	{
		bss_ = align_up(bss_, from.align);
		if (from.size > kMaxSegment - bss_)
			return fail(objects_[object].path + ": zero-filled storage beyond the address space");
		offset = bss_;
		bss_ += from.size;
	}
	else
	{
		offset = align_up(buffer->size(), from.align);
		buffer->resize(offset, fill);
		buffer->insert(buffer->end(), from.bytes.begin(), from.bytes.end());
		// The relocations are applied to the segment's copy.
		std::vector<uint8_t>().swap(from.bytes);
	}
	location_[object][section] = Location{ segment, offset };
	++stats_.sections;
	return true;
}

bool Linker::layout()
{
	// The segment buffers are sized first, so placing never reallocates
	// them while the inputs are still held.
	uint64_t sizes[5] = { 0, 0, 0, 0, 0 };
	for (uint32_t o = 0; o < objects_.size(); ++o)
	{
		for (uint32_t s = 1; s < objects_[o].sections.size(); ++s)
		{
			const Section& section = objects_[o].sections[s];
			if (!kept_[o][s] || section.align > kMaxAlign)
				continue;
			const SectionKind kind = section.kind;
			const unsigned segment = kind == SectionKind::Text ? kText
				: kind == SectionKind::Data                    ? kData
				: kind == SectionKind::Rodata || kind == SectionKind::InitArray || kind == SectionKind::FiniArray
				? kRodata
				: 0;
			if (segment != 0)
				sizes[segment] = align_up(sizes[segment], section.align) + section.size;
		}
	}
	text_.reserve(sizes[kText]);
	rodata_.reserve(sizes[kRodata] + 16);
	data_.reserve(sizes[kData]);
	// Discarded and unloaded sections keep kNoSegment.
	for (uint32_t o = 0; o < objects_.size(); ++o)
	{
		for (uint32_t s = 1; s < objects_[o].sections.size(); ++s)
		{
			if (!kept_[o][s])
				continue;
			bool placed = true;
			switch (objects_[o].sections[s].kind)
			{
			case SectionKind::Text:
				placed = place(o, s, &text_, kText, 0xcc);
				break;
			case SectionKind::Rodata:
				placed = place(o, s, &rodata_, kRodata, 0);
				break;
			case SectionKind::Data:
				placed = place(o, s, &data_, kData, 0);
				break;
			case SectionKind::Bss:
				placed = place(o, s, nullptr, kBss, 0);
				break;
			default:
				break;
			}
			if (!placed)
				return false;
		}
	}
	// The startup arrays follow the constants, each in priority, then
	// input order.
	std::vector<std::pair<uint64_t, uint64_t>> order;
	for (unsigned array = 0; array < 2; ++array)
	{
		const SectionKind kind = array == 0 ? SectionKind::InitArray : SectionKind::FiniArray;
		order.clear();
		for (uint32_t o = 0; o < objects_.size(); ++o)
		{
			for (uint32_t s = 1; s < objects_[o].sections.size(); ++s)
			{
				const Section& section = objects_[o].sections[s];
				if (section.kind == kind && kept_[o][s])
					order.push_back(
						std::make_pair(static_cast<uint64_t>(startup_order(objects_[o].string(section.name))),
							(static_cast<uint64_t>(o) << 32) | s));
			}
		}
		std::sort(order.begin(), order.end());
		rodata_.resize(align_up(rodata_.size(), 8), 0);
		array_bounds_[2 * array] = rodata_.size();
		for (const std::pair<uint64_t, uint64_t>& entry : order)
		{
			if (!place(static_cast<uint32_t>(entry.second >> 32), static_cast<uint32_t>(entry.second), &rodata_,
					kRodata, 0))
				return false;
		}
		array_bounds_[2 * array + 1] = rodata_.size();
	}
	for (GlobalSymbol& global : globals_)
	{
		if (global.state != State::Common)
			continue;
		const uint64_t align = global.common_align == 0 ? 1 : global.common_align;
		if (align > kMaxAlign || global.common_size > kMaxSegment - align_up(bss_, align))
			return fail("common symbol " + std::string(global.name.text, global.name.length) + " is too large");
		if (align > segment_align_[kBss])
			segment_align_[kBss] = align;
		bss_ = align_up(bss_, align);
		global.offset = bss_;
		bss_ += global.common_size;
	}
	assign_got();
	return true;
}

void Linker::assign_got()
{
	// One slot per symbol a GOT-relative reference names.
	data_.resize(align_up(data_.size(), 8), 0);
	got_base_ = data_.size();
	for (uint32_t o = 0; o < objects_.size(); ++o)
	{
		const ObjectFile& file = objects_[o];
		for (uint32_t s = 1; s < file.sections.size(); ++s)
		{
			if (location_[o][s].segment == kNoSegment)
				continue;
			for (const Relocation& relocation : file.sections[s].relocations)
			{
				if (!got_relocation(relocation.type))
					continue;
				const uint32_t g = global_of_[o][relocation.symbol];
				data_.resize(align_up(data_.size(), 8), 0);
				if (g != kNone)
				{
					if (globals_[g].got == kNoSlot)
					{
						globals_[g].got = data_.size();
						data_.resize(data_.size() + 8, 0);
						++stats_.got_slots;
					}
					continue;
				}
				const uint64_t key = (static_cast<uint64_t>(o) << 32) | relocation.symbol;
				if (local_got_.find(key) == nullptr)
				{
					local_got_.insert(key, data_.size());
					data_.resize(data_.size() + 8, 0);
					++stats_.got_slots;
				}
			}
		}
	}
}

uint64_t Linker::segment_address(uint8_t segment) const
{
	switch (segment)
	{
	case kText:
		return layout_.text_address;
	case kRodata:
		return layout_.rodata_address;
	case kData:
		return layout_.data_address;
	case kBss:
		return layout_.bss_address;
	default:
		return 0;
	}
}

std::vector<uint8_t>* Linker::segment_buffer(uint8_t segment)
{
	return segment == kText ? &text_ : segment == kRodata ? &rodata_ : segment == kData ? &data_ : nullptr;
}

bool Linker::target_address(uint32_t object, uint32_t symbol, uint64_t& address)
{
	// gABI: STN_UNDEF gives the value 0.
	if (symbol == 0)
	{
		address = 0;
		return true;
	}
	const uint32_t g = global_of_[object][symbol];
	if (g != kNone)
	{
		address = globals_[g].address;
		return true;
	}
	const Symbol& local = objects_[object].symbols[symbol];
	if (local.section == object::kAbsoluteSection)
	{
		address = local.value;
		return true;
	}
	if (local.section == object::kUndefinedSection || local.section == object::kCommonSection)
		return fail(objects_[object].path + ": relocation against an undefined local symbol");
	const Location& location = location_[object][local.section];
	if (location.segment == kNoSegment)
		return fail(objects_[object].path + ": relocation against a symbol in a discarded section");
	address = segment_address(location.segment) + location.offset + local.value;
	return true;
}

void Linker::assign_addresses()
{
	layout_ = layout_executable(text_.size(), rodata_.size(), data_.size(), bss_, segment_align_[kText],
		segment_align_[kRodata], segment_align_[kData], segment_align_[kBss]);
	for (GlobalSymbol& global : globals_)
	{
		switch (global.state)
		{
		case State::Common:
			global.address = layout_.bss_address + global.offset;
			break;
		case State::Linker:
			global.address = global.offset == kGotBase ? layout_.data_address + got_base_
													   : layout_.rodata_address + array_bounds_[global.offset];
			break;
		case State::Weak:
		case State::Strong:
		{
			const Symbol& symbol = objects_[global.object].symbols[global.symbol];
			if (symbol.section == object::kAbsoluteSection)
				global.address = symbol.value;
			else
			{
				const Location& location = location_[global.object][symbol.section];
				global.address = segment_address(location.segment) + location.offset + symbol.value;
			}
			break;
		}
		default:
			global.address = 0;
			break;
		}
		if (global.got != kNoSlot)
		{
			for (unsigned k = 0; k < 8; ++k)
				data_[global.got + k] = static_cast<uint8_t>(global.address >> (8 * k));
		}
	}
}

bool Linker::apply(uint32_t object, const Relocation& relocation, uint8_t* field, uint64_t place, uint64_t room)
{
	uint64_t symbol = 0;
	if (relocation.type != object::R_X86_64_NONE && !target_address(object, relocation.symbol, symbol))
		return false;
	if (got_relocation(relocation.type))
	{
		const uint32_t g = global_of_[object][relocation.symbol];
		uint64_t slot;
		if (g != kNone)
			slot = globals_[g].got;
		else
		{
			slot = *local_got_.find((static_cast<uint64_t>(object) << 32) | relocation.symbol);
			for (unsigned k = 0; k < 8; ++k)
				data_[slot + k] = static_cast<uint8_t>(symbol >> (8 * k));
		}
		symbol = layout_.data_address + slot;
	}
	// The GOT-based forms (psABI 4.4.1): GOTOFF64 is S + A - GOT, GOTPC*
	// GOT + A - P.
	const uint64_t got = layout_.data_address + got_base_;
	if (relocation.type == object::R_X86_64_GOTOFF64)
		symbol -= got;
	else if (relocation.type == object::R_X86_64_GOTPC32 || relocation.type == object::R_X86_64_GOTPC64)
		symbol = got;
	const uint64_t value = symbol + static_cast<uint64_t>(relocation.addend);
	const int64_t relative = static_cast<int64_t>(value - place);
	unsigned bytes = 0;
	uint64_t stored = value;
	bool fits = true;
	switch (relocation.type)
	{
	case object::R_X86_64_NONE:
		return true;
	case object::R_X86_64_64:
		bytes = 8;
		break;
	case object::R_X86_64_GOTOFF64:
		bytes = 8;
		break;
	case object::R_X86_64_PC64:
	case object::R_X86_64_GOTPC64:
		bytes = 8;
		stored = static_cast<uint64_t>(relative);
		break;
	case object::R_X86_64_PC32:
	case object::R_X86_64_PLT32:
	case object::R_X86_64_GOTPC32:
	case object::R_X86_64_GOTPCREL:
	case object::R_X86_64_GOTPCRELX:
	case object::R_X86_64_REX_GOTPCRELX:
		bytes = 4;
		stored = static_cast<uint64_t>(relative);
		fits = relative >= INT32_MIN && relative <= INT32_MAX;
		break;
	case object::R_X86_64_32:
		bytes = 4;
		fits = value <= UINT32_MAX;
		break;
	case object::R_X86_64_32S:
		bytes = 4;
		fits = static_cast<int64_t>(value) >= INT32_MIN && static_cast<int64_t>(value) <= INT32_MAX;
		break;
	case object::R_X86_64_16:
		bytes = 2;
		fits = value <= UINT16_MAX || (static_cast<int64_t>(value) < 0 && static_cast<int64_t>(value) >= INT16_MIN);
		break;
	case object::R_X86_64_PC16:
		bytes = 2;
		stored = static_cast<uint64_t>(relative);
		fits = relative >= INT16_MIN && relative <= INT16_MAX;
		break;
	case object::R_X86_64_8:
		bytes = 1;
		fits = value <= UINT8_MAX || (static_cast<int64_t>(value) < 0 && static_cast<int64_t>(value) >= INT8_MIN);
		break;
	case object::R_X86_64_PC8:
		bytes = 1;
		stored = static_cast<uint64_t>(relative);
		fits = relative >= INT8_MIN && relative <= INT8_MAX;
		break;
	default:
		return fail(objects_[object].path + ": unsupported relocation type " + std::to_string(relocation.type));
	}
	if (bytes > room)
		return fail(objects_[object].path + ": relocation outside its section");
	if (!fits)
		return fail(objects_[object].path + ": relocation value out of range for " +
			std::string(objects_[object].symbol_name(relocation.symbol)));
	for (unsigned k = 0; k < bytes; ++k)
		field[k] = static_cast<uint8_t>(stored >> (8 * k));
	return true;
}

bool Linker::relocate()
{
	for (uint32_t o = 0; o < objects_.size(); ++o)
	{
		const ObjectFile& file = objects_[o];
		for (uint32_t s = 1; s < file.sections.size(); ++s)
		{
			const Location& location = location_[o][s];
			const std::vector<Relocation>& relocations = file.sections[s].relocations;
			if (location.segment == kNoSegment || relocations.empty())
				continue;
			std::vector<uint8_t>* buffer = segment_buffer(location.segment);
			if (buffer == nullptr)
				return fail(file.path + ": relocations in zero-filled storage");
			const uint64_t base = segment_address(location.segment) + location.offset;
			const uint64_t size = file.sections[s].size;
			for (const Relocation& relocation : relocations)
			{
				if (relocation.offset >= size)
					return fail(file.path + ": relocation outside its section");
				if (!apply(o, relocation, buffer->data() + location.offset + relocation.offset,
						base + relocation.offset, size - relocation.offset))
					return false;
				++stats_.relocations;
			}
		}
	}
	return true;
}

bool Linker::link(const std::string& output)
{
	const std::chrono::steady_clock::time_point start = std::chrono::steady_clock::now();
	objects_.reserve(objects_.size() + 2);
	global_of_.resize(objects_.size());
	kept_.resize(objects_.size());
	location_.resize(objects_.size());
	for (uint32_t o = 0; o < objects_.size(); ++o)
	{
		if (!add_object(o))
			return false;
	}
	if (!add_runtime() || !check_undefined())
		return false;
	if (!layout())
		return false;
	assign_addresses();
	if (!relocate())
		return false;
	uint32_t* entry = names_.find(key_of(kEntrySymbol));
	if (entry == nullptr || globals_[*entry].state == State::Undefined)
		return fail("no process entry");
	stats_.objects = objects_.size();
	stats_.symbols = globals_.size();
	stats_.text_bytes = text_.size();
	stats_.rodata_bytes = rodata_.size();
	stats_.data_bytes = data_.size();
	stats_.bss_bytes = bss_;
	const bool written = write_executable_file(
		output, layout_, text_, rodata_, data_, bss_, globals_[*entry].address, error_);
	stats_.link_ms = milliseconds_since(start);
	return written;
}

} // namespace

bool link_executable(
	std::vector<object::ObjectFile>& objects, const std::string& output, std::string& error, LinkStats* stats)
{
	LinkStats local;
	Linker linker(objects, stats != nullptr ? *stats : local, error);
	return linker.link(output);
}

} // namespace native
