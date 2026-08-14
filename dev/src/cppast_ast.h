#ifndef CPPGM_CPPAST_AST_H
#define CPPGM_CPPAST_AST_H

#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include "cpp_decl_model.h"
#include "recog_token.h"
#include "template_angle_lookup.h"

#define CPP_AST_KIND_LIST(X) \
  X(invalid, "<invalid>") \
  X(abstract_declarator, "abstract-declarator") \
  X(access_specifier, "access-specifier") \
  X(alias_declaration, "alias-declaration") \
  X(asm_clause, "asm-clause") \
  X(asm_operand, "asm-operand") \
  X(asm_statement, "asm-statement") \
  X(argument_list, "argument-list") \
  X(array_delete, "array-delete") \
  X(array_suffix, "array-suffix") \
  X(assignment_expression, "assignment-expression") \
  X(attributed_statement, "attributed-statement") \
  X(base_clause, "base-clause") \
  X(base_name, "base-name") \
  X(base_specifier, "base-specifier") \
  X(bit_field_declaration, "bit-field-declaration") \
  X(bit_field_declarator, "bit-field-declarator") \
  X(binary_expression, "binary-expression") \
  X(braced_init_list, "braced-init-list") \
  X(break_statement, "break-statement") \
  X(call_expression, "call-expression") \
  X(case_statement, "case-statement") \
  X(cast_expression, "cast-expression") \
  X(class_forward_declaration, "class-forward-declaration") \
  X(class_key, "class-key") \
  X(class_specifier, "class-specifier") \
  X(compound_statement, "compound-statement") \
  X(condition, "condition") \
  X(condition_declaration, "condition-declaration") \
  X(conditional_expression, "conditional-expression") \
  X(continue_statement, "continue-statement") \
  X(coroutine_return_statement, "coroutine-return-statement") \
  X(ctor_initializer, "ctor-initializer") \
  X(cv_qualifier, "cv-qualifier") \
  X(decl_specifier, "decl-specifier") \
  X(decl_specifier_seq, "decl-specifier-seq") \
  X(declarator, "declarator") \
  X(designated_initializer, "designated-initializer") \
  X(designator, "designator") \
  X(decltype_specifier, "decltype-specifier") \
  X(default_argument, "default-argument") \
  X(default_statement, "default-statement") \
  X(default_template_argument, "default-template-argument") \
  X(delete_expression, "delete-expression") \
  X(deduction_guide_declaration, "deduction-guide-declaration") \
  X(do_statement, "do-statement") \
  X(ellipsis, "ellipsis") \
  X(else_node, "else") \
  X(empty_declaration, "empty-declaration") \
  X(enum_key, "enum-key") \
  X(enum_specifier, "enum-specifier") \
  X(enumerator, "enumerator") \
  X(exception_declaration, "exception-declaration") \
  X(explicit_instantiation_declaration, "explicit-instantiation-declaration") \
  X(explicit_instantiation_definition, "explicit-instantiation-definition") \
  X(expression_statement, "expression-statement") \
  X(for_init_statement, "for-init-statement") \
  X(for_statement, "for-statement") \
  X(fold_expression, "fold-expression") \
  X(function_definition, "function-definition") \
  X(function_qualifier, "function-qualifier") \
  X(lazy_function_body, "lazy-function-body") \
  X(global_scope, "global-scope") \
  X(goto_statement, "goto-statement") \
  X(handler, "handler") \
  X(id_expression, "id-expression") \
  X(identifier, "identifier") \
  X(if_statement, "if-statement") \
  X(init_declarator, "init-declarator") \
  X(init_declarator_list, "init-declarator-list") \
  X(initializer, "initializer") \
  X(inline_node, "inline") \
  X(iteration, "iteration") \
  X(keyword_literal, "keyword-literal") \
  X(linkage_specification, "linkage-specification") \
  X(labeled_statement, "labeled-statement") \
  X(lambda_declarator, "lambda-declarator") \
  X(lambda_expression, "lambda-expression") \
  X(lambda_introducer, "lambda-introducer") \
  X(lambda_specifier, "lambda-specifier") \
  X(literal, "literal") \
  X(mem_initializer, "mem-initializer") \
  X(mem_initializer_id, "mem-initializer-id") \
  X(member_expression, "member-expression") \
  X(member_specifiers, "member-specifiers") \
  X(message, "message") \
  X(namespace_alias_definition, "namespace-alias-definition") \
  X(namespace_definition, "namespace-definition") \
  X(nested_declarator, "nested-declarator") \
  X(new_expression, "new-expression") \
  X(noexcept_specification, "noexcept-specification") \
  X(nullability_qualifier, "nullability-qualifier") \
  X(non_type_template_parameter, "non-type-template-parameter") \
  X(parameter_clause, "parameter-clause") \
  X(parameter_declaration, "parameter-declaration") \
  X(parameter_key, "parameter-key") \
  X(parameter_pack, "parameter-pack") \
  X(paren_argument_list, "paren-argument-list") \
  X(paren_initializer, "paren-initializer") \
  X(pack_expansion_expression, "pack-expansion-expression") \
  X(parenthesized_expression, "parenthesized-expression") \
  X(placement, "placement") \
  X(postfix_expression, "postfix-expression") \
  X(ptr_operator, "ptr-operator") \
  X(range_declaration, "range-declaration") \
  X(range_for_statement, "range-for-statement") \
  X(range_initializer, "range-initializer") \
  X(ref_qualifier, "ref-qualifier") \
  X(return_statement, "return-statement") \
  X(simple_declaration, "simple-declaration") \
  X(sizeof_expression, "sizeof-expression") \
  X(sizeof_pack_expression, "sizeof-pack-expression") \
  X(statement_expression, "statement-expression") \
  X(special_definition, "special-definition") \
  X(special_initializer, "special-initializer") \
  X(special_member_declaration, "special-member-declaration") \
  X(special_member_definition, "special-member-definition") \
  X(specifier, "specifier") \
  X(static_assert_declaration, "static-assert-declaration") \
  X(structured_binding_declaration, "structured-binding-declaration") \
  X(structured_binding_declarator, "structured-binding-declarator") \
  X(structured_binding_identifier_list, "structured-binding-identifier-list") \
  X(subscript_expression, "subscript-expression") \
  X(switch_statement, "switch-statement") \
  X(target, "target") \
  X(template_declaration, "template-declaration") \
  X(template_parameter_clause, "template-parameter-clause") \
  X(template_parameter_list, "template-parameter-list") \
  X(template_template_parameter, "template-template-parameter") \
  X(then_node, "then") \
  X(throw_statement, "throw-statement") \
  X(trailing_return_type, "trailing-return-type") \
  X(translation_unit, "translation-unit") \
  X(try_block, "try-block") \
  X(type_id, "type-id") \
  X(type_name, "type-name") \
  X(type_parameter, "type-parameter") \
  X(type_specifier, "type-specifier") \
  X(type_specifier_seq, "type-specifier-seq") \
  X(type_trait_expression, "type-trait-expression") \
  X(unary_expression, "unary-expression") \
  X(using_declaration, "using-declaration") \
  X(using_directive, "using-directive") \
  X(virt_specifier, "virt-specifier") \
  X(virtual_node, "virtual") \
  X(while_statement, "while-statement")

