#include "lowir_object_backend.h"

#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <cstdint>
#include <fstream>
#include <map>
#include <set>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

using namespace std;

#include "cy86_internal.h"
#include "eh_runtime.h"
#include "host_eh_object_sections.h"
#include "lowir_machine_ir.h"
#include "machine_ir_optimizer.h"
#include "native_format.h"
#include "parser_trace.h"
#include "runtime_symbol_policy.h"
#include "symbol_linkage.h"

namespace {

namespace mir = machine_ir;
namespace mobj = machine_object;
namespace hosteh = host_eh_object_sections;

const string kHostEhAllocateExceptionSymbol =
    symbol_linkage::internal_symbol_from_name("__external_runtime::__cxa_allocate_exception");
const string kHostEhAllocateExceptionObjectSymbol = "___cxa_allocate_exception";

const uint32_t MACHO_STATIC_INIT_FLAGS = 0x80000400;
const uint32_t MACHO_DEBUG_SECTION_FLAGS = 0x02000000;

const uint16_t DWARF_VERSION_4 = 4;
const uint8_t DW_CHILDREN_NO = 0;
const uint8_t DW_CHILDREN_YES = 1;
const uint16_t DW_TAG_FORMAL_PARAMETER = 0x05;
const uint16_t DW_TAG_BASE_TYPE = 0x24;
const uint16_t DW_TAG_COMPILE_UNIT = 0x11;
const uint16_t DW_TAG_SUBPROGRAM = 0x2e;
const uint16_t DW_TAG_VARIABLE = 0x34;
const uint16_t DW_AT_LOCATION = 0x02;
const uint16_t DW_AT_NAME = 0x03;
const uint16_t DW_AT_BYTE_SIZE = 0x0b;
const uint16_t DW_AT_STMT_LIST = 0x10;
const uint16_t DW_AT_LOW_PC = 0x11;
const uint16_t DW_AT_HIGH_PC = 0x12;
const uint16_t DW_AT_LANGUAGE = 0x13;
const uint16_t DW_AT_COMP_DIR = 0x1b;
const uint16_t DW_AT_PRODUCER = 0x25;
const uint16_t DW_AT_DECL_FILE = 0x3a;
const uint16_t DW_AT_DECL_LINE = 0x3b;
const uint16_t DW_AT_ENCODING = 0x3e;
const uint16_t DW_AT_TYPE = 0x49;
const uint16_t DW_FORM_ADDR = 0x01;
const uint16_t DW_FORM_DATA1 = 0x0b;
const uint16_t DW_FORM_DATA2 = 0x05;
const uint16_t DW_FORM_DATA4 = 0x06;
const uint16_t DW_FORM_DATA8 = 0x07;
const uint16_t DW_FORM_REF4 = 0x13;
const uint16_t DW_FORM_STRING = 0x08;
const uint16_t DW_FORM_EXPRLOC = 0x18;
const uint16_t DW_FORM_SEC_OFFSET = 0x17;
const uint16_t DW_LANG_C_PLUS_PLUS = 0x0004;
const uint8_t DW_ATE_ADDRESS = 0x01;
const uint8_t DW_ATE_FLOAT = 0x04;
const uint8_t DW_ATE_SIGNED = 0x05;
const uint8_t DW_ATE_UNSIGNED = 0x07;
const uint8_t DW_LNS_COPY = 1;
const uint8_t DW_LNS_ADVANCE_LINE = 3;
const uint8_t DW_LNS_SET_FILE = 4;
const uint8_t DW_LNS_SET_COLUMN = 5;
const uint8_t DW_LNE_END_SEQUENCE = 1;
const uint8_t DW_LNE_SET_ADDRESS = 2;
const uint8_t DW_OP_BREG6 = 0x76;
const uint8_t DW_OP_REG0 = 0x50;

struct DwarfFileEntry
{
  string path;
  string directory;
  string name;
  size_t directory_index = 0;
};

struct DwarfFileCatalog
{
  vector<string> directories;
  map<string, size_t> directory_index;
  vector<DwarfFileEntry> files;
  map<string, size_t> file_index;
};

struct DwarfLineRow
{
  size_t file_index = 0;
  size_t line = 0;
  size_t column = 0;
  size_t address_addend = 0;
};

struct DwarfFunctionInfo
{
  struct VariableLocation
  {
    enum Kind
    {
      LK_FRAME,
      LK_REG,
      LK_XMM
    } kind = LK_FRAME;

    size_t low_pc_offset = 0;
    size_t high_pc_offset = 0;
    long long frame_offset = 0;
    uint16_t dwarf_register = 0;
  };

  string display_name;
  string return_type;
  size_t file_index = 0;
  size_t decl_line = 0;
  size_t low_pc_offset = 0;
  size_t size = 0;
  vector<DwarfLineRow> rows;
  struct Variable
  {
    string name;
    string type;
    long long frame_offset = 0;
    vector<VariableLocation> locations;
    size_t location_list_offset = 0;

    bool uses_location_list() const
    {
      return !locations.empty();
    }
  };
  vector<Variable> parameters;
  vector<Variable> locals;
};

struct DwarfCompilationUnitInfo
{
  DwarfFileCatalog files;
  string unit_name;
  string unit_directory;
  size_t low_pc_offset = 0;
  size_t high_pc_end = 0;
  vector<DwarfFunctionInfo> functions;

  bool empty() const
  {
    return functions.empty();
  }
};

void append_u8(vector<unsigned char> & out, uint8_t value)
{
  out.push_back(value);
}

void append_u16(vector<unsigned char> & out, uint16_t value)
{
  out.push_back(static_cast<unsigned char>(value & 0xFFu));
  out.push_back(static_cast<unsigned char>((value >> 8) & 0xFFu));
}

void append_u32(vector<unsigned char> & out, uint32_t value)
{
  out.push_back(static_cast<unsigned char>(value & 0xFFu));
  out.push_back(static_cast<unsigned char>((value >> 8) & 0xFFu));
  out.push_back(static_cast<unsigned char>((value >> 16) & 0xFFu));
  out.push_back(static_cast<unsigned char>((value >> 24) & 0xFFu));
}

void append_u64(vector<unsigned char> & out, uint64_t value)
{
  for(size_t i = 0; i < 8; ++i) {
    out.push_back(static_cast<unsigned char>((value >> (i * 8)) & 0xFFu));
  }
}

void overwrite_u32(vector<unsigned char> & out, size_t offset, uint32_t value)
{
  out[offset + 0] = static_cast<unsigned char>(value & 0xFFu);
  out[offset + 1] = static_cast<unsigned char>((value >> 8) & 0xFFu);
  out[offset + 2] = static_cast<unsigned char>((value >> 16) & 0xFFu);
  out[offset + 3] = static_cast<unsigned char>((value >> 24) & 0xFFu);
}

void append_uleb128(vector<unsigned char> & out, uint64_t value)
{
  do {
    unsigned char byte = static_cast<unsigned char>(value & 0x7Fu);
    value >>= 7;
    if(value != 0) {
      byte |= 0x80u;
    }
    out.push_back(byte);
  } while(value != 0);
}

void append_sleb128(vector<unsigned char> & out, long long value)
{
  bool more = true;
  while(more) {
    unsigned char byte = static_cast<unsigned char>(value & 0x7F);
    const bool sign_bit = (byte & 0x40u) != 0;
    value >>= 7;
    more = !((value == 0 && !sign_bit) || (value == -1 && sign_bit));
    if(more) {
      byte |= 0x80u;
    }
    out.push_back(byte);
  }
}

void append_cstring(vector<unsigned char> & out, const string & text)
{
  out.insert(out.end(), text.begin(), text.end());
  out.push_back(0);
}

string path_directory(const string & path)
{
  const size_t split = path.find_last_of("/\\");
  if(split == string::npos) {
    return string();
  }
  return path.substr(0, split);
}

string path_basename(const string & path)
{
  const size_t split = path.find_last_of("/\\");
  if(split == string::npos) {
    return path;
  }
  return path.substr(split + 1);
}

bool path_is_absolute(const string & path)
{
  return !path.empty() && (path[0] == '/' || path[0] == '\\');
}

bool is_plain_debug_identifier(const string & name)
{
  if(name.empty()) {
    return false;
  }
  const unsigned char first = static_cast<unsigned char>(name[0]);
  if(!(std::isalpha(first) || first == '_')) {
    return false;
  }
  for(size_t i = 1; i < name.size(); ++i) {
    const unsigned char ch = static_cast<unsigned char>(name[i]);
    if(!(std::isalnum(ch) || ch == '_')) {
      return false;
    }
  }
  return true;
}

string normalized_debug_name(const string & name)
{
  if(is_plain_debug_identifier(name)) {
    return name;
  }
  if(!name.empty() &&
     (name[0] == '%' || name[0] == '$') &&
     is_plain_debug_identifier(name.substr(1))) {
    return name.substr(1);
  }
  return string();
}

bool has_builtin_debug_type(const string & type)
{
  return type == "ptr" ||
         type == "i8" ||
         type == "u8" ||
         type == "i16" ||
         type == "u16" ||
         type == "i32" ||
         type == "u32" ||
         type == "i64" ||
         type == "u64" ||
         type == "f32" ||
         type == "f64" ||
         type == "f80";
}

string builtin_debug_type_name(const string & type)
{
  if(type == "ptr") return "void *";
  if(type == "i8") return "signed char";
  if(type == "u8") return "unsigned char";
  if(type == "i16") return "short";
  if(type == "u16") return "unsigned short";
  if(type == "i32") return "int";
  if(type == "u32") return "unsigned int";
  if(type == "i64") return "long long";
  if(type == "u64") return "unsigned long long";
  if(type == "f32") return "float";
  if(type == "f64") return "double";
  if(type == "f80") return "long double";
  throw logic_error("unsupported builtin debug type " + type);
}

uint8_t builtin_debug_type_encoding(const string & type)
{
  if(type == "ptr") return DW_ATE_ADDRESS;
  if(type == "i8" || type == "i16" || type == "i32" || type == "i64") {
    return DW_ATE_SIGNED;
  }
  if(type == "u8" || type == "u16" || type == "u32" || type == "u64") {
    return DW_ATE_UNSIGNED;
  }
  if(type == "f32" || type == "f64" || type == "f80") {
    return DW_ATE_FLOAT;
  }
  throw logic_error("unsupported builtin debug type " + type);
}

uint8_t builtin_debug_type_size(const string & type)
{
  if(type == "ptr") return 8;
  if(type == "i8" || type == "u8") return 1;
  if(type == "i16" || type == "u16") return 2;
  if(type == "i32" || type == "u32" || type == "f32") return 4;
  if(type == "i64" || type == "u64" || type == "f64") return 8;
  if(type == "f80") return 16;
  throw logic_error("unsupported builtin debug type " + type);
}

uint16_t dwarf_register_number(X64Register reg)
{
  switch(reg) {
    case XR_RAX: return 0;
    case XR_RDX: return 1;
    case XR_RCX: return 2;
    case XR_RBX: return 3;
    case XR_RSI: return 4;
    case XR_RDI: return 5;
    case XR_RBP: return 6;
    case XR_RSP: return 7;
    case XR_R8: return 8;
    case XR_R9: return 9;
    case XR_R10: return 10;
    case XR_R11: return 11;
    case XR_R12: return 12;
    case XR_R13: return 13;
    case XR_R14: return 14;
    case XR_R15: return 15;
  }
  throw logic_error("unsupported DWARF register mapping");
}

uint16_t dwarf_register_number(XmmRegister reg)
{
  return static_cast<uint16_t>(17 + reg);
}

const mir::FrameBinding * find_frame_binding(const mir::Function & function,
                                             mir::FrameBinding::Kind kind,
                                             const string & name)
{
  for(size_t i = 0; i < function.frame_bindings.size(); ++i) {
    if(function.frame_bindings[i].kind == kind &&
       function.frame_bindings[i].name == name) {
      return &function.frame_bindings[i];
    }
  }
  return nullptr;
}

void set_unit_source_path(DwarfCompilationUnitInfo & info,
                          const string & path)
{
  if(path.empty()) {
    return;
  }
  if(path_is_absolute(path)) {
    info.unit_name = path_basename(path);
    info.unit_directory = path_directory(path);
    if(info.unit_name.empty()) {
      info.unit_name = path;
    }
    return;
  }
  info.unit_name = path;
  info.unit_directory.clear();
}

size_t dwarf_directory_index(DwarfFileCatalog & catalog, const string & directory)
{
  if(directory.empty()) {
    return 0;
  }
  map<string, size_t>::const_iterator found = catalog.directory_index.find(directory);
  if(found != catalog.directory_index.end()) {
    return found->second;
  }
  catalog.directories.push_back(directory);
  const size_t index = catalog.directories.size();
  catalog.directory_index[directory] = index;
  return index;
}

size_t dwarf_file_index(DwarfFileCatalog & catalog, const string & path)
{
  map<string, size_t>::const_iterator found = catalog.file_index.find(path);
  if(found != catalog.file_index.end()) {
    return found->second;
  }

  DwarfFileEntry entry;
  entry.path = path;
  entry.directory = path_directory(path);
  entry.name = path_basename(path);
  entry.directory_index = dwarf_directory_index(catalog, entry.directory);
  catalog.files.push_back(entry);
  const size_t index = catalog.files.size();
  catalog.file_index[path] = index;
  return index;
}

string debug_function_display_name(const string & symbol)
{
  if(!symbol.empty() && symbol[0] == '@') {
    return symbol.substr(1);
  }
  return symbol;
}

struct DebugSectionNames
{
  string segment_name;
  string abbrev_name;
  string info_name;
  string line_name;
  string loc_name;
};

DebugSectionNames debug_section_names_for_target(const string & target)
{
  if(target == "macos") {
    return {"__DWARF", "__debug_abbrev", "__debug_info", "__debug_line", "__debug_loc"};
  }
  return {".elf", ".debug_abbrev", ".debug_info", ".debug_line", ".debug_loc"};
}

bool is_generated_debug_section(const string & target,
                                const string & section_name)
{
  const DebugSectionNames names = debug_section_names_for_target(target);
  return section_name == names.abbrev_name ||
         section_name == names.info_name ||
         section_name == names.line_name ||
         section_name == names.loc_name;
}

const char * object_binding_name(mobj::Symbol::Binding binding)
{
  switch(binding) {
    case mobj::Symbol::SB_LOCAL:
      return "local";
    case mobj::Symbol::SB_GLOBAL:
      return "global";
    case mobj::Symbol::SB_WEAK:
      return "weak";
  }
  return "unknown";
}

string block_symbol(const string & function_name, const string & block_label)
{
  return function_name + "$" + block_label;
}

bool is_conditional_jump(const mir::Instruction & inst)
{
  return inst.opcode == mir::Instruction::MI_JNE ||
         inst.opcode == mir::Instruction::MI_JCC;
}

X86Condition jump_condition(const mir::Instruction & inst)
{
  return inst.opcode == mir::Instruction::MI_JNE ? XC_NE : inst.condition;
}

struct FunctionLayout
{
  map<string, size_t> block_offsets;
  map<string, vector<size_t> > instruction_offsets;
  size_t size = 0;
};

struct ObjectLayout
{
  map<string, size_t> function_offsets;
  map<string, FunctionLayout> function_layouts;
  map<string, size_t> thread_local_wrapper_offsets;
  map<string, size_t> global_offsets;
  map<string, size_t> readonly_global_offsets;
  map<string, size_t> thread_local_template_offsets;
  map<string, size_t> thread_local_descriptor_offsets;
  size_t code_size = 0;
  size_t data_size = 0;
  size_t readonly_data_size = 0;
  size_t thread_local_template_data_size = 0;
  size_t thread_local_template_bss_size = 0;
  size_t thread_local_descriptor_size = 0;
};

struct ReadonlySectionInfo
{
  string segment_name;
  string section_name;
};

struct ThreadLocalSectionInfo
{
  enum Abi
  {
    ABI_MACHO_TLV,
    ABI_EMUTLS,
    ABI_DIRECT_NATIVE,
    ABI_ELF
  } abi = ABI_MACHO_TLV;

  string data_segment_name;
  string data_section_name;
  string bss_segment_name;
  string bss_section_name;
  string vars_segment_name;
  string vars_section_name;
};

thread_local bool g_use_direct_native_tls_abi = false;

struct ScopedDirectNativeTlsAbi
{
  explicit ScopedDirectNativeTlsAbi(bool enabled)
      : saved(g_use_direct_native_tls_abi)
  {
    g_use_direct_native_tls_abi = enabled;
  }

  ~ScopedDirectNativeTlsAbi()
  {
    g_use_direct_native_tls_abi = saved;
  }

  bool saved = false;
};

string configured_host_cxx_command()
{
  const char * env_host_cxx = std::getenv("CPPGM_HOST_CXX");
  if(env_host_cxx && *env_host_cxx) {
    return env_host_cxx;
  }
#ifdef CPPGM_DEFAULT_HOST_CXX
  return CPPGM_DEFAULT_HOST_CXX;
#else
  return string();
#endif
}

bool configured_host_cxx_looks_gnu()
{
  const string command = configured_host_cxx_command();
  if(command.find("clang") != string::npos) {
    return false;
  }
  return command.find("g++") != string::npos || command.find("gcc") != string::npos;
}

bool target_uses_gnu_thread_local_abi(const string & target)
{
  return target == "macos" && configured_host_cxx_looks_gnu();
}

ReadonlySectionInfo readonly_section_info_for_target(const string & target)
{
  if(target == "macos") {
    return {"__DATA_CONST", "__const"};
  }
  return {".cppgm", ".rodata"};
}

ThreadLocalSectionInfo thread_local_section_info_for_target(const string & target)
{
  if(target == "macos") {
    ThreadLocalSectionInfo info;
    if(g_use_direct_native_tls_abi) {
      info.abi = ThreadLocalSectionInfo::ABI_DIRECT_NATIVE;
      info.data_segment_name = "__DATA";
      info.data_section_name = "__data";
      info.vars_segment_name = "__DATA";
      info.vars_section_name = "__data";
    } else if(target_uses_gnu_thread_local_abi(target)) {
      info.abi = ThreadLocalSectionInfo::ABI_EMUTLS;
      info.data_segment_name = "__TEXT";
      info.data_section_name = "__const";
      info.vars_segment_name = "__DATA";
      info.vars_section_name = "__data";
    } else {
      info.abi = ThreadLocalSectionInfo::ABI_MACHO_TLV;
      info.data_segment_name = "__DATA";
      info.data_section_name = "__thread_data";
      info.bss_segment_name = "__DATA";
      info.bss_section_name = "__thread_bss";
      info.vars_segment_name = "__DATA";
      info.vars_section_name = "__thread_vars";
    }
    return info;
  }
  if(target == "linux") {
    ThreadLocalSectionInfo info;
    if(g_use_direct_native_tls_abi) {
      info.abi = ThreadLocalSectionInfo::ABI_DIRECT_NATIVE;
      info.data_segment_name = ".cppgm";
      info.data_section_name = ".data";
      return info;
    }
    info.abi = ThreadLocalSectionInfo::ABI_ELF;
    info.data_segment_name = ".elf";
    info.data_section_name = ".tdata";
    return info;
  }
  throw logic_error("unsupported thread_local object emission target " + target);
}

string translated_symbol_name(const map<string, symbol_linkage::SymbolIdentity> & exports,
                              const string & internal_name);
map<string, symbol_linkage::SymbolIdentity> export_map(
    const vector<symbol_linkage::SymbolIdentity> & exported_symbols);

string thread_local_template_symbol(const map<string, symbol_linkage::SymbolIdentity> & exports,
                                    const string & global_name,
                                    const string & target)
{
  const ThreadLocalSectionInfo info = thread_local_section_info_for_target(target);
  if(info.abi == ThreadLocalSectionInfo::ABI_MACHO_TLV) {
    return translated_symbol_name(exports, global_name) + "$tlv$init";
  }
  if(info.abi == ThreadLocalSectionInfo::ABI_EMUTLS) {
    const string internal = global_name.size() > 1 && global_name[0] == '@' &&
            global_name.find("__", 1) == string::npos
        ? global_name.substr(1)
        : translated_symbol_name(exports, global_name);
    const string suffix =
        !internal.empty() && internal[0] == '_' && (internal.size() < 2 || internal[1] != 'Z')
            ? internal.substr(1)
            : internal;
    return string("__emutls_t.") + suffix;
  }
  return global_name + "__tls_init_data";
}

string thread_local_runtime_object_symbol(
    const map<string, symbol_linkage::SymbolIdentity> & exports,
    const string & global_name,
    const string & target)
{
  const ThreadLocalSectionInfo info = thread_local_section_info_for_target(target);
  if(info.abi != ThreadLocalSectionInfo::ABI_EMUTLS) {
    return translated_symbol_name(exports, global_name);
  }

  string suffix;
  if(global_name.size() > 1 &&
     global_name[0] == '@' &&
     global_name.find("__", 1) == string::npos) {
    suffix = global_name.substr(1);
  } else {
    suffix = translated_symbol_name(exports, global_name);
    if(!suffix.empty() &&
       suffix[0] == '_' &&
       (suffix.size() < 2 || suffix[1] != 'Z')) {
      suffix.erase(0, 1);
    }
  }
  return string("__emutls_v.") + suffix;
}

string linux_simple_thread_local_object_symbol(const string & internal_name);

void append_extra_abs64_symbol_relocation(mobj::ExtraSection & section,
                                          size_t offset,
                                          const string & symbol,
                                          long long addend = 0)
{
  mobj::ExtraRelocation reloc;
  reloc.kind = mobj::ExtraRelocation::RK_ABS64;
  reloc.target_kind = mobj::ExtraRelocation::TK_SYMBOL;
  reloc.offset = offset;
  reloc.symbol = symbol;
  reloc.addend = addend;
  section.relocations.push_back(reloc);
}

size_t align_up(size_t value, size_t alignment)
{
  if(alignment == 0) {
    return value;
  }
  const size_t mask = alignment - 1;
  return (value + mask) & ~mask;
}

size_t type_size_text(const string & type)
{
  return lowir_internal::type_size(lowir_internal::LowType{type});
}

size_t type_alignment_text(const string & type)
{
  return lowir_internal::type_alignment(lowir_internal::LowType{type});
}

size_t x87_memory_width_text(const string & type)
{
  if(type == "f32") {
    return 4;
  }
  if(type == "f64") {
    return 8;
  }
  if(type == "f80") {
    return 10;
  }
  throw logic_error("x87 width requested for non-floating type " + type);
}


bool is_negative_literal_text(const string & source)
{
  return !source.empty() && source[0] == '-';
}

string unsigned_literal_text(const string & source)
{
  if(!source.empty() && (source[0] == '-' || source[0] == '+')) {
    return source.substr(1);
  }
  return source;
}

bool signaling_nan_storage_bytes(const string & type,
                                 const string & source,
                                 vector<unsigned char> & out)
{
  const string text = unsigned_literal_text(source);
  const bool negative = is_negative_literal_text(source);
  if(type == "f32" && text == "snanf") {
    const uint64_t bits = (negative ? 0x80000000ULL : 0) | 0x7FA00000ULL;
    out = cy86_internal::encode_uint64(bits, 4);
    return true;
  }
  if(type == "f64" && text == "snan") {
    const uint64_t bits =
        (negative ? 0x8000000000000000ULL : 0) | 0x7FF4000000000000ULL;
    out = cy86_internal::encode_uint64(bits, 8);
    return true;
  }
  if(type == "f80" && text == "snanL") {
    out.assign(16, 0);
    out[7] = 0xA0;
    out[8] = 0xFF;
    out[9] = negative ? 0xFF : 0x7F;
    return true;
  }
  return false;
}

vector<unsigned char> float_literal_storage_bytes(const string & type,
                                                  long double value,
                                                  const string & literal_text = string())
{
  vector<unsigned char> snan_bytes;
  if(signaling_nan_storage_bytes(type, literal_text, snan_bytes)) {
    return snan_bytes;
  }
  if(type == "f32") {
    return cy86_internal::bytes_of(static_cast<float>(value));
  }
  if(type == "f64") {
    return cy86_internal::bytes_of(static_cast<double>(value));
  }
  if(type == "f80") {
    vector<unsigned char> bytes(16, 0);
    const vector<unsigned char> payload = cy86_internal::store_float80(value);
    copy(payload.begin(), payload.end(), bytes.begin());
    return bytes;
  }
  throw logic_error("floating storage bytes requested for non-floating type " + type);
}

uint32_t little_endian_u32(const vector<unsigned char> & bytes)
{
  if(bytes.size() < 4) {
    throw logic_error("insufficient bytes for u32 literal");
  }
  return static_cast<uint32_t>(bytes[0]) |
         (static_cast<uint32_t>(bytes[1]) << 8) |
         (static_cast<uint32_t>(bytes[2]) << 16) |
         (static_cast<uint32_t>(bytes[3]) << 24);
}

uint64_t little_endian_u64(const vector<unsigned char> & bytes)
{
  if(bytes.size() < 8) {
    throw logic_error("insufficient bytes for u64 literal");
  }
  uint64_t out = 0;
  for(size_t i = 0; i < 8; ++i) {
    out |= static_cast<uint64_t>(bytes[i]) << (i * 8);
  }
  return out;
}

vector<unsigned char> integer_literal_storage_bytes(long long value,
                                                    size_t width)
{
  if(width <= 8) {
    return cy86_internal::encode_uint64(static_cast<uint64_t>(value), width);
  }
  vector<unsigned char> out(width, value < 0 ? 0xFF : 0x00);
  const vector<unsigned char> low =
      cy86_internal::encode_uint64(static_cast<uint64_t>(value), 8);
  copy(low.begin(), low.end(), out.begin());
  return out;
}

bool storage_bytes_all_zero(const vector<unsigned char> & bytes)
{
  for(size_t i = 0; i < bytes.size(); ++i) {
    if(bytes[i] != 0) {
      return false;
    }
  }
  return true;
}

bool thread_local_global_uses_macho_zerofill_template(
    const mir::GlobalDefinition & global,
    const ThreadLocalSectionInfo & info)
{
  if(info.abi != ThreadLocalSectionInfo::ABI_MACHO_TLV) {
    return false;
  }

  if(global.storage_kind == mir::GlobalDefinition::GS_SCALAR) {
    if(global.init_kind == mir::GlobalDefinition::GI_ZERO) {
      return true;
    }
    if(global.init_kind == mir::GlobalDefinition::GI_INTEGER) {
      return storage_bytes_all_zero(
          integer_literal_storage_bytes(global.int_value, type_size_text(global.type)));
    }
    if(global.init_kind == mir::GlobalDefinition::GI_FLOAT) {
      return storage_bytes_all_zero(
          float_literal_storage_bytes(global.type, global.float_value, global.literal_text));
    }
    return false;
  }

  for(size_t di = 0; di < global.data_items.size(); ++di) {
    const mir::GlobalDefinition::DataItem & item = global.data_items[di];
    if(item.kind == mir::GlobalDefinition::DataItem::ITEM_ZERO) {
      continue;
    }
    if(item.kind == mir::GlobalDefinition::DataItem::ITEM_ADDR) {
      return false;
    }
    const vector<unsigned char> bytes =
        item.kind == mir::GlobalDefinition::DataItem::ITEM_INTEGER
            ? integer_literal_storage_bytes(item.int_value, type_size_text(item.type))
            : float_literal_storage_bytes(item.type, item.float_value, item.literal_text);
    if(!storage_bytes_all_zero(bytes)) {
      return false;
    }
  }
  return true;
}

string thread_local_template_extra_section(const mir::GlobalDefinition & global,
                                           const ThreadLocalSectionInfo & info)
{
  if(thread_local_global_uses_macho_zerofill_template(global, info)) {
    return info.bss_segment_name + "," + info.bss_section_name;
  }
  return info.data_segment_name + "," + info.data_section_name;
}

X86Memory frame_memory(long long offset)
{
  return X86Memory(XR_RBP, static_cast<int32_t>(offset));
}

void emit_store_reg_to_frame(X86Assembler & out,
                             const string & type,
                             long long offset,
                             X64Register src)
{
  const X86Memory mem = frame_memory(offset);
  const size_t width = type_size_text(type);
  if(width == 1) out.emit_mov_m8_r64(mem, src);
  else if(width == 2) out.emit_mov_m16_r64(mem, src);
  else if(width == 4) out.emit_mov_m32_r64(mem, src);
  else out.emit_mov_m64_r64(mem, src);
}

void emit_load_reg_from_frame(X86Assembler & out,
                              const string & type,
                              X64Register dst,
                              long long offset)
{
  const X86Memory mem = frame_memory(offset);
  const size_t width = type_size_text(type);
  if(width == 1) out.emit_movzx_r64_m8(dst, mem);
  else if(width == 2) out.emit_movzx_r64_m16(dst, mem);
  else if(width == 4) out.emit_mov_r32_m32(dst, mem);
  else out.emit_mov_r64_m64(dst, mem);
}

void emit_store_reg_to_memory(X86Assembler & out,
                              const string & type,
                              const X86Memory & mem,
                              X64Register src)
{
  const size_t width = type_size_text(type);
  if(width == 1) out.emit_mov_m8_r64(mem, src);
  else if(width == 2) out.emit_mov_m16_r64(mem, src);
  else if(width == 4) out.emit_mov_m32_r64(mem, src);
  else out.emit_mov_m64_r64(mem, src);
}

void emit_load_reg_from_memory(X86Assembler & out,
                               const string & type,
                               X64Register dst,
                               const X86Memory & mem)
{
  const size_t width = type_size_text(type);
  if(width == 1) out.emit_movzx_r64_m8(dst, mem);
  else if(width == 2) out.emit_movzx_r64_m16(dst, mem);
  else if(width == 4) out.emit_mov_r32_m32(dst, mem);
  else out.emit_mov_r64_m64(dst, mem);
}

void emit_zero_memory(X86Assembler & out,
                      const X86Memory & dst,
                      size_t width,
                      X64Register tmp)
{
  out.emit_xor_r64_r64(tmp, tmp);
  for(size_t offset = 0; offset < width; ++offset) {
    out.emit_mov_m8_r64(X86Memory(dst.base, dst.disp + static_cast<int32_t>(offset)), tmp);
  }
}

X86Memory deref_memory(X64Register reg, long long offset)
{
  return X86Memory(reg, static_cast<int32_t>(offset));
}

size_t global_alignment(const mir::GlobalDefinition & global)
{
  if(global.storage_kind == mir::GlobalDefinition::GS_SCALAR) {
    return type_alignment_text(global.type);
  }
  size_t alignment = 8;
  for(size_t i = 0; i < global.data_items.size(); ++i) {
    const mir::GlobalDefinition::DataItem & item = global.data_items[i];
    if(item.kind == mir::GlobalDefinition::DataItem::ITEM_INTEGER ||
       item.kind == mir::GlobalDefinition::DataItem::ITEM_FLOAT) {
      alignment = max(alignment, type_alignment_text(item.type));
    } else if(item.kind == mir::GlobalDefinition::DataItem::ITEM_ADDR) {
      alignment = max(alignment, static_cast<size_t>(8));
    }
  }
  return alignment;
}

size_t global_size(const mir::GlobalDefinition & global)
{
  if(global.storage_kind == mir::GlobalDefinition::GS_SCALAR) {
    return type_size_text(global.type);
  }
  size_t offset = 0;
  for(size_t i = 0; i < global.data_items.size(); ++i) {
    const mir::GlobalDefinition::DataItem & item = global.data_items[i];
      if(item.kind == mir::GlobalDefinition::DataItem::ITEM_ZERO) {
        offset += item.zero_bytes;
        continue;
      }
    const size_t width = item.kind == mir::GlobalDefinition::DataItem::ITEM_ADDR
        ? 8
        : type_size_text(item.type);
    const size_t align = item.kind == mir::GlobalDefinition::DataItem::ITEM_ADDR
        ? 8
        : type_alignment_text(item.type);
    while(align > 1 && (offset % align) != 0) {
      ++offset;
    }
    offset += width;
  }
  return offset;
}

size_t thread_local_wrapper_size(const string & target)
{
  const ThreadLocalSectionInfo info = thread_local_section_info_for_target(target);
  if(info.abi == ThreadLocalSectionInfo::ABI_MACHO_TLV) {
    return 15;
  }
  if(info.abi == ThreadLocalSectionInfo::ABI_EMUTLS) {
    return 21;
  }
  if(info.abi == ThreadLocalSectionInfo::ABI_DIRECT_NATIVE) {
    return 8;
  }
  if(info.abi == ThreadLocalSectionInfo::ABI_ELF) {
    return 17;
  }
  throw logic_error("unsupported thread_local wrapper target " + target);
}

struct Fixup
{
  enum Kind
  {
    FX_CALL_REL32,
    FX_MOV_ABS64,
    FX_RIP_REL32,
    FX_INDIRECT_REL32
  } kind = FX_CALL_REL32;

