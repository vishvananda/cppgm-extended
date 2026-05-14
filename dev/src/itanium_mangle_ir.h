#pragma once

#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

namespace itanium_mangle_ir {

struct DependentExpression;
struct Type;

struct SubstitutionKey
{
  enum Kind
  {
    SK_NONE,
    SK_LEGACY,
    SK_NAMED,
    SK_TYPE,
    SK_TEMPLATE_ENTITY,
    SK_PREFIX,
    SK_TYPE_BUILTIN,
    SK_TYPE_CV,
    SK_TYPE_POINTER,
    SK_TYPE_LVALUE_REFERENCE,
    SK_TYPE_RVALUE_REFERENCE,
    SK_TYPE_ARRAY,
    SK_TYPE_FUNCTION,
    SK_TYPE_MEMBER_POINTER,
    SK_TYPE_TEMPLATE_PARAMETER,
    SK_CLASS_TEMPLATE_SPECIALIZATION,
    SK_TEMPLATE_ARGUMENT_TYPE,
    SK_TEMPLATE_ARGUMENT_VALUE,
    SK_TEMPLATE_ARGUMENT_TEMPLATE,
    SK_FUNCTION_TEMPLATE_PREFIX
  };

  Kind kind = SK_NONE;
  std::uintptr_t id = 0;
  std::string payload;
  std::vector<SubstitutionKey> children;
  mutable std::size_t cached_hash = 0;

  static SubstitutionKey none()
  {
    return SubstitutionKey();
  }

  static SubstitutionKey legacy(const std::string & key)
  {
    return make(SK_LEGACY, key);
  }

  static SubstitutionKey named(const std::string & name)
  {
    return make(SK_NAMED, name);
  }

  static SubstitutionKey type(const std::string & key)
  {
    return make(SK_TYPE, key);
  }

  static SubstitutionKey template_entity(const std::string & key)
  {
    return make(SK_TEMPLATE_ENTITY, key);
  }

  static SubstitutionKey prefix(const std::string & key)
  {
    return make(SK_PREFIX, key);
  }

  static SubstitutionKey type_builtin(const std::string & code)
  {
    return make(SK_TYPE_BUILTIN, code);
  }

  static SubstitutionKey type_cv(bool is_const,
                                 bool is_volatile,
                                 const SubstitutionKey & inner)
  {
    std::string payload;
    if(is_const) {
      payload += 'K';
    }
    if(is_volatile) {
      payload += 'V';
    }
    return make_unary(SK_TYPE_CV, payload, inner);
  }

  static SubstitutionKey type_pointer(const SubstitutionKey & inner)
  {
    return make_unary(SK_TYPE_POINTER, std::string(), inner);
  }

  static SubstitutionKey type_lvalue_reference(const SubstitutionKey & inner)
  {
    return make_unary(SK_TYPE_LVALUE_REFERENCE, std::string(), inner);
  }

  static SubstitutionKey type_rvalue_reference(const SubstitutionKey & inner)
  {
    return make_unary(SK_TYPE_RVALUE_REFERENCE, std::string(), inner);
  }

  static SubstitutionKey type_array(const std::string & bound_key,
                                    const SubstitutionKey & inner)
  {
    return make_unary(SK_TYPE_ARRAY, bound_key, inner);
  }

  static SubstitutionKey type_function(const SubstitutionKey & result,
                                       const std::vector<SubstitutionKey> & params,
                                       bool variadic)
  {
    SubstitutionKey key;
    key.kind = SK_TYPE_FUNCTION;
    key.payload = variadic ? "z" : "v";
    key.children.reserve(params.size() + 1);
    key.children.push_back(result);
    key.children.insert(key.children.end(), params.begin(), params.end());
    return key;
  }

  static SubstitutionKey type_member_pointer(const SubstitutionKey & owner,
                                             const SubstitutionKey & member)
  {
    SubstitutionKey key;
    key.kind = SK_TYPE_MEMBER_POINTER;
    key.children.reserve(2);
    key.children.push_back(owner);
    key.children.push_back(member);
    return key;
  }

  static SubstitutionKey type_template_parameter(std::size_t index)
  {
    SubstitutionKey key;
    key.kind = SK_TYPE_TEMPLATE_PARAMETER;
    key.id = index;
    return key;
  }

  static SubstitutionKey class_template_specialization(
      std::uintptr_t template_id,
      const std::string & fallback_name,
      const std::vector<SubstitutionKey> & arguments)
  {
    SubstitutionKey key;
    key.kind = SK_CLASS_TEMPLATE_SPECIALIZATION;
    key.id = template_id;
    key.payload = fallback_name;
    key.children = arguments;
    return key;
  }

  static SubstitutionKey template_argument_type(const SubstitutionKey & type)
  {
    return make_unary(SK_TEMPLATE_ARGUMENT_TYPE, std::string(), type);
  }

  static SubstitutionKey template_argument_value(const std::string & value)
  {
    return make(SK_TEMPLATE_ARGUMENT_VALUE, value);
  }

  static SubstitutionKey template_argument_template(std::uintptr_t template_id,
                                                    const std::string & fallback_name)
  {
    SubstitutionKey key;
    key.kind = SK_TEMPLATE_ARGUMENT_TEMPLATE;
    key.id = template_id;
    key.payload = fallback_name;
    return key;
  }

  static SubstitutionKey function_template_prefix(const std::string & name)
  {
    return make(SK_FUNCTION_TEMPLATE_PREFIX, name);
  }

  bool empty() const
  {
    switch(kind) {
    case SK_NONE:
      return true;
    case SK_LEGACY:
    case SK_NAMED:
    case SK_TYPE:
    case SK_TEMPLATE_ENTITY:
    case SK_PREFIX:
    case SK_TYPE_BUILTIN:
    case SK_TEMPLATE_ARGUMENT_VALUE:
    case SK_FUNCTION_TEMPLATE_PREFIX:
      return payload.empty();
    case SK_CLASS_TEMPLATE_SPECIALIZATION:
    case SK_TEMPLATE_ARGUMENT_TEMPLATE:
      return id == 0 && payload.empty();
    case SK_TYPE_CV:
    case SK_TYPE_POINTER:
    case SK_TYPE_LVALUE_REFERENCE:
    case SK_TYPE_RVALUE_REFERENCE:
    case SK_TYPE_ARRAY:
    case SK_TEMPLATE_ARGUMENT_TYPE:
      return children.size() != 1 || children[0].empty();
    case SK_TYPE_FUNCTION:
      return children.empty() || children[0].empty() || payload.empty();
    case SK_TYPE_MEMBER_POINTER:
      return children.size() != 2 || children[0].empty() || children[1].empty();
    case SK_TYPE_TEMPLATE_PARAMETER:
      return false;
    }
    return true;
  }

  std::string legacy_text() const
  {
    switch(kind) {
    case SK_NONE:
      return std::string();
    case SK_LEGACY:
      return payload;
    case SK_NAMED:
      return std::string("name:") + payload;
    case SK_TYPE:
      return std::string("type:") + payload;
    case SK_TEMPLATE_ENTITY:
      return std::string("entity-template:") + payload;
    case SK_PREFIX:
      return std::string("template-prefix:") + payload;
    case SK_TYPE_BUILTIN:
      return std::string("type:builtin(") + payload + ")";
    case SK_TYPE_CV:
      if(children.size() != 1) {
        return std::string();
      }
      return std::string("type:cv(") + payload + "," +
             children[0].legacy_text() + ")";
    case SK_TYPE_POINTER:
      if(children.size() != 1) {
        return std::string();
      }
      return std::string("type:ptr(") + children[0].legacy_text() + ")";
    case SK_TYPE_LVALUE_REFERENCE:
      if(children.size() != 1) {
        return std::string();
      }
      return std::string("type:lref(") + children[0].legacy_text() + ")";
    case SK_TYPE_RVALUE_REFERENCE:
      if(children.size() != 1) {
        return std::string();
      }
      return std::string("type:rref(") + children[0].legacy_text() + ")";
    case SK_TYPE_ARRAY:
      if(children.size() != 1) {
        return std::string();
      }
      return std::string("type:array(") + payload + "," +
             children[0].legacy_text() + ")";
    case SK_TYPE_FUNCTION: {
      if(children.empty()) {
        return std::string();
      }
      std::string out = std::string("type:fn(") + children[0].legacy_text() + ";";
      for(std::size_t i = 1; i < children.size(); ++i) {
        if(i != 1) {
          out += ",";
        }
        out += children[i].legacy_text();
      }
      out += ";";
      out += payload;
      out += ")";
      return out;
    }
    case SK_TYPE_MEMBER_POINTER:
      if(children.size() != 2) {
        return std::string();
      }
      return std::string("type:mptr(") + children[0].legacy_text() + "," +
             children[1].legacy_text() + ")";
    case SK_TYPE_TEMPLATE_PARAMETER:
      return std::string("type:tparam(index:") + std::to_string(id) + ")";
    case SK_CLASS_TEMPLATE_SPECIALIZATION:
    case SK_TEMPLATE_ARGUMENT_TYPE:
    case SK_TEMPLATE_ARGUMENT_VALUE:
    case SK_TEMPLATE_ARGUMENT_TEMPLATE:
      return std::string();
    case SK_FUNCTION_TEMPLATE_PREFIX:
      return std::string("function-template-prefix:") + payload;
    }
    return std::string();
  }

  std::string structural_text() const
  {
    std::string out = std::to_string(static_cast<int>(kind));
    out += ':';
    out += std::to_string(id);
    out += ':';
    out += std::to_string(payload.size());
    out += ':';
    out += payload;
    out += '[';
    for(std::size_t i = 0; i < children.size(); ++i) {
      if(i != 0) {
        out += ',';
      }
      out += children[i].structural_text();
    }
    out += ']';
    return out;
  }

  bool operator<(const SubstitutionKey & rhs) const
  {
    if(kind != rhs.kind) {
      return kind < rhs.kind;
    }
    if(payload != rhs.payload) {
      return payload < rhs.payload;
    }
    if(id != rhs.id) {
      return id < rhs.id;
    }
    return children < rhs.children;
  }

  bool operator==(const SubstitutionKey & rhs) const
  {
    return kind == rhs.kind &&
           id == rhs.id &&
           payload == rhs.payload &&
           children == rhs.children;
  }

private:
  static SubstitutionKey make(Kind kind, const std::string & payload)
  {
    SubstitutionKey key;
    key.kind = kind;
    key.payload = payload;
    return key;
  }

  static SubstitutionKey make_unary(Kind kind,
                                    const std::string & payload,
                                    const SubstitutionKey & inner)
  {
    SubstitutionKey key;
    key.kind = kind;
    key.payload = payload;
    key.children.push_back(inner);
    return key;
  }
};

struct SubstitutionSlot
{
  std::string legacy_key;
  SubstitutionKey ir_key;

  static SubstitutionSlot legacy(const std::string & key)
  {
    SubstitutionSlot slot;
    slot.legacy_key = key;
    return slot;
  }

  static SubstitutionSlot typed(const SubstitutionKey & key)
  {
    SubstitutionSlot slot;
    slot.ir_key = key;
    return slot;
  }

  static SubstitutionSlot combined(const std::string & legacy_key,
                                   const SubstitutionKey & ir_key)
  {
    SubstitutionSlot slot;
    slot.legacy_key = legacy_key;
    slot.ir_key = ir_key;
    return slot;
  }

  bool empty() const
  {
    return legacy_key.empty() && ir_key.empty();
  }
};

struct SubstitutionSink
{
  virtual ~SubstitutionSink() {}
  virtual bool emit_substitution(const SubstitutionKey & key, std::string & out) = 0;
  virtual void register_substitution(const SubstitutionKey & key) = 0;
  virtual bool emit_dependent_parameter_type(const Type & type,
                                             std::string & out);
  virtual bool suppress_template_parameter_type_substitution_in_template_argument()
      const
  {
    return false;
  }
};

struct Type
{
  enum Kind
  {
    TK_INVALID,
    TK_BUILTIN,
    TK_CV,
    TK_POINTER,
    TK_LVALUE_REFERENCE,
    TK_RVALUE_REFERENCE,
    TK_ARRAY,
    TK_FUNCTION,
    TK_MEMBER_POINTER,
    TK_BUILTIN_TYPE_TRANSFORM,
    TK_PACK_EXPANSION,
    TK_TEMPLATE_PARAMETER,
    TK_NAMED,
    TK_CLASS_TEMPLATE_SPECIALIZATION,
    TK_DECLTYPE_EXPRESSION,
    TK_LAMBDA_CLOSURE
  };

  struct NameComponent
  {
    std::string source_name;
    std::string substitution_name;
    bool std_abbrev = false;

    static NameComponent source(const std::string & name,
                                const std::string & substitution)
    {
      NameComponent component;
      component.source_name = name;
      component.substitution_name = substitution;
      return component;
    }

    static NameComponent std_namespace()
    {
      NameComponent component;
      component.source_name = "std";
      component.std_abbrev = true;
      return component;
    }
  };

  struct ClassTemplateArgument
  {
    enum Kind
    {
      CTAK_INVALID,
      CTAK_TYPE,
      CTAK_INTEGRAL_VALUE,
      CTAK_DEPENDENT_INTEGRAL_VALUE,
      CTAK_DEPENDENT_EXPRESSION,
      CTAK_UNTYPED_INTEGRAL_VALUE,
      CTAK_TEMPLATE_ENTITY,
      CTAK_EXTERNAL_ENTITY,
      CTAK_ARGUMENT_PACK
    };

    struct Metadata
    {
      std::vector<NameComponent> prefix_components;
      std::string template_name;
      std::string template_name_substitution;
      std::string external_entity_symbol;
      std::vector<ClassTemplateArgument> pack_arguments;
      bool external_entity_address_of = false;
    };

    Kind kind = CTAK_INVALID;
    std::shared_ptr<Type> type;
    std::shared_ptr<Type> parameter_type;
    std::shared_ptr<DependentExpression> expression;
    std::shared_ptr<Metadata> metadata;
    long long integral_value = 0;

