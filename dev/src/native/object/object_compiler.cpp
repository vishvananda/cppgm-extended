#include "native/object/object_compiler.h"

#include <chrono>
#include <cstring>

#include "native/backend_error.h"
#include "native/image/global_data.h"
#include "native/image/image.h"
#include "native/object/function_census.h"
#include "native/object/host_unwind.h"
#include "native/runtime/runtime.h"
#include "support/flat_map.h"
#include "support/internal_error.h"

namespace native {

using object::Binding;
using object::ObjectFile;
using object::SectionKind;
using object::SymbolType;

namespace {

double milliseconds_since(std::chrono::steady_clock::time_point start)
{
	return std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - start).count();
}

uint64_t align_up(uint64_t value, uint64_t align)
{
	return (value + align - 1) & ~(align - 1);
}

// Where an image symbol was placed: an object section (0: undefined).
struct Placement
{
	uint32_t section;
	uint64_t offset;
	uint64_t size;
};

// One encoded function's bytes in the image's code buffer and the object
// section they went to.
struct CodeRange
{
	uint32_t symbol;
	uint64_t start;
	uint64_t end;
	uint32_t section;
	uint64_t base;
	bool dropped; // an unreferenced unit-local function (prune_functions)
};

// A data relocation recorded before the object symbols exist.
struct PendingRelocation
{
	uint32_t section;
	uint64_t offset;
	uint32_t type;
	uint32_t target; // image symbol
	int64_t addend;
};

// Host EH: one function's tables until placement (runs of the pools),
// with its code range (an index in ranges_).
struct UnwindEntry
{
	uint32_t range;
	uint32_t cfi_first;
	uint32_t cfi_count;
	uint32_t lsda_first;
	uint32_t lsda_count; // 0: no LSDA
	uint32_t types_first;
	uint32_t types_count;
};

// Host EH: a pc-relative relocation of .eh_frame or .gcc_except_table
// recorded before the object symbols exist, against a DW.ref cell (an
// index in cells_) or a section's symbol.
struct UnwindRelocation
{
	uint32_t section;
	uint64_t offset;
	uint32_t cell; // kNoId: target_section's symbol
	uint32_t target_section;
	int64_t addend;
};

struct WordHash32
{
	size_t operator()(uint32_t key) const { return key; }
};

const uint32_t kNoCell = 0xffffffffu;

// The builtin routines' host names (host objects): the C library's, and
// the Itanium names of the global allocation functions.
const unsigned kHostBuiltinCount = 9;
const char* const kHostBuiltins[kHostBuiltinCount][2] = { { "cppgm_builtin_strlen", "strlen" },
	{ "cppgm_builtin_memcpy", "memcpy" }, { "cppgm_builtin_memmove", "memmove" }, { "cppgm_builtin_memset", "memset" },
	{ "cppgm_builtin_memcmp", "memcmp" }, { "cppgm_builtin_operator_new", "_Znwm" },
	{ "cppgm_builtin_operator_new_array", "_Znam" }, { "cppgm_builtin_operator_delete", "_ZdlPv" },
	{ "cppgm_builtin_operator_delete_array", "_ZdaPv" } };

} // namespace

class ObjectCompiler
{
public:
	ObjectCompiler(lowir::Program& program, ObjectStats& stats, EhModel eh_model, int optimization_level);

	// Selects and encodes one function definition.
	void function(lowir::SymbolId symbol);
	void finish(ObjectFile& out);

private:
	// The program's jump thunk defining `symbol`, or null (the program's
	// list indexed as it grows: the lowering adds a thunk before its body).
	const lowir::JumpThunk* jump_thunk(lowir::SymbolId symbol);
	void declare_symbols();
	void emit_remaining();
	bool local_function(uint32_t image_symbol) const;
	void prune_functions();
	void place_text();
	void place_globals();
	void place_startup();
	void record_unwind(uint32_t range, bool encoded);
	void place_unwind();
	uint32_t cell(uint32_t image_symbol);
	void add_unwind_relocations();
	uint32_t section_symbol(uint32_t section);
	void add_symbols();
	void add_aliases();
	void add_relocations();
	void add_groups();
	uint32_t object_symbol(uint32_t image_symbol);
	// A function symbol of the program this object does not define.
	bool external_function(uint32_t image_symbol);
	uint32_t shared_section(const char* name, SectionKind kind);
	uint32_t own_section(const char* name, lowir::SymbolId symbol, SectionKind kind, uint32_t& spelling);
	uint32_t shared_relro_section();
	uint32_t named_section(lowir::NameId name, SectionKind kind);
	lowir::NameId object_name(lowir::SymbolId symbol) const;
	bool comdat_definition(lowir::SymbolId symbol) const;
	uint32_t define_alias(lowir::NameId name, uint32_t target);
	// Image symbols' slots in the per-symbol tables: the program's, then
	// the image's own.
	uint32_t slot(uint32_t image_symbol) const
	{
		return image_symbol < Image::kLocalSymbols ? image_symbol
												   : program_symbols_ + (image_symbol - Image::kLocalSymbols);
	}

	lowir::Program& program_;
	ObjectFile* out_ = nullptr;
	ObjectStats& stats_;
	Image image_;
	RuntimeSymbols runtime_;
	Selector selector_;
	FunctionEmitter emitter_;
	Function mir_;
	std::vector<uint8_t> emitted_;           // per program symbol
	std::vector<uint32_t> jump_thunk_index_; // per program symbol: 1 + its jump thunk, 0 none
	size_t jump_thunks_indexed_ = 0;
	std::vector<const char*> runtime_names_; // the first image-local symbols
	uint32_t program_symbols_ = 0;
	std::vector<Placement> placement_;
	std::vector<uint32_t> object_symbol_;
	std::vector<uint32_t> by_name_; // NameId -> non-local object symbol
	std::vector<CodeRange> ranges_;
	std::vector<PendingRelocation> pending_;
	std::vector<uint32_t> grouped_; // image symbols with their own COMDAT section
	// Host EH tables: per function, the pools, the DW.ref cells (target
	// image symbol, .data offset, object symbol), their index, the
	// relocations awaiting symbols, each section's symbol and the extra
	// members of COMDAT groups (image symbol, section).
	EhModel eh_model_;
	std::vector<UnwindEntry> unwind_;
	std::vector<uint8_t> cfi_pool_;
	std::vector<uint8_t> lsda_pool_;
	std::vector<LsdaTypeField> type_fields_;
	LsdaScratch lsda_scratch_;
	std::string spelling_; // scratch: an internal or DW.ref symbol's name
	struct Cell
	{
		uint32_t target;
		uint64_t offset;
		uint32_t symbol;
	};
	std::vector<Cell> cells_;
	FlatMap<uint32_t, uint32_t, WordHash32> cell_index_;
	std::vector<UnwindRelocation> unwind_relocations_;
	std::vector<uint32_t> section_symbols_;
	std::vector<std::pair<uint32_t, uint32_t>> group_members_;
	std::pair<lowir::NameId, lowir::NameId> host_builtins_[kHostBuiltinCount];
	uint32_t personality_ = 0;
	uint32_t text_ = 0;
	uint32_t rodata_ = 0;
	uint32_t data_ = 0;
	uint32_t bss_ = 0;
	uint32_t relro_ = 0;
	uint32_t tdata_ = 0;
	uint32_t tbss_ = 0;
	uint32_t section_names_[9] = { 0, 0, 0, 0, 0, 0, 0, 0, 0 }; // by SectionKind
	uint32_t relro_name_ = 0;
	// Sections named by globals' section= metadata (name -> section, 0
	// none; sized on the first).
	std::vector<uint32_t> named_sections_;
};

