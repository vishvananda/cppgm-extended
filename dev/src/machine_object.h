#pragma once

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

namespace machine_object {

struct Symbol
{
  enum Binding
  {
    SB_GLOBAL,
    SB_LOCAL,
    SB_WEAK
  } binding = SB_GLOBAL;

  enum Section
  {
    SS_CODE,
    SS_DATA,
    SS_EXTRA,
    SS_UNDEFINED
  } section = SS_CODE;

  std::string name;
  std::size_t offset = 0;
  std::size_t size = 0;
  bool hidden_code_label = false;
  std::string comdat_group;
  std::string extra_section;
};

struct Relocation
{
  enum Kind
  {
    RK_BRANCH32,
    RK_PCREL32,
    RK_INDIRECT_REL32,
    RK_TLV_REL32,
    RK_TPOFF32,
    RK_ABS64
  } kind = RK_BRANCH32;

  Symbol::Section section = Symbol::SS_CODE;
  std::size_t offset = 0;
  std::string symbol;
  long long addend = 0;
};

struct ExtraRelocation
{
  enum Kind
  {
    RK_BRANCH32,
    RK_INDIRECT_REL32,
    RK_PCREL32,
    RK_ABS64
  } kind = RK_ABS64;

  enum TargetKind
  {
    TK_SYMBOL,
    TK_CODE,
    TK_DATA,
    TK_EXTRA
  } target_kind = TK_SYMBOL;

  std::size_t offset = 0;
  std::string symbol;
  std::string target_extra_section;
  long long addend = 0;
};

struct ExtraSection
{
  std::string segment_name;
  std::string section_name;
  uint32_t macho_flags = 0;
  uint32_t macho_align_pow2 = 0;
  std::vector<unsigned char> bytes;
  std::vector<ExtraRelocation> relocations;
};

struct ObjectFile
{
  std::string target;
  std::vector<unsigned char> code;
  std::vector<unsigned char> data;
  std::vector<ExtraSection> extra_sections;
  std::vector<Symbol> symbols;
  std::vector<Relocation> relocations;
};

void write_object_file(const std::string & path, const ObjectFile & object);
ObjectFile parse_object_file(const std::string & path);

}  // namespace machine_object