enum class CppAstKind
{
#define CPP_AST_KIND_ENUM(name, text) name,
  CPP_AST_KIND_LIST(CPP_AST_KIND_ENUM)
#undef CPP_AST_KIND_ENUM
};

template <typename T>
class CppAstLazyVector
{
public:
  // Copies share populated storage. Every mutable entry point reaches
  // mutable_vector(), which detaches before exposing the underlying vector.
  typedef typename std::vector<T>::iterator iterator;
  typedef typename std::vector<T>::const_iterator const_iterator;

  CppAstLazyVector() : values_(nullptr) {}

  CppAstLazyVector(const CppAstLazyVector & other)
    : values_(other.values_)
  {
    retain();
  }

  CppAstLazyVector(CppAstLazyVector && other) noexcept
    : values_(other.values_)
  {
    other.values_ = nullptr;
  }

  CppAstLazyVector(const std::vector<T> & values)
    : values_(values.empty() ? nullptr : new Holder(values))
  {
  }

  CppAstLazyVector(std::vector<T> && values)
    : values_(values.empty() ? nullptr : new Holder(std::move(values)))
  {
  }

  ~CppAstLazyVector()
  {
    release();
  }

  CppAstLazyVector & operator=(const CppAstLazyVector & other)
  {
    if(this != &other) {
      CppAstLazyVector replacement(other);
      swap(replacement);
    }
    return *this;
  }

  CppAstLazyVector & operator=(CppAstLazyVector && other) noexcept
  {
    if(this != &other) {
      release();
      values_ = other.values_;
      other.values_ = nullptr;
    }
    return *this;
  }

  CppAstLazyVector & operator=(const std::vector<T> & values)
  {
    CppAstLazyVector replacement(values);
    swap(replacement);
    return *this;
  }

  CppAstLazyVector & operator=(std::vector<T> && values)
  {
    CppAstLazyVector replacement(std::move(values));
    swap(replacement);
    return *this;
  }

  operator const std::vector<T> &() const
  {
    return as_vector();
  }

  operator std::vector<T> &()
  {
    return mutable_vector();
  }

  bool empty() const
  {
    return !values_ || values_->values.empty();
  }

  std::size_t size() const
  {
    return values_ ? values_->values.size() : 0;
  }

  void clear()
  {
    release();
  }

  void reserve(std::size_t count)
  {
    if(count != 0) {
      mutable_vector().reserve(count);
    }
  }

  void push_back(const T & value)
  {
    mutable_vector().push_back(value);
  }

  void push_back(T && value)
  {
    mutable_vector().push_back(std::move(value));
  }

  template <typename InputIt>
  void assign(InputIt first, InputIt last)
  {
    if(first == last) {
      release();
      return;
    }
    mutable_vector().assign(first, last);
  }

  T & operator[](std::size_t index)
  {
    return mutable_vector()[index];
  }

  const T & operator[](std::size_t index) const
  {
    return as_vector()[index];
  }

  T & front()
  {
    return mutable_vector().front();
  }

  const T & front() const
  {
    return as_vector().front();
  }

  T & back()
  {
    return mutable_vector().back();
  }

  const T & back() const
  {
    return as_vector().back();
  }

  iterator begin()
  {
    return mutable_vector().begin();
  }

  iterator end()
  {
    return mutable_vector().end();
  }

  const_iterator begin() const
  {
    return as_vector().begin();
  }

