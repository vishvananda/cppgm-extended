#pragma once

#include <algorithm>
#include <functional>
#include <map>
#include <memory>
#include <set>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include "callsem_output.h"
#include "constant_value.h"
#include "cpp_decl_model.h"
#include "cppast_ast.h"
#include "symbol_linkage.h"
#include "template_model.h"

namespace semantic_model {

struct ClassInfo;
// Index of instantiated classes keyed by their mangled name. Used only for
// keyed lookup (never iterated for output), so it is hashed rather than ordered.
typedef std::unordered_map<std::string, ClassInfo *> ClassIndexMap;
struct FunctionBinding;
struct FunctionTemplateDecl;
struct VariableTemplateDecl;
struct AliasTemplateDecl;
struct ClassTemplateDecl;
struct DeductionGuideDecl;

struct FunctionTemplateInstantiationCacheEntry
{
  FunctionTemplateDecl * decl = nullptr;
  std::string key;
};

struct FunctionTemplateInstantiationCacheEntries
{
  FunctionTemplateInstantiationCacheEntry first;
  std::vector<FunctionTemplateInstantiationCacheEntry> extra;
  std::unique_ptr<std::vector<template_model::TemplateValueDependency> >
      signature_value_dependencies;
};
struct ClassTemplateSpecializationDecl;
struct PartialClassTemplateSpecializationDecl;
struct VariableTemplateSpecializationDecl;
struct Scope;

struct UsingDirectiveEntry
{
  UsingDirectiveEntry() = default;

  UsingDirectiveEntry(Scope * target,
                      std::size_t first_token_start = 0)
    : target(target),
      first_token_start(first_token_start)
  {}

  operator Scope *() const
  {
    return target;
  }

  Scope * target = nullptr;
  std::size_t first_token_start = 0;
};

// Index of durable type scopes keyed by their mangled name. Used only for keyed
// lookup (never iterated for output), so it is hashed rather than ordered.
typedef std::unordered_map<std::string, Scope *> TypeScopeIndexMap;

enum ExplicitFunctionNothrowKind
{
  EFNK_UNINITIALIZED,
  EFNK_NONE,
  EFNK_ALWAYS_TRUE,
  EFNK_ALWAYS_FALSE,
  EFNK_EXPR,
  EFNK_INVALID
};

enum BuiltinConstantEvaluationKind
{
  BCEK_NONE,
  BCEK_EXPECT
};

std::size_t next_scope_instance_id();
std::size_t next_class_instance_id();

enum MemberAccess : int
{
  MA_PUBLIC,
  MA_PROTECTED,
  MA_PRIVATE
};

enum RefQualifier : int
{
  RQ_NONE,
  RQ_LVALUE,
  RQ_RVALUE
};

struct FunctionSemanticFlags
{
  MemberAccess access = MA_PUBLIC;
  bool is_constructor = false;
  bool is_inherited_constructor = false;
  bool is_destructor = false;
  bool is_conversion_operator = false;
  bool is_explicit = false;
  bool is_const_method = false;
  bool is_volatile_method = false;
  bool is_variadic = false;
  RefQualifier ref_qualifier = RQ_NONE;
  bool is_virtual_specified = false;
  bool is_override_specified = false;
  bool is_final = false;
  bool is_defaulted = false;
  bool is_deleted = false;
  bool is_constexpr = false;
  bool is_inline = false;
  const CppAstNode * function_qualifier = nullptr;
};

enum OutputRequirementKind : unsigned int
{
  ORK_NONE = 0,
  ORK_DECLARATION = 1u << 0,
  ORK_DEFINITION = 1u << 1,
  ORK_EXPORT = 1u << 2,
  ORK_RUNTIME = 1u << 3
};

inline bool has_output_requirement(unsigned int flags, OutputRequirementKind kind)
{
  return (flags & static_cast<unsigned int>(kind)) != 0;
}

inline void add_output_requirement(unsigned int & flags, OutputRequirementKind kind)
{
  flags |= static_cast<unsigned int>(kind);
}

inline void remove_output_requirement(unsigned int & flags, OutputRequirementKind kind)
{
  flags &= ~static_cast<unsigned int>(kind);
}

struct SourceDeclAnchorCache
{
  mutable bool cached = false;
  mutable std::string name_location;
  mutable std::string approximate_location;
};

inline const std::string & source_decl_anchor_location(
    const SourceDeclAnchorCache & cache)
{
  return !cache.name_location.empty() ? cache.name_location :
                                        cache.approximate_location;
}

inline bool source_decl_anchor_has_name_location(
    const SourceDeclAnchorCache & cache)
{
  return !cache.name_location.empty();
}

struct VariableTemplateInstantiationIdentity
{
  VariableTemplateDecl * source_template = nullptr;
  std::vector<template_model::TemplateArgument> arguments;
};

struct ValueBinding
{
  enum Kind
  {
    VK_VARIABLE,
    VK_PARAMETER,
    VK_FIELD
  };

  enum TemplateWitnessSourceValueDependency : unsigned char
  {
    TWVD_UNKNOWN,
    TWVD_FIXED,
    TWVD_DEPENDENT
  };

  ValueBinding() : kind(VK_VARIABLE) {}
  ValueBinding(Kind kind, const std::string & name, const cpp_decl::TypePtr & type)
    : kind(kind), name(name), type(type)
  {}

  Kind kind;
  std::string name;
  cpp_decl::TypePtr type;
  ClassInfo * owner_class = nullptr;
  std::size_t field_offset = 0;
  std::string anonymous_storage_field_name;
  std::string anonymous_storage_variable_name;
  std::size_t anonymous_storage_member_offset = 0;
  bool is_mutable = false;
  bool is_bit_field = false;
  std::size_t bit_field_width = 0;
  std::size_t bit_field_offset = 0;
  std::size_t bit_field_storage_size = 0;
  MemberAccess access = MA_PUBLIC;
  bool is_c_linkage = false;
  bool is_thread_local = false;
  bool has_storage_definition = true;
  bool is_explicit_specialization = false;
  bool has_constant_value = false;
  long long constant_value = 0;
  bool has_constexpr_value = false;
  std::shared_ptr<constant_eval::ConstexprValue> constexpr_value;
  bool dependent_template_value = false;
  std::string non_type_template_argument_text;
  std::shared_ptr<CppAstNode> non_type_template_argument_expression;
  FunctionBinding * non_type_template_function_value = nullptr;
  std::string non_type_template_function_internal_symbol;
  const ValueBinding * non_type_template_value_binding = nullptr;
  std::shared_ptr<VariableTemplateInstantiationIdentity> variable_template_instantiation;
  Scope * declaration_scope = nullptr;
  const CppAstNode * declaration_node = nullptr;
  const CppAstNode * definition_node = nullptr;
  const CppAstNode * constant_initializer = nullptr;
  Scope * constant_initializer_scope = nullptr;
  bool constant_value_in_progress = false;
  bool requires_constant_initializer = false;
  symbol_linkage::SymbolIdentity symbol;
  unsigned int output_requirements = ORK_NONE;
  bool definition_output_emitted = false;
  mutable unsigned char template_witness_state_flags = 0;
  mutable TemplateWitnessSourceValueDependency
      template_witness_source_value_dependency = TWVD_UNKNOWN;
  mutable SourceDeclAnchorCache declaration_anchor;
};

inline bool value_binding_has_constexpr_value(const ValueBinding & binding)
{
  return binding.has_constexpr_value && binding.constexpr_value;
}

inline const constant_eval::ConstexprValue & value_binding_constexpr_value(
    const ValueBinding & binding)
{
  return *binding.constexpr_value;
}

inline void set_value_binding_constexpr_value(
    ValueBinding & binding,
    const constant_eval::ConstexprValue & value)
{
  binding.has_constexpr_value = true;
  binding.constexpr_value.reset(new constant_eval::ConstexprValue(value));
}

inline void clear_value_binding_constexpr_value(ValueBinding & binding)
{
  binding.has_constexpr_value = false;
  binding.constexpr_value.reset();
}

template<typename Key,
         typename Value,
         typename Compare = std::less<Key> >
class LazyMap
{
public:
  typedef std::map<Key, Value, Compare> Map;
  typedef typename Map::key_type key_type;
  typedef typename Map::mapped_type mapped_type;
  typedef typename Map::value_type value_type;
  typedef typename Map::size_type size_type;
  typedef typename Map::iterator iterator;
  typedef typename Map::const_iterator const_iterator;