ObjectCompiler::ObjectCompiler(lowir::Program& program, ObjectStats& stats, EhModel eh_model, int optimization_level)
	: program_(program), stats_(stats), selector_(program), emitter_(image_, runtime_, stats.emit), eh_model_(eh_model)
{
	selector_.set_eh_model(eh_model);
	selector_.set_optimization_level(optimization_level);
	emitter_.set_eh_model(eh_model);
	for (unsigned k = 0; k < kHostBuiltinCount; ++k)
		host_builtins_[k] = std::make_pair(0u, 0u);
	declare_symbols();
}

lowir::NameId ObjectCompiler::object_name(lowir::SymbolId symbol) const
{
	const lowir::Symbol& s = program_.symbols[symbol];
	const lowir::NameId name = s.metadata.object != 0 ? s.metadata.object : s.name;
	for (unsigned k = 0; k < kHostBuiltinCount; ++k)
		if (host_builtins_[k].first == name && name != 0)
			return host_builtins_[k].second;
	return name;
}

bool ObjectCompiler::comdat_definition(lowir::SymbolId symbol) const
{
	// A vague-linkage definition, which every unit needing it emits: its
	// own section in a COMDAT group. A GNU weak one, which a strong
	// definition replaces, stays in the shared section.
	const lowir::Symbol& s = program_.symbols[symbol];
	return (s.kind == lowir::SymbolKind::FunctionDefinition || s.kind == lowir::SymbolKind::GlobalDefinition) &&
		s.metadata.binding == ir_model::SBM_WEAK && (s.metadata.flags & lowir::SF_REPLACEABLE) == 0;
}

uint32_t ObjectCompiler::shared_section(const char* name, SectionKind kind)
{
	uint32_t& index = kind == SectionKind::Text ? text_
		: kind == SectionKind::Rodata           ? rodata_
		: kind == SectionKind::Data             ? data_
		: kind == SectionKind::TlsData          ? tdata_
		: kind == SectionKind::TlsBss           ? tbss_
												: bss_;
	if (index == 0)
		index = out_->add_section(name, kind, 1);
	return index;
}

uint32_t ObjectCompiler::own_section(const char* name, lowir::SymbolId symbol, SectionKind kind, uint32_t& spelling)
{
	// COMDAT members share their section name (the group's signature
	// identifies them): one string however many groups.
	if (spelling == 0)
		spelling = out_->add_string(name, std::strlen(name));
	grouped_.push_back(symbol);
	return out_->add_section(spelling, kind, 1);
}

uint32_t ObjectCompiler::shared_relro_section()
{
	if (relro_ == 0)
		relro_ = out_->add_section(".data.rel.ro", SectionKind::Data, 1);
	return relro_;
}

uint32_t ObjectCompiler::named_section(lowir::NameId name, SectionKind kind)
{
	// pa27 README 9: a global's section= name is its ELF section, shared
	// by every global naming it (writable if any of them is), PROGBITS
	// even for zero bytes, as GNU as makes an unknown-named section.
	if (named_sections_.size() <= name)
		named_sections_.resize(program_.names.size() > name ? program_.names.size() : name + 1, 0);
	uint32_t& index = named_sections_[name];
	if (index == 0)
		index = out_->add_section(program_.spelling(name), kind, 1);
	else if (kind == SectionKind::Data)
		out_->sections[index].kind = SectionKind::Data;
	return index;
}

void ObjectCompiler::declare_symbols()
{
	const char* const names[] = { kEhTopSymbol, kEhPayloadSymbol, kEhSelectorSymbol, kEhDispatchSymbol,
		kEhUncaughtSymbol, kEhCatchSymbol };
	uint32_t* const slots[] = { &runtime_.eh_top, &runtime_.eh_payload, &runtime_.eh_selector, &runtime_.eh_dispatch,
		&runtime_.eh_uncaught, &runtime_.eh_catch };
	for (unsigned k = 0; k < 6; ++k)
	{
		*slots[k] = image_.add_symbol();
		runtime_names_.push_back(names[k]);
	}
	for (unsigned k = 0; k < 4; ++k)
	{
		runtime_.wide_division[k] = image_.add_symbol();
		runtime_names_.push_back(kWideDivisionSymbols[k]);
	}
	for (unsigned k = 0; k < 12; ++k)
	{
		runtime_.float_conversion[k] = image_.add_symbol();
		runtime_names_.push_back(kFloatConversionSymbols[k]);
	}
	// The host EH runtime (host objects).
	runtime_.unwind_resume = image_.add_symbol();
	runtime_names_.push_back("_Unwind_Resume");
	personality_ = image_.add_symbol();
	runtime_names_.push_back("__gxx_personality_v0");
}

const lowir::JumpThunk* ObjectCompiler::jump_thunk(lowir::SymbolId symbol)
{
	for (; jump_thunks_indexed_ < program_.jump_thunks.size(); ++jump_thunks_indexed_)
	{
		const lowir::SymbolId thunk = program_.jump_thunks[jump_thunks_indexed_].symbol;
		if (thunk >= jump_thunk_index_.size())
			jump_thunk_index_.resize(std::max<size_t>(thunk + 1, jump_thunk_index_.size() * 2), 0);
		jump_thunk_index_[thunk] = static_cast<uint32_t>(jump_thunks_indexed_ + 1);
	}
	return symbol < jump_thunk_index_.size() && jump_thunk_index_[symbol] != 0
		? &program_.jump_thunks[jump_thunk_index_[symbol] - 1]
		: nullptr;
}

