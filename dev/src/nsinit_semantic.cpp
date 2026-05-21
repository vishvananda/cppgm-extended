#include <algorithm>
#include <cerrno>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <cstdlib>
#include <map>
#include <memory>
#include <set>
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

using namespace std;

#include "callsemantic.h"
#include "cpp_decl_bridge.h"
#include "cpp_decl_model.h"
#include "cppast_parser.h"
#include "nsinit_image.h"
#include "nsinit_semantic.h"
#include "types.h"

using namespace cpp_decl;

namespace {

const size_t INVALID_TU = static_cast<size_t>(-1);

using nsinit_image::append_zero_bytes;
using nsinit_image::ImageSymbol;
using nsinit_image::layout_symbol;
using nsinit_image::apply_symbol_relocations;
using nsinit_image::little_endian_bytes;
using nsinit_image::Relocation;

enum ValueCategory
{
  VC_LVALUE,
  VC_PRVALUE
};

struct ConstantValue
{
  enum Kind
  {
    CK_NONE,
    CK_SCALAR,
    CK_POINTER
  };

  Kind kind = CK_NONE;
  vector<char> bytes;
  ImageSymbol * target = nullptr;
  size_t addend = 0;
  bool is_null_pointer = false;
};

struct ExprResult
{
  TypePtr type;
  ImageSymbol * symbol = nullptr;
  ValueCategory value_category = VC_PRVALUE;
  bool has_constant_value = false;
  ConstantValue constant_value;
};

struct InitResult
{
  vector<char> bytes;
  vector<Relocation> relocations;
  bool has_constant_value = false;
  ConstantValue constant_value;
  ImageSymbol * reference_target = nullptr;
};

struct VariableEntity
{
  string key;
  string name;
  size_t order = 0;
  size_t tu_index = INVALID_TU;
  TypePtr type;
  bool has_definition = false;
  const CallSemNode * initializer = nullptr;
  ImageSymbol image;
  ImageSymbol * reference_target = nullptr;
  bool has_constant_value = false;
  ConstantValue constant_value;
};

struct FunctionEntity
{
  string key;
  string name;
  size_t order = 0;
  TypePtr type;
  bool has_definition = false;
  size_t definition_tu_index = INVALID_TU;
  ImageSymbol image;
};

struct StringLiteralEntity
{
  size_t order = 0;
  TypePtr type;
  ImageSymbol image;
};

struct TemporaryEntity
{
  size_t order = 0;
  TypePtr type;
  ImageSymbol image;
  bool has_constant_value = false;
  ConstantValue constant_value;
};

struct VariableRecord
{
  size_t encounter_order = 0;
  size_t tu_index = 0;
  string name;
  bool unnamed_scope = false;
  const CallSemNode * node = nullptr;
};

struct FunctionRecord
{
  size_t encounter_order = 0;
  size_t tu_index = 0;
  string name;
  bool unnamed_scope = false;
  const CallSemNode * node = nullptr;
};

struct Program
{
  size_t next_order = 0;
  vector<unique_ptr<VariableEntity> > owned_variables;
  vector<unique_ptr<FunctionEntity> > owned_functions;
  vector<unique_ptr<StringLiteralEntity> > owned_strings;
  vector<StringLiteralEntity *> string_order;
  vector<unique_ptr<TemporaryEntity> > owned_temps;
  vector<TemporaryEntity *> temp_order;
  map<string, vector<pair<size_t, VariableEntity *> > > variable_aliases;
  map<string, vector<pair<size_t, FunctionEntity *> > > function_aliases;
};

bool declarator_has_array_suffix(const CppAstNode & node)
{
  if(node.kind == CppAstKind::array_suffix) {
    return true;
  }
  for(size_t i = 0; i < node.children.size(); ++i) {
    if(declarator_has_array_suffix(node.children[i])) {
      return true;
    }
  }
  return false;
}

template<typename T>
vector<char> pod_bytes(const T & value)
{
  vector<char> result(sizeof(T));
  memcpy(result.data(), &value, sizeof(T));
  return result;
}

ConstantValue make_scalar_constant(const vector<char> & bytes)
{
  ConstantValue result;
  result.kind = ConstantValue::CK_SCALAR;
  result.bytes = bytes;
  return result;
}

ConstantValue make_pointer_constant(ImageSymbol * target)
{
  ConstantValue result;
  result.kind = ConstantValue::CK_POINTER;
  result.target = target;
  result.is_null_pointer = target == nullptr;
  result.bytes.assign(8, '\0');
  return result;
}

bool is_integral_fundamental(EFundamentalType type)
{
  switch(type) {
    case FT_BOOL:
    case FT_SIGNED_CHAR:
    case FT_SHORT_INT:
    case FT_INT:
    case FT_LONG_INT:
    case FT_LONG_LONG_INT:
    case FT_INT128:
    case FT_UNSIGNED_CHAR:
    case FT_UNSIGNED_SHORT_INT:
    case FT_UNSIGNED_INT:
    case FT_UNSIGNED_LONG_INT:
    case FT_UNSIGNED_LONG_LONG_INT:
    case FT_UINT128:
    case FT_WCHAR_T:
    case FT_CHAR:
    case FT_CHAR16_T:
    case FT_CHAR32_T:
    case FT_NULLPTR_T:
      return true;
    case FT_FLOAT:
    case FT_DOUBLE:
    case FT_LONG_DOUBLE:
    case FT_VOID:
      return false;
  }
  return false;
}

bool is_floating_fundamental(EFundamentalType type)
{
  return type == FT_FLOAT ||
         type == FT_DOUBLE ||
         type == FT_LONG_DOUBLE;
}

unsigned long long decode_unsigned_scalar_bytes(const vector<char> & bytes)
{
  unsigned long long value = 0;
  const size_t byte_count = min<size_t>(bytes.size(), sizeof(value));
  for(size_t i = 0; i < byte_count; ++i) {
    value |= static_cast<unsigned long long>(static_cast<unsigned char>(bytes[i]))
             << (i * 8);
  }
  return value;
}

long long decode_signed_scalar_bytes(const vector<char> & bytes)
{
  if(bytes.empty()) {
    return 0;
  }

  const size_t byte_count = min<size_t>(bytes.size(), sizeof(unsigned long long));
  unsigned long long value = decode_unsigned_scalar_bytes(bytes);
  const size_t bit_count = byte_count * 8;
  const unsigned char top = static_cast<unsigned char>(bytes[byte_count - 1]);
  if(bit_count < sizeof(unsigned long long) * 8 &&
     ((top >> 7) & 1U) != 0U) {
    value |= (~static_cast<unsigned long long>(0)) << bit_count;
  }
  return static_cast<long long>(value);
}

template<typename T>
T decode_pod_scalar_bytes(const vector<char> & bytes)
{
  T value = T();
  if(bytes.size() != sizeof(T)) {
    throw logic_error("unexpected scalar byte width");
  }
  memcpy(&value, bytes.data(), sizeof(T));
  return value;
}

vector<char> encode_unsigned_scalar_bytes(unsigned long long value, size_t size)
{
  vector<char> bytes(size, '\0');
  const size_t byte_count = min(size, sizeof(value));
  for(size_t i = 0; i < byte_count; ++i) {
    bytes[i] = static_cast<char>((value >> (i * 8)) & 0xffU);
  }
  return bytes;
}

vector<char> encode_signed_scalar_bytes(long long value, size_t size)
{
  vector<char> bytes(size, value < 0 ? static_cast<char>(0xff) : '\0');
  const size_t byte_count = min(size, sizeof(value));
  const unsigned long long raw = static_cast<unsigned long long>(value);
  for(size_t i = 0; i < byte_count; ++i) {
    bytes[i] = static_cast<char>((raw >> (i * 8)) & 0xffU);
  }
  return bytes;
}

vector<char> reshape_integral_scalar_bytes(const vector<char> & bytes,
                                           bool source_signed,
                                           size_t dest_size)
{
  const bool negative =
      source_signed &&
      !bytes.empty() &&
      (static_cast<unsigned char>(bytes.back()) & 0x80U) != 0U;
  vector<char> result(dest_size, negative ? static_cast<char>(0xff) : '\0');
  const size_t byte_count = min(bytes.size(), dest_size);
  for(size_t i = 0; i < byte_count; ++i) {
    result[i] = bytes[i];
  }
  return result;
}

long double decode_scalar_as_long_double(const ConstantValue & value,
                                         EFundamentalType type)
{
  if(is_floating_fundamental(type)) {
    switch(type) {
      case FT_FLOAT:
        return decode_pod_scalar_bytes<float>(value.bytes);
      case FT_DOUBLE:
        return decode_pod_scalar_bytes<double>(value.bytes);
      case FT_LONG_DOUBLE:
        return decode_pod_scalar_bytes<long double>(value.bytes);
      default:
        break;
    }
  }

  if(is_integral_fundamental(type)) {
    if(type == FT_BOOL) {
      return decode_unsigned_scalar_bytes(value.bytes) != 0 ? 1.0L : 0.0L;
    }
    if(type_is_signed(type)) {
      return static_cast<long double>(decode_signed_scalar_bytes(value.bytes));
    }
    return static_cast<long double>(decode_unsigned_scalar_bytes(value.bytes));
  }

  throw logic_error("unsupported source scalar constant");
}

ConstantValue convert_scalar_constant(const ConstantValue & value,
                                      const TypePtr & source_type,
                                      const TypePtr & dest_type)
{
  TypePtr source_base = strip_top_level_cv(remove_reference_type(source_type));
  TypePtr dest_base = strip_top_level_cv(remove_reference_type(dest_type));
  if(!source_base || !dest_base ||
     source_base->kind != Type::TK_FUNDAMENTAL ||
     dest_base->kind != Type::TK_FUNDAMENTAL) {
    return value;
  }

  const EFundamentalType source_fundamental = source_base->fundamental;
  const EFundamentalType dest_fundamental = dest_base->fundamental;
  const size_t dest_size = type_size(dest_base);

  if(source_fundamental == dest_fundamental) {
    return value;
  }

  if(dest_fundamental == FT_BOOL) {
    const long double numeric = decode_scalar_as_long_double(value, source_fundamental);
    return make_scalar_constant(vector<char>(1, numeric == 0.0L ? '\0' : '\1'));
  }

  if(is_integral_fundamental(dest_fundamental)) {
    if(is_floating_fundamental(source_fundamental)) {
      const long double numeric = decode_scalar_as_long_double(value, source_fundamental);
      if(type_is_signed(dest_fundamental)) {
        return make_scalar_constant(
            encode_signed_scalar_bytes(static_cast<long long>(numeric), dest_size));
      }
      return make_scalar_constant(
          encode_unsigned_scalar_bytes(static_cast<unsigned long long>(numeric),
                                       dest_size));
    }

    return make_scalar_constant(
        reshape_integral_scalar_bytes(value.bytes,
                                     type_is_signed(source_fundamental),
                                     dest_size));
  }

  if(is_floating_fundamental(dest_fundamental)) {
    const long double numeric = decode_scalar_as_long_double(value, source_fundamental);
    switch(dest_fundamental) {
      case FT_FLOAT:
        return make_scalar_constant(pod_bytes(static_cast<float>(numeric)));
      case FT_DOUBLE:
        return make_scalar_constant(pod_bytes(static_cast<double>(numeric)));
      case FT_LONG_DOUBLE:
        return make_scalar_constant(pod_bytes(static_cast<long double>(numeric)));
      default:
        break;
    }
  }

  return value;
}

ValueCategory to_value_category(CallValueCategory category)
{
  return category == CVC_LVALUE ? VC_LVALUE : VC_PRVALUE;
}

string entity_name(const CallSemNode & node)
{
  return callsem_resolved_name(node).empty() ? node.text.str() : callsem_resolved_name(node);
}

string qualified_entity_name(const QualifiedName & name)
{
  string out;
  for(size_t i = 0; i < name.qualifiers.size(); ++i) {
    if(!out.empty()) {
      out += "::";
    }
    out += name.qualifiers[i];
  }
  if(!out.empty()) {
    out += "::";
  }
  out += name.name;
  return out;
}

string symbol_lookup_key(const symbol_linkage::SymbolIdentity & symbol)
{
  if(!symbol.object_symbol.empty()) {
    return string("obj:") + symbol.object_symbol;
  }
  if(!symbol.internal_symbol.empty()) {
    return string("int:") + symbol.internal_symbol;
  }
  return string();
}

void register_function_symbol_alias(
    map<string, vector<pair<size_t, FunctionEntity *> > > & aliases,
    const string & key,
    size_t tu_index,
    FunctionEntity * entity)
{
  if(key.empty()) {
    return;
  }
  vector<pair<size_t, FunctionEntity *> > & bucket = aliases[key];
  for(size_t i = 0; i < bucket.size(); ++i) {
    if(bucket[i].first == tu_index && bucket[i].second == entity) {
      return;
    }
  }
  bucket.push_back(make_pair(tu_index, entity));
}

void register_variable_symbol_alias(
    map<string, vector<pair<size_t, VariableEntity *> > > & aliases,
    const string & key,
    size_t tu_index,
    VariableEntity * entity)
{
  if(key.empty()) {
    return;
  }
  vector<pair<size_t, VariableEntity *> > & bucket = aliases[key];
  for(size_t i = 0; i < bucket.size(); ++i) {
    if(bucket[i].first == tu_index && bucket[i].second == entity) {
      return;
    }
  }
  bucket.push_back(make_pair(tu_index, entity));
}

FunctionEntity * lookup_function_symbol_alias(
    const map<string, vector<pair<size_t, FunctionEntity *> > > & aliases,
    const string & key,
    size_t tu_index)
{
  map<string, vector<pair<size_t, FunctionEntity *> > >::const_iterator found =
      aliases.find(key);
  if(found == aliases.end()) {
    return nullptr;
  }

  FunctionEntity * shared = nullptr;
  for(size_t i = 0; i < found->second.size(); ++i) {
    if(found->second[i].first == tu_index) {
      return found->second[i].second;
    }
    if(!shared) {
      shared = found->second[i].second;
    } else if(shared != found->second[i].second) {
      shared = nullptr;
    }
  }
  return shared;
}

VariableEntity * lookup_variable_symbol_alias(
    const map<string, vector<pair<size_t, VariableEntity *> > > & aliases,
    const string & key,
    size_t tu_index)
{
  map<string, vector<pair<size_t, VariableEntity *> > >::const_iterator found =
      aliases.find(key);
  if(found == aliases.end()) {
    return nullptr;
  }

  VariableEntity * shared = nullptr;
  for(size_t i = 0; i < found->second.size(); ++i) {
    if(found->second[i].first == tu_index) {
      return found->second[i].second;
    }
    if(!shared) {
      shared = found->second[i].second;
    } else if(shared != found->second[i].second) {
      shared = nullptr;
    }
  }
  return shared;
}

vector<char> encode_string_units(const QuoteLiteralData & literal,
                                 EFundamentalType element_type)
{
  vector<char> result;
  const size_t element_size = type_to_size(element_type);
  const vector<unsigned long long> & units = quote_literal_string_units(literal);
  for(size_t i = 0; i < units.size(); ++i) {
    const unsigned long long value = units[i];
    if(element_size == 1 && value > 0xffU) {
      throw logic_error("string literal element out of range");
    }
    if(element_size == 2 && value > 0xffffU) {
      throw logic_error("string literal element out of range");
    }
    const vector<char> bytes = little_endian_bytes(value, element_size);
    result.insert(result.end(), bytes.begin(), bytes.end());
  }
  const vector<char> zero(element_size, '\0');
  result.insert(result.end(), zero.begin(), zero.end());
  return result;
}

ConstantValue encode_scalar_literal(const TypePtr & type,
                                    const CallSemNode & node)
{
  TypePtr base = strip_top_level_cv(type);
  if(!base) {
    throw logic_error("missing literal type");
  }
  if(base->kind == Type::TK_CV) {
    base = strip_top_level_cv(base->inner);
  }
  if(!base) {
    throw logic_error("missing literal base type");
  }

  if(base->kind == Type::TK_FUNDAMENTAL) {
    switch(base->fundamental) {
      case FT_BOOL:
      case FT_SIGNED_CHAR:
      case FT_SHORT_INT:
      case FT_INT:
      case FT_LONG_INT:
      case FT_LONG_LONG_INT:
      case FT_UNSIGNED_CHAR:
      case FT_UNSIGNED_SHORT_INT:
      case FT_UNSIGNED_INT:
      case FT_UNSIGNED_LONG_INT:
      case FT_UNSIGNED_LONG_LONG_INT:
      case FT_INT128:
      case FT_UINT128:
      case FT_WCHAR_T:
      case FT_CHAR:
      case FT_CHAR16_T:
      case FT_CHAR32_T:
      case FT_NULLPTR_T:
      {
        unsigned long long value = 0;
        if(node.has_uint_value) {
          value = callsem_uint_value(node);
        } else if(node.has_int_value) {
          value = static_cast<unsigned long long>(callsem_int_value(node));
        } else {
          char * end = nullptr;
          errno = 0;
          value = strtoull(node.text.c_str(), &end, 0);
          if(end == node.text.c_str() || errno != 0) {
            throw logic_error("unable to encode scalar literal");
          }
        }
        return make_scalar_constant(little_endian_bytes(value, type_size(base)));
      }
      case FT_FLOAT:
      {
        const float value = static_cast<float>(strtod(node.text.c_str(), nullptr));
        return make_scalar_constant(pod_bytes(value));
      }
      case FT_DOUBLE:
      {
        const double value = strtod(node.text.c_str(), nullptr);
        return make_scalar_constant(pod_bytes(value));
      }
      case FT_LONG_DOUBLE:
      {
        const long double value = strtold(node.text.c_str(), nullptr);
        return make_scalar_constant(pod_bytes(value));
      }
      case FT_VOID:
        break;
    }
  }

  throw logic_error("unsupported scalar literal type");
}

bool array_accepts_string_literal(const TypePtr & array_type,
                                  EFundamentalType literal_element_type)
{
  TypePtr element = strip_top_level_cv(array_type->inner);
  if(!element || element->kind != Type::TK_FUNDAMENTAL) {
    return false;
  }

  EFundamentalType dest = element->fundamental;
  if(literal_element_type == FT_CHAR) {
    return dest == FT_CHAR ||
           dest == FT_SIGNED_CHAR ||
           dest == FT_UNSIGNED_CHAR;
  }

  return dest == literal_element_type;
}

StringLiteralEntity * create_string_literal(Program & program,
                                            const TypePtr & literal_type,
                                            const string & text)
{
  QuoteLiteralData literal = parse_quote_literal(text);
  TypePtr array_type = strip_top_level_cv(literal_type);
  if(!array_type || array_type->kind != Type::TK_ARRAY) {
    throw logic_error("string literal missing array type");
  }
  TypePtr element = strip_top_level_cv(array_type->inner);
  if(!element || element->kind != Type::TK_FUNDAMENTAL) {
    throw logic_error("string literal missing element type");
  }

  unique_ptr<StringLiteralEntity> entity(new StringLiteralEntity());
  entity->order = program.next_order++;
  entity->type = literal_type;
  entity->image.alignment = type_alignment(literal_type);
  entity->image.bytes = encode_string_units(literal, element->fundamental);
  StringLiteralEntity * result = entity.get();
  program.string_order.push_back(result);
  program.owned_strings.push_back(std::move(entity));
  return result;
}

TemporaryEntity * create_temporary(Program & program,
                                   const TypePtr & type,
                                   const InitResult & init)
{
  unique_ptr<TemporaryEntity> entity(new TemporaryEntity());
  entity->order = program.next_order++;
  entity->type = type;
  entity->image.alignment = type_alignment(type);
  entity->image.bytes = init.bytes;
  entity->image.relocations = init.relocations;
  entity->has_constant_value = init.has_constant_value;
  entity->constant_value = init.constant_value;
  TemporaryEntity * result = entity.get();
  program.temp_order.push_back(result);
  program.owned_temps.push_back(std::move(entity));
  return result;
}

void collect_records(const CallSemNode & node,
                     size_t & encounter_order,
                     size_t tu_index,
                     bool in_unnamed_namespace,
                     const string & namespace_prefix,
                     set<string> & seen_named_namespaces,
                     vector<VariableRecord> & variables,
                     vector<FunctionRecord> & functions)
{
  const auto record_name = [&namespace_prefix](const CallSemNode & current) -> string
  {
    const string name = entity_name(current);
    const QualifiedName * qualified = callsem_qualified_name_syntax(current).get();
    if(qualified && (qualified->rooted || !qualified->qualifiers.empty())) {
      return qualified_entity_name(*qualified);
    }
    if(namespace_prefix.empty() ||
       (qualified && (qualified->rooted || !qualified->qualifiers.empty()))) {
      return name;
    }
    return namespace_prefix + "::" + name;
  };

  if(node.kind == CallSemKind::translation_unit) {
    for(size_t i = 0; i < node.children.size(); ++i) {
      collect_records(node.children[i],
                      encounter_order,
                      tu_index,
                      in_unnamed_namespace,
                      namespace_prefix,
                      seen_named_namespaces,
                      variables,
                      functions);
    }
    return;
  }

  if(node.kind == CallSemKind::namespace_definition) {
    const bool nested_unnamed = in_unnamed_namespace || node.text == "<unnamed>";
    string nested_prefix = namespace_prefix;
    if(node.text != "<unnamed>") {
      nested_prefix = namespace_prefix.empty() ?
          node.text.str() :
          namespace_prefix + "::" + node.text;
      if(node.is_inline_namespace &&
         seen_named_namespaces.count(nested_prefix) != 0) {
        throw logic_error("extension namespace cannot be inline");
      }
      seen_named_namespaces.insert(nested_prefix);
    }
    for(size_t i = 0; i < node.children.size(); ++i) {
      collect_records(node.children[i],
                      encounter_order,
                      tu_index,
                      nested_unnamed,
                      nested_prefix,
                      seen_named_namespaces,
                      variables,
                      functions);
    }
    return;
  }

  if(node.kind == CallSemKind::variable) {
    VariableRecord record;
    record.encounter_order = encounter_order++;
    record.tu_index = tu_index;
    record.name = record_name(node);
    record.unnamed_scope = in_unnamed_namespace;
    record.node = &node;
    variables.push_back(record);
    return;
  }

  if(node.kind == CallSemKind::function_declaration ||
     node.kind == CallSemKind::function_definition) {
    FunctionRecord record;
    record.encounter_order = encounter_order++;
    record.tu_index = tu_index;
    record.name = record_name(node);
    record.unnamed_scope = in_unnamed_namespace;
    record.node = &node;
    functions.push_back(record);
    return;
  }
}

TypePtr make_string_literal_storage_type(const string & text)
{
  QuoteLiteralData literal = parse_quote_literal(text);
  if(literal.quote != '"' || !literal.ud_suffix.empty()) {
    return TypePtr();
  }
  return make_array(make_fundamental(string_literal_element_type(literal)),
                    true,
                    quote_literal_string_unit_count(literal) + 1);
}

void collect_array_string_literal_entities(const CppAstNode & node,
                                           Program & program)
{
  if(node.kind == CppAstKind::translation_unit ||
     node.kind == CppAstKind::namespace_definition) {
    for(size_t i = 0; i < node.children.size(); ++i) {
      collect_array_string_literal_entities(node.children[i], program);
    }
    return;
  }

  if(node.kind != CppAstKind::simple_declaration) {
    return;
  }

  const CppAstNode * init_list = find_child(node, CppAstKind::init_declarator_list);
  if(!init_list) {
    return;
  }

  for(size_t i = 0; i < init_list->children.size(); ++i) {
    const CppAstNode * declarator = find_child(init_list->children[i], CppAstKind::declarator);
    const CppAstNode * initializer = find_child(init_list->children[i], CppAstKind::initializer);
    if(!declarator || !initializer || initializer->children.size() != 1 ||
       !declarator_has_array_suffix(*declarator)) {
      continue;
    }

    const CppAstNode & payload = initializer->children[0];
    if(payload.kind != CppAstKind::literal) {
      continue;
    }

    TypePtr literal_type = make_string_literal_storage_type(payload.value);
    if(!literal_type) {
      continue;
    }
    create_string_literal(program, literal_type, payload.value);
  }
}

FunctionEntity * lookup_function(Program & program,
                                 const CallSemNode & node,
                                 size_t tu_index)
{
  const string lookup_key = symbol_lookup_key(callsem_symbol(node));
  if(!lookup_key.empty()) {
    return lookup_function_symbol_alias(program.function_aliases, lookup_key, tu_index);
  }
  return nullptr;
}

VariableEntity * lookup_variable(Program & program,
                                 const CallSemNode & node,
                                 size_t tu_index)
{
  const string lookup_key = symbol_lookup_key(callsem_symbol(node));
  if(!lookup_key.empty()) {
    return lookup_variable_symbol_alias(program.variable_aliases, lookup_key, tu_index);
  }
  return nullptr;
}

ExprResult evaluate_expression(Program & program,
                               const CallSemNode & node,
                               size_t tu_index);

InitResult build_initializer(Program & program,
                             const TypePtr & type,
                             const CallSemNode & initializer,
                             size_t tu_index)
{
  TypePtr dest = strip_top_level_cv(type);
  if(!dest) {
    throw logic_error("missing initializer target type");
  }

  if(dest->kind == Type::TK_ARRAY) {
    if(!dest->has_bound) {
      throw logic_error("array bound must be known");
    }

    if(initializer.kind == CallSemKind::braced_init_list) {
      InitResult result;
      TypePtr element_type = strip_top_level_cv(dest->inner);
      for(size_t i = 0; i < initializer.children.size(); ++i) {
        ExprResult element = evaluate_expression(program, initializer.children[i], tu_index);
        if(!element.has_constant_value || element.constant_value.kind != ConstantValue::CK_SCALAR) {
          throw logic_error("array initializer element must be scalar");
        }
        const ConstantValue converted =
            convert_scalar_constant(element.constant_value, element.type, element_type);
        result.bytes.insert(result.bytes.end(),
                            converted.bytes.begin(),
                            converted.bytes.end());
      }
      result.bytes.resize(type_size(dest), '\0');
      return result;
    }

    ExprResult expr = evaluate_expression(program, initializer, tu_index);
    if(!expr.symbol || expr.value_category != VC_LVALUE || !is_array_type(expr.type)) {
      throw logic_error("array initializer requires braced-init-list");
    }

    TypePtr source_array = strip_top_level_cv(expr.type);
    TypePtr source_element = source_array ? strip_top_level_cv(source_array->inner) : TypePtr();
    if(!source_element || source_element->kind != Type::TK_FUNDAMENTAL ||
       !array_accepts_string_literal(dest, source_element->fundamental)) {
      throw logic_error("invalid string literal initializer");
    }

    const size_t dest_element_size = type_size(strip_top_level_cv(dest->inner));
    const size_t source_elements = source_array->has_bound ? source_array->bound : 0;
    if(source_elements > dest->bound) {
      throw logic_error("string literal too long");
    }

    InitResult result;
    result.bytes = expr.symbol->bytes;
    result.bytes.resize(dest->bound * dest_element_size, '\0');
    return result;
  }

  ExprResult expr = evaluate_expression(program, initializer, tu_index);
  if(dest->kind == Type::TK_POINTER) {
    InitResult result;
    result.bytes.assign(8, '\0');
    if(expr.value_category == VC_LVALUE && expr.symbol) {
      result.relocations.push_back({0, expr.symbol, 0});
      result.has_constant_value = true;
      result.constant_value = make_pointer_constant(expr.symbol);
      return result;
    }
    if(expr.has_constant_value &&
       expr.constant_value.kind == ConstantValue::CK_POINTER &&
       expr.constant_value.is_null_pointer) {
      result.has_constant_value = true;
      result.constant_value = expr.constant_value;
      return result;
    }
    throw logic_error("invalid pointer initializer");
  }

  if(dest->kind == Type::TK_LVALUE_REFERENCE ||
     dest->kind == Type::TK_RVALUE_REFERENCE) {
    InitResult result;
    result.bytes.assign(8, '\0');
    if(expr.value_category == VC_LVALUE && expr.symbol) {
      result.relocations.push_back({0, expr.symbol, 0});
      result.reference_target = expr.symbol;
      if(expr.has_constant_value) {
        result.has_constant_value = true;
        result.constant_value = expr.constant_value;
      }
      return result;
    }
    if(dest->kind == Type::TK_LVALUE_REFERENCE &&
       type_is_const_object(dest->inner) &&
       expr.value_category == VC_PRVALUE &&
       expr.has_constant_value &&
       expr.constant_value.kind == ConstantValue::CK_SCALAR) {
      const ConstantValue converted =
          convert_scalar_constant(expr.constant_value, expr.type, dest->inner);
      InitResult temp_init;
      temp_init.bytes = converted.bytes;
      temp_init.has_constant_value = true;
      temp_init.constant_value = converted;
      TemporaryEntity * temp = create_temporary(program, dest->inner, temp_init);
      result.relocations.push_back({0, &temp->image, 0});
      result.reference_target = &temp->image;
      result.has_constant_value = temp->has_constant_value;
      result.constant_value = temp->constant_value;
      return result;
    }
    throw logic_error("invalid reference initializer");
  }

  if(dest->kind == Type::TK_FUNDAMENTAL || dest->kind == Type::TK_CV) {
    if(!expr.has_constant_value || expr.constant_value.kind != ConstantValue::CK_SCALAR) {
      throw logic_error("unsupported non-constant scalar initializer");
    }
    const ConstantValue converted =
        convert_scalar_constant(expr.constant_value, expr.type, dest);
    InitResult result;
    result.bytes = converted.bytes;
    result.has_constant_value = true;
    result.constant_value = converted;
    return result;
  }

  throw logic_error("unsupported initializer type");
}

ExprResult evaluate_literal(Program & program,
                            const CallSemNode & node,
                            size_t tu_index)
{
  (void)tu_index;
  ExprResult result;
  result.type = node.semantic_type;
  result.value_category = to_value_category(node.value_category);
  TypePtr value_type = strip_top_level_cv(result.type);
  if(value_type &&
     is_reference_type(value_type) &&
     result.value_category == VC_PRVALUE) {
    value_type = strip_top_level_cv(value_type->inner);
  }

  if(value_type &&
     value_type->kind == Type::TK_ARRAY &&
     result.value_category == VC_LVALUE) {
    StringLiteralEntity * entity = create_string_literal(program, value_type, node.text);
    result.symbol = &entity->image;
    return result;
  }

  if(value_type &&
     value_type->kind == Type::TK_FUNDAMENTAL &&
     value_type->fundamental == FT_NULLPTR_T) {
    result.has_constant_value = true;
    result.constant_value = make_pointer_constant(nullptr);
    return result;
  }

  if(value_type && value_type->kind == Type::TK_POINTER) {
    result.has_constant_value = true;
    result.constant_value = make_pointer_constant(nullptr);
    return result;
  }

  result.has_constant_value = true;
  result.constant_value = encode_scalar_literal(value_type ? value_type : result.type, node);
  return result;
}

ExprResult evaluate_expression(Program & program,
                               const CallSemNode & node,
                               size_t tu_index)
{
  if(node.kind == CallSemKind::literal) {
    return evaluate_literal(program, node, tu_index);
  }

  if(node.kind == CallSemKind::id_expression) {
    ExprResult result;
    result.type = node.semantic_type;
    result.value_category = VC_LVALUE;
    VariableEntity * variable = lookup_variable(program, node, tu_index);
    if(variable) {
      result.symbol =
          is_reference_type(variable->type) && variable->reference_target ?
              variable->reference_target :
              &variable->image;
      if(variable->has_constant_value) {
        result.has_constant_value = true;
        result.constant_value = variable->constant_value;
      }
      return result;
    }

    FunctionEntity * function = lookup_function(program, node, tu_index);
    if(function) {
      result.symbol = &function->image;
      return result;
    }

    throw logic_error("unknown id-expression target");
  }

  throw logic_error("unsupported initializer expression");
}

const CppAstNode * find_child_kind(const CppAstNode & node, CppAstKind kind)
{
  for(size_t i = 0; i < node.children.size(); ++i) {
    if(node.children[i].kind == kind) {
      return &node.children[i];
    }
  }
  return nullptr;
}

bool decl_specifier_seq_has_token(const CppAstNode & node, ETokenType token)
{
  if(node.kind != CppAstKind::decl_specifier_seq) {
    return false;
  }
  for(size_t i = 0; i < node.children.size(); ++i) {
    if(node.children[i].kind == CppAstKind::decl_specifier &&
       node_has_simple_type(node.children[i], token)) {
      return true;
    }
  }
  return false;
}

void collect_ptr_operator_tokens(const CppAstNode & node,
                                 vector<ETokenType> & out)
{
  if(node.kind == CppAstKind::ptr_operator &&
     node.has_token &&
     node.token_kind == RT_SIMPLE) {
    out.push_back(node.simple_type);
  }
  for(size_t i = 0; i < node.children.size(); ++i) {
    collect_ptr_operator_tokens(node.children[i], out);
  }
}

void validate_typedef_reference_rules(const CppAstNode & declaration)
{
  const CppAstNode * specifiers = find_child_kind(declaration, CppAstKind::decl_specifier_seq);
  const CppAstNode * init_list = find_child_kind(declaration, CppAstKind::init_declarator_list);
  if(!specifiers || !init_list ||
     !decl_specifier_seq_has_token(*specifiers, KW_TYPEDEF)) {
    return;
  }

  const bool base_is_void = decl_specifier_seq_has_token(*specifiers, KW_VOID);
  for(size_t i = 0; i < init_list->children.size(); ++i) {
    const CppAstNode * declarator = find_child_kind(init_list->children[i], CppAstKind::declarator);
    if(!declarator) {
      continue;
    }
    vector<ETokenType> ptr_operators;
    collect_ptr_operator_tokens(*declarator, ptr_operators);
    for(size_t j = 0; j + 1 < ptr_operators.size(); ++j) {
      if(ptr_operators[j] == OP_AMP) {
        throw logic_error("invalid reference declarator");
      }
    }
    if(!ptr_operators.empty() &&
       ptr_operators.back() == OP_AMP &&
       base_is_void) {
      throw logic_error("invalid reference declarator");
    }
  }
}

const CppAstNode * first_identifier_node(const CppAstNode & node)
{
  if(node.kind == CppAstKind::identifier) {
    return &node;
  }
  for(size_t i = 0; i < node.children.size(); ++i) {
    if(const CppAstNode * child = first_identifier_node(node.children[i])) {
      return child;
    }
  }
  return nullptr;
}

bool qualified_prefix_matches_scope(const cpp_decl::QualifiedName & name,
                                    const vector<string> & scope_path)
{
  if(name.qualifiers.empty() || name.qualifiers.size() > scope_path.size()) {
    return false;
  }
  for(size_t i = 0; i < name.qualifiers.size(); ++i) {
    if(name.qualifiers[i] != scope_path[i]) {
      return false;
    }
  }
  return true;
}

void validate_qualified_declaration_scope(const CppAstNode & declaration,
                                          const vector<string> & scope_path)
{
  if(scope_path.empty()) {
    return;
  }
  const CppAstNode * init_list = find_child_kind(declaration, CppAstKind::init_declarator_list);
  if(!init_list) {
    return;
  }

  for(size_t i = 0; i < init_list->children.size(); ++i) {
    const CppAstNode * declarator = find_child_kind(init_list->children[i], CppAstKind::declarator);
    if(!declarator) {
      continue;
    }
    const CppAstNode * identifier = first_identifier_node(*declarator);
    if(!identifier || identifier->value.find("::") == string::npos) {
      continue;
    }

    const cpp_decl::QualifiedName * qualified = cppast_qualified_name_syntax(*identifier);
    if(!qualified ||
       !qualified_prefix_matches_scope(*qualified, scope_path)) {
      throw logic_error("qualified declaration names a non-enclosing namespace");
    }
  }
}

struct NamespaceScopeState
{
  set<string> named_namespaces;
  map<string, string> namespace_alias_targets;
  set<string> nonnamespace_bindings;
};

string namespace_scope_key(const vector<string> & scope_path)
{
  string key;
  for(size_t i = 0; i < scope_path.size(); ++i) {
    if(i != 0) {
      key += "::";
    }
    key += scope_path[i];
  }
  return key;
}

vector<string> split_namespace_scope_key(const string & key)
{
  vector<string> out;
  if(key.empty()) {
    return out;
  }

  size_t start = 0;
  while(start < key.size()) {
    const size_t next = key.find("::", start);
    if(next == string::npos) {
      out.push_back(key.substr(start));
      break;
    }
    out.push_back(key.substr(start, next - start));
    start = next + 2;
  }
  return out;
}

bool resolve_namespace_name(const QualifiedName & name,
                            const vector<string> & scope_path,
                            const map<string, NamespaceScopeState> & namespace_scopes,
                            string & out_scope_key)
{
  vector<string> resolved = name.rooted ? vector<string>() : scope_path;
  vector<string> components = name.qualifiers;
  components.push_back(name.name);

  for(size_t i = 0; i < components.size(); ++i) {
    const string current_key = namespace_scope_key(resolved);
    map<string, NamespaceScopeState>::const_iterator found =
        namespace_scopes.find(current_key);
    if(found == namespace_scopes.end()) {
      return false;
    }
    if(found->second.named_namespaces.count(components[i]) != 0) {
      resolved.push_back(components[i]);
      continue;
    }
    map<string, string>::const_iterator alias =
        found->second.namespace_alias_targets.find(components[i]);
    if(alias == found->second.namespace_alias_targets.end()) {
      return false;
    }
    resolved = split_namespace_scope_key(alias->second);
  }

  out_scope_key = namespace_scope_key(resolved);
  return true;
}

bool declarator_declared_name(const CppAstNode & declarator,
                              string & out_name)
{
  const CppAstNode * identifier = first_identifier_node(declarator);
  if(!identifier) {
    return false;
  }

  const QualifiedName * qualified = cppast_qualified_name_syntax(*identifier);
  if(qualified) {
    out_name = qualified->name;
    return true;
  }

  out_name = identifier->value;
  return true;
}

void validate_pa8_ast_node(const CppAstNode & node,
                           vector<string> & scope_path,
                           map<string, NamespaceScopeState> & namespace_scopes)
{
  NamespaceScopeState & current_scope =
      namespace_scopes[namespace_scope_key(scope_path)];

  if(node.kind == CppAstKind::translation_unit) {
    for(size_t i = 0; i < node.children.size(); ++i) {
      validate_pa8_ast_node(node.children[i], scope_path, namespace_scopes);
    }
    return;
  }

  if(node.kind == CppAstKind::namespace_definition) {
    const bool named = node.value != "<unnamed>" && !node.value.empty();
    if(named) {
      if(current_scope.namespace_alias_targets.count(node.value) != 0) {
        throw logic_error("namespace alias misuse");
      }
      if(current_scope.nonnamespace_bindings.count(node.value) != 0) {
        throw logic_error("namespace alias misuse");
      }
      current_scope.named_namespaces.insert(node.value);
      scope_path.push_back(node.value);
    }
    for(size_t i = 0; i < node.children.size(); ++i) {
      validate_pa8_ast_node(node.children[i], scope_path, namespace_scopes);
    }
    if(named) {
      scope_path.pop_back();
    }
    return;
  }

  if(node.kind == CppAstKind::namespace_alias_definition) {
    if(current_scope.named_namespaces.count(node.value) != 0 ||
       current_scope.nonnamespace_bindings.count(node.value) != 0) {
      throw logic_error("namespace alias misuse");
    }
    const CppAstNode * target = find_child(node, CppAstKind::target);
    if(!target) {
      throw logic_error("namespace alias misuse");
    }
    const QualifiedName * target_name = cppast_qualified_name_syntax(*target);
    string resolved_key;
    if(!target_name ||
       !resolve_namespace_name(*target_name, scope_path, namespace_scopes, resolved_key)) {
      throw logic_error("namespace alias misuse");
    }
    current_scope.namespace_alias_targets[node.value] = resolved_key;
    return;
  }

  if(node.kind == CppAstKind::using_declaration) {
    const CppAstNode * target = find_child(node, CppAstKind::target);
    if(target) {
      const QualifiedName * target_name = cppast_qualified_name_syntax(*target);
      string resolved_key;
      if(target_name &&
         resolve_namespace_name(*target_name, scope_path, namespace_scopes, resolved_key)) {
        throw logic_error("namespace alias misuse");
      }
      if(target_name) {
        current_scope.nonnamespace_bindings.insert(target_name->name);
      }
    }
    return;
  }

  if(node.kind == CppAstKind::simple_declaration) {
    validate_typedef_reference_rules(node);
    validate_qualified_declaration_scope(node, scope_path);
    const CppAstNode * init_list = find_child(node, CppAstKind::init_declarator_list);
    if(init_list) {
      for(size_t i = 0; i < init_list->children.size(); ++i) {
        const CppAstNode * declarator = find_child(init_list->children[i], CppAstKind::declarator);
        string declared_name;
        if(declarator && declarator_declared_name(*declarator, declared_name)) {
          current_scope.nonnamespace_bindings.insert(declared_name);
        }
      }
    }
    return;
  }

  if(node.kind == CppAstKind::alias_declaration) {
    if(!node.value.empty()) {
      current_scope.nonnamespace_bindings.insert(node.value);
    }
    return;
  }

  if(node.kind == CppAstKind::function_definition) {
    const CppAstNode * declarator = find_child(node, CppAstKind::declarator);
    string declared_name;
    if(declarator && declarator_declared_name(*declarator, declared_name)) {
      current_scope.nonnamespace_bindings.insert(declared_name);
    }
  }
}

void validate_pa8_translation_unit(IRecogTokenSequence & tokens)
{
  const vector<RecogToken> raw_tokens = tokens.slice(0, tokens.size());
  CppAstParser parser(raw_tokens);
  CppAstNode unit;
  if(!parser.parse_translation_unit(unit)) {
    throw logic_error(parser.error());
  }
  if(unit.kind != CppAstKind::translation_unit) {
    throw logic_error("expected translation-unit");
  }
  vector<string> scope_path;
  map<string, NamespaceScopeState> namespace_scopes;
  validate_pa8_ast_node(unit, scope_path, namespace_scopes);
}

void collect_pa8_adapter_side_literals(IRecogTokenSequence & tokens,
                                       Program & program)
{
  const vector<RecogToken> raw_tokens = tokens.slice(0, tokens.size());
  CppAstParser parser(raw_tokens);
  CppAstNode unit;
  if(!parser.parse_translation_unit(unit)) {
    throw logic_error(parser.error());
  }
  if(unit.kind != CppAstKind::translation_unit) {
    throw logic_error("expected translation-unit");
  }
  collect_array_string_literal_entities(unit, program);
}

void collect_functions(Program & program,
                       const vector<FunctionRecord> & records)
{
  map<string, FunctionEntity *> functions_by_key;
  for(size_t i = 0; i < records.size(); ++i) {
    const string type_key = describe_type(records[i].node->semantic_type);
    const string key = records[i].unnamed_scope ?
        records[i].name + "@tu" + to_string(records[i].tu_index) + "|" + type_key :
        records[i].name + "|" + type_key;

    FunctionEntity * entity = nullptr;
    map<string, FunctionEntity *>::iterator found = functions_by_key.find(key);
    if(found == functions_by_key.end()) {
      unique_ptr<FunctionEntity> created(new FunctionEntity());
      created->key = key;
      created->name = records[i].name;
      created->order = records[i].encounter_order;
      created->type = records[i].node->semantic_type;
      created->image.alignment = 1;
      created->image.bytes.assign({'f', 'u', 'n', '\0'});
      entity = created.get();
      program.owned_functions.push_back(std::move(created));
      functions_by_key[key] = entity;
    } else {
      entity = found->second;
    }

    register_function_symbol_alias(program.function_aliases,
                                   symbol_lookup_key(callsem_symbol(*records[i].node)),
                                   records[i].tu_index,
                                   entity);
    if(records[i].node->kind == CallSemKind::function_definition) {
      if(entity->has_definition) {
        if(entity->definition_tu_index == records[i].tu_index) {
          throw logic_error("multiple function definitions");
        }
        continue;
      }
      entity->has_definition = true;
      entity->definition_tu_index = records[i].tu_index;
    }
  }
}

void collect_variables(Program & program,
                       const vector<VariableRecord> & records)
{
  map<string, bool> has_extern_decl;
  map<string, size_t> definition_count;
  for(size_t i = 0; i < records.size(); ++i) {
    if(is_void_type(records[i].node->semantic_type)) {
      throw logic_error("object of void type");
    }
    if(records[i].unnamed_scope) {
      continue;
    }
    if(records[i].node->is_extern_declaration) {
      has_extern_decl[records[i].name] = true;
    } else {
      ++definition_count[records[i].name];
    }
  }

  map<string, VariableEntity *> variables_by_key;
  for(size_t i = 0; i < records.size(); ++i) {
    const bool shared_entity =
        !records[i].unnamed_scope &&
        (has_extern_decl[records[i].name] || definition_count[records[i].name] <= 1);
    const string key = shared_entity ?
        records[i].name :
        records[i].name + "@tu" + to_string(records[i].tu_index);

    VariableEntity * entity = nullptr;
    map<string, VariableEntity *>::iterator found = variables_by_key.find(key);
    if(found == variables_by_key.end()) {
      unique_ptr<VariableEntity> created(new VariableEntity());
      created->key = key;
      created->name = records[i].name;
      created->order = records[i].encounter_order;
      created->tu_index = shared_entity ? INVALID_TU : records[i].tu_index;
      created->type = records[i].node->semantic_type;
      entity = created.get();
      variables_by_key[key] = entity;
      program.owned_variables.push_back(std::move(created));
    } else {
      entity = found->second;
    }

    register_variable_symbol_alias(program.variable_aliases,
                                   symbol_lookup_key(callsem_symbol(*records[i].node)),
                                   records[i].tu_index,
                                   entity);

    if(!records[i].node->is_extern_declaration) {
      if(entity->has_definition) {
        throw logic_error("multiple variable definitions");
      }
      entity->has_definition = true;
      entity->type = records[i].node->semantic_type;
      entity->image.alignment = type_alignment(entity->type);
      if(!records[i].node->children.empty()) {
        entity->initializer = &records[i].node->children[0];
      }
    }
  }
}

void finalize_variable_images(Program & program)
{
  for(size_t i = 0; i < program.owned_variables.size(); ++i) {
    VariableEntity & entity = *program.owned_variables[i];
    if(!entity.has_definition) {
      continue;
    }
    if(entity.initializer) {
      const InitResult init =
          build_initializer(program, entity.type, *entity.initializer, entity.tu_index);
      entity.image.bytes = init.bytes;
      entity.image.relocations = init.relocations;
      entity.reference_target = init.reference_target;
      entity.has_constant_value = init.has_constant_value;
      entity.constant_value = init.constant_value;
    } else {
      if(is_reference_type(entity.type) || type_is_const_object(entity.type)) {
        throw logic_error("type cannot be default initialized");
      }
      entity.image.bytes.assign(type_size(entity.type), '\0');
      entity.reference_target = nullptr;
      entity.has_constant_value = false;
      entity.constant_value = ConstantValue();
    }
  }
}

vector<char> build_program_image(Program & program)
{
  vector<VariableEntity *> variables;
  vector<FunctionEntity *> functions;
  for(size_t i = 0; i < program.owned_variables.size(); ++i) {
    if(program.owned_variables[i]->has_definition) {
      variables.push_back(program.owned_variables[i].get());
    }
  }
  for(size_t i = 0; i < program.owned_functions.size(); ++i) {
    functions.push_back(program.owned_functions[i].get());
  }

  sort(variables.begin(), variables.end(),
       [](VariableEntity * lhs, VariableEntity * rhs) {
         return lhs->order < rhs->order;
       });
  sort(functions.begin(), functions.end(),
       [](FunctionEntity * lhs, FunctionEntity * rhs) {
         return lhs->order < rhs->order;
       });
  sort(program.temp_order.begin(), program.temp_order.end(),
       [](TemporaryEntity * lhs, TemporaryEntity * rhs) {
         return lhs->order < rhs->order;
       });
  sort(program.string_order.begin(), program.string_order.end(),
       [](StringLiteralEntity * lhs, StringLiteralEntity * rhs) {
         return lhs->order < rhs->order;
       });

  vector<pair<size_t, ImageSymbol *> > block1;
  for(size_t i = 0; i < variables.size(); ++i) {
    block1.push_back(make_pair(variables[i]->order, &variables[i]->image));
  }
  for(size_t i = 0; i < functions.size(); ++i) {
    block1.push_back(make_pair(functions[i]->order, &functions[i]->image));
  }
  sort(block1.begin(), block1.end(),
       [](const pair<size_t, ImageSymbol *> & lhs,
          const pair<size_t, ImageSymbol *> & rhs) {
         return lhs.first < rhs.first;
       });

  vector<char> image;
  image.push_back('P');
  image.push_back('A');
  image.push_back('8');
  image.push_back('\0');

  for(size_t i = 0; i < block1.size(); ++i) {
    layout_symbol(image, *block1[i].second);
  }
  for(size_t i = 0; i < program.temp_order.size(); ++i) {
    layout_symbol(image, program.temp_order[i]->image);
  }
  for(size_t i = 0; i < program.string_order.size(); ++i) {
    layout_symbol(image, program.string_order[i]->image);
  }

  for(size_t i = 0; i < block1.size(); ++i) {
    apply_symbol_relocations(image, *block1[i].second);
  }
  for(size_t i = 0; i < program.temp_order.size(); ++i) {
    apply_symbol_relocations(image, program.temp_order[i]->image);
  }
  for(size_t i = 0; i < program.string_order.size(); ++i) {
    apply_symbol_relocations(image, program.string_order[i]->image);
  }

  return image;
}

}  // namespace

