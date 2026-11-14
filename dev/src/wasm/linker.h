#pragma once

#include <map>
#include <set>
#include "wasm/object.h"

namespace wasm {

using ObjectRef = std::pair<uint32_t, uint32_t>;
const uint32_t kAbsent = UINT32_MAX;

class Linker
{
public:
	Linker(const std::vector<Object>& objects, const backend::Options& options) : objects_(objects), options_(options)
	{}
	void run(Bytes& output);

private:
	void resolve();
	void activate(uint32_t object);
	void resolve_archive_references();
	void validate_symbols() const;
	ObjectRef lookup(const std::string& name);
	void collect();
	void visit(ObjectRef symbol);
	void dependencies(uint32_t object, const ObjectChunk& chunk);
	void require_function_type(ObjectRef symbol);
	ObjectRef selected(ObjectRef ref) const;
	bool host_import(ObjectRef ref) const;
	const ObjectSymbol& symbol(ObjectRef ref) const { return objects_[ref.first].symbols[ref.second]; }
	uint32_t function_index(ObjectRef ref) const;
	ObjectRef indexed_symbol(uint32_t object, uint8_t kind, uint32_t index) const;
	uint32_t symbol_value(ObjectRef ref) const;
	bool synthetic_data(const std::string& name, uint32_t& value) const;
	Bytes relocated(uint32_t object, const ObjectChunk& chunk, bool constant = false) const;
	void emit(Bytes& output);
	void data_section(Bytes& output) const;
	void prepare_address_globals();
	void prepare_globals();
	bool runtime_global(ObjectRef ref, uint32_t& index) const;
	void global_imports(Bytes& output) const;
	void global_section(Bytes& output) const;
	void tag_imports(Bytes& output) const;
	void tag_section(Bytes& output) const;
	const std::vector<Object>& objects_;
	const backend::Options& options_;
	std::map<std::string, ObjectRef> definitions_, archive_index_;
	std::set<std::string> groups_;
	std::vector<bool> active_;
	std::vector<std::vector<uint8_t>> typed_function_uses_;
	std::vector<uint32_t> pending_objects_;
	size_t next_pending_ = 0;
	std::vector<ObjectRef> roots_;
	std::set<ObjectRef> discarded_functions_, discarded_data_, discarded_tags_;
	std::set<ObjectRef> discarded_globals_;
	std::set<ObjectRef> weak_functions_;
	std::map<ObjectRef, uint32_t> functions_, data_, tags_, globals_;
	std::map<ObjectRef, uint32_t> address_globals_;
	std::vector<ObjectRef> ordered_functions_, ordered_data_, ordered_tags_;
	std::vector<ObjectRef> ordered_globals_, imported_globals_;
	std::vector<std::pair<uint32_t, ObjectRef>> initializers_;
	std::vector<Signature> types_;
	std::vector<std::vector<uint32_t>> type_indices_;
	uint32_t imported_ = 0;
	uint32_t imported_tags_ = 0;
	uint32_t ctor_index_ = kAbsent;
	bool needs_ctors_ = false;
	uint32_t stack_floor_ = 0, stack_top_ = 0;
};

} // namespace wasm
