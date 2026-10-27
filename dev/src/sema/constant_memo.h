#pragma once

#include "sema/constant_values.h"
#include "support/flat_map.h"
#include "support/memory/arena.h"
#include "support/pointer_map.h"

namespace syntax {
struct Expr;
}

namespace sema {

// One evaluation's successful integer calls. Arguments have already been
// converted and bound; no reference, object or caller storage enters a key.
// The separate arena survives the interpreter's per-call scratch rewinds.
struct ConstantCallKey
{
	const FunctionEntity* function;
	const uint64_t* arguments;
	uint32_t count;
	bool operator==(const ConstantCallKey& other) const;
};

struct ConstantCallHash
{
	size_t operator()(const ConstantCallKey& key) const;
};

struct ConstantCallResult
{
	ConstValue value;
	uint64_t steps;
	size_t depth;
};

class ConstantMemo
{
public:
	static bool scalar_signature(const FunctionEntity* function);
	bool eligible(const FunctionEntity* function);
	bool remember(const ConstantCallKey& key, const ConstantCallResult& result);
	FlatMap<ConstantCallKey, ConstantCallResult, ConstantCallHash> calls;

private:
	bool expression(const syntax::Expr* expr, const FunctionEntity* function, uint32_t depth);
	bool function_body(const FunctionEntity* function);
	Arena arena_;
	uint32_t argument_words_ = 0;
	PointerMap purity_;
};

} // namespace sema
