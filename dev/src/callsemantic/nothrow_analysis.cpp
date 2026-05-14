#include "callsemantic/nothrow_analysis.h"

#include "callsemantic/type_trait_analysis.h"
#include "callsemantic_internal.h"
#include "semantic_class_model.h"

#include <stdexcept>
#include <string>
#include <vector>

namespace callsemantic {

using callsemantic_internal::find_child_kind;
using cpp_decl::Type;
using cpp_decl::TypePtr;
using semantic_conversion::ExprInfo;
using semantic_model::ClassInfo;
using semantic_model::FieldInfo;
using semantic_model::FunctionBinding;
using semantic_model::Scope;

TypeTraitCallbacks NothrowCallbacks::type_traits() const
{
  return make_type_trait_callbacks();
}

bool initializer_is_nothrow(const TypePtr & target,
                            Scope & scope,
                            const CppAstNode & initializer,
                            std::set<FunctionBinding *> & visiting,
                            const NothrowCallbacks & callbacks)
{
  const CppAstNode * payload = &initializer;
  if(initializer.kind == CppAstKind::initializer &&
     initializer.children.size() == 1) {
    payload = &initializer.children[0];
  }

  if(payload->kind == CppAstKind::braced_init_list &&
     payload->children.empty()) {
    return default_initialization_is_nothrow(target, scope, visiting, callbacks);
  }

  TypePtr target_base = strip_top_level_cv(remove_reference_type(target));
  if(payload->kind == CppAstKind::braced_init_list && target_base) {
    if(target_base->kind == Type::TK_ARRAY) {
      for(std::size_t i = 0; i < payload->children.size(); ++i) {
        if(!initializer_is_nothrow(target_base->inner,
                                   scope,
                                   payload->children[i],
                                   visiting,
                                   callbacks)) {
          return false;
        }
      }
      return true;
    }

    ClassInfo * aggregate_info =
        callbacks.type_traits().class_info_for_type(target_base);
    if(aggregate_info &&
       semantic_class_model::can_synthesize_aggregate_constructor(
           *aggregate_info)) {
      const std::size_t aggregate_count =
          semantic_class_model::aggregate_element_count(*aggregate_info);
      if(payload->children.size() > aggregate_count) {
        return false;
      }
      for(std::size_t i = 0; i < aggregate_info->fields.size(); ++i) {
        const FieldInfo & field = aggregate_info->fields[i];
        const FieldInfo * input_field = &field;
        if(field.is_anonymous_storage) {
          ClassInfo * storage_info =
              callbacks.type_traits().class_info_for_type(field.type);
          if(storage_info && !storage_info->fields.empty()) {
            input_field = &storage_info->fields[0];
          }
        }
        if(i >= payload->children.size() || i >= aggregate_count) {
          break;
        }
        if(!initializer_is_nothrow(input_field->type,
                                   scope,
                                   payload->children[i],
                                   visiting,
                                   callbacks)) {
          return false;
        }
        if(aggregate_info->class_kind == "union") {
          break;
        }
      }
      for(std::size_t i = payload->children.size();
          i < aggregate_info->fields.size();
          ++i) {
        const FieldInfo & field = aggregate_info->fields[i];
        const FieldInfo * input_field = &field;
        if(field.is_anonymous_storage) {
          ClassInfo * storage_info =
              callbacks.type_traits().class_info_for_type(field.type);
          if(storage_info && !storage_info->fields.empty()) {
            input_field = &storage_info->fields[0];
          }
        }
        if(input_field->default_initializer) {
          if(!initializer_is_nothrow(input_field->type,
                                     scope,
                                     *input_field->default_initializer,
                                     visiting,
                                     callbacks)) {
            return false;
          }
        } else if(!default_initialization_is_nothrow(input_field->type,
                                                     scope,
                                                     visiting,
                                                     callbacks)) {
          return false;
        }
        if(aggregate_info->class_kind == "union") {
          break;
        }
      }
      return true;
    }
  }

  ExprInfo converted;
  try
  {
    converted = callbacks.analyze_expression_for_target(scope, *payload, target);
  }
  catch(const std::logic_error &)
  {
    return false;
  }
  return !callsem_node_can_throw(scope, converted.node, visiting, callbacks);
}

bool default_initialization_is_nothrow(
    const TypePtr & type,
    Scope & scope,
    std::set<FunctionBinding *> & visiting,
    const NothrowCallbacks & callbacks)
{
  TypePtr base = strip_top_level_cv(remove_reference_type(type));
  if(!base) {
    return false;
  }
  if(base->kind == Type::TK_ARRAY) {
    return default_initialization_is_nothrow(base->inner,
                                             scope,
                                             visiting,
                                             callbacks);
  }
  if(base->kind == Type::TK_FUNDAMENTAL ||
     is_scalar_or_member_pointer_type(base, callbacks.type_traits())) {
    return true;
  }
  ClassInfo * info = callbacks.complete_class_type(base);
  if(!info || !info->complete) {
    return false;
  }

  FunctionBinding * ctor = nullptr;
  try
  {
    ctor = callbacks.select_default_constructor(scope, *info);
  }
  catch(const std::logic_error &)
  {
    return false;
  }
  return ctor && function_binding_is_nothrow(*ctor, visiting, callbacks);
}

namespace {

bool destruction_is_nothrow(const TypePtr & type,
                            std::set<FunctionBinding *> & visiting,
                            const NothrowCallbacks & callbacks)
{
  TypePtr base = strip_top_level_cv(remove_reference_type(type));
  if(!base) {
    return false;
  }
  if(base->kind == Type::TK_ARRAY) {
    return destruction_is_nothrow(base->inner, visiting, callbacks);
  }
  if(base->kind == Type::TK_FUNDAMENTAL ||
     is_scalar_or_member_pointer_type(base, callbacks.type_traits())) {
    return true;
  }
  if(is_trivially_destructible_type(base, callbacks.type_traits())) {
    return true;
  }
  ClassInfo * info = callbacks.complete_class_type(base);
  if(!info || !info->complete) {
    return false;
  }
  FunctionBinding * dtor = callbacks.destructor_for(*info);
  return dtor && function_binding_is_nothrow(*dtor, visiting, callbacks);
}

bool assignment_is_nothrow(const TypePtr & type,
                           bool move,
                           std::set<FunctionBinding *> & visiting,
                           const NothrowCallbacks & callbacks)
{
  TypePtr base = strip_top_level_cv(remove_reference_type(type));
  if(!base) {
    return false;
  }
  if(base->kind == Type::TK_ARRAY) {
    return assignment_is_nothrow(base->inner, move, visiting, callbacks);
  }
  if(base->kind == Type::TK_FUNDAMENTAL ||
     is_scalar_or_member_pointer_type(base, callbacks.type_traits())) {
    return true;
  }
  if(is_trivially_copy_assignable_type(base, callbacks.type_traits())) {
    return true;
  }
  if(base->kind != Type::TK_NAMED) {
    return false;
  }
  ClassInfo * info = callbacks.complete_class_type(base);
  if(!info || !info->complete) {
    return false;
  }
  FunctionBinding * op =
      move ? callbacks.move_assignment_for(*info) :
             callbacks.copy_assignment_for(*info);
  if(!op && move) {
    op = callbacks.copy_assignment_for(*info);
  }
  return op && function_binding_is_nothrow(*op, visiting, callbacks);
}

bool constructor_binding_is_implicitly_nothrow(
    FunctionBinding & binding,
    std::set<FunctionBinding *> & visiting,
    const NothrowCallbacks & callbacks)
{
  if(!binding.owner_class || !binding.declaration_scope) {
    return false;
  }
  if(!binding.is_constructor) {
    return false;
  }
  if(!binding.is_defaulted && !binding.synthesized &&
     !binding.is_aggregate_constructor) {
    return false;
  }

  ClassInfo & info = *binding.owner_class;
  Scope & init_scope = *binding.declaration_scope;
  std::set<std::string> explicitly_initialized;

  if(binding.ctor_initializer) {
    for(std::size_t i = 0; i < binding.ctor_initializer->children.size(); ++i) {
      const CppAstNode & mem_init = binding.ctor_initializer->children[i];
      const CppAstNode * id =
          find_child_kind(mem_init, CppAstKind::mem_initializer_id);
      if(!id) {
        return false;
      }
      explicitly_initialized.insert(id->value);

      TypePtr target_type;
      bool matched = false;
      for(std::size_t j = 0; j < info.fields.size(); ++j) {
        if(info.fields[j].name == id->value) {
          target_type = info.fields[j].type;
          matched = true;
          break;
        }
      }
      if(!matched) {
        for(std::size_t j = 0; j < info.bases.size(); ++j) {
          if(info.bases[j].type &&
             (info.bases[j].type->qualified_name == id->value ||
              info.bases[j].type->name == id->value)) {
            target_type = info.bases[j].type->type;
            matched = true;
            break;
          }
        }
      }
      if(!matched) {
        TypePtr named = callbacks.lookup_type(init_scope, id->value);
        if(named) {
          target_type = named;
          matched = true;
        }
      }
      if(!matched) {
        return false;
      }

      const CppAstNode * payload = nullptr;
      if(!mem_init.children.empty()) {
        payload = &mem_init.children.back();
      }
      if(!payload ||
         !initializer_is_nothrow(target_type,
                                 init_scope,
                                 *payload,
                                 visiting,
                                 callbacks)) {
        return false;
      }
    }
  }

  for(std::size_t i = 0; i < info.bases.size(); ++i) {
    if(!info.bases[i].type) {
      return false;
    }
    const std::string & base_name = info.bases[i].type->name;
    const std::string & qualified_name = info.bases[i].type->qualified_name;
    if(explicitly_initialized.count(base_name) != 0 ||
       explicitly_initialized.count(qualified_name) != 0) {
      continue;
    }
    if(!default_initialization_is_nothrow(info.bases[i].type->type,
                                          init_scope,
                                          visiting,
                                          callbacks)) {
      return false;
    }
  }

  for(std::size_t i = 0; i < info.fields.size(); ++i) {
    if(explicitly_initialized.count(info.fields[i].name) != 0) {
      continue;
    }
    if(info.fields[i].default_initializer) {
      if(!initializer_is_nothrow(info.fields[i].type,
                                 init_scope,
                                 *info.fields[i].default_initializer,
                                 visiting,
                                 callbacks)) {
        return false;
      }
    } else if(!default_initialization_is_nothrow(info.fields[i].type,
                                                init_scope,
                                                visiting,
                                                callbacks)) {
      return false;
    }
  }

  return true;
}

bool destructor_binding_is_implicitly_nothrow(
    FunctionBinding & binding,
    std::set<FunctionBinding *> & visiting,
    const NothrowCallbacks & callbacks)
{
  if(!binding.owner_class || !binding.is_destructor) {
    return false;
  }
  const bool implicit_like =
      binding.synthesized ||
      binding.is_defaulted ||
      (!binding.declaration_node && !binding.definition_node && !binding.body);
  if(!implicit_like) {
    return false;
  }

  ClassInfo & info = *binding.owner_class;
  for(std::size_t i = 0; i < info.bases.size(); ++i) {
    if(!info.bases[i].type ||
       !destruction_is_nothrow(info.bases[i].type->type,
                               visiting,
                               callbacks)) {
      return false;
    }
  }
  for(std::size_t i = 0; i < info.fields.size(); ++i) {
    if(!destruction_is_nothrow(info.fields[i].type, visiting, callbacks)) {
      return false;
    }
  }
  return true;
}

bool assignment_binding_is_implicitly_nothrow(
    FunctionBinding & binding,
    std::set<FunctionBinding *> & visiting,
    const NothrowCallbacks & callbacks)
{
  if(!binding.owner_class ||
     (!binding.is_copy_assignment && !binding.is_move_assignment)) {
    return false;
  }
  const bool implicit_like =
      binding.synthesized ||
      binding.is_defaulted ||
      (!binding.declaration_node && !binding.definition_node && !binding.body);
  if(!implicit_like) {
    return false;
  }

  ClassInfo & info = *binding.owner_class;
  if(info.class_kind == "union") {
    return false;
  }

  const bool move = binding.is_move_assignment;
  for(std::size_t i = 0; i < info.bases.size(); ++i) {
    if(info.bases[i].is_virtual ||
       !assignment_is_nothrow(info.bases[i].type->type,
                              move,
                              visiting,
                              callbacks)) {
      return false;
    }
  }
  for(std::size_t i = 0; i < info.fields.size(); ++i) {
    const TypePtr field_type = strip_top_level_cv(info.fields[i].type);
    if(info.fields[i].is_bit_field ||
       is_reference_type(field_type)) {
      continue;
    }
    if(!assignment_is_nothrow(info.fields[i].type, move, visiting, callbacks)) {
      return false;
    }
  }
  return true;
}

}  // namespace

bool function_binding_is_nothrow(
    FunctionBinding & binding,
    std::set<FunctionBinding *> & visiting,
    const NothrowCallbacks & callbacks)
{
  bool explicit_value = false;
  if(callbacks.evaluate_explicit_function_nothrow_semantically(binding,
                                                               explicit_value)) {
    return explicit_value;
  }

  if(!visiting.insert(&binding).second) {
    return false;
  }

  const bool result =
      binding.is_constructor ?
          constructor_binding_is_implicitly_nothrow(binding,
                                                    visiting,
                                                    callbacks) :
      binding.is_destructor ?
          destructor_binding_is_implicitly_nothrow(binding,
                                                   visiting,
                                                   callbacks) :
      (binding.is_copy_assignment || binding.is_move_assignment) ?
          assignment_binding_is_implicitly_nothrow(binding,
                                                   visiting,
                                                   callbacks) :
      false;
  visiting.erase(&binding);
  return result;
}

bool function_binding_is_nothrow(FunctionBinding & binding,
                                 const NothrowCallbacks & callbacks)
{
  std::set<FunctionBinding *> visiting;
  return function_binding_is_nothrow(binding, visiting, callbacks);
}

bool callsem_node_can_throw(
    Scope & scope,
    const CallSemNode & node,
    std::set<FunctionBinding *> & visiting,
    const NothrowCallbacks & callbacks)
{
  const auto any_child_throws =
      [&scope, &visiting, &callbacks](const CallSemNode & current) -> bool
      {
        for(std::size_t i = 0; i < current.children.size(); ++i) {
          if(callsem_node_can_throw(scope,
                                    current.children[i],
                                    visiting,
                                    callbacks)) {
            return true;
          }
        }
        return false;
      };

  switch(node.kind) {
  case CallSemKind::literal:
  case CallSemKind::id_expression:
  case CallSemKind::parameter:
  case CallSemKind::variable:
  case CallSemKind::callee:
  case CallSemKind::sizeof_expression:
    return any_child_throws(node);

  case CallSemKind::member_expression:
  case CallSemKind::braced_init_list:
  case CallSemKind::unary_expression:
  case CallSemKind::binary_expression:
  case CallSemKind::assignment_expression:
  case CallSemKind::conditional_expression:
  case CallSemKind::condition:
  case CallSemKind::postfix_expression:
  case CallSemKind::subscript_expression:
    return any_child_throws(node);

  case CallSemKind::call_expression: {
    if(node.children.empty()) {
      return true;
    }
    if(node.children[0].kind == CallSemKind::callee &&
       node.children[0].text == "__pseudo_destructor") {
      return any_child_throws(node);
    }
    for(std::size_t i = 1; i < node.children.size(); ++i) {
      if(callsem_node_can_throw(scope, node.children[i], visiting, callbacks)) {
        return true;
      }
    }
    FunctionBinding * binding = nullptr;
    if(!callbacks.resolve_dump_callee_binding(node.children[0], binding) ||
       !binding) {
      if(callbacks.is_declval_dump_callee(scope, node.children[0])) {
        return false;
      }
      return true;
    }
    return !function_binding_is_nothrow(*binding, visiting, callbacks);
  }

  case CallSemKind::dynamic_cast_expression:
  case CallSemKind::throw_statement:
  case CallSemKind::typeid_expression:
    return true;

  case CallSemKind::constructor_action:
  case CallSemKind::destructor_action:
    if(node.trivial_lifecycle || node.object_trivial_lifecycle) {
      return false;
    }
    return any_child_throws(node);

  default:
    return any_child_throws(node);
  }
}

}  // namespace callsemantic
