#include "native/asm/unwind_rules.h"

#include "asm/registers.h"
#include "native/object/host_unwind.h"

namespace native {
namespace {
uint32_t reg(assembly::ModuleCursor& cursor)
{
	if (cursor.numeric())
	{
		const uint64_t value = cursor.number();
		if (value > 255)
			cursor.fail("CFI register number exceeds supported range");
		return static_cast<uint32_t>(value);
	}
	cursor.take('%');
	std::string name = cursor.word();
	for (char& c : name)
		if (c >= 'A' && c <= 'Z')
			c = static_cast<char>(c + ('a' - 'A'));
	if (name == "rip")
		return 16;
	if (name == "st" && cursor.take('('))
	{
		const uint64_t index = cursor.number();
		if (index >= 8 || !cursor.take(')'))
			cursor.fail("invalid x87 CFI register");
		return static_cast<uint32_t>(33 + index);
	}
	const char* const special[] = { "eflags", "rflags", "mxcsr", "fcw", "fsw", "fs.base", "gs.base" };
	const uint32_t numbers[] = { 49, 49, 64, 65, 66, 58, 59 };
	for (unsigned i = 0; i < 7; ++i)
		if (name == special[i])
			return numbers[i];
	asm_model::Register reg;
	if (!assembly::parse_register(name.data(), name.size(), reg))
		cursor.fail("unknown CFI register");
	if (reg.bank == asm_model::RegisterBank::Gpr && reg.bits == 64 && reg.offset == 0 && reg.number < 16)
	{
		static const uint8_t numbers[] = { 0, 2, 1, 3, 7, 6, 4, 5, 8, 9, 10, 11, 12, 13, 14, 15 };
		return numbers[reg.number];
	}
	if (reg.bank == asm_model::RegisterBank::Vector && reg.number < 32)
		return reg.number < 16 ? 17 + reg.number : 67 + reg.number - 16;
	if (reg.bank == asm_model::RegisterBank::Mask && reg.number < 8)
		return 118 + reg.number;
	if (reg.bank == asm_model::RegisterBank::X87 && reg.number < 8)
		return 33 + reg.number;
	if (reg.bank == asm_model::RegisterBank::Mmx && reg.number < 8)
		return 41 + reg.number;
	if (reg.bank == asm_model::RegisterBank::Segment && reg.number < 6)
		return 50 + reg.number;
	cursor.fail("CFI register name is not implemented");
}
void comma(assembly::ModuleCursor& cursor)
{
	if (!cursor.take(','))
		cursor.fail("expected comma in CFI directive");
}
void pointer(assembly::ModuleCursor& cursor, AssemblyCfiEvent& event, assembly::SymbolLookup* symbols)
{
	const uint64_t encoding = cursor.number();
	if (encoding > 255)
		cursor.fail("CFI pointer encoding exceeds one byte");
	event.encoding = static_cast<uint8_t>(encoding);
	if (event.encoding == 0xff)
		return;
	if (unwind_pointer_bytes(event.encoding) == 0 || ((event.encoding & 0x70) != 0 && (event.encoding & 0x70) != 0x10))
		cursor.fail("CFI pointer encoding is not implemented");
	comma(cursor);
	cursor.space();
	if (const assembly::SymbolUse* use = cursor.symbol_use())
	{
		event.reference = use->reference;
		cursor.advance(use->count);
	}
	else if (!symbols || !symbols->lookup(cursor.word(), event.reference))
		cursor.fail("unknown CFI symbol");
	if (cursor.take('+'))
		event.reference.addend = static_cast<int64_t>(uint64_t(event.reference.addend) + cursor.number());
	else if (cursor.take('-'))
		event.reference.addend = static_cast<int64_t>(uint64_t(event.reference.addend) - cursor.number());
}

} // namespace
AssemblyCfiEvent parse_assembly_cfi(const assembly::ModuleLine& line, uint32_t anchor, assembly::SymbolLookup* symbols)
{
	assembly::ModuleCursor cursor(line);
	const std::string name = cursor.word();
	AssemblyCfiEvent event;
	event.anchor = anchor;
	event.line = line.line;
	if (name == ".cfi_startproc")
	{
		event.op = AssemblyCfiOp::Start;
		if (!cursor.end())
		{
			if (cursor.word() != "simple")
				cursor.fail("unknown CFI procedure mode");
			event.simple = true;
		}
	}
	else if (name == ".cfi_personality" || name == ".cfi_lsda")
	{
		event.op = name == ".cfi_personality" ? AssemblyCfiOp::Personality : AssemblyCfiOp::Lsda;
		pointer(cursor, event, symbols);
	}
	else if (name == ".cfi_sections")
	{
		event.op = AssemblyCfiOp::Sections;
		if (cursor.word() != ".eh_frame")
			cursor.fail("CFI section is not implemented");
	}
	else if (name == ".cfi_escape")
	{
		event.op = AssemblyCfiOp::Escape;
		do
		{
			const uint64_t value = cursor.number();
			if (value > 255)
				cursor.fail("CFI escape byte exceeds 255");
			event.bytes.push_back(static_cast<uint8_t>(value));
		} while (cursor.take(','));
	}
	else if (name == ".cfi_endproc")
		event.op = AssemblyCfiOp::End;
	else if (name == ".cfi_signal_frame")
		event.op = AssemblyCfiOp::Signal;
	else if (name == ".cfi_remember_state")
		event.op = AssemblyCfiOp::Remember;
	else if (name == ".cfi_restore_state")
		event.op = AssemblyCfiOp::RestoreState;
	else if (name == ".cfi_def_cfa")
	{
		event.op = AssemblyCfiOp::DefCfa;
		event.reg = reg(cursor);
		comma(cursor);
		event.value = static_cast<int64_t>(cursor.number());
	}
	else if (name == ".cfi_def_cfa_register")
	{
		event.op = AssemblyCfiOp::DefRegister;
		event.reg = reg(cursor);
	}
	else if (name == ".cfi_def_cfa_offset" || name == ".cfi_adjust_cfa_offset")
	{
		event.op = name == ".cfi_def_cfa_offset" ? AssemblyCfiOp::DefOffset : AssemblyCfiOp::Adjust;
		event.value = static_cast<int64_t>(cursor.number());
	}
	else if (name == ".cfi_offset" || name == ".cfi_rel_offset" || name == ".cfi_val_offset")
	{
		event.op = name == ".cfi_offset" ? AssemblyCfiOp::Offset
			: name == ".cfi_rel_offset"  ? AssemblyCfiOp::RelOffset
										 : AssemblyCfiOp::ValOffset;
		event.reg = reg(cursor);
		comma(cursor);
		event.value = static_cast<int64_t>(cursor.number());
	}
	else if (name == ".cfi_register")
	{
		event.op = AssemblyCfiOp::Register;
		event.reg = reg(cursor);
		comma(cursor);
		event.second = reg(cursor);
	}
	else if (name == ".cfi_restore" || name == ".cfi_undefined" || name == ".cfi_same_value" ||
		name == ".cfi_return_column")
	{
		event.op = name == ".cfi_restore" ? AssemblyCfiOp::Restore
			: name == ".cfi_undefined"    ? AssemblyCfiOp::Undefined
			: name == ".cfi_same_value"   ? AssemblyCfiOp::Same
										  : AssemblyCfiOp::ReturnColumn;
		event.reg = reg(cursor);
	}
	else
		cursor.fail("CFI directive is not implemented: " + name);
	cursor.finish();
	return event;
}
} // namespace native