  size_t patch_offset = 0;
  string target;
};

long long callee_saved_slot_offset(const mir::Function & function,
                                   size_t index);

void append_relocation(vector<mobj::Relocation> & relocs,
                       mobj::Symbol::Section section,
                       size_t offset,
                       mobj::Relocation::Kind kind,
                       const string & symbol,
                       long long addend = 0);

void emit_emutls_wrapper_bytes(X86Assembler & out,
                               size_t section_offset,
                               const string & emutls_variable_symbol,
                               vector<mobj::Relocation> & relocations)
{
  out.append(vector<unsigned char>(1, 0x55));
  out.emit_mov_r64_r64(XR_RBP, XR_RSP);
  const size_t var_patch = out.emit_mov_r64_rip_rel32_placeholder(XR_RAX);
  append_relocation(relocations,
                    mobj::Symbol::SS_CODE,
                    section_offset + var_patch,
                    mobj::Relocation::RK_INDIRECT_REL32,
                    emutls_variable_symbol);
  out.emit_mov_r64_r64(XR_RDI, XR_RAX);
  const size_t call_patch = out.emit_call_rel32_placeholder();
  append_relocation(relocations,
                    mobj::Symbol::SS_CODE,
                    section_offset + call_patch,
                    mobj::Relocation::RK_BRANCH32,
                    "__emutls_get_address");
  out.append(vector<unsigned char>(1, 0x5d));
  out.emit_ret();
}

void emit_elf_tls_address_bytes(X86Assembler & out,
                                size_t section_offset,
                                const string & tls_object_symbol,
                                vector<mobj::Relocation> & relocations)
{
  const size_t wrapper_start = out.offset();
  out.append(vector<unsigned char>{0x64, 0x48, 0x8B, 0x04, 0x25, 0x00, 0x00, 0x00, 0x00});
  const size_t tpoff_patch = section_offset + (out.offset() - wrapper_start) + 3;
  out.append(vector<unsigned char>{0x48, 0x8D, 0x80, 0x00, 0x00, 0x00, 0x00});
  append_relocation(relocations,
                    mobj::Symbol::SS_CODE,
                    tpoff_patch,
                    mobj::Relocation::RK_TPOFF32,
                    tls_object_symbol);
}

void patch_rel32_local(X86Assembler & out,
                       size_t patch_offset,
                       uint64_t base_vaddr,
                       uint64_t target_vaddr)
{
  const int64_t disp = static_cast<int64_t>(target_vaddr) -
      static_cast<int64_t>(base_vaddr + patch_offset + 4);
  if(disp < INT32_MIN || disp > INT32_MAX) {
    throw logic_error("rel32 target out of range");
  }
  out.patch_u32(patch_offset, static_cast<uint32_t>(static_cast<int32_t>(disp)));
}

void emit_exit_sequence(X86Assembler & out,
                        const native_format::Hooks & target_hooks)
{
  out.emit_mov_r64_imm64(XR_RAX, target_hooks.exit_syscall_number);
  out.emit_syscall();
  out.emit_ud2();
}

X86Memory scratch_memory(long long scratch_base, size_t slot)
{
  return X86Memory(XR_RBP, static_cast<int32_t>(scratch_base + slot * 16));
}

size_t function_frame_bytes(const mir::Function & function)
{
  size_t out = function.frame_bytes;
  for(size_t i = 0; i < function.frame_bindings.size(); ++i) {
    out = max(out,
              static_cast<size_t>(function.frame_bindings[i].offset < 0
                                      ? -function.frame_bindings[i].offset
                                      : function.frame_bindings[i].offset));
  }
  return (out + 7) & ~static_cast<size_t>(7);
}

long long callee_saved_slot_offset(const mir::Function & function,
                                   size_t index)
{
  return -static_cast<long long>(function_frame_bytes(function) + (index + 1) * 8);
}

void emit_function_prologue(X86Assembler & out,
                            const mir::Function & function)
{
  if(function.host_eh_enabled) {
    out.append(vector<unsigned char>(1, 0x55));
    out.emit_mov_r64_r64(XR_RBP, XR_RSP);
  } else {
    out.emit_sub_r64_imm32(XR_RSP, 8);
    out.emit_mov_m64_r64(X86Memory(XR_RSP, 0), XR_RBP);
    out.emit_mov_r64_r64(XR_RBP, XR_RSP);
  }
  if(function.stack_size != 0) {
    out.emit_sub_r64_imm32(XR_RSP, static_cast<int32_t>(function.stack_size));
  }
  for(size_t i = 0; i < function.callee_saved_regs.size(); ++i) {
    emit_store_reg_to_frame(out,
                            "i64",
                            callee_saved_slot_offset(function, i),
                            function.callee_saved_regs[i]);
  }
}

void emit_function_epilogue(X86Assembler & out,
                            const mir::Function & function)
{
  for(size_t i = function.callee_saved_regs.size(); i > 0; --i) {
    emit_load_reg_from_frame(out,
                             "i64",
                             function.callee_saved_regs[i - 1],
                             callee_saved_slot_offset(function, i - 1));
  }
  out.emit_mov_r64_r64(XR_RSP, XR_RBP);
  if(function.host_eh_enabled) {
    out.append(vector<unsigned char>(1, 0x5d));
  } else {
    out.emit_mov_r64_m64(XR_RBP, X86Memory(XR_RSP, 0));
    out.emit_add_r64_imm32(XR_RSP, 8);
  }
}

size_t eh_handler_record_size()
{
  return 72;
}

int32_t eh_handler_saved_reg_offset(X64Register reg)
{
  switch(reg) {
    case XR_RBX: return 32;
    case XR_R12: return 40;
    case XR_R13: return 48;
    case XR_R14: return 56;
    case XR_R15: return 64;
    default:
      throw logic_error("unsupported EH callee-saved register");
  }
}

void emit_eh_save_callee_saved(X86Assembler & out)
{
  out.emit_mov_m64_r64(X86Memory(XR_RSP, eh_handler_saved_reg_offset(XR_RBX)), XR_RBX);
  out.emit_mov_m64_r64(X86Memory(XR_RSP, eh_handler_saved_reg_offset(XR_R12)), XR_R12);
  out.emit_mov_m64_r64(X86Memory(XR_RSP, eh_handler_saved_reg_offset(XR_R13)), XR_R13);
  out.emit_mov_m64_r64(X86Memory(XR_RSP, eh_handler_saved_reg_offset(XR_R14)), XR_R14);
  out.emit_mov_m64_r64(X86Memory(XR_RSP, eh_handler_saved_reg_offset(XR_R15)), XR_R15);
}

void emit_eh_restore_callee_saved(X86Assembler & out,
                                  X64Register record)
{
  out.emit_mov_r64_m64(XR_RBX, X86Memory(record, eh_handler_saved_reg_offset(XR_RBX)));
  out.emit_mov_r64_m64(XR_R12, X86Memory(record, eh_handler_saved_reg_offset(XR_R12)));
  out.emit_mov_r64_m64(XR_R13, X86Memory(record, eh_handler_saved_reg_offset(XR_R13)));
  out.emit_mov_r64_m64(XR_R14, X86Memory(record, eh_handler_saved_reg_offset(XR_R14)));
  out.emit_mov_r64_m64(XR_R15, X86Memory(record, eh_handler_saved_reg_offset(XR_R15)));
}

void emit_write_bytes_to_memory(X86Assembler & out,
                                const X86Memory & dst,
                                const vector<unsigned char> & bytes,
                                X64Register tmp)
{
  size_t offset = 0;
  while(offset < bytes.size()) {
    const size_t chunk = bytes.size() - offset >= 8 ? 8
                         : bytes.size() - offset >= 4 ? 4
                         : bytes.size() - offset >= 2 ? 2
                         : 1;
    uint64_t value = 0;
    for(size_t i = 0; i < chunk; ++i) {
      value |= uint64_t(bytes[offset + i]) << (8 * i);
    }
    out.emit_mov_r64_imm64(tmp, value);
    X86Memory mem(dst.base, dst.disp + static_cast<int32_t>(offset));
    if(chunk == 8) {
      out.emit_mov_m64_r64(mem, tmp);
    } else if(chunk == 4) {
      out.emit_mov_m32_r64(mem, tmp);
    } else if(chunk == 2) {
      out.emit_mov_m16_r64(mem, tmp);
    } else {
      out.emit_mov_m8_r64(mem, tmp);
    }
    offset += chunk;
  }
}

X86Memory emit_global_memory(X86Assembler & out,
                             const string & symbol_name,
                             X64Register addr_tmp,
                             vector<Fixup> * fixups,
                             bool use_indirect_rel32)
{
  const size_t patch_offset = use_indirect_rel32
      ? out.emit_mov_r64_rip_rel32_placeholder(addr_tmp)
      : out.emit_lea_r64_rip_rel32_placeholder(addr_tmp);
  Fixup fixup;
  fixup.kind = use_indirect_rel32 ? Fixup::FX_INDIRECT_REL32 : Fixup::FX_RIP_REL32;
  fixup.patch_offset = patch_offset;
  fixup.target = symbol_name;
  fixups->push_back(fixup);
  return X86Memory(addr_tmp, 0);
}

bool use_indirect_rel32_for_imported_symbol(cy86_internal::NativeTarget target,
                                            const set<string> & defined_symbols,
                                            const string & symbol_name)
{
  (void) target;
  return defined_symbols.count(symbol_name) == 0 &&
         !runtime_symbol_policy::is_reserved_internal_symbol(symbol_name);
}

void emit_sign_extend_register(X86Assembler & out,
                               X64Register reg,
                               size_t source_width)
{
  if(source_width == 0 || source_width >= 8) {
    return;
  }
  const unsigned char shift = static_cast<unsigned char>((8 - source_width) * 8);
  out.emit_shl_r64_imm8(reg, shift);
  out.emit_sar_r64_imm8(reg, shift);
}

uint64_t truncate_to_width(uint64_t value, size_t width)
{
  if(width >= 8) {
    return value;
  }
  return value & ((uint64_t(1) << (width * 8)) - 1);
}

void emit_store_reg_to_operand(X86Assembler & out,
                               const mir::Operand & dst,
                               const string & type,
                               X64Register src,
                               X64Register addr_tmp,
                               vector<Fixup> * fixups,
                               cy86_internal::NativeTarget target,
                               const set<string> & defined_globals)
{
  if(dst.kind == mir::Operand::OP_REG) {
    if(type == "i64" || type == "ptr") {
      if(dst.reg != src) {
        out.emit_mov_r64_r64(dst.reg, src);
      }
      return;
    }
    if(type == "i32") {
      if(dst.reg != src) {
        out.emit_mov_r32_r32(dst.reg, src);
      } else {
        out.emit_movsxd_r64_r32(dst.reg, dst.reg);
        return;
      }
      out.emit_movsxd_r64_r32(dst.reg, dst.reg);
      return;
    }
    if(type == "u32") {
      out.emit_mov_r32_r32(dst.reg, src);
      return;
    }
    if(type == "i16" || type == "i8") {
      if(dst.reg != src) {
        out.emit_mov_r64_r64(dst.reg, src);
      }
      emit_sign_extend_register(out, dst.reg, type == "i16" ? 2 : 1);
      return;
    }
    if(type == "u16" || type == "u8") {
      if(dst.reg != src) {
        out.emit_mov_r64_r64(dst.reg, src);
      }
      out.emit_and_r64_imm32(dst.reg, type == "u16" ? 0xFFFF : 0xFF);
      return;
    }
    throw logic_error("unsupported integer conversion register destination type " + type);
  }
  if(dst.kind == mir::Operand::OP_FRAME) {
    emit_store_reg_to_frame(out, type, dst.offset, src);
    return;
  }
  if(dst.kind == mir::Operand::OP_DEREF) {
    emit_store_reg_to_memory(out,
                             type,
                             X86Memory(dst.reg, static_cast<int32_t>(dst.offset)),
                             src);
    return;
  }
  if(dst.kind == mir::Operand::OP_GLOBAL) {
    emit_store_reg_to_memory(out,
                             type,
                             emit_global_memory(out,
                                                dst.text,
                                                addr_tmp,
                                                fixups,
                                                use_indirect_rel32_for_imported_symbol(target,
                                                                                 defined_globals,
                                                                                 dst.text)),
                             src);
    return;
  }
  throw logic_error("unsupported integer conversion destination");
}

void emit_load_integer_conversion_source(X86Assembler & out,
                                         const mir::Operand & src,
                                         size_t width,
                                         bool is_signed,
                                         X64Register dst,
                                         X64Register addr_tmp,
                                         vector<Fixup> * fixups,
                                         cy86_internal::NativeTarget target,
                                         const set<string> & defined_globals)
{
  const string width_type = width == 1 ? "i8" :
                            width == 2 ? "i16" :
                            width == 4 ? "i32" : "i64";
  if(src.kind == mir::Operand::OP_IMM) {
    uint64_t value = truncate_to_width(static_cast<uint64_t>(src.imm), width);
    out.emit_mov_r64_imm64(dst, value);
    if(is_signed) {
      emit_sign_extend_register(out, dst, width);
    }
    return;
  }

  if(src.kind == mir::Operand::OP_FRAME) {
    emit_load_reg_from_frame(out, width_type, dst, src.offset);
    if(is_signed) {
      emit_sign_extend_register(out, dst, width);
    }
    return;
  }

  if(src.kind == mir::Operand::OP_DEREF) {
    emit_load_reg_from_memory(out,
                              width_type,
                              dst,
                              X86Memory(src.reg, static_cast<int32_t>(src.offset)));
    if(is_signed) {
      emit_sign_extend_register(out, dst, width);
    }
    return;
  }

  if(src.kind == mir::Operand::OP_GLOBAL) {
    emit_load_reg_from_memory(out,
                              width_type,
                              dst,
                              emit_global_memory(out,
                                                 src.text,
                                                 addr_tmp,
                                                 fixups,
                                                 use_indirect_rel32_for_imported_symbol(target,
                                                                                  defined_globals,
                                                                                  src.text)));
    if(is_signed) {
      emit_sign_extend_register(out, dst, width);
    }
    return;
  }

  if(src.kind == mir::Operand::OP_REG) {
    if(dst != src.reg) {
      out.emit_mov_r64_r64(dst, src.reg);
    }
    if(!is_signed) {
      if(width < 8) {
        out.emit_and_r64_imm32(dst,
                               static_cast<uint32_t>((uint64_t(1) << (width * 8)) - 1));
      }
    } else {
      emit_sign_extend_register(out, dst, width);
    }
    return;
  }

  throw logic_error("unsupported integer conversion source");
}

void emit_fld_from_memory(X86Assembler & out,
                          const string & type,
                          const X86Memory & mem)
{
  const size_t width = x87_memory_width_text(type);
  if(width == 4) {
    out.emit_fld_m32(mem);
  } else if(width == 8) {
    out.emit_fld_m64(mem);
  } else {
    out.emit_fld_m80(mem);
  }
}

void emit_fstp_to_memory(X86Assembler & out,
                         const string & type,
                         const X86Memory & mem,
                         X64Register zero_tmp)
{
  const size_t width = x87_memory_width_text(type);
  if(width == 4) {
    out.emit_fstp_m32(mem);
    return;
  }
  if(width == 8) {
    out.emit_fstp_m64(mem);
    return;
  }
  out.emit_fstp_m80(mem);
  emit_zero_memory(out, X86Memory(mem.base, mem.disp + 10), 6, zero_tmp);
}

void emit_load_float_operand(X86Assembler & out,
                             const mir::Operand & operand,
                             const string & type,
                             long long scratch_base,
                             X64Register addr_tmp,
                             X64Register scratch_tmp,
                             vector<Fixup> * fixups,
                             cy86_internal::NativeTarget target,
                             const set<string> & defined_globals)
{
  if(operand.kind == mir::Operand::OP_FRAME) {
    emit_fld_from_memory(out, type, frame_memory(operand.offset));
    return;
  }
  if(operand.kind == mir::Operand::OP_DEREF) {
    emit_fld_from_memory(out, type, X86Memory(operand.reg, static_cast<int32_t>(operand.offset)));
    return;
  }
  if(operand.kind == mir::Operand::OP_GLOBAL) {
    emit_fld_from_memory(out,
                         type,
                         emit_global_memory(out,
                                            operand.text,
                                            addr_tmp,
                                            fixups,
                                            use_indirect_rel32_for_imported_symbol(target,
                                                                             defined_globals,
                                                                             operand.text)));
    return;
  }
  if(operand.kind == mir::Operand::OP_FLOAT_IMM) {
    const X86Memory temp = scratch_memory(scratch_base, 0);
    emit_write_bytes_to_memory(out,
                               temp,
                               float_literal_storage_bytes(type, operand.float_imm, operand.text),
                               scratch_tmp);
    emit_fld_from_memory(out, type, temp);
    return;
  }
  if(operand.kind == mir::Operand::OP_XMM) {
    const X86Memory temp = scratch_memory(scratch_base, 0);
    if(type == "f32") {
      out.emit_movss_m32_xmm(temp, operand.xmm);
    } else if(type == "f64") {
      out.emit_movsd_m64_xmm(temp, operand.xmm);
    } else {
      throw logic_error("invalid xmm float source type");
    }
    emit_fld_from_memory(out, type, temp);
    return;
  }
  throw logic_error("invalid float operand kind");
}

void emit_load_float_operand_to_xmm(X86Assembler & out,
                                    XmmRegister dst,
                                    const mir::Operand & operand,
                                    const string & type,
                                    long long scratch_base,
                                    X64Register addr_tmp,
                                    X64Register scratch_tmp,
                                    vector<Fixup> * fixups,
                                    cy86_internal::NativeTarget target,
                                    const set<string> & defined_globals)
{
  if(type != "f32" && type != "f64") {
    throw logic_error("unsupported xmm float load type");
  }

  const auto emit_from_memory =
      [&](const X86Memory & mem)
      {
        if(type == "f32") {
          out.emit_movss_xmm_m32(dst, mem);
        } else {
          out.emit_movsd_xmm_m64(dst, mem);
        }
      };

  if(operand.kind == mir::Operand::OP_FRAME) {
    emit_from_memory(frame_memory(operand.offset));
    return;
  }
  if(operand.kind == mir::Operand::OP_DEREF) {
    emit_from_memory(X86Memory(operand.reg, static_cast<int32_t>(operand.offset)));
    return;
  }
  if(operand.kind == mir::Operand::OP_GLOBAL) {
    emit_from_memory(emit_global_memory(out,
                                        operand.text,
                                        addr_tmp,
                                        fixups,
                                        use_indirect_rel32_for_imported_symbol(target,
                                                                         defined_globals,
                                                                         operand.text)));
    return;
  }
  if(operand.kind == mir::Operand::OP_FLOAT_IMM) {
    const vector<unsigned char> bytes =
        float_literal_storage_bytes(type, operand.float_imm, operand.text);
    if(type == "f32") {
      out.emit_mov_r32_imm32(scratch_tmp, little_endian_u32(bytes));
      out.emit_movd_xmm_r32(dst, scratch_tmp);
    } else {
      out.emit_mov_r64_imm64(scratch_tmp, little_endian_u64(bytes));
      out.emit_movq_xmm_r64(dst, scratch_tmp);
    }
    return;
  }
  if(operand.kind == mir::Operand::OP_XMM) {
    if(operand.xmm != dst) {
      if(type == "f32") {
        out.emit_movss_xmm_xmm(dst, operand.xmm);
      } else {
        out.emit_movsd_xmm_xmm(dst, operand.xmm);
      }
    }
    return;
  }
  throw logic_error("invalid xmm float operand kind");
}

void emit_store_float_from_xmm(X86Assembler & out,
                               const mir::Operand & operand,
                               const string & type,
                               XmmRegister src,
                               X64Register addr_tmp,
                               vector<Fixup> * fixups,
                               cy86_internal::NativeTarget target,
                               const set<string> & defined_globals)
{
  if(type != "f32" && type != "f64") {
    throw logic_error("unsupported xmm float store type");
  }

  const auto emit_to_memory =
      [&](const X86Memory & mem)
      {
        if(type == "f32") {
          out.emit_movss_m32_xmm(mem, src);
        } else {
          out.emit_movsd_m64_xmm(mem, src);
        }
      };

  if(operand.kind == mir::Operand::OP_FRAME) {
    emit_to_memory(frame_memory(operand.offset));
    return;
  }
  if(operand.kind == mir::Operand::OP_DEREF) {
    emit_to_memory(X86Memory(operand.reg, static_cast<int32_t>(operand.offset)));
    return;
  }
  if(operand.kind == mir::Operand::OP_GLOBAL) {
    emit_to_memory(emit_global_memory(out,
                                      operand.text,
                                      addr_tmp,
                                      fixups,
                                      use_indirect_rel32_for_imported_symbol(target,
                                                                       defined_globals,
                                                                       operand.text)));
    return;
  }
  if(operand.kind == mir::Operand::OP_XMM) {
    if(operand.xmm != src) {
      if(type == "f32") {
        out.emit_movss_xmm_xmm(operand.xmm, src);
      } else {
        out.emit_movsd_xmm_xmm(operand.xmm, src);
      }
    }
    return;
  }
  throw logic_error("invalid xmm float destination");
}

void emit_load_float_bits_to_reg(X86Assembler & out,
                                 const mir::Operand & operand,
                                 const string & type,
                                 X64Register dst,
                                 long long scratch_base,
                                 X64Register addr_tmp,
                                 X64Register scratch_tmp,
                                 vector<Fixup> * fixups,
                                 cy86_internal::NativeTarget target,
                                 const set<string> & defined_globals)
{
  (void)scratch_tmp;
  const string int_type = type == "f32" ? "u32" : type == "f64" ? "i64" : "";
  if(int_type.empty()) {
    throw logic_error("unsupported float-bit load type " + type);
  }

  const auto load_from_memory =
      [&](const X86Memory & mem)
      {
        emit_load_reg_from_memory(out, int_type, dst, mem);
      };

  if(operand.kind == mir::Operand::OP_FLOAT_IMM) {
    const vector<unsigned char> bytes =
        float_literal_storage_bytes(type, operand.float_imm, operand.text);
    if(type == "f32") {
      out.emit_mov_r32_imm32(dst, little_endian_u32(bytes));
    } else {
      out.emit_mov_r64_imm64(dst, little_endian_u64(bytes));
    }
    return;
  }
  if(operand.kind == mir::Operand::OP_FRAME) {
    load_from_memory(frame_memory(operand.offset));
    return;
  }
  if(operand.kind == mir::Operand::OP_DEREF) {
    load_from_memory(X86Memory(operand.reg, static_cast<int32_t>(operand.offset)));
    return;
  }
  if(operand.kind == mir::Operand::OP_GLOBAL) {
    load_from_memory(emit_global_memory(out,
                                        operand.text,
                                        addr_tmp,
                                        fixups,
                                        use_indirect_rel32_for_imported_symbol(target,
                                                                         defined_globals,
                                                                         operand.text)));
    return;
  }
  if(operand.kind == mir::Operand::OP_XMM) {
    const X86Memory temp = scratch_memory(scratch_base, 0);
    if(type == "f32") {
      out.emit_movss_m32_xmm(temp, operand.xmm);
    } else {
      out.emit_movsd_m64_xmm(temp, operand.xmm);
    }
    load_from_memory(temp);
    return;
  }
  throw logic_error("invalid float-bit source");
}

void emit_store_float_bits_from_reg(X86Assembler & out,
                                    const mir::Operand & operand,
                                    const string & type,
                                    X64Register src,
                                    X64Register addr_tmp,
                                    vector<Fixup> * fixups,
                                    cy86_internal::NativeTarget target,
                                    const set<string> & defined_globals)
{
  if(type == "f32" && operand.kind == mir::Operand::OP_XMM) {
    out.emit_movd_xmm_r32(operand.xmm, src);
    return;
  }
  if(type == "f64" && operand.kind == mir::Operand::OP_XMM) {
    out.emit_movq_xmm_r64(operand.xmm, src);
    return;
  }

  const string int_type = type == "f32" ? "u32" : type == "f64" ? "i64" : "";
  if(int_type.empty()) {
    throw logic_error("unsupported float-bit store type " + type);
  }
  emit_store_reg_to_operand(out,
                            operand,
                            int_type,
                            src,
                            addr_tmp,
                            fixups,
                            target,
                            defined_globals);
}

void emit_store_float_pop(X86Assembler & out,
                          const mir::Operand & operand,
                          const string & type,
                          long long scratch_base,
                          X64Register addr_tmp,
                          X64Register scratch_tmp,
                          vector<Fixup> * fixups,
                          cy86_internal::NativeTarget target,
                          const set<string> & defined_globals)
{
  if(operand.kind == mir::Operand::OP_FRAME) {
    emit_fstp_to_memory(out, type, frame_memory(operand.offset), scratch_tmp);
    return;
  }
  if(operand.kind == mir::Operand::OP_DEREF) {
    emit_fstp_to_memory(out,
                        type,
                        X86Memory(operand.reg, static_cast<int32_t>(operand.offset)),
                        scratch_tmp);
    return;
  }
  if(operand.kind == mir::Operand::OP_GLOBAL) {
    emit_fstp_to_memory(out,
                        type,
                        emit_global_memory(out,
                                           operand.text,
                                           addr_tmp,
                                           fixups,
                                           use_indirect_rel32_for_imported_symbol(target,
                                                                            defined_globals,
                                                                            operand.text)),
                        scratch_tmp);
    return;
  }
  if(operand.kind == mir::Operand::OP_XMM) {
    const X86Memory temp = scratch_memory(scratch_base, 0);
    emit_fstp_to_memory(out, type, temp, scratch_tmp);
    if(type == "f32") {
      out.emit_movss_xmm_m32(operand.xmm, temp);
    } else if(type == "f64") {
      out.emit_movsd_xmm_m64(operand.xmm, temp);
    } else {
      throw logic_error("invalid xmm float destination type");
    }
    return;
  }
  throw logic_error("invalid float destination operand");
}

void emit_instruction(X86Assembler & out,
                      const mir::Instruction & inst,
                      long long scratch_base,
                      const mir::Function & current_function,
                      cy86_internal::NativeTarget target,
                      const set<string> & defined_symbols,
                      const set<string> & defined_globals,
                      vector<Fixup> * fixups)
{
  switch(inst.opcode) {
    case mir::Instruction::MI_MOV:
      if(inst.operands[0].kind != mir::Operand::OP_REG) {
        throw logic_error("mov destination must be register");
      }
      if(inst.operands[1].kind == mir::Operand::OP_REG) {
        out.emit_mov_r64_r64(inst.operands[0].reg, inst.operands[1].reg);
      } else if(inst.operands[1].kind == mir::Operand::OP_IMM) {
        out.emit_mov_r64_imm64(inst.operands[0].reg,
                               static_cast<uint64_t>(inst.operands[1].imm));
      } else if(inst.operands[1].kind == mir::Operand::OP_SYMBOL) {
        const bool use_indirect_rel32 =
            use_indirect_rel32_for_imported_symbol(target,
                                             defined_symbols,
                                             inst.operands[1].text);
        const size_t patch_offset = use_indirect_rel32
            ? out.emit_mov_r64_rip_rel32_placeholder(inst.operands[0].reg)
            : out.emit_lea_r64_rip_rel32_placeholder(inst.operands[0].reg);
        Fixup fixup;
        fixup.kind = use_indirect_rel32 ? Fixup::FX_INDIRECT_REL32 : Fixup::FX_RIP_REL32;
        fixup.patch_offset = patch_offset;
        fixup.target = inst.operands[1].text;
        fixups->push_back(fixup);
      } else {
        throw logic_error("unsupported mov source");
      }
      return;

    case mir::Instruction::MI_LOAD: {
      const X64Register dst = inst.operands[0].reg;
      const mir::Operand & src = inst.operands[1];
      if(src.kind == mir::Operand::OP_FRAME) {
        emit_load_reg_from_frame(out, inst.type, dst, src.offset);
        return;
      }
      if(src.kind == mir::Operand::OP_DEREF) {
        emit_load_reg_from_memory(out, inst.type, dst, X86Memory(src.reg, static_cast<int32_t>(src.offset)));
        return;
      }
      if(src.kind == mir::Operand::OP_GLOBAL) {
        const X64Register tmp = dst == XR_R11 ? XR_R10 : XR_R11;
        emit_load_reg_from_memory(out,
                                  inst.type,
                                  dst,
                                  emit_global_memory(out,
                                                     src.text,
                                                     tmp,
                                                     fixups,
                                                     use_indirect_rel32_for_imported_symbol(target,
                                                                                      defined_globals,
                                                                                      src.text)));
        return;
      }
      throw logic_error("unsupported load source");
    }

    case mir::Instruction::MI_STORE: {
      const mir::Operand & dst = inst.operands[0];
      const X64Register src = inst.operands[1].reg;
      if(dst.kind == mir::Operand::OP_FRAME) {
        emit_store_reg_to_frame(out, inst.type, dst.offset, src);
        return;
      }
      if(dst.kind == mir::Operand::OP_DEREF) {
        emit_store_reg_to_memory(out, inst.type, X86Memory(dst.reg, static_cast<int32_t>(dst.offset)), src);
        return;
      }
      if(dst.kind == mir::Operand::OP_GLOBAL) {
        const X64Register tmp = src == XR_R11 ? XR_R10 : XR_R11;
        emit_store_reg_to_memory(out,
                                 inst.type,
                                 emit_global_memory(out,
                                                    dst.text,
                                                    tmp,
                                                    fixups,
                                                    use_indirect_rel32_for_imported_symbol(target,
                                                                                     defined_globals,
                                                                                     dst.text)),
                                 src);
        return;
      }
      throw logic_error("unsupported store destination");
    }

    case mir::Instruction::MI_MFENCE:
      out.emit_mfence();
      return;

    case mir::Instruction::MI_LOCK_XADD: {
      const mir::Operand & dst = inst.operands[0];
      const X64Register src = inst.operands[1].reg;
      const size_t width = type_size_text(inst.type);
      const auto emit_lock_xadd =
          [&](const X86Memory & mem)
          {
            if(width == 1) out.emit_lock_xadd_m8_r64(mem, src);
            else if(width == 2) out.emit_lock_xadd_m16_r64(mem, src);
            else if(width == 4) out.emit_lock_xadd_m32_r64(mem, src);
            else out.emit_lock_xadd_m64_r64(mem, src);
          };
      if(dst.kind == mir::Operand::OP_FRAME) {
        emit_lock_xadd(frame_memory(dst.offset));
        return;
      }
      if(dst.kind == mir::Operand::OP_DEREF) {
        emit_lock_xadd(X86Memory(dst.reg, static_cast<int32_t>(dst.offset)));
        return;
      }
      if(dst.kind == mir::Operand::OP_GLOBAL) {
        const X64Register tmp = src == XR_R11 ? XR_R10 : XR_R11;
        emit_lock_xadd(emit_global_memory(out,
                                          dst.text,
                                          tmp,
                                          fixups,
                                          use_indirect_rel32_for_imported_symbol(target,
                                                                           defined_globals,
                                                                           dst.text)));
        return;
      }
      throw logic_error("unsupported lock_xadd destination");
    }

    case mir::Instruction::MI_XCHG: {
      const mir::Operand & dst = inst.operands[0];
      const X64Register src = inst.operands[1].reg;
      const size_t width = type_size_text(inst.type);
      const auto emit_xchg =
          [&](const X86Memory & mem)
          {
            if(width == 1) out.emit_xchg_m8_r64(mem, src);
            else if(width == 2) out.emit_xchg_m16_r64(mem, src);
            else if(width == 4) out.emit_xchg_m32_r64(mem, src);
            else out.emit_xchg_m64_r64(mem, src);
          };
      if(dst.kind == mir::Operand::OP_FRAME) {
        emit_xchg(frame_memory(dst.offset));
        return;
      }
      if(dst.kind == mir::Operand::OP_DEREF) {
        emit_xchg(X86Memory(dst.reg, static_cast<int32_t>(dst.offset)));
        return;
      }
      if(dst.kind == mir::Operand::OP_GLOBAL) {
        const X64Register tmp = src == XR_R11 ? XR_R10 : XR_R11;
        emit_xchg(emit_global_memory(out,
                                     dst.text,
                                     tmp,
                                     fixups,
                                     use_indirect_rel32_for_imported_symbol(target,
                                                                      defined_globals,
                                                                      dst.text)));
        return;
      }
      throw logic_error("unsupported xchg destination");
    }

    case mir::Instruction::MI_LOCK_CMPXCHG: {
      const mir::Operand & dst = inst.operands[0];
      const X64Register src = inst.operands[1].reg;
      const size_t width = type_size_text(inst.type);
      const auto emit_lock_cmpxchg =
          [&](const X86Memory & mem)
          {
            if(width == 1) out.emit_lock_cmpxchg_m8_r64(mem, src);
            else if(width == 2) out.emit_lock_cmpxchg_m16_r64(mem, src);
            else if(width == 4) out.emit_lock_cmpxchg_m32_r64(mem, src);
            else out.emit_lock_cmpxchg_m64_r64(mem, src);
          };
      if(dst.kind == mir::Operand::OP_FRAME) {
        emit_lock_cmpxchg(frame_memory(dst.offset));
        return;
      }
      if(dst.kind == mir::Operand::OP_DEREF) {
        emit_lock_cmpxchg(X86Memory(dst.reg, static_cast<int32_t>(dst.offset)));
        return;
      }
      if(dst.kind == mir::Operand::OP_GLOBAL) {
        const X64Register tmp = src == XR_R11 ? XR_R10 : XR_R11;
        emit_lock_cmpxchg(emit_global_memory(out,
                                             dst.text,
                                             tmp,
                                             fixups,
                                             use_indirect_rel32_for_imported_symbol(target,
                                                                              defined_globals,
                                                                              dst.text)));
        return;
      }
      throw logic_error("unsupported lock_cmpxchg destination");
    }

    case mir::Instruction::MI_LEA: {
      const mir::Operand & src = inst.operands[1];
      if(src.kind == mir::Operand::OP_FRAME) {
        out.emit_lea_r64_m(inst.operands[0].reg, frame_memory(src.offset));
        return;
      }
      if(src.kind == mir::Operand::OP_DEREF) {
        out.emit_lea_r64_m(inst.operands[0].reg,
                           X86Memory(src.reg, static_cast<int32_t>(src.offset)));
        return;
      }
      throw logic_error("unsupported lea source");
    }
    case mir::Instruction::MI_FMOV:
      if(inst.type == "f32" || inst.type == "f64") {
        if(inst.operands[0].kind == mir::Operand::OP_XMM) {
          emit_load_float_operand_to_xmm(out,
                                         inst.operands[0].xmm,
                                         inst.operands[1],
                                         inst.type,
                                         scratch_base,
                                         XR_R11,
                                         XR_RAX,
                                         fixups,
                                         target,
                                         defined_globals);
          return;
        }
        if(inst.operands[1].kind == mir::Operand::OP_XMM) {
          emit_store_float_from_xmm(out,
                                    inst.operands[0],
                                    inst.type,
                                    inst.operands[1].xmm,
                                    XR_R11,
                                    fixups,
                                    target,
                                    defined_globals);
          return;
        }
        emit_load_float_bits_to_reg(out,
                                    inst.operands[1],
                                    inst.type,
                                    XR_RAX,
                                    scratch_base,
                                    XR_R11,
                                    XR_R10,
                                    fixups,
                                    target,
                                    defined_globals);
        emit_store_float_bits_from_reg(out,
                                       inst.operands[0],
                                       inst.type,
                                       XR_RAX,
                                       XR_R11,
                                       fixups,
                                       target,
                                       defined_globals);
        return;
      }
      emit_load_float_operand(out,
                              inst.operands[1],
                              inst.type,
                              scratch_base,
                              XR_R11,
                              XR_RAX,
                              fixups,
                              target,
                              defined_globals);
      emit_store_float_pop(out,
                           inst.operands[0],
                           inst.type,
                           scratch_base,
                           XR_R11,
                           XR_RAX,
                           fixups,
                           target,
                           defined_globals);
      return;
    case mir::Instruction::MI_FNEG:
      if(inst.type == "f32" || inst.type == "f64") {
        emit_load_float_bits_to_reg(out,
                                    inst.operands[1],
                                    inst.type,
                                    XR_RAX,
                                    scratch_base,
                                    XR_R11,
                                    XR_R10,
                                    fixups,
                                    target,
                                    defined_globals);
        if(inst.type == "f32") {
          out.emit_mov_r64_imm64(XR_R11, 0x80000000ULL);
        } else {
          out.emit_mov_r64_imm64(XR_R11, 0x8000000000000000ULL);
        }
        out.emit_xor_r64_r64(XR_RAX, XR_R11);
        emit_store_float_bits_from_reg(out,
                                       inst.operands[0],
                                       inst.type,
                                       XR_RAX,
                                       XR_R11,
                                       fixups,
                                       target,
                                       defined_globals);
        return;
      }
      emit_load_float_operand(out,
                              inst.operands[1],
                              inst.type,
                              scratch_base,
                              XR_R11,
                              XR_RAX,
                              fixups,
                              target,
                              defined_globals);
      out.emit_fchs();
      emit_store_float_pop(out,
                           inst.operands[0],
                           inst.type,
                           scratch_base,
                           XR_R11,
                           XR_RAX,
                           fixups,
                           target,
                           defined_globals);
      return;
    case mir::Instruction::MI_FADD:
    case mir::Instruction::MI_FSUB:
    case mir::Instruction::MI_FMUL:
    case mir::Instruction::MI_FDIV:
      emit_load_float_operand(out,
                              inst.operands[1],
                              inst.type,
                              scratch_base,
                              XR_R11,
                              XR_RAX,
                              fixups,
                              target,
                              defined_globals);
      emit_load_float_operand(out,
                              inst.operands[2],
                              inst.type,
                              scratch_base,
                              XR_R11,
                              XR_RAX,
                              fixups,
                              target,
                              defined_globals);
      if(inst.opcode == mir::Instruction::MI_FADD) {
        out.emit_faddp_st1();
      } else if(inst.opcode == mir::Instruction::MI_FSUB) {
        out.emit_fsubp_st1();
      } else if(inst.opcode == mir::Instruction::MI_FMUL) {
        out.emit_fmulp_st1();
      } else {
        out.emit_fdivp_st1();
      }
      emit_store_float_pop(out,
                           inst.operands[0],
                           inst.type,
                           scratch_base,
                           XR_R11,
                           XR_RAX,
                           fixups,
                           target,
                           defined_globals);
      return;
    case mir::Instruction::MI_FEQ:
    case mir::Instruction::MI_FNE:
    case mir::Instruction::MI_FLT:
    case mir::Instruction::MI_FGT:
    case mir::Instruction::MI_FLE:
    case mir::Instruction::MI_FGE:
    case mir::Instruction::MI_FCMP:
      emit_load_float_operand(out,
                              inst.opcode == mir::Instruction::MI_FCMP ? inst.operands[0] : inst.operands[1],
                              inst.type,
                              scratch_base,
                              XR_R11,
                              XR_RAX,
                              fixups,
                              target,
                              defined_globals);
      emit_load_float_operand(out,
                              inst.opcode == mir::Instruction::MI_FCMP ? inst.operands[1] : inst.operands[2],
                              inst.type,
                              scratch_base,
                              XR_R11,
                              XR_RAX,
                              fixups,
                              target,
                              defined_globals);
      out.emit_fucomip_st1();
      out.emit_fstp_st0();
      if(inst.opcode == mir::Instruction::MI_FCMP) {
        return;
      }
      if(inst.opcode == mir::Instruction::MI_FEQ) {
        out.emit_setcc_r8(XC_E, XR_RCX);
        out.emit_setcc_r8(XC_NP, XR_RDX);
        out.emit_and_r64_r64(XR_RCX, XR_RDX);
        out.emit_and_r64_imm32(XR_RCX, 0xFF);
        if(inst.operands[0].reg != XR_RCX) {
          out.emit_mov_r64_r64(inst.operands[0].reg, XR_RCX);
        }
      } else if(inst.opcode == mir::Instruction::MI_FNE) {
        out.emit_setcc_r8(XC_NE, XR_RCX);
        out.emit_setcc_r8(XC_P, XR_RDX);
        out.emit_or_r64_r64(XR_RCX, XR_RDX);
        out.emit_and_r64_imm32(XR_RCX, 0xFF);
        if(inst.operands[0].reg != XR_RCX) {
          out.emit_mov_r64_r64(inst.operands[0].reg, XR_RCX);
        }
      } else {
        X86Condition cond = XC_A;
        if(inst.opcode == mir::Instruction::MI_FLT) {
          cond = XC_A;
        } else if(inst.opcode == mir::Instruction::MI_FLE) {
          cond = XC_AE;
        } else if(inst.opcode == mir::Instruction::MI_FGT) {
          cond = XC_B;
        } else if(inst.opcode == mir::Instruction::MI_FGE) {
          cond = XC_BE;
        }
        out.emit_setcc_r8(cond, XR_RCX);
        if(inst.opcode == mir::Instruction::MI_FGT ||
           inst.opcode == mir::Instruction::MI_FGE) {
          out.emit_setcc_r8(XC_NP, XR_RDX);
          out.emit_and_r64_r64(XR_RCX, XR_RDX);
        }
        out.emit_and_r64_imm32(XR_RCX, 0xFF);
        if(inst.operands[0].reg != XR_RCX) {
          out.emit_mov_r64_r64(inst.operands[0].reg, XR_RCX);
        }
      }
      return;
    case mir::Instruction::MI_FSTP:
      emit_store_float_pop(out,
                           inst.operands[0],
                           inst.type,
                           scratch_base,
                           XR_R11,
                           XR_RAX,
                           fixups,
                           target,
                           defined_globals);
      return;
    case mir::Instruction::MI_SITOFP:
    case mir::Instruction::MI_UITOFP: {
      const size_t source_width = inst.byte_count;
      emit_load_integer_conversion_source(out,
                                          inst.operands[1],
                                          source_width,
                                          inst.opcode == mir::Instruction::MI_SITOFP,
                                          XR_RAX,
                                          XR_R11,
                                          fixups,
                                          target,
                                          defined_globals);
      out.emit_mov_m64_r64(scratch_memory(scratch_base, 0), XR_RAX);
      out.emit_fild_m64(scratch_memory(scratch_base, 0));
      if(inst.opcode == mir::Instruction::MI_UITOFP && source_width == 8) {
        out.emit_test_r64_r64(XR_RAX, XR_RAX);
        const size_t skip_adjust = out.emit_jcc_rel32_placeholder(XC_NS);
        emit_write_bytes_to_memory(out,
                                   scratch_memory(scratch_base, 1),
                                   cy86_internal::encode_uint64(0x5F800000U, 4),
                                   XR_R10);
        out.emit_fld_m32(scratch_memory(scratch_base, 1));
        out.emit_faddp_st1();
        out.patch_rel32(skip_adjust, out.offset());
      }
      emit_store_float_pop(out,
                           inst.operands[0],
                           inst.type,
                           scratch_base,
                           XR_R11,
                           XR_RAX,
                           fixups,
                           target,
                           defined_globals);
      return;
    }
    case mir::Instruction::MI_FPTOSI:
    case mir::Instruction::MI_FPTOUI: {
      emit_load_float_operand(out,
                              inst.operands[1],
                              inst.type,
                              scratch_base,
                              XR_R11,
                              XR_RAX,
                              fixups,
                              target,
                              defined_globals);
      if(inst.opcode == mir::Instruction::MI_FPTOUI && inst.byte_count == 8) {
        emit_write_bytes_to_memory(out,
                                   scratch_memory(scratch_base, 1),
                                   cy86_internal::encode_uint64(0x5F000000U, 4),
                                   XR_R10);
        out.emit_fld_m32(scratch_memory(scratch_base, 1));
        out.emit_fucomip_st1();
        const size_t direct_path = out.emit_jcc_rel32_placeholder(XC_A);

        emit_write_bytes_to_memory(out,
                                   scratch_memory(scratch_base, 1),
                                   cy86_internal::encode_uint64(0x5F000000U, 4),
                                   XR_R10);
        out.emit_fld_m32(scratch_memory(scratch_base, 1));
        out.emit_fsubp_st1();
        out.emit_fisttp_m64(scratch_memory(scratch_base, 2));
        out.emit_mov_r64_m64(XR_RAX, scratch_memory(scratch_base, 2));
        out.emit_mov_r64_imm64(XR_RDX, 0x8000000000000000ULL);
        out.emit_or_r64_r64(XR_RAX, XR_RDX);
        const size_t done_path = out.emit_jmp_rel32_placeholder();

        out.patch_rel32(direct_path, out.offset());
        out.emit_fisttp_m64(scratch_memory(scratch_base, 2));
        out.emit_mov_r64_m64(XR_RAX, scratch_memory(scratch_base, 2));
        out.patch_rel32(done_path, out.offset());
      } else {
        out.emit_fisttp_m64(scratch_memory(scratch_base, 2));
        out.emit_mov_r64_m64(XR_RAX, scratch_memory(scratch_base, 2));
      }
      emit_store_reg_to_operand(out,
                                inst.operands[0],
                                inst.byte_count == 1 ? "i8" :
                                inst.byte_count == 2 ? "i16" :
                                inst.byte_count == 4 ? "i32" : "i64",
                                XR_RAX,
                                XR_R11,
                                fixups,
                                target,
                                defined_globals);
      return;
    }
    case mir::Instruction::MI_FPEXT:
    case mir::Instruction::MI_FPTRUNC:
      emit_load_float_operand(out,
                              inst.operands[1],
                              inst.byte_count == 4 ? "f32" :
                              inst.byte_count == 8 ? "f64" : "f80",
                              scratch_base,
                              XR_R11,
                              XR_RAX,
                              fixups,
                              target,
                              defined_globals);
      emit_store_float_pop(out,
                           inst.operands[0],
                           inst.type,
                           scratch_base,
                           XR_R11,
                           XR_RAX,
                           fixups,
                           target,
                           defined_globals);
      return;
    case mir::Instruction::MI_ADD:
      if(inst.operands[1].kind == mir::Operand::OP_REG) out.emit_add_r64_r64(inst.operands[0].reg, inst.operands[1].reg);
      else out.emit_add_r64_imm32(inst.operands[0].reg, static_cast<int32_t>(inst.operands[1].imm));
      return;
    case mir::Instruction::MI_SUB:
      if(inst.operands[1].kind == mir::Operand::OP_REG) out.emit_sub_r64_r64(inst.operands[0].reg, inst.operands[1].reg);
      else out.emit_sub_r64_imm32(inst.operands[0].reg, static_cast<int32_t>(inst.operands[1].imm));
      return;
    case mir::Instruction::MI_IMUL:
      if(inst.operands[1].kind == mir::Operand::OP_REG) out.emit_imul_r64_r64(inst.operands[0].reg, inst.operands[1].reg);
      else {
        const X64Register tmp = inst.operands[0].reg == XR_R11 ? XR_R10 : XR_R11;
        out.emit_mov_r64_imm64(tmp, static_cast<uint64_t>(inst.operands[1].imm));
        out.emit_imul_r64_r64(inst.operands[0].reg, tmp);
      }
      return;
    case mir::Instruction::MI_AND: out.emit_and_r64_r64(inst.operands[0].reg, inst.operands[1].reg); return;
    case mir::Instruction::MI_OR: out.emit_or_r64_r64(inst.operands[0].reg, inst.operands[1].reg); return;
    case mir::Instruction::MI_XOR: out.emit_xor_r64_r64(inst.operands[0].reg, inst.operands[1].reg); return;
    case mir::Instruction::MI_NEG: out.emit_neg_r64(inst.operands[0].reg); return;
    case mir::Instruction::MI_NOT: out.emit_not_r64(inst.operands[0].reg); return;
    case mir::Instruction::MI_BSWAP:
      if(inst.type == "i32" || inst.type == "u32") {
        out.emit_bswap_r32(inst.operands[0].reg);
      }
      else out.emit_bswap_r64(inst.operands[0].reg);
      return;
    case mir::Instruction::MI_CMP:
      if(inst.operands[0].kind == mir::Operand::OP_REG &&
         inst.operands[1].kind == mir::Operand::OP_IMM &&
         inst.operands[1].imm == 0) {
        if(inst.type == "i32" || inst.type == "u32") {
          out.emit_test_r32_r32(inst.operands[0].reg, inst.operands[0].reg);
        } else {
          out.emit_test_r64_r64(inst.operands[0].reg, inst.operands[0].reg);
        }
      } else {
        const auto pick_compare_addr_tmp =
            [&](X64Register avoid0, X64Register avoid1)
            {
              static const X64Register candidates[] = {
                  XR_R11, XR_R10, XR_R9, XR_R8, XR_RCX, XR_RDX, XR_RAX
              };
              for(size_t i = 0; i < sizeof(candidates) / sizeof(candidates[0]); ++i) {
                if(candidates[i] != avoid0 && candidates[i] != avoid1) {
                  return candidates[i];
                }
              }
              throw logic_error("no scratch register available for cmp memory operand");
            };
        const auto memory_operand =
            [&](const mir::Operand & operand,
                X64Register avoid0,
                X64Register avoid1)
            {
              if(operand.kind == mir::Operand::OP_FRAME) {
                return frame_memory(operand.offset);
              }
              if(operand.kind == mir::Operand::OP_DEREF) {
                return X86Memory(operand.reg, static_cast<int32_t>(operand.offset));
              }
              if(operand.kind == mir::Operand::OP_GLOBAL) {
                return emit_global_memory(out,
                                          operand.text,
                                          pick_compare_addr_tmp(avoid0, avoid1),
                                          fixups,
                                          use_indirect_rel32_for_imported_symbol(target,
                                                                                 defined_globals,
                                                                                 operand.text));
              }
              throw logic_error("unsupported cmp memory operand");
            };
        const mir::Operand & lhs = inst.operands[0];
        const mir::Operand & rhs = inst.operands[1];
        const bool type32 = inst.type == "i32" || inst.type == "u32";
        const bool type64 = inst.type == "i64" || inst.type == "u64" || inst.type == "ptr";
        const bool lhs_memory =
            lhs.kind == mir::Operand::OP_FRAME ||
            lhs.kind == mir::Operand::OP_DEREF ||
            lhs.kind == mir::Operand::OP_GLOBAL;
        const bool rhs_memory =
            rhs.kind == mir::Operand::OP_FRAME ||
            rhs.kind == mir::Operand::OP_DEREF ||
            rhs.kind == mir::Operand::OP_GLOBAL;
        if(lhs.kind == mir::Operand::OP_REG && rhs.kind == mir::Operand::OP_IMM) {
          if(type32) {
            out.emit_cmp_r32_imm32(lhs.reg, static_cast<uint32_t>(rhs.imm));
          } else if(type64) {
            out.emit_cmp_r64_imm32(lhs.reg, static_cast<int32_t>(rhs.imm));
          } else {
            throw logic_error("unsupported cmp register/immediate type");
          }
        } else if(lhs.kind == mir::Operand::OP_REG && rhs_memory) {
          const X86Memory rhs_mem = memory_operand(rhs, lhs.reg, XR_RBP);
          if(type32) {
            out.emit_cmp_r32_m32(lhs.reg, rhs_mem);
          } else if(type64) {
            out.emit_cmp_r64_m64(lhs.reg, rhs_mem);
          } else {
            throw logic_error("unsupported cmp register/memory type");
          }
        } else if(lhs_memory && rhs.kind == mir::Operand::OP_IMM) {
          const X86Memory lhs_mem = memory_operand(lhs, XR_RBP, XR_RBP);
          if(inst.type == "i8" || inst.type == "u8") {
            out.emit_cmp_m8_imm8(lhs_mem, static_cast<uint8_t>(rhs.imm));
          } else if(inst.type == "i16" || inst.type == "u16") {
            out.emit_cmp_m16_imm16(lhs_mem, static_cast<uint16_t>(rhs.imm));
          } else if(type32) {
            out.emit_cmp_m32_imm32(lhs_mem, static_cast<uint32_t>(rhs.imm));
          } else if(type64) {
            out.emit_cmp_m64_imm32(lhs_mem, static_cast<int32_t>(rhs.imm));
          } else {
            throw logic_error("unsupported cmp memory/immediate type");
          }
        } else if(lhs_memory && rhs.kind == mir::Operand::OP_REG) {
          const X86Memory lhs_mem = memory_operand(lhs, rhs.reg, XR_RBP);
          if(type32) {
            out.emit_cmp_m32_r32(lhs_mem, rhs.reg);
          } else if(type64) {
            out.emit_cmp_m64_r64(lhs_mem, rhs.reg);
          } else {
            throw logic_error("unsupported cmp memory/register type");
          }
        } else if(type32) {
          out.emit_cmp_r32_r32(lhs.reg, rhs.reg);
        } else {
          out.emit_cmp_r64_r64(lhs.reg, rhs.reg);
        }
      }
      return;
    case mir::Instruction::MI_TEST:
      if(inst.type == "i32" || inst.type == "u32") {
        out.emit_test_r32_r32(inst.operands[0].reg, inst.operands[1].reg);
      } else {
        out.emit_test_r64_r64(inst.operands[0].reg, inst.operands[1].reg);
      }
      return;
    case mir::Instruction::MI_SETCC: out.emit_setcc_r8(inst.condition, inst.operands[0].reg); return;
    case mir::Instruction::MI_MOVZX:
      if(inst.operands[0].reg != inst.operands[1].reg) out.emit_mov_r64_r64(inst.operands[0].reg, inst.operands[1].reg);
      out.emit_and_r64_imm32(inst.operands[0].reg, 0xFF);
      return;
    case mir::Instruction::MI_SEXT: {
      const X64Register dst = inst.operands[0].reg;
      if(inst.byte_count == 4) {
        out.emit_movsxd_r64_r32(dst, dst);
      } else {
        const unsigned char shift = static_cast<unsigned char>((8 - inst.byte_count) * 8);
        out.emit_shl_r64_imm8(dst, shift);
        out.emit_sar_r64_imm8(dst, shift);
      }
      return;
    }
    case mir::Instruction::MI_ZEXT: {
      const X64Register dst = inst.operands[0].reg;
      if(inst.byte_count == 4) {
        out.emit_mov_r32_r32(dst, dst);
      } else if(inst.byte_count == 2) {
        out.emit_and_r64_imm32(dst, 0xFFFF);
      } else if(inst.byte_count == 1) {
        out.emit_and_r64_imm32(dst, 0xFF);
      }
      return;
    }
    case mir::Instruction::MI_CQO: out.emit_cqo(); return;
    case mir::Instruction::MI_IDIV: out.emit_idiv_r64(inst.operands[0].reg); return;
    case mir::Instruction::MI_DIV: out.emit_div_r64(inst.operands[0].reg); return;
    case mir::Instruction::MI_SHL_CL: out.emit_shl_r64_cl(inst.operands[0].reg); return;
    case mir::Instruction::MI_SHR_CL: out.emit_shr_r64_cl(inst.operands[0].reg); return;
    case mir::Instruction::MI_SAR_CL: out.emit_sar_r64_cl(inst.operands[0].reg); return;
    case mir::Instruction::MI_TLS_ADDR: {
      Fixup fixup;
      fixup.kind = Fixup::FX_CALL_REL32;
      fixup.patch_offset = out.emit_call_rel32_placeholder();
      fixup.target = inst.operands[1].text;
      fixups->push_back(fixup);
      if(inst.operands[0].reg != XR_RAX) {
        out.emit_mov_r64_r64(inst.operands[0].reg, XR_RAX);
      }
      return;
    }
    case mir::Instruction::MI_CALL: {
      Fixup fixup;
      fixup.kind = Fixup::FX_CALL_REL32;
      fixup.patch_offset = out.emit_call_rel32_placeholder();
      fixup.target = inst.operands[0].text;
      fixups->push_back(fixup);
      return;
    }
    case mir::Instruction::MI_CALL_INDIRECT: out.emit_call_r64(inst.operands[0].reg); return;
    case mir::Instruction::MI_COPY_BYTES: {
      const X64Register dst = inst.operands[0].reg;
      const X64Register src = inst.operands[1].reg;
      if(inst.byte_count <= 16) {
        size_t copied = 0;
        while(copied < inst.byte_count) {
          const size_t remaining = inst.byte_count - copied;
          if(remaining >= 8) {
            out.emit_mov_r64_m64(XR_RCX, X86Memory(src, static_cast<int32_t>(copied)));
            out.emit_mov_m64_r64(X86Memory(dst, static_cast<int32_t>(copied)), XR_RCX);
            copied += 8;
          } else if(remaining >= 4) {
            out.emit_mov_r32_m32(XR_RCX, X86Memory(src, static_cast<int32_t>(copied)));
            out.emit_mov_m32_r64(X86Memory(dst, static_cast<int32_t>(copied)), XR_RCX);
            copied += 4;
          } else if(remaining >= 2) {
            out.emit_movzx_r64_m16(XR_RCX, X86Memory(src, static_cast<int32_t>(copied)));
            out.emit_mov_m16_r64(X86Memory(dst, static_cast<int32_t>(copied)), XR_RCX);
            copied += 2;
          } else {
            out.emit_movzx_r64_m8(XR_RCX, X86Memory(src, static_cast<int32_t>(copied)));
            out.emit_mov_m8_r64(X86Memory(dst, static_cast<int32_t>(copied)), XR_RCX);
            copied += 1;
          }
        }
        return;
      }
      if(dst != XR_RDI) out.emit_mov_r64_r64(XR_RDI, dst);
      if(src != XR_RSI) out.emit_mov_r64_r64(XR_RSI, src);
      out.emit_mov_r64_imm64(XR_RCX, inst.byte_count);
      out.emit_cld();
      out.emit_rep_movsb();
      return;
    }
    case mir::Instruction::MI_ZERO_BYTES:
      if(inst.operands[0].reg != XR_RDI) out.emit_mov_r64_r64(XR_RDI, inst.operands[0].reg);
      out.emit_xor_r64_r64(XR_RAX, XR_RAX);
      out.emit_mov_r64_imm64(XR_RCX, inst.byte_count);
      out.emit_cld();
      out.emit_rep_stosb();
      return;
    case mir::Instruction::MI_EH_PUSH: {
      if(current_function.host_eh_enabled) {
        return;
      }
      Fixup fixup;
      out.emit_sub_r64_imm32(XR_RSP, static_cast<int32_t>(eh_handler_record_size()));

      fixup.kind = use_indirect_rel32_for_imported_symbol(target,
                                                    defined_globals,
                                                    eh_runtime::kEhTopSymbol)
          ? Fixup::FX_INDIRECT_REL32
          : Fixup::FX_RIP_REL32;
      fixup.patch_offset = fixup.kind == Fixup::FX_INDIRECT_REL32
          ? out.emit_mov_r64_rip_rel32_placeholder(XR_R11)
          : out.emit_lea_r64_rip_rel32_placeholder(XR_R11);
      fixup.target = eh_runtime::kEhTopSymbol;
      fixups->push_back(fixup);

      out.emit_mov_r64_m64(XR_RAX, X86Memory(XR_R11, 0));
      out.emit_mov_m64_r64(deref_memory(XR_RSP, 0), XR_RAX);

      fixup.kind = use_indirect_rel32_for_imported_symbol(target,
                                                    defined_symbols,
                                                    inst.operands[0].text)
          ? Fixup::FX_INDIRECT_REL32
          : Fixup::FX_RIP_REL32;
      fixup.patch_offset = fixup.kind == Fixup::FX_INDIRECT_REL32
          ? out.emit_mov_r64_rip_rel32_placeholder(XR_RAX)
          : out.emit_lea_r64_rip_rel32_placeholder(XR_RAX);
      fixup.target = inst.operands[0].text;
      fixups->push_back(fixup);
      out.emit_mov_m64_r64(deref_memory(XR_RSP, 8), XR_RAX);

      out.emit_mov_m64_r64(deref_memory(XR_RSP, 16), XR_RBP);
      out.emit_lea_r64_m(XR_RAX, deref_memory(XR_RSP, static_cast<long long>(eh_handler_record_size())));
      out.emit_mov_m64_r64(deref_memory(XR_RSP, 24), XR_RAX);
      emit_eh_save_callee_saved(out);

      out.emit_lea_r64_m(XR_RAX, deref_memory(XR_RSP, 0));
      out.emit_mov_m64_r64(X86Memory(XR_R11, 0), XR_RAX);
      return;
    }
    case mir::Instruction::MI_EH_POP: {
      if(current_function.host_eh_enabled) {
        return;
      }
      Fixup fixup;
      fixup.kind = use_indirect_rel32_for_imported_symbol(target,
                                                    defined_globals,
                                                    eh_runtime::kEhTopSymbol)
          ? Fixup::FX_INDIRECT_REL32
          : Fixup::FX_RIP_REL32;
      fixup.patch_offset = fixup.kind == Fixup::FX_INDIRECT_REL32
          ? out.emit_mov_r64_rip_rel32_placeholder(XR_R11)
          : out.emit_lea_r64_rip_rel32_placeholder(XR_R11);
      fixup.target = eh_runtime::kEhTopSymbol;
      fixups->push_back(fixup);
      out.emit_mov_r64_m64(XR_RAX, deref_memory(XR_RSP, 0));
      out.emit_mov_m64_r64(X86Memory(XR_R11, 0), XR_RAX);
      out.emit_add_r64_imm32(XR_RSP, static_cast<int32_t>(eh_handler_record_size()));
      return;
    }
    case mir::Instruction::MI_LOAD_EXCEPTION: {
      if(current_function.host_eh_enabled) {
        emit_load_reg_from_frame(out,
                                 "ptr",
                                 inst.operands[0].reg,
                                 current_function.host_eh_exception_offset);
        return;
      }
      Fixup fixup;
      fixup.kind = use_indirect_rel32_for_imported_symbol(target,
                                                    defined_globals,
                                                    eh_runtime::kEhValueSymbol)
          ? Fixup::FX_INDIRECT_REL32
          : Fixup::FX_RIP_REL32;
      fixup.patch_offset = fixup.kind == Fixup::FX_INDIRECT_REL32
          ? out.emit_mov_r64_rip_rel32_placeholder(XR_R11)
          : out.emit_lea_r64_rip_rel32_placeholder(XR_R11);
      fixup.target = eh_runtime::kEhValueSymbol;
      fixups->push_back(fixup);
      emit_load_reg_from_memory(out, inst.type, inst.operands[0].reg, X86Memory(XR_R11, 0));
      return;
    }
    case mir::Instruction::MI_LOAD_EXCEPTION_SELECTOR: {
      if(current_function.host_eh_enabled) {
        emit_load_reg_from_frame(out,
                                 inst.type,
                                 inst.operands[0].reg,
                                 current_function.host_eh_selector_offset);
        return;
      }
      throw logic_error("MI_LOAD_EXCEPTION_SELECTOR requires host EH-enabled function");
    }
    case mir::Instruction::MI_THROW:
    case mir::Instruction::MI_RESUME: {
      if(current_function.host_eh_enabled) {
        if(inst.opcode != mir::Instruction::MI_RESUME) {
          throw logic_error("host EH path should not lower MI_THROW");
        }
        emit_load_reg_from_frame(out,
                                 "ptr",
                                 XR_RDI,
                                 current_function.host_eh_exception_offset);
        Fixup fixup;
        fixup.kind = Fixup::FX_CALL_REL32;
        fixup.patch_offset = out.emit_call_rel32_placeholder();
        fixup.target =
            symbol_linkage::internal_symbol_from_name("__external_runtime::_Unwind_Resume");
        fixups->push_back(fixup);
        out.emit_ud2();
        return;
      }
      Fixup fixup;
      if(inst.opcode == mir::Instruction::MI_THROW) {
        if(inst.operands[0].reg != XR_RAX) {
          out.emit_mov_r64_r64(XR_RAX, inst.operands[0].reg);
        }
        fixup.kind = use_indirect_rel32_for_imported_symbol(target,
                                                      defined_globals,
                                                      eh_runtime::kEhValueSymbol)
            ? Fixup::FX_INDIRECT_REL32
            : Fixup::FX_RIP_REL32;
        fixup.patch_offset = fixup.kind == Fixup::FX_INDIRECT_REL32
            ? out.emit_mov_r64_rip_rel32_placeholder(XR_R11)
            : out.emit_lea_r64_rip_rel32_placeholder(XR_R11);
        fixup.target = eh_runtime::kEhValueSymbol;
        fixups->push_back(fixup);
        out.emit_mov_m64_r64(X86Memory(XR_R11, 0), XR_RAX);
      }

      fixup.kind = use_indirect_rel32_for_imported_symbol(target,
                                                    defined_globals,
                                                    eh_runtime::kEhTopSymbol)
          ? Fixup::FX_INDIRECT_REL32
          : Fixup::FX_RIP_REL32;
      fixup.patch_offset = fixup.kind == Fixup::FX_INDIRECT_REL32
          ? out.emit_mov_r64_rip_rel32_placeholder(XR_R11)
          : out.emit_lea_r64_rip_rel32_placeholder(XR_R11);
      fixup.target = eh_runtime::kEhTopSymbol;
      fixups->push_back(fixup);
      out.emit_mov_r64_m64(XR_RCX, X86Memory(XR_R11, 0));
      out.emit_test_r64_r64(XR_RCX, XR_RCX);
      const size_t has_handler_patch = out.emit_jcc_rel32_placeholder(XC_NE);

      fixup.kind = use_indirect_rel32_for_imported_symbol(target,
                                                    defined_globals,
                                                    eh_runtime::kEhValueSymbol)
          ? Fixup::FX_INDIRECT_REL32
          : Fixup::FX_RIP_REL32;
      fixup.patch_offset = fixup.kind == Fixup::FX_INDIRECT_REL32
          ? out.emit_mov_r64_rip_rel32_placeholder(XR_RAX)
          : out.emit_lea_r64_rip_rel32_placeholder(XR_RAX);
      fixup.target = eh_runtime::kEhValueSymbol;
      fixups->push_back(fixup);
      out.emit_mov_r64_m64(XR_RDI, X86Memory(XR_RAX, 0));
      fixup.kind = Fixup::FX_CALL_REL32;
      fixup.patch_offset = out.emit_call_rel32_placeholder();
        fixup.target = eh_runtime::kEhUnhandledSymbol;
      fixups->push_back(fixup);
      out.emit_ud2();

      patch_rel32_local(out, has_handler_patch, 0, out.offset());
      out.emit_mov_r64_m64(XR_RDX, deref_memory(XR_RCX, 0));
      out.emit_mov_m64_r64(X86Memory(XR_R11, 0), XR_RDX);
      out.emit_mov_r64_m64(XR_RAX, deref_memory(XR_RCX, 8));
      emit_eh_restore_callee_saved(out, XR_RCX);
      out.emit_mov_r64_m64(XR_RBP, deref_memory(XR_RCX, 16));
      out.emit_mov_r64_m64(XR_RSP, deref_memory(XR_RCX, 24));
      out.emit_jmp_r64(XR_RAX);
      return;
    }
    case mir::Instruction::MI_JMP: {
      const size_t patch_offset = out.emit_jmp_rel32_placeholder();
      const string target = current_function.name + "::" + inst.operands[0].text;
      const size_t split = target.find("::");
      if(split == string::npos) {
        throw logic_error("invalid local jump target");
      }
      (void)patch_offset;
      throw logic_error("unpatched jump placeholder");
    }
    case mir::Instruction::MI_JMP_INDIRECT:
      out.emit_jmp_r64(inst.operands[0].reg);
      return;
    case mir::Instruction::MI_JCC: {
      const size_t patch_offset = out.emit_jcc_rel32_placeholder(inst.condition);
      const string target = current_function.name + "::" + inst.operands[0].text;
      const size_t split = target.find("::");
      if(split == string::npos) {
        throw logic_error("invalid local jcc target");
      }
      (void)patch_offset;
      throw logic_error("unpatched jcc placeholder");
    }
    case mir::Instruction::MI_JNE: {
      const size_t patch_offset = out.emit_jcc_rel32_placeholder(XC_NE);
      const string target = current_function.name + "::" + inst.operands[0].text;
      const size_t split = target.find("::");
      if(split == string::npos) {
        throw logic_error("invalid local jne target");
      }
      (void)patch_offset;
      throw logic_error("unpatched jne placeholder");
    }
    case mir::Instruction::MI_RET:
      if(!inst.operands.empty() && inst.operands[0].reg != XR_RAX) {
        out.emit_mov_r64_r64(XR_RAX, inst.operands[0].reg);
      }
      emit_function_epilogue(out, current_function);
      out.emit_ret();
      return;
    case mir::Instruction::MI_FRET:
      if(inst.type == "f32" || inst.type == "f64") {
        emit_load_float_operand_to_xmm(out,
                                       XMM_0,
                                       inst.operands[0],
                                       inst.type,
                                       scratch_base,
                                       XR_R11,
                                       XR_RAX,
                                       fixups,
                                       target,
                                       defined_globals);
        emit_function_epilogue(out, current_function);
        out.emit_ret();
        return;
      }
      emit_load_float_operand(out,
                              inst.operands[0],
                              inst.type,
                              scratch_base,
                              XR_R11,
                              XR_RAX,
                              fixups,
                              target,
                              defined_globals);
      emit_function_epilogue(out, current_function);
      out.emit_ret();
      return;
    case mir::Instruction::MI_EXIT:
      emit_exit_sequence(out, native_format::hooks_for_target(target));
      return;
  }
}

set<string> collect_defined_global_symbols(const mir::Program & program)
{
  set<string> out;
  for(size_t i = 0; i < program.globals.size(); ++i) {
    out.insert(program.globals[i].name);
  }
  return out;
}

set<string> collect_defined_symbols(const mir::Program & program)
{
  set<string> out = collect_defined_global_symbols(program);
  for(size_t i = 0; i < program.functions.size(); ++i) {
    out.insert(program.functions[i].name);
    for(size_t bi = 0; bi < program.functions[i].blocks.size(); ++bi) {
      out.insert(block_symbol(program.functions[i].name,
                              program.functions[i].blocks[bi].label));
    }
  }
  return out;
}

vector<size_t> host_eh_successors(const mir::Function & function, size_t block_index)
{
  map<string, size_t> block_indices;
  for(size_t bi = 0; bi < function.blocks.size(); ++bi) {
    block_indices[function.blocks[bi].label] = bi;
  }

  vector<size_t> out;
  const mir::Block & block = function.blocks[block_index];
  if(block.instructions.empty()) {
    if(block_index + 1 < function.blocks.size()) {
      out.push_back(block_index + 1);
    }
    return out;
  }

  const mir::Instruction & last = block.instructions.back();
  const auto append_target = [&](const string & label) {
    map<string, size_t>::const_iterator it = block_indices.find(label);
    if(it == block_indices.end()) {
      throw logic_error("unknown host EH successor block " + function.name + "$" + label);
    }
    if(find(out.begin(), out.end(), it->second) == out.end()) {
      out.push_back(it->second);
    }
  };

  if(last.opcode == mir::Instruction::MI_RET ||
     last.opcode == mir::Instruction::MI_FRET ||
     last.opcode == mir::Instruction::MI_EXIT ||
     last.opcode == mir::Instruction::MI_RESUME ||
     last.opcode == mir::Instruction::MI_THROW) {
    return out;
  }

  if(last.opcode == mir::Instruction::MI_JMP) {
    append_target(last.operands[0].text);
    if(block.instructions.size() >= 2) {
      const mir::Instruction & prev = block.instructions[block.instructions.size() - 2];
      if(is_conditional_jump(prev)) {
        append_target(prev.operands[0].text);
      }
    }
    return out;
  }

  if(is_conditional_jump(last)) {
    append_target(last.operands[0].text);
    if(block_index + 1 < function.blocks.size()) {
      out.push_back(block_index + 1);
    }
    return out;
  }

  if(block_index + 1 < function.blocks.size()) {
    out.push_back(block_index + 1);
  }
  return out;
}

map<string, vector<string> > compute_host_eh_entry_stacks(const mir::Function & function)
{
  map<string, vector<string> > entry_stacks;
  if(function.blocks.empty()) {
    return entry_stacks;
  }
  set<string> landingpad_block_labels;
  map<string, string> landingpad_block_symbols;
  map<string, string> landingpad_entry_symbols;
  for(size_t bi = 0; bi < function.blocks.size(); ++bi) {
    for(size_t ii = 0; ii < function.blocks[bi].instructions.size(); ++ii) {
      const mir::Instruction & inst = function.blocks[bi].instructions[ii];
      if(inst.opcode != mir::Instruction::MI_EH_PUSH || inst.operands.empty()) {
        continue;
      }
      const string & landingpad_symbol = inst.operands[0].text;
      const size_t split = landingpad_symbol.rfind('$');
      if(split == string::npos) {
          throw logic_error("invalid host EH landing pad symbol " + landingpad_symbol);
      }
      const string landingpad_label = landingpad_symbol.substr(split + 1);
      landingpad_block_labels.insert(landingpad_label);
      landingpad_block_symbols[landingpad_label] = landingpad_symbol;
    }
  }
  for(size_t bi = 0; bi < function.blocks.size(); ++bi) {
    map<string, string>::const_iterator landingpad =
        landingpad_block_symbols.find(function.blocks[bi].label);
    if(landingpad == landingpad_block_symbols.end()) {
      continue;
    }
    const vector<size_t> successors = host_eh_successors(function, bi);
    if(successors.size() == 1) {
      landingpad_entry_symbols[function.blocks[successors[0]].label] = landingpad->second;
    }
  }

  const auto assign_entry_stack = [&](const string & label,
                                      const vector<string> & stack,
                                      vector<size_t> & worklist) {
    map<string, vector<string> >::iterator found = entry_stacks.find(label);
    if(found == entry_stacks.end()) {
      entry_stacks[label] = stack;
      for(size_t bi = 0; bi < function.blocks.size(); ++bi) {
        if(function.blocks[bi].label == label) {
          worklist.push_back(bi);
          return;
        }
      }
      throw logic_error("unknown host EH block " + function.name + "$" + label);
    }
    if(found->second != stack) {
      throw logic_error("inconsistent host EH region stack at " +
                        function.name + "$" + label);
    }
  };

  vector<size_t> worklist(1, 0);
  entry_stacks[function.blocks[0].label] = vector<string>();
  while(!worklist.empty()) {
    const size_t bi = worklist.back();
    worklist.pop_back();
    const mir::Block & block = function.blocks[bi];
    vector<string> exit_stack = entry_stacks.find(block.label)->second;
    bool has_implicit_landingpad_region =
        landingpad_block_labels.count(block.label) != 0;
    for(size_t ii = 0; ii < block.instructions.size(); ++ii) {
      const mir::Instruction & inst = block.instructions[ii];
      if(inst.opcode == mir::Instruction::MI_EH_PUSH) {
        const string & landingpad_symbol = inst.operands[0].text;
        const size_t split = landingpad_symbol.rfind('$');
        if(split == string::npos) {
          throw logic_error("invalid host EH landing pad symbol " + landingpad_symbol);
        }
        assign_entry_stack(landingpad_symbol.substr(split + 1), exit_stack, worklist);
        exit_stack.push_back(landingpad_symbol);
        continue;
      }
      if(inst.opcode == mir::Instruction::MI_EH_POP) {
        if(has_implicit_landingpad_region) {
          has_implicit_landingpad_region = false;
          continue;
        }
        if(!exit_stack.empty()) {
          exit_stack.pop_back();
          continue;
        }
        if(exit_stack.empty()) {
          ostringstream outmsg;
          outmsg << "host EH region pop underflow in " << function.name
                 << " block " << block.label
                 << " instruction " << ii
                 << " during entry-stack analysis";
          throw logic_error(outmsg.str());
        }
      }
    }
    const vector<size_t> successors = host_eh_successors(function, bi);
    for(size_t si = 0; si < successors.size(); ++si) {
      const string & successor_label = function.blocks[successors[si]].label;
      vector<string> successor_stack = exit_stack;
      string landingpad_symbol;
      map<string, string>::const_iterator direct_landingpad =
          landingpad_block_symbols.find(successor_label);
      if(direct_landingpad != landingpad_block_symbols.end()) {
        landingpad_symbol = direct_landingpad->second;
      } else {
        map<string, string>::const_iterator landingpad_entry =
            landingpad_entry_symbols.find(successor_label);
        if(landingpad_entry != landingpad_entry_symbols.end()) {
          landingpad_symbol = landingpad_entry->second;
        }
      }
      if(!landingpad_symbol.empty() &&
         !successor_stack.empty() &&
         successor_stack.back() == landingpad_symbol) {
        successor_stack.pop_back();
      }
      assign_entry_stack(successor_label, successor_stack, worklist);
    }
  }

  return entry_stacks;
}

FunctionLayout measure_function_layout(const mir::Function & function,
                                       cy86_internal::NativeTarget target,
                                       const set<string> & defined_symbols,
                                       const set<string> & defined_globals)
{
  X86Assembler out(true);
  emit_function_prologue(out, function);
  FunctionLayout layout;
  set<string> landingpad_labels;
  if(function.host_eh_enabled) {
    for(size_t bi = 0; bi < function.blocks.size(); ++bi) {
      for(size_t ii = 0; ii < function.blocks[bi].instructions.size(); ++ii) {
        const mir::Instruction & inst = function.blocks[bi].instructions[ii];
        if(inst.opcode == mir::Instruction::MI_EH_PUSH && !inst.operands.empty()) {
          landingpad_labels.insert(inst.operands[0].text);
        }
      }
    }
  }
  for(size_t bi = 0; bi < function.blocks.size(); ++bi) {
    layout.block_offsets[function.blocks[bi].label] = out.offset();
    vector<size_t> & instruction_offsets =
        layout.instruction_offsets[function.blocks[bi].label];
    if(function.host_eh_enabled &&
       landingpad_labels.count(block_symbol(function.name, function.blocks[bi].label)) != 0) {
      emit_store_reg_to_frame(out,
                              "ptr",
                              function.host_eh_exception_offset,
                              XR_RAX);
      emit_store_reg_to_frame(out,
                              "i32",
                              function.host_eh_selector_offset,
                              XR_RDX);
    }
    for(size_t ii = 0; ii < function.blocks[bi].instructions.size(); ++ii) {
      const mir::Instruction & inst = function.blocks[bi].instructions[ii];
      instruction_offsets.push_back(out.offset());
      if(inst.opcode == mir::Instruction::MI_JMP) {
        out.emit_jmp_rel32_placeholder();
      } else if(is_conditional_jump(inst)) {
        out.emit_jcc_rel32_placeholder(jump_condition(inst));
      } else {
        vector<Fixup> ignored;
        emit_instruction(out,
                         inst,
                         -static_cast<long long>(function.stack_size),
                         function,
                         target,
                         defined_symbols,
                         defined_globals,
                         &ignored);
      }
    }
  }
  layout.size = out.size();
  return layout;
}

ObjectLayout layout_object(const mir::Program & program)
{
  ObjectLayout layout;
  const ThreadLocalSectionInfo tls_info =
      thread_local_section_info_for_target(program.target);
  const cy86_internal::NativeTarget target =
      native_format::hooks_for_target_text(program.target).target;
  const set<string> defined_symbols = collect_defined_symbols(program);
  const set<string> defined_globals = collect_defined_global_symbols(program);
  size_t code_offset = 0;
  for(size_t i = 0; i < program.functions.size(); ++i) {
    code_offset = align_up(code_offset, 16);
    layout.function_offsets[program.functions[i].name] = code_offset;
    layout.function_layouts[program.functions[i].name] =
        measure_function_layout(program.functions[i],
                                target,
                                defined_symbols,
                                defined_globals);
    code_offset += layout.function_layouts[program.functions[i].name].size;
  }
  for(size_t i = 0; i < program.globals.size(); ++i) {
    if(!program.globals[i].thread_local_storage ||
       program.globals[i].thread_local_wrapper_symbol.empty()) {
      continue;
    }
    code_offset = align_up(code_offset, 16);
    layout.thread_local_wrapper_offsets[program.globals[i].name] = code_offset;
    code_offset += thread_local_wrapper_size(program.target);
  }
  layout.code_size = code_offset;

  size_t data_offset = 0;
  size_t readonly_data_offset = 0;
  size_t thread_local_data_template_offset = 0;
  size_t thread_local_bss_template_offset = 0;
  size_t thread_local_descriptor_offset = 0;
  for(size_t i = 0; i < program.globals.size(); ++i) {
    const mir::GlobalDefinition & global = program.globals[i];
    if(global.thread_local_storage) {
      if(tls_info.abi == ThreadLocalSectionInfo::ABI_DIRECT_NATIVE) {
        data_offset = align_up(data_offset, global_alignment(global));
        layout.global_offsets[global.name] = data_offset;
        data_offset += global_size(global);
        continue;
      }
      size_t & template_offset =
          thread_local_global_uses_macho_zerofill_template(global, tls_info)
              ? thread_local_bss_template_offset
              : thread_local_data_template_offset;
      template_offset = align_up(template_offset, global_alignment(global));
      layout.thread_local_template_offsets[global.name] = template_offset;
      template_offset += global_size(global);
      if(tls_info.abi == ThreadLocalSectionInfo::ABI_MACHO_TLV) {
        layout.thread_local_descriptor_offsets[global.name] = thread_local_descriptor_offset;
        thread_local_descriptor_offset += 24;
      } else if(tls_info.abi == ThreadLocalSectionInfo::ABI_EMUTLS) {
        data_offset = align_up(data_offset, 8);
        layout.thread_local_descriptor_offsets[global.name] = data_offset;
        data_offset += 32;
      } else {
        layout.thread_local_descriptor_offsets[global.name] =
            layout.thread_local_template_offsets[global.name];
      }
      continue;
    }
    if(global.readonly) {
      readonly_data_offset = align_up(readonly_data_offset, global_alignment(global));
      layout.readonly_global_offsets[global.name] = readonly_data_offset;
      readonly_data_offset += global_size(global);
    } else {
      data_offset = align_up(data_offset, global_alignment(global));
      layout.global_offsets[global.name] = data_offset;
      data_offset += global_size(global);
    }
  }
  layout.data_size = data_offset;
  layout.readonly_data_size = readonly_data_offset;
  layout.thread_local_template_data_size = thread_local_data_template_offset;
  layout.thread_local_template_bss_size = thread_local_bss_template_offset;
  layout.thread_local_descriptor_size =
      tls_info.abi == ThreadLocalSectionInfo::ABI_MACHO_TLV ? thread_local_descriptor_offset : 0;
  return layout;
}

void append_relocation(vector<mobj::Relocation> & relocs,
                       mobj::Symbol::Section section,
                       size_t offset,
                       mobj::Relocation::Kind kind,
                       const string & symbol,
                       long long addend)
{
  mobj::Relocation reloc;
  reloc.section = section;
  reloc.offset = offset;
  reloc.kind = kind;
  reloc.symbol = symbol;
  reloc.addend = addend;
  relocs.push_back(reloc);
}

vector<unsigned char> build_code_bytes(const mir::Program & program,
                                       const ObjectLayout & layout,
                                       const map<string, symbol_linkage::SymbolIdentity> & exports,
                                       vector<mobj::Relocation> & relocations,
                                       vector<hosteh::HostEhFunctionInfo> * host_eh_functions)
{
  const native_format::Hooks & target_hooks =
      native_format::hooks_for_target_text(program.target);
  const ThreadLocalSectionInfo tls_info =
      thread_local_section_info_for_target(program.target);
  const cy86_internal::NativeTarget target = target_hooks.target;
  const set<string> defined_symbols = collect_defined_symbols(program);
  const set<string> defined_globals = collect_defined_global_symbols(program);
  X86Assembler out;
  for(size_t fi = 0; fi < program.functions.size(); ++fi) {
    const mir::Function & function = program.functions[fi];
    const size_t want_offset = layout.function_offsets.find(function.name)->second;
    if(out.offset() > want_offset) {
      ostringstream outmsg;
      outmsg << "object function start drift before " << function.name
             << " expected_start " << want_offset
             << " actual_start " << out.offset();
      if(fi > 0) {
        outmsg << " previous_function " << program.functions[fi - 1].name;
      }
      throw logic_error(outmsg.str());
    }
    while(out.offset() < want_offset) {
      out.append(vector<unsigned char>(want_offset - out.offset(), 0x90));
    }
    const size_t function_start = out.offset();
    emit_function_prologue(out, function);

    hosteh::HostEhFunctionInfo host_eh_function;
    set<string> landingpad_labels;
    map<string, vector<string> > host_eh_entry_stacks;
    if(function.host_eh_enabled) {
      host_eh_function.function_name = function.name;
      host_eh_function.function_offset = want_offset;
      host_eh_entry_stacks = compute_host_eh_entry_stacks(function);
      for(size_t bi = 0; bi < function.blocks.size(); ++bi) {
        for(size_t ii = 0; ii < function.blocks[bi].instructions.size(); ++ii) {
          const mir::Instruction & inst = function.blocks[bi].instructions[ii];
          if(inst.opcode == mir::Instruction::MI_EH_PUSH && !inst.operands.empty()) {
            landingpad_labels.insert(inst.operands[0].text);
          }
        }
      }
    }
    const bool track_host_eh_call_sites =
        function.host_eh_enabled && !landingpad_labels.empty();

    const FunctionLayout & func_layout = layout.function_layouts.find(function.name)->second;
    size_t host_eh_region_start = function_start;
    size_t host_eh_region_landingpad_offset = 0;
    string host_eh_region_landingpad_symbol;
    bool host_eh_region_open = false;
    const auto resolve_host_eh_region =
        [&](const vector<string> & active_host_eh_regions,
            bool is_landingpad_block,
            size_t start_offset) -> hosteh::HostEhCallSite {
      hosteh::HostEhCallSite call_site;
      call_site.start = start_offset - function_start;
      if(is_landingpad_block || active_host_eh_regions.empty()) {
        return call_site;
      }
      const string landingpad_symbol = active_host_eh_regions.back();
      const size_t split = landingpad_symbol.rfind('$');
      if(split == string::npos) {
        throw logic_error("invalid host EH landing pad symbol " + landingpad_symbol);
      }
      const string block_label = landingpad_symbol.substr(split + 1);
      map<string, size_t>::const_iterator landingpad =
          func_layout.block_offsets.find(block_label);
      if(landingpad == func_layout.block_offsets.end()) {
        throw logic_error("unknown host EH landing pad block " + landingpad_symbol);
      }
      call_site.landingpad_offset = landingpad->second;
      call_site.landingpad_symbol = landingpad_symbol;
      return call_site;
    };
    const auto append_host_eh_call_site = [&](const hosteh::HostEhCallSite & call_site) {
      if(call_site.length == 0) {
        return;
      }
      if(!host_eh_function.call_sites.empty()) {
        hosteh::HostEhCallSite & last = host_eh_function.call_sites.back();
        if(last.landingpad_offset == call_site.landingpad_offset &&
           last.landingpad_symbol == call_site.landingpad_symbol &&
           last.start + last.length == call_site.start) {
          last.length += call_site.length;
          return;
        }
      }
      host_eh_function.call_sites.push_back(call_site);
    };
    const auto flush_host_eh_region = [&](size_t boundary_offset) {
      if(!host_eh_region_open) {
        return;
      }
      hosteh::HostEhCallSite finished;
      finished.start = host_eh_region_start - function_start;
      finished.length = boundary_offset - host_eh_region_start;
      finished.landingpad_offset = host_eh_region_landingpad_offset;
      finished.landingpad_symbol = host_eh_region_landingpad_symbol;
      append_host_eh_call_site(finished);
      host_eh_region_open = false;
    };
    const auto open_host_eh_region =
        [&](const vector<string> & active_host_eh_regions,
            bool is_landingpad_block,
            size_t start_offset) {
      hosteh::HostEhCallSite desired =
          resolve_host_eh_region(active_host_eh_regions, is_landingpad_block, start_offset);
      host_eh_region_open = true;
      host_eh_region_start = start_offset;
      host_eh_region_landingpad_offset = desired.landingpad_offset;
      host_eh_region_landingpad_symbol = desired.landingpad_symbol;
    };
    const auto sync_host_eh_region_state =
        [&](const vector<string> & active_host_eh_regions,
            bool is_landingpad_block,
            size_t boundary_offset) {
      if(!track_host_eh_call_sites || !host_eh_region_open) {
        return;
      }
      hosteh::HostEhCallSite desired =
          resolve_host_eh_region(active_host_eh_regions, is_landingpad_block, boundary_offset);
      if(host_eh_region_landingpad_offset == desired.landingpad_offset &&
         host_eh_region_landingpad_symbol == desired.landingpad_symbol) {
        return;
      }
      flush_host_eh_region(boundary_offset);
    };
    for(size_t bi = 0; bi < function.blocks.size(); ++bi) {
      const size_t expected = want_offset + func_layout.block_offsets.find(function.blocks[bi].label)->second;
      if(out.offset() != expected) {
        ostringstream outmsg;
        outmsg << "object block layout drift in function " << function.name
               << " block " << function.blocks[bi].label
               << " expected " << expected
               << " actual " << out.offset();
        throw logic_error(outmsg.str());
      }
      if(function.host_eh_enabled &&
         landingpad_labels.count(block_symbol(function.name, function.blocks[bi].label)) != 0) {
        emit_store_reg_to_frame(out,
                                "ptr",
                                function.host_eh_exception_offset,
                                XR_RAX);
        emit_store_reg_to_frame(out,
                                "i32",
                                function.host_eh_selector_offset,
                                XR_RDX);
      }
      vector<string> active_host_eh_regions;
      bool track_host_eh_regions = false;
      if(track_host_eh_call_sites) {
        map<string, vector<string> >::const_iterator found =
            host_eh_entry_stacks.find(function.blocks[bi].label);
        if(found != host_eh_entry_stacks.end()) {
          active_host_eh_regions = found->second;
          track_host_eh_regions = true;
        }
      }
      const bool is_landingpad_block =
          landingpad_labels.count(block_symbol(function.name, function.blocks[bi].label)) != 0;
      bool has_implicit_landingpad_region = is_landingpad_block;
      if(track_host_eh_call_sites) {
        sync_host_eh_region_state(active_host_eh_regions,
                                  is_landingpad_block,
                                  out.offset());
      }
      for(size_t ii = 0; ii < function.blocks[bi].instructions.size(); ++ii) {
        const mir::Instruction & inst = function.blocks[bi].instructions[ii];
        if(track_host_eh_call_sites && track_host_eh_regions &&
           inst.opcode == mir::Instruction::MI_EH_PUSH) {
          active_host_eh_regions.push_back(inst.operands[0].text);
          sync_host_eh_region_state(active_host_eh_regions,
                                    is_landingpad_block,
                                    out.offset());
          continue;
        }
        if(track_host_eh_call_sites && track_host_eh_regions &&
           inst.opcode == mir::Instruction::MI_EH_POP) {
          if(!active_host_eh_regions.empty()) {
            active_host_eh_regions.pop_back();
            sync_host_eh_region_state(active_host_eh_regions,
                                      is_landingpad_block,
                                      out.offset());
            continue;
          }
          if(has_implicit_landingpad_region) {
            has_implicit_landingpad_region = false;
            sync_host_eh_region_state(active_host_eh_regions,
                                      is_landingpad_block,
                                      out.offset());
            continue;
          }
          if(active_host_eh_regions.empty()) {
            ostringstream outmsg;
            outmsg << "host EH region pop underflow in " << function.name
                   << " block " << function.blocks[bi].label
                   << " instruction " << ii
                   << " during code emission";
            throw logic_error(outmsg.str());
          }
        }
        if(inst.opcode == mir::Instruction::MI_JMP ||
           is_conditional_jump(inst)) {
          const string block_name = inst.operands[0].text;
          map<string, size_t>::const_iterator block =
              func_layout.block_offsets.find(block_name);
          if(block == func_layout.block_offsets.end()) {
            ostringstream outmsg;
            outmsg << "unknown block " << block_name
                   << " in function " << function.name
                   << " known blocks:";
            for(map<string, size_t>::const_iterator it = func_layout.block_offsets.begin();
                it != func_layout.block_offsets.end(); ++it) {
              outmsg << " " << it->first;
            }
            throw logic_error(outmsg.str());
          }
          const size_t patch_offset = inst.opcode == mir::Instruction::MI_JMP
              ? out.emit_jmp_rel32_placeholder()
              : out.emit_jcc_rel32_placeholder(jump_condition(inst));
          patch_rel32_local(out, patch_offset, 0,
                            want_offset + block->second);
          continue;
        }
        vector<Fixup> fixups;
        const size_t before = out.offset();
        const bool may_unwind_in_host_eh =
            (inst.opcode == mir::Instruction::MI_CALL &&
             !inst.call_unwind_no &&
             !(inst.operands.size() == 1 &&
               inst.operands[0].kind == mir::Operand::OP_SYMBOL &&
               (inst.operands[0].text == kHostEhAllocateExceptionSymbol ||
                inst.operands[0].text == kHostEhAllocateExceptionObjectSymbol))) ||
            ((inst.opcode == mir::Instruction::MI_CALL_INDIRECT ||
              inst.opcode == mir::Instruction::MI_TLS_ADDR) &&
             !inst.call_unwind_no);
        if(track_host_eh_call_sites && track_host_eh_regions && may_unwind_in_host_eh &&
           !host_eh_region_open) {
          open_host_eh_region(active_host_eh_regions, is_landingpad_block, before);
        }
        emit_instruction(out,
                         inst,
                         -static_cast<long long>(function.stack_size),
                         function,
                         target,
                         defined_symbols,
                         defined_globals,
                         &fixups);
        for(size_t fi2 = 0; fi2 < fixups.size(); ++fi2) {
          const Fixup & fixup = fixups[fi2];
          if(fixup.kind == Fixup::FX_RIP_REL32) {
            const string block_prefix = function.name + "$";
            if(fixup.target.compare(0, block_prefix.size(), block_prefix) == 0) {
              const string block_name = fixup.target.substr(block_prefix.size());
              map<string, size_t>::const_iterator block =
                  func_layout.block_offsets.find(block_name);
              if(block != func_layout.block_offsets.end()) {
                patch_rel32_local(out,
                                  fixup.patch_offset,
                                  0,
                                  want_offset + block->second);
                continue;
              }
            }
          }
          append_relocation(relocations,
                            mobj::Symbol::SS_CODE,
                            before + (fixup.patch_offset - before),
                            fixup.kind == Fixup::FX_CALL_REL32
                                ? mobj::Relocation::RK_BRANCH32
                                : fixup.kind == Fixup::FX_RIP_REL32
                                    ? mobj::Relocation::RK_PCREL32
                                    : fixup.kind == Fixup::FX_INDIRECT_REL32
                                        ? mobj::Relocation::RK_INDIRECT_REL32
                                    : mobj::Relocation::RK_ABS64,
                            fixup.target);
        }
        if(track_host_eh_call_sites &&
           host_eh_region_open &&
           may_unwind_in_host_eh &&
           inst.call_returns_noreturn &&
           (host_eh_region_landingpad_offset != 0 ||
            !host_eh_region_landingpad_symbol.empty())) {
          flush_host_eh_region(out.offset());
        }
      }
    }
    const size_t emitted_size = out.offset() - function_start;
    if(track_host_eh_call_sites) {
      flush_host_eh_region(out.offset());
    }
    if(emitted_size != func_layout.size) {
      ostringstream outmsg;
      outmsg << "object function layout drift in function " << function.name
             << " expected_size " << func_layout.size
             << " actual_size " << emitted_size;
      throw logic_error(outmsg.str());
    }
    if(function.host_eh_enabled) {
      host_eh_function.function_size = emitted_size;
      if(!host_eh_function.call_sites.empty()) {
        host_eh_functions->push_back(host_eh_function);
      }
    }
  }
  for(size_t gi = 0; gi < program.globals.size(); ++gi) {
    const mir::GlobalDefinition & global = program.globals[gi];
    if(!global.thread_local_storage ||
       global.thread_local_wrapper_symbol.empty()) {
      continue;
    }
    const size_t want_offset = layout.thread_local_wrapper_offsets.find(global.name)->second;
    while(out.offset() < want_offset) {
      out.append(vector<unsigned char>(want_offset - out.offset(), 0x90));
    }
    const size_t wrapper_start = out.offset();
    if(tls_info.abi == ThreadLocalSectionInfo::ABI_MACHO_TLV) {
      out.append(vector<unsigned char>(1, 0x55));
      out.emit_mov_r64_r64(XR_RBP, XR_RSP);
      const size_t tlv_patch = out.emit_mov_r64_rip_rel32_placeholder(XR_RDI);
      append_relocation(relocations,
                        mobj::Symbol::SS_CODE,
                        wrapper_start + (tlv_patch - wrapper_start),
                        mobj::Relocation::RK_TLV_REL32,
                        global.name);
      out.emit_call_m64(X86Memory(XR_RDI));
      out.append(vector<unsigned char>(1, 0x5d));
    } else if(tls_info.abi == ThreadLocalSectionInfo::ABI_ELF) {
      const string tls_object_symbol = linux_simple_thread_local_object_symbol(global.name);
      emit_elf_tls_address_bytes(out,
                                 wrapper_start,
                                 tls_object_symbol.empty() ? global.name : tls_object_symbol,
                                 relocations);
    } else if(tls_info.abi == ThreadLocalSectionInfo::ABI_DIRECT_NATIVE) {
      const size_t data_patch = out.emit_lea_r64_rip_rel32_placeholder(XR_RAX);
      append_relocation(relocations,
                        mobj::Symbol::SS_CODE,
                        wrapper_start + (data_patch - wrapper_start),
                        mobj::Relocation::RK_PCREL32,
                        global.name);
    } else {
      emit_emutls_wrapper_bytes(out,
                                0,
                                thread_local_runtime_object_symbol(exports,
                                                                   global.name,
                                                                   program.target),
                                relocations);
    }
    if(tls_info.abi != ThreadLocalSectionInfo::ABI_EMUTLS) {
      out.emit_ret();
    }
    const size_t emitted_size = out.offset() - wrapper_start;
    if(emitted_size != thread_local_wrapper_size(program.target)) {
      throw logic_error("unexpected thread_local wrapper size drift");
    }
  }
  return out.bytes();
}

vector<unsigned char> build_data_bytes(const mir::Program & program,
                                       const ObjectLayout & layout,
                                       vector<mobj::Relocation> & relocations)
{
  const map<string, symbol_linkage::SymbolIdentity> exports =
      export_map(program.exported_symbols);
  const ThreadLocalSectionInfo tls_info =
      thread_local_section_info_for_target(program.target);
  vector<unsigned char> data(layout.data_size, 0);
  for(size_t i = 0; i < program.globals.size(); ++i) {
    const mir::GlobalDefinition & global = program.globals[i];
    if(global.thread_local_storage) {
      if(tls_info.abi == ThreadLocalSectionInfo::ABI_EMUTLS) {
        const size_t cursor = layout.thread_local_descriptor_offsets.find(global.name)->second;
        const vector<unsigned char> size_bytes =
            cy86_internal::encode_uint64(global_size(global), 8);
        const vector<unsigned char> align_bytes =
            cy86_internal::encode_uint64(global_alignment(global), 8);
        copy(size_bytes.begin(), size_bytes.end(), data.begin() + cursor);
        copy(align_bytes.begin(), align_bytes.end(), data.begin() + cursor + 8);
        append_relocation(relocations,
                          mobj::Symbol::SS_DATA,
                          cursor + 24,
                          mobj::Relocation::RK_ABS64,
                          thread_local_template_symbol(exports, global.name, program.target));
      }
      if(tls_info.abi != ThreadLocalSectionInfo::ABI_DIRECT_NATIVE) {
        continue;
      }
    }
    if(global.readonly) {
      continue;
    }
    size_t offset = layout.global_offsets.find(global.name)->second;
    if(global.storage_kind == mir::GlobalDefinition::GS_SCALAR) {
      const size_t width = type_size_text(global.type);
      if(global.init_kind == mir::GlobalDefinition::GI_INTEGER) {
        vector<unsigned char> bytes =
            integer_literal_storage_bytes(global.int_value, width);
        copy(bytes.begin(), bytes.end(), data.begin() + offset);
      } else if(global.init_kind == mir::GlobalDefinition::GI_FLOAT) {
        const vector<unsigned char> bytes =
            float_literal_storage_bytes(global.type, global.float_value, global.literal_text);
        copy(bytes.begin(), bytes.end(), data.begin() + offset);
      } else if(global.init_kind == mir::GlobalDefinition::GI_ADDR) {
        vector<unsigned char> bytes =
            cy86_internal::encode_uint64(static_cast<uint64_t>(global.addr_addend), width);
        copy(bytes.begin(), bytes.end(), data.begin() + offset);
        append_relocation(relocations, mobj::Symbol::SS_DATA, offset,
                          mobj::Relocation::RK_ABS64, global.symbol, global.addr_addend);
      }
      continue;
    }

    size_t cursor = offset;
    for(size_t di = 0; di < global.data_items.size(); ++di) {
      const mir::GlobalDefinition::DataItem & item = global.data_items[di];
      if(item.kind == mir::GlobalDefinition::DataItem::ITEM_ZERO) {
        cursor += item.zero_bytes;
        continue;
      }
      const size_t width = item.kind == mir::GlobalDefinition::DataItem::ITEM_ADDR
          ? 8
          : type_size_text(item.type);
      const size_t align = item.kind == mir::GlobalDefinition::DataItem::ITEM_ADDR
          ? 8
          : type_alignment_text(item.type);
      while(align > 1 && (cursor % align) != 0) {
        ++cursor;
      }
      if(item.kind == mir::GlobalDefinition::DataItem::ITEM_INTEGER) {
        vector<unsigned char> bytes =
            integer_literal_storage_bytes(item.int_value, width);
        copy(bytes.begin(), bytes.end(), data.begin() + cursor);
      } else if(item.kind == mir::GlobalDefinition::DataItem::ITEM_FLOAT) {
        const vector<unsigned char> bytes =
            float_literal_storage_bytes(item.type, item.float_value, item.literal_text);
        copy(bytes.begin(), bytes.end(), data.begin() + cursor);
      } else {
        vector<unsigned char> bytes =
            cy86_internal::encode_uint64(static_cast<uint64_t>(item.addr_addend), width);
        copy(bytes.begin(), bytes.end(), data.begin() + cursor);
        append_relocation(relocations, mobj::Symbol::SS_DATA, cursor,
                          mobj::Relocation::RK_ABS64, item.symbol, item.addr_addend);
      }
      cursor += width;
    }
  }
  return data;
}

mobj::ExtraSection build_thread_local_template_section(const mir::Program & program,
                                                       const ObjectLayout & layout,
                                                       bool zero_fill)
{
  const ThreadLocalSectionInfo info = thread_local_section_info_for_target(program.target);
  mobj::ExtraSection section;
  section.segment_name = zero_fill ? info.bss_segment_name : info.data_segment_name;
  section.section_name = zero_fill ? info.bss_section_name : info.data_section_name;
  section.macho_flags =
      info.abi == ThreadLocalSectionInfo::ABI_MACHO_TLV ? (zero_fill ? 0x12 : 0x11) : 0;
  section.bytes.resize(zero_fill ? layout.thread_local_template_bss_size
                                 : layout.thread_local_template_data_size,
                       0);

  size_t max_alignment = 1;
  for(size_t i = 0; i < program.globals.size(); ++i) {
    const mir::GlobalDefinition & global = program.globals[i];
    if(!global.thread_local_storage ||
       thread_local_global_uses_macho_zerofill_template(global, info) != zero_fill) {
      continue;
    }
    max_alignment = max(max_alignment, global_alignment(global));
    size_t cursor = layout.thread_local_template_offsets.find(global.name)->second;
    if(global.storage_kind == mir::GlobalDefinition::GS_SCALAR) {
      const size_t width = type_size_text(global.type);
      if(global.init_kind == mir::GlobalDefinition::GI_INTEGER) {
        vector<unsigned char> bytes =
            integer_literal_storage_bytes(global.int_value, width);
        copy(bytes.begin(), bytes.end(), section.bytes.begin() + cursor);
      } else if(global.init_kind == mir::GlobalDefinition::GI_FLOAT) {
        const vector<unsigned char> bytes =
            float_literal_storage_bytes(global.type, global.float_value, global.literal_text);
        copy(bytes.begin(), bytes.end(), section.bytes.begin() + cursor);
      } else if(global.init_kind == mir::GlobalDefinition::GI_ADDR) {
        vector<unsigned char> bytes =
            cy86_internal::encode_uint64(static_cast<uint64_t>(global.addr_addend), width);
        copy(bytes.begin(), bytes.end(), section.bytes.begin() + cursor);
        append_extra_abs64_symbol_relocation(section, cursor, global.symbol, global.addr_addend);
      }
      continue;
    }

    for(size_t di = 0; di < global.data_items.size(); ++di) {
      const mir::GlobalDefinition::DataItem & item = global.data_items[di];
      if(item.kind == mir::GlobalDefinition::DataItem::ITEM_ZERO) {
        cursor += item.zero_bytes;
        continue;
      }
      const size_t width = item.kind == mir::GlobalDefinition::DataItem::ITEM_ADDR
          ? 8
          : type_size_text(item.type);
      const size_t align = item.kind == mir::GlobalDefinition::DataItem::ITEM_ADDR
          ? 8
          : type_alignment_text(item.type);
      while(align > 1 && (cursor % align) != 0) {
        ++cursor;
      }
      if(item.kind == mir::GlobalDefinition::DataItem::ITEM_INTEGER) {
        vector<unsigned char> bytes =
            integer_literal_storage_bytes(item.int_value, width);
        copy(bytes.begin(), bytes.end(), section.bytes.begin() + cursor);
      } else if(item.kind == mir::GlobalDefinition::DataItem::ITEM_FLOAT) {
        const vector<unsigned char> bytes =
            float_literal_storage_bytes(item.type, item.float_value, item.literal_text);
        copy(bytes.begin(), bytes.end(), section.bytes.begin() + cursor);
      } else {
        vector<unsigned char> bytes =
            cy86_internal::encode_uint64(static_cast<uint64_t>(item.addr_addend), width);
        copy(bytes.begin(), bytes.end(), section.bytes.begin() + cursor);
        append_extra_abs64_symbol_relocation(section, cursor, item.symbol, item.addr_addend);
      }
      cursor += width;
    }
  }

  while((size_t(1) << section.macho_align_pow2) < max_alignment) {
    ++section.macho_align_pow2;
  }
  return section;
}

vector<mobj::ExtraSection> build_thread_local_template_sections(
    const mir::Program & program,
    const ObjectLayout & layout)
{
  const ThreadLocalSectionInfo info = thread_local_section_info_for_target(program.target);
  vector<mobj::ExtraSection> sections;
  if(layout.thread_local_template_data_size != 0) {
    sections.push_back(build_thread_local_template_section(program, layout, false));
  }
  if(info.abi == ThreadLocalSectionInfo::ABI_MACHO_TLV &&
     layout.thread_local_template_bss_size != 0) {
    sections.push_back(build_thread_local_template_section(program, layout, true));
  }
  return sections;
}

mobj::ExtraSection build_thread_local_vars_section(
    const mir::Program & program,
    const ObjectLayout & layout,
    const map<string, symbol_linkage::SymbolIdentity> & exports)
{
  const ThreadLocalSectionInfo info = thread_local_section_info_for_target(program.target);
  if(info.abi != ThreadLocalSectionInfo::ABI_MACHO_TLV) {
    throw logic_error("thread_local vars section requested on target without TLS descriptors");
  }
  mobj::ExtraSection section;
  section.segment_name = info.vars_segment_name;
  section.section_name = info.vars_section_name;
  section.macho_flags = 0x13;
  section.macho_align_pow2 = 3;
  section.bytes.resize(layout.thread_local_descriptor_size, 0);
  for(size_t i = 0; i < program.globals.size(); ++i) {
    const mir::GlobalDefinition & global = program.globals[i];
    if(!global.thread_local_storage) {
      continue;
    }
    const size_t cursor = layout.thread_local_descriptor_offsets.find(global.name)->second;
    append_extra_abs64_symbol_relocation(section, cursor, "__tlv_bootstrap");
    append_extra_abs64_symbol_relocation(section,
                                         cursor + 16,
                                         thread_local_template_symbol(exports,
                                                                      global.name,
                                                                      program.target));
  }
  return section;
}

mobj::ExtraSection build_readonly_data_section(const mir::Program & program,
                                               const ObjectLayout & layout)
{
  const ReadonlySectionInfo info = readonly_section_info_for_target(program.target);
  mobj::ExtraSection section;
  section.segment_name = info.segment_name;
  section.section_name = info.section_name;
  section.macho_align_pow2 = 4;
  section.bytes.resize(layout.readonly_data_size, 0);

  size_t max_alignment = 1;
  for(size_t i = 0; i < program.globals.size(); ++i) {
    const mir::GlobalDefinition & global = program.globals[i];
    if(!global.readonly || global.thread_local_storage) {
      continue;
    }
    max_alignment = max(max_alignment, global_alignment(global));
    size_t cursor = layout.readonly_global_offsets.find(global.name)->second;
    if(global.storage_kind == mir::GlobalDefinition::GS_SCALAR) {
      const size_t width = type_size_text(global.type);
      if(global.init_kind == mir::GlobalDefinition::GI_INTEGER) {
        vector<unsigned char> bytes =
            integer_literal_storage_bytes(global.int_value, width);
        copy(bytes.begin(), bytes.end(), section.bytes.begin() + cursor);
      } else if(global.init_kind == mir::GlobalDefinition::GI_FLOAT) {
        const vector<unsigned char> bytes =
            float_literal_storage_bytes(global.type, global.float_value, global.literal_text);
        copy(bytes.begin(), bytes.end(), section.bytes.begin() + cursor);
      } else if(global.init_kind == mir::GlobalDefinition::GI_ADDR) {
        vector<unsigned char> bytes =
            cy86_internal::encode_uint64(static_cast<uint64_t>(global.addr_addend), width);
        copy(bytes.begin(), bytes.end(), section.bytes.begin() + cursor);
        append_extra_abs64_symbol_relocation(section, cursor, global.symbol, global.addr_addend);
      }
      continue;
    }

    for(size_t di = 0; di < global.data_items.size(); ++di) {
      const mir::GlobalDefinition::DataItem & item = global.data_items[di];
      if(item.kind == mir::GlobalDefinition::DataItem::ITEM_ZERO) {
        cursor += item.zero_bytes;
        continue;
      }
      const size_t width = item.kind == mir::GlobalDefinition::DataItem::ITEM_ADDR
          ? 8
          : type_size_text(item.type);
      const size_t align = item.kind == mir::GlobalDefinition::DataItem::ITEM_ADDR
          ? 8
          : type_alignment_text(item.type);
      while(align > 1 && (cursor % align) != 0) {
        ++cursor;
      }
      if(item.kind == mir::GlobalDefinition::DataItem::ITEM_INTEGER) {
        vector<unsigned char> bytes =
            integer_literal_storage_bytes(item.int_value, width);
        copy(bytes.begin(), bytes.end(), section.bytes.begin() + cursor);
      } else if(item.kind == mir::GlobalDefinition::DataItem::ITEM_FLOAT) {
        const vector<unsigned char> bytes =
            float_literal_storage_bytes(item.type, item.float_value, item.literal_text);
        copy(bytes.begin(), bytes.end(), section.bytes.begin() + cursor);
      } else {
        vector<unsigned char> bytes =
            cy86_internal::encode_uint64(static_cast<uint64_t>(item.addr_addend), width);
        copy(bytes.begin(), bytes.end(), section.bytes.begin() + cursor);
        append_extra_abs64_symbol_relocation(section, cursor, item.symbol, item.addr_addend);
      }
      cursor += width;
    }
  }

  size_t align_pow2 = 0;
  while((static_cast<size_t>(1) << align_pow2) < max_alignment) {
    ++align_pow2;
  }
  section.macho_align_pow2 = static_cast<uint32_t>(align_pow2);
  return section;
}

DwarfCompilationUnitInfo collect_dwarf_compilation_unit_info(const mir::Program & program,
                                                             const ObjectLayout & layout)
{
  DwarfCompilationUnitInfo out;
  for(size_t fi = 0; fi < program.functions.size(); ++fi) {
    const mir::Function & function = program.functions[fi];
    map<string, FunctionLayout>::const_iterator function_layout =
        layout.function_layouts.find(function.name);
    if(function_layout == layout.function_layouts.end()) {
      continue;
    }

    DwarfFunctionInfo debug_function;
    debug_function.display_name = debug_function_display_name(function.name);
    debug_function.return_type = function.return_type;
    debug_function.size = function_layout->second.size;
    if(function.debug_location.present()) {
      debug_function.file_index =
          dwarf_file_index(out.files, function.debug_location.file);
      debug_function.decl_line = function.debug_location.line;
      if(out.unit_name.empty()) {
        set_unit_source_path(out, function.debug_location.file);
      }
    }

    struct SourcePositionCodeRange
    {
      size_t low_pc_offset = 0;
      size_t high_pc_offset = 0;
      bool present = false;
    };
    map<size_t, SourcePositionCodeRange> source_position_ranges;
    for(size_t bi = 0; bi < function.blocks.size(); ++bi) {
      map<string, vector<size_t> >::const_iterator instruction_offsets =
          function_layout->second.instruction_offsets.find(function.blocks[bi].label);
      if(instruction_offsets == function_layout->second.instruction_offsets.end()) {
        continue;
      }
      size_t next_block_offset = function_layout->second.size;
      for(size_t next_bi = bi + 1; next_bi < function.blocks.size(); ++next_bi) {
        map<string, vector<size_t> >::const_iterator next_offsets =
            function_layout->second.instruction_offsets.find(function.blocks[next_bi].label);
        if(next_offsets != function_layout->second.instruction_offsets.end() &&
           !next_offsets->second.empty()) {
          next_block_offset = next_offsets->second.front();
          break;
        }
      }
      for(size_t ii = 0; ii < function.blocks[bi].instructions.size(); ++ii) {
        if(ii >= instruction_offsets->second.size()) {
          break;
        }
        const mir::Instruction & inst = function.blocks[bi].instructions[ii];
        if(!inst.has_source_position) {
          continue;
        }
        const size_t start_offset = instruction_offsets->second[ii];
        const size_t end_offset =
            ii + 1 < instruction_offsets->second.size()
                ? instruction_offsets->second[ii + 1]
                : next_block_offset;
        if(end_offset <= start_offset) {
          continue;
        }
        SourcePositionCodeRange & range =
            source_position_ranges[inst.source_position];
        if(!range.present) {
          range.low_pc_offset = start_offset;
          range.high_pc_offset = end_offset;
          range.present = true;
          continue;
        }
        range.low_pc_offset = min(range.low_pc_offset, start_offset);
        range.high_pc_offset = max(range.high_pc_offset, end_offset);
      }
    }

    auto append_debug_variable_locations =
        [&](DwarfFunctionInfo::Variable & out_variable,
            const mir::DebugVariable & variable)
        {
          for(size_t ri = 0; ri < variable.ranges.size(); ++ri) {
            DwarfFunctionInfo::VariableLocation location;
            if(variable.ranges[ri].location ==
               mir::DebugVariable::Range::LK_FRAME) {
              location.kind = DwarfFunctionInfo::VariableLocation::LK_FRAME;
              location.frame_offset = variable.ranges[ri].frame_offset;
            } else if(variable.ranges[ri].location ==
                      mir::DebugVariable::Range::LK_REG) {
              location.kind = DwarfFunctionInfo::VariableLocation::LK_REG;
              location.dwarf_register =
                  dwarf_register_number(variable.ranges[ri].reg);
            } else {
              location.kind = DwarfFunctionInfo::VariableLocation::LK_XMM;
              location.dwarf_register =
                  dwarf_register_number(variable.ranges[ri].xmm);
            }

            bool have_code_range = false;
            for(size_t source_position = variable.ranges[ri].start_source_position;
                source_position < variable.ranges[ri].end_source_position;
                ++source_position) {
              map<size_t, SourcePositionCodeRange>::const_iterator found =
                  source_position_ranges.find(source_position);
              if(found == source_position_ranges.end() || !found->second.present) {
                continue;
              }
              if(!have_code_range) {
                location.low_pc_offset = found->second.low_pc_offset;
                location.high_pc_offset = found->second.high_pc_offset;
                have_code_range = true;
                continue;
              }
              location.low_pc_offset =
                  min(location.low_pc_offset, found->second.low_pc_offset);
              location.high_pc_offset =
                  max(location.high_pc_offset, found->second.high_pc_offset);
            }
            if(have_code_range &&
               location.high_pc_offset > location.low_pc_offset) {
              out_variable.locations.push_back(location);
            }
          }
        };

    set<string> seen_param_names;
    for(size_t pi = 0; pi < function.params.size(); ++pi) {
      const string & param_name = function.params[pi].name;
      const string debug_name = normalized_debug_name(param_name);
      if(debug_name.empty() ||
         !seen_param_names.insert(debug_name).second) {
        continue;
      }
      const mir::FrameBinding * binding =
          find_frame_binding(function, mir::FrameBinding::FB_PARAM_SLOT, param_name);
      if(binding != nullptr && has_builtin_debug_type(binding->type)) {
        DwarfFunctionInfo::Variable param;
        param.name = debug_name;
        param.type = binding->type;
        param.frame_offset = binding->offset;
        debug_function.parameters.push_back(param);
        continue;
      }
      for(size_t vi = 0; vi < function.debug_variables.size(); ++vi) {
        const mir::DebugVariable & variable = function.debug_variables[vi];
        if(variable.name != debug_name ||
           !has_builtin_debug_type(variable.type)) {
          continue;
        }
        DwarfFunctionInfo::Variable param;
        param.name = debug_name;
        param.type = variable.type;
        append_debug_variable_locations(param, variable);
        if(!param.locations.empty()) {
          debug_function.parameters.push_back(param);
        }
        break;
      }
    }

    set<string> seen_local_names;
    for(size_t bi = 0; bi < function.frame_bindings.size(); ++bi) {
      const mir::FrameBinding & binding = function.frame_bindings[bi];
      string generated_debug_name;
      if(lowir_internal::lowir_debug_value_source_name(binding.name,
                                                       generated_debug_name)) {
        continue;
      }
      const string debug_name = normalized_debug_name(binding.name);
      if(binding.kind != mir::FrameBinding::FB_SLOT ||
         debug_name.empty() ||
         seen_param_names.count(debug_name) != 0 ||
         !seen_local_names.insert(debug_name).second ||
         !has_builtin_debug_type(binding.type)) {
        continue;
      }
      DwarfFunctionInfo::Variable local;
      local.name = debug_name;
      local.type = binding.type;
      local.frame_offset = binding.offset;
      debug_function.locals.push_back(local);
    }

    for(size_t vi = 0; vi < function.debug_variables.size(); ++vi) {
      const mir::DebugVariable & variable = function.debug_variables[vi];
      if(!has_builtin_debug_type(variable.type) ||
         variable.name.empty() ||
         seen_param_names.count(variable.name) != 0 ||
         !seen_local_names.insert(variable.name).second) {
        continue;
      }
      DwarfFunctionInfo::Variable local;
      local.name = variable.name;
      local.type = variable.type;
      append_debug_variable_locations(local, variable);
      if(!local.locations.empty()) {
        debug_function.locals.push_back(local);
      }
    }

    for(size_t bi = 0; bi < function.blocks.size(); ++bi) {
      map<string, vector<size_t> >::const_iterator instruction_offsets =
          function_layout->second.instruction_offsets.find(function.blocks[bi].label);
      if(instruction_offsets == function_layout->second.instruction_offsets.end()) {
        continue;
      }
      for(size_t ii = 0; ii < function.blocks[bi].instructions.size(); ++ii) {
        if(ii >= instruction_offsets->second.size()) {
          break;
        }
        const mir::Instruction & inst = function.blocks[bi].instructions[ii];
        if(!inst.debug_location.present()) {
          continue;
        }
        const size_t file_index = dwarf_file_index(out.files, inst.debug_location.file);
        if(debug_function.file_index == 0) {
          debug_function.file_index = file_index;
        }
        if(debug_function.decl_line == 0) {
          debug_function.decl_line = inst.debug_location.line;
        }
        DwarfLineRow row;
        row.file_index = file_index;
        row.line = inst.debug_location.line;
        row.column = inst.debug_location.column;
        row.address_addend = instruction_offsets->second[ii];
        if(!debug_function.rows.empty()) {
          const DwarfLineRow & prev = debug_function.rows.back();
          if(prev.file_index == row.file_index &&
             prev.line == row.line &&
             prev.column == row.column &&
             prev.address_addend == row.address_addend) {
            continue;
          }
        }
        debug_function.rows.push_back(row);
      }
    }

    if(debug_function.rows.empty()) {
      continue;
    }
    map<string, size_t>::const_iterator function_offset =
        layout.function_offsets.find(function.name);
    if(function_offset != layout.function_offsets.end()) {
      const size_t low_offset = function_offset->second;
      const size_t high_end = low_offset + function_layout->second.size;
      debug_function.low_pc_offset = low_offset;
      if(out.functions.empty() || low_offset < out.low_pc_offset) {
        out.low_pc_offset = low_offset;
      }
      out.high_pc_end = max(out.high_pc_end, high_end);
    }
    if(out.unit_name.empty() && debug_function.file_index != 0) {
      set_unit_source_path(out, out.files.files[debug_function.file_index - 1].path);
    }
    out.functions.push_back(debug_function);
  }
  if(out.unit_name.empty()) {
    out.unit_name = "<unknown>";
  }
  return out;
}

mobj::ExtraSection make_debug_extra_section(const mir::Program & program,
                                            const string & section_name)
{
  const DebugSectionNames names = debug_section_names_for_target(program.target);
  mobj::ExtraSection section;
  section.segment_name = names.segment_name;
  section.section_name = section_name;
  section.macho_align_pow2 = 0;
  section.macho_flags = program.target == "macos" ? MACHO_DEBUG_SECTION_FLAGS : 0;
  return section;
}

void append_dwarf_set_code_address(mobj::ExtraSection & section,
                                   size_t addend)
{
  append_u8(section.bytes, 0);
  append_uleb128(section.bytes, 9);
  append_u8(section.bytes, DW_LNE_SET_ADDRESS);
  const size_t reloc_offset = section.bytes.size();
  append_u64(section.bytes, 0);

  mobj::ExtraRelocation reloc;
  reloc.kind = mobj::ExtraRelocation::RK_ABS64;
  reloc.target_kind = mobj::ExtraRelocation::TK_CODE;
  reloc.offset = reloc_offset;
  reloc.addend = static_cast<long long>(addend);
  section.relocations.push_back(reloc);
}

mobj::ExtraSection build_dwarf_abbrev_section(const mir::Program & program)
{
  const DebugSectionNames names = debug_section_names_for_target(program.target);
  mobj::ExtraSection section = make_debug_extra_section(program, names.abbrev_name);

  append_uleb128(section.bytes, 1);
  append_uleb128(section.bytes, DW_TAG_COMPILE_UNIT);
  append_u8(section.bytes, DW_CHILDREN_YES);
  append_uleb128(section.bytes, DW_AT_PRODUCER);
  append_uleb128(section.bytes, DW_FORM_STRING);
  append_uleb128(section.bytes, DW_AT_LANGUAGE);
  append_uleb128(section.bytes, DW_FORM_DATA2);
  append_uleb128(section.bytes, DW_AT_NAME);
  append_uleb128(section.bytes, DW_FORM_STRING);
  append_uleb128(section.bytes, DW_AT_COMP_DIR);
  append_uleb128(section.bytes, DW_FORM_STRING);
  append_uleb128(section.bytes, DW_AT_STMT_LIST);
  append_uleb128(section.bytes, DW_FORM_SEC_OFFSET);
  append_uleb128(section.bytes, DW_AT_LOW_PC);
  append_uleb128(section.bytes, DW_FORM_ADDR);
  append_uleb128(section.bytes, DW_AT_HIGH_PC);
  append_uleb128(section.bytes, DW_FORM_DATA8);
  append_uleb128(section.bytes, 0);
  append_uleb128(section.bytes, 0);

  append_uleb128(section.bytes, 7);
  append_uleb128(section.bytes, DW_TAG_COMPILE_UNIT);
  append_u8(section.bytes, DW_CHILDREN_YES);
  append_uleb128(section.bytes, DW_AT_PRODUCER);
  append_uleb128(section.bytes, DW_FORM_STRING);
  append_uleb128(section.bytes, DW_AT_LANGUAGE);
  append_uleb128(section.bytes, DW_FORM_DATA2);
  append_uleb128(section.bytes, DW_AT_NAME);
  append_uleb128(section.bytes, DW_FORM_STRING);
  append_uleb128(section.bytes, DW_AT_STMT_LIST);
  append_uleb128(section.bytes, DW_FORM_SEC_OFFSET);
  append_uleb128(section.bytes, DW_AT_LOW_PC);
  append_uleb128(section.bytes, DW_FORM_ADDR);
  append_uleb128(section.bytes, DW_AT_HIGH_PC);
  append_uleb128(section.bytes, DW_FORM_DATA8);
  append_uleb128(section.bytes, 0);
  append_uleb128(section.bytes, 0);

  append_uleb128(section.bytes, 2);
  append_uleb128(section.bytes, DW_TAG_BASE_TYPE);
  append_u8(section.bytes, DW_CHILDREN_NO);
  append_uleb128(section.bytes, DW_AT_NAME);
  append_uleb128(section.bytes, DW_FORM_STRING);
  append_uleb128(section.bytes, DW_AT_ENCODING);
  append_uleb128(section.bytes, DW_FORM_DATA1);
  append_uleb128(section.bytes, DW_AT_BYTE_SIZE);
  append_uleb128(section.bytes, DW_FORM_DATA1);
  append_uleb128(section.bytes, 0);
  append_uleb128(section.bytes, 0);

  append_uleb128(section.bytes, 3);
  append_uleb128(section.bytes, DW_TAG_SUBPROGRAM);
  append_u8(section.bytes, DW_CHILDREN_YES);
  append_uleb128(section.bytes, DW_AT_NAME);
  append_uleb128(section.bytes, DW_FORM_STRING);
  append_uleb128(section.bytes, DW_AT_DECL_FILE);
  append_uleb128(section.bytes, DW_FORM_DATA4);
  append_uleb128(section.bytes, DW_AT_DECL_LINE);
  append_uleb128(section.bytes, DW_FORM_DATA4);
  append_uleb128(section.bytes, DW_AT_LOW_PC);
  append_uleb128(section.bytes, DW_FORM_ADDR);
  append_uleb128(section.bytes, DW_AT_HIGH_PC);
  append_uleb128(section.bytes, DW_FORM_DATA8);
  append_uleb128(section.bytes, 0);
  append_uleb128(section.bytes, 0);

  append_uleb128(section.bytes, 4);
  append_uleb128(section.bytes, DW_TAG_SUBPROGRAM);
  append_u8(section.bytes, DW_CHILDREN_YES);
  append_uleb128(section.bytes, DW_AT_NAME);
  append_uleb128(section.bytes, DW_FORM_STRING);
  append_uleb128(section.bytes, DW_AT_DECL_FILE);
  append_uleb128(section.bytes, DW_FORM_DATA4);
  append_uleb128(section.bytes, DW_AT_DECL_LINE);
  append_uleb128(section.bytes, DW_FORM_DATA4);
  append_uleb128(section.bytes, DW_AT_LOW_PC);
  append_uleb128(section.bytes, DW_FORM_ADDR);
  append_uleb128(section.bytes, DW_AT_HIGH_PC);
  append_uleb128(section.bytes, DW_FORM_DATA8);
  append_uleb128(section.bytes, DW_AT_TYPE);
  append_uleb128(section.bytes, DW_FORM_REF4);
  append_uleb128(section.bytes, 0);
  append_uleb128(section.bytes, 0);

  append_uleb128(section.bytes, 5);
  append_uleb128(section.bytes, DW_TAG_FORMAL_PARAMETER);
  append_u8(section.bytes, DW_CHILDREN_NO);
  append_uleb128(section.bytes, DW_AT_NAME);
  append_uleb128(section.bytes, DW_FORM_STRING);
  append_uleb128(section.bytes, DW_AT_TYPE);
  append_uleb128(section.bytes, DW_FORM_REF4);
  append_uleb128(section.bytes, DW_AT_LOCATION);
  append_uleb128(section.bytes, DW_FORM_EXPRLOC);
  append_uleb128(section.bytes, 0);
  append_uleb128(section.bytes, 0);

  append_uleb128(section.bytes, 6);
  append_uleb128(section.bytes, DW_TAG_VARIABLE);
  append_u8(section.bytes, DW_CHILDREN_NO);
  append_uleb128(section.bytes, DW_AT_NAME);
  append_uleb128(section.bytes, DW_FORM_STRING);
  append_uleb128(section.bytes, DW_AT_TYPE);
  append_uleb128(section.bytes, DW_FORM_REF4);
  append_uleb128(section.bytes, DW_AT_LOCATION);
  append_uleb128(section.bytes, DW_FORM_EXPRLOC);
  append_uleb128(section.bytes, 0);
  append_uleb128(section.bytes, 0);

  append_uleb128(section.bytes, 8);
  append_uleb128(section.bytes, DW_TAG_FORMAL_PARAMETER);
  append_u8(section.bytes, DW_CHILDREN_NO);
  append_uleb128(section.bytes, DW_AT_NAME);
  append_uleb128(section.bytes, DW_FORM_STRING);
  append_uleb128(section.bytes, DW_AT_TYPE);
  append_uleb128(section.bytes, DW_FORM_REF4);
  append_uleb128(section.bytes, DW_AT_LOCATION);
  append_uleb128(section.bytes, DW_FORM_SEC_OFFSET);
  append_uleb128(section.bytes, 0);
  append_uleb128(section.bytes, 0);

  append_uleb128(section.bytes, 9);
  append_uleb128(section.bytes, DW_TAG_VARIABLE);
  append_u8(section.bytes, DW_CHILDREN_NO);
  append_uleb128(section.bytes, DW_AT_NAME);
  append_uleb128(section.bytes, DW_FORM_STRING);
  append_uleb128(section.bytes, DW_AT_TYPE);
  append_uleb128(section.bytes, DW_FORM_REF4);
  append_uleb128(section.bytes, DW_AT_LOCATION);
  append_uleb128(section.bytes, DW_FORM_SEC_OFFSET);
  append_uleb128(section.bytes, 0);
  append_uleb128(section.bytes, 0);

  append_u8(section.bytes, 0);
  return section;
}

void append_dwarf_frame_location(vector<unsigned char> & out,
                                 long long frame_offset)
{
  vector<unsigned char> expr;
  append_u8(expr, DW_OP_BREG6);
  append_sleb128(expr, frame_offset);
  append_uleb128(out, expr.size());
  out.insert(out.end(), expr.begin(), expr.end());
}

void append_dwarf_register_location_expr(vector<unsigned char> & out,
                                         uint16_t dwarf_register)
{
  if(dwarf_register >= 32) {
    throw logic_error("unsupported DWARF register expression");
  }
  out.push_back(static_cast<unsigned char>(DW_OP_REG0 + dwarf_register));
}

mobj::ExtraSection build_dwarf_location_section(const mir::Program & program,
                                                DwarfCompilationUnitInfo & info)
{
  const DebugSectionNames names = debug_section_names_for_target(program.target);
  mobj::ExtraSection section = make_debug_extra_section(program, names.loc_name);

  const auto append_location_entry =
      [&section](const DwarfFunctionInfo::VariableLocation & location)
      {
        const size_t low_offset = section.bytes.size();
        append_u64(section.bytes, 0);
        const size_t high_offset = section.bytes.size();
        append_u64(section.bytes, 0);

        mobj::ExtraRelocation low_reloc;
        low_reloc.kind = mobj::ExtraRelocation::RK_ABS64;
        low_reloc.target_kind = mobj::ExtraRelocation::TK_CODE;
        low_reloc.offset = low_offset;
        low_reloc.addend = static_cast<long long>(location.low_pc_offset);
        section.relocations.push_back(low_reloc);

        mobj::ExtraRelocation high_reloc;
        high_reloc.kind = mobj::ExtraRelocation::RK_ABS64;
        high_reloc.target_kind = mobj::ExtraRelocation::TK_CODE;
        high_reloc.offset = high_offset;
        high_reloc.addend = static_cast<long long>(location.high_pc_offset);
        section.relocations.push_back(high_reloc);

        vector<unsigned char> expr;
        if(location.kind == DwarfFunctionInfo::VariableLocation::LK_FRAME) {
          append_u8(expr, DW_OP_BREG6);
          append_sleb128(expr, location.frame_offset);
        } else {
          append_dwarf_register_location_expr(expr, location.dwarf_register);
        }
        append_u16(section.bytes, static_cast<uint16_t>(expr.size()));
        section.bytes.insert(section.bytes.end(), expr.begin(), expr.end());
      };

  for(size_t fi = 0; fi < info.functions.size(); ++fi) {
    for(size_t pi = 0; pi < info.functions[fi].parameters.size(); ++pi) {
      DwarfFunctionInfo::Variable & variable = info.functions[fi].parameters[pi];
      if(!variable.uses_location_list()) {
        continue;
      }
      variable.location_list_offset = section.bytes.size();
      for(size_t li = 0; li < variable.locations.size(); ++li) {
        append_location_entry(variable.locations[li]);
      }
      append_u64(section.bytes, 0);
      append_u64(section.bytes, 0);
    }
    for(size_t vi = 0; vi < info.functions[fi].locals.size(); ++vi) {
      DwarfFunctionInfo::Variable & variable = info.functions[fi].locals[vi];
      if(!variable.uses_location_list()) {
        continue;
      }
      variable.location_list_offset = section.bytes.size();
      for(size_t li = 0; li < variable.locations.size(); ++li) {
        append_location_entry(variable.locations[li]);
      }
      append_u64(section.bytes, 0);
      append_u64(section.bytes, 0);
    }
  }

  return section;
}

mobj::ExtraSection build_dwarf_info_section(const mir::Program & program,
                                            const DwarfCompilationUnitInfo & info)
{
  const DebugSectionNames names = debug_section_names_for_target(program.target);
  mobj::ExtraSection section = make_debug_extra_section(program, names.info_name);

  const size_t unit_length_offset = section.bytes.size();
  append_u32(section.bytes, 0);
  append_u16(section.bytes, DWARF_VERSION_4);
  append_u32(section.bytes, 0);
  append_u8(section.bytes, 8);

  append_uleb128(section.bytes, info.unit_directory.empty() ? 7 : 1);
  append_cstring(section.bytes, "cppgm");
  append_u16(section.bytes, DW_LANG_C_PLUS_PLUS);
  append_cstring(section.bytes, info.unit_name);
  if(!info.unit_directory.empty()) {
    append_cstring(section.bytes, info.unit_directory);
  }
  append_u32(section.bytes, 0);
  const size_t cu_low_pc_reloc_offset = section.bytes.size();
  append_u64(section.bytes, 0);
  append_u64(section.bytes,
             static_cast<uint64_t>(info.high_pc_end - info.low_pc_offset));

  mobj::ExtraRelocation cu_reloc;
  cu_reloc.kind = mobj::ExtraRelocation::RK_ABS64;
  cu_reloc.target_kind = mobj::ExtraRelocation::TK_CODE;
  cu_reloc.offset = cu_low_pc_reloc_offset;
  cu_reloc.addend = static_cast<long long>(info.low_pc_offset);
  section.relocations.push_back(cu_reloc);

  const string builtin_types[] = {
    "ptr", "i8", "u8", "i16", "u16", "i32", "u32", "i64", "u64", "f32", "f64", "f80",
  };
  map<string, uint32_t> type_die_offsets;
  for(size_t i = 0; i < sizeof(builtin_types) / sizeof(builtin_types[0]); ++i) {
    type_die_offsets[builtin_types[i]] = static_cast<uint32_t>(section.bytes.size());
    append_uleb128(section.bytes, 2);
    append_cstring(section.bytes, builtin_debug_type_name(builtin_types[i]));
    append_u8(section.bytes, builtin_debug_type_encoding(builtin_types[i]));
    append_u8(section.bytes, builtin_debug_type_size(builtin_types[i]));
  }

  for(size_t i = 0; i < info.functions.size(); ++i) {
    map<string, uint32_t>::const_iterator return_type =
        type_die_offsets.find(info.functions[i].return_type);
    append_uleb128(section.bytes,
                   return_type == type_die_offsets.end() ? 3 : 4);
    append_cstring(section.bytes, info.functions[i].display_name);
    append_u32(section.bytes, static_cast<uint32_t>(info.functions[i].file_index));
    append_u32(section.bytes, static_cast<uint32_t>(info.functions[i].decl_line));
    const size_t reloc_offset = section.bytes.size();
    append_u64(section.bytes, 0);

    mobj::ExtraRelocation reloc;
    reloc.kind = mobj::ExtraRelocation::RK_ABS64;
    reloc.target_kind = mobj::ExtraRelocation::TK_CODE;
    reloc.offset = reloc_offset;
    reloc.addend = static_cast<long long>(info.functions[i].low_pc_offset);
    section.relocations.push_back(reloc);

    append_u64(section.bytes, static_cast<uint64_t>(info.functions[i].size));
    if(return_type != type_die_offsets.end()) {
      append_u32(section.bytes, return_type->second);
    }

    for(size_t pi = 0; pi < info.functions[i].parameters.size(); ++pi) {
      map<string, uint32_t>::const_iterator type =
          type_die_offsets.find(info.functions[i].parameters[pi].type);
      if(type == type_die_offsets.end()) {
        continue;
      }
      append_uleb128(section.bytes,
                     info.functions[i].parameters[pi].uses_location_list() ? 8 : 5);
      append_cstring(section.bytes, info.functions[i].parameters[pi].name);
      append_u32(section.bytes, type->second);
      if(info.functions[i].parameters[pi].uses_location_list()) {
        append_u32(section.bytes,
                   static_cast<uint32_t>(
                       info.functions[i].parameters[pi].location_list_offset));
      } else {
        append_dwarf_frame_location(section.bytes,
                                    info.functions[i].parameters[pi].frame_offset);
      }
    }
    for(size_t li = 0; li < info.functions[i].locals.size(); ++li) {
      map<string, uint32_t>::const_iterator type =
          type_die_offsets.find(info.functions[i].locals[li].type);
      if(type == type_die_offsets.end()) {
        continue;
      }
      append_uleb128(section.bytes,
                     info.functions[i].locals[li].uses_location_list() ? 9 : 6);
      append_cstring(section.bytes, info.functions[i].locals[li].name);
      append_u32(section.bytes, type->second);
      if(info.functions[i].locals[li].uses_location_list()) {
        append_u32(section.bytes,
                   static_cast<uint32_t>(info.functions[i].locals[li].location_list_offset));
      } else {
        append_dwarf_frame_location(section.bytes,
                                    info.functions[i].locals[li].frame_offset);
      }
    }
    append_u8(section.bytes, 0);
  }

  append_u8(section.bytes, 0);
  overwrite_u32(section.bytes,
                unit_length_offset,
                static_cast<uint32_t>(section.bytes.size() - unit_length_offset - 4));
  return section;
}

mobj::ExtraSection build_dwarf_line_section(const mir::Program & program,
                                            const DwarfCompilationUnitInfo & info)
{
  const DebugSectionNames names = debug_section_names_for_target(program.target);
  mobj::ExtraSection section = make_debug_extra_section(program, names.line_name);

  const size_t unit_length_offset = section.bytes.size();
  append_u32(section.bytes, 0);
  append_u16(section.bytes, DWARF_VERSION_4);
  const size_t header_length_offset = section.bytes.size();
  append_u32(section.bytes, 0);
  const size_t header_start = section.bytes.size();

  append_u8(section.bytes, 1);
  append_u8(section.bytes, 1);
  append_u8(section.bytes, 1);
  append_u8(section.bytes, static_cast<uint8_t>(-5));
  append_u8(section.bytes, 14);
  append_u8(section.bytes, 13);
  const uint8_t standard_opcode_lengths[] = {0, 1, 1, 1, 1, 0, 0, 0, 1, 0, 0, 1};
  for(size_t i = 0; i < sizeof(standard_opcode_lengths); ++i) {
    append_u8(section.bytes, standard_opcode_lengths[i]);
  }

  for(size_t i = 0; i < info.files.directories.size(); ++i) {
    append_cstring(section.bytes, info.files.directories[i]);
  }
  append_u8(section.bytes, 0);

  for(size_t i = 0; i < info.files.files.size(); ++i) {
    append_cstring(section.bytes, info.files.files[i].name);
    append_uleb128(section.bytes, info.files.files[i].directory_index);
    append_uleb128(section.bytes, 0);
    append_uleb128(section.bytes, 0);
  }
  append_u8(section.bytes, 0);

  overwrite_u32(section.bytes,
                header_length_offset,
                static_cast<uint32_t>(section.bytes.size() - header_start));

  for(size_t fi = 0; fi < info.functions.size(); ++fi) {
    size_t state_file = 1;
    size_t state_line = 1;
    size_t state_column = 0;
    for(size_t ri = 0; ri < info.functions[fi].rows.size(); ++ri) {
      const DwarfLineRow & row = info.functions[fi].rows[ri];
      if(row.file_index != state_file) {
        append_u8(section.bytes, DW_LNS_SET_FILE);
        append_uleb128(section.bytes, row.file_index);
        state_file = row.file_index;
      }
      if(row.column != state_column) {
        append_u8(section.bytes, DW_LNS_SET_COLUMN);
        append_uleb128(section.bytes, row.column);
        state_column = row.column;
      }
      if(row.line != state_line) {
        append_u8(section.bytes, DW_LNS_ADVANCE_LINE);
        append_sleb128(section.bytes,
                       static_cast<long long>(row.line) -
                           static_cast<long long>(state_line));
        state_line = row.line;
      }
      append_dwarf_set_code_address(section,
                                    info.functions[fi].low_pc_offset +
                                        row.address_addend);
      append_u8(section.bytes, DW_LNS_COPY);
    }
    append_dwarf_set_code_address(section,
                                  info.functions[fi].low_pc_offset +
                                      info.functions[fi].size);
    append_u8(section.bytes, 0);
    append_uleb128(section.bytes, 1);
    append_u8(section.bytes, DW_LNE_END_SEQUENCE);
  }

  overwrite_u32(section.bytes,
                unit_length_offset,
                static_cast<uint32_t>(section.bytes.size() - unit_length_offset - 4));
  return section;
}

map<string, symbol_linkage::SymbolIdentity> export_map(
    const vector<symbol_linkage::SymbolIdentity> & exported_symbols)
{
  map<string, symbol_linkage::SymbolIdentity> out;
  for(size_t i = 0; i < exported_symbols.size(); ++i) {
    if(exported_symbols[i].internal_symbol.empty()) {
      continue;
    }
    map<string, symbol_linkage::SymbolIdentity>::const_iterator found =
        out.find(exported_symbols[i].internal_symbol);
    if(found != out.end() &&
       (symbol_linkage::exported_object_symbol(found->second) !=
            symbol_linkage::exported_object_symbol(exported_symbols[i]) ||
        found->second.keep_internal_alias != exported_symbols[i].keep_internal_alias ||
        found->second.linkage != exported_symbols[i].linkage)) {
      throw logic_error("conflicting object symbol mapping for " +
                        exported_symbols[i].internal_symbol);
    }
    out[exported_symbols[i].internal_symbol] = exported_symbols[i];
  }
  return out;
}

map<string, vector<string> > object_alias_map(
    const vector<machine_ir::ObjectAlias> & object_aliases)
{
  map<string, vector<string> > out;
  for(size_t i = 0; i < object_aliases.size(); ++i) {
    out[object_aliases[i].target].push_back(object_aliases[i].object_symbol);
  }
  return out;
}

void append_object_alias_symbols(
    const map<string, vector<string> > & aliases,
    const string & target,
    const set<string> & reserved_object_symbols,
    map<string, bool> & defined_symbols,
    const machine_object::Symbol & base,
    vector<machine_object::Symbol> & symbols)
{
  map<string, vector<string> >::const_iterator found = aliases.find(target);
  if(found == aliases.end()) {
    return;
  }
  for(size_t i = 0; i < found->second.size(); ++i) {
    const string & alias_name = found->second[i];
    if(alias_name == base.name ||
       reserved_object_symbols.count(alias_name) != 0 ||
       defined_symbols.count(alias_name) != 0) {
      continue;
    }
    machine_object::Symbol alias = base;
    alias.name = alias_name;
    alias.comdat_group.clear();
    symbols.push_back(alias);
    defined_symbols[alias.name] = true;
  }
}

string translated_symbol_name(const map<string, symbol_linkage::SymbolIdentity> & exports,
                              const string & internal_name)
{
  if(internal_name.empty()) {
    throw logic_error("object symbol translation requested for empty internal name");
  }
  const string external_runtime_prefix = "@__external_runtime__";
  if(internal_name.compare(0,
                           external_runtime_prefix.size(),
                           external_runtime_prefix) == 0) {
    return internal_name.substr(external_runtime_prefix.size());
  }
  string object_name = internal_name;
  map<string, symbol_linkage::SymbolIdentity>::const_iterator found =
      exports.find(internal_name);
  const bool is_exported_definition = found != exports.end();
  const string normalized_internal_name = runtime_symbol_policy::normalize_lookup_name(internal_name);
  if(found != exports.end() && symbol_linkage::has_object_symbol(found->second)) {
    object_name = symbol_linkage::exported_object_symbol(found->second);
    if(object_name.empty()) {
      throw logic_error("object symbol translation produced empty symbol for " + internal_name);
    }
  }
  if(is_exported_definition) {
    const runtime_symbol_policy::RuntimeSymbolInfo object_runtime_info =
        runtime_symbol_policy::classify(object_name);
    if(normalized_internal_name.compare(0, 14, "cppgm_builtin_") == 0 &&
       object_runtime_info.policy ==
           runtime_symbol_policy::RuntimeSymbolMigrationPolicy::host_libcall) {
      return object_name;
    }
  }
  const string runtime_alias = runtime_symbol_policy::object_symbol_alias(object_name);
  if(!runtime_alias.empty()) {
    return runtime_alias;
  }
  return object_name;
}

string translated_debug_symbol_name(
    const map<string, symbol_linkage::SymbolIdentity> & exports,
    const string & internal_name)
{
  map<string, symbol_linkage::SymbolIdentity>::const_iterator found =
      exports.find(internal_name);
  if(found != exports.end() &&
     found->second.keep_internal_alias &&
     !internal_name.empty()) {
    return internal_name;
  }
  return translated_symbol_name(exports, internal_name);
}

bool should_emit_keep_internal_alias_for_function(
    const machine_ir::Function & function,
    const symbol_linkage::SymbolIdentity & exported)
{
  if(!exported.keep_internal_alias) {
    return false;
  }
  const string object_symbol = symbol_linkage::exported_object_symbol(exported);
  if(object_symbol == function.name) {
    return false;
  }
  if(function.name == "@main" && object_symbol == "main") {
    return false;
  }
  return true;
}

string linux_simple_thread_local_object_symbol(const string & internal_name)
{
  if(internal_name.size() < 2 || internal_name[0] != '@') {
    return string();
  }
  const string candidate = internal_name.substr(1);
  if(candidate.empty() || candidate.find("__") != string::npos) {
    return string();
  }
  for(size_t i = 0; i < candidate.size(); ++i) {
    const unsigned char ch = static_cast<unsigned char>(candidate[i]);
    if((ch >= 'a' && ch <= 'z') ||
       (ch >= 'A' && ch <= 'Z') ||
       (ch >= '0' && ch <= '9') ||
       ch == '_') {
      continue;
    }
    return string();
  }
  return candidate;
}

}  // namespace

