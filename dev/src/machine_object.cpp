#include "machine_object.h"

#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <cstdio>
#include <fstream>
#include <limits>
#include <map>
#include <sstream>
#include <stdexcept>
#include <vector>

using namespace std;

#include "symbol_linkage.h"

namespace machine_object {

namespace {

const uint32_t MH_MAGIC_64 = 0xFEEDFACF;
const uint32_t CPU_TYPE_X86_64 = 0x01000007;
const uint32_t CPU_SUBTYPE_X86_64_ALL = 3;
const uint32_t MH_OBJECT = 1;
const uint32_t MH_SUBSECTIONS_VIA_SYMBOLS = 0x00002000;

const uint32_t LC_SEGMENT_64 = 0x19;
const uint32_t LC_SYMTAB = 0x2;
const uint32_t LC_DYSYMTAB = 0xB;
const uint32_t LC_BUILD_VERSION = 0x32;
const uint32_t PLATFORM_MACOS = 1;

const uint32_t VM_PROT_READ = 1;
const uint32_t VM_PROT_WRITE = 2;
const uint32_t VM_PROT_EXECUTE = 4;

const uint32_t S_REGULAR = 0x0;
const uint32_t S_ZEROFILL = 0x1;
const uint32_t S_MOD_INIT_FUNC_POINTERS = 0x9;
const uint32_t S_MOD_TERM_FUNC_POINTERS = 0xA;
const uint32_t S_GB_ZEROFILL = 0xC;
const uint32_t S_THREAD_LOCAL_ZEROFILL = 0x12;
const uint32_t SECTION_TYPE = 0xFF;
const uint32_t S_ATTR_SOME_INSTRUCTIONS = 0x00000400;
const uint32_t S_ATTR_PURE_INSTRUCTIONS = 0x80000000;

const uint8_t N_EXT = 0x01;
const uint8_t N_TYPE = 0x0E;
const uint8_t N_UNDF = 0x00;
const uint8_t N_SECT = 0x0E;
const uint8_t NO_SECT = 0;
const uint16_t N_WEAK_REF = 0x0040;
const uint16_t N_WEAK_DEF = 0x0080;

const uint32_t X86_64_RELOC_UNSIGNED = 0;
const uint32_t X86_64_RELOC_SIGNED = 1;
const uint32_t X86_64_RELOC_BRANCH = 2;
const uint32_t X86_64_RELOC_GOT_LOAD = 3;
const uint32_t X86_64_RELOC_GOT = 4;
const uint32_t X86_64_RELOC_TLV = 9;

const unsigned char ELFMAG0 = 0x7F;
const unsigned char ELFMAG1 = 'E';
const unsigned char ELFMAG2 = 'L';
const unsigned char ELFMAG3 = 'F';
const unsigned char ELFCLASS64 = 2;
const unsigned char ELFDATA2LSB = 1;
const unsigned char EV_CURRENT = 1;
const unsigned char ELFOSABI_SYSV = 0;

const uint16_t ET_REL = 1;
const uint16_t EM_X86_64 = 62;

const uint32_t SHT_NULL = 0;
const uint32_t SHT_PROGBITS = 1;
const uint32_t SHT_SYMTAB = 2;
const uint32_t SHT_STRTAB = 3;
const uint32_t SHT_RELA = 4;
const uint32_t SHT_NOBITS = 8;
const uint32_t SHT_INIT_ARRAY = 14;
const uint32_t SHT_FINI_ARRAY = 15;
const uint32_t SHT_GROUP = 17;
const uint32_t SHT_X86_64_UNWIND = 0x70000001U;

const uint64_t SHF_WRITE = 0x1;
const uint64_t SHF_ALLOC = 0x2;
const uint64_t SHF_EXECINSTR = 0x4;
const uint64_t SHF_GROUP = 0x200;
const uint64_t SHF_TLS = 0x400;

const uint32_t GRP_COMDAT = 0x1;

const uint8_t STB_LOCAL = 0;
const uint8_t STB_GLOBAL = 1;
const uint8_t STB_WEAK = 2;
const uint8_t STT_NOTYPE = 0;
const uint8_t STT_OBJECT = 1;
const uint8_t STT_FUNC = 2;
const uint8_t STT_SECTION = 3;
const uint8_t STT_TLS = 6;
const uint16_t SHN_UNDEF = 0;

const uint32_t R_X86_64_64 = 1;
const uint32_t R_X86_64_PC32 = 2;
const uint32_t R_X86_64_PLT32 = 4;
const uint32_t R_X86_64_GOTPCREL = 9;
const uint32_t R_X86_64_TPOFF32 = 23;

const char * kCppgmSymbolPrefix = "__cppgm_";

const char * kWriteLocalSymbolMapEnv = "CPPGM_WRITE_LOCAL_SYMBOL_MAP";

struct MachOSectionInfo
{
  uint8_t index = 0;
  string sectname;
  string segname;
  Symbol::Section section = Symbol::SS_CODE;
  bool is_extra = false;
  uint64_t addr = 0;
  uint64_t size = 0;
  uint32_t offset = 0;
  uint32_t reloff = 0;
  uint32_t nreloc = 0;
  uint32_t flags = 0;
  uint32_t align_pow2 = 0;
};

bool macho_section_is_zerofill(uint32_t flags)
{
  const uint32_t type = flags & SECTION_TYPE;
  return type == S_ZEROFILL ||
         type == S_GB_ZEROFILL ||
         type == S_THREAD_LOCAL_ZEROFILL;
}

struct MachOSymbolInfo
{
  string name;
  uint8_t type = 0;
  uint8_t sect = 0;
  uint16_t desc = 0;
  uint64_t value = 0;
};

struct ElfSectionInfo
{
  uint32_t index = 0;
  string name;
  uint32_t type = SHT_NULL;
  uint64_t flags = 0;
  uint64_t offset = 0;
  uint64_t size = 0;
  uint32_t link = 0;
  uint32_t info = 0;
  uint64_t addralign = 0;
  uint64_t entsize = 0;
};

struct ElfSymbolInfo
{
  string name;
  uint8_t bind = STB_LOCAL;
  uint8_t type = STT_NOTYPE;
  uint16_t shndx = SHN_UNDEF;
  uint64_t value = 0;
  uint64_t size = 0;
};

unsigned parse_hex_byte(const string & text)
{
  if(text.size() != 2) {
    throw logic_error("invalid byte token " + text);
  }
  unsigned value = 0;
  for(size_t i = 0; i < 2; ++i) {
    const char c = text[i];
    value <<= 4;
    if(c >= '0' && c <= '9') value |= static_cast<unsigned>(c - '0');
    else if(c >= 'a' && c <= 'f') value |= static_cast<unsigned>(10 + c - 'a');
    else if(c >= 'A' && c <= 'F') value |= static_cast<unsigned>(10 + c - 'A');
    else throw logic_error("invalid hex digit in " + text);
  }
  return value;
}

string macho_extra_section_key(const string & segname, const string & sectname)
{
  return segname + "," + sectname;
}

size_t align_up(size_t value, size_t alignment)
{
  if(alignment == 0) {
    return value;
  }
  const size_t mask = alignment - 1;
  return (value + mask) & ~mask;
}

void append_u32(vector<unsigned char> & out, uint32_t value)
{
  for(size_t i = 0; i < 4; ++i) {
    out.push_back(static_cast<unsigned char>((value >> (8 * i)) & 0xFF));
  }
}

void overwrite_i32_le(vector<unsigned char> & out, size_t offset, int32_t value)
{
  if(offset + 4 > out.size()) {
    throw logic_error("invalid 32-bit relocation patch offset");
  }
  const uint32_t raw = static_cast<uint32_t>(value);
  for(size_t i = 0; i < 4; ++i) {
    out[offset + i] = static_cast<unsigned char>((raw >> (8 * i)) & 0xFF);
  }
}

void append_u16(vector<unsigned char> & out, uint16_t value)
{
  out.push_back(static_cast<unsigned char>(value & 0xFF));
  out.push_back(static_cast<unsigned char>((value >> 8) & 0xFF));
}

void append_u64(vector<unsigned char> & out, uint64_t value)
{
  for(size_t i = 0; i < 8; ++i) {
    out.push_back(static_cast<unsigned char>((value >> (8 * i)) & 0xFF));
  }
}

void append_fixed_string(vector<unsigned char> & out,
                         const string & text,
                         size_t width)
{
  for(size_t i = 0; i < width; ++i) {
    out.push_back(i < text.size() ? static_cast<unsigned char>(text[i]) : 0);
  }
}

uint32_t read_u32(const vector<unsigned char> & bytes, size_t offset)
{
  if(offset + 4 > bytes.size()) {
    throw logic_error("truncated object file");
  }
  return static_cast<uint32_t>(bytes[offset + 0]) |
      (static_cast<uint32_t>(bytes[offset + 1]) << 8) |
      (static_cast<uint32_t>(bytes[offset + 2]) << 16) |
      (static_cast<uint32_t>(bytes[offset + 3]) << 24);
}

uint16_t read_u16(const vector<unsigned char> & bytes, size_t offset)
{
  if(offset + 2 > bytes.size()) {
    throw logic_error("truncated object file");
  }
  return static_cast<uint16_t>(bytes[offset + 0]) |
      static_cast<uint16_t>(bytes[offset + 1] << 8);
}

uint64_t read_u64(const vector<unsigned char> & bytes, size_t offset)
{
  if(offset + 8 > bytes.size()) {
    throw logic_error("truncated object file");
  }
  uint64_t value = 0;
  for(size_t i = 0; i < 8; ++i) {
    value |= static_cast<uint64_t>(bytes[offset + i]) << (8 * i);
  }
  return value;
}

int32_t read_i32(const vector<unsigned char> & bytes, size_t offset)
{
  return static_cast<int32_t>(read_u32(bytes, offset));
}

string read_c_string(const vector<unsigned char> & bytes,
                     size_t offset,
                     size_t end_offset)
{
  if(offset >= end_offset) {
    throw logic_error("invalid string table offset");
  }
  size_t end = offset;
  while(end < end_offset && bytes[end] != 0) {
    ++end;
  }
  if(end == end_offset) {
    throw logic_error("unterminated string table entry");
  }
  return string(reinterpret_cast<const char *>(&bytes[offset]), end - offset);
}

string read_padded_string(const vector<unsigned char> & bytes,
                          size_t offset,
                          size_t end_offset)
{
  if(offset > end_offset || end_offset > bytes.size()) {
    throw logic_error("invalid padded string range");
  }
  size_t end = offset;
  while(end < end_offset && bytes[end] != 0) {
    ++end;
  }
  return string(reinterpret_cast<const char *>(&bytes[offset]), end - offset);
}

string encode_symbol_name(const string & name, const string & target)
{
  if(name.empty() || name[0] != '@') {
    if(target == "macos" && !name.empty()) {
      return string("_") + name;
    }
    return name;
  }
  ostringstream out;
  out << kCppgmSymbolPrefix;
  for(size_t i = 0; i < name.size(); ++i) {
    char buffer[3];
    snprintf(buffer, sizeof(buffer), "%02x",
             static_cast<unsigned char>(name[i]));
    out << buffer;
  }
  return out.str();
}

string synthetic_local_symbol_name(size_t ordinal)
{
  char buffer[32];
  snprintf(buffer, sizeof(buffer), "Lcppgm_%zu", ordinal);
  return buffer;
}

bool should_preserve_local_symbol_name(const string & name)
{
  if(name == "@__cppgm_init" || name == "@__cppgm_fini") {
    return true;
  }
  return !name.empty() &&
         name[0] != '@' &&
         name.compare(0, 7, "Lcppgm_") != 0;
}

bool should_emit_macho_local_symbol(const Symbol & symbol)
{
  // Keep block-local code labels out of the Mach-O symbol table. With
  // MH_SUBSECTIONS_VIA_SYMBOLS enabled, ld64 can otherwise treat each one as a
  // function boundary, which fragments compact-unwind coverage for the parent
  // function.
  return !(symbol.binding == Symbol::SB_LOCAL &&
           symbol.section == Symbol::SS_CODE &&
           symbol.hidden_code_label);
}

const char * symbol_section_name(Symbol::Section section)
{
  switch(section) {
    case Symbol::SS_CODE:
      return "code";
    case Symbol::SS_DATA:
      return "data";
    case Symbol::SS_EXTRA:
      return "extra";
    case Symbol::SS_UNDEFINED:
      return "undefined";
  }
  return "unknown";
}

string describe_symbol_name_bytes(const string & name)
{
  ostringstream out;
  for(size_t i = 0; i < name.size(); ++i) {
    const unsigned char ch = static_cast<unsigned char>(name[i]);
    if(isprint(ch) && ch != '\\') {
      out << static_cast<char>(ch);
      continue;
    }
    if(ch == '\\') {
      out << "\\\\";
      continue;
    }
    char buffer[5];
    snprintf(buffer, sizeof(buffer), "\\x%02x", ch);
    out << buffer;
  }
  return out.str();
}

bool same_serialized_symbol(const Symbol & lhs, const Symbol & rhs)
{
  return lhs.binding == rhs.binding &&
         lhs.section == rhs.section &&
         lhs.name == rhs.name &&
         lhs.offset == rhs.offset &&
         lhs.size == rhs.size &&
         lhs.hidden_code_label == rhs.hidden_code_label &&
         lhs.comdat_group == rhs.comdat_group &&
         lhs.extra_section == rhs.extra_section;
}

struct OrderedSymbols
{
  vector<string> local_names;
  vector<string> global_names;
  vector<string> undefined_names;
  map<string, Symbol::Binding> undefined_bindings;
  map<string, Symbol> by_name;
};

OrderedSymbols collect_ordered_symbols(const ObjectFile & object)
{
  OrderedSymbols out;
  for(size_t i = 0; i < object.symbols.size(); ++i) {
    const Symbol & symbol = object.symbols[i];
    map<string, Symbol>::const_iterator found = out.by_name.find(symbol.name);
    if(found != out.by_name.end()) {
      if(!same_serialized_symbol(found->second, symbol)) {
        throw logic_error("conflicting duplicate symbol " +
                          describe_symbol_name_bytes(symbol.name));
      }
      continue;
    }
    out.by_name[symbol.name] = symbol;
    if(symbol.binding == Symbol::SB_LOCAL) {
      out.local_names.push_back(symbol.name);
    } else if(symbol.section == Symbol::SS_UNDEFINED) {
      out.undefined_names.push_back(symbol.name);
      out.undefined_bindings[symbol.name] = symbol.binding;
    } else {
      out.global_names.push_back(symbol.name);
    }
  }

  for(size_t i = 0; i < object.relocations.size(); ++i) {
    const string & name = object.relocations[i].symbol;
    if(out.by_name.find(name) != out.by_name.end() ||
       out.undefined_bindings.find(name) != out.undefined_bindings.end()) {
      continue;
    }
    out.undefined_names.push_back(name);
    out.undefined_bindings[name] = Symbol::SB_GLOBAL;
  }
  for(size_t si = 0; si < object.extra_sections.size(); ++si) {
    for(size_t ri = 0; ri < object.extra_sections[si].relocations.size(); ++ri) {
      const ExtraRelocation & reloc = object.extra_sections[si].relocations[ri];
      if(reloc.target_kind != ExtraRelocation::TK_SYMBOL) {
        continue;
      }
      if(out.by_name.find(reloc.symbol) != out.by_name.end() ||
         out.undefined_bindings.find(reloc.symbol) != out.undefined_bindings.end()) {
        continue;
      }
      out.undefined_names.push_back(reloc.symbol);
      out.undefined_bindings[reloc.symbol] = Symbol::SB_GLOBAL;
    }
  }
  return out;
}

bool should_write_local_symbol_map()
{
  const char * value = getenv(kWriteLocalSymbolMapEnv);
  return value != nullptr &&
         value[0] != '\0' &&
         string(value) != "0" &&
         string(value) != "false";
}

string local_symbol_map_path(const string & object_path)
{
  return object_path + ".localsymmap";
}

void write_local_symbol_map_file(const string & path, const ObjectFile & object)
{
  if(!should_write_local_symbol_map()) {
    return;
  }

  const OrderedSymbols ordered = collect_ordered_symbols(object);
  const string map_path = local_symbol_map_path(path);
  ofstream out(map_path.c_str());
  if(!out) {
    throw logic_error("unable to write local symbol map " + map_path);
  }

  out << "# object\t" << path << "\n";
  out << "# env\t" << kWriteLocalSymbolMapEnv << "\n";
  out << "ordinal\tserialized\toriginal\tsection\toffset\tsize\n";
  for(size_t i = 0; i < ordered.local_names.size(); ++i) {
    const string & original = ordered.local_names[i];
    const string serialized = should_preserve_local_symbol_name(original)
        ? original
        : synthetic_local_symbol_name(i);
    map<string, Symbol>::const_iterator found = ordered.by_name.find(original);
    if(found == ordered.by_name.end()) {
      throw logic_error("missing local symbol while writing map " +
                        describe_symbol_name_bytes(original));
    }
    out << i << "\t"
        << serialized << "\t"
        << original << "\t"
        << symbol_section_name(found->second.section) << "\t"
        << found->second.offset << "\t"
        << found->second.size << "\n";
  }
}

string decode_symbol_name(const string & encoded, const string & target)
{
  const string prefix(kCppgmSymbolPrefix);
  if(encoded.compare(0, prefix.size(), prefix) != 0) {
    if(target == "macos" && !encoded.empty() && encoded[0] == '_') {
      return encoded.substr(1);
    }
    return encoded;
  }
  const string hex = encoded.substr(prefix.size());
  if(hex.size() % 2 != 0) {
    return encoded;
  }
  string result;
  result.reserve(hex.size() / 2);
  for(size_t i = 0; i < hex.size(); i += 2) {
    result.push_back(static_cast<char>(parse_hex_byte(hex.substr(i, 2))));
  }
  return result;
}

void append_mach_header(vector<unsigned char> & out,
                        uint32_t ncmds,
                        uint32_t sizeofcmds,
                        uint32_t flags)
{
  append_u32(out, MH_MAGIC_64);
  append_u32(out, CPU_TYPE_X86_64);
  append_u32(out, CPU_SUBTYPE_X86_64_ALL);
  append_u32(out, MH_OBJECT);
  append_u32(out, ncmds);
  append_u32(out, sizeofcmds);
  append_u32(out, flags);
  append_u32(out, 0);
}

void append_segment_command(vector<unsigned char> & out,
                            uint64_t vmsize,
                            uint64_t fileoff,
                            uint64_t filesize,
                            uint32_t nsects)
{
  append_u32(out, LC_SEGMENT_64);
  append_u32(out, static_cast<uint32_t>(72 + nsects * 80));
  append_fixed_string(out, "", 16);
  append_u64(out, 0);
  append_u64(out, vmsize);
  append_u64(out, fileoff);
  append_u64(out, filesize);
  append_u32(out, VM_PROT_READ | VM_PROT_WRITE | VM_PROT_EXECUTE);
  append_u32(out, VM_PROT_READ | VM_PROT_WRITE | VM_PROT_EXECUTE);
  append_u32(out, nsects);
  append_u32(out, 0);
}

void append_section_command(vector<unsigned char> & out,
                            const string & sectname,
                            const string & segname,
                            uint64_t addr,
                            uint64_t size,
                            uint32_t offset,
                            uint32_t align_pow2,
                            uint32_t reloff,
                            uint32_t nreloc,
                            uint32_t flags)
{
  append_fixed_string(out, sectname, 16);
  append_fixed_string(out, segname, 16);
  append_u64(out, addr);
  append_u64(out, size);
  append_u32(out, offset);
  append_u32(out, align_pow2);
  append_u32(out, reloff);
  append_u32(out, nreloc);
  append_u32(out, flags);
  append_u32(out, 0);
  append_u32(out, 0);
  append_u32(out, 0);
}

void append_build_version_command(vector<unsigned char> & out)
{
  append_u32(out, LC_BUILD_VERSION);
  append_u32(out, 24);
  append_u32(out, PLATFORM_MACOS);
  append_u32(out, 0x000A0F00);
  append_u32(out, 0x000A0F00);
  append_u32(out, 0);
}

void append_symtab_command(vector<unsigned char> & out,
                           uint32_t symoff,
                           uint32_t nsyms,
                           uint32_t stroff,
                           uint32_t strsize)
{
  append_u32(out, LC_SYMTAB);
  append_u32(out, 24);
  append_u32(out, symoff);
  append_u32(out, nsyms);
  append_u32(out, stroff);
  append_u32(out, strsize);
}

void append_dysymtab_command(vector<unsigned char> & out,
                             uint32_t ilocalsym,
                             uint32_t nlocalsym,
                             uint32_t iextdefsym,
                             uint32_t nextdefsym,
                             uint32_t iundefsym,
                             uint32_t nundefsym)
{
  append_u32(out, LC_DYSYMTAB);
  append_u32(out, 80);
  append_u32(out, ilocalsym);
  append_u32(out, nlocalsym);
  append_u32(out, iextdefsym);
  append_u32(out, nextdefsym);
  append_u32(out, iundefsym);
  append_u32(out, nundefsym);
  for(size_t i = 0; i < 12; ++i) {
    append_u32(out, 0);
  }
}

struct MachORelocEntry
{
  uint32_t address = 0;
  uint32_t info = 0;
};

void append_relocation_entry(vector<unsigned char> & out,
                             const MachORelocEntry & reloc)
{
  append_u32(out, reloc.address);
  append_u32(out, reloc.info);
}

void append_nlist64(vector<unsigned char> & out,
                    uint32_t strx,
                    uint8_t type,
                    uint8_t sect,
                    uint16_t desc,
                    uint64_t value)
{
  append_u32(out, strx);
  out.push_back(type);
  out.push_back(sect);
  out.push_back(static_cast<unsigned char>(desc & 0xFF));
  out.push_back(static_cast<unsigned char>((desc >> 8) & 0xFF));
  append_u64(out, value);
}

void write_macho_object_file(const string & path, const ObjectFile & object)
{
  struct MachOEmitSection
  {
    string sectname;
    string segname;
    uint64_t addr = 0;
    uint64_t size = 0;
    uint32_t offset = 0;
    uint32_t align_pow2 = 0;
    uint32_t flags = 0;
    uint32_t reloff = 0;
    uint8_t index = 0;
    vector<unsigned char> bytes;
    vector<MachORelocEntry> reloc_entries;
  };

  const size_t text_size = object.code.size();
  const size_t data_size = object.data.size();
  vector<string> init_symbol_names;
  vector<string> fini_symbol_names;
  for(size_t i = 0; i < object.symbols.size(); ++i) {
    if(object.symbols[i].section == Symbol::SS_UNDEFINED) {
      continue;
    }
    if(object.symbols[i].name == "@__cppgm_init") {
      init_symbol_names.push_back(object.symbols[i].name);
    } else if(object.symbols[i].name == "@__cppgm_fini") {
      fini_symbol_names.push_back(object.symbols[i].name);
    }
  }
  const size_t init_size = init_symbol_names.size() * 8;
  const size_t fini_size = fini_symbol_names.size() * 8;

  vector<Relocation> text_relocs;
  vector<Relocation> data_relocs;
  for(size_t i = 0; i < object.relocations.size(); ++i) {
    if(object.relocations[i].section == Symbol::SS_CODE) {
      text_relocs.push_back(object.relocations[i]);
    } else {
      data_relocs.push_back(object.relocations[i]);
    }
  }
  vector<Relocation> init_relocs;
  for(size_t i = 0; i < init_symbol_names.size(); ++i) {
    Relocation reloc;
    reloc.section = Symbol::SS_DATA;
    reloc.offset = i * 8;
    reloc.kind = Relocation::RK_ABS64;
    reloc.symbol = init_symbol_names[i];
    init_relocs.push_back(reloc);
  }
  vector<Relocation> fini_relocs;
  for(size_t i = 0; i < fini_symbol_names.size(); ++i) {
    Relocation reloc;
    reloc.section = Symbol::SS_DATA;
    reloc.offset = i * 8;
    reloc.kind = Relocation::RK_ABS64;
    reloc.symbol = fini_symbol_names[i];
    fini_relocs.push_back(reloc);
  }

  vector<MachOEmitSection> sections;
  sections.reserve(2 + object.extra_sections.size() +
                   (init_size != 0 ? 1 : 0) + (fini_size != 0 ? 1 : 0));

  MachOEmitSection text_section;
  text_section.sectname = "__text";
  text_section.segname = "__TEXT";
  text_section.align_pow2 = 4;
  text_section.flags =
      S_REGULAR | S_ATTR_PURE_INSTRUCTIONS | S_ATTR_SOME_INSTRUCTIONS;
  text_section.bytes = object.code;
  text_section.size = text_size;
  const size_t text_section_emit_index = sections.size();
  sections.push_back(text_section);

  map<string, size_t> extra_section_emit_index;
  const auto append_extra_sections_for_segment = [&](const string & segment_name) {
    for(size_t i = 0; i < object.extra_sections.size(); ++i) {
      if(object.extra_sections[i].segment_name != segment_name) {
        continue;
      }
      MachOEmitSection section;
      section.sectname = object.extra_sections[i].section_name;
      section.segname = object.extra_sections[i].segment_name;
      section.align_pow2 = object.extra_sections[i].macho_align_pow2;
      section.flags = object.extra_sections[i].macho_flags;
      section.bytes = object.extra_sections[i].bytes;
      section.size = object.extra_sections[i].bytes.size();
      extra_section_emit_index[macho_extra_section_key(section.segname,
                                                       section.sectname)] = sections.size();
      sections.push_back(section);
    }
  };
  append_extra_sections_for_segment("__TEXT");

  MachOEmitSection data_section;
  data_section.sectname = "__data";
  data_section.segname = "__DATA";
  data_section.align_pow2 = 4;
  data_section.flags = S_REGULAR;
  data_section.bytes = object.data;
  data_section.size = data_size;
  const size_t data_section_emit_index = sections.size();
  sections.push_back(data_section);

  for(size_t i = 0; i < object.extra_sections.size(); ++i) {
    if(object.extra_sections[i].segment_name == "__TEXT") {
      continue;
    }
    MachOEmitSection section;
    section.sectname = object.extra_sections[i].section_name;
    section.segname = object.extra_sections[i].segment_name;
    section.align_pow2 = object.extra_sections[i].macho_align_pow2;
    section.flags = object.extra_sections[i].macho_flags;
    section.bytes = object.extra_sections[i].bytes;
    section.size = object.extra_sections[i].bytes.size();
    extra_section_emit_index[macho_extra_section_key(section.segname,
                                                     section.sectname)] = sections.size();
    sections.push_back(section);
  }

  const size_t init_section_emit_index = sections.size();
  if(init_size != 0) {
    MachOEmitSection section;
    section.sectname = "__mod_init_func";
    section.segname = "__DATA";
    section.align_pow2 = 3;
    section.flags = S_MOD_INIT_FUNC_POINTERS;
    section.bytes.assign(init_size, 0);
    section.size = init_size;
    sections.push_back(section);
  }
  const size_t fini_section_emit_index = sections.size();
  if(fini_size != 0) {
    MachOEmitSection section;
    section.sectname = "__mod_term_func";
    section.segname = "__DATA";
    section.align_pow2 = 3;
    section.flags = S_MOD_TERM_FUNC_POINTERS;
    section.bytes.assign(fini_size, 0);
    section.size = fini_size;
    sections.push_back(section);
  }

  const OrderedSymbols ordered = collect_ordered_symbols(object);
  vector<string> local_names;
  local_names.reserve(ordered.local_names.size());
  for(size_t i = 0; i < ordered.local_names.size(); ++i) {
    map<string, Symbol>::const_iterator found = ordered.by_name.find(ordered.local_names[i]);
    if(found == ordered.by_name.end()) {
      throw logic_error("missing local symbol " +
                        describe_symbol_name_bytes(ordered.local_names[i]));
    }
    if(should_emit_macho_local_symbol(found->second)) {
      local_names.push_back(ordered.local_names[i]);
    }
  }
  const vector<string> & global_names = ordered.global_names;
  const vector<string> & undefined_names = ordered.undefined_names;
  const map<string, Symbol::Binding> & undefined_bindings =
      ordered.undefined_bindings;
  const map<string, Symbol> & symbol_by_name = ordered.by_name;

  const uint32_t nlocalsym = static_cast<uint32_t>(local_names.size());
  const uint32_t nextdefsym = static_cast<uint32_t>(global_names.size());
  const uint32_t nundefsym = static_cast<uint32_t>(undefined_names.size());
  const uint32_t nsyms = nlocalsym + nextdefsym + nundefsym;

  vector<unsigned char> strtab(1, 0);
  map<string, uint32_t> symbol_strx;
  map<string, uint32_t> symbol_index;
  map<string, string> serialized_local_names;
  for(size_t i = 0; i < local_names.size(); ++i) {
    if(!should_preserve_local_symbol_name(local_names[i])) {
      serialized_local_names[local_names[i]] = synthetic_local_symbol_name(i);
    }
  }

  const vector<string> symbol_order = [&]() {
    vector<string> names;
    names.insert(names.end(), local_names.begin(), local_names.end());
    names.insert(names.end(), global_names.begin(), global_names.end());
    names.insert(names.end(), undefined_names.begin(), undefined_names.end());
    return names;
  }();

  for(size_t i = 0; i < symbol_order.size(); ++i) {
    map<string, string>::const_iterator serialized_local =
        serialized_local_names.find(symbol_order[i]);
    const string encoded = encode_symbol_name(serialized_local == serialized_local_names.end()
                                                  ? symbol_order[i]
                                                  : serialized_local->second,
                                              object.target);
    symbol_strx[symbol_order[i]] = static_cast<uint32_t>(strtab.size());
    symbol_index[symbol_order[i]] = static_cast<uint32_t>(i);
    strtab.insert(strtab.end(), encoded.begin(), encoded.end());
    strtab.push_back(0);
  }

  const uint32_t section_count = static_cast<uint32_t>(sections.size());
  const uint32_t ncmds = 4;
  const uint32_t sizeofcmds = 72 + section_count * 80 + 24 + 24 + 80;
  const uint32_t header_size = 32;
  const uint32_t section_data_offset = header_size + sizeofcmds;

  uint64_t addr_cursor = 0;
  uint32_t file_cursor = section_data_offset;
  for(size_t i = 0; i < sections.size(); ++i) {
    const uint32_t align_bytes = 1U << sections[i].align_pow2;
    const bool file_backed = !macho_section_is_zerofill(sections[i].flags);
    addr_cursor = align_up(addr_cursor, align_bytes);
    if(i != 0 && file_backed) {
      const uint32_t payload_offset = file_cursor - section_data_offset;
      file_cursor = section_data_offset +
          static_cast<uint32_t>(align_up(payload_offset, align_bytes));
    }
    sections[i].addr = addr_cursor;
    sections[i].offset = file_backed ? file_cursor : 0;
    sections[i].index = static_cast<uint8_t>(i + 1);
    addr_cursor += sections[i].size;
    if(file_backed) {
      file_cursor += static_cast<uint32_t>(sections[i].size);
    }
  }

  const uint32_t reloc_base = file_cursor;

  uint8_t text_section_index = 0;
  uint8_t data_section_index = 0;
  uint64_t text_section_addr = 0;
  uint64_t data_section_addr = 0;
  map<string, uint8_t> extra_section_index;
  map<string, uint64_t> extra_section_addr;
  for(size_t i = 0; i < sections.size(); ++i) {
    if(sections[i].segname == "__TEXT" && sections[i].sectname == "__text") {
      text_section_index = sections[i].index;
      text_section_addr = sections[i].addr;
    } else if(sections[i].segname == "__DATA" && sections[i].sectname == "__data") {
      data_section_index = sections[i].index;
      data_section_addr = sections[i].addr;
    } else {
      const string key = macho_extra_section_key(sections[i].segname, sections[i].sectname);
      extra_section_index[key] = sections[i].index;
      extra_section_addr[key] = sections[i].addr;
    }
  }

  const auto symbol_section_index =
      [&](const Symbol & symbol) -> uint8_t
      {
        if(symbol.section == Symbol::SS_CODE) {
          return text_section_index;
        }
        if(symbol.section == Symbol::SS_DATA) {
          return data_section_index;
        }
        if(symbol.section == Symbol::SS_EXTRA) {
          map<string, uint8_t>::const_iterator found =
              extra_section_index.find(symbol.extra_section);
          if(found == extra_section_index.end()) {
            throw logic_error("missing Mach-O extra section index for symbol " +
                              describe_symbol_name_bytes(symbol.name));
          }
          return found->second;
        }
        throw logic_error("undefined symbol cannot have section index");
      };

  const auto symbol_section_addr =
      [&](const Symbol & symbol) -> uint64_t
      {
        if(symbol.section == Symbol::SS_CODE) {
          return text_section_addr;
        }
        if(symbol.section == Symbol::SS_DATA) {
          return data_section_addr;
        }
        if(symbol.section == Symbol::SS_EXTRA) {
          map<string, uint64_t>::const_iterator found =
              extra_section_addr.find(symbol.extra_section);
          if(found == extra_section_addr.end()) {
            throw logic_error("missing Mach-O extra section address for symbol " +
                              describe_symbol_name_bytes(symbol.name));
          }
          return found->second;
        }
        throw logic_error("undefined symbol cannot have section address");
      };

  {
    vector<Relocation> kept_text_relocs;
    for(size_t i = 0; i < text_relocs.size(); ++i) {
      const Relocation & reloc = text_relocs[i];
      if(reloc.kind != Relocation::RK_BRANCH32 &&
         reloc.kind != Relocation::RK_PCREL32) {
        kept_text_relocs.push_back(reloc);
        continue;
      }
      map<string, Symbol>::const_iterator found = symbol_by_name.find(reloc.symbol);
      if(found == symbol_by_name.end() ||
         found->second.section != Symbol::SS_CODE ||
         found->second.binding != Symbol::SB_LOCAL ||
         should_emit_macho_local_symbol(found->second)) {
        kept_text_relocs.push_back(reloc);
        continue;
      }
      // Hidden block-local code labels do not survive into the Mach-O symbol table,
      // so we must resolve their intra-object branches now. For link-visible local
      // text symbols, ld64 may still reorder subsections under
      // MH_SUBSECTIONS_VIA_SYMBOLS, so those relocations must stay live.
      const int64_t disp =
          static_cast<int64_t>(found->second.offset) +
          static_cast<int64_t>(reloc.addend) -
          static_cast<int64_t>(reloc.offset) - 4;
      if(disp < static_cast<int64_t>(numeric_limits<int32_t>::min()) ||
         disp > static_cast<int64_t>(numeric_limits<int32_t>::max())) {
        throw logic_error("local Mach-O pcrel relocation out of range for " +
                          describe_symbol_name_bytes(reloc.symbol));
      }
      overwrite_i32_le(sections[text_section_emit_index].bytes,
                       reloc.offset,
                       static_cast<int32_t>(disp));
    }
    text_relocs.swap(kept_text_relocs);
  }

  const auto build_standard_reloc_entry =
      [&](const Relocation & reloc) -> MachORelocEntry
      {
        MachORelocEntry entry;
        entry.address = static_cast<uint32_t>(reloc.offset);
        if(reloc.kind == Relocation::RK_TPOFF32) {
          throw logic_error("ELF-only TPOFF32 relocation emitted while writing Mach-O object");
        }
        const uint32_t type = reloc.kind == Relocation::RK_BRANCH32
            ? X86_64_RELOC_BRANCH
            : reloc.kind == Relocation::RK_PCREL32
                ? X86_64_RELOC_SIGNED
                : reloc.kind == Relocation::RK_INDIRECT_REL32
                    ? X86_64_RELOC_GOT_LOAD
                    : reloc.kind == Relocation::RK_TLV_REL32
                        ? X86_64_RELOC_TLV
                    : X86_64_RELOC_UNSIGNED;
        const uint32_t pcrel =
            (reloc.kind == Relocation::RK_BRANCH32 ||
             reloc.kind == Relocation::RK_PCREL32 ||
             reloc.kind == Relocation::RK_INDIRECT_REL32 ||
             reloc.kind == Relocation::RK_TLV_REL32) ? 1 : 0;
        const uint32_t length =
            (reloc.kind == Relocation::RK_BRANCH32 ||
             reloc.kind == Relocation::RK_PCREL32 ||
             reloc.kind == Relocation::RK_INDIRECT_REL32 ||
             reloc.kind == Relocation::RK_TLV_REL32) ? 2 : 3;
        entry.info = symbol_index.find(reloc.symbol)->second |
            (pcrel << 24) | (length << 25) | (1U << 27) | (type << 28);
        return entry;
      };

  for(size_t i = 0; i < text_relocs.size(); ++i) {
    sections[text_section_emit_index].reloc_entries.push_back(
        build_standard_reloc_entry(text_relocs[i]));
  }
  for(size_t i = 0; i < data_relocs.size(); ++i) {
    sections[data_section_emit_index].reloc_entries.push_back(
        build_standard_reloc_entry(data_relocs[i]));
  }
  if(init_size != 0) {
    for(size_t i = 0; i < init_relocs.size(); ++i) {
      sections[init_section_emit_index].reloc_entries.push_back(
          build_standard_reloc_entry(init_relocs[i]));
    }
  }
  if(fini_size != 0) {
    for(size_t i = 0; i < fini_relocs.size(); ++i) {
      sections[fini_section_emit_index].reloc_entries.push_back(
          build_standard_reloc_entry(fini_relocs[i]));
    }
  }

  for(size_t si = 0; si < object.extra_sections.size(); ++si) {
    const string current_extra_key =
        macho_extra_section_key(object.extra_sections[si].segment_name,
                                object.extra_sections[si].section_name);
    MachOEmitSection & section =
        sections[extra_section_emit_index.find(current_extra_key)->second];
    const uint64_t section_addr = extra_section_addr.find(current_extra_key)->second;
    for(size_t ri = 0; ri < object.extra_sections[si].relocations.size(); ++ri) {
      const ExtraRelocation & reloc = object.extra_sections[si].relocations[ri];
      if(reloc.kind == ExtraRelocation::RK_PCREL32 &&
         reloc.target_kind != ExtraRelocation::TK_SYMBOL) {
        uint64_t target_addr = 0;
        if(reloc.target_kind == ExtraRelocation::TK_CODE) {
          target_addr = text_section_addr;
        } else if(reloc.target_kind == ExtraRelocation::TK_DATA) {
          target_addr = data_section_addr;
        } else {
          map<string, uint64_t>::const_iterator found =
              extra_section_addr.find(reloc.target_extra_section);
          if(found == extra_section_addr.end()) {
            throw logic_error("missing Mach-O extra pcrel relocation target section");
          }
          target_addr = found->second;
        }
        // Extra-section PC-relative relocations are used for DWARF-style
        // references, which are based on the address of the encoded field
        // itself rather than the next instruction.
        const int64_t disp =
            static_cast<int64_t>(target_addr) +
            static_cast<int64_t>(reloc.addend) -
            static_cast<int64_t>(section_addr + reloc.offset);
        if(disp < static_cast<int64_t>(numeric_limits<int32_t>::min()) ||
           disp > static_cast<int64_t>(numeric_limits<int32_t>::max())) {
          throw logic_error("Mach-O extra pcrel relocation out of range");
        }
        overwrite_i32_le(section.bytes, reloc.offset, static_cast<int32_t>(disp));
        continue;
      }
      if(reloc.kind == ExtraRelocation::RK_ABS64) {
        uint64_t encoded_value = static_cast<uint64_t>(reloc.addend);
        if(reloc.target_kind == ExtraRelocation::TK_CODE) {
          encoded_value += text_section_addr;
        } else if(reloc.target_kind == ExtraRelocation::TK_DATA) {
          encoded_value += data_section_addr;
        } else if(reloc.target_kind == ExtraRelocation::TK_EXTRA) {
          map<string, uint64_t>::const_iterator found =
              extra_section_addr.find(reloc.target_extra_section);
          if(found == extra_section_addr.end()) {
            throw logic_error("missing Mach-O extra relocation target section");
          }
          encoded_value += found->second;
        }
        if(reloc.offset + 8 > section.bytes.size()) {
          throw logic_error("extra relocation offset past section payload");
        }
        for(size_t bi = 0; bi < 8; ++bi) {
          section.bytes[reloc.offset + bi] =
              static_cast<unsigned char>((encoded_value >> (bi * 8)) & 0xFFU);
        }
      }
      MachORelocEntry entry;
      entry.address = static_cast<uint32_t>(reloc.offset);
      uint32_t symbolnum = 0;
      uint32_t is_extern = 1;
      if(reloc.target_kind == ExtraRelocation::TK_SYMBOL) {
        symbolnum = symbol_index.find(reloc.symbol)->second;
      } else {
        is_extern = 0;
        if(reloc.target_kind == ExtraRelocation::TK_CODE) {
          symbolnum = text_section_index;
        } else if(reloc.target_kind == ExtraRelocation::TK_DATA) {
          symbolnum = data_section_index;
        } else {
          symbolnum = sections[extra_section_emit_index.find(
              reloc.target_extra_section)->second].index;
        }
      }
      const uint32_t type = reloc.kind == ExtraRelocation::RK_BRANCH32
          ? X86_64_RELOC_BRANCH
          : reloc.kind == ExtraRelocation::RK_INDIRECT_REL32
          ? X86_64_RELOC_GOT
          : reloc.kind == ExtraRelocation::RK_PCREL32
              ? X86_64_RELOC_SIGNED
          : X86_64_RELOC_UNSIGNED;
      const uint32_t pcrel =
          (reloc.kind == ExtraRelocation::RK_BRANCH32 ||
           reloc.kind == ExtraRelocation::RK_INDIRECT_REL32 ||
           reloc.kind == ExtraRelocation::RK_PCREL32) ? 1 : 0;
      const uint32_t length =
          (reloc.kind == ExtraRelocation::RK_BRANCH32 ||
           reloc.kind == ExtraRelocation::RK_INDIRECT_REL32 ||
           reloc.kind == ExtraRelocation::RK_PCREL32) ? 2 : 3;
      entry.info = symbolnum |
          (pcrel << 24) | (length << 25) | (is_extern << 27) | (type << 28);
      section.reloc_entries.push_back(entry);
    }
  }

  uint32_t symoff = reloc_base;
  for(size_t i = 0; i < sections.size(); ++i) {
    sections[i].reloff = symoff;
    symoff += static_cast<uint32_t>(sections[i].reloc_entries.size() * 8);
  }
  const uint32_t stroff = symoff + nsyms * 16;

  const uint32_t segment_filesize = file_cursor - section_data_offset;
  const uint64_t segment_vmsize = addr_cursor;

  vector<unsigned char> out;
  append_mach_header(out, ncmds, sizeofcmds, MH_SUBSECTIONS_VIA_SYMBOLS);
  append_segment_command(out,
                         segment_vmsize,
                         section_data_offset,
                         segment_filesize,
                         section_count);
  for(size_t i = 0; i < sections.size(); ++i) {
    append_section_command(out,
                           sections[i].sectname,
                           sections[i].segname,
                           sections[i].addr,
                           sections[i].size,
                           sections[i].offset,
                           sections[i].align_pow2,
                           sections[i].reloff,
                           static_cast<uint32_t>(sections[i].reloc_entries.size()),
                           sections[i].flags);
  }
  append_build_version_command(out);
  append_symtab_command(out, symoff, nsyms, stroff,
                        static_cast<uint32_t>(strtab.size()));
  append_dysymtab_command(out,
                          0,
                          nlocalsym,
                          nlocalsym,
                          nextdefsym,
                          nlocalsym + nextdefsym,
                          nundefsym);

  out.resize(section_data_offset, 0);
  for(size_t i = 0; i < sections.size(); ++i) {
    if(macho_section_is_zerofill(sections[i].flags)) {
      continue;
    }
    out.resize(sections[i].offset, 0);
    out.insert(out.end(), sections[i].bytes.begin(), sections[i].bytes.end());
  }

  for(size_t i = 0; i < sections.size(); ++i) {
    out.resize(sections[i].reloff, 0);
    for(size_t ri = 0; ri < sections[i].reloc_entries.size(); ++ri) {
      append_relocation_entry(out, sections[i].reloc_entries[ri]);
    }
  }

  for(size_t i = 0; i < local_names.size(); ++i) {
    const string & name = local_names[i];
    map<string, Symbol>::const_iterator found = symbol_by_name.find(name);
    if(found == symbol_by_name.end()) {
      throw logic_error("missing local symbol " + name);
    }
    const Symbol & symbol = found->second;
    append_nlist64(out,
                   symbol_strx.find(name)->second,
                   N_SECT,
                   symbol_section_index(symbol),
                   0,
                   symbol_section_addr(symbol) + symbol.offset);
  }

  for(size_t i = 0; i < global_names.size(); ++i) {
    const string & name = global_names[i];
    map<string, Symbol>::const_iterator found = symbol_by_name.find(name);
    if(found == symbol_by_name.end()) {
      throw logic_error("missing global symbol " + name);
    }
    const Symbol & symbol = found->second;
    const bool weak = symbol.binding == Symbol::SB_WEAK;
    const uint16_t desc = weak ? N_WEAK_DEF : 0;
    const uint8_t type = static_cast<uint8_t>(N_SECT | N_EXT);
    append_nlist64(out,
                   symbol_strx.find(name)->second,
                   type,
                   symbol_section_index(symbol),
                   desc,
                   symbol_section_addr(symbol) + symbol.offset);
  }

  for(size_t i = 0; i < undefined_names.size(); ++i) {
    const string & name = undefined_names[i];
    const bool weak = undefined_bindings.find(name) != undefined_bindings.end() &&
        undefined_bindings.find(name)->second == Symbol::SB_WEAK;
    append_nlist64(out,
                   symbol_strx.find(name)->second,
                   static_cast<uint8_t>(N_UNDF | N_EXT),
                   NO_SECT,
                   weak ? N_WEAK_REF : 0,
                   0);
  }

  out.insert(out.end(), strtab.begin(), strtab.end());

  ofstream file(path.c_str(), ios::binary | ios::trunc);
  if(!file) {
    throw logic_error("unable to open object output file: " + path);
  }
  file.write(reinterpret_cast<const char *>(&out[0]),
             static_cast<streamsize>(out.size()));
  file.close();
  if(!file) {
    throw logic_error("unable to write object output file: " + path);
  }
}

ObjectFile parse_macho_object_bytes(const vector<unsigned char> & bytes,
                                    const string & path)
{
  if(bytes.size() < 32) {
    throw logic_error("truncated Mach-O object " + path);
  }
  if(read_u32(bytes, 0) != MH_MAGIC_64 ||
     read_u32(bytes, 4) != CPU_TYPE_X86_64 ||
     read_u32(bytes, 12) != MH_OBJECT) {
    throw logic_error("unsupported Mach-O object " + path);
  }

  const uint32_t ncmds = read_u32(bytes, 16);
  const uint32_t sizeofcmds = read_u32(bytes, 20);
  if(32 + sizeofcmds > bytes.size()) {
    throw logic_error("truncated load commands in " + path);
  }

  map<uint8_t, MachOSectionInfo> sections;
  uint32_t symoff = 0;
  uint32_t nsyms = 0;
  uint32_t stroff = 0;
  uint32_t strsize = 0;
  uint8_t next_section_index = 1;

  size_t command_offset = 32;
  for(uint32_t i = 0; i < ncmds; ++i) {
    const uint32_t cmd = read_u32(bytes, command_offset);
    const uint32_t cmdsize = read_u32(bytes, command_offset + 4);
    if(cmdsize < 8 || command_offset + cmdsize > bytes.size()) {
      throw logic_error("invalid Mach-O load command in " + path);
    }
    if(cmd == LC_SEGMENT_64) {
      const uint32_t nsects = read_u32(bytes, command_offset + 64);
      size_t section_offset = command_offset + 72;
      for(uint32_t si = 0; si < nsects; ++si) {
        if(section_offset + 80 > command_offset + cmdsize) {
          throw logic_error("truncated Mach-O section table in " + path);
        }
        MachOSectionInfo info;
        info.index = next_section_index++;
        info.sectname = read_padded_string(bytes, section_offset,
                                           section_offset + 16);
        info.segname = read_padded_string(bytes, section_offset + 16,
                                          section_offset + 32);
        if(info.sectname == "__text" && info.segname == "__TEXT") {
          info.section = Symbol::SS_CODE;
        } else if(info.sectname == "__data" && info.segname == "__DATA") {
          info.section = Symbol::SS_DATA;
        } else {
          info.is_extra = true;
        }
        info.addr = read_u64(bytes, section_offset + 32);
        info.size = read_u64(bytes, section_offset + 40);
        info.offset = read_u32(bytes, section_offset + 48);
        info.reloff = read_u32(bytes, section_offset + 56);
        info.nreloc = read_u32(bytes, section_offset + 60);
        info.align_pow2 = read_u32(bytes, section_offset + 52);
        info.flags = read_u32(bytes, section_offset + 64);
        sections[info.index] = info;
        section_offset += 80;
      }
    } else if(cmd == LC_SYMTAB) {
      symoff = read_u32(bytes, command_offset + 8);
      nsyms = read_u32(bytes, command_offset + 12);
      stroff = read_u32(bytes, command_offset + 16);
      strsize = read_u32(bytes, command_offset + 20);
    }
    command_offset += cmdsize;
  }

  if(symoff == 0 || stroff == 0) {
    throw logic_error("missing Mach-O symbol table in " + path);
  }
  if(symoff + static_cast<size_t>(nsyms) * 16 > bytes.size() ||
     stroff + strsize > bytes.size()) {
    throw logic_error("truncated Mach-O symbol data in " + path);
  }

  vector<MachOSymbolInfo> symbols;
  symbols.reserve(nsyms);
  for(uint32_t i = 0; i < nsyms; ++i) {
    const size_t offset = symoff + static_cast<size_t>(i) * 16;
    const uint32_t strx = read_u32(bytes, offset);
    MachOSymbolInfo symbol;
    symbol.name = decode_symbol_name(read_c_string(bytes, stroff + strx,
                                                   stroff + strsize),
                                     "macos");
    symbol.type = bytes[offset + 4];
    symbol.sect = bytes[offset + 5];
    symbol.desc = read_u16(bytes, offset + 6);
    symbol.value = read_u64(bytes, offset + 8);
    symbols.push_back(symbol);
  }

  ObjectFile object;
  object.target = "macos";
  map<uint8_t, size_t> extra_section_by_index;

  for(map<uint8_t, MachOSectionInfo>::const_iterator it = sections.begin();
      it != sections.end(); ++it) {
    const MachOSectionInfo & section = it->second;
    vector<unsigned char> section_bytes;
    if(macho_section_is_zerofill(section.flags)) {
      section_bytes.assign(static_cast<size_t>(section.size), 0);
    } else {
      if(section.offset + section.size > bytes.size()) {
        throw logic_error("truncated Mach-O section payload in " + path);
      }
      section_bytes.assign(bytes.begin() + section.offset,
                           bytes.begin() + section.offset + section.size);
    }
    if(section.is_extra) {
      ExtraSection extra;
      extra.segment_name = section.segname;
      extra.section_name = section.sectname;
      extra.macho_align_pow2 = section.align_pow2;
      extra.macho_flags = section.flags;
      extra.bytes = section_bytes;
      extra_section_by_index[section.index] = object.extra_sections.size();
      object.extra_sections.push_back(extra);
    } else if(section.section == Symbol::SS_CODE) {
      object.code = section_bytes;
    } else {
      object.data = section_bytes;
    }
  }

  for(size_t i = 0; i < symbols.size(); ++i) {
    const MachOSymbolInfo & macho_symbol = symbols[i];
    if((macho_symbol.type & N_TYPE) == N_UNDF) {
      continue;
    }
    if((macho_symbol.type & N_TYPE) != N_SECT) {
      continue;
    }
    map<uint8_t, MachOSectionInfo>::const_iterator section =
        sections.find(macho_symbol.sect);
    if(section == sections.end()) {
      continue;
    }
    Symbol symbol;
    if(!(macho_symbol.type & N_EXT)) {
      symbol.binding = Symbol::SB_LOCAL;
    } else if(macho_symbol.desc & (N_WEAK_DEF | N_WEAK_REF)) {
      symbol.binding = Symbol::SB_WEAK;
    } else {
      symbol.binding = Symbol::SB_GLOBAL;
    }
    symbol.section = section->second.section;
    if(section->second.is_extra) {
      symbol.section = Symbol::SS_EXTRA;
      symbol.extra_section = macho_extra_section_key(section->second.segname,
                                                     section->second.sectname);
    }
    symbol.name = macho_symbol.name;
    if(macho_symbol.value < section->second.addr) {
      throw logic_error("invalid Mach-O symbol value in " + path);
    }
    symbol.offset = static_cast<size_t>(macho_symbol.value - section->second.addr);
    object.symbols.push_back(symbol);
  }

  for(map<uint8_t, MachOSectionInfo>::const_iterator it = sections.begin();
      it != sections.end(); ++it) {
    const MachOSectionInfo & section = it->second;
    if(section.nreloc == 0) {
      continue;
    }
    if(section.reloff + static_cast<size_t>(section.nreloc) * 8 > bytes.size()) {
      throw logic_error("truncated Mach-O relocation table in " + path);
    }
    for(uint32_t ri = 0; ri < section.nreloc; ++ri) {
      const size_t reloc_offset = section.reloff + static_cast<size_t>(ri) * 8;
      const uint32_t info = read_u32(bytes, reloc_offset + 4);
      const uint32_t symbolnum = info & 0x00FFFFFFU;
      const uint32_t pcrel = (info >> 24) & 0x1U;
      const uint32_t length = (info >> 25) & 0x3U;
      const uint32_t is_extern = (info >> 27) & 0x1U;
      const uint32_t type = (info >> 28) & 0xFU;
      const size_t reloc_site = static_cast<size_t>(read_i32(bytes, reloc_offset));
      if(section.is_extra) {
        map<uint8_t, size_t>::const_iterator extra_found =
            extra_section_by_index.find(section.index);
        if(extra_found == extra_section_by_index.end()) {
          throw logic_error("missing Mach-O extra section payload in " + path);
        }
        ExtraSection & extra = object.extra_sections[extra_found->second];
        ExtraRelocation reloc;
        reloc.offset = reloc_site;
        if(type == X86_64_RELOC_BRANCH && pcrel == 1 && length == 2) {
          reloc.kind = ExtraRelocation::RK_BRANCH32;
        } else if(type == X86_64_RELOC_GOT && pcrel == 1 && length == 2) {
          reloc.kind = ExtraRelocation::RK_INDIRECT_REL32;
        } else if(type == X86_64_RELOC_SIGNED && pcrel == 1 && length == 2) {
          reloc.kind = ExtraRelocation::RK_PCREL32;
          if(reloc.offset + 4 > extra.bytes.size()) {
            throw logic_error("truncated Mach-O extra pcrel relocation addend in " + path);
          }
          reloc.addend = static_cast<int64_t>(read_i32(extra.bytes, reloc.offset));
        } else if(type == X86_64_RELOC_UNSIGNED && pcrel == 0 && length == 3) {
          reloc.kind = ExtraRelocation::RK_ABS64;
          if(reloc.offset + 8 > extra.bytes.size()) {
            throw logic_error("truncated Mach-O extra abs64 relocation addend in " + path);
          }
          reloc.addend = static_cast<int64_t>(read_u64(extra.bytes, reloc.offset));
        } else {
          throw logic_error("unsupported Mach-O extra relocation kind in " + path);
        }
        if(is_extern) {
          if(symbolnum >= symbols.size()) {
            throw logic_error("invalid Mach-O relocation symbol index in " + path);
          }
          reloc.target_kind = ExtraRelocation::TK_SYMBOL;
          reloc.symbol = symbols[symbolnum].name;
        } else {
          map<uint8_t, MachOSectionInfo>::const_iterator target = sections.find(
              static_cast<uint8_t>(symbolnum));
          if(target == sections.end()) {
            throw logic_error("invalid Mach-O local relocation section in " + path);
          }
          if(target->second.is_extra) {
            reloc.target_kind = ExtraRelocation::TK_EXTRA;
            reloc.target_extra_section = macho_extra_section_key(target->second.segname,
                                                                 target->second.sectname);
          } else if(target->second.section == Symbol::SS_CODE) {
            reloc.target_kind = ExtraRelocation::TK_CODE;
          } else {
            reloc.target_kind = ExtraRelocation::TK_DATA;
          }
        }
        extra.relocations.push_back(reloc);
        continue;
      }
      if(!is_extern) {
        throw logic_error("unsupported local Mach-O relocation in " + path);
      }
      if(symbolnum >= symbols.size()) {
        throw logic_error("invalid Mach-O relocation symbol index in " + path);
      }
      Relocation reloc;
      reloc.section = section.section;
      reloc.offset = reloc_site;
      if(type == X86_64_RELOC_UNSIGNED && pcrel == 0 && length == 3) {
        reloc.kind = Relocation::RK_ABS64;
        const vector<unsigned char> & section_bytes =
            section.section == Symbol::SS_CODE ? object.code : object.data;
        if(reloc.offset + 8 > section_bytes.size()) {
          throw logic_error("truncated Mach-O abs64 relocation addend in " + path);
        }
        reloc.addend = static_cast<int64_t>(read_u64(section_bytes, reloc.offset));
      } else if(type == X86_64_RELOC_BRANCH && pcrel == 1 && length == 2) {
        reloc.kind = Relocation::RK_BRANCH32;
      } else if(type == X86_64_RELOC_SIGNED && pcrel == 1 && length == 2) {
        reloc.kind = Relocation::RK_PCREL32;
      } else if(type == X86_64_RELOC_GOT_LOAD && pcrel == 1 && length == 2) {
        reloc.kind = Relocation::RK_INDIRECT_REL32;
      } else if(type == X86_64_RELOC_TLV && pcrel == 1 && length == 2) {
        reloc.kind = Relocation::RK_TLV_REL32;
      } else {
        throw logic_error("unsupported Mach-O relocation kind in " + path);
      }
      reloc.symbol = symbols[symbolnum].name;
      object.relocations.push_back(reloc);
    }
  }

  return object;
}

void append_elf_header(vector<unsigned char> & out,
                       uint64_t shoff,
                       uint16_t shnum,
                       uint16_t shstrndx)
{
  out.push_back(ELFMAG0);
  out.push_back(ELFMAG1);
  out.push_back(ELFMAG2);
  out.push_back(ELFMAG3);
  out.push_back(ELFCLASS64);
  out.push_back(ELFDATA2LSB);
  out.push_back(EV_CURRENT);
  out.push_back(ELFOSABI_SYSV);
  out.push_back(0);
  out.insert(out.end(), 7, 0);
  append_u16(out, ET_REL);
  append_u16(out, EM_X86_64);
  append_u32(out, EV_CURRENT);
  append_u64(out, 0);
  append_u64(out, 0);
  append_u64(out, shoff);
  append_u32(out, 0);
  append_u16(out, 64);
  append_u16(out, 0);
  append_u16(out, 0);
  append_u16(out, 64);
  append_u16(out, shnum);
  append_u16(out, shstrndx);
}

void append_elf_section_header(vector<unsigned char> & out,
                               uint32_t name,
                               uint32_t type,
                               uint64_t flags,
                               uint64_t offset,
                               uint64_t size,
                               uint32_t link,
                               uint32_t info,
                               uint64_t addralign,
                               uint64_t entsize)
{
  append_u32(out, name);
  append_u32(out, type);
  append_u64(out, flags);
  append_u64(out, 0);
  append_u64(out, offset);
  append_u64(out, size);
  append_u32(out, link);
  append_u32(out, info);
  append_u64(out, addralign);
  append_u64(out, entsize);
}

void append_elf_symbol(vector<unsigned char> & out,
                       uint32_t name,
                       uint8_t info,
                       uint16_t shndx,
                       uint64_t value,
                       uint64_t size)
{
  append_u32(out, name);
  out.push_back(info);
  out.push_back(0);
  append_u16(out, shndx);
  append_u64(out, value);
  append_u64(out, size);
}

void append_elf_rela(vector<unsigned char> & out,
                     uint64_t offset,
                     uint64_t info,
                     int64_t addend)
{
  append_u64(out, offset);
  append_u64(out, info);
  append_u64(out, static_cast<uint64_t>(addend));
}

bool is_elf_tls_section_name(const string & name)
{
  return name == ".tdata" || name == ".tbss";
}

bool is_elf_debug_section_name(const string & name)
{
  return name.compare(0, 7, ".debug_") == 0;
}

void write_elf_object_file(const string & path, const ObjectFile & object)
{
  struct CodeRange
  {
    size_t original_offset = 0;
    size_t size = 0;
    bool weak = false;
    string comdat_group;
    string section_name;
    size_t shared_offset = 0;
    uint32_t text_index = 0;
    uint32_t rela_index = 0;
    uint32_t group_index = 0;
    vector<unsigned char> bytes;
    vector<Relocation> relocs;
  };

  struct EmitSection
  {
    string name;
    uint32_t type = SHT_NULL;
    uint64_t flags = 0;
    vector<unsigned char> bytes;
    uint32_t link = 0;
    uint32_t info = 0;
    uint64_t addralign = 1;
    uint64_t entsize = 0;
  };

  const OrderedSymbols ordered = collect_ordered_symbols(object);
  const vector<string> & local_names = ordered.local_names;
  const vector<string> & global_names = ordered.global_names;
  const vector<string> & undefined_names = ordered.undefined_names;
  const map<string, Symbol::Binding> & undefined_bindings =
      ordered.undefined_bindings;
  const map<string, Symbol> & symbol_by_name = ordered.by_name;
  map<string, bool> undefined_tls_names;
  for(size_t i = 0; i < object.relocations.size(); ++i) {
    if(object.relocations[i].kind == Relocation::RK_TPOFF32) {
      undefined_tls_names[object.relocations[i].symbol] = true;
    }
  }
  map<string, bool> is_local_name;
  for(size_t i = 0; i < local_names.size(); ++i) {
    is_local_name[local_names[i]] = true;
  }

  vector<string> init_symbol_names;
  vector<string> fini_symbol_names;
  for(size_t i = 0; i < object.symbols.size(); ++i) {
    if(object.symbols[i].section == Symbol::SS_UNDEFINED) {
      continue;
    }
    if(object.symbols[i].name == "@__cppgm_init") {
      init_symbol_names.push_back(object.symbols[i].name);
    } else if(object.symbols[i].name == "@__cppgm_fini") {
      fini_symbol_names.push_back(object.symbols[i].name);
    }
  }

  vector<Relocation> init_relocs;
  for(size_t i = 0; i < init_symbol_names.size(); ++i) {
    Relocation reloc;
    reloc.section = Symbol::SS_DATA;
    reloc.offset = i * 8;
    reloc.kind = Relocation::RK_ABS64;
    reloc.symbol = init_symbol_names[i];
    init_relocs.push_back(reloc);
  }

  vector<Relocation> fini_relocs;
  for(size_t i = 0; i < fini_symbol_names.size(); ++i) {
    Relocation reloc;
    reloc.section = Symbol::SS_DATA;
    reloc.offset = i * 8;
    reloc.kind = Relocation::RK_ABS64;
    reloc.symbol = fini_symbol_names[i];
    fini_relocs.push_back(reloc);
  }

  map<pair<size_t, size_t>, size_t> range_lookup;
  vector<CodeRange> code_ranges;
  for(size_t i = 0; i < object.symbols.size(); ++i) {
    const Symbol & symbol = object.symbols[i];
    if(symbol.section != Symbol::SS_CODE ||
       symbol.binding == Symbol::SB_LOCAL ||
       symbol.size == 0) {
      continue;
    }
    const pair<size_t, size_t> key(symbol.offset, symbol.size);
    map<pair<size_t, size_t>, size_t>::const_iterator found = range_lookup.find(key);
    if(found == range_lookup.end()) {
      CodeRange range;
      range.original_offset = symbol.offset;
      range.size = symbol.size;
      range_lookup[key] = code_ranges.size();
      code_ranges.push_back(range);
      found = range_lookup.find(key);
    }
    CodeRange & range = code_ranges[found->second];
    if(symbol.binding == Symbol::SB_WEAK || !symbol.comdat_group.empty()) {
      range.weak = true;
      if(range.comdat_group.empty()) {
        range.comdat_group = symbol.comdat_group.empty() ? symbol.name : symbol.comdat_group;
      }
    }
  }

  sort(code_ranges.begin(), code_ranges.end(),
       [](const CodeRange & lhs, const CodeRange & rhs) {
         return lhs.original_offset < rhs.original_offset;
       });

  vector<CodeRange> complete_code_ranges;
  size_t next_offset = 0;
  for(size_t i = 0; i < code_ranges.size(); ++i) {
    if(code_ranges[i].original_offset > next_offset) {
      CodeRange filler;
      filler.original_offset = next_offset;
      filler.size = code_ranges[i].original_offset - next_offset;
      complete_code_ranges.push_back(filler);
    }
    complete_code_ranges.push_back(code_ranges[i]);
    const size_t range_end = code_ranges[i].original_offset + code_ranges[i].size;
    if(range_end > next_offset) {
      next_offset = range_end;
    }
  }
  if(next_offset < object.code.size()) {
    CodeRange filler;
    filler.original_offset = next_offset;
    filler.size = object.code.size() - next_offset;
    complete_code_ranges.push_back(filler);
  }
  if(!complete_code_ranges.empty()) {
    code_ranges.swap(complete_code_ranges);
  }

  const auto find_code_range =
      [&](size_t offset) -> vector<CodeRange>::iterator
      {
        for(vector<CodeRange>::iterator it = code_ranges.begin();
            it != code_ranges.end();
            ++it) {
          if(offset >= it->original_offset &&
             offset < it->original_offset + it->size) {
            return it;
          }
        }
        return code_ranges.end();
      };

  vector<unsigned char> shared_text;
  for(size_t i = 0; i < code_ranges.size(); ++i) {
    CodeRange & range = code_ranges[i];
    if(range.original_offset + range.size > object.code.size()) {
      throw logic_error("invalid function range in ELF object writer");
    }
    range.bytes.assign(object.code.begin() + range.original_offset,
                       object.code.begin() + range.original_offset + range.size);
    if(range.weak) {
      const string suffix =
          symbol_linkage::mangle_symbol_name(range.comdat_group.empty() ?
                                                 ("fn_" + to_string(i)) :
                                                 range.comdat_group);
      range.section_name = ".text." + suffix;
    } else {
      range.shared_offset = shared_text.size();
      shared_text.insert(shared_text.end(), range.bytes.begin(), range.bytes.end());
    }
  }

  vector<Relocation> shared_text_relocs;
  vector<Relocation> data_relocs;
  for(size_t i = 0; i < object.relocations.size(); ++i) {
    const Relocation & reloc = object.relocations[i];
    if(reloc.section == Symbol::SS_DATA) {
      data_relocs.push_back(reloc);
      continue;
    }
    vector<CodeRange>::iterator range = find_code_range(reloc.offset);
    if(range == code_ranges.end()) {
      shared_text_relocs.push_back(reloc);
      continue;
    }
    Relocation adjusted = reloc;
    adjusted.offset -= range->original_offset;
    if(range->weak) {
      range->relocs.push_back(adjusted);
    } else {
      adjusted.offset += range->shared_offset;
      shared_text_relocs.push_back(adjusted);
    }
  }

  const auto resolve_shared_text_local_offset =
      [&](const string & name, size_t & out) -> bool
      {
        map<string, Symbol>::const_iterator found = symbol_by_name.find(name);
        if(found == symbol_by_name.end() ||
           found->second.section != Symbol::SS_CODE) {
          return false;
        }
        vector<CodeRange>::iterator range = find_code_range(found->second.offset);
        if(range == code_ranges.end()) {
          out = found->second.offset;
          return true;
        }
        if(range->weak) {
          return false;
        }
        out = range->shared_offset + (found->second.offset - range->original_offset);
        return true;
      };

  vector<Relocation> kept_shared_text_relocs;
  for(size_t i = 0; i < shared_text_relocs.size(); ++i) {
    const Relocation & reloc = shared_text_relocs[i];
    size_t target_offset = 0;
    if((reloc.kind == Relocation::RK_BRANCH32 ||
        reloc.kind == Relocation::RK_PCREL32) &&
       is_local_name.find(reloc.symbol) != is_local_name.end() &&
       resolve_shared_text_local_offset(reloc.symbol, target_offset)) {
      const int64_t disp = static_cast<int64_t>(target_offset) +
                           static_cast<int64_t>(reloc.addend) -
                           static_cast<int64_t>(reloc.offset) - 4;
      if(disp < static_cast<int64_t>(numeric_limits<int32_t>::min()) ||
         disp > static_cast<int64_t>(numeric_limits<int32_t>::max())) {
        throw logic_error("local ELF pcrel relocation out of range for " +
                          reloc.symbol);
      }
      overwrite_i32_le(shared_text, reloc.offset, static_cast<int32_t>(disp));
      continue;
    }
    kept_shared_text_relocs.push_back(reloc);
  }
  shared_text_relocs.swap(kept_shared_text_relocs);

  vector<EmitSection> sections;
  uint32_t text_index = 0;
  uint32_t rela_text_index = 0;
  {
    EmitSection section;
    section.name = ".text";
    section.type = SHT_PROGBITS;
    section.flags = SHF_ALLOC | SHF_EXECINSTR;
    section.bytes = shared_text;
    section.addralign = 16;
    sections.push_back(section);
    text_index = static_cast<uint32_t>(sections.size());
    if(!shared_text_relocs.empty()) {
      EmitSection rela;
      rela.name = ".rela.text";
      rela.type = SHT_RELA;
      rela.addralign = 8;
      rela.entsize = 24;
      sections.push_back(rela);
      rela_text_index = static_cast<uint32_t>(sections.size());
    }
  }

  const uint32_t data_index = static_cast<uint32_t>(sections.size() + 1);
  EmitSection data_section;
  data_section.name = ".data";
  data_section.type = SHT_PROGBITS;
  data_section.flags = SHF_ALLOC | SHF_WRITE;
  data_section.bytes = object.data;
  data_section.addralign = 16;
  sections.push_back(data_section);

  uint32_t rela_data_index = 0;
  if(!data_relocs.empty()) {
    EmitSection rela;
    rela.name = ".rela.data";
    rela.type = SHT_RELA;
    rela.addralign = 8;
    rela.entsize = 24;
    sections.push_back(rela);
    rela_data_index = static_cast<uint32_t>(sections.size());
  }

  uint32_t init_index = 0;
  uint32_t rela_init_index = 0;
  if(!init_relocs.empty()) {
    EmitSection init_array;
    init_array.name = ".init_array";
    init_array.type = SHT_INIT_ARRAY;
    init_array.flags = SHF_ALLOC | SHF_WRITE;
    init_array.bytes.assign(init_relocs.size() * 8, 0);
    init_array.addralign = 8;
    init_array.entsize = 8;
    sections.push_back(init_array);
    init_index = static_cast<uint32_t>(sections.size());

    EmitSection rela;
    rela.name = ".rela.init_array";
    rela.type = SHT_RELA;
    rela.addralign = 8;
    rela.entsize = 24;
    sections.push_back(rela);
    rela_init_index = static_cast<uint32_t>(sections.size());
  }

  uint32_t fini_index = 0;
  uint32_t rela_fini_index = 0;
  if(!fini_relocs.empty()) {
    EmitSection fini_array;
    fini_array.name = ".fini_array";
    fini_array.type = SHT_FINI_ARRAY;
    fini_array.flags = SHF_ALLOC | SHF_WRITE;
    fini_array.bytes.assign(fini_relocs.size() * 8, 0);
    fini_array.addralign = 8;
    fini_array.entsize = 8;
    sections.push_back(fini_array);
    fini_index = static_cast<uint32_t>(sections.size());

    EmitSection rela;
    rela.name = ".rela.fini_array";
    rela.type = SHT_RELA;
    rela.addralign = 8;
    rela.entsize = 24;
    sections.push_back(rela);
    rela_fini_index = static_cast<uint32_t>(sections.size());
  }

  vector<uint32_t> extra_section_indices(object.extra_sections.size(), 0);
  vector<uint32_t> extra_rela_indices(object.extra_sections.size(), 0);
  map<string, uint32_t> extra_index_by_key;
  const auto configure_elf_extra_section =
      [&](EmitSection & section, const ExtraSection & extra) {
        section.name = extra.section_name;
        section.type = SHT_PROGBITS;
        section.flags = SHF_ALLOC;
        section.addralign =
            extra.macho_align_pow2 == 0 ? 1U : (1ULL << extra.macho_align_pow2);
        if(is_elf_debug_section_name(extra.section_name)) {
          section.flags = 0;
          section.addralign = 1;
        } else if(is_elf_tls_section_name(extra.section_name)) {
          section.flags = SHF_ALLOC | SHF_WRITE | SHF_TLS;
          if(extra.section_name == ".tbss") {
            section.type = SHT_NOBITS;
          }
        } else if(extra.section_name == ".eh_frame") {
          section.type = SHT_X86_64_UNWIND;
          section.addralign = 8;
        } else if(extra.section_name == ".gcc_except_table") {
          section.addralign = 4;
        } else if(extra.section_name.compare(0, 6, ".data.") == 0) {
          section.flags = SHF_ALLOC | SHF_WRITE;
          section.addralign = 8;
        }
      };
  for(size_t i = 0; i < object.extra_sections.size(); ++i) {
    EmitSection extra;
    configure_elf_extra_section(extra, object.extra_sections[i]);
    extra.bytes = object.extra_sections[i].bytes;
    sections.push_back(extra);
    extra_section_indices[i] = static_cast<uint32_t>(sections.size());
    extra_index_by_key[macho_extra_section_key(object.extra_sections[i].segment_name,
                                               object.extra_sections[i].section_name)] =
        extra_section_indices[i];
    if(!object.extra_sections[i].relocations.empty()) {
      EmitSection rela;
      rela.name = ".rela" + extra.name;
      rela.type = SHT_RELA;
      rela.addralign = 8;
      rela.entsize = 24;
      sections.push_back(rela);
      extra_rela_indices[i] = static_cast<uint32_t>(sections.size());
    }
  }

  for(size_t i = 0; i < code_ranges.size(); ++i) {
    CodeRange & range = code_ranges[i];
    if(!range.weak) {
      continue;
    }

    const string suffix =
        symbol_linkage::mangle_symbol_name(range.comdat_group.empty() ?
                                               ("fn_" + to_string(i)) :
                                               range.comdat_group);
    EmitSection group;
    group.name = ".group." + suffix;
    group.type = SHT_GROUP;
    group.addralign = 4;
    group.entsize = 4;
    sections.push_back(group);
    range.group_index = static_cast<uint32_t>(sections.size());

    EmitSection text;
    text.name = range.section_name;
    text.type = SHT_PROGBITS;
    text.flags = SHF_ALLOC | SHF_EXECINSTR | SHF_GROUP;
    text.bytes = range.bytes;
    text.addralign = 16;
    sections.push_back(text);
    range.text_index = static_cast<uint32_t>(sections.size());

    if(!range.relocs.empty()) {
      EmitSection rela;
      rela.name = ".rela" + range.section_name;
      rela.type = SHT_RELA;
      rela.flags = SHF_GROUP;
      rela.addralign = 8;
      rela.entsize = 24;
      sections.push_back(rela);
      range.rela_index = static_cast<uint32_t>(sections.size());
    }
  }

  EmitSection stack_note;
  stack_note.name = ".note.GNU-stack";
  stack_note.type = SHT_PROGBITS;
  stack_note.addralign = 1;
  sections.push_back(stack_note);

  EmitSection symtab_section;
  symtab_section.name = ".symtab";
  symtab_section.type = SHT_SYMTAB;
  symtab_section.addralign = 8;
  symtab_section.entsize = 24;
  sections.push_back(symtab_section);
  const uint32_t symtab_index = static_cast<uint32_t>(sections.size());

  EmitSection strtab_section;
  strtab_section.name = ".strtab";
  strtab_section.type = SHT_STRTAB;
  strtab_section.addralign = 1;
  sections.push_back(strtab_section);
  const uint32_t strtab_index = static_cast<uint32_t>(sections.size());

  EmitSection shstrtab_section;
  shstrtab_section.name = ".shstrtab";
  shstrtab_section.type = SHT_STRTAB;
  shstrtab_section.addralign = 1;
  sections.push_back(shstrtab_section);
  const uint32_t shstrtab_index = static_cast<uint32_t>(sections.size());

  map<string, uint32_t> symbol_strx;
  map<string, uint32_t> symbol_index;
  map<uint32_t, uint32_t> section_symbol_index_by_section_index;
  vector<unsigned char> symtab;
  append_elf_symbol(symtab, 0, 0, SHN_UNDEF, 0, 0);
  auto append_section_symbol =
      [&](uint32_t section_index) -> uint32_t
      {
        const uint32_t symbol_table_index =
            static_cast<uint32_t>(1 + section_symbol_index_by_section_index.size());
        append_elf_symbol(symtab,
                          0,
                          static_cast<uint8_t>((STB_LOCAL << 4) | STT_SECTION),
                          static_cast<uint16_t>(section_index),
                          0,
                          0);
        section_symbol_index_by_section_index[section_index] = symbol_table_index;
        return symbol_table_index;
      };
  if(text_index != 0) {
    append_section_symbol(text_index);
  }
  if(data_index != 0) {
    append_section_symbol(data_index);
  }
  for(size_t i = 0; i < code_ranges.size(); ++i) {
    if(code_ranges[i].weak && code_ranges[i].text_index != 0) {
      append_section_symbol(code_ranges[i].text_index);
    }
  }
  for(size_t i = 0; i < object.extra_sections.size(); ++i) {
    if(extra_section_indices[i] != 0) {
      append_section_symbol(extra_section_indices[i]);
    }
  }
  const uint32_t named_symbol_base =
      static_cast<uint32_t>(1 + section_symbol_index_by_section_index.size());
  vector<unsigned char> strtab(1, 0);
  map<string, string> serialized_local_names;
  for(size_t i = 0; i < local_names.size(); ++i) {
    if(!should_preserve_local_symbol_name(local_names[i])) {
      serialized_local_names[local_names[i]] = synthetic_local_symbol_name(i);
    }
  }
  vector<string> symbol_order;
  symbol_order.insert(symbol_order.end(), local_names.begin(), local_names.end());
  symbol_order.insert(symbol_order.end(), global_names.begin(), global_names.end());
  symbol_order.insert(symbol_order.end(), undefined_names.begin(), undefined_names.end());
  for(size_t i = 0; i < symbol_order.size(); ++i) {
    map<string, string>::const_iterator serialized_local =
        serialized_local_names.find(symbol_order[i]);
    const string encoded = encode_symbol_name(serialized_local == serialized_local_names.end()
                                                  ? symbol_order[i]
                                                  : serialized_local->second,
                                              object.target);
    symbol_strx[symbol_order[i]] = static_cast<uint32_t>(strtab.size());
    symbol_index[symbol_order[i]] = static_cast<uint32_t>(named_symbol_base + i);
    strtab.insert(strtab.end(), encoded.begin(), encoded.end());
    strtab.push_back(0);
  }

  const auto symbol_section_index =
      [&](const Symbol & symbol) -> uint16_t
      {
        if(symbol.section == Symbol::SS_DATA) {
          return data_index;
        }
        if(symbol.section == Symbol::SS_EXTRA) {
          map<string, uint32_t>::const_iterator found =
              extra_index_by_key.find(symbol.extra_section);
          if(found == extra_index_by_key.end()) {
            throw logic_error("missing ELF extra section for symbol " + symbol.name);
          }
          return static_cast<uint16_t>(found->second);
        }
        if(code_ranges.empty()) {
          return text_index;
        }
        for(size_t i = 0; i < code_ranges.size(); ++i) {
          const CodeRange & range = code_ranges[i];
          if(symbol.offset >= range.original_offset &&
             symbol.offset < range.original_offset + range.size) {
            return range.weak ? static_cast<uint16_t>(range.text_index)
                              : static_cast<uint16_t>(text_index);
          }
        }
        return text_index;
      };

  const auto symbol_section_value =
      [&](const Symbol & symbol) -> uint64_t
      {
        if(symbol.section == Symbol::SS_DATA) {
          return symbol.offset;
        }
        if(symbol.section == Symbol::SS_EXTRA) {
          return symbol.offset;
        }
        if(code_ranges.empty()) {
          return symbol.offset;
        }
        for(size_t i = 0; i < code_ranges.size(); ++i) {
          const CodeRange & range = code_ranges[i];
          if(symbol.offset >= range.original_offset &&
             symbol.offset < range.original_offset + range.size) {
            if(range.weak) {
              return static_cast<uint64_t>(symbol.offset - range.original_offset);
            }
            return static_cast<uint64_t>(range.shared_offset +
                                         (symbol.offset - range.original_offset));
          }
        }
        return symbol.offset;
      };

  const auto symbol_is_tls =
      [&](const Symbol & symbol) -> bool
      {
        if(symbol.section != Symbol::SS_EXTRA) {
          return false;
        }
        map<string, uint32_t>::const_iterator found =
            extra_index_by_key.find(symbol.extra_section);
        if(found == extra_index_by_key.end()) {
          throw logic_error("missing ELF extra section for TLS query on symbol " + symbol.name);
        }
        return (sections[found->second - 1].flags & SHF_TLS) != 0;
      };

  for(size_t i = 0; i < local_names.size(); ++i) {
    const string & name = local_names[i];
    map<string, Symbol>::const_iterator found = symbol_by_name.find(name);
    if(found == symbol_by_name.end()) {
      throw logic_error("missing local symbol " + name);
    }
    const Symbol & symbol = found->second;
    const uint8_t type =
        symbol.size == 0
            ? STT_NOTYPE
            : symbol.section == Symbol::SS_CODE
                ? STT_FUNC
                : symbol.section == Symbol::SS_DATA
                    ? STT_OBJECT
                    : symbol_is_tls(symbol)
                        ? STT_TLS
                        : STT_NOTYPE;
    append_elf_symbol(symtab,
                      symbol_strx.find(name)->second,
                      static_cast<uint8_t>((STB_LOCAL << 4) | type),
                      symbol_section_index(symbol),
                      symbol_section_value(symbol),
                      symbol.size);
  }
  for(size_t i = 0; i < global_names.size(); ++i) {
    const string & name = global_names[i];
    map<string, Symbol>::const_iterator found = symbol_by_name.find(name);
    if(found == symbol_by_name.end()) {
      throw logic_error("missing global symbol " + name);
    }
    const Symbol & symbol = found->second;
    const uint8_t bind = symbol.binding == Symbol::SB_WEAK ? STB_WEAK : STB_GLOBAL;
    const uint8_t type = symbol.section == Symbol::SS_CODE
        ? STT_FUNC
        : symbol.section == Symbol::SS_DATA
            ? STT_OBJECT
            : symbol_is_tls(symbol)
                ? STT_TLS
                : STT_NOTYPE;
    append_elf_symbol(symtab,
                      symbol_strx.find(name)->second,
                      static_cast<uint8_t>((bind << 4) | type),
                      symbol_section_index(symbol),
                      symbol_section_value(symbol),
                      symbol.size);
  }
  for(size_t i = 0; i < undefined_names.size(); ++i) {
    const string & name = undefined_names[i];
    const uint8_t bind =
        undefined_bindings.find(name) != undefined_bindings.end() &&
            undefined_bindings.find(name)->second == Symbol::SB_WEAK ?
                STB_WEAK :
                STB_GLOBAL;
    const uint8_t type = undefined_tls_names.count(name) != 0 ? STT_TLS : STT_NOTYPE;
    append_elf_symbol(symtab,
                      symbol_strx.find(name)->second,
                      static_cast<uint8_t>((bind << 4) | type),
                      SHN_UNDEF,
                      0,
                      0);
  }

  const auto build_rela_bytes =
      [&](const vector<Relocation> & relocs) -> vector<unsigned char>
      {
        vector<unsigned char> out;
        for(size_t i = 0; i < relocs.size(); ++i) {
          const bool branch_to_local =
              relocs[i].kind == Relocation::RK_BRANCH32 &&
              is_local_name.find(relocs[i].symbol) != is_local_name.end();
          const uint64_t type = relocs[i].kind == Relocation::RK_BRANCH32
              ? (branch_to_local ? R_X86_64_PC32 : R_X86_64_PLT32)
              : relocs[i].kind == Relocation::RK_PCREL32
                  ? R_X86_64_PC32
                  : relocs[i].kind == Relocation::RK_INDIRECT_REL32
                      ? R_X86_64_GOTPCREL
                  : relocs[i].kind == Relocation::RK_TLV_REL32
                      ? R_X86_64_GOTPCREL
                  : relocs[i].kind == Relocation::RK_TPOFF32
                      ? R_X86_64_TPOFF32
                  : R_X86_64_64;
          const int64_t addend =
              (relocs[i].kind == Relocation::RK_BRANCH32 ||
               relocs[i].kind == Relocation::RK_PCREL32 ||
               relocs[i].kind == Relocation::RK_INDIRECT_REL32 ||
               relocs[i].kind == Relocation::RK_TLV_REL32) ? -4 : relocs[i].addend;
          const uint64_t info =
              (static_cast<uint64_t>(symbol_index.find(relocs[i].symbol)->second) << 32) | type;
          append_elf_rela(out, relocs[i].offset, info, addend);
        }
        return out;
      };

  const auto build_extra_rela_bytes =
      [&](const ExtraSection & section) -> vector<unsigned char>
      {
        vector<unsigned char> out;
        vector<ExtraRelocation> relocs = section.relocations;
        stable_sort(relocs.begin(),
                    relocs.end(),
                    [](const ExtraRelocation & lhs, const ExtraRelocation & rhs) {
                      return lhs.offset < rhs.offset;
                    });
        const auto code_target =
            [&](long long original_offset) -> pair<uint32_t, int64_t>
            {
              if(code_ranges.empty()) {
                return make_pair(section_symbol_index_by_section_index.find(text_index)->second,
                                 static_cast<int64_t>(original_offset));
              }
              for(size_t i = 0; i < code_ranges.size(); ++i) {
                const CodeRange & range = code_ranges[i];
                if(static_cast<size_t>(original_offset) >= range.original_offset &&
                   static_cast<size_t>(original_offset) <
                       range.original_offset + range.size) {
                  if(range.weak) {
                    return make_pair(
                        section_symbol_index_by_section_index.find(range.text_index)->second,
                        static_cast<int64_t>(original_offset - range.original_offset));
                  }
                  return make_pair(
                      section_symbol_index_by_section_index.find(text_index)->second,
                      static_cast<int64_t>(range.shared_offset +
                                           (original_offset - range.original_offset)));
                }
              }
              return make_pair(section_symbol_index_by_section_index.find(text_index)->second,
                               static_cast<int64_t>(original_offset));
            };
        for(size_t i = 0; i < relocs.size(); ++i) {
          const ExtraRelocation & reloc = relocs[i];
          uint32_t target_symbol_index = 0;
          uint64_t type = R_X86_64_64;
          int64_t addend = reloc.addend;
          if(reloc.kind == ExtraRelocation::RK_BRANCH32) {
            type = R_X86_64_PLT32;
          } else if(reloc.kind == ExtraRelocation::RK_PCREL32) {
            type = R_X86_64_PC32;
          } else if(reloc.kind == ExtraRelocation::RK_ABS64) {
            type = R_X86_64_64;
          } else {
            throw logic_error("unsupported ELF extra relocation kind");
          }
          if(reloc.target_kind == ExtraRelocation::TK_SYMBOL) {
            map<string, uint32_t>::const_iterator symbol =
                symbol_index.find(reloc.symbol);
            if(symbol == symbol_index.end()) {
              throw logic_error("missing ELF extra relocation symbol " + reloc.symbol);
            }
            target_symbol_index = symbol->second;
          } else if(reloc.target_kind == ExtraRelocation::TK_CODE) {
            pair<uint32_t, int64_t> target = code_target(reloc.addend);
            target_symbol_index = target.first;
            addend = target.second;
          } else if(reloc.target_kind == ExtraRelocation::TK_DATA) {
            target_symbol_index = section_symbol_index_by_section_index.find(data_index)->second;
          } else if(reloc.target_kind == ExtraRelocation::TK_EXTRA) {
            map<string, uint32_t>::const_iterator extra_section =
                extra_index_by_key.find(reloc.target_extra_section);
            if(extra_section == extra_index_by_key.end()) {
              throw logic_error("missing ELF extra relocation section " +
                                reloc.target_extra_section);
            }
            target_symbol_index =
                section_symbol_index_by_section_index.find(extra_section->second)->second;
          } else {
            throw logic_error("unsupported ELF extra relocation target kind");
          }
          const uint64_t info =
              (static_cast<uint64_t>(target_symbol_index) << 32) | type;
          append_elf_rela(out, reloc.offset, info, addend);
        }
        return out;
      };

  if(rela_text_index != 0) {
    sections[rela_text_index - 1].bytes = build_rela_bytes(shared_text_relocs);
    sections[rela_text_index - 1].link = symtab_index;
    sections[rela_text_index - 1].info = text_index;
  }
  if(rela_data_index != 0) {
    sections[rela_data_index - 1].bytes = build_rela_bytes(data_relocs);
    sections[rela_data_index - 1].link = symtab_index;
    sections[rela_data_index - 1].info = data_index;
  }
  if(rela_init_index != 0) {
    sections[rela_init_index - 1].bytes = build_rela_bytes(init_relocs);
    sections[rela_init_index - 1].link = symtab_index;
    sections[rela_init_index - 1].info = init_index;
  }
  if(rela_fini_index != 0) {
    sections[rela_fini_index - 1].bytes = build_rela_bytes(fini_relocs);
    sections[rela_fini_index - 1].link = symtab_index;
    sections[rela_fini_index - 1].info = fini_index;
  }
  for(size_t i = 0; i < object.extra_sections.size(); ++i) {
    if(extra_rela_indices[i] == 0) {
      continue;
    }
    sections[extra_rela_indices[i] - 1].bytes =
        build_extra_rela_bytes(object.extra_sections[i]);
    sections[extra_rela_indices[i] - 1].link = symtab_index;
    sections[extra_rela_indices[i] - 1].info = extra_section_indices[i];
  }
  for(size_t i = 0; i < code_ranges.size(); ++i) {
    CodeRange & range = code_ranges[i];
    if(!range.weak) {
      continue;
    }
    if(range.rela_index != 0) {
      sections[range.rela_index - 1].bytes = build_rela_bytes(range.relocs);
      sections[range.rela_index - 1].link = symtab_index;
      sections[range.rela_index - 1].info = range.text_index;
    }
    vector<unsigned char> group_bytes;
    append_u32(group_bytes, GRP_COMDAT);
    append_u32(group_bytes, range.text_index);
    if(range.rela_index != 0) {
      append_u32(group_bytes, range.rela_index);
    }
    sections[range.group_index - 1].bytes = group_bytes;
    sections[range.group_index - 1].link = symtab_index;
    map<string, uint32_t>::const_iterator signature =
        symbol_index.find(range.comdat_group);
    if(signature == symbol_index.end()) {
      throw logic_error("missing COMDAT signature symbol " + range.comdat_group);
    }
    sections[range.group_index - 1].info = signature->second;
  }

  sections[symtab_index - 1].bytes = symtab;
  sections[symtab_index - 1].link = strtab_index;
  sections[symtab_index - 1].info =
      static_cast<uint32_t>(named_symbol_base + local_names.size());

  sections[strtab_index - 1].bytes = strtab;

  map<string, uint32_t> sh_name_offset;
  vector<unsigned char> shstrtab(1, 0);
  for(size_t i = 0; i < sections.size(); ++i) {
    sh_name_offset[sections[i].name] = static_cast<uint32_t>(shstrtab.size());
    shstrtab.insert(shstrtab.end(), sections[i].name.begin(), sections[i].name.end());
    shstrtab.push_back(0);
  }
  sections[shstrtab_index - 1].bytes = shstrtab;

  size_t offset = 64;
  vector<uint64_t> section_offsets(sections.size(), 0);
  for(size_t i = 0; i < sections.size(); ++i) {
    offset = align_up(offset, sections[i].addralign);
    section_offsets[i] = offset;
    offset += sections[i].bytes.size();
  }
  offset = align_up(offset, 8);
  const uint64_t shoff = offset;
  const uint16_t shnum = static_cast<uint16_t>(sections.size() + 1);

  vector<unsigned char> out;
  append_elf_header(out, shoff, shnum, shstrtab_index);
  for(size_t i = 0; i < sections.size(); ++i) {
    out.resize(section_offsets[i], 0);
    out.insert(out.end(), sections[i].bytes.begin(), sections[i].bytes.end());
  }
  out.resize(shoff, 0);

  append_elf_section_header(out, 0, SHT_NULL, 0, 0, 0, 0, 0, 0, 0);
  for(size_t i = 0; i < sections.size(); ++i) {
    append_elf_section_header(out,
                              sh_name_offset.find(sections[i].name)->second,
                              sections[i].type,
                              sections[i].flags,
                              section_offsets[i],
                              sections[i].bytes.size(),
                              sections[i].link,
                              sections[i].info,
                              sections[i].addralign,
                              sections[i].entsize);
  }

  ofstream file(path.c_str(), ios::binary | ios::trunc);
  if(!file) {
    throw logic_error("unable to open object output file: " + path);
  }
  file.write(reinterpret_cast<const char *>(&out[0]),
             static_cast<streamsize>(out.size()));
  file.close();
  if(!file) {
    throw logic_error("unable to write object output file: " + path);
  }
}

ObjectFile parse_elf_object_bytes(const vector<unsigned char> & bytes,
                                  const string & path)
{
  if(bytes.size() < 64 ||
     bytes[0] != ELFMAG0 || bytes[1] != ELFMAG1 ||
     bytes[2] != ELFMAG2 || bytes[3] != ELFMAG3 ||
     bytes[4] != ELFCLASS64 || bytes[5] != ELFDATA2LSB ||
     read_u16(bytes, 16) != ET_REL || read_u16(bytes, 18) != EM_X86_64) {
    throw logic_error("unsupported ELF object " + path);
  }

  const uint64_t shoff = read_u64(bytes, 40);
  const uint16_t shentsize = read_u16(bytes, 58);
  const uint16_t shnum = read_u16(bytes, 60);
  const uint16_t shstrndx = read_u16(bytes, 62);
  if(shentsize != 64 || shoff + static_cast<uint64_t>(shentsize) * shnum > bytes.size()) {
    throw logic_error("invalid ELF section table in " + path);
  }

  vector<ElfSectionInfo> sections(shnum);
  for(uint16_t i = 0; i < shnum; ++i) {
    const size_t offset = static_cast<size_t>(shoff + static_cast<uint64_t>(i) * shentsize);
    sections[i].index = i;
    sections[i].type = read_u32(bytes, offset + 4);
    sections[i].flags = read_u64(bytes, offset + 8);
    sections[i].offset = read_u64(bytes, offset + 24);
    sections[i].size = read_u64(bytes, offset + 32);
    sections[i].link = read_u32(bytes, offset + 40);
    sections[i].info = read_u32(bytes, offset + 44);
    sections[i].addralign = read_u64(bytes, offset + 48);
    sections[i].entsize = read_u64(bytes, offset + 56);
    sections[i].name = to_string(read_u32(bytes, offset + 0));
  }

  if(shstrndx >= sections.size()) {
    throw logic_error("invalid ELF shstrtab index in " + path);
  }
  const ElfSectionInfo & shstr = sections[shstrndx];
  for(size_t i = 0; i < sections.size(); ++i) {
    const uint32_t name_off = static_cast<uint32_t>(strtoul(sections[i].name.c_str(), NULL, 10));
    sections[i].name = read_c_string(bytes,
                                     static_cast<size_t>(shstr.offset + name_off),
                                     static_cast<size_t>(shstr.offset + shstr.size));
  }

  vector<int> code_section_indices;
  vector<int> data_section_indices;
  vector<int> extra_section_indices;
  int symtab_index = -1;
  int strtab_index = -1;
  vector<int> reloc_section_indices;
  for(size_t i = 0; i < sections.size(); ++i) {
    if((sections[i].type == SHT_PROGBITS || sections[i].type == SHT_NOBITS) &&
       (sections[i].flags & SHF_TLS) != 0) {
      extra_section_indices.push_back(static_cast<int>(i));
    } else if(sections[i].type == SHT_PROGBITS &&
       (sections[i].flags & (SHF_ALLOC | SHF_EXECINSTR)) ==
           (SHF_ALLOC | SHF_EXECINSTR)) {
      code_section_indices.push_back(static_cast<int>(i));
    } else if(sections[i].type == SHT_PROGBITS &&
              (sections[i].flags & (SHF_ALLOC | SHF_WRITE)) ==
                  (SHF_ALLOC | SHF_WRITE)) {
      data_section_indices.push_back(static_cast<int>(i));
    } else if(sections[i].name == ".symtab") {
      symtab_index = static_cast<int>(i);
    } else if(sections[i].name == ".strtab") {
      strtab_index = static_cast<int>(i);
    } else if(sections[i].type == SHT_RELA) {
      reloc_section_indices.push_back(static_cast<int>(i));
    } else if(sections[i].type == SHT_PROGBITS ||
              sections[i].type == SHT_NOBITS ||
              sections[i].type == SHT_X86_64_UNWIND) {
      extra_section_indices.push_back(static_cast<int>(i));
    }
  }
  if(symtab_index < 0 || strtab_index < 0) {
    throw logic_error("missing ELF sections in " + path);
  }

  ObjectFile object;
  object.target = "linux";
  map<int, size_t> code_section_base;
  for(size_t i = 0; i < code_section_indices.size(); ++i) {
    const ElfSectionInfo & text = sections[code_section_indices[i]];
    if(text.offset + text.size > bytes.size()) {
      throw logic_error("truncated ELF code section in " + path);
    }
    code_section_base[code_section_indices[i]] = object.code.size();
    object.code.insert(object.code.end(),
                       bytes.begin() + text.offset,
                       bytes.begin() + text.offset + text.size);
  }
  map<int, size_t> data_section_base;
  for(size_t i = 0; i < data_section_indices.size(); ++i) {
    const ElfSectionInfo & data = sections[data_section_indices[i]];
    if(data.offset + data.size > bytes.size()) {
      throw logic_error("truncated ELF data section in " + path);
    }
    data_section_base[data_section_indices[i]] = object.data.size();
    object.data.insert(object.data.end(),
                       bytes.begin() + data.offset,
                       bytes.begin() + data.offset + data.size);
  }
  map<int, size_t> extra_section_by_index;
  for(size_t i = 0; i < extra_section_indices.size(); ++i) {
    const ElfSectionInfo & extra_info = sections[extra_section_indices[i]];
    ExtraSection extra;
    extra.segment_name = ".elf";
    extra.section_name = extra_info.name;
    uint32_t align_pow2 = 0;
    uint64_t align = extra_info.addralign == 0 ? 1 : extra_info.addralign;
    while((uint64_t(1) << align_pow2) < align) {
      ++align_pow2;
    }
    extra.macho_align_pow2 = align_pow2;
    if(extra_info.type == SHT_NOBITS) {
      extra.bytes.assign(static_cast<size_t>(extra_info.size), 0);
    } else {
      if(extra_info.offset + extra_info.size > bytes.size()) {
        throw logic_error("truncated ELF extra section in " + path);
      }
      extra.bytes.insert(extra.bytes.end(),
                         bytes.begin() + extra_info.offset,
                         bytes.begin() + extra_info.offset + extra_info.size);
    }
    extra_section_by_index[extra_section_indices[i]] = object.extra_sections.size();
    object.extra_sections.push_back(extra);
  }

  const ElfSectionInfo & symtab = sections[symtab_index];
  const ElfSectionInfo & strtab = sections[strtab_index];
  if(symtab.entsize != 24 || symtab.offset + symtab.size > bytes.size() ||
     strtab.offset + strtab.size > bytes.size()) {
    throw logic_error("invalid ELF symbol tables in " + path);
  }

  vector<ElfSymbolInfo> symbols(symtab.size / symtab.entsize);
  for(size_t i = 0; i < symbols.size(); ++i) {
    const size_t offset = static_cast<size_t>(symtab.offset + i * symtab.entsize);
    const uint32_t name_off = read_u32(bytes, offset + 0);
    symbols[i].name = name_off == 0 ? string() :
        decode_symbol_name(read_c_string(bytes,
                                         static_cast<size_t>(strtab.offset + name_off),
                                         static_cast<size_t>(strtab.offset + strtab.size)),
                           "linux");
    const uint8_t info = bytes[offset + 4];
    symbols[i].bind = static_cast<uint8_t>(info >> 4);
    symbols[i].type = static_cast<uint8_t>(info & 0xF);
    symbols[i].shndx = read_u16(bytes, offset + 6);
    symbols[i].value = read_u64(bytes, offset + 8);
    symbols[i].size = read_u64(bytes, offset + 16);
  }

  for(size_t i = 1; i < symbols.size(); ++i) {
    const ElfSymbolInfo & elf_symbol = symbols[i];
    if(elf_symbol.name.empty() || elf_symbol.shndx == SHN_UNDEF) {
      continue;
    }
    Symbol symbol;
    if(elf_symbol.bind == STB_LOCAL) {
      symbol.binding = Symbol::SB_LOCAL;
    } else if(elf_symbol.bind == STB_WEAK) {
      symbol.binding = Symbol::SB_WEAK;
    } else {
      symbol.binding = Symbol::SB_GLOBAL;
    }
    map<int, size_t>::const_iterator code_base =
        code_section_base.find(static_cast<int>(elf_symbol.shndx));
    map<int, size_t>::const_iterator data_base =
        data_section_base.find(static_cast<int>(elf_symbol.shndx));
    if(code_base != code_section_base.end()) {
      symbol.section = Symbol::SS_CODE;
      symbol.offset = static_cast<size_t>(code_base->second + elf_symbol.value);
    } else if(data_base != data_section_base.end()) {
      symbol.section = Symbol::SS_DATA;
      symbol.offset = static_cast<size_t>(data_base->second + elf_symbol.value);
    } else {
      map<int, size_t>::const_iterator extra_found =
          extra_section_by_index.find(static_cast<int>(elf_symbol.shndx));
      if(extra_found == extra_section_by_index.end()) {
        continue;
      }
      symbol.section = Symbol::SS_EXTRA;
      const ExtraSection & extra = object.extra_sections[extra_found->second];
      symbol.extra_section = macho_extra_section_key(extra.segment_name, extra.section_name);
      symbol.offset = static_cast<size_t>(elf_symbol.value);
      symbol.name = elf_symbol.name;
      symbol.size = static_cast<size_t>(elf_symbol.size);
      object.symbols.push_back(symbol);
      continue;
    }
    symbol.name = elf_symbol.name;
    symbol.size = static_cast<size_t>(elf_symbol.size);
    object.symbols.push_back(symbol);
  }

  for(size_t rsi = 0; rsi < reloc_section_indices.size(); ++rsi) {
    const ElfSectionInfo & relsec = sections[reloc_section_indices[rsi]];
    if(relsec.entsize != 24 || relsec.offset + relsec.size > bytes.size()) {
      throw logic_error("invalid ELF relocation section in " + path);
    }
    Symbol::Section reloc_section;
    map<int, size_t>::const_iterator code_base =
        code_section_base.find(static_cast<int>(relsec.info));
    map<int, size_t>::const_iterator data_base =
        data_section_base.find(static_cast<int>(relsec.info));
    if(code_base != code_section_base.end()) {
      reloc_section = Symbol::SS_CODE;
    } else if(data_base != data_section_base.end()) {
      reloc_section = Symbol::SS_DATA;
    } else {
      // Host-produced objects commonly include relocation-bearing metadata sections such as
      // .eh_frame. Our object model only preserves code/data, so those auxiliary relocation
      // sections can be ignored safely.
      continue;
    }
    const size_t reloc_count = static_cast<size_t>(relsec.size / relsec.entsize);
    for(size_t i = 0; i < reloc_count; ++i) {
      const size_t offset = static_cast<size_t>(relsec.offset + i * relsec.entsize);
      const uint64_t r_offset = read_u64(bytes, offset + 0);
      const uint64_t r_info = read_u64(bytes, offset + 8);
      const int64_t r_addend = static_cast<int64_t>(read_u64(bytes, offset + 16));
      const uint32_t sym_index = static_cast<uint32_t>(r_info >> 32);
      const uint32_t type = static_cast<uint32_t>(r_info & 0xFFFFFFFFU);
      if(sym_index >= symbols.size()) {
        throw logic_error("invalid ELF relocation symbol index in " + path);
      }
      if(symbols[sym_index].name.empty()) {
        throw logic_error("unsupported ELF relocation target in " + path);
      }
      Relocation reloc;
      reloc.section = reloc_section;
      if(reloc_section == Symbol::SS_CODE) {
        reloc.offset = static_cast<size_t>(code_base->second + r_offset);
      } else {
        reloc.offset = static_cast<size_t>(data_base->second + r_offset);
      }
      if(type == R_X86_64_64) {
        reloc.kind = Relocation::RK_ABS64;
        reloc.addend = r_addend;
      } else if(type == R_X86_64_PC32 ||
                type == R_X86_64_PLT32 ||
                type == R_X86_64_GOTPCREL) {
        if(r_addend != -4) {
          throw logic_error("unsupported ELF rel32 addend in " + path);
        }
        reloc.kind = type == R_X86_64_PLT32
            ? Relocation::RK_BRANCH32
            : type == R_X86_64_GOTPCREL
                ? Relocation::RK_INDIRECT_REL32
                : Relocation::RK_PCREL32;
      } else if(type == R_X86_64_TPOFF32) {
        reloc.kind = Relocation::RK_TPOFF32;
        reloc.addend = r_addend;
      } else {
        throw logic_error("unsupported ELF relocation kind in " + path);
      }
      reloc.symbol = symbols[sym_index].name;
      object.relocations.push_back(reloc);
    }
  }

  return object;
}

}  // namespace

void write_object_file(const string & path, const ObjectFile & object)
{
  if(object.target == "macos") {
    write_macho_object_file(path, object);
  } else if(object.target == "linux") {
    write_elf_object_file(path, object);
  } else {
    throw logic_error("unknown object target: " + object.target);
  }
  write_local_symbol_map_file(path, object);
}

ObjectFile parse_object_file(const string & path)
{
  ifstream in(path.c_str(), ios::binary);
  if(!in) {
    throw logic_error("unable to open object file " + path);
  }
  vector<unsigned char> bytes((istreambuf_iterator<char>(in)),
                              istreambuf_iterator<char>());
  if(bytes.size() >= 4 && read_u32(bytes, 0) == MH_MAGIC_64) {
    return parse_macho_object_bytes(bytes, path);
  }
  if(bytes.size() >= 4 &&
     bytes[0] == ELFMAG0 && bytes[1] == ELFMAG1 &&
     bytes[2] == ELFMAG2 && bytes[3] == ELFMAG3) {
    return parse_elf_object_bytes(bytes, path);
  }

  throw logic_error("unsupported object file format in " + path);
}

}  // namespace machine_object