void ObjectCompiler::function(lowir::SymbolId symbol)
{
	// A jump thunk is its adjustment and a jump (no frame: the CIE's rule
	// describes it), whatever its body.
	if (const lowir::JumpThunk* thunk = jump_thunk(symbol))
	{
		emit_jump_thunk(
			image_, symbol, thunk->target, thunk->this_index == 0 ? RDI : RSI, thunk->adjustment, thunk->vcall_offset);
		ranges_.push_back(CodeRange{ symbol, image_.symbol(symbol).offset, image_.text().size(), 0, 0, false });
		if (symbol >= emitted_.size())
			emitted_.resize(std::max<size_t>(symbol + 1, emitted_.size() * 2), 0);
		emitted_[symbol] = 1;
		if (eh_model_ == EhModel::Host)
			record_unwind(static_cast<uint32_t>(ranges_.size() - 1), false);
		return;
	}
	if (!stats_.clocks && !stats_.census)
	{
		selector_.select(symbol, mir_);
		emitter_.emit(mir_);
	}
	else
	{
		const SelectStats before = selector_.stats();
		std::chrono::steady_clock::time_point phase = std::chrono::steady_clock::now();
		selector_.select(symbol, mir_);
		stats_.select_ms += milliseconds_since(phase);
		if (stats_.census)
			append_function_census(program_, mir_, before, selector_.stats(), stats_.census_lines);
		phase = std::chrono::steady_clock::now();
		emitter_.emit(mir_);
		stats_.encode_ms += milliseconds_since(phase);
	}
	ranges_.push_back(CodeRange{ symbol, image_.symbol(symbol).offset, image_.text().size(), 0, 0, false });
	if (symbol >= emitted_.size())
		emitted_.resize(std::max<size_t>(symbol + 1, emitted_.size() * 2), 0);
	emitted_[symbol] = 1;
	if (eh_model_ == EhModel::Host)
		record_unwind(static_cast<uint32_t>(ranges_.size() - 1), true);
}

// Host EH: the function just encoded (or, without MIR, a TLS wrapper: a
// leaf the CIE's rule describes) gets its FDE instructions and LSDA.
void ObjectCompiler::record_unwind(uint32_t range, bool encoded)
{
	UnwindEntry entry = UnwindEntry();
	entry.range = range;
	entry.cfi_first = static_cast<uint32_t>(cfi_pool_.size());
	entry.lsda_first = static_cast<uint32_t>(lsda_pool_.size());
	entry.types_first = static_cast<uint32_t>(type_fields_.size());
	bool lsda = false;
	if (encoded)
	{
		const UnwindFacts& facts = emitter_.unwind();
		const size_t before = lsda_pool_.size();
		lsda = encode_lsda(facts, mir_, lsda_pool_, type_fields_, lsda_scratch_);
		encode_frame_instructions(facts, lsda, cfi_pool_);
		entry.lsda_count = static_cast<uint32_t>(lsda_pool_.size() - before);
		stats_.landings += mir_.eh.landings.size();
		stats_.lsdas += lsda ? 1 : 0;
	}
	entry.cfi_count = static_cast<uint32_t>(cfi_pool_.size()) - entry.cfi_first;
	entry.types_count = static_cast<uint32_t>(type_fields_.size()) - entry.types_first;
	unwind_.push_back(entry);
}

void ObjectCompiler::emit_remaining()
{
	// Definitions no sink delivered, in program order, their bodies
	// released once encoded.
	for (size_t i = 0; i < program_.order.size(); ++i)
	{
		const lowir::SymbolId s = program_.order[i];
		const lowir::Symbol& symbol = program_.symbols[s];
		if (symbol.kind != lowir::SymbolKind::FunctionDefinition || (s < emitted_.size() && emitted_[s]))
			continue;
		function(s);
		program_.functions[symbol.detail].body.reset();
	}
	stats_.select = selector_.stats();
	// A declared thread-local wrapper returns its object's address.
	for (lowir::SymbolId s = 0; s < program_.symbols.size(); ++s)
	{
		const lowir::Symbol& symbol = program_.symbols[s];
		if (symbol.kind != lowir::SymbolKind::FunctionDeclaration || symbol.metadata.tls_for == lowir::kNoId)
			continue;
		emit_tls_wrapper(image_, s, symbol.metadata.tls_for);
		ranges_.push_back(CodeRange{ s, image_.symbol(s).offset, image_.text().size(), 0, 0, false });
		if (eh_model_ == EhModel::Host)
			record_unwind(static_cast<uint32_t>(ranges_.size() - 1), false);
	}
}

bool ObjectCompiler::local_function(uint32_t image_symbol) const
{
	// A function no other object can name (a local symbol, object_symbol):
	// internal, or a thread-local wrapper the unit defines for itself.
	const lowir::Symbol& s = program_.symbols[image_symbol];
	if (s.metadata.role != ir_model::SR_NONE || (s.metadata.flags & lowir::SF_OBJECT_ROOT) != 0)
		return false;
	return s.metadata.binding == ir_model::SBM_INTERNAL || s.kind == lowir::SymbolKind::FunctionDeclaration;
}