machine_object::ObjectFile build_machine_object(const vector<string> & srcfiles,
                                                const string & output_target,
                                                bool enable_host_eh,
                                                bool use_macos_static_init_sections,
                                                int debug_info_level,
                                                int optimization_level,
                                                bool use_direct_native_tls_abi)
{
  return build_machine_object(lowir_internal::parse_program(srcfiles),
                              output_target,
                              enable_host_eh,
                              use_macos_static_init_sections,
                              debug_info_level,
                              optimization_level,
                              use_direct_native_tls_abi);
}

namespace {

int infer_debug_info_level_from_program(const lowir_internal::Program & program,
                                        int debug_info_level)
{
  if(debug_info_level > 0) {
    return debug_info_level;
  }

  for(size_t fi = 0; fi < program.functions.size(); ++fi) {
    const lowir_internal::Function & function = program.functions[fi];
    if(function.debug_location.present()) {
      return 1;
    }
    for(size_t bi = 0; bi < function.blocks.size(); ++bi) {
      for(size_t ii = 0; ii < function.blocks[bi].instructions.size(); ++ii) {
        if(function.blocks[bi].instructions[ii].debug_location.present()) {
          return 1;
        }
      }
    }
  }

  return 0;
}

string legacy_startup_alias_for_role(lowir_internal::SymbolRole role)
{
  switch(role) {
    case lowir_internal::SR_ENTRY:
      return "@main";
    case lowir_internal::SR_INIT:
      return "@__cppgm_init";
    case lowir_internal::SR_FINI:
      return "@__cppgm_fini";
    default:
      return string();
  }
}

void add_role_startup_aliases(const lowir_internal::Program & program,
                              const map<string, symbol_linkage::SymbolIdentity> & exports,
                              machine_object::ObjectFile & object)
{
  map<string, size_t> defined_symbol_index;
  for(size_t i = 0; i < object.symbols.size(); ++i) {
    if(object.symbols[i].section == machine_object::Symbol::SS_UNDEFINED) {
      continue;
    }
    defined_symbol_index[object.symbols[i].name] = i;
  }

  for(size_t i = 0; i < program.functions.size(); ++i) {
    const string alias_name =
        legacy_startup_alias_for_role(program.functions[i].metadata.role);
    if(alias_name.empty() || alias_name == program.functions[i].name) {
      continue;
    }

    const string object_name =
        translated_symbol_name(exports, program.functions[i].name);
    map<string, size_t>::const_iterator source =
        defined_symbol_index.find(object_name);
    if(source == defined_symbol_index.end()) {
      throw lowir_internal::ParseError("missing object symbol for LowIR function " +
                                       program.functions[i].name);
    }

    map<string, size_t>::const_iterator existing =
        defined_symbol_index.find(alias_name);
    if(existing != defined_symbol_index.end()) {
      if(existing->second != source->second) {
        throw lowir_internal::ParseError("startup alias collision for " + alias_name);
      }
      continue;
    }

    machine_object::Symbol alias = object.symbols[source->second];
    alias.name = alias_name;
    alias.binding = alias.binding == machine_object::Symbol::SB_WEAK ?
        machine_object::Symbol::SB_WEAK :
        machine_object::Symbol::SB_GLOBAL;
    alias.comdat_group.clear();
    object.symbols.push_back(alias);
    defined_symbol_index[alias.name] = object.symbols.size() - 1;
  }
}

size_t find_defined_symbol_index(const machine_object::ObjectFile & object,
                                 const string & name)
{
  for(size_t i = 0; i < object.symbols.size(); ++i) {
    if(object.symbols[i].section != machine_object::Symbol::SS_UNDEFINED &&
       object.symbols[i].name == name) {
      return i;
    }
  }
  return static_cast<size_t>(-1);
}

string unique_startup_body_symbol_name(const machine_object::ObjectFile & object,
                                       const string & alias_name)
{
  string candidate = alias_name + "_body";
  size_t ordinal = 2;
  while(find_defined_symbol_index(object, candidate) != static_cast<size_t>(-1)) {
    ostringstream name;
    name << alias_name << "_body" << ordinal;
    candidate = name.str();
    ++ordinal;
  }
  return candidate;
}

string startup_target_symbol_name(const lowir_internal::Program & program,
                                  const map<string, symbol_linkage::SymbolIdentity> & exports,
                                  lowir_internal::SymbolRole role)
{
  for(size_t i = 0; i < program.functions.size(); ++i) {
    if(program.functions[i].metadata.role == role) {
      return translated_symbol_name(exports, program.functions[i].name);
    }
  }
  const string legacy = legacy_startup_alias_for_role(role);
  for(size_t i = 0; i < program.functions.size(); ++i) {
    if(program.functions[i].name == legacy) {
      return translated_symbol_name(exports, program.functions[i].name);
    }
  }
  return string();
}

machine_object::ExtraSection & ensure_macho_static_init_section(machine_object::ObjectFile & object)
{
  for(size_t i = 0; i < object.extra_sections.size(); ++i) {
    if(object.extra_sections[i].segment_name == "__TEXT" &&
       object.extra_sections[i].section_name == "__StaticInit") {
      return object.extra_sections[i];
    }
  }
  machine_object::ExtraSection section;
  section.segment_name = "__TEXT";
  section.section_name = "__StaticInit";
  section.macho_flags = MACHO_STATIC_INIT_FLAGS;
  section.macho_align_pow2 = 0;
  object.extra_sections.push_back(section);
  return object.extra_sections.back();
}

void retarget_macos_init_alias_to_static_init(
    const lowir_internal::Program & program,
    const map<string, symbol_linkage::SymbolIdentity> & exports,
    machine_object::ObjectFile & object)
{
  const string alias_name = "@__cppgm_init";
  const size_t alias_index = find_defined_symbol_index(object, alias_name);
  if(alias_index == static_cast<size_t>(-1)) {
    return;
  }

  string target_name =
      startup_target_symbol_name(program, exports, lowir_internal::SR_INIT);
  if(target_name.empty()) {
    target_name = alias_name;
  }
  if(target_name == alias_name) {
    machine_object::Symbol body_symbol = object.symbols[alias_index];
    body_symbol.name = unique_startup_body_symbol_name(object, alias_name);
    body_symbol.binding = machine_object::Symbol::SB_LOCAL;
    body_symbol.comdat_group.clear();
    object.symbols.push_back(body_symbol);
    target_name = body_symbol.name;
  }

  machine_object::ExtraSection & section = ensure_macho_static_init_section(object);
  const size_t wrapper_offset = section.bytes.size();
  const unsigned char wrapper_bytes[] = {
      0x55,
      0x48, 0x89, 0xE5,
      0xE8, 0x00, 0x00, 0x00, 0x00,
      0x5D,
      0xC3};
  section.bytes.insert(section.bytes.end(),
                       wrapper_bytes,
                       wrapper_bytes + sizeof(wrapper_bytes));

  machine_object::ExtraRelocation reloc;
  reloc.kind = machine_object::ExtraRelocation::RK_BRANCH32;
  reloc.target_kind = machine_object::ExtraRelocation::TK_SYMBOL;
  reloc.offset = wrapper_offset + 5;
  reloc.symbol = target_name;
  section.relocations.push_back(reloc);

  machine_object::Symbol & alias = object.symbols[alias_index];
  alias.binding = machine_object::Symbol::SB_LOCAL;
  alias.section = machine_object::Symbol::SS_EXTRA;
  alias.offset = wrapper_offset;
  alias.size = sizeof(wrapper_bytes);
  alias.comdat_group.clear();
  alias.extra_section = "__TEXT,__StaticInit";
}

void collect_lowir_thread_local_wrapper(
    map<string, string> & wrappers_by_global,
    const string & wrapper_symbol,
    const lowir_internal::SymbolMetadata & metadata)
{
  if(metadata.tls_for_symbol.empty()) {
    return;
  }
  map<string, string>::const_iterator found =
      wrappers_by_global.find(metadata.tls_for_symbol);
  if(found != wrappers_by_global.end() && found->second != wrapper_symbol) {
    throw lowir_internal::ParseError(
        "duplicate thread_local wrapper for " + metadata.tls_for_symbol);
  }
  wrappers_by_global[metadata.tls_for_symbol] = wrapper_symbol;
}

map<string, string> lowir_thread_local_wrappers_by_global(
    const lowir_internal::Program & program)
{
  map<string, string> wrappers;
  for(size_t i = 0; i < program.function_declarations.size(); ++i) {
    collect_lowir_thread_local_wrapper(
        wrappers,
        program.function_declarations[i].name,
        program.function_declarations[i].metadata);
  }
  for(size_t i = 0; i < program.functions.size(); ++i) {
    collect_lowir_thread_local_wrapper(wrappers,
                                       program.functions[i].name,
                                       program.functions[i].metadata);
  }
  return wrappers;
}

void append_emutls_thread_local_declaration_wrappers(
    const lowir_internal::Program & program,
    const map<string, symbol_linkage::SymbolIdentity> & exports,
    machine_object::ObjectFile & object)
{
  const ThreadLocalSectionInfo tls_info =
      thread_local_section_info_for_target(object.target);
  if(tls_info.abi != ThreadLocalSectionInfo::ABI_EMUTLS &&
     tls_info.abi != ThreadLocalSectionInfo::ABI_DIRECT_NATIVE &&
     tls_info.abi != ThreadLocalSectionInfo::ABI_ELF) {
    return;
  }

  const map<string, string> wrapper_symbols =
      lowir_thread_local_wrappers_by_global(program);
  const bool default_unexported_symbols_global = exports.empty();
  set<string> defined_symbols;
  set<string> defined_tls_globals;
  for(size_t i = 0; i < object.symbols.size(); ++i) {
    if(object.symbols[i].section == machine_object::Symbol::SS_UNDEFINED) {
      continue;
    }
    defined_symbols.insert(object.symbols[i].name);
  }
  for(size_t i = 0; i < program.globals.size(); ++i) {
    if(program.globals[i].storage == lowir_internal::GSM_THREAD_LOCAL) {
      defined_tls_globals.insert(program.globals[i].name);
    }
  }

  for(size_t i = 0; i < program.global_declarations.size(); ++i) {
    const lowir_internal::GlobalDeclaration & decl = program.global_declarations[i];
    if(decl.storage != lowir_internal::GSM_THREAD_LOCAL ||
       defined_tls_globals.count(decl.name) != 0) {
      continue;
    }

    map<string, string>::const_iterator wrapper =
        wrapper_symbols.find(decl.name);
    if(wrapper == wrapper_symbols.end()) {
      continue;
    }
    const string & wrapper_internal = wrapper->second;
    const string wrapper_object = translated_symbol_name(exports, wrapper_internal);
    if(wrapper_object.empty() || defined_symbols.count(wrapper_object) != 0) {
      continue;
    }

    const size_t wrapper_offset = align_up(object.code.size(), 16);
    if(wrapper_offset > object.code.size()) {
      object.code.insert(object.code.end(), wrapper_offset - object.code.size(), 0x90);
    }

    X86Assembler wrapper_bytes;
    if(tls_info.abi == ThreadLocalSectionInfo::ABI_DIRECT_NATIVE) {
      const size_t data_patch = wrapper_bytes.emit_lea_r64_rip_rel32_placeholder(XR_RAX);
      append_relocation(object.relocations,
                        mobj::Symbol::SS_CODE,
                        wrapper_offset + data_patch,
                        mobj::Relocation::RK_PCREL32,
                        decl.name);
      wrapper_bytes.emit_ret();
    } else if(tls_info.abi == ThreadLocalSectionInfo::ABI_ELF) {
      const string simple_tls_object_symbol =
          linux_simple_thread_local_object_symbol(decl.name);
      const string tls_object_symbol = !simple_tls_object_symbol.empty()
          ? simple_tls_object_symbol
          : thread_local_runtime_object_symbol(exports, decl.name, object.target);
      emit_elf_tls_address_bytes(wrapper_bytes,
                                 wrapper_offset,
                                 tls_object_symbol,
                                 object.relocations);
      wrapper_bytes.emit_ret();
    } else {
      emit_emutls_wrapper_bytes(wrapper_bytes,
                                wrapper_offset,
                                thread_local_runtime_object_symbol(exports,
                                                                   decl.name,
                                                                   object.target),
                                object.relocations);
    }
    object.code.insert(object.code.end(),
                       wrapper_bytes.bytes().begin(),
                       wrapper_bytes.bytes().end());

    machine_object::Symbol symbol;
    symbol.binding = default_unexported_symbols_global ?
        machine_object::Symbol::SB_GLOBAL :
        machine_object::Symbol::SB_LOCAL;
    symbol.section = machine_object::Symbol::SS_CODE;
    symbol.name = wrapper_object;
    symbol.offset = wrapper_offset;
    symbol.size = wrapper_bytes.size();

    map<string, symbol_linkage::SymbolIdentity>::const_iterator exported =
        exports.find(wrapper_internal);
    if(exported == exports.end()) {
      exported = exports.find(decl.name);
    }
    if(exported != exports.end()) {
      if(exported->second.linkage == symbol_linkage::SL_INTERNAL ||
         (exported->second.prefer_local_object_binding &&
          !symbol_linkage::has_weak_linkage(exported->second))) {
        symbol.binding = machine_object::Symbol::SB_LOCAL;
      } else {
        symbol.binding = symbol_linkage::has_weak_linkage(exported->second) ?
            machine_object::Symbol::SB_WEAK :
            machine_object::Symbol::SB_GLOBAL;
      }
    }
    if(tls_info.abi == ThreadLocalSectionInfo::ABI_EMUTLS &&
       symbol.binding == machine_object::Symbol::SB_GLOBAL) {
      // GCC emits imported emutls access wrappers as weak definitions on
      // Darwin so multiple TUs can access the same extern thread_local object.
      symbol.binding = machine_object::Symbol::SB_WEAK;
    }
    if(tls_info.abi == ThreadLocalSectionInfo::ABI_ELF &&
       symbol.binding == machine_object::Symbol::SB_GLOBAL) {
      symbol.binding = machine_object::Symbol::SB_WEAK;
    }

    bool replaced_undefined = false;
    for(size_t si = 0; si < object.symbols.size(); ++si) {
      if(object.symbols[si].name == symbol.name &&
         object.symbols[si].section == machine_object::Symbol::SS_UNDEFINED) {
        object.symbols[si] = symbol;
        replaced_undefined = true;
        break;
      }
    }
    if(!replaced_undefined) {
      object.symbols.push_back(symbol);
    }
    defined_symbols.insert(symbol.name);
  }
}

void force_external_binding_for_function_declaration_imports(
    const lowir_internal::Program & source_program,
    machine_ir::Program & machine_program)
{
  set<string> defined_functions;
  for(size_t i = 0; i < source_program.functions.size(); ++i) {
    defined_functions.insert(source_program.functions[i].name);
  }

  set<string> defined_thread_local_globals;
  for(size_t i = 0; i < source_program.globals.size(); ++i) {
    if(source_program.globals[i].storage == lowir_internal::GSM_THREAD_LOCAL) {
      defined_thread_local_globals.insert(source_program.globals[i].name);
    }
  }

  set<string> declaration_only_functions;
  for(size_t i = 0; i < source_program.function_declarations.size(); ++i) {
    const string & tls_target =
        source_program.function_declarations[i].metadata.tls_for_symbol;
    if(!tls_target.empty() &&
       defined_thread_local_globals.count(tls_target) != 0) {
      continue;
    }
    const string & name = source_program.function_declarations[i].name;
    if(defined_functions.count(name) == 0) {
      declaration_only_functions.insert(name);
    }
  }
  if(declaration_only_functions.empty()) {
    return;
  }

  for(size_t i = 0; i < machine_program.exported_symbols.size(); ++i) {
    symbol_linkage::SymbolIdentity & symbol = machine_program.exported_symbols[i];
    if(declaration_only_functions.count(symbol.internal_symbol) == 0 ||
       !symbol_linkage::has_object_symbol(symbol)) {
      continue;
    }
    // Declaration-only functions have no local fallback in this object.  Even
    // when the source entity has ODR/weak linkage, a call import must be strong
    // so the host linker keeps the provider library under --as-needed.
    symbol.linkage = symbol_linkage::SL_EXTERNAL;
    symbol.prefer_local_object_binding = false;
  }
}

}  // namespace

