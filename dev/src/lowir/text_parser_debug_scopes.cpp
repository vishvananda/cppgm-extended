#include "lowir/text_parser.h"

namespace lowir {
namespace text {

void Parser::parse_debug_scope()
{
	if (!program_.debug)
		program_.debug.reset(new DebugInfo());
	const int64_t id = parse_small_integer("debug scope id");
	if (id <= 0 || uint64_t(id) + scope_base_ != program_.debug->scopes.size())
		fail("debug scope ids must be consecutive from one");
	const NameId owner = expect_name(TokenKind::Global, "debug scope owner");
	const int64_t parent = parse_small_integer("debug parent scope");
	if (parent < 0 || parent >= id)
		fail("debug scope parent must precede child");
	const uint32_t parent_id = parent != 0 ? static_cast<uint32_t>(parent) + scope_base_ : 0;
	if (parent_id != 0 && program_.debug->scopes[parent_id].owner != owner)
		fail("debug scope parent belongs to another function");
	program_.debug->scopes.push_back(DebugLexicalScope{ owner, parent_id, 0 });
}

void Parser::finish_debug_scopes()
{
	DebugInfo& debug = *program_.debug;
	for (uint32_t id = 1; id < debug.scopes.size(); ++id)
	{
		DebugLexicalScope& scope = debug.scopes[id];
		const SymbolId symbol = program_.find_symbol(scope.owner);
		if (symbol == kNoId || program_.symbols[symbol].kind != SymbolKind::FunctionDefinition)
			continue; // An optimizer may remove the scope's original function.
		if (symbol >= debug.functions.size())
			debug.functions.resize(symbol + 1);
		DebugFunction& function = debug.functions[symbol];
		if (function.last_scope != 0)
			debug.scopes[function.last_scope].next = id;
		else
			function.first_scope = id;
		function.last_scope = id;
	}
	for (SymbolId id = 0; id < program_.symbols.size(); ++id)
	{
		const Symbol& symbol = program_.symbols[id];
		if (symbol.kind != SymbolKind::FunctionDefinition)
			continue;
		const Function& function = program_.functions[symbol.detail];
		auto valid = [&](uint32_t scope) { return scope < debug.scopes.size(); };
		if (!valid(function.location.scope))
			fail_at(symbol_uses_[id], "undefined debug scope");
		if (function.body)
			for (const DebugLocation& location : function.body->locations)
				if (!valid(location.scope))
					fail_at(symbol_uses_[id], "undefined debug scope");
		if (id >= debug.functions.size())
			continue;
		for (const DebugLocal& local : debug.functions[id].locals)
			if (!valid(local.scope) || !valid(local.declaration.scope) ||
				(local.scope != 0 && debug.scopes[local.scope].owner != symbol.name))
				fail_at(symbol_uses_[id], "undefined or foreign debug local scope");
	}
}
} // namespace text
} // namespace lowir
