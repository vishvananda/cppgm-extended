#include "native/backend.h"

#include <chrono>
#include <cstdio>
#include <cstring>
#include <sstream>
#include <sys/resource.h>

#include "native/backend_error.h"
#include "native/image/global_data.h"
#include "native/image/image.h"
#include "native/mir/mir_writer.h"
#include "native/runtime/runtime.h"
#include "native/select/call_abi.h"
#include "native/select/selector.h"
#include "native/x86/emitter.h"
#include "support/file_handle.h"
#include "support/telemetry.h"

namespace native {

using lowir::Type;
using lowir::TypeKind;

namespace {

double milliseconds_since(std::chrono::steady_clock::time_point start)
{
	return std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - start).count();
}

// Lays out one global definition in the data section, in definition order
// after the previous global, and writes its constants and address fixups in
// place (zero bytes need no writes).
void emit_global(
	Image& image, const lowir::Program& program, lowir::SymbolId symbol, std::vector<GlobalAddress>& addresses)
{
	const lowir::Symbol& s = program.symbols[symbol];
	const lowir::Global& g = program.globals[s.detail];
	// Every global, read-only or not, zero or not, follows the previous one
	// in definition order: producers address table groups by their layout.
	// Trailing zero bytes of the section are left out of the file.
	const Section section = Section::Data;
	uint32_t align = 1;
	const uint64_t size = global_size(program, g, align);
	const uint64_t base = image.reserve(section, size, align);
	image.define(symbol, section, base);
	addresses.clear();
	write_global(program, g, image.data(), base, addresses);
	for (const GlobalAddress& address : addresses)
		image.add_data_fixup(section, base + address.offset, address.target, address.addend);
}

// A function's object name: its `object=` name, else its LowIR name.
const char* object_name(const lowir::Program& program, const lowir::Symbol& symbol)
{
	return program.spelling(symbol.metadata.object != 0 ? symbol.metadata.object : symbol.name);
}

// The external C `atexit`, `i32 (ptr)` (C99 7.20.4.2; C++11 18.5): a
// declaration the runtime defines unless the program defines that object.
bool declares_atexit(const lowir::Program& program, const lowir::Symbol& symbol)
{
	if (symbol.metadata.tls_for != lowir::kNoId || std::strcmp(object_name(program, symbol), "atexit") != 0)
		return false;
	const lowir::Signature& signature = program.signatures[program.functions[symbol.detail].signature];
	return signature.parameter_count == 1 && signature.boundary.arity == ir_model::CAM_FIXED &&
		program.parameters_of(signature)[0].type.kind() == TypeKind::Ptr && signature.result.kind() == TypeKind::I32;
}

Startup find_startup(const lowir::Program& program)
{
	Startup startup;
	bool defines_atexit = false;
	for (lowir::SymbolId s = 0; s < program.symbols.size(); ++s)
	{
		const lowir::Symbol& symbol = program.symbols[s];
		if (symbol.kind == lowir::SymbolKind::FunctionDeclaration && declares_atexit(program, symbol))
			startup.atexit = s;
		if (symbol.kind != lowir::SymbolKind::FunctionDefinition)
			continue;
		defines_atexit = defines_atexit || std::strcmp(object_name(program, symbol), "atexit") == 0;
		switch (symbol.metadata.role)
		{
		case ir_model::SR_ENTRY:
			startup.entry = s;
			startup.entry_params = program.signatures[program.functions[symbol.detail].signature].parameter_count;
			break;
		case ir_model::SR_INIT:
			startup.init = s;
			break;
		case ir_model::SR_FINI:
			startup.fini = s;
			break;
		default:
			break;
		}
	}
	if (defines_atexit)
		startup.atexit = lowir::kNoId;
	return startup;
}

} // namespace