machine_object::ObjectFile build_machine_object(const lowir_internal::Program & program,
                                                const string & output_target,
                                                bool enable_host_eh,
                                                bool use_macos_static_init_sections,
                                                int debug_info_level,
                                                int optimization_level,
                                                bool use_direct_native_tls_abi)
{
  const ScopedDirectNativeTlsAbi tls_abi_guard(use_direct_native_tls_abi);
  debug_info_level = infer_debug_info_level_from_program(program, debug_info_level);
  if(parser_trace::enabled("object.symbol")) {
    for(size_t i = 0; i < program.functions.size(); ++i) {
      parser_trace::note("object.symbol",
                         string(),
                         string("stage=lowir-function internal=") + program.functions[i].name);
    }
  }
  machine_ir::Program machine_program =
      build_lowir_machine_ir_object(program, output_target, enable_host_eh);
  machine_program = optimize_machine_ir_program(machine_program, optimization_level);
  force_external_binding_for_function_declaration_imports(program, machine_program);
  if(parser_trace::enabled("object.symbol")) {
    for(size_t i = 0; i < machine_program.functions.size(); ++i) {
      parser_trace::note("object.symbol",
                         string(),
                         string("stage=machine-function internal=") +
                             machine_program.functions[i].name);
    }
  }
  machine_object::ObjectFile object =
      build_machine_object(machine_program,
                           debug_info_level,
                           use_direct_native_tls_abi);
  const map<string, symbol_linkage::SymbolIdentity> exports =
      export_map(program.exported_symbols);
  for(size_t i = 0; i < object.symbols.size(); ++i) {
    if(object.symbols[i].section == machine_object::Symbol::SS_UNDEFINED) {
      object.symbols[i].name =
          translated_symbol_name(exports, object.symbols[i].name);
    }
  }
  add_role_startup_aliases(program, exports, object);
  if(machine_program.target == "macos" && use_macos_static_init_sections) {
    retarget_macos_init_alias_to_static_init(program, exports, object);
  }
  append_emutls_thread_local_declaration_wrappers(program, exports, object);
  const map<string, string> thread_local_wrapper_symbols =
      lowir_thread_local_wrappers_by_global(program);
  set<string> declared_symbols;
  set<string> declared_thread_local_wrappers;
  for(size_t i = 0; i < program.function_declarations.size(); ++i) {
    declared_symbols.insert(
        translated_symbol_name(exports, program.function_declarations[i].name));
  }
  for(size_t i = 0; i < program.global_declarations.size(); ++i) {
    declared_symbols.insert(program.global_declarations[i].storage == lowir_internal::GSM_THREAD_LOCAL
                                ? thread_local_runtime_object_symbol(exports,
                                                                     program.global_declarations[i].name,
                                                                     machine_program.target)
                                : translated_symbol_name(exports,
                                                         program.global_declarations[i].name));
    if(program.global_declarations[i].storage == lowir_internal::GSM_THREAD_LOCAL) {
      map<string, string>::const_iterator wrapper =
          thread_local_wrapper_symbols.find(program.global_declarations[i].name);
      if(wrapper != thread_local_wrapper_symbols.end()) {
        declared_thread_local_wrappers.insert(
            translated_symbol_name(exports, wrapper->second));
      }
    }
  }
  for(size_t i = 0; i < program.globals.size(); ++i) {
    if(program.globals[i].storage == lowir_internal::GSM_THREAD_LOCAL) {
      declared_symbols.insert(
          thread_local_runtime_object_symbol(exports,
                                             program.globals[i].name,
                                             machine_program.target));
    }
    if(program.globals[i].storage == lowir_internal::GSM_THREAD_LOCAL) {
      map<string, string>::const_iterator wrapper =
          thread_local_wrapper_symbols.find(program.globals[i].name);
      if(wrapper != thread_local_wrapper_symbols.end()) {
        declared_thread_local_wrappers.insert(
            translated_symbol_name(exports, wrapper->second));
      }
    }
  }
  for(size_t i = 0; i < object.symbols.size(); ++i) {
    if(object.symbols[i].section != machine_object::Symbol::SS_UNDEFINED) {
      continue;
    }
    if(runtime_symbol_policy::is_reserved_internal_symbol(object.symbols[i].name)) {
      continue;
    }
    const runtime_symbol_policy::RuntimeSymbolRole runtime_role =
        runtime_symbol_policy::classify(object.symbols[i].name).role;
    const runtime_symbol_policy::RuntimeSymbolMigrationPolicy runtime_policy =
        runtime_symbol_policy::classify(object.symbols[i].name).policy;
    if(runtime_role == runtime_symbol_policy::RuntimeSymbolRole::eh_resume ||
       runtime_role == runtime_symbol_policy::RuntimeSymbolRole::eh_personality) {
      continue;
    }
    if(runtime_policy == runtime_symbol_policy::RuntimeSymbolMigrationPolicy::host_libcall ||
       runtime_policy == runtime_symbol_policy::RuntimeSymbolMigrationPolicy::host_abi) {
      continue;
    }
    if(declared_symbols.count(object.symbols[i].name) != 0) {
      continue;
    }
    if(declared_thread_local_wrappers.count(object.symbols[i].name) != 0) {
      continue;
    }
    throw lowir_internal::ParseError("undefined symbol lacks LowIR declaration " +
                                     object.symbols[i].name);
  }
  return object;
}

