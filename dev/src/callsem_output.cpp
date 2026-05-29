#include "callsem_output.h"

#include <algorithm>
#include <cstdlib>
#include <map>
#include <ostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>

#include "semantic_context.h"

using namespace std;

namespace {

thread_local const SemanticContext * g_callsem_dump_source_location_context = nullptr;
thread_local const char * g_callsem_construction_path = nullptr;
const uint32_t kCallSemSourceFileIndexMax = UINT16_MAX;
const uint32_t kCallSemSourceLineMax = (1u << 24) - 1u;
const uint32_t kCallSemSourceColumnMax = (1u << 24) - 1u;

struct CallSemConstructionStats
{
  unsigned long long count = 0;
  unsigned long long explicit_child_count = 0;
  unsigned long long text_chars = 0;
};

struct CallSemConstructionPathKindKey
{
  string path;
  CallSemKind kind = CallSemKind::invalid;

  bool operator<(const CallSemConstructionPathKindKey & rhs) const
  {
    if(path != rhs.path) {
      return path < rhs.path;
    }
    return kind < rhs.kind;
  }
};

struct CallSemConstructionPathKindEntry
{
  CallSemConstructionPathKindKey key;
  CallSemConstructionStats stats;
};

map<CallSemKind, CallSemConstructionStats> & callsem_construction_stats()
{
  static map<CallSemKind, CallSemConstructionStats> stats;
  return stats;
}

map<string, CallSemConstructionStats> & callsem_construction_path_stats()
{
  static map<string, CallSemConstructionStats> stats;
  return stats;
}

map<CallSemConstructionPathKindKey, CallSemConstructionStats> &
callsem_construction_path_kind_stats()
{
  static map<CallSemConstructionPathKindKey, CallSemConstructionStats> stats;
  return stats;
}

void add_callsem_construction_stats(CallSemConstructionStats & stats,
                                    const string & text,
                                    size_t explicit_child_count)
{
  ++stats.count;
  stats.explicit_child_count += explicit_child_count;
  stats.text_chars += text.size();
}

void dump_callsem_construction_stats(ostream & out,
                                     const string & prefix,
                                     size_t rank,
                                     const CallSemConstructionStats & stats)
{
  out << prefix;
  if(rank != 0) {
    out << " rank=" << rank;
  }
  out << " count=" << stats.count
      << " explicit_child_count=" << stats.explicit_child_count
      << " text_chars=" << stats.text_chars
      << '\n';
}

const string & visible_node_text(const CallSemNode & node)
{
  return callsem_resolved_name(node).empty() ? node.text.str() :
      callsem_resolved_name(node);
}

map<string, uint32_t> & callsem_source_file_indices()
{
  static map<string, uint32_t> values;
  return values;
}

vector<string> & callsem_source_file_pool()
{
  static vector<string> values;
  return values;
}

uint64_t callsem_hash_mix(uint64_t value, uint64_t item)
{
  value ^= item + 0x9e3779b97f4a7c15ULL + (value << 6) + (value >> 2);
  return value;
}

uint64_t callsem_hash_string(uint64_t value, const string & text)
{
  for(size_t i = 0; i < text.size(); ++i) {
    value = callsem_hash_mix(value, static_cast<unsigned char>(text[i]));
  }
  return callsem_hash_mix(value, text.size());
}

bool callsem_symbol_identity_equal(const symbol_linkage::SymbolIdentity & lhs,
                                   const symbol_linkage::SymbolIdentity & rhs)
{
  if(lhs.internal_symbol != rhs.internal_symbol ||
     lhs.object_symbol != rhs.object_symbol ||
     lhs.thread_local_wrapper_object_symbol !=
         rhs.thread_local_wrapper_object_symbol ||
     static_cast<bool>(lhs.abi_mangle_facts) !=
         static_cast<bool>(rhs.abi_mangle_facts) ||
     lhs.keep_internal_alias != rhs.keep_internal_alias ||
     lhs.prefer_local_object_binding != rhs.prefer_local_object_binding ||
     lhs.linkage != rhs.linkage) {
    return false;
  }
  if(!lhs.abi_mangle_facts) {
    return true;
  }
  if(lhs.abi_mangle_facts->size() != rhs.abi_mangle_facts->size()) {
    return false;
  }
  for(size_t i = 0; i < lhs.abi_mangle_facts->size(); ++i) {
    const symbol_linkage::SymbolIdentity::AbiMangleFactEntry & lhs_fact =
        (*lhs.abi_mangle_facts)[i];
    const symbol_linkage::SymbolIdentity::AbiMangleFactEntry & rhs_fact =
        (*rhs.abi_mangle_facts)[i];
    if(lhs_fact.object_symbol != rhs_fact.object_symbol ||
       lhs_fact.target.kind != rhs_fact.target.kind ||
       lhs_fact.target.qualified_name != rhs_fact.target.qualified_name ||
       lhs_fact.target.c_linkage != rhs_fact.target.c_linkage) {
      return false;
    }
  }
  return true;
}

uint64_t callsem_symbol_identity_hash(
    const symbol_linkage::SymbolIdentity & symbol)
{
  uint64_t value = 1469598103934665603ULL;
  value = callsem_hash_string(value, symbol.internal_symbol);
  value = callsem_hash_string(value, symbol.object_symbol);
  value = callsem_hash_string(value, symbol.thread_local_wrapper_object_symbol);
  const size_t abi_fact_count =
      symbol.abi_mangle_facts ? symbol.abi_mangle_facts->size() : 0;
  value = callsem_hash_mix(value, abi_fact_count);
  for(size_t i = 0; i < abi_fact_count; ++i) {
    const symbol_linkage::SymbolIdentity::AbiMangleFactEntry & fact =
        (*symbol.abi_mangle_facts)[i];
    value = callsem_hash_string(value, fact.object_symbol);
    value = callsem_hash_mix(
        value,
        static_cast<unsigned>(fact.target.kind));
    value = callsem_hash_string(
        value,
        fact.target.qualified_name);
    value = callsem_hash_mix(
        value,
        fact.target.c_linkage ? 1 : 0);
  }
  value = callsem_hash_mix(value, symbol.keep_internal_alias ? 1 : 0);
  value = callsem_hash_mix(value, symbol.prefer_local_object_binding ? 1 : 0);
  value = callsem_hash_mix(value, static_cast<unsigned>(symbol.linkage));
  return value;
}

unordered_map<uint64_t, vector<shared_ptr<symbol_linkage::SymbolIdentity> > > &
callsem_symbol_pool()
{
  static unordered_map<uint64_t, vector<shared_ptr<symbol_linkage::SymbolIdentity> > >
      values;
  return values;
}

bool is_char_fundamental_type(const cpp_decl::TypePtr & type)
{
  const cpp_decl::TypePtr base =
      cpp_decl::strip_top_level_cv(cpp_decl::remove_reference_type(type));
  return base &&
         base->kind == cpp_decl::Type::TK_FUNDAMENTAL &&
         base->fundamental == FT_CHAR;
}

bool is_const_void_pointer_type(const cpp_decl::TypePtr & type)
{
  const cpp_decl::TypePtr base =
      cpp_decl::strip_top_level_cv(cpp_decl::remove_reference_type(type));
  if(!base || base->kind != cpp_decl::Type::TK_POINTER) {
    return false;
  }
  const cpp_decl::TypePtr inner = cpp_decl::strip_top_level_cv(base->inner);
  return inner &&
         inner->kind == cpp_decl::Type::TK_FUNDAMENTAL &&
         inner->fundamental == FT_VOID;
}

bool parse_decimal_unsigned(const string & text, unsigned long long & out)
{
  if(text.empty()) {
    return false;
  }
  unsigned long long value = 0;
  for(size_t i = 0; i < text.size(); ++i) {
    const char ch = text[i];
    if(ch < '0' || ch > '9') {
      return false;
    }
    value = value * 10 + static_cast<unsigned long long>(ch - '0');
  }
  out = value;
  return value != 0;
}

void maybe_set_dump_source_location(CallSemNode & node, const CppAstNode & ast)
{
  if(g_callsem_dump_source_location_context == nullptr) {
    return;
  }

  const string note = g_callsem_dump_source_location_context->source_location_for_node(ast);
  const string prefix = " at ";
  if(note.size() <= prefix.size() ||
     note.compare(0, prefix.size(), prefix) != 0) {
    return;
  }

  const string text = note.substr(prefix.size());
  const string::size_type column_colon = text.rfind(':');
  if(column_colon == string::npos) {
    return;
  }
  const string::size_type line_colon = text.rfind(':', column_colon - 1);
  if(line_colon == string::npos) {
    return;
  }

  unsigned long long line = 0;
  unsigned long long column = 0;
  if(!parse_decimal_unsigned(text.substr(line_colon + 1, column_colon - line_colon - 1), line) ||
     !parse_decimal_unsigned(text.substr(column_colon + 1), column)) {
    return;
  }

  const string file = text.substr(0, line_colon);
  if(file.empty()) {
    return;
  }

  set_callsem_source_file(node, file);
  set_callsem_source_line(node, line);
  set_callsem_source_column(node, column);
}

}  // namespace

