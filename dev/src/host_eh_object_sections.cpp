#include "host_eh_object_sections.h"

#include <algorithm>
#include <cstdint>
#include <map>
#include <set>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

using namespace std;

#include "symbol_linkage.h"

namespace host_eh_object_sections {

namespace mir = machine_ir;
namespace mobj = machine_object;

namespace {

const uint32_t MACHO_S_ATTR_DEBUG = 0x02000000U;
const uint32_t MACHO_UNWIND_HAS_LSDA = 0x40000000U;
const uint32_t MACHO_UNWIND_X86_64_MODE_RBP_FRAME = 0x01000000U;

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

vector<HostEhCallSite> add_no_landingpad_gaps(const vector<HostEhCallSite> & call_sites,
                                              size_t function_size)
{
  vector<HostEhCallSite> out;
  if(call_sites.empty()) {
    return out;
  }
  size_t cursor = call_sites[0].start;
  for(size_t i = 0; i < call_sites.size(); ++i) {
    const HostEhCallSite & call_site = call_sites[i];
    if(call_site.start < cursor) {
      throw logic_error("overlapping host EH call-site ranges");
    }
    if(call_site.start > cursor) {
      HostEhCallSite gap;
      gap.start = cursor;
      gap.length = call_site.start - cursor;
      out.push_back(gap);
    }
    if(call_site.length != 0) {
      out.push_back(call_site);
    }
    cursor = call_site.start + call_site.length;
  }
  if(cursor < function_size) {
    HostEhCallSite tail;
    tail.start = cursor;
    tail.length = function_size - cursor;
    out.push_back(tail);
  }
  return out;
}

struct HostEhTypeTablePlan
{
  vector<string> type_entries;
  map<string, long long> filter_index_by_symbol;
};

string host_eh_clause_key(const mir::HostEhClause & clause)
{
  return clause.catch_all ? string() : clause.type_symbol;
}

void assign_host_eh_selector(map<long long, string> & key_by_selector,
                             map<string, long long> & selector_by_key,
                             long long selector,
                             const string & key,
                             const string & function_name)
{
  if(selector <= 0) {
    throw logic_error("invalid host EH selector in " + function_name);
  }
  map<long long, string>::const_iterator existing_selector =
      key_by_selector.find(selector);
  if(existing_selector != key_by_selector.end() &&
     existing_selector->second != key) {
    throw logic_error("conflicting host EH selector in " + function_name);
  }
  key_by_selector[selector] = key;
  if(selector_by_key.count(key) == 0) {
    selector_by_key[key] = selector;
  }
}

HostEhTypeTablePlan plan_host_eh_type_table(
    const string & function_name,
    const map<string, vector<mir::HostEhClause> > & clauses_by_landingpad)
{
  map<long long, string> key_by_selector;
  map<string, long long> selector_by_key;

  for(map<string, vector<mir::HostEhClause> >::const_iterator it =
          clauses_by_landingpad.begin();
      it != clauses_by_landingpad.end();
      ++it) {
    for(size_t ci = 0; ci < it->second.size(); ++ci) {
      const mir::HostEhClause & clause = it->second[ci];
      if(clause.kind == mir::HostEhClause::HC_CLEANUP) {
        continue;
      }
      if(clause.kind == mir::HostEhClause::HC_FILTER) {
        continue;
      }
      if(clause.selector > 0) {
        assign_host_eh_selector(key_by_selector,
                                selector_by_key,
                                clause.selector,
                                host_eh_clause_key(clause),
                                function_name);
      }
    }
  }

  long long next_selector = 1;
  auto assign_fallback_selector = [&](const string & key) {
    if(selector_by_key.count(key) != 0) {
      return;
    }
    while(key_by_selector.count(next_selector) != 0) {
      ++next_selector;
    }
    assign_host_eh_selector(key_by_selector,
                            selector_by_key,
                            next_selector,
                            key,
                            function_name);
    ++next_selector;
  };

  for(map<string, vector<mir::HostEhClause> >::const_iterator it =
          clauses_by_landingpad.begin();
      it != clauses_by_landingpad.end();
      ++it) {
    for(size_t ci = 0; ci < it->second.size(); ++ci) {
      const mir::HostEhClause & clause = it->second[ci];
      if(clause.kind == mir::HostEhClause::HC_CLEANUP) {
        continue;
      }
      if(clause.kind == mir::HostEhClause::HC_FILTER) {
        for(size_t ti = 0; ti < clause.filter_type_symbols.size(); ++ti) {
          assign_fallback_selector(clause.filter_type_symbols[ti]);
        }
        continue;
      }
      if(clause.selector == 0) {
        assign_fallback_selector(host_eh_clause_key(clause));
      }
    }
  }

  long long max_selector = 0;
  for(map<long long, string>::const_iterator it = key_by_selector.begin();
      it != key_by_selector.end();
      ++it) {
    max_selector = max(max_selector, it->first);
  }

  HostEhTypeTablePlan plan;
  plan.type_entries.assign(static_cast<size_t>(max_selector), string());
  for(map<long long, string>::const_iterator it = key_by_selector.begin();
      it != key_by_selector.end();
      ++it) {
    plan.type_entries[static_cast<size_t>(max_selector - it->first)] = it->second;
  }
  plan.filter_index_by_symbol = selector_by_key;
  return plan;
}

long long callee_saved_slot_offset(const mir::Function & function,
                                   size_t index)
{
  return -static_cast<long long>(function_frame_bytes(function) + (index + 1) * 8);
}

uint32_t macho_unwind_reg_code(X64Register reg)
{
  switch(reg) {
    case XR_RBX: return 1U;
    case XR_R12: return 2U;
    case XR_R13: return 3U;
    case XR_R14: return 4U;
    case XR_R15: return 5U;
    case XR_RBP: return 6U;
    default:
      throw logic_error("unsupported compact-unwind register");
  }
}

uint32_t dwarf_reg_for_x64(X64Register reg)
{
  switch(reg) {
    case XR_RAX: return 0U;
    case XR_RDX: return 1U;
    case XR_RCX: return 2U;
    case XR_RBX: return 3U;
    case XR_RSI: return 4U;
    case XR_RDI: return 5U;
    case XR_RBP: return 6U;
    case XR_RSP: return 7U;
    case XR_R8: return 8U;
    case XR_R9: return 9U;
    case XR_R10: return 10U;
    case XR_R11: return 11U;
    case XR_R12: return 12U;
    case XR_R13: return 13U;
    case XR_R14: return 14U;
    case XR_R15: return 15U;
    default:
      throw logic_error("unsupported DWARF register");
  }
}

uint32_t compact_unwind_encoding(const mir::Function & function,
                                 bool has_lsda)
{
  uint32_t encoding = MACHO_UNWIND_X86_64_MODE_RBP_FRAME;
  if(has_lsda) {
    encoding |= MACHO_UNWIND_HAS_LSDA;
  }
  uint32_t farthest_saved_offset_words = 0;
  if(!function.callee_saved_regs.empty()) {
    const long long farthest_saved_offset =
        -callee_saved_slot_offset(function, function.callee_saved_regs.size() - 1);
    // Mach-O compact unwind stores the farthest saved-register offset in
    // 8-bit words. Larger frames must fall back to DWARF unwind info.
    if(farthest_saved_offset / 8 > 0xFF) {
      return 0;
    }
    farthest_saved_offset_words =
        static_cast<uint32_t>(farthest_saved_offset / 8);
  }
  encoding |= (farthest_saved_offset_words & 0xFFU) << 16;
  for(size_t i = 0; i < function.callee_saved_regs.size() && i < 5; ++i) {
    const X64Register reg =
        function.callee_saved_regs[function.callee_saved_regs.size() - 1 - i];
    encoding |= macho_unwind_reg_code(reg) << (i * 3);
  }
  return encoding;
}

void append_u32(vector<unsigned char> & out, uint32_t value)
{
  out.push_back(static_cast<unsigned char>(value & 0xFFU));
  out.push_back(static_cast<unsigned char>((value >> 8) & 0xFFU));
  out.push_back(static_cast<unsigned char>((value >> 16) & 0xFFU));
  out.push_back(static_cast<unsigned char>((value >> 24) & 0xFFU));
}

void append_u64(vector<unsigned char> & out, uint64_t value)
{
  for(size_t i = 0; i < 8; ++i) {
    out.push_back(static_cast<unsigned char>((value >> (i * 8)) & 0xFFU));
  }
}

void overwrite_u32(vector<unsigned char> & out, size_t offset, uint32_t value)
{
  if(offset + 4 > out.size()) {
    throw logic_error("invalid u32 patch offset");
  }
  for(size_t i = 0; i < 4; ++i) {
    out[offset + i] = static_cast<unsigned char>((value >> (i * 8)) & 0xFFU);
  }
}

void append_uleb128(vector<unsigned char> & out, uint64_t value)
{
  do {
    unsigned char byte = static_cast<unsigned char>(value & 0x7FU);
    value >>= 7;
    if(value != 0) {
      byte |= 0x80U;
    }
    out.push_back(byte);
  } while(value != 0);
}

void append_sleb128(vector<unsigned char> & out, long long value)
{
  bool more = true;
  while(more) {
    unsigned char byte = static_cast<unsigned char>(value & 0x7F);
    const bool sign_bit = (byte & 0x40U) != 0;
    value >>= 7;
    more = !((value == 0 && !sign_bit) || (value == -1 && sign_bit));
    if(more) {
      byte |= 0x80U;
    }
    out.push_back(byte);
  }
}

void append_cfi_advance_loc(vector<unsigned char> & out, size_t delta)
{
  if(delta == 0) {
    return;
  }
  if(delta <= 0x3F) {
    out.push_back(static_cast<unsigned char>(0x40U | delta));
  } else if(delta <= 0xFF) {
    out.push_back(0x02);
    out.push_back(static_cast<unsigned char>(delta));
  } else if(delta <= 0xFFFF) {
    out.push_back(0x03);
    out.push_back(static_cast<unsigned char>(delta & 0xFFU));
    out.push_back(static_cast<unsigned char>((delta >> 8) & 0xFFU));
  } else {
    out.push_back(0x04);
    append_u32(out, static_cast<uint32_t>(delta));
  }
}

void append_cfi_def_cfa_offset(vector<unsigned char> & out, uint64_t offset)
{
  out.push_back(0x0E);
  append_uleb128(out, offset);
}

void append_cfi_def_cfa_register(vector<unsigned char> & out, uint64_t reg)
{
  out.push_back(0x0D);
  append_uleb128(out, reg);
}

void append_cfi_offset(vector<unsigned char> & out, uint64_t reg, uint64_t offset_units)
{
  if(reg <= 0x3F) {
    out.push_back(static_cast<unsigned char>(0x80U | reg));
  } else {
    out.push_back(0x05);
    append_uleb128(out, reg);
  }
  append_uleb128(out, offset_units);
}

size_t frame_memory_instruction_size(long long offset)
{
  return offset >= -128 && offset <= 127 ? 4 : 7;
}

void append_common_cfi(vector<unsigned char> & out)
{
  out.push_back(0x0C);
  out.push_back(0x07);
  out.push_back(0x08);
  out.push_back(0x90);
  out.push_back(0x01);
}

void append_function_fde_cfi(vector<unsigned char> & out,
                             const mir::Function & function)
{
  const uint64_t rbp = dwarf_reg_for_x64(XR_RBP);

  if(function.host_eh_enabled) {
    append_cfi_advance_loc(out, 1);
    append_cfi_def_cfa_offset(out, 16);
    append_cfi_offset(out, rbp, 2);

    append_cfi_advance_loc(out, 3);
    append_cfi_def_cfa_register(out, rbp);
  } else {
    append_cfi_advance_loc(out, 7);
    append_cfi_def_cfa_offset(out, 16);

    append_cfi_advance_loc(out, 4);
    append_cfi_offset(out, rbp, 2);

    append_cfi_advance_loc(out, 3);
    append_cfi_def_cfa_register(out, rbp);
  }

  if(function.stack_size != 0) {
    append_cfi_advance_loc(out, 7);
  }

  for(size_t i = 0; i < function.callee_saved_regs.size(); ++i) {
    const long long slot_offset = callee_saved_slot_offset(function, i);
    const size_t store_size = frame_memory_instruction_size(slot_offset);
    append_cfi_advance_loc(out, store_size);
    append_cfi_offset(out,
                      dwarf_reg_for_x64(function.callee_saved_regs[i]),
                      static_cast<uint64_t>((16 - slot_offset) / 8));
  }
}

string extra_section_key(const string & segment_name, const string & section_name)
{
  return segment_name + "," + section_name;
}

size_t align_up(size_t value, size_t alignment)
{
  if(alignment == 0) {
    return value;
  }
  const size_t mask = alignment - 1;
  return (value + mask) & ~mask;
}

void append_extra_abs64_relocation(mobj::ExtraSection & section,
                                   size_t offset,
                                   mobj::ExtraRelocation::TargetKind target_kind,
                                   const string & symbol,
                                   const string & extra_section_name,
                                   long long addend)
{
  mobj::ExtraRelocation reloc;
  reloc.kind = mobj::ExtraRelocation::RK_ABS64;
  reloc.offset = offset;
  reloc.target_kind = target_kind;
  reloc.symbol = symbol;
  reloc.target_extra_section = extra_section_name;
  reloc.addend = addend;
  section.relocations.push_back(reloc);
}

void append_extra_indirect_rel32_relocation(mobj::ExtraSection & section,
                                            size_t offset,
                                            const string & symbol)
{
  mobj::ExtraRelocation reloc;
  reloc.kind = mobj::ExtraRelocation::RK_INDIRECT_REL32;
  reloc.offset = offset;
  reloc.target_kind = mobj::ExtraRelocation::TK_SYMBOL;
  reloc.symbol = symbol;
  section.relocations.push_back(reloc);
}

void append_extra_pcrel32_relocation(mobj::ExtraSection & section,
                                     size_t offset,
                                     const string & symbol,
                                     long long addend)
{
  mobj::ExtraRelocation reloc;
  reloc.kind = mobj::ExtraRelocation::RK_PCREL32;
  reloc.offset = offset;
  reloc.target_kind = mobj::ExtraRelocation::TK_SYMBOL;
  reloc.symbol = symbol;
  reloc.addend = addend;
  section.relocations.push_back(reloc);
}

void append_extra_pcrel32_code_relocation(mobj::ExtraSection & section,
                                          size_t offset,
                                          long long addend)
{
  mobj::ExtraRelocation reloc;
  reloc.kind = mobj::ExtraRelocation::RK_PCREL32;
  reloc.offset = offset;
  reloc.target_kind = mobj::ExtraRelocation::TK_CODE;
  reloc.addend = addend;
  section.relocations.push_back(reloc);
}

void append_extra_pcrel32_data_relocation(mobj::ExtraSection & section,
                                          size_t offset,
                                          long long addend)
{
  mobj::ExtraRelocation reloc;
  reloc.kind = mobj::ExtraRelocation::RK_PCREL32;
  reloc.offset = offset;
  reloc.target_kind = mobj::ExtraRelocation::TK_DATA;
  reloc.addend = addend;
  section.relocations.push_back(reloc);
}

void append_extra_pcrel32_extra_relocation(mobj::ExtraSection & section,
                                           size_t offset,
                                           const string & extra_section_name,
                                           long long addend)
{
  mobj::ExtraRelocation reloc;
  reloc.kind = mobj::ExtraRelocation::RK_PCREL32;
  reloc.offset = offset;
  reloc.target_kind = mobj::ExtraRelocation::TK_EXTRA;
  reloc.target_extra_section = extra_section_name;
  reloc.addend = addend;
  section.relocations.push_back(reloc);
}

void append_macho_host_eh_sections(const mir::Program & program,
                                   const HostEhObjectLayout & layout,
                                   const vector<HostEhFunctionInfo> & host_eh_functions,
                                   mobj::ObjectFile & object)
{
  if(program.target != "macos") {
    return;
  }

  const string lsda_key = extra_section_key("__TEXT", "__gcc_except_tab");
  const string compact_key = extra_section_key("__LD", "__compact_unwind");

  mobj::ExtraSection lsda_section;
  lsda_section.segment_name = "__TEXT";
  lsda_section.section_name = "__gcc_except_tab";
  lsda_section.macho_flags = 0;
  lsda_section.macho_align_pow2 = 0;

  mobj::ExtraSection compact_section;
  compact_section.segment_name = "__LD";
  compact_section.section_name = "__compact_unwind";
  compact_section.macho_flags = MACHO_S_ATTR_DEBUG;
  compact_section.macho_align_pow2 = 3;

  map<string, const mir::Function *> function_by_name;
  for(size_t i = 0; i < program.functions.size(); ++i) {
    function_by_name[program.functions[i].name] = &program.functions[i];
  }
  map<string, const HostEhFunctionInfo *> host_eh_by_name;
  for(size_t i = 0; i < host_eh_functions.size(); ++i) {
    host_eh_by_name[host_eh_functions[i].function_name] = &host_eh_functions[i];
  }
  map<string, size_t> lsda_offset_by_function;
  map<string, string> lsda_symbol_by_function;
  set<string> dwarf_unwind_function_names;

  for(size_t i = 0; i < host_eh_functions.size(); ++i) {
    map<string, const mir::Function *>::const_iterator function_it =
        function_by_name.find(host_eh_functions[i].function_name);
    if(function_it == function_by_name.end()) {
      throw logic_error("host EH function metadata missing " +
                        host_eh_functions[i].function_name);
    }
    const mir::Function & function = *function_it->second;
    const size_t lsda_offset = lsda_section.bytes.size();
    lsda_offset_by_function[function.name] = lsda_offset;
    lsda_symbol_by_function[function.name] = "cppgm_lsda_" + to_string(i);
    const HostEhTypeTablePlan type_table_plan =
        plan_host_eh_type_table(function.name, function.host_eh_clauses);
    const vector<string> & type_entries = type_table_plan.type_entries;
    const map<string, long long> & filter_index_by_symbol =
        type_table_plan.filter_index_by_symbol;

    map<vector<long long>, long long> exception_spec_value_by_types;
    vector<unsigned char> exception_spec_table;
    map<string, uint64_t> action_value_by_landingpad;
    vector<unsigned char> action_table;
    for(map<string, vector<mir::HostEhClause> >::const_iterator it =
            function.host_eh_clauses.begin();
        it != function.host_eh_clauses.end();
        ++it) {
      if(it->second.empty()) {
        continue;
      }
      size_t first_record_start = 0;
      bool have_first_record = false;
      size_t next_record_start = 0;
      bool have_next_record = false;
      for(size_t ci = it->second.size(); ci-- > 0;) {
        const mir::HostEhClause & clause = it->second[ci];
        long long filter_value = 0;
        if(clause.kind == mir::HostEhClause::HC_CLEANUP) {
          filter_value = 0;
        } else if(clause.kind == mir::HostEhClause::HC_FILTER) {
          vector<long long> filter_indices;
          for(size_t fi2 = 0; fi2 < clause.filter_type_symbols.size(); ++fi2) {
            map<string, long long>::const_iterator filter_found =
                filter_index_by_symbol.find(clause.filter_type_symbols[fi2]);
            if(filter_found == filter_index_by_symbol.end()) {
              throw logic_error("missing host EH filter type entry for " + function.name);
            }
            filter_indices.push_back(filter_found->second);
          }
          map<vector<long long>, long long>::const_iterator existing =
              exception_spec_value_by_types.find(filter_indices);
          if(existing != exception_spec_value_by_types.end()) {
            filter_value = existing->second;
          } else {
            const size_t start_offset = exception_spec_table.size();
            for(size_t fi2 = 0; fi2 < filter_indices.size(); ++fi2) {
              append_uleb128(exception_spec_table,
                             static_cast<uint64_t>(filter_indices[fi2]));
            }
            append_uleb128(exception_spec_table, 0);
            filter_value = -static_cast<long long>(start_offset + 1);
            exception_spec_value_by_types[filter_indices] = filter_value;
          }
        } else {
          if(clause.selector > 0) {
            filter_value = clause.selector;
          } else {
            const string filter_key = host_eh_clause_key(clause);
            map<string, long long>::const_iterator filter_found =
                filter_index_by_symbol.find(filter_key);
            if(filter_found == filter_index_by_symbol.end()) {
              throw logic_error("missing host EH type table entry for " + function.name);
            }
            filter_value = filter_found->second;
          }
        }
        const size_t record_start = action_table.size();
        append_sleb128(action_table, filter_value);
        const size_t next_field_offset = action_table.size();
        const long long next_offset =
            have_next_record ? static_cast<long long>(next_record_start) -
                                   static_cast<long long>(next_field_offset)
                             : 0;
        append_sleb128(action_table, next_offset);
        first_record_start = record_start;
        have_first_record = true;
        next_record_start = record_start;
        have_next_record = true;
      }
      if(have_first_record) {
        action_value_by_landingpad[it->first] =
            static_cast<uint64_t>(first_record_start + 1);
      }
    }

    const vector<HostEhCallSite> macho_call_sites =
        add_no_landingpad_gaps(host_eh_functions[i].call_sites,
                               host_eh_functions[i].function_size);
    vector<unsigned char> call_site_table;
    for(size_t ci = 0; ci < macho_call_sites.size(); ++ci) {
      const HostEhCallSite & call_site = macho_call_sites[ci];
      append_uleb128(call_site_table, call_site.start);
      append_uleb128(call_site_table, call_site.length);
      append_uleb128(call_site_table, call_site.landingpad_offset);
      map<string, uint64_t>::const_iterator action =
          action_value_by_landingpad.find(call_site.landingpad_symbol);
      append_uleb128(call_site_table, action == action_value_by_landingpad.end() ? 0
                                                                                 : action->second);
    }

    vector<unsigned char> type_table;
    vector<pair<size_t, string> > type_table_relocs;
    for(size_t ti = 0; ti < type_entries.size(); ++ti) {
      const size_t entry_offset = type_table.size();
      append_u32(type_table, type_entries[ti].empty() ? 0U : 4U);
      if(!type_entries[ti].empty()) {
        type_table_relocs.push_back(make_pair(entry_offset, type_entries[ti]));
      }
    }

    vector<unsigned char> lsda_rest;
    lsda_rest.push_back(0x01);
    append_uleb128(lsda_rest, call_site_table.size());
    lsda_rest.insert(lsda_rest.end(), call_site_table.begin(), call_site_table.end());
    lsda_rest.insert(lsda_rest.end(), action_table.begin(), action_table.end());
    if(!type_entries.empty() || !exception_spec_table.empty()) {
      const size_t lsda_header_size = 3;
      const size_t pad =
          (4 - ((lsda_header_size + lsda_rest.size()) % 4)) % 4;
      lsda_rest.insert(lsda_rest.end(), pad, 0x00);
    }
    const size_t type_table_offset_in_rest = lsda_rest.size();
    lsda_rest.insert(lsda_rest.end(), type_table.begin(), type_table.end());
    const size_t ttbase_offset_in_rest = lsda_rest.size();
    lsda_rest.insert(lsda_rest.end(),
                     exception_spec_table.begin(),
                     exception_spec_table.end());

    lsda_section.bytes.push_back(0xFF);
    lsda_section.bytes.push_back((type_entries.empty() && exception_spec_table.empty()) ? 0xFF
                                                                                        : 0x9B);
    if(!type_entries.empty() || !exception_spec_table.empty()) {
      append_uleb128(lsda_section.bytes, ttbase_offset_in_rest);
    }
    const size_t rest_offset = lsda_section.bytes.size();
    lsda_section.bytes.insert(lsda_section.bytes.end(), lsda_rest.begin(), lsda_rest.end());
    for(size_t ti = 0; ti < type_table_relocs.size(); ++ti) {
      append_extra_indirect_rel32_relocation(lsda_section,
                                             rest_offset + type_table_offset_in_rest +
                                                 type_table_relocs[ti].first,
                                             type_table_relocs[ti].second);
    }

    mobj::Symbol lsda_symbol;
    lsda_symbol.binding = mobj::Symbol::SB_LOCAL;
    lsda_symbol.section = mobj::Symbol::SS_EXTRA;
    lsda_symbol.name = lsda_symbol_by_function.find(function.name)->second;
    lsda_symbol.offset = lsda_offset;
    lsda_symbol.extra_section = lsda_key;
    object.symbols.push_back(lsda_symbol);
    while(lsda_section.bytes.size() % 4 != 0) {
      lsda_section.bytes.push_back(0x00);
    }
  }

  for(size_t i = 0; i < program.functions.size(); ++i) {
    const mir::Function & function = program.functions[i];
    map<string, size_t>::const_iterator function_offset =
        layout.function_offsets.find(function.name);
    map<string, HostEhFunctionLayout>::const_iterator function_layout =
        layout.function_layouts.find(function.name);
    if(function_offset == layout.function_offsets.end() ||
       function_layout == layout.function_layouts.end()) {
      throw logic_error("missing function layout for compact unwind " + function.name);
    }

    const bool has_lsda = host_eh_by_name.count(function.name) != 0;
    const uint32_t unwind_encoding = compact_unwind_encoding(function, has_lsda);
    if(unwind_encoding == 0) {
      dwarf_unwind_function_names.insert(function.name);
      continue;
    }

    const size_t compact_offset = compact_section.bytes.size();
    append_u64(compact_section.bytes,
               static_cast<uint64_t>(function_offset->second));
    append_u32(compact_section.bytes,
               static_cast<uint32_t>(function_layout->second.size));
    append_u32(compact_section.bytes, unwind_encoding);
    append_u64(compact_section.bytes, 0);
    append_u64(compact_section.bytes,
               has_lsda ? static_cast<uint64_t>(lsda_offset_by_function.find(function.name)->second)
                        : 0);

    append_extra_abs64_relocation(compact_section,
                                  compact_offset,
                                  mobj::ExtraRelocation::TK_CODE,
                                  string(),
                                  string(),
                                  function_offset->second);
    if(has_lsda) {
      append_extra_abs64_relocation(compact_section,
                                    compact_offset + 16,
                                    mobj::ExtraRelocation::TK_SYMBOL,
                                    symbol_linkage::internal_symbol_from_name(
                                        "__external_runtime::__gxx_personality_v0"),
                                    string(),
                                    0);
      append_extra_abs64_relocation(compact_section,
                                    compact_offset + 24,
                                    mobj::ExtraRelocation::TK_EXTRA,
                                    string(),
                                    lsda_key,
                                    static_cast<long long>(
                                        lsda_offset_by_function.find(function.name)->second));
    }
  }

  bool any_host_eh = false;
  for(size_t i = 0; i < program.functions.size(); ++i) {
    if(program.functions[i].host_eh_enabled) {
      any_host_eh = true;
      break;
    }
  }

  if(any_host_eh || !dwarf_unwind_function_names.empty()) {
    const unsigned char kPcRelSData4 = 0x1B;

    mobj::ExtraSection eh_frame_section;
    eh_frame_section.segment_name = "__TEXT";
    eh_frame_section.section_name = "__eh_frame";
    eh_frame_section.macho_flags = 0;
    eh_frame_section.macho_align_pow2 = 3;

    const auto finish_eh_frame_record =
        [&](size_t length_offset) {
          while(eh_frame_section.bytes.size() % 4 != 0) {
            eh_frame_section.bytes.push_back(0x00);
          }
          overwrite_u32(eh_frame_section.bytes,
                        length_offset,
                        static_cast<uint32_t>(eh_frame_section.bytes.size() -
                                              length_offset - 4));
        };
    const auto append_macho_fde =
        [&](const mir::Function & function, size_t cie_start, bool has_lsda) {
          map<string, HostEhFunctionLayout>::const_iterator layout_it =
              layout.function_layouts.find(function.name);
          if(layout_it == layout.function_layouts.end()) {
            throw logic_error("missing Mach-O EH layout for " + function.name);
          }
          const size_t fde_start = eh_frame_section.bytes.size();
          const size_t fde_length_offset = eh_frame_section.bytes.size();
          append_u32(eh_frame_section.bytes, 0);
          append_u32(eh_frame_section.bytes,
                     static_cast<uint32_t>(fde_start + 4 - cie_start));
          const size_t function_pointer_offset = eh_frame_section.bytes.size();
          append_u32(eh_frame_section.bytes, 0);
          append_u32(eh_frame_section.bytes, static_cast<uint32_t>(layout_it->second.size));
          if(has_lsda) {
            append_uleb128(eh_frame_section.bytes, 4);
            const size_t lsda_pointer_offset = eh_frame_section.bytes.size();
            append_u32(eh_frame_section.bytes, 0);
            map<string, size_t>::const_iterator lsda_offset =
                lsda_offset_by_function.find(function.name);
            if(lsda_offset == lsda_offset_by_function.end()) {
              throw logic_error("missing Mach-O LSDA offset for " + function.name);
            }
            append_extra_pcrel32_extra_relocation(eh_frame_section,
                                                  lsda_pointer_offset,
                                                  lsda_key,
                                                  static_cast<long long>(lsda_offset->second));
          } else {
            append_uleb128(eh_frame_section.bytes, 0);
          }
          append_function_fde_cfi(eh_frame_section.bytes, function);
          finish_eh_frame_record(fde_length_offset);
          map<string, size_t>::const_iterator function_offset =
              layout.function_offsets.find(function.name);
          if(function_offset == layout.function_offsets.end()) {
            throw logic_error("missing Mach-O EH function offset for " + function.name);
          }
          append_extra_pcrel32_code_relocation(eh_frame_section,
                                               function_pointer_offset,
                                               static_cast<long long>(function_offset->second));
        };

    const size_t base_cie_start = eh_frame_section.bytes.size();
    const size_t base_cie_length_offset = eh_frame_section.bytes.size();
    append_u32(eh_frame_section.bytes, 0);
    append_u32(eh_frame_section.bytes, 0);
    eh_frame_section.bytes.push_back(0x01);
    eh_frame_section.bytes.push_back('z');
    eh_frame_section.bytes.push_back('R');
    eh_frame_section.bytes.push_back(0x00);
    append_uleb128(eh_frame_section.bytes, 1);
    append_sleb128(eh_frame_section.bytes, -8);
    append_uleb128(eh_frame_section.bytes, 16);
    append_uleb128(eh_frame_section.bytes, 1);
    eh_frame_section.bytes.push_back(kPcRelSData4);
    append_common_cfi(eh_frame_section.bytes);
    finish_eh_frame_record(base_cie_length_offset);

    for(size_t i = 0; i < program.functions.size(); ++i) {
      const mir::Function & function = program.functions[i];
      if(!function.host_eh_enabled &&
         dwarf_unwind_function_names.count(function.name) == 0) {
        continue;
      }
      if(host_eh_by_name.find(function.name) != host_eh_by_name.end()) {
        continue;
      }
      append_macho_fde(function, base_cie_start, false);
    }

    if(!host_eh_functions.empty()) {
      const size_t lsda_cie_start = eh_frame_section.bytes.size();
      const size_t lsda_cie_length_offset = eh_frame_section.bytes.size();
      append_u32(eh_frame_section.bytes, 0);
      append_u32(eh_frame_section.bytes, 0);
      eh_frame_section.bytes.push_back(0x01);
      eh_frame_section.bytes.push_back('z');
      eh_frame_section.bytes.push_back('P');
      eh_frame_section.bytes.push_back('L');
      eh_frame_section.bytes.push_back('R');
      eh_frame_section.bytes.push_back(0x00);
      append_uleb128(eh_frame_section.bytes, 1);
      append_sleb128(eh_frame_section.bytes, -8);
      append_uleb128(eh_frame_section.bytes, 16);
      append_uleb128(eh_frame_section.bytes, 7);
      eh_frame_section.bytes.push_back(0x9B);
      const size_t personality_offset = eh_frame_section.bytes.size();
      append_u32(eh_frame_section.bytes, 0);
      eh_frame_section.bytes.push_back(kPcRelSData4);
      eh_frame_section.bytes.push_back(kPcRelSData4);
      append_common_cfi(eh_frame_section.bytes);
      finish_eh_frame_record(lsda_cie_length_offset);
      append_extra_indirect_rel32_relocation(
          eh_frame_section,
          personality_offset,
          symbol_linkage::internal_symbol_from_name("__external_runtime::__gxx_personality_v0"));

      for(size_t i = 0; i < host_eh_functions.size(); ++i) {
        map<string, const mir::Function *>::const_iterator function_it =
            function_by_name.find(host_eh_functions[i].function_name);
        if(function_it == function_by_name.end()) {
          throw logic_error("Mach-O EH function metadata missing " +
                            host_eh_functions[i].function_name);
        }
        append_macho_fde(*function_it->second, lsda_cie_start, true);
      }
    }

    object.extra_sections.push_back(eh_frame_section);
  }

  if(!lsda_section.bytes.empty()) {
    object.extra_sections.push_back(lsda_section);
  }
  object.extra_sections.push_back(compact_section);
}

void append_linux_host_eh_sections(const mir::Program & program,
                                   const HostEhObjectLayout & layout,
                                   const vector<HostEhFunctionInfo> & host_eh_functions,
                                   mobj::ObjectFile & object)
{
  if(program.target != "linux") {
    return;
  }
  if(program.functions.empty()) {
    return;
  }

  const string extra_segment = "__ELF";
  const string personality_ref_key =
      extra_section_key(extra_segment, ".data.DW.ref.__gxx_personality_v0");
  const string lsda_key = extra_section_key(extra_segment, ".gcc_except_table");

  mobj::ExtraSection lsda_section;
  lsda_section.segment_name = extra_segment;
  lsda_section.section_name = ".gcc_except_table";
  lsda_section.macho_align_pow2 = 2;

  map<string, size_t> typeinfo_ref_offset_by_type_symbol;
  const auto ensure_typeinfo_ref_offset =
      [&](const string & type_symbol) -> size_t
      {
        map<string, size_t>::const_iterator found =
            typeinfo_ref_offset_by_type_symbol.find(type_symbol);
        if(found != typeinfo_ref_offset_by_type_symbol.end()) {
          return found->second;
        }
        const size_t aligned_data_size = align_up(object.data.size(), 8);
        if(aligned_data_size > object.data.size()) {
          object.data.insert(object.data.end(), aligned_data_size - object.data.size(), 0x00);
        }
        const size_t offset = object.data.size();
        append_u64(object.data, 0);
        mobj::Relocation reloc;
        reloc.section = mobj::Symbol::SS_DATA;
        reloc.offset = offset;
        reloc.kind = mobj::Relocation::RK_ABS64;
        reloc.symbol = type_symbol;
        object.relocations.push_back(reloc);
        typeinfo_ref_offset_by_type_symbol[type_symbol] = offset;
        return offset;
      };

  map<string, const mir::Function *> function_by_name;
  for(size_t i = 0; i < program.functions.size(); ++i) {
    function_by_name[program.functions[i].name] = &program.functions[i];
  }
  map<string, const HostEhFunctionInfo *> host_eh_by_name;
  for(size_t i = 0; i < host_eh_functions.size(); ++i) {
    host_eh_by_name[host_eh_functions[i].function_name] = &host_eh_functions[i];
  }
  const bool have_lsda_functions = !host_eh_by_name.empty();
  mobj::ExtraSection personality_ref_section;
  mobj::Symbol personality_ref_symbol;
  if(have_lsda_functions) {
    personality_ref_section.segment_name = extra_segment;
    personality_ref_section.section_name = ".data.DW.ref.__gxx_personality_v0";
    personality_ref_section.macho_align_pow2 = 3;
    append_u64(personality_ref_section.bytes, 0);
    append_extra_abs64_relocation(
        personality_ref_section,
        0,
        mobj::ExtraRelocation::TK_SYMBOL,
        symbol_linkage::internal_symbol_from_name("__external_runtime::__gxx_personality_v0"),
        string(),
        0);
    personality_ref_symbol.binding = mobj::Symbol::SB_LOCAL;
    personality_ref_symbol.section = mobj::Symbol::SS_EXTRA;
    personality_ref_symbol.name = "DW.ref.__gxx_personality_v0";
    personality_ref_symbol.offset = 0;
    personality_ref_symbol.extra_section = personality_ref_key;
    object.symbols.push_back(personality_ref_symbol);
  }
  map<string, string> lsda_symbol_by_function;
  map<string, size_t> lsda_offset_by_function;
  for(size_t i = 0; i < host_eh_functions.size(); ++i) {
    map<string, const mir::Function *>::const_iterator function_it =
        function_by_name.find(host_eh_functions[i].function_name);
    if(function_it == function_by_name.end()) {
      throw logic_error("host EH function metadata missing " +
                        host_eh_functions[i].function_name);
    }
    const mir::Function & function = *function_it->second;
    const size_t lsda_offset = lsda_section.bytes.size();
    lsda_symbol_by_function[function.name] = "cppgm_lsda_" + to_string(i);
    lsda_offset_by_function[function.name] = lsda_offset;

    const HostEhTypeTablePlan type_table_plan =
        plan_host_eh_type_table(function.name, function.host_eh_clauses);
    const vector<string> & type_entries = type_table_plan.type_entries;
    const map<string, long long> & filter_index_by_symbol =
        type_table_plan.filter_index_by_symbol;

    map<vector<long long>, long long> exception_spec_value_by_types;
    vector<unsigned char> exception_spec_table;
    map<string, uint64_t> action_value_by_landingpad;
    vector<unsigned char> action_table;
    for(map<string, vector<mir::HostEhClause> >::const_iterator it =
            function.host_eh_clauses.begin();
        it != function.host_eh_clauses.end();
        ++it) {
      if(it->second.empty()) {
        continue;
      }
      size_t first_record_start = 0;
      bool have_first_record = false;
      size_t next_record_start = 0;
      bool have_next_record = false;
      for(size_t ci = it->second.size(); ci-- > 0;) {
        const mir::HostEhClause & clause = it->second[ci];
        long long filter_value = 0;
        if(clause.kind == mir::HostEhClause::HC_CLEANUP) {
          filter_value = 0;
        } else if(clause.kind == mir::HostEhClause::HC_FILTER) {
          vector<long long> filter_indices;
          for(size_t fi2 = 0; fi2 < clause.filter_type_symbols.size(); ++fi2) {
            map<string, long long>::const_iterator filter_found =
                filter_index_by_symbol.find(clause.filter_type_symbols[fi2]);
            if(filter_found == filter_index_by_symbol.end()) {
              throw logic_error("missing host EH filter type entry for " + function.name);
            }
            filter_indices.push_back(filter_found->second);
          }
          map<vector<long long>, long long>::const_iterator existing =
              exception_spec_value_by_types.find(filter_indices);
          if(existing != exception_spec_value_by_types.end()) {
            filter_value = existing->second;
          } else {
            const size_t start_offset = exception_spec_table.size();
            for(size_t fi2 = 0; fi2 < filter_indices.size(); ++fi2) {
              append_uleb128(exception_spec_table,
                             static_cast<uint64_t>(filter_indices[fi2]));
            }
            append_uleb128(exception_spec_table, 0);
            filter_value = -static_cast<long long>(start_offset + 1);
            exception_spec_value_by_types[filter_indices] = filter_value;
          }
        } else {
          if(clause.selector > 0) {
            filter_value = clause.selector;
          } else {
            const string filter_key = host_eh_clause_key(clause);
            map<string, long long>::const_iterator filter_found =
                filter_index_by_symbol.find(filter_key);
            if(filter_found == filter_index_by_symbol.end()) {
              throw logic_error("missing host EH type table entry for " + function.name);
            }
            filter_value = filter_found->second;
          }
        }
        const size_t record_start = action_table.size();
        append_sleb128(action_table, filter_value);
        const size_t next_field_offset = action_table.size();
        const long long next_offset =
            have_next_record ? static_cast<long long>(next_record_start) -
                                   static_cast<long long>(next_field_offset)
                             : 0;
        append_sleb128(action_table, next_offset);
        first_record_start = record_start;
        have_first_record = true;
        next_record_start = record_start;
        have_next_record = true;
      }
      if(have_first_record) {
        action_value_by_landingpad[it->first] =
            static_cast<uint64_t>(first_record_start + 1);
      }
    }

    map<string, HostEhFunctionLayout>::const_iterator function_layout =
        layout.function_layouts.find(function.name);
    if(function_layout == layout.function_layouts.end()) {
      throw logic_error("missing Linux EH function layout for " + function.name);
    }
    set<string> landingpad_labels;
    for(size_t bi = 0; bi < function.blocks.size(); ++bi) {
      for(size_t ii = 0; ii < function.blocks[bi].instructions.size(); ++ii) {
        const mir::Instruction & inst = function.blocks[bi].instructions[ii];
        if(inst.opcode == mir::Instruction::MI_EH_PUSH && !inst.operands.empty()) {
          landingpad_labels.insert(inst.operands[0].text);
        }
      }
    }
    vector<pair<size_t, size_t> > landingpad_ranges;
    for(size_t bi = 0; bi < function.blocks.size(); ++bi) {
      const string label_symbol = function.name + "$" + function.blocks[bi].label;
      if(landingpad_labels.count(label_symbol) == 0) {
        continue;
      }
      map<string, size_t>::const_iterator block_offset =
          function_layout->second.block_offsets.find(function.blocks[bi].label);
      if(block_offset == function_layout->second.block_offsets.end()) {
        throw logic_error("missing Linux EH landingpad block offset for " +
                          function.name + "$" + function.blocks[bi].label);
      }
      const size_t start = block_offset->second;
      const size_t end =
          bi + 1 < function.blocks.size() ?
              function_layout->second.block_offsets.find(function.blocks[bi + 1].label)->second :
              host_eh_functions[i].function_size;
      landingpad_ranges.push_back(make_pair(start, end));
    }
    sort(landingpad_ranges.begin(), landingpad_ranges.end());

    vector<HostEhCallSite> linux_call_sites;
    for(size_t ci = 0; ci < host_eh_functions[i].call_sites.size(); ++ci) {
      const HostEhCallSite & call_site = host_eh_functions[i].call_sites[ci];
      if(call_site.landingpad_offset == 0 && call_site.landingpad_symbol.empty()) {
        linux_call_sites.push_back(call_site);
        continue;
      }
      const size_t call_site_end = call_site.start + call_site.length;
      size_t cursor = call_site.start;
      for(size_t ri = 0; ri < landingpad_ranges.size() && cursor < call_site_end; ++ri) {
        const size_t range_start = landingpad_ranges[ri].first;
        const size_t range_end = landingpad_ranges[ri].second;
        if(range_end <= cursor || range_start >= call_site_end) {
          continue;
        }
        if(range_start > cursor) {
          HostEhCallSite trimmed = call_site;
          trimmed.start = cursor;
          trimmed.length = range_start - cursor;
          if(trimmed.length != 0) {
            linux_call_sites.push_back(trimmed);
          }
        }
        cursor = max(cursor, min(call_site_end, range_end));
      }
      if(cursor < call_site_end) {
        HostEhCallSite trimmed = call_site;
        trimmed.start = cursor;
        trimmed.length = call_site_end - cursor;
        if(trimmed.length != 0) {
          linux_call_sites.push_back(trimmed);
        }
      }
    }
    vector<HostEhCallSite> merged_linux_call_sites;
    for(size_t ci = 0; ci < linux_call_sites.size(); ++ci) {
      if(!merged_linux_call_sites.empty()) {
        HostEhCallSite & last = merged_linux_call_sites.back();
        if(last.start + last.length == linux_call_sites[ci].start &&
           last.landingpad_offset == linux_call_sites[ci].landingpad_offset &&
           last.landingpad_symbol == linux_call_sites[ci].landingpad_symbol) {
          last.length += linux_call_sites[ci].length;
          continue;
        }
      }
      merged_linux_call_sites.push_back(linux_call_sites[ci]);
    }
    linux_call_sites.swap(merged_linux_call_sites);

    const vector<HostEhCallSite> linux_call_sites_with_gaps =
        add_no_landingpad_gaps(linux_call_sites,
                               host_eh_functions[i].function_size);
    vector<unsigned char> call_site_table;
    for(size_t ci = 0; ci < linux_call_sites_with_gaps.size(); ++ci) {
      const HostEhCallSite & call_site = linux_call_sites_with_gaps[ci];
      append_uleb128(call_site_table, call_site.start);
      append_uleb128(call_site_table, call_site.length);
      append_uleb128(call_site_table, call_site.landingpad_offset);
      map<string, uint64_t>::const_iterator action =
          action_value_by_landingpad.find(call_site.landingpad_symbol);
      append_uleb128(call_site_table, action == action_value_by_landingpad.end() ? 0
                                                                                 : action->second);
    }

    const unsigned char kTypeTableEncoding = 0x9B;
    vector<unsigned char> type_table;
    vector<pair<size_t, size_t> > type_table_relocs;
    for(size_t ti = 0; ti < type_entries.size(); ++ti) {
      const size_t entry_offset = type_table.size();
      append_u32(type_table, 0);
      if(!type_entries[ti].empty()) {
        type_table_relocs.push_back(
            make_pair(entry_offset, ensure_typeinfo_ref_offset(type_entries[ti])));
      }
    }

    vector<unsigned char> lsda_rest;
    lsda_rest.push_back(0x01);
    append_uleb128(lsda_rest, call_site_table.size());
    lsda_rest.insert(lsda_rest.end(), call_site_table.begin(), call_site_table.end());
    lsda_rest.insert(lsda_rest.end(), action_table.begin(), action_table.end());
    if(!type_entries.empty() || !exception_spec_table.empty()) {
      const size_t lsda_header_size = 3;
      const size_t pad =
          (4 - ((lsda_header_size + lsda_rest.size()) % 4)) % 4;
      lsda_rest.insert(lsda_rest.end(), pad, 0x00);
    }
    const size_t type_table_offset_in_rest = lsda_rest.size();
    lsda_rest.insert(lsda_rest.end(), type_table.begin(), type_table.end());
    const size_t ttbase_offset_in_rest = lsda_rest.size();
    lsda_rest.insert(lsda_rest.end(),
                     exception_spec_table.begin(),
                     exception_spec_table.end());

    lsda_section.bytes.push_back(0xFF);
    lsda_section.bytes.push_back((type_entries.empty() && exception_spec_table.empty()) ? 0xFF
                                                                                        : kTypeTableEncoding);
    if(!type_entries.empty() || !exception_spec_table.empty()) {
      append_uleb128(lsda_section.bytes, ttbase_offset_in_rest);
    }
    const size_t rest_offset = lsda_section.bytes.size();
    lsda_section.bytes.insert(lsda_section.bytes.end(), lsda_rest.begin(), lsda_rest.end());
    for(size_t ti = 0; ti < type_table_relocs.size(); ++ti) {
      append_extra_pcrel32_data_relocation(
          lsda_section,
          rest_offset + type_table_offset_in_rest + type_table_relocs[ti].first,
          static_cast<long long>(type_table_relocs[ti].second));
    }

    mobj::Symbol lsda_symbol;
    lsda_symbol.binding = mobj::Symbol::SB_LOCAL;
    lsda_symbol.section = mobj::Symbol::SS_EXTRA;
    lsda_symbol.name = lsda_symbol_by_function.find(function.name)->second;
    lsda_symbol.offset = lsda_offset;
    lsda_symbol.extra_section = lsda_key;
    object.symbols.push_back(lsda_symbol);
    while(lsda_section.bytes.size() % 4 != 0) {
      lsda_section.bytes.push_back(0x00);
    }
  }

  const unsigned char kPcRelSData4 = 0x1B;

  mobj::ExtraSection eh_frame_section;
  eh_frame_section.segment_name = extra_segment;
  eh_frame_section.section_name = ".eh_frame";
  eh_frame_section.macho_align_pow2 = 3;

  const auto finish_eh_frame_record =
      [&](size_t length_offset) {
        while(eh_frame_section.bytes.size() % 4 != 0) {
          eh_frame_section.bytes.push_back(0x00);
        }
        overwrite_u32(eh_frame_section.bytes,
                      length_offset,
                      static_cast<uint32_t>(eh_frame_section.bytes.size() -
                                            length_offset - 4));
      };

  const size_t base_cie_start = eh_frame_section.bytes.size();
  const size_t base_cie_length_offset = eh_frame_section.bytes.size();
  append_u32(eh_frame_section.bytes, 0);
  append_u32(eh_frame_section.bytes, 0);
  eh_frame_section.bytes.push_back(0x01);
  eh_frame_section.bytes.push_back('z');
  eh_frame_section.bytes.push_back('R');
  eh_frame_section.bytes.push_back(0x00);
  append_uleb128(eh_frame_section.bytes, 1);
  append_sleb128(eh_frame_section.bytes, -8);
  append_uleb128(eh_frame_section.bytes, 16);
  append_uleb128(eh_frame_section.bytes, 1);
  eh_frame_section.bytes.push_back(kPcRelSData4);
  append_common_cfi(eh_frame_section.bytes);
  finish_eh_frame_record(base_cie_length_offset);

  const auto append_linux_fde =
      [&](const mir::Function & function,
          const size_t cie_start,
          const bool has_lsda) {
        map<string, HostEhFunctionLayout>::const_iterator layout_it =
            layout.function_layouts.find(function.name);
        if(layout_it == layout.function_layouts.end()) {
          throw logic_error("missing Linux EH layout for " + function.name);
        }
        const size_t fde_start = eh_frame_section.bytes.size();
        const size_t fde_length_offset = eh_frame_section.bytes.size();
        append_u32(eh_frame_section.bytes, 0);
        append_u32(eh_frame_section.bytes,
                   static_cast<uint32_t>(fde_start + 4 - cie_start));
        const size_t function_pointer_offset = eh_frame_section.bytes.size();
        append_u32(eh_frame_section.bytes, 0);
        append_u32(eh_frame_section.bytes, static_cast<uint32_t>(layout_it->second.size));
        if(has_lsda) {
          append_uleb128(eh_frame_section.bytes, 4);
          const size_t lsda_pointer_offset = eh_frame_section.bytes.size();
          append_u32(eh_frame_section.bytes, 0);
          map<string, size_t>::const_iterator lsda_offset =
              lsda_offset_by_function.find(function.name);
          if(lsda_offset == lsda_offset_by_function.end()) {
            throw logic_error("missing Linux LSDA offset for " + function.name);
          }
          append_extra_pcrel32_extra_relocation(
              eh_frame_section,
              lsda_pointer_offset,
              lsda_key,
              static_cast<long long>(lsda_offset->second));
        } else {
          append_uleb128(eh_frame_section.bytes, 0);
        }
        append_function_fde_cfi(eh_frame_section.bytes, function);
        finish_eh_frame_record(fde_length_offset);
        map<string, size_t>::const_iterator function_offset =
            layout.function_offsets.find(function.name);
        if(function_offset == layout.function_offsets.end()) {
          throw logic_error("missing Linux EH function offset for " + function.name);
        }
        append_extra_pcrel32_code_relocation(eh_frame_section,
                                             function_pointer_offset,
                                             static_cast<long long>(function_offset->second));
      };

  for(size_t i = 0; i < program.functions.size(); ++i) {
    const mir::Function & function = program.functions[i];
    if(host_eh_by_name.find(function.name) != host_eh_by_name.end()) {
      continue;
    }
    append_linux_fde(function, base_cie_start, false);
  }

  size_t lsda_cie_start = 0;
  if(have_lsda_functions) {
    lsda_cie_start = eh_frame_section.bytes.size();
    const size_t lsda_cie_length_offset = eh_frame_section.bytes.size();
    append_u32(eh_frame_section.bytes, 0);
    append_u32(eh_frame_section.bytes, 0);
    eh_frame_section.bytes.push_back(0x01);
    eh_frame_section.bytes.push_back('z');
    eh_frame_section.bytes.push_back('P');
    eh_frame_section.bytes.push_back('L');
    eh_frame_section.bytes.push_back('R');
    eh_frame_section.bytes.push_back(0x00);
    append_uleb128(eh_frame_section.bytes, 1);
    append_sleb128(eh_frame_section.bytes, -8);
    append_uleb128(eh_frame_section.bytes, 16);
    append_uleb128(eh_frame_section.bytes, 7);
    eh_frame_section.bytes.push_back(0x9B);
    const size_t personality_offset = eh_frame_section.bytes.size();
    append_u32(eh_frame_section.bytes, 0);
    eh_frame_section.bytes.push_back(kPcRelSData4);
    eh_frame_section.bytes.push_back(kPcRelSData4);
    append_common_cfi(eh_frame_section.bytes);
    finish_eh_frame_record(lsda_cie_length_offset);
    append_extra_pcrel32_relocation(eh_frame_section,
                                    personality_offset,
                                    personality_ref_symbol.name,
                                    0);
  }

  for(size_t i = 0; i < program.functions.size(); ++i) {
    const mir::Function & function = program.functions[i];
    if(!function.host_eh_enabled) {
      continue;
    }
    if(host_eh_by_name.find(function.name) == host_eh_by_name.end()) {
      continue;
    }
    if(lsda_cie_start == 0) {
      throw logic_error("missing Linux LSDA CIE for " + function.name);
    }
    append_linux_fde(function, lsda_cie_start, true);
  }

  if(have_lsda_functions) {
    object.extra_sections.push_back(personality_ref_section);
  }
  if(!lsda_section.bytes.empty()) {
    object.extra_sections.push_back(lsda_section);
  }
  object.extra_sections.push_back(eh_frame_section);
}

}  // namespace

void append_host_eh_sections(const mir::Program & program,
                             const HostEhObjectLayout & layout,
                             const vector<HostEhFunctionInfo> & host_eh_functions,
                             mobj::ObjectFile & object)
{
  append_macho_host_eh_sections(program, layout, host_eh_functions, object);
  append_linux_host_eh_sections(program, layout, host_eh_functions, object);
}

}  // namespace host_eh_object_sections