machine_object::ObjectFile build_machine_object(const machine_ir::Program & program,
                                                int debug_info_level,
                                                bool use_direct_native_tls_abi)
{
  const ScopedDirectNativeTlsAbi tls_abi_guard(use_direct_native_tls_abi);
  machine_object::ObjectFile object;
  object.target = program.target;
  const ObjectLayout layout = layout_object(program);
  const ThreadLocalSectionInfo tls_info =
      thread_local_section_info_for_target(program.target);
  const map<string, symbol_linkage::SymbolIdentity> exports =
      export_map(program.exported_symbols);
  const bool default_unexported_symbols_global = exports.empty();
  vector<hosteh::HostEhFunctionInfo> host_eh_functions;
  object.code = build_code_bytes(program,
                                 layout,
                                 exports,
                                 object.relocations,
                                 &host_eh_functions);
  object.data = build_data_bytes(program, layout, object.relocations);
  if(layout.thread_local_template_data_size != 0 ||
     layout.thread_local_template_bss_size != 0) {
    const vector<mobj::ExtraSection> tls_sections =
        build_thread_local_template_sections(program, layout);
    object.extra_sections.insert(object.extra_sections.end(),
                                 tls_sections.begin(),
                                 tls_sections.end());
  }
  if(layout.thread_local_descriptor_size != 0 &&
     tls_info.abi == ThreadLocalSectionInfo::ABI_MACHO_TLV) {
    object.extra_sections.push_back(build_thread_local_vars_section(program, layout, exports));
  }
  if(layout.readonly_data_size != 0) {
    object.extra_sections.push_back(build_readonly_data_section(program, layout));
  }
  if(debug_info_level >= 1) {
    DwarfCompilationUnitInfo dwarf_info =
        collect_dwarf_compilation_unit_info(program, layout);
    if(!dwarf_info.empty()) {
      object.extra_sections.push_back(build_dwarf_abbrev_section(program));
      object.extra_sections.push_back(build_dwarf_location_section(program, dwarf_info));
      object.extra_sections.push_back(build_dwarf_info_section(program, dwarf_info));
      object.extra_sections.push_back(build_dwarf_line_section(program, dwarf_info));
    }
  }
  hosteh::HostEhObjectLayout host_eh_layout;
  host_eh_layout.function_offsets = layout.function_offsets;
  for(map<string, FunctionLayout>::const_iterator it = layout.function_layouts.begin();
      it != layout.function_layouts.end();
      ++it) {
    hosteh::HostEhFunctionLayout host_layout;
    host_layout.block_offsets = it->second.block_offsets;
    host_layout.size = it->second.size;
    host_eh_layout.function_layouts[it->first] = host_layout;
  }
  hosteh::append_host_eh_sections(program, host_eh_layout, host_eh_functions, object);
  for(size_t i = 0; i < object.relocations.size(); ++i) {
    object.relocations[i].symbol =
        translated_symbol_name(exports, object.relocations[i].symbol);
  }
  for(size_t si = 0; si < object.extra_sections.size(); ++si) {
    const bool use_debug_symbol_translation =
        is_generated_debug_section(program.target,
                                   object.extra_sections[si].section_name);
    for(size_t ri = 0; ri < object.extra_sections[si].relocations.size(); ++ri) {
      mobj::ExtraRelocation & reloc = object.extra_sections[si].relocations[ri];
      if(reloc.target_kind == mobj::ExtraRelocation::TK_SYMBOL) {
        reloc.symbol = use_debug_symbol_translation ?
            translated_debug_symbol_name(exports, reloc.symbol) :
            translated_symbol_name(exports, reloc.symbol);
      }
    }
  }

  map<string, mobj::Symbol::Binding> exported_bindings;
  for(map<string, symbol_linkage::SymbolIdentity>::const_iterator it = exports.begin();
      it != exports.end();
      ++it) {
    if(it->second.linkage == symbol_linkage::SL_INTERNAL ||
       (it->second.prefer_local_object_binding &&
        !symbol_linkage::has_weak_linkage(it->second))) {
      continue;
    }
    const string object_symbol = translated_symbol_name(exports, it->first);
    exported_bindings[object_symbol] =
        symbol_linkage::has_weak_linkage(it->second) ?
            mobj::Symbol::SB_WEAK :
            mobj::Symbol::SB_GLOBAL;
  }
  const map<string, vector<string> > explicit_object_aliases =
      object_alias_map(program.object_aliases);

  map<string, bool> defined_symbols;
  for(size_t i = 0; i < object.symbols.size(); ++i) {
    if(object.symbols[i].section != machine_object::Symbol::SS_UNDEFINED) {
      defined_symbols[object.symbols[i].name] = true;
    }
  }
  set<string> translated_definition_symbols;
  for(size_t i = 0; i < program.functions.size(); ++i) {
    translated_definition_symbols.insert(
        translated_symbol_name(exports, program.functions[i].name));
  }
  for(size_t i = 0; i < program.globals.size(); ++i) {
    if(program.globals[i].thread_local_storage) {
      string global_symbol =
          thread_local_runtime_object_symbol(exports, program.globals[i].name, program.target);
      if(tls_info.abi == ThreadLocalSectionInfo::ABI_ELF) {
        const string simple_tls_object_symbol =
            linux_simple_thread_local_object_symbol(program.globals[i].name);
        if(!simple_tls_object_symbol.empty()) {
          global_symbol = simple_tls_object_symbol;
        }
      }
      translated_definition_symbols.insert(global_symbol);
    } else {
      translated_definition_symbols.insert(
          translated_symbol_name(exports, program.globals[i].name));
    }
  }

  for(size_t i = 0; i < program.functions.size(); ++i) {
    machine_object::Symbol symbol;
    symbol.binding = default_unexported_symbols_global ?
        machine_object::Symbol::SB_GLOBAL :
        machine_object::Symbol::SB_LOCAL;
    symbol.section = machine_object::Symbol::SS_CODE;
    symbol.name = translated_symbol_name(exports, program.functions[i].name);
    symbol.offset = layout.function_offsets.find(program.functions[i].name)->second;
    symbol.size = layout.function_layouts.find(program.functions[i].name)->second.size;
    map<string, symbol_linkage::SymbolIdentity>::const_iterator exported =
        exports.find(program.functions[i].name);
    if(exported != exports.end()) {
      if(exported->second.linkage == symbol_linkage::SL_INTERNAL ||
         (exported->second.prefer_local_object_binding &&
          !symbol_linkage::has_weak_linkage(exported->second))) {
        symbol.binding = machine_object::Symbol::SB_LOCAL;
      } else {
        symbol.binding = symbol_linkage::has_weak_linkage(exported->second) ?
            machine_object::Symbol::SB_WEAK :
            machine_object::Symbol::SB_GLOBAL;
        if(symbol.binding == machine_object::Symbol::SB_WEAK) {
          symbol.comdat_group = symbol.name;
        }
      }
    }
    if(parser_trace::enabled("object.symbol")) {
      ostringstream trace;
      trace << "kind=function"
            << " internal=" << program.functions[i].name
            << " object=" << symbol.name
            << " binding=" << object_binding_name(symbol.binding);
      if(exported != exports.end()) {
        trace << " exported-linkage="
              << (exported->second.linkage == symbol_linkage::SL_INTERNAL ? "internal" :
                  exported->second.linkage == symbol_linkage::SL_WEAK ? "weak" :
                  "external")
              << " keep-alias=" << (exported->second.keep_internal_alias ? "yes" : "no");
      } else {
        trace << " exported-linkage=none";
      }
      parser_trace::note("object.symbol", string(), trace.str());
    }
    object.symbols.push_back(symbol);
    defined_symbols[symbol.name] = true;

    if(exported != exports.end() &&
       should_emit_keep_internal_alias_for_function(program.functions[i], exported->second)) {
      machine_object::Symbol alias = symbol;
      alias.name = program.functions[i].name;
      alias.comdat_group.clear();
      object.symbols.push_back(alias);
      defined_symbols[alias.name] = true;
    }

    append_object_alias_symbols(explicit_object_aliases,
                                program.functions[i].name,
                                translated_definition_symbols,
                                defined_symbols,
                                symbol,
                                object.symbols);

    const FunctionLayout & function_layout =
        layout.function_layouts.find(program.functions[i].name)->second;
    for(size_t bi = 0; bi < program.functions[i].blocks.size(); ++bi) {
      machine_object::Symbol local;
      local.binding = machine_object::Symbol::SB_LOCAL;
      local.section = machine_object::Symbol::SS_CODE;
      local.name = block_symbol(program.functions[i].name,
                                program.functions[i].blocks[bi].label);
      local.offset = layout.function_offsets.find(program.functions[i].name)->second +
          function_layout.block_offsets.find(program.functions[i].blocks[bi].label)->second;
      local.hidden_code_label = true;
      object.symbols.push_back(local);
      defined_symbols[local.name] = true;
    }
  }
  for(size_t i = 0; i < program.globals.size(); ++i) {
    machine_object::Symbol symbol;
    symbol.binding = default_unexported_symbols_global ?
        machine_object::Symbol::SB_GLOBAL :
        machine_object::Symbol::SB_LOCAL;
    const bool is_thread_local = program.globals[i].thread_local_storage;
    symbol.section = is_thread_local
        ? ((tls_info.abi == ThreadLocalSectionInfo::ABI_MACHO_TLV ||
            tls_info.abi == ThreadLocalSectionInfo::ABI_ELF) ?
               machine_object::Symbol::SS_EXTRA :
               machine_object::Symbol::SS_DATA)
        : (program.globals[i].readonly ?
               machine_object::Symbol::SS_EXTRA :
               machine_object::Symbol::SS_DATA);
    symbol.name = is_thread_local
        ? thread_local_runtime_object_symbol(exports, program.globals[i].name, program.target)
        : translated_symbol_name(exports, program.globals[i].name);
    if(is_thread_local &&
       program.target == "linux" &&
       tls_info.abi == ThreadLocalSectionInfo::ABI_ELF) {
      const string linux_tls_name =
          linux_simple_thread_local_object_symbol(program.globals[i].name);
      if(!linux_tls_name.empty()) {
        symbol.name = linux_tls_name;
      }
    }
    if(is_thread_local) {
      if(tls_info.abi == ThreadLocalSectionInfo::ABI_MACHO_TLV ||
         tls_info.abi == ThreadLocalSectionInfo::ABI_EMUTLS) {
        if(tls_info.abi == ThreadLocalSectionInfo::ABI_EMUTLS) {
          symbol.offset =
              layout.thread_local_descriptor_offsets.find(program.globals[i].name)->second;
        } else {
          symbol.extra_section = tls_info.vars_segment_name + "," + tls_info.vars_section_name;
          symbol.offset =
              layout.thread_local_descriptor_offsets.find(program.globals[i].name)->second;
        }
      } else if(tls_info.abi == ThreadLocalSectionInfo::ABI_DIRECT_NATIVE) {
        symbol.offset = layout.global_offsets.find(program.globals[i].name)->second;
      } else {
        symbol.extra_section = tls_info.data_segment_name + "," + tls_info.data_section_name;
        symbol.offset =
            layout.thread_local_template_offsets.find(program.globals[i].name)->second;
      }
    } else if(program.globals[i].readonly) {
      const ReadonlySectionInfo info = readonly_section_info_for_target(program.target);
      symbol.extra_section = info.segment_name + "," + info.section_name;
      symbol.offset = layout.readonly_global_offsets.find(program.globals[i].name)->second;
    } else {
      symbol.offset = layout.global_offsets.find(program.globals[i].name)->second;
    }
    map<string, symbol_linkage::SymbolIdentity>::const_iterator exported =
        exports.find(program.globals[i].name);
    if(exported != exports.end()) {
      symbol.binding = (exported->second.linkage == symbol_linkage::SL_INTERNAL ||
                        (exported->second.prefer_local_object_binding &&
                         !symbol_linkage::has_weak_linkage(exported->second))) ?
          machine_object::Symbol::SB_LOCAL :
          (symbol_linkage::has_weak_linkage(exported->second) ?
               machine_object::Symbol::SB_WEAK :
               machine_object::Symbol::SB_GLOBAL);
    }
    if(is_thread_local && tls_info.abi == ThreadLocalSectionInfo::ABI_MACHO_TLV) {
      symbol.binding = machine_object::Symbol::SB_LOCAL;
    }
    if(parser_trace::enabled("object.symbol")) {
      ostringstream trace;
      trace << "kind=global"
            << " internal=" << program.globals[i].name
            << " object=" << symbol.name
            << " binding=" << object_binding_name(symbol.binding);
      if(exported != exports.end()) {
        trace << " exported-linkage="
              << (exported->second.linkage == symbol_linkage::SL_INTERNAL ? "internal" :
                  exported->second.linkage == symbol_linkage::SL_WEAK ? "weak" :
                  "external")
              << " keep-alias=" << (exported->second.keep_internal_alias ? "yes" : "no");
      } else {
        trace << " exported-linkage=none";
      }
      parser_trace::note("object.symbol", string(), trace.str());
    }
    object.symbols.push_back(symbol);

    if(exported != exports.end() && exported->second.keep_internal_alias &&
       symbol_linkage::exported_object_symbol(exported->second) != program.globals[i].name) {
      machine_object::Symbol alias = symbol;
      alias.name = program.globals[i].name;
      object.symbols.push_back(alias);
      defined_symbols[alias.name] = true;
    }
    if(is_thread_local &&
       (tls_info.abi == ThreadLocalSectionInfo::ABI_MACHO_TLV ||
        tls_info.abi == ThreadLocalSectionInfo::ABI_EMUTLS)) {
      machine_object::Symbol init_symbol;
      init_symbol.binding = machine_object::Symbol::SB_LOCAL;
      init_symbol.section = machine_object::Symbol::SS_EXTRA;
      init_symbol.extra_section =
          thread_local_template_extra_section(program.globals[i], tls_info);
      init_symbol.name = thread_local_template_symbol(exports,
                                                      program.globals[i].name,
                                                      program.target);
      init_symbol.offset =
          layout.thread_local_template_offsets.find(program.globals[i].name)->second;
      object.symbols.push_back(init_symbol);
      defined_symbols[init_symbol.name] = true;
    }
    defined_symbols[symbol.name] = true;
    append_object_alias_symbols(explicit_object_aliases,
                                program.globals[i].name,
                                translated_definition_symbols,
                                defined_symbols,
                                symbol,
                                object.symbols);
  }

  for(size_t i = 0; i < program.globals.size(); ++i) {
    if(!program.globals[i].thread_local_storage ||
       program.globals[i].thread_local_wrapper_symbol.empty()) {
      continue;
    }
    machine_object::Symbol symbol;
    symbol.binding = default_unexported_symbols_global ?
        machine_object::Symbol::SB_GLOBAL :
        machine_object::Symbol::SB_LOCAL;
    symbol.section = machine_object::Symbol::SS_CODE;
    symbol.name = translated_symbol_name(
        exports, program.globals[i].thread_local_wrapper_symbol);
    symbol.offset = layout.thread_local_wrapper_offsets.find(program.globals[i].name)->second;
    symbol.size = thread_local_wrapper_size(program.target);
    map<string, symbol_linkage::SymbolIdentity>::const_iterator exported =
        exports.find(program.globals[i].thread_local_wrapper_symbol);
    if(exported == exports.end()) {
      exported = exports.find(program.globals[i].name);
    }
    if(exported != exports.end()) {
      if(exported->second.linkage == symbol_linkage::SL_INTERNAL ||
         (exported->second.prefer_local_object_binding &&
          !symbol_linkage::has_weak_linkage(exported->second))) {
        symbol.binding = machine_object::Symbol::SB_LOCAL;
      } else {
        symbol.binding = symbol_linkage::has_weak_linkage(exported->second) ?
            machine_object::Symbol::SB_WEAK :
            machine_object::Symbol::SB_GLOBAL;
      }
    }
    if(tls_info.abi == ThreadLocalSectionInfo::ABI_EMUTLS) {
      // Under GCC's emutls ABI on Darwin, the defining TU owns the
      // descriptor/template objects while importing TUs own the external
      // __ZTW* wrapper surface. Keeping the defining-TU wrapper local avoids
      // duplicate exported wrapper symbols across TUs.
      symbol.binding = machine_object::Symbol::SB_LOCAL;
    } else if(tls_info.abi == ThreadLocalSectionInfo::ABI_ELF &&
              symbol.binding == machine_object::Symbol::SB_GLOBAL) {
      symbol.binding = machine_object::Symbol::SB_WEAK;
    }
    if(parser_trace::enabled("object.symbol")) {
      ostringstream trace;
      trace << "kind=tls-wrapper"
            << " internal=" << program.globals[i].thread_local_wrapper_symbol
            << " object=" << symbol.name
            << " binding=" << object_binding_name(symbol.binding);
      if(exported != exports.end()) {
        trace << " exported-linkage="
              << (exported->second.linkage == symbol_linkage::SL_INTERNAL ? "internal" :
                  exported->second.linkage == symbol_linkage::SL_WEAK ? "weak" :
                  "external");
      } else {
        trace << " exported-linkage=none";
      }
      parser_trace::note("object.symbol", string(), trace.str());
    }
    object.symbols.push_back(symbol);
    defined_symbols[symbol.name] = true;
  }

  map<string, bool> declared_undefined_symbols;
  for(size_t i = 0; i < object.relocations.size(); ++i) {
    const string & name = object.relocations[i].symbol;
    if(defined_symbols.count(name) != 0 || declared_undefined_symbols.count(name) != 0) {
      continue;
    }
    machine_object::Symbol symbol;
    symbol.binding = exported_bindings.count(name) != 0 ?
        exported_bindings.find(name)->second :
        machine_object::Symbol::SB_GLOBAL;
    symbol.section = machine_object::Symbol::SS_UNDEFINED;
    symbol.name = name;
    if(parser_trace::enabled("object.undefined")) {
      ostringstream trace;
      trace << "object=" << symbol.name
            << " binding=" << object_binding_name(symbol.binding);
      parser_trace::note("object.undefined", string(), trace.str());
    }
    object.symbols.push_back(symbol);
    declared_undefined_symbols[name] = true;
  }
  for(size_t si = 0; si < object.extra_sections.size(); ++si) {
    for(size_t ri = 0; ri < object.extra_sections[si].relocations.size(); ++ri) {
      const mobj::ExtraRelocation & reloc = object.extra_sections[si].relocations[ri];
      if(reloc.target_kind != mobj::ExtraRelocation::TK_SYMBOL) {
        continue;
      }
      const string & name = reloc.symbol;
      if(defined_symbols.count(name) != 0 || declared_undefined_symbols.count(name) != 0) {
        continue;
      }
      machine_object::Symbol symbol;
      symbol.binding = exported_bindings.count(name) != 0 ?
          exported_bindings.find(name)->second :
          machine_object::Symbol::SB_GLOBAL;
      symbol.section = machine_object::Symbol::SS_UNDEFINED;
      symbol.name = name;
      object.symbols.push_back(symbol);
      declared_undefined_symbols[name] = true;
    }
  }
  return object;
}