ostream & operator<<(ostream & out, const CallSemText & text)
{
  return out << text.str();
}

const char * callsem_kind_text(CallSemKind kind)
{
  switch(kind) {
#define CALL_SEM_KIND_CASE(name, text) case CallSemKind::name: return text;
    CALL_SEM_KIND_LIST(CALL_SEM_KIND_CASE)
#undef CALL_SEM_KIND_CASE
  }
  return "<invalid>";
}

bool callsem_construction_census_enabled()
{
  static const bool enabled = []()
  {
    const char * value = getenv("CPPGM_CALLSEM_CONSTRUCTION_CENSUS");
    return value != nullptr &&
           *value != '\0' &&
           string(value) != "0" &&
           string(value) != "false" &&
           string(value) != "FALSE";
  }();
  return enabled;
}

void callsem_note_constructed_node(CallSemKind kind,
                                  const string & text,
                                  size_t explicit_child_count)
{
  if(!callsem_construction_census_enabled()) {
    return;
  }
  add_callsem_construction_stats(callsem_construction_stats()[kind],
                                 text,
                                 explicit_child_count);
  const string path =
      g_callsem_construction_path ? g_callsem_construction_path : "unscoped";
  add_callsem_construction_stats(callsem_construction_path_stats()[path],
                                 text,
                                 explicit_child_count);
  CallSemConstructionPathKindKey key;
  key.path = path;
  key.kind = kind;
  add_callsem_construction_stats(callsem_construction_path_kind_stats()[key],
                                 text,
                                 explicit_child_count);
}