  LazyMap() = default;

  LazyMap(const LazyMap & other)
      : values_(other.values_ ? new Map(*other.values_) : nullptr)
  {
  }

  LazyMap(LazyMap && other) noexcept = default;

  LazyMap & operator=(const LazyMap & other)
  {
    if(this != &other) {
      values_.reset(other.values_ ? new Map(*other.values_) : nullptr);
    }
    return *this;
  }

  LazyMap & operator=(LazyMap && other) noexcept = default;

  mapped_type & operator[](const key_type & key)
  {
    return mutable_values()[key];
  }

  mapped_type & operator[](key_type && key)
  {
    return mutable_values()[std::move(key)];
  }

  iterator begin()
  {
    return values_ ? values_->begin() : empty_map().begin();
  }

  const_iterator begin() const
  {
    return values_ ? values_->begin() : empty_map().begin();
  }

  iterator end()
  {
    return values_ ? values_->end() : empty_map().end();
  }

  const_iterator end() const
  {
    return values_ ? values_->end() : empty_map().end();
  }

  iterator find(const key_type & key)
  {
    return values_ ? values_->find(key) : empty_map().end();
  }

  const_iterator find(const key_type & key) const
  {
    return values_ ? values_->find(key) : empty_map().end();
  }

  size_type count(const key_type & key) const
  {
    return values_ ? values_->count(key) : 0;
  }

  bool empty() const
  {
    return !values_ || values_->empty();
  }

  size_type size() const
  {
    return values_ ? values_->size() : 0;
  }

  std::pair<iterator, bool> insert(const value_type & value)
  {
    return mutable_values().insert(value);
  }

  std::pair<iterator, bool> insert(value_type && value)
  {
    return mutable_values().insert(std::move(value));
  }

  template<typename InputIterator>
  void insert(InputIterator first, InputIterator last)
  {
    if(first != last) {
      mutable_values().insert(first, last);
    }
  }

  template<typename... Args>
  std::pair<iterator, bool> emplace(Args &&... args)
  {
    return mutable_values().emplace(std::forward<Args>(args)...);
  }

  size_type erase(const key_type & key)
  {
    if(!values_) {
      return 0;
    }
    const size_type erased = values_->erase(key);
    reset_if_empty();
    return erased;
  }

  iterator erase(iterator position)
  {
    if(!values_) {
      return empty_map().end();
    }
    iterator next = values_->erase(position);
    if(values_->empty()) {
      values_.reset();
      return empty_map().end();
    }
    return next;
  }

  void clear()
  {
    values_.reset();
  }

  void swap(Map & other)
  {
    if(!values_) {
      if(other.empty()) {
        return;
      }
      values_.reset(new Map);
    }
    values_->swap(other);
    reset_if_empty();
  }

  const Map & get() const
  {
    return values_ ? *values_ : empty_map();
  }

  operator const Map &() const
  {
    return get();
  }

private:
  static Map & empty_map()
  {
    static Map empty;
    return empty;
  }

  Map & mutable_values()
  {
    if(!values_) {
      values_.reset(new Map);
    }
    return *values_;
  }

  void reset_if_empty()
  {
    if(values_ && values_->empty()) {
      values_.reset();
    }
  }

  std::unique_ptr<Map> values_;
};

struct Scope
{
  explicit Scope(Scope * parent = nullptr,
                 const std::string & name = std::string(),
                 bool namespace_scope = false)
    : parent(parent),
      name(name),
      namespace_scope(namespace_scope),
      instance_id(next_scope_instance_id())
  {}

  Scope(const Scope & other);

  Scope & operator=(const Scope & other)
  {
    if(this == &other) {
      return *this;
    }

    parent = other.parent;
    name = other.name;
    namespace_scope = other.namespace_scope;
    inline_namespace = other.inline_namespace;
    persistent_lifetime = other.persistent_lifetime;
    class_info = other.class_info;
    function = other.function;
    named_types = other.named_types;
    named_type_access = other.named_type_access;
    named_type_packs = other.named_type_packs;
    named_value_packs = other.named_value_packs;
    named_pack_sizes = other.named_pack_sizes;
    template_bound_type_names = other.template_bound_type_names;
    template_bound_type_pack_names = other.template_bound_type_pack_names;
    template_bound_value_names = other.template_bound_value_names;
    template_bound_value_pack_names = other.template_bound_value_pack_names;
    template_bound_template_names = other.template_bound_template_names;
    template_bound_template_arguments = other.template_bound_template_arguments;
    values = other.values;
    namespace_bindings = other.namespace_bindings;
    if(other.namespace_binding_first_token_starts) {
      namespace_binding_first_token_starts.reset(
          new std::map<std::string, std::size_t>(
              *other.namespace_binding_first_token_starts));
    } else {
      namespace_binding_first_token_starts.reset();
    }
    function_sets = other.function_sets;
    if(other.function_binding_first_token_starts) {
      function_binding_first_token_starts.reset(
          new std::map<std::string,
                       std::map<const FunctionBinding *, std::size_t> >(
              *other.function_binding_first_token_starts));
    } else {
      function_binding_first_token_starts.reset();
    }
    function_set_access_overrides = other.function_set_access_overrides;
    cached_direct_function_lookups.clear();
    direct_function_lookup_cache_epoch = other.direct_function_lookup_cache_epoch;
    class_templates = other.class_templates;
    function_templates = other.function_templates;
    if(other.function_template_introduction_nodes) {
      function_template_introduction_nodes.reset(
          new std::map<
              std::string,
              std::map<const FunctionTemplateDecl *, const CppAstNode *> >(
                  *other.function_template_introduction_nodes));
    } else {
      function_template_introduction_nodes.reset();
    }
    collected_template_declarations = other.collected_template_declarations;
    alias_templates = other.alias_templates;
    variable_templates = other.variable_templates;
    using_directives = other.using_directives;
    namespace_children.clear();
    instance_id = next_scope_instance_id();
    binding_fingerprint_epoch = other.binding_fingerprint_epoch;
    cached_binding_scope_fingerprint_valid = false;
    cached_binding_scope_fingerprint = 0;
    cached_binding_scope_fingerprint_epoch = 0;
    cached_binding_scope_parent_fingerprint = 0;
    cached_binding_scope_global_epoch = 0;
    cached_instance_scope_fingerprint_valid = false;
    cached_instance_scope_fingerprint = 0;
    return *this;
  }

  Scope(Scope && other);
  Scope & operator=(Scope && other);