    static Metadata & ensure_metadata(ClassTemplateArgument & argument)
    {
      if(!argument.metadata || !argument.metadata.unique()) {
        argument.metadata.reset(argument.metadata ?
            new Metadata(*argument.metadata) :
            new Metadata);
      }
      return *argument.metadata;
    }

    static ClassTemplateArgument type_arg(const Type & type)
    {
      ClassTemplateArgument argument;
      argument.kind = CTAK_TYPE;
      argument.type.reset(new Type(type));
      return argument;
    }

    static ClassTemplateArgument integral_value_arg(const Type & type,
                                                    long long value)
    {
      ClassTemplateArgument argument;
      argument.kind = CTAK_INTEGRAL_VALUE;
      argument.type.reset(new Type(type));
      argument.integral_value = value;
      return argument;
    }

    static ClassTemplateArgument dependent_integral_value_arg(
        const Type & parameter_type,
        const Type & value_type,
        long long value)
    {
      ClassTemplateArgument argument;
      argument.kind = CTAK_DEPENDENT_INTEGRAL_VALUE;
      argument.parameter_type.reset(new Type(parameter_type));
      argument.type.reset(new Type(value_type));
      argument.integral_value = value;
      return argument;
    }

    static ClassTemplateArgument dependent_untyped_integral_value_arg(
        const Type & parameter_type,
        long long value)
    {
      ClassTemplateArgument argument;
      argument.kind = CTAK_DEPENDENT_INTEGRAL_VALUE;
      argument.parameter_type.reset(new Type(parameter_type));
      argument.integral_value = value;
      return argument;
    }

    static ClassTemplateArgument untyped_integral_value_arg(long long value)
    {
      ClassTemplateArgument argument;
      argument.kind = CTAK_UNTYPED_INTEGRAL_VALUE;
      argument.integral_value = value;
      return argument;
    }

    static ClassTemplateArgument dependent_expression_arg(
        const DependentExpression & expression);

    static ClassTemplateArgument template_entity_arg(
        const std::vector<NameComponent> & prefix_components,
        const std::string & name,
        const std::string & substitution)
    {
      ClassTemplateArgument argument;
      argument.kind = CTAK_TEMPLATE_ENTITY;
      Metadata & metadata = ensure_metadata(argument);
      metadata.prefix_components = prefix_components;
      metadata.template_name = name;
      metadata.template_name_substitution = substitution;
      return argument;
    }

    static ClassTemplateArgument external_entity_arg(const std::string & symbol,
                                                     bool address_of)
    {
      ClassTemplateArgument argument;
      argument.kind = CTAK_EXTERNAL_ENTITY;
      Metadata & metadata = ensure_metadata(argument);
      metadata.external_entity_symbol = symbol;
      metadata.external_entity_address_of = address_of;
      return argument;
    }

    static ClassTemplateArgument argument_pack(
        const std::vector<ClassTemplateArgument> & arguments)
    {
      ClassTemplateArgument argument;
      argument.kind = CTAK_ARGUMENT_PACK;
      Metadata & metadata = ensure_metadata(argument);
      metadata.pack_arguments = arguments;
      return argument;
    }
  };

  struct SubstitutionMetadata
  {
    std::vector<std::string> preregister_legacy_keys;
    SubstitutionKey key;
  };

  struct LambdaMetadata
  {
    std::string context_fragment;
    std::vector<SubstitutionSlot> context_substitution_slots;
    std::string source_name;
    std::string discriminator;
  };

  struct NameMetadata
  {
    std::size_t template_name_parameter_index = 0;
    std::vector<NameComponent> prefix_components;
    std::string template_name;
    std::string template_name_substitution;
    std::vector<ClassTemplateArgument> template_arguments;
    std::string standard_substitution;
    bool standard_substitution_includes_arguments = false;
    bool register_member_expression_template_name = false;
    bool template_name_is_template_parameter = false;
  };

  Kind kind = TK_INVALID;
  char builtin_code[3] = {0, 0, 0};
  std::string builtin_transform_name;
  std::string array_bound;
  std::string array_substitution_bound_key;
  std::size_t template_parameter_index = 0;
  bool cv_const = false;
  bool cv_volatile = false;
  bool variadic = false;
  std::shared_ptr<Type> inner;
  std::shared_ptr<Type> owner;
  std::shared_ptr<Type> name_owner;
  std::shared_ptr<DependentExpression> expression;
  std::vector<Type> params;
  std::shared_ptr<SubstitutionMetadata> substitution;
  std::shared_ptr<LambdaMetadata> lambda;
  std::shared_ptr<NameMetadata> name;

  static NameMetadata & ensure_name_metadata(Type & type)
  {
    if(!type.name || !type.name.unique()) {
      type.name.reset(type.name ? new NameMetadata(*type.name) :
                                  new NameMetadata);
    }
    return *type.name;
  }

  static Type builtin(const std::string & code)
  {
    Type type;
    type.kind = TK_BUILTIN;
    const std::size_t count = code.size() < 2 ? code.size() : 2;
    for(std::size_t i = 0; i < count; ++i) {
      type.builtin_code[i] = code[i];
    }
    return type;
  }

  static Type cv(bool is_const, bool is_volatile, const Type & inner)
  {
    Type type;
    type.kind = TK_CV;
    type.cv_const = is_const;
    type.cv_volatile = is_volatile;
    type.inner.reset(new Type(inner));
    return type;
  }

  static Type pointer(const Type & inner)
  {
    Type type;
    type.kind = TK_POINTER;
    type.inner.reset(new Type(inner));
    return type;
  }

  static Type lvalue_reference(const Type & inner)
  {
    Type type;
    type.kind = TK_LVALUE_REFERENCE;
    type.inner.reset(new Type(inner));
    return type;
  }

  static Type rvalue_reference(const Type & inner)
  {
    Type type;
    type.kind = TK_RVALUE_REFERENCE;
    type.inner.reset(new Type(inner));
    return type;
  }

  static Type array(const std::string & bound, const Type & inner)
  {
    Type type;
    type.kind = TK_ARRAY;
    type.array_bound = bound;
    type.inner.reset(new Type(inner));
    return type;
  }

  static Type array(const std::string & bound,
                    const std::string & substitution_bound_key,
                    const Type & inner)
  {
    Type type = array(bound, inner);
    type.array_substitution_bound_key = substitution_bound_key;
    return type;
  }

  static Type function(const Type & result,
                       const std::vector<Type> & params,
                       bool variadic)
  {
    Type type;
    type.kind = TK_FUNCTION;
    type.inner.reset(new Type(result));
    type.params = params;
    type.variadic = variadic;
    return type;
  }

  static Type member_pointer(const Type & owner, const Type & member)
  {
    Type type;
    type.kind = TK_MEMBER_POINTER;
    type.owner.reset(new Type(owner));
    type.inner.reset(new Type(member));
    return type;
  }

  static Type builtin_type_transform(const std::string & name,
                                     const Type & argument)
  {
    Type type;
    type.kind = TK_BUILTIN_TYPE_TRANSFORM;
    type.builtin_transform_name = name;
    type.inner.reset(new Type(argument));
    return type;
  }

  static Type pack_expansion(const Type & pattern)
  {
    Type type;
    type.kind = TK_PACK_EXPANSION;
    type.inner.reset(new Type(pattern));
    return type;
  }

  static Type template_parameter(std::size_t index)
  {
    Type type;
    type.kind = TK_TEMPLATE_PARAMETER;
    type.template_parameter_index = index;
    return type;
  }

  static Type named_type(const std::vector<NameComponent> & prefix_components,
                         const std::string & name,
                         const std::string & substitution)
  {
    Type type;
    type.kind = TK_NAMED;
    NameMetadata & metadata = ensure_name_metadata(type);
    metadata.prefix_components = prefix_components;
    metadata.template_name = name;
    metadata.template_name_substitution = substitution;
    return type;
  }

  static Type member_named_type(const Type & owner,
                                const std::string & name,
                                const std::string & substitution)
  {
    Type type;
    type.kind = TK_NAMED;
    type.name_owner.reset(new Type(owner));
    NameMetadata & metadata = ensure_name_metadata(type);
    metadata.template_name = name;
    metadata.template_name_substitution = substitution;
    return type;
  }

  static Type class_template_specialization(
      const std::vector<NameComponent> & prefix_components,
      const std::string & template_name,
      const std::string & template_name_substitution,
      const std::vector<ClassTemplateArgument> & arguments,
      const std::string & standard_substitution,
      bool standard_substitution_includes_arguments)
  {
    Type type;
    type.kind = TK_CLASS_TEMPLATE_SPECIALIZATION;
    NameMetadata & metadata = ensure_name_metadata(type);
    metadata.prefix_components = prefix_components;
    metadata.template_name = template_name;
    metadata.template_name_substitution = template_name_substitution;
    metadata.template_arguments = arguments;
    metadata.standard_substitution = standard_substitution;
    metadata.standard_substitution_includes_arguments =
        standard_substitution_includes_arguments;
    return type;
  }

  static Type template_parameter_class_template_specialization(
      std::size_t template_parameter_index,
      const std::vector<ClassTemplateArgument> & arguments)
  {
    Type type;
    type.kind = TK_CLASS_TEMPLATE_SPECIALIZATION;
    NameMetadata & metadata = ensure_name_metadata(type);
    metadata.template_name_is_template_parameter = true;
    metadata.template_name_parameter_index = template_parameter_index;
    metadata.template_arguments = arguments;
    return type;
  }

  static Type member_class_template_specialization(
      const Type & owner,
      const std::string & template_name,
      const std::string & template_name_substitution,
      const std::vector<ClassTemplateArgument> & arguments)
  {
    Type type;
    type.kind = TK_CLASS_TEMPLATE_SPECIALIZATION;
    type.name_owner.reset(new Type(owner));
    NameMetadata & metadata = ensure_name_metadata(type);
    metadata.template_name = template_name;
    metadata.template_name_substitution = template_name_substitution;
    metadata.template_arguments = arguments;
    return type;
  }

  static Type lambda_closure(
      const std::string & context_fragment,
      const std::vector<SubstitutionSlot> & context_substitution_slots,
      const std::vector<Type> & signature_parameter_types,
      const std::string & discriminator,
      const std::string & source_name = std::string())
  {
    Type type;
    type.kind = TK_LAMBDA_CLOSURE;
    type.lambda.reset(new LambdaMetadata());
    type.lambda->context_fragment = context_fragment;
    type.lambda->context_substitution_slots = context_substitution_slots;
    type.lambda->source_name = source_name;
    type.params = signature_parameter_types;
    type.lambda->discriminator = discriminator;
    return type;
  }
};

struct TemplateArgument
{
  enum Kind
  {
    TAK_INVALID,
    TAK_TYPE,
    TAK_INTEGRAL_VALUE,
    TAK_DEPENDENT_INTEGRAL_VALUE,
    TAK_DEPENDENT_EXPRESSION,
    TAK_UNTYPED_INTEGRAL_VALUE,
    TAK_TEMPLATE_ENTITY,
    TAK_EXTERNAL_ENTITY,
    TAK_ARGUMENT_PACK
  };

  struct Metadata
  {
    std::vector<Type::NameComponent> prefix_components;
    std::string template_name;
    std::string template_name_substitution;
    std::string external_entity_symbol;
    std::vector<TemplateArgument> pack_arguments;
    bool external_entity_address_of = false;
  };

  Kind kind = TAK_INVALID;
  std::shared_ptr<Type> value_type;
  std::shared_ptr<Type> parameter_type;
  std::shared_ptr<DependentExpression> expression;
  std::shared_ptr<Metadata> metadata;
  long long integral_value = 0;

  static Metadata & ensure_metadata(TemplateArgument & argument)
  {
    if(!argument.metadata || !argument.metadata.unique()) {
      argument.metadata.reset(argument.metadata ?
          new Metadata(*argument.metadata) :
          new Metadata);
    }
    return *argument.metadata;
  }

  static TemplateArgument type_arg(const Type & type)
  {
    TemplateArgument argument;
    argument.kind = TAK_TYPE;
    argument.value_type.reset(new Type(type));
    return argument;
  }

  static TemplateArgument integral_value_arg(const Type & type, long long value)
  {
    TemplateArgument argument;
    argument.kind = TAK_INTEGRAL_VALUE;
    argument.value_type.reset(new Type(type));
    argument.integral_value = value;
    return argument;
  }

  static TemplateArgument dependent_integral_value_arg(
      const Type & parameter_type,
      const Type & value_type,
      long long value)
  {
    TemplateArgument argument;
    argument.kind = TAK_DEPENDENT_INTEGRAL_VALUE;
    argument.parameter_type.reset(new Type(parameter_type));
    argument.value_type.reset(new Type(value_type));
    argument.integral_value = value;
    return argument;
  }

  static TemplateArgument dependent_untyped_integral_value_arg(
      const Type & parameter_type,
      long long value)
  {
    TemplateArgument argument;
    argument.kind = TAK_DEPENDENT_INTEGRAL_VALUE;
    argument.parameter_type.reset(new Type(parameter_type));
    argument.integral_value = value;
    return argument;
  }

  static TemplateArgument untyped_integral_value_arg(long long value)
  {
    TemplateArgument argument;
    argument.kind = TAK_UNTYPED_INTEGRAL_VALUE;
    argument.integral_value = value;
    return argument;
  }

  static TemplateArgument dependent_expression_arg(
      const DependentExpression & expression);

  static TemplateArgument template_entity_arg(
      const std::vector<Type::NameComponent> & prefix_components,
      const std::string & name,
      const std::string & substitution)
  {
    TemplateArgument argument;
    argument.kind = TAK_TEMPLATE_ENTITY;
    Metadata & metadata = ensure_metadata(argument);
    metadata.prefix_components = prefix_components;
    metadata.template_name = name;
    metadata.template_name_substitution = substitution;
    return argument;
  }