void dump_callsem_construction_census(ostream & out)
{
  if(!callsem_construction_census_enabled()) {
    return;
  }
  typedef pair<CallSemKind, CallSemConstructionStats> Entry;
  vector<Entry> entries(callsem_construction_stats().begin(),
                        callsem_construction_stats().end());
  sort(entries.begin(),
       entries.end(),
       [](const Entry & lhs, const Entry & rhs)
       {
         if(lhs.second.count != rhs.second.count) {
           return lhs.second.count > rhs.second.count;
         }
         return string(callsem_kind_text(lhs.first)) <
                string(callsem_kind_text(rhs.first));
       });

  unsigned long long total_count = 0;
  unsigned long long total_explicit_children = 0;
  unsigned long long total_text_chars = 0;
  for(size_t i = 0; i < entries.size(); ++i) {
    total_count += entries[i].second.count;
    total_explicit_children += entries[i].second.explicit_child_count;
    total_text_chars += entries[i].second.text_chars;
  }
  out << "callsem-construction-summary"
      << " nodes=" << total_count
      << " distinct_kinds=" << entries.size()
      << " distinct_paths=" << callsem_construction_path_stats().size()
      << " distinct_path_kinds=" << callsem_construction_path_kind_stats().size()
      << " explicit_child_count=" << total_explicit_children
      << " text_chars=" << total_text_chars
      << '\n';
  for(size_t i = 0; i < entries.size(); ++i) {
    ostringstream prefix;
    prefix << "callsem-construction-kind"
           << " kind=" << callsem_kind_text(entries[i].first);
    dump_callsem_construction_stats(out, prefix.str(), i + 1, entries[i].second);
  }

  typedef pair<string, CallSemConstructionStats> PathEntry;
  vector<PathEntry> path_entries(callsem_construction_path_stats().begin(),
                                 callsem_construction_path_stats().end());
  sort(path_entries.begin(),
       path_entries.end(),
       [](const PathEntry & lhs, const PathEntry & rhs)
       {
         if(lhs.second.count != rhs.second.count) {
           return lhs.second.count > rhs.second.count;
         }
         return lhs.first < rhs.first;
       });
  for(size_t i = 0; i < path_entries.size(); ++i) {
    ostringstream prefix;
    prefix << "callsem-construction-path"
           << " path=" << path_entries[i].first;
    dump_callsem_construction_stats(out, prefix.str(), i + 1, path_entries[i].second);
  }

  vector<CallSemConstructionPathKindEntry> path_kind_entries;
  for(map<CallSemConstructionPathKindKey, CallSemConstructionStats>::const_iterator it =
          callsem_construction_path_kind_stats().begin();
      it != callsem_construction_path_kind_stats().end();
      ++it) {
    CallSemConstructionPathKindEntry entry;
    entry.key = it->first;
    entry.stats = it->second;
    path_kind_entries.push_back(entry);
  }
  sort(path_kind_entries.begin(),
       path_kind_entries.end(),
       [](const CallSemConstructionPathKindEntry & lhs,
          const CallSemConstructionPathKindEntry & rhs)
       {
         if(lhs.stats.count != rhs.stats.count) {
           return lhs.stats.count > rhs.stats.count;
         }
         if(lhs.key.path != rhs.key.path) {
           return lhs.key.path < rhs.key.path;
         }
         return lhs.key.kind < rhs.key.kind;
       });
  for(size_t i = 0; i < path_kind_entries.size(); ++i) {
    ostringstream prefix;
    prefix << "callsem-construction-path-kind"
           << " path=" << path_kind_entries[i].key.path
           << " kind=" << callsem_kind_text(path_kind_entries[i].key.kind);
    dump_callsem_construction_stats(out,
                                    prefix.str(),
                                    i + 1,
                                    path_kind_entries[i].stats);
  }
  callsem_construction_stats().clear();
  callsem_construction_path_stats().clear();
  callsem_construction_path_kind_stats().clear();
}