bool compile_program(
	const lowir::Program& program, const BackendOptions& options, std::string& error, std::string* stats_line)
{
	const std::chrono::steady_clock::time_point start = std::chrono::steady_clock::now();
	const Startup startup = find_startup(program);
	if (!options.output.empty() && startup.entry == lowir::kNoId)
	{
		error = "no entry function for an executable";
		return false;
	}
	Image image;
	RuntimeSymbols runtime;
	declare_runtime(image, runtime);
	FileHandle mir_handle(options.machine_ir.empty() ? nullptr : std::fopen(options.machine_ir.c_str(), "wb"));
	std::FILE* const mir_file = mir_handle.get();
	if (!options.machine_ir.empty() && mir_file == nullptr)
	{
		error = "cannot write " + options.machine_ir;
		return false;
	}
	EmitStats emit_stats;
	Selector selector(program);
	selector.set_optimization_level(options.optimization_level);
	double select_ms = 0;
	double encode_ms = 0;
	try
	{
		MirWriter writer(program, mir_file);
		if (mir_file != nullptr)
			writer.header(startup);
		// The data section is sized up front so it never grows by copying.
		uint64_t data_bytes = 0;
		for (size_t i = 0; i < program.order.size(); ++i)
		{
			const lowir::Symbol& symbol = program.symbols[program.order[i]];
			if (symbol.kind != lowir::SymbolKind::GlobalDefinition)
				continue;
			uint32_t align = 1;
			data_bytes += global_size(program, program.globals[symbol.detail], align) + align;
		}
		image.data().reserve(data_bytes);
		std::vector<GlobalAddress> addresses;
		for (size_t i = 0; i < program.order.size(); ++i)
		{
			const lowir::SymbolId s = program.order[i];
			if (program.symbols[s].kind != lowir::SymbolKind::GlobalDefinition)
				continue;
			emit_global(image, program, s, addresses);
			if (mir_file != nullptr)
				writer.global(s);
		}
		FunctionEmitter emitter(image, runtime, emit_stats);
		Function mir;
		for (size_t i = 0; i < program.order.size(); ++i)
		{
			const lowir::SymbolId s = program.order[i];
			if (program.symbols[s].kind != lowir::SymbolKind::FunctionDefinition)
				continue;
			// Phase clocks are read only for --stats.
			std::chrono::steady_clock::time_point phase;
			if (stats_line != nullptr)
				phase = std::chrono::steady_clock::now();
			selector.select(s, mir);
			if (stats_line != nullptr)
				select_ms += milliseconds_since(phase);
			if (mir_file != nullptr)
				writer.function(mir);
			if (!options.output.empty())
			{
				if (stats_line != nullptr)
					phase = std::chrono::steady_clock::now();
				emitter.emit(mir);
				if (stats_line != nullptr)
					encode_ms += milliseconds_since(phase);
			}
		}
		if (mir_file != nullptr && !writer.finish())
		{
			error = "cannot write " + options.machine_ir;
			return false;
		}
	}
	catch (const BackendError& e)
	{
		// LowIR outside the native subset; an InternalError (a compiler
		// bug, not an error of the program) leaves for the tool to report.
		error = e.what();
		return false;
	}
	if (!mir_handle.close())
	{
		error = "cannot write " + options.machine_ir;
		return false;
	}
	if (!options.output.empty())
	{
		const uint32_t entry = emit_runtime(image, program, startup, runtime);
		uint32_t undefined = lowir::kNoId;
		if (!image.write_executable(options.output, entry, error, undefined))
		{
			if (options.undefined_symbol != nullptr && undefined != lowir::kNoId)
				*options.undefined_symbol = true;
			if (undefined < program.symbols.size())
				error += std::string(": ") + program.spelling(program.symbols[undefined].name);
			else if (undefined == runtime.eh_catch)
				error += ": C++ catch clauses need cppgm++'s link-mode runtime library";
			else
			{
				for (unsigned k = 0; k < 4; ++k)
				{
					if (undefined == runtime.wide_division[k])
						error += std::string(": ") + kWideDivisionSymbols[k] +
							" (i128 division) needs cppgm++'s link mode";
				}
				for (unsigned k = 0; k < 12; ++k)
				{
					if (undefined == runtime.float_conversion[k])
						error += std::string(": ") + kFloatConversionSymbols[k] +
							" (i128/floating conversion) needs cppgm++'s link mode";
				}
			}
			return false;
		}
	}
#if CPPGM_TELEMETRY_ENABLED
	if (stats_line != nullptr)
	{
		const SelectStats& s = selector.stats();
		struct rusage usage;
		getrusage(RUSAGE_SELF, &usage);
		std::ostringstream line;
		line << "lowir2native_stats functions=" << s.functions << " lowir_instructions=" << s.lowir_instructions
			 << " mir_instructions=" << s.mir_instructions << " spills=" << s.spills << " homes=" << s.homes
			 << " lazy_addresses=" << s.lazy_addresses << " folded_loads=" << s.folded_loads
			 << " fused_compares=" << s.fused_compares << " promoted_slots=" << s.promoted_slots
			 << " dead_slots=" << s.dead_slots << " global_registers=" << s.global_registers
			 << " liveness_steps=" << s.liveness_steps << " hull_fallbacks=" << s.hull_fallbacks
			 << " edge_blocks=" << s.edge_blocks << " threaded_blocks=" << s.threaded_blocks
			 << " layout_branches=" << s.layout_branches << " strlen_prefixes=" << s.strlen_prefixes
			 << " dynamic_copies=" << s.dynamic_copies << " jump_tables=" << s.jump_tables
			 << " rotated_loops=" << s.rotated_loops << " loop_splits=" << s.loop_splits
			 << " home_splits=" << s.home_splits << " ordered_frames=" << s.ordered_frames
			 << " dead_frame_stores=" << s.dead_frame_stores << " native_returns=" << emit_stats.returns
			 << " short_branches=" << emit_stats.short_branches << " long_branches=" << emit_stats.long_branches
			 << " fixups=" << emit_stats.fixups << " scratch_carried_reloads=" << emit_stats.scratch_carried_reloads
			 << " code_bytes=" << emit_stats.code_bytes << " select_ms=" << select_ms << " encode_ms=" << encode_ms
			 << " total_ms=" << milliseconds_since(start) << " peak_rss_kb=" << usage.ru_maxrss;
		*stats_line = line.str();
	}
#else
	(void)start;
	(void)select_ms;
	(void)encode_ms;
#endif
	return true;
}

} // namespace native