  const_iterator end() const
  {
    return as_vector().end();
  }

  const_iterator cbegin() const
  {
    return as_vector().cbegin();
  }

  const_iterator cend() const
  {
    return as_vector().cend();
  }

  const std::vector<T> & as_vector() const
  {
    return values_ ? values_->values : empty_vector();
  }

  std::vector<T> & mutable_vector()
  {
    if(!values_) {
      values_ = new Holder();
    } else if(values_->reference_count != 1) {
      Holder * replacement = new Holder(values_->values);
      release();
      values_ = replacement;
    }
    return values_->values;
  }

private:
  struct Holder
  {
    Holder() : reference_count(1) {}

    explicit Holder(const std::vector<T> & values_in)
      : reference_count(1), values(values_in)
    {}

    explicit Holder(std::vector<T> && values_in)
      : reference_count(1), values(std::move(values_in))
    {}

    std::size_t reference_count;
    std::vector<T> values;
  };

  void retain()
  {
    if(values_) {
      ++values_->reference_count;
    }
  }

  void release()
  {
    if(values_ && --values_->reference_count == 0) {
      delete values_;
    }
    values_ = nullptr;
  }

  void swap(CppAstLazyVector & other) noexcept
  {
    std::swap(values_, other.values_);
  }

  static const std::vector<T> & empty_vector()
  {
    static const std::vector<T> empty;
    return empty;
  }

  Holder * values_;
};

template <typename T>
class CppAstSharedPtr
{
public:
  CppAstSharedPtr() : holder_(nullptr) {}

  CppAstSharedPtr(const CppAstSharedPtr & other)
      : holder_(other.holder_)
  {
    retain();
  }

  CppAstSharedPtr(CppAstSharedPtr && other) noexcept
      : holder_(other.holder_)
  {
    other.holder_ = nullptr;
  }

  ~CppAstSharedPtr()
  {
    release();
  }

  CppAstSharedPtr & operator=(const CppAstSharedPtr & other)
  {
    if(this != &other) {
      CppAstSharedPtr replacement(other);
      swap(replacement);
    }
    return *this;
  }

  CppAstSharedPtr & operator=(CppAstSharedPtr && other) noexcept
  {
    if(this != &other) {
      release();
      holder_ = other.holder_;
      other.holder_ = nullptr;
    }
    return *this;
  }

  explicit operator bool() const
  {
    return holder_ != nullptr;
  }

  T & operator*() const
  {
    return *holder_->value;
  }

  T * operator->() const
  {
    return holder_->value;
  }

  T * get() const
  {
    return holder_ ? holder_->value : nullptr;
  }

  bool unique() const
  {
    return holder_ && holder_->reference_count == 1;
  }

  void reset(T * value = nullptr)
  {
    Holder * replacement = nullptr;
    if(value) {
      try {
        replacement = new Holder(value);
      } catch(...) {
        delete value;
        throw;
      }
    }
    release();
    holder_ = replacement;
  }

  void swap(CppAstSharedPtr & other) noexcept
  {
    std::swap(holder_, other.holder_);
  }

private:
  struct Holder
  {
    explicit Holder(T * value_in)
        : reference_count(1), value(value_in)
    {}

    ~Holder()
    {
      delete value;
    }

    std::size_t reference_count;
    T * value;
  };

  void retain()
  {
    if(holder_) {
      ++holder_->reference_count;
    }
  }

  void release()
  {
    if(holder_ && --holder_->reference_count == 0) {
      delete holder_;
    }
    holder_ = nullptr;
  }

  Holder * holder_;
};

template <typename T>
inline bool operator==(const CppAstSharedPtr<T> & value, std::nullptr_t)
{
  return !value;
}

template <typename T>
inline bool operator==(std::nullptr_t, const CppAstSharedPtr<T> & value)
{
  return !value;
}

template <typename T>
inline bool operator!=(const CppAstSharedPtr<T> & value, std::nullptr_t)
{
  return static_cast<bool>(value);
}

template <typename T>
inline bool operator!=(std::nullptr_t, const CppAstSharedPtr<T> & value)
{
  return static_cast<bool>(value);
}

inline const char * cppast_kind_text(CppAstKind kind)
{
  switch(kind) {
#define CPP_AST_KIND_CASE(name, text) case CppAstKind::name: return text;
    CPP_AST_KIND_LIST(CPP_AST_KIND_CASE)
#undef CPP_AST_KIND_CASE
  }
  return "<invalid>";
}

struct CppAstNameLookupSnapshot
{
  typedef template_angle_lookup::NameSet NameSet;
  struct NameSetStack
  {
    // Snapshot names are immutable and never contain null atoms.  Delimit
    // scopes in one contiguous buffer instead of retaining a hash table (or
    // even a separate allocation) for every small saved set.
    template<typename InputIt>
    void push_scope(InputIt first, InputIt last)
    {
      names.insert(names.end(), first, last);
      names.push_back(nullptr);
      ++scope_count;
    }

    void clear()
    {
      names.clear();
      scope_count = 0;
    }

    std::size_t size() const
    {
      return scope_count;
    }

    std::vector<text_intern::Atom> names;
    std::size_t scope_count = 0;
  };

  struct NamespaceNameMap
  {
    // scope_names and the null-delimited atom buffer advance in lockstep.
    void clear()
    {
      scope_names.clear();
      names.clear();
    }