void ObjectCompiler::prune_functions()
{
	// pa27 README 7: a unit-local function no emitted code or data reaches
	// is left out of the object. The roots are the functions other objects
	// can name or the startup runs, and every function a global's data
	// addresses (globals are all kept); reachability follows the kept
	// functions' code references. Linear in functions and references.
	const std::vector<CodeFixup>& fixups = image_.code_fixups();
	std::vector<uint32_t> range_of(program_.symbols.size(), lowir::kNoId);
	std::vector<uint32_t> first_fixup(ranges_.size() + 1, 0);
	size_t f = 0;
	for (uint32_t r = 0; r < ranges_.size(); ++r)
	{
		range_of[ranges_[r].symbol] = r;
		while (f < fixups.size() && fixups[f].offset < ranges_[r].start)
			++f;
		first_fixup[r] = static_cast<uint32_t>(f);
		ranges_[r].dropped = true;
	}
	first_fixup[ranges_.size()] = static_cast<uint32_t>(fixups.size());
	std::vector<uint32_t> work;
	auto reach = [&](uint32_t image_symbol) {
		if (image_symbol >= range_of.size() || range_of[image_symbol] == lowir::kNoId)
			return;
		CodeRange& range = ranges_[range_of[image_symbol]];
		if (!range.dropped)
			return;
		range.dropped = false;
		work.push_back(range_of[image_symbol]);
	};
	for (const CodeRange& range : ranges_)
	{
		if (!local_function(range.symbol))
			reach(range.symbol);
	}
	for (size_t i = 0; i < program_.order.size(); ++i)
	{
		const lowir::Symbol& symbol = program_.symbols[program_.order[i]];
		if (symbol.kind != lowir::SymbolKind::GlobalDefinition)
			continue;
		const lowir::Global& global = program_.globals[symbol.detail];
		if (global.init == lowir::GlobalInit::Address)
			reach(global.address);
		else if (global.init == lowir::GlobalInit::Data)
			for (uint32_t k = 0; k < global.item_count; ++k)
			{
				const lowir::DataItem& item = program_.data_items[global.first_item + k];
				if (item.kind == lowir::DataItemKind::Address)
					reach(item.address);
			}
	}
	for (const lowir::ObjectAlias& alias : program_.aliases)
		reach(alias.target);
	while (!work.empty())
	{
		const uint32_t r = work.back();
		work.pop_back();
		for (uint32_t k = first_fixup[r]; k < first_fixup[r + 1] && fixups[k].offset < ranges_[r].end; ++k)
			reach(fixups[k].target);
	}
	for (const CodeRange& range : ranges_)
		stats_.pruned_functions += range.dropped ? 1 : 0;
}

void ObjectCompiler::place_text()
{
	program_symbols_ = static_cast<uint32_t>(program_.symbols.size());
	placement_.assign(program_symbols_ + image_.local_count(), Placement());
	const std::vector<uint8_t>& code = image_.text();
	for (CodeRange& range : ranges_)
	{
		if (range.dropped)
			continue;
		const bool comdat = range.symbol < program_.symbols.size() && comdat_definition(range.symbol);
		range.section = comdat ? own_section(".text", range.symbol, SectionKind::Text,
									 section_names_[static_cast<unsigned>(SectionKind::Text)])
							   : shared_section(".text", SectionKind::Text);
		object::Section& section = out_->sections[range.section];
		section.align = 16;
		range.base = align_up(section.bytes.size(), 16);
		section.bytes.resize(range.base, 0xcc);
		section.bytes.insert(section.bytes.end(), code.begin() + range.start, code.begin() + range.end);
		section.size = section.bytes.size();
		placement_[slot(range.symbol)] = Placement{ range.section, range.base, range.end - range.start };
	}
	// The floating constants start the read-only data.
	if (!image_.rodata().empty())
	{
		const uint32_t rodata = shared_section(".rodata", SectionKind::Rodata);
		out_->sections[rodata].bytes = image_.rodata();
		out_->sections[rodata].size = image_.rodata().size();
		out_->sections[rodata].align = 16;
		for (uint32_t k = 0; k < image_.local_count(); ++k)
		{
			const uint32_t s = Image::kLocalSymbols + k;
			if (image_.symbol(s).section == Section::Rodata)
				placement_[slot(s)] = Placement{ rodata, image_.symbol(s).offset, 0 };
		}
	}
}

void ObjectCompiler::place_globals()
{
	std::vector<GlobalAddress> addresses;
	for (size_t i = 0; i < program_.order.size(); ++i)
	{
		const lowir::SymbolId s = program_.order[i];
		const lowir::Symbol& symbol = program_.symbols[s];
		if (symbol.kind != lowir::SymbolKind::GlobalDefinition)
			continue;
		const lowir::Global& global = program_.globals[symbol.detail];
		const bool readonly = symbol.metadata.storage == ir_model::GSM_READONLY;
		// A host object's read-only data holding addresses (vtables, type
		// information) is relocated by the loader: .data.rel.ro, which a
		// position-independent link makes read-only after relocation (no
		// text relocations).
		const bool relro = readonly && eh_model_ == EhModel::Host && global_has_addresses(program_, global);
		SectionKind kind = readonly && !relro
			? SectionKind::Rodata
			: (!relro && global_is_zero(program_, global) ? SectionKind::Bss : SectionKind::Data);
		// A host object's thread-local object is the thread's: its initial
		// bytes in .tdata (.tbss when zero), offsets in the TLS block.
		if (eh_model_ == EhModel::Host && symbol.metadata.storage == ir_model::GSM_THREAD_LOCAL)
			kind = kind == SectionKind::Bss ? SectionKind::TlsBss : SectionKind::TlsData;
		static const char* const shared_names[] = { "", ".text", ".rodata", ".data", ".bss", "", "", ".tdata",
			".tbss" };
		const unsigned k = static_cast<unsigned>(kind);
		const char* name = relro ? ".data.rel.ro" : shared_names[k];
		uint32_t index;
		if (symbol.metadata.section != 0)
		{
			// A named section holds its globals' bytes (no COMDAT group: a
			// weak one is a plain weak symbol there).
			index = named_section(
				symbol.metadata.section, readonly && !relro ? SectionKind::Rodata : SectionKind::Data);
			kind = out_->sections[index].kind;
		}
		else if (relro)
			index = comdat_definition(s) ? own_section(name, s, kind, relro_name_) : shared_relro_section();
		else
			index = comdat_definition(s) ? own_section(name, s, kind, section_names_[k]) : shared_section(name, kind);
		object::Section& section = out_->sections[index];
		uint32_t align = 1;
		const uint64_t size = global_size(program_, global, align);
		if (align > section.align)
			section.align = align;
		const uint64_t offset = align_up(section.size, align);
		section.size = offset + size;
		placement_[slot(s)] = Placement{ index, offset, size };
		if (kind == SectionKind::Bss || kind == SectionKind::TlsBss)
			continue;
		section.bytes.resize(section.size, 0);
		addresses.clear();
		write_global(program_, global, section.bytes, offset, addresses);
		for (const GlobalAddress& address : addresses)
			pending_.push_back(PendingRelocation{
				index, offset + address.offset, object::R_X86_64_64, address.target, address.addend });
	}
}

