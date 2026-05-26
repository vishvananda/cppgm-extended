#include "lowirgensemantic.h"

#include <map>
#include <memory>
#include <set>
#include <algorithm>
#include <cerrno>
#include <cctype>
#include <cmath>
#include <cstdlib>
#include <deque>
#include <limits>
#include <sstream>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

#include "cpp_decl_model.h"
#include "eh_runtime.h"
#include "encoding.h"
#include "parser_trace.h"
#include "rtti_names.h"
#include "runtime_symbol_policy.h"
#include "semantic_builtins.h"
#include "semantic_conversion.h"
#include "semantic_metrics.h"
#include "semantic_output.h"
#include "semantic_utils.h"

using namespace std;

namespace {

using namespace cpp_decl;

const char * const kCppgmCallTerminateSupportSymbol = "cppgm_call_terminate";
const char * const kStdTerminateObjectSymbol = "_ZSt9terminatev";

const char * exported_linkage_name(symbol_linkage::SymbolLinkage linkage)
{
  switch(linkage) {
  case symbol_linkage::SL_INTERNAL:
    return "internal";
  case symbol_linkage::SL_EXTERNAL:
    return "external";
  case symbol_linkage::SL_WEAK:
    return "weak";
  }
  return "unknown";
}

string thread_local_init_internal_symbol(const string & global_symbol)
{
  return global_symbol + "__tls_init";
}

struct LowIRGlobal
{
  enum Kind
  {
    LG_SCALAR,
    LG_DATA
  } kind = LG_SCALAR;

  string name;
  bool readonly = false;
  bool thread_local_storage = false;
  string type;
  bool is_addr = false;
  string value;
  long long addr_addend = 0;
  vector<string> data_items;
  lowir_internal::SymbolMetadata metadata;
};

struct LowIRBlock
{
  string label;
  vector<string> instructions;
  bool terminated = false;
};

struct LowIRParameterText
{
  string name;
  string type;
  lowir_internal::ParameterMetadata metadata;
};

LowIRParameterText make_lowir_parameter_text(const string & name,
                                             const string & type,
                                             lowir_internal::ParamPassingMode passing =
                                                 lowir_internal::PPM_DIRECT,
                                             lowir_internal::ParamCaptureMode capture =
                                                 lowir_internal::PCM_DEFAULT)
{
  LowIRParameterText out;
  out.name = name;
  out.type = type;
  out.metadata.passing = passing;
  out.metadata.capture = capture;
  return out;
}

struct LowIRFunction
{
  string name;
  vector<LowIRParameterText> params;
  string return_type;
  vector<pair<string, string> > slots;
  vector<LowIRBlock> blocks;
  lowir_internal::InstructionDebugLocation debug_location;
  lowir_internal::FunctionBoundaryMetadata boundary_metadata;
  lowir_internal::SymbolMetadata metadata;
};

struct VariableBinding
{
  enum Mode
  {
    VBM_SCALAR_SLOT,
    VBM_REFERENCE_SLOT,
    VBM_DECAY_VIEW_SLOT,
    VBM_ARRAY_STORAGE,
    VBM_INDIRECT_STORAGE
  };

  TypePtr original_semantic_type;
  TypePtr semantic_type;
  string lowir_type;
  Mode mode = VBM_SCALAR_SLOT;
  bool is_parameter = false;
  bool uses_external_storage_address = false;
  bool is_named_return_slot_alias = false;
  string external_storage_address;
  vector<string> slots;
  map<string, string> hidden_virtual_base_slots;
};

struct GlobalBinding
{
  TypePtr semantic_type;
  string lowir_type;
  string storage;
  bool thread_local_storage = false;
  string thread_local_guard_symbol;
  string thread_local_init_symbol;
  bool is_definition = true;
  symbol_linkage::SymbolIdentity symbol;
};

struct FunctionSymbolEntry
{
  string name;
  TypePtr type;
  string symbol;
  bool has_definition = false;
};

struct ParameterVirtualBaseLayout
{
  size_t parameter_index = 0;
  vector<pair<string, unsigned long long> > layout;
};

struct ParameterVirtualBaseForwardingCandidate
{
  string caller_symbol;
  vector<size_t> argument_parameter_indices;
};

struct VirtualMemberPointerThunkRequest
{
  string symbol;
  TypePtr member_pointer_type;
  unsigned long long virtual_slot = 0;
  bool uses_extended_vtable_layout = false;
};

struct VTableEntryThunkRequest
{
  string symbol;
  string target_symbol;
  TypePtr function_type;
  long long this_adjust = 0;
  long long return_adjust = 0;
  long long virtual_adjust_offset = 0;
  bool uses_vcall_offset_adjust = false;
  bool has_exported_symbol = false;
  symbol_linkage::SymbolIdentity exported_symbol;
};

thread_local const CallSemNode * g_lowir_current_function_node = nullptr;
thread_local const CallSemNode * g_lowir_current_expr_node = nullptr;
thread_local const CallSemNode * g_lowir_current_stmt_node = nullptr;

bool lowir_debug_file_is_textually_representable(const string & file)
{
  if(file.empty()) {
    return false;
  }
  for(size_t i = 0; i < file.size(); ++i) {
    const unsigned char ch = static_cast<unsigned char>(file[i]);
    if(isspace(ch) ||
       ch == '(' || ch == ')' ||
       ch == '[' || ch == ']' ||
       ch == '{' || ch == '}' ||
       ch == ',' || ch == ':' || ch == '=') {
      return false;
    }
  }
  return true;
}

const CallSemNode * current_lowir_debug_node()
{
  if(g_lowir_current_expr_node &&
     g_lowir_current_expr_node->has_source_location() &&
     lowir_debug_file_is_textually_representable(
         callsem_source_file(*g_lowir_current_expr_node))) {
    return g_lowir_current_expr_node;
  }
  if(g_lowir_current_stmt_node &&
     g_lowir_current_stmt_node->has_source_location() &&
     lowir_debug_file_is_textually_representable(
         callsem_source_file(*g_lowir_current_stmt_node))) {
    return g_lowir_current_stmt_node;
  }
  if(g_lowir_current_function_node &&
     g_lowir_current_function_node->has_source_location() &&
     lowir_debug_file_is_textually_representable(
         callsem_source_file(*g_lowir_current_function_node))) {
    return g_lowir_current_function_node;
  }
  return nullptr;
}

string lowir_debug_suffix_for_current_node()
{
  const CallSemNode * node = current_lowir_debug_node();
  if(node == nullptr) {
    return string();
  }

  ostringstream out;
  out << " !dbg(" << callsem_source_file(*node)
      << ", " << callsem_source_line(*node)
      << ", " << callsem_source_column(*node) << ")";
  return out.str();
}

struct ScopedLowIRCurrentExpr
{
  explicit ScopedLowIRCurrentExpr(const CallSemNode & node)
    : saved_(g_lowir_current_expr_node)
  {
    g_lowir_current_expr_node = &node;
  }

  ~ScopedLowIRCurrentExpr()
  {
    g_lowir_current_expr_node = saved_;
  }

private:
  const CallSemNode * saved_;
};

struct ScopedLowIRCurrentStatement
{
  explicit ScopedLowIRCurrentStatement(const CallSemNode & node)
    : saved_(g_lowir_current_stmt_node)
  {
    g_lowir_current_stmt_node = &node;
  }

  ~ScopedLowIRCurrentStatement()
  {
    g_lowir_current_stmt_node = saved_;
  }

private:
  const CallSemNode * saved_;
};

struct VTableBinding
{
  string base_symbol;
  unsigned long long address_point_offset = 0;
};

struct CleanupAction
{
  enum Kind
  {
    CK_NODE,
    CK_BOUND_LOCAL_NODE,
    CK_DESTROY_CLASS_OBJECT,
    CK_DESTROY_CLASS_AT_NODE,
    CK_DESTROY_CLASS_AT_PTR,
    CK_EH_END,
    CK_CLEAR_EXCEPTION
  } kind = CK_NODE;

  const CallSemNode * node = nullptr;
  string storage_slot;
  string bound_local_name;
  TypePtr object_type;
};

struct BindingScopeEntry
{
  string name;
  bool had_previous = false;
  VariableBinding previous;
};

struct ControlTransferTarget
{
  ControlTransferTarget() {}
  ControlTransferTarget(const string & label, size_t cleanup_depth)
    : label(label)
    , cleanup_depth(cleanup_depth)
  {}

  string label;
  size_t cleanup_depth = 0;
};

struct ParamAbiPlan
{
  enum Kind
  {
    PAK_SCALAR,
    PAK_INDIRECT,
    PAK_DIRECT_OBJECT
  } kind = PAK_SCALAR;

  vector<pair<string, string> > inputs;
};

string normalize_literal_token(const CallSemNode & node)
{
  if(node.has_int_value) {
    return to_string(callsem_int_value(node));
  }
  if(node.has_uint_value) {
    return to_string(callsem_uint_value(node));
  }
  if(callsem_has_token(node, KW_TRUE)) {
    return "1";
  }
  if(callsem_has_token(node, KW_FALSE)) {
    return "0";
  }
  return node.text;
}

string zero_literal_for_lowir_type(const string & type)
{
  if(type == "f32") {
    return "0.0f";
  }
  if(type == "f64") {
    return "0.0";
  }
  if(type == "f80") {
    return "0.0L";
  }
  return "0";
}

string encode_data_member_pointer_offset(unsigned long long offset)
{
  return to_string(offset + 1);
}

bool try_get_integral_literal_value(const CallSemNode & node, long long & out)
{
  if(node.has_int_value) {
    out = callsem_int_value(node);
    return true;
  }
  if(node.has_uint_value) {
    out = static_cast<long long>(callsem_uint_value(node));
    return true;
  }
  if(callsem_has_token(node, KW_TRUE)) {
    out = 1;
    return true;
  }
  if(callsem_has_token(node, KW_FALSE)) {
    out = 0;
    return true;
  }
  if(node.kind == CallSemKind::literal && !node.text.empty()) {
    char * end = nullptr;
    const long long value = strtoll(node.text.c_str(), &end, 10);
    if(end && *end == '\0') {
      out = value;
      return true;
    }
  }
  return false;
}

bool string_literal_type_matches(const TypePtr & type,
                                 const QuoteLiteralData & literal)
{
  TypePtr base = strip_top_level_cv(type);
  if(!base) {
    return false;
  }
  TypePtr element;
  if(base->kind == Type::TK_ARRAY || base->kind == Type::TK_POINTER) {
    element = strip_top_level_cv(base->inner);
  }
  return element &&
         element->kind == Type::TK_FUNDAMENTAL &&
         element->fundamental == string_literal_element_type(literal);
}

bool try_parse_string_literal_node(const CallSemNode & node, QuoteLiteralData & out)
{
  if(node.kind != CallSemKind::literal || node.text.find('"') == string::npos) {
    return false;
  }
  const QuoteLiteralData literal = parse_quote_literal(node.text);
  if(literal.quote != '"' || !literal.ud_suffix.empty() ||
     !string_literal_type_matches(node.semantic_type, literal)) {
    return false;
  }
  out = literal;
  return true;
}

vector<unsigned long long> string_literal_code_units(const QuoteLiteralData & literal)
{
  return quote_literal_string_units(literal);
}

string lowir_name(const string & qualified)
{
  return symbol_linkage::internal_symbol_from_name(qualified);
}

TypePtr std_terminate_function_type()
{
  return make_function(make_fundamental(FT_VOID), vector<TypePtr>(), false);
}

bool has_function_symbol_entry(const vector<FunctionSymbolEntry> & entries,
                               const string & symbol)
{
  for(size_t i = 0; i < entries.size(); ++i) {
    if(entries[i].symbol == symbol) {
      return true;
    }
  }
  return false;
}

void note_host_std_terminate_symbol(
    const vector<FunctionSymbolEntry> & function_symbol_entries,
    map<string, string> & external_function_symbols,
    map<string, TypePtr> & referenced_function_signature_types,
    const string & symbol)
{
  if(symbol.empty() || has_function_symbol_entry(function_symbol_entries, symbol)) {
    return;
  }
  external_function_symbols[symbol] = kStdTerminateObjectSymbol;
  referenced_function_signature_types[symbol] = std_terminate_function_type();
}

struct NumPutRuntimeBridgeSpec
{
  const char * symbol;
  const char * value_lowir_type;
  unsigned long long slot_offset;
};

const NumPutRuntimeBridgeSpec * find_num_put_runtime_bridge_spec(const string & symbol)
{
  static const NumPutRuntimeBridgeSpec kSpecs[] = {
    {"cppgm_host_num_put_char_put_bool", "i8", 0x18},
    {"cppgm_host_num_put_char_put_long", "i64", 0x20},
    {"cppgm_host_num_put_char_put_long_long", "i64", 0x28},
    {"cppgm_host_num_put_char_put_unsigned_long", "i64", 0x30},
    {"cppgm_host_num_put_char_put_unsigned_long_long", "i64", 0x38},
    {"cppgm_host_num_put_char_put_double", "f64", 0x40},
    {"cppgm_host_num_put_char_put_long_double", "f80", 0x48},
    {"cppgm_host_num_put_char_put_ptr", "ptr", 0x50},
  };
  for(size_t i = 0; i < sizeof(kSpecs) / sizeof(kSpecs[0]); ++i) {
    if(symbol == kSpecs[i].symbol) {
      return &kSpecs[i];
    }
  }
  return nullptr;
}

TypePtr lowir_value_conversion_type(const TypePtr & type);

string virtual_member_pointer_thunk_symbol(const string & target_symbol)
{
  return target_symbol + "__member_pointer_thunk";
}

TypePtr callable_function_type_for_member_pointer(const TypePtr & member_pointer_type)
{
  TypePtr base = strip_top_level_cv(member_pointer_type);
  if(!base || base->kind != Type::TK_MEMBER_POINTER || !is_function_type(base->inner)) {
    return TypePtr();
  }

  TypePtr owner_type = strip_top_level_cv(base->owner);
  if(!owner_type) {
    owner_type = base->owner;
  }
  if(base->inner->function_const || base->inner->function_volatile) {
    owner_type = make_cv(owner_type,
                         base->inner->function_const,
                         base->inner->function_volatile);
  }

  vector<TypePtr> params;
  params.push_back(make_pointer(owner_type));
  for(size_t i = 0; i < base->inner->params.size(); ++i) {
    params.push_back(base->inner->params[i]);
  }
  return make_function(base->inner->inner,
                       params,
                       base->inner->variadic,
                       base->inner->function_const,
                       base->inner->function_volatile,
                       base->inner->prototype_relaxed);
}

TypePtr exception_object_type(const TypePtr & type)
{
  TypePtr base = strip_top_level_cv(type);
  if(!base) {
    return TypePtr();
  }
  if(is_reference_type(base)) {
    return strip_top_level_cv(base->inner);
  }
  return base;
}

string exception_storage_symbol(const TypePtr & type)
{
  string symbol = rtti_symbol_for_type(type);
  const string prefix = "@__rtti_";
  if(symbol.compare(0, prefix.size(), prefix) == 0) {
    symbol.replace(0, prefix.size(), "@__ehobj_");
  } else {
    symbol = "@__ehobj_" + symbol;
  }
  return symbol;
}

string node_internal_symbol(const CallSemNode & node)
{
  return callsem_symbol(node).internal_symbol.empty() ? lowir_name(node.text.str())
                                             : callsem_symbol(node).internal_symbol;
}

bool constructor_call_targets_whole_variable(const CallSemNode & call,
                                             const CallSemNode & variable)
{
  if(call.kind != CallSemKind::call_expression ||
     call.children.size() < 2 ||
     variable.text.empty()) {
    return false;
  }
  const CallSemNode & target_arg = call.children[1];
  if(target_arg.kind != CallSemKind::unary_expression ||
     !callsem_has_token(target_arg, OP_AMP) ||
     target_arg.children.size() != 1) {
    return false;
  }
  const CallSemNode & target = target_arg.children[0];
  return (target.kind == CallSemKind::id_expression ||
          target.kind == CallSemKind::variable) &&
         target.text == variable.text;
}

const CallSemNode * peel_base_subobject_root_shared(const CallSemNode & node)
{
  const CallSemNode * current = &node;
  while(current) {
    if(current->kind == CallSemKind::unary_expression &&
       current->children.size() == 1 &&
       (callsem_has_token(*current, OP_AMP) || callsem_has_token(*current, OP_STAR))) {
      current = &current->children[0];
      continue;
    }
    if(current->kind == CallSemKind::member_expression &&
       current->is_base_subobject &&
       current->children.size() == 1) {
      current = &current->children[0];
      continue;
    }
    return current;
  }
  return nullptr;
}

string class_qualified_name(const TypePtr & type);
vector<pair<string, unsigned long long> > normalize_parameter_virtual_base_layout(
    const vector<pair<string, unsigned long long> > & layout);

TypePtr lowir_class_object_type(const TypePtr & type)
{
  TypePtr object_type = strip_top_level_cv(remove_reference_type(type));
  if(object_type && object_type->kind == Type::TK_POINTER) {
    object_type = strip_top_level_cv(object_type->inner);
  }
  return object_type;
}

bool try_find_unique_same_class_reference_result_argument(
    const CallSemNode & call,
    size_t & out_child_index)
{
  if(call.kind != CallSemKind::call_expression ||
     !is_reference_type(call.semantic_type)) {
    return false;
  }

  const string result_class =
      class_qualified_name(lowir_class_object_type(call.semantic_type));
  if(result_class.empty()) {
    return false;
  }

  bool found = false;
  for(size_t i = 1; i < call.children.size(); ++i) {
    const string arg_class =
        class_qualified_name(lowir_class_object_type(call.children[i].semantic_type));
    if(arg_class != result_class) {
      continue;
    }
    if(found) {
      return false;
    }
    out_child_index = i;
    found = true;
  }
  return found;
}

bool infer_parameter_virtual_base_layout(
    const CallSemNode & function_node,
    ParameterVirtualBaseLayout & out_layout)
{
  map<string, size_t> parameter_indices;
  size_t parameter_index = 0;
  for(size_t i = 0; i < function_node.children.size(); ++i) {
    if(function_node.children[i].kind == CallSemKind::parameter &&
       !function_node.children[i].text.empty()) {
      parameter_indices[function_node.children[i].text] = parameter_index;
    }
    if(function_node.children[i].kind == CallSemKind::parameter) {
      ++parameter_index;
    }
  }
  if(parameter_indices.empty()) {
    return false;
  }

  vector<pair<const CallSemNode *, unsigned long long> > stack(
      1, make_pair(&function_node, 0ULL));
  while(!stack.empty()) {
    const CallSemNode * current = stack.back().first;
    const unsigned long long current_base_offset = stack.back().second;
    stack.pop_back();
    const unsigned long long node_base_offset =
                current->kind == CallSemKind::member_expression &&
                current->is_base_subobject &&
                current->has_uint_value ?
            current_base_offset + callsem_uint_value(*current) :
            current_base_offset;
    const CallSemVirtualBaseLayout & current_virtual_base_layout =
        callsem_virtual_base_layout(*current);
    if(current->kind != CallSemKind::parameter &&
       !current_virtual_base_layout.empty()) {
      const CallSemNode * root = peel_base_subobject_root_shared(*current);
      map<string, size_t>::const_iterator parameter_it =
          root ? parameter_indices.find(root->text) : parameter_indices.end();
      if(root &&
         (root->kind == CallSemKind::variable ||
         root->kind == CallSemKind::id_expression ||
          root->kind == CallSemKind::parameter) &&
        parameter_it != parameter_indices.end()) {
        out_layout.parameter_index = parameter_it->second;
        out_layout.layout =
            normalize_parameter_virtual_base_layout(current_virtual_base_layout);
        return true;
      }
      if(root && root->kind == CallSemKind::call_expression) {
        size_t forwarded_child_index = 0;
        if(try_find_unique_same_class_reference_result_argument(
               *root,
               forwarded_child_index)) {
          const CallSemNode & forwarded_arg = root->children[forwarded_child_index];
          const CallSemNode * forwarded_root =
              peel_base_subobject_root_shared(forwarded_arg);
          map<string, size_t>::const_iterator forwarded_parameter_it =
              forwarded_root ?
                  parameter_indices.find(forwarded_root->text) :
                  parameter_indices.end();
          if(forwarded_root &&
             (forwarded_root->kind == CallSemKind::variable ||
              forwarded_root->kind == CallSemKind::id_expression ||
              forwarded_root->kind == CallSemKind::parameter) &&
             forwarded_parameter_it != parameter_indices.end()) {
            out_layout.parameter_index = forwarded_parameter_it->second;
            out_layout.layout =
                normalize_parameter_virtual_base_layout(current_virtual_base_layout);
            return true;
          }
        }
      }
    }
    for(size_t i = 0; i < current->children.size(); ++i) {
      stack.push_back(make_pair(&current->children[i], node_base_offset));
    }
  }
  return false;
}

bool merge_parameter_virtual_base_layout(ParameterVirtualBaseLayout & target,
                                         const ParameterVirtualBaseLayout & incoming,
                                         bool & changed)
{
  changed = false;
  if(target.parameter_index != incoming.parameter_index) {
    return false;
  }

  for(size_t i = 0; i < incoming.layout.size(); ++i) {
    bool found = false;
    for(size_t j = 0; j < target.layout.size(); ++j) {
      if(target.layout[j].first != incoming.layout[i].first) {
        continue;
      }
      if(target.layout[j].second != incoming.layout[i].second) {
        return false;
      }
      found = true;
      break;
    }
    if(!found) {
      target.layout.push_back(incoming.layout[i]);
      changed = true;
    }
  }
  return true;
}

vector<pair<string, unsigned long long> > normalize_parameter_virtual_base_layout(
    const vector<pair<string, unsigned long long> > & layout)
{
  vector<pair<string, unsigned long long> > out;
  out.reserve(layout.size());
  for(size_t i = 0; i < layout.size(); ++i) {
    out.push_back(make_pair(layout[i].first, 0ULL));
  }
  return out;
}

bool infer_reference_parameter_type_virtual_base_layout(
    const CallSemNode & function_node,
    const map<string, vector<pair<string, unsigned long long> > > & class_virtual_base_layouts,
    ParameterVirtualBaseLayout & out_layout)
{
  size_t parameter_index = 0;
  for(size_t i = 0; i < function_node.children.size(); ++i) {
    if(function_node.children[i].kind != CallSemKind::parameter) {
      continue;
    }

    const TypePtr parameter_type = function_node.children[i].semantic_type;
    if(is_reference_type(parameter_type)) {
      const string parameter_class =
          class_qualified_name(strip_top_level_cv(remove_reference_type(parameter_type)));
      map<string, vector<pair<string, unsigned long long> > >::const_iterator layout_it =
          class_virtual_base_layouts.find(parameter_class);
      if(!parameter_class.empty() &&
         layout_it != class_virtual_base_layouts.end() &&
         !layout_it->second.empty()) {
        out_layout.parameter_index = parameter_index;
        out_layout.layout = normalize_parameter_virtual_base_layout(layout_it->second);
        return true;
      }
    }

    ++parameter_index;
  }
  return false;
}

bool infer_function_type_reference_parameter_virtual_base_layout(
    const TypePtr & function_type,
    const map<string, vector<pair<string, unsigned long long> > > & class_virtual_base_layouts,
    ParameterVirtualBaseLayout & out_layout)
{
  TypePtr base = strip_top_level_cv(function_type);
  if(!base || base->kind != Type::TK_FUNCTION) {
    return false;
  }

  for(size_t i = 0; i < base->params.size(); ++i) {
    const TypePtr parameter_type = base->params[i];
    if(!is_reference_type(parameter_type)) {
      continue;
    }

    const string parameter_class =
        class_qualified_name(strip_top_level_cv(remove_reference_type(parameter_type)));
    map<string, vector<pair<string, unsigned long long> > >::const_iterator layout_it =
        class_virtual_base_layouts.find(parameter_class);
    if(!parameter_class.empty() &&
       layout_it != class_virtual_base_layouts.end() &&
       !layout_it->second.empty()) {
      out_layout.parameter_index = i;
      out_layout.layout = normalize_parameter_virtual_base_layout(layout_it->second);
      return true;
    }
  }

  return false;
}

bool infer_reference_storage_parameter_virtual_base_layout(
    const CallSemNode & function_node,
    const map<string, vector<pair<string, unsigned long long> > > & class_virtual_base_layouts,
    ParameterVirtualBaseLayout & out_layout)
{
  map<string, size_t> parameter_indices;
  size_t parameter_index = 0;
  for(size_t i = 0; i < function_node.children.size(); ++i) {
    if(function_node.children[i].kind == CallSemKind::parameter &&
       !function_node.children[i].text.empty()) {
      parameter_indices[function_node.children[i].text] = parameter_index;
    }
    if(function_node.children[i].kind == CallSemKind::parameter) {
      ++parameter_index;
    }
  }
  if(parameter_indices.empty()) {
    return false;
  }

  auto try_infer_layout = [&](const TypePtr & target_type,
                              const CallSemNode & source_node) -> bool
  {
    const string target_class = class_qualified_name(target_type);
    map<string, vector<pair<string, unsigned long long> > >::const_iterator layout_it =
        class_virtual_base_layouts.find(target_class);
    const CallSemNode * root = peel_base_subobject_root_shared(source_node);
    map<string, size_t>::const_iterator parameter_it =
        root ? parameter_indices.find(root->text) : parameter_indices.end();
    if(target_class.empty() ||
       layout_it == class_virtual_base_layouts.end() ||
       layout_it->second.empty() ||
       !root ||
       (root->kind != CallSemKind::variable &&
        root->kind != CallSemKind::id_expression &&
        root->kind != CallSemKind::parameter) ||
       parameter_it == parameter_indices.end()) {
      return false;
    }

    out_layout.parameter_index = parameter_it->second;
    out_layout.layout = normalize_parameter_virtual_base_layout(layout_it->second);
    return true;
  };

  vector<const CallSemNode *> stack(1, &function_node);
  while(!stack.empty()) {
    const CallSemNode * current = stack.back();
    stack.pop_back();

    if(current->kind == CallSemKind::assignment_expression &&
       current->children.size() == 2 &&
       current->children[0].is_reference_storage_target &&
       current->children[1].kind == CallSemKind::unary_expression &&
       current->children[1].children.size() == 1 &&
       callsem_has_token(current->children[1], OP_AMP)) {
      TypePtr target_type = strip_top_level_cv(current->children[0].semantic_type);
      target_type =
          target_type && target_type->kind == Type::TK_POINTER ?
              strip_top_level_cv(target_type->inner) :
              TypePtr();
      if(try_infer_layout(target_type, current->children[1].children[0])) {
        return true;
      }
    }

    if(current->kind == CallSemKind::constructor_action &&
       current->trivial_lifecycle &&
       current->children.size() == 1 &&
       current->children[0].kind == CallSemKind::call_expression &&
       current->children[0].children.size() == 3) {
      const CallSemNode & call = current->children[0];
      const CallSemNode & target_arg = call.children[1];
      const CallSemNode & source_arg = call.children[2];
      TypePtr target_type = strip_top_level_cv(target_arg.semantic_type);
      target_type =
          target_type && target_type->kind == Type::TK_POINTER ?
              strip_top_level_cv(target_type->inner) :
              TypePtr();
      if(is_reference_type(target_type) &&
         try_infer_layout(strip_top_level_cv(remove_reference_type(target_type)), source_arg)) {
        return true;
      }
    }

    if(current->kind == CallSemKind::closure_object) {
      for(size_t i = 0; i < current->children.size(); ++i) {
        const CallSemNode & capture = current->children[i];
        if(capture.kind != CallSemKind::closure_capture ||
           capture.children.size() != 1 ||
           !is_reference_type(capture.semantic_type)) {
          continue;
        }
        TypePtr target_type = strip_top_level_cv(remove_reference_type(capture.semantic_type));
        if(try_infer_layout(target_type, capture.children[0])) {
          return true;
        }
      }
    }

    for(size_t i = 0; i < current->children.size(); ++i) {
      stack.push_back(&current->children[i]);
    }
  }

  return false;
}

string stable_function_type_key(const TypePtr & type);

void append_stable_function_type_key(std::ostringstream & out, const TypePtr & type)
{
  if(!type) {
    out << "<null>";
    return;
  }

  out << 'K' << static_cast<int>(type->kind) << ':';
  switch(type->kind) {
  case Type::TK_FUNDAMENTAL:
    out << static_cast<int>(type->fundamental);
    return;

  case Type::TK_NAMED:
    out << type->named_key;
    return;

  case Type::TK_CV:
    out << (type->cv_const ? 'C' : '_')
        << (type->cv_volatile ? 'V' : '_')
        << '{';
    append_stable_function_type_key(out, type->inner);
    out << '}';
    return;

  case Type::TK_ATOMIC:
  case Type::TK_POINTER:
  case Type::TK_BLOCK_POINTER:
  case Type::TK_LVALUE_REFERENCE:
  case Type::TK_RVALUE_REFERENCE:
    out << '{';
    append_stable_function_type_key(out, type->inner);
    out << '}';
    return;

  case Type::TK_MEMBER_POINTER:
    out << '{';
    append_stable_function_type_key(out, type->owner);
    out << "->";
    append_stable_function_type_key(out, type->inner);
    out << '}';
    return;

  case Type::TK_ARRAY:
    out << '['
        << (type->has_bound ? (type->bound_text.empty() ? std::to_string(type->bound) :
                                                      type->bound_text) :
                              std::string("?"))
        << "]{";
    append_stable_function_type_key(out, type->inner);
    out << '}';
    return;

  case Type::TK_FUNCTION:
    out << '(';
    for(size_t i = 0; i < type->params.size(); ++i) {
      if(i != 0) {
        out << ',';
      }
      append_stable_function_type_key(out, type->params[i]);
    }
    out << ")->";
    append_stable_function_type_key(out, type->inner);
    if(type->variadic) {
      out << "...";
    }
    if(type->prototype_relaxed) {
      out << ";relaxed";
    }
    if(type->function_const) {
      out << ";const";
    }
    if(type->function_volatile) {
      out << ";volatile";
    }
    return;
  }

  out << "<unknown>";
}

string stable_function_type_key(const TypePtr & type)
{
  std::ostringstream out;
  append_stable_function_type_key(out, type);
  return out.str();
}

string function_key(const string & name, const TypePtr & type)
{
  return name + " " + stable_function_type_key(type);
}

string compact_lookup_text(const string & text)
{
  string out;
  out.reserve(text.size());
  for(size_t i = 0; i < text.size(); ++i) {
    const char ch = text[i];
    if(ch == ' ' || ch == '\t' || ch == '\n' || ch == '\r') {
      continue;
    }
    out.push_back(ch);
  }
  return out;
}

string simple_lookup_name(const string & text)
{
  const size_t split = text.rfind("::");
  return split == string::npos ? text : text.substr(split + 2);
}

struct FunctionSymbolLookupIndex
{
  vector<string> entry_type_keys;
  unordered_map<string, vector<size_t> > entries_by_name;
  unordered_map<string, vector<size_t> > entries_by_compact_name;
  unordered_map<string, vector<size_t> > entries_by_simple_name;
  unordered_set<string> mapped_symbols;

  void rebuild(const map<string, string> & function_symbols,
               const vector<FunctionSymbolEntry> & entries)
  {
    entry_type_keys.clear();
    entries_by_name.clear();
    entries_by_compact_name.clear();
    entries_by_simple_name.clear();
    mapped_symbols.clear();

    entry_type_keys.reserve(entries.size());
    entries_by_name.reserve(entries.size());
    entries_by_compact_name.reserve(entries.size());
    entries_by_simple_name.reserve(entries.size());
    mapped_symbols.reserve(function_symbols.size());

    for(map<string, string>::const_iterator it = function_symbols.begin();
        it != function_symbols.end();
        ++it) {
      mapped_symbols.insert(it->second);
    }

    for(size_t i = 0; i < entries.size(); ++i) {
      const string compact_name = compact_lookup_text(entries[i].name);
      entry_type_keys.push_back(stable_function_type_key(entries[i].type));
      entries_by_name[entries[i].name].push_back(i);
      entries_by_compact_name[compact_name].push_back(i);
      entries_by_simple_name[simple_lookup_name(compact_name)].push_back(i);
    }
  }
};

void append_unique_entry_indices(vector<size_t> & out,
                                 const vector<size_t> * candidates)
{
  if(!candidates) {
    return;
  }
  for(size_t i = 0; i < candidates->size(); ++i) {
    const size_t candidate = (*candidates)[i];
    if(find(out.begin(), out.end(), candidate) == out.end()) {
      out.push_back(candidate);
    }
  }
}

const vector<size_t> * find_entry_indices(
    const unordered_map<string, vector<size_t> > & index,
    const string & key)
{
  unordered_map<string, vector<size_t> >::const_iterator found = index.find(key);
  return found == index.end() ? nullptr : &found->second;
}

string try_lookup_function_symbol_from_entries(
    const vector<FunctionSymbolEntry> & entries,
    const FunctionSymbolLookupIndex & index,
    const vector<size_t> & candidates,
    const TypePtr & type,
    const string & type_key)
{
  for(size_t i = 0; i < candidates.size(); ++i) {
    const size_t entry_index = candidates[i];
    if(entry_index >= entries.size()) {
      continue;
    }
    const FunctionSymbolEntry & entry = entries[entry_index];
    const string & entry_type_key =
        entry_index < index.entry_type_keys.size() ?
            index.entry_type_keys[entry_index] :
            stable_function_type_key(entry.type);
    if(type_equals(entry.type, type) || entry_type_key == type_key) {
      return entry.symbol;
    }
  }
  return string();
}

string try_lookup_function_symbol_with_index(
    const map<string, string> & function_symbols,
    const vector<FunctionSymbolEntry> & function_symbol_entries,
    const FunctionSymbolLookupIndex & index,
    const string & name,
    const TypePtr & type)
{
  const string type_key = stable_function_type_key(type);
  map<string, string>::const_iterator found =
      function_symbols.find(name + " " + type_key);
  if(found != function_symbols.end()) {
    return found->second;
  }

  vector<size_t> candidates;
  append_unique_entry_indices(
      candidates,
      find_entry_indices(index.entries_by_name, name));
  string symbol = try_lookup_function_symbol_from_entries(
      function_symbol_entries, index, candidates, type, type_key);
  if(!symbol.empty()) {
    return symbol;
  }

  const string compact_name = compact_lookup_text(name);
  append_unique_entry_indices(
      candidates,
      find_entry_indices(index.entries_by_compact_name, compact_name));
  append_unique_entry_indices(
      candidates,
      find_entry_indices(index.entries_by_simple_name, simple_lookup_name(compact_name)));
  return try_lookup_function_symbol_from_entries(
      function_symbol_entries, index, candidates, type, type_key);
}

bool special_member_lookup_name_matches(const string & entry_name,
                                        const string & target_name)
{
  if(entry_name == target_name) {
    return true;
  }

  const string compact_entry = compact_lookup_text(entry_name);
  const string compact_target = compact_lookup_text(target_name);
  return compact_entry == compact_target ||
         simple_lookup_name(compact_entry) == simple_lookup_name(compact_target);
}

template<typename Matcher>
string try_lookup_special_member_symbol_by_index(
    const vector<FunctionSymbolEntry> & entries,
    const FunctionSymbolLookupIndex & index,
    const string & name,
    const Matcher & matches)
{
  vector<size_t> candidates;
  append_unique_entry_indices(
      candidates,
      find_entry_indices(index.entries_by_name, name));
  const string compact_name = compact_lookup_text(name);
  append_unique_entry_indices(
      candidates,
      find_entry_indices(index.entries_by_compact_name, compact_name));
  append_unique_entry_indices(
      candidates,
      find_entry_indices(index.entries_by_simple_name, simple_lookup_name(compact_name)));

  for(size_t i = 0; i < candidates.size(); ++i) {
    const size_t entry_index = candidates[i];
    if(entry_index < entries.size() && matches(entries[entry_index].type)) {
      return entries[entry_index].symbol;
    }
  }
  return string();
}

bool same_class_pointer_parameter_for_lowir(const TypePtr & class_type,
                                            const TypePtr & param_type)
{
  TypePtr base = strip_top_level_cv(param_type);
  return base && base->kind == Type::TK_POINTER &&
         semantic_conversion::same_type_with_compatible_top_cv(base->inner, class_type);
}

bool same_class_reference_parameter_for_lowir(const TypePtr & class_type,
                                              const TypePtr & param_type,
                                              Type::Kind ref_kind)
{
  TypePtr base = strip_top_level_cv(param_type);
  return base && base->kind == ref_kind &&
         semantic_conversion::same_type_with_compatible_top_cv(base->inner, class_type);
}

bool matches_constructor_entry_type_for_lowir(const TypePtr & entry_type,
                                              const TypePtr & class_type,
                                              Type::Kind ref_kind)
{
  TypePtr base = strip_top_level_cv(entry_type);
  if(!base || base->kind != Type::TK_FUNCTION || !is_void_type(base->inner)) {
    return false;
  }
  if(base->params.size() == 1) {
    return same_class_reference_parameter_for_lowir(class_type, base->params[0], ref_kind);
  }
  for(size_t i = 1; i < base->params.size(); ++i) {
    if(same_class_reference_parameter_for_lowir(class_type, base->params[i], ref_kind)) {
      return true;
    }
  }
  return false;
}

bool matches_destructor_entry_type_for_lowir(const TypePtr & entry_type,
                                             const TypePtr & class_type)
{
  TypePtr base = strip_top_level_cv(entry_type);
  if(!base || base->kind != Type::TK_FUNCTION || !is_void_type(base->inner)) {
    return false;
  }
  if(base->params.empty()) {
    return true;
  }
  return base->params.size() == 1 &&
         same_class_pointer_parameter_for_lowir(class_type, base->params[0]);
}

template<typename Matcher>
string try_lookup_special_member_symbol_by_entry(const vector<FunctionSymbolEntry> & entries,
                                                 const string & name,
                                                 const Matcher & matches)
{
  for(size_t i = 0; i < entries.size(); ++i) {
    if(!special_member_lookup_name_matches(entries[i].name, name)) {
      continue;
    }
    if(matches(entries[i].type)) {
      return entries[i].symbol;
    }
  }
  return string();
}

symbol_linkage::SymbolIdentity derive_vtable_entry_symbol_identity_for_name(
    const CallSemNode & node,
    const string & qualified_name);

symbol_linkage::FunctionSymbolOptions vtable_entry_function_symbol_options(
    const CallSemNode & node)
{
  symbol_linkage::FunctionSymbolOptions options;
  options.is_member_function = true;
  options.has_implicit_object_parameter = true;
  options.is_const_method = node.is_const_method;
  options.is_volatile_method = node.is_volatile_method;
  options.ref_qualifier = node.has_function_ref_qualifier ?
      callsem_function_ref_qualifier(node) :
      symbol_linkage::FRQ_NONE;
  options.is_constructor = node.is_constructor;
  options.is_destructor = node.is_destructor;
  options.abi_tags = callsem_abi_tags(node);
  if(node.has_special_member_entry_point_kind) {
    options.special_member_entry_point_kind =
        callsem_special_member_entry_point_kind(node);
  }
  TypePtr function_type = strip_top_level_cv(node.semantic_type);
  if(function_type && function_type->kind == Type::TK_FUNCTION &&
     !node.is_const_method && !node.is_volatile_method &&
     !node.has_function_ref_qualifier) {
    options.is_const_method = function_type->function_const;
    options.is_volatile_method = function_type->function_volatile;
  }
  return options;
}

symbol_linkage::SymbolIdentity derive_vtable_entry_symbol_identity(const CallSemNode & node)
{
  return derive_vtable_entry_symbol_identity_for_name(
      node,
      callsem_resolved_name(node).empty() ? node.text.str() :
          callsem_resolved_name(node));
}

symbol_linkage::SymbolIdentity derive_vtable_entry_symbol_identity_for_name(
    const CallSemNode & node,
    const string & qualified_name)
{
  if(symbol_linkage::has_exported_object_symbol(callsem_symbol(node)) ||
     qualified_name.empty() ||
     callsem_symbol(node).linkage == symbol_linkage::SL_INTERNAL) {
    return callsem_symbol(node);
  }

  symbol_linkage::FunctionSymbolOptions options =
      vtable_entry_function_symbol_options(node);

  const string display_name = simple_lookup_name(qualified_name);
  if(!callsem_qualified_name_syntax(node) && !node.is_c_linkage) {
    return callsem_symbol(node);
  }
  symbol_linkage::SymbolIdentity derived =
      callsem_qualified_name_syntax(node) ?
          symbol_linkage::make_function_symbol_identity(*callsem_qualified_name_syntax(node),
                                                        display_name,
                                                        node.is_c_linkage,
                                                        node.semantic_type,
                                                        options) :
          symbol_linkage::make_c_function_symbol_identity(display_name);
  if(derived.object_symbol.empty()) {
    return callsem_symbol(node);
  }

  derived.internal_symbol = callsem_symbol(node).internal_symbol.empty() ? lowir_name(qualified_name)
                                                                : callsem_symbol(node).internal_symbol;
  derived.keep_internal_alias = callsem_symbol(node).keep_internal_alias;
  derived.linkage = callsem_symbol(node).linkage;
  return derived;
}

string lowir_block_name(const string & label)
{
  return string("^") + label;
}

string summarize_lowir_special_member_candidates(
    const vector<FunctionSymbolEntry> & entries,
    const string & target_name,
    size_t limit = 4)
{
  ostringstream out;
  size_t matched = 0;
  for(size_t i = 0; i < entries.size(); ++i) {
    if(!special_member_lookup_name_matches(entries[i].name, target_name)) {
      continue;
    }
    if(matched != 0) {
      out << "; ";
    }
    out << entries[i].name << " :: " << describe_type(entries[i].type)
        << " => " << entries[i].symbol;
    ++matched;
    if(matched >= limit) {
      break;
    }
  }
  if(matched == 0) {
    return string("<none>");
  }
  return out.str();
}

string lowir_temp_name(size_t index)
{
  ostringstream out;
  out << "%t" << index;
  return out.str();
}

string lowir_slot_name(const string & name)
{
  return string("$") + name;
}

string lowir_hidden_slot_name(const string & prefix, size_t index)
{
  ostringstream out;
  out << "$" << prefix << "__" << index;
  return out.str();
}

size_t backend_storage_size(const TypePtr & type)
{
  TypePtr base = strip_top_level_cv(type);
  if(!base) {
    throw logic_error("missing backend type");
  }
  if(base->kind == Type::TK_ARRAY) {
    if(!base->has_bound) {
      throw logic_error("backend array requires bound");
    }
    return base->bound * backend_storage_size(base->inner);
  }
  if(base->kind == Type::TK_NAMED) {
    if(!base->named_has_layout) {
      throw logic_error("backend named type requires layout");
    }
    return base->named_size;
  }
  if(base->kind == Type::TK_FUNCTION) {
    throw logic_error("function object storage unsupported");
  }
  return type_size(type);
}

size_t backend_storage_alignment(const TypePtr & type)
{
  TypePtr base = strip_top_level_cv(type);
  if(!base) {
    throw logic_error("missing backend type");
  }
  if(base->kind == Type::TK_ARRAY) {
    return backend_storage_alignment(base->inner);
  }
  return type_alignment(base);
}

string storage_span_text(const TypePtr & type)
{
  ostringstream out;
  out << backend_storage_size(type) << "x" << backend_storage_alignment(type);
  return out.str();
}

string lowir_storage_type_for_span(size_t bytes, size_t alignment)
{
  ostringstream out;
  out << "obj<" << bytes << "x" << max<size_t>(1, min<size_t>(16, alignment)) << ">";
  return out.str();
}

string lowir_storage_type_for(const TypePtr & type)
{
  return lowir_storage_type_for_span(backend_storage_size(type),
                                     backend_storage_alignment(type));
}

bool is_complete_class_value_type(const TypePtr & type)
{
  TypePtr base = strip_top_level_cv(type);
  if(!base || base->kind != Type::TK_NAMED || !base->named_has_layout) {
    return false;
  }
  return base->named_key.compare(0, 6, "class ") == 0 ||
         base->named_key.compare(0, 7, "struct ") == 0 ||
         base->named_key.compare(0, 6, "union ") == 0;
}

bool is_class_like_value_type(const TypePtr & type)
{
  TypePtr base = strip_top_level_cv(remove_reference_type(type));
  if(!base || base->kind != Type::TK_NAMED) {
    return false;
  }
  return base->named_key.compare(0, 6, "class ") == 0 ||
         base->named_key.compare(0, 7, "struct ") == 0 ||
         base->named_key.compare(0, 6, "union ") == 0;
}

bool is_synthetic_subobject_pointer_node(const CallSemNode & node)
{
  TypePtr base = strip_top_level_cv(node.semantic_type);
  return base &&
         base->kind == Type::TK_POINTER &&
         node.kind == CallSemKind::member_expression &&
         node.children.size() == 1 &&
         (node.value_category == CVC_PRVALUE ||
          node.is_base_subobject ||
          node.is_virtual_base_subobject ||
          !callsem_virtual_base_layout(node).empty()) &&
         !callsem_has_token(node, OP_ARROW) &&
         !node.is_reference_storage;
}

bool is_empty_class_storage_type(const TypePtr & type)
{
  TypePtr base = strip_top_level_cv(type);
  return is_complete_class_value_type(type) && base && base->named_is_empty;
}

bool is_gnu_complex_value_type(const TypePtr & type, TypePtr * component_type = nullptr)
{
  return semantic_builtins::is_gnu_complex_type(type, component_type);
}

bool is_indirect_value_type(const TypePtr & type)
{
  if(semantic_builtins::is_builtin_va_list_type(type)) {
    return true;
  }
  if(is_complete_class_value_type(type)) {
    return true;
  }
  if(is_gnu_complex_value_type(type)) {
    return true;
  }

  TypePtr base = strip_top_level_cv(type);
  if(!base || base->kind != Type::TK_FUNDAMENTAL) {
    return false;
  }
  if(base->fundamental == FT_FLOAT ||
     base->fundamental == FT_DOUBLE ||
     base->fundamental == FT_LONG_DOUBLE ||
     base->fundamental == FT_VOID ||
     base->fundamental == FT_NULLPTR_T) {
    return false;
  }
  if(base->fundamental == FT_INT128 || base->fundamental == FT_UINT128) {
    return false;
  }
  return type_size(type) > 8;
}

bool array_element_uses_storage_slots(const TypePtr & type)
{
  TypePtr base = strip_top_level_cv(type);
  return (base && base->kind == Type::TK_ARRAY) ||
         is_indirect_value_type(type);
}

bool is_member_function_pointer_type(const TypePtr & type)
{
  if(!type) {
    return false;
  }
  TypePtr base = strip_top_level_cv(remove_reference_type(type));
  return base && base->kind == Type::TK_MEMBER_POINTER && is_function_type(base->inner);
}

void append_member_function_pointer_global_data_items(vector<string> & data_items,
                                                      const string & symbol)
{
  data_items.push_back(string("ptr addr ") + symbol);
  data_items.push_back("i64 0");
}

bool is_named_enum_scalar_type(const TypePtr & type)
{
  TypePtr base = strip_top_level_cv(type);
  return base && base->kind == Type::TK_NAMED && base->named_has_layout &&
         base->named_key.compare(0, 5, "enum ") == 0;
}

bool is_nullptr_scalar_type(const TypePtr & type)
{
  TypePtr base = strip_top_level_cv(remove_reference_type(type));
  return base && base->kind == Type::TK_FUNDAMENTAL &&
         base->fundamental == FT_NULLPTR_T;
}

bool is_null_pointer_global_initializer(const CallSemNode & node)
{
  if(is_nullptr_scalar_type(node.semantic_type)) {
    return true;
  }
  if(!is_pointer_type(node.semantic_type) || node.kind != CallSemKind::literal) {
    return false;
  }
  if(node.has_int_value && callsem_int_value(node) == 0) {
    return true;
  }
  return node.has_uint_value && callsem_uint_value(node) == 0;
}

string class_qualified_name(const TypePtr & type)
{
  TypePtr base = strip_top_level_cv(type);
  if(!base || base->kind != Type::TK_NAMED) {
    return string();
  }
  static const char * const prefixes[] = {"class ", "struct ", "union "};
  for(size_t i = 0; i < sizeof(prefixes) / sizeof(prefixes[0]); ++i) {
    const string prefix = prefixes[i];
    if(base->named_key.compare(0, prefix.size(), prefix) == 0) {
      return base->named_key.substr(prefix.size());
    }
  }
  for(size_t i = 0; i < sizeof(prefixes) / sizeof(prefixes[0]); ++i) {
    const string prefix = prefixes[i];
    if(base->named_display.compare(0, prefix.size(), prefix) == 0) {
      return base->named_display.substr(prefix.size());
    }
  }
  return base->named_display;
}

string canonical_host_runtime_rtti_class_name(string qualified_name)
{
  const string abi_marker = "[abi:";
  const size_t abi_pos = qualified_name.find(abi_marker);
  if(abi_pos != string::npos) {
    qualified_name.erase(abi_pos);
  }
  const string libcxx_prefix = "std::__1::";
  if(qualified_name.compare(0, libcxx_prefix.size(), libcxx_prefix) == 0) {
    qualified_name = string("std::") + qualified_name.substr(libcxx_prefix.size());
  }
  const string libstdcxx_cxx11_prefix = "std::__cxx11::";
  if(qualified_name.compare(0,
                            libstdcxx_cxx11_prefix.size(),
                            libstdcxx_cxx11_prefix) == 0) {
    qualified_name =
        string("std::") + qualified_name.substr(libstdcxx_cxx11_prefix.size());
  }
  return qualified_name;
}

bool is_host_runtime_rtti_class_name(const string & input_name)
{
  const string qualified_name = canonical_host_runtime_rtti_class_name(input_name);
  return qualified_name == "std::exception" ||
         qualified_name == "std::bad_exception" ||
         qualified_name == "std::bad_alloc" ||
         qualified_name == "std::bad_array_new_length" ||
         qualified_name == "std::bad_cast" ||
         qualified_name == "std::bad_typeid" ||
         qualified_name == "std::logic_error" ||
         qualified_name == "std::runtime_error" ||
         qualified_name == "std::domain_error" ||
         qualified_name == "std::invalid_argument" ||
         qualified_name == "std::length_error" ||
         qualified_name == "std::out_of_range" ||
         qualified_name == "std::range_error" ||
         qualified_name == "std::overflow_error" ||
         qualified_name == "std::underflow_error" ||
         qualified_name == "std::bad_function_call" ||
         qualified_name == "std::system_error" ||
         qualified_name == "std::ios_base::failure";
}

bool is_host_runtime_rtti_class_type(const TypePtr & type)
{
  const string qualified_name = class_qualified_name(type);
  return is_host_runtime_rtti_class_name(qualified_name);
}

bool has_external_vtable_symbol_candidate_for_type(const TypePtr & semantic_type)
{
  return semantic_type &&
         symbol_linkage::has_external_vtable_symbol_candidate(semantic_type);
}

string function_object_class_qualified_name(const CallSemNode & function_node)
{
  for(size_t i = 0; i < function_node.children.size(); ++i) {
    const CallSemNode & child = function_node.children[i];
    if(child.kind != CallSemKind::parameter || child.text != "this" || !child.semantic_type) {
      continue;
    }
    TypePtr object_type = strip_top_level_cv(remove_reference_type(child.semantic_type));
    if(object_type && object_type->kind == Type::TK_POINTER) {
      object_type = strip_top_level_cv(object_type->inner);
    }
    return class_qualified_name(object_type);
  }
  const string name =
      !callsem_resolved_name(function_node).empty() ?
          callsem_resolved_name(function_node) :
          function_node.text.str();
  const size_t sep = name.rfind("::");
  if(sep != string::npos) {
    return name.substr(0, sep);
  }
  return string();
}

string class_constructor_name(const string & qualified)
{
  const string unqualified = semantic_utils::unqualified_member_name(qualified);
  string out;
  int angle_depth = 0;
  for(char ch : unqualified) {
    if(ch == '<' && angle_depth == 0) {
      break;
    }
    if(ch == '<') {
      ++angle_depth;
    } else if(ch == '>' && angle_depth > 0) {
      --angle_depth;
    }
    out.push_back(ch);
  }
  const string local_marker = "__local_";
  const size_t local_suffix = out.find(local_marker);
  if(local_suffix != string::npos) {
    bool all_digits = local_suffix + local_marker.size() < out.size();
    for(size_t i = local_suffix + local_marker.size(); i < out.size(); ++i) {
      if(!std::isdigit(static_cast<unsigned char>(out[i]))) {
        all_digits = false;
        break;
      }
    }
    if(all_digits) {
      out.erase(local_suffix);
    }
  }
  return out;
}

bool is_constructor_function_name(const string & qualified)
{
  const size_t split = qualified.rfind("::");
  if(split == string::npos) {
    return false;
  }
  const string unqualified = semantic_utils::unqualified_member_name(qualified);
  if(unqualified.empty() || unqualified[0] == '~') {
    return false;
  }
  return unqualified == class_constructor_name(qualified.substr(0, split));
}

bool is_destructor_function_name(const string & qualified)
{
  const size_t split = qualified.rfind("::");
  if(split == string::npos) {
    return false;
  }
  const string unqualified = semantic_utils::unqualified_member_name(qualified);
  if(unqualified.size() < 2 || unqualified[0] != '~') {
    return false;
  }
  return unqualified.substr(1) == class_constructor_name(qualified.substr(0, split));
}

string lowir_type_for(const TypePtr & type);

LowIRGlobal make_data_global(const string & name,
                             bool readonly = false,
                             bool thread_local_storage = false)
{
  LowIRGlobal global;
  global.kind = LowIRGlobal::LG_DATA;
  global.name = name;
  global.readonly = readonly;
  global.thread_local_storage = thread_local_storage;
  return global;
}

LowIRGlobal make_scalar_global(const string & name,
                               const string & type,
                               const string & value,
                               bool is_addr,
                               bool readonly = false,
                               bool thread_local_storage = false)
{
  LowIRGlobal global;
  global.kind = LowIRGlobal::LG_SCALAR;
  global.name = name;
  global.readonly = readonly;
  global.thread_local_storage = thread_local_storage;
  global.type = type;
  global.value = value;
  global.is_addr = is_addr;
  return global;
}

string lowir_memory_type_for(const TypePtr & type)
{
  TypePtr base = strip_top_level_cv(type);
  if(!base) {
    throw logic_error("missing memory type");
  }
  if(base->kind == Type::TK_POINTER ||
     base->kind == Type::TK_BLOCK_POINTER ||
     base->kind == Type::TK_FUNCTION ||
     base->kind == Type::TK_LVALUE_REFERENCE ||
     base->kind == Type::TK_RVALUE_REFERENCE) {
    return "ptr";
  }
  if(is_member_function_pointer_type(base)) {
    return "i128";
  }
  if(base->kind == Type::TK_FUNDAMENTAL && base->fundamental == FT_VOID) {
    return "void";
  }
  if(base->kind == Type::TK_FUNDAMENTAL) {
    if(base->fundamental == FT_FLOAT) {
      return "f32";
    }
    if(base->fundamental == FT_DOUBLE) {
      return "f64";
    }
    if(base->fundamental == FT_LONG_DOUBLE) {
      return "f80";
    }
  }
  const size_t width = type_size(type);
  const bool is_unsigned =
      base->kind == Type::TK_FUNDAMENTAL && is_unsigned_integral_type(base);
  if(width <= 1) {
    return is_unsigned ? "u8" : "i8";
  }
  if(width == 2) {
    return is_unsigned ? "u16" : "i16";
  }
  if(width == 4) {
    return is_unsigned ? "u32" : "i32";
  }
  if(width == 8) {
    return "i64";
  }
  if(width == 16) {
    return is_unsigned ? "u128" : "i128";
  }
  return "i64";
}

TypePtr lowir_parameter_type_for(const TypePtr & type)
{
  TypePtr lowered_param_type = type;
  TypePtr param_base = strip_top_level_cv(remove_reference_type(type));
  if(!is_reference_type(type) &&
     param_base &&
     (param_base->kind == Type::TK_ARRAY || param_base->kind == Type::TK_FUNCTION)) {
    lowered_param_type = lowir_value_conversion_type(type);
  }
  return lowered_param_type;
}

string lowir_integer_type_for_size(size_t width);

vector<string> lowir_direct_value_chunk_types(const TypePtr & type)
{
  vector<string> out;
  TypePtr base = strip_top_level_cv(remove_reference_type(type));
  if(!base || base->kind != Type::TK_NAMED || is_reference_type(type)) {
    return out;
  }
  for(size_t i = 0; i < base->named_host_abi_chunks.size(); ++i) {
    if(base->named_host_abi_chunks[i].kind != Type::HostAbiChunk::HC_INTEGER ||
       base->named_host_abi_chunks[i].size == 0 ||
       base->named_host_abi_chunks[i].size > 8) {
      out.clear();
      return out;
    }
    out.push_back(lowir_integer_type_for_size(base->named_host_abi_chunks[i].size));
  }
  return out;
}

string lowir_direct_object_type(const TypePtr & type)
{
  TypePtr base = strip_top_level_cv(remove_reference_type(type));
  if(!base || base->kind != Type::TK_NAMED || is_reference_type(type)) {
    return string();
  }
  const vector<string> direct_chunk_types = lowir_direct_value_chunk_types(type);
  if(direct_chunk_types.empty() || !base->named_has_layout || base->named_size == 0) {
    return string();
  }
  const size_t alignment = max<size_t>(1, min<size_t>(16, base->named_alignment));
  ostringstream out;
  out << "obj<" << base->named_size << "x" << alignment << ">";
  return out.str();
}

bool lowir_uses_indirect_result_boundary(const TypePtr & result_type)
{
  return lowir_direct_object_type(result_type).empty() &&
         is_indirect_value_type(result_type);
}

string lowir_result_type_text(const TypePtr & result_type)
{
  const string direct_object_type = lowir_direct_object_type(result_type);
  return !direct_object_type.empty() ? direct_object_type : lowir_type_for(result_type);
}

string lowir_object_span_text(const string & lowir_type)
{
  ostringstream out;
  out << lowir_internal::type_size(lowir_internal::LowType{lowir_type})
      << "x"
      << lowir_internal::type_alignment(lowir_internal::LowType{lowir_type});
  return out.str();
}

vector<string> lowir_parameter_abi_type_texts(const TypePtr & type)
{
  const TypePtr lowered_param_type = lowir_parameter_type_for(type);
  const string direct_object_type = lowir_direct_object_type(lowered_param_type);
  if(!direct_object_type.empty()) {
    return vector<string>(1, direct_object_type);
  }
  const vector<string> direct_chunk_types =
      lowir_direct_value_chunk_types(lowered_param_type);
  if(!is_reference_type(lowered_param_type) && !direct_chunk_types.empty()) {
    return direct_chunk_types;
  }
  vector<string> out;
  out.push_back(is_indirect_value_type(lowered_param_type) ?
                    "ptr" :
                    lowir_type_for(lowered_param_type));
  return out;
}

size_t lowir_physical_argument_index(const TypePtr & function_type,
                                     size_t logical_parameter_index)
{
  TypePtr base = strip_top_level_cv(function_type);
  if(!base || base->kind != Type::TK_FUNCTION) {
    return logical_parameter_index;
  }

  size_t out = lowir_uses_indirect_result_boundary(base->inner) ? 1 : 0;
  const size_t limit = min(logical_parameter_index, base->params.size());
  for(size_t i = 0; i < limit; ++i) {
    out += lowir_parameter_abi_type_texts(base->params[i]).size();
  }
  return out;
}

lowir_internal::ParamPassingMode lowir_parameter_passing_mode(const TypePtr & original_type,
                                                              const TypePtr & lowered_param_type)
{
  TypePtr param_base = strip_top_level_cv(remove_reference_type(original_type));
  if(is_reference_type(original_type)) {
    return lowir_internal::PPM_REFERENCE;
  }
  if(param_base &&
     (param_base->kind == Type::TK_ARRAY || param_base->kind == Type::TK_FUNCTION)) {
    return lowir_internal::PPM_DECAY;
  }
  if(is_indirect_value_type(lowered_param_type)) {
    return lowir_internal::PPM_BY_ADDRESS;
  }
  return lowir_internal::PPM_DIRECT;
}

struct LowIRFunctionSignatureText
{
  vector<LowIRParameterText> params;
  string return_type;
  lowir_internal::FunctionBoundaryMetadata boundary_metadata;
};

void merge_boundary_metadata(lowir_internal::FunctionBoundaryMetadata & dst,
                             const lowir_internal::FunctionBoundaryMetadata & src)
{
  if(dst.arity == lowir_internal::CAM_FIXED &&
     src.arity != lowir_internal::CAM_FIXED) {
    dst.arity = src.arity;
  }
  if(dst.effects == lowir_internal::CFXM_DEFAULT &&
     src.effects != lowir_internal::CFXM_DEFAULT) {
    dst.effects = src.effects;
  }
  if(dst.unwind == lowir_internal::CUM_DEFAULT &&
     src.unwind != lowir_internal::CUM_DEFAULT) {
    dst.unwind = src.unwind;
  }
  if(dst.returns == lowir_internal::CRM_DEFAULT &&
     src.returns != lowir_internal::CRM_DEFAULT) {
    dst.returns = src.returns;
  }
}

lowir_internal::FunctionBoundaryMetadata known_function_boundary_metadata(
    const string & symbol)
{
  lowir_internal::FunctionBoundaryMetadata out;
  switch(runtime_symbol_policy::classify(symbol).role) {
    case runtime_symbol_policy::RuntimeSymbolRole::eh_unhandled:
      out.effects = lowir_internal::CFXM_READNONE;
      out.unwind = lowir_internal::CUM_NO;
      out.returns = lowir_internal::CRM_NORETURN;
      break;
    case runtime_symbol_policy::RuntimeSymbolRole::eh_call_unexpected:
    case runtime_symbol_policy::RuntimeSymbolRole::eh_rethrow:
    case runtime_symbol_policy::RuntimeSymbolRole::eh_throw:
    case runtime_symbol_policy::RuntimeSymbolRole::eh_resume:
      out.returns = lowir_internal::CRM_NORETURN;
      break;
    case runtime_symbol_policy::RuntimeSymbolRole::builtin_memchr:
    case runtime_symbol_policy::RuntimeSymbolRole::builtin_memcmp:
    case runtime_symbol_policy::RuntimeSymbolRole::builtin_strcmp:
    case runtime_symbol_policy::RuntimeSymbolRole::builtin_strchr:
    case runtime_symbol_policy::RuntimeSymbolRole::builtin_strlen:
      out.effects = lowir_internal::CFXM_READONLY;
      out.unwind = lowir_internal::CUM_NO;
      break;
    case runtime_symbol_policy::RuntimeSymbolRole::builtin_bzero:
    case runtime_symbol_policy::RuntimeSymbolRole::builtin_memcpy:
    case runtime_symbol_policy::RuntimeSymbolRole::builtin_memmove:
      out.effects = lowir_internal::CFXM_READWRITE;
      out.unwind = lowir_internal::CUM_NO;
      break;
    case runtime_symbol_policy::RuntimeSymbolRole::builtin_expect:
    case runtime_symbol_policy::RuntimeSymbolRole::builtin_is_constant_evaluated:
      out.effects = lowir_internal::CFXM_READNONE;
      out.unwind = lowir_internal::CUM_NO;
      break;
    case runtime_symbol_policy::RuntimeSymbolRole::builtin_unreachable:
      out.effects = lowir_internal::CFXM_READNONE;
      out.unwind = lowir_internal::CUM_NO;
      out.returns = lowir_internal::CRM_NORETURN;
      break;
    case runtime_symbol_policy::RuntimeSymbolRole::builtin_operator_delete:
    case runtime_symbol_policy::RuntimeSymbolRole::builtin_operator_delete_sized:
    case runtime_symbol_policy::RuntimeSymbolRole::builtin_operator_delete_aligned:
    case runtime_symbol_policy::RuntimeSymbolRole::builtin_operator_delete_sized_aligned:
    case runtime_symbol_policy::RuntimeSymbolRole::builtin_operator_delete_array:
    case runtime_symbol_policy::RuntimeSymbolRole::builtin_operator_delete_array_sized:
    case runtime_symbol_policy::RuntimeSymbolRole::builtin_operator_delete_array_aligned:
    case runtime_symbol_policy::RuntimeSymbolRole::builtin_operator_delete_array_sized_aligned:
      out.unwind = lowir_internal::CUM_NO;
      break;
    default:
      break;
  }

  const string normalized = runtime_symbol_policy::normalize_lookup_name(symbol);
  if(normalized == "__builtin_memchr" ||
     normalized == "__builtin_memcmp" ||
     normalized == "__builtin_strcmp" ||
     normalized == "__builtin_strchr" ||
     normalized == "__builtin_strlen") {
    out.effects = lowir_internal::CFXM_READONLY;
    out.unwind = lowir_internal::CUM_NO;
  } else if(normalized == "__builtin_bzero" ||
            normalized == "__builtin_memcpy" ||
            normalized == "__builtin_memmove") {
    out.effects = lowir_internal::CFXM_READWRITE;
    out.unwind = lowir_internal::CUM_NO;
  } else if(normalized == "__builtin_expect" ||
            normalized == "__builtin_is_constant_evaluated") {
    out.effects = lowir_internal::CFXM_READNONE;
    out.unwind = lowir_internal::CUM_NO;
  } else if(normalized == "__builtin_unreachable") {
    out.effects = lowir_internal::CFXM_READNONE;
    out.unwind = lowir_internal::CUM_NO;
    out.returns = lowir_internal::CRM_NORETURN;
  }
  if(normalized == "abort" ||
     normalized == "std::terminate" ||
     normalized == kStdTerminateObjectSymbol ||
     normalized == kCppgmCallTerminateSupportSymbol ||
     normalized == "__cxa_pure_virtual") {
    out.effects = lowir_internal::CFXM_READNONE;
    out.unwind = lowir_internal::CUM_NO;
    out.returns = lowir_internal::CRM_NORETURN;
  }
  if(normalized == "__cxa_bad_cast" ||
     normalized == "__cxa_bad_typeid") {
    out.effects = lowir_internal::CFXM_READNONE;
    out.unwind = lowir_internal::CUM_MAY;
    out.returns = lowir_internal::CRM_NORETURN;
  }

  return out;
}

void apply_known_function_boundary_metadata(lowir_internal::FunctionBoundaryMetadata & boundary,
                                            const string & symbol)
{
  merge_boundary_metadata(boundary, known_function_boundary_metadata(symbol));
}

void apply_callsem_function_boundary_metadata(lowir_internal::FunctionBoundaryMetadata & boundary,
                                              const CallSemNode & node)
{
  if(node.is_explicit_nothrow || node.is_semantically_nothrow) {
    boundary.unwind = lowir_internal::CUM_NO;
  }
}

lowir_internal::ParamCaptureMode known_parameter_capture_mode(const string & symbol,
                                                              size_t param_index)
{
  switch(runtime_symbol_policy::classify(symbol).role) {
    case runtime_symbol_policy::RuntimeSymbolRole::builtin_bzero:
      return param_index == 0 ? lowir_internal::PCM_NOCAPTURE : lowir_internal::PCM_DEFAULT;
    case runtime_symbol_policy::RuntimeSymbolRole::builtin_memchr:
      return param_index == 0 ? lowir_internal::PCM_NOCAPTURE : lowir_internal::PCM_DEFAULT;
    case runtime_symbol_policy::RuntimeSymbolRole::builtin_memcmp:
    case runtime_symbol_policy::RuntimeSymbolRole::builtin_memcpy:
    case runtime_symbol_policy::RuntimeSymbolRole::builtin_memmove:
    case runtime_symbol_policy::RuntimeSymbolRole::builtin_strcmp:
    case runtime_symbol_policy::RuntimeSymbolRole::builtin_strchr:
      return param_index < 2 ? lowir_internal::PCM_NOCAPTURE : lowir_internal::PCM_DEFAULT;
    case runtime_symbol_policy::RuntimeSymbolRole::builtin_strlen:
    case runtime_symbol_policy::RuntimeSymbolRole::builtin_operator_delete:
    case runtime_symbol_policy::RuntimeSymbolRole::builtin_operator_delete_sized:
    case runtime_symbol_policy::RuntimeSymbolRole::builtin_operator_delete_aligned:
    case runtime_symbol_policy::RuntimeSymbolRole::builtin_operator_delete_sized_aligned:
    case runtime_symbol_policy::RuntimeSymbolRole::builtin_operator_delete_array:
    case runtime_symbol_policy::RuntimeSymbolRole::builtin_operator_delete_array_sized:
    case runtime_symbol_policy::RuntimeSymbolRole::builtin_operator_delete_array_aligned:
    case runtime_symbol_policy::RuntimeSymbolRole::builtin_operator_delete_array_sized_aligned:
      return param_index == 0 ? lowir_internal::PCM_NOCAPTURE : lowir_internal::PCM_DEFAULT;
    default:
      break;
  }

  const string normalized = runtime_symbol_policy::normalize_lookup_name(symbol);
  if(normalized == "__builtin_bzero" ||
     normalized == "__builtin_memchr") {
    return param_index == 0 ? lowir_internal::PCM_NOCAPTURE : lowir_internal::PCM_DEFAULT;
  }
  if(normalized == "__builtin_memcmp" ||
     normalized == "__builtin_memcpy" ||
     normalized == "__builtin_memmove" ||
     normalized == "__builtin_strcmp" ||
     normalized == "__builtin_strchr") {
    return param_index < 2 ? lowir_internal::PCM_NOCAPTURE : lowir_internal::PCM_DEFAULT;
  }
  if(normalized == "__builtin_strlen") {
    return param_index == 0 ? lowir_internal::PCM_NOCAPTURE : lowir_internal::PCM_DEFAULT;
  }
  return lowir_internal::PCM_DEFAULT;
}

lowir_internal::ParamAccessMode known_parameter_access_mode(const string & symbol,
                                                            size_t param_index)
{
  switch(runtime_symbol_policy::classify(symbol).role) {
    case runtime_symbol_policy::RuntimeSymbolRole::builtin_bzero:
      return param_index == 0 ? lowir_internal::PAM_WRITE : lowir_internal::PAM_DEFAULT;
    case runtime_symbol_policy::RuntimeSymbolRole::builtin_memchr:
      return param_index == 0 ? lowir_internal::PAM_READ : lowir_internal::PAM_DEFAULT;
    case runtime_symbol_policy::RuntimeSymbolRole::builtin_memcmp:
    case runtime_symbol_policy::RuntimeSymbolRole::builtin_strcmp:
    case runtime_symbol_policy::RuntimeSymbolRole::builtin_strchr:
      return param_index < 2 ? lowir_internal::PAM_READ : lowir_internal::PAM_DEFAULT;
    case runtime_symbol_policy::RuntimeSymbolRole::builtin_memcpy:
      if(param_index == 0) return lowir_internal::PAM_WRITE;
      if(param_index == 1) return lowir_internal::PAM_READ;
      return lowir_internal::PAM_DEFAULT;
    case runtime_symbol_policy::RuntimeSymbolRole::builtin_memmove:
      if(param_index == 0) return lowir_internal::PAM_READWRITE;
      if(param_index == 1) return lowir_internal::PAM_READ;
      return lowir_internal::PAM_DEFAULT;
    case runtime_symbol_policy::RuntimeSymbolRole::builtin_strlen:
      return param_index == 0 ? lowir_internal::PAM_READ : lowir_internal::PAM_DEFAULT;
    default:
      break;
  }

  const string normalized = runtime_symbol_policy::normalize_lookup_name(symbol);
  if(normalized == "__builtin_bzero") {
    return param_index == 0 ? lowir_internal::PAM_WRITE : lowir_internal::PAM_DEFAULT;
  }
  if(normalized == "__builtin_memchr") {
    return param_index == 0 ? lowir_internal::PAM_READ : lowir_internal::PAM_DEFAULT;
  }
  if(normalized == "__builtin_memcmp" ||
     normalized == "__builtin_strcmp" ||
     normalized == "__builtin_strchr") {
    return param_index < 2 ? lowir_internal::PAM_READ : lowir_internal::PAM_DEFAULT;
  }
  if(normalized == "__builtin_memcpy") {
    if(param_index == 0) return lowir_internal::PAM_WRITE;
    if(param_index == 1) return lowir_internal::PAM_READ;
    return lowir_internal::PAM_DEFAULT;
  }
  if(normalized == "__builtin_memmove") {
    if(param_index == 0) return lowir_internal::PAM_READWRITE;
    if(param_index == 1) return lowir_internal::PAM_READ;
    return lowir_internal::PAM_DEFAULT;
  }
  if(normalized == "__builtin_strlen") {
    return param_index == 0 ? lowir_internal::PAM_READ : lowir_internal::PAM_DEFAULT;
  }
  return lowir_internal::PAM_DEFAULT;
}

lowir_internal::ParamAliasMode known_parameter_alias_mode(const string & symbol,
                                                          size_t param_index)
{
  switch(runtime_symbol_policy::classify(symbol).role) {
    case runtime_symbol_policy::RuntimeSymbolRole::builtin_memcpy:
      return param_index < 2 ? lowir_internal::PALM_NOALIAS :
                               lowir_internal::PALM_DEFAULT;
    default:
      break;
  }

  const string normalized = runtime_symbol_policy::normalize_lookup_name(symbol);
  if(normalized == "__builtin_memcpy") {
    return param_index < 2 ? lowir_internal::PALM_NOALIAS :
                             lowir_internal::PALM_DEFAULT;
  }
  return lowir_internal::PALM_DEFAULT;
}

void apply_known_parameter_capture_metadata(LowIRParameterText & param,
                                            const string & symbol,
                                            size_t param_index)
{
  if(param.type != "ptr") {
    return;
  }
  param.metadata.capture = known_parameter_capture_mode(symbol, param_index);
}

void apply_known_parameter_access_metadata(LowIRParameterText & param,
                                           const string & symbol,
                                           size_t param_index)
{
  if(param.type != "ptr") {
    return;
  }
  param.metadata.access = known_parameter_access_mode(symbol, param_index);
}

void apply_known_parameter_alias_metadata(LowIRParameterText & param,
                                          const string & symbol,
                                          size_t param_index)
{
  if(param.type != "ptr") {
    return;
  }
  param.metadata.alias = known_parameter_alias_mode(symbol, param_index);
}

void append_parameter_metadata_suffix(ostringstream & out,
                                      const lowir_internal::ParameterMetadata & metadata)
{
  const bool has_pass = metadata.passing != lowir_internal::PPM_DIRECT;
  const bool has_capture = metadata.capture != lowir_internal::PCM_DEFAULT;
  const bool has_access = metadata.access != lowir_internal::PAM_DEFAULT;
  const bool has_alias = metadata.alias != lowir_internal::PALM_DEFAULT;
  if(!has_pass && !has_capture && !has_access && !has_alias) {
    return;
  }

  out << " [";
  bool first = true;
  const auto append_item =
      [&out, &first](const string & text)
      {
        if(!first) {
          out << ", ";
        }
        first = false;
        out << text;
      };
  if(has_pass) {
    append_item(string("pass=") +
                lowir_internal::param_passing_mode_text(metadata.passing));
  }
  if(has_capture) {
    append_item(string("capture=") +
                lowir_internal::param_capture_mode_text(metadata.capture));
  }
  if(has_access) {
    append_item(string("access=") +
                lowir_internal::param_access_mode_text(metadata.access));
  }
  if(has_alias) {
    append_item(string("alias=") +
                lowir_internal::param_alias_mode_text(metadata.alias));
  }
  out << "]";
}

void append_call_boundary_metadata_suffix(
    ostringstream & out,
    const lowir_internal::FunctionBoundaryMetadata & boundary)
{
  const bool has_arity = boundary.arity != lowir_internal::CAM_FIXED;
  const bool has_effects = boundary.effects != lowir_internal::CFXM_DEFAULT;
  const bool has_unwind = boundary.unwind != lowir_internal::CUM_DEFAULT;
  const bool has_return = boundary.returns != lowir_internal::CRM_DEFAULT;
  if(!has_arity && !has_effects && !has_unwind && !has_return) {
    return;
  }

  out << " [";
  bool first = true;
  const auto append_item =
      [&out, &first](const string & text)
      {
        if(!first) {
          out << ", ";
        }
        first = false;
        out << text;
      };
  if(has_arity) {
    append_item(string("arity=") +
                lowir_internal::call_arity_mode_text(boundary.arity));
  }
  if(has_effects) {
    append_item(string("effects=") +
                lowir_internal::call_effects_mode_text(boundary.effects));
  }
  if(has_unwind) {
    append_item(string("unwind=") +
                lowir_internal::call_unwind_mode_text(boundary.unwind));
  }
  if(has_return) {
    append_item(string("return=") +
                lowir_internal::call_return_mode_text(boundary.returns));
  }
  out << "]";
}

lowir_internal::CallArityMode lowir_call_arity_for(const TypePtr & function_type)
{
  TypePtr base = strip_top_level_cv(function_type);
  if(!base || base->kind != Type::TK_FUNCTION) {
    throw logic_error("LowIR call arity requires function type");
  }
  if(base->prototype_relaxed) {
    return lowir_internal::CAM_PROTOTYPE_RELAXED;
  }
  if(base->variadic) {
    return lowir_internal::CAM_VARIADIC;
  }
  return lowir_internal::CAM_FIXED;
}

LowIRFunctionSignatureText lowir_function_signature_text(
    const TypePtr & function_type,
    const string & symbol = string())
{
  LowIRFunctionSignatureText out;
  TypePtr base = strip_top_level_cv(function_type);
  if(!base || base->kind != Type::TK_FUNCTION) {
    throw logic_error("LowIR function signature requires function type");
  }
  out.boundary_metadata.arity = lowir_call_arity_for(base);
  apply_known_function_boundary_metadata(out.boundary_metadata, symbol);
  TypePtr result_type = base->inner;
  const bool indirect_result = lowir_uses_indirect_result_boundary(result_type);
  out.return_type = indirect_result ? "void" : lowir_result_type_text(result_type);
  if(indirect_result) {
    out.params.push_back(
        make_lowir_parameter_text("%ret", "ptr", lowir_internal::PPM_INDIRECT_RESULT));
  }
  for(size_t i = 0; i < base->params.size(); ++i) {
    const TypePtr lowered_param_type = lowir_parameter_type_for(base->params[i]);
    const vector<string> abi_types = lowir_parameter_abi_type_texts(base->params[i]);
    const string direct_object_type = lowir_direct_object_type(lowered_param_type);
    const lowir_internal::ParamPassingMode passing =
        !direct_object_type.empty() ?
            lowir_internal::PPM_DIRECT :
            lowir_parameter_passing_mode(base->params[i], lowered_param_type);
    for(size_t ai = 0; ai < abi_types.size(); ++ai) {
      ostringstream name;
      name << "%arg" << i;
      if(ai != 0) {
        name << "__" << ai;
      }
      LowIRParameterText param =
          make_lowir_parameter_text(name.str(), abi_types[ai], passing);
      apply_known_parameter_capture_metadata(param, symbol, i);
      apply_known_parameter_access_metadata(param, symbol, i);
      apply_known_parameter_alias_metadata(param, symbol, i);
      out.params.push_back(param);
    }
  }
  return out;
}

LowIRFunctionSignatureText lowir_function_signature_text_for_callsem_node(
    const CallSemNode & node,
    const string & symbol = string())
{
  LowIRFunctionSignatureText out;
  TypePtr base = strip_top_level_cv(node.semantic_type);
  if(!base || base->kind != Type::TK_FUNCTION) {
    throw logic_error("LowIR function signature requires function node type");
  }
  out.boundary_metadata.arity = lowir_call_arity_for(base);
  apply_known_function_boundary_metadata(out.boundary_metadata, symbol);
  TypePtr result_type = base->inner;
  const bool indirect_result = lowir_uses_indirect_result_boundary(result_type);
  out.return_type = indirect_result ? "void" : lowir_result_type_text(result_type);
  if(indirect_result) {
    out.params.push_back(
        make_lowir_parameter_text("%ret", "ptr", lowir_internal::PPM_INDIRECT_RESULT));
  }

  size_t param_index = 0;
  for(size_t i = 0; i < node.children.size(); ++i) {
    const CallSemNode & child = node.children[i];
    if(child.kind != CallSemKind::parameter) {
      continue;
    }
    const TypePtr lowered_param_type = lowir_parameter_type_for(child.semantic_type);
    const vector<string> abi_types = lowir_parameter_abi_type_texts(child.semantic_type);
    const string direct_object_type = lowir_direct_object_type(lowered_param_type);
    const lowir_internal::ParamPassingMode passing =
        !direct_object_type.empty() ?
            lowir_internal::PPM_DIRECT :
            lowir_parameter_passing_mode(child.semantic_type, lowered_param_type);
    for(size_t ai = 0; ai < abi_types.size(); ++ai) {
      ostringstream name;
      name << "%arg" << param_index;
      if(ai != 0) {
        name << "__" << ai;
      }
      LowIRParameterText param =
          make_lowir_parameter_text(name.str(), abi_types[ai], passing);
      apply_known_parameter_capture_metadata(param, symbol, param_index);
      apply_known_parameter_access_metadata(param, symbol, param_index);
      apply_known_parameter_alias_metadata(param, symbol, param_index);
      out.params.push_back(param);
    }
    ++param_index;
  }
  if(param_index == 0 && !base->params.empty()) {
    return lowir_function_signature_text(node.semantic_type, symbol);
  }
  return out;
}

void append_parameter_virtual_base_signature_params_for_layout(
    LowIRFunctionSignatureText & signature,
    const ParameterVirtualBaseLayout & layout)
{
  for(size_t i = 0; i < layout.layout.size(); ++i) {
    signature.params.push_back(
        make_lowir_parameter_text(string("%__pvbptr") + to_string(i), "ptr"));
  }
}

string lowir_call_signature_suffix(const LowIRFunctionSignatureText & signature)
{
  ostringstream out;
  out << " as (";
  for(size_t i = 0; i < signature.params.size(); ++i) {
    if(i != 0) {
      out << ", ";
    }
    out << signature.params[i].name << " : " << signature.params[i].type;
    append_parameter_metadata_suffix(out, signature.params[i].metadata);
  }
  out << ") -> " << signature.return_type;
  append_call_boundary_metadata_suffix(out, signature.boundary_metadata);
  return out.str();
}

string lowir_call_signature_suffix(const TypePtr & function_type)
{
  return lowir_call_signature_suffix(lowir_function_signature_text(function_type));
}

string lowir_integer_type_for_size(size_t width)
{
  if(width <= 1) {
    return "i8";
  }
  if(width == 2) {
    return "i16";
  }
  if(width == 4) {
    return "i32";
  }
  return "i64";
}

string format_global_address_operand(const string & symbol, long long addend)
{
  if(addend == 0) {
    return symbol;
  }
  return symbol + (addend > 0 ? " + " : " - ") + to_string(addend > 0 ? addend : -addend);
}

string lowir_lvalue_memory_type(const CallSemNode & node)
{
  TypePtr lvalue_type = node.semantic_type;
  if(is_reference_type(lvalue_type)) {
    lvalue_type = remove_reference_type(lvalue_type);
  }
  if(node.kind == CallSemKind::member_expression) {
    if(node.is_bit_field && callsem_bit_field_storage_size(node) != 0) {
      return lowir_integer_type_for_size(
          static_cast<size_t>(callsem_bit_field_storage_size(node)));
    }
    return lowir_memory_type_for(lvalue_type);
  }
  if(node.kind == CallSemKind::subscript_expression) {
    return lowir_memory_type_for(lvalue_type);
  }
  return lowir_memory_type_for(lvalue_type);
}

unsigned long long lowir_bit_field_mask(size_t bits)
{
  if(bits == 0) {
    return 0;
  }
  if(bits >= sizeof(unsigned long long) * 8) {
    return ~0ULL;
  }
  return (1ULL << bits) - 1ULL;
}

TypePtr lowir_value_conversion_type(const TypePtr & type)
{
  if(!type) {
    return TypePtr();
  }

  TypePtr base = remove_reference_type(type);
  if(!base) {
    base = type;
  }
  base = strip_top_level_cv(base);
  if(!base) {
    return TypePtr();
  }

  if(base->kind == Type::TK_ARRAY) {
    return make_pointer(base->inner);
  }
  if(base->kind == Type::TK_FUNCTION) {
    return make_pointer(base);
  }
  return base;
}

string lowir_type_for(const TypePtr & type)
{
  TypePtr base = strip_top_level_cv(type);
  if(!base) {
    throw logic_error("missing semantic type");
  }
  if(base->kind == Type::TK_LVALUE_REFERENCE || base->kind == Type::TK_RVALUE_REFERENCE) {
    return "ptr";
  }
  if(base->kind == Type::TK_POINTER ||
     base->kind == Type::TK_BLOCK_POINTER ||
     base->kind == Type::TK_FUNCTION) {
    return "ptr";
  }
  if(is_member_function_pointer_type(base)) {
    return "i128";
  }
  if(base->kind == Type::TK_ARRAY) {
    throw logic_error("array type has no direct LowIR scalar type [type " +
                      describe_type(type) + "]");
  }
  if(base->kind == Type::TK_ATOMIC) {
    return lowir_type_for(base->inner);
  }
  if(base->kind == Type::TK_NAMED) {
    if(is_named_enum_scalar_type(base)) {
      const size_t width = type_size(type);
      if(width <= 1) {
        return "i8";
      }
      if(width == 2) {
        return "i16";
      }
      if(width == 4) {
        return "i32";
      }
      if(width == 16) {
        return "i128";
      }
      return "i64";
    }
    throw logic_error(string("named type not supported in procedural LowIR [type ") +
                      describe_type(type) + "]" +
                      (g_lowir_current_function_node ?
                           string(" [function ") +
                               (g_lowir_current_function_node->text.empty() ?
                                    node_internal_symbol(*g_lowir_current_function_node) :
                                    g_lowir_current_function_node->text.str()) + "]" :
                           string()) +
                      (g_lowir_current_expr_node ?
                           string(" [expr-kind ") +
                               callsem_kind_text(g_lowir_current_expr_node->kind) + "]" +
                               (!g_lowir_current_expr_node->text.empty() ?
                                    string(" [expr-text ") + g_lowir_current_expr_node->text + "]" :
                                    string()) :
                           string()));
  }
  if(base->kind == Type::TK_FUNDAMENTAL && base->fundamental == FT_VOID) {
    return "void";
  }
  if(base->kind == Type::TK_FUNDAMENTAL) {
    if(base->fundamental == FT_FLOAT) {
      return "f32";
    }
    if(base->fundamental == FT_DOUBLE) {
      return "f64";
    }
    if(base->fundamental == FT_LONG_DOUBLE) {
      return "f80";
    }
  }
  const size_t width = type_size(type);
  const bool is_unsigned =
      base->kind == Type::TK_FUNDAMENTAL && is_unsigned_integral_type(base);
  if(width <= 1) {
    return is_unsigned ? "u8" : "i8";
  }
  if(width == 2) {
    return is_unsigned ? "u16" : "i16";
  }
  if(width == 4) {
    return is_unsigned ? "u32" : "i32";
  }
  if(width == 8) {
    return "i64";
  }
  if(width == 16) {
    return is_unsigned ? "u128" : "i128";
  }
  return "i64";
}

string lowir_value_type_for(const TypePtr & type)
{
  TypePtr value_type = lowir_value_conversion_type(type);
  return lowir_type_for(value_type ? value_type : type);
}

const CallSemNode * find_child(const CallSemNode & node, CallSemKind kind)
{
  for(size_t i = 0; i < node.children.size(); ++i) {
    if(node.children[i].kind == kind) {
      return &node.children[i];
    }
  }
  return nullptr;
}

void collect_value_return_statements(const CallSemNode & node,
                                     vector<const CallSemNode *> & out)
{
  if(node.kind == CallSemKind::function_definition) {
    return;
  }
  if(node.kind == CallSemKind::return_statement && !node.children.empty()) {
    out.push_back(&node);
    return;
  }
  for(size_t i = 0; i < node.children.size(); ++i) {
    collect_value_return_statements(node.children[i], out);
  }
}

bool contains_other_named_local_variable(const CallSemNode & node,
                                         const string & name,
                                         const CallSemNode * ignore)
{
  if(node.kind == CallSemKind::function_definition) {
    return false;
  }
  if(node.kind == CallSemKind::variable &&
     &node != ignore &&
     node.text == name) {
    return true;
  }
  for(size_t i = 0; i < node.children.size(); ++i) {
    if(contains_other_named_local_variable(node.children[i], name, ignore)) {
      return true;
    }
  }
  return false;
}

const CallSemNode * find_named_return_slot_variable(const CallSemNode & function_node,
                                                    const TypePtr & function_type)
{
  TypePtr base_function_type = strip_top_level_cv(function_type);
  if(!base_function_type || base_function_type->kind != Type::TK_FUNCTION) {
    return nullptr;
  }

  TypePtr return_type = base_function_type->inner;
  if(!return_type ||
     !lowir_uses_indirect_result_boundary(return_type) ||
     !is_complete_class_value_type(return_type)) {
    return nullptr;
  }

  const CallSemNode * body = find_child(function_node, CallSemKind::compound_statement);
  if(!body) {
    return nullptr;
  }

  vector<const CallSemNode *> returns;
  collect_value_return_statements(*body, returns);
  if(returns.empty()) {
    return nullptr;
  }

  string candidate_name;
  for(size_t i = 0; i < returns.size(); ++i) {
    const CallSemNode & expr = returns[i]->children[0];
    if(expr.kind != CallSemKind::id_expression ||
       expr.text.empty() ||
       !expr.implicit_return_move_eligible ||
       !semantic_conversion::same_type_with_compatible_top_cv(
           strip_top_level_cv(expr.semantic_type),
           strip_top_level_cv(return_type))) {
      return nullptr;
    }
    if(i == 0) {
      candidate_name = expr.text;
    } else if(expr.text != candidate_name) {
      return nullptr;
    }
  }

  const CallSemNode * candidate = nullptr;
  for(size_t i = 0; i < body->children.size(); ++i) {
    const CallSemNode & child = body->children[i];
    if(child.kind != CallSemKind::simple_declaration) {
      continue;
    }
    for(size_t j = 0; j < child.children.size(); ++j) {
      const CallSemNode & variable = child.children[j];
      if(variable.kind != CallSemKind::variable ||
         variable.text != candidate_name) {
        continue;
      }
      if(candidate != nullptr ||
         variable.is_static_storage ||
         is_reference_type(variable.semantic_type) ||
         !is_complete_class_value_type(variable.semantic_type) ||
         !semantic_conversion::same_type_with_compatible_top_cv(
             strip_top_level_cv(variable.semantic_type),
             strip_top_level_cv(return_type))) {
        return nullptr;
      }
      candidate = &variable;
    }
  }

  if(candidate == nullptr) {
    return nullptr;
  }

  if(contains_other_named_local_variable(*body, candidate_name, candidate)) {
    return nullptr;
  }

  return candidate;
}

class LowIRFunctionBuilder
{
public:
  LowIRFunctionBuilder(const CallSemNode & function_node,
                       const map<string, GlobalBinding> & globals,
                       const map<string, VTableBinding> & vtables,
                       const map<string, string> & function_symbols,
                       const vector<FunctionSymbolEntry> & function_symbol_entries,
                       const FunctionSymbolLookupIndex & function_symbol_lookup_index,
                       const map<string, const CallSemNode *> & function_symbol_nodes,
                       const set<string> & c_linkage_function_symbols,
                       const map<string, vector<pair<string, unsigned long long> > > &
                           function_virtual_base_layouts,
                       const map<string, vector<pair<string, unsigned long long> > > &
                           class_virtual_base_layouts,
                       const map<string, ParameterVirtualBaseLayout> &
                           function_parameter_virtual_base_layouts,
                       const set<string> & classes_with_virtual_functions,
                       const set<string> & throwing_function_symbols,
                       const set<string> & rtti_definition_symbols,
                       const map<string, string> & string_literal_symbols,
                       const map<string, TypePtr> & exception_storage_types,
                       map<string, VirtualMemberPointerThunkRequest> & virtual_member_pointer_thunks,
                       map<string, string> & external_function_symbols,
                       map<string, string> & external_object_symbols,
                       set<string> & runtime_bridge_support_symbols,
                       set<string> & referenced_function_symbols,
                       map<string, TypePtr> & referenced_function_signature_types,
                       map<string, set<string> > & function_references,
                       bool emit_runtime_support,
                       bool enable_debug_value_names)
    : function_node_(&function_node)
    , globals_(globals)
    , vtables_(vtables)
    , function_symbols_(function_symbols)
    , function_symbol_entries_(function_symbol_entries)
    , function_symbol_lookup_index_(function_symbol_lookup_index)
    , function_symbol_nodes_(function_symbol_nodes)
    , c_linkage_function_symbols_(c_linkage_function_symbols)
    , function_virtual_base_layouts_(function_virtual_base_layouts)
    , class_virtual_base_layouts_(class_virtual_base_layouts)
    , function_parameter_virtual_base_layouts_(function_parameter_virtual_base_layouts)
    , classes_with_virtual_functions_(classes_with_virtual_functions)
    , throwing_function_symbols_(throwing_function_symbols)
    , rtti_definition_symbols_(rtti_definition_symbols)
    , string_literal_symbols_(string_literal_symbols)
    , exception_storage_types_(exception_storage_types)
    , virtual_member_pointer_thunks_(virtual_member_pointer_thunks)
    , external_function_symbols_(external_function_symbols)
    , external_object_symbols_(external_object_symbols)
    , runtime_bridge_support_symbols_(runtime_bridge_support_symbols)
    , referenced_function_symbols_(referenced_function_symbols)
    , referenced_function_signature_types_(referenced_function_signature_types)
    , function_references_(function_references)
    , emit_runtime_support_(emit_runtime_support)
    , enable_debug_value_names_(enable_debug_value_names)
  {
    g_lowir_current_function_node = function_node_;
    function_.name = node_internal_symbol(*function_node_);
    function_.metadata.object_trivial_lifecycle =
        function_node_->object_trivial_lifecycle || function_node_->trivial_lifecycle;
    if(function_node_->has_source_location()) {
      function_.debug_location.file = callsem_source_file(*function_node_);
      function_.debug_location.line = callsem_source_line(*function_node_);
      function_.debug_location.column = callsem_source_column(*function_node_);
    }
    is_constructor_function_ = is_constructor_function_name(function_node_->text);
    TypePtr function_type = strip_top_level_cv(function_node_->semantic_type);
    if(!function_type || function_type->kind != Type::TK_FUNCTION) {
      throw logic_error("function-definition missing function type");
    }
    function_.boundary_metadata.arity = lowir_call_arity_for(function_type);
    apply_known_function_boundary_metadata(function_.boundary_metadata, function_.name);
    apply_callsem_function_boundary_metadata(function_.boundary_metadata, *function_node_);
    function_result_type_ = function_type->inner;
    direct_object_return_ = !lowir_direct_object_type(function_result_type_).empty();
    indirect_class_return_ =
        !direct_object_return_ && lowir_uses_indirect_result_boundary(function_result_type_);
    function_.return_type =
        indirect_class_return_ ? "void" : lowir_result_type_text(function_result_type_);
    if(indirect_class_return_) {
      function_.params.push_back(
          make_lowir_parameter_text("%ret", "ptr", lowir_internal::PPM_INDIRECT_RESULT));
    }

    for(size_t i = 0; i < function_node_->children.size(); ++i) {
      const CallSemNode & child = function_node_->children[i];
      if(child.kind == CallSemKind::parameter) {
        const string source_param_name =
            child.text.empty() ? string("__param") + to_string(param_names_.size()) :
                child.text.str();
        const string param_name = next_parameter_binding_name(source_param_name);
        const string param_temp = string("%") + param_name;
        TypePtr lowered_param_type = child.semantic_type;
        TypePtr param_base = strip_top_level_cv(remove_reference_type(child.semantic_type));
        if(!is_reference_type(child.semantic_type) &&
           param_base &&
           (param_base->kind == Type::TK_ARRAY || param_base->kind == Type::TK_FUNCTION)) {
          lowered_param_type = lowir_value_conversion_type(child.semantic_type);
        }
        ParamAbiPlan abi_plan;
        const string direct_object_type = lowir_direct_object_type(lowered_param_type);
        const lowir_internal::ParamPassingMode param_passing =
            !direct_object_type.empty() ?
                lowir_internal::PPM_DIRECT :
                lowir_parameter_passing_mode(child.semantic_type, lowered_param_type);
        if(!direct_object_type.empty()) {
          abi_plan.kind = ParamAbiPlan::PAK_DIRECT_OBJECT;
          function_.params.push_back(
              make_lowir_parameter_text(param_temp, direct_object_type, param_passing));
          abi_plan.inputs.push_back(make_pair(param_temp, direct_object_type));
        } else if(is_indirect_value_type(lowered_param_type)) {
          abi_plan.kind = ParamAbiPlan::PAK_INDIRECT;
          function_.params.push_back(
              make_lowir_parameter_text(param_temp, "ptr", param_passing));
          abi_plan.inputs.push_back(make_pair(param_temp, "ptr"));
        } else {
          abi_plan.kind = ParamAbiPlan::PAK_SCALAR;
          function_.params.push_back(
              make_lowir_parameter_text(param_temp,
                                        lowir_type_for(lowered_param_type),
                                        param_passing));
          abi_plan.inputs.push_back(make_pair(param_temp, lowir_type_for(lowered_param_type)));
        }
        VariableBinding binding =
            create_variable_binding(param_name, child.semantic_type, lowered_param_type);
        if(abi_plan.kind == ParamAbiPlan::PAK_INDIRECT) {
          binding.uses_external_storage_address = true;
          binding.external_storage_address = abi_plan.inputs[0].first;
        }
        binding.is_parameter = true;
        bindings_[param_name] = binding;
        param_names_.push_back(param_name);
        param_types_.push_back(lowered_param_type);
        param_abi_plans_.push_back(abi_plan);
      }
    }
    const vector<pair<string, unsigned long long> > * function_virtual_base_layout =
        resolve_current_function_virtual_base_layout();
    if(function_node_->uses_vtt_parameter) {
      current_vtt_param_ = "%__vtt";
      function_.params.push_back(make_lowir_parameter_text(current_vtt_param_, "ptr"));
    }
    if(function_node_->has_special_member_entry_point_kind &&
       callsem_special_member_entry_point_kind(*function_node_) ==
           symbol_linkage::SMEK_BASE) {
      for(size_t i = 0; i < function_virtual_base_layout->size(); ++i) {
        const string param_temp = string("%__vbptr") + to_string(i);
        function_.params.push_back(make_lowir_parameter_text(param_temp, "ptr"));
        hidden_virtual_base_params_[(*function_virtual_base_layout)[i].first] = param_temp;
      }
    } else if(!function_node_->has_special_member_entry_point_kind &&
              !function_virtual_base_layout->empty()) {
      for(size_t i = 0; i < function_virtual_base_layout->size(); ++i) {
        const string param_temp = string("%__vbptr") + to_string(i);
        function_.params.push_back(make_lowir_parameter_text(param_temp, "ptr"));
        hidden_virtual_base_params_[(*function_virtual_base_layout)[i].first] = param_temp;
      }
    }
    for(size_t i = 0; i < function_virtual_base_layout->size(); ++i) {
      current_virtual_base_offsets_[(*function_virtual_base_layout)[i].first] =
          (*function_virtual_base_layout)[i].second;
    }
    const string function_symbol = node_internal_symbol(*function_node_);
    map<string, ParameterVirtualBaseLayout>::const_iterator parameter_layout_it =
        function_parameter_virtual_base_layouts_.find(function_symbol);
    if(parameter_layout_it != function_parameter_virtual_base_layouts_.end()) {
      for(size_t i = 0; i < parameter_layout_it->second.layout.size(); ++i) {
        const string param_temp = string("%__pvbptr") + to_string(i);
        function_.params.push_back(make_lowir_parameter_text(param_temp, "ptr"));
        parameter_hidden_virtual_base_params_[parameter_layout_it->second.layout[i].first] =
            param_temp;
      }
    }
  }

  LowIRFunctionBuilder(const string & qualified_name,
                       const map<string, GlobalBinding> & globals,
                       const map<string, VTableBinding> & vtables,
                       const map<string, string> & function_symbols,
                       const vector<FunctionSymbolEntry> & function_symbol_entries,
                       const FunctionSymbolLookupIndex & function_symbol_lookup_index,
                       const map<string, const CallSemNode *> & function_symbol_nodes,
                       const set<string> & c_linkage_function_symbols,
                       const map<string, vector<pair<string, unsigned long long> > > &
                           function_virtual_base_layouts,
                       const map<string, vector<pair<string, unsigned long long> > > &
                           class_virtual_base_layouts,
                       const map<string, ParameterVirtualBaseLayout> &
                           function_parameter_virtual_base_layouts,
                       const set<string> & classes_with_virtual_functions,
                       const set<string> & throwing_function_symbols,
                       const set<string> & rtti_definition_symbols,
                       const map<string, string> & string_literal_symbols,
                       const map<string, TypePtr> & exception_storage_types,
                       map<string, VirtualMemberPointerThunkRequest> & virtual_member_pointer_thunks,
                       map<string, string> & external_function_symbols,
                       map<string, string> & external_object_symbols,
                       set<string> & runtime_bridge_support_symbols,
                       set<string> & referenced_function_symbols,
                       map<string, TypePtr> & referenced_function_signature_types,
                       map<string, set<string> > & function_references,
                       bool emit_runtime_support,
                       bool enable_debug_value_names)
    : function_node_(nullptr)
    , globals_(globals)
    , vtables_(vtables)
    , function_symbols_(function_symbols)
    , function_symbol_entries_(function_symbol_entries)
    , function_symbol_lookup_index_(function_symbol_lookup_index)
    , function_symbol_nodes_(function_symbol_nodes)
    , c_linkage_function_symbols_(c_linkage_function_symbols)
    , function_virtual_base_layouts_(function_virtual_base_layouts)
    , class_virtual_base_layouts_(class_virtual_base_layouts)
    , function_parameter_virtual_base_layouts_(function_parameter_virtual_base_layouts)
    , classes_with_virtual_functions_(classes_with_virtual_functions)
    , throwing_function_symbols_(throwing_function_symbols)
    , rtti_definition_symbols_(rtti_definition_symbols)
    , string_literal_symbols_(string_literal_symbols)
    , exception_storage_types_(exception_storage_types)
    , virtual_member_pointer_thunks_(virtual_member_pointer_thunks)
    , external_function_symbols_(external_function_symbols)
    , external_object_symbols_(external_object_symbols)
    , runtime_bridge_support_symbols_(runtime_bridge_support_symbols)
    , referenced_function_symbols_(referenced_function_symbols)
    , referenced_function_signature_types_(referenced_function_signature_types)
    , function_references_(function_references)
    , emit_runtime_support_(emit_runtime_support)
    , enable_debug_value_names_(enable_debug_value_names)
  {
    g_lowir_current_function_node = nullptr;
    function_.name = (!qualified_name.empty() && qualified_name[0] == '@') ?
                         qualified_name :
                         lowir_name(qualified_name);
    function_.return_type = "void";
  }

  LowIRFunction build()
  {
    if(!function_node_) {
      throw logic_error("synthetic LowIR builder requires explicit body");
    }
    start_block("entry");
    push_cleanup_scope();
    push_binding_scope();
    for(size_t i = 0; i < param_names_.size(); ++i) {
      const VariableBinding & binding = bindings_.find(param_names_[i])->second;
      const ParamAbiPlan & abi_plan = param_abi_plans_[i];
      if(abi_plan.kind == ParamAbiPlan::PAK_INDIRECT) {
        if(!binding_has_external_storage_address(binding)) {
          const string & param_temp = abi_plan.inputs[0].first;
          emit_copy_construct_to_target(param_types_[i],
                                        emit_storage_address(binding.slots[0]),
                                        param_temp);
          register_class_object_cleanup(binding);
        } else {
          register_class_at_ptr_cleanup(param_types_[i],
                                        binding.external_storage_address);
        }
      } else if(abi_plan.kind == ParamAbiPlan::PAK_DIRECT_OBJECT) {
        // The caller has already performed by-value parameter initialization at
        // the ABI boundary. Materialize our local addressable slot from the
        // direct object payload without invoking another copy constructor.
        if(!is_empty_class_storage_type(param_types_[i])) {
          emit_line("copyobj " + storage_span_text(param_types_[i]) + " " +
                    abi_plan.inputs[0].first + ", " +
                    emit_storage_address(binding.slots[0]));
        }
      } else {
        const string & param_temp = abi_plan.inputs[0].first;
        emit_line("store " + binding.lowir_type + " " + param_temp + ", " + binding.slots[0]);
        if(is_reference_type(param_types_[i])) {
          const vector<pair<string, unsigned long long> > * reference_layout =
              reference_virtual_base_layout(binding.semantic_type);
          for(map<string, string>::const_iterator hidden =
                  binding.hidden_virtual_base_slots.begin();
              hidden != binding.hidden_virtual_base_slots.end();
              ++hidden) {
            map<string, string>::const_iterator parameter_hidden =
                parameter_hidden_virtual_base_params_.find(hidden->first);
            if(parameter_hidden != parameter_hidden_virtual_base_params_.end()) {
              emit_line("store ptr " + parameter_hidden->second + ", " + hidden->second);
              continue;
            }
            string dynamic_external;
            if(try_emit_dynamic_external_virtual_base_pointer(binding.semantic_type,
                                                              param_temp,
                                                              hidden->first,
                                                              dynamic_external)) {
              emit_line("store ptr " + dynamic_external + ", " + hidden->second);
              continue;
            }
            if(!reference_layout) {
              continue;
            }
            for(size_t layout_index = 0; layout_index < reference_layout->size(); ++layout_index) {
              if((*reference_layout)[layout_index].first != hidden->first) {
                continue;
              }
              emit_line("store ptr " +
                        adjust_hidden_virtual_base_pointer(param_temp,
                                                           (*reference_layout)[layout_index].second) +
                        ", " + hidden->second);
              break;
            }
          }
        }
      }
    }

    const CallSemNode * body = find_child(*function_node_, CallSemKind::compound_statement);
    if(!body) {
      throw logic_error("function-definition missing compound-statement");
    }
    named_return_slot_variable_ =
        find_named_return_slot_variable(*function_node_, function_node_->semantic_type);
    if(has_dynamic_exception_spec()) {
      const string dispatch_label = new_block("function_exception_dispatch");
      const string handler_entry_label =
          use_host_eh_runtime() ? new_block("function_exception_entry") : dispatch_label;
      const string continue_label = new_block("function_exception_continue");
      const size_t host_dispatch_target_depth = host_eh_region_depth_;
      close_shared_host_call_unwind_region();
      emit_line("eh_try " + lowir_block_name(dispatch_label));
      if(use_host_eh_runtime()) {
        ++host_eh_region_depth_;
        host_eh_dispatch_labels_.push_back(handler_entry_label);
        host_eh_dispatch_depths_.push_back(host_dispatch_target_depth);
        host_eh_handler_nodes_.push_back(nullptr);
      }
      emit_statement(*body);
      if(use_host_eh_runtime()) {
        if(host_eh_region_depth_ == 0) {
          throw logic_error("host EH region depth underflow");
        }
        --host_eh_region_depth_;
        host_eh_handler_nodes_.pop_back();
        host_eh_dispatch_depths_.pop_back();
        host_eh_dispatch_labels_.pop_back();
      }
      const bool body_fallthrough = current_block_ && !current_block_->terminated;
      if(body_fallthrough) {
        terminate("jump " + lowir_block_name(continue_label));
      }
      start_block(dispatch_label);
      if(use_host_eh_runtime()) {
        emit_host_dynamic_exception_spec_metadata();
      }
      if(handler_entry_label != dispatch_label) {
        terminate("jump " + lowir_block_name(handler_entry_label));
        start_block(handler_entry_label);
      }
      emit_dynamic_exception_spec_dispatch();
      if(body_fallthrough) {
        start_block(continue_label);
      }
    } else if(needs_noexcept_terminate()) {
      const string dispatch_label = new_block("function_noexcept_dispatch");
      const string handler_entry_label =
          use_host_eh_runtime() ? new_block("function_noexcept_entry") : dispatch_label;
      const string continue_label = new_block("function_noexcept_continue");
      const size_t host_dispatch_target_depth = host_eh_region_depth_;
      close_shared_host_call_unwind_region();
      emit_line("eh_try " + lowir_block_name(dispatch_label));
      register_eh_end_cleanup();
      if(use_host_eh_runtime()) {
        note_external_runtime_function("__gxx_personality_v0");
        ++host_eh_region_depth_;
        host_eh_dispatch_labels_.push_back(handler_entry_label);
        host_eh_dispatch_depths_.push_back(host_dispatch_target_depth);
        host_eh_handler_nodes_.push_back(nullptr);
      }
      emit_statement(*body);
      if(use_host_eh_runtime()) {
        if(host_eh_region_depth_ == 0) {
          throw logic_error("host EH region depth underflow");
        }
        --host_eh_region_depth_;
        host_eh_handler_nodes_.pop_back();
        host_eh_dispatch_depths_.pop_back();
        host_eh_dispatch_labels_.pop_back();
      }
      const bool body_fallthrough = current_block_ && !current_block_->terminated;
      if(body_fallthrough) {
        terminate("jump " + lowir_block_name(continue_label));
      }
      start_block(dispatch_label);
      if(use_host_eh_runtime()) {
        emit_line("eh_catch_all");
      }
      if(handler_entry_label != dispatch_label) {
        terminate("jump " + lowir_block_name(handler_entry_label));
        start_block(handler_entry_label);
      }
      emit_noexcept_terminate_current_exception();
      if(body_fallthrough) {
        start_block(continue_label);
      }
    } else {
      emit_statement(*body);
    }

    if(current_block_ && !current_block_->terminated) {
      if(!cleanup_scopes_.empty()) {
        emit_scope_cleanups(cleanup_scopes_.back());
        pop_cleanup_scope();
      }
      pop_binding_scope();
      emit_noreturn_fallback_return();
    } else if(!cleanup_scopes_.empty()) {
      pop_cleanup_scope();
      pop_binding_scope();
    }
    return function_;
  }

  LowIRFunction build_actions(const vector<const CallSemNode *> & actions)
  {
    start_block("entry");
    push_cleanup_scope();
    push_binding_scope();
    for(size_t i = 0; i < actions.size(); ++i) {
      emit_statement(*actions[i]);
    }
    if(current_block_ && !current_block_->terminated) {
      if(!cleanup_scopes_.empty()) {
        emit_scope_cleanups(cleanup_scopes_.back());
        pop_cleanup_scope();
      }
      pop_binding_scope();
      terminate("return void");
    } else if(!cleanup_scopes_.empty()) {
      pop_cleanup_scope();
      pop_binding_scope();
    }
    return function_;
  }

private:
  const vector<pair<string, unsigned long long> > * resolve_current_function_virtual_base_layout() const
  {
    const vector<pair<string, unsigned long long> > * function_virtual_base_layout =
        function_node_ ? &callsem_virtual_base_layout(*function_node_) : nullptr;
    if(function_virtual_base_layout && function_virtual_base_layout->empty()) {
      const string function_symbol = node_internal_symbol(*function_node_);
      map<string, vector<pair<string, unsigned long long> > >::const_iterator layout_it =
          function_virtual_base_layouts_.find(function_symbol);
      if(layout_it != function_virtual_base_layouts_.end()) {
        function_virtual_base_layout = &layout_it->second;
      }
    }
    return function_virtual_base_layout;
  }

  const CallSemNode * peel_base_subobject_root(const CallSemNode & node) const
  {
    return peel_base_subobject_root_shared(node);
  }

  bool root_is_current_this(const CallSemNode & node) const
  {
    const CallSemNode * root = peel_base_subobject_root(node);
    return root &&
           (root->kind == CallSemKind::id_expression ||
            root->kind == CallSemKind::variable) &&
           root->text == "this";
  }

  bool root_is_parameter_binding(const CallSemNode & node) const
  {
    const CallSemNode * root = peel_base_subobject_root(node);
    if(!root ||
       (root->kind != CallSemKind::variable &&
        root->kind != CallSemKind::id_expression &&
        root->kind != CallSemKind::parameter)) {
      return false;
    }

    const VariableBinding * binding = find_local_binding(root->text);
    return binding && binding->is_parameter;
  }

  bool base_subobject_pointer_operand_may_be_null(const CallSemNode & node) const
  {
    if(root_is_current_this(node)) {
      return false;
    }
    if(node.kind == CallSemKind::unary_expression &&
       node.children.size() == 1 &&
       callsem_has_token(node, OP_AMP)) {
      return false;
    }
    return true;
  }

  string emit_pointer_operand(const CallSemNode & node)
  {
    TypePtr base_type = strip_top_level_cv(node.semantic_type);
    if(base_type &&
       (base_type->kind == Type::TK_POINTER ||
        base_type->kind == Type::TK_LVALUE_REFERENCE ||
        base_type->kind == Type::TK_RVALUE_REFERENCE)) {
      if(is_synthetic_subobject_pointer_node(node)) {
        return emit_lvalue_address(node);
      }
      return emit_rvalue(node);
    }
    return emit_lvalue_address(node);
  }

  bool try_resolve_class_virtual_base_offset(const CallSemNode & object_arg,
                                             const string & qualified_name,
                                             unsigned long long & out) const
  {
    const CallSemNode * root = peel_base_subobject_root(object_arg);
    TypePtr object_type =
        root && root->semantic_type ? root->semantic_type : object_arg.semantic_type;
    object_type = strip_top_level_cv(remove_reference_type(object_type));
    if(object_type && object_type->kind == Type::TK_POINTER) {
      object_type = strip_top_level_cv(object_type->inner);
    }

    const string object_class = class_qualified_name(object_type);
    if(object_class.empty()) {
      return false;
    }

    map<string, vector<pair<string, unsigned long long> > >::const_iterator layout_it =
        class_virtual_base_layouts_.find(object_class);
    if(layout_it == class_virtual_base_layouts_.end()) {
      return false;
    }

    for(size_t i = 0; i < layout_it->second.size(); ++i) {
      if(layout_it->second[i].first == qualified_name) {
        out = layout_it->second[i].second;
        return true;
      }
    }
    return false;
  }

  const vector<pair<string, unsigned long long> > * class_virtual_base_layout(
      const TypePtr & type) const
  {
    TypePtr referent = strip_top_level_cv(remove_reference_type(type));
    const string qualified_name = class_qualified_name(referent);
    if(qualified_name.empty()) {
      return nullptr;
    }

    map<string, vector<pair<string, unsigned long long> > >::const_iterator layout_it =
        class_virtual_base_layouts_.find(qualified_name);
    if(layout_it == class_virtual_base_layouts_.end() || layout_it->second.empty()) {
      return nullptr;
    }
    return &layout_it->second;
  }

  const vector<pair<string, unsigned long long> > * reference_virtual_base_layout(
      const TypePtr & type) const
  {
    TypePtr base = strip_top_level_cv(type);
    if(!base || !is_reference_type(base)) {
      return nullptr;
    }
    return class_virtual_base_layout(type);
  }

  bool class_uses_vtable_runtime(const TypePtr & type) const
  {
    TypePtr class_type = strip_top_level_cv(remove_reference_type(type));
    if(class_type && class_type->kind == Type::TK_POINTER) {
      class_type = strip_top_level_cv(class_type->inner);
    }
    const string qualified_name = class_qualified_name(class_type);
    return !qualified_name.empty() &&
           (classes_with_virtual_functions_.count(qualified_name) != 0 ||
            vtables_.count(qualified_name) != 0);
  }

  bool class_has_local_function_definition(const string & qualified_name) const
  {
    if(qualified_name.empty()) {
      return false;
    }
    const string member_prefix = qualified_name + "::";
    for(size_t i = 0; i < function_symbol_entries_.size(); ++i) {
      if(function_symbol_entries_[i].has_definition &&
         function_symbol_entries_[i].name.compare(0,
                                                  member_prefix.size(),
                                                  member_prefix) == 0) {
        return true;
      }
    }
    return false;
  }

  bool class_has_external_virtual_base_runtime_layout(const TypePtr & type) const
  {
    TypePtr class_type = strip_top_level_cv(remove_reference_type(type));
    if(class_type && class_type->kind == Type::TK_POINTER) {
      class_type = strip_top_level_cv(class_type->inner);
    }
    const string qualified_name = class_qualified_name(class_type);
    return !qualified_name.empty() &&
           symbol_linkage::has_external_vtable_symbol_candidate(class_type) &&
           class_virtual_base_layouts_.count(qualified_name) != 0 &&
           !class_has_local_function_definition(qualified_name) &&
           vtables_.count(qualified_name) == 0;
  }

  bool expression_path_uses_reference_storage(const CallSemNode & node) const
  {
    const CallSemNode * current = &node;
    size_t guard = 0;
    while(current && guard++ < 64) {
      if(current->is_reference_storage && !current->is_reference_storage_target) {
        return true;
      }
      TypePtr semantic_type = strip_top_level_cv(current->semantic_type);
      if(semantic_type && is_reference_type(semantic_type)) {
        return true;
      }
      if(current->children.empty()) {
        break;
      }
      current = &current->children[0];
    }
    return false;
  }

  bool dynamic_external_virtual_base_pointer_available(const TypePtr & object_type,
                                                       const string & qualified_name,
                                                       bool allow_reference_layout_runtime = false) const
  {
    TypePtr class_type = strip_top_level_cv(remove_reference_type(object_type));
    if(class_type && class_type->kind == Type::TK_POINTER) {
      class_type = strip_top_level_cv(class_type->inner);
    }
    const string class_name = class_qualified_name(class_type);
    if(class_name.empty()) {
      return false;
    }
    if(!class_uses_vtable_runtime(class_type) &&
       !(allow_reference_layout_runtime &&
         class_has_external_virtual_base_runtime_layout(class_type))) {
      return false;
    }

    map<string, vector<pair<string, unsigned long long> > >::const_iterator layout_it =
        class_virtual_base_layouts_.find(class_name);
    if(layout_it == class_virtual_base_layouts_.end()) {
      return false;
    }
    for(size_t i = 0; i < layout_it->second.size(); ++i) {
      if(layout_it->second[i].first == qualified_name) {
        return true;
      }
    }
    return false;
  }

  bool try_emit_dynamic_external_virtual_base_pointer(const TypePtr & object_type,
                                                      const string & object_ptr,
                                                      const string & qualified_name,
                                                      string & out,
                                                      bool allow_reference_layout_runtime = false)
  {
    TypePtr class_type = strip_top_level_cv(remove_reference_type(object_type));
    if(class_type && class_type->kind == Type::TK_POINTER) {
      class_type = strip_top_level_cv(class_type->inner);
    }
    const string class_name = class_qualified_name(class_type);
    if(class_name.empty()) {
      return false;
    }
    if(!class_uses_vtable_runtime(class_type) &&
       !(allow_reference_layout_runtime &&
         class_has_external_virtual_base_runtime_layout(class_type))) {
      return false;
    }

    map<string, vector<pair<string, unsigned long long> > >::const_iterator layout_it =
        class_virtual_base_layouts_.find(class_name);
    if(layout_it == class_virtual_base_layouts_.end()) {
      return false;
    }

    size_t layout_index = 0;
    bool found = false;
    for(; layout_index < layout_it->second.size(); ++layout_index) {
      if(layout_it->second[layout_index].first == qualified_name) {
        found = true;
        break;
      }
    }
    if(!found) {
      return false;
    }

    const string vptr = emit_temp_assignment("ptr", string("load ptr ") + object_ptr);
    const size_t slots_from_address_point =
        layout_it->second.size() - layout_index + 2;
    const long long slot_offset =
        -static_cast<long long>(slots_from_address_point * 8);
    const string offset_slot =
        emit_temp_assignment("ptr",
                             string("index i8 ") + vptr + ", " + to_string(slot_offset));
    const string runtime_offset =
        emit_temp_assignment("i64", string("load i64 ") + offset_slot);
    out = emit_temp_assignment("ptr",
                               string("index i8 ") + object_ptr + ", " + runtime_offset);
    return true;
  }

  bool try_load_hidden_virtual_base_from_local_binding(const CallSemNode & node,
                                                       const string & qualified_name,
                                                       string & out)
  {
    const CallSemNode * root = peel_base_subobject_root(node);
    if(!root ||
       (root->kind != CallSemKind::variable &&
        root->kind != CallSemKind::id_expression &&
        root->kind != CallSemKind::parameter)) {
      return false;
    }

    const VariableBinding * binding = find_local_binding(root->text);
    if(!binding) {
      return false;
    }

    map<string, string>::const_iterator hidden =
        binding->hidden_virtual_base_slots.find(qualified_name);
    if(hidden == binding->hidden_virtual_base_slots.end()) {
      return false;
    }

    out = emit_temp_assignment("ptr", string("load ptr ") + hidden->second);
    return true;
  }

  string adjust_hidden_virtual_base_pointer(const string & object_ptr,
                                            unsigned long long offset)
  {
    if(offset == 0) {
      return object_ptr;
    }
    return emit_temp_assignment("ptr",
                                string("index i8 ") + object_ptr + ", " +
                                    to_string(offset));
  }

  bool try_emit_forwarded_hidden_virtual_base_argument(
      const pair<string, unsigned long long> & virtual_base,
      const CallSemNode & object_arg,
      string & out)
  {
    const CallSemNode * root = peel_base_subobject_root(object_arg);
    if(root_is_current_this(object_arg)) {
      map<string, string>::const_iterator hidden = hidden_virtual_base_params_.find(virtual_base.first);
      if(hidden != hidden_virtual_base_params_.end()) {
        out = hidden->second;
        return true;
      }

      map<string, unsigned long long>::const_iterator offset =
          current_virtual_base_offsets_.find(virtual_base.first);
      if(offset != current_virtual_base_offsets_.end()) {
        if(!root) {
          throw logic_error("missing current object root for virtual base argument");
        }
        out = adjust_hidden_virtual_base_pointer(
            emit_pointer_operand(*root),
            offset->second);
        return true;
      }
    }

    if(try_load_hidden_virtual_base_from_local_binding(object_arg, virtual_base.first, out)) {
      return true;
    }

    if(root_is_parameter_binding(object_arg)) {
      map<string, string>::const_iterator parameter_hidden =
          parameter_hidden_virtual_base_params_.find(virtual_base.first);
      if(parameter_hidden != parameter_hidden_virtual_base_params_.end()) {
        out = parameter_hidden->second;
        return true;
      }
    }

    return false;
  }

  string emit_hidden_virtual_base_argument(const pair<string, unsigned long long> & virtual_base,
                                           const CallSemNode & object_arg,
                                           const string * object_ptr = nullptr)
  {
    string forwarded;
    if(try_emit_forwarded_hidden_virtual_base_argument(virtual_base, object_arg, forwarded)) {
      return forwarded;
    }

    const CallSemNode * root = peel_base_subobject_root(object_arg);
    if(root &&
       root->kind == CallSemKind::call_expression &&
       !root->children.empty()) {
      const pair<string, unsigned long long> requested_virtual_base =
          make_pair(virtual_base.first, 0ULL);
      const string callee_symbol = lookup_function_symbol(root->children[0]);
      map<string, ParameterVirtualBaseLayout>::const_iterator layout_it =
          function_parameter_virtual_base_layouts_.find(callee_symbol);
      if(layout_it != function_parameter_virtual_base_layouts_.end()) {
        const size_t child_index = layout_it->second.parameter_index + 1;
        if(child_index < root->children.size()) {
          return emit_hidden_virtual_base_argument(requested_virtual_base,
                                                  root->children[child_index],
                                                  object_ptr);
        }
      }

      size_t forwarded_child_index = 0;
      if(try_find_unique_same_class_reference_result_argument(
             *root,
             forwarded_child_index)) {
        return emit_hidden_virtual_base_argument(requested_virtual_base,
                                                root->children[forwarded_child_index],
                                                object_ptr);
      }

      const bool has_direct_object_layout =
          !callsem_virtual_base_layout(root->children[0]).empty();
      map<string, vector<pair<string, unsigned long long> > >::const_iterator object_layout_it =
          function_virtual_base_layouts_.find(callee_symbol);
      if((has_direct_object_layout ||
          object_layout_it != function_virtual_base_layouts_.end()) &&
         root->children.size() >= 2) {
        TypePtr result_type = strip_top_level_cv(remove_reference_type(root->semantic_type));
        TypePtr object_type =
            strip_top_level_cv(remove_reference_type(root->children[1].semantic_type));
        if(object_type && object_type->kind == Type::TK_POINTER) {
          object_type = strip_top_level_cv(object_type->inner);
        }
        if(!class_qualified_name(result_type).empty() &&
           class_qualified_name(result_type) == class_qualified_name(object_type)) {
          return emit_hidden_virtual_base_argument(requested_virtual_base,
                                                  root->children[1],
                                                  object_ptr);
        }
      }
    }

    const string base_object_ptr =
        object_ptr ? *object_ptr : emit_pointer_operand(object_arg);

    if(object_arg.is_reference_storage &&
       !object_arg.is_reference_storage_target) {
      string dynamic_external;
      if(try_emit_dynamic_external_virtual_base_pointer(object_arg.semantic_type,
                                                        base_object_ptr,
                                                        virtual_base.first,
                                                        dynamic_external)) {
        if(virtual_base.second == 0) {
          return dynamic_external;
        }
        return adjust_hidden_virtual_base_pointer(dynamic_external, virtual_base.second);
      }
    }

    const CallSemVirtualBaseLayout & object_arg_virtual_base_layout =
        callsem_virtual_base_layout(object_arg);
    for(size_t i = 0; i < object_arg_virtual_base_layout.size(); ++i) {
      if(object_arg_virtual_base_layout[i].first == virtual_base.first) {
        const unsigned long long total_offset =
            object_arg_virtual_base_layout[i].second + virtual_base.second;
        const CallSemNode * root = peel_base_subobject_root(object_arg);
        const bool root_layout_base =
            root && root != &object_arg &&
            (root->kind == CallSemKind::variable ||
             root->kind == CallSemKind::id_expression ||
             root->kind == CallSemKind::parameter);
        const string layout_base_ptr =
            root_layout_base && !object_arg.is_virtual_base_subobject ?
            emit_pointer_operand(*root) :
            base_object_ptr;
        return adjust_hidden_virtual_base_pointer(
            layout_base_ptr,
            total_offset);
      }
    }

    string dynamic_external;
    if(try_emit_dynamic_external_virtual_base_pointer(object_arg.semantic_type,
                                                      base_object_ptr,
                                                      virtual_base.first,
                                                      dynamic_external)) {
      if(virtual_base.second == 0) {
        return dynamic_external;
      }
      return adjust_hidden_virtual_base_pointer(dynamic_external, virtual_base.second);
    }

    unsigned long long class_offset = 0;
    if(virtual_base.second == 0 &&
       try_resolve_class_virtual_base_offset(object_arg, virtual_base.first, class_offset)) {
      const CallSemNode * root = peel_base_subobject_root(object_arg);
      const string base_object_ptr =
          (root && root != &object_arg) ? emit_pointer_operand(*root)
                                        : (object_ptr ? *object_ptr
                                                      : emit_pointer_operand(object_arg));
      return adjust_hidden_virtual_base_pointer(
          base_object_ptr,
          class_offset);
    }

    return adjust_hidden_virtual_base_pointer(
        base_object_ptr,
        virtual_base.second);
  }

  void append_hidden_virtual_base_arguments(const CallSemNode & call,
                                            vector<string> & args,
                                            size_t object_arg_index)
  {
    if(call.kind != CallSemKind::call_expression || call.children.size() < 2) {
      return;
    }
    const CallSemNode & callee = call.children[0];
    const vector<pair<string, unsigned long long> > * virtual_base_layout =
        &callsem_virtual_base_layout(callee);
    if(virtual_base_layout->empty()) {
      const string callee_symbol = lookup_function_symbol(callee);
      map<string, vector<pair<string, unsigned long long> > >::const_iterator layout_it =
          function_virtual_base_layouts_.find(callee_symbol);
      if(layout_it != function_virtual_base_layouts_.end()) {
        virtual_base_layout = &layout_it->second;
      }
    }
    if(virtual_base_layout->empty()) {
      return;
    }
    if(callee.has_special_member_entry_point_kind &&
       callsem_special_member_entry_point_kind(callee) != symbol_linkage::SMEK_BASE) {
      return;
    }
    const CallSemNode & object_arg = call.children[1];
    for(size_t i = 0; i < virtual_base_layout->size(); ++i) {
      const string * object_ptr =
          object_arg_index < args.size() ? &args[object_arg_index] : nullptr;
      const pair<string, unsigned long long> requested_virtual_base =
          make_pair((*virtual_base_layout)[i].first, 0ULL);
      args.push_back(
          emit_hidden_virtual_base_argument(requested_virtual_base, object_arg, object_ptr));
    }
  }

  void append_vtt_argument(const CallSemNode & call,
                           size_t object_arg_index,
                           vector<string> & args)
  {
    if(call.kind != CallSemKind::call_expression || call.children.size() < 2) {
      return;
    }
    const CallSemNode & callee = call.children[0];
    if(!callee.uses_vtt_parameter ||
       callsem_vtt_symbol(callee).empty() ||
       !callee.has_vtt_slice_offset) {
      return;
    }
    if(!callsem_vtt_object_symbol(callee).empty()) {
      external_object_symbols_[callsem_vtt_symbol(callee)] =
          callsem_vtt_object_symbol(callee);
    }

    const CallSemNode & object_arg = call.children[object_arg_index];
    if(!current_vtt_param_.empty() && root_is_current_this(object_arg)) {
      args.push_back(emit_vtt_slice_address(current_vtt_param_,
                                            callsem_vtt_slice_offset(callee)));
      return;
    }

    if(callsem_vtt_owner_type(callee)) {
      const string external_vtt =
          symbol_linkage::vtt_object_symbol_for_type(callsem_vtt_owner_type(callee));
      if(!external_vtt.empty()) {
        external_object_symbols_[callsem_vtt_symbol(callee)] = external_vtt;
      }
    }

    const string vtt_base =
        emit_temp_assignment("ptr", string("addr ") + callsem_vtt_symbol(callee));
    args.push_back(emit_vtt_slice_address(vtt_base, callsem_vtt_slice_offset(callee)));
  }

  string direct_parameter_virtual_base_layout_symbol(const CallSemNode & callee) const
  {
    if(callee.kind == CallSemKind::callee) {
      return lookup_function_symbol(callee);
    }
    return string();
  }

  bool resolve_call_parameter_virtual_base_layout(
      const CallSemNode & call,
      const TypePtr & function_type,
      ParameterVirtualBaseLayout & out_layout) const
  {
    if(call.kind != CallSemKind::call_expression || call.children.empty()) {
      return false;
    }

    const string callee_symbol =
        direct_parameter_virtual_base_layout_symbol(call.children[0]);
    if(!callee_symbol.empty()) {
      map<string, ParameterVirtualBaseLayout>::const_iterator layout_it =
          function_parameter_virtual_base_layouts_.find(callee_symbol);
      if(layout_it != function_parameter_virtual_base_layouts_.end()) {
        out_layout = layout_it->second;
        return true;
      }
      return false;
    }

    return infer_function_type_reference_parameter_virtual_base_layout(
        function_type,
        class_virtual_base_layouts_,
        out_layout);
  }

  string lowir_call_signature_suffix_for_call(
      const CallSemNode & call,
      const TypePtr & function_type) const
  {
    LowIRFunctionSignatureText signature =
        lowir_function_signature_text(function_type);
    ParameterVirtualBaseLayout parameter_virtual_base_layout;
    if(resolve_call_parameter_virtual_base_layout(call,
                                                  function_type,
                                                  parameter_virtual_base_layout)) {
      append_parameter_virtual_base_signature_params_for_layout(
          signature,
          parameter_virtual_base_layout);
    }
    return lowir_call_signature_suffix(signature);
  }

  void append_parameter_virtual_base_arguments(const CallSemNode & call,
                                               bool constructor_call,
                                               vector<string> & args)
  {
    if(call.kind != CallSemKind::call_expression || call.children.empty()) {
      return;
    }
    TypePtr function_type;
    const bool have_function_type =
        resolve_callable_function_type(call.children[0].semantic_type, function_type) &&
        function_type;
    const vector<pair<string, unsigned long long> > * current_function_virtual_base_layout =
        resolve_current_function_virtual_base_layout();
    if(!constructor_call &&
       function_node_ &&
       current_function_virtual_base_layout &&
       !current_function_virtual_base_layout->empty() &&
       call.children.size() >= 2 &&
       call.children[0].kind != CallSemKind::callee &&
       root_is_current_this(call.children[1])) {
      const CallSemNode & object_arg = call.children[1];
      for(size_t i = 0; i < current_function_virtual_base_layout->size(); ++i) {
        args.push_back(emit_hidden_virtual_base_argument((*current_function_virtual_base_layout)[i],
                                                         object_arg));
      }
      return;
    }
    ParameterVirtualBaseLayout parameter_virtual_base_layout;
    if(!resolve_call_parameter_virtual_base_layout(call,
                                                   function_type,
                                                   parameter_virtual_base_layout)) {
      return;
    }
    const size_t child_index =
        constructor_call ? parameter_virtual_base_layout.parameter_index :
            (parameter_virtual_base_layout.parameter_index + 1);
    if(child_index >= call.children.size()) {
      throw logic_error("parameter virtual base argument source missing");
    }
    const CallSemNode & object_arg = call.children[child_index];
    const CallSemVirtualBaseLayout & object_arg_virtual_base_layout =
        callsem_virtual_base_layout(object_arg);
    const size_t physical_parameter_index =
        have_function_type ?
            lowir_physical_argument_index(function_type,
                                          parameter_virtual_base_layout.parameter_index) :
            parameter_virtual_base_layout.parameter_index;
    bool parameter_reference_to_pointer = false;
    if(have_function_type) {
      TypePtr base = strip_top_level_cv(function_type);
      if(base &&
         base->kind == Type::TK_FUNCTION &&
         parameter_virtual_base_layout.parameter_index < base->params.size()) {
        TypePtr parameter_type =
            strip_top_level_cv(base->params[parameter_virtual_base_layout.parameter_index]);
        if(parameter_type && is_reference_type(parameter_type)) {
          TypePtr referent = strip_top_level_cv(remove_reference_type(parameter_type));
          parameter_reference_to_pointer = referent && referent->kind == Type::TK_POINTER;
        }
      }
    }
    string referenced_pointer_object;
    for(size_t i = 0; i < parameter_virtual_base_layout.layout.size(); ++i) {
      const string * object_ptr =
          physical_parameter_index < args.size() ?
              &args[physical_parameter_index] :
              nullptr;
      if(object_ptr && parameter_reference_to_pointer) {
        if(referenced_pointer_object.empty()) {
          referenced_pointer_object =
              emit_temp_assignment("ptr", string("load ptr ") + *object_ptr);
        }
        object_ptr = &referenced_pointer_object;
      }
      const pair<string, unsigned long long> requested_virtual_base =
          make_pair(parameter_virtual_base_layout.layout[i].first, 0ULL);
      if(object_ptr) {
        const CallSemNode * root = peel_base_subobject_root(object_arg);
        if(root &&
           (root->kind == CallSemKind::variable ||
            root->kind == CallSemKind::id_expression ||
            root->kind == CallSemKind::parameter ||
            root->kind == CallSemKind::call_expression)) {
          args.push_back(
              emit_hidden_virtual_base_argument(requested_virtual_base, object_arg, object_ptr));
          continue;
        }
        if(object_arg.kind == CallSemKind::member_expression &&
           is_reference_type(object_arg.semantic_type)) {
          args.push_back(
              emit_hidden_virtual_base_argument(requested_virtual_base, object_arg, object_ptr));
          continue;
        }
        bool appended_from_root_layout = false;
        if(root && root != &object_arg && !object_arg_virtual_base_layout.empty()) {
          bool found_root_offset = false;
          unsigned long long root_offset = 0;
          for(size_t j = 0; j < object_arg_virtual_base_layout.size(); ++j) {
            if(object_arg_virtual_base_layout[j].first ==
               parameter_virtual_base_layout.layout[i].first) {
              root_offset = object_arg_virtual_base_layout[j].second;
              found_root_offset = true;
              break;
            }
          }
          if(!found_root_offset && i < object_arg_virtual_base_layout.size()) {
            root_offset = object_arg_virtual_base_layout[i].second;
            found_root_offset = true;
          }
          if(found_root_offset) {
            args.push_back(
                adjust_hidden_virtual_base_pointer(
                    emit_pointer_operand(*root),
                    root_offset));
            appended_from_root_layout = true;
          }
        }
        if(!appended_from_root_layout) {
          args.push_back(
              emit_hidden_virtual_base_argument(requested_virtual_base, object_arg, object_ptr));
        }
        continue;
      }
      args.push_back(
          emit_hidden_virtual_base_argument(requested_virtual_base, object_arg, object_ptr));
    }
  }

  void store_reference_hidden_virtual_base_slots(const VariableBinding & binding,
                                                 const CallSemNode & source,
                                                 const string & object_ptr)
  {
    const vector<pair<string, unsigned long long> > * layout =
        reference_virtual_base_layout(binding.semantic_type);
    if(!layout || binding.hidden_virtual_base_slots.empty()) {
      return;
    }

    for(size_t i = 0; i < layout->size(); ++i) {
      map<string, string>::const_iterator slot =
          binding.hidden_virtual_base_slots.find((*layout)[i].first);
      if(slot == binding.hidden_virtual_base_slots.end()) {
        continue;
      }
      const pair<string, unsigned long long> requested_virtual_base =
          make_pair((*layout)[i].first, 0ULL);
      emit_line("store ptr " +
                emit_hidden_virtual_base_argument(requested_virtual_base, source, &object_ptr) +
                ", " + slot->second);
    }
  }

  void store_reference_hidden_virtual_base_slots_from_pointer(
      const VariableBinding & binding,
      const string & object_ptr)
  {
    const vector<pair<string, unsigned long long> > * layout =
        reference_virtual_base_layout(binding.semantic_type);
    if(!layout || binding.hidden_virtual_base_slots.empty()) {
      return;
    }

    for(size_t i = 0; i < layout->size(); ++i) {
      map<string, string>::const_iterator slot =
          binding.hidden_virtual_base_slots.find((*layout)[i].first);
      if(slot == binding.hidden_virtual_base_slots.end()) {
        continue;
      }
      string dynamic_external;
      if(try_emit_dynamic_external_virtual_base_pointer(binding.semantic_type,
                                                        object_ptr,
                                                        (*layout)[i].first,
                                                        dynamic_external)) {
        emit_line("store ptr " + dynamic_external + ", " + slot->second);
        continue;
      }
      emit_line("store ptr " +
                adjust_hidden_virtual_base_pointer(object_ptr, (*layout)[i].second) +
                ", " + slot->second);
    }
  }

  const CallSemNode * function_node_;
  const map<string, GlobalBinding> & globals_;
  const map<string, VTableBinding> & vtables_;
  const map<string, string> & function_symbols_;
  const vector<FunctionSymbolEntry> & function_symbol_entries_;
  const FunctionSymbolLookupIndex & function_symbol_lookup_index_;
  const map<string, const CallSemNode *> & function_symbol_nodes_;
  const set<string> & c_linkage_function_symbols_;
  const map<string, vector<pair<string, unsigned long long> > > &
      function_virtual_base_layouts_;
  const map<string, vector<pair<string, unsigned long long> > > &
      class_virtual_base_layouts_;
  const map<string, ParameterVirtualBaseLayout> &
      function_parameter_virtual_base_layouts_;
  const set<string> & classes_with_virtual_functions_;
  const set<string> & throwing_function_symbols_;
  const set<string> & rtti_definition_symbols_;
  const map<string, string> & string_literal_symbols_;
  const map<string, TypePtr> & exception_storage_types_;
  map<string, VirtualMemberPointerThunkRequest> & virtual_member_pointer_thunks_;
  map<string, string> & external_function_symbols_;
  map<string, string> & external_object_symbols_;
  set<string> & runtime_bridge_support_symbols_;
  set<string> & referenced_function_symbols_;
  map<string, TypePtr> & referenced_function_signature_types_;
  map<string, set<string> > & function_references_;
  bool emit_runtime_support_ = false;
  bool enable_debug_value_names_ = false;
  LowIRFunction function_;
  map<string, VariableBinding> bindings_;
  map<string, size_t> named_storage_counts_;
  map<string, size_t> debug_value_versions_;
  map<string, size_t> parameter_binding_counts_;
  vector<string> param_names_;
  vector<TypePtr> param_types_;
  vector<ParamAbiPlan> param_abi_plans_;
  map<string, string> hidden_virtual_base_params_;
  map<string, string> parameter_hidden_virtual_base_params_;
  map<string, unsigned long long> current_virtual_base_offsets_;
  string current_vtt_param_;
  size_t temp_counter_ = 0;
  size_t block_counter_ = 0;
  size_t hidden_slot_counter_ = 0;
  LowIRBlock * current_block_ = nullptr;
  vector<unique_ptr<CallSemNode> > synthetic_nodes_;
  vector<ControlTransferTarget> break_targets_;
  vector<ControlTransferTarget> continue_targets_;
  map<string, string> goto_targets_;
  const map<const CallSemNode *, string> * active_switch_labels_ = nullptr;
  vector<vector<CleanupAction> > cleanup_scopes_;
  vector<size_t> cleanup_scope_normal_eh_end_counts_;
  vector<bool> cleanup_scope_host_unwind_cleanup_;
  vector<bool> cleanup_scope_is_full_expression_;
  map<string, string> shared_call_unwind_dispatch_labels_;
  vector<string> active_host_cleanup_labels_;
  vector<vector<BindingScopeEntry> > binding_scopes_;
  vector<CleanupAction> constructor_unwind_cleanups_;
  vector<string> constructor_function_try_dispatch_labels_;
  vector<string> host_eh_dispatch_labels_;
  vector<size_t> host_eh_dispatch_depths_;
  vector<const CallSemNode *> host_eh_handler_nodes_;
  map<const CallSemNode *, vector<long long> > host_eh_handler_selectors_;
  long long next_host_eh_selector_ = 1;
  size_t host_eh_region_depth_ = 0;
  bool shared_host_call_unwind_region_open_ = false;
  string shared_host_call_unwind_dispatch_label_;
  size_t shared_host_call_unwind_host_dispatch_depth_ = 0;
  bool shared_host_call_unwind_created_dispatch_ = false;
  TypePtr function_result_type_;
  const CallSemNode * named_return_slot_variable_ = nullptr;
  bool direct_object_return_ = false;
  bool indirect_class_return_ = false;
  bool is_constructor_function_ = false;
  size_t constructor_action_depth_ = 0;
  size_t cleanup_emission_depth_ = 0;
  string direct_object_return_slot_;

  bool has_dynamic_exception_spec() const
  {
    return function_node_ && function_node_->has_dynamic_exception_spec;
  }

  bool needs_noexcept_terminate() const
  {
    return function_node_ &&
           function_node_->needs_noexcept_terminate &&
           !function_node_->has_dynamic_exception_spec;
  }

  void note_referenced_function_signature(const string & symbol,
                                          const TypePtr & type) const
  {
    if(symbol.empty() || !type) {
      return;
    }
    map<string, TypePtr>::const_iterator found =
        referenced_function_signature_types_.find(symbol);
    if(found == referenced_function_signature_types_.end()) {
      referenced_function_signature_types_[symbol] = type;
      return;
    }
    if(type_equals(found->second, type) ||
       stable_function_type_key(found->second) == stable_function_type_key(type)) {
      return;
    }
  }

  string terminate_function_symbol()
  {
    TypePtr terminate_type = std_terminate_function_type();
    const string symbol =
        lookup_function_symbol("std::terminate",
                               terminate_type);
    note_host_std_terminate_symbol(function_symbol_entries_,
                                   external_function_symbols_,
                                   referenced_function_signature_types_,
                                   symbol);
    note_generated_function_reference(symbol);
    return symbol;
  }

  string call_terminate_support_symbol()
  {
    runtime_bridge_support_symbols_.insert(kCppgmCallTerminateSupportSymbol);
    return string("@") + kCppgmCallTerminateSupportSymbol;
  }

  string unexpected_function_symbol()
  {
    const string symbol =
        lookup_function_symbol("std::unexpected",
                               make_function(make_fundamental(FT_VOID),
                                             vector<TypePtr>(),
                                             false));
    note_generated_function_reference(symbol);
    return symbol;
  }

  void emit_noreturn_fallback_return()
  {
    if(function_.return_type == "void") {
      terminate("return void");
      return;
    }
    if(direct_object_return_) {
      const string result_slot = ensure_direct_object_return_slot();
      emit_line("zeroinit " + storage_span_text(function_result_type_) + " " + result_slot);
      terminate("return " + function_.return_type + " " + result_slot);
      return;
    }
    terminate("return " + function_.return_type + " 0");
  }

  void emit_terminate_current_function()
  {
    emit_line("call void " + terminate_function_symbol() + "()");
    emit_noreturn_fallback_return();
  }

  void emit_noexcept_terminate_current_exception()
  {
    if(use_host_eh_runtime()) {
      const string exception_ptr = emit_temp_assignment("ptr", "exception ptr");
      emit_line("call void " + call_terminate_support_symbol() + "(" +
                exception_ptr + ")");
      emit_noreturn_fallback_return();
      return;
    }
    emit_terminate_current_function();
  }

  bool emit_function_exception_spec_match(const string & current_type,
                                          const string & hit_label)
  {
    bool emitted_match = false;
    string next_label;
    for(size_t i = 0; i < function_node_->children.size(); ++i) {
      const CallSemNode & candidate = function_node_->children[i];
      if(candidate.kind != CallSemKind::rtti_candidate || !candidate.semantic_type) {
        continue;
      }
      if(emitted_match) {
        start_block(next_label);
      }
      emitted_match = true;
      const string match =
          emit_temp_assignment("i64",
                               string("cmp eq ptr ") + current_type + ", " +
                                   emit_exception_match_rtti_address(candidate.semantic_type));
      const string match_label = new_block("function_exception_spec_match");
      next_label = new_block("function_exception_spec_next");
      terminate("branch " + match + ", " + lowir_block_name(match_label) + ", " +
                lowir_block_name(next_label));
      start_block(match_label);
      terminate("jump " + lowir_block_name(hit_label));
    }
    if(emitted_match) {
      start_block(next_label);
    }
    return emitted_match;
  }

  void emit_host_dynamic_exception_spec_metadata()
  {
    if(!use_host_eh_runtime() || !function_node_) {
      return;
    }
    ostringstream out;
    out << "eh_filter";
    bool emitted_any = false;
    for(size_t i = 0; i < function_node_->children.size(); ++i) {
      const CallSemNode & candidate = function_node_->children[i];
      if(candidate.kind != CallSemKind::rtti_candidate || !candidate.semantic_type) {
        continue;
      }
      out << (emitted_any ? ", " : " ")
          << host_exception_match_rtti_symbol(candidate.semantic_type);
      emitted_any = true;
    }
    emit_line(out.str());
  }

  void emit_dynamic_exception_spec_dispatch()
  {
    if(use_host_eh_runtime()) {
      const string unexpected_label = new_block("function_exception_unexpected");
      const string allowed_label = new_block("function_exception_allowed");
      const string current_selector = emit_current_exception_selector();
      const string needs_unexpected =
          emit_temp_assignment("i64",
                               string("cmp eq i32 ") + current_selector + ", -1");
      terminate("branch " + needs_unexpected + ", " + lowir_block_name(unexpected_label) +
                ", " + lowir_block_name(allowed_label));
      start_block(unexpected_label);
      const string current_storage = emit_temp_assignment("ptr", "exception ptr");
      emit_line("call void " + external_runtime_symbol("__cxa_call_unexpected") +
                "(" + current_storage + ")");
      emit_noreturn_fallback_return();
      start_block(allowed_label);
      terminate("resume");
      return;
    }
    const string allowed_label = new_block("function_exception_allowed");
    const string current_type = emit_current_exception_type();
    emit_function_exception_spec_match(current_type, allowed_label);

    const string unexpected_dispatch = new_block("unexpected_dispatch");
    const string unexpected_return = new_block("unexpected_return");
    emit_line("eh_try " + lowir_block_name(unexpected_dispatch));
    emit_line("call void " + unexpected_function_symbol() + "()");
    terminate("jump " + lowir_block_name(unexpected_return));

    start_block(unexpected_dispatch);
    const string replacement_type = emit_current_exception_type();
    emit_function_exception_spec_match(replacement_type, allowed_label);
    emit_terminate_current_function();

    start_block(unexpected_return);
    emit_terminate_current_function();

    start_block(allowed_label);
    terminate("resume");
  }

  string lookup_function_symbol(const string & name, const TypePtr & type) const
  {
    const string symbol =
        try_lookup_function_symbol_with_index(function_symbols_,
                                              function_symbol_entries_,
                                              function_symbol_lookup_index_,
                                              name,
                                              type);
    const string resolved = symbol.empty() ? lowir_name(name) : symbol;
    note_referenced_function_signature(resolved, type);
    return resolved;
  }

  string lookup_function_symbol(const CallSemNode & node) const
  {
    const string lookup_name =
        callsem_resolved_name(node).empty() ? node.text.str() :
            callsem_resolved_name(node);
    if(!callsem_symbol(node).internal_symbol.empty()) {
      note_referenced_function_signature(callsem_symbol(node).internal_symbol, node.semantic_type);
      return callsem_symbol(node).internal_symbol;
    }
    return lookup_function_symbol(lookup_name, node.semantic_type);
  }

  bool local_generated_function_symbol_exists(const string & symbol) const
  {
    for(size_t i = 0; i < function_symbol_entries_.size(); ++i) {
      if(function_symbol_entries_[i].symbol == symbol &&
         function_symbol_entries_[i].has_definition) {
        return true;
      }
    }
    return false;
  }

  string register_virtual_member_pointer_thunk(const CallSemNode & node)
  {
    TypePtr member_pointer_type = strip_top_level_cv(node.semantic_type);
    if(!member_pointer_type ||
       member_pointer_type->kind != Type::TK_MEMBER_POINTER ||
       !is_function_type(member_pointer_type->inner)) {
      throw logic_error("virtual member pointer thunk requires member function pointer type");
    }

    string target_symbol = callsem_symbol(node).internal_symbol;
    if(target_symbol.empty() && !node.children.empty()) {
      target_symbol = lookup_function_symbol(node.children[0]);
    }
    if(target_symbol.empty()) {
      throw logic_error("virtual member pointer thunk missing target symbol");
    }

    const string thunk_symbol = virtual_member_pointer_thunk_symbol(target_symbol);
    VirtualMemberPointerThunkRequest & request = virtual_member_pointer_thunks_[thunk_symbol];
    if(request.symbol.empty()) {
      request.symbol = thunk_symbol;
      request.member_pointer_type = member_pointer_type;
      request.virtual_slot = node.has_uint_value ? callsem_uint_value(node) : 0ULL;
      request.uses_extended_vtable_layout = node.uses_extended_vtable_layout;
    }
    return thunk_symbol;
  }

  string copy_constructor_symbol(const TypePtr & class_type) const
  {
    TypePtr object_type = strip_top_level_cv(remove_reference_type(class_type));
    if(!is_complete_class_value_type(object_type)) {
      return string();
    }

    const string qualified = class_qualified_name(object_type);
    if(qualified.empty()) {
      return string();
    }
    const string simple = class_constructor_name(qualified);
    vector<TypePtr> params;
    params.push_back(make_pointer(object_type));
    params.push_back(make_lvalue_reference_raw(make_cv(object_type, true, false)));
    const string lookup_name = qualified + "::" + simple;
    const TypePtr lookup_type =
        make_function(make_fundamental(FT_VOID), params, false);
    map<string, string>::const_iterator found =
        function_symbols_.find(function_key(lookup_name, lookup_type));
    if(found != function_symbols_.end()) {
      note_referenced_function_signature(
          found->second,
          lookup_type);
      if(parser_trace::enabled("lowir.copy")) {
        ostringstream trace;
        trace << "action=copy-ctor-lookup class=" << qualified
              << " type=" << describe_type(object_type)
              << " result=exact symbol=" << found->second;
        parser_trace::note("lowir.copy", string(), trace.str());
      }
      return found->second;
    }
    string symbol = try_lookup_special_member_symbol_by_index(
        function_symbol_entries_,
        function_symbol_lookup_index_,
        lookup_name,
        [&](const TypePtr & entry_type)
        {
          return matches_constructor_entry_type_for_lowir(entry_type,
                                                          object_type,
                                                          Type::TK_LVALUE_REFERENCE);
        });
    note_referenced_function_signature(
        symbol,
        lookup_type);
    if(parser_trace::enabled("lowir.copy")) {
      ostringstream trace;
      trace << "action=copy-ctor-lookup class=" << qualified
            << " type=" << describe_type(object_type)
            << " result=" << (symbol.empty() ? "miss" : "entry")
            << " symbol=" << (symbol.empty() ? "<none>" : symbol)
            << " lookup_type=" << describe_type(lookup_type)
            << " candidates=[" <<
                summarize_lowir_special_member_candidates(function_symbol_entries_, lookup_name)
            << "]";
      parser_trace::note("lowir.copy", string(), trace.str());
    }
    return symbol;
  }

  string move_constructor_symbol(const TypePtr & class_type) const
  {
    TypePtr object_type = strip_top_level_cv(remove_reference_type(class_type));
    if(!is_complete_class_value_type(object_type)) {
      return string();
    }

    const string qualified = class_qualified_name(object_type);
    if(qualified.empty()) {
      return string();
    }
    const string simple = class_constructor_name(qualified);
    vector<TypePtr> params;
    params.push_back(make_pointer(object_type));
    params.push_back(make_rvalue_reference_raw(object_type));
    const string lookup_name = qualified + "::" + simple;
    const TypePtr lookup_type =
        make_function(make_fundamental(FT_VOID), params, false);
    map<string, string>::const_iterator found =
        function_symbols_.find(function_key(lookup_name, lookup_type));
    if(found != function_symbols_.end()) {
      note_referenced_function_signature(
          found->second,
          lookup_type);
      return found->second;
    }
    string symbol = try_lookup_special_member_symbol_by_index(
        function_symbol_entries_,
        function_symbol_lookup_index_,
        lookup_name,
        [&](const TypePtr & entry_type)
        {
          return matches_constructor_entry_type_for_lowir(entry_type,
                                                          object_type,
                                                          Type::TK_RVALUE_REFERENCE);
        });
    note_referenced_function_signature(
        symbol,
        lookup_type);
    return symbol;
  }

  bool special_member_symbol_has_trivial_lifecycle(const string & symbol) const
  {
    if(symbol.empty()) {
      return false;
    }
    map<string, const CallSemNode *>::const_iterator owner =
        function_symbol_nodes_.find(symbol);
    return owner != function_symbol_nodes_.end() &&
           owner->second != nullptr &&
           owner->second->trivial_lifecycle;
  }

  string external_vtable_symbol_for_type(const TypePtr & semantic_type) const
  {
    TypePtr vtable_type = strip_top_level_cv(remove_reference_type(semantic_type));
    if(vtable_type && vtable_type->kind == Type::TK_POINTER) {
      vtable_type = strip_top_level_cv(vtable_type->inner);
    }
    return vtable_type ? symbol_linkage::vtable_object_symbol_for_type(vtable_type) :
                         string();
  }

  string external_vtable_internal_symbol(const string & owner,
                                         const string & external_symbol) const
  {
    const string key = owner.empty() ? external_symbol : owner;
    return symbol_linkage::internal_symbol_from_name("__external_vtable::" + key);
  }

  string vtable_base_symbol(const string & qualified_name,
                            const TypePtr & semantic_type)
  {
    map<string, VTableBinding>::const_iterator found = vtables_.find(qualified_name);
    if(found != vtables_.end() && !found->second.base_symbol.empty()) {
      return found->second.base_symbol;
    }
    const string external =
        external_vtable_symbol_for_type(semantic_type);
    if(!external.empty()) {
      const string internal =
          external_vtable_internal_symbol(qualified_name, external);
      external_object_symbols_[internal] = external;
      return internal;
    }
    throw logic_error("missing vtable binding for " + qualified_name +
                      " while building " + function_.name);
  }

  unsigned long long vtable_address_point_offset(const string & qualified_name,
                                                 const TypePtr & semantic_type) const
  {
    map<string, VTableBinding>::const_iterator found = vtables_.find(qualified_name);
    if(found != vtables_.end()) {
      return found->second.address_point_offset;
    }
    if(symbol_linkage::has_external_vtable_symbol_candidate(semantic_type)) {
      return 16ULL;
    }
    return 0ULL;
  }

  string emit_vtable_address_point(const string & qualified_name,
                                   const TypePtr & semantic_type = TypePtr())
  {
    const string base =
        emit_temp_assignment("ptr",
                             string("addr ") +
                                 vtable_base_symbol(qualified_name, semantic_type));
    const unsigned long long offset =
        vtable_address_point_offset(qualified_name, semantic_type);
    if(offset == 0) {
      return base;
    }
    return emit_temp_assignment("ptr",
                                string("index i8 ") + base + ", " + to_string(offset));
  }

  string emit_external_vtable_group_address_point(const string & owner,
                                                  const TypePtr & semantic_type,
                                                  unsigned long long address_point_offset)
  {
    const string external =
        external_vtable_symbol_for_type(semantic_type);
    if(external.empty()) {
      throw logic_error("missing external vtable symbol for " + owner +
                        " while building " + function_.name);
    }
    const string internal = external_vtable_internal_symbol(owner, external);
    external_object_symbols_[internal] = external;
    const string base = emit_temp_assignment("ptr", string("addr ") + internal);
    if(address_point_offset == 0) {
      return base;
    }
    return emit_temp_assignment("ptr",
                                string("index i8 ") + base + ", " +
                                    to_string(address_point_offset));
  }

  string emit_vtt_slice_address(const string & vtt_base,
                                unsigned long long slice_offset)
  {
    if(slice_offset == 0) {
      return vtt_base;
    }
    return emit_temp_assignment("ptr",
                                string("index i8 ") + vtt_base + ", " +
                                    to_string(slice_offset));
  }

  string emit_vtt_entry_address_point(const string & vtt_base,
                                      unsigned long long entry_index)
  {
    const string entry_ptr =
        emit_vtt_slice_address(vtt_base, entry_index * 8ULL);
    return emit_temp_assignment("ptr", string("load ptr ") + entry_ptr);
  }

  string emit_vptr_action_address_point(const CallSemNode & action)
  {
    if(action.has_vtt_entry_index && !current_vtt_param_.empty()) {
      return emit_vtt_entry_address_point(current_vtt_param_,
                                          callsem_vtt_entry_index(action));
    }
    if(vtables_.count(action.text) != 0) {
      return emit_vtable_address_point(action.text);
    }
    if(action.has_uint_value &&
       !callsem_resolved_name(action).empty() &&
      has_external_vtable_symbol_candidate_for_type(action.semantic_type)) {
      return emit_external_vtable_group_address_point(callsem_resolved_name(action),
                                                      action.semantic_type,
                                                      callsem_uint_value(action));
    }
    return emit_vtable_address_point(action.text, action.semantic_type);
  }

  void note_generated_function_reference(const string & symbol) const
  {
    if(symbol.empty()) {
      return;
    }
    if(function_.name.empty()) {
      referenced_function_symbols_.insert(symbol);
      return;
    }
    function_references_[function_.name].insert(symbol);
  }

  void maybe_emit_thread_local_global_init(const GlobalBinding * global)
  {
    if(!global ||
       global->thread_local_init_symbol.empty() ||
       function_.name == global->thread_local_init_symbol) {
      return;
    }
    note_referenced_function_signature(
        global->thread_local_init_symbol,
        make_function(make_fundamental(FT_VOID), vector<TypePtr>(), false));
    note_generated_function_reference(global->thread_local_init_symbol);
    emit_line("call void " + global->thread_local_init_symbol + "()");
  }

  const VariableBinding * find_local_binding(const string & name) const
  {
    map<string, VariableBinding>::const_iterator found = bindings_.find(name);
    return found == bindings_.end() ? nullptr : &found->second;
  }

  const GlobalBinding * find_global_binding(const CallSemNode & node) const
  {
    const string key = !callsem_symbol(node).internal_symbol.empty() ? callsem_symbol(node).internal_symbol
                                                            : lowir_name(node.text);
    map<string, GlobalBinding>::const_iterator found = globals_.find(key);
    return found == globals_.end() ? nullptr : &found->second;
  }

  bool try_emit_known_id_rvalue(const CallSemNode & node, string & out)
  {
    const bool preserve_member_pointer_storage =
        is_member_function_pointer_type(callsem_materialization_source_type(node));
    const VariableBinding * local = find_local_binding(node.text);
    if(local) {
      TypePtr expr_base = strip_top_level_cv(remove_reference_type(node.semantic_type));
      const bool local_indirect_value =
          binding_is_indirect_storage(*local) ||
          is_indirect_value_type(remove_reference_type(local->semantic_type));
      if(binding_has_external_storage_address(*local)) {
        if(local_indirect_value) {
          out = local->external_storage_address;
          return true;
        }
        const TypePtr loaded_type =
            materialization_source_type_for(node, local->semantic_type);
        const string value_type = lowir_memory_type_for(loaded_type);
        const string loaded_value =
            emit_temp_assignment(value_type,
                                 string("load ") + value_type + " " +
                                     local->external_storage_address);
        out = emit_loaded_scalar_value(loaded_value, loaded_type, node);
        return true;
      }
      if(binding_is_reference_slot(*local)) {
        const string referent =
            emit_temp_assignment("ptr", string("load ptr ") + local->slots[0]);
        if(expr_base && expr_base->kind == Type::TK_ARRAY) {
          out = emit_decay_pointer(referent);
          return true;
        }
        if(expr_base && is_function_type(expr_base) && !preserve_member_pointer_storage) {
          out = emit_decay_pointer(referent);
          return true;
        }
        if(local_indirect_value) {
          out = referent;
          return true;
        }
        TypePtr referent_type = remove_reference_type(local->semantic_type);
        TypePtr loaded_type =
            materialization_source_type_for(node, referent_type);
        const string memory_type = lowir_memory_type_for(loaded_type);
        const string loaded_value =
            emit_temp_assignment(memory_type, string("load ") + memory_type + " " + referent);
        out = emit_loaded_scalar_value(loaded_value, loaded_type, node);
        return true;
      }
      if(binding_is_decay_view_slot(*local)) {
        out = emit_temp_assignment("ptr", string("load ptr ") + local->slots[0]);
        return true;
      }
      if(binding_is_array_storage(*local)) {
        out = emit_decay_pointer(emit_storage_address(local->slots[0]));
        return true;
      }
      if(binding_is_indirect_storage(*local)) {
        out = emit_storage_address(local->slots[0]);
        return true;
      }
      if(expr_base && is_function_type(expr_base) && !preserve_member_pointer_storage) {
        out = emit_decay_pointer(
            emit_temp_assignment("ptr", string("load ptr ") + local->slots[0]));
        return true;
      }
      const TypePtr loaded_type =
          materialization_source_type_for(node, local->semantic_type);
      const string value_type = lowir_memory_type_for(loaded_type);
      const string loaded_value =
          emit_temp_assignment(value_type,
                               string("load ") + value_type + " " + local->slots[0]);
      out = emit_loaded_scalar_value(loaded_value, loaded_type, node);
      return true;
    }

    const GlobalBinding * global = find_global_binding(node);
    if(global) {
      maybe_emit_thread_local_global_init(global);
      TypePtr expr_type = strip_top_level_cv(node.semantic_type);
      TypePtr expr_base = strip_top_level_cv(remove_reference_type(node.semantic_type));
      if(is_reference_type(global->semantic_type)) {
        const string referent =
            emit_temp_assignment("ptr", string("load ptr ") + global->storage);
        if(expr_base && expr_base->kind == Type::TK_ARRAY) {
          out = emit_decay_pointer(referent);
          return true;
        }
        if(expr_base && is_function_type(expr_base) && !preserve_member_pointer_storage) {
          out = emit_decay_pointer(referent);
          return true;
        }
        if(is_indirect_value_type(node.semantic_type)) {
          out = referent;
          return true;
        }
        TypePtr referent_type = remove_reference_type(global->semantic_type);
        TypePtr loaded_type = materialization_source_type_for(node, referent_type);
        const string memory_type = lowir_memory_type_for(loaded_type);
        const string loaded_value =
            emit_temp_assignment(memory_type, string("load ") + memory_type + " " + referent);
        out = emit_loaded_scalar_value(loaded_value, loaded_type, node);
        return true;
      }
      if(expr_base && is_function_type(expr_base) && !preserve_member_pointer_storage) {
        out = emit_decay_pointer(
            emit_temp_assignment("ptr", string("addr ") + lookup_function_symbol(node)));
        return true;
      }
      if(expr_type &&
         expr_type->kind == Type::TK_POINTER &&
         expr_type->inner &&
         is_function_type(strip_top_level_cv(expr_type->inner)) &&
         !preserve_member_pointer_storage) {
        out = emit_temp_assignment("ptr",
                                   string("addr ") +
                                       lookup_function_symbol(node.text,
                                                              strip_top_level_cv(
                                                                  expr_type->inner)));
        return true;
      }
      if(expr_base && expr_base->kind == Type::TK_ARRAY) {
        out = emit_decay_pointer(
            emit_temp_assignment("ptr", string("addr ") + global->storage));
        return true;
      }
      if(is_indirect_value_type(node.semantic_type)) {
        out = emit_temp_assignment("ptr", string("addr ") + global->storage);
        return true;
      }
      const TypePtr loaded_type =
          materialization_source_type_for(node,
                                          global->semantic_type ? global->semantic_type
                                                                : node.semantic_type);
      const string value_type = lowir_memory_type_for(loaded_type);
      const string loaded_value =
          emit_temp_assignment(value_type,
                               string("load ") + value_type + " " + global->storage);
      out = emit_loaded_scalar_value(loaded_value, loaded_type, node);
      return true;
    }

    if(!callsem_symbol(node).internal_symbol.empty()) {
      TypePtr expr_type = strip_top_level_cv(node.semantic_type);
      TypePtr expr_base = strip_top_level_cv(remove_reference_type(node.semantic_type));
      if(expr_base && is_function_type(expr_base) && !preserve_member_pointer_storage) {
        out = emit_decay_pointer(
            emit_temp_assignment("ptr", string("addr ") + callsem_symbol(node).internal_symbol));
        return true;
      }
      if(expr_type &&
         expr_type->kind == Type::TK_POINTER &&
         expr_type->inner &&
         is_function_type(strip_top_level_cv(expr_type->inner)) &&
         !preserve_member_pointer_storage) {
        out = emit_temp_assignment("ptr", string("addr ") + callsem_symbol(node).internal_symbol);
        return true;
      }
      if(expr_base && expr_base->kind == Type::TK_ARRAY) {
        out = emit_decay_pointer(
            emit_temp_assignment("ptr", string("addr ") + callsem_symbol(node).internal_symbol));
        return true;
      }
      if(is_indirect_value_type(node.semantic_type)) {
        out = emit_temp_assignment("ptr", string("addr ") + callsem_symbol(node).internal_symbol);
        return true;
      }
      const TypePtr loaded_type =
          materialization_source_type_for(node, node.semantic_type);
      const string memory_type = lowir_memory_type_for(loaded_type);
      out = emit_temp_assignment(memory_type,
                                 string("load ") + memory_type + " " +
                                 callsem_symbol(node).internal_symbol);
      out = emit_loaded_scalar_value(out, loaded_type, node);
      return true;
    }

    if(node.semantic_type && is_function_type(node.semantic_type)) {
      const string symbol = lookup_function_symbol(node);
      if(symbol != lowir_name(node.text) ||
         function_symbols_.count(function_key(node.text, node.semantic_type))) {
        out = emit_decay_pointer(
            emit_temp_assignment("ptr", string("addr ") + symbol));
        return true;
      }
    }

    return false;
  }

  bool try_emit_known_id_address(const CallSemNode & node, string & out)
  {
    const VariableBinding * local = find_local_binding(node.text);
    if(local) {
      if(binding_has_external_storage_address(*local)) {
        out = local->external_storage_address;
        return true;
      }
      TypePtr pointee_type = nullptr;
      TypePtr local_base = strip_top_level_cv(local->semantic_type);
      if(node.text == "this" &&
         local_base &&
         local_base->kind == Type::TK_POINTER &&
         local_base->inner) {
        pointee_type = strip_top_level_cv(local_base->inner);
      }
      TypePtr node_object_type = strip_top_level_cv(remove_reference_type(node.semantic_type));
      if(pointee_type && node_object_type && type_equals(pointee_type, node_object_type)) {
        out = emit_temp_assignment("ptr", string("load ptr ") + local->slots[0]);
        return true;
      }
      if(binding_is_decay_view_slot(*local)) {
        out = emit_temp_assignment("ptr", string("load ptr ") + local->slots[0]);
        return true;
      }
      if(binding_is_reference_slot(*local)) {
        out = emit_temp_assignment("ptr", string("load ptr ") + local->slots[0]);
        return true;
      }
      out = emit_storage_address(local->slots[0]);
      return true;
    }

    const GlobalBinding * global = find_global_binding(node);
    if(global) {
      maybe_emit_thread_local_global_init(global);
      if(is_reference_type(global->semantic_type) && !node.is_reference_storage_target) {
        out = emit_temp_assignment("ptr", string("load ptr ") + global->storage);
        return true;
      }
      out = emit_temp_assignment("ptr", string("addr ") + global->storage);
      return true;
    }

    if(!callsem_symbol(node).internal_symbol.empty()) {
      out = emit_temp_assignment("ptr", string("addr ") + callsem_symbol(node).internal_symbol);
      return true;
    }

    if(node.semantic_type && is_function_type(node.semantic_type)) {
      const string symbol = lookup_function_symbol(node);
      if(symbol != lowir_name(node.text) ||
         function_symbols_.count(function_key(node.text, node.semantic_type))) {
        out = emit_temp_assignment("ptr", string("addr ") + symbol);
        return true;
      }
    }

    return false;
  }

  bool try_emit_known_id_storage(const CallSemNode & node, string & out)
  {
    const VariableBinding * local = find_local_binding(node.text);
    if(local) {
      if(binding_has_external_storage_address(*local)) {
        out = local->external_storage_address;
        return true;
      }
      TypePtr local_base = strip_top_level_cv(local->semantic_type);
      TypePtr pointee_type = nullptr;
      if(node.text == "this" &&
         local_base &&
         local_base->kind == Type::TK_POINTER &&
         local_base->inner) {
        pointee_type = strip_top_level_cv(local_base->inner);
      }
      TypePtr node_object_type = strip_top_level_cv(remove_reference_type(node.semantic_type));
      if(pointee_type && node_object_type && type_equals(pointee_type, node_object_type)) {
        out = emit_temp_assignment("ptr", string("load ptr ") + local->slots[0]);
        return true;
      }
      if(binding_is_array_storage(*local)) {
        throw logic_error("array lvalue requires subscript");
      }
      if(binding_is_reference_slot(*local)) {
        return try_emit_known_id_address(node, out);
      }
      out = local->slots[0];
      return true;
    }

    const GlobalBinding * global = find_global_binding(node);
    if(global) {
      maybe_emit_thread_local_global_init(global);
      if(is_reference_type(global->semantic_type) && !node.is_reference_storage_target) {
        return try_emit_known_id_address(node, out);
      }
      out = global->storage;
      return true;
    }

    if(!callsem_symbol(node).internal_symbol.empty()) {
      out = callsem_symbol(node).internal_symbol;
      return true;
    }

    return false;
  }

  bool binding_is_array_storage(const VariableBinding & binding) const
  {
    return binding.mode == VariableBinding::VBM_ARRAY_STORAGE;
  }

  bool binding_is_indirect_storage(const VariableBinding & binding) const
  {
    return binding.mode == VariableBinding::VBM_INDIRECT_STORAGE;
  }

  bool binding_has_external_storage_address(const VariableBinding & binding) const
  {
    return binding.uses_external_storage_address && !binding.external_storage_address.empty();
  }

  bool binding_is_reference_slot(const VariableBinding & binding) const
  {
    return binding.mode == VariableBinding::VBM_REFERENCE_SLOT;
  }

  bool binding_is_decay_view_slot(const VariableBinding & binding) const
  {
    return binding.mode == VariableBinding::VBM_DECAY_VIEW_SLOT;
  }

  VariableBinding create_variable_binding(const string & name,
                                          const TypePtr & original_type,
                                          const TypePtr & lowered_type)
  {
    VariableBinding binding;
    binding.original_semantic_type = original_type;
    binding.semantic_type = lowered_type;
    TypePtr base = strip_top_level_cv(lowered_type);
    TypePtr original_base = strip_top_level_cv(remove_reference_type(original_type));
    if(is_reference_type(lowered_type)) {
      binding.mode = VariableBinding::VBM_REFERENCE_SLOT;
    } else if(base &&
              base->kind == Type::TK_POINTER &&
              original_base &&
              (original_base->kind == Type::TK_ARRAY || is_function_type(original_base))) {
      binding.mode = VariableBinding::VBM_DECAY_VIEW_SLOT;
    } else if(original_base && original_base->kind == Type::TK_ARRAY) {
      binding.mode = VariableBinding::VBM_ARRAY_STORAGE;
    } else if(is_indirect_value_type(lowered_type)) {
      binding.mode = VariableBinding::VBM_INDIRECT_STORAGE;
    }
    if(base && base->kind == Type::TK_ARRAY) {
      const string base_slot_name = next_named_storage_base(name);
      binding.lowir_type = lowir_storage_type_for(lowered_type);
      binding.slots.push_back(base_slot_name);
      function_.slots.push_back(make_pair(binding.slots[0], binding.lowir_type));
      return binding;
    }
    if(is_indirect_value_type(lowered_type)) {
      const string base_slot_name = next_named_storage_base(name);
      binding.lowir_type = lowir_storage_type_for(lowered_type);
      binding.slots.push_back(base_slot_name);
      function_.slots.push_back(make_pair(binding.slots[0], binding.lowir_type));
      return binding;
    }
    const string base_slot_name = next_named_storage_base(name);
    binding.lowir_type = lowir_memory_type_for(lowered_type);
    binding.slots.push_back(base_slot_name);
    function_.slots.push_back(make_pair(binding.slots[0], binding.lowir_type));
    const vector<pair<string, unsigned long long> > * virtual_base_layout =
        reference_virtual_base_layout(lowered_type);
    if(virtual_base_layout) {
      for(size_t i = 0; i < virtual_base_layout->size(); ++i) {
        const string hidden_slot_name =
            base_slot_name + "__pvb" + to_string(i);
        binding.hidden_virtual_base_slots[(*virtual_base_layout)[i].first] =
            hidden_slot_name;
        function_.slots.push_back(make_pair(hidden_slot_name, "ptr"));
      }
    }
    return binding;
  }

  VariableBinding create_variable_binding(const string & name, const TypePtr & type)
  {
    return create_variable_binding(name, type, type);
  }

  VariableBinding create_named_return_slot_binding(const string & name,
                                                   const TypePtr & type)
  {
    VariableBinding binding;
    binding.original_semantic_type = type;
    binding.semantic_type = type;
    binding.lowir_type = lowir_storage_type_for(type);
    binding.mode = VariableBinding::VBM_INDIRECT_STORAGE;
    binding.uses_external_storage_address = true;
    binding.is_named_return_slot_alias = true;
    binding.external_storage_address = function_.params[0].name;
    return binding;
  }

  bool is_named_return_slot_variable(const CallSemNode & variable) const
  {
    return named_return_slot_variable_ != nullptr && &variable == named_return_slot_variable_;
  }

  bool storage_name_in_use(const string & slot_name) const
  {
    for(size_t i = 0; i < function_.params.size(); ++i) {
      if(function_.params[i].name == slot_name) {
        return true;
      }
    }
    for(size_t i = 0; i < function_.slots.size(); ++i) {
      if(function_.slots[i].first == slot_name) {
        return true;
      }
    }
    return false;
  }

  string next_named_storage_base(const string & name, size_t indexed_slots = 0)
  {
    size_t & count = named_storage_counts_[name];
    while(true) {
      ++count;
      string candidate = lowir_slot_name(name);
      if(count != 1) {
        ostringstream out;
        out << candidate << "__shadow" << count;
        candidate = out.str();
      }
      bool conflict = false;
      if(indexed_slots == 0) {
        conflict = storage_name_in_use(candidate);
      } else {
        for(size_t i = 0; i < indexed_slots; ++i) {
          ostringstream indexed_name;
          indexed_name << candidate << "__" << i;
          if(storage_name_in_use(indexed_name.str())) {
            conflict = true;
            break;
          }
        }
      }
      if(!conflict) {
        return candidate;
      }
    }
  }

  string next_parameter_binding_name(const string & name)
  {
    size_t & count = parameter_binding_counts_[name];
    ++count;
    if(count == 1) {
      return name;
    }
    ostringstream out;
    out << name << "__pack" << count;
    return out.str();
  }

  string new_hidden_object_address(const TypePtr & type, const string & prefix)
  {
    const vector<string> names = new_hidden_slots(lowir_storage_type_for(type), prefix, 1);
    return emit_storage_address(names[0]);
  }

  vector<string> new_hidden_object_slots(const TypePtr & type, const string & prefix)
  {
    return new_hidden_slots(lowir_storage_type_for(type), prefix, 1);
  }

  string emit_index_address_with_projection(
      const string & element_type,
      const string & base_ptr,
      const string & offset,
      lowir_internal::IndexProjectionKind projection = lowir_internal::IPK_NONE,
      bool elide_zero_offset = true)
  {
    if(elide_zero_offset && projection == lowir_internal::IPK_NONE && offset == "0") {
      return base_ptr;
    }
    string op = string("index ") + element_type;
    if(projection != lowir_internal::IPK_NONE) {
      op += " [projection=";
      op += lowir_internal::index_projection_text(projection);
      op += "]";
    }
    op += " " + base_ptr + ", " + offset;
    return emit_temp_assignment("ptr", op);
  }

  string emit_index_address_with_projection(
      const string & element_type,
      const string & base_ptr,
      size_t offset,
      lowir_internal::IndexProjectionKind projection = lowir_internal::IPK_NONE,
      bool elide_zero_offset = true)
  {
    if(elide_zero_offset && projection == lowir_internal::IPK_NONE && offset == 0) {
      return base_ptr;
    }
    return emit_index_address_with_projection(element_type,
                                              base_ptr,
                                              to_string(offset),
                                              projection,
                                              false);
  }

  string emit_null_preserving_index_address_with_projection(
      const string & element_type,
      const string & base_ptr,
      const string & offset,
      lowir_internal::IndexProjectionKind projection)
  {
    if(offset == "0") {
      return emit_index_address_with_projection(element_type,
                                                base_ptr,
                                                offset,
                                                projection,
                                                false);
    }

    const string result_slot = new_hidden_slot("ptr", "basecast");
    const string null_label = new_block("basecast_null");
    const string adjust_label = new_block("basecast_adjust");
    const string end_label = new_block("basecast_end");
    const string is_null =
        emit_temp_assignment("i64", string("cmp eq ptr ") + base_ptr + ", 0");
    terminate(string("branch ") + is_null + ", " + lowir_block_name(null_label) + ", " +
              lowir_block_name(adjust_label));

    start_block(null_label);
    emit_line("store ptr 0, " + result_slot);
    terminate(string("jump ") + lowir_block_name(end_label));

    start_block(adjust_label);
    const string adjusted =
        emit_index_address_with_projection(element_type,
                                           base_ptr,
                                           offset,
                                           projection,
                                           false);
    emit_line("store ptr " + adjusted + ", " + result_slot);
    terminate(string("jump ") + lowir_block_name(end_label));

    start_block(end_label);
    return emit_temp_assignment("ptr", string("load ptr ") + result_slot);
  }

  string emit_decay_pointer(const string & ptr)
  {
    return emit_temp_assignment("ptr", string("unary decay ptr ") + ptr);
  }

  lowir_internal::IndexProjectionKind member_projection_kind(const CallSemNode & node) const
  {
    if(node.is_base_subobject || node.is_virtual_base_subobject) {
      return lowir_internal::IPK_BASE_SUBOBJECT;
    }
    if(node.is_reference_storage && !node.is_reference_storage_target) {
      return lowir_internal::IPK_REFERENCE_FIELD;
    }
    return lowir_internal::IPK_FIELD;
  }

  string emit_byte_offset_address(const string & base_ptr, size_t offset)
  {
    return emit_index_address_with_projection("i8", base_ptr, offset);
  }

  size_t pointer_arithmetic_stride(const TypePtr & pointer_type) const
  {
    TypePtr value_type = lowir_value_conversion_type(pointer_type);
    if(!value_type || value_type->kind != Type::TK_POINTER) {
      throw logic_error("pointer arithmetic requires pointer type");
    }
    return backend_storage_size(value_type->inner);
  }

  string emit_pointer_element_offset_bytes(const string & element_count,
                                           const TypePtr & count_type,
                                           const TypePtr & pointer_type)
  {
    string byte_offset =
        emit_scalar_value_conversion(element_count,
                                     count_type,
                                     make_fundamental(FT_LONG_LONG_INT));
    const size_t stride = pointer_arithmetic_stride(pointer_type);
    if(stride != 1) {
      byte_offset = emit_temp_assignment("i64",
                                         string("binary mul i64 ") + byte_offset + ", " +
                                         to_string(stride));
    }
    return byte_offset;
  }

  string emit_pointer_index(const string & base_ptr,
                            const string & element_count,
                            const TypePtr & count_type,
                            const TypePtr & pointer_type)
  {
    return emit_index_address_with_projection("i8",
                                              base_ptr,
                                              emit_pointer_element_offset_bytes(element_count,
                                                                               count_type,
                                                                               pointer_type));
  }

  string emit_pointer_index_negated(const string & base_ptr,
                                    const string & element_count,
                                    const TypePtr & count_type,
                                    const TypePtr & pointer_type)
  {
    const string byte_offset =
        emit_pointer_element_offset_bytes(element_count, count_type, pointer_type);
    const string negated =
        emit_temp_assignment("i64", string("binary sub i64 0, ") + byte_offset);
    return emit_index_address_with_projection("i8", base_ptr, negated);
  }

  string emit_pointer_difference_elements(const string & lhs_ptr,
                                          const string & rhs_ptr,
                                          const TypePtr & pointer_type)
  {
    const string byte_distance =
        emit_temp_assignment("i64", string("binary sub ptr ") + lhs_ptr + ", " + rhs_ptr);
    const size_t stride = pointer_arithmetic_stride(pointer_type);
    if(stride == 1) {
      return byte_distance;
    }
    return emit_temp_assignment("i64",
                                string("binary div i64 ") + byte_distance + ", " +
                                to_string(stride));
  }

  string emit_incdec_next_value(const CallSemNode & node,
                                const string & memory_type,
                                const string & old_value)
  {
    if(node.children.size() != 1 ||
       (!callsem_has_token(node, OP_INC) && !callsem_has_token(node, OP_DEC))) {
      throw logic_error("inc/dec expression shape");
    }
    const TypePtr operand_value_type =
        lowir_value_conversion_type(node.children[0].semantic_type);
    if(operand_value_type && operand_value_type->kind == Type::TK_POINTER) {
      return callsem_has_token(node, OP_INC) ?
          emit_pointer_index(old_value,
                             "1",
                             make_fundamental(FT_LONG_LONG_INT),
                             node.children[0].semantic_type) :
          emit_pointer_index_negated(old_value,
                                     "1",
                                     make_fundamental(FT_LONG_LONG_INT),
                                     node.children[0].semantic_type);
    }
    return emit_temp_assignment(memory_type,
                                string("binary ") +
                                (callsem_has_token(node, OP_INC) ? "add " : "sub ") +
                                memory_type + " " + old_value + ", 1");
  }

  void emit_zero_storage_bytes(const string & target_ptr, size_t byte_count)
  {
    size_t offset = 0;
    while(byte_count >= 8) {
      emit_line("store i64 0, " + emit_byte_offset_address(target_ptr, offset));
      offset += 8;
      byte_count -= 8;
    }
    if(byte_count >= 4) {
      emit_line("store i32 0, " + emit_byte_offset_address(target_ptr, offset));
      offset += 4;
      byte_count -= 4;
    }
    if(byte_count >= 2) {
      emit_line("store i16 0, " + emit_byte_offset_address(target_ptr, offset));
      offset += 2;
      byte_count -= 2;
    }
    if(byte_count >= 1) {
      emit_line("store i8 0, " + emit_byte_offset_address(target_ptr, offset));
    }
  }

  void emit_dynamic_zero_storage_bytes(const string & target_ptr, const string & byte_count)
  {
    const string offset_slot = new_hidden_slot("i64", "zeroinit_offset");
    const string cond_label = new_block("zeroinit_cond");
    const string body_label = new_block("zeroinit_body");
    const string end_label = new_block("zeroinit_end");

    emit_line("store i64 0, " + offset_slot);
    terminate("jump " + lowir_block_name(cond_label));

    start_block(cond_label);
    const string offset = emit_temp_assignment("i64", string("load i64 ") + offset_slot);
    const string keep_going =
        emit_temp_assignment("i64", string("cmp ult i64 ") + offset + ", " + byte_count);
    terminate("branch " + keep_going + ", " + lowir_block_name(body_label) + ", " +
              lowir_block_name(end_label));

    start_block(body_label);
    const string element_ptr =
        emit_temp_assignment("ptr", string("index i8 ") + target_ptr + ", " + offset);
    emit_line("store i8 0, " + element_ptr);
    const string next =
        emit_temp_assignment("i64", string("binary add i64 ") + offset + ", 1");
    emit_line("store i64 " + next + ", " + offset_slot);
    terminate("jump " + lowir_block_name(cond_label));

    start_block(end_label);
  }

  bool is_zero_storage_initializer(const CallSemNode & node) const
  {
    if(node.kind != CallSemKind::literal) {
      return false;
    }
    if(node.has_int_value) {
      return callsem_int_value(node) == 0;
    }
    if(node.has_uint_value) {
      return callsem_uint_value(node) == 0;
    }
    if(callsem_has_token(node, KW_FALSE)) {
      return true;
    }
    return node.text == "0";
  }

  string emit_reference_storage_value(const TypePtr & referent_type,
                                      const CallSemNode & node)
  {
    if(node.kind == CallSemKind::call_expression &&
       is_reference_type(node.semantic_type)) {
      const TypePtr result_object_type = indirect_call_result_object_type(node);
      if(result_object_type) {
        const string temp_ptr = new_hidden_object_address(result_object_type, "refcall");
        emit_call_expression_to_target(node, temp_ptr);
        register_materialized_temporary_cleanup_live(result_object_type, temp_ptr);
        return temp_ptr;
      }
      return emit_call_expression_raw(node);
    }
    if(node.is_base_subobject || node.is_virtual_base_subobject) {
      return emit_lvalue_address(node);
    }
    if(node.value_category == CVC_LVALUE || is_reference_type(node.semantic_type)) {
      return emit_lvalue_address(node);
    }

    const vector<pair<string, unsigned long long> > * layout =
        class_virtual_base_layout(referent_type);
    if(layout && !layout->empty()) {
      const pair<string, unsigned long long> & virtual_base = (*layout)[0];
      const string actual_virtual_base =
          emit_hidden_virtual_base_argument(make_pair(virtual_base.first, 0ULL), node);
      if(virtual_base.second == 0) {
        return actual_virtual_base;
      }
      const string rewind =
          emit_temp_assignment("i64",
                               string("binary sub i64 0, ") +
                                   to_string(virtual_base.second));
      return emit_temp_assignment("ptr",
                                  string("index i8 ") + actual_virtual_base + ", " + rewind);
    }
    if(class_uses_vtable_runtime(referent_type)) {
      return emit_lvalue_address(node);
    }
    return emit_lvalue_address(node);
  }

  void emit_storage_value_to_target(const TypePtr & target_type,
                                    const CallSemNode & node,
                                    const string & target_ptr)
  {
    if(is_reference_type(target_type)) {
      emit_line("store ptr " +
                emit_reference_storage_value(remove_reference_type(target_type), node) +
                ", " + target_ptr);
      return;
    }
    if(emit_special_class_value_to_target(node, target_ptr)) {
      return;
    }
    if(node.kind == CallSemKind::call_expression &&
       (is_indirect_value_type(node.semantic_type) ||
        is_complete_class_value_type(node.semantic_type) ||
        is_constructor_materialization_call(node))) {
      emit_call_expression_to_target(node, target_ptr);
      return;
    }
    if(is_zero_storage_initializer(node)) {
      if(is_empty_class_storage_type(target_type)) {
        return;
      }
      emit_zero_storage_bytes(target_ptr, backend_storage_size(target_type));
      return;
    }
    if(is_indirect_class_reference_type(node.semantic_type)) {
      const string source_ptr = emit_rvalue(node);
      if(node.value_category == CVC_XVALUE) {
        emit_move_construct_to_target(target_type, target_ptr, source_ptr);
      } else {
        emit_copy_construct_to_target(target_type, target_ptr, source_ptr);
      }
      return;
    }
    if(node.value_category == CVC_XVALUE &&
       is_complete_class_value_type(target_type)) {
      emit_move_construct_to_target(target_type, target_ptr, emit_lvalue_address(node));
      return;
    }
    emit_copy_construct_to_target(target_type, target_ptr, emit_lvalue_address(node));
  }

  bool emit_trivial_class_storage_copy_to_target(const TypePtr & target_type,
                                                 const CallSemNode & source_arg,
                                                 const string & target_ptr)
  {
    TypePtr base = strip_top_level_cv(target_type);
    if(!is_complete_class_value_type(base)) {
      return false;
    }
    if(is_empty_class_storage_type(base)) {
      return true;
    }

    string source_ptr;
    if(is_indirect_class_reference_type(source_arg.semantic_type)) {
      source_ptr = emit_rvalue(source_arg);
    } else if(source_arg.value_category == CVC_XVALUE &&
              is_complete_class_value_type(remove_reference_type(source_arg.semantic_type))) {
      source_ptr = emit_lvalue_address(source_arg);
    } else if(source_arg.value_category == CVC_LVALUE ||
              is_reference_type(source_arg.semantic_type)) {
      source_ptr = emit_lvalue_address(source_arg);
    } else {
      return false;
    }

    emit_line("copyobj " + storage_span_text(base) + " " + source_ptr + ", " + target_ptr);
    return true;
  }

  bool emit_trivial_storage_prefix_copy_to_target(const TypePtr & target_type,
                                                  const CallSemNode & source_arg,
                                                  const string & target_ptr,
                                                  size_t byte_count)
  {
    TypePtr base = strip_top_level_cv(target_type);
    if(!base || !is_complete_class_value_type(base) || byte_count == 0) {
      return false;
    }
    if(byte_count > backend_storage_size(base)) {
      return false;
    }

    string source_ptr;
    if(is_indirect_class_reference_type(source_arg.semantic_type)) {
      source_ptr = emit_rvalue(source_arg);
    } else if(source_arg.value_category == CVC_XVALUE &&
              is_complete_class_value_type(remove_reference_type(source_arg.semantic_type))) {
      source_ptr = emit_lvalue_address(source_arg);
    } else if(source_arg.value_category == CVC_LVALUE ||
              is_reference_type(source_arg.semantic_type)) {
      source_ptr = emit_lvalue_address(source_arg);
    } else {
      return false;
    }

    emit_line("copyobj " + to_string(byte_count) + "x" +
              to_string(backend_storage_alignment(base)) + " " +
              source_ptr + ", " + target_ptr);
    return true;
  }

  void emit_local_array_initializer(const TypePtr & array_type,
                                    const CallSemNode & init,
                                    const string & target_ptr)
  {
    TypePtr base = strip_top_level_cv(array_type);
    if(!base || base->kind != Type::TK_ARRAY) {
      throw logic_error("local array initializer requires array type");
    }
    if(init.kind != CallSemKind::braced_init_list) {
      throw logic_error("local array initializer requires braced-init-list");
    }
    if(init.children.size() > base->bound) {
      throw logic_error("too many array initializer elements");
    }

    const TypePtr element_type = base->inner;
    const TypePtr element_base = strip_top_level_cv(element_type);
    const size_t element_stride = backend_storage_size(element_type);
    const bool storage_elements = array_element_uses_storage_slots(element_type);

    for(size_t i = 0; i < base->bound; ++i) {
      const string element_ptr = emit_byte_offset_address(target_ptr, i * element_stride);
      if(i >= init.children.size()) {
        emit_zero_storage_bytes(element_ptr, element_stride);
        continue;
      }

      const CallSemNode & child = init.children[i];
      if(element_base && element_base->kind == Type::TK_ARRAY) {
        if(is_zero_storage_initializer(child)) {
          emit_zero_storage_bytes(element_ptr, element_stride);
          continue;
        }
        if(child.kind != CallSemKind::braced_init_list) {
          throw logic_error("nested local array initializer requires braced-init-list");
        }
        emit_local_array_initializer(element_type, child, element_ptr);
        continue;
      }

      if(storage_elements) {
        emit_storage_value_to_target(element_type, child, element_ptr);
        continue;
      }

      const string memory_type = lowir_memory_type_for(element_type);
      emit_line("store " + memory_type + " " +
                emit_scalar_storage_value(element_type, child) + ", " + element_ptr);
    }
  }

  string emit_rtti_symbol_address(const string & symbol)
  {
    return emit_temp_assignment("ptr", string("addr ") + symbol);
  }

  string host_rtti_reference_symbol(const TypePtr & type)
  {
    const string internal_symbol = rtti_symbol_for_type(type);
    if(emit_runtime_support_ && type && is_complete_class_value_type(type)) {
      if(rtti_definition_symbols_.count(internal_symbol) != 0 ||
         !is_host_runtime_rtti_class_type(type)) {
        return internal_symbol;
      }
    }
    const string host_symbol = symbol_linkage::typeinfo_symbol_for_type(type);
    if(host_symbol.empty()) {
      return internal_symbol;
    }
    return external_object_symbol("rtti", describe_type(type), host_symbol);
  }

  string emit_host_rtti_symbol_address(const TypePtr & type)
  {
    return emit_temp_assignment("ptr", string("addr ") + host_rtti_reference_symbol(type));
  }

  string host_typeid_rtti_reference_symbol(const TypePtr & type)
  {
    const string internal_symbol = rtti_symbol_for_type(type);
    if(rtti_definition_symbols_.count(internal_symbol) != 0) {
      return internal_symbol;
    }
    return host_rtti_reference_symbol(type);
  }

  string emit_host_typeid_rtti_symbol_address(const TypePtr & type)
  {
    return emit_temp_assignment("ptr", string("addr ") +
                                host_typeid_rtti_reference_symbol(type));
  }

  string emit_host_dynamic_typeinfo_address(const string & object_ptr)
  {
    const string vptr = emit_temp_assignment("ptr", string("load ptr ") + object_ptr);
    const string typeinfo_slot =
        emit_temp_assignment("ptr", string("index i8 ") + vptr + ", -8");
    return emit_temp_assignment("ptr", string("load ptr ") + typeinfo_slot);
  }

  string emit_host_dynamic_cast_void_pointer(const string & source_ptr)
  {
    const string vptr = emit_temp_assignment("ptr", string("load ptr ") + source_ptr);
    const string offset_slot =
        emit_temp_assignment("ptr", string("index i8 ") + vptr + ", -16");
    const string offset = emit_temp_assignment("i64", string("load i64 ") + offset_slot);
    return emit_temp_assignment("ptr", string("index i8 ") + source_ptr + ", " + offset);
  }

  long long host_dynamic_cast_src2dst_hint(const CallSemNode & node,
                                          const TypePtr & source_object_type,
                                          const TypePtr & target_object_type) const
  {
    if(!target_object_type) {
      return -1;
    }
    for(size_t i = 1; i < node.children.size(); ++i) {
      const CallSemNode & candidate = node.children[i];
      if(candidate.kind != CallSemKind::rtti_candidate || !candidate.semantic_type) {
        continue;
      }
      if(describe_type(strip_top_level_cv(candidate.semantic_type)) !=
         describe_type(strip_top_level_cv(target_object_type))) {
        continue;
      }
      return candidate.has_int_value ? callsem_int_value(candidate) : -1;
    }
    if(source_object_type &&
       describe_type(strip_top_level_cv(source_object_type)) !=
           describe_type(strip_top_level_cv(target_object_type))) {
      return -2;
    }
    return -1;
  }

  string host_exception_match_rtti_symbol(const TypePtr & type)
  {
    return host_rtti_reference_symbol(type);
  }

  string emit_exception_match_rtti_address(const TypePtr & type)
  {
    return use_host_eh_runtime() ? emit_host_rtti_symbol_address(type)
                                 : emit_rtti_symbol_address(rtti_symbol_for_type(type));
  }

  string emit_exception_type_address()
  {
    return emit_temp_assignment("ptr", string("addr ") + eh_runtime::kEhTypeSymbol);
  }

  string emit_exception_storage_slot_address()
  {
    return emit_temp_assignment("ptr", string("addr ") + eh_runtime::kEhValueSymbol);
  }

  bool use_host_eh_runtime() const
  {
    return emit_runtime_support_;
  }

  string emit_current_exception_type()
  {
    if(use_host_eh_runtime()) {
      return emit_temp_assignment(
          "ptr",
          string("call ptr ") +
              external_runtime_symbol("__cxa_current_exception_type") + "()");
    }
    return emit_temp_assignment("ptr", string("load ptr ") + emit_exception_type_address());
  }

  string emit_current_exception_selector()
  {
    if(!use_host_eh_runtime()) {
      throw logic_error("exception selector only available for host EH runtime");
    }
    return emit_temp_assignment("i32", "exception_selector i32");
  }

  string emit_current_exception_storage()
  {
    if(use_host_eh_runtime()) {
      const string exception_ptr = emit_temp_assignment("ptr", "exception ptr");
      return emit_temp_assignment(
          "ptr",
          string("call ptr ") +
              external_runtime_symbol("__cxa_begin_catch") + "(" +
              exception_ptr + ")");
    }
    return emit_temp_assignment("ptr", string("load ptr ") + emit_exception_storage_slot_address());
  }

  void emit_host_runtime_throw_helper(const char * helper_symbol)
  {
    emit_line("call void " + external_runtime_symbol(helper_symbol) + "()");
    emit_noreturn_fallback_return();
  }

  void emit_rtti_failure_throw(const char * host_helper_symbol)
  {
    if(use_host_eh_runtime()) {
      emit_host_runtime_throw_helper(host_helper_symbol);
      return;
    }
    const string type_slot = emit_exception_type_address();
    const string storage_slot = emit_exception_storage_slot_address();
    emit_line("store ptr 0, " + type_slot);
    emit_line("store ptr " + storage_slot + ", " + storage_slot);
    terminate("throw ptr " + storage_slot);
  }

  void emit_set_current_exception_type(const TypePtr & type)
  {
    const string rtti_address = emit_rtti_symbol_address(rtti_symbol_for_type(type));
    const string exception_type_address = emit_exception_type_address();
    emit_line("store ptr " + rtti_address + ", " + exception_type_address);
  }

  void emit_set_current_exception_storage(const string & storage_ptr)
  {
    emit_line("store ptr " + storage_ptr + ", " + emit_exception_storage_slot_address());
  }

  string lookup_destructor_symbol_no_note(const TypePtr & class_type,
                                          TypePtr * object_type_out = nullptr) const
  {
    TypePtr object_type = strip_top_level_cv(remove_reference_type(class_type));
    if(object_type_out) {
      *object_type_out = object_type;
    }
    const string qualified = class_qualified_name(object_type);
    if(qualified.empty()) {
      return string();
    }
    const string simple = class_constructor_name(qualified);
    vector<TypePtr> params;
    params.push_back(make_pointer(object_type));
    const string lookup_name = qualified + "::~" + simple;
    const TypePtr lookup_type =
        make_function(make_fundamental(FT_VOID), params, false);
    map<string, string>::const_iterator found =
        function_symbols_.find(function_key(lookup_name, lookup_type));
    if(found != function_symbols_.end()) {
      return found->second;
    }
    string symbol = try_lookup_special_member_symbol_by_index(
        function_symbol_entries_,
        function_symbol_lookup_index_,
        lookup_name,
        [&](const TypePtr & entry_type)
        {
          return matches_destructor_entry_type_for_lowir(entry_type, object_type);
        });
    return symbol;
  }

  void note_destructor_symbol_reference(const string & symbol,
                                        const TypePtr & object_type) const
  {
    if(symbol.empty() || !object_type) {
      return;
    }
    vector<TypePtr> params;
    params.push_back(make_pointer(object_type));
    note_referenced_function_signature(
        symbol,
        make_function(make_fundamental(FT_VOID), params, false));
    note_generated_function_reference(symbol);
  }

  string destructor_symbol(const TypePtr & class_type) const
  {
    TypePtr object_type;
    const string symbol = lookup_destructor_symbol_no_note(class_type, &object_type);
    note_destructor_symbol_reference(symbol, object_type);
    return symbol;
  }

  bool destructor_runtime_call_required(const TypePtr & class_type) const
  {
    const string symbol = lookup_destructor_symbol_no_note(class_type);
    return !symbol.empty() && !special_member_symbol_has_trivial_lifecycle(symbol);
  }

  string destructor_symbol_for_runtime_call(const TypePtr & class_type) const
  {
    TypePtr object_type;
    const string symbol = lookup_destructor_symbol_no_note(class_type, &object_type);
    if(symbol.empty() || special_member_symbol_has_trivial_lifecycle(symbol)) {
      return string();
    }
    note_destructor_symbol_reference(symbol, object_type);
    return symbol;
  }

  bool node_references_constructor_this_subobject(const CallSemNode & node) const
  {
    if(node.kind == CallSemKind::id_expression) {
      return node.text == "this";
    }
    return node.kind == CallSemKind::member_expression &&
           node.children.size() == 1 &&
           node_references_constructor_this_subobject(node.children[0]);
  }

  bool try_make_constructor_unwind_cleanup(const CallSemNode & action,
                                           CleanupAction & cleanup) const
  {
    if(!is_constructor_function_ ||
       action.kind != CallSemKind::constructor_action ||
       action.children.size() != 1) {
      return false;
    }

    const CallSemNode & call = action.children[0];
    if(call.kind != CallSemKind::call_expression || call.children.size() < 2) {
      return false;
    }

    const CallSemNode & target_arg = call.children[1];
    if(target_arg.kind != CallSemKind::unary_expression ||
       !callsem_has_token(target_arg, OP_AMP) ||
       target_arg.children.size() != 1 ||
       !node_references_constructor_this_subobject(target_arg.children[0])) {
      return false;
    }

    TypePtr target_type = strip_top_level_cv(remove_reference_type(target_arg.semantic_type));
    if(!target_type || target_type->kind != Type::TK_POINTER || !target_type->inner ||
       !destructor_runtime_call_required(target_type->inner)) {
      return false;
    }

    cleanup.kind = CleanupAction::CK_DESTROY_CLASS_AT_NODE;
    cleanup.node = &target_arg;
    cleanup.object_type = target_type->inner;
    return true;
  }

  void register_constructor_unwind_cleanup(const CallSemNode & action)
  {
    CleanupAction cleanup;
    if(try_make_constructor_unwind_cleanup(action, cleanup)) {
      constructor_unwind_cleanups_.push_back(cleanup);
    }
  }

  void emit_destroy_complete_class_temporary(const TypePtr & type,
                                             const string & object_ptr)
  {
    if(!is_complete_class_value_type(type)) {
      return;
    }
    const string dtor = destructor_symbol_for_runtime_call(type);
    if(dtor.empty()) {
      return;
    }
    emit_line("call void " + dtor + "(" + object_ptr + ")");
  }

  bool is_marked_scalar_delete_expression(const CallSemNode & node) const
  {
    return node.kind == CallSemKind::call_expression &&
           callsem_has_token(node, KW_DELETE) &&
           node.children.size() == 2 &&
           node.children[0].kind == CallSemKind::callee;
  }

  string emit_marked_scalar_delete_expression(const CallSemNode & node)
  {
    if(!is_marked_scalar_delete_expression(node)) {
      throw logic_error("expected marked scalar delete-expression");
    }

    TypePtr pointer_type =
        strip_top_level_cv(remove_reference_type(node.children[1].semantic_type));
    if(!pointer_type || pointer_type->kind != Type::TK_POINTER) {
      throw logic_error("marked scalar delete-expression requires pointer operand");
    }

    const string object_ptr = emit_rvalue(node.children[1]);
    const string nonnull =
        emit_temp_assignment("i64", string("cmp ne ptr ") + object_ptr + ", 0");
    const string delete_label = new_block("delete_nonnull");
    const string end_label = new_block("delete_end");
    terminate("branch " + nonnull + ", " + lowir_block_name(delete_label) +
              ", " + lowir_block_name(end_label));

    start_block(delete_label);
    string delete_ptr = object_ptr;
    if(node.has_uint_value) {
      const string vtable_ptr = emit_temp_assignment("ptr", string("load ptr ") + object_ptr);
      string fn_ptr = vtable_ptr;
      if(node.uses_extended_vtable_layout) {
        string entry_ptr = vtable_ptr;
        const unsigned long long slot_offset = callsem_uint_value(node) * 16ULL;
        if(slot_offset != 0) {
          entry_ptr = emit_temp_assignment("ptr",
                                           string("index i8 ") + vtable_ptr + ", " +
                                           to_string(slot_offset));
        }
        const string adjust_ptr =
            emit_temp_assignment("ptr", string("index i8 ") + entry_ptr + ", 8");
        const string this_adjust =
            emit_temp_assignment("i64", string("load i64 ") + adjust_ptr);
        delete_ptr =
            emit_temp_assignment("ptr", string("index i8 ") + object_ptr + ", " + this_adjust);
        fn_ptr = emit_temp_assignment("ptr", string("load ptr ") + entry_ptr);
      } else {
        if(callsem_uint_value(node) != 0) {
          fn_ptr = emit_temp_assignment("ptr",
                                        string("index i8 ") + vtable_ptr + ", " +
                                        to_string(callsem_uint_value(node) * 8ULL));
        }
        fn_ptr = emit_temp_assignment("ptr", string("load ptr ") + fn_ptr);
      }
      emit_line("call void " + fn_ptr + "(" + delete_ptr + ")" +
                lowir_call_signature_suffix(
                    make_function(make_fundamental(FT_VOID),
                                  vector<TypePtr>(1, pointer_type),
                                  false)));
      terminate("jump " + lowir_block_name(end_label));
    } else {
      const string dtor =
          pointer_type->inner ? destructor_symbol_for_runtime_call(pointer_type->inner) : string();
      if(!dtor.empty()) {
        emit_line("call void " + dtor + "(" + object_ptr + ")");
      }
      emit_line("call void " + lookup_function_symbol(node.children[0]) + "(" +
                delete_ptr + ")");
      terminate("jump " + lowir_block_name(end_label));
    }

    start_block(end_label);
    return "0";
  }

  void emit_clear_current_exception()
  {
    if(use_host_eh_runtime()) {
      emit_line("call void " + external_runtime_symbol("__cxa_end_catch") + "()");
      return;
    }

    const string current_type = emit_current_exception_type();
    const string cleanup_end = new_block("eh_clear_end");
    string next_label = new_block("eh_clear_check");
    terminate("jump " + lowir_block_name(next_label));

    for(map<string, TypePtr>::const_iterator it = exception_storage_types_.begin();
        it != exception_storage_types_.end();
        ++it) {
      const TypePtr & type = it->second;
      if(!is_complete_class_value_type(type)) {
        continue;
      }
      const string dtor = destructor_symbol_for_runtime_call(type);
      if(dtor.empty()) {
        continue;
      }

      start_block(next_label);
      const string match =
          emit_temp_assignment("i64",
                               string("cmp eq ptr ") + current_type + ", " +
                                   emit_rtti_symbol_address(rtti_symbol_for_type(type)));
      const string hit_label = new_block("eh_clear_hit");
      const string miss_label = new_block("eh_clear_next");
      terminate("branch " + match + ", " + lowir_block_name(hit_label) + ", " +
                lowir_block_name(miss_label));

      start_block(hit_label);
      emit_line("call void " + dtor + "(" +
                emit_temp_assignment("ptr", string("addr ") + exception_storage_symbol(type)) +
                ")");
      terminate("jump " + lowir_block_name(cleanup_end));
      next_label = miss_label;
    }

    start_block(next_label);
    terminate("jump " + lowir_block_name(cleanup_end));

    start_block(cleanup_end);
    emit_line("store ptr 0, " + emit_exception_type_address());
    emit_line("store ptr 0, " + emit_exception_storage_slot_address());
  }

  void emit_host_eh_handler_metadata(const CallSemNode & node)
  {
    if(!use_host_eh_runtime()) {
      return;
    }
    const vector<long long> & selectors = host_eh_selectors_for_try(node);
    size_t selector_index = 0;
    for(size_t i = 0; i < node.children.size(); ++i) {
      const CallSemNode & handler = node.children[i];
      if(handler.kind != CallSemKind::catch_handler) {
        continue;
      }
      if(selector_index >= selectors.size()) {
        throw logic_error("host EH selector metadata mismatch");
      }
      const string selector_suffix = ", " + to_string(selectors[selector_index++]);
      if(handler.text == "...") {
        emit_line("eh_catch_all" + selector_suffix);
        continue;
      }
      TypePtr match_type = exception_object_type(handler.semantic_type);
      if(match_type) {
        emit_line("eh_catch " + host_exception_match_rtti_symbol(match_type) +
                  selector_suffix);
      }
    }
  }

  const vector<long long> & host_eh_selectors_for_try(const CallSemNode & node)
  {
    map<const CallSemNode *, vector<long long> >::const_iterator existing =
        host_eh_handler_selectors_.find(&node);
    if(existing != host_eh_handler_selectors_.end()) {
      return existing->second;
    }

    vector<long long> selectors;
    for(size_t i = 0; i < node.children.size(); ++i) {
      const CallSemNode & handler = node.children[i];
      if(handler.kind != CallSemKind::catch_handler) {
        continue;
      }
      selectors.push_back(next_host_eh_selector_++);
    }
    host_eh_handler_selectors_[&node] = selectors;
    return host_eh_handler_selectors_.find(&node)->second;
  }

  CallSemNode & make_synthetic_node(CallSemKind kind, const string & text = string())
  {
    synthetic_nodes_.push_back(unique_ptr<CallSemNode>(new CallSemNode()));
    CallSemNode & node = *synthetic_nodes_.back();
    node = make_dump_node(kind, text);
    return node;
  }

  bool is_special_class_materialization_node(const CallSemNode & node) const
  {
    return node.kind == CallSemKind::closure_object ||
           node.kind == CallSemKind::initializer_list_object ||
           node.kind == CallSemKind::statement_expression ||
           node.kind == CallSemKind::conditional_expression ||
           (node.kind == CallSemKind::binary_expression &&
            callsem_has_token(node, OP_COMMA));
  }

  void emit_discarded_expression(const CallSemNode & node)
  {
    if(node.kind == CallSemKind::throw_statement) {
      emit_statement(node);
      return;
    }

    if(node.kind == CallSemKind::cast_expression &&
       node.children.size() == 1 &&
       is_void_type(node.semantic_type)) {
      emit_discarded_expression(node.children[0]);
      return;
    }

    TypePtr discard_type = node.semantic_type;
    if(is_void_type(discard_type)) {
      if(callsem_conversion_source_type(node)) {
        discard_type = callsem_conversion_source_type(node);
      } else if(callsem_materialization_source_type(node)) {
        discard_type = callsem_materialization_source_type(node);
      }
    }
    TypePtr discard_base = strip_top_level_cv(remove_reference_type(discard_type));
    if(node.kind == CallSemKind::braced_init_list &&
       discard_base &&
       discard_base->kind == Type::TK_ARRAY) {
      const string temp_ptr = new_hidden_object_address(discard_type, "discardarr");
      emit_local_array_initializer(discard_type, node, temp_ptr);
      return;
    }

    if(node.kind == CallSemKind::braced_init_list) {
      for(size_t i = 0; i < node.children.size(); ++i) {
        emit_discarded_expression(node.children[i]);
        if(!current_block_) {
          return;
        }
      }
      return;
    }

    if(emit_discarded_indirect_prvalue(node)) {
      return;
    }

    const bool can_discard_by_address =
        node.kind == CallSemKind::id_expression ||
        node.kind == CallSemKind::variable ||
        node.kind == CallSemKind::member_expression ||
        node.kind == CallSemKind::subscript_expression ||
        node.kind == CallSemKind::conditional_expression ||
        node.value_category == CVC_LVALUE ||
        is_reference_type(node.semantic_type);
    if(can_discard_by_address &&
       discard_base &&
       !is_function_type(discard_base) &&
       (is_class_like_value_type(discard_type) ||
        is_indirect_value_type(discard_type) ||
        discard_base->kind == Type::TK_ARRAY)) {
      emit_lvalue_address(node);
      return;
    }

    if(node.kind == CallSemKind::binary_expression &&
       callsem_has_token(node, OP_COMMA)) {
      if(node.children.size() != 2) {
        throw logic_error("binary-expression arity");
      }
      emit_discarded_expression(node.children[0]);
      if(current_block_) {
        emit_discarded_expression(node.children[1]);
      }
      return;
    }

    if(node.kind == CallSemKind::binary_expression &&
       (callsem_has_token(node, OP_LAND) || callsem_has_token(node, OP_LOR))) {
      if(node.children.size() != 2) {
        throw logic_error("binary-expression arity");
      }
      const bool is_land = callsem_has_token(node, OP_LAND);
      const string rhs_label = new_block(is_land ? "land_rhs" : "lor_rhs");
      const string short_label = new_block(is_land ? "land_short" : "lor_short");
      const string end_label = new_block(is_land ? "land_end" : "lor_end");
      const string lhs = emit_branch_condition_value(node.children[0]);
      terminate(string("branch ") + lhs + ", " +
                lowir_block_name(is_land ? rhs_label : short_label) + ", " +
                lowir_block_name(is_land ? short_label : rhs_label));

      start_block(rhs_label);
      emit_discarded_expression(node.children[1]);
      if(current_block_) {
        terminate(string("jump ") + lowir_block_name(end_label));
      }

      start_block(short_label);
      terminate(string("jump ") + lowir_block_name(end_label));
      start_block(end_label);
      return;
    }

    if(node.kind == CallSemKind::conditional_expression) {
      if(node.children.size() != 3) {
        throw logic_error("conditional-expression arity");
      }

      const string then_label = new_block("discard_cond_then");
      const string else_label = new_block("discard_cond_else");
      const string end_label = new_block("discard_cond_end");
      const string cond = emit_branch_condition_value(node.children[0]);
      terminate(string("branch ") + cond + ", " + lowir_block_name(then_label) + ", " +
                lowir_block_name(else_label));

      const auto emit_discarded_branch = [&](const CallSemNode & branch)
      {
        push_cleanup_scope();
        emit_discarded_expression(branch);
        if(current_block_) {
          emit_scope_cleanups(cleanup_scopes_.back());
        }
        pop_cleanup_scope();
      };

      bool then_fallthrough = false;
      start_block(then_label);
      emit_discarded_branch(node.children[1]);
      if(current_block_) {
        terminate(string("jump ") + lowir_block_name(end_label));
        then_fallthrough = true;
      }

      bool else_fallthrough = false;
      start_block(else_label);
      emit_discarded_branch(node.children[2]);
      if(current_block_) {
        terminate(string("jump ") + lowir_block_name(end_label));
        else_fallthrough = true;
      }

      if(then_fallthrough || else_fallthrough) {
        start_block(end_label);
      } else {
        current_block_ = nullptr;
      }
      return;
    }

    emit_rvalue(node);
  }

  bool emit_special_class_value_to_target(const CallSemNode & node,
                                          const string & target_ptr)
  {
    if(node.kind == CallSemKind::statement_expression) {
      if(node.children.empty() ||
         node.children.size() > 2 ||
         node.children[0].kind != CallSemKind::compound_statement) {
        throw logic_error("statement-expression shape");
      }

      push_cleanup_scope();
      push_binding_scope();
      const CallSemNode & prefix = node.children[0];
      for(size_t i = 0; i < prefix.children.size(); ++i) {
        emit_statement(prefix.children[i]);
      }

      if(node.children.size() == 2) {
        if(!current_block_) {
          throw logic_error("statement-expression prefix terminated control flow");
        }
        emit_storage_value_to_target(node.semantic_type, node.children[1], target_ptr);
      }

      if(current_block_) {
        emit_scope_cleanups(cleanup_scopes_.back());
      }
      pop_cleanup_scope();
      pop_binding_scope();
      return true;
    }

    if(node.kind == CallSemKind::binary_expression &&
       callsem_has_token(node, OP_COMMA)) {
      if(node.children.size() != 2) {
        throw logic_error("binary-expression arity");
      }
      emit_discarded_expression(node.children[0]);
      emit_storage_value_to_target(node.semantic_type, node.children[1], target_ptr);
      return true;
    }

    if(node.kind == CallSemKind::conditional_expression) {
      if(node.children.size() != 3) {
        throw logic_error("conditional-expression arity");
      }

      const string then_label = new_block("condobj_then");
      const string else_label = new_block("condobj_else");
      const string end_label = new_block("condobj_end");
      const string cond = emit_branch_condition_value(node.children[0]);
      terminate(string("branch ") + cond + ", " + lowir_block_name(then_label) + ", " +
                lowir_block_name(else_label));

      const auto emit_branch_value = [&](const CallSemNode & branch)
      {
        push_cleanup_scope();
        if(emit_special_class_value_to_target(branch, target_ptr)) {
          if(current_block_) {
            emit_scope_cleanups(cleanup_scopes_.back());
          }
          pop_cleanup_scope();
          return;
        }
        if(is_indirect_class_reference_type(branch.semantic_type)) {
          if(branch.value_category == CVC_XVALUE) {
            emit_move_construct_to_target(node.semantic_type,
                                          target_ptr,
                                          emit_rvalue(branch));
          } else {
            emit_copy_construct_to_target(node.semantic_type,
                                          target_ptr,
                                          emit_rvalue(branch));
          }
          if(current_block_) {
            emit_scope_cleanups(cleanup_scopes_.back());
          }
          pop_cleanup_scope();
          return;
        }
        if(branch.kind == CallSemKind::call_expression &&
           (is_indirect_value_type(branch.semantic_type) ||
            is_complete_class_value_type(branch.semantic_type))) {
          emit_call_expression_to_target(branch, target_ptr);
          if(current_block_) {
            emit_scope_cleanups(cleanup_scopes_.back());
          }
          pop_cleanup_scope();
          return;
        }
        emit_copy_construct_to_target(node.semantic_type,
                                      target_ptr,
                                      emit_lvalue_address(branch));
        if(current_block_) {
          emit_scope_cleanups(cleanup_scopes_.back());
        }
        pop_cleanup_scope();
      };

      start_block(then_label);
      emit_branch_value(node.children[1]);
      terminate(string("jump ") + lowir_block_name(end_label));

      start_block(else_label);
      emit_branch_value(node.children[2]);
      terminate(string("jump ") + lowir_block_name(end_label));

      start_block(end_label);
      return true;
    }

    if(node.kind == CallSemKind::closure_object) {
      for(size_t i = 0; i < node.children.size(); ++i) {
        const CallSemNode & capture = node.children[i];
        if(capture.kind != CallSemKind::closure_capture || capture.children.size() != 1 ||
           !capture.has_uint_value) {
          throw logic_error("invalid closure object shape");
        }
        const TypePtr capture_type = capture.semantic_type;
        const string field_ptr =
            emit_byte_offset_address(target_ptr, callsem_uint_value(capture));
        if(is_reference_type(capture_type)) {
          emit_line("store ptr " +
                    emit_reference_storage_value(remove_reference_type(capture_type),
                                                capture.children[0]) +
                    ", " + field_ptr);
          continue;
        }
        if(is_indirect_value_type(capture_type)) {
          emit_copy_construct_to_target(capture_type,
                                        field_ptr,
                                        emit_lvalue_address(capture.children[0]));
          continue;
        }
        const string value =
            emit_scalar_storage_value(capture_type, capture.children[0]);
        emit_line("store " + lowir_memory_type_for(capture_type) + " " + value + ", " + field_ptr);
      }
      return true;
    }

    if(node.kind == CallSemKind::initializer_list_object) {
      TypePtr initlist_type = strip_top_level_cv(node.semantic_type);
      if(!initlist_type || initlist_type->kind != Type::TK_NAMED ||
         initlist_type->named_key.find("class std::initializer_list<") != 0) {
        throw logic_error("invalid initializer_list object");
      }
      const size_t begin_offset = 0;
      const size_t size_offset = 8;

      const TypePtr element_object_type =
          strip_top_level_cv(remove_reference_type(
              callsem_initializer_list_element_type(node)));
      if(!element_object_type) {
        throw logic_error("missing initializer_list element type");
      }

      string begin_ptr = "0";
      if(!node.children.empty()) {
        if(is_complete_class_value_type(element_object_type)) {
          const size_t element_stride = backend_storage_size(element_object_type);
          const string element_storage =
              new_hidden_slot(lowir_storage_type_for_span(element_stride * node.children.size(),
                                                          backend_storage_alignment(
                                                              element_object_type)),
                              "initlist");
          begin_ptr = emit_storage_address(element_storage);
          for(size_t i = 0; i < node.children.size(); ++i) {
            const string element_ptr =
                emit_byte_offset_address(begin_ptr, i * element_stride);
            const CallSemNode & element = node.children[i];
            if(element.kind == CallSemKind::closure_object ||
               element.kind == CallSemKind::initializer_list_object) {
              emit_special_class_value_to_target(element, element_ptr);
            } else if(element.kind == CallSemKind::call_expression &&
                      is_indirect_value_type(element.semantic_type)) {
              emit_call_expression_to_target(element, element_ptr);
            } else {
              emit_copy_construct_to_target(element_object_type,
                                            element_ptr,
                                            emit_lvalue_address(element));
            }
          }
        } else {
          const string element_type =
              lowir_memory_type_for(strip_top_level_cv(element_object_type));
          const size_t element_stride = backend_storage_size(element_object_type);
          const string element_storage =
              new_hidden_slot(lowir_storage_type_for_span(element_stride * node.children.size(),
                                                          backend_storage_alignment(
                                                              element_object_type)),
                              "initlist");
          begin_ptr = emit_storage_address(element_storage);
          for(size_t i = 0; i < node.children.size(); ++i) {
            const string value =
                emit_scalar_storage_value(element_object_type, node.children[i]);
            emit_line("store " + element_type + " " + value + ", " +
                      emit_byte_offset_address(begin_ptr, i * element_stride));
          }
        }
      }

      emit_line("store ptr " + begin_ptr + ", " +
                emit_byte_offset_address(target_ptr, begin_offset));
      emit_line("store i64 " + to_string(node.children.size()) + ", " +
                emit_byte_offset_address(target_ptr, size_offset));
      return true;
    }

    return false;
  }

  string emit_typeid_value(const CallSemNode & node)
  {
    if(node.kind != CallSemKind::typeid_expression) {
      throw logic_error("invalid typeid node");
    }

    if(node.children.empty()) {
      const TypePtr typeid_operand_type = callsem_typeid_operand_type(node);
      if(use_host_eh_runtime() && typeid_operand_type) {
        return emit_host_typeid_rtti_symbol_address(typeid_operand_type);
      }
      return emit_rtti_symbol_address(node.text);
    }

    const string object_ptr = emit_lvalue_address(node.children[0]);
    const string null_test =
        emit_temp_assignment("i64", string("cmp eq ptr ") + object_ptr + ", 0");
    const string fail_label = new_block("typeid_fail");
    const string scan_label = new_block("typeid_scan");
    terminate("branch " + null_test + ", " + lowir_block_name(fail_label) + ", " +
              lowir_block_name(scan_label));

    start_block(fail_label);
    emit_rtti_failure_throw("__cxa_bad_typeid");

    if(use_host_eh_runtime()) {
      start_block(scan_label);
      return emit_host_dynamic_typeinfo_address(object_ptr);
    }

    const string result_slot = new_hidden_slot("ptr", "typeid");
    const string end_label = new_block("typeid_end");
    start_block(scan_label);
    const string fallback = emit_rtti_symbol_address(node.text);
    emit_line("store ptr " + fallback + ", " + result_slot);
    const string vptr = emit_temp_assignment("ptr", string("load ptr ") + object_ptr);
    string next_label = new_block("typeid_check");
    terminate("jump " + lowir_block_name(next_label));

    for(size_t i = 1; i < node.children.size(); ++i) {
      const CallSemNode & candidate = node.children[i];
      if(candidate.kind != CallSemKind::rtti_candidate) {
        continue;
      }
      start_block(next_label);
      const string candidate_vtable =
          emit_vtable_address_point(candidate.text, candidate.semantic_type);
      const string is_match =
          emit_temp_assignment("i64", string("cmp eq ptr ") + vptr + ", " + candidate_vtable);
      const string match_label = new_block("typeid_match");
      const string miss_label = new_block("typeid_next");
      terminate("branch " + is_match + ", " + lowir_block_name(match_label) + ", " +
                lowir_block_name(miss_label));

      start_block(match_label);
      emit_line("store ptr " + emit_rtti_symbol_address(rtti_symbol_for_type(candidate.semantic_type)) +
                ", " + result_slot);
      terminate("jump " + lowir_block_name(end_label));

      next_label = miss_label;
    }

    start_block(next_label);
    terminate("jump " + lowir_block_name(end_label));

    start_block(end_label);
    return emit_temp_assignment("ptr", string("load ptr ") + result_slot);
  }

  string emit_dynamic_cast_value(const CallSemNode & node)
  {
    if(node.kind != CallSemKind::dynamic_cast_expression || node.children.empty()) {
      throw logic_error("invalid dynamic_cast node");
    }

    const bool reference_result = is_reference_type(node.semantic_type);
    const TypePtr target_base = strip_top_level_cv(node.semantic_type);
    const bool target_pointer_form = target_base && target_base->kind == Type::TK_POINTER;
    const bool target_reference_form =
        target_base &&
        (target_base->kind == Type::TK_LVALUE_REFERENCE ||
         target_base->kind == Type::TK_RVALUE_REFERENCE);
    const TypePtr operand_type =
        strip_top_level_cv(remove_reference_type(node.children[0].semantic_type));
    const bool reference_operand =
        !operand_type || operand_type->kind != Type::TK_POINTER;
    const TypePtr source_object_type =
        strip_top_level_cv(reference_operand ? operand_type :
                                               (operand_type ? operand_type->inner : TypePtr()));
    const TypePtr target_object_type =
        strip_top_level_cv((target_pointer_form || target_reference_form) && target_base ?
                               target_base->inner :
                               TypePtr());
    const string source_ptr =
        reference_operand ? emit_lvalue_address(node.children[0]) : emit_rvalue(node.children[0]);
    const string result_slot = new_hidden_slot("ptr", "dyn_cast");
    emit_line("store ptr 0, " + result_slot);

    const string null_test = emit_temp_assignment("i64", string("cmp eq ptr ") + source_ptr + ", 0");
    const string scan_label = new_block("dyn_cast_scan");
    const string end_label = new_block("dyn_cast_end");
    terminate("branch " + null_test + ", " + lowir_block_name(end_label) + ", " +
              lowir_block_name(scan_label));

    if(use_host_eh_runtime()) {
      start_block(scan_label);
      if(target_pointer_form && target_object_type && is_void_type(target_object_type)) {
        emit_line("store ptr " + emit_host_dynamic_cast_void_pointer(source_ptr) + ", " +
                  result_slot);
        terminate("jump " + lowir_block_name(end_label));
      }

      const string source_rtti = emit_host_rtti_symbol_address(source_object_type);
      const string target_rtti = emit_host_rtti_symbol_address(target_object_type);
      const string runtime_result =
          emit_temp_assignment("ptr",
                               string("call ptr ") +
                                   external_runtime_symbol("__dynamic_cast") + "(" +
                                   source_ptr + ", " +
                                   source_rtti + ", " +
                                   target_rtti + ", " +
                                   to_string(host_dynamic_cast_src2dst_hint(
                                       node, source_object_type, target_object_type)) + ")");
      emit_line("store ptr " + runtime_result + ", " + result_slot);
      if(reference_result) {
        const string fail_label = new_block("dyn_cast_fail");
        const string found_label = new_block("dyn_cast_found");
        const string is_null =
            emit_temp_assignment("i64", string("cmp eq ptr ") + runtime_result + ", 0");
        terminate("branch " + is_null + ", " + lowir_block_name(fail_label) + ", " +
                  lowir_block_name(found_label));

        start_block(fail_label);
        emit_rtti_failure_throw("__cxa_bad_cast");

        start_block(found_label);
        terminate("jump " + lowir_block_name(end_label));
      }
      terminate("jump " + lowir_block_name(end_label));

      start_block(end_label);
      return emit_temp_assignment("ptr", string("load ptr ") + result_slot);
    }

    start_block(scan_label);
    const string vptr = emit_temp_assignment("ptr", string("load ptr ") + source_ptr);
    string next_label = new_block("dyn_cast_check");
    terminate("jump " + lowir_block_name(next_label));

    for(size_t i = 1; i < node.children.size(); ++i) {
      const CallSemNode & candidate = node.children[i];
      if(candidate.kind != CallSemKind::rtti_candidate) {
        continue;
      }
      start_block(next_label);
      const string candidate_vtable =
          emit_vtable_address_point(candidate.text, candidate.semantic_type);
      const string is_match =
          emit_temp_assignment("i64", string("cmp eq ptr ") + vptr + ", " + candidate_vtable);
      const string match_label = new_block("dyn_cast_match");
      const string miss_label = new_block("dyn_cast_next");
      terminate("branch " + is_match + ", " + lowir_block_name(match_label) + ", " +
                lowir_block_name(miss_label));

      start_block(match_label);
      if(candidate.has_int_value && callsem_int_value(candidate) != 0) {
        emit_line("store ptr " +
                  emit_temp_assignment("ptr",
                                       string("index i8 ") + source_ptr + ", " +
                                       to_string(callsem_int_value(candidate))) +
                  ", " + result_slot);
      } else {
        emit_line("store ptr " + source_ptr + ", " + result_slot);
      }
      terminate("jump " + lowir_block_name(end_label));

      next_label = miss_label;
    }

    start_block(next_label);
    if(reference_result) {
      emit_rtti_failure_throw("__cxa_bad_cast");
    } else {
      terminate("jump " + lowir_block_name(end_label));
    }

    start_block(end_label);
    return emit_temp_assignment("ptr", string("load ptr ") + result_slot);
  }

  void emit_copy_construct_to_target(const TypePtr & class_type,
                                     const string & target_ptr,
                                     const string & source_ptr)
  {
    const string symbol = copy_constructor_symbol(class_type);
    if(!symbol.empty() &&
       !special_member_symbol_has_trivial_lifecycle(symbol)) {
      note_generated_function_reference(symbol);
      emit_line("call void " + symbol + "(" + target_ptr + ", " + source_ptr + ")");
      return;
    }
    if(is_empty_class_storage_type(class_type)) {
      return;
    }
    if(parser_trace::enabled("lowir.copy")) {
      ostringstream trace;
      trace << "action=copy-ctor-fallback kind=copyobj class="
            << class_qualified_name(strip_top_level_cv(remove_reference_type(class_type)))
            << " type=" << describe_type(class_type)
            << " storage=" << storage_span_text(class_type)
            << " source=" << source_ptr
            << " target=" << target_ptr;
      parser_trace::note("lowir.copy", string(), trace.str());
    }
    emit_line("copyobj " + storage_span_text(class_type) + " " +
              source_ptr + ", " + target_ptr);
  }

  void emit_move_construct_to_target(const TypePtr & class_type,
                                     const string & target_ptr,
                                     const string & source_ptr)
  {
    const string symbol = move_constructor_symbol(class_type);
    if(!symbol.empty() &&
       !special_member_symbol_has_trivial_lifecycle(symbol)) {
      note_generated_function_reference(symbol);
      emit_line("call void " + symbol + "(" + target_ptr + ", " + source_ptr + ")");
      return;
    }
    const string copy_symbol = copy_constructor_symbol(class_type);
    if(!copy_symbol.empty() &&
       !special_member_symbol_has_trivial_lifecycle(copy_symbol)) {
      note_generated_function_reference(copy_symbol);
      emit_line("call void " + copy_symbol + "(" + target_ptr + ", " + source_ptr + ")");
      return;
    }
    if(is_empty_class_storage_type(class_type)) {
      return;
    }
    emit_line("copyobj " + storage_span_text(class_type) + " " +
              source_ptr + ", " + target_ptr);
  }

  bool should_implicitly_move_return_object(const CallSemNode & node) const
  {
    if(node.kind != CallSemKind::id_expression ||
       !node.implicit_return_move_eligible ||
       !is_complete_class_value_type(node.semantic_type)) {
      return false;
    }
    return true;
  }

  string named_return_slot_alias_address(const CallSemNode & node) const
  {
    if(node.kind != CallSemKind::id_expression) {
      return string();
    }
    const VariableBinding * binding = find_local_binding(node.text);
    if(!binding ||
       !binding->is_named_return_slot_alias ||
       !binding_has_external_storage_address(*binding)) {
      return string();
    }
    return binding->external_storage_address;
  }

  bool is_indirect_class_reference_type(const TypePtr & type)
  {
    return is_reference_type(type) &&
           is_complete_class_value_type(remove_reference_type(type));
  }

  string emit_call_argument(const TypePtr & param_type, const CallSemNode & arg)
  {
    if(callsem_has_token(arg, OP_AMP) &&
       arg.children.size() == 1 &&
       param_type &&
       describe_type(param_type).find(
           "pointer to") != string::npos &&
       describe_type(param_type).find(
           "std::__1::ostreambuf_iterator<char, std::__1::char_traits<char>>") != string::npos &&
       arg.children[0].semantic_type &&
       describe_type(arg.children[0].semantic_type) ==
           "class std::__1::ostreambuf_iterator<char, std::__1::char_traits<char>>") {
      return emit_lvalue_address(arg.children[0]);
    }

    TypePtr param_base = strip_top_level_cv(param_type);
    if(param_base && param_base->kind == Type::TK_ARRAY) {
      if(arg.kind == CallSemKind::braced_init_list) {
        const string temp_ptr = new_hidden_object_address(arg.semantic_type, "argarr");
        emit_local_array_initializer(arg.semantic_type, arg, temp_ptr);
        return temp_ptr;
      }
      return emit_array_base(arg);
    }

    if(is_reference_type(param_type)) {
      TypePtr referent_type = remove_reference_type(param_type);
      TypePtr referent_base = strip_top_level_cv(referent_type);
      if(referent_base &&
         referent_base->kind == Type::TK_ARRAY &&
         arg.kind == CallSemKind::braced_init_list) {
        const string temp_ptr = new_hidden_object_address(referent_type, "argarr");
        emit_local_array_initializer(referent_type, arg, temp_ptr);
        return temp_ptr;
      }
      if((arg.is_base_subobject || arg.is_virtual_base_subobject) &&
         arg.value_category != CVC_LVALUE) {
        return emit_lvalue_address(arg);
      }
      if(param_base && is_indirect_value_type(param_base->inner)) {
        if(arg.value_category == CVC_PRVALUE) {
          const string temp_ptr = new_hidden_object_address(referent_type, "arg");
          emit_storage_value_to_target(referent_type, arg, temp_ptr);
          register_materialized_temporary_cleanup_live(referent_type, temp_ptr);
          return temp_ptr;
        }
        if(is_reference_type(arg.semantic_type) ||
           arg.value_category == CVC_LVALUE) {
          return emit_reference_storage_value(referent_type, arg);
        }
        if(arg.value_category == CVC_XVALUE &&
           is_indirect_value_type(arg.semantic_type)) {
          return emit_lvalue_address(arg);
        }
        if(arg.kind == CallSemKind::call_expression &&
           is_indirect_value_type(arg.semantic_type)) {
          return emit_lvalue_address(arg);
        }
        ostringstream out;
        out << "indirect reference temporary unsupported in PA16 LowIR";
        out << " [param " << describe_type(param_type) << "]";
        if(arg.semantic_type) {
          out << " [arg-type " << describe_type(arg.semantic_type) << "]";
          TypePtr arg_base = strip_top_level_cv(remove_reference_type(arg.semantic_type));
          if(arg_base && arg_base->kind == Type::TK_NAMED) {
            out << " [arg-has-layout " << (arg_base->named_has_layout ? "yes" : "no") << "]";
            out << " [arg-key " << arg_base->named_key << "]";
          }
          out << " [arg-indirect " << (is_indirect_value_type(arg.semantic_type) ? "yes" : "no")
              << "]";
        }
        out << " [arg-kind " << callsem_kind_text(arg.kind) << "]";
        if(!arg.text.empty()) {
          out << " [arg-text " << arg.text << "]";
        }
        out << " [arg-vc "
            << (arg.value_category == CVC_LVALUE ? "lvalue" :
                arg.value_category == CVC_PRVALUE ? "prvalue" : "xvalue")
            << "]";
        if(function_node_) {
          out << " [function " << function_node_->text << "]";
        }
        if(arg.kind == CallSemKind::call_expression && !arg.children.empty()) {
          if(!arg.children[0].text.empty()) {
            out << " [arg-callee " << arg.children[0].text << "]";
          }
          if(arg.children[0].semantic_type) {
            out << " [arg-callee-type " << describe_type(arg.children[0].semantic_type) << "]";
          }
        }
        throw logic_error(out.str());
      }
      if(is_reference_type(arg.semantic_type) ||
         arg.value_category == CVC_LVALUE) {
        return emit_reference_storage_value(referent_type, arg);
      }
      if(is_complete_class_value_type(referent_type)) {
        const string temp_ptr = new_hidden_object_address(referent_type, "arg");
        emit_storage_value_to_target(referent_type, arg, temp_ptr);
        register_materialized_temporary_cleanup_live(referent_type, temp_ptr);
        return temp_ptr;
      }
      if(referent_base &&
         !is_indirect_value_type(referent_type) &&
         !is_function_type(referent_base) &&
         referent_base->kind != Type::TK_ARRAY) {
        const string memory_type = lowir_memory_type_for(referent_type);
        const string temp_slot = new_hidden_slot(memory_type, "refarg");
        const string value =
            emit_scalar_value_conversion(emit_rvalue(arg),
                                         arg.semantic_type,
                                         referent_type,
                                         true);
        emit_line("store " + memory_type + " " + value + ", " + temp_slot);
        return emit_storage_address(temp_slot);
      }
      return emit_rvalue(arg);
    }

    if(is_indirect_value_type(param_type)) {
      TypePtr object_param_type =
          strip_top_level_cv(remove_reference_type(param_type));
      if(is_complete_class_value_type(object_param_type)) {
        const string temp_ptr = new_hidden_object_address(object_param_type, "arg");
        if(emit_special_class_value_to_target(arg, temp_ptr)) {
          return temp_ptr;
        }
        emit_storage_value_to_target(object_param_type, arg, temp_ptr);
        return temp_ptr;
      }
      if(is_indirect_class_reference_type(arg.semantic_type)) {
        return emit_rvalue(arg);
      }
      if(is_indirect_value_type(arg.semantic_type)) {
        return emit_lvalue_address(arg);
      }
      if(arg.value_category == CVC_LVALUE) {
        return emit_lvalue_address(arg);
      }
      ostringstream out;
      out << "indirect value argument requires lvalue in PA16 LowIR";
      out << " [arg kind " << callsem_kind_text(arg.kind) << "]";
      if(!arg.text.empty()) {
        out << " [arg text " << arg.text << "]";
      }
      if(arg.semantic_type) {
        out << " [arg type " << describe_type(arg.semantic_type) << "]";
      }
      out << " [arg vc " << int(arg.value_category) << "]";
      if(param_type) {
        out << " [param type " << describe_type(param_type) << "]";
      }
      out << " [arg child_count " << arg.children.size() << "]";
      throw logic_error(out.str());
    }

    if(param_base && param_base->kind == Type::TK_POINTER &&
       is_class_like_value_type(arg.semantic_type)) {
      const TypePtr arg_object_type = remove_reference_type(arg.semantic_type);
      if(is_reference_type(arg.semantic_type) || arg.value_category == CVC_LVALUE) {
        return emit_lvalue_address(arg);
      }
      if(is_complete_class_value_type(arg.semantic_type) &&
         is_special_class_materialization_node(arg)) {
        const string temp_ptr = new_hidden_object_address(arg_object_type, "arg");
        if(emit_special_class_value_to_target(arg, temp_ptr)) {
          register_materialized_temporary_cleanup_live(arg_object_type, temp_ptr);
          return temp_ptr;
        }
      }
      if(is_complete_class_value_type(arg.semantic_type) &&
         arg.kind == CallSemKind::call_expression) {
        const string temp_ptr = new_hidden_object_address(arg_object_type, "arg");
        emit_call_expression_to_target(arg, temp_ptr);
        register_materialized_temporary_cleanup_live(arg_object_type, temp_ptr);
        return temp_ptr;
      }
    }

    return emit_scalar_value_conversion(emit_rvalue(arg),
                                        arg.semantic_type,
                                        param_type,
                                        true);
  }

  void append_call_argument_values(vector<string> & args,
                                   const TypePtr & param_type,
                                   const CallSemNode & arg)
  {
    const TypePtr lowered_param_type = lowir_parameter_type_for(param_type);
    const string direct_object_type = lowir_direct_object_type(lowered_param_type);
    if(!direct_object_type.empty()) {
      const TypePtr object_type =
          strip_top_level_cv(remove_reference_type(lowered_param_type));
      const vector<string> object_slots = new_hidden_object_slots(object_type, "argobj");
      emit_storage_value_to_target(object_type, arg, emit_storage_address(object_slots[0]));
      args.push_back(object_slots[0]);
      return;
    }

    args.push_back(emit_call_argument(param_type, arg));
  }

  TypePtr default_promoted_variadic_argument_type(const CallSemNode & arg) const
  {
    TypePtr value_type = lowir_value_conversion_type(arg.semantic_type);
    TypePtr base = strip_top_level_cv(value_type);
    if(base && base->kind == Type::TK_FUNDAMENTAL &&
       base->fundamental == FT_FLOAT) {
      return make_fundamental(FT_DOUBLE);
    }
    return semantic_conversion::promoted_integral_type(base);
  }

  string emit_variadic_call_argument(const CallSemNode & arg)
  {
    TypePtr promoted = default_promoted_variadic_argument_type(arg);
    if(!promoted) {
      return emit_rvalue(arg);
    }
    return emit_scalar_value_conversion(emit_rvalue(arg),
                                        arg.semantic_type,
                                        promoted,
                                        true);
  }

  void append_variadic_call_argument_value(vector<string> & args,
                                           const CallSemNode & arg)
  {
    args.push_back(emit_variadic_call_argument(arg));
  }

  string emit_reference_return_address(const CallSemNode & value_node)
  {
    if(!is_reference_type(function_result_type_)) {
      return emit_rvalue(value_node);
    }
    if(is_reference_type(value_node.semantic_type) ||
       value_node.value_category == CVC_LVALUE) {
      return emit_lvalue_address(value_node);
    }

    TypePtr referent_type = remove_reference_type(function_result_type_);
    TypePtr referent_base = strip_top_level_cv(referent_type);
    TypePtr value_base = strip_top_level_cv(value_node.semantic_type);
    if((value_node.kind == CallSemKind::id_expression ||
        value_node.kind == CallSemKind::variable) &&
       value_node.text == "this" &&
       value_base &&
       value_base->kind == Type::TK_POINTER &&
       value_base->inner &&
       referent_base &&
       type_equals(strip_top_level_cv(value_base->inner), referent_base)) {
      return emit_rvalue(value_node);
    }
    if(referent_base &&
       !is_indirect_value_type(referent_type) &&
       !is_function_type(referent_base) &&
       referent_base->kind != Type::TK_ARRAY) {
      const string memory_type = lowir_memory_type_for(referent_type);
      const string temp_slot = new_hidden_slot(memory_type, "retref");
      const string value =
          emit_scalar_value_conversion(emit_rvalue(value_node),
                                       value_node.semantic_type,
                                       referent_type,
                                       true);
      emit_line("store " + memory_type + " " + value + ", " + temp_slot);
      return emit_storage_address(temp_slot);
    }

    return emit_lvalue_address(value_node);
  }

  string ensure_direct_object_return_slot()
  {
    if(!direct_object_return_) {
      throw logic_error("direct object return slot requested for non-object return");
    }
    if(direct_object_return_slot_.empty()) {
      direct_object_return_slot_ = new_hidden_object_slots(function_result_type_, "retobj")[0];
    }
    return direct_object_return_slot_;
  }

  void emit_return_object_value_to_target(const CallSemNode & value_node,
                                          const string & target_ptr)
  {
    if(is_complete_class_value_type(value_node.semantic_type) &&
       emit_special_class_value_to_target(value_node, target_ptr)) {
      return;
    }
    if(is_indirect_class_reference_type(value_node.semantic_type)) {
      if(value_node.value_category == CVC_XVALUE) {
        emit_move_construct_to_target(function_result_type_, target_ptr, emit_rvalue(value_node));
      } else {
        emit_copy_construct_to_target(function_result_type_, target_ptr, emit_rvalue(value_node));
      }
      return;
    }
    if(value_node.kind == CallSemKind::call_expression &&
       (is_indirect_value_type(value_node.semantic_type) ||
        is_complete_class_value_type(value_node.semantic_type))) {
      emit_call_expression_to_target(value_node, target_ptr);
      return;
    }
    const string source_ptr = emit_lvalue_address(value_node);
    if(value_node.value_category == CVC_XVALUE) {
      emit_move_construct_to_target(function_result_type_, target_ptr, source_ptr);
    } else if(should_implicitly_move_return_object(value_node)) {
      emit_move_construct_to_target(function_result_type_, target_ptr, source_ptr);
    } else {
      emit_copy_construct_to_target(function_result_type_, target_ptr, source_ptr);
    }
  }

  bool emit_discarded_indirect_prvalue(const CallSemNode & node)
  {
    if(node.kind == CallSemKind::call_expression && !node.children.empty()) {
      TypePtr function_type;
      if(resolve_callable_function_type(node.children[0].semantic_type, function_type) &&
         function_type &&
         function_type->inner &&
         !is_reference_type(function_type->inner) &&
         is_complete_class_value_type(function_type->inner)) {
        const TypePtr result_type = strip_top_level_cv(function_type->inner);
        const string temp_ptr = new_hidden_object_address(result_type, "discard");
        emit_call_expression_to_target(node, temp_ptr);
        emit_destroy_complete_class_temporary(result_type, temp_ptr);
        return true;
      }
    }

    if(node.kind == CallSemKind::call_expression &&
       (is_indirect_value_type(node.semantic_type) ||
        is_complete_class_value_type(node.semantic_type))) {
      const string temp_ptr = new_hidden_object_address(node.semantic_type, "discard");
      emit_call_expression_to_target(node, temp_ptr);
      emit_destroy_complete_class_temporary(node.semantic_type, temp_ptr);
      return true;
    }

    if(is_complete_class_value_type(node.semantic_type) &&
       is_special_class_materialization_node(node)) {
      const string temp_ptr = new_hidden_object_address(node.semantic_type, "discard");
      emit_special_class_value_to_target(node, temp_ptr);
      emit_destroy_complete_class_temporary(node.semantic_type, temp_ptr);
      return true;
    }

    return false;
  }

  bool call_expression_needs_constructor_unwind_wrapper(const CallSemNode & node) const
  {
    if(!is_constructor_function_ ||
       constructor_action_depth_ != 0 ||
       constructor_unwind_cleanups_.empty() ||
       node.kind != CallSemKind::call_expression ||
       node.children.empty() ||
       node.children[0].kind != CallSemKind::callee) {
      return false;
    }

    const string symbol = lookup_function_symbol(node.children[0]);
    return !symbol.empty() && throwing_function_symbols_.count(symbol) != 0;
  }

  bool cleanup_action_needs_host_unwind(const CleanupAction & cleanup) const
  {
    switch(cleanup.kind) {
    case CleanupAction::CK_NODE:
    case CleanupAction::CK_BOUND_LOCAL_NODE:
      return cleanup.node &&
             cleanup.node->kind == CallSemKind::destructor_action &&
             !cleanup.node->trivial_lifecycle;
    case CleanupAction::CK_DESTROY_CLASS_OBJECT:
    case CleanupAction::CK_DESTROY_CLASS_AT_NODE:
    case CleanupAction::CK_DESTROY_CLASS_AT_PTR:
      return true;
    case CleanupAction::CK_EH_END:
    case CleanupAction::CK_CLEAR_EXCEPTION:
      return false;
    }
    return false;
  }

  bool current_scope_has_host_unwind_cleanups() const
  {
    if(!use_host_eh_runtime() || cleanup_emission_depth_ != 0) {
      return false;
    }
    for(size_t i = cleanup_scopes_.size(); i-- > 0;) {
      const vector<CleanupAction> & scope = cleanup_scopes_[i];
      if(any_of(scope.begin(),
                scope.end(),
                [this](const CleanupAction & cleanup)
                {
                  return cleanup_action_needs_host_unwind(cleanup);
                })) {
        return true;
      }
      if(any_of(scope.begin(),
                scope.end(),
                [](const CleanupAction & cleanup)
                {
                  return cleanup.kind == CleanupAction::CK_EH_END;
                })) {
        break;
      }
    }
    return false;
  }

  bool call_expression_needs_host_unwind_wrapper(const CallSemNode & node) const
  {
    return node.kind == CallSemKind::call_expression &&
           current_scope_has_host_unwind_cleanups() &&
           !call_expression_is_known_nothrow(node);
  }

  bool call_expression_is_known_nothrow(const CallSemNode & node) const
  {
    if(node.kind != CallSemKind::call_expression || node.children.empty()) {
      return false;
    }
    if(node.children[0].kind == CallSemKind::callee &&
       node.children[0].text == "__pseudo_destructor") {
      return true;
    }

    const CallSemNode & callee = node.children[0];
    if(callee.kind != CallSemKind::callee) {
      return false;
    }

    const string symbol = lookup_function_symbol(callee);
    if(symbol.empty()) {
      return false;
    }

    lowir_internal::FunctionBoundaryMetadata boundary;
    apply_known_function_boundary_metadata(boundary, symbol);
    map<string, string>::const_iterator external = external_function_symbols_.find(symbol);
    if(external != external_function_symbols_.end()) {
      apply_known_function_boundary_metadata(boundary, external->second);
    }
    map<string, const CallSemNode *>::const_iterator owner =
        function_symbol_nodes_.find(symbol);
    if(owner != function_symbol_nodes_.end()) {
      apply_callsem_function_boundary_metadata(boundary, *owner->second);
    }
    apply_callsem_function_boundary_metadata(boundary, callee);
    return boundary.unwind == lowir_internal::CUM_NO;
  }

  string cleanup_action_signature(const CleanupAction & cleanup) const
  {
    ostringstream out;
    out << cleanup.kind;
    out << "|node=" << cleanup.node;
    out << "|slot=" << cleanup.storage_slot;
    out << "|bound=" << cleanup.bound_local_name;
    out << "|type=";
    if(cleanup.object_type) {
      out << describe_type(cleanup.object_type);
      out << "@" << cleanup.object_type.get();
    } else {
      out << "-";
    }
    return out.str();
  }

  string call_unwind_cleanup_signature() const
  {
    ostringstream out;
    for(size_t i = cleanup_scopes_.size(); i-- > 0;) {
      const vector<CleanupAction> & scope = cleanup_scopes_[i];
      for(size_t j = scope.size(); j-- > 0;) {
        out << "[" << cleanup_action_signature(scope[j]) << "]";
      }
      const bool crossed_try_boundary =
          any_of(scope.begin(),
                 scope.end(),
                 [](const CleanupAction & cleanup)
                 {
                   return cleanup.kind == CleanupAction::CK_EH_END;
                 });
      if(crossed_try_boundary) {
        break;
      }
    }
    return out.str();
  }

  string constructor_unwind_cleanup_signature() const
  {
    ostringstream out;
    for(size_t i = 0; i < constructor_unwind_cleanups_.size(); ++i) {
      out << "[" << cleanup_action_signature(constructor_unwind_cleanups_[i]) << "]";
    }
    return out.str();
  }

  string active_host_cleanup_label_signature() const
  {
    ostringstream out;
    for(size_t i = 0; i < active_host_cleanup_labels_.size(); ++i) {
      out << "[" << active_host_cleanup_labels_[i] << "]";
    }
    return out.str();
  }

  string call_unwind_dispatch_cache_key(bool include_constructor_unwind_cleanups,
                                        size_t host_dispatch_depth) const
  {
    ostringstream out;
    out << "host=" << (use_host_eh_runtime() ? 1 : 0);
    out << "|ctor=" << (include_constructor_unwind_cleanups ? 1 : 0);
    out << "|host_depth=" << host_dispatch_depth;
    out << "|cleanup=" << call_unwind_cleanup_signature();
    if(include_constructor_unwind_cleanups) {
      out << "|ctor_cleanup=" << constructor_unwind_cleanup_signature();
      if(!constructor_function_try_dispatch_labels_.empty()) {
        out << "|ctor_dispatch=" << constructor_function_try_dispatch_labels_.back();
      }
    }
    out << "|host_cleanup_labels=" << active_host_cleanup_label_signature();
    out << "|has_host_target=" << (has_host_eh_dispatch_target() ? 1 : 0);
    if(has_host_eh_dispatch_target()) {
      out << "|host_dispatch=" << host_eh_dispatch_labels_.back();
      out << "|host_target_depth=" << host_eh_dispatch_depths_.back();
      out << "|host_handler=";
      if(!host_eh_handler_nodes_.empty()) {
        out << host_eh_handler_nodes_.back();
      } else {
        out << "0";
      }
    }
    return out.str();
  }

  string shared_call_unwind_dispatch_label(bool include_constructor_unwind_cleanups,
                                           size_t host_dispatch_depth,
                                           bool & created)
  {
    const string key =
        call_unwind_dispatch_cache_key(include_constructor_unwind_cleanups,
                                       host_dispatch_depth);
    map<string, string>::const_iterator found =
        shared_call_unwind_dispatch_labels_.find(key);
    if(found != shared_call_unwind_dispatch_labels_.end()) {
      created = false;
      return found->second;
    }
    const string label = new_block("call_unwind_dispatch");
    shared_call_unwind_dispatch_labels_[key] = label;
    created = true;
    return label;
  }

  void close_shared_host_call_unwind_region()
  {
    if(!shared_host_call_unwind_region_open_) {
      return;
    }
    const bool created_dispatch = shared_host_call_unwind_created_dispatch_;
    const string dispatch_label = shared_host_call_unwind_dispatch_label_;
    const size_t host_dispatch_depth = shared_host_call_unwind_host_dispatch_depth_;
    emit_line("eh_end");
    if(use_host_eh_runtime()) {
      if(host_eh_region_depth_ == 0) {
        throw logic_error("host EH region depth underflow");
      }
      --host_eh_region_depth_;
    }
    shared_host_call_unwind_region_open_ = false;
    shared_host_call_unwind_dispatch_label_.clear();
    shared_host_call_unwind_host_dispatch_depth_ = 0;
    shared_host_call_unwind_created_dispatch_ = false;
    if(created_dispatch) {
      const string end_label = new_block("call_unwind_end");
      terminate_no_close("jump " + lowir_block_name(end_label));
      emit_shared_call_unwind_dispatch_block(dispatch_label,
                                             false,
                                             host_dispatch_depth);
      start_block(end_label);
    }
  }

  void open_shared_host_call_unwind_region()
  {
    if(shared_host_call_unwind_region_open_) {
      return;
    }
    const size_t host_dispatch_depth =
        use_host_eh_runtime() ? host_eh_region_depth_ + 1 : 0;
    bool created_dispatch = false;
    const string dispatch_label =
        shared_call_unwind_dispatch_label(false,
                                          host_dispatch_depth,
                                          created_dispatch);
    emit_line("eh_try " + lowir_block_name(dispatch_label));
    if(use_host_eh_runtime()) {
      ++host_eh_region_depth_;
    }
    shared_host_call_unwind_region_open_ = true;
    shared_host_call_unwind_dispatch_label_ = dispatch_label;
    shared_host_call_unwind_host_dispatch_depth_ = host_dispatch_depth;
    shared_host_call_unwind_created_dispatch_ = created_dispatch;
  }

  bool has_host_eh_dispatch_target() const
  {
    return use_host_eh_runtime() && !host_eh_dispatch_labels_.empty();
  }

  void emit_host_eh_unwind_to_depth(size_t current_depth, size_t target_depth)
  {
    close_shared_host_call_unwind_region();
    if(current_depth < target_depth) {
      throw logic_error("host EH unwind depth underflow");
    }
    for(size_t depth = current_depth; depth > target_depth; --depth) {
      emit_line("eh_end");
    }
  }

  void terminate_host_eh_dispatch_or_resume(size_t current_depth)
  {
    if(has_host_eh_dispatch_target()) {
      emit_host_eh_unwind_to_depth(current_depth, host_eh_dispatch_depths_.back());
      terminate("jump " + lowir_block_name(host_eh_dispatch_labels_.back()));
    } else {
      emit_host_eh_unwind_to_depth(current_depth, 0);
      terminate("resume");
    }
  }

  void emit_call_unwind_dispatch_cleanups(bool include_constructor_unwind_cleanups,
                                          const string & excluded_destroy_ptr = string())
  {
    if(excluded_destroy_ptr.empty()) {
      emit_all_cleanups(true);
    } else {
      emit_all_cleanups_excluding_destroy_at_ptr(excluded_destroy_ptr, true);
    }
    if(include_constructor_unwind_cleanups &&
       is_constructor_function_ &&
       (throw_will_escape_current_function() ||
        !constructor_function_try_dispatch_labels_.empty())) {
      emit_constructor_unwind_cleanups();
    }
  }

  void emit_shared_call_unwind_dispatch_block(const string & dispatch_label,
                                              bool include_constructor_unwind_cleanups,
                                              size_t host_dispatch_depth,
                                              const string & excluded_destroy_ptr = string())
  {
    start_block(dispatch_label);
    if(has_host_eh_dispatch_target() &&
       !host_eh_handler_nodes_.empty() &&
       host_eh_handler_nodes_.back()) {
      emit_host_eh_handler_metadata(*host_eh_handler_nodes_.back());
      emit_line("eh_cleanup");
    }
    emit_call_unwind_dispatch_cleanups(include_constructor_unwind_cleanups,
                                       excluded_destroy_ptr);
    if(has_host_eh_dispatch_target()) {
      terminate_host_eh_dispatch_or_resume(host_dispatch_depth);
    } else if(include_constructor_unwind_cleanups &&
              !constructor_function_try_dispatch_labels_.empty()) {
      emit_line("eh_end");
      terminate("jump " +
                lowir_block_name(constructor_function_try_dispatch_labels_.back()));
    } else {
      terminate("resume");
    }
  }

  string bridge_symbol_for_callee(const CallSemNode & callee,
                                  const TypePtr & function_type) const
  {
    (void)function_type;
    if(callee.kind != CallSemKind::callee) {
      return "";
    }
    if(!callsem_runtime_bridge_symbol(callee).empty()) {
      return callsem_runtime_bridge_symbol(callee);
    }
    const string direct =
        runtime_bridge_symbol_for_function_name_and_type(callee.text, callee.semantic_type);
    if(!direct.empty()) {
      return direct;
    }
    return "";
  }

  void note_runtime_bridge_support_symbol(const string & symbol)
  {
    if(find_num_put_runtime_bridge_spec(symbol)) {
      runtime_bridge_support_symbols_.insert(symbol);
    }
  }

  void apply_virtual_dispatch_view_offset(const CallSemNode & callee, string & object_arg)
  {
    if(!callee.has_virtual_dispatch_view_offset ||
       callsem_virtual_dispatch_view_offset(callee) == 0) {
      return;
    }
    object_arg = emit_temp_assignment(
        "ptr",
        string("index i8 ") + object_arg + ", " +
            to_string(callsem_virtual_dispatch_view_offset(callee)));
  }

  string host_num_put_bridge_iterator_bits_ptr(const string & iterator_arg_ptr)
  {
    return emit_temp_assignment("ptr", string("index i8 ") + iterator_arg_ptr + ", 8");
  }

  void emit_call_expression_to_target_impl(const CallSemNode & node, const string & target_ptr)
  {
    if(node.kind != CallSemKind::call_expression || node.children.empty()) {
      throw logic_error("expected class-returning call-expression");
    }
    TypePtr gnu_complex_component;
    if(node.children[0].kind == CallSemKind::callee &&
       node.children[0].text == "__builtin_complex" &&
       is_gnu_complex_value_type(node.semantic_type, &gnu_complex_component)) {
      if(node.children.size() != 3) {
        throw logic_error("__builtin_complex call-expression arity");
      }
      const string component_memory_type = lowir_memory_type_for(gnu_complex_component);
      const string real_value = emit_rvalue(node.children[1]);
      const string imag_value = emit_rvalue(node.children[2]);
      emit_line("store " + component_memory_type + " " + real_value + ", " + target_ptr);
      const size_t imag_offset = type_size(gnu_complex_component);
      const string imag_ptr =
          emit_temp_assignment("ptr",
                               string("index i8 ") + target_ptr + ", " +
                               to_string(imag_offset));
      emit_line("store " + component_memory_type + " " + imag_value + ", " + imag_ptr);
      return;
    }
    TypePtr function_type;
    if(!resolve_callable_function_type(node.children[0].semantic_type, function_type) ||
       !function_type) {
      throw logic_error("call-expression does not return indirect value");
    }
    TypePtr object_type = remove_reference_type(node.semantic_type);
    if(function_type->inner &&
       !is_reference_type(function_type->inner) &&
       is_complete_class_value_type(function_type->inner)) {
      object_type = strip_top_level_cv(function_type->inner);
    }
    const bool constructor_call =
        is_complete_class_value_type(object_type) &&
        is_void_type(function_type->inner);
    const bool virtual_dispatch =
        !constructor_call &&
        node.children[0].kind == CallSemKind::callee &&
        node.children[0].is_virtual_dispatch;
    const string host_num_put_bridge =
        bridge_symbol_for_callee(node.children[0], function_type);
    const string current_runtime_bridge =
        g_lowir_current_function_node ?
            callsem_runtime_bridge_symbol(*g_lowir_current_function_node) :
            "";
    const string callee_symbol =
        (!virtual_dispatch && node.children[0].kind == CallSemKind::callee) ?
            lookup_function_symbol(node.children[0]) :
            "";
    const string direct_object_result_type =
        !constructor_call ? lowir_direct_object_type(function_type->inner) : string();
    const bool callee_is_direct_symbol =
        !callee_symbol.empty() && callee_symbol[0] == '@';
    const bool explicit_indirect_call_signature =
        virtual_dispatch || !callee_is_direct_symbol;
    if(!constructor_call &&
       direct_object_result_type.empty() &&
       !lowir_uses_indirect_result_boundary(function_type->inner)) {
      throw logic_error("call-expression does not return indirect value");
    }
    if(!constructor_call && !direct_object_result_type.empty()) {
      vector<string> call_args;
      for(size_t i = 1; i < node.children.size(); ++i) {
        const size_t param_index = i - 1;
        if(param_index < function_type->params.size()) {
          append_call_argument_values(call_args,
                                      function_type->params[param_index],
                                      node.children[i]);
        } else {
          append_variadic_call_argument_value(call_args, node.children[i]);
        }
      }
      append_vtt_argument(node, 1, call_args);
      append_hidden_virtual_base_arguments(node, call_args, 0);
      append_parameter_virtual_base_arguments(node, false, call_args);

      if(!host_num_put_bridge.empty() &&
         host_num_put_bridge != current_runtime_bridge) {
        note_runtime_bridge_support_symbol(host_num_put_bridge);
        vector<string> bridge_args = call_args;
        if(bridge_args.size() >= 2) {
          bridge_args[1] = host_num_put_bridge_iterator_bits_ptr(bridge_args[1]);
        }
        ostringstream bridge_call;
        bridge_call << "call void @" << host_num_put_bridge << "(" << target_ptr;
        for(size_t i = 0; i < bridge_args.size(); ++i) {
          bridge_call << ", " << bridge_args[i];
        }
        bridge_call << ")";
        emit_line(bridge_call.str());
        return;
      }

      ostringstream op;
      op << "call " << direct_object_result_type << " ";
      if(virtual_dispatch) {
        if(call_args.empty()) {
          throw logic_error("virtual direct-object call missing object argument");
        }
        apply_virtual_dispatch_view_offset(node.children[0], call_args[0]);
        const string vtable_ptr = emit_temp_assignment("ptr", string("load ptr ") + call_args[0]);
        if(node.children[0].uses_extended_vtable_layout) {
          string entry_ptr = vtable_ptr;
          const long long slot_offset =
              static_cast<long long>(node.children[0].has_uint_value ? callsem_uint_value(node.children[0]) : 0ULL) * 16LL;
          if(slot_offset != 0) {
            entry_ptr = emit_temp_assignment("ptr",
                                             string("index i8 ") + vtable_ptr + ", " +
                                             to_string(slot_offset));
          }
          const string adjust_ptr =
              emit_temp_assignment("ptr", string("index i8 ") + entry_ptr + ", 8");
          const string this_adjust =
              emit_temp_assignment("i64", string("load i64 ") + adjust_ptr);
          call_args[0] = emit_temp_assignment("ptr",
                                              string("index i8 ") + call_args[0] + ", " +
                                              this_adjust);
          op << emit_temp_assignment("ptr", string("load ptr ") + entry_ptr);
        } else {
          string fn_ptr = vtable_ptr;
          const unsigned long long slot =
              node.children[0].has_uint_value ? callsem_uint_value(node.children[0]) : 0ULL;
          if(slot != 0) {
            fn_ptr = emit_temp_assignment("ptr",
                                          string("index i8 ") + vtable_ptr + ", " +
                                          to_string(static_cast<unsigned long long>(slot * 8ULL)));
          }
          op << emit_temp_assignment("ptr", string("load ptr ") + fn_ptr);
        }
      } else if(node.children[0].kind == CallSemKind::callee) {
        op << callee_symbol;
      } else {
        op << emit_rvalue(node.children[0]);
      }
      op << "(";
      for(size_t i = 0; i < call_args.size(); ++i) {
        if(i != 0) {
          op << ", ";
        }
        op << call_args[i];
      }
      op << ")";
      if(explicit_indirect_call_signature) {
        op << lowir_call_signature_suffix_for_call(node, function_type);
      }
      const string result = emit_temp_assignment(direct_object_result_type, op.str());
      emit_line("copyobj " + storage_span_text(object_type) + " " + result + ", " + target_ptr);
      return;
    }
    vector<string> args;
    args.push_back(target_ptr);
    for(size_t i = 1; i < node.children.size(); ++i) {
      const size_t param_index = constructor_call ? i : (i - 1);
      if(param_index < function_type->params.size()) {
        append_call_argument_values(args, function_type->params[param_index], node.children[i]);
      } else {
        append_variadic_call_argument_value(args, node.children[i]);
      }
    }
    append_vtt_argument(node, 1, args);
    if(!constructor_call) {
      // `args[0]` is the indirect result slot in this path, so the receiver
      // object for any hidden virtual-base helpers begins at `args[1]`.
      append_hidden_virtual_base_arguments(node, args, 1);
    }
    append_parameter_virtual_base_arguments(node, constructor_call, args);

    if(constructor_call &&
       node.value_initializes_result &&
       !is_empty_class_storage_type(object_type)) {
      emit_zero_storage_bytes(target_ptr, backend_storage_size(object_type));
    }

    ostringstream op;
    op << "call void ";
    if(!host_num_put_bridge.empty() &&
       host_num_put_bridge != current_runtime_bridge) {
      note_runtime_bridge_support_symbol(host_num_put_bridge);
      vector<string> bridge_args = args;
      if(bridge_args.size() >= 3) {
        bridge_args[2] = host_num_put_bridge_iterator_bits_ptr(bridge_args[2]);
      }
      op << "@" << host_num_put_bridge << "(";
      for(size_t i = 0; i < bridge_args.size(); ++i) {
        if(i != 0) {
          op << ", ";
        }
        op << bridge_args[i];
      }
      op << ")";
      emit_line(op.str());
      return;
    }
    if(virtual_dispatch) {
      if(args.size() < 2) {
        throw logic_error("virtual indirect call missing object argument");
      }
      apply_virtual_dispatch_view_offset(node.children[0], args[1]);
      const string vtable_ptr = emit_temp_assignment("ptr", string("load ptr ") + args[1]);
      if(node.children[0].uses_extended_vtable_layout) {
        string entry_ptr = vtable_ptr;
        const long long slot_offset =
            static_cast<long long>(node.children[0].has_uint_value ? callsem_uint_value(node.children[0]) : 0ULL) * 16LL;
        if(slot_offset != 0) {
          entry_ptr = emit_temp_assignment("ptr",
                                           string("index i8 ") + vtable_ptr + ", " +
                                           to_string(slot_offset));
        }
        const string adjust_ptr =
            emit_temp_assignment("ptr", string("index i8 ") + entry_ptr + ", 8");
        const string this_adjust =
            emit_temp_assignment("i64", string("load i64 ") + adjust_ptr);
        args[1] = emit_temp_assignment("ptr",
                                       string("index i8 ") + args[1] + ", " + this_adjust);
        op << emit_temp_assignment("ptr", string("load ptr ") + entry_ptr);
      } else {
        string fn_ptr = vtable_ptr;
        const unsigned long long slot =
            node.children[0].has_uint_value ? callsem_uint_value(node.children[0]) : 0ULL;
        if(slot != 0) {
          fn_ptr = emit_temp_assignment("ptr",
                                        string("index i8 ") + vtable_ptr + ", " +
                                        to_string(static_cast<unsigned long long>(slot * 8ULL)));
        }
        op << emit_temp_assignment("ptr", string("load ptr ") + fn_ptr);
      }
    } else if(node.children[0].kind == CallSemKind::callee) {
      op << callee_symbol;
    } else {
      op << emit_rvalue(node.children[0]);
    }
    op << "(";
    for(size_t i = 0; i < args.size(); ++i) {
      if(i != 0) {
        op << ", ";
      }
      op << args[i];
    }
    op << ")";
    if(explicit_indirect_call_signature) {
      op << lowir_call_signature_suffix_for_call(node, function_type);
    }
    emit_line(op.str());
  }

  void emit_constructor_call_expression_to_target(const CallSemNode & node,
                                                  const string & target_ptr)
  {
    if(node.kind != CallSemKind::call_expression || node.children.size() < 2) {
      throw logic_error("expected constructor call-expression with explicit target");
    }

    TypePtr function_type;
    if(!resolve_callable_function_type(node.children[0].semantic_type, function_type) ||
       !function_type ||
       !is_void_type(function_type->inner)) {
      throw logic_error("constructor call-expression missing void function type");
    }
    const size_t semantic_arg_count = node.children.size() - 1;
    if(semantic_arg_count != function_type->params.size()) {
      throw logic_error("constructor call-expression arity mismatch");
    }

    const string host_num_put_bridge =
        bridge_symbol_for_callee(node.children[0], function_type);
    const string current_runtime_bridge =
        g_lowir_current_function_node ?
            callsem_runtime_bridge_symbol(*g_lowir_current_function_node) :
            "";
    const string callee_symbol =
        node.children[0].kind == CallSemKind::callee ?
            lookup_function_symbol(node.children[0]) :
            "";
    const bool callee_is_direct_symbol =
        !callee_symbol.empty() && callee_symbol[0] == '@';
    const bool explicit_indirect_call_signature =
        node.children[0].kind != CallSemKind::callee || !callee_is_direct_symbol;

    vector<string> args;
    args.push_back(target_ptr);
    for(size_t i = 2; i < node.children.size(); ++i) {
      const size_t param_index = i - 1;
      if(param_index < function_type->params.size()) {
        append_call_argument_values(args, function_type->params[param_index], node.children[i]);
      } else {
        append_variadic_call_argument_value(args, node.children[i]);
      }
    }
    append_vtt_argument(node, 1, args);
    append_parameter_virtual_base_arguments(node, false, args);

    ostringstream op;
    op << "call void ";
    if(!host_num_put_bridge.empty() &&
       host_num_put_bridge != current_runtime_bridge) {
      note_runtime_bridge_support_symbol(host_num_put_bridge);
      vector<string> bridge_args = args;
      if(bridge_args.size() >= 3) {
        bridge_args[2] = host_num_put_bridge_iterator_bits_ptr(bridge_args[2]);
      }
      op << "@" << host_num_put_bridge << "(";
      for(size_t i = 0; i < bridge_args.size(); ++i) {
        if(i != 0) {
          op << ", ";
        }
        op << bridge_args[i];
      }
      op << ")";
      emit_line(op.str());
      return;
    }

    if(node.children[0].kind == CallSemKind::callee) {
      op << callee_symbol;
    } else {
      op << emit_rvalue(node.children[0]);
    }
    op << "(";
    for(size_t i = 0; i < args.size(); ++i) {
      if(i != 0) {
        op << ", ";
      }
      op << args[i];
    }
    op << ")";
    if(explicit_indirect_call_signature) {
      op << lowir_call_signature_suffix_for_call(node, function_type);
    }
    emit_line(op.str());
  }

  void emit_call_expression_to_target(const CallSemNode & node, const string & target_ptr)
  {
    const bool needs_constructor_wrapper =
        call_expression_needs_constructor_unwind_wrapper(node);
    const bool needs_host_wrapper =
        call_expression_needs_host_unwind_wrapper(node);
    if(shared_host_call_unwind_region_open_ && needs_constructor_wrapper) {
      close_shared_host_call_unwind_region();
    }
    if(!needs_constructor_wrapper && !needs_host_wrapper) {
      emit_call_expression_to_target_impl(node, target_ptr);
      return;
    }
    if(use_host_eh_runtime() && needs_host_wrapper && !needs_constructor_wrapper) {
      open_shared_host_call_unwind_region();
      emit_call_expression_to_target_impl(node, target_ptr);
      return;
    }

    const size_t host_dispatch_depth =
        use_host_eh_runtime() ? host_eh_region_depth_ + 1 : 0;
    bool created_dispatch = false;
    const string dispatch_label =
        shared_call_unwind_dispatch_label(needs_constructor_wrapper,
                                          host_dispatch_depth,
                                          created_dispatch);
    const string end_label =
        created_dispatch ? new_block("call_unwind_end") : string();
    emit_line("eh_try " + lowir_block_name(dispatch_label));
    if(use_host_eh_runtime()) {
      ++host_eh_region_depth_;
    }
    emit_call_expression_to_target_impl(node, target_ptr);
    emit_line("eh_end");
    if(use_host_eh_runtime()) {
      if(host_eh_region_depth_ == 0) {
        throw logic_error("host EH region depth underflow");
      }
      --host_eh_region_depth_;
    }
    if(created_dispatch) {
      terminate("jump " + lowir_block_name(end_label));
      emit_shared_call_unwind_dispatch_block(dispatch_label,
                                             needs_constructor_wrapper,
                                             host_dispatch_depth,
                                             target_ptr);
      start_block(end_label);
    }
  }

  bool is_constructor_materialization_call(const CallSemNode & node) const
  {
    if(node.kind != CallSemKind::call_expression || node.children.empty()) {
      return false;
    }
    const TypePtr object_type = remove_reference_type(node.semantic_type);
    if(!is_complete_class_value_type(object_type)) {
      return false;
    }
    TypePtr function_type;
    return resolve_callable_function_type(node.children[0].semantic_type, function_type) &&
           function_type && is_void_type(function_type->inner);
  }

  TypePtr indirect_call_result_object_type(const CallSemNode & node) const
  {
    if(node.kind != CallSemKind::call_expression || node.children.empty()) {
      return TypePtr();
    }
    TypePtr function_type;
    if(!resolve_callable_function_type(node.children[0].semantic_type, function_type) ||
       !function_type ||
       !function_type->inner ||
       is_reference_type(function_type->inner) ||
       !lowir_uses_indirect_result_boundary(function_type->inner)) {
      return TypePtr();
    }
    return strip_top_level_cv(function_type->inner);
  }

  bool is_lowir_integral_scalar_type(const TypePtr & type) const
  {
    return is_integral_type(type) || is_named_enum_scalar_type(type);
  }

  bool is_lowir_unsigned_integral_scalar_type(const TypePtr & type) const
  {
    return type && !is_named_enum_scalar_type(type) && is_unsigned_integral_type(type);
  }

  static bool is_explicit_lowir_integer_type_text(const string & type)
  {
    return type == "i8" || type == "u8" ||
           type == "i16" || type == "u16" ||
           type == "i32" || type == "u32" ||
           type == "i64" || type == "i128" ||
           type == "u128";
  }

  string emit_lowir_convert(const string & op,
                            const string & target_lowir,
                            const string & source_lowir,
                            const string & value)
  {
    return emit_temp_assignment(target_lowir,
                                string("convert ") + op + " " +
                                target_lowir + " " + source_lowir + " " + value);
  }

  string integral_to_floating_runtime_helper(const TypePtr & source_type,
                                             const TypePtr & target_type) const
  {
    const string source_lowir = lowir_type_for(source_type);
    const string target_lowir = lowir_type_for(target_type);
    const bool source_unsigned =
        !is_named_enum_scalar_type(source_type) && is_unsigned_integral_type(source_type);

    if(source_lowir == "i32" && target_lowir == "f32") {
      return source_unsigned ? "__floatunsisf" : "__floatsisf";
    }
    if(source_lowir == "i32" && target_lowir == "f64") {
      return source_unsigned ? "__floatunsidf" : "__floatsidf";
    }
    if(source_lowir == "i64" && target_lowir == "f32") {
      return source_unsigned ? "__floatundisf" : "__floatdisf";
    }
    if(source_lowir == "i64" && target_lowir == "f64") {
      return source_unsigned ? "__floatundidf" : "__floatdidf";
    }
    return string();
  }

  string floating_to_integral_runtime_helper(const TypePtr & source_type,
                                             const TypePtr & target_type) const
  {
    const string source_lowir = lowir_type_for(source_type);
    const string target_lowir = lowir_type_for(target_type);
    const bool target_unsigned =
        !is_named_enum_scalar_type(target_type) && is_unsigned_integral_type(target_type);

    if(source_lowir == "f32" && target_lowir == "i32") {
      return target_unsigned ? "__fixunssfsi" : "__fixsfsi";
    }
    if(source_lowir == "f32" && target_lowir == "i64") {
      return target_unsigned ? "__fixunssfdi" : "__fixsfdi";
    }
    if(source_lowir == "f64" && target_lowir == "i32") {
      return target_unsigned ? "__fixunsdfsi" : "__fixdfsi";
    }
    if(source_lowir == "f64" && target_lowir == "i64") {
      return target_unsigned ? "__fixunsdfdi" : "__fixdfdi";
    }
    return string();
  }

  string floating_to_floating_runtime_helper(const TypePtr & source_type,
                                             const TypePtr & target_type) const
  {
    const string source_lowir = lowir_type_for(source_type);
    const string target_lowir = lowir_type_for(target_type);
    if(source_lowir == "f32" && target_lowir == "f64") {
      return "__extendsfdf2";
    }
    if(source_lowir == "f64" && target_lowir == "f32") {
      return "__truncdfsf2";
    }
    return string();
  }

  string emit_runtime_scalar_conversion(const string & helper_symbol,
                                        const string & target_lowir_type,
                                        const string & value)
  {
    const string internal_symbol = external_runtime_symbol(helper_symbol);
    return emit_temp_assignment(target_lowir_type,
                                string("call ") + target_lowir_type + " " +
                                internal_symbol + "(" + value + ")");
  }

  bool direct_runtime_function_symbol_available(const string & symbol) const
  {
    if(c_linkage_function_symbols_.count(symbol) == 0) {
      return false;
    }
    if(function_symbol_nodes_.count(symbol) != 0 ||
       function_symbol_lookup_index_.mapped_symbols.count(symbol) != 0) {
      return true;
    }
    for(size_t i = 0; i < function_symbol_entries_.size(); ++i) {
      if(function_symbol_entries_[i].symbol == symbol) {
        return true;
      }
    }
    return false;
  }

  string external_runtime_symbol(const string & helper_symbol)
  {
    const string direct_symbol = "@" + helper_symbol;
    if(direct_runtime_function_symbol_available(direct_symbol)) {
      return direct_symbol;
    }

    const string internal_symbol =
        symbol_linkage::internal_symbol_from_name("__external_runtime::" + helper_symbol);
    external_function_symbols_[internal_symbol] = helper_symbol;
    return internal_symbol;
  }

  string external_runtime_object_symbol(const string & helper_symbol)
  {
    const string internal_symbol =
        symbol_linkage::internal_symbol_from_name("__external_runtime::" + helper_symbol);
    external_object_symbols_[internal_symbol] = helper_symbol;
    return internal_symbol;
  }

  void note_external_runtime_function(const string & helper_symbol)
  {
    external_function_symbols_[
        symbol_linkage::internal_symbol_from_name("__external_runtime::" + helper_symbol)] =
        helper_symbol;
  }

  string external_object_symbol(const string & category,
                                const string & key,
                                const string & object_symbol)
  {
    const string internal_symbol =
        symbol_linkage::internal_symbol_from_name("__external_" + category + "::" + key);
    external_object_symbols_[internal_symbol] = object_symbol;
    return internal_symbol;
  }

  static bool try_parse_integral_immediate_text(const string & text,
                                                long long & out)
  {
    if(text.empty()) {
      return false;
    }
    char * end = nullptr;
    errno = 0;
    const long long parsed = strtoll(text.c_str(), &end, 10);
    if(errno != 0 || end == text.c_str() || *end != '\0') {
      return false;
    }
    out = parsed;
    return true;
  }

  string normalize_integral_immediate_bits(long long value_bits,
                                           const TypePtr & type) const
  {
    const size_t source_bits = type_size(type) * 8;
    if(source_bits == 0 || source_bits >= 64) {
      return to_string(value_bits);
    }

    const unsigned long long mask = lowir_bit_field_mask(source_bits);
    unsigned long long normalized =
        static_cast<unsigned long long>(value_bits) & mask;
    if(!is_lowir_unsigned_integral_scalar_type(type)) {
      const unsigned long long sign_bit = 1ULL << (source_bits - 1);
      if((normalized & sign_bit) != 0) {
        normalized |= ~mask;
      }
      return to_string(static_cast<long long>(normalized));
    }
    return to_string(normalized);
  }

  string integral_immediate_for_target(long long value_bits,
                                       const TypePtr & source_type,
                                       const TypePtr & target_type) const
  {
    const size_t source_bits = type_size(source_type) * 8;
    const size_t target_bits = type_size(target_type) * 8;
    if(source_bits == 0 || target_bits == 0 ||
       source_bits > 64 || target_bits > 64) {
      return string();
    }

    const unsigned long long source_mask =
        source_bits >= 64 ? ~0ULL : lowir_bit_field_mask(source_bits);
    unsigned long long bits =
        static_cast<unsigned long long>(value_bits) & source_mask;
    if(!is_lowir_unsigned_integral_scalar_type(source_type) && source_bits < 64) {
      const unsigned long long sign_bit = 1ULL << (source_bits - 1);
      if((bits & sign_bit) != 0) {
        bits |= ~source_mask;
      }
    }

    const unsigned long long target_mask =
        target_bits >= 64 ? ~0ULL : lowir_bit_field_mask(target_bits);
    bits &= target_mask;
    if(!is_lowir_unsigned_integral_scalar_type(target_type) && target_bits < 64) {
      const unsigned long long sign_bit = 1ULL << (target_bits - 1);
      if((bits & sign_bit) != 0) {
        bits |= ~target_mask;
      }
      return to_string(static_cast<long long>(bits));
    }
    if(target_bits >= 64 && is_lowir_unsigned_integral_scalar_type(target_type)) {
      return string();
    }
    return to_string(static_cast<long long>(bits));
  }

  string normalize_integral_scalar_value(const string & value,
                                         const TypePtr & type)
  {
    long long immediate_value = 0;
    if(try_parse_integral_immediate_text(value, immediate_value)) {
      return normalize_integral_immediate_bits(immediate_value, type);
    }

    string result = value;
    const size_t source_bits = type_size(type) * 8;
    if(source_bits != 0 && source_bits < 64) {
      if(is_lowir_unsigned_integral_scalar_type(type)) {
        result = emit_temp_assignment("i64",
                                      string("binary and i64 ") + result + ", " +
                                      to_string(lowir_bit_field_mask(source_bits)));
      } else {
        const size_t shift = 64 - source_bits;
        const string shifted =
            emit_temp_assignment("i64",
                                 string("binary shl i64 ") + result + ", " +
                                 to_string(shift));
        result = emit_temp_assignment("i64",
                                      string("binary shr i64 ") + shifted + ", " +
                                      to_string(shift));
      }
    }
    return result;
  }

  string emit_scalar_value_conversion(const string & value,
                                      const TypePtr & source_type,
                                      const TypePtr & target_type,
                                      bool allow_integral_scalar_copy = false,
                                      const string & source_lowir_override = string())
  {
    TypePtr source_value_type = lowir_value_conversion_type(source_type);
    TypePtr target_value_type = lowir_value_conversion_type(target_type);
    if(is_void_type(source_value_type) || is_void_type(target_value_type)) {
      return value;
    }
    if(!source_value_type || !target_value_type ||
       type_equals(source_value_type, target_value_type)) {
      return value;
    }

    const string semantic_source_lowir = lowir_type_for(source_value_type);
    string result = value;
    if(!source_lowir_override.empty() &&
       source_lowir_override != semantic_source_lowir) {
      if(is_lowir_integral_scalar_type(source_value_type) &&
         is_explicit_lowir_integer_type_text(source_lowir_override) &&
         semantic_source_lowir == "i64") {
        result = emit_lowir_convert(
            is_lowir_unsigned_integral_scalar_type(source_value_type) ? "zext" : "sext",
            semantic_source_lowir,
            source_lowir_override,
            result);
      } else {
        ostringstream out;
        out << "unsupported storage-to-value materialization in LowIR";
        out << " [source-lowir " << source_lowir_override << "]";
        out << " [semantic-source " << describe_type(source_type) << "]";
        throw logic_error(out.str());
      }
    }
    const string source_lowir = semantic_source_lowir;
    const string target_lowir = lowir_type_for(target_value_type);
    if(is_lowir_integral_scalar_type(source_value_type) &&
       target_value_type->kind == Type::TK_FUNDAMENTAL &&
       target_value_type->fundamental == FT_BOOL) {
      const string normalized = normalize_integral_scalar_value(result, source_value_type);
      const string truthy =
          emit_temp_assignment("i64",
                               string("cmp ne ") + source_lowir + " " + normalized + ", " +
                               zero_literal_for_lowir_type(source_lowir));
      return target_lowir == "i64" ?
          truthy :
          emit_temp_assignment(target_lowir,
                               string("copy ") + target_lowir + " " + truthy);
    }

    if(is_pointer_type(source_value_type) &&
       target_value_type->kind == Type::TK_FUNDAMENTAL &&
       target_value_type->fundamental == FT_BOOL) {
      const string truthy =
          emit_temp_assignment("i64", string("cmp ne ptr ") + result + ", 0");
      return target_lowir == "i64" ?
          truthy :
          emit_temp_assignment(target_lowir,
                               string("copy ") + target_lowir + " " + truthy);
    }

    if(source_value_type->kind == Type::TK_MEMBER_POINTER &&
       target_value_type->kind == Type::TK_FUNDAMENTAL &&
       target_value_type->fundamental == FT_BOOL) {
      const string truthy =
          emit_temp_assignment("i64",
                               string("cmp ne ") + source_lowir + " " + result + ", " +
                               zero_literal_for_lowir_type(source_lowir));
      return target_lowir == "i64" ?
          truthy :
          emit_temp_assignment(target_lowir,
                               string("copy ") + target_lowir + " " + truthy);
    }

    if(is_floating_type(source_value_type) &&
       target_value_type->kind == Type::TK_FUNDAMENTAL &&
       target_value_type->fundamental == FT_BOOL) {
      const string truthy =
          emit_temp_assignment("i64",
                               string("cmp ne ") + source_lowir + " " + result + ", " +
                               zero_literal_for_lowir_type(source_lowir));
      return target_lowir == "i64" ?
          truthy :
          emit_temp_assignment(target_lowir,
                               string("copy ") + target_lowir + " " + truthy);
    }

    if(is_nullptr_scalar_type(source_value_type) &&
       target_value_type->kind == Type::TK_FUNDAMENTAL &&
       target_value_type->fundamental == FT_BOOL) {
      return target_lowir == "i64" ? string("0") : emit_temp_assignment(target_lowir, "copy " + target_lowir + " 0");
    }

    if(is_lowir_integral_scalar_type(source_value_type) &&
       is_floating_type(target_value_type)) {
      const string normalized = normalize_integral_scalar_value(result, source_value_type);
      return emit_lowir_convert(
          is_lowir_unsigned_integral_scalar_type(source_value_type) ? "uitofp" : "sitofp",
          target_lowir,
          source_lowir,
          normalized);
    }

    if(is_lowir_integral_scalar_type(source_value_type) &&
       is_lowir_integral_scalar_type(target_value_type)) {
      const size_t source_bits = type_size(source_value_type) * 8;
      const size_t target_bits = type_size(target_value_type) * 8;
      const bool same_width = source_bits == target_bits;
      const bool same_signedness =
          is_lowir_unsigned_integral_scalar_type(source_value_type) ==
          is_lowir_unsigned_integral_scalar_type(target_value_type);
      if(same_width && same_signedness) {
        return source_lowir == target_lowir ?
            result :
            emit_temp_assignment(target_lowir,
                                string("copy ") + target_lowir + " " + result);
      }

      long long immediate_value = 0;
      if(try_parse_integral_immediate_text(result, immediate_value)) {
        const string immediate =
            integral_immediate_for_target(immediate_value,
                                          source_value_type,
                                          target_value_type);
        if(!immediate.empty()) {
          return immediate;
        }
      }

      if(same_width) {
        return emit_temp_assignment(target_lowir,
                                    string("copy ") + target_lowir + " " + result);
      }
      if(source_bits < target_bits) {
        return emit_lowir_convert(
            is_lowir_unsigned_integral_scalar_type(source_value_type) ? "zext" : "sext",
            target_lowir,
            source_lowir,
            result);
      }
      return emit_lowir_convert("trunc", target_lowir, source_lowir, result);
    }

    if(is_pointer_type(source_value_type) &&
       is_lowir_integral_scalar_type(target_value_type)) {
      result = emit_temp_assignment("i64", string("copy i64 ") + result);
      const size_t target_bits = type_size(target_value_type) * 8;
      if(target_bits > 64) {
        return emit_lowir_convert("zext", target_lowir, "i64", result);
      }
      if(target_bits != 0 && target_bits < 64) {
        result = emit_temp_assignment("i64",
                                      string("binary and i64 ") + result + ", " +
                                      to_string(lowir_bit_field_mask(target_bits)));
        if(!is_lowir_unsigned_integral_scalar_type(target_value_type)) {
          const size_t shift = 64 - target_bits;
          const string shifted =
              emit_temp_assignment("i64",
                                   string("binary shl i64 ") + result + ", " +
                                   to_string(shift));
          result = emit_temp_assignment("i64",
                                        string("binary shr i64 ") + shifted + ", " +
                                        to_string(shift));
        }
      }
      if(target_lowir == "i64") {
        return result;
      }
      return emit_temp_assignment(target_lowir,
                                  string("copy ") + target_lowir + " " + result);
    }

    if(is_member_function_pointer_type(source_value_type) &&
       is_pointer_type(target_value_type)) {
      const string function_bits = emit_lowir_convert("trunc", "i64", source_lowir, result);
      return emit_temp_assignment("ptr", string("copy ptr ") + function_bits);
    }

    if(is_lowir_integral_scalar_type(source_value_type) &&
       is_pointer_type(target_value_type)) {
      const string normalized = normalize_integral_scalar_value(result, source_value_type);
      if(type_size(source_value_type) > 8) {
        const string truncated = emit_lowir_convert("trunc", "i64", source_lowir, normalized);
        return emit_temp_assignment("ptr", string("copy ptr ") + truncated);
      }
      return emit_temp_assignment("ptr", string("copy ptr ") + normalized);
    }

    if(is_lowir_integral_scalar_type(source_value_type) &&
       target_value_type->kind == Type::TK_MEMBER_POINTER) {
      long long immediate_value = 0;
      if(try_parse_integral_immediate_text(result, immediate_value) &&
         immediate_value == 0) {
        return zero_literal_for_lowir_type(target_lowir);
      }
    }

    if(source_lowir == target_lowir &&
       !(is_floating_type(source_value_type) && is_floating_type(target_value_type))) {
      return result;
    }

    if(is_nullptr_scalar_type(source_value_type)) {
      if(target_lowir == "ptr") {
        return emit_temp_assignment("ptr", string("copy ptr ") + result);
      }
      if(target_value_type && target_value_type->kind == Type::TK_MEMBER_POINTER) {
        return result;
      }
    }

    if(is_floating_type(source_value_type) &&
       is_lowir_integral_scalar_type(target_value_type)) {
      const string raw_target_lowir = lowir_memory_type_for(target_value_type);
      string converted = emit_lowir_convert(
          is_lowir_unsigned_integral_scalar_type(target_value_type) ? "fptoui" : "fptosi",
          raw_target_lowir,
          source_lowir,
          result);
      if(raw_target_lowir != target_lowir) {
        converted = emit_temp_assignment(target_lowir,
                                         string("copy ") + target_lowir + " " + converted);
        const size_t target_bits = type_size(target_value_type) * 8;
        if(target_bits != 0 && target_bits < 64) {
          if(is_lowir_unsigned_integral_scalar_type(target_value_type)) {
            converted = emit_temp_assignment("i64",
                                             string("binary and i64 ") + converted + ", " +
                                          to_string(lowir_bit_field_mask(target_bits)));
          } else {
            const size_t shift = 64 - target_bits;
            const string shifted =
                emit_temp_assignment("i64",
                                     string("binary shl i64 ") + converted + ", " +
                                     to_string(shift));
            converted = emit_temp_assignment("i64",
                                             string("binary shr i64 ") + shifted + ", " +
                                             to_string(shift));
          }
        }
      }
      return converted;
    }

    if(is_floating_type(source_value_type) &&
       is_floating_type(target_value_type)) {
      const size_t source_size = type_size(source_value_type);
      const size_t target_size = type_size(target_value_type);
      if(source_size < target_size) {
        return emit_lowir_convert("fpext", target_lowir, source_lowir, result);
      }
      if(source_size > target_size) {
        return emit_lowir_convert("fptrunc", target_lowir, source_lowir, result);
      }
      return result;
    }

    ostringstream out;
    out << "unsupported scalar conversion in PA16 LowIR";
    out << " [source " << describe_type(source_type) << "]";
    out << " [target " << describe_type(target_type) << "]";
    throw logic_error(out.str());
  }

  string emit_scalar_storage_value(const TypePtr & target_type,
                                   const CallSemNode & node)
  {
    if(is_reference_type(node.semantic_type) &&
       !is_reference_type(target_type)) {
      TypePtr referent_type = remove_reference_type(node.semantic_type);
      TypePtr referent_base = strip_top_level_cv(referent_type);
      TypePtr target_base = strip_top_level_cv(remove_reference_type(target_type));
      if(referent_base &&
         target_base &&
         !is_indirect_value_type(referent_type) &&
         !is_function_type(referent_base) &&
         referent_base->kind != Type::TK_ARRAY &&
         !is_indirect_value_type(target_type) &&
         !is_function_type(target_base) &&
         target_base->kind != Type::TK_ARRAY) {
        const TypePtr loaded_type =
            materialization_source_type_for(node, referent_type);
        const string memory_type = lowir_memory_type_for(loaded_type);
        const string loaded_value =
            emit_temp_assignment(memory_type,
                                 string("load ") + memory_type + " " +
                                 emit_lvalue_address(node));
        return emit_scalar_value_conversion(loaded_value,
                                            loaded_type,
                                            target_type,
                                            true,
                                            memory_type);
      }
    }
    return emit_scalar_value_conversion(emit_rvalue(node),
                                        node.semantic_type,
                                        target_type,
                                        true);
  }

  TypePtr materialization_source_type_for(const CallSemNode & node,
                                          const TypePtr & fallback) const
  {
    return callsem_materialization_source_type(node) ?
        callsem_materialization_source_type(node) :
        fallback;
  }

  string emit_loaded_scalar_value(const string & loaded_value,
                                  const TypePtr & loaded_type,
                                  const CallSemNode & node)
  {
    string result = loaded_value;
    TypePtr current_type = loaded_type;
    string current_lowir = lowir_memory_type_for(loaded_type);
    if(callsem_conversion_source_type(node) &&
       (!current_type || !type_equals(current_type, callsem_conversion_source_type(node)))) {
      result =
          emit_scalar_value_conversion(result,
                                       current_type,
                                       callsem_conversion_source_type(node),
                                       false,
                                       current_lowir);
      current_type = callsem_conversion_source_type(node);
      current_lowir = lowir_type_for(lowir_value_conversion_type(current_type));
    } else if(callsem_conversion_source_type(node)) {
      current_type = callsem_conversion_source_type(node);
      current_lowir = lowir_type_for(lowir_value_conversion_type(current_type));
    }
    return emit_scalar_value_conversion(result,
                                        current_type,
                                        node.semantic_type,
                                        false,
                                        current_lowir);
  }

  string emit_call_expression_value(const CallSemNode & node,
                                    const TypePtr & function_result_type,
                                    const string & raw_call_result)
  {
    if(!lowir_direct_object_type(function_result_type).empty() &&
       !is_reference_type(function_result_type)) {
      return raw_call_result;
    }
    if(is_reference_type(function_result_type)) {
      TypePtr referent_type = remove_reference_type(function_result_type);
      TypePtr referent_base = strip_top_level_cv(referent_type);
      if(!referent_base || is_indirect_value_type(referent_type) ||
         is_function_type(referent_base) || referent_base->kind == Type::TK_ARRAY) {
        return raw_call_result;
      }

      const string memory_type = lowir_memory_type_for(referent_type);
      const string loaded_value =
          emit_temp_assignment(memory_type, string("load ") + memory_type + " " + raw_call_result);
      return emit_scalar_value_conversion(loaded_value,
                                          referent_type,
                                          node.semantic_type,
                                          true,
                                          memory_type);
    }

    return emit_scalar_value_conversion(raw_call_result,
                                        function_result_type,
                                        node.semantic_type,
                                        true);
  }

  struct CallArgumentCapture
  {
    size_t index = 0;
    string lowir_type;
    string slot;
  };

  struct CallArgumentOverride
  {
    size_t index = 0;
    string value;
  };

  const CallSemNode * left_nested_reference_call_chain_next(
      const CallSemNode & node) const
  {
    if(node.kind != CallSemKind::call_expression ||
       node.children.size() < 2) {
      return nullptr;
    }

    TypePtr function_type;
    if(!resolve_callable_function_type(node.children[0].semantic_type, function_type) ||
       !function_type ||
       !function_type->inner ||
       !is_reference_type(function_type->inner) ||
       function_type->params.empty() ||
       !function_type->params[0]) {
      return nullptr;
    }

    TypePtr param_base = strip_top_level_cv(function_type->params[0]);
    if(param_base &&
       (param_base->kind == Type::TK_LVALUE_REFERENCE ||
        param_base->kind == Type::TK_RVALUE_REFERENCE)) {
      const CallSemNode & first_arg = node.children[1];
      if(first_arg.kind != CallSemKind::call_expression ||
         !is_reference_type(first_arg.semantic_type)) {
        return nullptr;
      }
      TypePtr arg_referent =
          strip_top_level_cv(remove_reference_type(first_arg.semantic_type));
      TypePtr param_referent =
          strip_top_level_cv(remove_reference_type(function_type->params[0]));
      if(param_referent && arg_referent && type_equals(param_referent, arg_referent)) {
        return &first_arg;
      }
      return nullptr;
    }

    if(param_base &&
       param_base->kind == Type::TK_POINTER &&
       param_base->inner) {
      const CallSemNode & first_arg = node.children[1];
      if(first_arg.kind != CallSemKind::unary_expression ||
         !callsem_has_token(first_arg, OP_AMP) ||
         first_arg.children.size() != 1 ||
         first_arg.children[0].kind != CallSemKind::call_expression ||
         !is_reference_type(first_arg.children[0].semantic_type)) {
        return nullptr;
      }
      TypePtr arg_referent =
          strip_top_level_cv(remove_reference_type(first_arg.children[0].semantic_type));
      TypePtr pointed = strip_top_level_cv(param_base->inner);
      if(pointed && arg_referent && type_equals(pointed, arg_referent)) {
        return &first_arg.children[0];
      }
      return nullptr;
    }

    return nullptr;
  }

  bool try_emit_left_nested_reference_call_chain(const CallSemNode & node,
                                                string & result)
  {
    vector<const CallSemNode *> chain;
    const CallSemNode * current = &node;
    while(const CallSemNode * next =
              left_nested_reference_call_chain_next(*current)) {
      chain.push_back(current);
      current = next;
    }
    if(chain.empty()) {
      return false;
    }

    string previous = emit_call_expression_raw(*current);
    for(size_t i = chain.size(); i-- > 0;) {
      CallArgumentOverride first_arg;
      first_arg.index = 0;
      first_arg.value = previous;
      previous = emit_call_expression_raw(*chain[i], nullptr, &first_arg);
    }
    result = previous;
    return true;
  }

  string emit_call_expression_raw_impl(const CallSemNode & node,
                                       const CallArgumentCapture * capture = nullptr,
                                       const CallArgumentOverride * override_arg = nullptr)
  {
    if(node.kind != CallSemKind::call_expression || node.children.empty()) {
      throw logic_error("expected call-expression");
    }

    TypePtr function_type;
    if(!resolve_callable_function_type(node.children[0].semantic_type, function_type)) {
      throw logic_error("call-expression missing function type");
    }
    TypePtr function_result_type = function_type->inner;
    if(!function_result_type) {
      throw logic_error("call-expression missing function result type");
    }
    const bool constructor_call =
        node.children[0].kind == CallSemKind::callee &&
        is_constructor_function_name(node.children[0].text);
    const string constructor_callee_symbol =
        constructor_call ? lookup_function_symbol(node.children[0]) : string();
    const bool constructor_base_entry_call =
        constructor_callee_symbol.find("__base_entry") != string::npos;
    const string direct_object_result_type = lowir_direct_object_type(function_result_type);
    if(is_indirect_value_type(node.semantic_type) &&
       direct_object_result_type.empty()) {
      ostringstream out;
      out << "indirect return value requires materialization context in PA16 LowIR";
      out << " [function " << function_.name << "]";
      out << " [call-type " << describe_type(node.semantic_type) << "]";
      out << " [callee-kind " << callsem_kind_text(node.children[0].kind) << "]";
      if(!node.children[0].text.empty()) {
        out << " [callee-text " << node.children[0].text << "]";
      }
      if(node.children[0].semantic_type) {
        out << " [callee-type " << describe_type(node.children[0].semantic_type) << "]";
      }
      throw logic_error(out.str());
    }
    vector<string> args;
    for(size_t i = 1; i < node.children.size(); ++i) {
      const size_t arg_index = i - 1;
      if(override_arg && override_arg->index == arg_index) {
        args.push_back(override_arg->value);
      } else if(arg_index < function_type->params.size()) {
        append_call_argument_values(args, function_type->params[arg_index], node.children[i]);
      } else {
        append_variadic_call_argument_value(args, node.children[i]);
      }
    }
    if(function_type->params.size() >= 2 &&
       args.size() >= 2 &&
       describe_type(function_type->params[1]) ==
           "pointer to const class std::__1::ostreambuf_iterator<char, std::__1::char_traits<char>>" &&
       node.children.size() >= 3 &&
       callsem_has_token(node.children[2], OP_AMP) &&
       node.children[2].children.size() == 1 &&
       node.children[2].children[0].semantic_type &&
       describe_type(node.children[2].children[0].semantic_type) ==
           "class std::__1::ostreambuf_iterator<char, std::__1::char_traits<char>>") {
      args[1] = emit_lvalue_address(node.children[2].children[0]);
    }
    if(capture) {
      if(capture->index >= args.size()) {
        throw logic_error("call argument capture index out of range");
      }
      emit_line("store " + capture->lowir_type + " " +
                args[capture->index] + ", " + capture->slot);
    }
    if(node.value_initializes_result && is_void_type(function_result_type)) {
      if(node.children.size() < 2 || args.empty()) {
        throw logic_error("value-initialized constructor call missing target");
      }
      TypePtr target_ptr_type = strip_top_level_cv(node.children[1].semantic_type);
      TypePtr target_type =
          target_ptr_type && target_ptr_type->kind == Type::TK_POINTER ?
              strip_top_level_cv(target_ptr_type->inner) :
              TypePtr();
      if(!target_type || !is_complete_class_value_type(target_type)) {
        throw logic_error("value-initialized constructor call target must be class pointer");
      }
      if(!is_empty_class_storage_type(target_type)) {
        emit_zero_storage_bytes(args[0], backend_storage_size(target_type));
      }
    }
    append_vtt_argument(node, 1, args);
    if(!constructor_call || constructor_base_entry_call) {
      append_hidden_virtual_base_arguments(node, args, 0);
    }
    append_parameter_virtual_base_arguments(node, false, args);
    const bool virtual_dispatch =
        node.children[0].kind == CallSemKind::callee &&
        node.children[0].is_virtual_dispatch;
    const string host_num_put_bridge =
        bridge_symbol_for_callee(node.children[0], function_type);
    const string current_runtime_bridge =
        g_lowir_current_function_node ?
            callsem_runtime_bridge_symbol(*g_lowir_current_function_node) :
            "";
    const string callee_symbol =
        (!virtual_dispatch && node.children[0].kind == CallSemKind::callee) ?
            lookup_function_symbol(node.children[0]) :
            "";
    const bool explicit_indirect_call_signature =
        virtual_dispatch ||
        !(node.children[0].kind == CallSemKind::callee &&
          !callee_symbol.empty() &&
          callee_symbol[0] == '@');
    ostringstream op;
    const string raw_result_lowir_type =
        direct_object_result_type.empty() ? lowir_type_for(function_result_type)
                                          : direct_object_result_type;
    if(!host_num_put_bridge.empty() &&
       host_num_put_bridge != current_runtime_bridge) {
      note_runtime_bridge_support_symbol(host_num_put_bridge);
      const string result_slot = new_hidden_slot(raw_result_lowir_type, "numput");
      vector<string> bridge_args = args;
      if(bridge_args.size() >= 2) {
        bridge_args[1] = host_num_put_bridge_iterator_bits_ptr(bridge_args[1]);
      }
      emit_line("call void @" + host_num_put_bridge + "(" +
                emit_storage_address(result_slot) + ", " +
                bridge_args[0] + ", " +
                bridge_args[1] + ", " +
                bridge_args[2] + ", " +
                bridge_args[3] + ", " +
                bridge_args[4] + ")");
      if(lowir_internal::is_object_type(lowir_internal::LowType{raw_result_lowir_type})) {
        return result_slot;
      }
      return emit_temp_assignment(raw_result_lowir_type,
                                  string("load ") + raw_result_lowir_type + " " + result_slot);
    }
    op << "call " << raw_result_lowir_type << " ";
    if(virtual_dispatch) {
      if(args.empty()) {
        throw logic_error("virtual call missing object argument");
      }
      apply_virtual_dispatch_view_offset(node.children[0], args[0]);
      const string vtable_ptr = emit_temp_assignment("ptr", string("load ptr ") + args[0]);
      if(node.children[0].uses_extended_vtable_layout) {
        string entry_ptr = vtable_ptr;
        const long long slot_offset =
            static_cast<long long>(node.children[0].has_uint_value ? callsem_uint_value(node.children[0]) : 0ULL) * 16LL;
        if(slot_offset != 0) {
          entry_ptr = emit_temp_assignment("ptr",
                                           string("index i8 ") + vtable_ptr + ", " +
                                           to_string(slot_offset));
        }
        const string adjust_ptr =
            emit_temp_assignment("ptr", string("index i8 ") + entry_ptr + ", 8");
        const string this_adjust =
            emit_temp_assignment("i64", string("load i64 ") + adjust_ptr);
        args[0] = emit_temp_assignment("ptr",
                                       string("index i8 ") + args[0] + ", " + this_adjust);
        op << emit_temp_assignment("ptr", string("load ptr ") + entry_ptr);
      } else {
        string fn_ptr = vtable_ptr;
        const unsigned long long slot =
            node.children[0].has_uint_value ? callsem_uint_value(node.children[0]) : 0ULL;
        if(slot != 0) {
          fn_ptr = emit_temp_assignment("ptr",
                                        string("index i8 ") + vtable_ptr + ", " +
                                        to_string(static_cast<unsigned long long>(slot * 8ULL)));
        }
        op << emit_temp_assignment("ptr", string("load ptr ") + fn_ptr);
      }
    } else if(node.children[0].kind == CallSemKind::callee) {
      op << callee_symbol;
    } else {
      op << emit_rvalue(node.children[0]);
    }
    op << "(";
    for(size_t i = 0; i < args.size(); ++i) {
      if(i != 0) {
        op << ", ";
      }
      op << args[i];
    }
    op << ")";
    if(explicit_indirect_call_signature) {
      op << lowir_call_signature_suffix_for_call(node, function_type);
    }

    if(raw_result_lowir_type == "void") {
      emit_line(op.str());
      return "0";
    }
    return emit_temp_assignment(raw_result_lowir_type, op.str());
  }

  string emit_call_expression_raw(const CallSemNode & node,
                                  const CallArgumentCapture * capture = nullptr,
                                  const CallArgumentOverride * override_arg = nullptr)
  {
    if(!capture && !override_arg) {
      string chain_result;
      if(try_emit_left_nested_reference_call_chain(node, chain_result)) {
        return chain_result;
      }
    }

    const bool needs_constructor_wrapper =
        call_expression_needs_constructor_unwind_wrapper(node);
    const bool needs_host_wrapper =
        call_expression_needs_host_unwind_wrapper(node);
    if(shared_host_call_unwind_region_open_ && needs_constructor_wrapper) {
      close_shared_host_call_unwind_region();
    }
    if(!needs_constructor_wrapper && !needs_host_wrapper) {
      return emit_call_expression_raw_impl(node, capture, override_arg);
    }

    TypePtr function_type;
    if(!resolve_callable_function_type(node.children[0].semantic_type, function_type) ||
       !function_type || !function_type->inner) {
      throw logic_error("call-expression missing function type");
    }
    const string raw_result_lowir_type = lowir_result_type_text(function_type->inner);
    const string result_slot =
        raw_result_lowir_type == "void" ? string() : new_hidden_slot(raw_result_lowir_type, "call");
    if(use_host_eh_runtime() && needs_host_wrapper && !needs_constructor_wrapper) {
      open_shared_host_call_unwind_region();
      const string result = emit_call_expression_raw_impl(node, capture, override_arg);
      if(raw_result_lowir_type == "void") {
        return "0";
      }
      if(lowir_internal::is_object_type(lowir_internal::LowType{raw_result_lowir_type})) {
        emit_line("copyobj " + lowir_object_span_text(raw_result_lowir_type) + " " +
                  result + ", " + result_slot);
        return result_slot;
      }
      emit_line("store " + raw_result_lowir_type + " " + result + ", " + result_slot);
      return emit_temp_assignment(raw_result_lowir_type,
                                  string("load ") + raw_result_lowir_type + " " + result_slot);
    }
    const size_t host_dispatch_depth =
        use_host_eh_runtime() ? host_eh_region_depth_ + 1 : 0;
    bool created_dispatch = false;
    const string dispatch_label =
        shared_call_unwind_dispatch_label(needs_constructor_wrapper,
                                          host_dispatch_depth,
                                          created_dispatch);
    const string end_label =
        created_dispatch ? new_block("call_unwind_end") : string();
    emit_line("eh_try " + lowir_block_name(dispatch_label));
    if(use_host_eh_runtime()) {
      ++host_eh_region_depth_;
    }
    const string result = emit_call_expression_raw_impl(node, capture, override_arg);
    if(raw_result_lowir_type != "void") {
      if(lowir_internal::is_object_type(lowir_internal::LowType{raw_result_lowir_type})) {
        emit_line("copyobj " + lowir_object_span_text(raw_result_lowir_type) + " " +
                  result + ", " + result_slot);
      } else {
        emit_line("store " + raw_result_lowir_type + " " + result + ", " + result_slot);
      }
    }
    emit_line("eh_end");
    if(use_host_eh_runtime()) {
      if(host_eh_region_depth_ == 0) {
        throw logic_error("host EH region depth underflow");
      }
      --host_eh_region_depth_;
    }
    if(created_dispatch) {
      terminate("jump " + lowir_block_name(end_label));
      emit_shared_call_unwind_dispatch_block(dispatch_label,
                                             needs_constructor_wrapper,
                                             host_dispatch_depth);
      start_block(end_label);
    }
    if(raw_result_lowir_type == "void") {
      return "0";
    }
    if(lowir_internal::is_object_type(lowir_internal::LowType{raw_result_lowir_type})) {
      return result_slot;
    }
    return emit_temp_assignment(raw_result_lowir_type,
                                string("load ") + raw_result_lowir_type + " " + result_slot);
  }

  string new_temp()
  {
    while(true) {
      const string candidate = lowir_temp_name(++temp_counter_);
      if(!value_name_in_use(candidate)) {
        return candidate;
      }
    }
  }

  bool value_name_in_use(const string & name) const
  {
    for(size_t i = 0; i < function_.params.size(); ++i) {
      if(function_.params[i].name == name) {
        return true;
      }
    }
    return false;
  }

  bool can_name_debug_local_value(const string & name) const
  {
    return enable_debug_value_names_ &&
           lowir_internal::is_plain_identifier_text(name);
  }

  string new_debug_value_temp(const string & source_name)
  {
    return lowir_internal::lowir_debug_value_temp_name(
        source_name,
        ++debug_value_versions_[source_name]);
  }

  string new_block(const string & prefix)
  {
    ostringstream out;
    out << prefix << "_" << (++block_counter_);
    return out.str();
  }

  string new_hidden_slot(const string & type, const string & prefix)
  {
    return new_hidden_slots(type, prefix, 1)[0];
  }

  vector<string> new_hidden_slots(const string & type,
                                  const string & prefix,
                                  size_t count)
  {
    vector<string> names;
    for(size_t i = 0; i < count; ++i) {
      string candidate;
      do {
        candidate = lowir_hidden_slot_name(prefix, ++hidden_slot_counter_);
      } while(storage_name_in_use(candidate));
      names.push_back(candidate);
    }
    for(size_t i = names.size(); i > 0; --i) {
      function_.slots.push_back(make_pair(names[i - 1], type));
    }
    return names;
  }

  void start_block(const string & label)
  {
    function_.blocks.push_back(LowIRBlock());
    function_.blocks.back().label = lowir_block_name(label);
    current_block_ = &function_.blocks.back();
  }

  void ensure_current_block()
  {
    if(!current_block_) {
      start_block(new_block("block"));
    }
  }

  void emit_line(const string & line)
  {
    ensure_current_block();
    if(line.find("!dbg(") != string::npos) {
      current_block_->instructions.push_back(line);
      return;
    }
    current_block_->instructions.push_back(line + lowir_debug_suffix_for_current_node());
  }

  void terminate_no_close(const string & line)
  {
    emit_line(line);
    current_block_->terminated = true;
    current_block_ = nullptr;
  }

  void terminate(const string & line)
  {
    close_shared_host_call_unwind_region();
    terminate_no_close(line);
  }

  void push_cleanup_scope(bool is_full_expression = false)
  {
    close_shared_host_call_unwind_region();
    cleanup_scopes_.push_back(vector<CleanupAction>());
    cleanup_scope_normal_eh_end_counts_.push_back(0);
    cleanup_scope_host_unwind_cleanup_.push_back(false);
    cleanup_scope_is_full_expression_.push_back(is_full_expression);
  }

  void pop_cleanup_scope()
  {
    close_shared_host_call_unwind_region();
    if(cleanup_scopes_.empty()) {
      return;
    }
    cleanup_scopes_.pop_back();
    cleanup_scope_normal_eh_end_counts_.pop_back();
    cleanup_scope_host_unwind_cleanup_.pop_back();
    cleanup_scope_is_full_expression_.pop_back();
  }

  bool current_cleanup_scope_is_full_expression() const
  {
    return !cleanup_scope_is_full_expression_.empty() &&
           cleanup_scope_is_full_expression_.back();
  }

  void mark_current_cleanup_scope_host_unwind_cleanup()
  {
    if(cleanup_scopes_.empty()) {
      push_cleanup_scope();
    }
    cleanup_scope_host_unwind_cleanup_.back() = true;
  }

  void push_binding_scope()
  {
    binding_scopes_.push_back(vector<BindingScopeEntry>());
  }

  void pop_binding_scope()
  {
    if(binding_scopes_.empty()) {
      return;
    }
    vector<BindingScopeEntry> & scope = binding_scopes_.back();
    for(size_t i = scope.size(); i-- > 0;) {
      if(scope[i].had_previous) {
        bindings_[scope[i].name] = scope[i].previous;
      } else {
        bindings_.erase(scope[i].name);
      }
    }
    binding_scopes_.pop_back();
  }

  void register_local_binding(const string & name, const VariableBinding & binding)
  {
    if(binding_scopes_.empty()) {
      push_binding_scope();
    }
    BindingScopeEntry entry;
    entry.name = name;
    map<string, VariableBinding>::const_iterator found = bindings_.find(name);
    if(found != bindings_.end()) {
      entry.had_previous = true;
      entry.previous = found->second;
    }
    binding_scopes_.back().push_back(entry);
    bindings_[name] = binding;
  }

  void ensure_cleanup_registration_scope()
  {
    close_shared_host_call_unwind_region();
    if(cleanup_scopes_.empty()) {
      push_cleanup_scope();
    }
  }

  void append_cleanup_action(const CleanupAction & cleanup)
  {
    ensure_cleanup_registration_scope();
    cleanup_scopes_.back().push_back(cleanup);
  }

  void register_cleanup(const CallSemNode & action)
  {
    CleanupAction cleanup;
    cleanup.kind = CleanupAction::CK_NODE;
    cleanup.node = &action;
    append_cleanup_action(cleanup);
  }

  void register_bound_local_cleanup(const CallSemNode & action,
                                    const string & local_name,
                                    const string & storage_slot)
  {
    CleanupAction cleanup;
    cleanup.kind = CleanupAction::CK_BOUND_LOCAL_NODE;
    cleanup.node = &action;
    cleanup.storage_slot = storage_slot;
    cleanup.bound_local_name = local_name;
    append_cleanup_action(cleanup);
  }

  void register_eh_end_cleanup()
  {
    CleanupAction cleanup;
    cleanup.kind = CleanupAction::CK_EH_END;
    append_cleanup_action(cleanup);
  }

  void register_pre_scope_eh_end_cleanup()
  {
    ensure_cleanup_registration_scope();
    ++cleanup_scope_normal_eh_end_counts_.back();
  }

  void register_clear_exception_cleanup()
  {
    CleanupAction cleanup;
    cleanup.kind = CleanupAction::CK_CLEAR_EXCEPTION;
    append_cleanup_action(cleanup);
  }

  void register_class_object_cleanup(const VariableBinding & binding)
  {
    if(!is_complete_class_value_type(binding.semantic_type) ||
       binding.slots.empty()) {
      return;
    }
    if(!destructor_runtime_call_required(binding.semantic_type)) {
      return;
    }
    CleanupAction cleanup;
    cleanup.kind = CleanupAction::CK_DESTROY_CLASS_OBJECT;
    cleanup.storage_slot = binding.slots[0];
    cleanup.object_type = binding.semantic_type;
    append_cleanup_action(cleanup);
  }

  void register_class_at_ptr_cleanup(const TypePtr & type,
                                     const string & object_ptr)
  {
    TypePtr object_type = strip_top_level_cv(remove_reference_type(type));
    if(!object_type || !is_complete_class_value_type(object_type) ||
       object_ptr.empty() || !destructor_runtime_call_required(object_type)) {
      return;
    }
    CleanupAction cleanup;
    cleanup.kind = CleanupAction::CK_DESTROY_CLASS_AT_PTR;
    cleanup.storage_slot = object_ptr;
    cleanup.object_type = object_type;
    append_cleanup_action(cleanup);
  }

  void register_materialized_temporary_cleanup(const TypePtr & type,
                                               const string & object_ptr)
  {
    TypePtr object_type = strip_top_level_cv(remove_reference_type(type));
    if(!current_cleanup_scope_is_full_expression()) {
      return;
    }

    if(!is_complete_class_value_type(object_type) ||
       !destructor_runtime_call_required(object_type)) {
      return;
    }

    CleanupAction cleanup;
    cleanup.kind = CleanupAction::CK_DESTROY_CLASS_AT_PTR;
    cleanup.storage_slot = object_ptr;
    cleanup.object_type = object_type;
    append_cleanup_action(cleanup);
  }

  void register_materialized_temporary_cleanup_live(const TypePtr & type,
                                                    const string & object_ptr)
  {
    const bool refresh_shared_host_region = shared_host_call_unwind_region_open_;
    if(refresh_shared_host_region) {
      close_shared_host_call_unwind_region();
    }
    register_materialized_temporary_cleanup(type, object_ptr);
    if(refresh_shared_host_region && current_scope_has_host_unwind_cleanups()) {
      open_shared_host_call_unwind_region();
    }
  }

  CallSemNode bind_cleanup_local_node(const CallSemNode & node,
                                      const string & local_name,
                                      const string & storage_slot) const
  {
    CallSemNode out = node;
    if(node.kind == CallSemKind::id_expression &&
       node.text == local_name &&
      !storage_slot.empty()) {
      out.text.clear();
      clear_callsem_resolved_name(out);
      symbol_linkage::SymbolIdentity local_symbol;
      local_symbol.internal_symbol = storage_slot;
      local_symbol.object_symbol = storage_slot;
      local_symbol.linkage = symbol_linkage::SL_INTERNAL;
      set_callsem_symbol(out, local_symbol);
      return out;
    }
    for(size_t i = 0; i < node.children.size(); ++i) {
      out.children[i] = bind_cleanup_local_node(node.children[i], local_name, storage_slot);
    }
    if(callsem_lowered_condition_test(node)) {
      mutable_callsem_lowered_condition_test(out) =
          make_shared<CallSemNode>(
              bind_cleanup_local_node(*callsem_lowered_condition_test(node),
                                      local_name,
                                      storage_slot));
    }
    return out;
  }

  void emit_cleanup_action(const CleanupAction & cleanup, bool for_throw)
  {
    switch(cleanup.kind) {
    case CleanupAction::CK_NODE:
      if(cleanup.node) {
        emit_action(*cleanup.node);
      }
      return;
    case CleanupAction::CK_BOUND_LOCAL_NODE:
      if(cleanup.node) {
        emit_action(bind_cleanup_local_node(*cleanup.node,
                                            cleanup.bound_local_name,
                                            cleanup.storage_slot));
      }
      return;
    case CleanupAction::CK_DESTROY_CLASS_OBJECT:
      emit_destroy_complete_class_temporary(cleanup.object_type,
                                            emit_storage_address(cleanup.storage_slot));
      return;
    case CleanupAction::CK_DESTROY_CLASS_AT_NODE:
      if(cleanup.node) {
        emit_destroy_complete_class_temporary(cleanup.object_type, emit_rvalue(*cleanup.node));
      }
      return;
    case CleanupAction::CK_DESTROY_CLASS_AT_PTR:
      emit_destroy_complete_class_temporary(cleanup.object_type, cleanup.storage_slot);
      return;
    case CleanupAction::CK_EH_END:
      if(!for_throw) {
        emit_line("eh_end");
      }
      return;
    case CleanupAction::CK_CLEAR_EXCEPTION:
      if(!for_throw) {
        emit_clear_current_exception();
      }
      return;
    }
  }

  bool should_skip_cleanup_action(const CleanupAction & cleanup,
                                  bool for_throw,
                                  const string & excluded_destroy_ptr) const
  {
    return !for_throw &&
           !excluded_destroy_ptr.empty() &&
           cleanup.kind == CleanupAction::CK_DESTROY_CLASS_AT_PTR &&
           cleanup.storage_slot == excluded_destroy_ptr;
  }

  void emit_scope_cleanups(const vector<CleanupAction> & scope,
                           bool for_throw = false)
  {
    ++cleanup_emission_depth_;
    for(size_t i = scope.size(); i-- > 0;) {
      emit_cleanup_action(scope[i], for_throw);
    }
    --cleanup_emission_depth_;
  }

  void emit_scope_cleanups_excluding_destroy_at_ptr(const vector<CleanupAction> & scope,
                                                    const string & excluded_destroy_ptr,
                                                    bool for_throw = false)
  {
    ++cleanup_emission_depth_;
    for(size_t i = scope.size(); i-- > 0;) {
      if(should_skip_cleanup_action(scope[i], for_throw, excluded_destroy_ptr)) {
        continue;
      }
      emit_cleanup_action(scope[i], for_throw);
    }
    --cleanup_emission_depth_;
  }

  void emit_normal_scope_cleanups(const vector<CleanupAction> & scope,
                                  size_t normal_eh_end_count)
  {
    close_shared_host_call_unwind_region();
    for(size_t i = 0; i < normal_eh_end_count; ++i) {
      emit_line("eh_end");
    }
    emit_scope_cleanups(scope, false);
  }

  void emit_cleanups_to_depth(size_t target_cleanup_depth)
  {
    if(target_cleanup_depth > cleanup_scopes_.size()) {
      throw logic_error("control transfer cleanup depth overflow");
    }
    for(size_t i = cleanup_scopes_.size(); i-- > target_cleanup_depth;) {
      emit_normal_scope_cleanups(cleanup_scopes_[i],
                                 cleanup_scope_normal_eh_end_counts_[i]);
    }
  }

  void emit_all_cleanups(bool for_throw = false)
  {
    close_shared_host_call_unwind_region();
    for(size_t i = cleanup_scopes_.size(); i-- > 0;) {
      if(for_throw) {
        emit_scope_cleanups(cleanup_scopes_[i], true);
      } else {
        emit_normal_scope_cleanups(cleanup_scopes_[i],
                                   cleanup_scope_normal_eh_end_counts_[i]);
      }
      if(for_throw) {
        const bool crossed_try_boundary =
            any_of(cleanup_scopes_[i].begin(),
                   cleanup_scopes_[i].end(),
                   [](const CleanupAction & cleanup)
                   {
                     return cleanup.kind == CleanupAction::CK_EH_END;
                   });
        if(crossed_try_boundary) {
          break;
        }
      }
    }
  }

  void emit_all_cleanups_excluding_destroy_at_ptr(const string & excluded_destroy_ptr,
                                                  bool for_throw = false)
  {
    close_shared_host_call_unwind_region();
    for(size_t i = cleanup_scopes_.size(); i-- > 0;) {
      if(for_throw) {
        emit_scope_cleanups_excluding_destroy_at_ptr(cleanup_scopes_[i],
                                                     excluded_destroy_ptr,
                                                     true);
      } else {
        for(size_t j = 0; j < cleanup_scope_normal_eh_end_counts_[i]; ++j) {
          emit_line("eh_end");
        }
        emit_scope_cleanups_excluding_destroy_at_ptr(cleanup_scopes_[i],
                                                     excluded_destroy_ptr,
                                                     false);
      }
      if(for_throw) {
        const bool crossed_try_boundary =
            any_of(cleanup_scopes_[i].begin(),
                   cleanup_scopes_[i].end(),
                   [](const CleanupAction & cleanup)
                   {
                     return cleanup.kind == CleanupAction::CK_EH_END;
                   });
        if(crossed_try_boundary) {
          break;
        }
      }
    }
  }

  void emit_explicit_host_throw_cleanups()
  {
    close_shared_host_call_unwind_region();
    for(size_t i = cleanup_scopes_.size(); i-- > 0;) {
      if(cleanup_scope_host_unwind_cleanup_[i]) {
        continue;
      }
      emit_scope_cleanups(cleanup_scopes_[i], true);
      const bool crossed_try_boundary =
          any_of(cleanup_scopes_[i].begin(),
                 cleanup_scopes_[i].end(),
                 [](const CleanupAction & cleanup)
                 {
                   return cleanup.kind == CleanupAction::CK_EH_END;
                 });
      if(crossed_try_boundary) {
        break;
      }
    }
  }

  bool throw_will_escape_current_function() const
  {
    for(size_t i = cleanup_scopes_.size(); i-- > 0;) {
      const vector<CleanupAction> & scope = cleanup_scopes_[i];
      if(any_of(scope.begin(),
                scope.end(),
                [](const CleanupAction & cleanup)
                {
                  return cleanup.kind == CleanupAction::CK_EH_END;
                })) {
        return false;
      }
    }
    return true;
  }

  void emit_constructor_unwind_cleanups()
  {
    close_shared_host_call_unwind_region();
    for(size_t i = constructor_unwind_cleanups_.size(); i-- > 0;) {
      emit_cleanup_action(constructor_unwind_cleanups_[i], true);
    }
  }

  string emit_temp_assignment(const string & type, const string & op)
  {
    const string temp = new_temp();
    emit_line(temp + " = " + op);
    return temp;
  }

  string emit_named_temp_assignment(const string & temp,
                                    const string & type,
                                    const string & op)
  {
    (void) type;
    emit_line(temp + " = " + op);
    return temp;
  }

  string emit_debug_named_local_value(const string & source_name,
                                      const string & type,
                                      const string & value)
  {
    if(!can_name_debug_local_value(source_name)) {
      return value;
    }
    return emit_named_temp_assignment(new_debug_value_temp(source_name),
                                      type,
                                      string("copy ") + type + " " + value);
  }

  string emit_atomic_compare_exchange_loop(const string & value_type,
                                           const string & ptr,
                                           const string & update_value,
                                           long long order,
                                           const string & op)
  {
    const string expected_slot = new_hidden_slot(value_type, "atomic_expected");
    const string expected_ptr = emit_storage_address(expected_slot);
    const string initial_expected =
        emit_temp_assignment(value_type,
                             string("atomic_load ") + value_type + " " + ptr +
                             ", " + to_string(order));
    emit_line(string("store ") + value_type + " " + initial_expected + ", " +
              expected_slot);

    const string loop_label = new_block("atomic_loop");
    const string done_label = new_block("atomic_done");
    terminate(string("jump ") + lowir_block_name(loop_label));

    start_block(loop_label);
    const string current_expected =
        emit_temp_assignment(value_type,
                             string("load ") + value_type + " " + expected_slot);
    const string desired =
        emit_temp_assignment(value_type,
                             string("binary ") + op + " " + value_type + " " +
                             current_expected + ", " + update_value);
    const string success =
        emit_temp_assignment("i64",
                             string("atomic_compare_exchange ") + value_type +
                             " " + ptr + ", " + expected_ptr + ", " + desired +
                             ", " + to_string(order) + ", " + to_string(order));
    terminate(string("branch ") + success + ", " + lowir_block_name(done_label) +
              ", " + lowir_block_name(loop_label));

    start_block(done_label);
    return emit_temp_assignment(value_type,
                                string("load ") + value_type + " " +
                                expected_slot);
  }

  string emit_builtin_bswap_value(const string & builtin_name,
                                  const string & value)
  {
    if(builtin_name == "__builtin_bswap16") {
      const string masked = emit_temp_assignment("i64", string("binary and i64 ") + value + ", 65535");
      const string lo = emit_temp_assignment("i64", string("binary and i64 ") + masked + ", 255");
      const string hi = emit_temp_assignment("i64", string("binary and i64 ") + masked + ", 65280");
      const string lo_shift = emit_temp_assignment("i64", string("binary shl i64 ") + lo + ", 8");
      const string hi_shift = emit_temp_assignment("i64", string("binary shr i64 ") + hi + ", 8");
      return emit_temp_assignment("i64", string("binary or i64 ") + lo_shift + ", " + hi_shift);
    }

    if(builtin_name == "__builtin_bswap32") {
      const string masked =
          emit_temp_assignment("i64", string("binary and i64 ") + value + ", 4294967295");
      const string b0 = emit_temp_assignment("i64", string("binary and i64 ") + masked + ", 255");
      const string b1 = emit_temp_assignment("i64", string("binary and i64 ") + masked + ", 65280");
      const string b2 =
          emit_temp_assignment("i64", string("binary and i64 ") + masked + ", 16711680");
      const string b3 =
          emit_temp_assignment("i64", string("binary and i64 ") + masked + ", 4278190080");
      const string s0 = emit_temp_assignment("i64", string("binary shl i64 ") + b0 + ", 24");
      const string s1 = emit_temp_assignment("i64", string("binary shl i64 ") + b1 + ", 8");
      const string s2 = emit_temp_assignment("i64", string("binary shr i64 ") + b2 + ", 8");
      const string s3 = emit_temp_assignment("i64", string("binary shr i64 ") + b3 + ", 24");
      const string p0 = emit_temp_assignment("i64", string("binary or i64 ") + s0 + ", " + s1);
      const string p1 = emit_temp_assignment("i64", string("binary or i64 ") + s2 + ", " + s3);
      return emit_temp_assignment("i64", string("binary or i64 ") + p0 + ", " + p1);
    }

    if(builtin_name == "__builtin_bswap64") {
      return emit_temp_assignment("i64", string("unary bswap i64 ") + value);
    }

    throw logic_error("unknown bswap builtin " + builtin_name);
  }

  static string host_fpclassify_symbol_for_lowir_type(const string & value_type)
  {
    if(value_type == "f32") {
      return "__fpclassifyf";
    }
    if(value_type == "f80") {
      return "__fpclassifyl";
    }
    if(value_type == "f64") {
#if defined(__linux__)
      return "__fpclassify";
#else
      return "__fpclassifyd";
#endif
    }
    throw logic_error("__builtin_fpclassify unsupported value type " + value_type);
  }

  string emit_fpclassify_case_value(const string & classification,
                                    int host_classification_value,
                                    const string & mapped_value)
  {
    const string selected =
        emit_temp_assignment("i64",
                             string("cmp eq i32 ") + classification + ", " +
                             to_string(host_classification_value));
    const string selected_i32 = emit_lowir_convert("trunc", "i32", "i64", selected);
    return emit_temp_assignment("i32",
                                string("binary mul i32 ") + mapped_value + ", " +
                                selected_i32);
  }

  string emit_builtin_fpclassify_value(const CallSemNode & node)
  {
    if(node.children.size() != 7) {
      throw logic_error("__builtin_fpclassify child count");
    }

    vector<string> mapped_values;
    mapped_values.reserve(5);
    for(size_t i = 1; i <= 5; ++i) {
      mapped_values.push_back(emit_rvalue(node.children[i]));
    }

    const string value_type = lowir_type_for(node.children[6].semantic_type);
    const string value = emit_rvalue(node.children[6]);
    const string classifier =
        external_runtime_symbol(host_fpclassify_symbol_for_lowir_type(value_type));
    const string classification =
        emit_temp_assignment("i32",
                             string("call i32 ") + classifier + "(" + value + ")");

    const string nan_part =
        emit_fpclassify_case_value(classification, FP_NAN, mapped_values[0]);
    const string infinite_part =
        emit_fpclassify_case_value(classification, FP_INFINITE, mapped_values[1]);
    const string normal_part =
        emit_fpclassify_case_value(classification, FP_NORMAL, mapped_values[2]);
    const string subnormal_part =
        emit_fpclassify_case_value(classification, FP_SUBNORMAL, mapped_values[3]);
    const string zero_part =
        emit_fpclassify_case_value(classification, FP_ZERO, mapped_values[4]);

    const string nan_or_infinite =
        emit_temp_assignment("i32",
                             string("binary add i32 ") + nan_part + ", " +
                             infinite_part);
    const string with_normal =
        emit_temp_assignment("i32",
                             string("binary add i32 ") + nan_or_infinite + ", " +
                             normal_part);
    const string with_subnormal =
        emit_temp_assignment("i32",
                             string("binary add i32 ") + with_normal + ", " +
                             subnormal_part);
    return emit_temp_assignment("i32",
                                string("binary add i32 ") + with_subnormal + ", " +
                                zero_part);
  }

  string emit_builtin_signbit_value(const CallSemNode & node)
  {
    if(node.children.size() != 2) {
      throw logic_error("__builtin_signbit child count");
    }

    const string value_type = lowir_type_for(node.children[1].semantic_type);
    const string value = emit_rvalue(node.children[1]);
    const string value_slot = new_hidden_slot(value_type, "signbit_value");
    emit_line("store " + value_type + " " + value + ", " + value_slot);

    string bits_type;
    string sign_bit_index;
    string bits_address = value_slot;
    if(value_type == "f32") {
      bits_type = "u32";
      sign_bit_index = "31";
    } else if(value_type == "f64") {
      bits_type = "i64";
      sign_bit_index = "63";
    } else if(value_type == "f80") {
      bits_type = "u16";
      sign_bit_index = "15";
      bits_address =
          emit_index_address_with_projection("i8", emit_storage_address(value_slot), 8);
    } else {
      throw logic_error("__builtin_signbit unsupported value type " + value_type);
    }

    const string bits =
        emit_temp_assignment(bits_type, string("load ") + bits_type + " " + bits_address);
    const string shifted =
        emit_temp_assignment(bits_type,
                             string("binary ushr ") + bits_type + " " + bits + ", " +
                             sign_bit_index);
    const string one =
        emit_temp_assignment(bits_type, string("binary and ") + bits_type + " " +
                                        shifted + ", 1");
    const string result_type = lowir_type_for(node.semantic_type);
    if(result_type == bits_type) {
      return one;
    }
    return emit_lowir_convert("trunc", result_type, bits_type, one);
  }

  string emit_classification_result(const CallSemNode & node, const string & value)
  {
    const string result_type = lowir_type_for(node.semantic_type);
    if(result_type == "i64") {
      return value;
    }
    return emit_lowir_convert("trunc", result_type, "i64", value);
  }

  string bool_not_value(const string & value)
  {
    return emit_temp_assignment("i64", string("cmp eq i64 ") + value + ", 0");
  }

  string bool_and_value(const string & lhs, const string & rhs)
  {
    return emit_temp_assignment("i64", string("binary and i64 ") + lhs + ", " + rhs);
  }

  string emit_builtin_fp_classification_value(const string & builtin_name,
                                              const CallSemNode & node)
  {
    if(node.children.size() != 2) {
      throw logic_error(builtin_name + " child count");
    }

    const string value_type = lowir_type_for(node.children[1].semantic_type);
    const string value = emit_rvalue(node.children[1]);
    const string value_slot = new_hidden_slot(value_type, "fpclass_value");
    emit_line("store " + value_type + " " + value + ", " + value_slot);

    string exp_all;
    string exp_zero;
    string frac_zero;
    string inf_sig;
    if(value_type == "f32") {
      const string bits =
          emit_temp_assignment("u32", string("load u32 ") + value_slot);
      const string exp =
          emit_temp_assignment("u32", string("binary and u32 ") + bits + ", 2139095040");
      const string frac =
          emit_temp_assignment("u32", string("binary and u32 ") + bits + ", 8388607");
      exp_all = emit_temp_assignment("i64", string("cmp eq u32 ") + exp + ", 2139095040");
      exp_zero = emit_temp_assignment("i64", string("cmp eq u32 ") + exp + ", 0");
      frac_zero = emit_temp_assignment("i64", string("cmp eq u32 ") + frac + ", 0");
      inf_sig = frac_zero;
    } else if(value_type == "f64") {
      const string bits =
          emit_temp_assignment("i64", string("load i64 ") + value_slot);
      const string exp =
          emit_temp_assignment("i64",
                               string("binary and i64 ") + bits + ", 9218868437227405312");
      const string frac =
          emit_temp_assignment("i64",
                               string("binary and i64 ") + bits + ", 4503599627370495");
      exp_all =
          emit_temp_assignment("i64",
                               string("cmp eq i64 ") + exp + ", 9218868437227405312");
      exp_zero = emit_temp_assignment("i64", string("cmp eq i64 ") + exp + ", 0");
      frac_zero = emit_temp_assignment("i64", string("cmp eq i64 ") + frac + ", 0");
      inf_sig = frac_zero;
    } else if(value_type == "f80") {
      const string significand =
          emit_temp_assignment("i64", string("load i64 ") + value_slot);
      const string tag_address =
          emit_index_address_with_projection("i8", emit_storage_address(value_slot), 8);
      const string tag =
          emit_temp_assignment("u16", string("load u16 ") + tag_address);
      const string exp =
          emit_temp_assignment("u16", string("binary and u16 ") + tag + ", 32767");
      const string frac =
          emit_temp_assignment("i64",
                               string("binary and i64 ") + significand +
                                   ", 9223372036854775807");
      exp_all = emit_temp_assignment("i64", string("cmp eq u16 ") + exp + ", 32767");
      exp_zero = emit_temp_assignment("i64", string("cmp eq u16 ") + exp + ", 0");
      frac_zero = emit_temp_assignment("i64", string("cmp eq i64 ") + frac + ", 0");
      const string integer_bit =
          emit_temp_assignment("i64", string("cmp lt i64 ") + significand + ", 0");
      inf_sig = bool_and_value(integer_bit, frac_zero);
    } else {
      throw logic_error(builtin_name + " unsupported value type " + value_type);
    }

    string result;
    if(builtin_name == "__builtin_isnan") {
      result = bool_and_value(exp_all, bool_not_value(inf_sig));
    } else if(builtin_name == "__builtin_isinf") {
      result = bool_and_value(exp_all, inf_sig);
    } else if(builtin_name == "__builtin_isfinite") {
      result = bool_not_value(exp_all);
    } else if(builtin_name == "__builtin_isnormal") {
      result = bool_and_value(bool_not_value(exp_zero), bool_not_value(exp_all));
    } else {
      throw logic_error("unknown fp classification builtin " + builtin_name);
    }
    return emit_classification_result(node, result);
  }

  string emit_builtin_popcount_value(const CallSemNode & node)
  {
    if(node.children.size() != 2) {
      throw logic_error("__builtin_popcount child count");
    }

    const string result_type = lowir_type_for(node.semantic_type);
    const string value_type = lowir_type_for(node.children[1].semantic_type);
    const size_t bit_count = type_size(node.children[1].semantic_type) * 8;
    if(bit_count == 0 || bit_count > 64) {
      throw logic_error("__builtin_popcount unsupported operand width");
    }

    const string value = emit_rvalue(node.children[1]);
    const string value_slot = new_hidden_slot(value_type, "popcount_value");
    const string count_slot = new_hidden_slot(result_type, "popcount_count");
    const string check_label = new_block("popcount_check");
    const string next_label = new_block("popcount_next");
    const string done_label = new_block("popcount_done");

    emit_line("store " + value_type + " " + value + ", " + value_slot);
    emit_line("store " + result_type + " 0, " + count_slot);
    terminate(string("jump ") + lowir_block_name(check_label));

    start_block(check_label);
    const string current =
        emit_temp_assignment(value_type, string("load ") + value_type + " " + value_slot);
    const string is_zero =
        emit_temp_assignment("i64", string("cmp eq ") + value_type + " " + current + ", 0");
    terminate(string("branch ") + is_zero + ", " + lowir_block_name(done_label) + ", " +
              lowir_block_name(next_label));

    start_block(next_label);
    const string low_bit =
        emit_temp_assignment(value_type, string("binary and ") + value_type + " " + current + ", 1");
    const string bit_set =
        emit_temp_assignment("i64", string("cmp ne ") + value_type + " " + low_bit + ", 0");
    const string count =
        emit_temp_assignment(result_type, string("load ") + result_type + " " + count_slot);
    const string next_count =
        emit_temp_assignment(result_type,
                             string("binary add ") + result_type + " " + count + ", " + bit_set);
    emit_line("store " + result_type + " " + next_count + ", " + count_slot);
    const string shifted =
        emit_temp_assignment(value_type, string("binary ushr ") + value_type + " " + current + ", 1");
    emit_line("store " + value_type + " " + shifted + ", " + value_slot);
    terminate(string("jump ") + lowir_block_name(check_label));

    start_block(done_label);
    return emit_temp_assignment(result_type,
                                string("load ") + result_type + " " + count_slot);
  }

  string emit_builtin_clzg_value(const CallSemNode & node)
  {
    if(node.children.size() != 2 && node.children.size() != 3) {
      throw logic_error("__builtin_clzg child count");
    }

    const string result_type = lowir_type_for(node.semantic_type);
    const string value_type = lowir_type_for(node.children[1].semantic_type);
    const size_t bit_count = type_size(node.children[1].semantic_type) * 8;
    if(bit_count == 0 || bit_count > 64) {
      throw logic_error("__builtin_clzg unsupported operand width");
    }

    const string value = emit_rvalue(node.children[1]);
    const string zero_result =
        node.children.size() == 3
            ? emit_rvalue(node.children[2])
            : emit_temp_assignment(result_type,
                                   string("const ") + result_type + " " +
                                   to_string(static_cast<unsigned long long>(bit_count)));
    const string is_zero =
        emit_temp_assignment("i64", string("cmp eq ") + value_type + " " + value + ", 0");
    const string result_slot = new_hidden_slot(result_type, "clzg");
    const string value_slot = new_hidden_slot(value_type, "clzg_value");
    const string count_slot = new_hidden_slot(result_type, "clzg_count");
    const string zero_label = new_block("clzg_zero");
    const string init_label = new_block("clzg_init");
    const string check_label = new_block("clzg_check");
    const string next_label = new_block("clzg_next");
    const string done_label = new_block("clzg_done");
    const string end_label = new_block("clzg_end");

    terminate(string("branch ") + is_zero + ", " + lowir_block_name(zero_label) + ", " +
              lowir_block_name(init_label));

    start_block(zero_label);
    emit_line("store " + result_type + " " + zero_result + ", " + result_slot);
    terminate(string("jump ") + lowir_block_name(end_label));

    start_block(init_label);
    emit_line("store " + value_type + " " + value + ", " + value_slot);
    emit_line("store " + result_type + " 0, " + count_slot);
    terminate(string("jump ") + lowir_block_name(check_label));

    start_block(check_label);
    const string current =
        emit_temp_assignment(value_type, string("load ") + value_type + " " + value_slot);
    const string top =
        emit_temp_assignment(value_type,
                             string("binary shr ") + value_type + " " + current + ", " +
                             to_string(static_cast<unsigned long long>(bit_count - 1)));
    const string has_high =
        emit_temp_assignment("i64", string("cmp ne ") + value_type + " " + top + ", 0");
    terminate(string("branch ") + has_high + ", " + lowir_block_name(done_label) + ", " +
              lowir_block_name(next_label));

    start_block(next_label);
    const string shifted =
        emit_temp_assignment(value_type,
                             string("binary shl ") + value_type + " " + current + ", 1");
    emit_line("store " + value_type + " " + shifted + ", " + value_slot);
    const string count =
        emit_temp_assignment(result_type, string("load ") + result_type + " " + count_slot);
    const string next_count =
        emit_temp_assignment(result_type,
                             string("binary add ") + result_type + " " + count + ", 1");
    emit_line("store " + result_type + " " + next_count + ", " + count_slot);
    terminate(string("jump ") + lowir_block_name(check_label));

    start_block(done_label);
    const string final_count =
        emit_temp_assignment(result_type, string("load ") + result_type + " " + count_slot);
    emit_line("store " + result_type + " " + final_count + ", " + result_slot);
    terminate(string("jump ") + lowir_block_name(end_label));

    start_block(end_label);
    return emit_temp_assignment(result_type,
                                string("load ") + result_type + " " + result_slot);
  }

  string emit_builtin_ctzg_value(const CallSemNode & node)
  {
    if(node.children.size() != 2 && node.children.size() != 3) {
      throw logic_error("__builtin_ctzg child count");
    }

    const string result_type = lowir_type_for(node.semantic_type);
    const string value_type = lowir_type_for(node.children[1].semantic_type);
    const size_t bit_count = type_size(node.children[1].semantic_type) * 8;
    if(bit_count == 0 || bit_count > 64) {
      throw logic_error("__builtin_ctzg unsupported operand width");
    }

    const string value = emit_rvalue(node.children[1]);
    const string zero_result =
        node.children.size() == 3
            ? emit_rvalue(node.children[2])
            : emit_temp_assignment(result_type,
                                   string("const ") + result_type + " " +
                                   to_string(static_cast<unsigned long long>(bit_count)));
    const string is_zero =
        emit_temp_assignment("i64", string("cmp eq ") + value_type + " " + value + ", 0");
    const string result_slot = new_hidden_slot(result_type, "ctzg");
    const string value_slot = new_hidden_slot(value_type, "ctzg_value");
    const string count_slot = new_hidden_slot(result_type, "ctzg_count");
    const string zero_label = new_block("ctzg_zero");
    const string init_label = new_block("ctzg_init");
    const string check_label = new_block("ctzg_check");
    const string next_label = new_block("ctzg_next");
    const string done_label = new_block("ctzg_done");
    const string end_label = new_block("ctzg_end");

    terminate(string("branch ") + is_zero + ", " + lowir_block_name(zero_label) + ", " +
              lowir_block_name(init_label));

    start_block(zero_label);
    emit_line("store " + result_type + " " + zero_result + ", " + result_slot);
    terminate(string("jump ") + lowir_block_name(end_label));

    start_block(init_label);
    emit_line("store " + value_type + " " + value + ", " + value_slot);
    emit_line("store " + result_type + " 0, " + count_slot);
    terminate(string("jump ") + lowir_block_name(check_label));

    start_block(check_label);
    const string current =
        emit_temp_assignment(value_type, string("load ") + value_type + " " + value_slot);
    const string low_bit =
        emit_temp_assignment(value_type, string("binary and ") + value_type + " " + current + ", 1");
    const string has_low =
        emit_temp_assignment("i64", string("cmp ne ") + value_type + " " + low_bit + ", 0");
    terminate(string("branch ") + has_low + ", " + lowir_block_name(done_label) + ", " +
              lowir_block_name(next_label));

    start_block(next_label);
    const string shifted =
        emit_temp_assignment(value_type,
                             string("binary shr ") + value_type + " " + current + ", 1");
    emit_line("store " + value_type + " " + shifted + ", " + value_slot);
    const string count =
        emit_temp_assignment(result_type, string("load ") + result_type + " " + count_slot);
    const string next_count =
        emit_temp_assignment(result_type,
                             string("binary add ") + result_type + " " + count + ", 1");
    emit_line("store " + result_type + " " + next_count + ", " + count_slot);
    terminate(string("jump ") + lowir_block_name(check_label));

    start_block(done_label);
    const string final_count =
        emit_temp_assignment(result_type, string("load ") + result_type + " " + count_slot);
    emit_line("store " + result_type + " " + final_count + ", " + result_slot);
    terminate(string("jump ") + lowir_block_name(end_label));

    start_block(end_label);
    return emit_temp_assignment(result_type,
                                string("load ") + result_type + " " + result_slot);
  }

  string emit_explicit_integer_value_from_normalized(const string & normalized_value,
                                                     const TypePtr & value_type)
  {
    const string raw_lowir = lowir_memory_type_for(value_type);
    if(raw_lowir == "i64") {
      return normalized_value;
    }
    return emit_lowir_convert("trunc", raw_lowir, "i64", normalized_value);
  }

  string emit_explicit_integer_argument_value(const CallSemNode & arg,
                                              const TypePtr & value_type)
  {
    const string normalized_value =
        emit_scalar_value_conversion(emit_rvalue(arg), arg.semantic_type, value_type, true);
    return emit_explicit_integer_value_from_normalized(normalized_value, value_type);
  }

  string widen_explicit_integer_value(const string & raw_value,
                                      const TypePtr & value_type)
  {
    const string raw_lowir = lowir_memory_type_for(value_type);
    if(raw_lowir == "i64") {
      return raw_value;
    }
    return emit_lowir_convert(is_unsigned_integral_type(value_type) ? "zext" : "sext",
                              "i64",
                              raw_lowir,
                              raw_value);
  }

  string emit_builtin_same_type_overflow_value(const string & builtin_name,
                                               const CallSemNode & node)
  {
    if(node.children.size() != 4) {
      throw logic_error(builtin_name + " child count");
    }

    TypePtr value_type = strip_top_level_cv(remove_reference_type(node.children[1].semantic_type));
    if(!value_type || !is_integral_type(value_type)) {
      throw logic_error(builtin_name + " requires integral value type");
    }

    const string value_lowir = lowir_memory_type_for(value_type);
    const size_t bit_count = type_size(value_type) * 8;
    if(bit_count == 0 || bit_count > 64) {
      throw logic_error(builtin_name + " unsupported operand width");
    }

    const bool is_unsigned = is_unsigned_integral_type(value_type);
    const string lhs = emit_explicit_integer_argument_value(node.children[1], value_type);
    const string rhs = emit_explicit_integer_argument_value(node.children[2], value_type);
    const string result =
        emit_temp_assignment(value_lowir,
                             string("binary ") +
                                 (builtin_name == "__builtin_add_overflow" ? "add" :
                                  builtin_name == "__builtin_sub_overflow" ? "sub" :
                                                                               "mul") +
                                 " " + value_lowir + " " + lhs + ", " + rhs);
    const string out_ptr = emit_rvalue(node.children[3]);
    emit_line("store " + value_lowir + " " + result + ", " + out_ptr);

    if(builtin_name == "__builtin_add_overflow") {
      if(is_unsigned) {
        return emit_temp_assignment("i64",
                                    string("cmp ult ") + value_lowir + " " + result + ", " + lhs);
      }

      const string sum_xor_lhs =
          emit_temp_assignment(value_lowir,
                               string("binary xor ") + value_lowir + " " + result + ", " + lhs);
      const string sum_xor_rhs =
          emit_temp_assignment(value_lowir,
                               string("binary xor ") + value_lowir + " " + result + ", " + rhs);
      const string sign_mask =
          emit_temp_assignment(value_lowir,
                               string("binary and ") + value_lowir + " " + sum_xor_lhs + ", " +
                                   sum_xor_rhs);
      return emit_temp_assignment("i64",
                                  string("cmp lt ") + value_lowir + " " + sign_mask + ", 0");
    }

    if(builtin_name == "__builtin_sub_overflow") {
      if(is_unsigned) {
        return emit_temp_assignment("i64",
                                    string("cmp ult ") + value_lowir + " " + lhs + ", " + rhs);
      }

      const string lhs_xor_rhs =
          emit_temp_assignment(value_lowir,
                               string("binary xor ") + value_lowir + " " + lhs + ", " + rhs);
      const string diff_xor_lhs =
          emit_temp_assignment(value_lowir,
                               string("binary xor ") + value_lowir + " " + result + ", " + lhs);
      const string sign_mask =
          emit_temp_assignment(value_lowir,
                               string("binary and ") + value_lowir + " " + lhs_xor_rhs + ", " +
                                   diff_xor_lhs);
      return emit_temp_assignment("i64",
                                  string("cmp lt ") + value_lowir + " " + sign_mask + ", 0");
    }

    if(builtin_name != "__builtin_mul_overflow") {
      throw logic_error("unknown overflow builtin " + builtin_name);
    }

    if(value_lowir != "i64") {
      const string lhs_wide = widen_explicit_integer_value(lhs, value_type);
      const string rhs_wide = widen_explicit_integer_value(rhs, value_type);
      const string exact_result =
          emit_temp_assignment("i64", string("binary mul i64 ") + lhs_wide + ", " + rhs_wide);
      if(is_unsigned) {
        return emit_temp_assignment("i64",
                                    string("cmp ugt i64 ") + exact_result + ", " +
                                        to_string(lowir_bit_field_mask(bit_count)));
      }

      const long long min_value =
          bit_count == 64 ? std::numeric_limits<long long>::min()
                          : -(1LL << (bit_count - 1));
      const long long max_value =
          bit_count == 64 ? std::numeric_limits<long long>::max()
                          : ((1LL << (bit_count - 1)) - 1);
      const string below_min =
          emit_temp_assignment("i64",
                               string("cmp lt i64 ") + exact_result + ", " +
                                   to_string(min_value));
      const string above_max =
          emit_temp_assignment("i64",
                               string("cmp gt i64 ") + exact_result + ", " +
                                   to_string(max_value));
      return emit_temp_assignment("i64",
                                  string("binary or i64 ") + below_min + ", " + above_max);
    }

    const string lhs_slot = new_hidden_slot("i64", "ov_lhs");
    const string rhs_slot = new_hidden_slot("i64", "ov_rhs");
    const string result_slot = new_hidden_slot("i64", "ov_result");
    const string overflow_slot = new_hidden_slot("i64", "ov_flag");
    const string zero_label = new_block("ov_zero");
    const string check_label = new_block("ov_check");
    const string div_label = new_block("ov_div");
    const string overflow_label = new_block("ov_overflow");
    const string no_overflow_label = new_block("ov_no_overflow");
    const string end_label = new_block("ov_end");

    emit_line("store i64 " + lhs + ", " + lhs_slot);
    emit_line("store i64 " + rhs + ", " + rhs_slot);
    emit_line("store i64 " + result + ", " + result_slot);
    const string lhs_zero =
        emit_temp_assignment("i64", string("cmp eq i64 ") + lhs + ", 0");
    const string rhs_zero =
        emit_temp_assignment("i64", string("cmp eq i64 ") + rhs + ", 0");
    const string any_zero =
        emit_temp_assignment("i64", string("binary or i64 ") + lhs_zero + ", " + rhs_zero);
    terminate(string("branch ") + any_zero + ", " + lowir_block_name(zero_label) + ", " +
              lowir_block_name(check_label));

    start_block(zero_label);
    emit_line("store i64 0, " + overflow_slot);
    terminate(string("jump ") + lowir_block_name(end_label));

    start_block(check_label);
    const string current_lhs =
        emit_temp_assignment("i64", string("load i64 ") + lhs_slot);
    const string current_rhs =
        emit_temp_assignment("i64", string("load i64 ") + rhs_slot);

    if(is_unsigned) {
      terminate(string("jump ") + lowir_block_name(div_label));
    } else {
      const string lhs_neg1 =
          emit_temp_assignment("i64", string("cmp eq i64 ") + current_lhs + ", -1");
      const string rhs_neg1 =
          emit_temp_assignment("i64", string("cmp eq i64 ") + current_rhs + ", -1");
      const string lhs_min =
          emit_temp_assignment("i64",
                               string("cmp eq i64 ") + current_lhs + ", " +
                                   to_string(std::numeric_limits<long long>::min()));
      const string rhs_min =
          emit_temp_assignment("i64",
                               string("cmp eq i64 ") + current_rhs + ", " +
                                   to_string(std::numeric_limits<long long>::min()));
      const string lhs_neg1_rhs_min =
          emit_temp_assignment("i64",
                               string("binary and i64 ") + lhs_neg1 + ", " + rhs_min);
      const string rhs_neg1_lhs_min =
          emit_temp_assignment("i64",
                               string("binary and i64 ") + rhs_neg1 + ", " + lhs_min);
      const string special_overflow =
          emit_temp_assignment("i64",
                               string("binary or i64 ") + lhs_neg1_rhs_min + ", " +
                                   rhs_neg1_lhs_min);
      terminate(string("branch ") + special_overflow + ", " +
                lowir_block_name(overflow_label) + ", " + lowir_block_name(div_label));
    }

    start_block(div_label);
    const string div_result =
        emit_temp_assignment("i64", string("load i64 ") + result_slot);
    const string div_rhs =
        emit_temp_assignment("i64", string("load i64 ") + rhs_slot);
    const string div_lhs =
        emit_temp_assignment("i64", string("load i64 ") + lhs_slot);
    const string quotient =
        emit_temp_assignment("i64",
                             string("binary ") + (is_unsigned ? "udiv" : "div") + " i64 " +
                                 div_result + ", " + div_rhs);
    const string mismatch =
        emit_temp_assignment("i64", string("cmp ne i64 ") + quotient + ", " + div_lhs);
    terminate(string("branch ") + mismatch + ", " + lowir_block_name(overflow_label) + ", " +
              lowir_block_name(no_overflow_label));

    start_block(overflow_label);
    emit_line("store i64 1, " + overflow_slot);
    terminate(string("jump ") + lowir_block_name(end_label));

    start_block(no_overflow_label);
    emit_line("store i64 0, " + overflow_slot);
    terminate(string("jump ") + lowir_block_name(end_label));

    start_block(end_label);
    return emit_temp_assignment("i64", string("load i64 ") + overflow_slot);
  }

  string emit_storage_address(const string & storage)
  {
    return emit_temp_assignment("ptr", string("addr ") + storage);
  }

  string emit_normalized_truthy(const CallSemNode & node)
  {
    const string value = emit_rvalue(node);
    const string value_type = lowir_value_type_for(node.semantic_type);
    if(value_type == "f32" || value_type == "f64") {
      return emit_temp_assignment("i64",
                                  string("cmp ne ") + value_type + " " + value + ", " +
                                  zero_literal_for_lowir_type(value_type));
    }
    if(value_type == "ptr") {
      return emit_temp_assignment("i64", string("cmp ne ptr ") + value + ", 0");
    }
    return emit_temp_assignment("i64", string("cmp ne i64 ") + value + ", 0");
  }

  string emit_branch_condition_value(const CallSemNode & node)
  {
    const string value_type = lowir_value_type_for(node.semantic_type);
    if(value_type == "f32" || value_type == "f64") {
      return emit_normalized_truthy(node);
    }
    return emit_rvalue(node);
  }

  void emit_throw_object_to_storage(const TypePtr & throw_type,
                                    const CallSemNode & node,
                                    const string & storage_ptr)
  {
    if(is_complete_class_value_type(throw_type)) {
      if(node.kind == CallSemKind::closure_object ||
         node.kind == CallSemKind::initializer_list_object) {
        emit_special_class_value_to_target(node, storage_ptr);
      } else if(is_indirect_class_reference_type(node.semantic_type)) {
        if(node.value_category == CVC_XVALUE) {
          emit_move_construct_to_target(throw_type, storage_ptr, emit_rvalue(node));
        } else {
          emit_copy_construct_to_target(throw_type, storage_ptr, emit_rvalue(node));
        }
      } else if(node.value_category == CVC_LVALUE) {
        emit_copy_construct_to_target(throw_type, storage_ptr, emit_lvalue_address(node));
      } else if(node.kind == CallSemKind::call_expression &&
                is_indirect_value_type(node.semantic_type)) {
        emit_call_expression_to_target(node, storage_ptr);
      } else {
        throw logic_error("class throw requires materialization context");
      }
    } else {
      emit_line("store " + lowir_memory_type_for(throw_type) + " " +
                emit_scalar_storage_value(throw_type, node) + ", " + storage_ptr);
    }
  }

  string emit_private_throw_value(const CallSemNode & node, string & lowir_type)
  {
    const TypePtr throw_type = exception_object_type(node.semantic_type);
    if(!throw_type) {
      throw logic_error("throw-expression missing type");
    }

    const string storage_ptr =
        emit_temp_assignment("ptr", string("addr ") + exception_storage_symbol(throw_type));
    emit_throw_object_to_storage(throw_type, node, storage_ptr);

    emit_set_current_exception_type(throw_type);
    emit_set_current_exception_storage(storage_ptr);
    lowir_type = "ptr";
    return storage_ptr;
  }

  string emit_host_throw_value(const CallSemNode & node)
  {
    const TypePtr throw_type = exception_object_type(node.semantic_type);
    if(!throw_type) {
      throw logic_error("throw-expression missing type");
    }

    const string allocate_call =
        string("call ptr ") +
        external_runtime_symbol("__cxa_allocate_exception") + "(" +
        to_string(backend_storage_size(throw_type)) + ")";
    string storage_ptr;
    const bool needs_constructor_wrapper =
        is_constructor_function_ &&
        constructor_action_depth_ == 0 &&
        !constructor_unwind_cleanups_.empty();
    if(current_scope_has_host_unwind_cleanups() || needs_constructor_wrapper) {
      const string result_slot = new_hidden_slot("ptr", "throw_alloc");
      const size_t host_dispatch_depth =
          use_host_eh_runtime() ? host_eh_region_depth_ + 1 : 0;
      bool created_dispatch = false;
      const string dispatch_label =
          shared_call_unwind_dispatch_label(needs_constructor_wrapper,
                                            host_dispatch_depth,
                                            created_dispatch);
      const string end_label =
          created_dispatch ? new_block("throw_alloc_unwind_end") : string();
      emit_line("eh_try " + lowir_block_name(dispatch_label));
      if(use_host_eh_runtime()) {
        ++host_eh_region_depth_;
      }
      const string result = emit_temp_assignment("ptr", allocate_call);
      emit_line("store ptr " + result + ", " + result_slot);
      emit_line("eh_end");
      if(use_host_eh_runtime()) {
        if(host_eh_region_depth_ == 0) {
          throw logic_error("host EH region depth underflow");
        }
        --host_eh_region_depth_;
      }
      if(created_dispatch) {
        terminate("jump " + lowir_block_name(end_label));
        emit_shared_call_unwind_dispatch_block(dispatch_label,
                                               needs_constructor_wrapper,
                                               host_dispatch_depth);
        start_block(end_label);
      }
      storage_ptr = emit_temp_assignment("ptr", string("load ptr ") + result_slot);
    } else {
      storage_ptr = emit_temp_assignment("ptr", allocate_call);
    }
    emit_throw_object_to_storage(throw_type, node, storage_ptr);
    return storage_ptr;
  }

  string emit_host_throw_destructor(const TypePtr & throw_type)
  {
    if(!throw_type || !is_complete_class_value_type(throw_type)) {
      return "0";
    }
    const string dtor = destructor_symbol_for_runtime_call(throw_type);
    if(dtor.empty()) {
      return "0";
    }
    return emit_temp_assignment("ptr", string("addr ") + dtor);
  }

  const CallSemNode & array_new_allocation_byte_count_node(const CallSemNode & node) const
  {
    if(node.children.empty() ||
       node.children[0].kind != CallSemKind::call_expression ||
       node.children[0].children.size() < 2) {
      throw logic_error("array new value-init missing allocation size");
    }
    return node.children[0].children[1];
  }

  void emit_array_new_value_initialization(const CallSemNode & node,
                                           const string & object_ptr,
                                           const string & evaluated_byte_count)
  {
    if(!node.value_initializes_result) {
      return;
    }
    TypePtr result_type = strip_top_level_cv(remove_reference_type(node.semantic_type));
    if(!result_type || result_type->kind != Type::TK_POINTER || !result_type->inner) {
      throw logic_error("array new value-init requires pointer result");
    }

    const CallSemNode & byte_count_node = array_new_allocation_byte_count_node(node);
    if(byte_count_node.has_uint_value) {
      const unsigned long long byte_count = callsem_uint_value(byte_count_node);
      if(byte_count != 0) {
        emit_line("zeroinit " + to_string(byte_count) + "x" +
                  to_string(backend_storage_alignment(result_type->inner)) + " " +
                  object_ptr);
      }
      return;
    }
    if(evaluated_byte_count.empty()) {
      throw logic_error("array new value-init dynamic size was not captured");
    }
    emit_dynamic_zero_storage_bytes(object_ptr, evaluated_byte_count);
  }

  string array_new_element_count(const CallSemNode & node,
                                 const string & evaluated_byte_count)
  {
    if(!node.has_uint_value || callsem_uint_value(node) == 0) {
      throw logic_error("class array new-expression missing element size");
    }
    const unsigned long long element_size = callsem_uint_value(node);
    const CallSemNode & byte_count_node = array_new_allocation_byte_count_node(node);
    if(byte_count_node.has_uint_value) {
      return emit_temp_assignment(
          "i64",
          string("const i64 ") +
              to_string(callsem_uint_value(byte_count_node) / element_size));
    }
    if(evaluated_byte_count.empty()) {
      throw logic_error("class array new-expression dynamic size was not captured");
    }
    return emit_temp_assignment("i64",
                                string("binary udiv i64 ") + evaluated_byte_count +
                                    ", " + to_string(element_size));
  }

  void emit_array_new_default_construction(const CallSemNode & node,
                                           const string & object_ptr,
                                           const string & evaluated_byte_count)
  {
    if(!node.has_uint_value || node.children.size() == 1) {
      return;
    }
    if(node.children.size() != 2 || node.children[1].kind != CallSemKind::callee) {
      throw logic_error("class array new-expression constructor shape");
    }

    TypePtr result_type = strip_top_level_cv(remove_reference_type(node.semantic_type));
    if(!result_type || result_type->kind != Type::TK_POINTER || !result_type->inner) {
      throw logic_error("class array new-expression requires pointer result");
    }
    const unsigned long long element_size = callsem_uint_value(node);
    const string element_count = array_new_element_count(node, evaluated_byte_count);
    const string index_slot = new_hidden_slot("i64", "array_new_index");
    const string cond_label = new_block("array_new_ctor_cond");
    const string body_label = new_block("array_new_ctor_body");
    const string end_label = new_block("array_new_ctor_end");

    emit_line("store i64 0, " + index_slot);
    terminate("jump " + lowir_block_name(cond_label));

    start_block(cond_label);
    const string index = emit_temp_assignment("i64", string("load i64 ") + index_slot);
    const string keep_going =
        emit_temp_assignment("i64", string("cmp ult i64 ") + index + ", " + element_count);
    terminate("branch " + keep_going + ", " + lowir_block_name(body_label) + ", " +
              lowir_block_name(end_label));

    start_block(body_label);
    string element_ptr = object_ptr;
    if(element_size != 0) {
      const string byte_offset =
          emit_temp_assignment("i64",
                               string("binary mul i64 ") + index + ", " +
                                   to_string(element_size));
      element_ptr = emit_temp_assignment("ptr",
                                         string("index i8 ") + object_ptr + ", " +
                                             byte_offset);
    }
    emit_line("call void " + lookup_function_symbol(node.children[1]) + "(" +
              element_ptr + ")");
    const string next =
        emit_temp_assignment("i64", string("binary add i64 ") + index + ", 1");
    emit_line("store i64 " + next + ", " + index_slot);
    terminate("jump " + lowir_block_name(cond_label));

    start_block(end_label);
  }

  string emit_call_expression_rvalue(const CallSemNode & node,
                                     const CallArgumentCapture * capture = nullptr)
  {
    TypePtr function_type;
    if(!resolve_callable_function_type(node.children[0].semantic_type, function_type) ||
       !function_type || !function_type->inner) {
      throw logic_error("call-expression missing function type");
    }
    const string call_result = emit_call_expression_raw(node, capture);
    return emit_call_expression_value(node, function_type->inner, call_result);
  }

  bool new_expression_allocation_is_known_nothrow(const CallSemNode & node) const
  {
    if(node.children.empty() ||
       node.children[0].kind != CallSemKind::call_expression ||
       node.children[0].children.empty() ||
       !call_expression_is_known_nothrow(node.children[0])) {
      return false;
    }

    TypePtr function_type;
    if(!resolve_callable_function_type(node.children[0].children[0].semantic_type,
                                       function_type) ||
       !function_type ||
       function_type->kind != Type::TK_FUNCTION) {
      return false;
    }
    for(size_t i = 1; i < function_type->params.size(); ++i) {
      const string param_class =
          class_qualified_name(strip_top_level_cv(remove_reference_type(function_type->params[i])));
      if(param_class == "std::nothrow_t" || param_class == "std::__1::nothrow_t") {
        return true;
      }
    }
    return false;
  }

  bool begin_nothrow_new_initialization(const CallSemNode & node,
                                        const string & object_ptr,
                                        string & end_label)
  {
    if(!new_expression_allocation_is_known_nothrow(node)) {
      return false;
    }
    const string nonnull =
        emit_temp_assignment("i64", string("cmp ne ptr ") + object_ptr + ", 0");
    const string init_label = new_block("new_init");
    end_label = new_block("new_end");
    terminate("branch " + nonnull + ", " + lowir_block_name(init_label) +
              ", " + lowir_block_name(end_label));
    start_block(init_label);
    return true;
  }

  void finish_nothrow_new_initialization(const string & end_label)
  {
    if(end_label.empty()) {
      return;
    }
    if(current_block_) {
      terminate("jump " + lowir_block_name(end_label));
    }
    start_block(end_label);
  }

  string emit_new_expression_value(const CallSemNode & node)
  {
    if(node.children.empty()) {
      throw logic_error("new-expression missing allocation child");
    }

    string captured_array_new_byte_count;
    string object_ptr;
    const bool needs_array_byte_count_capture =
        (node.children.size() == 1 && node.value_initializes_result) ||
        (node.has_uint_value && node.children.size() > 1);
    if(needs_array_byte_count_capture &&
       !array_new_allocation_byte_count_node(node).has_uint_value) {
      const string byte_count_slot = new_hidden_slot("i64", "array_new_size");
      CallArgumentCapture capture;
      capture.index = 0;
      capture.lowir_type = "i64";
      capture.slot = byte_count_slot;
      object_ptr = emit_call_expression_rvalue(node.children[0], &capture);
      captured_array_new_byte_count =
          emit_temp_assignment("i64", string("load i64 ") + byte_count_slot);
    } else {
      object_ptr = emit_rvalue(node.children[0]);
    }
    if(node.has_uint_value) {
      string nothrow_end_label;
      if(node.value_initializes_result || node.children.size() > 1) {
        begin_nothrow_new_initialization(node, object_ptr, nothrow_end_label);
      }
      emit_array_new_value_initialization(node, object_ptr, captured_array_new_byte_count);
      emit_array_new_default_construction(node, object_ptr, captured_array_new_byte_count);
      finish_nothrow_new_initialization(nothrow_end_label);
      return object_ptr;
    }
    if(node.children.size() == 1) {
      string nothrow_end_label;
      if(node.value_initializes_result) {
        begin_nothrow_new_initialization(node, object_ptr, nothrow_end_label);
      }
      emit_array_new_value_initialization(node, object_ptr, captured_array_new_byte_count);
      finish_nothrow_new_initialization(nothrow_end_label);
      return object_ptr;
    }
    if(node.children[1].kind != CallSemKind::callee) {
      TypePtr result_type = strip_top_level_cv(remove_reference_type(node.semantic_type));
      if(!result_type || result_type->kind != Type::TK_POINTER || !result_type->inner) {
        throw logic_error("new-expression scalar initializer requires pointer result");
      }
      string nothrow_end_label;
      begin_nothrow_new_initialization(node, object_ptr, nothrow_end_label);
      emit_line("store " +
                lowir_memory_type_for(result_type->inner) + " " +
                emit_scalar_storage_value(result_type->inner, node.children[1]) + ", " +
                object_ptr);
      finish_nothrow_new_initialization(nothrow_end_label);
      return object_ptr;
    }

    TypePtr function_type = strip_top_level_cv(node.children[1].semantic_type);
    if(!function_type || function_type->kind != Type::TK_FUNCTION) {
      throw logic_error("new-expression constructor callee requires function type");
    }
    TypePtr result_type = strip_top_level_cv(remove_reference_type(node.semantic_type));
    if(!result_type || result_type->kind != Type::TK_POINTER || !result_type->inner) {
      throw logic_error("new-expression constructor result requires pointer type");
    }

    vector<string> args;
    args.push_back(object_ptr);
    for(size_t i = 2; i < node.children.size(); ++i) {
      const size_t param_index = i - 1;
      if(param_index < function_type->params.size()) {
        append_call_argument_values(args, function_type->params[param_index], node.children[i]);
      } else {
        append_variadic_call_argument_value(args, node.children[i]);
      }
    }

    if(node.value_initializes_result && !is_empty_class_storage_type(result_type->inner)) {
      string nothrow_end_label;
      begin_nothrow_new_initialization(node, object_ptr, nothrow_end_label);
      emit_zero_storage_bytes(object_ptr, backend_storage_size(result_type->inner));
      finish_nothrow_new_initialization(nothrow_end_label);
    }

    string nothrow_end_label;
    begin_nothrow_new_initialization(node, object_ptr, nothrow_end_label);
    ostringstream op;
    op << "call void " << lookup_function_symbol(node.children[1]) << "(";
    for(size_t i = 0; i < args.size(); ++i) {
      if(i != 0) {
        op << ", ";
      }
      op << args[i];
    }
    op << ")";
    emit_line(op.str());
    finish_nothrow_new_initialization(nothrow_end_label);
    return object_ptr;
  }

  string emit_rvalue(const CallSemNode & node)
  {
    ScopedLowIRCurrentExpr current_expr(node);

    if(node.kind == CallSemKind::statement_expression) {
      if(node.children.empty() ||
         node.children.size() > 2 ||
         node.children[0].kind != CallSemKind::compound_statement) {
        throw logic_error("statement-expression shape");
      }

      push_cleanup_scope();
      push_binding_scope();
      const CallSemNode & prefix = node.children[0];
      for(size_t i = 0; i < prefix.children.size(); ++i) {
        emit_statement(prefix.children[i]);
      }

      string value;
      if(node.children.size() == 2) {
        if(!current_block_) {
          throw logic_error("statement-expression prefix terminated control flow");
        }
        value = emit_rvalue(node.children[1]);
      } else {
        value = emit_temp_assignment("i32", "const i32 0");
      }

      if(current_block_) {
        emit_scope_cleanups(cleanup_scopes_.back());
      }
      pop_cleanup_scope();
      pop_binding_scope();
      return value;
    }

    if(node.kind == CallSemKind::literal) {
      QuoteLiteralData string_literal;
      if(try_parse_string_literal_node(node, string_literal)) {
        map<string, string>::const_iterator found = string_literal_symbols_.find(node.text);
        if(found == string_literal_symbols_.end()) {
          throw logic_error("missing string literal global");
        }
        return emit_temp_assignment("ptr", string("addr ") + found->second);
      }
      return normalize_literal_token(node);
    }

    if(node.kind == CallSemKind::sizeof_expression) {
      if(!node.has_uint_value) {
        throw logic_error("sizeof-expression missing constant value");
      }
      return emit_temp_assignment("i64",
                                  string("const i64 ") + to_string(callsem_uint_value(node)));
    }

    if(node.kind == CallSemKind::typeid_expression) {
      return emit_typeid_value(node);
    }

    if(node.kind == CallSemKind::dynamic_cast_expression) {
      return emit_dynamic_cast_value(node);
    }

    if(node.kind == CallSemKind::cast_expression) {
      if(node.children.size() != 1) {
        throw logic_error("cast-expression arity");
      }
      if(is_void_type(node.semantic_type)) {
        throw logic_error("void cast-expression has no rvalue");
      }
      return emit_scalar_value_conversion(emit_rvalue(node.children[0]),
                                          node.children[0].semantic_type,
                                          node.semantic_type);
    }

    if(is_complete_class_value_type(node.semantic_type) &&
       is_special_class_materialization_node(node)) {
      const string temp_ptr = new_hidden_object_address(node.semantic_type, "arg");
      emit_special_class_value_to_target(node, temp_ptr);
      register_materialized_temporary_cleanup_live(node.semantic_type, temp_ptr);
      return temp_ptr;
    }

    if(node.kind == CallSemKind::new_expression) {
      return emit_new_expression_value(node);
    }

    const TypePtr indirect_result_object_type =
        node.kind == CallSemKind::call_expression ?
            indirect_call_result_object_type(node) :
            TypePtr();
    if(node.kind == CallSemKind::call_expression &&
       (is_complete_class_value_type(node.semantic_type) ||
        indirect_result_object_type ||
        is_constructor_materialization_call(node))) {
      const TypePtr object_type =
          indirect_result_object_type ?
              indirect_result_object_type :
              remove_reference_type(node.semantic_type);
      const string temp_ptr =
          new_hidden_object_address(object_type, "arg");
      emit_call_expression_to_target(node, temp_ptr);
      register_materialized_temporary_cleanup_live(object_type, temp_ptr);
      return temp_ptr;
    }

    if(node.kind == CallSemKind::id_expression) {
      string out;
      if(try_emit_known_id_rvalue(node, out)) {
        return out;
      }
      ostringstream msg;
      msg << "unknown id-expression " << node.text;
      if(node.semantic_type) {
        msg << " [type " << describe_type(node.semantic_type) << "]";
      }
      msg << " [value_category " << int(node.value_category) << "]";
      if(function_node_) {
        msg << " [function " << function_node_->text << "]";
        if(function_node_->semantic_type) {
          msg << " [function type " << describe_type(function_node_->semantic_type) << "]";
        }
      }
      throw logic_error(msg.str());
    }

    if(node.kind == CallSemKind::variable) {
      const VariableBinding * binding = find_local_binding(node.text);
      if(!binding) {
        throw logic_error("unknown variable node " + node.text);
      }
      if(binding_is_array_storage(*binding)) {
        return emit_decay_pointer(emit_storage_address(binding->slots[0]));
      }
      if(binding_is_indirect_storage(*binding)) {
        return emit_storage_address(binding->slots[0]);
      }
      if(binding_is_decay_view_slot(*binding)) {
        return emit_temp_assignment("ptr", string("load ptr ") + binding->slots[0]);
      }
      const TypePtr loaded_type =
          materialization_source_type_for(node, binding->semantic_type);
      const string value_type = lowir_memory_type_for(loaded_type);
      const string loaded_value =
          emit_temp_assignment(value_type,
                               string("load ") + value_type + " " + binding->slots[0]);
      return emit_loaded_scalar_value(loaded_value, loaded_type, node);
    }

    if(node.kind == CallSemKind::member_expression) {
      if(node.is_bit_field) {
        return emit_bit_field_rvalue(node);
      }
      if(is_reference_type(node.semantic_type)) {
        const string referent_ptr = emit_lvalue_storage(node);
        TypePtr referent_type = remove_reference_type(node.semantic_type);
        TypePtr referent_base = strip_top_level_cv(referent_type);
        if(!referent_base || is_indirect_value_type(referent_type)) {
          return referent_ptr;
        }
        if(is_function_type(referent_base) || referent_base->kind == Type::TK_ARRAY) {
          return emit_decay_pointer(referent_ptr);
        }
        TypePtr loaded_type =
            materialization_source_type_for(node, referent_type);
        const string memory_type = lowir_memory_type_for(loaded_type);
        const string loaded_value =
            emit_temp_assignment(memory_type,
                                string("load ") + memory_type + " " +
                                referent_ptr);
        return emit_loaded_scalar_value(loaded_value, loaded_type, node);
      }
      TypePtr member_base = strip_top_level_cv(node.semantic_type);
      if(is_indirect_value_type(node.semantic_type) ||
         (member_base && member_base->kind == Type::TK_ARRAY)) {
        const string address = emit_lvalue_address(node);
        return member_base && member_base->kind == Type::TK_ARRAY ?
            emit_decay_pointer(address) : address;
      }
      const string storage = emit_lvalue_storage(node);
      const TypePtr loaded_type =
          materialization_source_type_for(node, node.semantic_type);
      const string memory_type = lowir_memory_type_for(loaded_type);
      const string loaded_value =
          emit_temp_assignment(memory_type, string("load ") + memory_type + " " + storage);
      return emit_loaded_scalar_value(loaded_value, loaded_type, node);
    }

    if(node.kind == CallSemKind::binary_expression &&
       node.children.size() == 2 &&
       (callsem_has_token(node, OP_DOTSTAR) || callsem_has_token(node, OP_ARROWSTAR))) {
      TypePtr member_pointer_type =
          strip_top_level_cv(remove_reference_type(node.children[1].semantic_type));
      if(member_pointer_type &&
         member_pointer_type->kind == Type::TK_MEMBER_POINTER &&
         !is_function_type(member_pointer_type->inner)) {
        if(is_reference_type(node.semantic_type)) {
          const string referent_ptr = emit_lvalue_storage(node);
          TypePtr referent_type = remove_reference_type(node.semantic_type);
          TypePtr referent_base = strip_top_level_cv(referent_type);
          if(!referent_base || is_indirect_value_type(referent_type)) {
            return referent_ptr;
          }
          if(is_function_type(referent_base) || referent_base->kind == Type::TK_ARRAY) {
            return emit_decay_pointer(referent_ptr);
          }
          TypePtr loaded_type =
              materialization_source_type_for(node, referent_type);
          const string memory_type = lowir_memory_type_for(loaded_type);
          const string loaded_value =
              emit_temp_assignment(memory_type,
                                  string("load ") + memory_type + " " +
                                  referent_ptr);
          return emit_loaded_scalar_value(loaded_value, loaded_type, node);
        }
        TypePtr member_base = strip_top_level_cv(node.semantic_type);
        if(is_indirect_value_type(node.semantic_type) ||
           (member_base && member_base->kind == Type::TK_ARRAY)) {
          const string address = emit_lvalue_address(node);
          return member_base && member_base->kind == Type::TK_ARRAY ?
              emit_decay_pointer(address) : address;
        }
        const string storage = emit_lvalue_storage(node);
        const TypePtr loaded_type =
            materialization_source_type_for(node, node.semantic_type);
        const string memory_type = lowir_memory_type_for(loaded_type);
        const string loaded_value =
            emit_temp_assignment(memory_type, string("load ") + memory_type + " " + storage);
        return emit_loaded_scalar_value(loaded_value, loaded_type, node);
      }
    }

    if(node.kind == CallSemKind::assignment_expression) {
      if(node.children.size() != 2) {
        throw logic_error("assignment-expression arity");
      }
      if(callsem_has_token(node, OP_ASS)) {
        if(node.children[0].is_reference_storage_target) {
          TypePtr target_memory_type = strip_top_level_cv(node.children[0].semantic_type);
          TypePtr referent_type =
              target_memory_type && target_memory_type->kind == Type::TK_POINTER ?
                  target_memory_type->inner :
                  TypePtr();
          const CallSemNode * source = &node.children[1];
          if(source->kind == CallSemKind::unary_expression &&
             callsem_has_token(*source, OP_AMP) &&
             source->children.size() == 1) {
            source = &source->children[0];
          }
          const string rebound =
              emit_reference_storage_value(referent_type, *source);
          const string target = emit_lvalue_storage(node.children[0]);
          emit_line("store ptr " + rebound + ", " + target);
          return rebound;
        }
        TypePtr lhs_type = remove_reference_type(node.children[0].semantic_type);
        TypePtr lhs_base = strip_top_level_cv(lhs_type);
        if(lhs_base && lhs_base->kind == Type::TK_ARRAY) {
          const string target_ptr = emit_lvalue_address(node.children[0]);
          emit_storage_value_to_target(lhs_type, node.children[1], target_ptr);
          return target_ptr;
        }
        const string rhs = emit_scalar_storage_value(lhs_type, node.children[1]);
        if(is_bit_field_member_expression(node.children[0])) {
          emit_store_to_bit_field(node.children[0], rhs);
          return rhs;
        }
        const string target = emit_lvalue_storage(node.children[0]);
        const string memory_type = lowir_lvalue_memory_type(node.children[0]);
        const string debug_name = direct_local_debug_name(node.children[0]);
        const string stored_value =
            debug_name.empty() ? rhs
                               : emit_debug_named_local_value(debug_name, memory_type, rhs);
        emit_line("store " + memory_type + " " + stored_value + ", " + target);
        return stored_value;
      }

      const string memory_type = lowir_lvalue_memory_type(node.children[0]);
      const string old_value = is_bit_field_member_expression(node.children[0]) ?
          emit_bit_field_rvalue(node.children[0]) :
          emit_temp_assignment(memory_type,
                               string("load ") + memory_type + " " +
                               emit_lvalue_storage(node.children[0]));
      const string rhs = emit_rvalue(node.children[1]);
      const TypePtr lhs_value_type =
          lowir_value_conversion_type(node.children[0].semantic_type);
      const TypePtr rhs_value_type =
          lowir_value_conversion_type(node.children[1].semantic_type);
      const bool lhs_pointer = lhs_value_type && lhs_value_type->kind == Type::TK_POINTER;
      const bool rhs_integer_like =
          rhs_value_type &&
          (is_integral_type(rhs_value_type) || is_named_enum_scalar_type(rhs_value_type));
      const string type = lowir_type_for(node.semantic_type);
      const bool unsigned_integral =
          is_lowir_unsigned_integral_scalar_type(
              lowir_value_conversion_type(node.semantic_type));
      string next_value;
      if(callsem_has_token(node, OP_PLUSASS) && lhs_pointer && rhs_integer_like) {
        next_value =
            emit_pointer_index(old_value,
                               rhs,
                               node.children[1].semantic_type,
                               node.children[0].semantic_type);
      } else if(callsem_has_token(node, OP_MINUSASS) && lhs_pointer && rhs_integer_like) {
        next_value =
            emit_pointer_index_negated(old_value,
                                       rhs,
                                       node.children[1].semantic_type,
                                       node.children[0].semantic_type);
      } else {
        string op;
        if(callsem_has_token(node, OP_PLUSASS)) {
          op = "add";
        } else if(callsem_has_token(node, OP_MINUSASS)) {
          op = "sub";
        } else if(callsem_has_token(node, OP_STARASS)) {
          op = "mul";
        } else if(callsem_has_token(node, OP_DIVASS)) {
          op = unsigned_integral ? "udiv" : "div";
        } else if(callsem_has_token(node, OP_MODASS)) {
          op = unsigned_integral ? "umod" : "mod";
        } else if(callsem_has_token(node, OP_XORASS)) {
          op = "xor";
        } else if(callsem_has_token(node, OP_BANDASS)) {
          op = "and";
        } else if(callsem_has_token(node, OP_BORASS)) {
          op = "or";
        } else if(callsem_has_token(node, OP_LSHIFTASS)) {
          op = "shl";
        } else if(callsem_has_token(node, OP_RSHIFTASS)) {
          op = unsigned_integral ? "ushr" : "shr";
        } else {
          throw logic_error("unsupported assignment-expression");
        }
        next_value =
            emit_temp_assignment(type, string("binary ") + op + " " + type + " " +
                                           old_value + ", " + rhs);
      }
      if(is_bit_field_member_expression(node.children[0])) {
        emit_store_to_bit_field(node.children[0], next_value);
      } else {
        const string target = emit_lvalue_storage(node.children[0]);
        const string debug_name = direct_local_debug_name(node.children[0]);
        const string stored_value =
            debug_name.empty() ? next_value
                               : emit_debug_named_local_value(debug_name,
                                                              memory_type,
                                                              next_value);
        emit_line("store " + memory_type + " " + stored_value + ", " + target);
        return stored_value;
      }
      return next_value;
    }

    if(node.kind == CallSemKind::unary_expression) {
      if(node.children.size() != 1) {
        throw logic_error("unary-expression arity");
      }
      TypePtr unary_type = strip_top_level_cv(node.semantic_type);
      TypePtr member_pointer_source_type =
          strip_top_level_cv(remove_reference_type(
              callsem_materialization_source_type(node) ?
                  callsem_materialization_source_type(node) :
                  node.semantic_type));
      if(callsem_has_token(node, OP_AMP) &&
         member_pointer_source_type &&
         member_pointer_source_type->kind == Type::TK_MEMBER_POINTER) {
        if(is_function_type(member_pointer_source_type->inner)) {
          string symbol;
          if(node.is_virtual_dispatch) {
            const string thunk_symbol = register_virtual_member_pointer_thunk(node);
            referenced_function_symbols_.insert(thunk_symbol);
            symbol = thunk_symbol;
          } else {
            symbol = callsem_symbol(node).internal_symbol;
            if(symbol.empty() && !node.children.empty()) {
              symbol = lookup_function_symbol(node.children[0]);
            }
          }
          if(symbol.empty()) {
            throw logic_error("member-function pointer constant missing symbol");
          }
          if(unary_type &&
             unary_type->kind == Type::TK_POINTER &&
             unary_type->inner &&
             is_function_type(strip_top_level_cv(unary_type->inner))) {
            return emit_temp_assignment("ptr", string("addr ") + symbol);
          }
          const string function_ptr =
              emit_temp_assignment("ptr", string("addr ") + symbol);
          const string function_bits =
              emit_temp_assignment("i64", string("copy i64 ") + function_ptr);
          return emit_temp_assignment("i128",
                                      string("convert zext i128 i64 ") + function_bits);
        }
        if(!node.has_uint_value) {
          throw logic_error("member-object pointer constant missing offset");
        }
        return emit_temp_assignment("i64",
                                    string("const i64 ") +
                                        encode_data_member_pointer_offset(
                                            callsem_uint_value(node)));
      }
      TypePtr gnu_complex_component;
      if((node.text == "__real" || node.text == "__real__" ||
          node.text == "__imag" || node.text == "__imag__") &&
         is_gnu_complex_value_type(node.children[0].semantic_type, &gnu_complex_component)) {
        const string complex_ptr = emit_rvalue(node.children[0]);
        const size_t component_offset =
            (node.text == "__imag" || node.text == "__imag__")
                ? type_size(gnu_complex_component)
                : 0;
        string component_ptr = complex_ptr;
        if(component_offset != 0) {
          component_ptr =
              emit_temp_assignment("ptr",
                                   string("index i8 ") + complex_ptr + ", " +
                                   to_string(component_offset));
        }
        const string component_memory_type = lowir_memory_type_for(gnu_complex_component);
        return emit_temp_assignment(component_memory_type,
                                    string("load ") + component_memory_type + " " +
                                    component_ptr);
      }
      if(callsem_has_token(node, OP_AMP)) {
        return emit_lvalue_address(node.children[0]);
      }
      if(callsem_has_token(node, OP_STAR)) {
        const string pointer = emit_rvalue(node.children[0]);
        if(is_reference_type(node.semantic_type)) {
          return pointer;
        }
        if(node.semantic_type && !is_function_type(node.semantic_type) &&
           !is_array_type(node.semantic_type) &&
           !is_indirect_value_type(node.semantic_type)) {
          const string memory_type =
              lowir_memory_type_for(remove_reference_type(node.semantic_type));
          return emit_temp_assignment(memory_type,
                                      string("load ") + memory_type +
                                      " " + pointer);
        }
        return pointer;
      }
      if(callsem_has_token(node, OP_INC) || callsem_has_token(node, OP_DEC)) {
        const string memory_type = lowir_lvalue_memory_type(node.children[0]);
        const string old_value = is_bit_field_member_expression(node.children[0]) ?
            emit_bit_field_rvalue(node.children[0]) :
            emit_temp_assignment(memory_type,
                                 string("load ") + memory_type + " " +
                                 emit_lvalue_storage(node.children[0]));
        const string next_value =
            emit_incdec_next_value(node, memory_type, old_value);
        if(is_bit_field_member_expression(node.children[0])) {
          emit_store_to_bit_field(node.children[0], next_value);
        } else {
          const string target = emit_lvalue_storage(node.children[0]);
          emit_line("store " + memory_type + " " + next_value +
                    ", " + target);
        }
        return next_value;
      }

      const string child = emit_rvalue(node.children[0]);
      if(callsem_has_token(node, OP_PLUS)) {
        return child;
      }
      if(callsem_has_token(node, OP_MINUS)) {
        return emit_temp_assignment(lowir_type_for(node.semantic_type),
                                    string("unary neg ") + lowir_type_for(node.semantic_type) +
                                    " " + child);
      }
      if(callsem_has_token(node, OP_LNOT)) {
        const string child_type = lowir_value_type_for(node.children[0].semantic_type);
        if(child_type == "f32" || child_type == "f64") {
          return emit_temp_assignment("i64",
                                      string("cmp eq ") + child_type + " " + child + ", " +
                                      zero_literal_for_lowir_type(child_type));
        }
        if(child_type == "ptr") {
          return emit_temp_assignment("i64", string("cmp eq ptr ") + child + ", 0");
        }
        return emit_temp_assignment("i64", string("cmp eq i64 ") + child + ", 0");
      }
      if(callsem_has_token(node, OP_COMPL)) {
        return emit_temp_assignment(lowir_type_for(node.semantic_type),
                                    string("unary bitnot ") + lowir_type_for(node.semantic_type) +
                                    " " + child);
      }
      throw logic_error("unsupported unary operator");
    }

    if(node.kind == CallSemKind::postfix_expression) {
      if(node.children.size() != 1) {
        throw logic_error("postfix-expression arity");
      }
      const string memory_type = lowir_lvalue_memory_type(node.children[0]);
      const string old_value = is_bit_field_member_expression(node.children[0]) ?
          emit_bit_field_rvalue(node.children[0]) :
          emit_temp_assignment(memory_type,
                               string("load ") + memory_type + " " +
                               emit_lvalue_storage(node.children[0]));
      const string next_value =
          emit_incdec_next_value(node, memory_type, old_value);
      if(is_bit_field_member_expression(node.children[0])) {
        emit_store_to_bit_field(node.children[0], next_value);
      } else {
        const string target = emit_lvalue_storage(node.children[0]);
        emit_line("store " + memory_type + " " + next_value +
                  ", " + target);
      }
      return old_value;
    }

    if(node.kind == CallSemKind::binary_expression) {
      if(node.children.size() != 2) {
        throw logic_error("binary-expression arity");
      }
      const TypePtr lhs_value_type = lowir_value_conversion_type(node.children[0].semantic_type);
      const TypePtr rhs_value_type = lowir_value_conversion_type(node.children[1].semantic_type);
      const bool lhs_pointer = lhs_value_type && lhs_value_type->kind == Type::TK_POINTER;
      const bool rhs_pointer = rhs_value_type && rhs_value_type->kind == Type::TK_POINTER;
      const auto is_member_pointer_value_type =
          [](const TypePtr & type) -> bool
      {
        TypePtr base = strip_top_level_cv(remove_reference_type(type));
        return base && base->kind == Type::TK_MEMBER_POINTER;
      };
      const auto is_integral_zero_literal =
          [](const CallSemNode & child) -> bool
      {
        TypePtr base = strip_top_level_cv(remove_reference_type(child.semantic_type));
        if(!base || !is_integral_type(base)) {
          return false;
        }
        long long value = 0;
        return try_get_integral_literal_value(child, value) && value == 0;
      };
      const bool lhs_member_pointer = is_member_pointer_value_type(lhs_value_type);
      const bool rhs_member_pointer = is_member_pointer_value_type(rhs_value_type);
      const bool pointer_like_equality =
          (callsem_has_token(node, OP_EQ) || callsem_has_token(node, OP_NE)) &&
          semantic_conversion::pointer_equality_operands_compatible(lhs_value_type,
                                                                    rhs_value_type);
      const bool member_pointer_null_equality =
          (callsem_has_token(node, OP_EQ) || callsem_has_token(node, OP_NE)) &&
          ((lhs_member_pointer && is_integral_zero_literal(node.children[1])) ||
           (rhs_member_pointer && is_integral_zero_literal(node.children[0])));
      const bool lhs_integer_like =
          lhs_value_type &&
          (is_integral_type(lhs_value_type) || is_named_enum_scalar_type(lhs_value_type));
      const bool rhs_integer_like =
          rhs_value_type &&
          (is_integral_type(rhs_value_type) || is_named_enum_scalar_type(rhs_value_type));
      if(callsem_has_token(node, OP_COMMA)) {
        emit_discarded_expression(node.children[0]);
        return emit_rvalue(node.children[1]);
      }
      if(callsem_has_token(node, OP_LAND) || callsem_has_token(node, OP_LOR)) {
        const bool is_land = callsem_has_token(node, OP_LAND);
        const string result_slot = new_hidden_slot("i64", is_land ? "land" : "lor");
        const string rhs_label = new_block(is_land ? "land_rhs" : "lor_rhs");
        const string short_label = new_block(is_land ? "land_short" : "lor_short");
        const string end_label = new_block(is_land ? "land_end" : "lor_end");
        const string lhs = emit_branch_condition_value(node.children[0]);
        terminate(string("branch ") + lhs + ", " +
                  lowir_block_name(is_land ? rhs_label : short_label) + ", " +
                lowir_block_name(is_land ? short_label : rhs_label));

        start_block(rhs_label);
        push_cleanup_scope(true);
        const string rhs = emit_normalized_truthy(node.children[1]);
        emit_line("store i64 " + rhs + ", " + result_slot);
        if(current_block_) {
          emit_scope_cleanups(cleanup_scopes_.back());
        }
        pop_cleanup_scope();
        terminate(string("jump ") + lowir_block_name(end_label));

        start_block(short_label);
        emit_line(string("store i64 ") + (is_land ? "0" : "1") + ", " + result_slot);
        terminate(string("jump ") + lowir_block_name(end_label));

        start_block(end_label);
        return emit_temp_assignment("i64", string("load i64 ") + result_slot);
      }

      const string lhs_raw = emit_rvalue(node.children[0]);
      const string rhs_raw = emit_rvalue(node.children[1]);
      const bool typeid_equality =
          (callsem_has_token(node, OP_EQ) || callsem_has_token(node, OP_NE)) &&
          node.children[0].kind == CallSemKind::typeid_expression &&
          node.children[1].kind == CallSemKind::typeid_expression;
      const auto emit_converted_operand =
          [&](const string & value, size_t child_index, const TypePtr & target_type) -> string
      {
        if(!target_type) {
          return value;
        }
        return emit_scalar_value_conversion(value,
                                            node.children[child_index].semantic_type,
                                            target_type);
      };
      const auto raw_numeric_operand_type = [&]() -> TypePtr
      {
        if(callsem_has_token(node, OP_STAR) || callsem_has_token(node, OP_DIV)) {
          return semantic_conversion::common_arithmetic_result_type(lhs_value_type,
                                                                    rhs_value_type);
        }
        if(callsem_has_token(node, OP_MOD) ||
           callsem_has_token(node, OP_BOR) ||
           callsem_has_token(node, OP_XOR) ||
           callsem_has_token(node, OP_AMP)) {
          return semantic_conversion::common_integral_result_type(lhs_value_type,
                                                                  rhs_value_type);
        }
        if(callsem_has_token(node, OP_LSHIFT) || callsem_has_token(node, OP_RSHIFT)) {
          return semantic_conversion::promoted_integral_result_type(lhs_value_type);
        }
        if((callsem_has_token(node, OP_PLUS) || callsem_has_token(node, OP_MINUS)) &&
           !lhs_pointer && !rhs_pointer) {
          return semantic_conversion::common_arithmetic_result_type(lhs_value_type,
                                                                    rhs_value_type);
        }
        if((callsem_has_token(node, OP_EQ) || callsem_has_token(node, OP_NE) ||
            callsem_has_token(node, OP_LT) || callsem_has_token(node, OP_GT) ||
            callsem_has_token(node, OP_LE) || callsem_has_token(node, OP_GE)) &&
           !lhs_pointer && !rhs_pointer && !pointer_like_equality &&
           !member_pointer_null_equality) {
          return semantic_conversion::common_arithmetic_result_type(lhs_value_type,
                                                                    rhs_value_type);
        }
        return TypePtr();
      };
      const TypePtr operand_type = raw_numeric_operand_type();
      const string lhs = emit_converted_operand(lhs_raw, 0, operand_type);
      const string rhs = emit_converted_operand(rhs_raw, 1, operand_type);
      const string type =
          typeid_equality ? string() :
                            (operand_type ? lowir_type_for(operand_type) :
                                            lowir_value_type_for(node.children[0].semantic_type));
      const bool unsigned_integral_op =
          is_lowir_unsigned_integral_scalar_type(operand_type);
      const auto emit_converted_binary_result =
          [&](const string & raw_value, const TypePtr & raw_result_type) -> string
      {
        TypePtr node_value_type = lowir_value_conversion_type(node.semantic_type);
        if(!raw_result_type || !node_value_type ||
           type_equals(raw_result_type, node_value_type) ||
           is_pointer_type(raw_result_type) || is_pointer_type(node_value_type)) {
          return raw_value;
        }
        return emit_scalar_value_conversion(raw_value, raw_result_type, node.semantic_type);
      };
      const auto emit_raw_binary =
          [&](const string & opcode, const TypePtr & raw_result_type) -> string
      {
        const string raw_type = lowir_type_for(raw_result_type);
        const string raw_value =
            emit_temp_assignment(raw_type,
                                string("binary ") + opcode + " " + raw_type + " " + lhs + ", " +
                                rhs);
        return emit_converted_binary_result(raw_value, raw_result_type);
      };

      if(callsem_has_token(node, OP_PLUS)) {
        if(lhs_pointer && rhs_integer_like) {
          return emit_pointer_index(lhs, rhs, node.children[1].semantic_type, lhs_value_type);
        }
        if(lhs_integer_like && rhs_pointer) {
          return emit_pointer_index(rhs, lhs, node.children[0].semantic_type, rhs_value_type);
        }
        const TypePtr raw_result_type =
            operand_type ? operand_type :
                           semantic_conversion::common_arithmetic_result_type(lhs_value_type,
                                                                             rhs_value_type);
        return emit_raw_binary("add", raw_result_type);
      }
      if(callsem_has_token(node, OP_MINUS)) {
        if(lhs_pointer && rhs_pointer) {
          return emit_pointer_difference_elements(lhs, rhs, lhs_value_type);
        }
        if(lhs_pointer && rhs_integer_like) {
          return emit_pointer_index_negated(lhs,
                                            rhs,
                                            node.children[1].semantic_type,
                                            lhs_value_type);
        }
        const TypePtr raw_result_type =
            operand_type ? operand_type :
                           semantic_conversion::common_arithmetic_result_type(lhs_value_type,
                                                                             rhs_value_type);
        return emit_raw_binary("sub", raw_result_type);
      }
      if(callsem_has_token(node, OP_STAR)) {
        return emit_raw_binary("mul", operand_type);
      }
      if(callsem_has_token(node, OP_DIV)) {
        return emit_raw_binary(unsigned_integral_op ? "udiv" : "div", operand_type);
      }
      if(callsem_has_token(node, OP_MOD)) {
        return emit_raw_binary(unsigned_integral_op ? "umod" : "mod", operand_type);
      }
      if(callsem_has_token(node, OP_BOR)) {
        return emit_raw_binary("or", operand_type);
      }
      if(callsem_has_token(node, OP_XOR)) {
        return emit_raw_binary("xor", operand_type);
      }
      if(callsem_has_token(node, OP_AMP)) {
        return emit_raw_binary("and", operand_type);
      }
      if(callsem_has_token(node, OP_LSHIFT)) {
        return emit_raw_binary("shl", operand_type);
      }
      if(callsem_has_token(node, OP_RSHIFT)) {
        return emit_raw_binary(unsigned_integral_op ? "ushr" : "shr", operand_type);
      }
      if(callsem_has_token(node, OP_LT)) {
        if(lhs_pointer || rhs_pointer) {
          return emit_temp_assignment("i64", string("cmp lt ptr ") + lhs + ", " + rhs);
        }
        return emit_temp_assignment("i64",
                                    string("cmp ") +
                                    (unsigned_integral_op ? "ult " : "lt ") +
                                    type + " " + lhs + ", " + rhs);
      }
      if(callsem_has_token(node, OP_GT)) {
        if(lhs_pointer || rhs_pointer) {
          return emit_temp_assignment("i64", string("cmp gt ptr ") + lhs + ", " + rhs);
        }
        return emit_temp_assignment("i64",
                                    string("cmp ") +
                                    (unsigned_integral_op ? "ugt " : "gt ") +
                                    type + " " + lhs + ", " + rhs);
      }
      if(callsem_has_token(node, OP_LE)) {
        if(lhs_pointer || rhs_pointer) {
          return emit_temp_assignment("i64", string("cmp le ptr ") + lhs + ", " + rhs);
        }
        return emit_temp_assignment("i64",
                                    string("cmp ") +
                                    (unsigned_integral_op ? "ule " : "le ") +
                                    type + " " + lhs + ", " + rhs);
      }
      if(callsem_has_token(node, OP_GE)) {
        if(lhs_pointer || rhs_pointer) {
          return emit_temp_assignment("i64", string("cmp ge ptr ") + lhs + ", " + rhs);
        }
        return emit_temp_assignment("i64",
                                    string("cmp ") +
                                    (unsigned_integral_op ? "uge " : "ge ") +
                                    type + " " + lhs + ", " + rhs);
      }
      if(callsem_has_token(node, OP_EQ)) {
        if(typeid_equality) {
          return emit_temp_assignment("i64", string("cmp eq ptr ") + lhs_raw + ", " + rhs_raw);
        }
        if(member_pointer_null_equality) {
          const TypePtr compare_value_type =
              lhs_member_pointer ? lhs_value_type : rhs_value_type;
          const string compare_type = lowir_type_for(compare_value_type);
          const string lhs_compare =
              lhs_member_pointer ? lhs_raw : zero_literal_for_lowir_type(compare_type);
          const string rhs_compare =
              rhs_member_pointer ? rhs_raw : zero_literal_for_lowir_type(compare_type);
          return emit_temp_assignment("i64",
                                      string("cmp eq ") + compare_type + " " +
                                          lhs_compare + ", " + rhs_compare);
        }
        if(pointer_like_equality) {
          const string lhs_lowir_type = lowir_value_type_for(node.children[0].semantic_type);
          const string rhs_lowir_type = lowir_value_type_for(node.children[1].semantic_type);
          if(lhs_lowir_type == "ptr" || rhs_lowir_type == "ptr") {
            const TypePtr pointer_compare_type = lhs_lowir_type == "ptr" ? lhs_value_type :
                                                 rhs_value_type;
            const string lhs_ptr = emit_converted_operand(lhs_raw, 0, pointer_compare_type);
            const string rhs_ptr = emit_converted_operand(rhs_raw, 1, pointer_compare_type);
            return emit_temp_assignment("i64", string("cmp eq ptr ") + lhs_ptr + ", " + rhs_ptr);
          }
          const string compare_type =
              lhs_lowir_type == "i128" || rhs_lowir_type == "i128" ? "i128" : "i64";
          return emit_temp_assignment("i64",
                                      string("cmp eq ") + compare_type + " " +
                                          lhs_raw + ", " + rhs_raw);
        }
        if(lhs_pointer || rhs_pointer) {
          return emit_temp_assignment("i64", string("cmp eq ptr ") + lhs + ", " + rhs);
        }
        return emit_temp_assignment("i64", string("cmp eq ") + type + " " + lhs + ", " + rhs);
      }
      if(callsem_has_token(node, OP_NE)) {
        if(typeid_equality) {
          return emit_temp_assignment("i64", string("cmp ne ptr ") + lhs_raw + ", " + rhs_raw);
        }
        if(member_pointer_null_equality) {
          const TypePtr compare_value_type =
              lhs_member_pointer ? lhs_value_type : rhs_value_type;
          const string compare_type = lowir_type_for(compare_value_type);
          const string lhs_compare =
              lhs_member_pointer ? lhs_raw : zero_literal_for_lowir_type(compare_type);
          const string rhs_compare =
              rhs_member_pointer ? rhs_raw : zero_literal_for_lowir_type(compare_type);
          return emit_temp_assignment("i64",
                                      string("cmp ne ") + compare_type + " " +
                                          lhs_compare + ", " + rhs_compare);
        }
        if(pointer_like_equality) {
          const string lhs_lowir_type = lowir_value_type_for(node.children[0].semantic_type);
          const string rhs_lowir_type = lowir_value_type_for(node.children[1].semantic_type);
          if(lhs_lowir_type == "ptr" || rhs_lowir_type == "ptr") {
            const TypePtr pointer_compare_type = lhs_lowir_type == "ptr" ? lhs_value_type :
                                                 rhs_value_type;
            const string lhs_ptr = emit_converted_operand(lhs_raw, 0, pointer_compare_type);
            const string rhs_ptr = emit_converted_operand(rhs_raw, 1, pointer_compare_type);
            return emit_temp_assignment("i64", string("cmp ne ptr ") + lhs_ptr + ", " + rhs_ptr);
          }
          const string compare_type =
              lhs_lowir_type == "i128" || rhs_lowir_type == "i128" ? "i128" : "i64";
          return emit_temp_assignment("i64",
                                      string("cmp ne ") + compare_type + " " +
                                          lhs_raw + ", " + rhs_raw);
        }
        if(lhs_pointer || rhs_pointer) {
          return emit_temp_assignment("i64", string("cmp ne ptr ") + lhs + ", " + rhs);
        }
        return emit_temp_assignment("i64", string("cmp ne ") + type + " " + lhs + ", " + rhs);
      }
      throw logic_error("unsupported binary operator");
    }

    if(node.kind == CallSemKind::conditional_expression) {
      if(node.children.size() != 3) {
        throw logic_error("conditional-expression arity");
      }
      if(node.value_category == CVC_LVALUE &&
         is_complete_class_value_type(node.semantic_type)) {
        return emit_lvalue_address(node);
      }
      if(is_complete_class_value_type(node.semantic_type)) {
        const string temp_ptr = new_hidden_object_address(node.semantic_type, "condobj");
        emit_special_class_value_to_target(node, temp_ptr);
        return temp_ptr;
      }
      TypePtr cond_type = strip_top_level_cv(node.semantic_type);
      if(node.value_category == CVC_LVALUE &&
         cond_type &&
         (cond_type->kind == Type::TK_ARRAY || is_function_type(cond_type))) {
        return emit_lvalue_address(node);
      }
      const string result_slot = new_hidden_slot(lowir_type_for(node.semantic_type), "cond");
      const string then_label = new_block("cond_then");
      const string else_label = new_block("cond_else");
      const string end_label = new_block("cond_end");
      const string cond = emit_branch_condition_value(node.children[0]);
      terminate(string("branch ") + cond + ", " + lowir_block_name(then_label) + ", " +
                lowir_block_name(else_label));

      const auto emit_scalar_branch = [&](const CallSemNode & branch)
      {
        push_cleanup_scope();
        const string value = emit_scalar_storage_value(node.semantic_type, branch);
        emit_line("store " + lowir_type_for(node.semantic_type) + " " + value + ", " +
                  result_slot);
        if(current_block_) {
          emit_scope_cleanups(cleanup_scopes_.back());
        }
        pop_cleanup_scope();
      };

      bool then_fallthrough = false;
      start_block(then_label);
      emit_scalar_branch(node.children[1]);
      if(current_block_) {
        terminate(string("jump ") + lowir_block_name(end_label));
        then_fallthrough = true;
      }

      bool else_fallthrough = false;
      start_block(else_label);
      emit_scalar_branch(node.children[2]);
      if(current_block_) {
        terminate(string("jump ") + lowir_block_name(end_label));
        else_fallthrough = true;
      }

      if(then_fallthrough || else_fallthrough) {
        start_block(end_label);
      } else {
        current_block_ = nullptr;
      }
      return emit_temp_assignment(lowir_type_for(node.semantic_type),
                                  string("load ") + lowir_type_for(node.semantic_type) + " " +
                                  result_slot);
    }

    if(node.kind == CallSemKind::subscript_expression) {
      if(is_reference_type(node.semantic_type)) {
        const string referent_ptr = emit_lvalue_storage(node);
        TypePtr referent_type = remove_reference_type(node.semantic_type);
        TypePtr referent_base = strip_top_level_cv(referent_type);
        if(!referent_base || is_indirect_value_type(referent_type) ||
           is_function_type(referent_base) ||
           referent_base->kind == Type::TK_ARRAY) {
          return referent_ptr;
        }
        TypePtr loaded_type =
            materialization_source_type_for(node, referent_type);
        const string memory_type = lowir_memory_type_for(loaded_type);
        const string loaded_value =
            emit_temp_assignment(memory_type,
                                string("load ") + memory_type + " " +
                                referent_ptr);
        return emit_loaded_scalar_value(loaded_value, loaded_type, node);
      }
      TypePtr element_type = strip_top_level_cv(node.semantic_type);
      if(is_indirect_value_type(node.semantic_type) ||
         (element_type && element_type->kind == Type::TK_ARRAY)) {
        return emit_lvalue_address(node);
      }
      const string storage = emit_lvalue_storage(node);
      const TypePtr loaded_type =
          materialization_source_type_for(node, node.semantic_type);
      const string memory_type = lowir_memory_type_for(loaded_type);
      const string loaded_value =
          emit_temp_assignment(memory_type, string("load ") + memory_type + " " + storage);
      return emit_loaded_scalar_value(loaded_value, loaded_type, node);
    }

    if(node.kind == CallSemKind::call_expression) {
      if(node.children.empty()) {
        throw logic_error("call-expression missing callee");
      }
      if(is_marked_scalar_delete_expression(node)) {
        return emit_marked_scalar_delete_expression(node);
      }
      if(is_indirect_value_type(node.semantic_type)) {
        return emit_lvalue_address(node);
      }
      if(node.children[0].kind == CallSemKind::callee) {
        const string & builtin_name = node.children[0].text;
        const auto atomic_order_value =
            [&](size_t child_index) -> long long
            {
              long long order = 5;
              if(child_index < node.children.size()) {
                long long parsed = 0;
                if(try_get_integral_literal_value(node.children[child_index], parsed)) {
                  order = parsed;
                }
              }
              return order;
            };
        const auto atomic_value_type_for_pointer =
            [&](size_t child_index,
                const string & diagnostic_name) -> string
            {
              TypePtr ptr_type =
                  strip_top_level_cv(remove_reference_type(node.children[child_index].semantic_type));
              if(!ptr_type || ptr_type->kind != Type::TK_POINTER) {
                throw logic_error(diagnostic_name + " requires pointer argument");
              }
              TypePtr value_type = strip_top_level_cv(ptr_type->inner);
              if(value_type && value_type->kind == Type::TK_ATOMIC) {
                value_type = strip_top_level_cv(value_type->inner);
              }
              if(!value_type) {
                throw logic_error(diagnostic_name + " requires pointed value type");
              }
              return lowir_type_for(value_type);
            };
        if(builtin_name == "__atomic_load_n") {
          if(node.children.size() != 3) {
            throw logic_error("__atomic_load_n child count");
          }
          const string ptr = emit_rvalue(node.children[1]);
          const long long order = atomic_order_value(2);
          return emit_temp_assignment(lowir_type_for(node.semantic_type),
                                      string("atomic_load ") +
                                      lowir_type_for(node.semantic_type) + " " +
                                      ptr + ", " + to_string(order));
        }
        if(builtin_name == "__c11_atomic_init") {
          if(node.children.size() != 3) {
            throw logic_error("__c11_atomic_init child count");
          }
          const string value_type =
              atomic_value_type_for_pointer(1, "__c11_atomic_init");
          const string dst = emit_rvalue(node.children[1]);
          const string src = emit_rvalue(node.children[2]);
          emit_line(string("store ") + value_type + " " + src + ", " + dst);
          return "0";
        }
        if(builtin_name == "__c11_atomic_load") {
          if(node.children.size() != 3) {
            throw logic_error("__c11_atomic_load child count");
          }
          const string value_type =
              atomic_value_type_for_pointer(1, "__c11_atomic_load");
          const string ptr = emit_rvalue(node.children[1]);
          const long long order = atomic_order_value(2);
          return emit_temp_assignment(value_type,
                                      string("atomic_load ") + value_type + " " +
                                      ptr + ", " + to_string(order));
        }
        if(builtin_name == "__c11_atomic_store") {
          if(node.children.size() != 4) {
            throw logic_error("__c11_atomic_store child count");
          }
          const string value_type =
              atomic_value_type_for_pointer(1, "__c11_atomic_store");
          const string dst = emit_rvalue(node.children[1]);
          const string src = emit_rvalue(node.children[2]);
          const long long order = atomic_order_value(3);
          emit_line(string("atomic_store ") + value_type + " " + src + ", " +
                    dst + ", " + to_string(order));
          return "0";
        }
        if(builtin_name == "__pseudo_destructor") {
          if(node.children.size() != 2) {
            throw logic_error("__pseudo_destructor child count");
          }
          emit_rvalue(node.children[1]);
          return "0";
        }
        if(builtin_name == "__c11_atomic_exchange") {
          if(node.children.size() != 4) {
            throw logic_error("__c11_atomic_exchange child count");
          }
          const string value_type =
              atomic_value_type_for_pointer(1, "__c11_atomic_exchange");
          const string ptr = emit_rvalue(node.children[1]);
          const string value = emit_rvalue(node.children[2]);
          const long long order = atomic_order_value(3);
          return emit_temp_assignment(value_type,
                                      string("atomic_exchange ") + value_type +
                                      " " + ptr + ", " + value + ", " +
                                      to_string(order));
        }
        if(builtin_name == "__c11_atomic_compare_exchange_strong" ||
           builtin_name == "__c11_atomic_compare_exchange_weak") {
          if(node.children.size() != 6) {
            throw logic_error(builtin_name + " child count");
          }
          const string value_type =
              atomic_value_type_for_pointer(1, builtin_name);
          const string ptr = emit_rvalue(node.children[1]);
          const string expected_ptr = emit_rvalue(node.children[2]);
          const string desired = emit_rvalue(node.children[3]);
          const long long success_order = atomic_order_value(4);
          const long long failure_order = atomic_order_value(5);
          return emit_temp_assignment("i64",
                                      string("atomic_compare_exchange ") +
                                      value_type + " " + ptr + ", " +
                                      expected_ptr + ", " + desired + ", " +
                                      to_string(success_order) + ", " +
                                      to_string(failure_order));
        }
        if(builtin_name == "__atomic_load") {
          if(node.children.size() != 4) {
            throw logic_error("__atomic_load child count");
          }
          const string value_type =
              atomic_value_type_for_pointer(1, "__atomic_load");
          const string ptr = emit_rvalue(node.children[1]);
          const string dst = emit_rvalue(node.children[2]);
          const long long order = atomic_order_value(3);
          const string loaded =
              emit_temp_assignment(value_type,
                                  string("atomic_load ") + value_type + " " +
                                  ptr + ", " + to_string(order));
          emit_line(string("store ") + value_type + " " + loaded + ", " + dst);
          return "0";
        }
        if(builtin_name == "__atomic_store") {
          if(node.children.size() != 4) {
            throw logic_error("__atomic_store child count");
          }
          const string value_type =
              atomic_value_type_for_pointer(1, "__atomic_store");
          const string dst = emit_rvalue(node.children[1]);
          const string src = emit_rvalue(node.children[2]);
          const long long order = atomic_order_value(3);
          emit_line(string("atomic_store ") + value_type + " " + src + ", " +
                    dst + ", " + to_string(order));
          return "0";
        }
        if(builtin_name == "__atomic_add_fetch") {
          if(node.children.size() != 4) {
            throw logic_error("__atomic_add_fetch child count");
          }
          const string ptr = emit_rvalue(node.children[1]);
          const string value = emit_rvalue(node.children[2]);
          const long long order = atomic_order_value(3);
          return emit_temp_assignment(lowir_type_for(node.semantic_type),
                                      string("atomic_add_fetch ") +
                                      lowir_type_for(node.semantic_type) + " " +
                                      ptr + ", " + value + ", " +
                                      to_string(order));
        }
        if(builtin_name == "__atomic_fetch_add" ||
           builtin_name == "__c11_atomic_fetch_add") {
          if(node.children.size() != 4) {
            throw logic_error(builtin_name + " child count");
          }
          const string ptr = emit_rvalue(node.children[1]);
          const string value = emit_rvalue(node.children[2]);
          const string value_type = lowir_type_for(node.semantic_type);
          const long long order = atomic_order_value(3);
          const string updated =
              emit_temp_assignment(value_type,
                                  string("atomic_add_fetch ") + value_type + " " +
                                  ptr + ", " + value + ", " + to_string(order));
          return emit_temp_assignment(value_type,
                                      string("binary sub ") + value_type + " " +
                                      updated + ", " + value);
        }
        if(builtin_name == "__atomic_fetch_sub" ||
           builtin_name == "__c11_atomic_fetch_sub") {
          if(node.children.size() != 4) {
            throw logic_error(builtin_name + " child count");
          }
          const string ptr = emit_rvalue(node.children[1]);
          const string value = emit_rvalue(node.children[2]);
          const string delta_type = lowir_type_for(node.children[2].semantic_type);
          const string value_type = lowir_type_for(node.semantic_type);
          const string neg_value =
              emit_temp_assignment(delta_type,
                                  string("unary neg ") + delta_type + " " + value);
          const long long order = atomic_order_value(3);
          const string updated =
              emit_temp_assignment(value_type,
                                  string("atomic_add_fetch ") + value_type + " " +
                                  ptr + ", " + neg_value + ", " + to_string(order));
          return emit_temp_assignment(value_type,
                                      string("binary add ") + value_type + " " +
                                      updated + ", " + value);
        }
        if(builtin_name == "__c11_atomic_fetch_and" ||
           builtin_name == "__c11_atomic_fetch_or" ||
           builtin_name == "__c11_atomic_fetch_xor") {
          if(node.children.size() != 4) {
            throw logic_error(builtin_name + " child count");
          }
          const string ptr = emit_rvalue(node.children[1]);
          const string value = emit_rvalue(node.children[2]);
          const string value_type = lowir_type_for(node.semantic_type);
          const long long order = atomic_order_value(3);
          string op = "xor";
          if(builtin_name == "__c11_atomic_fetch_and") {
            op = "and";
          } else if(builtin_name == "__c11_atomic_fetch_or") {
            op = "or";
          }
          return emit_atomic_compare_exchange_loop(value_type, ptr, value, order, op);
        }
        if(builtin_name == "__atomic_thread_fence" ||
           builtin_name == "__c11_atomic_thread_fence") {
          if(node.children.size() != 2) {
            throw logic_error(builtin_name + " child count");
          }
          emit_line(string("atomic_thread_fence ") + to_string(atomic_order_value(1)));
          return "0";
        }
        if(builtin_name == "__atomic_signal_fence" ||
           builtin_name == "__c11_atomic_signal_fence") {
          if(node.children.size() != 2) {
            throw logic_error(builtin_name + " child count");
          }
          emit_line(string("atomic_signal_fence ") + to_string(atomic_order_value(1)));
          return "0";
        }
        if(builtin_name == "__builtin_bswap16" ||
           builtin_name == "__builtin_bswap32" ||
           builtin_name == "__builtin_bswap64") {
          if(node.children.size() != 2) {
            throw logic_error(builtin_name + " child count");
          }
          const string value = emit_rvalue(node.children[1]);
          return emit_builtin_bswap_value(builtin_name, value);
        }
        if(builtin_name == "__builtin_clz" ||
           builtin_name == "__builtin_clzl" ||
           builtin_name == "__builtin_clzll" ||
           builtin_name == "__builtin_clzg") {
          return emit_builtin_clzg_value(node);
        }
        if(builtin_name == "__builtin_ctz" ||
           builtin_name == "__builtin_ctzl" ||
           builtin_name == "__builtin_ctzll" ||
           builtin_name == "__builtin_ctzg") {
          return emit_builtin_ctzg_value(node);
        }
        if(builtin_name == "__builtin_popcount" ||
           builtin_name == "__builtin_popcountl" ||
           builtin_name == "__builtin_popcountll" ||
           builtin_name == "__builtin_popcountg") {
          return emit_builtin_popcount_value(node);
        }
        if(builtin_name == "__builtin_add_overflow" ||
           builtin_name == "__builtin_sub_overflow" ||
           builtin_name == "__builtin_mul_overflow") {
          return emit_builtin_same_type_overflow_value(builtin_name, node);
        }
        if(builtin_name == "__builtin_expect") {
          if(node.children.size() != 3) {
            throw logic_error("__builtin_expect child count");
          }
          const string value = emit_rvalue(node.children[1]);
          emit_rvalue(node.children[2]);
          return value;
        }
        if(builtin_name == "__builtin_is_constant_evaluated") {
          if(node.children.size() != 1) {
            throw logic_error("__builtin_is_constant_evaluated child count");
          }
          return emit_temp_assignment("i64", "const i64 0");
        }
        if(builtin_name == "__builtin_flt_rounds") {
          if(node.children.size() != 1) {
            throw logic_error("__builtin_flt_rounds child count");
          }
          return emit_temp_assignment("i32", "const i32 1");
        }
        if(builtin_name == "__builtin_fpclassify") {
          return emit_builtin_fpclassify_value(node);
        }
        if(builtin_name == "__builtin_signbit") {
          return emit_builtin_signbit_value(node);
        }
        if(builtin_name == "__builtin_isnan" ||
           builtin_name == "__builtin_isinf" ||
           builtin_name == "__builtin_isfinite" ||
           builtin_name == "__builtin_isnormal") {
          return emit_builtin_fp_classification_value(builtin_name, node);
        }
        if(builtin_name == "__builtin_inf" ||
           builtin_name == "__builtin_huge_val") {
          if(node.children.size() != 1) {
            throw logic_error(builtin_name + " child count");
          }
          return emit_temp_assignment("f64", "const f64 inf");
        }
        if(builtin_name == "__builtin_inff" ||
           builtin_name == "__builtin_huge_valf") {
          if(node.children.size() != 1) {
            throw logic_error(builtin_name + " child count");
          }
          return emit_temp_assignment("f32", "const f32 inff");
        }
        if(builtin_name == "__builtin_infl" ||
           builtin_name == "__builtin_huge_vall") {
          if(node.children.size() != 1) {
            throw logic_error(builtin_name + " child count");
          }
          return emit_temp_assignment("f80", "const f80 infl");
        }
        if(builtin_name == "__builtin_nan" ||
           builtin_name == "__builtin_nans") {
          if(node.children.size() != 2) {
            throw logic_error(builtin_name + " child count");
          }
          emit_rvalue(node.children[1]);
          return emit_temp_assignment(
              "f64",
              builtin_name == "__builtin_nans" ? "const f64 snan" : "const f64 nan");
        }
        if(builtin_name == "__builtin_nanf" ||
           builtin_name == "__builtin_nansf") {
          if(node.children.size() != 2) {
            throw logic_error(builtin_name + " child count");
          }
          emit_rvalue(node.children[1]);
          return emit_temp_assignment(
              "f32",
              builtin_name == "__builtin_nansf" ? "const f32 snanf" : "const f32 nanf");
        }
        if(builtin_name == "__builtin_nanl" ||
           builtin_name == "__builtin_nansl") {
          if(node.children.size() != 2) {
            throw logic_error(builtin_name + " child count");
          }
          emit_rvalue(node.children[1]);
          return emit_temp_assignment(
              "f80",
              builtin_name == "__builtin_nansl" ? "const f80 snanL" : "const f80 nanL");
        }
        if(builtin_name == "__builtin_assume_aligned") {
          if(node.children.size() != 3 && node.children.size() != 4) {
            throw logic_error(builtin_name + " child count");
          }
          return emit_rvalue(node.children[1]);
        }
        if(builtin_name == "__builtin_va_start") {
          if(node.children.size() != 3) {
            throw logic_error("__builtin_va_start child count");
          }
          const string list_ptr = emit_rvalue(node.children[1]);
          emit_line("va_start " + list_ptr);
          return "0";
        }
        if(builtin_name == "__builtin_va_end") {
          if(node.children.size() != 2) {
            throw logic_error("__builtin_va_end child count");
          }
          emit_rvalue(node.children[1]);
          return "0";
        }
        if(builtin_name == "__builtin_va_arg") {
          if(node.children.size() != 2) {
            throw logic_error("__builtin_va_arg child count");
          }
          const string list_ptr = emit_rvalue(node.children[1]);
          const string result_type = lowir_type_for(node.semantic_type);
          return emit_temp_assignment(result_type, "va_arg " + result_type + " " + list_ptr);
        }
        if(builtin_name == "__builtin_alloca") {
          if(node.children.size() != 2) {
            throw logic_error("__builtin_alloca child count");
          }
          const string size = emit_rvalue(node.children[1]);
          return emit_temp_assignment("ptr", string("stack_alloc ") + size);
        }
        if(builtin_name == "__atomic_always_lock_free" ||
           builtin_name == "__atomic_is_lock_free" ||
           builtin_name == "__c11_atomic_is_lock_free") {
          const bool c11_query = builtin_name == "__c11_atomic_is_lock_free";
          if(node.children.size() != (c11_query ? 2u : 3u)) {
            throw logic_error(builtin_name + " child count");
          }
          long long size_value = 0;
          if(try_get_integral_literal_value(node.children[1], size_value)) {
            const long long value =
                (size_value == 1 || size_value == 2 || size_value == 4 || size_value == 8) ? 1 : 0;
            return emit_temp_assignment("i64", string("const i64 ") + to_string(value));
          }
          const string size_expr = emit_rvalue(node.children[1]);
          const string eq1 = emit_temp_assignment("i64", string("cmp eq i64 ") + size_expr + ", 1");
          const string eq2 = emit_temp_assignment("i64", string("cmp eq i64 ") + size_expr + ", 2");
          const string eq4 = emit_temp_assignment("i64", string("cmp eq i64 ") + size_expr + ", 4");
          const string eq8 = emit_temp_assignment("i64", string("cmp eq i64 ") + size_expr + ", 8");
          const string any12 = emit_temp_assignment("i64", string("binary or i64 ") + eq1 + ", " + eq2);
          const string any48 = emit_temp_assignment("i64", string("binary or i64 ") + eq4 + ", " + eq8);
          return emit_temp_assignment("i64", string("binary or i64 ") + any12 + ", " + any48);
        }
      }
      TypePtr function_type;
      if(!resolve_callable_function_type(node.children[0].semantic_type, function_type) ||
         !function_type || !function_type->inner) {
        throw logic_error("call-expression missing function type");
      }
      const string call_result = emit_call_expression_raw(node);
      return emit_call_expression_value(node, function_type->inner, call_result);
    }

    {
      ostringstream outmsg;
      outmsg << "unsupported expression in PA14 LowIR lowering kind="
             << callsem_kind_text(node.kind);
      if(!node.text.empty()) {
        outmsg << " text=" << node.text;
      }
      if(node.semantic_type) {
        outmsg << " type=" << describe_type(node.semantic_type);
      }
      throw logic_error(outmsg.str());
    }
  }

  string emit_lvalue_address(const CallSemNode & node)
  {
    ScopedLowIRCurrentExpr current_expr(node);

    if(node.kind == CallSemKind::typeid_expression) {
      return emit_typeid_value(node);
    }

    const bool has_direct_special_lvalue_address =
        node.value_category == CVC_LVALUE &&
        (node.kind == CallSemKind::conditional_expression ||
         (node.kind == CallSemKind::binary_expression &&
          callsem_has_token(node, OP_COMMA)));
    if(is_complete_class_value_type(node.semantic_type) &&
       is_special_class_materialization_node(node) &&
       !has_direct_special_lvalue_address) {
      const string temp_ptr = new_hidden_object_address(node.semantic_type, "tmpobj");
      if(emit_special_class_value_to_target(node, temp_ptr)) {
        register_materialized_temporary_cleanup_live(node.semantic_type, temp_ptr);
        return temp_ptr;
      }
    }

    TypePtr node_base = strip_top_level_cv(remove_reference_type(node.semantic_type));
    if(node.value_category != CVC_LVALUE &&
       !is_reference_type(node.semantic_type) &&
       node_base &&
       !is_indirect_value_type(node.semantic_type) &&
       !is_function_type(node_base) &&
       node_base->kind != Type::TK_ARRAY &&
       node.kind != CallSemKind::id_expression &&
       node.kind != CallSemKind::variable &&
       node.kind != CallSemKind::member_expression &&
       node.kind != CallSemKind::subscript_expression &&
       node.kind != CallSemKind::conditional_expression) {
      const TypePtr materialized_type =
          materialization_source_type_for(node, node.semantic_type);
      const string memory_type = lowir_memory_type_for(materialized_type);
      const string temp_slot = new_hidden_slot(memory_type, "tmpref");
      const string value = emit_rvalue(node);
      emit_line("store " + memory_type + " " + value + ", " + temp_slot);
      return emit_storage_address(temp_slot);
    }

    if(node.kind == CallSemKind::literal) {
      QuoteLiteralData string_literal;
      if(try_parse_string_literal_node(node, string_literal)) {
        map<string, string>::const_iterator found = string_literal_symbols_.find(node.text);
        if(found == string_literal_symbols_.end()) {
          throw logic_error("missing string literal global");
        }
        return emit_temp_assignment("ptr", string("addr ") + found->second);
      }
    }

    if(node.kind == CallSemKind::id_expression) {
      string out;
      if(try_emit_known_id_address(node, out)) {
        return out;
      }
      ostringstream outmsg;
      outmsg << "unknown lvalue id-expression " << node.text;
      if(g_lowir_current_function_node) {
        outmsg << " [function "
               << (g_lowir_current_function_node->text.empty() ?
                       node_internal_symbol(*g_lowir_current_function_node) :
                       g_lowir_current_function_node->text.str())
               << "]";
      }
      if(!callsem_symbol(node).internal_symbol.empty()) {
        outmsg << " [internal_symbol " << callsem_symbol(node).internal_symbol << "]";
      }
      if(node.semantic_type) {
        outmsg << " [type " << describe_type(node.semantic_type) << "]";
      }
      throw logic_error(outmsg.str());
    }

    if(node.kind == CallSemKind::member_expression && node.children.size() == 1) {
      const lowir_internal::IndexProjectionKind projection =
          member_projection_kind(node);
      const auto address_from_parameter_hidden_virtual_base =
          [&](const string & actual_virtual_base_ptr,
              unsigned long long actual_virtual_base_offset) -> string
          {
            string base = actual_virtual_base_ptr;
            if(actual_virtual_base_offset != 0) {
              base = emit_index_address_with_projection("i8",
                                                        actual_virtual_base_ptr,
                                                        actual_virtual_base_offset,
                                                        lowir_internal::IPK_BASE_SUBOBJECT,
                                                        false);
            }
            const size_t offset =
                (node.is_base_subobject || node.is_virtual_base_subobject) ?
                    0 :
                    callsem_uint_value(node);
            const string field_storage =
                emit_index_address_with_projection("i8",
                                                   base,
                                                   offset,
                                                   projection,
                                                   false);
            if(node.is_reference_storage && !node.is_reference_storage_target) {
              return emit_temp_assignment("ptr", string("load ptr ") + field_storage);
            }
            return field_storage;
          };
      const CallSemVirtualBaseLayout & virtual_base_layout =
          callsem_virtual_base_layout(node);
      if(!virtual_base_layout.empty() &&
         !(node.is_base_subobject && !node.is_virtual_base_subobject)) {
        const pair<string, unsigned long long> & virtual_base = virtual_base_layout[0];
        if(root_is_current_this(node.children[0])) {
          map<string, string>::const_iterator hidden =
              hidden_virtual_base_params_.find(virtual_base.first);
          if(hidden != hidden_virtual_base_params_.end()) {
            return emit_index_address_with_projection("i8",
                                                      hidden->second,
                                                      virtual_base.second,
                                                      lowir_internal::IPK_BASE_SUBOBJECT,
                                                      false);
          }
          map<string, unsigned long long>::const_iterator offset =
              current_virtual_base_offsets_.find(virtual_base.first);
          if(offset != current_virtual_base_offsets_.end()) {
            const CallSemNode * current_root = peel_base_subobject_root(node.children[0]);
            if(!current_root) {
              throw logic_error("missing current object root for virtual base member");
            }
            const string root_ptr = emit_pointer_operand(*current_root);
            const unsigned long long total_offset = offset->second + virtual_base.second;
            return emit_index_address_with_projection("i8",
                                                      root_ptr,
                                                      total_offset,
                                                      lowir_internal::IPK_BASE_SUBOBJECT,
                                                      false);
          }
        }
        string local_hidden;
        if(try_load_hidden_virtual_base_from_local_binding(node.children[0],
                                                           virtual_base.first,
                                                           local_hidden)) {
          if(root_is_parameter_binding(node.children[0])) {
            return address_from_parameter_hidden_virtual_base(local_hidden,
                                                              virtual_base.second);
          }
          return emit_index_address_with_projection("i8",
                                                    local_hidden,
                                                    virtual_base.second,
                                                    lowir_internal::IPK_BASE_SUBOBJECT,
                                                    false);
        }
        const CallSemNode * root = peel_base_subobject_root(node.children[0]);
        if(root && root->kind == CallSemKind::call_expression) {
          const string object_ptr = emit_pointer_operand(node.children[0]);
          const string forwarded_virtual_base =
              emit_hidden_virtual_base_argument(virtual_base, node.children[0], &object_ptr);
          return address_from_parameter_hidden_virtual_base(forwarded_virtual_base,
                                                            virtual_base.second);
        }
        if(root_is_parameter_binding(node.children[0])) {
          map<string, string>::const_iterator hidden =
              parameter_hidden_virtual_base_params_.find(virtual_base.first);
          if(hidden != parameter_hidden_virtual_base_params_.end()) {
            return address_from_parameter_hidden_virtual_base(hidden->second,
                                                              virtual_base.second);
          }
        }
        if(dynamic_external_virtual_base_pointer_available(node.children[0].semantic_type,
                                                           virtual_base.first,
                                                           expression_path_uses_reference_storage(
                                                               node.children[0]))) {
          const string object_ptr = emit_pointer_operand(node.children[0]);
          string dynamic_external;
          if(!try_emit_dynamic_external_virtual_base_pointer(node.children[0].semantic_type,
                                                             object_ptr,
                                                             virtual_base.first,
                                                             dynamic_external,
                                                             expression_path_uses_reference_storage(
                                                                 node.children[0]))) {
            throw logic_error("failed to emit dynamic external virtual base pointer");
          }
          return address_from_parameter_hidden_virtual_base(dynamic_external,
                                                            virtual_base.second);
        }
      }
      const string qualified_name =
          callsem_resolved_name(node).empty() ? node.text.str() :
              callsem_resolved_name(node);
      if(!qualified_name.empty() && node.is_virtual_base_subobject) {
        if(root_is_current_this(node.children[0]) &&
           (hidden_virtual_base_params_.count(qualified_name) != 0 ||
            current_virtual_base_offsets_.count(qualified_name) != 0)) {
          map<string, string>::const_iterator hidden =
              hidden_virtual_base_params_.find(qualified_name);
          if(hidden != hidden_virtual_base_params_.end()) {
            return emit_index_address_with_projection("i8",
                                                      hidden->second,
                                                      0,
                                                      lowir_internal::IPK_BASE_SUBOBJECT,
                                                      false);
          }
          map<string, unsigned long long>::const_iterator offset =
              current_virtual_base_offsets_.find(qualified_name);
          if(offset != current_virtual_base_offsets_.end()) {
            const CallSemNode * current_root = peel_base_subobject_root(node.children[0]);
            if(!current_root) {
              throw logic_error("missing current object root for virtual base member");
            }
            const string root_ptr = emit_pointer_operand(*current_root);
            return emit_index_address_with_projection("i8",
                                                      root_ptr,
                                                      offset->second,
                                                      lowir_internal::IPK_BASE_SUBOBJECT,
                                                      false);
          }
        }
        string local_hidden;
        if(try_load_hidden_virtual_base_from_local_binding(node.children[0],
                                                           qualified_name,
                                                           local_hidden)) {
          return emit_index_address_with_projection("i8",
                                                    local_hidden,
                                                    0,
                                                    lowir_internal::IPK_BASE_SUBOBJECT,
                                                    false);
        }
        if(root_is_parameter_binding(node.children[0])) {
          map<string, string>::const_iterator hidden =
              parameter_hidden_virtual_base_params_.find(qualified_name);
          if(hidden != parameter_hidden_virtual_base_params_.end()) {
            return emit_index_address_with_projection("i8",
                                                      hidden->second,
                                                      0,
                                                      lowir_internal::IPK_BASE_SUBOBJECT,
                                                      false);
          }
        }
      }
      const string offset =
          node.has_int_value ? to_string(callsem_int_value(node)) :
                               to_string(callsem_uint_value(node));
      TypePtr base_type = strip_top_level_cv(node.children[0].semantic_type);
      string base;
      if(base_type &&
         (base_type->kind == Type::TK_POINTER ||
          base_type->kind == Type::TK_LVALUE_REFERENCE ||
          base_type->kind == Type::TK_RVALUE_REFERENCE)) {
        if(is_synthetic_subobject_pointer_node(node.children[0])) {
          base = emit_lvalue_address(node.children[0]);
        } else {
          base = emit_rvalue(node.children[0]);
        }
      } else {
        base = emit_lvalue_address(node.children[0]);
      }
      const string field_storage =
          node.is_base_subobject &&
                  base_type &&
                  base_type->kind == Type::TK_POINTER &&
                  base_subobject_pointer_operand_may_be_null(node.children[0]) ?
              emit_null_preserving_index_address_with_projection("i8",
                                                                 base,
                                                                 offset,
                                                                 projection) :
              emit_index_address_with_projection("i8",
                                                 base,
                                                 offset,
                                                 projection,
                                                 false);
      if(node.is_reference_storage && !node.is_reference_storage_target) {
        return emit_temp_assignment("ptr", string("load ptr ") + field_storage);
      }
      return field_storage;
    }

    if(node.kind == CallSemKind::binary_expression &&
       node.children.size() == 2 &&
       (callsem_has_token(node, OP_DOTSTAR) || callsem_has_token(node, OP_ARROWSTAR))) {
      TypePtr member_pointer_type =
          strip_top_level_cv(remove_reference_type(node.children[1].semantic_type));
      if(!member_pointer_type ||
         member_pointer_type->kind != Type::TK_MEMBER_POINTER ||
         is_function_type(member_pointer_type->inner)) {
        throw logic_error("pointer-to-member lvalue requires data member pointer");
      }

      const string base =
          callsem_has_token(node, OP_ARROWSTAR) ?
              emit_rvalue(node.children[0]) :
              emit_lvalue_address(node.children[0]);
      const string encoded_offset = emit_rvalue(node.children[1]);
      const string offset =
          emit_temp_assignment("i64",
                               string("binary sub i64 ") + encoded_offset + ", 1");
      const string field_storage =
          emit_index_address_with_projection("i8",
                                             base,
                                             offset,
                                             is_reference_type(node.semantic_type) ?
                                                 lowir_internal::IPK_REFERENCE_FIELD :
                                                 lowir_internal::IPK_FIELD,
                                             false);
      if(is_reference_type(node.semantic_type)) {
        return emit_temp_assignment("ptr", string("load ptr ") + field_storage);
      }
      return field_storage;
    }

    const TypePtr indirect_result_object_type =
        node.kind == CallSemKind::call_expression ?
            indirect_call_result_object_type(node) :
            TypePtr();
    if(node.kind == CallSemKind::call_expression &&
       (is_indirect_value_type(node.semantic_type) ||
        indirect_result_object_type ||
        is_constructor_materialization_call(node))) {
      const TypePtr object_type =
          indirect_result_object_type ?
              indirect_result_object_type :
              remove_reference_type(node.semantic_type);
      const string temp_ptr =
          new_hidden_object_address(object_type, "tmpobj");
      emit_call_expression_to_target(node, temp_ptr);
      register_materialized_temporary_cleanup_live(object_type, temp_ptr);
      return temp_ptr;
    }

    if(node.kind == CallSemKind::call_expression &&
       is_reference_type(node.semantic_type)) {
      return emit_call_expression_raw(node);
    }

    if(node.kind == CallSemKind::assignment_expression && node.children.size() == 2) {
      emit_rvalue(node);
      return emit_lvalue_address(node.children[0]);
    }

    if(node.kind == CallSemKind::binary_expression &&
       callsem_has_token(node, OP_COMMA)) {
      if(node.children.size() != 2) {
        throw logic_error("binary-expression arity");
      }
      emit_discarded_expression(node.children[0]);
      if(!current_block_) {
        return "0";
      }
      return emit_lvalue_address(node.children[1]);
    }

    if(node.kind == CallSemKind::unary_expression && callsem_has_token(node, OP_STAR) &&
       node.children.size() == 1) {
      return emit_rvalue(node.children[0]);
    }

    if(node.kind == CallSemKind::conditional_expression) {
      if(node.children.size() != 3) {
        throw logic_error("conditional-expression arity");
      }
      const string result_slot = new_hidden_slot("ptr", "condaddr");
      const string then_label = new_block("condaddr_then");
      const string else_label = new_block("condaddr_else");
      const string end_label = new_block("condaddr_end");
      const string cond = emit_branch_condition_value(node.children[0]);
      terminate(string("branch ") + cond + ", " + lowir_block_name(then_label) + ", " +
                lowir_block_name(else_label));

      const auto emit_address_branch = [&](const CallSemNode & branch)
      {
        push_cleanup_scope();
        emit_line("store ptr " + emit_lvalue_address(branch) + ", " + result_slot);
        if(current_block_) {
          emit_scope_cleanups(cleanup_scopes_.back());
        }
        pop_cleanup_scope();
      };

      bool then_fallthrough = false;
      start_block(then_label);
      emit_address_branch(node.children[1]);
      if(current_block_) {
        terminate(string("jump ") + lowir_block_name(end_label));
        then_fallthrough = true;
      }

      bool else_fallthrough = false;
      start_block(else_label);
      emit_address_branch(node.children[2]);
      if(current_block_) {
        terminate(string("jump ") + lowir_block_name(end_label));
        else_fallthrough = true;
      }

      if(then_fallthrough || else_fallthrough) {
        start_block(end_label);
      } else {
        current_block_ = nullptr;
      }
      return emit_temp_assignment("ptr", string("load ptr ") + result_slot);
    }

    if(node.kind == CallSemKind::subscript_expression && node.children.size() == 2) {
      const string base = emit_array_base(node.children[0]);
      const string index = emit_rvalue(node.children[1]);
      TypePtr element_type = remove_reference_type(node.semantic_type);
      TypePtr element_base = strip_top_level_cv(element_type);
      if(is_indirect_value_type(element_type) ||
         (element_base && element_base->kind == Type::TK_ARRAY)) {
        const size_t stride = backend_storage_size(element_type);
        string offset = index;
        if(stride != 1) {
          offset = emit_temp_assignment("i64",
                                        string("binary mul i64 ") + index + ", " +
                                        to_string(stride));
        }
        return emit_index_address_with_projection("i8",
                                                  base,
                                                  offset,
                                                  lowir_internal::IPK_ARRAY_ELEMENT,
                                                  false);
      }
      const string memory_type = lowir_memory_type_for(element_type);
      return emit_index_address_with_projection(memory_type,
                                                base,
                                                index,
                                                lowir_internal::IPK_ARRAY_ELEMENT,
                                                false);
    }

    if(is_reference_type(node.semantic_type)) {
      return emit_rvalue(node);
    }

    if(node.kind == CallSemKind::unary_expression &&
       node.children.size() == 1 &&
       (callsem_has_token(node, OP_INC) || callsem_has_token(node, OP_DEC)) &&
       node.value_category == CVC_LVALUE) {
      const string memory_type = lowir_lvalue_memory_type(node.children[0]);
      const string old_value = is_bit_field_member_expression(node.children[0]) ?
          emit_bit_field_rvalue(node.children[0]) :
          emit_temp_assignment(memory_type,
                               string("load ") + memory_type + " " +
                               emit_lvalue_storage(node.children[0]));
      const string next_value =
          emit_incdec_next_value(node, memory_type, old_value);
      const string target = emit_lvalue_storage(node.children[0]);
      if(is_bit_field_member_expression(node.children[0])) {
        emit_store_to_bit_field(node.children[0], next_value);
      } else {
        const string debug_name = direct_local_debug_name(node.children[0]);
        const string stored_value =
            debug_name.empty() ? next_value
                               : emit_debug_named_local_value(debug_name,
                                                              memory_type,
                                                              next_value);
        emit_line("store " + memory_type + " " + stored_value + ", " + target);
      }
      return target;
    }

    ostringstream out;
    out << "unsupported lvalue address in PA14 LowIR lowering";
    out << " [kind " << callsem_kind_text(node.kind) << "]";
    if(!node.text.empty()) {
      out << " [text " << node.text << "]";
    }
    if(node.semantic_type) {
      out << " [type " << describe_type(node.semantic_type) << "]";
    }
    out << " [child_count " << node.children.size() << "]";
    if(function_node_) {
      out << " [function " << function_node_->text << "]";
      if(function_node_->semantic_type) {
        out << " [function_type " << describe_type(function_node_->semantic_type) << "]";
      }
    }
    if(!function_.name.empty()) {
      out << " [lowir_function " << function_.name << "]";
    }
    throw logic_error(out.str());
  }

  string emit_lvalue_storage(const CallSemNode & node)
  {
    if(node.kind == CallSemKind::id_expression) {
      string out;
      if(try_emit_known_id_storage(node, out)) {
        return out;
      }
    ostringstream outmsg;
    outmsg << "unknown lvalue id-expression " << node.text;
    if(g_lowir_current_function_node) {
      outmsg << " [function "
             << (g_lowir_current_function_node->text.empty() ?
                     node_internal_symbol(*g_lowir_current_function_node) :
                     g_lowir_current_function_node->text.str())
             << "]";
    }
    if(!callsem_symbol(node).internal_symbol.empty()) {
      outmsg << " [internal_symbol " << callsem_symbol(node).internal_symbol << "]";
    }
      if(node.semantic_type) {
        outmsg << " [type " << describe_type(node.semantic_type) << "]";
      }
      throw logic_error(outmsg.str());
    }
    if(node.kind == CallSemKind::member_expression) {
      return emit_lvalue_address(node);
    }
    return emit_lvalue_address(node);
  }

  string direct_local_debug_name(const CallSemNode & node) const
  {
    if(node.kind != CallSemKind::id_expression || node.text.empty()) {
      return string();
    }
    const VariableBinding * binding = find_local_binding(node.text);
    if(binding == nullptr || binding->is_parameter) {
      return string();
    }
    return can_name_debug_local_value(node.text) ? node.text.str() : string();
  }

  bool is_bit_field_member_expression(const CallSemNode & node) const
  {
    return node.kind == CallSemKind::member_expression && node.is_bit_field;
  }

  string emit_bit_field_rvalue(const CallSemNode & node)
  {
    const string target = emit_lvalue_storage(node);
    const string memory_type = lowir_lvalue_memory_type(node);
    string value =
        emit_temp_assignment(memory_type, string("load ") + memory_type + " " + target);
    if(callsem_bit_field_offset(node) != 0) {
      value = emit_temp_assignment(memory_type,
                                   string("binary shr ") + memory_type + " " + value + ", " +
                                   to_string(callsem_bit_field_offset(node)));
    }
    const unsigned long long mask =
        lowir_bit_field_mask(static_cast<size_t>(callsem_bit_field_width(node)));
    const size_t storage_bits =
        static_cast<size_t>(
            callsem_bit_field_storage_size(node) != 0 ?
                callsem_bit_field_storage_size(node) :
                type_size(node.semantic_type)) * 8;
    if(mask != ~0ULL &&
       static_cast<size_t>(callsem_bit_field_width(node)) < storage_bits) {
      value = emit_temp_assignment(memory_type,
                                   string("binary and ") + memory_type + " " + value + ", " +
                                   to_string(mask));
    }
    return value;
  }

  void emit_store_to_bit_field(const CallSemNode & node, const string & rhs)
  {
    const string target = emit_lvalue_storage(node);
    const string memory_type = lowir_lvalue_memory_type(node);
    const size_t storage_bits =
        static_cast<size_t>(
            callsem_bit_field_storage_size(node) != 0 ?
                callsem_bit_field_storage_size(node) :
                type_size(node.semantic_type)) * 8;
    const unsigned long long field_mask =
        lowir_bit_field_mask(static_cast<size_t>(callsem_bit_field_width(node)));
    const unsigned long long storage_mask =
        field_mask == ~0ULL ? ~0ULL : (field_mask << callsem_bit_field_offset(node));

    string rhs_value = rhs;
    if(field_mask != ~0ULL &&
       static_cast<size_t>(callsem_bit_field_width(node)) < storage_bits) {
      rhs_value = emit_temp_assignment(memory_type,
                                       string("binary and ") + memory_type + " " + rhs_value +
                                       ", " + to_string(field_mask));
    }
    if(callsem_bit_field_offset(node) != 0) {
      rhs_value = emit_temp_assignment(memory_type,
                                       string("binary shl ") + memory_type + " " + rhs_value +
                                       ", " + to_string(callsem_bit_field_offset(node)));
    }

    if(storage_mask == ~0ULL && callsem_bit_field_offset(node) == 0) {
      emit_line("store " + memory_type + " " + rhs_value + ", " + target);
      return;
    }

    const string old_value =
        emit_temp_assignment(memory_type, string("load ") + memory_type + " " + target);
    const unsigned long long preserve_mask =
        lowir_bit_field_mask(storage_bits) & ~storage_mask;
    string merged = rhs_value;
    if(preserve_mask != 0) {
      const string preserved =
          emit_temp_assignment(memory_type,
                               string("binary and ") + memory_type + " " + old_value + ", " +
                               to_string(preserve_mask));
      merged = emit_temp_assignment(memory_type,
                                    string("binary or ") + memory_type + " " + preserved + ", " +
                                    rhs_value);
    }
    emit_line("store " + memory_type + " " + merged + ", " + target);
  }

  string emit_array_base(const CallSemNode & node)
  {
    if(node.kind == CallSemKind::id_expression) {
      const VariableBinding * binding = find_local_binding(node.text);
      if(binding) {
        TypePtr expr_type = strip_top_level_cv(node.semantic_type);
        if(binding_is_array_storage(*binding) && !binding->slots.empty()) {
          return emit_decay_pointer(emit_storage_address(binding->slots[0]));
        }
        if(binding_is_decay_view_slot(*binding)) {
          return emit_temp_assignment("ptr", string("load ptr ") + binding->slots[0]);
        }
        if(expr_type &&
           (expr_type->kind == Type::TK_POINTER || expr_type->kind == Type::TK_LVALUE_REFERENCE)) {
          return emit_rvalue(node);
        }
      }
      const GlobalBinding * global = find_global_binding(node);
      if(global) {
        return emit_decay_pointer(
            emit_temp_assignment("ptr", string("addr ") + global->storage));
      }
    }
    return emit_rvalue(node);
  }

  void emit_indirect_value_initializer_to_target(const CallSemNode & variable,
                                                 const string & target_ptr,
                                                 bool register_storage_cleanup,
                                                 const string & storage_slot = string())
  {
    for(size_t i = 0; i < variable.children.size(); ++i) {
      const CallSemNode & child = variable.children[i];
      if(is_complete_class_value_type(variable.semantic_type) &&
         is_special_class_materialization_node(child)) {
        push_cleanup_scope(true);
        emit_special_class_value_to_target(child, target_ptr);
        if(current_block_) {
          emit_scope_cleanups(cleanup_scopes_.back());
        }
        pop_cleanup_scope();
      } else if(is_complete_class_value_type(variable.semantic_type) &&
                child.kind == CallSemKind::expression_statement) {
        emit_action(child);
      } else if(is_complete_class_value_type(variable.semantic_type) &&
                child.kind == CallSemKind::constructor_action) {
        if(child.children.size() != 1 ||
           child.children[0].kind != CallSemKind::call_expression) {
          throw logic_error("indirect value constructor action shape");
        }
        const CallSemNode & call = child.children[0];
        if(call.children.size() < 2) {
          throw logic_error("indirect value constructor action missing target");
        }
        if(child.trivial_lifecycle) {
          if(call.children.size() > 3) {
            throw logic_error("unsupported trivial constructor action arity");
          }
          if(constructor_call_targets_whole_variable(call, variable)) {
            if(call.children.size() == 3 &&
               !is_empty_class_storage_type(variable.semantic_type)) {
              emit_storage_value_to_target(variable.semantic_type,
                                           call.children[2],
                                           target_ptr);
            }
          } else {
            emit_constructor_action(child);
          }
        } else {
          if(constructor_call_targets_whole_variable(call, variable)) {
            emit_constructor_call_expression_to_target(call, target_ptr);
          } else {
            emit_constructor_action(child);
          }
        }
      } else if(is_complete_class_value_type(variable.semantic_type) &&
                child.kind == CallSemKind::destructor_action) {
        if(register_storage_cleanup) {
          if(!storage_slot.empty()) {
            register_bound_local_cleanup(child, variable.text, storage_slot);
          } else {
            register_cleanup(child);
          }
        }
      } else if(is_indirect_class_reference_type(child.semantic_type)) {
        push_cleanup_scope(true);
        if(child.value_category == CVC_XVALUE) {
          emit_move_construct_to_target(variable.semantic_type,
                                        target_ptr,
                                        emit_rvalue(child));
        } else {
          emit_copy_construct_to_target(variable.semantic_type,
                                        target_ptr,
                                        emit_rvalue(child));
        }
        if(current_block_) {
          emit_scope_cleanups(cleanup_scopes_.back());
        }
        pop_cleanup_scope();
      } else if(child.kind == CallSemKind::call_expression &&
                (is_indirect_value_type(child.semantic_type) ||
                 is_complete_class_value_type(child.semantic_type))) {
        push_cleanup_scope(true);
        emit_call_expression_to_target(child, target_ptr);
        if(current_block_) {
          emit_scope_cleanups(cleanup_scopes_.back());
        }
        pop_cleanup_scope();
      } else {
        push_cleanup_scope(true);
        emit_copy_construct_to_target(variable.semantic_type,
                                      target_ptr,
                                      emit_lvalue_address(child));
        if(current_block_) {
          emit_scope_cleanups(cleanup_scopes_.back());
        }
        pop_cleanup_scope();
      }
    }
  }

  void emit_guarded_static_storage_initialization(const CallSemNode & variable)
  {
    if(callsem_local_static_guard_symbol(variable).empty()) {
      throw logic_error("missing function-local static guard symbol");
    }
    const string done_label = new_block("local_static_ready");
    const string init_label = new_block("local_static_init");
    const string guard_value =
        emit_temp_assignment("i64",
                             string("load i64 ") +
                                 callsem_local_static_guard_symbol(variable));
    const string already_initialized =
        emit_temp_assignment("i64", string("cmp ne i64 ") + guard_value + ", 0");
    terminate("branch " + already_initialized + ", " + lowir_block_name(done_label) + ", " +
              lowir_block_name(init_label));

    start_block(init_label);
    const string target_ptr = emit_storage_address(node_internal_symbol(variable));
    if(is_indirect_value_type(variable.semantic_type)) {
      emit_indirect_value_initializer_to_target(variable, target_ptr, false);
    } else {
      TypePtr base = strip_top_level_cv(variable.semantic_type);
      if(base && base->kind == Type::TK_ARRAY) {
        if(variable.children.size() == 1 &&
           variable.children[0].kind == CallSemKind::braced_init_list) {
          const CallSemNode & init = variable.children[0];
          emit_local_array_initializer(variable.semantic_type, init, target_ptr);
        } else {
          for(size_t i = 0; i < variable.children.size(); ++i) {
            const CallSemNode & child = variable.children[i];
            if(child.kind == CallSemKind::constructor_action) {
              emit_constructor_action(child);
            } else if(child.kind != CallSemKind::destructor_action) {
              throw logic_error("guarded static array initializer action shape");
            }
          }
        }
      } else if(is_reference_type(variable.semantic_type)) {
        if(variable.children.size() != 1) {
          throw logic_error("guarded static reference initializer arity");
        }
        emit_line("store ptr " +
                  emit_reference_storage_value(remove_reference_type(variable.semantic_type),
                                               variable.children[0]) +
                  ", " + target_ptr);
      } else if(!variable.children.empty()) {
        if(variable.children.size() != 1) {
          throw logic_error("guarded static initializer arity");
        }
        emit_line("store " + lowir_memory_type_for(variable.semantic_type) + " " +
                  emit_scalar_storage_value(variable.semantic_type,
                                            variable.children[0]) + ", " + target_ptr);
      }
    }
    if(variable.is_thread_local) {
      emit_thread_local_destructor_registration(variable.semantic_type, target_ptr);
    }
    emit_line("store i64 1, " + callsem_local_static_guard_symbol(variable));
    terminate("jump " + lowir_block_name(done_label));

    start_block(done_label);
  }

  void emit_variable_declaration(const CallSemNode & variable)
  {
    if(variable.is_static_storage) {
      if(!callsem_local_static_guard_symbol(variable).empty()) {
        emit_guarded_static_storage_initialization(variable);
      }
      return;
    }

    const bool alias_named_return_slot = is_named_return_slot_variable(variable);
    VariableBinding binding = alias_named_return_slot ?
        create_named_return_slot_binding(variable.text, variable.semantic_type) :
        create_variable_binding(variable.text, variable.semantic_type);
    register_local_binding(variable.text, binding);

    if(variable.children.empty()) {
      if(alias_named_return_slot) {
        register_class_at_ptr_cleanup(variable.semantic_type, binding.external_storage_address);
      }
      return;
    }
    if(alias_named_return_slot) {
      emit_indirect_value_initializer_to_target(variable,
                                                binding.external_storage_address,
                                                false);
      register_class_at_ptr_cleanup(variable.semantic_type, binding.external_storage_address);
      return;
    }
    if(is_reference_type(variable.semantic_type)) {
      if(variable.children.size() != 1) {
        throw logic_error("reference initialization requires single initializer");
      }
      const string object_ptr = emit_lvalue_address(variable.children[0]);
      emit_line("store ptr " + object_ptr + ", " + binding.slots[0]);
      store_reference_hidden_virtual_base_slots(binding, variable.children[0], object_ptr);
      return;
    }
    if(is_indirect_value_type(variable.semantic_type)) {
      emit_indirect_value_initializer_to_target(variable,
                                                emit_storage_address(binding.slots[0]),
                                                true,
                                                binding.slots[0]);
      return;
    }
    if(binding_is_array_storage(binding)) {
      if(variable.children.size() == 1 &&
         variable.children[0].kind == CallSemKind::braced_init_list) {
        push_cleanup_scope(true);
        const CallSemNode & init = variable.children[0];
        emit_local_array_initializer(variable.semantic_type,
                                     init,
                                     emit_storage_address(binding.slots[0]));
        if(current_block_) {
          emit_scope_cleanups(cleanup_scopes_.back());
        }
        pop_cleanup_scope();
        return;
      }
      for(size_t i = 0; i < variable.children.size(); ++i) {
        const CallSemNode & child = variable.children[i];
        if(child.kind == CallSemKind::constructor_action) {
          emit_constructor_action(child);
        } else if(child.kind == CallSemKind::destructor_action) {
          register_bound_local_cleanup(child, variable.text, binding.slots[0]);
        } else {
          throw logic_error("array initialization action shape");
        }
      }
      return;
    }
    push_cleanup_scope(true);
    const string value =
        emit_scalar_storage_value(variable.semantic_type, variable.children[0]);
    const string stored_value =
        emit_debug_named_local_value(variable.text, binding.lowir_type, value);
    emit_line("store " + binding.lowir_type + " " + stored_value + ", " + binding.slots[0]);
    if(current_block_) {
      emit_scope_cleanups(cleanup_scopes_.back());
    }
    pop_cleanup_scope();
  }

  void bind_catch_variable(const CallSemNode & variable,
                           const string & source_ptr,
                           long long source_offset)
  {
    VariableBinding binding = create_variable_binding(variable.text, variable.semantic_type);
    register_local_binding(variable.text, binding);

    const string adjusted_ptr =
        source_offset == 0 ? source_ptr
                           : emit_temp_assignment("ptr",
                                                  string("index i8 ") + source_ptr + ", " +
                                                      to_string(source_offset));

    if(is_reference_type(variable.semantic_type)) {
      emit_line("store ptr " + adjusted_ptr + ", " + binding.slots[0]);
      store_reference_hidden_virtual_base_slots_from_pointer(binding, adjusted_ptr);
      return;
    }

    if(is_indirect_value_type(variable.semantic_type)) {
      emit_copy_construct_to_target(variable.semantic_type,
                                    emit_storage_address(binding.slots[0]),
                                    adjusted_ptr);
      register_class_object_cleanup(binding);
      return;
    }

    const string loaded_value =
        emit_temp_assignment(binding.lowir_type,
                             string("load ") + binding.lowir_type + " " + adjusted_ptr);
    const string stored_value =
        emit_debug_named_local_value(variable.text, binding.lowir_type, loaded_value);
    emit_line("store " + binding.lowir_type + " " + stored_value + ", " + binding.slots[0]);
  }

  void bind_unnamed_catch_object(const TypePtr & type,
                                 const string & source_ptr,
                                 long long source_offset)
  {
    VariableBinding binding = create_variable_binding("__catch", type);
    const string adjusted_ptr =
        source_offset == 0 ? source_ptr
                           : emit_temp_assignment("ptr",
                                                  string("index i8 ") + source_ptr + ", " +
                                                      to_string(source_offset));
    if(is_indirect_value_type(type)) {
      emit_copy_construct_to_target(type,
                                    emit_storage_address(binding.slots[0]),
                                    adjusted_ptr);
      register_class_object_cleanup(binding);
      return;
    }

    emit_line("store " + binding.lowir_type + " " +
              emit_temp_assignment(binding.lowir_type,
                                   string("load ") + binding.lowir_type + " " + adjusted_ptr) +
              ", " + binding.slots[0]);
  }

  void emit_condition(const CallSemNode & condition, string & out_value)
  {
    if(condition.kind != CallSemKind::condition || condition.children.size() != 1) {
      throw logic_error("malformed condition");
    }
    const CallSemNode & child = condition.children[0];
    if(child.kind == CallSemKind::condition_declaration) {
      if(child.children.size() != 1 || child.children[0].kind != CallSemKind::variable) {
        throw logic_error("condition declaration shape");
      }
      emit_variable_declaration(child.children[0]);
      out_value = emit_rvalue(callsem_lowered_condition_test(child) ?
                                  *callsem_lowered_condition_test(child) :
                                  child.children[0]);
      return;
    }
    out_value = emit_rvalue(child);
  }

  void emit_truthy_branch(const CallSemNode & node,
                          const string & true_label,
                          const string & false_label)
  {
    if(node.kind == CallSemKind::binary_expression &&
       node.children.size() == 2 &&
       (callsem_has_token(node, OP_LAND) || callsem_has_token(node, OP_LOR))) {
      const bool is_land = callsem_has_token(node, OP_LAND);
      const string rhs_label = new_block(is_land ? "land_rhs" : "lor_rhs");
      emit_truthy_branch(node.children[0],
                         is_land ? rhs_label : true_label,
                         is_land ? false_label : rhs_label);

      start_block(rhs_label);
      push_cleanup_scope(true);
      const bool needs_cleanup = !cleanup_scopes_.back().empty();
      const string rhs_true_label =
          needs_cleanup ?
              new_block(is_land ? "land_rhs_true_cleanup" : "lor_rhs_true_cleanup") :
              true_label;
      const string rhs_false_label =
          needs_cleanup ?
              new_block(is_land ? "land_rhs_false_cleanup" : "lor_rhs_false_cleanup") :
              false_label;
      emit_truthy_branch(node.children[1], rhs_true_label, rhs_false_label);

      if(needs_cleanup) {
        start_block(rhs_true_label);
        emit_scope_cleanups(cleanup_scopes_.back());
        if(current_block_) {
          terminate("jump " + lowir_block_name(true_label));
        }

        start_block(rhs_false_label);
        emit_scope_cleanups(cleanup_scopes_.back());
        if(current_block_) {
          terminate("jump " + lowir_block_name(false_label));
        }
      }
      pop_cleanup_scope();
      return;
    }

    const string cond_value = emit_branch_condition_value(node);
    terminate("branch " + cond_value + ", " + lowir_block_name(true_label) + ", " +
              lowir_block_name(false_label));
  }

  void emit_condition_branch(const CallSemNode & condition,
                             const string & true_label,
                             const string & false_label)
  {
    if(condition.kind != CallSemKind::condition || condition.children.size() != 1) {
      throw logic_error("malformed condition");
    }
    const CallSemNode & child = condition.children[0];
    if(child.kind == CallSemKind::condition_declaration) {
      if(child.children.size() != 1 || child.children[0].kind != CallSemKind::variable) {
        throw logic_error("condition declaration shape");
      }
      emit_variable_declaration(child.children[0]);
      emit_truthy_branch(callsem_lowered_condition_test(child) ?
                             *callsem_lowered_condition_test(child) :
                             child.children[0],
                         true_label,
                         false_label);
      return;
    }
    emit_truthy_branch(child, true_label, false_label);
  }

  void emit_constructor_action_impl(const CallSemNode & action,
                                    bool register_current_unwind_cleanup = true)
  {
    if(action.kind != CallSemKind::constructor_action || action.children.size() != 1) {
      throw logic_error("invalid constructor lifetime action");
    }
    const CallSemNode & call = action.children[0];
    if(action.trivial_lifecycle) {
      if(call.kind != CallSemKind::call_expression || call.children.size() < 2) {
        throw logic_error("invalid trivial constructor action");
      }
      const CallSemNode & target_arg = call.children[1];
      TypePtr target_type = strip_top_level_cv(target_arg.semantic_type);
      if(!target_type || target_type->kind != Type::TK_POINTER || !target_type->inner) {
        throw logic_error("trivial constructor target must be pointer");
      }
      if(call.children.size() == 2) {
        if(call.value_initializes_result &&
           !is_empty_class_storage_type(target_type->inner)) {
          const string target_ptr = emit_rvalue(target_arg);
          emit_zero_storage_bytes(target_ptr, backend_storage_size(target_type->inner));
        }
        return;
      }
      if(call.children.size() != 3) {
        throw logic_error("unsupported trivial constructor arity");
      }
      const string target_ptr = emit_rvalue(target_arg);
      const CallSemNode & source_arg = call.children[2];
      if(action.has_trivial_storage_copy_prefix) {
        if(!emit_trivial_storage_prefix_copy_to_target(target_type->inner,
                                                       source_arg,
                                                       target_ptr,
                                                       static_cast<size_t>(
                                                           callsem_trivial_storage_copy_prefix_bytes(
                                                               action)))) {
          throw logic_error("invalid trivial storage prefix copy action");
        }
        if(register_current_unwind_cleanup) {
          register_constructor_unwind_cleanup(action);
        }
        return;
      }
      if(target_arg.is_reference_storage_target) {
        TypePtr target_memory_type = strip_top_level_cv(target_arg.semantic_type);
        TypePtr referent_type =
            target_memory_type && target_memory_type->kind == Type::TK_POINTER ?
                strip_top_level_cv(target_memory_type->inner) :
                TypePtr();
        emit_line("store ptr " + emit_reference_storage_value(referent_type, source_arg) +
                  ", " + target_ptr);
        if(register_current_unwind_cleanup) {
          register_constructor_unwind_cleanup(action);
        }
        return;
      }
      if(is_empty_class_storage_type(target_type->inner)) {
        if(register_current_unwind_cleanup) {
          register_constructor_unwind_cleanup(action);
        }
        return;
      }
      if(emit_trivial_class_storage_copy_to_target(target_type->inner, source_arg, target_ptr)) {
        if(register_current_unwind_cleanup) {
          register_constructor_unwind_cleanup(action);
        }
        return;
      }
      emit_storage_value_to_target(target_type->inner, source_arg, target_ptr);
      if(register_current_unwind_cleanup) {
        register_constructor_unwind_cleanup(action);
      }
      return;
    }

    if(call.kind == CallSemKind::call_expression && call.children.size() == 3) {
      const CallSemNode & target_arg = call.children[1];
      const CallSemNode & source_arg = call.children[2];
      TypePtr target_ptr_type = strip_top_level_cv(target_arg.semantic_type);
      TypePtr target_type =
          target_ptr_type && target_ptr_type->kind == Type::TK_POINTER ?
              strip_top_level_cv(target_ptr_type->inner) :
              TypePtr();
      TypePtr source_type = strip_top_level_cv(remove_reference_type(source_arg.semantic_type));
      if(target_type && source_type &&
         is_complete_class_value_type(target_type) &&
         source_arg.value_category == CVC_PRVALUE &&
         type_equals(target_type, source_type) &&
         (source_arg.kind == CallSemKind::call_expression ||
          is_special_class_materialization_node(source_arg))) {
        emit_storage_value_to_target(target_type, source_arg, emit_rvalue(target_arg));
        if(register_current_unwind_cleanup) {
          register_constructor_unwind_cleanup(action);
        }
        return;
      }
    }

    ++constructor_action_depth_;
    emit_rvalue(call);
    --constructor_action_depth_;
    if(register_current_unwind_cleanup) {
      register_constructor_unwind_cleanup(action);
    }
  }

  bool constructor_action_needs_local_unwind_wrapper(const CallSemNode & action) const
  {
    if(!is_constructor_function_ ||
       action.kind != CallSemKind::constructor_action ||
       action.children.size() != 1 ||
       action.trivial_lifecycle ||
       constructor_unwind_cleanups_.empty()) {
      return false;
    }

    const CallSemNode & call = action.children[0];
    if(call.kind != CallSemKind::call_expression || call.children.empty()) {
      return false;
    }

    const string symbol = lookup_function_symbol(call.children[0]);
    return throwing_function_symbols_.count(symbol) != 0;
  }

  void emit_constructor_action(const CallSemNode & action)
  {
    if(action.kind != CallSemKind::constructor_action) {
      throw logic_error("expected constructor_action");
    }

    if(!constructor_action_needs_local_unwind_wrapper(action)) {
      emit_constructor_action_impl(action);
      return;
    }

    const string dispatch_label = new_block("ctor_unwind_dispatch");
    const string end_label = new_block("ctor_unwind_end");
    close_shared_host_call_unwind_region();
    emit_line("eh_try " + lowir_block_name(dispatch_label));
    emit_constructor_action_impl(action, false);
    terminate("jump " + lowir_block_name(end_label));

    start_block(dispatch_label);
    emit_constructor_unwind_cleanups();
    if(!constructor_function_try_dispatch_labels_.empty()) {
      emit_line("eh_end");
      terminate("jump " +
                lowir_block_name(constructor_function_try_dispatch_labels_.back()));
    } else {
      terminate("resume");
    }

    start_block(end_label);
    register_constructor_unwind_cleanup(action);
  }

  bool constructor_action_target_object(const CallSemNode & action,
                                        const CallSemNode *& target_arg,
                                        TypePtr & object_type) const
  {
    target_arg = nullptr;
    object_type = TypePtr();
    if(action.kind != CallSemKind::constructor_action ||
       action.children.size() != 1) {
      return false;
    }
    const CallSemNode & call = action.children[0];
    if(call.kind != CallSemKind::call_expression || call.children.size() < 2) {
      return false;
    }
    const CallSemNode & candidate = call.children[1];
    TypePtr pointer_type = strip_top_level_cv(remove_reference_type(candidate.semantic_type));
    if(!pointer_type || pointer_type->kind != Type::TK_POINTER ||
       !pointer_type->inner) {
      return false;
    }
    TypePtr candidate_type = strip_top_level_cv(pointer_type->inner);
    if(!is_complete_class_value_type(candidate_type)) {
      return false;
    }
    target_arg = &candidate;
    object_type = candidate_type;
    return true;
  }

  bool type_needs_thread_local_destructor_registration(const TypePtr & type) const
  {
    TypePtr base = strip_top_level_cv(remove_reference_type(type));
    if(!base) {
      return false;
    }
    if(base->kind == Type::TK_ARRAY) {
      return base->inner &&
             type_needs_thread_local_destructor_registration(base->inner);
    }
    return is_complete_class_value_type(base) &&
           !destructor_symbol_for_runtime_call(base).empty();
  }

  void emit_thread_local_destructor_registration(const CallSemNode & action)
  {
    if(!action.is_thread_local) {
      return;
    }
    const CallSemNode * target_arg = nullptr;
    TypePtr object_type;
    if(!constructor_action_target_object(action, target_arg, object_type)) {
      return;
    }
    if(!type_needs_thread_local_destructor_registration(object_type)) {
      return;
    }
    emit_thread_local_destructor_registration(object_type, emit_rvalue(*target_arg));
  }

  void emit_thread_local_destructor_registration(const TypePtr & type,
                                                 const string & object_ptr)
  {
    TypePtr base = strip_top_level_cv(remove_reference_type(type));
    if(!base || object_ptr.empty()) {
      return;
    }
    if(base->kind == Type::TK_ARRAY) {
      TypePtr element_type = strip_top_level_cv(base->inner);
      if(!element_type) {
        return;
      }
      const size_t element_size = backend_storage_size(element_type);
      for(size_t i = 0; i < base->bound; ++i) {
        const string element_ptr =
            i == 0 ? object_ptr :
                emit_temp_assignment("ptr",
                                     string("index i8 ") + object_ptr + ", " +
                                         to_string(element_size * i));
        emit_thread_local_destructor_registration(element_type, element_ptr);
      }
      return;
    }
    if(!is_complete_class_value_type(base)) {
      return;
    }
    const string dtor = destructor_symbol_for_runtime_call(base);
    if(dtor.empty()) {
      return;
    }

    const string dtor_ptr = emit_temp_assignment("ptr", string("addr ") + dtor);
    const string dso_handle =
        emit_temp_assignment("ptr",
                             string("addr ") +
                                 external_runtime_object_symbol("__dso_handle"));
    emit_temp_assignment("i32",
                         string("call i32 ") +
                             external_runtime_symbol("__cxa_thread_atexit") + "(" +
                             dtor_ptr + ", " + object_ptr + ", " + dso_handle + ")");
  }

  void emit_action(const CallSemNode & action)
  {
    if(action.children.size() != 1) {
      throw logic_error("invalid lifetime action");
    }
    if(action.kind == CallSemKind::constructor_action &&
       !callsem_local_static_guard_symbol(action).empty()) {
      const string run_label = new_block("local_static_ctor_run");
      const string done_label = new_block("local_static_ctor_done");
      const string guard_value =
          emit_temp_assignment("i64",
                               string("load i64 ") +
                                   callsem_local_static_guard_symbol(action));
      const string initialized =
          emit_temp_assignment("i64", string("cmp ne i64 ") + guard_value + ", 0");
      terminate("branch " + initialized + ", " + lowir_block_name(done_label) + ", " +
                lowir_block_name(run_label));
      start_block(run_label);
      emit_constructor_action(action);
      if(current_block_ && !current_block_->terminated) {
        emit_thread_local_destructor_registration(action);
        emit_line("store i64 1, " + callsem_local_static_guard_symbol(action));
        terminate("jump " + lowir_block_name(done_label));
      }
      start_block(done_label);
      return;
    }
    if(action.kind == CallSemKind::destructor_action &&
       !callsem_local_static_guard_symbol(action).empty()) {
      const string run_label = new_block("local_static_dtor_run");
      const string done_label = new_block("local_static_dtor_done");
      const string guard_value =
          emit_temp_assignment("i64",
                               string("load i64 ") +
                                   callsem_local_static_guard_symbol(action));
      const string initialized =
          emit_temp_assignment("i64", string("cmp ne i64 ") + guard_value + ", 0");
      terminate("branch " + initialized + ", " + lowir_block_name(run_label) + ", " +
                lowir_block_name(done_label));
      start_block(run_label);
      emit_rvalue(action.children[0]);
      terminate("jump " + lowir_block_name(done_label));
      start_block(done_label);
      return;
    }
    if(action.kind == CallSemKind::constructor_action) {
      emit_constructor_action(action);
      return;
    }
    if(action.kind == CallSemKind::destructor_action) {
      if(action.trivial_lifecycle) {
        return;
      }
    }
    if(action.kind == CallSemKind::vptr_action) {
      const string object_ptr = emit_rvalue(action.children[0]);
      const string table_ptr = emit_vptr_action_address_point(action);
      emit_line("store ptr " + table_ptr + ", " + object_ptr);
      return;
    }
    emit_rvalue(action.children[0]);
  }

  void collect_switch_labels(const CallSemNode & node,
                             vector<const CallSemNode *> & cases,
                             const CallSemNode *& default_node)
  {
    if(node.kind == CallSemKind::switch_statement) {
      return;
    }

    if(node.kind == CallSemKind::case_statement) {
      cases.push_back(&node);
      if(node.children.size() == 2) {
        collect_switch_labels(node.children[1], cases, default_node);
      }
      return;
    }

    if(node.kind == CallSemKind::default_statement) {
      default_node = &node;
      if(node.children.size() == 1) {
        collect_switch_labels(node.children[0], cases, default_node);
      }
      return;
    }

    for(size_t i = 0; i < node.children.size(); ++i) {
      collect_switch_labels(node.children[i], cases, default_node);
    }
  }

  string emit_switch_value(const CallSemNode & condition)
  {
    if(condition.kind != CallSemKind::condition || condition.children.size() != 1) {
      throw logic_error("malformed switch condition");
    }

    const CallSemNode & child = condition.children[0];
    if(child.kind == CallSemKind::condition_declaration) {
      if(child.children.size() != 1 || child.children[0].kind != CallSemKind::variable) {
        throw logic_error("switch condition declaration shape");
      }
      emit_variable_declaration(child.children[0]);
      return emit_rvalue(callsem_lowered_condition_test(child) ?
                             *callsem_lowered_condition_test(child) :
                             child.children[0]);
    }

    return emit_rvalue(child);
  }

  void start_labeled_block(const string & label)
  {
    if(current_block_) {
      terminate("jump " + lowir_block_name(label));
    }
    start_block(label);
  }

  string ensure_goto_target(const string & name)
  {
    map<string, string>::const_iterator found = goto_targets_.find(name);
    if(found != goto_targets_.end()) {
      return found->second;
    }
    const string label = new_block("goto");
    goto_targets_[name] = label;
    return label;
  }

  void emit_switch_body(const CallSemNode & node,
                        const map<const CallSemNode *, string> & labels)
  {
    if(node.kind == CallSemKind::compound_statement) {
      push_cleanup_scope();
      push_binding_scope();
      for(size_t i = 0; i < node.children.size(); ++i) {
        emit_switch_body(node.children[i], labels);
      }
      if(current_block_) {
        emit_scope_cleanups(cleanup_scopes_.back());
      }
      pop_cleanup_scope();
      pop_binding_scope();
      return;
    }

    if(node.kind == CallSemKind::case_statement) {
      map<const CallSemNode *, string>::const_iterator found = labels.find(&node);
      if(found == labels.end()) {
        throw logic_error("missing switch case label");
      }
      if(node.children.size() != 2) {
        throw logic_error("malformed case-statement");
      }
      start_labeled_block(found->second);
      emit_switch_body(node.children[1], labels);
      return;
    }

    if(node.kind == CallSemKind::default_statement) {
      map<const CallSemNode *, string>::const_iterator found = labels.find(&node);
      if(found == labels.end()) {
        throw logic_error("missing switch default label");
      }
      if(node.children.size() != 1) {
        throw logic_error("malformed default-statement");
      }
      start_labeled_block(found->second);
      emit_switch_body(node.children[0], labels);
      return;
    }

    emit_statement(node);
  }

  void emit_statement(const CallSemNode & node)
  {
    ScopedLowIRCurrentStatement current_stmt(node);
    if(!current_block_ && node.kind != CallSemKind::translation_unit) {
      start_block(new_block("block"));
    }

    if(active_switch_labels_ && node.kind == CallSemKind::case_statement) {
      map<const CallSemNode *, string>::const_iterator found =
          active_switch_labels_->find(&node);
      if(found == active_switch_labels_->end()) {
        throw logic_error("missing switch case label");
      }
      if(node.children.size() != 2) {
        throw logic_error("malformed case-statement");
      }
      start_labeled_block(found->second);
      emit_statement(node.children[1]);
      return;
    }

    if(active_switch_labels_ && node.kind == CallSemKind::default_statement) {
      map<const CallSemNode *, string>::const_iterator found =
          active_switch_labels_->find(&node);
      if(found == active_switch_labels_->end()) {
        throw logic_error("missing switch default label");
      }
      if(node.children.size() != 1) {
        throw logic_error("malformed default-statement");
      }
      start_labeled_block(found->second);
      emit_statement(node.children[0]);
      return;
    }

    if(node.kind == CallSemKind::compound_statement ||
       node.kind == CallSemKind::then_node ||
       node.kind == CallSemKind::else_node) {
      push_cleanup_scope();
      push_binding_scope();
      for(size_t i = 0; i < node.children.size(); ++i) {
        emit_statement(node.children[i]);
      }
      if(current_block_) {
        emit_scope_cleanups(cleanup_scopes_.back());
      }
      pop_cleanup_scope();
      pop_binding_scope();
      return;
    }

    if(node.kind == CallSemKind::for_init_statement) {
      for(size_t i = 0; i < node.children.size(); ++i) {
        const CallSemNode & child = node.children[i];
        if(child.kind == CallSemKind::simple_declaration ||
           child.kind == CallSemKind::type_alias) {
          emit_statement(child);
        } else {
          push_cleanup_scope(true);
          emit_discarded_expression(child);
          if(current_block_) {
            emit_scope_cleanups(cleanup_scopes_.back());
          }
          pop_cleanup_scope();
        }
      }
      return;
    }

    if(node.kind == CallSemKind::simple_declaration) {
      for(size_t i = 0; i < node.children.size(); ++i) {
        if(node.children[i].kind == CallSemKind::variable) {
          emit_variable_declaration(node.children[i]);
        }
      }
      return;
    }

    if(node.kind == CallSemKind::type_alias) {
      return;
    }

    if(node.kind == CallSemKind::expression_statement) {
      if(!node.children.empty()) {
        push_cleanup_scope(true);
        emit_discarded_expression(node.children[0]);
        if(current_block_) {
          emit_scope_cleanups(cleanup_scopes_.back());
        }
        pop_cleanup_scope();
      }
      return;
    }

    if(node.kind == CallSemKind::asm_statement) {
      ostringstream out;
      out << "GNU asm statement lowering unsupported in PA32 host compatibility";
      if(!node.text.empty()) {
        out << " [text " << node.text << "]";
      }
      throw logic_error(out.str());
    }

    if(node.kind == CallSemKind::constructor_action ||
       node.kind == CallSemKind::destructor_action ||
       node.kind == CallSemKind::vptr_action) {
      emit_action(node);
      return;
    }

    if(node.kind == CallSemKind::return_statement) {
      if(!node.children.empty()) {
        push_cleanup_scope(true);
        if(indirect_class_return_) {
          const string target_ptr = function_.params[0].name;
          const string alias_ptr = named_return_slot_alias_address(node.children[0]);
          if(!alias_ptr.empty()) {
            if(current_block_) {
              emit_scope_cleanups(cleanup_scopes_.back());
            }
            pop_cleanup_scope();
            emit_all_cleanups_excluding_destroy_at_ptr(alias_ptr);
            terminate("return void");
            return;
          }
          emit_return_object_value_to_target(node.children[0], target_ptr);
          if(!current_block_) {
            pop_cleanup_scope();
            return;
          }
          emit_scope_cleanups(cleanup_scopes_.back());
          pop_cleanup_scope();
          emit_all_cleanups();
          terminate("return void");
          return;
        }
        if(direct_object_return_) {
          const string result_slot = ensure_direct_object_return_slot();
          emit_return_object_value_to_target(node.children[0], emit_storage_address(result_slot));
          if(!current_block_) {
            pop_cleanup_scope();
            return;
          }
          emit_scope_cleanups(cleanup_scopes_.back());
          pop_cleanup_scope();
          emit_all_cleanups();
          terminate("return " + function_.return_type + " " + result_slot);
          return;
        }
        if(function_.return_type == "void") {
          emit_discarded_expression(node.children[0]);
          if(!current_block_) {
            pop_cleanup_scope();
            return;
          }
          emit_scope_cleanups(cleanup_scopes_.back());
          pop_cleanup_scope();
          emit_all_cleanups();
          terminate("return void");
          return;
        }
        const string value =
            is_reference_type(function_result_type_) ?
                emit_reference_return_address(node.children[0]) :
                emit_scalar_storage_value(function_result_type_, node.children[0]);
        if(!current_block_) {
          pop_cleanup_scope();
          return;
        }
        emit_scope_cleanups(cleanup_scopes_.back());
        pop_cleanup_scope();
        emit_all_cleanups();
        terminate("return " + function_.return_type + " " + value);
        return;
      }
      emit_all_cleanups();
      if(node.children.empty()) {
        terminate("return void");
      }
      return;
    }

    if(node.kind == CallSemKind::throw_statement) {
      if(node.children.size() > 1) {
        throw logic_error("invalid throw-statement");
      }
      const bool crosses_try_boundary = !throw_will_escape_current_function();
      if(node.children.empty()) {
        if(use_host_eh_runtime()) {
          emit_explicit_host_throw_cleanups();
          if(is_constructor_function_ && throw_will_escape_current_function()) {
            emit_constructor_unwind_cleanups();
          }
          emit_line("call void " + external_runtime_symbol("__cxa_rethrow") + "()");
          emit_noreturn_fallback_return();
        } else {
          emit_all_cleanups(true);
          if(is_constructor_function_ && throw_will_escape_current_function()) {
            emit_constructor_unwind_cleanups();
          }
          terminate("resume");
        }
        return;
      }

      const bool use_host_throw = use_host_eh_runtime();
      string throw_value;
      if(use_host_throw) {
        throw_value = emit_host_throw_value(node.children[0]);
        emit_explicit_host_throw_cleanups();
        if(is_constructor_function_ && throw_will_escape_current_function()) {
          emit_constructor_unwind_cleanups();
        }
      } else {
        string lowir_throw_type;
        throw_value = emit_private_throw_value(node.children[0], lowir_throw_type);
        emit_all_cleanups(true);
        if(is_constructor_function_ && throw_will_escape_current_function()) {
          emit_constructor_unwind_cleanups();
        }
      }
      if(use_host_throw) {
        const TypePtr throw_type = exception_object_type(node.children[0].semantic_type);
        if(!throw_type) {
          throw logic_error("throw-statement missing exception object type");
        }
        const string throw_rtti = emit_host_rtti_symbol_address(throw_type);
        const string throw_destructor = emit_host_throw_destructor(throw_type);
        emit_line("call void " + external_runtime_symbol("__cxa_throw") + "(" +
                  throw_value + ", " +
                  throw_rtti + ", " +
                  throw_destructor + ")");
        if(crosses_try_boundary) {
          close_shared_host_call_unwind_region();
          emit_line("eh_end");
        }
        emit_noreturn_fallback_return();
      } else {
        terminate("throw ptr " + throw_value);
      }
      return;
    }

    if(node.kind == CallSemKind::try_statement) {
      const CallSemNode * try_body = find_child(node, CallSemKind::compound_statement);
      if(!try_body) {
        throw logic_error("try-statement missing body");
      }
      const bool constructor_function_try =
          is_constructor_function_ && node.text == "constructor_function_try";

      const string dispatch_label = new_block("catch_dispatch");
      const string handler_entry_label =
          (constructor_function_try || use_host_eh_runtime()) ?
              new_block("catch_entry") :
              dispatch_label;
      const string end_label = new_block("try_end");
      const size_t host_try_target_depth = host_eh_region_depth_;
      close_shared_host_call_unwind_region();
      emit_line("eh_try " + lowir_block_name(dispatch_label));
      if(use_host_eh_runtime()) {
        ++host_eh_region_depth_;
      }

      push_cleanup_scope();
      register_eh_end_cleanup();
      if(constructor_function_try) {
        constructor_function_try_dispatch_labels_.push_back(handler_entry_label);
      }
      if(use_host_eh_runtime()) {
        host_eh_dispatch_labels_.push_back(handler_entry_label);
        host_eh_dispatch_depths_.push_back(host_try_target_depth);
        host_eh_handler_nodes_.push_back(&node);
      }
      emit_statement(*try_body);
      if(use_host_eh_runtime()) {
        if(host_eh_region_depth_ == 0) {
          throw logic_error("host EH region depth underflow");
        }
        --host_eh_region_depth_;
        host_eh_handler_nodes_.pop_back();
        host_eh_dispatch_depths_.pop_back();
        host_eh_dispatch_labels_.pop_back();
      }
      if(constructor_function_try) {
        constructor_function_try_dispatch_labels_.pop_back();
      }
      const bool try_fallthrough = current_block_ != nullptr;
      if(try_fallthrough) {
        emit_scope_cleanups(cleanup_scopes_.back());
        pop_cleanup_scope();
        terminate("jump " + lowir_block_name(end_label));
      } else {
        pop_cleanup_scope();
      }

      start_block(dispatch_label);
      emit_host_eh_handler_metadata(node);
      if(handler_entry_label != dispatch_label) {
        terminate("jump " + lowir_block_name(handler_entry_label));
        start_block(handler_entry_label);
      }
      const string current_storage = emit_current_exception_storage();
      string current_type;
      string current_selector;
      string next_label = dispatch_label;
      bool have_handler = false;
      size_t catch_handler_count = 0;
      size_t typed_catch_handler_count = 0;
      size_t catch_all_handler_count = 0;
      for(size_t i = 0; i < node.children.size(); ++i) {
        if(node.children[i].kind != CallSemKind::catch_handler) {
          continue;
        }
        ++catch_handler_count;
        if(node.children[i].text == "...") {
          ++catch_all_handler_count;
        } else {
          ++typed_catch_handler_count;
        }
      }
      const bool use_host_selector_dispatch =
          use_host_eh_runtime() &&
          catch_handler_count == typed_catch_handler_count + catch_all_handler_count &&
          catch_handler_count != 0;
      const vector<long long> * host_handler_selectors =
          use_host_selector_dispatch ? &host_eh_selectors_for_try(node) : nullptr;
      size_t host_handler_selector_index = 0;

      for(size_t i = 0; i < node.children.size(); ++i) {
        const CallSemNode & handler = node.children[i];
        if(handler.kind != CallSemKind::catch_handler) {
          continue;
        }
        have_handler = true;
        if(next_label != dispatch_label) {
          start_block(next_label);
        }

        const CallSemNode * body = find_child(handler, CallSemKind::compound_statement);
        const CallSemNode * variable = find_child(handler, CallSemKind::variable);
        const string body_label = new_block("catch_body");
        const string miss_label = new_block("catch_next");
        const bool materialize_unnamed_catch_object =
            !variable &&
            handler.semantic_type &&
            !is_reference_type(handler.semantic_type) &&
            is_indirect_value_type(handler.semantic_type);
        const bool needs_match_ptr =
            variable || materialize_unnamed_catch_object;
        const string match_ptr_slot =
            needs_match_ptr ? new_hidden_slot("ptr", "catch") : string();
        long long host_handler_selector = 0;
        if(use_host_selector_dispatch) {
          if(!host_handler_selectors ||
             host_handler_selector_index >= host_handler_selectors->size()) {
            throw logic_error("host EH selector dispatch mismatch");
          }
          host_handler_selector = (*host_handler_selectors)[host_handler_selector_index];
        }
        ++host_handler_selector_index;

        if(use_host_selector_dispatch) {
          if(current_selector.empty()) {
            current_selector = emit_current_exception_selector();
          }
          const string match =
              emit_temp_assignment("i64",
                                   string("cmp eq i32 ") + current_selector + ", " +
                                       to_string(host_handler_selector));
          if(needs_match_ptr) {
            const string hit_label = new_block("catch_match");
            terminate("branch " + match + ", " + lowir_block_name(hit_label) + ", " +
                      lowir_block_name(miss_label));
            start_block(hit_label);
            emit_line("store ptr " + current_storage + ", " + match_ptr_slot);
            terminate("jump " + lowir_block_name(body_label));
          } else {
            terminate("branch " + match + ", " + lowir_block_name(body_label) + ", " +
                      lowir_block_name(miss_label));
          }
        } else if(handler.text == "...") {
          if(needs_match_ptr) {
            emit_line("store ptr " + current_storage + ", " + match_ptr_slot);
          }
          terminate("jump " + lowir_block_name(body_label));
        } else {
          if(current_type.empty()) {
            current_type = emit_current_exception_type();
          }
          TypePtr match_type = exception_object_type(handler.semantic_type);
          bool emitted_match = false;
          for(size_t j = 0; j < handler.children.size(); ++j) {
            const CallSemNode & candidate = handler.children[j];
            if(candidate.kind != CallSemKind::rtti_candidate || !candidate.semantic_type) {
              continue;
            }
            emitted_match = true;
            const string match =
                emit_temp_assignment("i64",
                                     string("cmp eq ptr ") + current_type + ", " +
                                         emit_exception_match_rtti_address(candidate.semantic_type));
            const string hit_label = new_block("catch_match");
            const string next_candidate_label = new_block("catch_scan");
            terminate("branch " + match + ", " + lowir_block_name(hit_label) + ", " +
                      lowir_block_name(next_candidate_label));

            start_block(hit_label);
            if(needs_match_ptr) {
              const long long offset =
                  candidate.has_int_value ? callsem_int_value(candidate) : 0;
              const string adjusted =
                  offset == 0 ? current_storage
                              : emit_temp_assignment("ptr",
                                                     string("index i8 ") + current_storage + ", " +
                                                         to_string(offset));
              emit_line("store ptr " + adjusted + ", " + match_ptr_slot);
            }
            terminate("jump " + lowir_block_name(body_label));
            start_block(next_candidate_label);
          }

          if(!emitted_match) {
            if(!match_type) {
              throw logic_error("catch handler missing type");
            }
            const string match =
                emit_temp_assignment("i64",
                                     string("cmp eq ptr ") + current_type + ", " +
                                         emit_exception_match_rtti_address(match_type));
            if(needs_match_ptr) {
              const string hit_label = new_block("catch_match");
              terminate("branch " + match + ", " + lowir_block_name(hit_label) + ", " +
                        lowir_block_name(miss_label));
              start_block(hit_label);
              emit_line("store ptr " + current_storage + ", " + match_ptr_slot);
              terminate("jump " + lowir_block_name(body_label));
            } else {
              terminate("branch " + match + ", " + lowir_block_name(body_label) + ", " +
                        lowir_block_name(miss_label));
            }
          } else {
            terminate("jump " + lowir_block_name(miss_label));
          }
        }

        start_block(body_label);
        const string cleanup_label =
            use_host_eh_runtime() ? new_block("catch_cleanup") : string();
        const size_t catch_cleanup_depth =
            use_host_eh_runtime() ? host_eh_region_depth_ + 1 : 0;
        push_cleanup_scope();
        push_binding_scope();
        if(use_host_eh_runtime()) {
          emit_line("eh_cleanup " + lowir_block_name(cleanup_label));
          active_host_cleanup_labels_.push_back(cleanup_label);
          mark_current_cleanup_scope_host_unwind_cleanup();
          register_pre_scope_eh_end_cleanup();
        }
        register_clear_exception_cleanup();
        if(variable) {
          bind_catch_variable(*variable,
                              emit_temp_assignment("ptr", string("load ptr ") + match_ptr_slot),
                              0);
        } else if(materialize_unnamed_catch_object) {
          bind_unnamed_catch_object(handler.semantic_type,
                                    emit_temp_assignment("ptr",
                                                         string("load ptr ") + match_ptr_slot),
                                    0);
        }
        if(body) {
          emit_statement(*body);
        }
        const bool body_fallthrough = current_block_ != nullptr;
        const vector<CleanupAction> body_cleanups = cleanup_scopes_.back();
        const size_t body_normal_eh_end_count = cleanup_scope_normal_eh_end_counts_.back();
        if(body_fallthrough) {
          if(constructor_function_try) {
            if(use_host_eh_runtime()) {
              emit_explicit_host_throw_cleanups();
              emit_line("call void " + external_runtime_symbol("__cxa_rethrow") + "()");
              emit_noreturn_fallback_return();
            } else {
              emit_normal_scope_cleanups(body_cleanups, body_normal_eh_end_count);
              terminate("resume");
            }
          } else {
            emit_normal_scope_cleanups(body_cleanups, body_normal_eh_end_count);
            terminate("jump " + lowir_block_name(end_label));
          }
        }
        if(use_host_eh_runtime()) {
          start_block(cleanup_label);
          if(!host_eh_handler_nodes_.empty() && host_eh_handler_nodes_.back()) {
            emit_host_eh_handler_metadata(*host_eh_handler_nodes_.back());
          }
          emit_scope_cleanups(body_cleanups);
          terminate_host_eh_dispatch_or_resume(catch_cleanup_depth);
          if(active_host_cleanup_labels_.empty() ||
             active_host_cleanup_labels_.back() != cleanup_label) {
            throw logic_error("host EH cleanup label stack mismatch");
          }
          active_host_cleanup_labels_.pop_back();
        }
        pop_cleanup_scope();
        pop_binding_scope();

        next_label = miss_label;
      }

      if(!have_handler) {
        throw logic_error("try-statement missing handlers");
      }

      start_block(next_label);
      terminate("resume");
      start_block(end_label);
      return;
    }

    if(node.kind == CallSemKind::if_statement) {
      push_cleanup_scope();
      push_binding_scope();
      for(size_t i = 0; i < node.children.size(); ++i) {
        if(node.children[i].kind == CallSemKind::for_init_statement) {
          emit_statement(node.children[i]);
        }
      }
      const CallSemNode * condition = find_child(node, CallSemKind::condition);
      const CallSemNode * then_branch = find_child(node, CallSemKind::then_node);
      const CallSemNode * else_branch = find_child(node, CallSemKind::else_node);
      if(!condition || !then_branch) {
        throw logic_error("malformed if-statement");
      }
      const string then_label = new_block("if_then");
      const string else_label = new_block("if_else");
      const string end_label = new_block("if_end");
      emit_condition_branch(*condition, then_label, else_label);

      bool then_fallthrough = false;
      start_block(then_label);
      emit_statement(*then_branch);
      if(current_block_) {
        terminate("jump " + lowir_block_name(end_label));
        then_fallthrough = true;
      }

      bool else_fallthrough = false;
      start_block(else_label);
      if(else_branch) {
        emit_statement(*else_branch);
      }
      if(current_block_) {
        terminate("jump " + lowir_block_name(end_label));
        else_fallthrough = true;
      }

      if(then_fallthrough || else_fallthrough) {
        start_block(end_label);
        emit_scope_cleanups(cleanup_scopes_.back());
      } else {
        current_block_ = nullptr;
      }
      pop_cleanup_scope();
      pop_binding_scope();
      return;
    }

    if(node.kind == CallSemKind::switch_statement) {
      push_binding_scope();
      if(node.children.size() != 2 || node.children[0].kind != CallSemKind::condition) {
        throw logic_error("malformed switch-statement");
      }

      const string selector = emit_switch_value(node.children[0]);
      const string dispatch_label = new_block("switch_dispatch");
      const string end_label = new_block("switch_end");
      terminate("jump " + lowir_block_name(dispatch_label));

      vector<const CallSemNode *> cases;
      const CallSemNode * default_node = nullptr;
      collect_switch_labels(node.children[1], cases, default_node);
      map<const CallSemNode *, string> labels;
      for(size_t i = 0; i < cases.size(); ++i) {
        labels[cases[i]] = new_block("switch_case");
      }
      if(default_node) {
        labels[default_node] = new_block("switch_default");
      }

      start_block(dispatch_label);
      ostringstream dispatch;
      dispatch << "switch " << selector << ", "
               << lowir_block_name(default_node ? labels[default_node] : end_label);
      for(size_t i = 0; i < cases.size(); ++i) {
        dispatch << ", " << emit_rvalue(cases[i]->children[0]) << ":"
                 << lowir_block_name(labels[cases[i]]);
      }
      terminate(dispatch.str());

      break_targets_.push_back(ControlTransferTarget{lowir_block_name(end_label),
                                                     cleanup_scopes_.size()});
      const map<const CallSemNode *, string> * previous_switch_labels =
          active_switch_labels_;
      active_switch_labels_ = &labels;
      emit_switch_body(node.children[1], labels);
      active_switch_labels_ = previous_switch_labels;
      if(current_block_) {
        terminate("jump " + lowir_block_name(end_label));
      }
      break_targets_.pop_back();
      start_block(end_label);
      pop_binding_scope();
      return;
    }

    if(node.kind == CallSemKind::while_statement) {
      push_binding_scope();
      if(node.children.size() != 2) {
        throw logic_error("malformed while-statement");
      }
      const string cond_label = new_block("while_cond");
      const string body_label = new_block("while_body");
      const string end_label = new_block("while_end");
      terminate("jump " + lowir_block_name(cond_label));
      start_block(cond_label);
      emit_condition_branch(node.children[0], body_label, end_label);
      break_targets_.push_back(ControlTransferTarget{lowir_block_name(end_label),
                                                     cleanup_scopes_.size()});
      continue_targets_.push_back(ControlTransferTarget{lowir_block_name(cond_label),
                                                        cleanup_scopes_.size()});
      start_block(body_label);
      emit_statement(node.children[1]);
      if(current_block_) {
        terminate("jump " + lowir_block_name(cond_label));
      }
      continue_targets_.pop_back();
      break_targets_.pop_back();
      start_block(end_label);
      pop_binding_scope();
      return;
    }

    if(node.kind == CallSemKind::do_statement) {
      if(node.children.size() != 2) {
        throw logic_error("malformed do-statement");
      }
      const string body_label = new_block("do_body");
      const string cond_label = new_block("do_cond");
      const string end_label = new_block("do_end");
      terminate("jump " + lowir_block_name(body_label));
      break_targets_.push_back(ControlTransferTarget{lowir_block_name(end_label),
                                                     cleanup_scopes_.size()});
      continue_targets_.push_back(ControlTransferTarget{lowir_block_name(cond_label),
                                                        cleanup_scopes_.size()});
      start_block(body_label);
      emit_statement(node.children[0]);
      if(current_block_) {
        terminate("jump " + lowir_block_name(cond_label));
      }
      start_block(cond_label);
      emit_condition_branch(node.children[1], body_label, end_label);
      continue_targets_.pop_back();
      break_targets_.pop_back();
      start_block(end_label);
      return;
    }

    if(node.kind == CallSemKind::for_statement) {
      push_cleanup_scope();
      push_binding_scope();
      size_t index = 0;
      if(index < node.children.size() &&
         node.children[index].kind == CallSemKind::for_init_statement) {
        emit_statement(node.children[index]);
        ++index;
      }
      const string cond_label = new_block("for_cond");
      const string body_label = new_block("for_body");
      const string iter_label = new_block("for_iter");
      const string end_label = new_block("for_end");
      terminate("jump " + lowir_block_name(cond_label));

      start_block(cond_label);
      if(index < node.children.size() && node.children[index].kind == CallSemKind::condition) {
        emit_condition_branch(node.children[index], body_label, end_label);
        ++index;
      } else {
        terminate("jump " + lowir_block_name(body_label));
      }

      const CallSemNode * iteration = nullptr;
      if(index < node.children.size() && node.children[index].kind == CallSemKind::iteration) {
        iteration = &node.children[index];
        ++index;
      }
      if(index >= node.children.size()) {
        throw logic_error("for-statement missing body");
      }

      break_targets_.push_back(ControlTransferTarget{lowir_block_name(end_label),
                                                     cleanup_scopes_.size()});
      continue_targets_.push_back(ControlTransferTarget{lowir_block_name(iter_label),
                                                        cleanup_scopes_.size()});
      start_block(body_label);
      emit_statement(node.children[index]);
      if(current_block_) {
        terminate("jump " + lowir_block_name(iter_label));
      }

      start_block(iter_label);
      if(iteration && iteration->children.size() == 1) {
        emit_discarded_expression(iteration->children[0]);
      }
      if(current_block_) {
        terminate("jump " + lowir_block_name(cond_label));
      }
      continue_targets_.pop_back();
      break_targets_.pop_back();
      start_block(end_label);
      if(current_block_) {
        emit_scope_cleanups(cleanup_scopes_.back());
      }
      pop_cleanup_scope();
      pop_binding_scope();
      return;
    }

    if(node.kind == CallSemKind::break_statement) {
      if(break_targets_.empty()) {
        throw logic_error("break outside loop");
      }
      emit_cleanups_to_depth(break_targets_.back().cleanup_depth);
      terminate("jump " + break_targets_.back().label);
      return;
    }

    if(node.kind == CallSemKind::continue_statement) {
      if(continue_targets_.empty()) {
        throw logic_error("continue outside loop");
      }
      emit_cleanups_to_depth(continue_targets_.back().cleanup_depth);
      terminate("jump " + continue_targets_.back().label);
      return;
    }

    if(node.kind == CallSemKind::goto_statement) {
      terminate("jump " + lowir_block_name(ensure_goto_target(node.text)));
      return;
    }

    if(node.kind == CallSemKind::labeled_statement) {
      if(node.children.size() != 1) {
        throw logic_error("malformed labeled-statement");
      }
      start_labeled_block(ensure_goto_target(node.text));
      emit_statement(node.children[0]);
      return;
    }

    ostringstream out;
    out << "unsupported statement in PA14 LowIR lowering";
    out << " [kind " << callsem_kind_text(node.kind) << "]";
    if(!node.text.empty()) {
      out << " [text " << node.text << "]";
    }
    if(node.semantic_type) {
      out << " [type " << describe_type(node.semantic_type) << "]";
    }
    out << " [child_count " << node.children.size() << "]";
    throw logic_error(out.str());
  }
};

class ProgramGenerator
{
public:
  explicit ProgramGenerator(const vector<CallSemNode> & translation_units,
                            bool validate_closure,
                            bool emit_runtime_support,
                            bool enable_debug_value_names)
    : translation_units_(translation_units),
      validate_closure_(validate_closure),
      emit_runtime_support_(emit_runtime_support),
      enable_debug_value_names_(enable_debug_value_names)
  {}

  lowir_internal::Program build_program()
  {
    collect();

    for(size_t i = 0; i < thread_local_init_actions_.size(); ++i) {
      vector<const CallSemNode *> actions;
      actions.push_back(thread_local_init_actions_[i].second);
      referenced_function_symbols_.insert(thread_local_init_actions_[i].first);
      functions_.push_back(
          LowIRFunctionBuilder(thread_local_init_actions_[i].first,
                               global_bindings_,
                               vtable_bindings_,
                               function_symbols_,
                               function_symbol_entries_,
                               function_symbol_lookup_index(),
                               function_symbol_nodes_,
                               c_linkage_function_symbols_,
                               function_virtual_base_layouts_,
                               class_virtual_base_layouts_,
                               function_parameter_virtual_base_layouts_,
                               classes_with_virtual_functions_,
                               throwing_function_symbols_,
                               rtti_definition_symbols_,
                               string_literal_symbols_,
                               exception_storage_types_,
                               virtual_member_pointer_thunks_,
                               external_function_symbols_,
                               external_object_symbols_,
                               runtime_bridge_support_symbols_,
                               referenced_function_symbols_,
                               referenced_function_signature_types_,
                               function_references_,
                               emit_runtime_support_,
                               enable_debug_value_names_)
              .build_actions(actions));
    }

    if(!global_ctor_actions_.empty()) {
      referenced_function_symbols_.insert(lowir_name("__cppgm_init"));
      functions_.push_back(
          LowIRFunctionBuilder("__cppgm_init", global_bindings_, vtable_bindings_,
                               function_symbols_, function_symbol_entries_,
                               function_symbol_lookup_index(),
                               function_symbol_nodes_,
                               c_linkage_function_symbols_,
                               function_virtual_base_layouts_,
                               class_virtual_base_layouts_,
                               function_parameter_virtual_base_layouts_,
                               classes_with_virtual_functions_,
                               throwing_function_symbols_,
                               rtti_definition_symbols_,
                               string_literal_symbols_,
                               exception_storage_types_,
                               virtual_member_pointer_thunks_,
                               external_function_symbols_,
                               external_object_symbols_,
                               runtime_bridge_support_symbols_,
                               referenced_function_symbols_,
                               referenced_function_signature_types_,
                               function_references_,
                               emit_runtime_support_,
                               enable_debug_value_names_)
              .build_actions(global_ctor_actions_));
    }
    if(!global_dtor_actions_.empty()) {
      vector<const CallSemNode *> actions;
      for(size_t i = global_dtor_actions_.size(); i-- > 0;) {
        actions.push_back(global_dtor_actions_[i]);
      }
      referenced_function_symbols_.insert(lowir_name("__cppgm_fini"));
      functions_.push_back(
          LowIRFunctionBuilder("__cppgm_fini", global_bindings_, vtable_bindings_,
                               function_symbols_, function_symbol_entries_,
                               function_symbol_lookup_index(),
                               function_symbol_nodes_,
                               c_linkage_function_symbols_,
                               function_virtual_base_layouts_,
                               class_virtual_base_layouts_,
                               function_parameter_virtual_base_layouts_,
                               classes_with_virtual_functions_,
                               throwing_function_symbols_,
                               rtti_definition_symbols_,
                               string_literal_symbols_,
                               exception_storage_types_,
                               virtual_member_pointer_thunks_,
                               external_function_symbols_,
                               external_object_symbols_,
                               runtime_bridge_support_symbols_,
                               referenced_function_symbols_,
                               referenced_function_signature_types_,
                               function_references_,
                               emit_runtime_support_,
                               enable_debug_value_names_)
              .build_actions(actions));
    }
    emit_referenced_output_on_use_function_definitions();
    synthesize_referenced_internal_global_definitions();
    emit_runtime_bridge_support_functions();
    register_external_symbol_aliases();
    if(validate_closure_) {
      validate_symbol_closure();
    }

    lowir_internal::Program program;
    collect_lowir_declarations(program);
    for(size_t i = 0; i < globals_.size(); ++i) {
      const LowIRGlobal & global = globals_[i];
      lowir_internal::GlobalDefinition out;
      out.name = global.name;
      out.storage = global.thread_local_storage ? lowir_internal::GSM_THREAD_LOCAL
                    : global.readonly ? lowir_internal::GSM_READONLY
                                      : lowir_internal::GSM_DEFAULT;
      out.metadata = metadata_with_role_linkage_and_binding(
          special_global_role_for_symbol(global.name),
          global_symbol_is_c_linkage(global.name),
          binding_for_defined_symbol(global.name));
      if(global.kind == LowIRGlobal::LG_DATA) {
        out.structured = true;
        for(size_t j = 0; j < global.data_items.size(); ++j) {
          out.data_items.push_back(
              lowir_internal::parse_global_data_item_text(global.data_items[j], global.name));
        }
      } else {
        out.type = lowir_internal::parse_type_text(global.type, global.name);
        if(global.is_addr) {
          out.init_kind = lowir_internal::GlobalDefinition::INIT_ADDR;
          out.init_operand = lowir_internal::parse_operand_text(global.value, global.name);
          out.addr_addend = global.addr_addend;
        } else if(global.value == "zero") {
          out.init_kind = lowir_internal::GlobalDefinition::INIT_ZERO;
        } else {
          out.init_kind = lowir_internal::GlobalDefinition::INIT_INTEGER;
          out.init_operand = lowir_internal::parse_operand_text(global.value, global.name);
        }
      }
      program.globals.push_back(out);
    }
    for(size_t i = 0; i < functions_.size(); ++i) {
      const LowIRFunction & function = functions_[i];
      lowir_internal::Function out;
      out.name = function.name;
      out.debug_location = function.debug_location;
      out.boundary = function.boundary_metadata;
      out.metadata = metadata_with_role_linkage_and_binding(
          special_function_role_for_symbol(function.name),
          function_symbol_is_c_linkage(function.name),
          binding_for_defined_symbol(function.name));
      out.metadata.object_trivial_lifecycle =
          function.metadata.object_trivial_lifecycle;
      for(size_t pi = 0; pi < function.params.size(); ++pi) {
        out.params.push_back(
            lowir_internal::Parameter{function.params[pi].name,
                                      lowir_internal::parse_type_text(function.params[pi].type,
                                                                      function.name),
                                      function.params[pi].metadata});
      }
      out.return_type = lowir_internal::parse_type_text(function.return_type, function.name);
      for(size_t si = 0; si < function.slots.size(); ++si) {
        out.slots.push_back(
            make_pair(function.slots[si].first,
                      lowir_internal::parse_type_text(function.slots[si].second,
                                                      function.name)));
      }
      for(size_t bi = 0; bi < function.blocks.size(); ++bi) {
        lowir_internal::Block block;
        block.label = function.blocks[bi].label;
        for(size_t ii = 0; ii < function.blocks[bi].instructions.size(); ++ii) {
          block.instructions.push_back(
              lowir_internal::parse_instruction_text(function.blocks[bi].instructions[ii],
                                                     function.name + ":" + block.label));
        }
        out.blocks.push_back(block);
      }
      program.functions.push_back(out);
    }
    for(map<string, symbol_linkage::SymbolIdentity>::const_iterator it =
            exported_symbols_.begin();
        it != exported_symbols_.end(); ++it) {
      program.exported_symbols.push_back(it->second);
    }
    program.object_aliases = object_aliases_;
    complete_lowir_declarations(program);
    lowir_internal::canonicalize_program_export_metadata(program);
    return program;
  }

private:
  static lowir_internal::LowType parsed_lowir_type(const string & text)
  {
    lowir_internal::LowType type;
    type.text = text;
    return type;
  }

  static lowir_internal::SymbolBindingMode lowir_binding_for_exported_linkage(
      symbol_linkage::SymbolLinkage linkage)
  {
    switch(linkage) {
    case symbol_linkage::SL_INTERNAL:
      return lowir_internal::SBM_INTERNAL;
    case symbol_linkage::SL_EXTERNAL:
      return lowir_internal::SBM_STRONG;
    case symbol_linkage::SL_WEAK:
      return lowir_internal::SBM_WEAK;
    }
    return lowir_internal::SBM_DEFAULT;
  }

  lowir_internal::SymbolBindingMode binding_for_defined_symbol(const string & symbol) const
  {
    map<string, symbol_linkage::SymbolIdentity>::const_iterator found =
        exported_symbols_.find(symbol);
    if(found == exported_symbols_.end()) {
      return lowir_internal::SBM_INTERNAL;
    }
    return lowir_binding_for_exported_linkage(found->second.linkage);
  }

  bool function_symbol_is_special_member_entry_point(const string & symbol) const
  {
    map<string, const CallSemNode *>::const_iterator owner =
        function_symbol_nodes_.find(symbol);
    return owner != function_symbol_nodes_.end() &&
           owner->second &&
           owner->second->has_special_member_entry_point_kind;
  }

  lowir_internal::SymbolBindingMode binding_for_declared_symbol(const string & symbol) const
  {
    map<string, symbol_linkage::SymbolIdentity>::const_iterator found =
        exported_symbols_.find(symbol);
    if(found != exported_symbols_.end()) {
      const lowir_internal::SymbolBindingMode binding =
          lowir_binding_for_exported_linkage(found->second.linkage);
      if(binding == lowir_internal::SBM_WEAK &&
         function_symbol_is_special_member_entry_point(symbol)) {
        return lowir_internal::SBM_STRONG;
      }
      return binding;
    }
    if(external_function_symbols_.count(symbol) != 0 ||
       external_object_symbols_.count(symbol) != 0 ||
       c_linkage_function_symbols_.count(symbol) != 0 ||
       c_linkage_global_symbols_.count(symbol) != 0 ||
       eh_runtime::is_reserved_symbol(symbol) ||
       is_backend_passthrough_symbol(symbol)) {
      return lowir_internal::SBM_STRONG;
    }
    return lowir_internal::SBM_INTERNAL;
  }

  lowir_internal::SymbolBindingMode binding_for_thread_local_wrapper_symbol(
      const string & symbol) const
  {
    map<string, symbol_linkage::SymbolIdentity>::const_iterator found =
        exported_symbols_.find(symbol);
    if(found != exported_symbols_.end()) {
      return lowir_binding_for_exported_linkage(found->second.linkage);
    }
    return lowir_internal::SBM_INTERNAL;
  }

  string declared_function_object_symbol(const string & symbol) const
  {
    map<string, string>::const_iterator external =
        external_function_symbols_.find(symbol);
    if(external != external_function_symbols_.end()) {
      return external->second;
    }
    map<string, symbol_linkage::SymbolIdentity>::const_iterator exported =
        exported_symbols_.find(symbol);
    if(exported != exported_symbols_.end()) {
      return exported->second.object_symbol;
    }
    return string();
  }

  void apply_declared_function_boundary_metadata(
      lowir_internal::FunctionBoundaryMetadata & boundary,
      const string & symbol) const
  {
    apply_known_function_boundary_metadata(boundary, symbol);
    const string object_symbol = declared_function_object_symbol(symbol);
    if(!object_symbol.empty()) {
      apply_known_function_boundary_metadata(boundary, object_symbol);
    }
  }

  static lowir_internal::SymbolMetadata metadata_with_role_linkage_and_binding(
      lowir_internal::SymbolRole role,
      bool is_c_linkage,
      lowir_internal::SymbolBindingMode binding)
  {
    lowir_internal::SymbolMetadata metadata;
    metadata.role = role;
    metadata.linkage =
        is_c_linkage ? lowir_internal::LLM_C : lowir_internal::LLM_DEFAULT;
    metadata.binding = binding;
    return metadata;
  }

  bool external_function_alias_is_c_linkage(const string & symbol) const
  {
    map<string, string>::const_iterator found = external_function_symbols_.find(symbol);
    if(found == external_function_symbols_.end()) {
      return false;
    }

    runtime_symbol_policy::RuntimeSymbolInfo info =
        runtime_symbol_policy::classify(found->second);
    switch(info.policy) {
    case runtime_symbol_policy::RuntimeSymbolMigrationPolicy::host_eh_runtime:
    case runtime_symbol_policy::RuntimeSymbolMigrationPolicy::host_libcall:
      return true;
    case runtime_symbol_policy::RuntimeSymbolMigrationPolicy::host_abi:
      return info.role == runtime_symbol_policy::RuntimeSymbolRole::none;
    default:
      return false;
    }
  }

  bool function_symbol_is_c_linkage(const string & symbol) const
  {
    return c_linkage_function_symbols_.count(symbol) != 0 ||
           external_function_alias_is_c_linkage(symbol);
  }

  bool global_symbol_is_c_linkage(const string & symbol) const
  {
    return c_linkage_global_symbols_.count(symbol) != 0;
  }

  static lowir_internal::SymbolRole
  lowir_role_for_runtime_symbol_role(runtime_symbol_policy::RuntimeSymbolRole role)
  {
    switch(role) {
      case runtime_symbol_policy::RuntimeSymbolRole::init:
        return lowir_internal::SR_INIT;
      case runtime_symbol_policy::RuntimeSymbolRole::fini:
        return lowir_internal::SR_FINI;
      case runtime_symbol_policy::RuntimeSymbolRole::eh_top:
        return lowir_internal::SR_EH_TOP;
      case runtime_symbol_policy::RuntimeSymbolRole::eh_value:
        return lowir_internal::SR_EH_VALUE;
      case runtime_symbol_policy::RuntimeSymbolRole::eh_type:
        return lowir_internal::SR_EH_TYPE;
      case runtime_symbol_policy::RuntimeSymbolRole::eh_unhandled:
        return lowir_internal::SR_EH_UNHANDLED;
      case runtime_symbol_policy::RuntimeSymbolRole::eh_allocate_exception:
        return lowir_internal::SR_EH_ALLOCATE_EXCEPTION;
      case runtime_symbol_policy::RuntimeSymbolRole::eh_begin_catch:
        return lowir_internal::SR_EH_BEGIN_CATCH;
      case runtime_symbol_policy::RuntimeSymbolRole::eh_call_unexpected:
        return lowir_internal::SR_EH_CALL_UNEXPECTED;
      case runtime_symbol_policy::RuntimeSymbolRole::eh_current_exception_type:
        return lowir_internal::SR_EH_CURRENT_EXCEPTION_TYPE;
      case runtime_symbol_policy::RuntimeSymbolRole::eh_end_catch:
        return lowir_internal::SR_EH_END_CATCH;
      case runtime_symbol_policy::RuntimeSymbolRole::eh_rethrow:
        return lowir_internal::SR_EH_RETHROW;
      case runtime_symbol_policy::RuntimeSymbolRole::eh_throw:
        return lowir_internal::SR_EH_THROW;
      case runtime_symbol_policy::RuntimeSymbolRole::eh_personality:
        return lowir_internal::SR_EH_PERSONALITY;
      case runtime_symbol_policy::RuntimeSymbolRole::eh_resume:
        return lowir_internal::SR_EH_RESUME;
      default:
        return lowir_internal::SR_NONE;
    }
  }

  static lowir_internal::SymbolRole special_function_role_for_symbol(const string & symbol)
  {
    if(symbol == "@main") return lowir_internal::SR_ENTRY;
    const lowir_internal::SymbolRole role =
        lowir_role_for_runtime_symbol_role(runtime_symbol_policy::classify(symbol).role);
    return lowir_internal::is_function_symbol_role(role) ? role : lowir_internal::SR_NONE;
  }

  static lowir_internal::SymbolRole special_global_role_for_symbol(const string & symbol)
  {
    const lowir_internal::SymbolRole role =
        lowir_role_for_runtime_symbol_role(runtime_symbol_policy::classify(symbol).role);
    return lowir_internal::is_global_symbol_role(role) ? role : lowir_internal::SR_NONE;
  }

  static lowir_internal::FunctionDeclaration
  make_function_declaration(const string & name,
                            const LowIRFunctionSignatureText & signature)
  {
    return make_function_declaration(name, signature, false, lowir_internal::SBM_STRONG);
  }

  static lowir_internal::FunctionDeclaration
  make_function_declaration(const string & name,
                            const LowIRFunctionSignatureText & signature,
                            bool is_c_linkage,
                            lowir_internal::SymbolBindingMode binding,
                            const string & tls_for_symbol = string())
  {
    lowir_internal::FunctionDeclaration out;
    out.name = name;
    out.boundary = signature.boundary_metadata;
    out.metadata = metadata_with_role_linkage_and_binding(
        special_function_role_for_symbol(name),
        is_c_linkage,
        binding);
    out.metadata.tls_for_symbol = tls_for_symbol;
    out.return_type = parsed_lowir_type(signature.return_type);
    for(size_t i = 0; i < signature.params.size(); ++i) {
      out.params.push_back(
          lowir_internal::Parameter{
              signature.params[i].name,
              parsed_lowir_type(signature.params[i].type),
              signature.params[i].metadata});
    }
    return out;
  }

  static bool is_opaque_global_declaration_type(const TypePtr & type)
  {
    TypePtr base = strip_top_level_cv(type);
    return base && (base->kind == Type::TK_ARRAY || base->kind == Type::TK_NAMED);
  }

  const FunctionSymbolEntry * find_function_symbol_entry_by_symbol(const string & symbol) const
  {
    for(size_t i = 0; i < function_symbol_entries_.size(); ++i) {
      if(function_symbol_entries_[i].symbol == symbol) {
        return &function_symbol_entries_[i];
      }
    }
    return nullptr;
  }

  static bool try_known_runtime_function_signature(const string & symbol,
                                                   LowIRFunctionSignatureText & signature)
  {
    string name = symbol;
    if(!name.empty() && name[0] == '@') {
      name.erase(name.begin());
    }
    signature.params.clear();
    signature.boundary_metadata = lowir_internal::FunctionBoundaryMetadata();
    apply_known_function_boundary_metadata(signature.boundary_metadata, name);
    if(name == "abort" || name == "__cxa_rethrow" ||
       name == "_Unwind_Resume" || name == "__gxx_personality_v0" ||
       name == "__cxa_pure_virtual") {
      signature.return_type = "void";
      return true;
    }
    if(name == "__cxa_current_exception_type") {
      signature.return_type = "ptr";
      return true;
    }
    if(name == "__cxa_begin_catch") {
      signature.return_type = "ptr";
      signature.params.push_back(make_lowir_parameter_text("%arg0", "ptr"));
      return true;
    }
    if(name == "__cxa_bad_cast" || name == "__cxa_bad_typeid") {
      signature.return_type = "void";
      return true;
    }
    if(name == "__cxa_end_catch") {
      signature.return_type = "void";
      return true;
    }
    if(name == "__cxa_call_unexpected") {
      signature.return_type = "void";
      signature.params.push_back(make_lowir_parameter_text("%arg0", "ptr"));
      return true;
    }
    if(name == "__cxa_allocate_exception") {
      signature.return_type = "ptr";
      signature.params.push_back(make_lowir_parameter_text("%arg0", "i64"));
      return true;
    }
    if(name == "__cxa_thread_atexit") {
      signature.return_type = "i32";
      signature.params.push_back(make_lowir_parameter_text("%arg0", "ptr"));
      signature.params.push_back(make_lowir_parameter_text("%arg1", "ptr"));
      signature.params.push_back(make_lowir_parameter_text("%arg2", "ptr"));
      return true;
    }
    if(name == "__cxa_throw") {
      signature.return_type = "void";
      signature.params.push_back(make_lowir_parameter_text("%arg0", "ptr"));
      signature.params.push_back(make_lowir_parameter_text("%arg1", "ptr"));
      signature.params.push_back(make_lowir_parameter_text("%arg2", "ptr"));
      return true;
    }
    if(name == "__dynamic_cast") {
      signature.return_type = "ptr";
      signature.params.push_back(make_lowir_parameter_text("%arg0", "ptr"));
      signature.params.push_back(make_lowir_parameter_text("%arg1", "ptr"));
      signature.params.push_back(make_lowir_parameter_text("%arg2", "ptr"));
      signature.params.push_back(make_lowir_parameter_text("%arg3", "i64"));
      return true;
    }
    if(name == "operator_new" || name == "operator_new__") {
      signature.return_type = "ptr";
      signature.params.push_back(make_lowir_parameter_text("%arg0", "i64"));
      return true;
    }
    if(name == "operator_delete" || name == "operator_delete__") {
      signature.return_type = "void";
      signature.params.push_back(make_lowir_parameter_text("%arg0", "ptr"));
      return true;
    }
    if(name == "cppgm_builtin_ceil") {
      signature.return_type = "f64";
      signature.params.push_back(make_lowir_parameter_text("%arg0", "f64"));
      return true;
    }
    if(name == "cppgm_builtin_ceilf") {
      signature.return_type = "f32";
      signature.params.push_back(make_lowir_parameter_text("%arg0", "f32"));
      return true;
    }
    if(name == "cppgm_builtin_ceill") {
      signature.return_type = "f80";
      signature.params.push_back(make_lowir_parameter_text("%arg0", "f80"));
      return true;
    }
    if(name == "cppgm_builtin_fabs") {
      signature.return_type = "f64";
      signature.params.push_back(make_lowir_parameter_text("%arg0", "f64"));
      return true;
    }
    if(name == "cppgm_builtin_fabsf") {
      signature.return_type = "f32";
      signature.params.push_back(make_lowir_parameter_text("%arg0", "f32"));
      return true;
    }
    if(name == "cppgm_builtin_fabsl") {
      signature.return_type = "f80";
      signature.params.push_back(make_lowir_parameter_text("%arg0", "f80"));
      return true;
    }
    if(name == "cppgm_builtin_inf") {
      signature.return_type = "f64";
      return true;
    }
    if(name == "cppgm_builtin_inff") {
      signature.return_type = "f32";
      return true;
    }
    if(name == "cppgm_builtin_infl") {
      signature.return_type = "f80";
      return true;
    }
    if(name == "cppgm_builtin_is_constant_evaluated") {
      signature.return_type = "u8";
      return true;
    }
    if(name == "cppgm_builtin_isfinite") {
      signature.return_type = "u8";
      signature.params.push_back(make_lowir_parameter_text("%arg0", "f64"));
      return true;
    }
    if(name == "cppgm_builtin_isfinitef") {
      signature.return_type = "u8";
      signature.params.push_back(make_lowir_parameter_text("%arg0", "f32"));
      return true;
    }
    if(name == "cppgm_builtin_isfinitel") {
      signature.return_type = "u8";
      signature.params.push_back(make_lowir_parameter_text("%arg0", "f80"));
      return true;
    }
    if(name == "cppgm_builtin_isinf") {
      signature.return_type = "u8";
      signature.params.push_back(make_lowir_parameter_text("%arg0", "f64"));
      return true;
    }
    if(name == "cppgm_builtin_isinff") {
      signature.return_type = "u8";
      signature.params.push_back(make_lowir_parameter_text("%arg0", "f32"));
      return true;
    }
    if(name == "cppgm_builtin_isinfl") {
      signature.return_type = "u8";
      signature.params.push_back(make_lowir_parameter_text("%arg0", "f80"));
      return true;
    }
    if(name == "cppgm_builtin_isnan") {
      signature.return_type = "u8";
      signature.params.push_back(make_lowir_parameter_text("%arg0", "f64"));
      return true;
    }
    if(name == "cppgm_builtin_isnanf") {
      signature.return_type = "u8";
      signature.params.push_back(make_lowir_parameter_text("%arg0", "f32"));
      return true;
    }
    if(name == "cppgm_builtin_isnanl") {
      signature.return_type = "u8";
      signature.params.push_back(make_lowir_parameter_text("%arg0", "f80"));
      return true;
    }
    if(name == "cppgm_builtin_isnormal") {
      signature.return_type = "u8";
      signature.params.push_back(make_lowir_parameter_text("%arg0", "f64"));
      return true;
    }
    if(name == "cppgm_builtin_isnormalf") {
      signature.return_type = "u8";
      signature.params.push_back(make_lowir_parameter_text("%arg0", "f32"));
      return true;
    }
    if(name == "cppgm_builtin_isnormall") {
      signature.return_type = "u8";
      signature.params.push_back(make_lowir_parameter_text("%arg0", "f80"));
      return true;
    }
    if(name == "__fpclassify") {
      signature.return_type = "i32";
      signature.params.push_back(make_lowir_parameter_text("%arg0", "f64"));
      return true;
    }
    if(name == "__fpclassifyd") {
      signature.return_type = "i32";
      signature.params.push_back(make_lowir_parameter_text("%arg0", "f64"));
      return true;
    }
    if(name == "__fpclassifyf") {
      signature.return_type = "i32";
      signature.params.push_back(make_lowir_parameter_text("%arg0", "f32"));
      return true;
    }
    if(name == "__fpclassifyl") {
      signature.return_type = "i32";
      signature.params.push_back(make_lowir_parameter_text("%arg0", "f80"));
      return true;
    }
    return false;
  }

  static void record_global_declaration(lowir_internal::Program & program,
                                        set<string> & emitted_global_declarations,
                                        const lowir_internal::GlobalDeclaration & declaration)
  {
    if(emitted_global_declarations.insert(declaration.name).second) {
      program.global_declarations.push_back(declaration);
    }
  }

  static void record_function_declaration(
      lowir_internal::Program & program,
      set<string> & emitted_function_declarations,
      const lowir_internal::FunctionDeclaration & declaration)
  {
    if(emitted_function_declarations.insert(declaration.name).second) {
      program.function_declarations.push_back(declaration);
    }
  }

  void append_parameter_virtual_base_signature_params(
      LowIRFunctionSignatureText & signature,
      const string & symbol) const
  {
    map<string, ParameterVirtualBaseLayout>::const_iterator layout_it =
        function_parameter_virtual_base_layouts_.find(symbol);
    if(layout_it == function_parameter_virtual_base_layouts_.end()) {
      return;
    }

    append_parameter_virtual_base_signature_params_for_layout(signature, layout_it->second);
  }

	  bool try_declare_known_function_symbol(lowir_internal::Program & program,
	                                         set<string> & emitted_function_declarations,
	                                         const string & symbol) const
	  {
    if(emitted_function_declarations.count(symbol) != 0 ||
       generated_function_symbol_exists(symbol)) {
      return true;
    }

    if(const FunctionSymbolEntry * entry = find_function_symbol_entry_by_symbol(symbol)) {
      map<string, const CallSemNode *>::const_iterator owner =
          function_symbol_nodes_.find(symbol);
      LowIRFunctionSignatureText signature =
          owner != function_symbol_nodes_.end() ?
              lowir_function_signature_text_for_callsem_node(*owner->second, symbol) :
              lowir_function_signature_text(entry->type, symbol);
      append_parameter_virtual_base_signature_params(signature, symbol);
      apply_declared_function_boundary_metadata(signature.boundary_metadata, symbol);
      if(owner != function_symbol_nodes_.end()) {
        apply_callsem_function_boundary_metadata(signature.boundary_metadata, *owner->second);
      }
      record_function_declaration(program,
                                  emitted_function_declarations,
                                  make_function_declaration(symbol,
                                                            signature,
                                                            function_symbol_is_c_linkage(symbol),
                                                            binding_for_declared_symbol(symbol)));
      return true;
    }

    map<string, TypePtr>::const_iterator signature_hint =
        referenced_function_signature_types_.find(symbol);
    if(signature_hint != referenced_function_signature_types_.end()) {
      map<string, const CallSemNode *>::const_iterator owner =
          function_symbol_nodes_.find(symbol);
      LowIRFunctionSignatureText signature =
          owner != function_symbol_nodes_.end() ?
              lowir_function_signature_text_for_callsem_node(*owner->second, symbol) :
              lowir_function_signature_text(signature_hint->second, symbol);
      append_parameter_virtual_base_signature_params(signature, symbol);
      apply_declared_function_boundary_metadata(signature.boundary_metadata, symbol);
      if(owner != function_symbol_nodes_.end()) {
        apply_callsem_function_boundary_metadata(signature.boundary_metadata, *owner->second);
      }
      record_function_declaration(program,
                                  emitted_function_declarations,
                                  make_function_declaration(
                                      symbol,
                                      signature,
                                      function_symbol_is_c_linkage(symbol),
                                      binding_for_declared_symbol(symbol)));
      return true;
    }

    LowIRFunctionSignatureText signature;
    map<string, string>::const_iterator external = external_function_symbols_.find(symbol);
    if(external != external_function_symbols_.end() &&
       try_known_runtime_function_signature(external->second, signature)) {
      record_function_declaration(program,
                                  emitted_function_declarations,
                                  make_function_declaration(symbol,
                                                            signature,
                                                            true,
                                                            binding_for_declared_symbol(symbol)));
      return true;
    }

    if(try_known_runtime_function_signature(symbol, signature)) {
      record_function_declaration(program,
                                  emitted_function_declarations,
                                  make_function_declaration(symbol,
                                                            signature,
                                                            function_symbol_is_c_linkage(symbol),
                                                            binding_for_declared_symbol(symbol)));
      return true;
    }
    return false;
  }

  void record_thread_local_wrapper_declaration(
      lowir_internal::Program & program,
      set<string> & emitted_function_declarations,
      const string & target_symbol,
      lowir_internal::SymbolBindingMode binding,
      const symbol_linkage::SymbolIdentity & variable_symbol) const
  {
    const string wrapper_object =
        variable_symbol.thread_local_wrapper_object_symbol;
    if(wrapper_object.empty()) {
      return;
    }

    const string wrapper_internal =
        symbol_linkage::thread_local_wrapper_internal_symbol(target_symbol);
    if(emitted_function_declarations.count(wrapper_internal) != 0 ||
       generated_function_symbol_exists(wrapper_internal)) {
      return;
    }

    LowIRFunctionSignatureText signature;
    signature.return_type = "ptr";
    lowir_internal::FunctionDeclaration declaration =
        make_function_declaration(wrapper_internal,
                                  signature,
                                  false,
                                  binding,
                                  target_symbol);
    declaration.metadata.object_symbol = wrapper_object;
    record_function_declaration(program,
                                emitted_function_declarations,
                                declaration);
  }

  void declare_global_symbol(lowir_internal::Program & program,
                             set<string> & emitted_function_declarations,
                             set<string> & emitted_global_declarations,
                             const string & symbol) const
  {
    if(emitted_global_declarations.count(symbol) != 0 ||
       generated_global_symbol_exists(symbol)) {
      return;
    }

    for(map<string, GlobalBinding>::const_iterator it = global_bindings_.begin();
        it != global_bindings_.end();
        ++it) {
      if(it->second.storage != symbol || it->second.is_definition) {
        continue;
      }
      lowir_internal::GlobalDeclaration out;
      out.name = symbol;
      out.storage = it->second.thread_local_storage ? lowir_internal::GSM_THREAD_LOCAL
                                                    : lowir_internal::GSM_DEFAULT;
      out.metadata = metadata_with_role_linkage_and_binding(
          special_global_role_for_symbol(symbol),
          global_symbol_is_c_linkage(symbol),
          binding_for_declared_symbol(symbol));
      if(!is_opaque_global_declaration_type(it->second.semantic_type)) {
        out.has_type = true;
        out.type = parsed_lowir_type(it->second.lowir_type);
      }
      if(out.storage == lowir_internal::GSM_THREAD_LOCAL) {
        record_thread_local_wrapper_declaration(program,
                                                emitted_function_declarations,
                                                symbol,
                                                out.metadata.binding,
                                                it->second.symbol);
      }
      record_global_declaration(program, emitted_global_declarations, out);
      return;
    }

    lowir_internal::GlobalDeclaration out;
    out.name = symbol;
    out.storage = lowir_internal::GSM_DEFAULT;
    out.metadata = metadata_with_role_linkage_and_binding(
        special_global_role_for_symbol(symbol),
        global_symbol_is_c_linkage(symbol),
        binding_for_declared_symbol(symbol));
    if(symbol == eh_runtime::kEhTopSymbol ||
       symbol == eh_runtime::kEhValueSymbol ||
       symbol == eh_runtime::kEhTypeSymbol) {
      out.has_type = true;
      out.type = parsed_lowir_type("ptr");
    }
    record_global_declaration(program, emitted_global_declarations, out);
  }

  static void collect_program_symbol_references(const lowir_internal::Program & program,
                                                set<string> & direct_calls,
                                                set<string> & address_symbols)
  {
    const auto note_global_operand =
        [&address_symbols](const lowir_internal::Operand & operand)
        {
          if(operand.kind == lowir_internal::Operand::OP_GLOBAL) {
            address_symbols.insert(operand.text);
          }
        };

    for(size_t i = 0; i < program.globals.size(); ++i) {
      const lowir_internal::GlobalDefinition & global = program.globals[i];
      if(!global.structured &&
         global.init_kind == lowir_internal::GlobalDefinition::INIT_ADDR &&
         global.init_operand.kind == lowir_internal::Operand::OP_GLOBAL) {
        address_symbols.insert(global.init_operand.text);
      }
      for(size_t di = 0; di < global.data_items.size(); ++di) {
        if(global.data_items[di].kind == lowir_internal::GlobalDefinition::DataItem::ITEM_ADDR) {
          address_symbols.insert(global.data_items[di].symbol);
        }
      }
    }

    for(size_t fi = 0; fi < program.functions.size(); ++fi) {
      const lowir_internal::Function & function = program.functions[fi];
      for(size_t bi = 0; bi < function.blocks.size(); ++bi) {
        for(size_t ii = 0; ii < function.blocks[bi].instructions.size(); ++ii) {
          const lowir_internal::Instruction & inst =
              function.blocks[bi].instructions[ii];
          if(inst.kind == lowir_internal::Instruction::IK_CALL &&
             inst.first.kind == lowir_internal::Operand::OP_GLOBAL) {
            direct_calls.insert(inst.first.text);
          }
          note_global_operand(inst.first);
          note_global_operand(inst.second);
          note_global_operand(inst.third);
          for(size_t ai = 0; ai < inst.args.size(); ++ai) {
            note_global_operand(inst.args[ai]);
          }
        }
      }
    }
  }

  void complete_lowir_declarations(lowir_internal::Program & program) const
  {
    set<string> emitted_function_declarations;
    set<string> emitted_global_declarations;
    for(size_t i = 0; i < program.function_declarations.size(); ++i) {
      emitted_function_declarations.insert(program.function_declarations[i].name);
    }
    for(size_t i = 0; i < program.functions.size(); ++i) {
      emitted_function_declarations.insert(program.functions[i].name);
    }
    for(size_t i = 0; i < program.global_declarations.size(); ++i) {
      emitted_global_declarations.insert(program.global_declarations[i].name);
    }
    for(size_t i = 0; i < program.globals.size(); ++i) {
      emitted_global_declarations.insert(program.globals[i].name);
    }

    set<string> direct_calls;
    set<string> address_symbols;
    collect_program_symbol_references(program, direct_calls, address_symbols);

    for(set<string>::const_iterator it = direct_calls.begin();
        it != direct_calls.end();
        ++it) {
      if(!try_declare_known_function_symbol(program, emitted_function_declarations, *it)) {
        throw logic_error("missing LowIR declaration signature for " + *it);
      }
    }

    for(set<string>::const_iterator it = address_symbols.begin();
        it != address_symbols.end();
        ++it) {
      if(emitted_function_declarations.count(*it) != 0 ||
         emitted_global_declarations.count(*it) != 0) {
        continue;
      }
      if(try_declare_known_function_symbol(program, emitted_function_declarations, *it)) {
        continue;
      }
      declare_global_symbol(program,
                            emitted_function_declarations,
                            emitted_global_declarations,
                            *it);
    }
  }

  void collect_lowir_declarations(lowir_internal::Program & program) const
  {
    set<string> emitted_function_declarations;
    set<string> emitted_global_declarations;

    for(map<string, string>::const_iterator it = thread_local_wrapper_targets_.begin();
        it != thread_local_wrapper_targets_.end();
        ++it) {
      LowIRFunctionSignatureText signature;
      signature.return_type = "ptr";
      lowir_internal::FunctionDeclaration declaration =
          make_function_declaration(
              it->first,
              signature,
              false,
              binding_for_thread_local_wrapper_symbol(it->first),
              it->second);
      map<string, string>::const_iterator object =
          thread_local_wrapper_object_symbols_.find(it->first);
      if(object != thread_local_wrapper_object_symbols_.end()) {
        declaration.metadata.object_symbol = object->second;
      }
      record_function_declaration(program,
                                  emitted_function_declarations,
                                  declaration);
    }

    for(map<string, string>::const_iterator it = external_function_symbols_.begin();
        it != external_function_symbols_.end();
        ++it) {
      if(!try_declare_known_function_symbol(program,
                                            emitted_function_declarations,
                                            it->first)) {
        throw logic_error("missing LowIR declaration signature for " + it->first);
      }
    }

    for(map<string, string>::const_iterator it = external_object_symbols_.begin();
        it != external_object_symbols_.end();
        ++it) {
      declare_global_symbol(program,
                            emitted_function_declarations,
                            emitted_global_declarations,
                            it->first);
    }
  }

  CallSemNode & make_synthetic_node(CallSemKind kind, const string & text = string())
  {
    synthetic_nodes_.push_back(unique_ptr<CallSemNode>(new CallSemNode()));
    CallSemNode & node = *synthetic_nodes_.back();
    node = make_dump_node(kind, text);
    return node;
  }

  void note_referenced_function_signature(const string & symbol,
                                          const TypePtr & type) const
  {
    if(symbol.empty() || !type) {
      return;
    }
    map<string, TypePtr>::const_iterator found =
        referenced_function_signature_types_.find(symbol);
    if(found == referenced_function_signature_types_.end()) {
      referenced_function_signature_types_[symbol] = type;
      return;
    }
    if(type_equals(found->second, type) ||
       describe_type(found->second) == describe_type(type)) {
      return;
    }
  }

  string constructor_symbol(const TypePtr & class_type) const
  {
    TypePtr object_type = strip_top_level_cv(remove_reference_type(class_type));
    const string qualified = class_qualified_name(object_type);
    if(qualified.empty()) {
      return string();
    }
    const string simple = class_constructor_name(qualified);
    vector<TypePtr> params;
    params.push_back(make_pointer(object_type));
    map<string, string>::const_iterator found =
        function_symbols_.find(function_key(qualified + "::" + simple,
                                            make_function(make_fundamental(FT_VOID),
                                                          params,
                                                          false)));
    if(found == function_symbols_.end()) {
      return string();
    }
    note_referenced_function_signature(
        found->second,
        make_function(make_fundamental(FT_VOID), params, false));
    return found->second;
  }

  string destructor_symbol(const TypePtr & class_type) const
  {
    TypePtr object_type = strip_top_level_cv(remove_reference_type(class_type));
    const string qualified = class_qualified_name(object_type);
    if(qualified.empty()) {
      return string();
    }
    const string simple = class_constructor_name(qualified);
    vector<TypePtr> params;
    params.push_back(make_pointer(object_type));
    map<string, string>::const_iterator found =
        function_symbols_.find(function_key(qualified + "::~" + simple,
                                            make_function(make_fundamental(FT_VOID),
                                                          params,
                                                          false)));
    if(found != function_symbols_.end()) {
      note_referenced_function_signature(
          found->second,
          make_function(make_fundamental(FT_VOID), params, false));
      return found->second;
    }
    const string symbol = try_lookup_special_member_symbol_by_index(
        function_symbol_entries_,
        function_symbol_lookup_index(),
        qualified + "::~" + simple,
        [&](const TypePtr & entry_type)
        {
          return matches_destructor_entry_type_for_lowir(entry_type, object_type);
        });
    note_referenced_function_signature(
        symbol,
        make_function(make_fundamental(FT_VOID), params, false));
    return symbol;
  }

  CallSemNode & make_global_array_element_address_expr(const CallSemNode & variable,
                                                       const TypePtr & array_type,
                                                       size_t index)
  {
    CallSemNode & id = make_synthetic_node(CallSemKind::id_expression, variable.text);
    id.semantic_type = variable.semantic_type;
    id.value_category = CVC_LVALUE;
    set_callsem_symbol(id, callsem_symbol(variable));

    CallSemNode & index_node =
        make_synthetic_node(CallSemKind::literal, to_string(index));
    index_node.semantic_type = make_fundamental(FT_INT);
    index_node.value_category = CVC_PRVALUE;
    set_callsem_uint_value(index_node, index);
    set_callsem_int_value(index_node, static_cast<long long>(index));

    CallSemNode & subscript =
        make_synthetic_node(CallSemKind::subscript_expression);
    subscript.semantic_type = array_type->inner;
    subscript.value_category = CVC_LVALUE;
    subscript.children.push_back(id);
    subscript.children.push_back(index_node);

    CallSemNode & address =
        make_synthetic_node(CallSemKind::unary_expression);
    address.semantic_type = make_pointer(array_type->inner);
    address.value_category = CVC_PRVALUE;
    address.has_token = true;
    address.token_type = OP_AMP;
    address.children.push_back(subscript);
    return address;
  }

  CallSemNode & make_global_object_address_expr(const CallSemNode & variable)
  {
    CallSemNode & id = make_synthetic_node(CallSemKind::id_expression, variable.text);
    id.semantic_type = variable.semantic_type;
    id.value_category = CVC_LVALUE;
    set_callsem_symbol(id, callsem_symbol(variable));

    CallSemNode & address =
        make_synthetic_node(CallSemKind::unary_expression);
    address.semantic_type = make_pointer(variable.semantic_type);
    address.value_category = CVC_PRVALUE;
    address.has_token = true;
    address.token_type = OP_AMP;
    address.children.push_back(id);
    return address;
  }

  CallSemNode & make_global_scalar_dynamic_initializer_action(
      const CallSemNode & variable,
      const CallSemNode & initializer)
  {
    CallSemNode & lhs = make_synthetic_node(CallSemKind::id_expression, variable.text);
    lhs.semantic_type = variable.semantic_type;
    lhs.value_category = CVC_LVALUE;
    set_callsem_symbol(lhs, callsem_symbol(variable));
    lhs.is_thread_local = variable.is_thread_local;

    CallSemNode & assignment =
        make_synthetic_node(CallSemKind::assignment_expression);
    assignment.semantic_type = variable.semantic_type;
    assignment.value_category = CVC_LVALUE;
    assignment.has_token = true;
    assignment.token_type = OP_ASS;
    assignment.children.push_back(lhs);
    assignment.children.push_back(initializer);

    CallSemNode & action =
        make_synthetic_node(CallSemKind::expression_statement, variable.text);
    action.semantic_type = make_fundamental(FT_VOID);
    action.value_category = CVC_PRVALUE;
    action.children.push_back(assignment);
    return action;
  }

  CallSemNode & make_global_reference_storage_target_expr(
      const CallSemNode & variable)
  {
    TypePtr referent = remove_reference_type(variable.semantic_type);
    if(!referent) {
      throw logic_error("global reference storage target requires reference type");
    }

    CallSemNode & target = make_synthetic_node(CallSemKind::id_expression, variable.text);
    target.semantic_type = make_pointer(referent);
    target.value_category = CVC_LVALUE;
    target.is_reference_storage_target = true;
    target.is_thread_local = variable.is_thread_local;
    set_callsem_symbol(target, callsem_symbol(variable));
    return target;
  }

  CallSemNode & make_global_reference_dynamic_initializer_action(
      const CallSemNode & variable,
      const CallSemNode & initializer)
  {
    CallSemNode & target = make_global_reference_storage_target_expr(variable);

    CallSemNode & assignment =
        make_synthetic_node(CallSemKind::assignment_expression);
    assignment.semantic_type = target.semantic_type;
    assignment.value_category = CVC_LVALUE;
    assignment.has_token = true;
    assignment.token_type = OP_ASS;
    assignment.children.push_back(target);
    assignment.children.push_back(initializer);

    CallSemNode & action =
        make_synthetic_node(CallSemKind::expression_statement, variable.text);
    action.semantic_type = make_fundamental(FT_VOID);
    action.value_category = CVC_PRVALUE;
    action.children.push_back(assignment);
    return action;
  }

  bool is_direct_global_class_materialization_child(const CallSemNode & variable,
                                                    const CallSemNode & child) const
  {
    if(variable.kind != CallSemKind::variable ||
       is_reference_type(variable.semantic_type) ||
       !semantic_conversion::same_type_with_compatible_top_cv(
           strip_top_level_cv(variable.semantic_type),
           strip_top_level_cv(child.semantic_type))) {
      return false;
    }

    if(child.kind == CallSemKind::closure_object ||
       child.kind == CallSemKind::initializer_list_object ||
       child.kind == CallSemKind::statement_expression ||
       child.kind == CallSemKind::conditional_expression) {
      return true;
    }

    if(child.kind == CallSemKind::call_expression &&
       (is_indirect_value_type(child.semantic_type) ||
        is_complete_class_value_type(child.semantic_type))) {
      return true;
    }

    return child.kind == CallSemKind::binary_expression &&
           callsem_has_token(child, OP_COMMA);
  }

  void append_global_object_materialization_constructor_action(
      const CallSemNode & variable,
      const CallSemNode & source)
  {
    CallSemNode & callee =
        make_synthetic_node(CallSemKind::id_expression, "<global-materialization>");
    callee.semantic_type =
        make_function(make_fundamental(FT_VOID),
                      std::vector<TypePtr>(),
                      false);
    callee.value_category = CVC_PRVALUE;

    CallSemNode & call =
        make_synthetic_node(CallSemKind::call_expression);
    call.semantic_type = make_fundamental(FT_VOID);
    call.value_category = CVC_PRVALUE;
    call.children.push_back(callee);
    call.children.push_back(make_global_object_address_expr(variable));
    call.children.push_back(source);

    CallSemNode & action =
        make_synthetic_node(CallSemKind::constructor_action, "<global-materialization>");
    action.trivial_lifecycle = true;
    action.children.push_back(call);
    global_ctor_actions_.push_back(&action);
  }

  void append_global_array_constructor_action(const CallSemNode & variable,
                                              const TypePtr & array_type,
                                              size_t index,
                                              const CallSemNode & init_call)
  {
    if(init_call.kind != CallSemKind::call_expression ||
       init_call.children.empty() ||
       init_call.children[0].kind != CallSemKind::callee ||
       !is_constructor_function_name(init_call.children[0].text)) {
      throw logic_error("unsupported global array class initializer");
    }

    CallSemNode & call =
        make_synthetic_node(CallSemKind::call_expression);
    call.semantic_type = make_fundamental(FT_VOID);
    call.value_category = CVC_PRVALUE;
    call.children.push_back(init_call.children[0]);
    call.children.push_back(make_global_array_element_address_expr(variable, array_type, index));
    for(size_t i = 1; i < init_call.children.size(); ++i) {
      call.children.push_back(init_call.children[i]);
    }

    CallSemNode & action =
        make_synthetic_node(CallSemKind::constructor_action, init_call.children[0].text);
    action.children.push_back(call);
    global_ctor_actions_.push_back(&action);
  }

  void append_global_array_destructor_action(const CallSemNode & variable,
                                             const TypePtr & array_type,
                                             size_t index,
                                             const string & symbol)
  {
    if(symbol.empty() || function_symbol_has_trivial_lifecycle(symbol)) {
      return;
    }

    const string qualified = class_qualified_name(array_type->inner);
    const string simple = class_constructor_name(qualified);
    vector<TypePtr> params;
    params.push_back(make_pointer(array_type->inner));

    CallSemNode & callee =
        make_synthetic_node(CallSemKind::callee, qualified + "::~" + simple);
    callee.semantic_type =
        make_function(make_fundamental(FT_VOID), params, false);
    callee.value_category = CVC_PRVALUE;
    set_callsem_symbol(
        callee,
        symbol_linkage::make_internal_symbol_identity(symbol, symbol_linkage::SL_WEAK));

    CallSemNode & call =
        make_synthetic_node(CallSemKind::call_expression);
    call.semantic_type = make_fundamental(FT_VOID);
    call.value_category = CVC_PRVALUE;
    call.children.push_back(callee);
    call.children.push_back(make_global_array_element_address_expr(variable, array_type, index));

    CallSemNode & action =
        make_synthetic_node(CallSemKind::destructor_action, qualified + "::~" + simple);
    action.children.push_back(call);
    global_dtor_actions_.push_back(&action);
  }

  bool try_collect_global_class_array(const CallSemNode & node,
                                      const TypePtr & array_type,
                                      const CallSemNode & init)
  {
    TypePtr element_type = strip_top_level_cv(array_type->inner);
    if(!is_complete_class_value_type(element_type)) {
      return false;
    }

    const GlobalBinding & binding = global_bindings_.find(node_internal_symbol(node))->second;
    LowIRGlobal global = make_data_global(binding.storage);
    global.data_items.push_back(string("zero ") + to_string(backend_storage_size(node.semantic_type)));
    globals_.push_back(global);

    size_t constructed_count = 0;
    for(size_t i = 0; i < init.children.size(); ++i) {
      append_global_array_constructor_action(node, array_type, i, init.children[i]);
      ++constructed_count;
    }

    const string default_ctor = constructor_symbol(element_type);
    for(size_t i = init.children.size(); i < array_type->bound && !default_ctor.empty(); ++i) {
      CallSemNode & callee =
          make_synthetic_node(CallSemKind::callee,
                              class_qualified_name(element_type) + "::" +
                                  class_constructor_name(class_qualified_name(element_type)));
      vector<TypePtr> params;
      params.push_back(make_pointer(element_type));
      callee.semantic_type =
          make_function(make_fundamental(FT_VOID), params, false);
      callee.value_category = CVC_PRVALUE;
      set_callsem_symbol(
          callee,
          symbol_linkage::make_internal_symbol_identity(default_ctor,
                                                        symbol_linkage::SL_WEAK));

      CallSemNode & call =
          make_synthetic_node(CallSemKind::call_expression);
      call.semantic_type = make_fundamental(FT_VOID);
      call.value_category = CVC_PRVALUE;
      call.children.push_back(callee);
      call.children.push_back(make_global_array_element_address_expr(node, array_type, i));

      CallSemNode & action =
          make_synthetic_node(CallSemKind::constructor_action, callee.text);
      action.children.push_back(call);
      global_ctor_actions_.push_back(&action);
      ++constructed_count;
    }

    const string dtor = destructor_symbol(element_type);
    if(!node.is_thread_local) {
      for(size_t i = 0; i < constructed_count; ++i) {
        append_global_array_destructor_action(node, array_type, i, dtor);
      }
    }
    return true;
  }

  bool append_global_array_initializer_items(vector<string> & data_items,
                                             const TypePtr & array_type,
                                             const CallSemNode & init)
  {
    TypePtr base = strip_top_level_cv(array_type);
    if(!base || base->kind != Type::TK_ARRAY ||
       init.kind != CallSemKind::braced_init_list) {
      return false;
    }
    if(init.children.size() > base->bound) {
      throw logic_error("too many global array initializer elements");
    }

    const TypePtr element_type = base->inner;
    const TypePtr element_base = strip_top_level_cv(element_type);
    const size_t element_size = backend_storage_size(element_type);

    for(size_t i = 0; i < base->bound; ++i) {
      if(i >= init.children.size()) {
        data_items.push_back(string("zero ") + to_string(element_size));
        continue;
      }

      const CallSemNode & child = init.children[i];
      if(element_base && element_base->kind == Type::TK_ARRAY) {
        if(child.kind == CallSemKind::literal &&
           ((child.has_int_value && callsem_int_value(child) == 0) ||
            (child.has_uint_value && callsem_uint_value(child) == 0) ||
            callsem_has_token(child, KW_FALSE) ||
            child.text == "0")) {
          data_items.push_back(string("zero ") + to_string(element_size));
          continue;
        }
        if(!append_global_array_initializer_items(data_items, element_type, child)) {
          return false;
        }
        continue;
      }

      if(is_complete_class_value_type(element_base)) {
        return false;
      }

      string value;
      bool is_addr = false;
      long long addr_addend = 0;
      if(!evaluate_global_initializer(child, value, is_addr, addr_addend)) {
        return false;
      }
      if(is_addr) {
        if(is_member_function_pointer_type(element_type)) {
          if(addr_addend != 0) {
            throw logic_error("member-function pointer global initializer addend unsupported");
          }
          append_member_function_pointer_global_data_items(data_items, value);
        } else {
          data_items.push_back(string("ptr addr ") +
                               format_global_address_operand(value, addr_addend));
        }
      } else if(is_pointer_type(element_type) &&
                is_null_pointer_global_initializer(child)) {
        data_items.push_back(string("zero ") + to_string(element_size));
      } else {
        data_items.push_back(lowir_memory_type_for(element_type) + " " + value);
      }
    }

    return true;
  }

  void append_local_static_guard_global(const CallSemNode & node)
  {
    if(callsem_local_static_guard_symbol(node).empty()) {
      return;
    }
    const string guard_symbol = callsem_local_static_guard_symbol(node);
    globals_.push_back(make_scalar_global(guard_symbol,
                                          "i64",
                                          "zero",
                                          false,
                                          false,
                                          node.is_thread_local));
    const string object_symbol = node_internal_symbol(node);
    const bool guard_for_named_thread_local_object =
        !object_symbol.empty() &&
        guard_symbol == symbol_linkage::thread_local_guard_internal_symbol(object_symbol);
    if(node.is_thread_local) {
      const string wrapper_internal =
          symbol_linkage::thread_local_wrapper_internal_symbol(guard_symbol);
      thread_local_wrapper_targets_[wrapper_internal] = guard_symbol;
      if(!guard_for_named_thread_local_object) {
        thread_local_wrapper_object_symbols_[wrapper_internal] = wrapper_internal;
        set_exported_symbol(
            wrapper_internal,
            symbol_linkage::make_internal_symbol_identity(wrapper_internal,
                                                          symbol_linkage::SL_WEAK),
            "tls-guard-wrapper",
            node.text);
      }
    }
  }

  const vector<CallSemNode> & translation_units_;
  vector<LowIRGlobal> globals_;
  vector<LowIRFunction> functions_;
  map<string, GlobalBinding> global_bindings_;
  map<string, VTableBinding> vtable_bindings_;
  map<string, string> function_symbols_;
  vector<FunctionSymbolEntry> function_symbol_entries_;
  map<string, const CallSemNode *> function_symbol_nodes_;
  set<string> c_linkage_function_symbols_;
  map<string, vector<pair<string, unsigned long long> > > function_virtual_base_layouts_;
  map<string, vector<pair<string, unsigned long long> > > class_virtual_base_layouts_;
  map<string, ParameterVirtualBaseLayout> function_parameter_virtual_base_layouts_;
  map<string, vector<ParameterVirtualBaseForwardingCandidate> >
      parameter_virtual_base_forwarding_candidates_;
  set<string> classes_with_virtual_functions_;
  set<string> throwing_function_symbols_;
  map<string, string> string_literal_symbols_;
  map<string, TypePtr> exception_storage_types_;
  bool uses_private_eh_runtime_ = false;
  map<string, VirtualMemberPointerThunkRequest> virtual_member_pointer_thunks_;
  map<string, VTableEntryThunkRequest> vtable_entry_thunks_;
  map<string, string> external_function_symbols_;
  map<string, string> external_object_symbols_;
  set<string> c_linkage_global_symbols_;
  set<string> runtime_bridge_support_symbols_;
  map<string, TypePtr> exception_rtti_symbols_;
  set<string> rtti_definition_symbols_;
  set<string> referenced_global_symbols_;
  set<string> referenced_function_symbols_;
  mutable map<string, TypePtr> referenced_function_signature_types_;
  map<string, set<string> > function_references_;
  map<string, symbol_linkage::SymbolIdentity> exported_symbols_;
  vector<lowir_internal::ObjectAlias> object_aliases_;
  map<string, string> object_alias_targets_;
  map<string, string> thread_local_wrapper_targets_;
  map<string, string> thread_local_wrapper_object_symbols_;
  vector<const CallSemNode *> function_nodes_;
  vector<const CallSemNode *> global_ctor_actions_;
  vector<const CallSemNode *> global_dtor_actions_;
  vector<pair<string, const CallSemNode *> > thread_local_init_actions_;
  vector<unique_ptr<CallSemNode> > synthetic_nodes_;
  mutable FunctionSymbolLookupIndex function_symbol_lookup_index_;
  mutable bool function_symbol_lookup_index_dirty_ = true;
  mutable unordered_set<string> generated_function_symbol_cache_;
  mutable size_t generated_function_symbol_cache_size_ = static_cast<size_t>(-1);
  size_t string_literal_counter_ = 0;
  bool validate_closure_ = false;
  bool emit_runtime_support_ = false;
  bool enable_debug_value_names_ = false;

  void invalidate_function_symbol_lookup_index()
  {
    function_symbol_lookup_index_dirty_ = true;
  }

  const FunctionSymbolLookupIndex & function_symbol_lookup_index() const
  {
    if(function_symbol_lookup_index_dirty_) {
      function_symbol_lookup_index_.rebuild(function_symbols_, function_symbol_entries_);
      function_symbol_lookup_index_dirty_ = false;
    }
    return function_symbol_lookup_index_;
  }

  void refresh_generated_function_symbol_cache() const
  {
    if(generated_function_symbol_cache_size_ == functions_.size()) {
      return;
    }
    generated_function_symbol_cache_.clear();
    generated_function_symbol_cache_.reserve(functions_.size());
    for(size_t i = 0; i < functions_.size(); ++i) {
      generated_function_symbol_cache_.insert(functions_[i].name);
    }
    generated_function_symbol_cache_size_ = functions_.size();
  }

  bool generated_function_symbol_exists(const string & symbol) const
  {
    refresh_generated_function_symbol_cache();
    return generated_function_symbol_cache_.count(symbol) != 0;
  }

  bool generated_global_symbol_exists(const string & symbol) const
  {
    for(size_t i = 0; i < globals_.size(); ++i) {
      if(globals_[i].name == symbol) {
        return true;
      }
    }
    return false;
  }

  bool function_symbol_has_trivial_lifecycle(const string & symbol) const
  {
    map<string, const CallSemNode *>::const_iterator owner =
        function_symbol_nodes_.find(symbol);
    return owner != function_symbol_nodes_.end() &&
           owner->second != nullptr &&
           owner->second->trivial_lifecycle;
  }

  bool known_function_symbol_exists(const string & symbol) const
  {
    if(generated_function_symbol_exists(symbol)) {
      return true;
    }
    if(function_symbol_nodes_.count(symbol) != 0) {
      return true;
    }
    return function_symbol_lookup_index().mapped_symbols.count(symbol) != 0;
  }

  bool known_function_symbol_has_definition(const string & symbol) const
  {
    if(generated_function_symbol_exists(symbol)) {
      return true;
    }
    const FunctionSymbolEntry * entry = find_function_symbol_entry_by_symbol(symbol);
    return entry && entry->has_definition;
  }

  bool known_global_symbol_exists(const string & symbol) const
  {
    if(generated_global_symbol_exists(symbol)) {
      return true;
    }
    for(map<string, GlobalBinding>::const_iterator it = global_bindings_.begin();
        it != global_bindings_.end();
        ++it) {
      if(it->second.storage == symbol) {
        return true;
      }
    }
    for(map<string, VTableBinding>::const_iterator it = vtable_bindings_.begin();
        it != vtable_bindings_.end();
        ++it) {
      if(it->second.base_symbol == symbol) {
        return true;
      }
    }
    for(map<string, string>::const_iterator it = string_literal_symbols_.begin();
        it != string_literal_symbols_.end();
        ++it) {
      if(it->second == symbol) {
        return true;
      }
    }
    for(map<string, TypePtr>::const_iterator it = exception_storage_types_.begin();
        it != exception_storage_types_.end();
        ++it) {
      if(exception_storage_symbol(it->second) == symbol) {
        return true;
      }
    }
    return false;
  }

  bool subtree_contains_kind(const CallSemNode & node, CallSemKind kind) const
  {
    if(node.kind == kind) {
      return true;
    }
    vector<const CallSemNode *> children;
    append_callsem_recursive_input_children(node, children);
    for(size_t i = 0; i < children.size(); ++i) {
      if(subtree_contains_kind(*children[i], kind)) {
        return true;
      }
    }
    return false;
  }

  symbol_linkage::SymbolIdentity rtti_symbol_identity(const string & internal_symbol,
                                                      const TypePtr & type) const
  {
    if(emit_runtime_support_ && type) {
      const string host_symbol = symbol_linkage::typeinfo_symbol_for_type(type);
      if(!host_symbol.empty()) {
        return symbol_linkage::make_object_symbol_identity(internal_symbol,
                                                           host_symbol,
                                                           symbol_linkage::SL_WEAK);
      }
    }
    return symbol_linkage::make_internal_symbol_identity(internal_symbol,
                                                         symbol_linkage::SL_WEAK);
  }

  void export_rtti_symbol(const string & internal_symbol,
                          const TypePtr & type,
                          const char * reason,
                          const string & owner = string())
  {
    set_exported_symbol(internal_symbol, rtti_symbol_identity(internal_symbol, type), reason, owner);
  }

  string host_typeinfo_name_symbol_for_type(const TypePtr & type) const
  {
    return symbol_linkage::typeinfo_name_symbol_for_type(type);
  }

  string ensure_host_typeinfo_name_global(const TypePtr & type)
  {
    const string object_symbol = host_typeinfo_name_symbol_for_type(type);
    if(object_symbol.empty()) {
      throw logic_error("failed to derive host typeinfo name symbol for " + describe_type(type));
    }
    const string internal_symbol =
        lowir_name(string("__typeinfo_name::") + describe_type(type));
    if(!has_global_name(internal_symbol)) {
      LowIRGlobal global = make_data_global(internal_symbol, true);
      const string encoded_name = object_symbol.substr(4);
      for(size_t i = 0; i < encoded_name.size(); ++i) {
        global.data_items.push_back(
            string("i8 ") +
            to_string(static_cast<unsigned int>(static_cast<unsigned char>(encoded_name[i]))));
      }
      global.data_items.push_back("i8 0");
      globals_.push_back(global);
    }
    set_exported_symbol(internal_symbol,
                        symbol_linkage::make_object_symbol_identity(internal_symbol,
                                                                    object_symbol,
                                                                    symbol_linkage::SL_WEAK),
                        "rtti-name",
                        describe_type(type));
    return internal_symbol;
  }

  string host_nonclass_typeinfo_vtable_symbol(const TypePtr & type)
  {
    TypePtr base = strip_top_level_cv(type);
    const char * internal_name = nullptr;
    const char * object_symbol = nullptr;
    if(is_void_type(base) || is_integral_type(base) || is_floating_type(base) ||
       is_nullptr_scalar_type(base)) {
      internal_name = "__external_rtti_vtable::__fundamental_type_info";
      object_symbol = "_ZTVN10__cxxabiv123__fundamental_type_infoE";
    } else if(is_named_enum_scalar_type(base)) {
      internal_name = "__external_rtti_vtable::__enum_type_info";
      object_symbol = "_ZTVN10__cxxabiv116__enum_type_infoE";
    } else if(base && base->kind == Type::TK_FUNCTION) {
      internal_name = "__external_rtti_vtable::__function_type_info";
      object_symbol = "_ZTVN10__cxxabiv120__function_type_infoE";
    } else if(base && base->kind == Type::TK_ARRAY) {
      internal_name = "__external_rtti_vtable::__array_type_info";
      object_symbol = "_ZTVN10__cxxabiv117__array_type_infoE";
    }
    if(!internal_name || !object_symbol) {
      return string();
    }
    const string internal_symbol = symbol_linkage::internal_symbol_from_name(internal_name);
    external_object_symbols_[internal_symbol] = object_symbol;
    return internal_symbol;
  }

  static bool is_named_class_like_rtti_type(const TypePtr & type)
  {
    TypePtr base = strip_top_level_cv(type);
    if(!base || base->kind != Type::TK_NAMED) {
      return false;
    }
    return base->named_key.compare(0, 6, "class ") == 0 ||
           base->named_key.compare(0, 7, "struct ") == 0 ||
           base->named_key.compare(0, 6, "union ") == 0;
  }

  void ensure_host_incomplete_class_rtti_global(const TypePtr & type,
                                                const char * reason,
                                                const string & owner = string())
  {
    if(!emit_runtime_support_ || !type || !is_named_class_like_rtti_type(type)) {
      return;
    }

    const string rtti_symbol = rtti_symbol_for_type(type);
    const LowIRGlobal * existing = find_global_name(rtti_symbol);
    if(existing && existing->kind == LowIRGlobal::LG_DATA &&
       existing->data_items.size() >= 2) {
      export_rtti_symbol(rtti_symbol, type, reason, owner);
      return;
    }

    const string typeinfo_vtable_symbol =
        symbol_linkage::internal_symbol_from_name(
            "__external_rtti_vtable::__class_type_info");
    external_object_symbols_[typeinfo_vtable_symbol] =
        "_ZTVN10__cxxabiv117__class_type_infoE";

    LowIRGlobal global = make_data_global(rtti_symbol, true);
    global.data_items.push_back(string("ptr addr ") + typeinfo_vtable_symbol + " + 16");
    global.data_items.push_back(string("ptr addr ") + ensure_host_typeinfo_name_global(type));

    LowIRGlobal * replacement = find_global_name(rtti_symbol);
    if(replacement) {
      *replacement = global;
    } else {
      globals_.push_back(global);
    }
    export_rtti_symbol(rtti_symbol, type, reason, owner);
  }

  unsigned int host_pointer_rtti_flags_for_pointee(const TypePtr & pointee) const
  {
    unsigned int flags = 0;
    TypePtr current = pointee;
    if(current && current->kind == Type::TK_CV) {
      if(current->cv_const) {
        flags |= 0x1u;
      }
      if(current->cv_volatile) {
        flags |= 0x2u;
      }
      current = current->inner;
    }

    TypePtr base = strip_top_level_cv(current);
    if(base && is_named_class_like_rtti_type(base) && !base->named_has_layout) {
      flags |= 0x8u;
    }
    return flags;
  }

  string host_pointer_pointee_typeinfo_reference_symbol(const TypePtr & pointee)
  {
    TypePtr base = strip_top_level_cv(pointee);
    if(base && is_named_class_like_rtti_type(base) && !base->named_has_layout) {
      ensure_host_incomplete_class_rtti_global(base, "pointer-rtti-pointee");
      return rtti_symbol_for_type(base);
    }
    return host_typeinfo_reference_symbol(base);
  }

  bool ensure_host_pointer_rtti_global(const TypePtr & type,
                                       const char * reason,
                                       const string & owner = string())
  {
    TypePtr base = strip_top_level_cv(type);
    if(!emit_runtime_support_ || !base || base->kind != Type::TK_POINTER ||
       !base->inner) {
      return false;
    }

    const string rtti_symbol = rtti_symbol_for_type(type);
    const LowIRGlobal * existing = find_global_name(rtti_symbol);
    if(existing && existing->kind == LowIRGlobal::LG_DATA &&
       existing->data_items.size() >= 4) {
      export_rtti_symbol(rtti_symbol, type, reason, owner);
      return true;
    }

    const string typeinfo_vtable_symbol =
        symbol_linkage::internal_symbol_from_name(
            "__external_rtti_vtable::__pointer_type_info");
    external_object_symbols_[typeinfo_vtable_symbol] =
        "_ZTVN10__cxxabiv119__pointer_type_infoE";

    LowIRGlobal global = make_data_global(rtti_symbol, true);
    global.data_items.push_back(string("ptr addr ") + typeinfo_vtable_symbol + " + 16");
    global.data_items.push_back(string("ptr addr ") + ensure_host_typeinfo_name_global(type));
    global.data_items.push_back(
        string("i32 ") + to_string(host_pointer_rtti_flags_for_pointee(base->inner)));
    global.data_items.push_back(
        string("ptr addr ") + host_pointer_pointee_typeinfo_reference_symbol(base->inner));

    LowIRGlobal * replacement = find_global_name(rtti_symbol);
    if(replacement) {
      *replacement = global;
    } else {
      globals_.push_back(global);
    }
    export_rtti_symbol(rtti_symbol, type, reason, owner);
    return true;
  }

  bool ensure_host_nonclass_rtti_global(const TypePtr & type,
                                        const char * reason,
                                        const string & owner = string())
  {
    if(!emit_runtime_support_ || !type || is_complete_class_value_type(type)) {
      return false;
    }
    if(ensure_host_pointer_rtti_global(type, reason, owner)) {
      return true;
    }
    const string typeinfo_vtable_symbol = host_nonclass_typeinfo_vtable_symbol(type);
    if(typeinfo_vtable_symbol.empty()) {
      return false;
    }

    const string rtti_symbol = rtti_symbol_for_type(type);
    const LowIRGlobal * existing = find_global_name(rtti_symbol);
    if(existing && existing->kind == LowIRGlobal::LG_DATA &&
       existing->data_items.size() >= 2) {
      export_rtti_symbol(rtti_symbol, type, reason, owner);
      return true;
    }

    LowIRGlobal global = make_data_global(rtti_symbol, true);
    global.data_items.push_back(string("ptr addr ") + typeinfo_vtable_symbol + " + 16");
    global.data_items.push_back(string("ptr addr ") + ensure_host_typeinfo_name_global(type));

    LowIRGlobal * replacement = find_global_name(rtti_symbol);
    if(replacement) {
      *replacement = global;
    } else {
      globals_.push_back(global);
    }
    export_rtti_symbol(rtti_symbol, type, reason, owner);
    return true;
  }

  string host_typeinfo_reference_symbol(const TypePtr & type)
  {
    const string internal_symbol = rtti_symbol_for_type(type);
    if(has_global_name(internal_symbol)) {
      return internal_symbol;
    }
    if(emit_runtime_support_ && type && is_complete_class_value_type(type)) {
      if(rtti_definition_symbols_.count(internal_symbol) != 0 ||
         !is_host_runtime_rtti_class_type(type)) {
        ensure_host_exception_class_rtti_global(type);
        return internal_symbol;
      }
      const string host_symbol = symbol_linkage::typeinfo_symbol_for_type(type);
      if(!host_symbol.empty()) {
        const string external_symbol =
            symbol_linkage::internal_symbol_from_name("__external_rtti::" +
                                                      describe_type(type));
        external_object_symbols_[external_symbol] = host_symbol;
        return external_symbol;
      }
    }
    if(ensure_host_nonclass_rtti_global(type, "rtti-reference")) {
      return internal_symbol;
    }
    const string host_symbol = symbol_linkage::typeinfo_symbol_for_type(type);
    if(host_symbol.empty()) {
      return internal_symbol;
    }
    const string external_symbol =
        symbol_linkage::internal_symbol_from_name("__external_rtti::" + describe_type(type));
    external_object_symbols_[external_symbol] = host_symbol;
    return external_symbol;
  }

  struct HostRttiBaseInfo
  {
    string key;
    TypePtr type;
    long long offset = 0;
    bool is_public = false;
    bool is_virtual = false;
  };

  unsigned long long host_vtable_address_point_offset(const CallSemNode & node) const
  {
    const CallSemVirtualBaseLayout & virtual_base_layout =
        callsem_virtual_base_layout(node);
    if(!virtual_base_layout.empty()) {
      return (static_cast<unsigned long long>(virtual_base_layout.size()) + 2ULL) * 8ULL;
    }
    return node.is_primary_vtable ? 16ULL : 0ULL;
  }

  long long host_virtual_base_rtti_offset(const CallSemNode & node,
                                          const string & base_key,
                                          long long fallback) const
  {
    const CallSemVirtualBaseLayout & virtual_base_layout =
        callsem_virtual_base_layout(node);
    if(virtual_base_layout.empty()) {
      return fallback;
    }
    const unsigned long long address_point = host_vtable_address_point_offset(node);
    for(size_t i = 0; i < virtual_base_layout.size(); ++i) {
      if(virtual_base_layout[i].first == base_key) {
        return static_cast<long long>(i * 8ULL) -
               static_cast<long long>(address_point);
      }
    }
    return fallback;
  }

  vector<HostRttiBaseInfo> host_rtti_direct_bases(const CallSemNode & vtable_node) const
  {
    vector<HostRttiBaseInfo> bases;
    for(size_t i = 0; i < vtable_node.children.size(); ++i) {
      const CallSemNode & child = vtable_node.children[i];
      if(child.kind != CallSemKind::rtti_base || !child.semantic_type) {
        continue;
      }
      HostRttiBaseInfo base;
      base.key = child.text;
      base.type = child.semantic_type;
      base.offset = child.has_int_value ? callsem_int_value(child) : 0;
      base.is_public = child.is_public_access;
      base.is_virtual = child.is_virtual_base_subobject;
      if(base.is_virtual) {
        base.offset = host_virtual_base_rtti_offset(vtable_node, base.key, base.offset);
      }
      bases.push_back(base);
    }
    return bases;
  }

  const CallSemNode * find_primary_vtable_definition(const CallSemNode & node,
                                                     const TypePtr & type) const
  {
    if(node.kind == CallSemKind::vtable_definition &&
       node.is_primary_vtable &&
       node.semantic_type &&
       type &&
       describe_type(strip_top_level_cv(node.semantic_type)) ==
           describe_type(strip_top_level_cv(type))) {
      return &node;
    }
    for(size_t i = 0; i < node.children.size(); ++i) {
      if(const CallSemNode * found = find_primary_vtable_definition(node.children[i], type)) {
        return found;
      }
    }
    return nullptr;
  }

  vector<HostRttiBaseInfo> host_rtti_direct_bases(const TypePtr & type) const
  {
    if(!type) {
      return vector<HostRttiBaseInfo>();
    }
    for(size_t i = 0; i < translation_units_.size(); ++i) {
      if(const CallSemNode * found =
             find_primary_vtable_definition(translation_units_[i], type)) {
        return host_rtti_direct_bases(*found);
      }
    }
    return vector<HostRttiBaseInfo>();
  }

  void ensure_host_class_rtti_global(const TypePtr & type,
                                     const vector<HostRttiBaseInfo> & bases,
                                     const char * reason,
                                     const string & owner = string())
  {
    if(!emit_runtime_support_ || !type) {
      return;
    }
    const string rtti_symbol = rtti_symbol_for_type(type);
    const LowIRGlobal * existing = find_global_name(rtti_symbol);
    if(existing && existing->kind == LowIRGlobal::LG_DATA) {
      const bool existing_has_base_payload = existing->data_items.size() > 2;
      if(bases.empty() || existing_has_base_payload) {
        export_rtti_symbol(rtti_symbol, type, reason, owner);
        return;
      }
    }

    const bool use_si_class_typeinfo =
        bases.size() == 1 && bases[0].type && bases[0].is_public &&
        !bases[0].is_virtual && bases[0].offset == 0;
    const bool use_vmi_class_typeinfo =
        !bases.empty() && !use_si_class_typeinfo;
    const string typeinfo_vtable_symbol =
        symbol_linkage::internal_symbol_from_name(
            use_vmi_class_typeinfo ? "__external_rtti_vtable::__vmi_class_type_info" :
            use_si_class_typeinfo ? "__external_rtti_vtable::__si_class_type_info" :
                                    "__external_rtti_vtable::__class_type_info");
    external_object_symbols_[typeinfo_vtable_symbol] =
        use_vmi_class_typeinfo ? "_ZTVN10__cxxabiv121__vmi_class_type_infoE" :
        use_si_class_typeinfo ? "_ZTVN10__cxxabiv120__si_class_type_infoE" :
                                "_ZTVN10__cxxabiv117__class_type_infoE";

    LowIRGlobal global = make_data_global(rtti_symbol, true);
    global.data_items.push_back(string("ptr addr ") + typeinfo_vtable_symbol + " + 16");
    global.data_items.push_back(string("ptr addr ") + ensure_host_typeinfo_name_global(type));
    if(use_vmi_class_typeinfo) {
      global.data_items.push_back("i32 0");
      global.data_items.push_back(string("i32 ") + to_string(bases.size()));
      for(size_t i = 0; i < bases.size(); ++i) {
        const HostRttiBaseInfo & base = bases[i];
        const long long flags = (base.is_virtual ? 1LL : 0LL) | (base.is_public ? 2LL : 0LL);
        const long long offset_flags = (base.offset << 8) | flags;
        global.data_items.push_back(
            string("ptr addr ") + host_typeinfo_reference_symbol(base.type));
        global.data_items.push_back(string("i64 ") + to_string(offset_flags));
      }
    } else if(use_si_class_typeinfo) {
      global.data_items.push_back(
          string("ptr addr ") + host_typeinfo_reference_symbol(bases[0].type));
    }
    LowIRGlobal * replacement = find_global_name(rtti_symbol);
    if(replacement) {
      *replacement = global;
    } else {
      globals_.push_back(global);
    }
    export_rtti_symbol(rtti_symbol, type, reason, owner);
  }

  void ensure_host_class_rtti_global(const CallSemNode & vtable_node)
  {
    if(!emit_runtime_support_ || !vtable_node.semantic_type) {
      return;
    }
    ensure_host_class_rtti_global(vtable_node.semantic_type,
                                  host_rtti_direct_bases(vtable_node),
                                  "vtable-rtti",
                                  vtable_node.text);
  }

  void ensure_host_exception_class_rtti_global(const TypePtr & type)
  {
    if(!emit_runtime_support_ || !type || !is_complete_class_value_type(type)) {
      return;
    }
    ensure_host_class_rtti_global(type,
                                  host_rtti_direct_bases(type),
                                  "exception-rtti-class");
  }

  void emit_rtti_definition_global(const CallSemNode & node)
  {
    if(node.text.empty()) {
      return;
    }
    if(emit_runtime_support_ &&
       node.semantic_type &&
       is_complete_class_value_type(node.semantic_type)) {
      vector<HostRttiBaseInfo> bases = host_rtti_direct_bases(node);
      if(bases.empty()) {
        bases = host_rtti_direct_bases(node.semantic_type);
      }
      ensure_host_class_rtti_global(node.semantic_type,
                                    bases,
                                    "rtti-definition",
                                    node.text);
      return;
    }
    if(ensure_host_nonclass_rtti_global(node.semantic_type,
                                        "rtti-definition",
                                        node.text)) {
      return;
    }
    if(has_global_name(node.text)) {
      return;
    }
    LowIRGlobal global = make_scalar_global(node.text, "i64", "zero", false);
    globals_.push_back(global);
    set_exported_symbol(node.text,
                        symbol_linkage::make_internal_symbol_identity(
                            node.text,
                            symbol_linkage::SL_WEAK),
                        "rtti-definition",
                        node.text);
  }

  bool subtree_contains_nontrivial_destructor_action(const CallSemNode & node) const
  {
    if(node.kind == CallSemKind::destructor_action && !node.trivial_lifecycle) {
      return true;
    }
    vector<const CallSemNode *> children;
    append_callsem_recursive_input_children(node, children);
    for(size_t i = 0; i < children.size(); ++i) {
      if(subtree_contains_nontrivial_destructor_action(*children[i])) {
        return true;
      }
    }
    return false;
  }

  void note_output_export_event(const char * action,
                                const string & symbol,
                                const symbol_linkage::SymbolIdentity * identity,
                                const string & reason,
                                const string & owner = string(),
                                const string & detail = string()) const
  {
    if(!parser_trace::enabled("output.export")) {
      return;
    }

    ostringstream trace;
    trace << "action=" << action
          << " symbol=" << symbol
          << " reason=" << reason;
    if(identity) {
      trace << " internal=" << identity->internal_symbol
            << " object=" << identity->object_symbol
            << " linkage=" << exported_linkage_name(identity->linkage);
    }
    if(!owner.empty()) {
      trace << " owner=" << owner;
    }
    if(!detail.empty()) {
      trace << " detail=" << detail;
    }
    parser_trace::note("output.export", string(), trace.str());
  }

  static const char * trace_bool_name(bool value)
  {
    return value ? "yes" : "no";
  }

  void note_output_export_closure_state(
      const string & symbol,
      const symbol_linkage::SymbolIdentity * identity,
      const string & reason,
      bool known_function,
      bool known_global,
      bool external_function,
      bool external_object,
      bool referenced_function,
      bool referenced_global,
      bool runtime_reserved,
      bool backend_passthrough) const
  {
    if(!parser_trace::enabled("output.export")) {
      return;
    }

    ostringstream trace;
    trace << "action=missing-closure"
          << " symbol=" << symbol
          << " reason=" << reason;
    if(identity) {
      trace << " internal=" << identity->internal_symbol
            << " object=" << identity->object_symbol
            << " linkage=" << exported_linkage_name(identity->linkage);
    }
    trace << " known-function=" << trace_bool_name(known_function)
          << " known-global=" << trace_bool_name(known_global)
          << " external-function=" << trace_bool_name(external_function)
          << " external-object=" << trace_bool_name(external_object)
          << " referenced-function=" << trace_bool_name(referenced_function)
          << " referenced-global=" << trace_bool_name(referenced_global)
          << " runtime-reserved=" << trace_bool_name(runtime_reserved)
          << " backend-passthrough=" << trace_bool_name(backend_passthrough);
    parser_trace::note("output.export", string(), trace.str());
  }

  static int callsem_owner_rank(CallSemKind kind)
  {
    switch(kind) {
    case CallSemKind::function_definition:
    case CallSemKind::variable:
    case CallSemKind::vtable_definition:
    case CallSemKind::vtt_definition:
    case CallSemKind::rtti_definition:
      return 4;

    case CallSemKind::function_declaration:
    case CallSemKind::simple_declaration:
      return 3;

    default:
      return 0;
    }
  }

  void collect_pre_lowir_symbol_owner(const CallSemNode & node,
                                      const string & symbol,
                                      const CallSemNode *& best,
                                      int & best_rank,
                                      size_t & match_count,
                                      bool & saw_definition,
                                      bool & saw_declaration) const
  {
    if(callsem_symbol(node).internal_symbol == symbol) {
      ++match_count;
      const int rank = callsem_owner_rank(node.kind);
      if(rank > best_rank) {
        best = &node;
        best_rank = rank;
      }
      if(node.kind == CallSemKind::function_definition ||
         node.kind == CallSemKind::variable ||
         node.kind == CallSemKind::vtable_definition ||
         node.kind == CallSemKind::vtt_definition ||
         node.kind == CallSemKind::rtti_definition) {
        saw_definition = true;
      }
      if(node.kind == CallSemKind::function_declaration ||
         node.kind == CallSemKind::simple_declaration) {
        saw_declaration = true;
      }
    }

    for(size_t i = 0; i < node.children.size(); ++i) {
      collect_pre_lowir_symbol_owner(node.children[i],
                                     symbol,
                                     best,
                                     best_rank,
                                     match_count,
                                     saw_definition,
                                     saw_declaration);
    }
  }

  string describe_pre_lowir_owner_state(const string & symbol) const
  {
    const CallSemNode * best = nullptr;
    int best_rank = -1;
    size_t match_count = 0;
    bool saw_definition = false;
    bool saw_declaration = false;
    for(size_t i = 0; i < translation_units_.size(); ++i) {
      collect_pre_lowir_symbol_owner(translation_units_[i],
                                     symbol,
                                     best,
                                     best_rank,
                                     match_count,
                                     saw_definition,
                                     saw_declaration);
    }

    std::ostringstream out;
    out << "pre-lowir-owner=";
    if(!best) {
      out << "missing";
      return out.str();
    }

    out << callsem_kind_text(best->kind)
        << " matches=" << match_count
        << " declarations=" << trace_bool_name(saw_declaration)
        << " definitions=" << trace_bool_name(saw_definition)
        << " text=" << callsem_display_text(*best);
    const symbol_linkage::SymbolIdentity & best_symbol = callsem_symbol(*best);
    if(!best_symbol.object_symbol.empty()) {
      out << " object=" << best_symbol.object_symbol;
    }
    return out.str();
  }

  void set_exported_symbol(const string & symbol,
                           const symbol_linkage::SymbolIdentity & identity,
                           const string & reason,
                           const string & owner = string())
  {
    symbol_linkage::SymbolIdentity normalized = identity;
    if(normalized.internal_symbol.empty()) {
      normalized.internal_symbol = symbol;
    }
    const char * action =
        exported_symbols_.count(symbol) != 0 ? "update" : "insert";
    exported_symbols_[symbol] = normalized;
    note_output_export_event(action, symbol, &normalized, reason, owner);
  }

  void add_object_alias(const string & object_symbol,
                        const string & target_symbol)
  {
    if(object_symbol.empty() || target_symbol.empty()) {
      return;
    }
    map<string, string>::const_iterator existing =
        object_alias_targets_.find(object_symbol);
    if(existing != object_alias_targets_.end()) {
      if(existing->second != target_symbol) {
        throw logic_error("conflicting LowIR object alias target for " + object_symbol);
      }
      return;
    }
    lowir_internal::ObjectAlias alias;
    alias.object_symbol = object_symbol;
    alias.target = target_symbol;
    object_alias_targets_[object_symbol] = target_symbol;
    object_aliases_.push_back(alias);
  }

  void maybe_add_special_member_base_alias(const CallSemNode & node,
                                           const string & target_symbol)
  {
    const vector<string> & aliases = callsem_object_aliases(node);
    for(size_t i = 0; i < aliases.size(); ++i) {
      add_object_alias(aliases[i], target_symbol);
    }
  }

  void note_runtime_function_symbol_identity(const string & symbol,
                                             const symbol_linkage::SymbolIdentity & identity,
                                             const string & reason,
                                             const string & owner)
  {
    if(!symbol_linkage::has_object_symbol(identity)) {
      return;
    }

    if(exported_symbols_.count(symbol) != 0) {
      return;
    }

    map<string, string>::const_iterator external =
        external_function_symbols_.find(symbol);
    if(external != external_function_symbols_.end()) {
      if(symbol_linkage::has_exported_object_symbol(identity) &&
         external->second != identity.object_symbol) {
        throw logic_error("conflicting external function alias for " + symbol);
      }
      return;
    }

    if(known_function_symbol_exists(symbol)) {
      if(identity.prefer_local_object_binding &&
         !known_function_symbol_has_definition(symbol) &&
         symbol_linkage::has_exported_object_symbol(identity)) {
        external_function_symbols_[symbol] = identity.object_symbol;
        return;
      }
      set_exported_symbol(symbol, identity, reason, owner);
      return;
    }

    if(symbol_linkage::has_exported_object_symbol(identity)) {
      external_function_symbols_[symbol] = identity.object_symbol;
      return;
    }

    set_exported_symbol(symbol, identity, reason, owner);
  }

  bool is_backend_passthrough_symbol(const string & symbol) const
  {
    if(symbol.empty() || symbol[0] != '@') {
      return false;
    }
    if(symbol.size() >= 13 &&
       symbol.compare(symbol.size() - 13, 13, "__tls_wrapper") == 0) {
      return true;
    }
    if(symbol.compare(0, 11, "@__builtin_") == 0 ||
       symbol.compare(0, 10, "@__atomic_") == 0 ||
       symbol.compare(0, 14, "@__c11_atomic_") == 0 ||
       symbol == "@__pseudo_destructor") {
      return true;
    }
    return false;
  }

  void validate_symbol_closure() const
  {
    for(set<string>::const_iterator it = referenced_function_symbols_.begin();
        it != referenced_function_symbols_.end();
        ++it) {
      const bool known_function = known_function_symbol_exists(*it);
      const bool exported = exported_symbols_.count(*it) != 0;
      const bool runtime_reserved = eh_runtime::is_reserved_symbol(*it);
      const bool backend_passthrough = is_backend_passthrough_symbol(*it);
      if(known_function || exported || runtime_reserved) {
        continue;
      }
      if(backend_passthrough) {
        continue;
      }
      note_output_export_closure_state(*it,
                                       nullptr,
                                       "referenced-function",
                                       known_function,
                                       false,
                                       false,
                                       false,
                                       true,
                                       false,
                                       runtime_reserved,
                                       backend_passthrough);
      throw logic_error("lowir referenced function symbol missing closure owner " + *it);
    }

    for(set<string>::const_iterator it = referenced_global_symbols_.begin();
        it != referenced_global_symbols_.end();
        ++it) {
      const bool known_global = known_global_symbol_exists(*it);
      const bool exported = exported_symbols_.count(*it) != 0;
      const bool backend_passthrough = is_backend_passthrough_symbol(*it);
      if(known_global || exported) {
        continue;
      }
      if(backend_passthrough) {
        continue;
      }
      note_output_export_closure_state(*it,
                                       nullptr,
                                       "referenced-global",
                                       false,
                                       known_global,
                                       false,
                                       false,
                                       false,
                                       true,
                                       false,
                                       backend_passthrough);
      throw logic_error("lowir referenced global symbol missing closure owner " + *it);
    }

    for(map<string, symbol_linkage::SymbolIdentity>::const_iterator it =
            exported_symbols_.begin();
        it != exported_symbols_.end();
        ++it) {
      if(it->first.empty()) {
        throw logic_error("lowir exported symbol missing internal name");
      }
      const bool known_function = known_function_symbol_exists(it->first);
      const bool known_global = known_global_symbol_exists(it->first);
      const bool external_function = external_function_symbols_.count(it->first) != 0;
      const bool external_object = external_object_symbols_.count(it->first) != 0;
      const bool referenced_function = referenced_function_symbols_.count(it->first) != 0;
      const bool referenced_global = referenced_global_symbols_.count(it->first) != 0;
      const bool runtime_reserved = eh_runtime::is_reserved_symbol(it->first);
      const bool backend_passthrough = is_backend_passthrough_symbol(it->first);
      if(known_function ||
         known_global ||
         external_function ||
         external_object ||
         referenced_function ||
         referenced_global ||
         runtime_reserved) {
        continue;
      }
      if(backend_passthrough) {
        continue;
      }
      note_output_export_closure_state(it->first,
                                       &it->second,
                                       "exported-symbol",
                                       known_function,
                                       known_global,
                                       external_function,
                                       external_object,
                                       referenced_function,
                                       referenced_global,
                                       runtime_reserved,
                                       backend_passthrough);
      const string owner_audit = describe_pre_lowir_owner_state(it->first);
      if(parser_trace::enabled("output.audit")) {
        ostringstream trace;
        trace << "action=lowir-missing-semantic-owner"
              << " symbol=" << it->first
              << " detail=" << owner_audit;
        parser_trace::note("output.audit", string(), trace.str());
      }
      throw logic_error("lowir exported symbol missing semantic owner " +
                        it->first + " [" + owner_audit + "]");
    }
  }

  string lookup_function_symbol(const string & name, const TypePtr & type) const
  {
    const string symbol =
        try_lookup_function_symbol_with_index(function_symbols_,
                                              function_symbol_entries_,
                                              function_symbol_lookup_index(),
                                              name,
                                              type);
    const string resolved = symbol.empty() ? lowir_name(name) : symbol;
    note_referenced_function_signature(resolved, type);
    return resolved;
  }

  string lookup_function_symbol(const CallSemNode & node) const
  {
    const string lookup_name =
        callsem_resolved_name(node).empty() ? node.text.str() :
            callsem_resolved_name(node);
    if(!callsem_symbol(node).internal_symbol.empty()) {
      note_referenced_function_signature(callsem_symbol(node).internal_symbol, node.semantic_type);
      return callsem_symbol(node).internal_symbol;
    }
    return lookup_function_symbol(lookup_name, node.semantic_type);
  }

  string lookup_runtime_reference_function_symbol(const CallSemNode & node) const
  {
    if(!callsem_symbol(node).internal_symbol.empty()) {
      if(known_function_symbol_exists(callsem_symbol(node).internal_symbol) ||
         symbol_linkage::has_exported_object_symbol(callsem_symbol(node)) ||
         eh_runtime::is_reserved_symbol(callsem_symbol(node).internal_symbol) ||
         is_backend_passthrough_symbol(callsem_symbol(node).internal_symbol)) {
        return callsem_symbol(node).internal_symbol;
      }
      return string();
    }
    return try_lookup_function_symbol_with_index(function_symbols_,
                                                 function_symbol_entries_,
                                                 function_symbol_lookup_index(),
                                                 node.text,
                                                 node.semantic_type);
  }

  string register_virtual_member_pointer_thunk(const CallSemNode & node)
  {
    TypePtr member_pointer_type = strip_top_level_cv(node.semantic_type);
    if(!member_pointer_type ||
       member_pointer_type->kind != Type::TK_MEMBER_POINTER ||
       !is_function_type(member_pointer_type->inner)) {
      throw logic_error("virtual member pointer thunk requires member function pointer type");
    }

    string target_symbol = callsem_symbol(node).internal_symbol;
    if(target_symbol.empty() && !node.children.empty()) {
      target_symbol = lookup_function_symbol(node.children[0]);
    }
    if(target_symbol.empty()) {
      throw logic_error("virtual member pointer thunk missing target symbol");
    }

    const string thunk_symbol = virtual_member_pointer_thunk_symbol(target_symbol);
    VirtualMemberPointerThunkRequest & request = virtual_member_pointer_thunks_[thunk_symbol];
    if(request.symbol.empty()) {
      request.symbol = thunk_symbol;
      request.member_pointer_type = member_pointer_type;
      request.virtual_slot = node.has_uint_value ? callsem_uint_value(node) : 0ULL;
      request.uses_extended_vtable_layout = node.uses_extended_vtable_layout;
    }
    return thunk_symbol;
  }

  LowIRFunction build_virtual_member_pointer_thunk(
      const VirtualMemberPointerThunkRequest & request) const
  {
    TypePtr member_pointer_type = strip_top_level_cv(request.member_pointer_type);
    TypePtr callable_function_type =
        callable_function_type_for_member_pointer(member_pointer_type);
    if(!member_pointer_type ||
       member_pointer_type->kind != Type::TK_MEMBER_POINTER ||
       !is_function_type(member_pointer_type->inner) ||
       !callable_function_type ||
       callable_function_type->kind != Type::TK_FUNCTION ||
       callable_function_type->params.empty()) {
      throw logic_error("invalid virtual member pointer thunk request");
    }

    LowIRFunction function;
    function.name = request.symbol;
    function.boundary_metadata.arity = lowir_call_arity_for(callable_function_type);
    apply_known_function_boundary_metadata(function.boundary_metadata, function.name);

    TypePtr result_type = callable_function_type->inner;
    const bool indirect_class_return = lowir_uses_indirect_result_boundary(result_type);
    function.return_type = indirect_class_return ? "void" : lowir_result_type_text(result_type);
    if(indirect_class_return) {
      function.params.push_back(
          make_lowir_parameter_text("%ret", "ptr", lowir_internal::PPM_INDIRECT_RESULT));
    }

    function.params.push_back(
        make_lowir_parameter_text("%this",
                                  "ptr",
                                  lowir_parameter_passing_mode(callable_function_type->params[0],
                                                               callable_function_type->params[0])));
    ParameterVirtualBaseLayout parameter_virtual_base_layout;
    const bool has_parameter_virtual_base_layout =
        infer_function_type_reference_parameter_virtual_base_layout(
            callable_function_type,
            class_virtual_base_layouts_,
            parameter_virtual_base_layout);
    vector<vector<string> > forwarded_arg_names(callable_function_type->params.size());
    forwarded_arg_names[0].push_back("%this");
    for(size_t i = 1; i < callable_function_type->params.size(); ++i) {
      const TypePtr lowered_param_type = lowir_parameter_type_for(callable_function_type->params[i]);
      const vector<string> abi_types =
          lowir_parameter_abi_type_texts(callable_function_type->params[i]);
      const string direct_object_type = lowir_direct_object_type(lowered_param_type);
      const lowir_internal::ParamPassingMode passing =
          !direct_object_type.empty() ?
              lowir_internal::PPM_DIRECT :
              lowir_parameter_passing_mode(callable_function_type->params[i], lowered_param_type);
      for(size_t chunk_index = 0; chunk_index < abi_types.size(); ++chunk_index) {
        const string arg_name =
            chunk_index == 0 ?
                (string("%arg") + to_string(i)) :
                (string("%arg") + to_string(i) + "__" + to_string(chunk_index));
        function.params.push_back(make_lowir_parameter_text(arg_name, abi_types[chunk_index], passing));
        forwarded_arg_names[i].push_back(arg_name);
      }
    }
    vector<string> forwarded_hidden_virtual_base_args;
    if(has_parameter_virtual_base_layout) {
      for(size_t i = 0; i < parameter_virtual_base_layout.layout.size(); ++i) {
        const string arg_name = string("%__pvbptr") + to_string(i);
        function.params.push_back(make_lowir_parameter_text(arg_name, "ptr"));
        forwarded_hidden_virtual_base_args.push_back(arg_name);
      }
    }

    LowIRBlock entry;
    entry.label = "^entry";

    size_t temp_counter = 0;
    const auto next_temp =
        [&temp_counter]() -> string
        {
          ++temp_counter;
          return string("%t") + to_string(temp_counter);
        };
    const auto emit_value =
        [&entry, &next_temp](const string & op) -> string
        {
          const string temp = next_temp();
          entry.instructions.push_back(temp + " = " + op);
          return temp;
        };

    const string vtable_ptr = emit_value("load ptr %this");
    string dispatch_object = "%this";
    string entry_ptr;
    if(request.uses_extended_vtable_layout) {
      entry_ptr = vtable_ptr;
      const unsigned long long slot_offset = request.virtual_slot * 16ULL;
      if(slot_offset != 0) {
        entry_ptr =
            emit_value(string("index i8 ") + vtable_ptr + ", " + to_string(slot_offset));
      }
      const string adjust_ptr = emit_value(string("index i8 ") + entry_ptr + ", 8");
      const string this_adjust = emit_value(string("load i64 ") + adjust_ptr);
      dispatch_object =
          emit_value(string("index i8 %this, ") + this_adjust);
    } else if(request.virtual_slot != 0) {
      entry_ptr =
          emit_value(string("index i8 ") + vtable_ptr + ", " + to_string(request.virtual_slot * 8ULL));
    } else {
      entry_ptr = vtable_ptr;
    }

    const string fn_ptr = emit_value(string("load ptr ") + entry_ptr);
    ostringstream call;
    call << "call " << function.return_type << " " << fn_ptr << "(";
    bool first_arg = true;
    if(indirect_class_return) {
      call << "%ret";
      first_arg = false;
    }
    if(!first_arg) {
      call << ", ";
    }
    call << dispatch_object;
    for(size_t i = 1; i < callable_function_type->params.size(); ++i) {
      for(size_t chunk_index = 0; chunk_index < forwarded_arg_names[i].size(); ++chunk_index) {
        call << ", " << forwarded_arg_names[i][chunk_index];
      }
    }
    for(size_t i = 0; i < forwarded_hidden_virtual_base_args.size(); ++i) {
      call << ", " << forwarded_hidden_virtual_base_args[i];
    }
    call << ")";
    LowIRFunctionSignatureText call_signature =
        lowir_function_signature_text(callable_function_type);
    if(has_parameter_virtual_base_layout) {
      append_parameter_virtual_base_signature_params_for_layout(
          call_signature,
          parameter_virtual_base_layout);
    }
    call << lowir_call_signature_suffix(call_signature);

    if(function.return_type == "void") {
      entry.instructions.push_back(call.str());
      entry.instructions.push_back("return void");
    } else {
      const string result_value = emit_value(call.str());
      entry.instructions.push_back(string("return ") + function.return_type + " " + result_value);
    }
    entry.terminated = true;
    function.blocks.push_back(entry);
    return function;
  }

  void emit_virtual_member_pointer_thunks()
  {
    for(map<string, VirtualMemberPointerThunkRequest>::const_iterator it =
            virtual_member_pointer_thunks_.begin();
        it != virtual_member_pointer_thunks_.end();
        ++it) {
      if(generated_function_symbol_exists(it->first)) {
        continue;
      }
      functions_.push_back(build_virtual_member_pointer_thunk(it->second));
    }
  }

  LowIRFunction build_vtable_entry_thunk(const VTableEntryThunkRequest & request) const
  {
    TypePtr function_type = strip_top_level_cv(request.function_type);
    if(!function_type || function_type->kind != Type::TK_FUNCTION) {
      throw logic_error("invalid vtable entry thunk request");
    }

    LowIRFunction function;
    function.name = request.symbol;
    function.boundary_metadata.arity = lowir_call_arity_for(function_type);
    apply_known_function_boundary_metadata(function.boundary_metadata, function.name);
    TypePtr result_type = function_type->inner;
    const bool indirect_class_return = lowir_uses_indirect_result_boundary(result_type);
    function.return_type = indirect_class_return ? "void" : lowir_result_type_text(result_type);
    if(indirect_class_return) {
      function.params.push_back(
          make_lowir_parameter_text("%ret", "ptr", lowir_internal::PPM_INDIRECT_RESULT));
    }
    vector<vector<string> > forwarded_arg_names(function_type->params.size());
    for(size_t i = 0; i < function_type->params.size(); ++i) {
      const TypePtr lowered_param_type = lowir_parameter_type_for(function_type->params[i]);
      const vector<string> abi_types = lowir_parameter_abi_type_texts(function_type->params[i]);
      const string direct_object_type = lowir_direct_object_type(lowered_param_type);
      const lowir_internal::ParamPassingMode passing =
          !direct_object_type.empty() ?
              lowir_internal::PPM_DIRECT :
              lowir_parameter_passing_mode(function_type->params[i], lowered_param_type);
      for(size_t chunk_index = 0; chunk_index < abi_types.size(); ++chunk_index) {
        const string arg_name =
            chunk_index == 0 ?
                (string("%arg") + to_string(i)) :
                (string("%arg") + to_string(i) + "__" + to_string(chunk_index));
        function.params.push_back(make_lowir_parameter_text(arg_name, abi_types[chunk_index], passing));
        forwarded_arg_names[i].push_back(arg_name);
      }
    }

    LowIRBlock entry;
    entry.label = "^entry";

    size_t temp_counter = 0;
    const auto next_temp =
        [&temp_counter]() -> string
        {
          ++temp_counter;
          return string("%t") + to_string(temp_counter);
        };
    const auto emit_value =
        [&entry, &next_temp](const string & op) -> string
        {
          const string temp = next_temp();
          entry.instructions.push_back(temp + " = " + op);
          return temp;
        };

    vector<string> args;
    for(size_t i = 0; i < forwarded_arg_names.size(); ++i) {
      for(size_t chunk_index = 0; chunk_index < forwarded_arg_names[i].size(); ++chunk_index) {
        args.push_back(forwarded_arg_names[i][chunk_index]);
      }
    }
    if(request.uses_vcall_offset_adjust && !args.empty()) {
      const string vtable_ptr = emit_value(string("load ptr ") + args[0]);
      const string adjust_ptr = emit_value(string("index i8 ") + vtable_ptr + ", " +
                                           to_string(request.virtual_adjust_offset));
      const string this_adjust = emit_value(string("load i64 ") + adjust_ptr);
      args[0] = emit_value(string("index i8 ") + args[0] + ", " + this_adjust);
    } else if(request.this_adjust != 0 && !args.empty()) {
      args[0] = emit_value(string("index i8 ") + args[0] + ", " +
                           to_string(request.this_adjust));
    }

    ostringstream call;
    call << "call " << function.return_type << " " << request.target_symbol << "(";
    bool first_arg = true;
    if(indirect_class_return) {
      call << "%ret";
      first_arg = false;
    }
    for(size_t i = 0; i < args.size(); ++i) {
      if(!first_arg) {
        call << ", ";
      }
      call << args[i];
      first_arg = false;
    }
    call << ")";

    if(function.return_type == "void") {
      entry.instructions.push_back(call.str());
      entry.instructions.push_back("return void");
    } else {
      string result_value = emit_value(call.str());
      if(request.return_adjust != 0 &&
         (is_pointer_type(function_type->inner) || is_reference_type(function_type->inner))) {
        result_value = emit_value(string("index i8 ") + result_value + ", " +
                                  to_string(request.return_adjust));
      }
      entry.instructions.push_back(string("return ") + function.return_type + " " + result_value);
    }
    entry.terminated = true;
    function.blocks.push_back(entry);
    return function;
  }

  void emit_vtable_entry_thunks()
  {
    for(map<string, VTableEntryThunkRequest>::const_iterator it = vtable_entry_thunks_.begin();
        it != vtable_entry_thunks_.end();
        ++it) {
      if(generated_function_symbol_exists(it->first)) {
        continue;
      }
      functions_.push_back(build_vtable_entry_thunk(it->second));
    }
  }

  LowIRFunction build_cppgm_call_terminate_support_function()
  {
    const string begin_catch = external_runtime_symbol("__cxa_begin_catch");
    const string terminate_symbol = lowir_name("std::terminate");
    note_host_std_terminate_symbol(function_symbol_entries_,
                                   external_function_symbols_,
                                   referenced_function_signature_types_,
                                   terminate_symbol);

    LowIRFunction function;
    function.name = string("@") + kCppgmCallTerminateSupportSymbol;
    function.boundary_metadata.arity = lowir_internal::CAM_FIXED;
    apply_known_function_boundary_metadata(function.boundary_metadata, function.name);
    function.return_type = "void";
    function.params.push_back(make_lowir_parameter_text("%exception", "ptr"));

    LowIRBlock entry;
    entry.label = "^entry";
    entry.instructions.push_back(string("%t1 = call ptr ") + begin_catch + "(%exception)");
    entry.instructions.push_back(string("call void ") + terminate_symbol + "()");
    entry.instructions.push_back("return void");
    entry.terminated = true;
    function.blocks.push_back(entry);
    return function;
  }

  LowIRFunction build_runtime_bridge_support_function(const string & symbol)
  {
    if(symbol == kCppgmCallTerminateSupportSymbol) {
      return build_cppgm_call_terminate_support_function();
    }

    const NumPutRuntimeBridgeSpec * spec = find_num_put_runtime_bridge_spec(symbol);
    if(!spec) {
      throw logic_error("unknown runtime bridge support function " + symbol);
    }

    LowIRFunction function;
    function.name = string("@") + symbol;
    function.boundary_metadata.arity = lowir_internal::CAM_FIXED;
    apply_known_function_boundary_metadata(function.boundary_metadata, function.name);
    function.return_type = "void";
    function.params.push_back(
        make_lowir_parameter_text("%ret", "ptr", lowir_internal::PPM_INDIRECT_RESULT));
    function.params.push_back(make_lowir_parameter_text("%facet", "ptr"));
    function.params.push_back(make_lowir_parameter_text("%iter", "ptr"));
    function.params.push_back(make_lowir_parameter_text("%iob", "ptr"));
    function.params.push_back(make_lowir_parameter_text("%fill", "i8"));
    function.params.push_back(make_lowir_parameter_text("%value", spec->value_lowir_type));

    LowIRBlock entry;
    entry.label = "^entry";

    size_t temp_counter = 0;
    const auto next_temp =
        [&temp_counter]() -> string
        {
          ++temp_counter;
          return string("%t") + to_string(temp_counter);
        };
    const auto emit_value =
        [&entry, &next_temp](const string & op) -> string
        {
          const string temp = next_temp();
          entry.instructions.push_back(temp + " = " + op);
          return temp;
        };

    const string address_point = emit_value("load ptr %facet");
    string entry_ptr = address_point;
    if(spec->slot_offset != 0) {
      entry_ptr = emit_value(string("index i8 ") + address_point + ", " +
                             to_string(spec->slot_offset));
    }
    const string fn_ptr = emit_value(string("load ptr ") + entry_ptr);
    const string iter_bits = emit_value("load ptr %iter");
    const string result =
        emit_value(string("call ptr ") + fn_ptr + "(%facet, " +
                   iter_bits + ", %iob, %fill, %value) as (%arg0 : ptr, %arg1 : ptr, %arg2 : ptr, %arg3 : i8, %arg4 : " +
                   string(spec->value_lowir_type) + ") -> ptr");
    entry.instructions.push_back("store ptr " + result + ", %ret");
    entry.instructions.push_back("return void");
    entry.terminated = true;
    function.blocks.push_back(entry);
    return function;
  }

  void emit_runtime_bridge_support_functions()
  {
    if(!emit_runtime_support_) {
      return;
    }
    for(set<string>::const_iterator it = runtime_bridge_support_symbols_.begin();
        it != runtime_bridge_support_symbols_.end();
        ++it) {
      const string internal_symbol = string("@") + *it;
      if(!generated_function_symbol_exists(internal_symbol)) {
        functions_.push_back(build_runtime_bridge_support_function(*it));
      }
      set_exported_symbol(internal_symbol,
                          symbol_linkage::make_object_symbol_identity(internal_symbol,
                                                                      *it,
                                                                      symbol_linkage::SL_WEAK),
                          "runtime-bridge-support");
    }
  }

  void collect()
  {
    {
      semantic_metrics::ScopedPhaseTimer phase("lowir.collect.rtti_definitions");
      for(size_t i = 0; i < translation_units_.size(); ++i) {
        collect_rtti_definition_symbols(translation_units_[i]);
      }
    }
    {
      semantic_metrics::ScopedPhaseTimer phase("lowir.collect.symbols");
      for(size_t i = 0; i < translation_units_.size(); ++i) {
        collect_symbols(translation_units_[i]);
      }
    }
    {
      semantic_metrics::ScopedPhaseTimer phase("lowir.collect.virtual_runtime_classes");
      for(size_t i = 0; i < translation_units_.size(); ++i) {
        collect_virtual_runtime_classes(translation_units_[i]);
      }
    }
    {
      semantic_metrics::ScopedPhaseTimer phase("lowir.collect.expression_virtual_base_layouts");
      for(size_t i = 0; i < translation_units_.size(); ++i) {
        collect_expression_virtual_base_layouts(translation_units_[i]);
      }
    }
    {
      semantic_metrics::ScopedPhaseTimer phase("lowir.collect.parameter_virtual_base_layouts");
      seed_parameter_virtual_base_layouts();
      propagate_parameter_virtual_base_layouts();
    }
    {
      semantic_metrics::ScopedPhaseTimer phase("lowir.collect.runtime_global_references");
      for(size_t i = 0; i < translation_units_.size(); ++i) {
        collect_runtime_global_references(translation_units_[i], true);
      }
    }
    {
      semantic_metrics::ScopedPhaseTimer phase("lowir.collect.runtime_function_references");
      for(size_t i = 0; i < translation_units_.size(); ++i) {
        collect_runtime_function_references(translation_units_[i], string(), true);
      }
    }
    {
      semantic_metrics::ScopedPhaseTimer phase("lowir.collect.reachable_function_symbols");
      collect_reachable_function_symbols();
    }
    {
      semantic_metrics::ScopedPhaseTimer phase("lowir.collect.string_literals");
      for(size_t i = 0; i < translation_units_.size(); ++i) {
        collect_string_literals(translation_units_[i]);
      }
    }
    {
      semantic_metrics::ScopedPhaseTimer phase("lowir.collect.exception_support");
      for(size_t i = 0; i < translation_units_.size(); ++i) {
        collect_exception_support(translation_units_[i]);
      }
    }
    {
      semantic_metrics::ScopedPhaseTimer phase("lowir.collect.scope");
      for(size_t i = 0; i < translation_units_.size(); ++i) {
        collect_scope(translation_units_[i]);
      }
    }
    {
      semantic_metrics::ScopedPhaseTimer phase("lowir.collect.post_scope_output");
      emit_referenced_output_on_use_function_definitions();
      synthesize_referenced_internal_global_definitions();
      emit_vtable_entry_thunks();
      emit_virtual_member_pointer_thunks();
      collect_exception_support_globals();
    }
  }

  void collect_rtti_definition_symbols(const CallSemNode & node)
  {
    if(node.kind == CallSemKind::rtti_definition && !node.text.empty()) {
      rtti_definition_symbols_.insert(node.text);
    }
    vector<const CallSemNode *> children;
    append_callsem_recursive_input_children(node, children);
    for(size_t i = 0; i < children.size(); ++i) {
      collect_rtti_definition_symbols(*children[i]);
    }
  }

  void collect_string_literals(const CallSemNode & node)
  {
    QuoteLiteralData literal;
    if(try_parse_string_literal_node(node, literal) &&
       string_literal_symbols_.count(node.text) == 0) {
      const vector<unsigned long long> units = string_literal_code_units(literal);
      const string item_type =
          lowir_type_for(make_fundamental(string_literal_element_type(literal)));
      ostringstream name;
      name << "@__strlit__" << (++string_literal_counter_);
      string_literal_symbols_[node.text] = name.str();
      LowIRGlobal global = make_data_global(name.str());
      for(size_t i = 0; i < units.size(); ++i) {
        global.data_items.push_back(
            item_type + " " + to_string(units[i]));
      }
      global.data_items.push_back(item_type + " 0");
      globals_.push_back(global);
    }
    vector<const CallSemNode *> children;
    append_callsem_recursive_input_children(node, children);
    for(size_t i = 0; i < children.size(); ++i) {
      collect_string_literals(*children[i]);
    }
  }

  void maybe_record_expression_virtual_base_layout(const CallSemNode & node)
  {
    const CallSemVirtualBaseLayout & virtual_base_layout =
        callsem_virtual_base_layout(node);
    if(virtual_base_layout.empty()) {
      return;
    }

    TypePtr class_type = strip_top_level_cv(remove_reference_type(node.semantic_type));
    if(class_type && class_type->kind == Type::TK_POINTER) {
      class_type = strip_top_level_cv(class_type->inner);
    }
    const string class_name = class_qualified_name(class_type);
    if(class_name.empty()) {
      return;
    }
    if(node.is_virtual_base_subobject) {
      const CallSemNode * root = peel_base_subobject_root_shared(node);
      if(!root || root == &node) {
        return;
      }
      TypePtr root_type = strip_top_level_cv(remove_reference_type(root->semantic_type));
      if(root_type && root_type->kind == Type::TK_POINTER) {
        root_type = strip_top_level_cv(root_type->inner);
      }
      const string root_class = class_qualified_name(root_type);
      if(root_class.empty() ||
         root_class == class_name ||
         classes_with_virtual_functions_.count(root_class) == 0 ||
         !symbol_linkage::has_external_vtable_symbol_candidate(root_type) ||
         vtable_bindings_.count(root_class) != 0 ||
         class_virtual_base_layouts_.count(root_class) != 0) {
        return;
      }
      class_virtual_base_layouts_[root_class] = virtual_base_layout;
      return;
    }

    bool describes_base_of_class = false;
    for(size_t i = 0; i < virtual_base_layout.size(); ++i) {
      if(virtual_base_layout[i].first != class_name) {
        describes_base_of_class = true;
        break;
      }
    }
    if(!describes_base_of_class) {
      return;
    }

    if(class_virtual_base_layouts_.count(class_name) == 0) {
      class_virtual_base_layouts_[class_name] = virtual_base_layout;
    }
  }

  void collect_expression_virtual_base_layouts(const CallSemNode & node)
  {
    maybe_record_expression_virtual_base_layout(node);
    vector<const CallSemNode *> children;
    append_callsem_recursive_input_children(node, children);
    for(size_t i = 0; i < children.size(); ++i) {
      collect_expression_virtual_base_layouts(*children[i]);
    }
  }

  void collect_virtual_runtime_classes(const CallSemNode & node)
  {
    if((node.kind == CallSemKind::function_definition ||
        node.kind == CallSemKind::function_declaration ||
        node.kind == CallSemKind::callee) &&
       (node.is_virtual_member_function || node.is_virtual_dispatch)) {
      const string object_class = function_object_class_qualified_name(node);
      if(!object_class.empty()) {
        classes_with_virtual_functions_.insert(object_class);
      }
    }
    if(node.kind == CallSemKind::vptr_action && !node.text.empty()) {
      string class_name = node.text;
      const string view_marker = "::__view__";
      const size_t view_pos = class_name.find(view_marker);
      if(view_pos != string::npos) {
        class_name = class_name.substr(0, view_pos);
      }
      if(!class_name.empty()) {
        classes_with_virtual_functions_.insert(class_name);
      }
    }
    vector<const CallSemNode *> children;
    append_callsem_recursive_input_children(node, children);
    for(size_t i = 0; i < children.size(); ++i) {
      collect_virtual_runtime_classes(*children[i]);
    }
  }

  void collect_symbols(const CallSemNode & node)
  {
    if(node.kind == CallSemKind::translation_unit ||
       node.kind == CallSemKind::namespace_definition) {
      for(size_t i = 0; i < node.children.size(); ++i) {
        collect_symbols(node.children[i]);
      }
      return;
    }

    if(node.kind == CallSemKind::function_definition ||
       node.kind == CallSemKind::function_declaration) {
      function_nodes_.push_back(&node);
      collect_static_storage_symbols(node);
      const string key = function_key(node.text, node.semantic_type);
      const string symbol = node_internal_symbol(node);
      const CallSemVirtualBaseLayout & virtual_base_layout =
          callsem_virtual_base_layout(node);
      if(!virtual_base_layout.empty()) {
        function_virtual_base_layouts_[symbol] = virtual_base_layout;
        const string object_class = function_object_class_qualified_name(node);
        if(!object_class.empty()) {
          map<string, vector<pair<string, unsigned long long> > >::const_iterator existing =
              class_virtual_base_layouts_.find(object_class);
          if(existing == class_virtual_base_layouts_.end()) {
            class_virtual_base_layouts_[object_class] = virtual_base_layout;
          } else if(existing->second != virtual_base_layout) {
            throw logic_error("conflicting class virtual base layout for " + object_class);
          }
        }
      }
      for(size_t i = 0; i < node.children.size(); ++i) {
        const CallSemNode & child = node.children[i];
        const CallSemVirtualBaseLayout & child_virtual_base_layout =
            callsem_virtual_base_layout(child);
        if(child.kind != CallSemKind::parameter || child_virtual_base_layout.empty()) {
          continue;
        }
        TypePtr class_type =
            strip_top_level_cv(remove_reference_type(child.semantic_type));
        if(class_type && class_type->kind == Type::TK_POINTER) {
          class_type = strip_top_level_cv(class_type->inner);
        }
        const string parameter_class = class_qualified_name(class_type);
        if(parameter_class.empty()) {
          continue;
        }
        map<string, vector<pair<string, unsigned long long> > >::const_iterator existing =
            class_virtual_base_layouts_.find(parameter_class);
        if(existing == class_virtual_base_layouts_.end()) {
          class_virtual_base_layouts_[parameter_class] = child_virtual_base_layout;
        } else if(existing->second != child_virtual_base_layout) {
          throw logic_error("conflicting class virtual base layout for " + parameter_class);
        }
      }
      const bool prefer_symbol =
          function_symbols_.find(key) == function_symbols_.end() ||
          node.kind == CallSemKind::function_definition;
      if(prefer_symbol) {
        function_symbols_[key] = symbol;
        function_symbol_nodes_[symbol] = &node;
        invalidate_function_symbol_lookup_index();
      }
      if(node.is_c_linkage) {
        c_linkage_function_symbols_.insert(symbol);
      }
      bool updated_entry = false;
      const string node_type_key = stable_function_type_key(node.semantic_type);
      for(size_t i = 0; i < function_symbol_entries_.size(); ++i) {
        if(function_symbol_entries_[i].name != node.text) {
          continue;
        }
        if(!type_equals(function_symbol_entries_[i].type, node.semantic_type) &&
           stable_function_type_key(function_symbol_entries_[i].type) != node_type_key) {
          continue;
        }
        if(prefer_symbol) {
          function_symbol_entries_[i].symbol = symbol;
          invalidate_function_symbol_lookup_index();
        }
        if(node.kind == CallSemKind::function_definition) {
          function_symbol_entries_[i].has_definition = true;
        }
        updated_entry = true;
        break;
      }
      if(!updated_entry) {
        FunctionSymbolEntry entry;
        entry.name = node.text;
        entry.type = node.semantic_type;
        entry.symbol = symbol;
        entry.has_definition = node.kind == CallSemKind::function_definition;
        function_symbol_entries_.push_back(entry);
        invalidate_function_symbol_lookup_index();
      }
      if(symbol_linkage::has_object_symbol(callsem_symbol(node)) &&
         !is_output_on_use_function_definition(node)) {
        const string internal_symbol = node_internal_symbol(node);
        set_exported_symbol(internal_symbol, callsem_symbol(node), "semantic-function", node.text);
        maybe_add_special_member_base_alias(node, internal_symbol);
      }
      if(node.kind == CallSemKind::function_definition &&
         subtree_contains_kind(node, CallSemKind::throw_statement)) {
        throwing_function_symbols_.insert(symbol);
      }
      return;
    }

    if(node.kind == CallSemKind::vtable_definition) {
      collect_vtable(node);
      return;
    }

    if(node.kind == CallSemKind::vtt_definition) {
      collect_vtt(node);
      return;
    }

    if(node.kind == CallSemKind::rtti_definition) {
      return;
    }

    if(node.kind == CallSemKind::variable) {
      note_global_binding(node);
      return;
    }
  }

  void seed_parameter_virtual_base_layouts()
  {
    for(size_t i = 0; i < function_nodes_.size(); ++i) {
      const CallSemNode & node = *function_nodes_[i];
      if(!callsem_virtual_base_layout(node).empty()) {
        continue;
      }

      ParameterVirtualBaseLayout inferred_layout;
      if(!infer_parameter_virtual_base_layout(node, inferred_layout) &&
         !infer_reference_parameter_type_virtual_base_layout(
             node,
             class_virtual_base_layouts_,
             inferred_layout) &&
         !infer_reference_storage_parameter_virtual_base_layout(
             node,
             class_virtual_base_layouts_,
             inferred_layout)) {
        continue;
      }

      const string symbol = node_internal_symbol(node);
      map<string, ParameterVirtualBaseLayout>::iterator existing =
          function_parameter_virtual_base_layouts_.find(symbol);
      if(existing == function_parameter_virtual_base_layouts_.end()) {
        function_parameter_virtual_base_layouts_[symbol] = inferred_layout;
        continue;
      }
      bool changed = false;
      if(!merge_parameter_virtual_base_layout(existing->second,
                                              inferred_layout,
                                              changed)) {
        throw logic_error("conflicting parameter virtual base layout for " + symbol);
      }
    }

    for(size_t i = 0; i < function_symbol_entries_.size(); ++i) {
      const FunctionSymbolEntry & entry = function_symbol_entries_[i];
      if(entry.symbol.empty() ||
         function_parameter_virtual_base_layouts_.count(entry.symbol) != 0) {
        continue;
      }

      ParameterVirtualBaseLayout inferred_layout;
      if(infer_function_type_reference_parameter_virtual_base_layout(
             entry.type,
             class_virtual_base_layouts_,
             inferred_layout)) {
        function_parameter_virtual_base_layouts_[entry.symbol] = inferred_layout;
      }
    }
  }

  static size_t invalid_parameter_index()
  {
    return static_cast<size_t>(-1);
  }

  static bool node_is_parameter_root(const CallSemNode & node)
  {
    return node.kind == CallSemKind::variable ||
           node.kind == CallSemKind::id_expression ||
           node.kind == CallSemKind::parameter;
  }

  map<string, size_t> function_parameter_indices(const CallSemNode & function_node)
  {
    map<string, size_t> parameter_indices;
    size_t parameter_index = 0;
    for(size_t i = 0; i < function_node.children.size(); ++i) {
      if(function_node.children[i].kind == CallSemKind::parameter &&
         !function_node.children[i].text.empty()) {
        parameter_indices[function_node.children[i].text] = parameter_index;
      }
      if(function_node.children[i].kind == CallSemKind::parameter) {
        ++parameter_index;
      }
    }
    return parameter_indices;
  }

  string call_callee_symbol(const CallSemNode & call) const
  {
    if(call.kind != CallSemKind::call_expression ||
       call.children.empty()) {
      return string();
    }
    const CallSemNode & callee = call.children[0];
    string callee_symbol = callsem_symbol(callee).internal_symbol;
    if(callee_symbol.empty() &&
       !callee.text.empty() &&
       callee.semantic_type) {
      const string lookup_name =
          callsem_resolved_name(callee).empty() ? callee.text.str() :
              callsem_resolved_name(callee);
      callee_symbol =
          try_lookup_function_symbol_with_index(function_symbols_,
                                                function_symbol_entries_,
                                                function_symbol_lookup_index(),
                                                lookup_name,
                                                callee.semantic_type);
    }
    return callee_symbol;
  }

  void maybe_record_parameter_virtual_base_forwarding_candidate(
      const string & caller_symbol,
      const map<string, size_t> & parameter_indices,
      const CallSemNode & call)
  {
    const string callee_symbol = call_callee_symbol(call);
    if(callee_symbol.empty() || call.children.size() <= 1) {
      return;
    }

    ParameterVirtualBaseForwardingCandidate candidate;
    candidate.caller_symbol = caller_symbol;
    const size_t explicit_argument_count = call.children.size() - 1;
    size_t callee_parameter_count = explicit_argument_count;
    bool call_omits_first_callee_parameter = false;
    TypePtr function_type;
    if(resolve_callable_function_type(call.children[0].semantic_type, function_type) &&
       function_type &&
       function_type->kind == Type::TK_FUNCTION) {
      callee_parameter_count =
          max(callee_parameter_count, function_type->params.size());
      TypePtr result_type = strip_top_level_cv(remove_reference_type(call.semantic_type));
      const bool constructor_prvalue_call =
          result_type &&
          is_complete_class_value_type(result_type) &&
          function_type->inner &&
          is_void_type(function_type->inner);
      call_omits_first_callee_parameter =
          constructor_prvalue_call &&
          function_type->params.size() == explicit_argument_count + 1;
    }
    candidate.argument_parameter_indices.assign(callee_parameter_count,
                                                invalid_parameter_index());
    bool has_parameter_argument = false;
    for(size_t i = 1; i < call.children.size(); ++i) {
      const size_t callee_parameter_index =
          call_omits_first_callee_parameter ? i : (i - 1);
      if(callee_parameter_index >= candidate.argument_parameter_indices.size()) {
        continue;
      }
      const CallSemNode * root = peel_base_subobject_root_shared(call.children[i]);
      map<string, size_t>::const_iterator parameter_it =
          root ? parameter_indices.find(root->text) : parameter_indices.end();
      if(root &&
         node_is_parameter_root(*root) &&
         parameter_it != parameter_indices.end()) {
        candidate.argument_parameter_indices[callee_parameter_index] =
            parameter_it->second;
        has_parameter_argument = true;
      }
    }
    if(has_parameter_argument) {
      parameter_virtual_base_forwarding_candidates_[callee_symbol].push_back(candidate);
    }
  }

  void collect_parameter_virtual_base_forwarding_candidates(
      const string & caller_symbol,
      const map<string, size_t> & parameter_indices,
      const CallSemNode & node)
  {
    if(node.kind == CallSemKind::call_expression) {
      maybe_record_parameter_virtual_base_forwarding_candidate(caller_symbol,
                                                              parameter_indices,
                                                              node);
    }
    for(size_t i = 0; i < node.children.size(); ++i) {
      collect_parameter_virtual_base_forwarding_candidates(caller_symbol,
                                                          parameter_indices,
                                                          node.children[i]);
    }
  }

  void build_parameter_virtual_base_forwarding_candidates()
  {
    parameter_virtual_base_forwarding_candidates_.clear();
    for(size_t i = 0; i < function_nodes_.size(); ++i) {
      const CallSemNode & node = *function_nodes_[i];
      if(!callsem_virtual_base_layout(node).empty()) {
        continue;
      }
      const string caller_symbol = node_internal_symbol(node);
      if(caller_symbol.empty()) {
        continue;
      }
      const map<string, size_t> parameter_indices =
          function_parameter_indices(node);
      if(parameter_indices.empty()) {
        continue;
      }
      collect_parameter_virtual_base_forwarding_candidates(caller_symbol,
                                                          parameter_indices,
                                                          node);
    }
  }

  void record_parameter_virtual_base_layout(const string & symbol,
                                            const ParameterVirtualBaseLayout & inferred_layout,
                                            deque<string> * pending)
  {
    map<string, ParameterVirtualBaseLayout>::iterator existing =
        function_parameter_virtual_base_layouts_.find(symbol);
    if(existing == function_parameter_virtual_base_layouts_.end()) {
      function_parameter_virtual_base_layouts_[symbol] = inferred_layout;
      if(pending) {
        pending->push_back(symbol);
      }
      return;
    }
    bool changed = false;
    if(!merge_parameter_virtual_base_layout(existing->second,
                                            inferred_layout,
                                            changed)) {
      throw logic_error("conflicting parameter virtual base layout for " + symbol);
    }
    if(changed && pending) {
      pending->push_back(symbol);
    }
  }

  void propagate_parameter_virtual_base_layouts()
  {
    build_parameter_virtual_base_forwarding_candidates();
    deque<string> pending;
    for(map<string, ParameterVirtualBaseLayout>::const_iterator it =
            function_parameter_virtual_base_layouts_.begin();
        it != function_parameter_virtual_base_layouts_.end();
        ++it) {
      pending.push_back(it->first);
    }

    while(!pending.empty()) {
      const string callee_symbol = pending.front();
      pending.pop_front();
      map<string, ParameterVirtualBaseLayout>::const_iterator callee_layout =
          function_parameter_virtual_base_layouts_.find(callee_symbol);
      if(callee_layout == function_parameter_virtual_base_layouts_.end()) {
        continue;
      }
      map<string, vector<ParameterVirtualBaseForwardingCandidate> >::const_iterator candidates =
          parameter_virtual_base_forwarding_candidates_.find(callee_symbol);
      if(candidates == parameter_virtual_base_forwarding_candidates_.end()) {
        continue;
      }

      for(size_t i = 0; i < candidates->second.size(); ++i) {
        const ParameterVirtualBaseForwardingCandidate & candidate =
            candidates->second[i];
        if(callee_layout->second.parameter_index >=
           candidate.argument_parameter_indices.size()) {
          continue;
        }
        const size_t caller_parameter_index =
            candidate.argument_parameter_indices[callee_layout->second.parameter_index];
        if(caller_parameter_index == invalid_parameter_index()) {
          continue;
        }

        ParameterVirtualBaseLayout inferred_layout;
        inferred_layout.parameter_index = caller_parameter_index;
        inferred_layout.layout =
            normalize_parameter_virtual_base_layout(callee_layout->second.layout);
        record_parameter_virtual_base_layout(candidate.caller_symbol,
                                            inferred_layout,
                                            &pending);
      }
    }
  }

  void collect_runtime_global_references(const CallSemNode & node, bool top_level)
  {
    if(node.kind == CallSemKind::variable && !top_level) {
      const string symbol = node_internal_symbol(node);
      if(global_bindings_.count(symbol) != 0) {
        referenced_global_symbols_.insert(symbol);
      }
    }

    if(node.kind == CallSemKind::id_expression &&
       !top_level &&
       !callsem_symbol(node).internal_symbol.empty() &&
       node.semantic_type &&
       !is_function_type(strip_top_level_cv(node.semantic_type))) {
      symbol_linkage::SymbolIdentity symbol = callsem_symbol(node);
      if(node.is_thread_local && symbol.thread_local_wrapper_object_symbol.empty()) {
        const shared_ptr<QualifiedName> & qualified =
            callsem_qualified_name_syntax(node);
        if(qualified) {
          symbol.thread_local_wrapper_object_symbol =
              symbol_linkage::thread_local_wrapper_object_symbol_for_qualified_name(
                  *qualified);
        }
      }
      referenced_global_symbols_.insert(symbol.internal_symbol);
      map<string, GlobalBinding>::iterator existing =
          global_bindings_.find(symbol.internal_symbol);
      if(existing == global_bindings_.end()) {
        GlobalBinding binding;
        binding.semantic_type = node.semantic_type;
        TypePtr base = strip_top_level_cv(node.semantic_type);
        if(base && (base->kind == Type::TK_ARRAY || base->kind == Type::TK_NAMED)) {
          binding.lowir_type = "i64";
        } else {
          binding.lowir_type = lowir_memory_type_for(node.semantic_type);
        }
        binding.storage = symbol.internal_symbol;
        binding.thread_local_storage = node.is_thread_local;
        binding.symbol = symbol;
        binding.is_definition = false;
        global_bindings_[binding.storage] = binding;
      } else if(node.is_thread_local) {
        existing->second.thread_local_storage = true;
        if(existing->second.symbol.thread_local_wrapper_object_symbol.empty()) {
          existing->second.symbol.thread_local_wrapper_object_symbol =
              symbol.thread_local_wrapper_object_symbol;
        }
      }
      if(symbol_linkage::has_object_symbol(symbol)) {
        set_exported_symbol(symbol.internal_symbol,
                            symbol,
                            "id-expression-global",
                            node.text);
      }
    }

    const bool child_top_level =
        node.kind == CallSemKind::translation_unit ||
        node.kind == CallSemKind::namespace_definition;
    vector<const CallSemNode *> children;
    append_callsem_recursive_input_children(node, children);
    for(size_t i = 0; i < children.size(); ++i) {
      collect_runtime_global_references(*children[i], child_top_level);
    }
  }

  void collect_runtime_function_references(const CallSemNode & node,
                                           const string & current_function,
                                           bool top_level)
  {
    if(node.kind == CallSemKind::function_definition) {
      const string function_symbol = node_internal_symbol(node);
      if(is_output_on_use_function_definition(node)) {
        function_references_[function_symbol];
      } else {
        referenced_function_symbols_.insert(function_symbol);
      }
      vector<const CallSemNode *> children;
      append_callsem_recursive_input_children(node, children);
      for(size_t i = 0; i < children.size(); ++i) {
        collect_runtime_function_references(*children[i], function_symbol, false);
      }
      return;
    }
    if(node.kind == CallSemKind::function_declaration) {
      return;
    }

    if(node.kind == CallSemKind::constructor_action &&
       node.trivial_lifecycle &&
       node.children.size() == 1 &&
       node.children[0].kind == CallSemKind::call_expression) {
      const CallSemNode & call = node.children[0];
      for(size_t i = 1; i < call.children.size(); ++i) {
        collect_runtime_function_references(call.children[i], current_function, false);
      }
      return;
    }

    if(node.kind == CallSemKind::destructor_action && node.trivial_lifecycle) {
      return;
    }

    if(!top_level &&
       node.kind == CallSemKind::id_expression &&
       node.semantic_type &&
       is_function_type(node.semantic_type)) {
      const string function_symbol = lookup_runtime_reference_function_symbol(node);
      if(!function_symbol.empty()) {
        note_referenced_function_signature(function_symbol, node.semantic_type);
        if(symbol_linkage::has_object_symbol(callsem_symbol(node))) {
          note_runtime_function_symbol_identity(function_symbol,
                                                callsem_symbol(node),
                                                "function-id",
                                                node.text);
        }
        if(current_function.empty()) {
          referenced_function_symbols_.insert(function_symbol);
        } else {
          function_references_[current_function].insert(function_symbol);
        }
      }
    }

    if(!top_level && node.kind == CallSemKind::callee) {
      const string callee_symbol =
          node.semantic_type ? lookup_runtime_reference_function_symbol(node) : string();
      if(!callee_symbol.empty()) {
        note_selected_callee_symbol(node, callee_symbol);
        note_referenced_function_signature(callee_symbol, node.semantic_type);
        note_runtime_function_symbol_identity(callee_symbol,
                                              callsem_symbol(node),
                                              "callee",
                                              node.text);
        if(current_function.empty()) {
          referenced_function_symbols_.insert(callee_symbol);
        } else {
          function_references_[current_function].insert(callee_symbol);
        }
      }
    }

    const bool child_top_level =
        node.kind == CallSemKind::translation_unit ||
        node.kind == CallSemKind::namespace_definition;
    vector<const CallSemNode *> children;
    append_callsem_recursive_input_children(node, children);
    for(size_t i = 0; i < children.size(); ++i) {
      collect_runtime_function_references(*children[i], current_function, child_top_level);
    }
  }

  void note_selected_callee_symbol(const CallSemNode & node,
                                   const string & symbol)
  {
    if(symbol.empty() || node.text.empty() || !node.semantic_type) {
      return;
    }

    const string name =
        callsem_resolved_name(node).empty() ? node.text.str() :
            callsem_resolved_name(node);
    if(!is_constructor_function_name(name) &&
       !is_destructor_function_name(name)) {
      return;
    }
    const string key = function_key(name, node.semantic_type);
    if(function_symbols_.find(key) == function_symbols_.end()) {
      function_symbols_[key] = symbol;
      invalidate_function_symbol_lookup_index();
    }
    if(function_symbol_nodes_.find(symbol) == function_symbol_nodes_.end()) {
      function_symbol_nodes_[symbol] = &node;
    }

    const string node_type_key = stable_function_type_key(node.semantic_type);
    for(size_t i = 0; i < function_symbol_entries_.size(); ++i) {
      if(function_symbol_entries_[i].name != name) {
        continue;
      }
      if(!type_equals(function_symbol_entries_[i].type, node.semantic_type) &&
         stable_function_type_key(function_symbol_entries_[i].type) != node_type_key) {
        continue;
      }
      return;
    }

    FunctionSymbolEntry entry;
    entry.name = name;
    entry.type = node.semantic_type;
    entry.symbol = symbol;
    entry.has_definition = false;
    function_symbol_entries_.push_back(entry);
    invalidate_function_symbol_lookup_index();
  }

  void collect_reachable_function_symbols()
  {
    vector<string> worklist(referenced_function_symbols_.begin(),
                            referenced_function_symbols_.end());
    for(map<string, set<string> >::const_iterator it = function_references_.begin();
        it != function_references_.end();
        ++it) {
      if(exported_symbols_.count(it->first) != 0 &&
         referenced_function_symbols_.insert(it->first).second) {
        worklist.push_back(it->first);
      }
    }
    for(size_t i = 0; i < worklist.size(); ++i) {
      map<string, set<string> >::const_iterator found =
          function_references_.find(worklist[i]);
      if(found == function_references_.end()) {
        continue;
      }
      for(set<string>::const_iterator it = found->second.begin();
          it != found->second.end();
          ++it) {
        if(referenced_function_symbols_.insert(*it).second) {
          worklist.push_back(*it);
        }
      }
    }
  }

  bool is_output_on_use_function_definition(const CallSemNode & node) const
  {
    return node.kind == CallSemKind::function_definition &&
           !node.is_explicit_instantiation_definition &&
           (symbol_linkage::has_weak_linkage(callsem_symbol(node)) ||
            callsem_symbol(node).prefer_local_object_binding);
  }

  bool should_skip_unreferenced_output_on_use_function(const CallSemNode & node) const
  {
    if(!is_output_on_use_function_definition(node)) {
      return false;
    }
    const string symbol = node_internal_symbol(node);
    return referenced_function_symbols_.count(symbol) == 0 &&
           exported_symbols_.count(symbol) == 0;
  }

  void emit_function_definition(const CallSemNode & node)
  {
    collect_static_storage_globals(node);
    collect_dynamic_typeid_fallback_support(node);
    if(symbol_linkage::has_object_symbol(callsem_symbol(node))) {
      const string internal_symbol = node_internal_symbol(node);
      set_exported_symbol(internal_symbol,
                          callsem_symbol(node),
                          "semantic-function",
                          node.text);
      maybe_add_special_member_base_alias(node, internal_symbol);
    }
    LowIRFunction function =
        LowIRFunctionBuilder(node, global_bindings_, vtable_bindings_,
                             function_symbols_, function_symbol_entries_,
                             function_symbol_lookup_index(),
                             function_symbol_nodes_,
                             c_linkage_function_symbols_,
                             function_virtual_base_layouts_,
                             class_virtual_base_layouts_,
                             function_parameter_virtual_base_layouts_,
                             classes_with_virtual_functions_,
                             throwing_function_symbols_,
                             rtti_definition_symbols_,
                             string_literal_symbols_,
                             exception_storage_types_,
                             virtual_member_pointer_thunks_,
                             external_function_symbols_,
                             external_object_symbols_,
                             runtime_bridge_support_symbols_,
                             referenced_function_symbols_,
                             referenced_function_signature_types_,
                             function_references_,
                             emit_runtime_support_,
                             enable_debug_value_names_)
            .build();
    functions_.push_back(function);
  }

  bool emit_referenced_output_on_use_function_definition_pass()
  {
    collect_reachable_function_symbols();
    bool emitted = false;
    for(size_t i = 0; i < function_nodes_.size(); ++i) {
      const CallSemNode & node = *function_nodes_[i];
      if(node.kind != CallSemKind::function_definition ||
         !is_output_on_use_function_definition(node)) {
        continue;
      }
      const string symbol = node_internal_symbol(node);
      if((referenced_function_symbols_.count(symbol) == 0 &&
          exported_symbols_.count(symbol) == 0) ||
         generated_function_symbol_exists(symbol)) {
        continue;
      }
      emit_function_definition(node);
      emitted = true;
    }
    return emitted;
  }

  void emit_referenced_output_on_use_function_definitions()
  {
    while(emit_referenced_output_on_use_function_definition_pass()) {
    }
  }

  bool is_unreferenced_unsupported_constexpr_global(const CallSemNode & node) const
  {
    const string symbol = node_internal_symbol(node);
    if(referenced_global_symbols_.count(symbol) != 0) {
      return false;
    }

    TypePtr base = strip_top_level_cv(node.semantic_type);
    if(!base) {
      return false;
    }
    if(base->kind == Type::TK_ARRAY) {
      base = strip_top_level_cv(base->inner);
    }
    if(!base || base->kind != Type::TK_FUNDAMENTAL) {
      return false;
    }
    return base->fundamental == FT_INT128 || base->fundamental == FT_UINT128;
  }

  bool has_global_name(const string & name) const
  {
    for(size_t i = 0; i < globals_.size(); ++i) {
      if(globals_[i].name == name) {
        return true;
      }
    }
    return false;
  }

  LowIRGlobal * find_global_name(const string & name)
  {
    for(size_t i = 0; i < globals_.size(); ++i) {
      if(globals_[i].name == name) {
        return &globals_[i];
      }
    }
    return nullptr;
  }

  const LowIRGlobal * find_global_name(const string & name) const
  {
    for(size_t i = 0; i < globals_.size(); ++i) {
      if(globals_[i].name == name) {
        return &globals_[i];
      }
    }
    return nullptr;
  }

  string external_runtime_symbol(const string & helper_symbol)
  {
    const string direct_symbol = "@" + helper_symbol;
    if(function_symbol_is_c_linkage(direct_symbol) &&
       known_function_symbol_exists(direct_symbol)) {
      return direct_symbol;
    }

    const string internal_symbol =
        symbol_linkage::internal_symbol_from_name("__external_runtime::" + helper_symbol);
    external_function_symbols_[internal_symbol] = helper_symbol;
    return internal_symbol;
  }

  void note_external_runtime_function(const string & helper_symbol)
  {
    external_function_symbols_[
        symbol_linkage::internal_symbol_from_name("__external_runtime::" + helper_symbol)] =
        helper_symbol;
  }

  void emit_private_eh_runtime_globals()
  {
    struct ReservedGlobalSpec
    {
      const char * internal_symbol;
      const char * object_symbol;
    };
    const ReservedGlobalSpec globals[] = {
      {eh_runtime::kEhTopSymbol, eh_runtime::kEhTopObjectSymbol},
      {eh_runtime::kEhValueSymbol, eh_runtime::kEhValueObjectSymbol},
      {eh_runtime::kEhTypeSymbol, eh_runtime::kEhTypeObjectSymbol},
    };
    for(size_t i = 0; i < sizeof(globals) / sizeof(globals[0]); ++i) {
      if(!has_global_name(globals[i].internal_symbol)) {
        globals_.push_back(make_scalar_global(globals[i].internal_symbol,
                                              "ptr",
                                              "zero",
                                              false));
      }
      set_exported_symbol(globals[i].internal_symbol,
                          symbol_linkage::make_object_symbol_identity(globals[i].internal_symbol,
                                                                      globals[i].object_symbol,
                                                                      symbol_linkage::SL_WEAK),
                          "private-eh-runtime");
    }
  }

  void emit_private_eh_runtime_unhandled()
  {
    if(!generated_function_symbol_exists(eh_runtime::kEhUnhandledSymbol)) {
      LowIRFunction function;
      function.name = eh_runtime::kEhUnhandledSymbol;
      function.boundary_metadata.arity = lowir_internal::CAM_FIXED;
      apply_known_function_boundary_metadata(function.boundary_metadata, function.name);
      function.return_type = "void";
      function.params.push_back(make_lowir_parameter_text("%exception", "i64"));

      LowIRBlock entry;
      entry.label = "^entry";
      entry.instructions.push_back(
          string("call void ") + external_runtime_symbol("abort") + "()");
      entry.instructions.push_back("return void");
      entry.terminated = true;
      function.blocks.push_back(entry);
      functions_.push_back(function);
    }
    set_exported_symbol(eh_runtime::kEhUnhandledSymbol,
                        symbol_linkage::make_object_symbol_identity(
                            eh_runtime::kEhUnhandledSymbol,
                            eh_runtime::kEhUnhandledObjectSymbol,
                            symbol_linkage::SL_WEAK),
                        "private-eh-runtime");
  }

  void emit_private_eh_runtime_support()
  {
    if(!emit_runtime_support_ || !uses_private_eh_runtime_) {
      return;
    }
    emit_private_eh_runtime_globals();
    emit_private_eh_runtime_unhandled();
  }

  void collect_exception_support(const CallSemNode & node)
  {
    if(node.kind == CallSemKind::function_definition &&
       should_skip_unreferenced_output_on_use_function(node)) {
      return;
    }

    if(!emit_runtime_support_ &&
       (node.kind == CallSemKind::throw_statement ||
        node.kind == CallSemKind::try_statement ||
        (node.kind == CallSemKind::dynamic_cast_expression &&
         is_reference_type(node.semantic_type)) ||
        (node.kind == CallSemKind::typeid_expression && !node.children.empty()))) {
      uses_private_eh_runtime_ = true;
    }

    if(node.kind == CallSemKind::function_definition && emit_runtime_support_) {
      const bool has_try_statement =
          subtree_contains_kind(node, CallSemKind::try_statement);
      const bool has_throw_statement =
          subtree_contains_kind(node, CallSemKind::throw_statement);
      const bool has_call_expression =
          subtree_contains_kind(node, CallSemKind::call_expression);
      const bool has_unwind_cleanup =
          subtree_contains_nontrivial_destructor_action(node);

      if(has_try_statement) {
        note_external_runtime_function("__gxx_personality_v0");
        note_external_runtime_function("_Unwind_Resume");
      }
      if(node.has_dynamic_exception_spec) {
        note_external_runtime_function("__gxx_personality_v0");
      }
      if(has_throw_statement && has_unwind_cleanup) {
        note_external_runtime_function("__gxx_personality_v0");
        note_external_runtime_function("_Unwind_Resume");
      }
      if(has_throw_statement) {
        note_external_runtime_function("__gxx_personality_v0");
      }
      if(has_call_expression && has_unwind_cleanup) {
        note_external_runtime_function("__gxx_personality_v0");
        note_external_runtime_function("_Unwind_Resume");
      }
    }

    if(node.kind == CallSemKind::function_definition &&
       node.has_dynamic_exception_spec) {
      for(size_t i = 0; i < node.children.size(); ++i) {
        if(node.children[i].kind == CallSemKind::rtti_candidate &&
           node.children[i].semantic_type) {
          exception_rtti_symbols_[rtti_symbol_for_type(node.children[i].semantic_type)] =
              node.children[i].semantic_type;
        }
      }
    }

    if(node.kind == CallSemKind::throw_statement && !node.children.empty()) {
      TypePtr type = exception_object_type(node.children[0].semantic_type);
      if(type) {
        exception_storage_types_[describe_type(type)] = type;
        exception_rtti_symbols_[rtti_symbol_for_type(type)] = type;
      }
    }

    if(node.kind == CallSemKind::catch_handler) {
      TypePtr catch_type = exception_object_type(node.semantic_type);
      if(catch_type) {
        exception_rtti_symbols_[rtti_symbol_for_type(catch_type)] = catch_type;
      }
      for(size_t i = 0; i < node.children.size(); ++i) {
        if(node.children[i].kind == CallSemKind::rtti_candidate &&
           node.children[i].semantic_type) {
          exception_rtti_symbols_[rtti_symbol_for_type(node.children[i].semantic_type)] =
              node.children[i].semantic_type;
        }
      }
    }

    vector<const CallSemNode *> children;
    append_callsem_recursive_input_children(node, children);
    for(size_t i = 0; i < children.size(); ++i) {
      collect_exception_support(*children[i]);
    }
  }

  void collect_exception_support_globals()
  {
    for(map<string, TypePtr>::const_iterator it = exception_rtti_symbols_.begin();
        it != exception_rtti_symbols_.end();
        ++it) {
      if(emit_runtime_support_ && it->second) {
        const string host_symbol = symbol_linkage::typeinfo_symbol_for_type(it->second);
        if(!host_symbol.empty()) {
          host_typeinfo_reference_symbol(it->second);
          continue;
        }
      }
      if(!has_global_name(it->first)) {
        globals_.push_back(make_scalar_global(it->first, "i64", "zero", false));
      }
      export_rtti_symbol(it->first, it->second, "exception-rtti");
    }

    for(map<string, TypePtr>::const_iterator it = exception_storage_types_.begin();
        it != exception_storage_types_.end();
        ++it) {
      const string storage = exception_storage_symbol(it->second);
      if(has_global_name(storage)) {
        set_exported_symbol(storage,
                            symbol_linkage::make_internal_symbol_identity(storage,
                                                                          symbol_linkage::SL_WEAK),
                            "exception-storage");
        continue;
      }
      LowIRGlobal global = make_data_global(storage);
      global.data_items.push_back(string("zero ") + to_string(backend_storage_size(it->second)));
      globals_.push_back(global);
      set_exported_symbol(storage,
                          symbol_linkage::make_internal_symbol_identity(storage,
                                                                        symbol_linkage::SL_WEAK),
                          "exception-storage");
    }
    emit_private_eh_runtime_support();
  }

  void register_external_symbol_aliases()
  {
    for(map<string, string>::const_iterator it = external_function_symbols_.begin();
        it != external_function_symbols_.end();
        ++it) {
      if(exported_symbols_.count(it->first) != 0) {
        continue;
      }
      symbol_linkage::SymbolIdentity identity =
          symbol_linkage::make_c_function_symbol_identity(it->second);
      identity.internal_symbol = it->first;
      set_exported_symbol(it->first, identity, "external-function-alias");
    }
    for(map<string, string>::const_iterator it = external_object_symbols_.begin();
        it != external_object_symbols_.end();
        ++it) {
      if(exported_symbols_.count(it->first) != 0 ||
         generated_global_symbol_exists(it->first)) {
        continue;
      }
      symbol_linkage::SymbolIdentity identity =
          symbol_linkage::make_object_symbol_identity(it->first,
                                                      it->second,
                                                      symbol_linkage::SL_EXTERNAL);
      set_exported_symbol(it->first, identity, "external-global-alias");
    }
  }

  void collect_dynamic_typeid_fallback_support(const CallSemNode & node)
  {
    if(emit_runtime_support_) {
      return;
    }
    if(node.kind == CallSemKind::typeid_expression &&
       !node.children.empty() &&
       !node.text.empty() &&
       !has_global_name(node.text)) {
      LowIRGlobal global = make_scalar_global(node.text, "i64", "zero", false);
      globals_.push_back(global);
      set_exported_symbol(node.text,
                          symbol_linkage::make_internal_symbol_identity(node.text,
                                                                        symbol_linkage::SL_WEAK),
                          "typeid-fallback-rtti",
                          node.text);
    }
    vector<const CallSemNode *> children;
    append_callsem_recursive_input_children(node, children);
    for(size_t i = 0; i < children.size(); ++i) {
      collect_dynamic_typeid_fallback_support(*children[i]);
    }
  }

  void collect_scope(const CallSemNode & node)
  {
    if(node.kind == CallSemKind::translation_unit ||
       node.kind == CallSemKind::namespace_definition) {
      for(size_t i = 0; i < node.children.size(); ++i) {
        collect_scope(node.children[i]);
      }
      return;
    }

    if(node.kind == CallSemKind::function_definition) {
      // ODR-mergeable definitions are emitted from the reachable output
      // closure. Header presence alone should not define weak helper symbols.
      if(should_skip_unreferenced_output_on_use_function(node)) {
        return;
      }
      emit_function_definition(node);
      return;
    }

    if(node.kind == CallSemKind::rtti_definition) {
      emit_rtti_definition_global(node);
      return;
    }

    if(node.kind == CallSemKind::variable && !node.is_extern_declaration) {
      collect_global(node);
      return;
    }
  }

  void note_global_binding(const CallSemNode & node)
  {
    GlobalBinding binding;
    binding.semantic_type = node.semantic_type;
    TypePtr base = strip_top_level_cv(node.semantic_type);
    if(base && (base->kind == Type::TK_ARRAY || base->kind == Type::TK_NAMED)) {
      binding.lowir_type = "i64";
    } else {
      binding.lowir_type = lowir_memory_type_for(node.semantic_type);
    }
    binding.storage = node_internal_symbol(node);
    binding.thread_local_storage = node.is_thread_local;
    binding.thread_local_guard_symbol = callsem_local_static_guard_symbol(node);
    binding.symbol = callsem_symbol(node);
    if(node.is_thread_local && binding.symbol.thread_local_wrapper_object_symbol.empty()) {
      const shared_ptr<QualifiedName> & qualified =
          callsem_qualified_name_syntax(node);
      if(qualified) {
        binding.symbol.thread_local_wrapper_object_symbol =
            symbol_linkage::thread_local_wrapper_object_symbol_for_qualified_name(
                *qualified);
      }
    }
    binding.is_definition = !node.is_extern_declaration;
    global_bindings_[binding.storage] = binding;
    if(node.is_c_linkage) {
      c_linkage_global_symbols_.insert(binding.storage);
    }
    if(symbol_linkage::has_object_symbol(binding.symbol)) {
      set_exported_symbol(binding.storage, binding.symbol, "semantic-global", node.text);
      if(node.is_thread_local) {
        const string wrapper_internal =
            symbol_linkage::thread_local_wrapper_internal_symbol(binding.storage);
        string wrapper_object =
            binding.symbol.thread_local_wrapper_object_symbol;
        if(!wrapper_object.empty()) {
          set_exported_symbol(wrapper_internal,
                              symbol_linkage::make_object_symbol_identity(wrapper_internal,
                                                                          wrapper_object,
                                                                          binding.symbol.linkage),
                              "tls-wrapper",
                              node.text);
          thread_local_wrapper_targets_[wrapper_internal] = binding.storage;
        }
      }
    }
  }

  bool should_synthesize_referenced_internal_global_definition(
      const GlobalBinding & binding) const
  {
    return !binding.is_definition &&
           !binding.storage.empty() &&
           !binding.thread_local_storage &&
           referenced_global_symbols_.count(binding.storage) != 0 &&
           !generated_global_symbol_exists(binding.storage) &&
           symbol_linkage::has_object_symbol(binding.symbol) &&
           binding.symbol.linkage == symbol_linkage::SL_INTERNAL;
  }

  void synthesize_referenced_internal_global_definition(GlobalBinding & binding)
  {
    TypePtr base = strip_top_level_cv(binding.semantic_type);
    if(base && (base->kind == Type::TK_ARRAY || base->kind == Type::TK_NAMED)) {
      LowIRGlobal global = make_data_global(binding.storage);
      global.data_items.push_back(
          string("zero ") + to_string(backend_storage_size(binding.semantic_type)));
      globals_.push_back(global);
      binding.is_definition = true;
      return;
    }

    LowIRGlobal global = make_scalar_global(binding.storage,
                                            lowir_memory_type_for(binding.semantic_type),
                                            "zero",
                                            false);
    globals_.push_back(global);
    binding.is_definition = true;
  }

  void synthesize_referenced_internal_global_definitions()
  {
    for(map<string, GlobalBinding>::iterator it = global_bindings_.begin();
        it != global_bindings_.end();
        ++it) {
      if(should_synthesize_referenced_internal_global_definition(it->second)) {
        synthesize_referenced_internal_global_definition(it->second);
      }
    }
  }

  void collect_static_storage_symbols(const CallSemNode & node)
  {
    for(size_t i = 0; i < node.children.size(); ++i) {
      const CallSemNode & child = node.children[i];
      if(child.kind == CallSemKind::function_declaration) {
        continue;
      }
      if(child.kind == CallSemKind::variable && child.is_static_storage) {
        note_global_binding(child);
      }
      collect_static_storage_symbols(child);
    }
  }

  void collect_static_storage_globals(const CallSemNode & node)
  {
    for(size_t i = 0; i < node.children.size(); ++i) {
      const CallSemNode & child = node.children[i];
      if(child.kind == CallSemKind::function_declaration) {
        continue;
      }
      if(child.kind == CallSemKind::variable &&
         child.is_static_storage &&
         !child.is_extern_declaration) {
        collect_global(child);
      }
      collect_static_storage_globals(child);
    }
  }

  void collect_vtable(const CallSemNode & node)
  {
    if(node.text.empty()) {
      throw logic_error("vtable definition missing name");
    }
    if(vtable_bindings_.count(node.text) != 0) {
      throw logic_error("duplicate vtable definition " + node.text);
    }

    VTableBinding binding;
    binding.base_symbol = !callsem_symbol(node).internal_symbol.empty() ?
                              callsem_symbol(node).internal_symbol :
                              lowir_name(node.text + "::vtable");
    LowIRGlobal global = make_data_global(binding.base_symbol, true);
    const CallSemVirtualBaseLayout & virtual_base_layout =
        callsem_virtual_base_layout(node);
    const unsigned long long view_offset =
        node.has_uint_value ? callsem_uint_value(node) : 0ULL;
    const bool has_host_vtable_prefix =
        node.is_primary_vtable || !virtual_base_layout.empty();
    if(has_host_vtable_prefix) {
      binding.address_point_offset = host_vtable_address_point_offset(node);
      for(size_t i = 0; i < virtual_base_layout.size(); ++i) {
        const long long virtual_base_offset =
            static_cast<long long>(virtual_base_layout[i].second) -
            static_cast<long long>(view_offset);
        global.data_items.push_back(string("i64 ") + to_string(virtual_base_offset));
      }
      const long long offset_to_top = -static_cast<long long>(view_offset);
      global.data_items.push_back(string("i64 ") + to_string(offset_to_top));
      if(node.semantic_type) {
        string rtti_symbol = rtti_symbol_for_type(node.semantic_type);
        if(emit_runtime_support_) {
          rtti_symbol = host_typeinfo_reference_symbol(node.semantic_type);
        } else {
          if(!has_global_name(rtti_symbol)) {
            globals_.push_back(make_scalar_global(rtti_symbol, "i64", "zero", false));
          }
          export_rtti_symbol(rtti_symbol, node.semantic_type, "vtable-rtti", node.text);
        }
        global.data_items.push_back(string("ptr addr ") + rtti_symbol);
      } else {
        global.data_items.push_back("i64 0");
      }
    }
    for(size_t i = 0; i < node.children.size(); ++i) {
      if(node.children[i].kind == CallSemKind::rtti_candidate ||
         node.children[i].kind == CallSemKind::rtti_base) {
        continue;
      }
      if(node.children[i].kind != CallSemKind::vtable_entry) {
        throw logic_error("invalid vtable entry");
      }
      string entry_symbol = lookup_function_symbol(node.children[i]);
      symbol_linkage::SymbolIdentity exported_symbol =
          derive_vtable_entry_symbol_identity(node.children[i]);
      if(callsem_symbol(node.children[i]).internal_symbol.empty() &&
         symbol_linkage::has_exported_object_symbol(exported_symbol) &&
         exported_symbol.object_symbol != "__cxa_pure_virtual") {
        entry_symbol =
            lookup_function_symbol(node.children[i].text, node.children[i].semantic_type);
      }
      if(symbol_linkage::has_object_symbol(exported_symbol)) {
        exported_symbol.internal_symbol = entry_symbol;
        set_exported_symbol(entry_symbol, exported_symbol, "vtable-entry", node.text);
      }
      const long long this_adjust =
          node.children[i].has_int_value ? callsem_int_value(node.children[i]) : 0;
      const bool needs_entry_thunk =
          (!node.uses_extended_vtable_layout && this_adjust != 0) ||
          (node.children[i].has_result_adjust &&
           callsem_result_adjust(node.children[i]) != 0);
      const bool needs_host_export_thunk =
          !node.is_virtual_base_subobject &&
          symbol_linkage::has_exported_object_symbol(exported_symbol) &&
          exported_symbol.object_symbol != "__cxa_pure_virtual" &&
          this_adjust != 0;
      symbol_linkage::SymbolIdentity virtual_export_symbol;
      const bool needs_host_virtual_export_thunk =
          node.uses_extended_vtable_layout &&
          !callsem_resolved_name(node.children[i]).empty() &&
          !node.children[i].has_result_adjust;
      if(needs_host_virtual_export_thunk) {
        virtual_export_symbol =
            derive_vtable_entry_symbol_identity_for_name(node.children[i],
                                                         callsem_resolved_name(node.children[i]));
      }
      if(needs_entry_thunk) {
        const string thunk_symbol = lowir_name(entry_symbol + "::vtable_return_adjust");
        symbol_linkage::SymbolIdentity thunk_exported_symbol;
        if(needs_host_export_thunk) {
          const string thunk_target_name =
              callsem_resolved_name(node.children[i]).empty() ?
                  node.children[i].text.str() :
                  callsem_resolved_name(node.children[i]);
          const string thunk_object_symbol = callsem_qualified_name_syntax(node.children[i]) ?
              symbol_linkage::virtual_override_thunk_object_symbol_for_function(
                  *callsem_qualified_name_syntax(node.children[i]),
                  simple_lookup_name(thunk_target_name),
                  node.children[i].is_c_linkage,
                  node.children[i].semantic_type,
                  vtable_entry_function_symbol_options(node.children[i]),
                  this_adjust,
                  node.children[i].has_result_adjust &&
                      callsem_result_adjust(node.children[i]) != 0,
                  callsem_result_adjust(node.children[i])) :
              string();
          if(!thunk_object_symbol.empty()) {
            thunk_exported_symbol =
                symbol_linkage::make_object_symbol_identity(thunk_symbol,
                                                            thunk_object_symbol,
                                                            exported_symbol.linkage);
          }
        }
        VTableEntryThunkRequest & request = vtable_entry_thunks_[thunk_symbol];
        if(request.symbol.empty()) {
          request.symbol = thunk_symbol;
          request.target_symbol = entry_symbol;
          request.function_type = node.children[i].semantic_type;
          request.this_adjust = node.uses_extended_vtable_layout ? 0 : this_adjust;
          request.return_adjust = callsem_result_adjust(node.children[i]);
          if(!thunk_exported_symbol.object_symbol.empty()) {
            request.has_exported_symbol = true;
            request.exported_symbol = thunk_exported_symbol;
          }
        }
        if(request.has_exported_symbol) {
          set_exported_symbol(thunk_symbol,
                              request.exported_symbol,
                              "vtable-entry-thunk",
                              node.text);
        }
        entry_symbol = thunk_symbol;
      } else if(needs_host_export_thunk) {
        const string thunk_symbol = lowir_name(entry_symbol + "::host_export_thunk");
        const string thunk_target_name =
            callsem_resolved_name(node.children[i]).empty() ?
                node.children[i].text.str() :
                callsem_resolved_name(node.children[i]);
        const string thunk_object_symbol = callsem_qualified_name_syntax(node.children[i]) ?
            symbol_linkage::virtual_override_thunk_object_symbol_for_function(
                *callsem_qualified_name_syntax(node.children[i]),
                simple_lookup_name(thunk_target_name),
                node.children[i].is_c_linkage,
                node.children[i].semantic_type,
                vtable_entry_function_symbol_options(node.children[i]),
                this_adjust) :
            string();
        if(!thunk_object_symbol.empty()) {
          VTableEntryThunkRequest & request = vtable_entry_thunks_[thunk_symbol];
          if(request.symbol.empty()) {
            request.symbol = thunk_symbol;
            request.target_symbol = entry_symbol;
            request.function_type = node.children[i].semantic_type;
            request.this_adjust = this_adjust;
            request.return_adjust = 0;
            request.has_exported_symbol = true;
            request.exported_symbol =
                symbol_linkage::make_object_symbol_identity(thunk_symbol,
                                                            thunk_object_symbol,
                                                            exported_symbol.linkage);
          }
          set_exported_symbol(thunk_symbol,
                              request.exported_symbol,
                              "vtable-entry-host-export-thunk",
                              node.text);
          referenced_function_symbols_.insert(thunk_symbol);
        }
      }
      if(needs_host_virtual_export_thunk &&
         symbol_linkage::has_exported_object_symbol(virtual_export_symbol) &&
         virtual_export_symbol.object_symbol != "__cxa_pure_virtual") {
        const string thunk_symbol = lowir_name(entry_symbol + "::host_virtual_export_thunk");
        const string thunk_target_name = callsem_resolved_name(node.children[i]);
        const string thunk_object_symbol = callsem_qualified_name_syntax(node.children[i]) ?
            symbol_linkage::virtual_base_override_thunk_object_symbol_for_function(
                *callsem_qualified_name_syntax(node.children[i]),
                simple_lookup_name(thunk_target_name),
                node.children[i].is_c_linkage,
                node.children[i].semantic_type,
                vtable_entry_function_symbol_options(node.children[i]),
                -24) :
            string();
        if(!thunk_object_symbol.empty()) {
          VTableEntryThunkRequest & request = vtable_entry_thunks_[thunk_symbol];
          if(request.symbol.empty()) {
            request.symbol = thunk_symbol;
            request.target_symbol = entry_symbol;
            request.function_type = node.children[i].semantic_type;
            request.this_adjust = 0;
            request.return_adjust = 0;
            request.virtual_adjust_offset = -24;
            request.uses_vcall_offset_adjust = true;
            request.has_exported_symbol = true;
            request.exported_symbol =
                symbol_linkage::make_object_symbol_identity(thunk_symbol,
                                                            thunk_object_symbol,
                                                            virtual_export_symbol.linkage);
          }
          set_exported_symbol(thunk_symbol,
                              request.exported_symbol,
                              "vtable-entry-host-virtual-export-thunk",
                              node.text);
          referenced_function_symbols_.insert(thunk_symbol);
        }
      }
      referenced_function_symbols_.insert(entry_symbol);
      global.data_items.push_back(
          string("ptr addr ") + entry_symbol);
      if(node.uses_extended_vtable_layout) {
        global.data_items.push_back(
            string("i64 ") +
            to_string(node.children[i].has_int_value ?
                          callsem_int_value(node.children[i]) :
                          0));
      }
    }
    globals_.push_back(global);
    if(symbol_linkage::has_object_symbol(callsem_symbol(node))) {
      set_exported_symbol(binding.base_symbol,
                          callsem_symbol(node),
                          "vtable-base",
                          node.text);
    } else {
      const bool should_export_external_vtable =
          node.is_primary_vtable &&
          has_external_vtable_symbol_candidate_for_type(node.semantic_type);
      if(should_export_external_vtable) {
        const string external_vtable_symbol =
            symbol_linkage::vtable_object_symbol_for_type(node.semantic_type);
        if(external_vtable_symbol.empty()) {
          throw logic_error("failed to mangle vtable symbol " + node.text);
        }
        set_exported_symbol(binding.base_symbol,
                            symbol_linkage::make_object_symbol_identity(binding.base_symbol,
                                                                        external_vtable_symbol,
                                                                        symbol_linkage::SL_WEAK),
                            "vtable-base",
                            node.text);
      } else {
        set_exported_symbol(binding.base_symbol,
                            symbol_linkage::make_internal_symbol_identity(binding.base_symbol,
                                                                          symbol_linkage::SL_WEAK),
                            "vtable-base",
                            node.text);
      }
    }
    vtable_bindings_[node.text] = binding;
  }

  void collect_vtt(const CallSemNode & node)
  {
    if(node.text.empty()) {
      throw logic_error("VTT definition missing name");
    }
    const string global_name =
        !callsem_symbol(node).internal_symbol.empty() ?
            callsem_symbol(node).internal_symbol :
            lowir_name(node.text + "::__vtt");
    LowIRGlobal global = make_data_global(global_name, true);
    for(size_t i = 0; i < node.children.size(); ++i) {
      if(node.children[i].kind != CallSemKind::vtt_entry) {
        throw logic_error("invalid VTT entry");
      }
      map<string, VTableBinding>::const_iterator found = vtable_bindings_.find(node.children[i].text);
      if(found == vtable_bindings_.end()) {
        throw logic_error("missing VTT table binding " + node.children[i].text);
      }
      const unsigned long long addend =
          node.children[i].has_uint_value ? callsem_uint_value(node.children[i]) :
                                            found->second.address_point_offset;
      global.data_items.push_back(string("ptr addr ") +
                                  format_global_address_operand(found->second.base_symbol,
                                                                static_cast<long long>(addend)));
    }
    globals_.push_back(global);
    if(symbol_linkage::has_object_symbol(callsem_symbol(node))) {
      set_exported_symbol(global_name, callsem_symbol(node), "vtt", node.text);
    } else {
      set_exported_symbol(global_name,
                          symbol_linkage::make_internal_symbol_identity(global_name,
                                                                        symbol_linkage::SL_WEAK),
                          "vtt",
                          node.text);
    }
  }

  void collect_global(const CallSemNode & node)
  {
    TypePtr base = strip_top_level_cv(node.semantic_type);
    if(base && is_reference_type(base)) {
      const string name = node_internal_symbol(node);
      globals_.push_back(make_scalar_global(name,
                                            "ptr",
                                            "zero",
                                            false,
                                            false,
                                            node.is_thread_local));
      if(!callsem_local_static_guard_symbol(node).empty()) {
        append_local_static_guard_global(node);
        return;
      }
      if(!node.children.empty()) {
        if(node.children.size() != 1) {
          throw logic_error("global reference initializer arity");
        }
        CallSemNode & action =
            make_global_reference_dynamic_initializer_action(node, node.children[0]);
        if(node.is_thread_local) {
          GlobalBinding & binding = global_bindings_.find(name)->second;
          const string init_symbol = thread_local_init_internal_symbol(name);
          binding.thread_local_init_symbol = init_symbol;
          thread_local_init_actions_.push_back(make_pair(init_symbol, &action));
        } else {
          global_ctor_actions_.push_back(&action);
        }
      }
      return;
    }
    if(base && base->kind == Type::TK_ARRAY) {
      const GlobalBinding & binding = global_bindings_.find(node_internal_symbol(node))->second;
      LowIRGlobal global = make_data_global(binding.storage, false, binding.thread_local_storage);
      const bool guarded_array =
          !callsem_local_static_guard_symbol(node).empty();
      if(node.children.empty()) {
        global.data_items.push_back(string("zero ") + to_string(backend_storage_size(node.semantic_type)));
      } else if(node.children.size() == 1 &&
                node.children[0].kind == CallSemKind::braced_init_list) {
        const CallSemNode & init = node.children[0];
        if(init.children.size() > base->bound) {
          throw logic_error("too many global array initializer elements");
        }
        if(guarded_array &&
           is_complete_class_value_type(strip_top_level_cv(base->inner))) {
          global.data_items.push_back(
              string("zero ") + to_string(backend_storage_size(node.semantic_type)));
          globals_.push_back(global);
          append_local_static_guard_global(node);
          return;
        }
        if(try_collect_global_class_array(node, base, init)) {
          append_local_static_guard_global(node);
          return;
        }
        if(!append_global_array_initializer_items(global.data_items, base, init)) {
          if(is_unreferenced_unsupported_constexpr_global(node)) {
            return;
          }
          throw logic_error("unsupported global array initializer for " + node.text);
        }
      } else if(guarded_array &&
                all_of(node.children.begin(),
                       node.children.end(),
                       [](const CallSemNode & child)
                       {
                         return child.kind == CallSemKind::constructor_action ||
                                child.kind == CallSemKind::destructor_action;
                       })) {
        global.data_items.push_back(
            string("zero ") + to_string(backend_storage_size(node.semantic_type)));
      } else {
        throw logic_error("unsupported global array initializer for " + node.text);
      }
      globals_.push_back(global);
      append_local_static_guard_global(node);
      return;
    }
    if(base && base->kind == Type::TK_NAMED) {
      GlobalBinding & binding = global_bindings_.find(node_internal_symbol(node))->second;
      LowIRGlobal global = make_data_global(binding.storage, false, binding.thread_local_storage);
      global.data_items.push_back(string("zero ") + to_string(backend_storage_size(node.semantic_type)));
      globals_.push_back(global);
      if(!callsem_local_static_guard_symbol(node).empty()) {
        append_local_static_guard_global(node);
      }
      for(size_t i = 0; i < node.children.size(); ++i) {
        const bool initializer_action =
            node.children[i].kind == CallSemKind::constructor_action ||
            node.children[i].kind == CallSemKind::expression_statement;
        if(initializer_action) {
          if(callsem_local_static_guard_symbol(node).empty()) {
            global_ctor_actions_.push_back(&node.children[i]);
          } else if(node.is_thread_local) {
            const string init_symbol = thread_local_init_internal_symbol(binding.storage);
            binding.thread_local_init_symbol = init_symbol;
            thread_local_init_actions_.push_back(make_pair(init_symbol, &node.children[i]));
          }
        } else if(callsem_local_static_guard_symbol(node).empty() &&
                  is_direct_global_class_materialization_child(node, node.children[i])) {
          append_global_object_materialization_constructor_action(node, node.children[i]);
        } else if(!node.is_thread_local &&
                  node.children[i].kind == CallSemKind::destructor_action &&
                  !node.children[i].trivial_lifecycle) {
          global_dtor_actions_.push_back(&node.children[i]);
        }
      }
      return;
    }

    const string name = node_internal_symbol(node);
    const string type = lowir_memory_type_for(node.semantic_type);
    if(node.children.empty()) {
      globals_.push_back(make_scalar_global(name,
                                            type,
                                            "zero",
                                            false,
                                            false,
                                            node.is_thread_local));
      return;
    }

    if(!callsem_local_static_guard_symbol(node).empty()) {
      if(base && base->kind == Type::TK_ARRAY) {
        LowIRGlobal global = make_data_global(name, false, node.is_thread_local);
        global.data_items.push_back(
            string("zero ") + to_string(backend_storage_size(node.semantic_type)));
        globals_.push_back(global);
      } else {
        globals_.push_back(make_scalar_global(name,
                                              type,
                                              "zero",
                                              false,
                                              false,
                                              node.is_thread_local));
      }
      append_local_static_guard_global(node);
      return;
    }

    const CallSemNode & init = node.children[0];
    string value;
    bool is_addr = false;
    long long addr_addend = 0;
    if(!evaluate_global_initializer(init, value, is_addr, addr_addend)) {
      if(is_unreferenced_unsupported_constexpr_global(node)) {
        return;
      }
      if(node.children.size() == 1 && !is_addr && !node.is_thread_local) {
        globals_.push_back(make_scalar_global(name,
                                              type,
                                              "zero",
                                              false,
                                              false,
                                              node.is_thread_local));
        CallSemNode & action =
            make_global_scalar_dynamic_initializer_action(node, init);
        global_ctor_actions_.push_back(&action);
        return;
      }
      throw logic_error("unsupported global initializer in PA14");
    }
    if(is_member_function_pointer_type(node.semantic_type) && is_addr) {
      if(addr_addend != 0) {
        throw logic_error("member-function pointer global initializer addend unsupported");
      }
      LowIRGlobal global = make_data_global(name, false, node.is_thread_local);
      append_member_function_pointer_global_data_items(global.data_items, value);
      globals_.push_back(global);
      return;
    }
    LowIRGlobal global = make_scalar_global(name,
                                            type,
                                            value,
                                            is_addr,
                                            false,
                                            node.is_thread_local);
    global.addr_addend = addr_addend;
    globals_.push_back(global);
  }

  bool evaluate_global_initializer(const CallSemNode & node,
                                   string & out,
                                   bool & is_addr,
                                   long long & addr_addend)
  {
    is_addr = false;
    addr_addend = 0;
    if(node.kind == CallSemKind::literal) {
      QuoteLiteralData string_literal;
      if(try_parse_string_literal_node(node, string_literal)) {
        map<string, string>::const_iterator found = string_literal_symbols_.find(node.text);
        if(found == string_literal_symbols_.end()) {
          throw logic_error("missing string literal global");
        }
        out = found->second;
        is_addr = true;
        return true;
      }
      out = normalize_literal_token(node);
      return true;
    }
    if(node.kind == CallSemKind::sizeof_expression && node.has_uint_value) {
      out = to_string(callsem_uint_value(node));
      return true;
    }
    if(node.kind == CallSemKind::id_expression &&
       node.semantic_type && is_function_type(node.semantic_type)) {
      out = node_internal_symbol(node);
      is_addr = true;
      return true;
    }
    TypePtr base = strip_top_level_cv(node.semantic_type);
    const auto parse_integer_text =
        [](const string & text, long long & value) -> bool
        {
          if(text.empty()) {
            return false;
          }
          char * end = NULL;
          value = strtoll(text.c_str(), &end, 10);
          return end && *end == 0;
        };
    TypePtr arithmetic_result_base =
        strip_top_level_cv(remove_reference_type(node.semantic_type));
    const bool arithmetic_result_is_128 =
        arithmetic_result_base &&
        arithmetic_result_base->kind == Type::TK_FUNDAMENTAL &&
        (arithmetic_result_base->fundamental == FT_INT128 ||
         arithmetic_result_base->fundamental == FT_UINT128);
    if(node.kind == CallSemKind::id_expression &&
       node.value_category == CVC_LVALUE &&
       base && base->kind == Type::TK_ARRAY) {
      out = node_internal_symbol(node);
      is_addr = true;
      return true;
    }
    if(node.kind == CallSemKind::cast_expression && node.children.size() == 1) {
      string child;
      bool child_addr = false;
      long long child_addend = 0;
      if(!evaluate_global_initializer(node.children[0], child, child_addr, child_addend) ||
         child_addr ||
         child_addend != 0) {
        return false;
      }
      TypePtr target = strip_top_level_cv(remove_reference_type(node.semantic_type));
      long long value = 0;
      if(!target ||
         !(is_integral_type(target) || is_named_enum_scalar_type(target)) ||
         !parse_integer_text(child, value)) {
        return false;
      }
      out = child;
      return true;
    }
    if(node.kind == CallSemKind::binary_expression &&
       node.children.size() == 2 &&
       (callsem_has_token(node, OP_PLUS) || callsem_has_token(node, OP_MINUS))) {
      string lhs;
      bool lhs_addr = false;
      long long lhs_addend = 0;
      if(!evaluate_global_initializer(node.children[0], lhs, lhs_addr, lhs_addend)) {
        return false;
      }
      string rhs;
      bool rhs_addr = false;
      long long rhs_addend = 0;
      if(!evaluate_global_initializer(node.children[1], rhs, rhs_addr, rhs_addend)) {
        return false;
      }

      const auto pointer_scale =
          [](const TypePtr & type) -> long long
          {
            TypePtr base = strip_top_level_cv(remove_reference_type(type));
            if(!base) {
              return 0;
            }
            if(base->kind == Type::TK_POINTER || base->kind == Type::TK_ARRAY) {
              return static_cast<long long>(backend_storage_size(base->inner));
            }
            return 0;
          };

      const long long lhs_scale = pointer_scale(node.children[0].semantic_type);
      const long long rhs_scale = pointer_scale(node.children[1].semantic_type);
      long long integer_value = 0;
      if(lhs_addr && !rhs_addr && rhs_addend == 0 &&
         lhs_scale != 0 && parse_integer_text(rhs, integer_value)) {
        out = lhs;
        is_addr = true;
        addr_addend =
            lhs_addend + integer_value * lhs_scale * (callsem_has_token(node, OP_MINUS) ? -1 : 1);
        return true;
      }
      if(!lhs_addr && !callsem_has_token(node, OP_MINUS) &&
         rhs_addr && lhs_scale == 0 && rhs_scale != 0 &&
         parse_integer_text(lhs, integer_value) && lhs_addend == 0) {
        out = rhs;
        is_addr = true;
        addr_addend = rhs_addend + integer_value * rhs_scale;
        return true;
      }
      long long lhs_value = 0;
      long long rhs_value = 0;
      if(!arithmetic_result_is_128 &&
         !lhs_addr && !rhs_addr &&
         lhs_addend == 0 && rhs_addend == 0 &&
         parse_integer_text(lhs, lhs_value) &&
         parse_integer_text(rhs, rhs_value)) {
        out = to_string(callsem_has_token(node, OP_MINUS) ?
                            (lhs_value - rhs_value) :
                            (lhs_value + rhs_value));
        return true;
      }
      return false;
    }
    if(node.kind == CallSemKind::binary_expression &&
       node.children.size() == 2 &&
       (callsem_has_token(node, OP_STAR) ||
        callsem_has_token(node, OP_DIV) ||
        callsem_has_token(node, OP_MOD) ||
        callsem_has_token(node, OP_LSHIFT) ||
        callsem_has_token(node, OP_RSHIFT))) {
      string lhs;
      bool lhs_addr = false;
      long long lhs_addend = 0;
      if(!evaluate_global_initializer(node.children[0], lhs, lhs_addr, lhs_addend)) {
        return false;
      }
      string rhs;
      bool rhs_addr = false;
      long long rhs_addend = 0;
      if(!evaluate_global_initializer(node.children[1], rhs, rhs_addr, rhs_addend)) {
        return false;
      }
      if(arithmetic_result_is_128) {
        return false;
      }
      long long lhs_value = 0;
      long long rhs_value = 0;
      if(lhs_addr || rhs_addr ||
         lhs_addend != 0 || rhs_addend != 0 ||
         !parse_integer_text(lhs, lhs_value) ||
         !parse_integer_text(rhs, rhs_value)) {
        return false;
      }
      if(callsem_has_token(node, OP_STAR)) {
        out = to_string(lhs_value * rhs_value);
        return true;
      }
      if(callsem_has_token(node, OP_DIV)) {
        if(rhs_value == 0) {
          return false;
        }
        out = to_string(lhs_value / rhs_value);
        return true;
      }
      if(callsem_has_token(node, OP_MOD)) {
        if(rhs_value == 0) {
          return false;
        }
        out = to_string(lhs_value % rhs_value);
        return true;
      }
      if(rhs_value < 0 || rhs_value >= 63) {
        return false;
      }
      out = to_string(callsem_has_token(node, OP_LSHIFT) ?
                          (lhs_value << rhs_value) :
                          (lhs_value >> rhs_value));
      return true;
    }
    if(node.kind == CallSemKind::binary_expression &&
       node.children.size() == 2 &&
       callsem_has_token(node, OP_BOR)) {
      string lhs;
      bool lhs_addr = false;
      long long lhs_addend = 0;
      if(!evaluate_global_initializer(node.children[0], lhs, lhs_addr, lhs_addend)) {
        return false;
      }
      string rhs;
      bool rhs_addr = false;
      long long rhs_addend = 0;
      if(!evaluate_global_initializer(node.children[1], rhs, rhs_addr, rhs_addend)) {
        return false;
      }
      long long lhs_value = 0;
      long long rhs_value = 0;
      if(lhs_addr || rhs_addr ||
         lhs_addend != 0 || rhs_addend != 0 ||
         !parse_integer_text(lhs, lhs_value) ||
         !parse_integer_text(rhs, rhs_value)) {
        return false;
      }
      out = to_string(lhs_value | rhs_value);
      return true;
    }
    if(node.kind == CallSemKind::unary_expression &&
       node.children.size() == 1 &&
       callsem_has_token(node, OP_AMP) &&
       base && base->kind == Type::TK_MEMBER_POINTER) {
      if(is_function_type(base->inner)) {
        if(node.is_virtual_dispatch) {
          out = register_virtual_member_pointer_thunk(node);
        } else {
          out = !callsem_symbol(node).internal_symbol.empty() ?
                    callsem_symbol(node).internal_symbol :
                    lookup_function_symbol(node.children[0]);
        }
        is_addr = true;
        return !out.empty();
      }
      if(node.has_uint_value) {
        out = encode_data_member_pointer_offset(callsem_uint_value(node));
        return true;
      }
      return false;
    }
    if(node.kind == CallSemKind::unary_expression && node.children.size() == 1) {
      string child;
      bool child_addr = false;
      long long child_addend = 0;
      if(!evaluate_global_initializer(node.children[0], child, child_addr, child_addend)) {
        return false;
      }
      if(callsem_has_token(node, OP_MINUS) && !child_addr) {
        out = string("-") + child;
        return true;
      }
      if(callsem_has_token(node, OP_PLUS)) {
        out = child;
        is_addr = child_addr;
        addr_addend = child_addend;
        return true;
      }
    }
    return false;
  }

  static void render_function(const LowIRFunction & function, ostringstream & out)
  {
    out << "function " << function.name << "(";
    for(size_t i = 0; i < function.params.size(); ++i) {
      if(i != 0) {
        out << ", ";
      }
      out << function.params[i].name << " : " << function.params[i].type;
      if(function.params[i].metadata.passing != lowir_internal::PPM_DIRECT) {
        out << " [pass="
            << lowir_internal::param_passing_mode_text(function.params[i].metadata.passing)
            << "]";
      }
    }
    out << ") -> " << function.return_type << " {\n";
    for(size_t i = 0; i < function.slots.size(); ++i) {
      out << "  slot " << function.slots[i].first << " : " << function.slots[i].second << "\n";
    }
    if(!function.slots.empty()) {
      out << "\n";
    }
    for(size_t i = 0; i < function.blocks.size(); ++i) {
      out << "  block " << function.blocks[i].label << ":\n";
      for(size_t j = 0; j < function.blocks[i].instructions.size(); ++j) {
        out << "    " << function.blocks[i].instructions[j] << "\n";
      }
      if(i + 1 != function.blocks.size()) {
        out << "\n";
      }
    }
    out << "}";
  }
};

} // namespace

lowir_internal::Program build_lowir_program(const vector<CallSemNode> & translation_units,
                                            bool validate_closure,
                                            bool emit_runtime_support,
                                            bool enable_debug_value_names)
{
  ProgramGenerator generator(translation_units,
                             validate_closure,
                             emit_runtime_support,
                             enable_debug_value_names);
  return generator.build_program();
}

string generate_lowir_program(const vector<CallSemNode> & translation_units,
                              bool validate_closure,
                              bool emit_runtime_support,
                              bool enable_debug_value_names)
{
  return lowir_internal::dump_program(
      build_lowir_program(translation_units,
                          validate_closure,
                          emit_runtime_support,
                          enable_debug_value_names));
}