  Scope * parent;
  std::string name;
  bool namespace_scope;
  bool inline_namespace = false;
  bool persistent_lifetime = false;
  ClassInfo * class_info = nullptr;
  FunctionBinding * function = nullptr;
  // Point-lookup keyed by type name; iteration order is not relied upon.
  typedef std::unordered_map<std::string, cpp_decl::TypePtr> NamedTypeMap;
  NamedTypeMap named_types;
  std::map<std::string, MemberAccess> named_type_access;
  LazyMap<std::string, std::vector<cpp_decl::TypePtr> > named_type_packs;
  LazyMap<std::string, std::vector<ValueBinding> > named_value_packs;
  LazyMap<std::string, std::size_t> named_pack_sizes;
  std::set<std::string> template_bound_type_names;
  std::set<std::string> template_bound_type_pack_names;
  std::set<std::string> template_bound_value_names;
  std::set<std::string> template_bound_value_pack_names;
  std::set<std::string> template_bound_template_names;
  LazyMap<std::string, template_model::TemplateArgument>
      template_bound_template_arguments;
  std::map<std::string, ValueBinding> values;
  LazyMap<std::string, Scope *> namespace_bindings;
  // Namespace collection is intentionally eager so later declarations are
  // available when deferred class bodies are completed.  Keep the first token
  // at which each spelling became visible so source-point qualified lookup can
  // avoid being shadowed by a namespace declared later in the translation
  // unit.
  std::unique_ptr<std::map<std::string, std::size_t> >
      namespace_binding_first_token_starts;
  std::map<std::string, std::vector<FunctionBinding *> > function_sets;
  // Namespace collection is eager, including using-declarations that occur
  // after deferred bodies.  Imported bindings retain the point at which each
  // using-declaration made them visible so ADL can honor point-of-declaration.
  std::unique_ptr<
      std::map<std::string,
               std::map<const FunctionBinding *, std::size_t> > >
      function_binding_first_token_starts;
  LazyMap<std::string, std::map<const FunctionBinding *, MemberAccess> >
      function_set_access_overrides;
  LazyMap<std::string, ClassTemplateDecl *> class_templates;
  LazyMap<std::string, std::vector<FunctionTemplateDecl *> > function_templates;
  // Imported function templates retain the using-declaration source point.
  // Deferred bodies can carry cloned token offsets that are not comparable
  // with offsets from the imported declaration's source file.
  std::unique_ptr<
      std::map<std::string,
               std::map<const FunctionTemplateDecl *, const CppAstNode *> > >
      function_template_introduction_nodes;
  std::set<const CppAstNode *> collected_template_declarations;
  LazyMap<std::string, AliasTemplateDecl *> alias_templates;
  LazyMap<std::string, VariableTemplateDecl *> variable_templates;
  // Namespace collection is eager so declarations remain available to lazy
  // class completion. Retain when each using-directive first became visible
  // alongside its target so scopes without directives pay no extra storage.
  std::vector<UsingDirectiveEntry> using_directives;
  std::vector<std::unique_ptr<Scope> > namespace_children;
  std::size_t instance_id;
  std::size_t binding_fingerprint_epoch = 0;
  struct DirectFunctionLookupCacheEntry
  {
    std::size_t dependency_token = 0;
    std::vector<FunctionBinding *> functions;
  };
  mutable LazyMap<std::string, DirectFunctionLookupCacheEntry>
      cached_direct_function_lookups;
  std::size_t direct_function_lookup_cache_epoch = 0;
  mutable bool cached_binding_scope_fingerprint_valid = false;
  mutable std::size_t cached_binding_scope_fingerprint = 0;
  mutable std::size_t cached_binding_scope_fingerprint_epoch = 0;
  mutable std::size_t cached_binding_scope_parent_fingerprint = 0;
  mutable std::size_t cached_binding_scope_global_epoch = 0;
  mutable bool cached_instance_scope_fingerprint_valid = false;
  mutable std::size_t cached_instance_scope_fingerprint = 0;
};

struct FunctionBinding
{
  std::string name;
  std::string display_name;
  cpp_decl::TypePtr declared_type;
  cpp_decl::TypePtr type;
  Scope * declaration_scope = nullptr;
  std::vector<std::pair<std::string, cpp_decl::TypePtr> > params;
  // Function signatures discard top-level cv-qualification from by-value
  // parameters.  Retain the corresponding parameter object types for uses
  // inside the function body, where that qualification remains observable.
  std::vector<cpp_decl::TypePtr> parameter_object_types;
  std::vector<std::string> parameter_aliases;
  std::vector<const CppAstNode *> default_arguments;
  const CppAstNode * declaration_node = nullptr;
  const CppAstNode * definition_node = nullptr;
  std::vector<std::string> declaration_abi_tags;
  std::vector<std::string> definition_abi_tags;
  bool definition_suppresses_declaration_abi_tags = false;
  const CppAstNode * parameter_syntax_node = nullptr;
  const CppAstNode * body = nullptr;
  const CppAstNode * function_qualifier = nullptr;
  bool has_definition = false;
  ClassInfo * owner_class = nullptr;
  ClassInfo * lexical_access_class = nullptr;
  FunctionBinding * lexical_access_function = nullptr;
  bool hidden_friend_only = false;
  MemberAccess access = MA_PUBLIC;
  bool is_method = false;
  bool is_constexpr = false;
  bool is_inline = false;
  bool is_force_inline = false;
  bool is_constructor = false;
  bool is_inherited_constructor = false;
  ClassInfo * inherited_constructor_access_class = nullptr;
  bool is_destructor = false;
  bool is_conversion_operator = false;
  bool is_defaulted = false;
  bool is_explicit = false;
  bool is_copy_constructor = false;
  bool is_move_constructor = false;
  bool is_copy_assignment = false;
  bool is_move_assignment = false;
  bool is_const_method = false;
  bool is_volatile_method = false;
  RefQualifier ref_qualifier = RQ_NONE;
  bool is_deleted = false;
  // Once the owning class is complete, the deleted state of an explicitly
  // defaulted special member is structural and cannot change.  Remember that
  // finalization so ordinary overload lookup does not re-walk the same base
  // and field graph.
  bool defaulted_deletion_state_finalized = false;
  bool is_virtual = false;
  bool is_virtual_specified = false;
  bool is_pure_virtual = false;
  bool is_override_specified = false;
  bool is_final = false;
  bool is_c_linkage = false;
  bool is_builtin = false;
  BuiltinConstantEvaluationKind builtin_constant_evaluation_kind = BCEK_NONE;
  bool permits_host_builtin_missing_nothrow_redeclaration = false;
  std::string object_symbol_override;
  symbol_linkage::SymbolIdentity symbol;
  mutable bool cached_lookup_dedupe_key_valid = false;
  mutable std::size_t cached_lookup_dedupe_name_hash = 0;
  mutable std::size_t cached_lookup_dedupe_type_hash = 0;
  mutable const cpp_decl::Type * cached_lookup_dedupe_type = nullptr;
  mutable std::size_t cached_lookup_dedupe_name_size = 0;
  bool has_virtual_slot = false;
  std::size_t virtual_slot = 0;
  bool synthesized = false;
  bool is_aggregate_constructor = false;
  unsigned int output_requirements = ORK_NONE;
  bool output_emitted = false;
  bool definition_output_in_progress = false;
  bool definition_output_emitted = false;
  bool odr_mergeable_definition = false;
  bool template_definition_materialized_by_enclosing_closure = false;
  bool template_definition_required_by_public_source_call = false;
  ExplicitFunctionNothrowKind explicit_function_nothrow_kind = EFNK_UNINITIALIZED;
  const CppAstNode * explicit_function_nothrow_cached_qualifier = nullptr;
  std::string explicit_function_nothrow_expr_text;
  bool explicit_function_nothrow_eval_cached = false;
  bool explicit_function_nothrow_eval_value = false;
  bool suppress_implicit_instantiation_definition = false;
  bool is_explicit_instantiation_definition = false;
  bool exclude_from_explicit_instantiation = false;
  std::unique_ptr<CallSemNode> cached_body_output;
  const CppAstNode * ctor_initializer = nullptr;
  FunctionBinding * delegating_constructor_target = nullptr;
  FunctionTemplateDecl * source_template = nullptr;
  std::vector<template_model::TemplateArgument> instantiation_arguments;
  bool has_instantiation_arguments = false;
  std::string instantiation_use_location;
  bool is_explicit_specialization = false;
  std::map<std::string, std::size_t> instantiation_pack_sizes;
  std::string template_instantiation_key;
  // A concrete function-template specialization has one immutable signature.
  // Signature-only overload probes may reuse it without rebuilding the
  // template argument scope once initial substitution and validation finish.
  bool instantiated_signature_finalized = false;
  std::unique_ptr<FunctionTemplateInstantiationCacheEntries>
      instantiation_cache_entries;
  mutable SourceDeclAnchorCache declaration_anchor;
};

inline std::string function_output_name(const FunctionBinding & binding);
std::string function_binding_display_name_for_symbol(const FunctionBinding & binding);
std::string predefined_pretty_function_text(const FunctionBinding & binding);
std::string function_binding_qualified_name_for_symbol(const FunctionBinding & binding);
bool function_binding_qualified_name_syntax_for_symbol(
    const FunctionBinding & binding,
    cpp_decl::QualifiedName & out);
bool function_binding_is_standard_library_builtin(
    const FunctionBinding & binding);

inline std::vector<std::string> function_binding_abi_tags(const FunctionBinding & binding);

struct FieldInfo
{
  std::string name;
  cpp_decl::TypePtr type;
  const CppAstNode * alignment_declaration = nullptr;
  const CppAstNode * default_initializer = nullptr;
  const CppAstNode * bit_width_expression = nullptr;
  std::size_t offset = 0;
  bool is_mutable = false;
  bool is_no_unique_address = false;
  bool is_anonymous_storage = false;
  bool is_bit_field = false;
  std::size_t bit_width = 0;
  std::size_t bit_offset = 0;
  std::size_t bit_storage_size = 0;
  MemberAccess access = MA_PUBLIC;
};

struct BaseInfo
{
  ClassInfo * type = nullptr;
  MemberAccess access = MA_PUBLIC;
  std::size_t offset = 0;
  bool is_virtual = false;
  bool source_dependent = false;
};

struct AnonymousMemberClassInfo
{
  std::string class_kind;
  const CppAstNode * class_node = nullptr;
};

struct SubobjectInfo
{
  ClassInfo * type = nullptr;
  std::size_t offset = 0;
  MemberAccess access = MA_PUBLIC;
  bool is_virtual = false;
};

struct VTableSlotInfo
{
  FunctionBinding * function = nullptr;
  // The virtual declaration that owns the slot's callable contract.  The
  // final overrider can have a covariant result, so retaining only `function`
  // loses the result type promised to callers through this slot.
  FunctionBinding * contract_function = nullptr;
  long long this_adjust = 0;
};

struct VTableInfo
{
  std::string key;
  ClassInfo * view_type = nullptr;
  std::size_t view_offset = 0;
  bool use_extended_layout = false;
  std::vector<VTableSlotInfo> slots;
};

struct ClassInfo
{
  ClassInfo()
      : semantic_identity_id(next_class_instance_id()),
        instantiation_arguments_storage(
            new std::vector<template_model::TemplateArgument>()),
        instantiation_arguments(*instantiation_arguments_storage)
  {
  }