ScopedCallSemConstructionPath::ScopedCallSemConstructionPath(const char * path)
  : saved_(g_callsem_construction_path),
    active_(callsem_construction_census_enabled())
{
  if(active_) {
    g_callsem_construction_path =
        (path && *path) ? path : "unscoped";
  }
}

ScopedCallSemConstructionPath::~ScopedCallSemConstructionPath()
{
  if(active_) {
    g_callsem_construction_path = saved_;
  }
}

const string & callsem_empty_extra_string()
{
  static const string empty;
  return empty;
}

uint32_t callsem_intern_source_file_index(const string & value)
{
  if(value.empty()) {
    return 0;
  }
  map<string, uint32_t> & indices = callsem_source_file_indices();
  map<string, uint32_t>::const_iterator found = indices.find(value);
  if(found != indices.end()) {
    return found->second;
  }
  vector<string> & pool = callsem_source_file_pool();
  if(pool.size() >= static_cast<size_t>(kCallSemSourceFileIndexMax)) {
    throw logic_error("too many CallSem source files");
  }
  pool.push_back(value);
  const uint32_t index = static_cast<uint32_t>(pool.size());
  indices[value] = index;
  return index;
}

const string & callsem_source_file_by_index(uint32_t index)
{
  if(index == 0) {
    return callsem_empty_extra_string();
  }
  vector<string> & pool = callsem_source_file_pool();
  if(index > pool.size()) {
    throw logic_error("invalid CallSem source file index");
  }
  return pool[index - 1];
}

uint32_t callsem_checked_source_line(unsigned long long value)
{
  if(value > kCallSemSourceLineMax) {
    throw logic_error("CallSem source line is too large");
  }
  return static_cast<uint32_t>(value);
}

uint32_t callsem_checked_source_column(unsigned long long value)
{
  if(value > kCallSemSourceColumnMax) {
    throw logic_error("CallSem source column is too large");
  }
  return static_cast<uint32_t>(value);
}

const cpp_decl::TypePtr & callsem_empty_extra_type()
{
  static const cpp_decl::TypePtr empty;
  return empty;
}

const symbol_linkage::SymbolIdentity & callsem_empty_symbol()
{
  static const symbol_linkage::SymbolIdentity empty;
  return empty;
}