namespace {

bool lowir_function_uses_host_eh_object_mode(
    const lowir_internal::Function & function,
    const map<string, lowir_internal::SymbolRole> & function_roles,
    bool has_host_eh_runtime_declaration)
{
  for(size_t bi = 0; bi < function.blocks.size(); ++bi) {
    for(size_t ii = 0; ii < function.blocks[bi].instructions.size(); ++ii) {
      const lowir_internal::Instruction & inst = function.blocks[bi].instructions[ii];
      if(inst.kind == lowir_internal::Instruction::IK_EXCEPTION_SELECTOR) {
        return true;
      }
      if(has_host_eh_runtime_declaration &&
         (inst.kind == lowir_internal::Instruction::IK_EH_TRY ||
          inst.kind == lowir_internal::Instruction::IK_EH_CLEANUP ||
          inst.kind == lowir_internal::Instruction::IK_RESUME ||
          inst.kind == lowir_internal::Instruction::IK_EXCEPTION)) {
        return true;
      }
      if(inst.kind == lowir_internal::Instruction::IK_CALL &&
         inst.first.kind == lowir_internal::Operand::OP_GLOBAL) {
        map<string, lowir_internal::SymbolRole>::const_iterator found =
            function_roles.find(inst.first.text);
        if(found != function_roles.end() &&
           lowir_internal::is_host_eh_symbol_role(found->second)) {
          return true;
        }
      }
    }
  }
  return false;
}

bool lowir_program_uses_host_eh_object_mode(const lowir_internal::Program & program)
{
  map<string, lowir_internal::SymbolRole> function_roles;
  bool has_host_eh_runtime_declaration = false;
  for(size_t i = 0; i < program.function_declarations.size(); ++i) {
    const lowir_internal::FunctionDeclaration & declaration =
        program.function_declarations[i];
    function_roles[declaration.name] = declaration.metadata.role;
    if(lowir_internal::is_host_eh_symbol_role(declaration.metadata.role)) {
      has_host_eh_runtime_declaration = true;
    }
  }
  for(size_t i = 0; i < program.functions.size(); ++i) {
    const lowir_internal::Function & function = program.functions[i];
    function_roles[function.name] = function.metadata.role;
    if(lowir_internal::is_host_eh_symbol_role(function.metadata.role)) {
      has_host_eh_runtime_declaration = true;
    }
  }

  for(size_t i = 0; i < program.functions.size(); ++i) {
    if(lowir_function_uses_host_eh_object_mode(program.functions[i],
                                              function_roles,
                                              has_host_eh_runtime_declaration)) {
      return true;
    }
  }
  return false;
}

}  // namespace

void write_lowir_object_file(const vector<string> & srcfiles,
                             const string & outfile,
                             const string & output_target)
{
  const lowir_internal::Program program = lowir_internal::parse_program(srcfiles);
  machine_object::write_object_file(outfile,
                                    build_machine_object(program,
                                                         output_target,
                                                         lowir_program_uses_host_eh_object_mode(
                                                             program),
                                                         false,
                                                         0,
                                                         0,
                                                         true));
}