void ObjectCompiler::place_startup()
{
	// Every object's initialization runs before `main` in link order, its
	// finalization after it in reverse order.
	for (lowir::SymbolId s = 0; s < program_.symbols.size(); ++s)
	{
		const lowir::Symbol& symbol = program_.symbols[s];
		if (symbol.kind != lowir::SymbolKind::FunctionDefinition)
			continue;
		const uint8_t role = symbol.metadata.role;
		if (role != ir_model::SR_INIT && role != ir_model::SR_FINI)
			continue;
		const bool init = role == ir_model::SR_INIT;
		const uint32_t index = out_->add_section(
			init ? ".init_array" : ".fini_array", init ? SectionKind::InitArray : SectionKind::FiniArray, 8);
		out_->sections[index].bytes.assign(8, 0);
		out_->sections[index].size = 8;
		pending_.push_back(PendingRelocation{ index, 0, object::R_X86_64_64, s, 0 });
	}
}

// The DW.ref cell holding `image_symbol`'s address (8 bytes in .data,
// relocated when the symbols exist).
uint32_t ObjectCompiler::cell(uint32_t image_symbol)
{
	if (const uint32_t* known = cell_index_.find(image_symbol))
		return *known;
	const uint32_t data = shared_section(".data", SectionKind::Data);
	object::Section& section = out_->sections[data];
	if (section.align < 8)
		section.align = 8;
	const uint64_t offset = align_up(section.size, 8);
	section.size = offset + 8;
	section.bytes.resize(section.size, 0);
	pending_.push_back(PendingRelocation{ data, offset, object::R_X86_64_64, image_symbol, 0 });
	const uint32_t index = static_cast<uint32_t>(cells_.size());
	cells_.push_back(Cell{ image_symbol, offset, 0 });
	cell_index_.insert(image_symbol, index);
	return index;
}

// Host EH: the LSDAs (each 4-aligned; a COMDAT function's in a
// .gcc_except_table section of its own group), then .eh_frame: a CIE per
// kind of FDE used, an FDE per function in code order.
void ObjectCompiler::place_unwind()
{
	if (eh_model_ != EhModel::Host || unwind_.empty())
		return;
	std::vector<uint32_t> lsda_section(unwind_.size(), 0);
	std::vector<uint64_t> lsda_offset(unwind_.size(), 0);
	uint32_t shared_lsda = 0;
	uint32_t lsda_name = 0;
	bool any_lsda = false;
	bool plain = false;
	for (const UnwindEntry& entry : unwind_)
		if (!ranges_[entry.range].dropped)
			(entry.lsda_count != 0 ? any_lsda : plain) = true;
	// The cells' .data exists before any section reference is taken.
	const uint32_t personality_cell = any_lsda ? cell(personality_) : kNoCell;
	for (size_t u = 0; u < unwind_.size(); ++u)
	{
		const UnwindEntry& entry = unwind_[u];
		if (entry.lsda_count == 0 || ranges_[entry.range].dropped)
			continue;
		const CodeRange& range = ranges_[entry.range];
		const bool comdat = range.symbol < program_symbols_ && comdat_definition(range.symbol);
		if (lsda_name == 0)
			lsda_name = out_->add_string(".gcc_except_table", 17);
		uint32_t index;
		if (comdat)
		{
			index = out_->add_section(lsda_name, SectionKind::Rodata, 4);
			group_members_.push_back(std::make_pair(range.symbol, index));
		}
		else
		{
			if (shared_lsda == 0)
				shared_lsda = out_->add_section(lsda_name, SectionKind::Rodata, 4);
			index = shared_lsda;
		}
		object::Section& section = out_->sections[index];
		const uint64_t offset = align_up(section.bytes.size(), 4);
		section.bytes.resize(offset, 0);
		section.bytes.insert(section.bytes.end(), lsda_pool_.begin() + entry.lsda_first,
			lsda_pool_.begin() + entry.lsda_first + entry.lsda_count);
		section.size = section.bytes.size();
		lsda_section[u] = index;
		lsda_offset[u] = offset;
		for (uint32_t t = 0; t < entry.types_count; ++t)
		{
			const LsdaTypeField& field = type_fields_[entry.types_first + t];
			unwind_relocations_.push_back(UnwindRelocation{ index, offset + field.offset, cell(field.type), 0, 0 });
		}
		stats_.unwind_bytes += entry.lsda_count;
	}
	const uint32_t frame = out_->add_section(".eh_frame", SectionKind::Rodata, 8);
	std::vector<uint8_t>& bytes = out_->sections[frame].bytes;
	uint32_t plain_cie = 0;
	uint32_t personality_cie = 0;
	if (plain)
	{
		plain_cie = static_cast<uint32_t>(bytes.size());
		encode_cie(false, bytes);
	}
	if (any_lsda)
	{
		personality_cie = static_cast<uint32_t>(bytes.size());
		const uint32_t field = encode_cie(true, bytes);
		unwind_relocations_.push_back(UnwindRelocation{ frame, field, personality_cell, 0, 0 });
	}
	for (size_t u = 0; u < unwind_.size(); ++u)
	{
		const UnwindEntry& entry = unwind_[u];
		const CodeRange& range = ranges_[entry.range];
		if (range.dropped)
			continue;
		const bool lsda = entry.lsda_count != 0;
		uint32_t pc_field = 0;
		uint32_t lsda_field = 0;
		encode_fde(lsda ? personality_cie : plain_cie, static_cast<uint32_t>(range.end - range.start), lsda,
			cfi_pool_.data() + entry.cfi_first, entry.cfi_count, bytes, pc_field, lsda_field);
		unwind_relocations_.push_back(
			UnwindRelocation{ frame, pc_field, kNoCell, range.section, static_cast<int64_t>(range.base) });
		if (lsda)
			unwind_relocations_.push_back(
				UnwindRelocation{ frame, lsda_field, kNoCell, lsda_section[u], static_cast<int64_t>(lsda_offset[u]) });
	}
	out_->sections[frame].size = bytes.size();
	stats_.unwind_bytes += bytes.size();
	// The tables are placed: the pools are released.
	std::vector<uint8_t>().swap(cfi_pool_);
	std::vector<uint8_t>().swap(lsda_pool_);
	std::vector<LsdaTypeField>().swap(type_fields_);
}

uint32_t ObjectCompiler::section_symbol(uint32_t section)
{
	if (section >= section_symbols_.size())
		section_symbols_.resize(out_->sections.size(), 0);
	if (section_symbols_[section] == 0)
	{
		object::Symbol symbol;
		symbol.type = SymbolType::Section;
		symbol.section = section;
		section_symbols_[section] = out_->add_symbol(symbol);
	}
	return section_symbols_[section];
}