shared_ptr<symbol_linkage::SymbolIdentity>
callsem_intern_symbol(const symbol_linkage::SymbolIdentity & symbol)
{
  const uint64_t hash = callsem_symbol_identity_hash(symbol);
  vector<shared_ptr<symbol_linkage::SymbolIdentity> > & bucket =
      callsem_symbol_pool()[hash];
  for(size_t i = 0; i < bucket.size(); ++i) {
    if(bucket[i] && callsem_symbol_identity_equal(*bucket[i], symbol)) {
      return bucket[i];
    }
  }
  shared_ptr<symbol_linkage::SymbolIdentity> interned(
      new symbol_linkage::SymbolIdentity(symbol));
  bucket.push_back(interned);
  return interned;
}

const shared_ptr<cpp_decl::QualifiedName> & callsem_empty_qualified_name_syntax()
{
  static const shared_ptr<cpp_decl::QualifiedName> empty;
  return empty;
}

const CallSemVirtualBaseLayout & callsem_empty_virtual_base_layout()
{
  static const CallSemVirtualBaseLayout empty;
  return empty;
}

const shared_ptr<CallSemNode> & callsem_empty_lowered_condition_test()
{
  static const shared_ptr<CallSemNode> empty;
  return empty;
}

bool callsem_has_token(const CallSemNode & node, ETokenType type)
{
  return node.has_token && node.token_type == type;
}

CallSemNode make_dump_node(CallSemKind kind, const string & text)
{
  CallSemNode node;
  node.kind = kind;
  node.text = text;
  callsem_note_constructed_node(kind, text, 0);
  return node;
}

void set_dump_token(CallSemNode & node, const CppAstNode & ast)
{
  if(!ast.has_token) {
    return;
  }
  node.has_token = true;
  node.token_type = ast.simple_type;
  maybe_set_dump_source_location(node, ast);
}

void set_dump_source_location(CallSemNode & node, const CppAstNode & ast)
{
  maybe_set_dump_source_location(node, ast);
}

void set_dump_symbol(CallSemNode & node, const symbol_linkage::SymbolIdentity & symbol)
{
  set_callsem_symbol(node, symbol);
  if(callsem_runtime_bridge_symbol(node).empty()) {
    set_callsem_runtime_bridge_symbol(
        node,
        runtime_bridge_symbol_for_object_symbol(symbol.object_symbol));
  }
  if(callsem_runtime_bridge_symbol(node).empty() && node.kind == CallSemKind::callee) {
    set_callsem_runtime_bridge_symbol(
        node,
        runtime_bridge_symbol_for_function_name_and_type(node.text, node.semantic_type));
  }
}

ScopedCallSemDumpSourceLocationContext::ScopedCallSemDumpSourceLocationContext(
    const SemanticContext * ctx)
  : saved_(g_callsem_dump_source_location_context)
{
  g_callsem_dump_source_location_context = ctx;
}

ScopedCallSemDumpSourceLocationContext::~ScopedCallSemDumpSourceLocationContext()
{
  g_callsem_dump_source_location_context = saved_;
}

namespace {

bool is_num_put_runtime_bridge_symbol(const string & object_symbol)
{
  return object_symbol == "cppgm_host_num_put_char_put_bool" ||
         object_symbol == "cppgm_host_num_put_char_put_long" ||
         object_symbol == "cppgm_host_num_put_char_put_long_long" ||
         object_symbol == "cppgm_host_num_put_char_put_unsigned_long" ||
         object_symbol == "cppgm_host_num_put_char_put_unsigned_long_long" ||
         object_symbol == "cppgm_host_num_put_char_put_double" ||
         object_symbol == "cppgm_host_num_put_char_put_long_double" ||
         object_symbol == "cppgm_host_num_put_char_put_ptr";
}

}

string runtime_bridge_symbol_for_function_type(const cpp_decl::TypePtr & function_type)
{
  using namespace cpp_decl;

  if(!function_type || function_type->kind != Type::TK_FUNCTION) {
    return "";
  }

  if(function_type->params.size() != 4 && function_type->params.size() != 5) {
    return "";
  }

  if(!is_char_fundamental_type(function_type->params[function_type->params.size() - 2])) {
    return "";
  }

  const TypePtr value_type =
      strip_top_level_cv(remove_reference_type(function_type->params.back()));
  if(!value_type) {
    return "";
  }

  if(value_type->kind == Type::TK_FUNDAMENTAL) {
    switch(value_type->fundamental) {
    case FT_BOOL:
      return "cppgm_host_num_put_char_put_bool";
    case FT_LONG_INT:
      return "cppgm_host_num_put_char_put_long";
    case FT_LONG_LONG_INT:
      return "cppgm_host_num_put_char_put_long_long";
    case FT_UNSIGNED_LONG_INT:
      return "cppgm_host_num_put_char_put_unsigned_long";
    case FT_UNSIGNED_LONG_LONG_INT:
      return "cppgm_host_num_put_char_put_unsigned_long_long";
    case FT_DOUBLE:
      return "cppgm_host_num_put_char_put_double";
    case FT_LONG_DOUBLE:
      return "cppgm_host_num_put_char_put_long_double";
    default:
      return "";
    }
  }

  if(is_const_void_pointer_type(value_type)) {
    return "cppgm_host_num_put_char_put_ptr";
  }
  return "";
}

