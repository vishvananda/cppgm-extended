#pragma once

#include <memory>
#include "native/asm/unwind_rules.h"
#include "native/object/object_file.h"
#include "native/object/host_unwind.h"
#include "native/target.h"

namespace native {
class ModuleUnwind
{
public:
	ModuleUnwind(object::ObjectFile& out, Target target);
	~ModuleUnwind();
	void begin(uint32_t anchor);
	void end(uint32_t anchor);
	void record(const assembly::ModuleLine& line, uint32_t anchor, assembly::SymbolLookup* symbols);
	bool pending() const { return !events_.empty(); }
	void flush(
		uint32_t section, const std::vector<uint32_t>& lines, const std::vector<uint32_t>& positions, uint32_t end);
	void finish();

private:
	struct Procedure;
	void apply(const AssemblyCfiEvent& event, uint32_t section, uint32_t position);
	void emit(uint32_t end);
	uint32_t section_symbol(uint32_t section);
	object::ObjectFile& out_;
	Target target_;
	void pointer_relocation(uint32_t section, uint32_t offset, uint8_t encoding,
		const asm_model::SymbolReference& reference, bool personality);
	std::vector<AssemblyCfiEvent> events_;
	std::unique_ptr<Procedure> current_;
	uint32_t frame_ = 0;
	struct Cie
	{
		CieDescription description;
		uint32_t personality;
		int64_t addend;
		uint32_t offset;
	};
	std::vector<Cie> cies_;
};
} // namespace native
