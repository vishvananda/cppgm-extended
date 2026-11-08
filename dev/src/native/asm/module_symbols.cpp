#include "native/asm/module_symbols.h"

#include "native/backend_error.h"

namespace native {

ModuleSymbols::ModuleSymbols(object::ObjectFile& out, Target target) : out_(out), target_(target)
{
	for (uint32_t i = 1; i < out_.symbols.size(); ++i)
	{
		const char* name = out_.symbol_name(i);
		if (*name != '\0')
			names_.emplace(name, i);
	}
}
uint32_t ModuleSymbols::intern(const std::string& name)
{
	const auto found = names_.find(name);
	if (found != names_.end())
		return found->second;
	object::Symbol symbol;
	symbol.name = out_.add_string(name);
	symbol.binding = object::Binding::Global;
	const uint32_t id = out_.add_symbol(symbol);
	names_.emplace(name, id);
	return id;
}
uint32_t ModuleSymbols::named(const std::string& name)
{
	if (name.empty() || (name[0] >= '0' && name[0] <= '9'))
		throw BackendError("invalid module linker symbol: " + name);
	return intern(target_ == Target::MacOS && name[0] == '_' ? name.substr(1) : name);
}
bool ModuleSymbols::lookup(const std::string& name, asm_model::SymbolReference& reference)
{
	if (name.empty() || (name[0] >= '0' && name[0] <= '9'))
		return false;
	reference.symbol = named(name);
	return true;
}
bool ModuleSymbols::interposable(const std::string& name) const
{
	const std::string normalized = target_ == Target::MacOS && !name.empty() && name[0] == '_' ? name.substr(1) : name;
	const auto found = names_.find(normalized);
	if (found == names_.end())
		return false;
	const auto binding = bindings_.find(found->second);
	return (binding == bindings_.end() ? out_.symbols[found->second].binding : binding->second) !=
		object::Binding::Local;
}
uint32_t ModuleSymbols::program_symbol(const lowir::Program& program, lowir::SymbolId symbol)
{
	const lowir::Symbol& source = program.symbols[symbol];
	std::string name = program.spelling(source.metadata.object != 0 ? source.metadata.object : source.name);
	if (source.metadata.binding == ir_model::SBM_INTERNAL && source.metadata.object == 0)
		name.insert(name.begin(), '@');
	return intern(name);
}
uint32_t ModuleSymbols::token(assembly::ModuleCursor& cursor)
{
	cursor.space();
	if (const assembly::SymbolUse* use = cursor.symbol_use())
	{
		if (use->reference.addend != 0)
			cursor.fail("symbol declaration cannot have an addend");
		cursor.advance(use->count);
		return use->reference.symbol;
	}
	return named(cursor.word());
}
void ModuleSymbols::define(uint32_t symbol, uint32_t section, uint64_t offset)
{
	object::Symbol& entry = out_.symbols[symbol];
	if (entry.section != object::kUndefinedSection)
		throw BackendError("duplicate module definition: " + std::string(out_.symbol_name(symbol)));
	entry.section = section;
	entry.value = offset;
	const auto binding = bindings_.find(symbol);
	entry.binding = binding == bindings_.end() ? object::Binding::Local : binding->second;
	if (entry.type == object::SymbolType::None)
		entry.type = out_.sections[section].kind == object::SectionKind::Text ? object::SymbolType::Function
																			  : object::SymbolType::Object;
}
void ModuleSymbols::bind(uint32_t symbol, object::Binding binding)
{
	const auto previous = bindings_.find(symbol);
	if (previous != bindings_.end() && previous->second == object::Binding::Weak && binding == object::Binding::Global)
		return;
	bindings_[symbol] = binding;
	out_.symbols[symbol].binding = binding;
}
void ModuleSymbols::alias(uint32_t symbol, uint32_t target, int64_t addend)
{
	aliases_.push_back(Alias{ symbol, target, addend });
}
void ModuleSymbols::finish()
{
	std::vector<bool> done(aliases_.size(), false);
	for (size_t pass = 0; pass < aliases_.size(); ++pass)
	{
		bool progress = false;
		for (size_t i = 0; i < aliases_.size(); ++i)
		{
			if (done[i])
				continue;
			const Alias alias = aliases_[i];
			const object::Symbol target = out_.symbols[alias.target];
			if (target.section == object::kUndefinedSection)
				continue;
			define(alias.symbol, target.section, target.value + alias.addend);
			out_.symbols[alias.symbol].type = target.type;
			out_.symbols[alias.symbol].size = target.size;
			done[i] = true;
			progress = true;
		}
		if (!progress)
			break;
	}
	for (bool complete : done)
		if (!complete)
			throw BackendError("cyclic or undefined module alias");
}
} // namespace native