  // Process-local, collision-free identity for semantic graph edges.  Textual
  // names remain available for diagnostics and ABI rendering, but must not be
  // recursively flattened merely to identify an already materialized class.
  std::size_t semantic_identity_id;
  std::string name;
  std::string qualified_name;
  cpp_decl::QualifiedName symbol_qualified_name_syntax;
  std::string display_qualified_name;
  bool has_display_qualified_name = false;
  std::string class_kind;
  std::string creation_context;
  cpp_decl::TypePtr type;
  Scope * enclosing_scope = nullptr;
  const CppAstNode * class_node = nullptr;
  const CppAstNode * template_output_node = nullptr;
  std::unique_ptr<Scope> member_scope;
  std::vector<FieldInfo> fields;
  std::map<std::string, std::vector<FunctionBinding *> > methods;
  std::vector<FunctionBinding *> method_declaration_order;
  std::vector<BaseInfo> bases;
  std::vector<AnonymousMemberClassInfo> anonymous_member_classes;
  std::vector<FunctionBinding *> vtable_entries;
  // Parallel to vtable_entries.  An override replaces the entry but preserves
  // the original typed slot contract for covariant-result lowering.
  std::vector<FunctionBinding *> vtable_entry_contracts;
  std::vector<SubobjectInfo> complete_subobjects;
  std::vector<SubobjectInfo> virtual_base_subobjects;
  std::vector<VTableInfo> vtables;
  std::vector<FunctionBinding *> friend_functions;
  std::vector<FunctionBinding *> friend_access_functions;
  std::vector<FunctionTemplateDecl *> friend_function_templates;
  std::vector<FunctionTemplateDecl *> friend_access_function_templates;
  std::vector<std::string> friend_class_names;
  struct DeferredMemberAlias
  {
    const CppAstNode * type_id = nullptr;
    const CppAstNode * typedef_specifiers = nullptr;
    const CppAstNode * typedef_declarators = nullptr;
    const CppAstNode * typedef_declarator = nullptr;
    const CppAstNode * declaration = nullptr;
    std::string type_id_text;
    bool dependent_class = false;
    bool resolving = false;
  };
  std::map<std::string, DeferredMemberAlias> deferred_member_aliases;
  bool is_polymorphic = false;
  bool rtti_required = false;
  bool is_final = false;
  bool is_initializer_list = false;
  cpp_decl::TypePtr initializer_list_element_type;
  bool is_lambda_closure = false;
  bool template_instantiation_tracked_from_source_capture_header = false;
  FunctionBinding * captureless_lambda_conversion_target = nullptr;
  bool source_is_unnamed_class = false;
  const CppAstNode * source_unnamed_class_node = nullptr;
  bool source_is_named_function_local_class = false;
  bool dependent_instantiation = false;
  bool template_instantiation_tracked = false;
  bool template_instantiation_in_progress = false;
  bool full_member_collection_in_progress = false;
  bool reference_member_collection_in_progress = false;
  bool reference_type_member_collection_in_progress = false;
  bool reference_type_members_collected = false;
  std::set<std::string> reference_named_members_collected;
  std::set<std::string> reference_named_members_in_progress;
  struct TypedefMemberDeclarationSite
  {
    enum SourceTemplateTypeDependency : unsigned char
    {
      STTD_UNKNOWN,
      STTD_FIXED,
      STTD_DEPENDENT
    };

