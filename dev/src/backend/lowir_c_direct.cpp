#include "backend/lowir_c_module.h"

#include "lowir/builder.h"

bool CppgmLowir::reserve_body(uint32_t symbol, const CppgmBodyCounts& minimum, CppgmBodyBuffers& buffers)
{
	if (!ready())
		return false;
	if (symbol >= program.symbols.size() || program.symbols[symbol].kind != lowir::SymbolKind::FunctionDefinition)
		return fail("direct body requires a defined function");
	lowir::FunctionBuilder builder(program, symbol);
	return builder.reserve(minimum, buffers) || fail(builder.construction_error());
}

bool CppgmLowir::commit_body(uint32_t symbol, const CppgmBodyCounts& counts, uint32_t temp_count)
{
	if (!ready())
		return false;
	if (symbol >= program.symbols.size() || program.symbols[symbol].kind != lowir::SymbolKind::FunctionDefinition)
		return fail("no open direct body");
	lowir::FunctionBuilder builder(program, symbol);
	if (!builder.commit(counts, temp_count))
		return fail(builder.construction_error());
	return true;
}