    std::size_t size() const
    {
      return scope_names.size();
    }

    std::vector<std::string> scope_names;
    std::vector<text_intern::Atom> names;
  };

  NameSetStack template_type_parameter_scopes;
  NameSetStack template_value_parameter_scopes;
  NameSetStack template_name_scopes;
  NameSetStack type_name_scopes;
  NameSetStack value_name_scopes;
  std::vector<std::string> class_name_stack;
  std::vector<std::string> namespace_path_stack;
  std::vector<bool> namespace_inline_stack;
  NamespaceNameMap namespace_template_name_scopes;
  NamespaceNameMap namespace_template_value_name_scopes;
  NamespaceNameMap namespace_type_name_scopes;
  NamespaceNameMap namespace_value_name_scopes;
  std::unordered_map<std::string, std::string> namespace_alias_targets;
};

struct CppAstRareStrings
{
  std::string gnu_ext_vector_type_argument_identifier;
  std::string gnu_section_segment;
  std::string gnu_section_name;
  std::string asm_label;
};

struct CppAstNode;

struct CppAstSparseData
{
  CppAstSparseData() : reference_count(1) {}
  CppAstSparseData(const CppAstSparseData & other)
      : reference_count(1),
        builtin_type_transform_name(other.builtin_type_transform_name),
        rare_strings(other.rare_strings),
        name_lookup_snapshot(other.name_lookup_snapshot),
        conversion_type_id_syntax(other.conversion_type_id_syntax),
        base_type_syntax(other.base_type_syntax),
        abi_tags(other.abi_tags),
        alignment_specifiers(other.alignment_specifiers),
        gnu_vector_size_bytes(other.gnu_vector_size_bytes)
  {}

  std::size_t reference_count;
  std::string builtin_type_transform_name;
  CppAstRareStrings rare_strings;
  std::shared_ptr<const CppAstNameLookupSnapshot> name_lookup_snapshot;
  std::shared_ptr<CppAstNode> conversion_type_id_syntax;
  std::shared_ptr<CppAstNode> base_type_syntax;
  CppAstLazyVector<std::string> abi_tags;
  CppAstLazyVector<std::string> alignment_specifiers;
  std::size_t gnu_vector_size_bytes = 0;
};

class CppAstSparseDataPtr
{
public:
  CppAstSparseDataPtr() : value_(nullptr) {}

  CppAstSparseDataPtr(const CppAstSparseDataPtr & other)
      : value_(other.value_)
  {
    retain();
  }

  CppAstSparseDataPtr(CppAstSparseDataPtr && other) noexcept
      : value_(other.value_)
  {
    other.value_ = nullptr;
  }

  ~CppAstSparseDataPtr()
  {
    release();
  }

  CppAstSparseDataPtr & operator=(const CppAstSparseDataPtr & other)
  {
    if(this != &other) {
      CppAstSparseDataPtr replacement(other);
      swap(replacement);
    }
    return *this;
  }

  CppAstSparseDataPtr & operator=(CppAstSparseDataPtr && other) noexcept
  {
    if(this != &other) {
      release();
      value_ = other.value_;
      other.value_ = nullptr;
    }
    return *this;
  }

  explicit operator bool() const
  {
    return value_ != nullptr;
  }

  CppAstSparseData & operator*() const
  {
    return *value_;
  }

  CppAstSparseData * operator->() const
  {
    return value_;
  }

  bool unique() const
  {
    return value_ && value_->reference_count == 1;
  }

  void reset(CppAstSparseData * value = nullptr)
  {
    if(value_ == value) {
      return;
    }
    release();
    value_ = value;
  }

  void swap(CppAstSparseDataPtr & other) noexcept
  {
    std::swap(value_, other.value_);
  }

private:
  void retain()
  {
    if(value_) {
      ++value_->reference_count;
    }
  }

  void release()
  {
    if(value_ && --value_->reference_count == 0) {
      delete value_;
    }
    value_ = nullptr;
  }

  CppAstSparseData * value_;
};

struct CppAstNode
{
  CppAstKind kind = CppAstKind::invalid;
  bool has_leading_typename = false;
  bool allows_implicit_typename = false;
  bool semantic_type_is_resolved_qualifier = false;
  bool has_exception_type_id_syntaxes = false;
  std::string value;
  cpp_decl::TypePtr semantic_type;
  CppAstSparseDataPtr sparse_data;
  CppAstSharedPtr<cpp_decl::QualifiedName> qualified_name_syntax;
  CppAstSharedPtr<cpp_decl::TemplateIdSyntax> template_id_syntax;
  CppAstLazyVector<cpp_decl::TemplateIdSyntax> qualifier_template_id_syntaxes;
  CppAstLazyVector<CppAstNode> qualifier_type_syntaxes;
  CppAstLazyVector<CppAstNode> exception_type_id_syntaxes;
  std::size_t maximum_field_alignment = 0;
  CppAstLazyVector<CppAstNode> alignment_specifier_nodes;
  // Token half-open range [token_start, token_end) in the original RecogToken stream.
  std::size_t token_start = 0;
  std::size_t token_end = 0;
  std::vector<CppAstNode> children;
  uint32_t source_location_id = 0;
  ERecogTokenKind token_kind = RT_EOF;
  ETokenType simple_type = static_cast<ETokenType>(0);
  bool linkage_has_braces = false;
  bool enum_has_definition = false;
  bool has_no_unique_address = false;
  bool has_using_if_exists = false;
  bool has_exclude_from_explicit_instantiation = false;
  bool has_weak_attribute = false;
  bool has_always_inline_attribute = false;
  bool is_final_specifier = false;
  bool uses_assignment_form = false;
  bool is_typeof_specifier = false;
  bool has_token = false;
};

