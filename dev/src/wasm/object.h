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

struct Object
{
	std::string path;
	bool archive_member = false;
	std::vector<Signature> types;
	std::vector<ObjectFunction> functions;
	std::vector<ObjectData> data;
	std::vector<ObjectSymbol> symbols;
	std::vector<std::pair<uint32_t, uint32_t>> initializers;
	std::vector<ObjectGroup> groups;
	std::vector<std::pair<char, std::string>> features;
	std::vector<ObjectGlobal> global_imports;
};

bool relocation_addend(RelocationKind kind);
void make_object(lowir::Program& program, const backend::Options& options, Object& object, std::string& dump);
void write_object(const Object& object, Bytes& output);
void read_object(const std::string& path, Object& object);
void read_link_inputs(const std::string& path, std::vector<Object>& objects);
void link_objects(const std::vector<Object>& objects, const backend::Options& options, Bytes& output);

} // namespace wasm
