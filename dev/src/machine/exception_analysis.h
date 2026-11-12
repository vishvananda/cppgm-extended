#pragma once

#include <stdexcept>
#include "lowir/program.h"
#include "machine/exception_tables.h"
#include "support/flat_map.h"

namespace machine {

class ExceptionError : public std::runtime_error
{
public:
	explicit ExceptionError(const std::string& message) : std::runtime_error(message) {}
};

// Indices in this graph are positions in the function's layout. The caller
// supplies its existing edges; exception analysis never builds a second CFG.
struct ExceptionGraph
{
	const std::vector<uint32_t>* layout_of;
	const std::vector<uint32_t>* succ_first;
	const std::vector<uint32_t>* succs;
	const std::vector<uint32_t>* preds_first;
	const std::vector<uint32_t>* preds;
	const std::vector<uint32_t>* handler_first;
	const std::vector<uint32_t>* handlers;
};

struct ExceptionBlock
{
	uint32_t id;
	bool reachable;
	bool handler;
};

int64_t exception_entry_selector(const lowir::FunctionBody& body, lowir::BlockId block, bool handler);

class ExceptionAnalysis
{
public:
	static constexpr uint32_t kResumeTerminal = lowir::kNoId;
	void reset(uint32_t blocks, uint32_t instructions);
	void analyze(const lowir::Program& program, lowir::SymbolId symbol, const lowir::FunctionBody& body,
		const ExceptionGraph& graph, ExceptionTables& tables);
	std::vector<ExceptionBlock> blocks_;
	uint64_t eh_walks_ = 0, eh_states_ = 0;
	std::vector<uint32_t> eh_sites_;
	std::vector<uint32_t> eh_landing_index_;
	std::vector<uint8_t> eh_translate_; // per landing: whether its pad translates
	std::vector<uint8_t> eh_resumed_;   // per landing block: a resume of the frame enters it
	std::vector<uint8_t> eh_live_;      // per block: 0 never entered, 1 reached, 2 a call's landing
	bool eh_raw_ = false;               // the pads keep the raw filter
	bool eh_selects(uint32_t block, uint32_t index) const;

private:
	struct ExceptionWordHash
	{
		size_t operator()(uint64_t key) const { return static_cast<size_t>(key); }
		size_t operator()(uint32_t key) const { return key; }
	};
	const lowir::Program* program_ = nullptr;
	const lowir::FunctionBody* body_ = nullptr;
	lowir::SymbolId symbol_ = lowir::kNoId;
	ExceptionGraph graph_{};
	ExceptionTables* tables_ = nullptr;
	std::vector<uint32_t> block_insts_;
	[[noreturn]] void eh_fail(const char* what) const;
	void eh_walk(uint32_t block, bool record);
	void eh_enter(uint32_t block, uint32_t state, uint64_t mask, std::vector<uint32_t>& work);
	uint64_t eh_landing_mask(uint32_t block);
	int64_t eh_entry_selector(uint32_t block) const;
	uint64_t eh_edge_mask(uint32_t target, uint64_t mask) const;
	void eh_refine(const lowir::Instruction& branch, uint32_t selector, uint64_t& yes, uint64_t& no) const;
	uint32_t eh_node(uint32_t parent, uint32_t landing, bool cleanup, bool active);
	uint32_t eh_tail(uint32_t base, uint32_t lo, uint32_t hi, uint8_t top);
	uint32_t eh_merge(uint32_t a, uint32_t b);
	bool eh_resume_only(uint32_t block) const;
	void eh_order();
	void eh_prune(uint32_t& landings, bool& reads);

	void eh_build_tables(uint32_t landings);
	void eh_choose_indexes();
	uint32_t eh_clause_index(const lowir::Instruction& inst);
	int32_t eh_filter_value(lowir::InstId id, const lowir::Instruction& inst);
	void eh_landing_clauses(uint32_t block, std::vector<int32_t>& filters, bool& catch_all);
	void eh_build_chain(uint32_t node);
	bool eh_unmatched_work(uint32_t block);
	void eh_build_landing(uint32_t block, uint32_t index);
	struct EhNode
	{
		uint32_t parent;
		uint32_t landing; // layout block (kNoId: the empty stack, a tail)
		uint32_t depth;
		bool cleanup;
		bool active;
		bool inside;     // above an active record: opened by a running landing
		uint8_t tail_lo; // a tail: tail_lo..tail_hi active records of the
		uint8_t tail_hi; // frame above the parent, the top one's kind
		uint8_t tail_top;
		uint32_t protect; // the innermost inactive node at or below
		uint32_t chain;   // inactive nodes: first of its chain in eh_chains_
		uint32_t chain_count;
	};
	std::vector<EhNode> eh_nodes_;
	FlatMap<uint64_t, uint32_t, ExceptionWordHash> eh_node_index_;
	std::vector<uint32_t> eh_in_;
	std::vector<uint64_t> eh_mask_in_; // per block: the selectors its dispatch can see
	std::vector<uint32_t> eh_landing_node_;
	std::vector<uint8_t> eh_chained_;                              // per landing block: its clauses are in some chain
	bool eh_mid_clauses_ = false;                                  // some catch-all follows a landing's leading clauses
	bool eh_identity_ = false;                                     // clause selectors are the type indexes
	FlatMap<uint32_t, uint32_t, ExceptionWordHash> eh_type_index_; // else: type -> index
	std::vector<std::pair<uint32_t, int32_t>> eh_filter_values_;   // filter clause -> its (negative) filter
	std::vector<int32_t> eh_chains_;
	std::vector<uint8_t> eh_work_;       // per block: the no-match walk's memo
	std::vector<uint32_t> eh_seen_;      // type table: per selector its type
	std::vector<uint32_t> eh_stamps_;    // chain construction: per selector its last chain
	std::vector<uint32_t> eh_work_list_; // scratch: a block worklist or stack
	std::vector<uint64_t> eh_rank_;      // per block: its walking priority (eh_order)
	std::vector<uint64_t> eh_heap_;      // scratch: the states' worklist by priority
	std::vector<uint8_t> eh_listed_;     // per block: on the worklist (seen, while ordering)
	std::vector<uint32_t> eh_dfs_next_;  // per block: the ordering walk's next edge
	std::vector<uint32_t> eh_pending_;   // scratch: a chain's unbuilt records
	std::vector<int32_t> eh_own_;        // scratch: a landing's clause filters
	uint32_t eh_stamp_ = 0;
};

} // namespace machine