inline CppAstNode cppast_copy_without_children(const CppAstNode & source)
{
  CppAstNode out;
  out.kind = source.kind;
  out.value = source.value;
  out.semantic_type = source.semantic_type;
  out.sparse_data = source.sparse_data;
  out.qualified_name_syntax = source.qualified_name_syntax;
  out.template_id_syntax = source.template_id_syntax;
  out.qualifier_template_id_syntaxes = source.qualifier_template_id_syntaxes;
  out.qualifier_type_syntaxes = source.qualifier_type_syntaxes;
  out.has_leading_typename = source.has_leading_typename;
  out.allows_implicit_typename = source.allows_implicit_typename;
  out.semantic_type_is_resolved_qualifier =
      source.semantic_type_is_resolved_qualifier;
  out.has_exception_type_id_syntaxes = source.has_exception_type_id_syntaxes;
  out.exception_type_id_syntaxes = source.exception_type_id_syntaxes;
  out.linkage_has_braces = source.linkage_has_braces;
  out.enum_has_definition = source.enum_has_definition;
  out.has_no_unique_address = source.has_no_unique_address;
  out.has_using_if_exists = source.has_using_if_exists;
  out.has_exclude_from_explicit_instantiation =
      source.has_exclude_from_explicit_instantiation;
  out.has_weak_attribute = source.has_weak_attribute;
  out.has_always_inline_attribute = source.has_always_inline_attribute;
  out.maximum_field_alignment = source.maximum_field_alignment;
  out.alignment_specifier_nodes = source.alignment_specifier_nodes;
  out.is_final_specifier = source.is_final_specifier;
  out.uses_assignment_form = source.uses_assignment_form;
  out.is_typeof_specifier = source.is_typeof_specifier;
  out.has_token = source.has_token;
  out.token_kind = source.token_kind;
  out.simple_type = source.simple_type;
  out.token_start = source.token_start;
  out.token_end = source.token_end;
  out.source_location_id = source.source_location_id;
  return out;
}

inline const std::string & cppast_empty_rare_string()
{
  static const std::string value;
  return value;
}

inline CppAstSparseData & mutable_cppast_sparse_data(CppAstNode & node)
{
  if(!node.sparse_data) {
    node.sparse_data.reset(new CppAstSparseData());
  } else if(!node.sparse_data.unique()) {
    node.sparse_data.reset(new CppAstSparseData(*node.sparse_data));
  }
  return *node.sparse_data;
}

inline const CppAstLazyVector<std::string> & cppast_abi_tags(
    const CppAstNode & node)
{
  static const CppAstLazyVector<std::string> empty;
  return node.sparse_data ? node.sparse_data->abi_tags : empty;
}

inline CppAstLazyVector<std::string> & mutable_cppast_abi_tags(
    CppAstNode & node)
{
  return mutable_cppast_sparse_data(node).abi_tags;
}

inline const CppAstLazyVector<std::string> & cppast_alignment_specifiers(
    const CppAstNode & node)
{
  static const CppAstLazyVector<std::string> empty;
  return node.sparse_data ? node.sparse_data->alignment_specifiers : empty;
}

inline CppAstLazyVector<std::string> & mutable_cppast_alignment_specifiers(
    CppAstNode & node)
{
  return mutable_cppast_sparse_data(node).alignment_specifiers;
}

inline std::size_t cppast_gnu_vector_size_bytes(const CppAstNode & node)
{
  return node.sparse_data ? node.sparse_data->gnu_vector_size_bytes : 0;
}

inline void set_cppast_gnu_vector_size_bytes(CppAstNode & node,
                                             std::size_t bytes)
{
  mutable_cppast_sparse_data(node).gnu_vector_size_bytes = bytes;
}

inline const std::string & cppast_builtin_type_transform_name(
    const CppAstNode & node)
{
  return node.sparse_data ?
      node.sparse_data->builtin_type_transform_name :
      cppast_empty_rare_string();
}

inline std::string & mutable_cppast_builtin_type_transform_name(
    CppAstNode & node)
{
  return mutable_cppast_sparse_data(node).builtin_type_transform_name;
}

inline const std::shared_ptr<const CppAstNameLookupSnapshot> &
cppast_name_lookup_snapshot(const CppAstNode & node)
{
  static const std::shared_ptr<const CppAstNameLookupSnapshot> empty;
  return node.sparse_data ? node.sparse_data->name_lookup_snapshot : empty;
}

inline std::shared_ptr<const CppAstNameLookupSnapshot> &
mutable_cppast_name_lookup_snapshot(CppAstNode & node)
{
  return mutable_cppast_sparse_data(node).name_lookup_snapshot;
}

inline const std::shared_ptr<CppAstNode> &
cppast_conversion_type_id_syntax_storage(const CppAstNode & node)
{
  static const std::shared_ptr<CppAstNode> empty;
  return node.sparse_data ? node.sparse_data->conversion_type_id_syntax : empty;
}