NSTranslationUnitInput::NSTranslationUnitInput()
  : source_locations(new SourceLocationTable())
{}

NSTranslationUnitInput::NSTranslationUnitInput(const string & source_path,
                                               time_t now) :
  source_path(source_path),
  source_locations(new SourceLocationTable())
{
  preprocessor.reset(new Preprocessor(source_path, now));
  posttokenizer.reset(new PostTokenizer(*preprocessor, source_locations.get(), preprocessor.get()));
  tokenizer.reset(new RecogTokenizer(*posttokenizer));
  token_buffer.reset(new RecogTokenBuffer(*tokenizer, source_path, source_locations.get()));
}

IRecogTokenSequence & NSTranslationUnitInput::token_sequence() const
{
  return *token_buffer;
}

vector<char> build_nsinit_program_image(
    const vector<unique_ptr<NSTranslationUnitInput>> & translation_units)
{
  Program program;
  vector<CallSemNode> outputs;
  outputs.reserve(translation_units.size());
  vector<VariableRecord> variable_records;
  vector<FunctionRecord> function_records;
  set<string> seen_named_namespaces;

  for(size_t i = 0; i < translation_units.size(); ++i) {
    size_t encounter_order = 0;
    if(i != 0) {
      encounter_order = variable_records.size() + function_records.size();
    }
    validate_pa8_translation_unit(translation_units[i]->token_sequence());
    collect_pa8_adapter_side_literals(translation_units[i]->token_sequence(), program);
    outputs.push_back(
        analyze_calls_translation_unit(translation_units[i]->token_sequence(),
                                       false,
                                       true));
    collect_records(outputs.back(),
                    encounter_order,
                    i,
                    false,
                    string(),
                    seen_named_namespaces,
                    variable_records,
                    function_records);
  }

  collect_functions(program, function_records);
  collect_variables(program, variable_records);
  finalize_variable_images(program);
  return build_program_image(program);
}