  static TemplateArgument external_entity_arg(const std::string & symbol,
                                              bool address_of)
  {
    TemplateArgument argument;
    argument.kind = TAK_EXTERNAL_ENTITY;
    Metadata & metadata = ensure_metadata(argument);
    metadata.external_entity_symbol = symbol;
    metadata.external_entity_address_of = address_of;
    return argument;
  }

  static TemplateArgument argument_pack(
      const std::vector<TemplateArgument> & arguments)
  {
    TemplateArgument argument;
    argument.kind = TAK_ARGUMENT_PACK;
    Metadata & metadata = ensure_metadata(argument);
    metadata.pack_arguments = arguments;
    return argument;
  }
};

struct DependentExpression
{
  enum Kind
  {
    EK_INVALID,
    EK_TEMPLATE_PARAMETER,
    EK_FUNCTION_PARAMETER,
    EK_LITERAL,
    EK_INTEGRAL_VALUE,
    EK_MEMBER,
    EK_OBJECT_MEMBER,
    EK_UNARY,
    EK_BINARY,
    EK_PACK_EXPANSION,
    EK_CALL,
    EK_CONVERSION,
    EK_TEMPLATE_ID,
    EK_TYPE_TRAIT,
    EK_SIZEOF_TYPE,
    EK_EXTERNAL_ENTITY
  };

  Kind kind = EK_INVALID;
  std::string text;
  std::string op_code;
  std::shared_ptr<Type> owner_type;
  bool close_member_owner = false;
  std::shared_ptr<DependentExpression> inner;
  std::vector<DependentExpression> arguments;
  std::vector<TemplateArgument> template_arguments;
  std::vector<Type> type_arguments;
  std::size_t template_parameter_index = 0;
  long long integral_value = 0;
  bool suppress_template_prefix_substitution = false;
  bool suppress_member_owner_prefix = false;
  bool external_entity_address_of = false;

  static DependentExpression template_parameter(std::size_t index)
  {
    DependentExpression expression;
    expression.kind = EK_TEMPLATE_PARAMETER;
    expression.template_parameter_index = index;
    return expression;
  }

  static DependentExpression function_parameter(std::size_t index)
  {
    DependentExpression expression;
    expression.kind = EK_FUNCTION_PARAMETER;
    expression.template_parameter_index = index;
    return expression;
  }

  static DependentExpression literal(const std::string & text)
  {
    DependentExpression expression;
    expression.kind = EK_LITERAL;
    expression.text = text;
    return expression;
  }

  static DependentExpression typed_integral_value(const Type & type,
                                                  long long value)
  {
    DependentExpression expression;
    expression.kind = EK_INTEGRAL_VALUE;
    expression.owner_type.reset(new Type(type));
    expression.integral_value = value;
    return expression;
  }

  static DependentExpression member(const Type & owner,
                                    bool close_owner,
                                    const std::string & name)
  {
    DependentExpression expression;
    expression.kind = EK_MEMBER;
    expression.owner_type.reset(new Type(owner));
    expression.close_member_owner = close_owner;
    expression.text = name;
    return expression;
  }

  static DependentExpression object_member(
      const std::string & op_code,
      const DependentExpression & object,
      const std::string & name,
      const std::vector<TemplateArgument> & template_arguments)
  {
    DependentExpression expression;
    expression.kind = EK_OBJECT_MEMBER;
    expression.op_code = op_code;
    expression.inner.reset(new DependentExpression(object));
    expression.text = name;
    expression.template_arguments = template_arguments;
    return expression;
  }

  static DependentExpression unary(const std::string & op_code,
                                   const DependentExpression & inner)
  {
    DependentExpression expression;
    expression.kind = EK_UNARY;
    expression.op_code = op_code;
    expression.inner.reset(new DependentExpression(inner));
    return expression;
  }

  static DependentExpression pack_expansion(const DependentExpression & inner)
  {
    DependentExpression expression;
    expression.kind = EK_PACK_EXPANSION;
    expression.inner.reset(new DependentExpression(inner));
    return expression;
  }

  static DependentExpression binary(const std::string & op_code,
                                    const DependentExpression & left,
                                    const DependentExpression & right)
  {
    DependentExpression expression;
    expression.kind = EK_BINARY;
    expression.op_code = op_code;
    expression.inner.reset(new DependentExpression(left));
    expression.arguments.push_back(right);
    return expression;
  }

  static DependentExpression call(const DependentExpression & callee,
                                  const std::vector<DependentExpression> & arguments)
  {
    DependentExpression expression;
    expression.kind = EK_CALL;
    expression.inner.reset(new DependentExpression(callee));
    expression.arguments = arguments;
    return expression;
  }

  static DependentExpression conversion(
      const Type & type,
      const std::vector<DependentExpression> & arguments)
  {
    DependentExpression expression;
    expression.kind = EK_CONVERSION;
    expression.op_code = "cv";
    expression.owner_type.reset(new Type(type));
    expression.arguments = arguments;
    return expression;
  }

  static DependentExpression cast(
      const std::string & op_code,
      const Type & type,
      const DependentExpression & argument)
  {
    DependentExpression expression;
    expression.kind = EK_CONVERSION;
    expression.op_code = op_code;
    expression.owner_type.reset(new Type(type));
    expression.arguments.push_back(argument);
    return expression;
  }

  static DependentExpression template_id(
      const std::string & name,
      const std::vector<TemplateArgument> & arguments)
  {
    DependentExpression expression;
    expression.kind = EK_TEMPLATE_ID;
    expression.text = name;
    expression.template_arguments = arguments;
    return expression;
  }

  static DependentExpression type_trait(
      const std::string & name,
      const std::vector<Type> & arguments)
  {
    DependentExpression expression;
    expression.kind = EK_TYPE_TRAIT;
    expression.text = name;
    expression.type_arguments = arguments;
    return expression;
  }

  static DependentExpression sizeof_type(const Type & type)
  {
    DependentExpression expression;
    expression.kind = EK_SIZEOF_TYPE;
    expression.owner_type.reset(new Type(type));
    return expression;
  }

  static DependentExpression external_entity(const std::string & symbol,
                                             bool address_of)
  {
    DependentExpression expression;
    expression.kind = EK_EXTERNAL_ENTITY;
    expression.text = symbol;
    expression.external_entity_address_of = address_of;
    return expression;
  }
};

inline Type::ClassTemplateArgument
Type::ClassTemplateArgument::dependent_expression_arg(
    const DependentExpression & expression)
{
  ClassTemplateArgument argument;
  argument.kind = CTAK_DEPENDENT_EXPRESSION;
  argument.expression.reset(new DependentExpression(expression));
  return argument;
}

inline TemplateArgument TemplateArgument::dependent_expression_arg(
    const DependentExpression & expression)
{
  TemplateArgument argument;
  argument.kind = TAK_DEPENDENT_EXPRESSION;
  argument.expression.reset(new DependentExpression(expression));
  return argument;
}

struct FunctionNameComponent
{
  std::string source_name;
  std::string substitution_name;
  std::string complete_substitution_name;
  std::string standard_substitution;
  bool std_abbrev = false;
  bool standard_substitution_includes_arguments = false;
  std::vector<TemplateArgument> template_arguments;

  static FunctionNameComponent source(const std::string & name,
                                      const std::string & substitution)
  {
    FunctionNameComponent component;
    component.source_name = name;
    component.substitution_name = substitution;
    return component;
  }

  static FunctionNameComponent std_namespace()
  {
    FunctionNameComponent component;
    component.source_name = "std";
    component.std_abbrev = true;
    return component;
  }

  static FunctionNameComponent template_component(
      const std::string & name,
      const std::string & substitution,
      const std::string & complete_substitution,
      const std::vector<TemplateArgument> & arguments,
      const std::string & standard_substitution,
      bool standard_substitution_includes_arguments)
  {
    FunctionNameComponent component;
    component.source_name = name;
    component.substitution_name = substitution;
    component.complete_substitution_name = complete_substitution;
    component.template_arguments = arguments;
    component.standard_substitution = standard_substitution;
    component.standard_substitution_includes_arguments =
        standard_substitution_includes_arguments;
    return component;
  }
};

struct FunctionEncoding
{
  struct LambdaMetadata
  {
    std::string context_fragment;
    std::vector<SubstitutionSlot> context_substitution_slots;
    std::string source_name;
    std::vector<Type> signature_parameter_types;
    std::string discriminator;
  };

  std::string name_fragment;
  std::vector<FunctionNameComponent> name_components;
  std::string terminal_fragment;
  Type conversion_type;
  bool has_conversion_type = false;
  std::vector<TemplateArgument> template_arguments;
  SubstitutionKey template_prefix_key;
  std::vector<std::string> abi_tags;
  std::vector<Type> parameter_types;
  std::shared_ptr<LambdaMetadata> lambda;
  bool variadic = false;
  bool nested_const = false;
  bool nested_volatile = false;
  bool nested_lvalue_ref = false;
  bool nested_rvalue_ref = false;