string runtime_bridge_symbol_for_function_name_and_type(const string & name,
                                                        const cpp_decl::TypePtr & function_type)
{
  if(name.find("std::__1::num_put<") == string::npos) {
    return "";
  }
  const string put_suffix = "::put";
  const string do_put_suffix = "::do_put";
  const bool matches_put =
      name.size() >= put_suffix.size() &&
      name.compare(name.size() - put_suffix.size(), put_suffix.size(), put_suffix) == 0;
  const bool matches_do_put =
      name.size() >= do_put_suffix.size() &&
      name.compare(name.size() - do_put_suffix.size(), do_put_suffix.size(), do_put_suffix) == 0;
  if(!matches_put && !matches_do_put) {
    return "";
  }
  return runtime_bridge_symbol_for_function_type(function_type);
}

string runtime_bridge_symbol_for_bound_function(const string & qualified_name,
                                                const string & owner_class_name,
                                                const cpp_decl::TypePtr & function_type)
{
  const string direct =
      runtime_bridge_symbol_for_function_name_and_type(qualified_name, function_type);
  if(!direct.empty()) {
    return direct;
  }

  if(owner_class_name.find("std::__1::num_put<") == string::npos) {
    return "";
  }

  const size_t member_sep = qualified_name.rfind("::");
  const string member_name =
      member_sep == string::npos ? qualified_name : qualified_name.substr(member_sep);
  if(member_name != "::put" && member_name != "::do_put" &&
     qualified_name != "put" && qualified_name != "do_put") {
    return "";
  }

  return runtime_bridge_symbol_for_function_type(function_type);
}

string runtime_bridge_symbol_for_object_symbol(const string & object_symbol)
{
  return is_num_put_runtime_bridge_symbol(object_symbol) ? object_symbol : "";
}

string call_value_category_text(CallValueCategory category)
{
  switch(category) {
  case CVC_NONE: return "";
  case CVC_LVALUE: return "lvalue";
  case CVC_PRVALUE: return "prvalue";
  case CVC_XVALUE: return "xvalue";
  }

  throw logic_error("unknown call value category");
}

string callsem_payload_text(const CallSemNode & node)
{
  string payload;
  if(!node.has_token) {
    payload = node.text;
  } else {
    payload = string(token_type_to_string(node.token_type)) + ":" + node.text;
  }
  if(node.is_base_subobject) {
    payload += " [base-subobject]";
  }
  if(node.is_virtual_base_subobject) {
    payload += " [virtual-base-subobject]";
  }
  if(node.is_reference_storage_target) {
    payload += " [ref-storage-target]";
  }
  if(node.is_thread_local) {
    payload += " [thread-local]";
  }
  const CallSemVirtualBaseLayout & virtual_base_layout =
      callsem_virtual_base_layout(node);
  if(!virtual_base_layout.empty()) {
    payload += " [virtual-base-layout";
    for(size_t i = 0; i < virtual_base_layout.size(); ++i) {
      payload += " " + virtual_base_layout[i].first +
                 "@" + to_string(virtual_base_layout[i].second);
    }
    payload += "]";
  }
  if(!callsem_runtime_bridge_symbol(node).empty()) {
    payload += " [runtime-bridge " + callsem_runtime_bridge_symbol(node) + "]";
  }
  return payload;
}

