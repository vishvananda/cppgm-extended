#pragma once

#include <map>
#include "asm/module_text.h"
#include "lowir/program.h"
#include "native/object/object_file.h"
#include "native/target.h"

namespace native {

class ModuleSymbols : public assembly::SymbolLookup
{
public:
	ModuleSymbols(object::ObjectFile& out, Target target);
	bool lookup(const std::string& name, asm_model::SymbolReference& reference) override;
	bool interposable(const std::string& name) const override;
	uint32_t named(const std::string& name);
	uint32_t program_symbol(const lowir::Program& program, lowir::SymbolId symbol);
	uint32_t token(assembly::ModuleCursor& cursor);
	void define(uint32_t symbol, uint32_t section, uint64_t offset);
	void bind(uint32_t symbol, object::Binding binding);
	void alias(uint32_t symbol, uint32_t target, int64_t addend);
	void finish();

private:
	uint32_t intern(const std::string& name);
	struct Alias
	{
		uint32_t symbol;
		uint32_t target;
		int64_t addend;
	};
	object::ObjectFile& out_;
	Target target_;
	std::map<std::string, uint32_t> names_;
	std::map<uint32_t, object::Binding> bindings_;
	std::vector<Alias> aliases_;
};

} // namespace native
