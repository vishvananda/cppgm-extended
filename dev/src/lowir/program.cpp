#include "lowir/program.h"

#include <cstring>

namespace lowir {

Program::Program() : names(arena)
{}

SymbolId Program::symbol_for(NameId name)
{
	if (name >= symbol_by_name.size())
		symbol_by_name.resize(names.size() > name ? names.size() : name + 1, kNoId);
	SymbolId& entry = symbol_by_name[name];
	if (entry == kNoId)
	{
		entry = static_cast<SymbolId>(symbols.size());
		Symbol symbol;
		symbol.name = name;
		symbol.kind = SymbolKind::Unresolved;
		symbol.detail = kNoId;
		symbols.push_back(symbol);
	}
	return entry;
}

void Program::declare_global(SymbolId id, SymbolKind kind, const Global& global)
{
	Symbol& symbol = symbols[id];
	symbol.kind = kind;
	symbol.detail = static_cast<uint32_t>(globals.size());
	globals.push_back(global);
	order.push_back(id);
}

void Program::declare_function(SymbolId id, SymbolKind kind, SignatureId signature)
{
	Symbol& symbol = symbols[id];
	symbol.kind = kind;
	symbol.detail = static_cast<uint32_t>(functions.size());
	Function function;
	function.signature = signature;
	function.location = DebugLocation{ 0, 0, 0 };
	if (kind == SymbolKind::FunctionDefinition)
		function.body.reset(new FunctionBody());
	functions.push_back(std::move(function));
	order.push_back(id);
}

const Signature* Program::call_boundary(const Instruction& call) const
{
	if ((call.flags & IF_SIGNATURE) != 0)
		return &signatures[static_cast<SignatureId>(call.immediate)];
	if (!call.ops[0].is(ValueKind::Symbol))
		return nullptr;
	const Symbol& callee = symbols[call.ops[0].index];
	if (!is_function_kind(callee.kind))
	{
		const SignatureId forward = callee.kind == SymbolKind::Unresolved ? forward_boundary(call.ops[0].index) : kNoId;
		return forward != kNoId ? &signatures[forward] : nullptr;
	}
	return &signatures[functions[callee.detail].signature];
}

void Program::set_forward_boundary(SymbolId symbol, SignatureId signature)
{
	if (symbol >= forward_boundaries.size())
		forward_boundaries.resize(symbols.size() > symbol ? symbols.size() : symbol + 1, kNoId);
	forward_boundaries[symbol] = signature;
}

void Program::set_forward_global(SymbolId symbol, uint8_t binding, uint8_t storage, bool defined)
{
	if (symbol >= forward_globals.size())
		forward_globals.resize(symbols.size() > symbol ? symbols.size() : symbol + 1, 0);
	forward_globals[symbol] = static_cast<uint8_t>(binding | (defined ? 8 : 0) | (storage << 4));
}

uint8_t Program::forward_binding(SymbolId symbol) const
{
	return symbol < forward_globals.size() ? forward_globals[symbol] & 7 : 0;
}

uint8_t Program::forward_storage(SymbolId symbol) const
{
	return symbol < forward_globals.size() ? forward_globals[symbol] >> 4 : 0;
}

bool Program::forward_defined(SymbolId symbol) const
{
	return symbol < forward_globals.size() && (forward_globals[symbol] & 8) != 0;
}

bool Program::may_end_block(const Instruction& inst) const
{
	if (is_terminator(inst.op))
		return true;
	if (inst.op != Opcode::Call)
		return false;
	const Signature* boundary = call_boundary(inst);
	return boundary != nullptr && boundary->boundary.returns == ir_model::CRM_NORETURN;
}

bool Program::strlen_function(SymbolId symbol) const
{
	const Symbol& callee = symbols[symbol];
	if (!is_function_kind(callee.kind))
		return false;
	const char* spelling = this->spelling(callee.metadata.object != 0 ? callee.metadata.object : callee.name);
	return (callee.metadata.object != 0 && std::strcmp(spelling, "cppgm_builtin_strlen") == 0) ||
		(callee.metadata.linkage == ir_model::LLM_C && std::strcmp(spelling, "strlen") == 0);
}

bool Program::builtin_memcpy(SymbolId symbol) const
{
	const Symbol& callee = symbols[symbol];
	return is_function_kind(callee.kind) && callee.metadata.object != 0 &&
		std::strcmp(spelling(callee.metadata.object), "cppgm_builtin_memcpy") == 0;
}

bool Program::trap_function(SymbolId symbol) const
{
	const Symbol& callee = symbols[symbol];
	return is_function_kind(callee.kind) && callee.metadata.object != 0 &&
		std::strcmp(spelling(callee.metadata.object), "cppgm_builtin_trap") == 0;
}

bool Program::memset_function(SymbolId symbol) const
{
	const Symbol& callee = symbols[symbol];
	if (!is_function_kind(callee.kind))
		return false;
	const char* spelling = this->spelling(callee.metadata.object != 0 ? callee.metadata.object : callee.name);
	return (callee.metadata.object != 0 && std::strcmp(spelling, "cppgm_builtin_memset") == 0) ||
		(callee.metadata.linkage == ir_model::LLM_C && std::strcmp(spelling, "memset") == 0);
}

bool Program::memcpy_function(SymbolId symbol) const
{
	const Symbol& callee = symbols[symbol];
	if (!is_function_kind(callee.kind))
		return false;
	const char* spelling = this->spelling(callee.metadata.object != 0 ? callee.metadata.object : callee.name);
	return (callee.metadata.object != 0 && std::strcmp(spelling, "cppgm_builtin_memcpy") == 0) ||
		(callee.metadata.linkage == ir_model::LLM_C && std::strcmp(spelling, "memcpy") == 0);
}

bool Program::memcmp_function(SymbolId symbol) const
{
	const Symbol& callee = symbols[symbol];
	if (!is_function_kind(callee.kind))
		return false;
	const char* spelling = this->spelling(callee.metadata.object != 0 ? callee.metadata.object : callee.name);
	return (callee.metadata.object != 0 && std::strcmp(spelling, "cppgm_builtin_memcmp") == 0) ||
		(callee.metadata.linkage == ir_model::LLM_C && std::strcmp(spelling, "memcmp") == 0);
}

SignatureId Program::add_signature(const Signature& signature)
{
	signatures.push_back(signature);
	return static_cast<SignatureId>(signatures.size() - 1);
}

} // namespace lowir