    uint32_t source_location_id = 0;
    SourceTemplateTypeDependency source_template_type_dependency =
        STTD_UNKNOWN;
    std::size_t token_start = 0;
    std::size_t token_end = 0;
  };
  std::map<std::string, std::vector<TypedefMemberDeclarationSite> >
      typedef_member_declaration_sites;
  std::set<const CppAstNode *> reference_named_member_declarations_collected;
  bool reference_members_collected = false;
  bool implicit_special_members_ensured = false;
  // Host ABI classification is structural and immutable once a concrete
  // class is complete.  Retain the result on the semantic class so parents do
  // not recursively re-walk the same base/field graph.
  bool host_abi_implicit_copy_allowed_known = false;
  bool host_abi_implicit_copy_allowed = false;
  bool host_abi_trivially_copy_constructible_known = false;
  bool host_abi_trivially_copy_constructible = false;
  bool host_abi_trivially_destructible_known = false;
  bool host_abi_trivially_destructible = false;
  bool out_of_class_member_function_template_definitions_applied = false;
  bool out_of_class_member_function_definitions_applied = false;
  bool out_of_class_special_member_definitions_applied = false;
  bool out_of_class_static_member_definitions_applied = false;
  bool definition_output_in_progress = false;
  bool definition_output_emitted = false;
  bool has_late_required_static_member_output = false;
  bool late_required_static_member_output_queued = false;
  bool has_late_required_class_method_output = false;
  bool has_late_required_class_static_function_output = false;
  bool vtable_output_in_progress = false;
  bool vtable_output_emitted = false;
  bool has_own_vptr = false;
  bool is_explicit_specialization = false;
  bool suppress_implicit_instantiation_definition = false;
  std::size_t nonvirtual_size = 0;
  std::size_t nonvirtual_alignment = 1;
  bool complete = false;
  // A concrete specialization can be structurally complete before a
  // recursively referenced concrete type has a layout. A later explicit
  // completion may rematerialize only specializations with this provenance.
  bool concrete_layout_deferred = false;
  MemberAccess default_access = MA_PRIVATE;
  ClassTemplateDecl * source_template = nullptr;
  std::string instantiation_key;
  const std::string * instantiation_key_view = nullptr;
  std::size_t instantiation_specialization_epoch = 0;
  std::vector<std::string> instantiation_arg_texts;
  std::shared_ptr<std::vector<template_model::TemplateArgument> >
      instantiation_arguments_storage;
  std::vector<template_model::TemplateArgument> & instantiation_arguments;
  std::vector<template_model::TemplateArgument> instantiation_binding_arguments;
  std::map<std::string, std::size_t> instantiation_binding_pack_sizes;
  bool has_instantiation_binding_arguments = false;
  const std::vector<template_model::TemplateArgument> *
      instantiation_binding_arguments_view = nullptr;
  std::vector<template_model::TemplateValueDependency> template_value_dependencies;
  std::string first_qualifier_use_location;
  bool reentrant_primary_selection = false;
  mutable SourceDeclAnchorCache declaration_anchor;
};

inline std::string class_instantiation_output_local_name(
    const ClassInfo & info)
{
  if(!info.source_template) {
    return std::string();
  }
  const bool have_canonical_texts =
      info.instantiation_arg_texts.size() ==
      info.instantiation_arguments.size();
  if(!have_canonical_texts && info.instantiation_arguments.empty()) {
    return std::string();
  }
  std::string out = info.name + "<";
  for(std::size_t i = 0; i < info.instantiation_arguments.size(); ++i) {
    if(i != 0) {
      out += ", ";
    }
    const std::string & text =
        have_canonical_texts ?
            info.instantiation_arg_texts[i] :
            info.instantiation_arguments[i].text;
    if(text.empty()) {
      return std::string();
    }
    out += text;
  }
  out += ">";
  return out;
}

inline std::size_t last_top_level_qualified_name_separator(
    const std::string & text)
{
  std::size_t depth = 0;
  std::size_t split = std::string::npos;
  for(std::size_t i = 0; i + 1 < text.size(); ++i) {
    if(text[i] == '<') {
      ++depth;
    } else if(text[i] == '>' && depth != 0) {
      --depth;
    } else if(text[i] == ':' &&
              text[i + 1] == ':' &&
              depth == 0) {
      split = i;
      ++i;
    }
  }
  return split;
}

inline std::string class_output_qualified_name(const ClassInfo & info)
{
  if(!info.has_display_qualified_name) {
    return info.qualified_name;
  }
  if(!info.display_qualified_name.empty()) {
    return info.display_qualified_name;
  }
  cpp_decl::TypePtr type = cpp_decl::strip_top_level_cv(info.type);
  if(type && type->kind == cpp_decl::Type::TK_NAMED) {
    const std::string prefix =
        info.class_kind.empty() ? std::string() : info.class_kind + " ";
    const std::string display = cpp_decl::named_type_display_text(type);
    if(!prefix.empty() &&
       display.compare(0, prefix.size(), prefix) == 0) {
      return display.substr(prefix.size());
    }
    if(!display.empty()) {
      return display;
    }
  }
  return info.qualified_name;
}

inline cpp_decl::QualifiedName
class_output_qualified_name_syntax(const ClassInfo & info)
{
  cpp_decl::QualifiedName out;
  cpp_decl::TypePtr type = cpp_decl::strip_top_level_cv(info.type);
  if(info.is_lambda_closure &&
     (!type ||
      type->kind != cpp_decl::Type::TK_NAMED ||
      !type->named_rare().named_member_owner_type) &&
     !info.symbol_qualified_name_syntax.name.empty()) {
    return info.symbol_qualified_name_syntax;
  }
  if(type &&
     type->kind == cpp_decl::Type::TK_NAMED &&
     !info.is_lambda_closure) {
    out.rooted = type->named_qualified_name_syntax().rooted;
    cpp_decl::TypePtr owner_type =
        cpp_decl::strip_top_level_cv(
            type->named_rare().named_member_owner_type);
    ClassInfo * owner =
        owner_type && owner_type->kind == cpp_decl::Type::TK_NAMED ?
            owner_type->named_rare().named_class_info :
            nullptr;
    if(owner && owner != &info) {
      out = class_output_qualified_name_syntax(*owner);
      if(!out.name.empty()) {
        out.qualifiers.push_back(out.name);
        const std::string instantiation_local =
            class_instantiation_output_local_name(info);
        out.name =
            !instantiation_local.empty() ?
                instantiation_local :
                (!info.symbol_qualified_name_syntax.name.empty() ?
                     info.symbol_qualified_name_syntax.name :
                     info.name);
        return out;
      }
    }
  }

  if(!info.source_template &&
     !info.is_lambda_closure &&
     !info.symbol_qualified_name_syntax.name.empty()) {
    return info.symbol_qualified_name_syntax;
  }

  const std::string text = class_output_qualified_name(info);
  std::size_t component_start = 0;
  std::size_t angle_depth = 0;
  for(std::size_t i = 0; i + 1 < text.size(); ++i) {
    if(text[i] == '<') {
      ++angle_depth;
    } else if(text[i] == '>' && angle_depth != 0) {
      --angle_depth;
    } else if(text[i] == ':' &&
              text[i + 1] == ':' &&
              angle_depth == 0) {
      if(i != component_start) {
        out.qualifiers.push_back(
            text.substr(component_start, i - component_start));
      } else if(component_start == 0) {
        out.rooted = true;
      }
      component_start = i + 2;
      ++i;
    }
  }
  out.name = text.substr(component_start);
  if(out.name.empty() && !out.qualifiers.empty()) {
    out.name = out.qualifiers.back();
    out.qualifiers.pop_back();
  }
  if(type && type->kind == cpp_decl::Type::TK_NAMED) {
    const cpp_decl::QualifiedName & internal =
        type->named_qualified_name_syntax();
    for(std::size_t i = 0; i < internal.qualifiers.size(); ++i) {
      if(internal.qualifiers[i] != "_GLOBAL__N_1") {
        continue;
      }
      if(std::find(out.qualifiers.begin(),
                   out.qualifiers.end(),
                   internal.qualifiers[i]) != out.qualifiers.end()) {
        continue;
      }
      out.qualifiers.insert(
          out.qualifiers.begin() +
              std::min(i, out.qualifiers.size()),
          internal.qualifiers[i]);
    }
  }
  return out;
}

inline const cpp_decl::QualifiedName &
class_symbol_qualified_name_syntax(const ClassInfo & info)
{
  if(!info.symbol_qualified_name_syntax.name.empty()) {
    return info.symbol_qualified_name_syntax;
  }
  cpp_decl::TypePtr type = cpp_decl::strip_top_level_cv(info.type);
  if(type && type->kind == cpp_decl::Type::TK_NAMED) {
    return type->named_qualified_name_syntax();
  }
  static const cpp_decl::QualifiedName empty;
  return empty;
}

inline const std::string & class_instantiation_key(const ClassInfo & info)
{
  return info.instantiation_key_view ?
      *info.instantiation_key_view :
      info.instantiation_key;
}

inline void set_class_instantiation_key(ClassInfo & info,
                                        const std::string & key)
{
  info.instantiation_key_view = nullptr;
  info.instantiation_key = key;
}

inline void borrow_class_instantiation_key(ClassInfo & info,
                                           const std::string & stable_key)
{
  std::string().swap(info.instantiation_key);
  info.instantiation_key_view = &stable_key;
}

inline std::string class_internal_output_qualified_name(const ClassInfo & info)
{
  std::string out = class_output_qualified_name(info);
  const std::string unnamed = "<unnamed>";
  const std::string internal_unnamed = "_GLOBAL__N_1";
  std::size_t position = 0;
  while((position = out.find(unnamed, position)) != std::string::npos) {
    out.replace(position, unnamed.size(), internal_unnamed);
    position += internal_unnamed.size();
  }
  return out;
}

inline void set_class_output_qualified_name(ClassInfo & info,
                                            const std::string & name)
{
  if(name == info.qualified_name) {
    info.has_display_qualified_name = false;
    std::string().swap(info.display_qualified_name);
    return;
  }

  info.has_display_qualified_name = true;
  cpp_decl::TypePtr type = cpp_decl::strip_top_level_cv(info.type);
  if(type && type->kind == cpp_decl::Type::TK_NAMED) {
    const std::string prefix =
        info.class_kind.empty() ? std::string() : info.class_kind + " ";
    const std::string display = cpp_decl::named_type_display_text(type);
    const bool type_owns_same_display =
        display.size() == prefix.size() + name.size() &&
        display.compare(0, prefix.size(), prefix) == 0 &&
        display.compare(prefix.size(), name.size(), name) == 0;
    if(type_owns_same_display) {
      std::string().swap(info.display_qualified_name);
      return;
    }
  }
  info.display_qualified_name = name;
}

inline const std::vector<template_model::TemplateArgument> &
class_instantiation_binding_arguments(const ClassInfo & info)
{
  return info.instantiation_binding_arguments_view ?
      *info.instantiation_binding_arguments_view :
      info.instantiation_binding_arguments;
}

inline void reuse_primary_class_instantiation_binding_arguments(
    ClassInfo & info)
{
  std::vector<template_model::TemplateArgument>().swap(
      info.instantiation_binding_arguments);
  info.has_instantiation_binding_arguments = true;
  info.instantiation_binding_arguments_view = &info.instantiation_arguments;
}

inline void set_class_instantiation_binding_arguments(
    ClassInfo & info,
    const std::vector<template_model::TemplateArgument> & arguments)
{
  info.instantiation_binding_arguments = arguments;
  info.has_instantiation_binding_arguments = true;
  info.instantiation_binding_arguments_view =
      &info.instantiation_binding_arguments;
}

inline void detach_primary_class_instantiation_binding_arguments(
    ClassInfo & info)
{
  if(info.instantiation_binding_arguments_view !=
     &info.instantiation_arguments) {
    return;
  }
  info.instantiation_binding_arguments = info.instantiation_arguments;
  info.instantiation_binding_arguments_view =
      &info.instantiation_binding_arguments;
}

inline std::string function_output_name(const FunctionBinding & binding)
{
  if(binding.owner_class) {
    const std::string member_name =
        !binding.display_name.empty() ? binding.display_name :
        (binding.name.rfind("::") == std::string::npos ?
             binding.name :
             binding.name.substr(binding.name.rfind("::") + 2));
    return class_output_qualified_name(*binding.owner_class) + "::" + member_name;
  }
  return binding.name;
}

struct FunctionTemplateDecl
{
  Scope * declaring_scope = nullptr;
  Scope * pattern_scope = nullptr;
  std::string name;
  ClassInfo * lexical_access_class = nullptr;
  FunctionBinding * lexical_access_function = nullptr;
  MemberAccess access = MA_PUBLIC;
  bool is_constructor = false;
  bool is_inherited_constructor = false;
  ClassInfo * inherited_constructor_access_class = nullptr;
  bool is_destructor = false;
  bool is_conversion_operator = false;
  bool is_static_member = false;
  bool is_constexpr = false;
  bool is_explicit = false;
  bool is_const_method = false;
  bool is_volatile_method = false;
  RefQualifier ref_qualifier = RQ_NONE;
  bool is_deleted = false;
  bool decl_virtual = false;
  bool is_override = false;
  bool is_final = false;
  bool is_lambda_call_operator_template = false;
  bool is_member_function_template = false;
  bool exclude_from_explicit_instantiation = false;
  bool definition_is_explicit_owner_specialization = false;
  const CppAstNode * declaration_node = nullptr;
  const CppAstNode * definition_node = nullptr;
  const CppAstNode * inner = nullptr;
  const CppAstNode * definition_inner = nullptr;
  const CppAstNode * specifiers = nullptr;
  const CppAstNode * definition_specifiers = nullptr;
  const CppAstNode * declarator = nullptr;
  const CppAstNode * definition_declarator = nullptr;
  std::shared_ptr<CppAstNode> function_qualifier_storage;
  const CppAstNode * function_qualifier = nullptr;
  const CppAstNode * body = nullptr;
  const CppAstNode * ctor_initializer = nullptr;
  std::vector<template_model::TemplateParameterInfo> parameters;
  std::vector<template_model::TemplateParameterInfo> definition_owner_parameters;
  cpp_decl::TypePtr type_pattern;
  std::vector<std::pair<std::string, cpp_decl::TypePtr> > params_pattern;
  CppAstNode result_type_pattern;
  std::vector<const CppAstNode *> parameter_declarations_pattern;
  std::vector<std::string> parameter_aliases_pattern;
  std::vector<const CppAstNode *> default_arguments_pattern;
  std::vector<ClassInfo *> friend_access_classes;
  bool has_trailing_function_parameter_pack = false;
  bool trailing_function_parameter_pack_analyzed = false;
  std::string debug_decl_location;
  std::string debug_decl_location_details;
  std::string debug_scope_name;
  std::string debug_signature;
  std::map<std::string, FunctionBinding *> instantiations;
  mutable SourceDeclAnchorCache declaration_anchor;
};

inline std::vector<std::string> function_template_definition_abi_tags(
    const FunctionTemplateDecl & decl)
{
  std::vector<std::string> tags;
  append_function_declaration_abi_tags(tags, decl.definition_node);
  return tags;
}

inline bool function_binding_definition_suppresses_declaration_abi_tags(
    const FunctionBinding & binding)
{
  if(binding.definition_suppresses_declaration_abi_tags) {
    return true;
  }
  return binding.has_definition &&
         binding.definition_abi_tags.empty() &&
         binding.definition_node &&
         binding.definition_node != binding.declaration_node &&
         !binding.source_template &&
         binding.owner_class &&
         binding.owner_class->source_template;
}

inline std::vector<std::string> function_binding_abi_tags(const FunctionBinding & binding)
{
  if(function_binding_definition_suppresses_declaration_abi_tags(binding)) {
    return std::vector<std::string>();
  }
  if(!binding.definition_abi_tags.empty()) {
    return binding.definition_abi_tags;
  }
  return binding.declaration_abi_tags;
}

inline void initialize_function_parameter_aliases(FunctionBinding & binding)
{
  binding.parameter_aliases.resize(binding.params.size());
  for(std::size_t i = 0; i < binding.params.size(); ++i) {
    binding.parameter_aliases[i] = binding.params[i].first;
  }
}

inline void ensure_function_parameter_aliases(FunctionBinding & binding)
{
  if(binding.parameter_aliases.size() != binding.params.size()) {
    initialize_function_parameter_aliases(binding);
  }
}

inline void initialize_function_template_parameter_aliases(FunctionTemplateDecl & decl)
{
  decl.parameter_aliases_pattern.resize(decl.params_pattern.size());
  for(std::size_t i = 0; i < decl.params_pattern.size(); ++i) {
    decl.parameter_aliases_pattern[i] = decl.params_pattern[i].first;
  }
}

inline void ensure_function_template_parameter_aliases(FunctionTemplateDecl & decl)
{
  if(decl.parameter_aliases_pattern.size() != decl.params_pattern.size()) {
    initialize_function_template_parameter_aliases(decl);
  }
}

inline std::size_t function_binding_explicit_parameter_offset(const FunctionBinding & binding)
{
  return binding.is_method && !binding.params.empty() ? 1 : 0;
}

inline std::string function_parameter_binding_name(const FunctionBinding & binding,
                                                   std::size_t index)
{
  if(index >= binding.params.size()) {
    return std::string();
  }
  if(!binding.params[index].first.empty()) {
    return binding.params[index].first;
  }
  return std::string("__param") + std::to_string(index);
}

inline std::string function_parameter_alias_name(const FunctionBinding & binding,
                                                 std::size_t index)
{
  if(index < binding.parameter_aliases.size() &&
     !binding.parameter_aliases[index].empty()) {
    return binding.parameter_aliases[index];
  }
  if(index < binding.params.size()) {
    return binding.params[index].first;
  }
  return std::string();
}

inline std::string function_template_parameter_alias_name(const FunctionTemplateDecl & decl,
                                                          std::size_t index)
{
  if(index < decl.parameter_aliases_pattern.size() &&
     !decl.parameter_aliases_pattern[index].empty()) {
    return decl.parameter_aliases_pattern[index];
  }
  if(index < decl.params_pattern.size()) {
    return decl.params_pattern[index].first;
  }
  return std::string();
}

struct ClassTemplateSpecializationDecl
{
  ClassTemplateSpecializationDecl() {}
  ClassTemplateSpecializationDecl(Scope * declaring_scope,
                                  const CppAstNode * class_node)
    : declaring_scope(declaring_scope),
      class_node(class_node)
  {}

