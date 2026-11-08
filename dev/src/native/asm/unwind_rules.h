#pragma once

#include "asm/module_text.h"

namespace native {

enum class AssemblyCfiOp : uint8_t
{
	Start,
	End,
	DefCfa,
	DefRegister,
	DefOffset,
	Adjust,
	Offset,
	RelOffset,
	ValOffset,
	Register,
	Restore,
	Undefined,
	Same,
	Remember,
	RestoreState,
	ReturnColumn,
	Signal,
	Personality,
	Lsda,
	Sections,
	Escape
};
struct AssemblyCfiEvent
{
	AssemblyCfiOp op;
	uint32_t anchor = 0;
	uint32_t line = 1;
	uint32_t reg = 0;
	uint32_t second = 0;
	int64_t value = 0;
	bool simple = false;
	uint8_t encoding = 0xff;
	asm_model::SymbolReference reference;
	std::vector<uint8_t> bytes;
};
AssemblyCfiEvent parse_assembly_cfi(
	const assembly::ModuleLine& line, uint32_t anchor, assembly::SymbolLookup* symbols = nullptr);

class AssemblyCfiRules
{
public:
	explicit AssemblyCfiRules(bool simple);
	void apply(const AssemblyCfiEvent& event, uint32_t pc);
	const std::vector<uint8_t>& bytes() const { return bytes_; }

private:
	struct State
	{
		int64_t offset = 8;
		uint32_t reg = 7;
		bool offset_known = true;
	};
	void advance(uint32_t pc);
	void cfa(const AssemblyCfiEvent& event);
	void saved(const AssemblyCfiEvent& event);
	State state_;
	std::vector<State> stack_;
	std::vector<uint8_t> bytes_;
	uint32_t pc_ = 0;
};
} // namespace native