inline std::shared_ptr<CppAstNode> &
mutable_cppast_conversion_type_id_syntax_storage(CppAstNode & node)
{
  return mutable_cppast_sparse_data(node).conversion_type_id_syntax;
}

inline const std::shared_ptr<CppAstNode> &
cppast_base_type_syntax_storage(const CppAstNode & node)
{
  static const std::shared_ptr<CppAstNode> empty;
  return node.sparse_data ? node.sparse_data->base_type_syntax : empty;
}

inline std::shared_ptr<CppAstNode> &
mutable_cppast_base_type_syntax_storage(CppAstNode & node)
{
  return mutable_cppast_sparse_data(node).base_type_syntax;
}

inline bool cppast_has_rare_strings(const CppAstNode & node)
{
  if(!node.sparse_data) {
    return false;
  }
  const CppAstRareStrings & strings = node.sparse_data->rare_strings;
  return !strings.gnu_ext_vector_type_argument_identifier.empty() ||
         !strings.gnu_section_segment.empty() ||
         !strings.gnu_section_name.empty() ||
         !strings.asm_label.empty();
}

inline CppAstRareStrings & mutable_cppast_rare_strings(CppAstNode & node)
{
  return mutable_cppast_sparse_data(node).rare_strings;
}

inline const std::string & cppast_gnu_ext_vector_type_argument_identifier(
    const CppAstNode & node)
{
  return node.sparse_data ?
      node.sparse_data->rare_strings.gnu_ext_vector_type_argument_identifier :
      cppast_empty_rare_string();
}

inline std::string & mutable_cppast_gnu_ext_vector_type_argument_identifier(
    CppAstNode & node)
{
  return mutable_cppast_rare_strings(node)
      .gnu_ext_vector_type_argument_identifier;
}

inline const std::string & cppast_gnu_section_segment(const CppAstNode & node)
{
  return node.sparse_data ?
      node.sparse_data->rare_strings.gnu_section_segment :
      cppast_empty_rare_string();
}

inline std::string & mutable_cppast_gnu_section_segment(CppAstNode & node)
{
  return mutable_cppast_rare_strings(node).gnu_section_segment;
}

inline const std::string & cppast_gnu_section_name(const CppAstNode & node)
{
  return node.sparse_data ?
      node.sparse_data->rare_strings.gnu_section_name :
      cppast_empty_rare_string();
}

inline std::string & mutable_cppast_gnu_section_name(CppAstNode & node)
{
  return mutable_cppast_rare_strings(node).gnu_section_name;
}

inline const std::string & cppast_asm_label(const CppAstNode & node)
{
  return node.sparse_data ?
      node.sparse_data->rare_strings.asm_label :
      cppast_empty_rare_string();
}

inline std::string & mutable_cppast_asm_label(CppAstNode & node)
{
  return mutable_cppast_rare_strings(node).asm_label;
}

inline bool node_has_simple_type(const CppAstNode & node, ETokenType type)
{
  return node.has_token && node.token_kind == RT_SIMPLE && node.simple_type == type;
}

inline bool node_has_kind(const CppAstNode & node, CppAstKind kind)
{
  return node.kind == kind;
}

inline const cpp_decl::QualifiedName * cppast_qualified_name_syntax(const CppAstNode & node)
{
  return node.qualified_name_syntax.get();
}

inline void set_cppast_qualified_name_syntax(
    CppAstNode & node,
    const cpp_decl::QualifiedName & qualified_name)
{
  node.qualified_name_syntax.reset(new cpp_decl::QualifiedName(qualified_name));
}

inline void set_cppast_qualified_name_syntax(
    CppAstNode & node,
    cpp_decl::QualifiedName && qualified_name)
{
  node.qualified_name_syntax.reset(
      new cpp_decl::QualifiedName(std::move(qualified_name)));
}

inline const cpp_decl::TemplateIdSyntax * cppast_template_id_syntax(
    const CppAstNode & node)
{
  return node.template_id_syntax.get();
}

inline void set_cppast_template_id_syntax(
    CppAstNode & node,
    const cpp_decl::TemplateIdSyntax & template_id)
{
  node.template_id_syntax.reset(new cpp_decl::TemplateIdSyntax(template_id));
}

inline void set_cppast_template_id_syntax(
    CppAstNode & node,
    cpp_decl::TemplateIdSyntax && template_id)
{
  node.template_id_syntax.reset(
      new cpp_decl::TemplateIdSyntax(std::move(template_id)));
}

inline const CppAstNode * cppast_conversion_type_id_syntax(
    const CppAstNode & node)
{
  return cppast_conversion_type_id_syntax_storage(node).get();
}

inline void set_cppast_conversion_type_id_syntax(
    CppAstNode & node,
    const CppAstNode & type_id)
{
  mutable_cppast_conversion_type_id_syntax_storage(node).reset(
      new CppAstNode(type_id));
}

inline void set_cppast_conversion_type_id_syntax(
    CppAstNode & node,
    CppAstNode && type_id)
{
  mutable_cppast_conversion_type_id_syntax_storage(node).reset(
      new CppAstNode(std::move(type_id)));
}

inline const std::vector<CppAstNode> * cppast_exception_type_id_syntaxes(
    const CppAstNode & node)
{
  return node.has_exception_type_id_syntaxes ?
      &node.exception_type_id_syntaxes.as_vector() :
      nullptr;
}