// The cells' symbols (`DW.ref.<target>`, local) and the tables'
// pc-relative relocations.
void ObjectCompiler::add_unwind_relocations()
{
	const uint32_t data = data_;
	for (Cell& c : cells_)
	{
		object::Symbol symbol;
		spelling_.assign("DW.ref.");
		spelling_ += out_->symbol_name(object_symbol(c.target));
		symbol.name = out_->add_string(spelling_);
		symbol.type = SymbolType::Object;
		symbol.section = data;
		symbol.value = c.offset;
		symbol.size = 8;
		c.symbol = out_->add_symbol(symbol);
	}
	for (const UnwindRelocation& r : unwind_relocations_)
	{
		object::Relocation relocation;
		relocation.offset = r.offset;
		relocation.type = object::R_X86_64_PC32;
		relocation.symbol = r.cell != kNoCell ? cells_[r.cell].symbol : section_symbol(r.target_section);
		relocation.addend = r.addend;
		out_->sections[r.section].relocations.push_back(relocation);
	}
}

bool ObjectCompiler::external_function(uint32_t image_symbol)
{
	return image_symbol < Image::kLocalSymbols && placement_[slot(image_symbol)].section == 0 &&
		lowir::is_function_kind(program_.symbols[image_symbol].kind);
}

uint32_t ObjectCompiler::object_symbol(uint32_t image_symbol)
{
	const uint32_t index = slot(image_symbol);
	if (object_symbol_[index] != 0)
		return object_symbol_[index];
	object::Symbol symbol;
	const Placement& place = placement_[index];
	symbol.section = place.section;
	symbol.value = place.offset;
	symbol.size = place.size;
	if (image_symbol >= Image::kLocalSymbols)
	{
		// The runtime's state and routines are undefined; constants are
		// unnamed locals.
		if (place.section == 0)
		{
			const char* name = runtime_names_[image_symbol - Image::kLocalSymbols];
			symbol.name = out_->add_string(name, std::strlen(name));
			symbol.binding = Binding::Global;
		}
		else
			symbol.type = SymbolType::Object;
		return object_symbol_[index] = out_->add_symbol(symbol);
	}
	const lowir::Symbol& s = program_.symbols[image_symbol];
	const bool defined = place.section != 0;
	const uint8_t binding = s.metadata.binding;
	symbol.type = lowir::is_function_kind(s.kind) ? SymbolType::Function : SymbolType::Object;
	// A host object's undefined reference has no type (STT_NOTYPE, as GCC
	// and Clang write it: a static link binds an IFUNC such as memcpy only
	// through an untyped reference); a thread-local one stays STT_TLS.
	if (eh_model_ == EhModel::Host && !defined)
		symbol.type = SymbolType::None;
	if (eh_model_ == EhModel::Host && s.metadata.storage == ir_model::GSM_THREAD_LOCAL)
		symbol.type = SymbolType::Tls;
	// A weak declaration is a weak reference, which links as address 0
	// where no object defines it (the object-mode lowering declares
	// vague-linkage entities strong: see ProgramLowering::finish).
	symbol.binding = binding == ir_model::SBM_WEAK ? Binding::Weak : Binding::Global;
	if (defined && (binding == ir_model::SBM_INTERNAL || s.kind == lowir::SymbolKind::FunctionDeclaration))
		symbol.binding = Binding::Local;
	const lowir::NameId name = object_name(image_symbol);
	// One object symbol per non-local name: a definition completes an
	// earlier undefined reference of the same spelling.
	if (symbol.binding != Binding::Local && by_name_[name] != 0)
	{
		object::Symbol& existing = out_->symbols[by_name_[name]];
		if (defined && existing.section == object::kUndefinedSection)
		{
			symbol.name = existing.name;
			existing = symbol;
		}
		return object_symbol_[index] = by_name_[name];
	}
	if (symbol.binding == Binding::Local && s.metadata.object == 0)
	{
		spelling_.assign(1, '@');
		spelling_.append(program_.spelling(name), program_.names.length(name));
		symbol.name = out_->add_string(spelling_);
	}
	else
		symbol.name = out_->add_string(program_.spelling(name), program_.names.length(name));
	const uint32_t added = out_->add_symbol(symbol);
	if (symbol.binding != Binding::Local)
		by_name_[name] = added;
	return object_symbol_[index] = added;
}

uint32_t ObjectCompiler::define_alias(lowir::NameId name, uint32_t target)
{
	object::Symbol alias = out_->symbols[target];
	if (by_name_[name] != 0)
	{
		object::Symbol& existing = out_->symbols[by_name_[name]];
		if (existing.section == object::kUndefinedSection)
		{
			alias.name = existing.name;
			existing = alias;
		}
		return by_name_[name];
	}
	alias.name = out_->add_string(program_.spelling(name), program_.names.length(name));
	if (alias.binding == Binding::Local)
		return out_->add_symbol(alias);
	by_name_[name] = out_->add_symbol(alias);
	return by_name_[name];
}

void ObjectCompiler::add_symbols()
{
	// Host objects call the builtin routines by their host names.
	if (eh_model_ == EhModel::Host)
	{
		for (unsigned k = 0; k < kHostBuiltinCount; ++k)
		{
			const char* const* names = kHostBuiltins[k];
			const lowir::NameId name = program_.names.find(names[0], static_cast<uint32_t>(std::strlen(names[0])));
			if (name != 0)
				host_builtins_[k] = std::make_pair(
					name, program_.names.intern(names[1], static_cast<uint32_t>(std::strlen(names[1]))));
		}
	}
	object_symbol_.assign(placement_.size(), 0);
	by_name_.assign(program_.names.size() + 1, 0);
	// Definitions first, so references of the same spelling share them, in
	// placement order (the code in encoding order, then the globals): the
	// symbol table depends only on the program's definitions, not on the
	// order its symbols were first named (a LowIR reader's or a lowering's).
	for (const CodeRange& range : ranges_)
		if (!range.dropped)
			object_symbol(range.symbol);
	for (size_t i = 0; i < program_.order.size(); ++i)
	{
		if (placement_[slot(program_.order[i])].section != 0)
			object_symbol(program_.order[i]);
	}
	add_aliases();
}

