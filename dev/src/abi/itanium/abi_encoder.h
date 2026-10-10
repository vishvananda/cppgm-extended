#pragma once

// Public ABI naming API: typed targets over the canonical ABI graph and the
// one Itanium encoder (Itanium C++ ABI 5.1) shared by the PA9 `abimangle`
// tool and the compiler stages that emit symbols. Producers build graph
// nodes and a target directly; nothing is serialized to fact text.
//
// Replaces the course's scaffold abi/itanium/abi_mangle.h, which ships to
// students unchanged. The ABI scaffold headers define some of these names
// differently, so v4opt includes none of them (scripts/check_scaffold_headers.sh).

#include "abi/itanium/abi_graph.h"
#include "abi/itanium/abi_stats.h"

#include <string>
#include <vector>

namespace abi_mangle {

enum AbiTargetKind : std::uint8_t
{
	ABI_TARGET_TYPE,                // node = type; the bare <type> encoding
	ABI_TARGET_ENTITY,              // node = function encoding or data name: _Z <encoding>
	ABI_TARGET_C_NAME,              // node = STRING: an unmangled C-linkage name
	ABI_TARGET_TYPEINFO,            // node = type: _ZTI
	ABI_TARGET_TYPEINFO_NAME,       // node = type: _ZTS
	ABI_TARGET_VTABLE,              // node = type: _ZTV
	ABI_TARGET_VTT,                 // node = type: _ZTT
	ABI_TARGET_CONSTRUCTION_VTABLE, // node = derived type, base = base type, offset: _ZTC
	ABI_TARGET_TLS_WRAPPER,         // node = variable name: _ZTW
	ABI_TARGET_THUNK,               // node = function encoding; call offsets: _ZT
	ABI_TARGET_TLS_INIT,            // node = variable name: _ZTH
	ABI_TARGET_GUARD_VARIABLE       // node = variable name (a local one's too): _ZGV
};

// A <call-offset> (5.1.4.2): a fixed adjustment, or a fixed adjustment to a
// virtual base followed by a vcall offset.
struct AbiCallOffset
{
	bool is_virtual = false;
	long long offset = 0;
	long long vcall_offset = 0;
};

struct AbiTarget
{
	AbiTargetKind kind = ABI_TARGET_TYPE;
	AbiNode node = ABI_NONE;
	AbiNode base = ABI_NONE;
	long long offset = 0;
	AbiCallOffset this_adjustment;
	bool has_result_adjustment = false;
	AbiCallOffset result_adjustment;
};

// Encodes targets against one graph. The substitution dictionary is
// per-name state, shared by the encodings nested in the name (local-name
// contexts and `L _Z <encoding> E` entity references); its storage is reused
// across names, indexed by node id so lookups are O(1).
//
// A name of internal linkage is `L <source-name>` wherever it appears,
// a variable's symbol as a function's (`_ZL1v`, `_ZN2nsL1vE`, `_ZL1fv`):
// Clang's and GCC's marker (5.1.2 leaves internal names to the
// implementation).
class AbiEncoder
{
public:
	explicit AbiEncoder(const AbiGraph& graph, AbiMangleStats* stats = nullptr);
	AbiEncoder(const AbiEncoder&) = delete;
	AbiEncoder& operator=(const AbiEncoder&) = delete;

	// Appends the mangled name of `target` to `out`. Throws
	// EncodingError (abi_encoding_error.h) for a target the grammar cannot
	// encode.
	void encode(const AbiTarget& target, std::string& out);

private:
	void put(const char* text) { out_->append(text); }
	void put(char c) { out_->push_back(c); }
	void put_number(unsigned long long value);
	void put_signed(long long value);
	void put_source_name(AbiNode string);
	void put_tags(AbiNode tagged);
	void put_discriminator(std::uint32_t discriminator_plus_one);

	bool lookup(AbiNode node, std::uint32_t* index) const;
	bool substitute(AbiNode node);
	void record(AbiNode node);

	void check_stack() const;
	void check_no_qualifiers(unsigned qualifiers) const;
	void encode_type(AbiNode type);
	void encode_type_node(AbiNode type);
	void encode_qualifiers(unsigned qualifiers);
	void encode_function_type(AbiNode type);
	void encode_name(AbiNode name, unsigned qualifiers);
	void encode_nested_name(AbiNode name, unsigned qualifiers);
	void encode_local_name(AbiNode name, AbiNode local, unsigned qualifiers);
	bool chain_has_known_prefix(AbiNode name) const;
	void encode_prefix(AbiNode scope);
	void encode_prefix_root(AbiNode scope);
	void encode_unqualified(AbiNode name);
	void encode_abbreviation(AbiNode abbreviation);
	void encode_template_args(AbiNode template_id);
	void encode_template_arg(AbiNode arg);
	void encode_template_param_decl(AbiNode declaration);
	void encode_literal(AbiNode literal);
	void encode_template_param(const AbiNodeData& param);
	void encode_expression(AbiNode expression);
	void encode_unresolved_prefix(AbiNode prefix);
	void encode_encoding(AbiNode entity);
	void encode_bare_function_type(AbiNode encoding);
	void encode_entity_reference(AbiNode entity);
	void encode_context(AbiNode function_scope);
	void encode_call_offset(const AbiCallOffset& offset);
	void encode_special(const AbiTarget& target);

	const AbiGraph& graph_;
	AbiMangleStats* stats_;
	std::string* out_;
	std::vector<AbiNode> entries_;
	// Per node: dictionary index + 1, valid only while entries_ agrees.
	std::vector<std::uint32_t> slots_;
	// Prefix-code chain links awaiting their record, innermost last.
	std::vector<AbiNode> chain_;
	std::uintptr_t stack_base_;
	std::uintptr_t stack_budget_;
};

} // namespace abi_mangle