inline void set_cppast_exception_type_id_syntaxes(
    CppAstNode & node,
    const std::vector<CppAstNode> & type_ids)
{
  node.has_exception_type_id_syntaxes = true;
  node.exception_type_id_syntaxes = type_ids;
}

inline void set_cppast_exception_type_id_syntaxes(
    CppAstNode & node,
    std::vector<CppAstNode> && type_ids)
{
  node.has_exception_type_id_syntaxes = true;
  node.exception_type_id_syntaxes = std::move(type_ids);
}

inline const cpp_decl::TemplateIdSyntax * cppast_qualifier_template_id_syntax(
    const CppAstNode & node,
    std::size_t qualifier_index)
{
  if(qualifier_index < node.qualifier_template_id_syntaxes.size() &&
     !node.qualifier_template_id_syntaxes[qualifier_index].name.name.empty()) {
    return &node.qualifier_template_id_syntaxes[qualifier_index];
  }
  if(node.template_id_syntax &&
     qualifier_index <
         node.template_id_syntax->qualifier_template_id_syntaxes.size() &&
     !node.template_id_syntax
          ->qualifier_template_id_syntaxes[qualifier_index]
          .name.name.empty()) {
    return &node.template_id_syntax
                ->qualifier_template_id_syntaxes[qualifier_index];
  }
  return nullptr;
}

inline bool cppast_has_qualifier_template_id_syntaxes(const CppAstNode & node)
{
  for(std::size_t i = 0; i < node.qualifier_template_id_syntaxes.size(); ++i) {
    if(!node.qualifier_template_id_syntaxes[i].name.name.empty()) {
      return true;
    }
  }
  if(!node.template_id_syntax) {
    return false;
  }
  for(std::size_t i = 0;
      i < node.template_id_syntax->qualifier_template_id_syntaxes.size();
      ++i) {
    if(!node.template_id_syntax
            ->qualifier_template_id_syntaxes[i]
            .name.name.empty()) {
      return true;
    }
  }
  return false;
}

inline void set_cppast_qualifier_template_id_syntaxes(
    CppAstNode & node,
    const std::vector<cpp_decl::TemplateIdSyntax> & qualifier_template_ids)
{
  node.qualifier_template_id_syntaxes = qualifier_template_ids;
}

inline void set_cppast_qualifier_template_id_syntaxes(
    CppAstNode & node,
    std::vector<cpp_decl::TemplateIdSyntax> && qualifier_template_ids)
{
  node.qualifier_template_id_syntaxes = std::move(qualifier_template_ids);
}

inline const CppAstNode * cppast_qualifier_type_syntax(
    const CppAstNode & node,
    std::size_t qualifier_index)
{
  if(qualifier_index >= node.qualifier_type_syntaxes.size() ||
     node.qualifier_type_syntaxes[qualifier_index].kind == CppAstKind::invalid) {
    return nullptr;
  }
  return &node.qualifier_type_syntaxes[qualifier_index];
}

inline void set_cppast_qualifier_type_syntaxes(
    CppAstNode & node,
    const std::vector<CppAstNode> & qualifier_type_syntaxes)
{
  node.qualifier_type_syntaxes = qualifier_type_syntaxes;
}

inline void set_cppast_qualifier_type_syntaxes(
    CppAstNode & node,
    std::vector<CppAstNode> && qualifier_type_syntaxes)
{
  node.qualifier_type_syntaxes = std::move(qualifier_type_syntaxes);
}

inline void append_unique_cppast_text(std::vector<std::string> & out,
                                      const std::string & value)
{
  if(value.empty()) {
    return;
  }
  for(std::size_t i = 0; i < out.size(); ++i) {
    if(out[i] == value) {
      return;
    }
  }
  out.push_back(value);
}

inline void append_cppast_alignment_specifier(CppAstNode & out,
                                              const std::string & text,
                                              const CppAstNode * syntax)
{
  if(text.empty()) {
    return;
  }
  const CppAstLazyVector<std::string> & existing =
      cppast_alignment_specifiers(out);
  for(std::size_t i = 0; i < existing.size(); ++i) {
    if(existing[i] == text) {
      return;
    }
  }
  mutable_cppast_alignment_specifiers(out).push_back(text);
  out.alignment_specifier_nodes.push_back(syntax ? *syntax : CppAstNode());
}

inline void append_cppast_abi_tags(std::vector<std::string> & out,
                                   const CppAstNode & node)
{
  const CppAstLazyVector<std::string> & tags = cppast_abi_tags(node);
  for(std::size_t i = 0; i < tags.size(); ++i) {
    append_unique_cppast_text(out, tags[i]);
  }
}

inline void append_cppast_alignment_specifiers(CppAstNode & out,
                                               const CppAstNode & node)
{
  const CppAstLazyVector<std::string> & specifiers =
      cppast_alignment_specifiers(node);
  for(std::size_t i = 0; i < specifiers.size(); ++i) {
    const CppAstNode * syntax =
        (i < node.alignment_specifier_nodes.size() &&
         node.alignment_specifier_nodes[i].kind != CppAstKind::invalid) ?
            &node.alignment_specifier_nodes[i] :
            nullptr;
    append_cppast_alignment_specifier(out, specifiers[i], syntax);
  }
}

