#include "wasm/exceptions.h"

#include "opt/function_ir.h"

namespace wasm {

using namespace lowir;

namespace {

void lsda(const machine::ExceptionTables& tables, ObjectData& data)
{
	Bytes actions, calls;
	for (const auto& landing : tables.landings)
	{
		const bool cleanup = landing.count == 0 || (landing.count == 1 && tables.filters[landing.first] == 0);
		calls.unsigned_leb(0);
		calls.unsigned_leb(cleanup ? 0 : actions.data.size() + 1);
		if (!cleanup)
			for (uint32_t k = 0; k < landing.count; ++k)
			{
				actions.signed_leb(tables.filters[landing.first + k]);
				actions.signed_leb(k + 1 == landing.count ? 0 : 1);
			}
	}
	Bytes rest;
	rest.byte(1);
	rest.unsigned_leb(calls.data.size());
	rest.append(calls);
	rest.append(actions);
	Bytes& out = data.contents.bytes;
	out.byte(0xff);
	if (tables.types.empty() && tables.specs.empty())
	{
		out.byte(0xff);
		out.append(rest);
		return;
	}
	// A fixed-width ULEB leaves the type base independent of header growth.
	out.byte(0);
	const uint32_t padding = static_cast<uint32_t>((4 - (7 + rest.data.size()) % 4) % 4);
	uint32_t offset = static_cast<uint32_t>(rest.data.size() + padding + 4 * tables.types.size());
	for (unsigned i = 0; i < 5; ++i)
	{
		out.byte(static_cast<uint8_t>((offset & 127) | (i == 4 ? 0 : 128)));
		offset >>= 7;
	}
	out.append(rest);
	for (uint32_t k = 0; k < padding; ++k)
		out.byte(0);
	for (size_t k = tables.types.size(); k-- > 0;)
	{
		if (tables.types[k] != kNoId)
			data.contents.relocations.push_back(Relocation{
				RelocationKind::MemoryAddressI32, static_cast<uint32_t>(out.data.size()), tables.types[k], 0 });
		out.fixed(0, 4);
	}
	for (uint8_t byte : tables.specs)
		out.byte(byte);
}

void control_edges(const FunctionBody& body, ExceptionPlan& plan)
{
	std::vector<BlockId> placed;
	std::vector<std::pair<BlockId, BlockId>> edges;
	std::vector<BlockId> seen(body.blocks.size(), kNoId);
	for (uint32_t b = 0; b < body.layout.size(); ++b)
	{
		if (plan.analysis.eh_live_[b] == 0)
			continue;
		const BlockId block = body.layout[b];
		placed.push_back(block);
		auto add = [&](BlockId to) {
			if (seen[to] != block)
			{
				seen[to] = block;
				edges.emplace_back(block, to);
			}
		};
		const auto& term = body.instructions[body.blocks[block].last];
		if (term.op != Opcode::Jump || plan.analysis.eh_sites_[body.blocks[block].last] == 0)
			for (uint32_t k = plan.succ_first[b]; k < plan.succ_first[b + 1]; ++k)
				add(body.layout[plan.succs[k]]);
		for (InstId i = body.blocks[block].first; i != kNoId; i = body.instructions[i].next)
		{
			const auto& inst = body.instructions[i];
			const uint32_t site = plan.analysis.eh_sites_[i];
			if ((inst.op == Opcode::Call || inst.op == Opcode::Throw) && site != 0)
				add(plan.landing_blocks[site - 1]);
			if ((inst.op == Opcode::Resume || inst.op == Opcode::Jump) && site != 0 &&
				site != machine::ExceptionAnalysis::kResumeTerminal)
				add(body.layout[site - 1]);
		}
	}
	opt::build_cfg_edges(body, placed, edges, plan.control);
}

} // namespace

void prepare_exception_plan(const Program& program, SymbolId symbol, Object& object, std::vector<uint32_t>& symbols,
	uint32_t group, ExceptionPlan& plan)
{
	const FunctionBody& body = *program.functions[program.symbols[symbol].detail].body;
	for (BlockId block : body.layout)
		for (InstId i = body.blocks[block].first; i != kNoId; i = body.instructions[i].next)
		{
			const auto op = body.instructions[i].op;
			if (program.exception_abi == ExceptionAbi::Opaque && (op == Opcode::EhCatch || op == Opcode::EhFilter))
				throw WasmError("typed catches require the C++ exception ABI");
			if (program.exception_abi == ExceptionAbi::Cxx && op == Opcode::Throw)
				throw WasmError("opaque throw requires the opaque exception ABI");
			plan.active = plan.active || opt::is_eh_instruction(op);
		}
	if (!plan.active)
		return;
	opt::Cfg cfg;
	opt::build_cfg_light(program, body, cfg);
	auto& layout_of = plan.layout_of;
	layout_of.assign(body.blocks.size(), kNoId);
	for (uint32_t b = 0; b < body.layout.size(); ++b)
		layout_of[body.layout[b]] = b;
	auto& succ_first = plan.succ_first;
	auto& succs = plan.succs;
	auto& handler_first = plan.handler_first;
	auto& handlers = plan.handlers;
	auto& pred_first = plan.pred_first;
	auto& preds = plan.preds;
	for (BlockId b : body.layout)
	{
		plan.analysis.blocks_.push_back(
			machine::ExceptionBlock{ b, cfg.reachable(b), (cfg.flags[b] & opt::BF_HANDLER) != 0 });
		succ_first.push_back(static_cast<uint32_t>(succs.size()));
		handler_first.push_back(static_cast<uint32_t>(handlers.size()));
		pred_first.push_back(static_cast<uint32_t>(preds.size()));
		for (uint32_t k = cfg.succ_first[b]; k < cfg.succ_first[b + 1]; ++k)
			succs.push_back(layout_of[cfg.succs[k]]);
		for (uint32_t k = cfg.handler_first[b]; k < cfg.handler_first[b + 1]; ++k)
			handlers.push_back(layout_of[cfg.handlers[k]]);
		for (uint32_t k = cfg.pred_first[b]; k < cfg.pred_first[b + 1]; ++k)
			preds.push_back(layout_of[cfg.preds[k]]);
	}
	succ_first.push_back(static_cast<uint32_t>(succs.size()));
	handler_first.push_back(static_cast<uint32_t>(handlers.size()));
	pred_first.push_back(static_cast<uint32_t>(preds.size()));
	const machine::ExceptionGraph graph{ &layout_of, &succ_first, &succs, &pred_first, &preds, &handler_first,
		&handlers };
	try
	{
		plan.analysis.analyze(program, symbol, body, graph, plan.tables, true);
	}
	catch (const machine::ExceptionError& error)
	{
		throw WasmError(error.what());
	}
	plan.entry_selectors.assign(body.blocks.size(), 0);
	plan.landing_blocks.assign(plan.tables.landings.size(), kNoId);
	for (uint32_t b = 0; b < body.layout.size(); ++b)
	{
		const BlockId block = body.layout[b];
		plan.entry_selectors[block] = machine::exception_entry_selector(body, block, plan.analysis.blocks_[b].handler);
		const uint32_t landing = plan.analysis.eh_landing_index_[b];
		if (landing != kNoId)
			plan.landing_blocks[landing] = block;
	}
	control_edges(body, plan);
	if (plan.tables.landings.empty() || program.exception_abi != ExceptionAbi::Cxx)
		return;
	ObjectData data;
	data.alignment = 2;
	data.name = ".gcc_except_table." + std::to_string(symbol);
	lsda(plan.tables, data);
	ObjectSymbol table;
	table.kind = 1;
	table.flags = 2;
	table.name = data.name;
	table.index = static_cast<uint32_t>(object.data.size());
	table.size = static_cast<uint32_t>(data.contents.bytes.data.size());
	if (group != kNoId)
		object.groups[group].members.push_back({ 0, table.index });
	plan.lsda_symbol = static_cast<uint32_t>(symbols.size());
	symbols.push_back(static_cast<uint32_t>(object.symbols.size()));
	object.symbols.push_back(std::move(table));
	object.data.push_back(std::move(data));
}

} // namespace wasm