  Scope * declaring_scope = nullptr;
  const CppAstNode * class_node = nullptr;
};

struct AliasTemplateDecl
{
  struct StableSubstitutionArgumentKey
  {
    enum Kind
    {
      AK_TYPE_POINTER,
      AK_TYPE_FUNDAMENTAL,
      AK_VALUE,
      AK_TEMPLATE_DECL
    };

    Kind kind = AK_TYPE_POINTER;
    const void * pointer = nullptr;
    int type_code = 0;
    long long value = 0;
    bool value_type_is_fundamental = false;

    bool operator<(const StableSubstitutionArgumentKey & other) const
    {
      if(kind != other.kind) {
        return kind < other.kind;
      }
      if(type_code != other.type_code) {
        return type_code < other.type_code;
      }
      if(value != other.value) {
        return value < other.value;
      }
      if(value_type_is_fundamental != other.value_type_is_fundamental) {
        return value_type_is_fundamental < other.value_type_is_fundamental;
      }
      return std::less<const void *>()(pointer, other.pointer);
    }
  };

  struct StableSubstitutionKey
  {
    std::vector<StableSubstitutionArgumentKey> arguments;

    bool operator<(const StableSubstitutionKey & other) const
    {
      return arguments < other.arguments;
    }
  };

  struct StableSubstitutionFailure
  {
    std::string owner_type_key;
    std::string owner_type_display;
    std::string member_name;
  };