inline const CppAstNode * cppast_find_child_kind(const CppAstNode & node,
                                                 CppAstKind kind,
                                                 std::size_t ordinal = 0)
{
  for(std::size_t i = 0; i < node.children.size(); ++i) {
    if(node.children[i].kind != kind) {
      continue;
    }
    if(ordinal == 0) {
      return &node.children[i];
    }
    --ordinal;
  }
  return nullptr;
}

inline bool cppast_recover_sizeof_type_id_expression_operand(
    const CppAstNode & type_id,
    CppAstNode & out)
{
  if(type_id.kind != CppAstKind::type_id) {
    return false;
  }

  const CppAstNode * specifiers =
      cppast_find_child_kind(type_id, CppAstKind::type_specifier_seq);
  if(!specifiers ||
     specifiers->children.size() != 1 ||
     specifiers->children[0].kind != CppAstKind::type_name) {
    return false;
  }

  const CppAstNode & name = specifiers->children[0];
  const cpp_decl::QualifiedName * qualified =
      cppast_qualified_name_syntax(name);
  if(name.has_leading_typename ||
     !qualified ||
     (!qualified->rooted && qualified->qualifiers.empty())) {
    return false;
  }

  CppAstNode expr = name;
  expr.kind = CppAstKind::id_expression;
  expr.children.clear();

  for(std::size_t i = 0; i < type_id.children.size(); ++i) {
    const CppAstNode & child = type_id.children[i];
    if(child.kind == CppAstKind::type_specifier_seq) {
      continue;
    }
    if(child.kind != CppAstKind::abstract_declarator) {
      return false;
    }
    for(std::size_t j = 0; j < child.children.size(); ++j) {
      const CppAstNode & suffix = child.children[j];
      if(suffix.kind != CppAstKind::array_suffix ||
         suffix.children.size() != 1) {
        return false;
      }

      CppAstNode subscript;
      subscript.kind = CppAstKind::subscript_expression;
      subscript.token_start = expr.token_start;
      subscript.token_end = suffix.token_end;
      subscript.source_location_id = expr.source_location_id;
      subscript.children.push_back(std::move(expr));
      subscript.children.push_back(suffix.children[0]);
      expr = std::move(subscript);
    }
  }

  out = std::move(expr);
  return true;
}

inline void append_function_declaration_abi_tags(std::vector<std::string> & out,
                                                 const CppAstNode * node)
{
  if(!node) {
    return;
  }

  append_cppast_abi_tags(out, *node);

  if(node->kind == CppAstKind::template_declaration) {
    for(std::size_t i = 0; i < node->children.size(); ++i) {
      if(node->children[i].kind == CppAstKind::template_parameter_clause) {
        continue;
      }
      append_function_declaration_abi_tags(out, &node->children[i]);
    }
    return;
  }

  if(const CppAstNode * specifiers =
         cppast_find_child_kind(*node, CppAstKind::decl_specifier_seq)) {
    append_cppast_abi_tags(out, *specifiers);
  }
  if(const CppAstNode * member_specifiers =
         cppast_find_child_kind(*node, CppAstKind::member_specifiers)) {
    append_cppast_abi_tags(out, *member_specifiers);
  }
  if(const CppAstNode * init_declarator_list =
         cppast_find_child_kind(*node, CppAstKind::init_declarator_list)) {
    for(std::size_t i = 0; i < init_declarator_list->children.size(); ++i) {
      const CppAstNode & child = init_declarator_list->children[i];
      if(child.kind != CppAstKind::init_declarator) {
        continue;
      }
      append_cppast_abi_tags(out, child);
      if(const CppAstNode * declarator =
             cppast_find_child_kind(child, CppAstKind::declarator)) {
        append_cppast_abi_tags(out, *declarator);
      }
    }
  }
  if(const CppAstNode * declarator = cppast_find_child_kind(*node, CppAstKind::declarator)) {
    append_cppast_abi_tags(out, *declarator);
  }
}

inline void append_effective_function_abi_tags(std::vector<std::string> & out,
                                               const CppAstNode * declaration_node,
                                               const CppAstNode * definition_node)
{
  if(definition_node) {
    append_function_declaration_abi_tags(out, definition_node);
    if(!out.empty()) {
      return;
    }
  }
  append_function_declaration_abi_tags(out, declaration_node);
}

inline void append_function_specifier_and_declarator_abi_tags(
    std::vector<std::string> & out,
    const CppAstNode * specifiers,
    const CppAstNode * declarator)
{
  if(specifiers) {
    append_cppast_abi_tags(out, *specifiers);
  }
  if(declarator) {
    append_cppast_abi_tags(out, *declarator);
  }
}

inline std::string cppast_display_value(const CppAstNode & node)
{
  if(!node.has_token) {
    return node.value;
  }

  switch(node.token_kind) {
  case RT_INVALID:
    return std::string("TT_INVALID:") + node.value;
  case RT_EOF:
    return "EOF";
  case RT_RSHIFT_1:
    return "ST_RSHIFT_1";
  case RT_RSHIFT_2:
    return "ST_RSHIFT_2";
  case RT_SIMPLE:
    return std::string(token_type_to_string(node.simple_type)) + ":" + node.value;
  case RT_IDENTIFIER:
    return std::string("TT_IDENTIFIER:") + node.value;
  case RT_LITERAL:
    return std::string("TT_LITERAL:") + node.value;
  }

  return node.value;
}

#endif