  static LambdaMetadata & ensure_lambda_metadata(FunctionEncoding & function)
  {
    if(!function.lambda || !function.lambda.unique()) {
      function.lambda.reset(function.lambda ?
          new LambdaMetadata(*function.lambda) :
          new LambdaMetadata);
    }
    return *function.lambda;
  }
};

inline void set_substitution(Type & type, const SubstitutionKey & key)
{
  if(!type.substitution || !type.substitution.unique()) {
    type.substitution.reset(type.substitution ?
        new Type::SubstitutionMetadata(*type.substitution) :
        new Type::SubstitutionMetadata);
  }
  type.substitution->key = key;
}

inline Type with_substitution(Type type, const SubstitutionKey & key)
{
  set_substitution(type, key);
  return type;
}

inline void add_preregister_legacy_key(Type & type, const std::string & key)
{
  if(key.empty()) {
    return;
  }
  if(!type.substitution || !type.substitution.unique()) {
    type.substitution.reset(type.substitution ?
        new Type::SubstitutionMetadata(*type.substitution) :
        new Type::SubstitutionMetadata);
  }
  type.substitution->preregister_legacy_keys.push_back(key);
}

inline const SubstitutionKey & type_substitution_key(const Type & type)
{
  static const SubstitutionKey empty_key;
  return type.substitution ? type.substitution->key : empty_key;
}

inline bool type_has_substitution(const Type & type)
{
  return !type_substitution_key(type).empty();
}

inline bool make_type_substitution_key(const Type & type, SubstitutionKey & out);
inline bool make_template_argument_substitution_key(
    const TemplateArgument & argument,
    SubstitutionKey & out);
inline bool make_class_template_argument_substitution_key(
    const Type::ClassTemplateArgument & argument,
    SubstitutionKey & out);
inline bool make_dependent_expression_substitution_key(
    const DependentExpression & expression,
    SubstitutionKey & out);

inline bool make_type_substitution_key(const Type & type, SubstitutionKey & out)
{
  if(type_has_substitution(type)) {
    out = type_substitution_key(type);
    return true;
  }
  switch(type.kind) {
  case Type::TK_BUILTIN:
    if(type.builtin_code[0] == '\0') {
      return false;
    }
    out = SubstitutionKey::type_builtin(std::string(type.builtin_code));
    return true;

  case Type::TK_CV: {
    if(!type.inner) {
      return false;
    }
    SubstitutionKey inner_key;
    if(!make_type_substitution_key(*type.inner, inner_key)) {
      return false;
    }
    out = SubstitutionKey::type_cv(type.cv_const, type.cv_volatile, inner_key);
    return true;
  }

  case Type::TK_POINTER: {
    if(!type.inner) {
      return false;
    }
    SubstitutionKey inner_key;
    if(!make_type_substitution_key(*type.inner, inner_key)) {
      return false;
    }
    out = SubstitutionKey::type_pointer(inner_key);
    return true;
  }

  case Type::TK_LVALUE_REFERENCE: {
    if(!type.inner) {
      return false;
    }
    SubstitutionKey inner_key;
    if(!make_type_substitution_key(*type.inner, inner_key)) {
      return false;
    }
    out = SubstitutionKey::type_lvalue_reference(inner_key);
    return true;
  }

  case Type::TK_RVALUE_REFERENCE: {
    if(!type.inner) {
      return false;
    }
    SubstitutionKey inner_key;
    if(!make_type_substitution_key(*type.inner, inner_key)) {
      return false;
    }
    out = SubstitutionKey::type_rvalue_reference(inner_key);
    return true;
  }

  case Type::TK_ARRAY: {
    if(!type.inner) {
      return false;
    }
    SubstitutionKey inner_key;
    if(!make_type_substitution_key(*type.inner, inner_key)) {
      return false;
    }
    const std::string bound_key =
        type.array_substitution_bound_key.empty() ?
            type.array_bound :
            type.array_substitution_bound_key;
    out = SubstitutionKey::type_array(bound_key, inner_key);
    return true;
  }

  case Type::TK_FUNCTION: {
    if(!type.inner) {
      return false;
    }
    SubstitutionKey result_key;
    if(!make_type_substitution_key(*type.inner, result_key)) {
      return false;
    }
    std::vector<SubstitutionKey> param_keys;
    param_keys.reserve(type.params.size());
    for(std::size_t i = 0; i < type.params.size(); ++i) {
      SubstitutionKey param_key;
      if(!make_type_substitution_key(type.params[i], param_key)) {
        return false;
      }
      param_keys.push_back(param_key);
    }
    out = SubstitutionKey::type_function(result_key, param_keys, type.variadic);
    return true;
  }

  case Type::TK_MEMBER_POINTER: {
    if(!type.owner || !type.inner) {
      return false;
    }
    SubstitutionKey owner_key;
    SubstitutionKey member_key;
    if(!make_type_substitution_key(*type.owner, owner_key) ||
       !make_type_substitution_key(*type.inner, member_key)) {
      return false;
    }
    out = SubstitutionKey::type_member_pointer(owner_key, member_key);
    return true;
  }

  case Type::TK_BUILTIN_TYPE_TRANSFORM: {
    if(type.builtin_transform_name.empty() || !type.inner) {
      return false;
    }
    SubstitutionKey inner_key;
    if(!make_type_substitution_key(*type.inner, inner_key)) {
      return false;
    }
    out = SubstitutionKey::type(
        std::string("builtin-type-transform:") +
        type.builtin_transform_name + ":" + inner_key.structural_text());
    return true;
  }

  case Type::TK_PACK_EXPANSION: {
    if(!type.inner) {
      return false;
    }
    SubstitutionKey inner_key;
    if(!make_type_substitution_key(*type.inner, inner_key)) {
      return false;
    }
    std::vector<SubstitutionKey> children;
    children.push_back(inner_key);
    out = SubstitutionKey::type(
        std::string("pack-expansion:") + inner_key.structural_text());
    return true;
  }

  case Type::TK_TEMPLATE_PARAMETER:
    out = SubstitutionKey::type_template_parameter(
        type.template_parameter_index);
    return true;

  case Type::TK_NAMED:
    if(!type_has_substitution(type)) {
      return false;
    }
    out = type_substitution_key(type);
    return true;

  case Type::TK_CLASS_TEMPLATE_SPECIALIZATION:
    if(!type_has_substitution(type)) {
      return false;
    }
    out = type_substitution_key(type);
    return true;

  case Type::TK_DECLTYPE_EXPRESSION:
    if(!type.expression) {
      return false;
    }
    {
      SubstitutionKey expression_key;
      if(!make_dependent_expression_substitution_key(*type.expression,
                                                     expression_key)) {
        return false;
      }
      out = SubstitutionKey::type(std::string("decltype(") +
                                  expression_key.structural_text() + ")");
      return true;
    }

  case Type::TK_LAMBDA_CLOSURE:
    return false;

  case Type::TK_INVALID:
    return false;
  }
  return false;
}

inline bool make_class_template_argument_substitution_key(
    const Type::ClassTemplateArgument & argument,
    SubstitutionKey & out)
{
  switch(argument.kind) {
  case Type::ClassTemplateArgument::CTAK_TYPE:
    if(!argument.type) {
      return false;
    }
    {
      SubstitutionKey type_key;
      if(!make_type_substitution_key(*argument.type, type_key)) {
        return false;
      }
      out = SubstitutionKey::template_argument_type(type_key);
      return true;
    }

  case Type::ClassTemplateArgument::CTAK_INTEGRAL_VALUE:
    if(!argument.type) {
      return false;
    }
    {
      SubstitutionKey type_key;
      if(!make_type_substitution_key(*argument.type, type_key)) {
        return false;
      }
      out = SubstitutionKey::template_argument_value(
          std::string("integral:") + type_key.structural_text() + ":" +
          std::to_string(argument.integral_value));
      return true;
    }

  case Type::ClassTemplateArgument::CTAK_DEPENDENT_INTEGRAL_VALUE:
    if(!argument.parameter_type) {
      return false;
    }
    {
      SubstitutionKey parameter_type_key;
      if(!make_type_substitution_key(*argument.parameter_type,
                                     parameter_type_key)) {
        return false;
      }
      std::string payload = std::string("dependent-integral:") +
                            parameter_type_key.structural_text() + ":";
      if(argument.type) {
        SubstitutionKey value_type_key;
        if(!make_type_substitution_key(*argument.type, value_type_key)) {
          return false;
        }
        payload += value_type_key.structural_text();
      }
      payload += ":";
      payload += std::to_string(argument.integral_value);
      out = SubstitutionKey::template_argument_value(payload);
      return true;
    }

  case Type::ClassTemplateArgument::CTAK_DEPENDENT_EXPRESSION:
    if(!argument.expression) {
      return false;
    }
    {
      SubstitutionKey expression_key;
      if(!make_dependent_expression_substitution_key(*argument.expression,
                                                     expression_key)) {
        return false;
      }
      out = SubstitutionKey::template_argument_value(
          std::string("dependent-expression:") +
          expression_key.structural_text());
      return true;
    }

  case Type::ClassTemplateArgument::CTAK_UNTYPED_INTEGRAL_VALUE:
    out = SubstitutionKey::template_argument_value(
        std::string("untyped-integral:") +
        std::to_string(argument.integral_value));
    return true;

  case Type::ClassTemplateArgument::CTAK_TEMPLATE_ENTITY:
    if(!argument.metadata) {
      return false;
    }
    out = SubstitutionKey::template_argument_template(
        0,
        argument.metadata->template_name_substitution.empty() ?
            argument.metadata->template_name :
            argument.metadata->template_name_substitution);
    return !out.empty();

  case Type::ClassTemplateArgument::CTAK_EXTERNAL_ENTITY:
    if(!argument.metadata) {
      return false;
    }
    out = SubstitutionKey::template_argument_value(
        std::string("external:") +
        (argument.metadata->external_entity_address_of ? "address:" : "reference:") +
        argument.metadata->external_entity_symbol);
    return !argument.metadata->external_entity_symbol.empty();

  case Type::ClassTemplateArgument::CTAK_ARGUMENT_PACK: {
    if(!argument.metadata) {
      return false;
    }
    std::string payload = "pack:";
    for(std::size_t i = 0; i < argument.metadata->pack_arguments.size(); ++i) {
      SubstitutionKey child_key;
      if(!make_class_template_argument_substitution_key(
             argument.metadata->pack_arguments[i],
             child_key)) {
        return false;
      }
      if(i != 0) {
        payload += ',';
      }
      payload += child_key.structural_text();
    }
    out = SubstitutionKey::template_argument_value(payload);
    return true;
  }

  case Type::ClassTemplateArgument::CTAK_INVALID:
    return false;
  }
  return false;
}

inline bool make_template_argument_substitution_key(
    const TemplateArgument & argument,
    SubstitutionKey & out)
{
  switch(argument.kind) {
  case TemplateArgument::TAK_TYPE: {
    SubstitutionKey type_key;
    if(!argument.value_type ||
       !make_type_substitution_key(*argument.value_type, type_key)) {
      return false;
    }
    out = SubstitutionKey::template_argument_type(type_key);
    return true;
  }

  case TemplateArgument::TAK_INTEGRAL_VALUE: {
    SubstitutionKey type_key;
    if(!argument.value_type ||
       !make_type_substitution_key(*argument.value_type, type_key)) {
      return false;
    }
    out = SubstitutionKey::template_argument_value(
        std::string("integral:") + type_key.structural_text() + ":" +
        std::to_string(argument.integral_value));
    return true;
  }

  case TemplateArgument::TAK_DEPENDENT_INTEGRAL_VALUE: {
    SubstitutionKey parameter_type_key;
    if(!argument.parameter_type ||
       !make_type_substitution_key(*argument.parameter_type,
                                   parameter_type_key)) {
      return false;
    }
    std::string payload = std::string("dependent-integral:") +
                          parameter_type_key.structural_text() + ":";
    if(argument.value_type) {
      SubstitutionKey value_type_key;
      if(!make_type_substitution_key(*argument.value_type, value_type_key)) {
        return false;
      }
      payload += value_type_key.structural_text();
    }
    payload += ":";
    payload += std::to_string(argument.integral_value);
    out = SubstitutionKey::template_argument_value(payload);
    return true;
  }

  case TemplateArgument::TAK_DEPENDENT_EXPRESSION:
    if(!argument.expression) {
      return false;
    }
    {
      SubstitutionKey expression_key;
      if(!make_dependent_expression_substitution_key(*argument.expression,
                                                     expression_key)) {
        return false;
      }
      out = SubstitutionKey::template_argument_value(
          std::string("dependent-expression:") +
          expression_key.structural_text());
      return true;
    }

  case TemplateArgument::TAK_UNTYPED_INTEGRAL_VALUE:
    out = SubstitutionKey::template_argument_value(
        std::string("untyped-integral:") +
        std::to_string(argument.integral_value));
    return true;

  case TemplateArgument::TAK_TEMPLATE_ENTITY:
    if(!argument.metadata) {
      return false;
    }
    out = SubstitutionKey::template_argument_template(
        0,
        argument.metadata->template_name_substitution.empty() ?
            argument.metadata->template_name :
            argument.metadata->template_name_substitution);
    return !out.empty();

  case TemplateArgument::TAK_EXTERNAL_ENTITY:
    if(!argument.metadata) {
      return false;
    }
    out = SubstitutionKey::template_argument_value(
        std::string("external:") +
        (argument.metadata->external_entity_address_of ? "address:" : "reference:") +
        argument.metadata->external_entity_symbol);
    return !argument.metadata->external_entity_symbol.empty();

  case TemplateArgument::TAK_ARGUMENT_PACK: {
    if(!argument.metadata) {
      return false;
    }
    std::string payload = "pack:";
    for(std::size_t i = 0; i < argument.metadata->pack_arguments.size(); ++i) {
      SubstitutionKey child_key;
      if(!make_template_argument_substitution_key(
             argument.metadata->pack_arguments[i],
             child_key)) {
        return false;
      }
      if(i != 0) {
        payload += ',';
      }
      payload += child_key.structural_text();
    }
    out = SubstitutionKey::template_argument_value(payload);
    return true;
  }

  case TemplateArgument::TAK_INVALID:
    return false;
  }
  return false;
}

inline bool make_dependent_expression_substitution_key(
    const DependentExpression & expression,
    SubstitutionKey & out)
{
  switch(expression.kind) {
  case DependentExpression::EK_TEMPLATE_PARAMETER:
    out = SubstitutionKey::template_argument_value(
        std::string("expr-template-parameter:") +
        std::to_string(expression.template_parameter_index));
    return true;

  case DependentExpression::EK_FUNCTION_PARAMETER:
    out = SubstitutionKey::template_argument_value(
        std::string("expr-function-parameter:") +
        std::to_string(expression.template_parameter_index));
    return true;

  case DependentExpression::EK_LITERAL:
    if(expression.text.empty()) {
      return false;
    }
    out = SubstitutionKey::template_argument_value(
        std::string("expr-literal:") +
        (expression.text[0] == '+' ? expression.text.substr(1) :
                                     expression.text));
    return true;

  case DependentExpression::EK_INTEGRAL_VALUE:
    if(!expression.owner_type) {
      return false;
    }
    {
      SubstitutionKey type_key;
      if(!make_type_substitution_key(*expression.owner_type, type_key)) {
        return false;
      }
      out = SubstitutionKey::template_argument_value(
          std::string("expr-integral:") + type_key.structural_text() + ":" +
          std::to_string(expression.integral_value));
      return true;
    }

  case DependentExpression::EK_MEMBER:
    if(!expression.owner_type || expression.text.empty()) {
      return false;
    }
    {
      SubstitutionKey owner_key;
      if(!make_type_substitution_key(*expression.owner_type, owner_key)) {
        return false;
      }
      std::string payload =
          std::string("expr-member:") + owner_key.structural_text() + ":" +
          (expression.close_member_owner ? "close:" : "open:") +
          expression.text;
      if(!expression.template_arguments.empty()) {
        payload += '<';
        for(std::size_t i = 0; i < expression.template_arguments.size(); ++i) {
          SubstitutionKey argument_key;
          if(!make_template_argument_substitution_key(
                 expression.template_arguments[i],
                 argument_key)) {
            return false;
          }
          if(i != 0) {
            payload += ',';
          }
          payload += argument_key.structural_text();
        }
        payload += '>';
      }
      out = SubstitutionKey::template_argument_value(payload);
      return true;
    }

  case DependentExpression::EK_OBJECT_MEMBER:
    if(expression.op_code.empty() ||
       !expression.inner ||
       expression.text.empty()) {
      return false;
    }
    {
      SubstitutionKey object_key;
      if(!make_dependent_expression_substitution_key(*expression.inner,
                                                     object_key)) {
        return false;
      }
      std::string payload =
          std::string("expr-object-member:") + expression.op_code + ":" +
          object_key.structural_text() + ":" + expression.text;
      if(!expression.template_arguments.empty()) {
        payload += '<';
        for(std::size_t i = 0; i < expression.template_arguments.size(); ++i) {
          SubstitutionKey argument_key;
          if(!make_template_argument_substitution_key(
                 expression.template_arguments[i],
                 argument_key)) {
            return false;
          }
          if(i != 0) {
            payload += ',';
          }
          payload += argument_key.structural_text();
        }
        payload += '>';
      }
      out = SubstitutionKey::template_argument_value(payload);
      return true;
    }

  case DependentExpression::EK_UNARY:
    if(expression.op_code.empty() || !expression.inner) {
      return false;
    }
    {
      SubstitutionKey inner_key;
      if(!make_dependent_expression_substitution_key(*expression.inner,
                                                     inner_key)) {
        return false;
      }
      out = SubstitutionKey::template_argument_value(
          std::string("expr-unary:") + expression.op_code + ":" +
          inner_key.structural_text());
      return true;
    }

  case DependentExpression::EK_BINARY:
    if(expression.op_code.empty() ||
       !expression.inner ||
       expression.arguments.size() != 1) {
      return false;
    }
    {
      SubstitutionKey left_key;
      SubstitutionKey right_key;
      if(!make_dependent_expression_substitution_key(*expression.inner,
                                                     left_key) ||
         !make_dependent_expression_substitution_key(expression.arguments[0],
                                                     right_key)) {
        return false;
      }
      out = SubstitutionKey::template_argument_value(
          std::string("expr-binary:") + expression.op_code + ":" +
          left_key.structural_text() + ":" + right_key.structural_text());
      return true;
    }

  case DependentExpression::EK_PACK_EXPANSION:
    if(!expression.inner) {
      return false;
    }
    {
      SubstitutionKey inner_key;
      if(!make_dependent_expression_substitution_key(*expression.inner,
                                                     inner_key)) {
        return false;
      }
      out = SubstitutionKey::template_argument_value(
          std::string("expr-pack-expansion:") + inner_key.structural_text());
      return true;
    }

  case DependentExpression::EK_CALL:
    if(!expression.inner) {
      return false;
    }
    {
      SubstitutionKey callee_key;
      if(!make_dependent_expression_substitution_key(*expression.inner,
                                                     callee_key)) {
        return false;
      }
      std::string payload = std::string("expr-call:") +
                            callee_key.structural_text() + "(";
      for(std::size_t i = 0; i < expression.arguments.size(); ++i) {
        SubstitutionKey argument_key;
        if(!make_dependent_expression_substitution_key(expression.arguments[i],
                                                       argument_key)) {
          return false;
        }
        if(i != 0) {
          payload += ',';
        }
        payload += argument_key.structural_text();
      }
      payload += ')';
      out = SubstitutionKey::template_argument_value(payload);
      return true;
    }

  case DependentExpression::EK_CONVERSION:
    if(!expression.owner_type) {
      return false;
    }
    {
      SubstitutionKey type_key;
      if(!make_type_substitution_key(*expression.owner_type, type_key)) {
        return false;
      }
      std::string payload =
          std::string("expr-conversion:") +
          (expression.op_code.empty() ? std::string("cv") : expression.op_code) +
          ":" + type_key.structural_text() + "(";
      for(std::size_t i = 0; i < expression.arguments.size(); ++i) {
        SubstitutionKey argument_key;
        if(!make_dependent_expression_substitution_key(expression.arguments[i],
                                                       argument_key)) {
          return false;
        }
        if(i != 0) {
          payload += ',';
        }
        payload += argument_key.structural_text();
      }
      payload += ')';
      out = SubstitutionKey::template_argument_value(payload);
      return true;
    }

  case DependentExpression::EK_TEMPLATE_ID: {
    if(expression.text.empty()) {
      return false;
    }
    std::string payload = std::string("expr-template-id:") +
                          expression.text + "<";
    for(std::size_t i = 0; i < expression.template_arguments.size(); ++i) {
      SubstitutionKey argument_key;
      if(!make_template_argument_substitution_key(
             expression.template_arguments[i],
             argument_key)) {
        return false;
      }
      if(i != 0) {
        payload += ',';
      }
      payload += argument_key.structural_text();
    }
    payload += '>';
    out = SubstitutionKey::template_argument_value(payload);
    return true;
  }

  case DependentExpression::EK_TYPE_TRAIT: {
    if(expression.text.empty()) {
      return false;
    }
    std::string payload = std::string("expr-type-trait:") +
                          expression.text + "(";
    for(std::size_t i = 0; i < expression.type_arguments.size(); ++i) {
      SubstitutionKey type_key;
      if(!make_type_substitution_key(expression.type_arguments[i], type_key)) {
        return false;
      }
      if(i != 0) {
        payload += ',';
      }
      payload += type_key.structural_text();
    }
    payload += ')';
    out = SubstitutionKey::template_argument_value(payload);
    return true;
  }

  case DependentExpression::EK_SIZEOF_TYPE:
    if(!expression.owner_type) {
      return false;
    }
    {
      SubstitutionKey type_key;
      if(!make_type_substitution_key(*expression.owner_type, type_key)) {
        return false;
      }
      out = SubstitutionKey::template_argument_value(
          std::string("expr-sizeof-type:") + type_key.structural_text());
      return true;
    }

  case DependentExpression::EK_EXTERNAL_ENTITY:
    if(expression.text.empty()) {
      return false;
    }
    out = SubstitutionKey::template_argument_value(
        std::string("expr-external:") +
        (expression.external_entity_address_of ? "address:" : "reference:") +
        expression.text);
    return true;

  case DependentExpression::EK_INVALID:
    return false;
  }
  return false;
}

inline bool emit_type(const Type & type, std::string & out, SubstitutionSink * sink);
inline bool emit_class_template_argument(
    const Type::ClassTemplateArgument & argument,
    std::string & out,
    SubstitutionSink * sink);
inline bool emit_class_template_arguments(
    const std::vector<Type::ClassTemplateArgument> & arguments,
    std::string & out,
    SubstitutionSink * sink);
inline bool emit_template_argument(const TemplateArgument & argument,
                                   std::string & out,
                                   SubstitutionSink * sink);
inline bool emit_template_arguments(
    const std::vector<TemplateArgument> & arguments,
    std::string & out,
    SubstitutionSink * sink);
inline bool emit_dependent_expression_body(const DependentExpression & expression,
                                           std::string & out,
                                           SubstitutionSink * sink);
inline bool emit_type_as_name_prefix(const Type & type,
                                     std::string & out,
                                     SubstitutionSink * sink);
inline bool emit_type_as_name_prefix_body(const Type & type,
                                          std::string & out,
                                          SubstitutionSink * sink);
inline bool emit_type_as_member_expression_owner_prefix_body(
    const Type & type,
    std::string & out,
    SubstitutionSink * sink);
inline bool emit_source_name(const std::string & name, std::string & out);
inline void emit_abi_tags(const std::vector<std::string> & abi_tags,
                          std::string & out);
inline bool emit_function_name(const FunctionEncoding & function,
                               std::string & out,
                               SubstitutionSink * sink);
inline bool emit_function_encoding(const FunctionEncoding & function,
                                   std::string & out,
                                   SubstitutionSink * sink);

inline bool type_needs_member_expression_template_name_registration(
    const Type & type);

inline bool class_template_argument_needs_member_expression_template_name_registration(
    const Type::ClassTemplateArgument & argument)
{
  if(argument.type &&
     type_needs_member_expression_template_name_registration(*argument.type)) {
    return true;
  }
  if(argument.parameter_type &&
     type_needs_member_expression_template_name_registration(
         *argument.parameter_type)) {
    return true;
  }
  if(argument.metadata) {
    for(std::size_t i = 0; i < argument.metadata->pack_arguments.size(); ++i) {
      if(class_template_argument_needs_member_expression_template_name_registration(
             argument.metadata->pack_arguments[i])) {
        return true;
      }
    }
  }
  return false;
}

inline bool template_arguments_need_member_expression_template_name_registration(
    const std::vector<Type::ClassTemplateArgument> & arguments)
{
  for(std::size_t i = 0; i < arguments.size(); ++i) {
    if(class_template_argument_needs_member_expression_template_name_registration(
           arguments[i])) {
      return true;
    }
  }
  return false;
}

inline bool type_needs_member_expression_template_name_registration(
    const Type & type)
{
  const Type::NameMetadata * metadata = type.name.get();
  if(type.kind == Type::TK_CLASS_TEMPLATE_SPECIALIZATION &&
     metadata &&
     (metadata->register_member_expression_template_name ||
      template_arguments_need_member_expression_template_name_registration(
          metadata->template_arguments))) {
    return true;
  }
  if(type.inner &&
     type_needs_member_expression_template_name_registration(*type.inner)) {
    return true;
  }
  if(type.owner &&
     type_needs_member_expression_template_name_registration(*type.owner)) {
    return true;
  }
  if(type.name_owner &&
     type_needs_member_expression_template_name_registration(*type.name_owner)) {
    return true;
  }
  for(std::size_t i = 0; i < type.params.size(); ++i) {
    if(type_needs_member_expression_template_name_registration(type.params[i])) {
      return true;
    }
  }
  return false;
}

struct LookupOnlySubstitutionSink : public SubstitutionSink
{
  explicit LookupOnlySubstitutionSink(SubstitutionSink * inner) : inner(inner) {}