  struct StableAliasExpansionScopeKey
  {
    std::size_t instance_id = 0;
    std::size_t binding_fingerprint = 0;

    bool operator<(const StableAliasExpansionScopeKey & other) const
    {
      if(instance_id != other.instance_id) {
        return instance_id < other.instance_id;
      }
      return binding_fingerprint < other.binding_fingerprint;
    }
  };

  struct StableAliasExpansionArgumentKey
  {
    int kind = 0;
    bool dependent = false;
    bool source_defaulted = false;
    const void * type_pointer = nullptr;
    int type_code = 0;
    const void * template_decl = nullptr;
    const void * function_value = nullptr;
    std::string function_internal_symbol;
    const void * value_binding = nullptr;
    long long value = 0;
    std::string text;

    bool operator<(const StableAliasExpansionArgumentKey & other) const
    {
      if(kind != other.kind) {
        return kind < other.kind;
      }
      if(dependent != other.dependent) {
        return dependent < other.dependent;
      }
      if(source_defaulted != other.source_defaulted) {
        return source_defaulted < other.source_defaulted;
      }
      if(type_code != other.type_code) {
        return type_code < other.type_code;
      }
      if(value != other.value) {
        return value < other.value;
      }
      if(text != other.text) {
        return text < other.text;
      }
      if(type_pointer != other.type_pointer) {
        return std::less<const void *>()(type_pointer, other.type_pointer);
      }
      if(template_decl != other.template_decl) {
        return std::less<const void *>()(template_decl, other.template_decl);
      }
      if(function_internal_symbol != other.function_internal_symbol) {
        return function_internal_symbol < other.function_internal_symbol;
      }
      if(function_internal_symbol.empty() &&
         other.function_internal_symbol.empty() &&
         function_value != other.function_value) {
        return std::less<const void *>()(function_value, other.function_value);
      }
      return std::less<const void *>()(value_binding, other.value_binding);
    }
  };

  struct StableAliasExpansionKey
  {
    bool allow_dependent_expansion = false;
    StableAliasExpansionScopeKey match_scope;
    StableAliasExpansionScopeKey argument_scope;
    StableAliasExpansionScopeKey resolution_scope;
    std::vector<StableAliasExpansionArgumentKey> arguments;

    bool operator<(const StableAliasExpansionKey & other) const
    {
      if(allow_dependent_expansion != other.allow_dependent_expansion) {
        return allow_dependent_expansion < other.allow_dependent_expansion;
      }
      if(match_scope < other.match_scope) {
        return true;
      }
      if(other.match_scope < match_scope) {
        return false;
      }
      if(argument_scope < other.argument_scope) {
        return true;
      }
      if(other.argument_scope < argument_scope) {
        return false;
      }
      if(resolution_scope < other.resolution_scope) {
        return true;
      }
      if(other.resolution_scope < resolution_scope) {
        return false;
      }
      return arguments < other.arguments;
    }
  };

  struct StableAliasExpansionValue
  {
    enum Kind
    {
      EK_SUCCESS,
      EK_DEPENDENT_DEFER
    };

    Kind kind = EK_SUCCESS;
    std::string expanded_text;
    cpp_decl::TypePtr expanded_type;
  };

