#pragma once

#include "wasm/module.h"

namespace wasm {

struct ObjectSymbol
{
	uint8_t kind = 0;
	uint32_t flags = 0;
	std::string name;
	uint32_t index = 0;
	uint32_t offset = 0;
	uint32_t size = 0;
	bool undefined() const { return (flags & 0x10) != 0; }
	bool local() const { return (flags & 3) == 2; }
	bool weak() const { return (flags & 3) == 1; }
};

struct ObjectChunk
{
	Bytes bytes;
	std::vector<Relocation> relocations;
	uint32_t section_offset = 0;
};

struct ObjectFunction
{
	uint32_t type = 0;
	std::string module;
	std::string name;
	ObjectChunk code;
	bool imported = false;
};

struct ObjectData
{
	std::string name;
	uint32_t alignment = 0;
	uint32_t flags = 0;
	ObjectChunk contents;
};

struct ObjectGroup
{
	std::string name;
	std::vector<std::pair<uint8_t, uint32_t>> members;
};

struct ObjectGlobal
{
	std::string module;
	std::string name;
	ValueType type;
	bool mutable_value;
};

struct ObjectGlobalDefinition
{
	ValueType type;
	bool mutable_value;
	ObjectChunk initializer;
};

struct ObjectElement
{
	ValueType type;
	bool declaration;
	std::vector<ObjectChunk> values;
};

struct ObjectExport
{
	std::string name;
	uint32_t index;
	uint8_t kind;
};

struct ObjectTag
{
	uint32_t type = 0;
	std::string module;
	std::string name;
	bool imported = false;
};

struct Object
{
	std::string path;
	bool archive_member = false;
	std::vector<Signature> types;
	std::vector<ObjectFunction> functions;
	std::vector<ObjectTag> tags;
	std::vector<ObjectData> data;
	std::vector<ObjectSymbol> symbols;
	std::vector<std::pair<uint32_t, uint32_t>> initializers;
	std::vector<ObjectGroup> groups;
	std::vector<std::pair<char, std::string>> features;
	std::vector<ObjectGlobal> global_imports;
	std::vector<ObjectGlobalDefinition> global_definitions;
	std::vector<uint8_t> declared_functions;
	std::vector<ObjectElement> elements;
	uint32_t table_count = 0;
	uint32_t memory_count = 0;
	std::vector<ObjectExport> exports;
};

// Input identities are retained once per file, never rediscovered from member
// diagnostic paths. Explicit duplicate objects still reach symbol resolution.
struct LinkInputs
{
	std::vector<Object> objects;
	std::vector<std::pair<uint64_t, uint64_t>> files;
};

bool relocation_addend(RelocationKind kind);
ValueType global_type(const Object& object, uint32_t index);
bool mutable_global(const Object& object, uint32_t index);
void prepare_finalizers(lowir::Program& program);
void adapt_wasi_entry(Object& object, uint32_t symbol);
void add_runtime_inputs(LinkInputs& inputs, const backend::Options& options);
void make_object(lowir::Program& program, const backend::Options& options, Object& object, std::string& dump);
void write_object(const Object& object, Bytes& output);
void read_object(const std::string& path, Object& object);
void read_link_inputs(const std::string& path, LinkInputs& inputs, bool once = false);
void link_objects(const std::vector<Object>& objects, const backend::Options& options, Bytes& output);

} // namespace wasm
