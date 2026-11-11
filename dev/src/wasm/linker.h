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
	void validate_symbols() const;
	ObjectRef lookup(const std::string& name);
	void collect();
	void visit(ObjectRef symbol);
	void dependencies(uint32_t object, const ObjectChunk& chunk);
	ObjectRef selected(ObjectRef ref) const;
	bool host_import(ObjectRef ref) const;
	const ObjectSymbol& symbol(ObjectRef ref) const { return objects_[ref.first].symbols[ref.second]; }
	uint32_t function_index(ObjectRef ref) const;
	uint32_t symbol_value(ObjectRef ref) const;
	Bytes relocated(uint32_t object, const ObjectChunk& chunk) const;
	void emit(Bytes& output);
	const std::vector<Object>& objects_;
	const backend::Options& options_;
	std::map<std::string, ObjectRef> definitions_, archive_index_;
	std::set<std::string> groups_;
	std::vector<bool> active_;
	std::vector<ObjectRef> roots_;
	std::set<ObjectRef> discarded_functions_, discarded_data_;
	std::map<ObjectRef, uint32_t> functions_, data_;
	std::vector<ObjectRef> ordered_functions_, ordered_data_;
	std::vector<std::pair<uint32_t, ObjectRef>> initializers_;
	std::vector<Signature> types_;
	std::vector<std::vector<uint32_t>> type_indices_;
	uint32_t imported_ = 0;
	uint32_t stack_floor_ = 0, stack_top_ = 0;
};

} // namespace wasm