  Scope * declaring_scope = nullptr;
  Scope * pattern_scope = nullptr;
  // Source class-template declaration that lexically owns this alias
  // declaration.  Source-pattern member scopes intentionally have no
  // ClassInfo, so retaining this typed declaration link avoids recovering the
  // public owner from whichever use-site completion happens to run first.
  ClassTemplateDecl * source_owner_template = nullptr;
  const std::vector<cpp_decl::TemplateArgumentSyntax> *
      source_owner_arguments = nullptr;
  std::string name;
  MemberAccess access = MA_PUBLIC;
  const CppAstNode * type_id = nullptr;
  cpp_decl::TypePtr resolved_type_pattern;
  std::size_t gnu_ext_vector_type_parameter_index =
      static_cast<std::size_t>(-1);
  std::vector<template_model::TemplateParameterInfo> parameters;
  std::map<std::string, cpp_decl::TypePtr> instantiations;
  std::map<std::string, cpp_decl::TypePtr> reference_instantiations;
  mutable bool dependent_qualified_member_scope_sensitive_cached = false;
  mutable bool dependent_qualified_member_scope_sensitive = false;
  mutable std::map<StableSubstitutionKey, StableSubstitutionFailure>
      stable_substitution_failures;
  mutable std::map<StableAliasExpansionKey, StableAliasExpansionValue>
      stable_alias_expansions;
  mutable SourceDeclAnchorCache declaration_anchor;
};

struct DeductionGuideDecl
{
  Scope * declaring_scope = nullptr;
  Scope * pattern_scope = nullptr;
  std::string name;
  const CppAstNode * node = nullptr;
  const CppAstNode * declarator = nullptr;
  std::vector<template_model::TemplateParameterInfo> parameters;
  std::vector<std::string> return_arg_texts;
};

struct OutOfClassStaticMemberDecl
{
  Scope * declaring_scope = nullptr;
  Scope * pattern_scope = nullptr;
  // Definitions for nested members are retained by the first source owner.
  // Keep both that declaration and the immediate semantic member owner so
  // replay can select the concrete owner without parsing the qualified name.
  ClassTemplateDecl * source_owner_template = nullptr;
  ClassTemplateDecl * member_owner_template = nullptr;
  const CppAstNode * source_owner_declaration = nullptr;
  const cpp_decl::TemplateIdSyntax * source_owner_syntax = nullptr;
  const cpp_decl::TemplateIdSyntax * source_type_syntax = nullptr;
  cpp_decl::QualifiedName qualified_name_syntax;
  const CppAstNode * node = nullptr;
  const CppAstNode * specifiers = nullptr;
  const CppAstNode * declarator = nullptr;
  const CppAstNode * initializer = nullptr;
  std::vector<template_model::TemplateParameterInfo> parameters;
  uint32_t owner_reference_handle = 0;
  bool has_storage_definition = false;
};

struct OutOfClassMemberClassDecl
{
  Scope * declaring_scope = nullptr;
  Scope * pattern_scope = nullptr;
  const CppAstNode * class_node = nullptr;
  std::vector<template_model::TemplateParameterInfo> parameters;
};

struct OutOfClassMemberFunctionDecl
{
  Scope * declaring_scope = nullptr;
  Scope * pattern_scope = nullptr;
  std::string qualified_name;
  cpp_decl::QualifiedName qualified_name_syntax;
  const CppAstNode * owner_output_node = nullptr;
  std::string owner_specialization_key;
  const CppAstNode * specifiers = nullptr;
  const CppAstNode * declarator = nullptr;
  const CppAstNode * body = nullptr;
  const CppAstNode * ctor_initializer = nullptr;
  cpp_decl::TypePtr declared_type_pattern;
  std::vector<std::pair<std::string, cpp_decl::TypePtr> > params;
  bool is_const_method = false;
  bool is_volatile_method = false;
  semantic_model::RefQualifier ref_qualifier = semantic_model::RQ_NONE;
  bool is_defaulted = false;
  bool is_deleted = false;
  bool exclude_from_explicit_instantiation = false;
  uint32_t owner_reference_handle = 0;
  std::vector<template_model::TemplateParameterInfo> parameters;
};

struct OutOfClassMemberFunctionTemplateDefinition
{
  FunctionTemplateDecl * declaration = nullptr;
  const CppAstNode * definition_node = nullptr;
  const CppAstNode * definition_specifiers = nullptr;
  const CppAstNode * definition_declarator = nullptr;
  const CppAstNode * body = nullptr;
  const CppAstNode * ctor_initializer = nullptr;
  std::vector<std::string> parameter_aliases_pattern;
  std::vector<template_model::TemplateParameterInfo> owner_parameters;
  uint32_t owner_reference_handle = 0;
};

struct PartialClassTemplateSpecializationDecl
{
  Scope * declaring_scope = nullptr;
  Scope * pattern_scope = nullptr;
  const CppAstNode * class_node = nullptr;
  std::vector<template_model::TemplateParameterInfo> parameters;
  std::vector<std::string> arg_texts;
  std::vector<cpp_decl::TemplateArgumentSyntax> arg_syntaxes;
  mutable std::vector<cpp_decl::TypePtr> placeholder_arg_type_patterns;
  mutable std::vector<unsigned char>
      concrete_expression_recheck_pattern_states;
  std::map<std::string, OutOfClassStaticMemberDecl> static_member_definitions;
  std::map<std::string, OutOfClassMemberClassDecl> member_class_definitions;
  std::map<std::string, std::vector<OutOfClassMemberFunctionDecl> >
      member_function_definitions;
  std::map<std::string, std::vector<OutOfClassMemberFunctionTemplateDefinition> >
      member_function_template_definitions;
};

struct ClassTemplateDecl
{
  struct SpecializationSelectionCacheEntry
  {
    std::size_t specialization_epoch = 0;
    const CppAstNode * class_node = nullptr;
    Scope * binding_scope = nullptr;
    const std::vector<template_model::TemplateParameterInfo> * parameters = nullptr;
    std::vector<template_model::TemplateArgument> arguments;
    std::map<std::string, std::size_t> pack_sizes;
    std::string selection_key;
    int kind = 0;
  };

  Scope * declaring_scope = nullptr;
  Scope * pattern_scope = nullptr;
  std::string name;
  MemberAccess access = MA_PUBLIC;
  const CppAstNode * class_node = nullptr;
  std::vector<template_model::TemplateParameterInfo> parameters;
  // ABI metadata retained past Analyzer teardown shares one immutable
  // parameter snapshot per source template instead of copying it into every
  // specialization.
  mutable std::shared_ptr<
      const std::vector<template_model::TemplateParameterInfo> >
      retained_mangle_parameters;
  std::map<std::string, ClassInfo *> instantiations;
  std::map<std::string, ClassInfo *> reference_instantiations;
  std::map<std::string, ClassInfo *> fast_reference_cache;
  std::size_t specialization_epoch = 0;
  mutable std::map<std::string, SpecializationSelectionCacheEntry>
      specialization_selection_cache;
  std::set<std::string> suppress_implicit_instantiation_definitions;
  std::set<std::pair<std::string, std::string> >
      suppress_implicit_member_function_instantiation_definitions;
  std::set<std::string> explicit_static_member_specializations;
  std::set<std::pair<std::string, std::string> >
      explicit_static_member_specialization_keys;
  std::set<std::pair<std::string, std::string> >
      explicit_member_function_specialization_keys;
  std::map<std::string, ClassTemplateSpecializationDecl> explicit_specializations;
  std::vector<PartialClassTemplateSpecializationDecl> partial_specializations;
  std::vector<DeductionGuideDecl> deduction_guides;
  std::map<std::string, OutOfClassStaticMemberDecl> static_member_definitions;
  std::map<std::string, OutOfClassMemberClassDecl> member_class_definitions;
  std::map<std::string, std::vector<PartialClassTemplateSpecializationDecl> >
      member_class_template_partial_specializations;
  std::map<std::string, std::vector<OutOfClassMemberFunctionDecl> > member_function_definitions;
  std::map<std::string, std::vector<OutOfClassMemberFunctionTemplateDefinition> >
      member_function_template_definitions;
  // Replayed nested class-template declarations may own cloned AST nodes.
  // Cache the canonical source declaration used to share deferred definitions
  // across those equivalent declarations.
  mutable const CppAstNode * deferred_definition_source_identity = nullptr;
  bool comes_from_standard_include_path = false;
  mutable SourceDeclAnchorCache declaration_anchor;
};

struct VariableTemplateSpecializationDecl
{
  Scope * declaring_scope = nullptr;
  Scope * pattern_scope = nullptr;
  const CppAstNode * specifiers = nullptr;
  const CppAstNode * declarator = nullptr;
  const CppAstNode * initializer = nullptr;
  std::vector<template_model::TemplateParameterInfo> parameters;
  std::vector<std::string> arg_texts;
  std::vector<cpp_decl::TemplateArgumentSyntax> arg_syntaxes;
  mutable std::vector<cpp_decl::TypePtr> placeholder_arg_type_patterns;
  mutable std::vector<unsigned char>
      concrete_expression_recheck_pattern_states;
};

struct VariableTemplateDecl
{
  Scope * declaring_scope = nullptr;
  Scope * pattern_scope = nullptr;
  std::string name;
  const CppAstNode * specifiers = nullptr;
  const CppAstNode * declarator = nullptr;
  const CppAstNode * initializer = nullptr;
  std::vector<template_model::TemplateParameterInfo> parameters;
  cpp_decl::TypePtr type_pattern;
  std::map<std::string, ValueBinding> instantiations;
  std::map<std::string, VariableTemplateSpecializationDecl> explicit_specializations;
  std::vector<VariableTemplateSpecializationDecl> partial_specializations;
  bool comes_from_standard_include_path = false;
  mutable SourceDeclAnchorCache declaration_anchor;
};

std::string describe_scope_bindings(const Scope & scope);

}  // namespace semantic_model