string callsem_display_text(const CallSemNode & node)
{
  switch(node.kind) {
  case CallSemKind::translation_unit:
  case CallSemKind::simple_declaration:
  case CallSemKind::condition_declaration:
  case CallSemKind::condition:
  case CallSemKind::do_statement:
  case CallSemKind::for_init_statement:
  case CallSemKind::expression_statement:
  case CallSemKind::asm_clause:
  case CallSemKind::statement_expression:
  case CallSemKind::asm_statement:
  case CallSemKind::return_statement:
  case CallSemKind::throw_statement:
  case CallSemKind::if_statement:
  case CallSemKind::then_node:
  case CallSemKind::else_node:
  case CallSemKind::while_statement:
  case CallSemKind::for_statement:
  case CallSemKind::goto_statement:
  case CallSemKind::iteration:
  case CallSemKind::break_statement:
  case CallSemKind::continue_statement:
  case CallSemKind::catch_handler:
  case CallSemKind::compound_statement:
  case CallSemKind::labeled_statement:
  case CallSemKind::vptr_action:
  case CallSemKind::vtable_definition:
  case CallSemKind::vtable_entry:
  case CallSemKind::vtt_definition:
  case CallSemKind::vtt_entry:
  case CallSemKind::case_statement:
  case CallSemKind::default_statement:
  case CallSemKind::switch_statement:
  case CallSemKind::try_statement:
    return node.text;

  case CallSemKind::constructor_action:
  case CallSemKind::destructor_action:
    return visible_node_text(node);

  case CallSemKind::namespace_definition:
    return node.text;

  case CallSemKind::function_definition:
  case CallSemKind::function_declaration:
  case CallSemKind::parameter:
  case CallSemKind::type_alias:
  case CallSemKind::variable:
  case CallSemKind::callee:
    return visible_node_text(node) + " " + cpp_decl::describe_type(node.semantic_type);

  case CallSemKind::closure_object:
  case CallSemKind::initializer_list_object:
    return call_value_category_text(node.value_category) + " " +
           cpp_decl::describe_type(node.semantic_type) + " " + node.text;

  case CallSemKind::closure_capture:
    return node.text + " " + cpp_decl::describe_type(node.semantic_type);

  case CallSemKind::literal:
  case CallSemKind::cast_expression:
  case CallSemKind::id_expression:
  case CallSemKind::member_expression:
  case CallSemKind::unary_expression:
  case CallSemKind::postfix_expression:
  case CallSemKind::binary_expression:
  case CallSemKind::assignment_expression:
  case CallSemKind::dynamic_cast_expression:
  case CallSemKind::typeid_expression:
  {
    string result = call_value_category_text(node.value_category) + " " +
                    cpp_decl::describe_type(node.semantic_type);
    const string payload = callsem_payload_text(node);
    if(!payload.empty()) {
      result += " " + payload;
    }
    return result;
  }

  case CallSemKind::subscript_expression:
  case CallSemKind::sizeof_expression:
  case CallSemKind::call_expression:
  case CallSemKind::new_expression:
  case CallSemKind::conditional_expression:
  case CallSemKind::braced_init_list:
    return call_value_category_text(node.value_category) + " " +
           cpp_decl::describe_type(node.semantic_type);

  case CallSemKind::range_for_statement:
  case CallSemKind::rtti_candidate:
  case CallSemKind::rtti_base:
  case CallSemKind::rtti_definition:
    return callsem_kind_text(node.kind);

  case CallSemKind::invalid:
    return node.text;
  }

  return node.text;
}

namespace {

void indent(string & out, size_t depth)
{
  for(size_t i = 0; i < depth; ++i) {
    out += "  ";
  }
}

bool should_dump_node(const CallSemNode & node)
{
  return !(node.kind == CallSemKind::function_definition &&
           node.has_special_member_entry_point_kind &&
           callsem_special_member_entry_point_kind(node) != symbol_linkage::SMEK_COMPLETE);
}

void dump_node(string & out, const CallSemNode & node, size_t depth)
{
  if(!should_dump_node(node)) {
    return;
  }
  indent(out, depth);
  out += callsem_kind_text(node.kind);
  const string text = callsem_display_text(node);
  if(!text.empty()) {
    out.push_back(' ');
    out += text;
  }
  out.push_back('\n');

  for(size_t i = 0; i < node.children.size(); ++i) {
    dump_node(out, node.children[i], depth + 1);
  }
}

}  // namespace

string callsem_dump_tree(const CallSemNode & node)
{
  string out;
  dump_node(out, node, 0);
  if(!out.empty() && out[out.size() - 1] != '\n') {
    out += '\n';
  }
  return out;
}