  bool emit_substitution(const SubstitutionKey & key, std::string & out) override
  {
    return inner && inner->emit_substitution(key, out);
  }

  void register_substitution(const SubstitutionKey & key) override
  {
    (void)key;
  }

  bool suppress_template_parameter_type_substitution_in_template_argument()
      const override
  {
    return inner &&
           inner->suppress_template_parameter_type_substitution_in_template_argument();
  }

  SubstitutionSink * inner;
};

struct NoLookupSubstitutionSink : public SubstitutionSink
{
  explicit NoLookupSubstitutionSink(SubstitutionSink * inner) : inner(inner) {}

  bool emit_substitution(const SubstitutionKey &, std::string &) override
  {
    return false;
  }

  void register_substitution(const SubstitutionKey &) override
  {
  }

  bool suppress_template_parameter_type_substitution_in_template_argument()
      const override
  {
    return inner &&
           inner->suppress_template_parameter_type_substitution_in_template_argument();
  }

  SubstitutionSink * inner;
};

inline bool emit_name_component(const Type::NameComponent & component,
                                std::string & out,
                                SubstitutionSink * sink)
{
  if(component.std_abbrev) {
    out += "St";
    return true;
  }
  if(component.source_name.empty()) {
    return false;
  }
  const SubstitutionKey key =
      component.substitution_name.empty() ?
          SubstitutionKey::none() :
          SubstitutionKey::named(component.substitution_name);
  if(sink && !key.empty() && sink->emit_substitution(key, out)) {
    return true;
  }
  static const char anonymous_enum_prefix[] = "__anonymous_enum";
  if(component.source_name.compare(0,
                                   sizeof(anonymous_enum_prefix) - 1,
                                   anonymous_enum_prefix) == 0) {
    out += "Ut_";
  } else if(!emit_source_name(component.source_name, out)) {
    return false;
  }
  if(sink && !key.empty()) {
    sink->register_substitution(key);
  }
  return true;
}

inline bool emit_name_prefix_components(
    const std::vector<Type::NameComponent> & components,
    std::string & out,
    SubstitutionSink * sink)
{
  std::size_t start = 0;
  if(sink) {
    for(std::size_t len = components.size(); len > 0; --len) {
      const std::string & substitution = components[len - 1].substitution_name;
      if(substitution.empty()) {
        continue;
      }
      if(sink->emit_substitution(SubstitutionKey::named(substitution), out)) {
        start = len;
        break;
      }
    }
  }
  for(std::size_t i = start; i < components.size(); ++i) {
    if(!emit_name_component(components[i], out, sink)) {
      return false;
    }
  }
  return true;
}

inline bool emit_template_name_component(const Type & type,
                                         std::string & out,
                                         SubstitutionSink * sink)
{
  const Type::NameMetadata * metadata = type.name.get();
  if(!metadata || metadata->template_name.empty()) {
    return false;
  }
  const SubstitutionKey key =
      metadata->template_name_substitution.empty() ?
          SubstitutionKey::none() :
          SubstitutionKey::named(metadata->template_name_substitution);
  if(sink && !key.empty() && sink->emit_substitution(key, out)) {
    return true;
  }
  if(!emit_source_name(metadata->template_name, out)) {
    return false;
  }
  if(sink && !key.empty()) {
    sink->register_substitution(key);
  }
  return true;
}

inline bool emit_type_as_name_prefix_body(const Type & type,
                                          std::string & out,
                                          SubstitutionSink * sink)
{
  switch(type.kind) {
  case Type::TK_TEMPLATE_PARAMETER:
    out += 'T';
    if(type.template_parameter_index > 0) {
      out += std::to_string(type.template_parameter_index - 1);
    }
    out += '_';
    return true;

  case Type::TK_NAMED: {
    const Type::NameMetadata * metadata = type.name.get();
    if(!metadata || metadata->template_name.empty()) {
      return false;
    }
    if(type.name_owner) {
      if(!emit_type_as_name_prefix(*type.name_owner, out, sink)) {
        return false;
      }
    } else if(!emit_name_prefix_components(metadata->prefix_components, out, sink)) {
      return false;
    }
    return emit_name_component(
        Type::NameComponent::source(metadata->template_name,
                                    metadata->template_name_substitution),
        out,
        sink);
  }

  case Type::TK_CLASS_TEMPLATE_SPECIALIZATION: {
    const Type::NameMetadata * metadata = type.name.get();
    if(!metadata) {
      return false;
    }
    if(!metadata->standard_substitution.empty()) {
      out += metadata->standard_substitution;
      if(metadata->standard_substitution_includes_arguments) {
        return true;
      }
      return emit_class_template_arguments(metadata->template_arguments, out, sink);
    }
    if(metadata->template_name.empty()) {
      return false;
    }
    if(type.name_owner) {
      if(!emit_type_as_name_prefix(*type.name_owner, out, sink)) {
        return false;
      }
    } else if(!emit_name_prefix_components(metadata->prefix_components, out, sink)) {
      return false;
    }
    return emit_template_name_component(type, out, sink) &&
           emit_class_template_arguments(metadata->template_arguments, out, sink);
  }

  default:
    return false;
  }
}

inline bool emit_type_as_member_expression_owner_prefix_body(
    const Type & type,
    std::string & out,
    SubstitutionSink * sink)
{
  NoLookupSubstitutionSink no_lookup_sink(sink);
  SubstitutionSink * const owner_name_sink = sink ? &no_lookup_sink : nullptr;
  switch(type.kind) {
  case Type::TK_NAMED: {
    const Type::NameMetadata * metadata = type.name.get();
    if(!metadata || metadata->template_name.empty()) {
      return false;
    }
    if(type.name_owner) {
      if(!emit_type_as_name_prefix(*type.name_owner, out, owner_name_sink)) {
        return false;
      }
    } else if(!emit_name_prefix_components(metadata->prefix_components,
                                           out,
                                           owner_name_sink)) {
      return false;
    }
    return emit_source_name(metadata->template_name, out);
  }

  case Type::TK_CLASS_TEMPLATE_SPECIALIZATION: {
    const Type::NameMetadata * metadata = type.name.get();
    if(!metadata) {
      return false;
    }
    if(!metadata->standard_substitution.empty()) {
      out += metadata->standard_substitution;
      if(metadata->standard_substitution_includes_arguments) {
        return true;
      }
      return emit_class_template_arguments(metadata->template_arguments, out, sink);
    }
    if(metadata->template_name.empty()) {
      return false;
    }
    if(type.name_owner) {
      if(!emit_type_as_name_prefix(*type.name_owner, out, owner_name_sink)) {
        return false;
      }
    } else if(!emit_name_prefix_components(metadata->prefix_components,
                                           out,
                                           owner_name_sink)) {
      return false;
    }
    if(!emit_source_name(metadata->template_name, out)) {
      return false;
    }
    if(sink &&
       (metadata->register_member_expression_template_name ||
        template_arguments_need_member_expression_template_name_registration(
            metadata->template_arguments)) &&
       !metadata->template_name_substitution.empty()) {
      sink->register_substitution(
          SubstitutionKey::named(metadata->template_name_substitution));
    }
    if(!emit_class_template_arguments(metadata->template_arguments, out, sink)) {
      return false;
    }
    return true;
  }

  default:
    return emit_type(type, out, sink);
  }
}

inline bool emit_type_as_unqualified_member_expression_owner_prefix_body(
    const Type & type,
    std::string & out,
    SubstitutionSink * sink)
{
  switch(type.kind) {
  case Type::TK_NAMED: {
    const Type::NameMetadata * metadata = type.name.get();
    if(!metadata || metadata->template_name.empty()) {
      return false;
    }
    return emit_source_name(metadata->template_name, out);
  }

  case Type::TK_CLASS_TEMPLATE_SPECIALIZATION: {
    const Type::NameMetadata * metadata = type.name.get();
    if(!metadata || metadata->template_name.empty()) {
      return false;
    }
    if(!emit_source_name(metadata->template_name, out)) {
      return false;
    }
    return emit_class_template_arguments(metadata->template_arguments, out, sink);
  }

  default:
    return emit_type_as_member_expression_owner_prefix_body(type, out, sink);
  }
}

inline bool emit_type_as_name_prefix(const Type & type,
                                     std::string & out,
                                     SubstitutionSink * sink)
{
  const SubstitutionKey & substitution_key = type_substitution_key(type);
  if(sink && !substitution_key.empty() &&
     sink->emit_substitution(substitution_key, out)) {
    return true;
  }

  const std::size_t begin = out.size();
  if(!emit_type_as_name_prefix_body(type, out, sink)) {
    out.resize(begin);
    return false;
  }

  if(sink && !substitution_key.empty()) {
    sink->register_substitution(substitution_key);
  }
  return true;
}

inline bool emit_named_type(const Type & type,
                            std::string & out,
                            SubstitutionSink * sink)
{
  const Type::NameMetadata * metadata = type.name.get();
  if(!metadata || metadata->template_name.empty()) {
    return false;
  }
  if(type.name_owner) {
    out += 'N';
    if(!emit_type_as_name_prefix_body(type, out, sink)) {
      return false;
    }
    out += 'E';
    return true;
  }
  const bool direct_std_prefix =
      metadata->prefix_components.size() == 1 &&
      metadata->prefix_components[0].std_abbrev;
  if(direct_std_prefix) {
    out += "St";
    return emit_name_component(
        Type::NameComponent::source(metadata->template_name,
                                    metadata->template_name_substitution),
        out,
        sink);
  }

  const bool nested = !metadata->prefix_components.empty();
  if(nested) {
    out += 'N';
  }
  if(!emit_name_prefix_components(metadata->prefix_components, out, sink)) {
    return false;
  }
  if(!emit_name_component(
         Type::NameComponent::source(metadata->template_name,
                                     metadata->template_name_substitution),
         out,
         sink)) {
    return false;
  }
  if(nested) {
    out += 'E';
  }
  return true;
}

inline bool class_template_argument_is_simple_parameter_ref(
    const Type::ClassTemplateArgument & argument)
{
  if(argument.kind != Type::ClassTemplateArgument::CTAK_TYPE ||
     !argument.type) {
    return false;
  }
  SubstitutionKey key;
  return make_type_substitution_key(*argument.type, key) &&
         key.kind == SubstitutionKey::SK_TYPE_TEMPLATE_PARAMETER;
}

inline bool substitution_key_contains_template_parameter(
    const SubstitutionKey & key)
{
  if(key.kind == SubstitutionKey::SK_TYPE_TEMPLATE_PARAMETER) {
    return true;
  }
  for(std::size_t i = 0; i < key.children.size(); ++i) {
    if(substitution_key_contains_template_parameter(key.children[i])) {
      return true;
    }
  }
  return false;
}

inline bool type_contains_template_parameter_ref(const Type & type);

inline bool class_template_argument_shape_contains_template_parameter_ref(
    const Type::ClassTemplateArgument & argument)
{
  switch(argument.kind) {
  case Type::ClassTemplateArgument::CTAK_TYPE:
    return argument.type &&
           type_contains_template_parameter_ref(*argument.type);

  case Type::ClassTemplateArgument::CTAK_INTEGRAL_VALUE:
    return argument.type &&
           type_contains_template_parameter_ref(*argument.type);

  case Type::ClassTemplateArgument::CTAK_DEPENDENT_INTEGRAL_VALUE:
    return (argument.parameter_type &&
            type_contains_template_parameter_ref(*argument.parameter_type)) ||
           (argument.type &&
            type_contains_template_parameter_ref(*argument.type));

  case Type::ClassTemplateArgument::CTAK_ARGUMENT_PACK:
    if(!argument.metadata) {
      return false;
    }
    for(std::size_t i = 0; i < argument.metadata->pack_arguments.size(); ++i) {
      if(class_template_argument_shape_contains_template_parameter_ref(
             argument.metadata->pack_arguments[i])) {
        return true;
      }
    }
    return false;

  case Type::ClassTemplateArgument::CTAK_DEPENDENT_EXPRESSION:
  case Type::ClassTemplateArgument::CTAK_UNTYPED_INTEGRAL_VALUE:
  case Type::ClassTemplateArgument::CTAK_TEMPLATE_ENTITY:
  case Type::ClassTemplateArgument::CTAK_EXTERNAL_ENTITY:
  case Type::ClassTemplateArgument::CTAK_INVALID:
    return false;
  }
  return false;
}

inline bool type_contains_template_parameter_ref(const Type & type)
{
  if(type.kind == Type::TK_TEMPLATE_PARAMETER) {
    return true;
  }
  const Type::NameMetadata * metadata = type.name.get();
  if(type.kind == Type::TK_CLASS_TEMPLATE_SPECIALIZATION &&
     metadata &&
     metadata->template_name_is_template_parameter) {
    return true;
  }
  if(type.inner && type_contains_template_parameter_ref(*type.inner)) {
    return true;
  }
  if(type.owner && type_contains_template_parameter_ref(*type.owner)) {
    return true;
  }
  if(type.name_owner &&
     type_contains_template_parameter_ref(*type.name_owner)) {
    return true;
  }
  for(std::size_t i = 0; i < type.params.size(); ++i) {
    if(type_contains_template_parameter_ref(type.params[i])) {
      return true;
    }
  }
  if(metadata) {
    for(std::size_t i = 0; i < metadata->template_arguments.size(); ++i) {
      if(class_template_argument_shape_contains_template_parameter_ref(
             metadata->template_arguments[i])) {
        return true;
      }
    }
  }
  return false;
}

inline bool class_template_argument_contains_template_parameter_ref(
    const Type::ClassTemplateArgument & argument)
{
  if(class_template_argument_shape_contains_template_parameter_ref(argument)) {
    return true;
  }
  SubstitutionKey key;
  return make_class_template_argument_substitution_key(argument, key) &&
         substitution_key_contains_template_parameter(key);
}

inline bool class_template_arguments_contain_template_parameter_ref(
    const std::vector<Type::ClassTemplateArgument> & arguments)
{
  for(std::size_t i = 0; i < arguments.size(); ++i) {
    if(class_template_argument_contains_template_parameter_ref(arguments[i])) {
      return true;
    }
  }
  return false;
}

inline bool class_template_arguments_are_only_simple_parameter_refs(
    const std::vector<Type::ClassTemplateArgument> & arguments)
{
  if(arguments.empty()) {
    return false;
  }
  for(std::size_t i = 0; i < arguments.size(); ++i) {
    if(!class_template_argument_is_simple_parameter_ref(arguments[i])) {
      return false;
    }
  }
  return true;
}

inline bool type_contains_pack_expansion(const Type & type);

inline bool class_template_argument_contains_pack_expansion(
    const Type::ClassTemplateArgument & argument)
{
  if(argument.type && type_contains_pack_expansion(*argument.type)) {
    return true;
  }
  if(argument.parameter_type &&
     type_contains_pack_expansion(*argument.parameter_type)) {
    return true;
  }
  if(argument.kind == Type::ClassTemplateArgument::CTAK_ARGUMENT_PACK) {
    return true;
  }
  if(argument.metadata) {
    for(std::size_t i = 0; i < argument.metadata->pack_arguments.size(); ++i) {
      if(class_template_argument_contains_pack_expansion(
             argument.metadata->pack_arguments[i])) {
        return true;
      }
    }
  }
  return false;
}

inline bool type_contains_pack_expansion(const Type & type)
{
  if(type.kind == Type::TK_PACK_EXPANSION) {
    return true;
  }
  if(type.inner && type_contains_pack_expansion(*type.inner)) {
    return true;
  }
  if(type.owner && type_contains_pack_expansion(*type.owner)) {
    return true;
  }
  if(type.name_owner && type_contains_pack_expansion(*type.name_owner)) {
    return true;
  }
  for(std::size_t i = 0; i < type.params.size(); ++i) {
    if(type_contains_pack_expansion(type.params[i])) {
      return true;
    }
  }
  const Type::NameMetadata * metadata = type.name.get();
  if(metadata) {
    for(std::size_t i = 0; i < metadata->template_arguments.size(); ++i) {
      if(class_template_argument_contains_pack_expansion(
             metadata->template_arguments[i])) {
        return true;
      }
    }
  }
  return false;
}

inline bool class_template_arguments_contain_pack_expansion(
    const std::vector<Type::ClassTemplateArgument> & arguments)
{
  for(std::size_t i = 0; i < arguments.size(); ++i) {
    if(class_template_argument_contains_pack_expansion(arguments[i])) {
      return true;
    }
  }
  return false;
}

inline bool emit_class_template_specialization_type(const Type & type,
                                                    std::string & out,
                                                    SubstitutionSink * sink)
{
  const Type::NameMetadata * metadata = type.name.get();
  if(!metadata) {
    return false;
  }
  if(type.name_owner) {
    out += 'N';
    if(!emit_type_as_name_prefix_body(type, out, sink)) {
      return false;
    }
    out += 'E';
    return true;
  }

  if(!metadata->standard_substitution.empty()) {
    out += metadata->standard_substitution;
    if(metadata->standard_substitution_includes_arguments) {
      return true;
    }
    return emit_class_template_arguments(metadata->template_arguments, out, sink);
  }

  const bool direct_std_prefix =
      metadata->prefix_components.size() == 1 &&
      metadata->prefix_components[0].std_abbrev;
  const bool nested = !metadata->prefix_components.empty() && !direct_std_prefix;
  const SubstitutionKey template_key =
      metadata->template_name_is_template_parameter ?
          SubstitutionKey::type_template_parameter(
              metadata->template_name_parameter_index) :
      metadata->template_name_substitution.empty() ?
          SubstitutionKey::none() :
          SubstitutionKey::named(metadata->template_name_substitution);
  const SubstitutionKey template_prefix_key =
      nested &&
              !metadata->template_name_substitution.empty() &&
              class_template_arguments_contain_pack_expansion(
                  metadata->template_arguments) ?
          SubstitutionKey::prefix(metadata->template_name_substitution) :
          SubstitutionKey::none();
  if(nested) {
    out += 'N';
  }
  if(sink && !template_key.empty() && sink->emit_substitution(template_key, out)) {
    if(!template_prefix_key.empty()) {
      sink->register_substitution(template_prefix_key);
    }
    if(!emit_class_template_arguments(metadata->template_arguments, out, sink)) {
      return false;
    }
    if(nested) {
      out += 'E';
    }
    return true;
  }
  if(!emit_name_prefix_components(metadata->prefix_components, out, sink)) {
    return false;
  }
  if(metadata->template_name_is_template_parameter) {
    if(sink && !template_key.empty() &&
       sink->emit_substitution(template_key, out)) {
      if(!emit_class_template_arguments(metadata->template_arguments, out, sink)) {
        return false;
      }
      if(nested) {
        out += 'E';
      }
      return true;
    }
    out += 'T';
    if(metadata->template_name_parameter_index > 0) {
      out += std::to_string(metadata->template_name_parameter_index - 1);
    }
    out += '_';
  } else if(!emit_source_name(metadata->template_name, out)) {
    return false;
  }
  if(sink && !template_key.empty()) {
    sink->register_substitution(template_key);
  }
  if(sink && !template_prefix_key.empty()) {
    sink->register_substitution(template_prefix_key);
  }
  if(!emit_class_template_arguments(metadata->template_arguments, out, sink)) {
    return false;
  }
  if(nested) {
    out += 'E';
  }
  return true;
}

inline bool emit_template_entity_name(
    const std::vector<Type::NameComponent> & prefix_components,
    const std::string & template_name,
    const std::string & template_name_substitution,
    std::string & out,
    SubstitutionSink * sink)
{
  if(template_name.empty()) {
    return false;
  }
  const bool direct_std_prefix =
      prefix_components.size() == 1 &&
      prefix_components[0].std_abbrev;
  if(direct_std_prefix) {
    out += "St";
    return emit_name_component(
        Type::NameComponent::source(template_name, template_name_substitution),
        out,
        sink);
  }

  const bool nested = !prefix_components.empty();
  if(nested) {
    out += 'N';
  }
  if(!emit_name_prefix_components(prefix_components, out, sink) ||
     !emit_name_component(
         Type::NameComponent::source(template_name, template_name_substitution),
         out,
         sink)) {
    return false;
  }
  if(nested) {
    out += 'E';
  }
  return true;
}

inline bool emit_external_entity_argument(const std::string & symbol,
                                          bool address_of,
                                          std::string & out)
{
  if(symbol.empty()) {
    return false;
  }
  if(address_of) {
    out += "Xad";
  }
  out += 'L';
  out += symbol;
  out += 'E';
  if(address_of) {
    out += 'E';
  }
  return true;
}

inline bool emit_integral_template_value(const Type * value_type,
                                         long long value,
                                         std::string & out,
                                         SubstitutionSink * sink)
{
  if(value_type) {
    const std::size_t begin = out.size();
    out += 'L';
    if(!emit_type(*value_type, out, sink)) {
      out.resize(begin);
      return false;
    }
    out += std::to_string(value);
    out += 'E';
    return true;
  }

  out += "Li";
  out += std::to_string(value);
  out += 'E';
  return true;
}

inline void register_lambda_context_substitutions(
    const std::vector<SubstitutionSlot> & slots,
    SubstitutionSink * sink)
{
  if(!sink) {
    return;
  }
  for(std::size_t i = 0; i < slots.size(); ++i) {
    if(!slots[i].ir_key.empty()) {
      sink->register_substitution(slots[i].ir_key);
    }
    if(!slots[i].legacy_key.empty()) {
      sink->register_substitution(SubstitutionKey::legacy(slots[i].legacy_key));
    }
  }
}

inline bool emit_type_body(const Type & type, std::string & out, SubstitutionSink * sink)
{
  switch(type.kind) {
  case Type::TK_BUILTIN:
    if(type.builtin_code[0] == '\0') {
      return false;
    }
    out += type.builtin_code;
    return true;

  case Type::TK_CV:
    if(!type.inner) {
      return false;
    }
    if(type.cv_const) {
      out += 'K';
    }
    if(type.cv_volatile) {
      out += 'V';
    }
    return emit_type(*type.inner, out, sink);

  case Type::TK_POINTER:
    if(!type.inner) {
      return false;
    }
    out += 'P';
    return emit_type(*type.inner, out, sink);

  case Type::TK_LVALUE_REFERENCE:
    if(!type.inner) {
      return false;
    }
    out += 'R';
    return emit_type(*type.inner, out, sink);

  case Type::TK_RVALUE_REFERENCE:
    if(!type.inner) {
      return false;
    }
    out += 'O';
    return emit_type(*type.inner, out, sink);

  case Type::TK_ARRAY:
    if(!type.inner) {
      return false;
    }
    out += 'A';
    out += type.array_bound;
    out += '_';
    return emit_type(*type.inner, out, sink);

  case Type::TK_FUNCTION:
    if(!type.inner) {
      return false;
    }
    out += 'F';
    if(!emit_type(*type.inner, out, sink)) {
      return false;
    }
    if(type.params.empty()) {
      out += type.variadic ? 'z' : 'v';
    } else {
      for(std::size_t i = 0; i < type.params.size(); ++i) {
        if(!emit_type(type.params[i], out, sink)) {
          return false;
        }
      }
      if(type.variadic) {
        out += 'z';
      }
    }
    out += 'E';
    return true;

  case Type::TK_MEMBER_POINTER:
    if(!type.owner || !type.inner) {
      return false;
    }
    out += 'M';
    return emit_type(*type.owner, out, sink) &&
           emit_type(*type.inner, out, sink);

  case Type::TK_BUILTIN_TYPE_TRANSFORM:
    if(type.builtin_transform_name.empty() || !type.inner) {
      return false;
    }
    out += 'u';
    if(!emit_source_name(type.builtin_transform_name, out)) {
      return false;
    }
    out += 'I';
    if(!emit_type(*type.inner, out, sink)) {
      return false;
    }
    out += 'E';
    return true;

  case Type::TK_PACK_EXPANSION:
    if(!type.inner) {
      return false;
    }
    out += "Dp";
    return emit_type(*type.inner, out, sink);

  case Type::TK_TEMPLATE_PARAMETER:
    out += 'T';
    if(type.template_parameter_index > 0) {
      out += std::to_string(type.template_parameter_index - 1);
    }
    out += '_';
    return true;

  case Type::TK_NAMED:
    return emit_named_type(type, out, sink);

  case Type::TK_CLASS_TEMPLATE_SPECIALIZATION:
    return emit_class_template_specialization_type(type, out, sink);

  case Type::TK_DECLTYPE_EXPRESSION:
    if(!type.expression) {
      return false;
    }
    out += "DT";
    if(!emit_dependent_expression_body(*type.expression, out, sink)) {
      return false;
    }
    out += 'E';
    return true;

  case Type::TK_LAMBDA_CLOSURE:
    if(!type.lambda || type.lambda->context_fragment.empty()) {
      return false;
    }
    register_lambda_context_substitutions(
        type.lambda->context_substitution_slots,
        sink);
    out += type.lambda->context_fragment;
    if(!type.lambda->source_name.empty()) {
      if(!emit_source_name(type.lambda->source_name, out)) {
        return false;
      }
    } else {
      out += "Ul";
      if(type.params.empty()) {
        out += 'v';
      } else {
        for(std::size_t i = 0; i < type.params.size(); ++i) {
          if(!emit_type(type.params[i], out, sink)) {
            return false;
          }
        }
      }
      out += 'E';
      out += type.lambda->discriminator;
      out += '_';
    }
    return true;

  case Type::TK_INVALID:
    return false;
  }
  return false;
}

inline bool emit_type(const Type & type, std::string & out, SubstitutionSink * sink)
{
  const SubstitutionKey & substitution_key = type_substitution_key(type);
  if(sink && !substitution_key.empty() &&
     sink->emit_substitution(substitution_key, out)) {
    return true;
  }

  if(sink && type.substitution) {
    const std::vector<std::string> & preregister_keys =
        type.substitution->preregister_legacy_keys;
    for(std::size_t i = 0; i < preregister_keys.size(); ++i) {
      sink->register_substitution(SubstitutionKey::legacy(preregister_keys[i]));
    }
  }

  const std::size_t begin = out.size();
  if(!emit_type_body(type, out, sink)) {
    out.resize(begin);
    return false;
  }

  if(sink && !substitution_key.empty()) {
    sink->register_substitution(substitution_key);
  }
  return true;
}

inline bool SubstitutionSink::emit_dependent_parameter_type(
    const Type & type,
    std::string & out)
{
  return emit_type(type, out, this);
}

struct SuppressTemplateParameterTypeSubstitutionSink : SubstitutionSink
{
  explicit SuppressTemplateParameterTypeSubstitutionSink(SubstitutionSink * inner)
      : inner(inner)
  {
  }