void ObjectCompiler::add_aliases()
{
	for (const lowir::ObjectAlias& alias : program_.aliases)
		define_alias(alias.object, object_symbol(alias.target));
	for (lowir::SymbolId s = 0; s < program_.symbols.size(); ++s)
	{
		const lowir::Symbol& symbol = program_.symbols[s];
		if (placement_[slot(s)].section == 0)
			continue;
		if ((symbol.metadata.flags & lowir::SF_KEEP_ALIAS) != 0 && symbol.metadata.object != 0 &&
			symbol.metadata.object != symbol.name)
			define_alias(symbol.name, object_symbol(s));
		// The process entry is `main` whatever its LowIR spelling.
		if (symbol.metadata.role == ir_model::SR_ENTRY && std::strcmp(out_->symbol_name(object_symbol(s)), "main") != 0)
		{
			object::Symbol entry = out_->symbols[object_symbol(s)];
			entry.name = out_->add_string("main", 4);
			entry.binding = Binding::Global;
			out_->add_symbol(entry);
		}
	}
}

void ObjectCompiler::add_relocations()
{
	// Code references: PC-relative from the field (the image's fixups are
	// relative to the instruction's end); calls and jumps take the PLT
	// form, addresses and operands PC32 (psABI 4.4). A host object takes
	// an undefined function's address from its GOT entry, as position-
	// independent code must (another module defines it, and a PIE link
	// makes no canonical PLT entry for a PC32 reference): its `lea` becomes
	// the `mov` load of the slot, R_X86_64_REX_GOTPCRELX (psABI 4.4.1),
	// which the linker relaxes back when the function is local.
	const std::vector<CodeFixup>& fixups = image_.code_fixups();
	size_t r = 0;
	for (const CodeFixup& fixup : fixups)
	{
		while (r < ranges_.size() && fixup.offset >= ranges_[r].end)
			++r;
		if (r == ranges_.size() || fixup.offset < ranges_[r].start)
			throw InternalError("code reference outside every function");
		const CodeRange& range = ranges_[r];
		if (range.dropped)
			continue;
		object::Relocation relocation;
		relocation.offset = range.base + (fixup.offset - range.start);
		relocation.symbol = object_symbol(fixup.target);
		if (fixup.absolute)
		{
			relocation.type = object::R_X86_64_64;
			relocation.addend = fixup.addend;
		}
		else if (fixup.kind == FixupKind::Got)
		{
			// The selector's GOT load of imported data (mov_got). A streamed
			// function selected before the unit defined the global took its
			// GOT entry; when the definition the unit ends up holding is
			// addressed directly (host_symbol_access on the final facts), the
			// load becomes the lea of its address, as the linker relaxes it
			// (psABI B.2: REX.W 8B /r -> 8D /r, same bytes otherwise), so the
			// object does not depend on where the unit defined it.
			relocation.type = object::R_X86_64_REX_GOTPCRELX;
			relocation.addend = static_cast<int64_t>(fixup.addend) - fixup.end;
			std::vector<uint8_t>& bytes = out_->sections[range.section].bytes;
			const uint64_t at = relocation.offset;
			if (fixup.addend == 0 && fixup.target < Image::kLocalSymbols &&
				placement_[slot(fixup.target)].section != 0 &&
				host_symbol_access(program_, fixup.target) == SymbolAccess::Direct && at >= 3 &&
				(bytes[at - 3] & 0xf8) == 0x48 && bytes[at - 2] == 0x8b && (bytes[at - 1] & 0xc7) == 0x05)
			{
				bytes[at - 2] = 0x8d;
				relocation.type = object::R_X86_64_PC32;
			}
		}
		else if (fixup.kind == FixupKind::TpOff)
		{
			relocation.type = object::R_X86_64_TPOFF32;
			relocation.addend = fixup.addend;
		}
		else if (fixup.kind == FixupKind::GotTpOff)
		{
			relocation.type = object::R_X86_64_GOTTPOFF;
			relocation.addend = static_cast<int64_t>(fixup.addend) - fixup.end;
		}
		else
		{
			relocation.type = fixup.branch ? object::R_X86_64_PLT32 : object::R_X86_64_PC32;
			relocation.addend = static_cast<int64_t>(fixup.addend) - fixup.end;
			if (!fixup.branch && eh_model_ == EhModel::Host && external_function(fixup.target))
			{
				std::vector<uint8_t>& bytes = out_->sections[range.section].bytes;
				const uint64_t at = relocation.offset;
				// REX.W 8D /r with a RIP-relative ModRM (mod 00, r/m 101).
				if (at < 3 || (bytes[at - 3] & 0xf8) != 0x48 || bytes[at - 2] != 0x8d ||
					(bytes[at - 1] & 0xc7) != 0x05 || fixup.addend != 0)
					throw InternalError("an external function's address outside a lea");
				bytes[at - 2] = 0x8b;
				relocation.type = object::R_X86_64_REX_GOTPCRELX;
			}
		}
		out_->sections[range.section].relocations.push_back(relocation);
	}
	for (const PendingRelocation& pending : pending_)
	{
		object::Relocation relocation;
		relocation.offset = pending.offset;
		relocation.type = pending.type;
		relocation.symbol = object_symbol(pending.target);
		relocation.addend = pending.addend;
		out_->sections[pending.section].relocations.push_back(relocation);
	}
}

void ObjectCompiler::add_groups()
{
	for (const uint32_t symbol : grouped_)
	{
		object::Group group;
		group.signature = object_symbol(symbol);
		group.sections.push_back(placement_[slot(symbol)].section);
		out_->sections[placement_[slot(symbol)].section].group = static_cast<uint32_t>(out_->groups.size());
		out_->groups.push_back(std::move(group));
	}
	// Host EH: a COMDAT function's LSDA section joins its group (members
	// pair in group order: grouped_ is in placement order).
	size_t g = 0;
	for (const std::pair<uint32_t, uint32_t>& member : group_members_)
	{
		while (g < grouped_.size() && grouped_[g] != member.first)
			++g;
		if (g == grouped_.size())
			throw InternalError("an LSDA of a function without its group");
		out_->groups[g].sections.push_back(member.second);
		out_->sections[member.second].group = static_cast<uint32_t>(g);
	}
}

