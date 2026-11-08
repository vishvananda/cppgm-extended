#include "native/asm/functions.h"

#include "native/asm/binding.h"
#include "native/asm/encoding.h"
#include "native/backend_error.h"
#include "native/image/image.h"

namespace native {

void emit_assembly_function(Image& image, const lowir::Program& program, const lowir::AssemblyFunction& function)
{
	const lowir::FunctionBody& body = *function.body;
	std::vector<uint8_t> code;
	Assembler assembler(code);
	for (const lowir::Instruction& inst : body.instructions)
	{
		if (inst.op != lowir::Opcode::Asm)
			continue;
		const std::vector<lowir::Value> arguments(
			body.operands.begin() + inst.extra_first, body.operands.begin() + inst.extra_first + inst.extra_count);
		const std::shared_ptr<const AssemblyPlan> plan = bind_assembly(
			*program.assembly, static_cast<uint32_t>(inst.immediate), body, arguments);
		encode_assembly(assembler, plan->instructions);
	}
	// A raw body must arrange its own return or branch. Guard accidental
	// fallthrough without inventing a C++ return value or an ABI epilogue.
	assembler.byte(0x0f);
	assembler.byte(0x0b);
	if (!assembler.resolve_labels())
		throw BackendError("unresolved assembly function label");
	std::vector<uint8_t>& text = image.text();
	while (text.size() % 16 != 0)
		text.push_back(0xcc);
	const uint64_t base = text.size();
	image.define(function.symbol, Section::Text, base);
	text.insert(text.end(), code.begin(), code.end());
	image.add_code_fixups(assembler.fixups(), base);
}

} // namespace native
