#include "native/mir/mir_writer.h"

#include <cmath>
#include <cstdlib>
#include <cstring>

#include "lowir/literals.h"
#include "ir_symbol_model.h"

namespace native {

using lowir::Type;
using lowir::TypeKind;

namespace {

void append_int(std::string& out, int64_t value)
{
	char buffer[32];
	std::snprintf(buffer, sizeof buffer, "%lld", static_cast<long long>(value));
	out += buffer;
}

// The width name of a sign/zero extension's source.
const char* extension_name(TypeKind kind)
{
	switch (kind)
	{
	case TypeKind::I8:
	case TypeKind::U8:
		return "i8";
	case TypeKind::I16:
	case TypeKind::U16:
		return "i16";
	case TypeKind::I1:
		return "i1";
	default:
		return "i32";
	}
}

bool typed(Op op)
{
	switch (op)
	{
	case Op::Load:
	case Op::Store:
	case Op::Cmp:
	case Op::Test:
	case Op::Bt:
	case Op::FMov:
	case Op::FAdd:
	case Op::FSub:
	case Op::FMul:
	case Op::FDiv:
	case Op::FNeg:
	case Op::FCmp:
	case Op::FRet:
	case Op::FStp:
	case Op::LockXadd:
	case Op::Xchg:
	case Op::LockCmpxchg:
	case Op::LockCmpxchg16b:
	case Op::VaArg:
		return true;
	default:
		return false;
	}
}

} // namespace

void append_mir_float(std::string& out, TypeKind kind, uint64_t low, uint64_t high)
{
	lowir::Literal bits;
	bits.low = low;
	bits.high = high;
	const long double value = lowir::float_value(kind, bits);
	if (std::isnan(value))
	{
		if (std::signbit(value))
			out += '-';
		out += lowir::spelling(lowir::NamedLiteral::QuietNaN);
		return;
	}
	if (std::isinf(value))
	{
		if (value < 0)
			out += '-';
		out += lowir::spelling(lowir::NamedLiteral::Infinity);
		return;
	}
	char buffer[64];
	const int max_digits = kind == TypeKind::F32 ? 9 : (kind == TypeKind::F64 ? 17 : 21);
	for (int digits = 1; digits <= max_digits; ++digits)
	{
		std::snprintf(buffer, sizeof buffer, "%.*Lg", digits, value);
		bool same = false;
		if (kind == TypeKind::F32)
			same = std::strtof(buffer, nullptr) == static_cast<float>(value);
		else if (kind == TypeKind::F64)
			same = std::strtod(buffer, nullptr) == static_cast<double>(value);
		else
			same = std::strtold(buffer, nullptr) == value;
		if (same)
			break;
	}
	out += buffer;
	if (std::strpbrk(buffer, ".e") == nullptr)
		out += ".0";
	out += lowir::float_suffix(kind);
}

MirWriter::MirWriter(const lowir::Program& program, std::FILE* file) : program_(program), file_(file)
{}

MirWriter::~MirWriter()
{
	finish();
}

bool MirWriter::finish()
{
	if (!text_.empty())
	{
		if (std::fwrite(text_.data(), 1, text_.size(), file_) != text_.size())
			ok_ = false;
		text_.clear();
	}
	return ok_;
}

void MirWriter::flush_if_large()
{
	if (text_.size() >= (1u << 20))
		finish();
}

void MirWriter::type(Type t)
{
	if (t.is_object())
	{
		char buffer[48];
		std::snprintf(
			buffer, sizeof buffer, "obj<%llux%u>", static_cast<unsigned long long>(t.bytes), 1u << t.align_log2);
		text_ += buffer;
		return;
	}
	text_ += lowir::spelling(t.kind());
}

void MirWriter::reg(Reg r)
{
	const char* name = register_spelling(r == kFrame ? static_cast<Reg>(RBP) : r);
	text_ += name != nullptr ? name : "?";
}

void MirWriter::symbol(uint32_t sym)
{
	text_ += '@';
	text_ += program_.spelling(program_.symbols[sym].name);
}

void MirWriter::block_name(const Function& function, const Block& block)
{
	text_ += '^';
	if (block.name != 0)
		text_ += program_.spelling(block.name);
	else if (block.edge_from == lowir::kNoId)
	{
		// A switch subtree's block: named by its position.
		text_ += "switch.";
		append_int(text_, &block - function.blocks.data());
	}
	else
	{
		text_ += program_.spelling(block.edge_from);
		text_ += '~';
		text_ += program_.spelling(block.edge_to);
	}
}

void MirWriter::label(const Function& function, uint32_t block)
{
	if (block < function.blocks.size())
		block_name(function, function.blocks[block]);
	else
		text_ += "^?";
}

void MirWriter::operand(const Function* function, const Operand& op)
{
	switch (op.kind)
	{
	case OperandKind::None:
		break;
	case OperandKind::Reg:
		reg(op.reg);
		break;
	case OperandKind::Imm:
		if (op.imm_high != (op.imm < 0 ? -1 : 0))
		{
			char buffer[48];
			std::snprintf(buffer, sizeof buffer, "0x%016llx%016llx", static_cast<unsigned long long>(op.imm_high),
				static_cast<unsigned long long>(op.imm));
			text_ += buffer;
		}
		else
			append_int(text_, op.imm);
		break;
	case OperandKind::FImm:
		append_mir_float(text_, op.type, static_cast<uint64_t>(op.imm), static_cast<uint64_t>(op.imm_high));
		break;
	case OperandKind::Symbol:
		symbol(op.symbol);
		if (op.access != SymbolAccess::Direct)
		{
			text_ += '@';
			text_ += spelling(op.access);
		}
		if (op.disp != 0)
		{
			text_ += op.disp > 0 ? '+' : '-';
			append_int(text_, op.disp > 0 ? op.disp : -static_cast<int64_t>(op.disp));
		}
		break;
	case OperandKind::Label:
		if (function != nullptr)
			label(*function, op.symbol);
		break;
	case OperandKind::Mem:
		if (op.symbol != lowir::kNoId)
		{
			symbol(op.symbol);
			if (op.disp != 0)
			{
				text_ += op.disp > 0 ? '+' : '-';
				append_int(text_, op.disp > 0 ? op.disp : -static_cast<int64_t>(op.disp));
			}
			break;
		}
		text_ += '[';
		if (op.reg != kNoReg)
			reg(op.reg);
		if (op.index != kNoReg)
		{
			if (op.reg != kNoReg)
				text_ += '+';
			reg(op.index);
			if (op.scale != 1)
			{
				text_ += '*';
				append_int(text_, op.scale);
			}
		}
		if (op.reg == kNoReg && op.index == kNoReg)
			append_int(text_, op.disp);
		else if (op.disp != 0)
		{
			text_ += op.disp > 0 ? '+' : '-';
			append_int(text_, op.disp > 0 ? op.disp : -static_cast<int64_t>(op.disp));
		}
		text_ += ']';
		break;
	}
}

void MirWriter::call_facts(const Inst& inst)
{
	text_ += " [args=(";
	bool first = true;
	for (unsigned r = 0; r < 32; ++r)
	{
		if (!(inst.aux & (1u << r)))
			continue;
		if (!first)
			text_ += ',';
		first = false;
		reg(static_cast<Reg>(r));
	}
	text_ += ')';
	if (inst.aux2 != 0)
	{
		text_ += ", stack=";
		append_int(text_, (inst.aux2 + 15) & ~15u);
	}
	for (const InstFlag fact : { MF_VARIADIC, MF_UNWIND_NO, MF_NORETURN, MF_STRLEN_PREFIX })
	{
		if (inst.flags & fact)
		{
			text_ += ", ";
			text_ += call_fact_spelling(fact);
		}
	}
	text_ += ']';
}

void MirWriter::instruction(const Function& function, const Inst& inst)
{
	if (inst.op == Op::Nop)
		return;
	text_ += "    ";
	switch (inst.op)
	{
	case Op::Setcc:
	case Op::Jcc:
	case Op::Cmov:
		text_ += spelling(inst.op);
		text_ += spelling(static_cast<Cond>(inst.sub & 15));
		break;
	case Op::Sext:
	case Op::Zext:
		text_ += spelling(inst.op);
		text_ += '.';
		text_ += extension_name(inst.type);
		break;
	case Op::FSetcc:
		text_ += spelling(inst.op);
		text_ += lowir::spelling(static_cast<lowir::ComparePredicate>(inst.sub % 10));
		text_ += '.';
		text_ += lowir::spelling(inst.type);
		break;
	case Op::Convert:
		text_ += spelling(static_cast<ConvKind>(inst.sub % 6));
		text_ += '.';
		text_ += lowir::spelling(inst.type);
		text_ += '.';
		text_ += lowir::spelling(inst.type2);
		break;
	case Op::Wide:
		text_ += (inst.flags & MF_VOLATILE) ? lowir::spelling(lowir::Opcode::AtomicLoad)
											: spelling(static_cast<WideOp>(inst.sub % kWideOpCount));
		text_ += ".i128";
		break;
	case Op::WideSetcc:
		text_ += spelling(inst.op);
		text_ += '.';
		text_ += lowir::spelling(static_cast<lowir::ComparePredicate>(inst.sub % 10));
		text_ += ".i128";
		break;
	case Op::WideCmpBranch:
		text_ += spelling(inst.op);
		text_ += '.';
		text_ += lowir::spelling(static_cast<lowir::ComparePredicate>(inst.sub % 10));
		text_ += ".i128";
		break;
	default:
		text_ += spelling(inst.op);
		if (typed(inst.op) ||
			((inst.op == Op::Add || inst.op == Op::Sub || inst.op == Op::And || inst.op == Op::Or ||
				 inst.op == Op::Xor || inst.op == Op::Imul || inst.op == Op::Shl || inst.op == Op::Shr ||
				 inst.op == Op::Sar || inst.op == Op::Rol || inst.op == Op::Ror || inst.op == Op::Lea) &&
				inst.type != TypeKind::I64 && inst.type != TypeKind::Ptr && inst.type != TypeKind::Void) ||
			((inst.op == Op::Bswap || inst.op == Op::Bsf || inst.op == Op::Bsr) && inst.type != TypeKind::I64))
		{
			text_ += '.';
			text_ += lowir::spelling(inst.type);
		}
		break;
	}
	if (inst.op == Op::CopyBytes || inst.op == Op::ZeroBytes)
	{
		if (inst.flags & MF_COPY_DYNAMIC)
		{
			text_ += "_dynamic";
		}
		else
		{
			text_ += ' ';
			append_int(text_, inst.aux);
			text_ += 'x';
			append_int(text_, inst.aux2);
		}
		for (unsigned k = 0; k < inst.nops; ++k)
		{
			text_ += ", ";
			operand(&function, inst.ops[k]);
		}
	}
	else if (inst.op == Op::LockCmpxchg16b)
	{
		text_ += ' ';
		reg(static_cast<Reg>(inst.aux));
		for (unsigned k = 0; k < inst.nops; ++k)
		{
			text_ += ", ";
			operand(&function, inst.ops[k]);
		}
	}
	else
	{
		for (unsigned k = 0; k < inst.nops; ++k)
		{
			text_ += k == 0 ? (inst.op == Op::Call && inst.ops[0].kind != OperandKind::Symbol ? " *" : " ") : ", ";
			operand(&function, inst.ops[k]);
		}
	}
	if (inst.op == Op::Call)
		call_facts(inst);
	else if (inst.op == Op::EhPush || inst.op == Op::EhCatch || inst.op == Op::EhSelect || inst.op == Op::CatchAll)
	{
		text_ += inst.nops == 0 ? " " : ", ";
		append_int(text_, inst.aux);
	}
	else if (inst.op == Op::WideCmpBranch)
	{
		text_ += ", ";
		label(function, inst.aux);
	}
	else if (inst.op == Op::JumpTable)
	{
		text_ += ", [";
		for (uint32_t k = 0; k < inst.aux2; ++k)
		{
			if (k != 0)
				text_ += ", ";
			label(function, function.jump_targets[inst.aux + k]);
		}
		text_ += ']';
	}
	if (inst.location != 0 && inst.location <= function.locations.size())
	{
		const lowir::DebugLocation& loc = function.locations[inst.location - 1];
		text_ += " !dbg(";
		text_ += program_.spelling(loc.file);
		text_ += ", ";
		append_int(text_, loc.line);
		text_ += ", ";
		append_int(text_, loc.column);
		text_ += ')';
	}
	text_ += '\n';
}

void MirWriter::header(const Startup& startup)
{
	text_ += "machine_ir x86_64 linux\n";
	if (startup.entry == lowir::kNoId)
		return;
	text_ += "\nstartup\n";
	if (startup.init != lowir::kNoId)
	{
		text_ += "    call ";
		symbol(startup.init);
		text_ += " [args=()]\n";
	}
	text_ += "    call ";
	symbol(startup.entry);
	text_ += startup.entry_params >= 2 ? " [args=(rsi,rdi)]\n"
									   : (startup.entry_params == 1 ? " [args=(rdi)]\n" : " [args=()]\n");
	if (startup.fini != lowir::kNoId || startup.atexit != lowir::kNoId)
	{
		text_ += "    mov r12, rax\n";
		if (startup.atexit != lowir::kNoId)
			text_ += "    atexit_drain\n";
		if (startup.fini != lowir::kNoId)
		{
			text_ += "    call ";
			symbol(startup.fini);
			text_ += " [args=()]\n";
		}
		text_ += "    mov rdi, r12\n    exit\n";
	}
	else
		text_ += "    mov rdi, rax\n    exit\n";
}

void MirWriter::global(lowir::SymbolId sym)
{
	const lowir::Symbol& s = program_.symbols[sym];
	const lowir::Global& g = program_.globals[s.detail];
	text_ += "\nglobal ";
	symbol(sym);
	if (s.metadata.storage != ir_model::GSM_DEFAULT)
	{
		text_ += ' ';
		text_ += spelling(s.metadata.storage);
	}
	text_ += '\n';
	auto constant = [&](Type t, const lowir::Constant& c) {
		if (c.kind == lowir::ValueKind::Float)
			append_mir_float(text_, c.float_kind, c.bits.low, c.bits.high);
		else if (t.kind() == TypeKind::I128 &&
			static_cast<int64_t>(c.bits.high) != (static_cast<int64_t>(c.bits.low) < 0 ? -1 : 0))
		{
			char buffer[48];
			std::snprintf(buffer, sizeof buffer, "0x%016llx%016llx", static_cast<unsigned long long>(c.bits.high),
				static_cast<unsigned long long>(c.bits.low));
			text_ += buffer;
		}
		else
			append_int(text_, static_cast<int64_t>(c.bits.low));
	};
	if (g.init == lowir::GlobalInit::Data)
	{
		text_ += "  storage data\n";
		for (uint32_t i = 0; i < g.item_count; ++i)
		{
			const lowir::DataItem& item = program_.data_items[g.first_item + i];
			text_ += "  item ";
			if (item.kind == lowir::DataItemKind::Zero)
			{
				text_ += "zero ";
				append_int(text_, item.amount);
			}
			else if (item.kind == lowir::DataItemKind::Address)
			{
				text_ += "ptr addr ";
				symbol(item.address);
				if (item.amount != 0)
				{
					text_ += item.amount > 0 ? '+' : '-';
					append_int(text_, item.amount > 0 ? item.amount : -item.amount);
				}
			}
			else
			{
				type(item.type);
				text_ += ' ';
				constant(item.type, item.constant);
			}
			text_ += '\n';
		}
		flush_if_large();
		return;
	}
	text_ += "  storage scalar ";
	if (g.typed)
		type(g.type);
	else
		text_ += lowir::spelling(TypeKind::Ptr);
	text_ += '\n';
	text_ += "  init ";
	if (g.init == lowir::GlobalInit::Address)
	{
		text_ += "addr ";
		symbol(g.address);
		if (g.addend != 0)
		{
			text_ += g.addend > 0 ? '+' : '-';
			append_int(text_, g.addend > 0 ? g.addend : -g.addend);
		}
	}
	else
	{
		type(g.type);
		text_ += ' ';
		if (g.init == lowir::GlobalInit::Constant)
			constant(g.type, g.constant);
		else
			text_ += g.type.is_float() ? "0.0" : "0";
	}
	text_ += '\n';
	flush_if_large();
}

void MirWriter::function(const Function& f)
{
	text_ += "\nfunction ";
	symbol(f.symbol);
	text_ += "\n  abi\n";
	for (size_t p = 0; p < f.params.size(); ++p)
	{
		const AbiParam& param = f.params[p];
		for (uint32_t k = 0; k < param.part_count; ++k)
		{
			const AbiPart& part = f.parts[param.first_part + k];
			text_ += "    param %";
			text_ += program_.spelling(param.name);
			text_ += " -> ";
			if (part.reg == kNoReg)
			{
				text_ += "[rbp+";
				append_int(text_, part.stack);
				text_ += ']';
			}
			else
				reg(part.reg);
			text_ += " : ";
			if (part.reg == kNoReg || param.part_count == 1)
				type(param.type);
			else
				text_ += "i64";
			text_ += '\n';
		}
	}
	text_ += "    return ";
	type(f.return_type);
	text_ += " -> ";
	const TypeKind rk = f.return_type.kind();
	if (f.return_type.is_void())
		text_ += lowir::spelling(TypeKind::Void);
	else if (rk == TypeKind::F32 || rk == TypeKind::F64)
		reg(XMM0);
	else if (rk == TypeKind::F80)
		reg(kSt0);
	else if (f.return_type.is_object() && f.return_type.bytes > 16)
	{
		text_ += '[';
		reg(RAX);
		text_ += ']';
	}
	else
		reg(RAX);
	text_ += "\n  frame\n    stack_size ";
	append_int(text_, f.stack_size);
	text_ += "\n    scratch_bytes ";
	append_int(text_, f.scratch_bytes);
	text_ += f.omit_frame_pointer ? "\n    frame_pointer omit" : "\n    frame_pointer keep";
	text_ += f.duplicate_epilogues ? "\n    epilogues duplicated\n" : "\n    epilogues shared\n";
	if (f.aligned_base != 0)
	{
		// The over-aligned objects' region, addressed from r12.
		text_ += "    aligned_base ";
		append_int(text_, f.aligned_base);
		text_ += " r12 = rbp";
		append_int(text_, f.aligned_disp);
		text_ += '\n';
	}
	if (f.preserve_mask != 0)
	{
		text_ += "    preserve";
		const Reg order[6] = { RBX, R12, R13, R14, R15, RBP };
		for (unsigned k = 0; k < 6; ++k)
			if (f.preserve_mask & (1u << order[k]))
			{
				text_ += ' ';
				reg(order[k]);
			}
		text_ += '\n';
	}
	for (size_t b = 0; b < f.bindings.size(); ++b)
	{
		const FrameBinding& binding = f.bindings[b];
		text_ += "    ";
		text_ += spelling(binding.kind);
		text_ += ' ';
		text_ += sigil(binding.kind);
		text_ += binding.label != nullptr ? binding.label : program_.spelling(binding.name);
		text_ += " -> [rbp";
		if (binding.offset >= 0)
			text_ += '+';
		append_int(text_, binding.offset);
		text_ += "] : ";
		type(binding.type);
		text_ += '\n';
	}
	for (size_t b = 0; b < f.blocks.size(); ++b)
	{
		const Block& block = f.blocks[b];
		text_ += "\n  block ";
		block_name(f, block);
		text_ += '\n';
		for (uint32_t i = 0; i < block.count; ++i)
			instruction(f, f.insts[block.first + i]);
	}
	flush_if_large();
}

} // namespace native