void ObjectCompiler::finish(ObjectFile& out)
{
	out_ = &out;
	emit_remaining();
	prune_functions();
	place_text();
	place_globals();
	place_startup();
	place_unwind();
	add_symbols();
	add_relocations();
	add_unwind_relocations();
	add_groups();
	stats_.sections = out.sections.size() - 1;
	stats_.symbols = out.symbols.size() - 1;
	for (const object::Section& section : out.sections)
		stats_.relocations += section.relocations.size();
}

ObjectBuilder::ObjectBuilder(lowir::Program& program, ObjectStats& stats, EhModel eh_model, int optimization_level)
	: compiler_(new ObjectCompiler(program, stats, eh_model, optimization_level))
{}

ObjectBuilder::~ObjectBuilder()
{
	delete compiler_;
}

void ObjectBuilder::append(const lowir::Program& program, lowir::SymbolId function)
{
	(void)program;
	compiler_->function(function);
}

void ObjectBuilder::finish(object::ObjectFile& out)
{
	compiler_->finish(out);
}

namespace {

// The literal of an i64 integer operand.
bool literal_offset(const lowir::FunctionBody& body, lowir::Value v, int64_t& out)
{
	if (!v.is(lowir::ValueKind::Integer))
		return false;
	out = static_cast<int64_t>(body.literals[v.index].low);
	return true;
}

// The definition of temporary `v` in `body`, or null.
const lowir::Instruction* defined(const lowir::FunctionBody& body, lowir::Value v)
{
	if (!v.is(lowir::ValueKind::Temp) || v.index >= body.temps.size() || body.temps[v.index].def == lowir::kNoId)
		return nullptr;
	return &body.instructions[body.temps[v.index].def];
}

// The jump thunk `symbol`'s body states (FunctionLowering::lower_thunk
// without a result adjustment): one block of `index i8 this, adjustment`
// (then `load ptr`, `index i8 _, vcall`, `load i64`, `index i8 this', _`;
// a virtual thunk without a fixed part starts at `load ptr this`), a call
// of the target passing every parameter in place with `this` adjusted, and
// the return of its result.
bool thunk_body(const lowir::Program& program, lowir::SymbolId symbol, lowir::JumpThunk& out)
{
	const lowir::FunctionBody& body = *program.functions[program.symbols[symbol].detail].body;
	if (body.layout.size() != 1)
		return false;
	const lowir::Block& block = body.blocks[body.layout[0]];
	const lowir::Instruction* call = nullptr;
	const lowir::Instruction* ret = nullptr;
	for (lowir::InstId i = block.first; i != lowir::kNoId; i = body.instructions[i].next)
	{
		const lowir::Instruction& inst = body.instructions[i];
		if (inst.op == lowir::Opcode::Call)
		{
			if (call != nullptr)
				return false;
			call = &inst;
		}
		else if (inst.op == lowir::Opcode::Return)
			ret = &inst;
		else if (inst.op != lowir::Opcode::Index && inst.op != lowir::Opcode::Load)
			return false;
	}
	if (call == nullptr || ret == nullptr || !call->ops[0].is(lowir::ValueKind::Symbol) ||
		call->extra_count != body.parameter_count ||
		!(ret->ops[0].is(lowir::ValueKind::None) ||
			(call->result != lowir::kNoId && ret->ops[0] == lowir::Value::temp(call->result))))
		return false;
	out = lowir::JumpThunk{ symbol, call->ops[0].index, lowir::kNoId, 0, 0 };
	for (uint32_t k = 0; k < call->extra_count; ++k)
	{
		const lowir::Value argument = body.operands[call->extra_first + k];
		if (argument == lowir::Value::temp(k))
			continue;
		if (out.this_index != lowir::kNoId || k > 1)
			return false;
		out.this_index = k;
		// [index i8 (index i8 this, a), (load i64 (index i8 (load ptr (index i8 this, a)), vcall))]
		const lowir::Instruction* outer = defined(body, argument);
		if (outer == nullptr || outer->op != lowir::Opcode::Index)
			return false;
		const lowir::Instruction* fixed = outer;
		if (!outer->ops[1].is(lowir::ValueKind::Integer))
		{
			const lowir::Instruction* offset = defined(body, outer->ops[1]);
			const lowir::Instruction* row = offset != nullptr && offset->op == lowir::Opcode::Load
				? defined(body, offset->ops[0])
				: nullptr;
			const lowir::Instruction* table = row != nullptr && row->op == lowir::Opcode::Index
				? defined(body, row->ops[0])
				: nullptr;
			fixed = defined(body, outer->ops[0]);
			if (table == nullptr || table->op != lowir::Opcode::Load || !(table->ops[0] == outer->ops[0]) ||
				!literal_offset(body, row->ops[1], out.vcall_offset) || out.vcall_offset == 0)
				return false;
			if (outer->ops[0] == lowir::Value::temp(k))
				continue;
		}
		if (fixed == nullptr || fixed->op != lowir::Opcode::Index || !(fixed->ops[0] == lowir::Value::temp(k)) ||
			!literal_offset(body, fixed->ops[1], out.adjustment))
			return false;
	}
	return out.this_index != lowir::kNoId;
}

} // namespace

void recover_jump_thunks(lowir::Program& program, EhModel eh_model)
{
	if (!program.jump_thunks.empty())
		return;
	for (lowir::SymbolId symbol : program.order)
	{
		const lowir::Symbol& s = program.symbols[symbol];
		if (s.kind != lowir::SymbolKind::FunctionDefinition || s.metadata.object == 0)
			continue;
		const char* object = program.spelling(s.metadata.object);
		if (std::strncmp(object, "_ZTh", 4) != 0 && std::strncmp(object, "_ZTv", 4) != 0)
			continue;
		const lowir::Signature& signature = program.signatures[program.functions[s.detail].signature];
		if (eh_model != EhModel::Host && signature.boundary.arity != ir_model::CAM_VARIADIC)
			continue;
		lowir::JumpThunk thunk;
		if (thunk_body(program, symbol, thunk))
			program.jump_thunks.push_back(thunk);
	}
}

bool compile_object(lowir::Program& program, object::ObjectFile& out, std::string& error, ObjectStats* stats,
	EhModel eh_model, int optimization_level)
{
	ObjectStats local;
	try
	{
		ObjectBuilder builder(program, stats != nullptr ? *stats : local, eh_model, optimization_level);
		builder.finish(out);
	}
	catch (const BackendError& e)
	{
		// LowIR outside the native subset; an InternalError (a compiler
		// bug, not an error of the program) leaves for the tool to report.
		error = e.what();
		return false;
	}
	return true;
}

} // namespace native
