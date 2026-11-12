#include "wasm/module.h"

#include <sstream>

namespace wasm {

namespace {

void types(std::ostream& stream, const std::vector<ValueType>& values)
{
	for (ValueType type : values)
		stream << ' ' << spelling(type);
}

} // namespace

void dump_module(const lowir::Program& program, const Module& module, std::string& output)
{
	std::ostringstream stream;
	for (uint32_t i = 0; i < module.signatures.size(); ++i)
	{
		stream << "type " << i << " params";
		types(stream, module.signatures[i].parameters);
		stream << " results";
		types(stream, module.signatures[i].results);
		stream << '\n';
	}
	for (lowir::SymbolId id : program.order)
		if (lowir::is_function_kind(program.symbols[id].kind))
		{
			const uint32_t index = module.function_indices[id];
			stream << "function_index " << index << ' ' << program.spelling(program.symbols[id].name) << " type "
				   << module.definition_types[index] << '\n';
		}
	for (uint32_t i = 0; i < module.global_types.size(); ++i)
		stream << "global " << i << ' ' << spelling(module.global_types[i]) << " mutable " << module.mutable_globals[i]
			   << '\n';
	for (uint32_t i = 0; i < module.tag_types.size(); ++i)
		stream << "tag " << i << " type " << module.tag_types[i] << '\n';
	output += stream.str();
}

void dump_function(const Function& function, const lowir::Program& program, lowir::SymbolId symbol, std::string& output)
{
	const auto& body = *program.functions[program.symbols[symbol].detail].body;
	std::ostringstream stream;
	stream << "function " << program.spelling(program.symbols[symbol].name) << " frame_bytes " << function.frame_bytes
		   << " params " << function.parameter_count << '\n';
	for (const FrameObject& object : function.frame_objects)
		stream << "  frame_object " << object.storage.identity << " offset " << object.offset << " bytes "
			   << object.storage.bytes << " align " << object.storage.alignment << '\n';
	for (uint32_t i = 0; i < function.locals.size(); ++i)
		stream << "  local " << i << ' ' << spelling(function.locals[i]) << '\n';
	for (const Instruction& inst : function.instructions)
	{
		stream << "  " << spelling(inst.op);
		for (uint32_t i = 0; i < inst.operands.count; ++i)
		{
			const uint64_t value = function.operands[inst.operands.first + i];
			if ((immediate_form(inst.op) == Immediate::BlockType ||
					(immediate_form(inst.op) == Immediate::TryTable && i == 0)) &&
				static_cast<int64_t>(value) < 0)
				stream << ' ' << spelling(static_cast<ValueType>(value + 128));
			else
				stream << ' ' << value;
		}
		stream << " ; effects";
		const uint16_t mask = effects(inst.op);
		for (unsigned bit = 1; bit <= machine_model::MayThrow; bit <<= 1)
			if ((mask & bit) != 0)
				stream << ' ' << machine_model::spelling(static_cast<machine_model::Effect>(bit));
		if (inst.location != 0)
		{
			const auto& location = body.locations[inst.location - 1];
			stream << " ; dbg " << program.spelling(location.file) << ' ' << location.line << ' ' << location.column
				   << " scope " << location.scope;
		}
		stream << '\n';
	}
	stream << "end_function\n";
	output += stream.str();
}

} // namespace wasm
