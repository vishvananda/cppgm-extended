#include "native/object/dwarf_lines.h"

#include "native/object/dwarf_bytes.h"
#include "native/object/dwarf_format.h"
#include "support/flat_map.h"

namespace native {

namespace {

struct FileHash
{
	size_t operator()(lowir::NameId file) const { return file; }
};

class LineEncoder
{
public:
	LineEncoder(object::ObjectFile& object, const lowir::Program& program,
		const std::vector<DwarfLines::Function>& functions, const std::vector<CodeLocation>& locations,
		const std::vector<uint32_t>& symbols)
		: object_(object), program_(program), functions_(functions), locations_(locations), symbols_(symbols)
	{}
	void encode();

private:
	void collect_files();
	uint32_t file(lowir::NameId name);
	uint32_t section(const char* name, uint32_t& symbol);
	void reference(uint32_t section, uint32_t symbol, unsigned width, int64_t addend = 0);
	void abbreviations();
	void line_table();
	void sequence(DwarfBytes& out, const DwarfLines::Function& function);
	void information();
	void function_info(DwarfBytes& out, const DwarfLines::Function& function);
	object::ObjectFile& object_;
	const lowir::Program& program_;
	const std::vector<DwarfLines::Function>& functions_;
	const std::vector<CodeLocation>& locations_;
	const std::vector<uint32_t>& symbols_;
	FlatMap<lowir::NameId, uint32_t, FileHash> files_index_;
	std::vector<lowir::NameId> files_;
	uint32_t line_, info_, strings_, abbreviation_, ranges_;
	uint32_t line_symbol_, info_symbol_, strings_symbol_, abbreviation_symbol_, ranges_symbol_;
};

uint32_t LineEncoder::file(lowir::NameId name)
{
	if (name == 0)
		return 0;
	if (const uint32_t* found = files_index_.find(name))
		return *found;
	files_.push_back(name);
	const uint32_t index = static_cast<uint32_t>(files_.size());
	files_index_.insert(name, index);
	return index;
}

void LineEncoder::collect_files()
{
	for (const DwarfLines::Function& function : functions_)
	{
		if (symbols_[function.symbol] == lowir::kNoId)
			continue;
		file(function.declaration.file);
		for (uint32_t i = 0; i < function.count; ++i)
			file(locations_[function.first + i].location.file);
	}
}

uint32_t LineEncoder::section(const char* name, uint32_t& symbol)
{
	const uint32_t index = object_.add_section(name, object::SectionKind::Metadata, 1);
	object::Symbol entry;
	entry.type = object::SymbolType::Section;
	entry.section = index;
	symbol = object_.add_symbol(entry);
	return index;
}

void LineEncoder::reference(uint32_t section, uint32_t symbol, unsigned width, int64_t addend)
{
	object::Section& target = object_.sections[section];
	const uint64_t offset = target.bytes.size();
	DwarfBytes(target.bytes).fixed(0, width);
	target.relocations.push_back(
		object::Relocation{ offset, width == 8 ? object::R_X86_64_64 : object::R_X86_64_32, symbol, addend });
}

void LineEncoder::abbreviations()
{
	DwarfBytes out(object_.sections[abbreviation_].bytes);
	const uint32_t unit[][2] = { { dwarf::Name, dwarf::StringPointer }, { dwarf::Producer, dwarf::StringPointer },
		{ dwarf::Language, dwarf::Data2 }, { dwarf::StatementList, dwarf::SectionOffset },
		{ dwarf::Ranges, dwarf::SectionOffset } };
	const uint32_t function[][2] = { { dwarf::Name, dwarf::StringPointer },
		{ dwarf::LinkageName, dwarf::StringPointer }, { dwarf::LowPc, dwarf::Address }, { dwarf::HighPc, dwarf::Data8 },
		{ dwarf::DeclFile, dwarf::Unsigned }, { dwarf::DeclLine, dwarf::Unsigned }, { dwarf::External, dwarf::Flag } };
	auto entry = [&](uint32_t code, uint32_t tag, bool children, const uint32_t (*attributes)[2], size_t count) {
		out.uleb(code);
		out.uleb(tag);
		out.fixed(children ? 1 : 0, 1);
		for (size_t i = 0; i < count; ++i)
		{
			out.uleb(attributes[i][0]);
			out.uleb(attributes[i][1]);
		}
		out.uleb(0);
		out.uleb(0);
	};
	entry(1, dwarf::CompileUnit, true, unit, sizeof(unit) / sizeof(unit[0]));
	entry(2, dwarf::Subprogram, false, function, sizeof(function) / sizeof(function[0]));
	out.uleb(0);
}

void LineEncoder::line_table()
{
	DwarfBytes out(object_.sections[line_].bytes);
	out.fixed(0, 4);
	out.fixed(4, 2);
	out.fixed(0, 4);
	out.fixed(1, 1);    // minimum instruction length
	out.fixed(1, 1);    // maximum operations per instruction
	out.fixed(1, 1);    // default is_stmt
	out.fixed(0xfb, 1); // line_base = -5
	out.fixed(14, 1);   // line_range
	out.fixed(13, 1);   // opcode_base
	const uint8_t arguments[] = { 0, 1, 1, 1, 1, 0, 0, 0, 1, 0, 0, 1 };
	for (uint8_t count : arguments)
		out.fixed(count, 1);
	out.fixed(0, 1); // include directories
	for (lowir::NameId name : files_)
	{
		out.text(program_.spelling(name));
		out.uleb(0);
		out.uleb(0);
		out.uleb(0);
	}
	out.fixed(0, 1);
	out.patch32(6, out.size() - 10);
	for (const DwarfLines::Function& function : functions_)
		if (symbols_[function.symbol] != lowir::kNoId)
			sequence(out, function);
	out.patch32(0, out.size() - 4);
}

void LineEncoder::sequence(DwarfBytes& out, const DwarfLines::Function& function)
{
	out.fixed(0, 1);
	out.uleb(9);
	out.fixed(dwarf::SetAddress, 1);
	reference(line_, symbols_[function.symbol], 8);
	uint32_t pc = 0, current_file = 1, column = 0;
	int64_t line = 1;
	for (uint32_t i = 0; i < function.count; ++i)
	{
		const CodeLocation& row = locations_[function.first + i];
		const uint32_t index = file(row.location.file);
		if (index != current_file)
		{
			out.fixed(dwarf::SetFile, 1);
			out.uleb(index);
			current_file = index;
		}
		if (line != row.location.line)
		{
			out.fixed(dwarf::AdvanceLine, 1);
			out.sleb(static_cast<int64_t>(row.location.line) - line);
			line = row.location.line;
		}
		if (column != row.location.column)
		{
			out.fixed(dwarf::SetColumn, 1);
			out.uleb(row.location.column);
			column = row.location.column;
		}
		if (pc != row.offset)
		{
			out.fixed(dwarf::AdvancePc, 1);
			out.uleb(row.offset - pc);
			pc = row.offset;
		}
		out.fixed(dwarf::Copy, 1);
	}
	out.fixed(dwarf::AdvancePc, 1);
	out.uleb(function.size - pc);
	out.fixed(0, 1);
	out.uleb(1);
	out.fixed(dwarf::EndSequence, 1);
}

void LineEncoder::function_info(DwarfBytes& out, const DwarfLines::Function& function)
{
	const uint32_t symbol = symbols_[function.symbol];
	DwarfBytes strings(object_.sections[strings_].bytes);
	const uint32_t name = strings.text(object_.symbol_name(symbol));
	out.uleb(2);
	reference(info_, strings_symbol_, 4, name);
	reference(info_, strings_symbol_, 4, name);
	reference(info_, symbol, 8);
	out.fixed(function.size, 8);
	const lowir::DebugLocation declared = function.declaration.present() ? function.declaration
																		 : locations_[function.first].location;
	out.uleb(file(declared.file));
	out.uleb(declared.line);
	out.fixed(object_.symbols[symbol].binding != object::Binding::Local ? 1 : 0, 1);
	reference(ranges_, symbol, 8);
	reference(ranges_, symbol, 8, function.size);
}

void LineEncoder::information()
{
	DwarfBytes out(object_.sections[info_].bytes);
	DwarfBytes strings(object_.sections[strings_].bytes);
	out.fixed(0, 4);
	out.fixed(4, 2);
	reference(info_, abbreviation_symbol_, 4);
	out.fixed(8, 1);
	out.uleb(1);
	// Identify this unit by its first recorded source file, not the driver's
	// input path: serialized LowIR carries the same source locations as a
	// direct compilation, even when its input has a different file name.
	reference(info_, strings_symbol_, 4, strings.text(program_.spelling(files_[0])));
	reference(info_, strings_symbol_, 4, strings.text("cppgm++"));
	out.fixed(4, 2); // DW_LANG_C_plus_plus
	reference(info_, line_symbol_, 4);
	reference(info_, ranges_symbol_, 4);
	for (const DwarfLines::Function& function : functions_)
		if (symbols_[function.symbol] != lowir::kNoId)
			function_info(out, function);
	out.uleb(0); // end compilation-unit children
	out.patch32(0, out.size() - 4);
	DwarfBytes(object_.sections[ranges_].bytes).fixed(0, 8);
	DwarfBytes(object_.sections[ranges_].bytes).fixed(0, 8);
}

void LineEncoder::encode()
{
	collect_files();
	if (files_.empty())
		return;
	line_ = section(".debug_line", line_symbol_);
	info_ = section(".debug_info", info_symbol_);
	strings_ = section(".debug_str", strings_symbol_);
	abbreviation_ = section(".debug_abbrev", abbreviation_symbol_);
	ranges_ = section(".debug_ranges", ranges_symbol_);
	abbreviations();
	line_table();
	information();
	for (uint32_t section : { line_, info_, strings_, abbreviation_, ranges_ })
		object_.sections[section].size = object_.sections[section].bytes.size();
}

} // namespace

void DwarfLines::append(
	lowir::SymbolId symbol, uint32_t size, lowir::DebugLocation declaration, const std::vector<CodeLocation>& locations)
{
	const uint32_t first = static_cast<uint32_t>(locations_.size());
	for (const CodeLocation& row : locations)
		if (row.offset < size && row.location.present())
			locations_.push_back(row);
	const uint32_t count = static_cast<uint32_t>(locations_.size()) - first;
	if (count != 0)
		functions_.push_back(Function{ symbol, size, declaration, first, count });
}

void DwarfLines::write(
	object::ObjectFile& object, const lowir::Program& program, const std::vector<uint32_t>& object_symbols) const
{
	LineEncoder(object, program, functions_, locations_, object_symbols).encode();
}

} // namespace native