  bool emit_substitution(const SubstitutionKey & key,
                         std::string & out) override
  {
    if(key.kind == SubstitutionKey::SK_TYPE_TEMPLATE_PARAMETER) {
      return false;
    }
    return inner && inner->emit_substitution(key, out);
  }

  void register_substitution(const SubstitutionKey & key) override
  {
    if(key.kind == SubstitutionKey::SK_TYPE_TEMPLATE_PARAMETER || !inner) {
      return;
    }
    inner->register_substitution(key);
  }

  bool suppress_template_parameter_type_substitution_in_template_argument()
      const override
  {
    return true;
  }

  SubstitutionSink * inner;
};

struct TypeTraitOperandSubstitutionSink : SubstitutionSink
{
  explicit TypeTraitOperandSubstitutionSink(SubstitutionSink * inner)
      : inner(inner)
  {
  }

  bool emit_substitution(const SubstitutionKey & key,
                         std::string & out) override
  {
    return inner && inner->emit_substitution(key, out);
  }

  void register_substitution(const SubstitutionKey & key) override
  {
    if(inner) {
      inner->register_substitution(key);
    }
  }

  bool suppress_template_parameter_type_substitution_in_template_argument()
      const override
  {
    return true;
  }

  SubstitutionSink * inner;
};

inline bool emit_class_template_argument(
    const Type::ClassTemplateArgument & argument,
    std::string & out,
    SubstitutionSink * sink)
{
  switch(argument.kind) {
  case Type::ClassTemplateArgument::CTAK_TYPE: {
    if(!argument.type) {
      return false;
    }
    SuppressTemplateParameterTypeSubstitutionSink argument_sink(sink);
    return emit_type(
        *argument.type,
        out,
        sink &&
                sink->suppress_template_parameter_type_substitution_in_template_argument() ?
            &argument_sink :
            sink);
  }

  case Type::ClassTemplateArgument::CTAK_INTEGRAL_VALUE: {
    if(!argument.type) {
      return false;
    }
    return emit_integral_template_value(argument.type.get(),
                                        argument.integral_value,
                                        out,
                                        sink);
  }

  case Type::ClassTemplateArgument::CTAK_DEPENDENT_INTEGRAL_VALUE:
    if(!argument.parameter_type) {
      return false;
    }
    out += "Tn";
    if(!sink ? !emit_type(*argument.parameter_type, out, sink) :
              !sink->emit_dependent_parameter_type(*argument.parameter_type, out)) {
      return false;
    }
    return emit_integral_template_value(argument.type.get(),
                                        argument.integral_value,
                                        out,
                                        sink);

  case Type::ClassTemplateArgument::CTAK_DEPENDENT_EXPRESSION:
    if(!argument.expression) {
      return false;
    }
    out += 'X';
    if(!emit_dependent_expression_body(*argument.expression, out, sink)) {
      return false;
    }
    out += 'E';
    return true;

  case Type::ClassTemplateArgument::CTAK_UNTYPED_INTEGRAL_VALUE:
    return emit_integral_template_value(nullptr, argument.integral_value, out, sink);

  case Type::ClassTemplateArgument::CTAK_TEMPLATE_ENTITY:
    return argument.metadata &&
           emit_template_entity_name(argument.metadata->prefix_components,
                                     argument.metadata->template_name,
                                     argument.metadata->template_name_substitution,
                                     out,
                                     sink);

  case Type::ClassTemplateArgument::CTAK_EXTERNAL_ENTITY:
    return argument.metadata &&
           emit_external_entity_argument(argument.metadata->external_entity_symbol,
                                         argument.metadata->external_entity_address_of,
                                         out);

  case Type::ClassTemplateArgument::CTAK_ARGUMENT_PACK:
    if(!argument.metadata) {
      return false;
    }
    out += 'J';
    for(std::size_t i = 0; i < argument.metadata->pack_arguments.size(); ++i) {
      if(!emit_class_template_argument(argument.metadata->pack_arguments[i], out, sink)) {
        return false;
      }
    }
    out += 'E';
    return true;

  case Type::ClassTemplateArgument::CTAK_INVALID:
    return false;
  }
  return false;
}

inline bool emit_class_template_arguments(
    const std::vector<Type::ClassTemplateArgument> & arguments,
    std::string & out,
    SubstitutionSink * sink)
{
  out += 'I';
  for(std::size_t i = 0; i < arguments.size(); ++i) {
    if(!emit_class_template_argument(arguments[i], out, sink)) {
      return false;
    }
  }
  out += 'E';
  return true;
}

inline bool emit_template_argument(const TemplateArgument & argument,
                                   std::string & out,
                                   SubstitutionSink * sink)
{
  switch(argument.kind) {
  case TemplateArgument::TAK_TYPE:
    return argument.value_type &&
           emit_type(*argument.value_type, out, sink);

  case TemplateArgument::TAK_INTEGRAL_VALUE: {
    return argument.value_type &&
           emit_integral_template_value(argument.value_type.get(),
                                        argument.integral_value,
                                        out,
                                        sink);
  }

  case TemplateArgument::TAK_DEPENDENT_INTEGRAL_VALUE:
    if(!argument.parameter_type) {
      return false;
    }
    out += "Tn";
    if(!sink ? !emit_type(*argument.parameter_type, out, sink) :
              !sink->emit_dependent_parameter_type(*argument.parameter_type, out)) {
      return false;
    }
    return emit_integral_template_value(argument.value_type.get(),
                                        argument.integral_value,
                                        out,
                                        sink);

  case TemplateArgument::TAK_DEPENDENT_EXPRESSION:
    if(!argument.expression) {
      return false;
    }
    out += 'X';
    if(!emit_dependent_expression_body(*argument.expression, out, sink)) {
      return false;
    }
    out += 'E';
    return true;

  case TemplateArgument::TAK_UNTYPED_INTEGRAL_VALUE:
    return emit_integral_template_value(nullptr, argument.integral_value, out, sink);

  case TemplateArgument::TAK_TEMPLATE_ENTITY:
    return argument.metadata &&
           emit_template_entity_name(argument.metadata->prefix_components,
                                     argument.metadata->template_name,
                                     argument.metadata->template_name_substitution,
                                     out,
                                     sink);

  case TemplateArgument::TAK_EXTERNAL_ENTITY:
    return argument.metadata &&
           emit_external_entity_argument(argument.metadata->external_entity_symbol,
                                         argument.metadata->external_entity_address_of,
                                         out);

  case TemplateArgument::TAK_ARGUMENT_PACK:
    if(!argument.metadata) {
      return false;
    }
    out += 'J';
    for(std::size_t i = 0; i < argument.metadata->pack_arguments.size(); ++i) {
      if(!emit_template_argument(argument.metadata->pack_arguments[i], out, sink)) {
        return false;
      }
    }
    out += 'E';
    return true;

  case TemplateArgument::TAK_INVALID:
    return false;
  }
  return false;
}

inline bool emit_template_arguments(
    const std::vector<TemplateArgument> & arguments,
    std::string & out,
    SubstitutionSink * sink)
{
  out += 'I';
  for(std::size_t i = 0; i < arguments.size(); ++i) {
    if(!emit_template_argument(arguments[i], out, sink)) {
      return false;
    }
  }
  out += 'E';
  return true;
}

inline bool emit_source_name(const std::string & name, std::string & out)
{
  if(name.empty()) {
    return false;
  }
  out += std::to_string(name.size());
  out += name;
  return true;
}

inline bool emit_dependent_expression_body(const DependentExpression & expression,
                                           std::string & out,
                                           SubstitutionSink * sink)
{
  switch(expression.kind) {
  case DependentExpression::EK_TEMPLATE_PARAMETER:
    out += 'T';
    if(expression.template_parameter_index > 0) {
      out += std::to_string(expression.template_parameter_index - 1);
    }
    out += '_';
    return true;

  case DependentExpression::EK_FUNCTION_PARAMETER:
    out += "fp";
    if(expression.template_parameter_index > 0) {
      out += std::to_string(expression.template_parameter_index - 1);
    }
    out += '_';
    return true;

  case DependentExpression::EK_LITERAL:
    if(expression.text.empty()) {
      return false;
    }
    out += "Li";
    out += expression.text[0] == '+' ? expression.text.substr(1) :
                                       expression.text;
    out += 'E';
    return true;

  case DependentExpression::EK_INTEGRAL_VALUE:
    if(!expression.owner_type) {
      return false;
    }
    return emit_integral_template_value(expression.owner_type.get(),
                                        expression.integral_value,
                                        out,
                                        sink);

  case DependentExpression::EK_MEMBER:
    if(!expression.owner_type || expression.text.empty()) {
      return false;
    }
    out += "sr";
    if(expression.suppress_member_owner_prefix ?
           !emit_type_as_unqualified_member_expression_owner_prefix_body(
               *expression.owner_type,
               out,
               sink) :
           !emit_type_as_member_expression_owner_prefix_body(
               *expression.owner_type,
               out,
               sink)) {
      return false;
    }
    if(expression.close_member_owner) {
      out += 'E';
    }
    if(!emit_source_name(expression.text, out)) {
      return false;
    }
    if(!expression.template_arguments.empty()) {
      if(sink && !expression.suppress_template_prefix_substitution) {
        sink->register_substitution(
            SubstitutionKey::function_template_prefix(expression.text));
      }
      if(!emit_template_arguments(expression.template_arguments, out, sink)) {
        return false;
      }
    }
    return true;

  case DependentExpression::EK_OBJECT_MEMBER:
    if(expression.op_code.empty() ||
       !expression.inner ||
       expression.text.empty()) {
      return false;
    }
    out += expression.op_code;
    if(!emit_dependent_expression_body(*expression.inner, out, sink) ||
       !emit_source_name(expression.text, out)) {
      return false;
    }
    if(!expression.template_arguments.empty()) {
      if(sink && !expression.suppress_template_prefix_substitution) {
        sink->register_substitution(
            SubstitutionKey::function_template_prefix(expression.text));
      }
      if(!emit_template_arguments(expression.template_arguments, out, sink)) {
        return false;
      }
    }
    return true;

  case DependentExpression::EK_UNARY:
    if(expression.op_code.empty() || !expression.inner) {
      return false;
    }
    out += expression.op_code;
    return emit_dependent_expression_body(*expression.inner, out, sink);

  case DependentExpression::EK_BINARY:
    if(expression.op_code.empty() ||
       !expression.inner ||
       expression.arguments.size() != 1) {
      return false;
    }
    out += expression.op_code;
    return emit_dependent_expression_body(*expression.inner, out, sink) &&
           emit_dependent_expression_body(expression.arguments[0], out, sink);

  case DependentExpression::EK_PACK_EXPANSION:
    if(!expression.inner) {
      return false;
    }
    out += "sp";
    return emit_dependent_expression_body(*expression.inner, out, sink);

  case DependentExpression::EK_CALL:
    if(!expression.inner) {
      return false;
    }
    out += "cl";
    if(!emit_dependent_expression_body(*expression.inner, out, sink)) {
      return false;
    }
    for(std::size_t i = 0; i < expression.arguments.size(); ++i) {
      if(!emit_dependent_expression_body(expression.arguments[i], out, sink)) {
        return false;
      }
    }
    out += 'E';
    return true;

  case DependentExpression::EK_CONVERSION:
    if(!expression.owner_type) {
      return false;
    }
    {
      const std::string op_code =
          expression.op_code.empty() ? std::string("cv") : expression.op_code;
      out += op_code;
      if(!emit_type(*expression.owner_type, out, sink)) {
        return false;
      }
      if(op_code == "cv") {
        out += '_';
        for(std::size_t i = 0; i < expression.arguments.size(); ++i) {
          if(!emit_dependent_expression_body(expression.arguments[i], out, sink)) {
            return false;
          }
        }
        out += 'E';
        return true;
      }
      if(expression.arguments.size() != 1 ||
         !emit_dependent_expression_body(expression.arguments[0], out, sink)) {
        return false;
      }
      return true;
    }

  case DependentExpression::EK_TEMPLATE_ID:
    if(expression.text.empty() ||
       !emit_source_name(expression.text, out)) {
      return false;
    }
    if(sink && !expression.suppress_template_prefix_substitution) {
      sink->register_substitution(
          SubstitutionKey::function_template_prefix(expression.text));
    }
    if(!emit_template_arguments(expression.template_arguments, out, sink)) {
      return false;
    }
    return true;

  case DependentExpression::EK_TYPE_TRAIT: {
    if(expression.text.empty()) {
      return false;
    }
    out += 'u';
    if(!emit_source_name(expression.text, out)) {
      return false;
    }
    TypeTraitOperandSubstitutionSink type_trait_sink(sink);
    for(std::size_t i = 0; i < expression.type_arguments.size(); ++i) {
      if(!emit_type(expression.type_arguments[i],
                    out,
                    sink ? &type_trait_sink : sink)) {
        return false;
      }
    }
    out += 'E';
    return true;
  }

  case DependentExpression::EK_SIZEOF_TYPE:
    if(!expression.owner_type) {
      return false;
    }
    out += "st";
    return emit_type(*expression.owner_type, out, sink);

  case DependentExpression::EK_EXTERNAL_ENTITY:
    return emit_external_entity_argument(expression.text,
                                         expression.external_entity_address_of,
                                         out);

  case DependentExpression::EK_INVALID:
    return false;
  }
  return false;
}

inline void emit_abi_tags(const std::vector<std::string> & abi_tags,
                          std::string & out)
{
  if(abi_tags.empty()) {
    return;
  }

  std::vector<std::string> sorted = abi_tags;
  for(std::size_t i = 1; i < sorted.size(); ++i) {
    std::string value = sorted[i];
    std::size_t j = i;
    while(j > 0 && value < sorted[j - 1]) {
      sorted[j] = sorted[j - 1];
      --j;
    }
    sorted[j] = value;
  }

  std::vector<std::string> unique_tags;
  unique_tags.reserve(sorted.size());
  for(std::size_t i = 0; i < sorted.size(); ++i) {
    if(sorted[i].empty()) {
      continue;
    }
    if(!unique_tags.empty() && unique_tags.back() == sorted[i]) {
      continue;
    }
    unique_tags.push_back(sorted[i]);
  }

  for(std::size_t i = 0; i < unique_tags.size(); ++i) {
    out += 'B';
    out += std::to_string(unique_tags[i].size());
    out += unique_tags[i];
  }
}

inline SubstitutionKey function_name_component_prefix_key(
    const FunctionNameComponent & component)
{
  if(!component.standard_substitution.empty() &&
     component.standard_substitution_includes_arguments) {
    return SubstitutionKey::none();
  }
  if(!component.complete_substitution_name.empty()) {
    return SubstitutionKey::named(component.complete_substitution_name);
  }
  if(!component.substitution_name.empty()) {
    return SubstitutionKey::named(component.substitution_name);
  }
  return SubstitutionKey::none();
}

inline bool emit_function_name_component(const FunctionNameComponent & component,
                                         std::string & out,
                                         SubstitutionSink * sink)
{
  if(component.std_abbrev) {
    out += "St";
    return true;
  }
  if(component.source_name.empty()) {
    return false;
  }

  if(!component.standard_substitution.empty()) {
    out += component.standard_substitution;
    if(!component.standard_substitution_includes_arguments &&
       !emit_template_arguments(component.template_arguments, out, sink)) {
      return false;
    }
    if(sink &&
       !component.standard_substitution_includes_arguments &&
       !component.complete_substitution_name.empty()) {
      sink->register_substitution(
          SubstitutionKey::named(component.complete_substitution_name));
    }
    return true;
  }

  const SubstitutionKey name_key =
      component.substitution_name.empty() ?
          SubstitutionKey::none() :
          SubstitutionKey::named(component.substitution_name);
  if(sink && !name_key.empty() && sink->emit_substitution(name_key, out)) {
    if(!component.template_arguments.empty() &&
       !emit_template_arguments(component.template_arguments, out, sink)) {
      return false;
    }
    if(sink && !component.complete_substitution_name.empty()) {
      sink->register_substitution(
          SubstitutionKey::named(component.complete_substitution_name));
    }
    return true;
  }
  if(!emit_source_name(component.source_name, out)) {
    return false;
  }
  if(sink && !name_key.empty()) {
    sink->register_substitution(name_key);
  }
  if(!component.template_arguments.empty() &&
     !emit_template_arguments(component.template_arguments, out, sink)) {
    return false;
  }
  if(sink && !component.complete_substitution_name.empty()) {
    sink->register_substitution(
        SubstitutionKey::named(component.complete_substitution_name));
  }
  return true;
}

inline bool emit_function_name_prefix_components(
    const std::vector<FunctionNameComponent> & components,
    std::string & out,
    SubstitutionSink * sink)
{
  std::size_t start = 0;
  if(sink) {
    for(std::size_t len = components.size(); len > 0; --len) {
      const SubstitutionKey key =
          function_name_component_prefix_key(components[len - 1]);
      if(key.empty()) {
        continue;
      }
      if(sink->emit_substitution(key, out)) {
        start = len;
        break;
      }
    }
  }
  for(std::size_t i = start; i < components.size(); ++i) {
    if(!emit_function_name_component(components[i], out, sink)) {
      return false;
    }
  }
  return true;
}

inline bool emit_function_name(const FunctionEncoding & function,
                               std::string & out,
                               SubstitutionSink * sink)
{
  if(function.lambda) {
    const FunctionEncoding::LambdaMetadata & lambda = *function.lambda;
    if(lambda.context_fragment.empty()) {
      return false;
    }
    out += lambda.context_fragment;
    out += 'N';
    if(function.nested_const) {
      out += 'K';
    }
    if(function.nested_volatile) {
      out += 'V';
    }
    if(function.nested_lvalue_ref) {
      out += 'R';
    } else if(function.nested_rvalue_ref) {
      out += 'O';
    }
    register_lambda_context_substitutions(
        lambda.context_substitution_slots,
        sink);
    const std::size_t signature_begin = out.size();
    if(!lambda.source_name.empty()) {
      if(!emit_source_name(lambda.source_name, out)) {
        return false;
      }
    } else {
      out += "Ul";
      if(lambda.signature_parameter_types.empty()) {
        out += 'v';
      } else {
        for(std::size_t i = 0;
            i < lambda.signature_parameter_types.size();
            ++i) {
          if(!emit_type(lambda.signature_parameter_types[i],
                        out,
                        sink)) {
            return false;
          }
        }
      }
      out += 'E';
      out += lambda.discriminator;
      out += '_';
    }
    if(sink) {
      sink->register_substitution(
          SubstitutionKey::legacy(
              std::string("lambda-closure:") +
              lambda.context_fragment +
              out.substr(signature_begin)));
    }
    out += "cl";
    emit_abi_tags(function.abi_tags, out);
    if(!function.template_arguments.empty()) {
      if(sink && !function.template_prefix_key.empty()) {
        sink->register_substitution(function.template_prefix_key);
      }
      if(!emit_template_arguments(function.template_arguments, out, sink)) {
        return false;
      }
    }
    out += 'E';
    return true;
  }

  if(!function.name_components.empty()) {
    const bool direct_std_prefix =
        function.name_components.size() == 2 &&
        function.name_components[0].std_abbrev;
    const bool nested = function.name_components.size() > 1 && !direct_std_prefix;
    if(nested) {
      out += 'N';
      if(function.nested_const) {
        out += 'K';
      }
      if(function.nested_volatile) {
        out += 'V';
      }
      if(function.nested_lvalue_ref) {
        out += 'R';
      } else if(function.nested_rvalue_ref) {
        out += 'O';
      }
    }
    const std::size_t terminal = function.name_components.size() - 1;
    if(direct_std_prefix) {
      out += "St";
    } else if(terminal != 0) {
      std::vector<FunctionNameComponent> prefix_components(
          function.name_components.begin(),
          function.name_components.begin() + terminal);
      if(!emit_function_name_prefix_components(prefix_components, out, sink)) {
        return false;
      }
    }
    if(function.has_conversion_type) {
      out += "cv";
      if(!emit_type(function.conversion_type,
                    out,
                    nullptr)) {
        return false;
      }
    } else if(!function.terminal_fragment.empty()) {
      out += function.terminal_fragment;
    } else if(!emit_function_name_component(function.name_components[terminal],
                                            out,
                                            sink)) {
      return false;
    }
    emit_abi_tags(function.abi_tags, out);
    if(!function.template_arguments.empty()) {
      if(sink && !function.template_prefix_key.empty()) {
        sink->register_substitution(function.template_prefix_key);
      }
      if(!emit_template_arguments(function.template_arguments, out, sink)) {
        return false;
      }
    }
    if(nested) {
      out += 'E';
    }
    return true;
  }
  if(function.name_fragment.empty()) {
    return false;
  }
  if(function.has_conversion_type) {
    out += "cv";
    if(!emit_type(function.conversion_type,
                  out,
                  nullptr)) {
      return false;
    }
  } else {
    out += function.name_fragment;
  }
  emit_abi_tags(function.abi_tags, out);
  if(!function.template_arguments.empty()) {
    if(sink && !function.template_prefix_key.empty()) {
      sink->register_substitution(function.template_prefix_key);
    }
    if(!emit_template_arguments(function.template_arguments, out, sink)) {
      return false;
    }
  }
  return true;
}

inline bool emit_function_encoding(const FunctionEncoding & function,
                                   std::string & out,
                                   SubstitutionSink * sink)
{
  out += "_Z";
  if(!emit_function_name(function, out, sink)) {
    return false;
  }
  if(function.parameter_types.empty()) {
    out += function.variadic ? 'z' : 'v';
    return true;
  }
  for(std::size_t i = 0; i < function.parameter_types.size(); ++i) {
    if(!emit_type(function.parameter_types[i], out, sink)) {
      return false;
    }
  }
  if(function.variadic) {
    out += 'z';
  }
  return true;
}

}  // namespace itanium_mangle_ir
