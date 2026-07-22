#include <cstdlib>
#include <sstream>

using namespace std;

#include "constexpr_eval.h"

#include "cpp_decl_bridge.h"
#include "types.h"

namespace constant_eval {

using namespace cpp_decl;

namespace {

const size_t kMaxConstexprLoopIterations = 100000;
const size_t kMaxConstexprCallDepth = 512;

string literal_storage_identity(const CppAstNode & node)
{
  ostringstream out;
  out << "literal@" << static_cast<const void *>(&node);
  return out.str();
}

bool parse_literal_value(const CppAstNode & node, ConstexprValue & out)
{
  if(node.kind == CppAstKind::keyword_literal && node.has_token) {
    if(node.simple_type == KW_TRUE) {
      out = make_integral_value(1, make_fundamental(FT_BOOL));
      return true;
    }
    if(node.simple_type == KW_FALSE) {
      out = make_integral_value(0, make_fundamental(FT_BOOL));
      return true;
    }
    if(node.simple_type == KW_NULLPTR) {
      out = make_nullptr_value();
      return true;
    }
    return false;
  }

  if(node.kind != CppAstKind::literal || node.value.empty()) {
    return false;
  }

  if(node.value.find_first_of("'\"") != string::npos) {
    try
    {
      QuoteLiteralData literal = parse_quote_literal(node.value);
      if(literal.quote == '"' && literal.ud_suffix.empty()) {
        const EFundamentalType element_type = string_literal_element_type(literal);
        const TypePtr char_type =
            make_cv(make_fundamental(element_type), true, false);
        vector<ConstexprValue> elements;
        const vector<unsigned long long> & units =
            quote_literal_string_units(literal);
        elements.reserve(units.size() + 1);
        for(size_t i = 0; i < units.size(); ++i) {
          elements.push_back(make_integral_value(static_cast<long long>(units[i]),
                                                 make_fundamental(element_type)));
        }
        elements.push_back(make_integral_value(0, make_fundamental(element_type)));
        out = make_array_value(make_array(char_type, true, elements.size()),
                               elements,
                               literal_storage_identity(node));
        return true;
      }
      if(literal.quote != '\'' || !literal.ud_suffix.empty() || literal.contents.empty()) {
        return false;
      }
      out = make_integral_value(static_cast<long long>(literal.contents[0]),
                                make_fundamental(character_literal_type(literal)));
      return true;
    }
    catch(const logic_error &)
    {
      return false;
    }
  }

  string floating_value_text;
  EFundamentalType literal_type = FT_VOID;
  string ud_suffix;
  if(split_floating_literal(node.value,
                            floating_value_text,
                            literal_type,
                            ud_suffix)) {
    if(!ud_suffix.empty() || literal_type == FT_VOID) {
      return false;
    }
    char * end = nullptr;
    const long double value = strtold(floating_value_text.c_str(), &end);
    if(!end || *end != '\0') {
      return false;
    }
    out = make_floating_value(value, make_fundamental(literal_type));
    return true;
  }

  try
  {
    unsigned long long parsed = 0;
    string ud_suffix;
    EFundamentalType literal_type = classify_int(node.value, parsed, ud_suffix);
    if(!ud_suffix.empty() || literal_type == FT_VOID) {
      return false;
    }
    out = type_is_signed(literal_type) ?
        make_integral_bits_value(static_cast<long long>(parsed),
                                 make_fundamental(literal_type)) :
        make_integral_bits_value(static_cast<unsigned long long>(parsed),
                                 make_fundamental(literal_type));
    return true;
  }
  catch(const logic_error &)
  {
    return false;
  }
}

const CppAstNode * find_child_kind(const CppAstNode & node, CppAstKind kind)
{
  return find_child(node, kind);
}

bool is_this_expression(const CppAstNode & node)
{
  const CppAstNode * current = &node;
  while(current->kind == CppAstKind::parenthesized_expression &&
        current->children.size() == 1) {
    current = &current->children[0];
  }
  return (current->kind == CppAstKind::id_expression &&
          current->value == "this") ||
         (current->kind == CppAstKind::keyword_literal &&
          current->has_token &&
          current->simple_type == KW_THIS);
}

bool type_can_select_overloaded_operator(const TypePtr & type)
{
  const Type * base = type.get();
  if(base &&
     (base->kind == Type::TK_LVALUE_REFERENCE ||
      base->kind == Type::TK_RVALUE_REFERENCE)) {
    base = base->inner.get();
  }
  if(base && base->kind == Type::TK_CV) {
    base = base->inner.get();
  }
  return base && base->kind == Type::TK_NAMED;
}

}  // namespace

Evaluator::Evaluator(Hooks hooks)
  : hooks_(std::move(hooks))
{}

bool Evaluator::lookup_value(const string & name,
                             const CppAstNode * node,
                             ConstexprValue & out) const
{
  for(size_t frame_index = frames_.size(); frame_index > 0; --frame_index) {
    const Frame & frame = frames_[frame_index - 1];
    for(size_t scope_index = frame.scopes.size(); scope_index > 0; --scope_index) {
      map<string, ConstexprValue>::const_iterator found =
          frame.scopes[scope_index - 1].find(name);
      if(found != frame.scopes[scope_index - 1].end()) {
        out = found->second;
        if((out.kind == ConstexprValue::CV_ARRAY ||
            out.kind == ConstexprValue::CV_AGGREGATE) &&
           out.storage_identity.empty()) {
          assign_storage_identity(out, name);
        }
        return out.kind != ConstexprValue::CV_INVALID;
      }
    }
  }
  if(hooks_.lookup_external_value &&
     hooks_.lookup_external_value(name, node, out)) {
    if(out.kind == ConstexprValue::CV_ARRAY ||
       out.kind == ConstexprValue::CV_AGGREGATE) {
      assign_storage_identity(out, name);
    }
    return out.kind != ConstexprValue::CV_INVALID;
  }
  return false;
}

bool Evaluator::current_this_object(ConstexprValue & out) const
{
  for(size_t frame_index = frames_.size(); frame_index > 0; --frame_index) {
    const Frame & frame = frames_[frame_index - 1];
    if(frame.has_this_object) {
      out = frame.this_object;
      return frame.this_object.kind != ConstexprValue::CV_INVALID;
    }
  }
  return false;
}

bool Evaluator::probing_overloaded_operator_operand() const
{
  return overloaded_operator_operand_probe_;
}

bool Evaluator::may_select_overloaded_operator(const CppAstNode & node)
{
  if(!hooks_.supports_overloaded_operator_operand_probe ||
     !hooks_.evaluate_special_expression) {
    return false;
  }
  overloaded_operator_operand_probe_ = true;
  ConstexprValue ignored;
  try {
    const bool result = hooks_.evaluate_special_expression(*this, node, ignored);
    overloaded_operator_operand_probe_ = false;
    return result;
  } catch(...) {
    overloaded_operator_operand_probe_ = false;
    throw;
  }
}

bool Evaluator::assign_local(const string & name, const ConstexprValue & value)
{
  for(size_t frame_index = frames_.size(); frame_index > 0; --frame_index) {
    Frame & frame = frames_[frame_index - 1];
    for(size_t scope_index = frame.scopes.size(); scope_index > 0; --scope_index) {
      map<string, ConstexprValue>::iterator found =
          frame.scopes[scope_index - 1].find(name);
      if(found != frame.scopes[scope_index - 1].end()) {
        found->second = value;
        return true;
      }
    }
  }
  return false;
}

void Evaluator::push_scope()
{
  if(frames_.empty()) {
    frames_.push_back(Frame());
  }
  frames_.back().scopes.push_back(map<string, ConstexprValue>());
}

void Evaluator::pop_scope()
{
  if(frames_.empty() || frames_.back().scopes.empty()) {
    return;
  }
  frames_.back().scopes.pop_back();
}

bool Evaluator::eval_initializer(const CppAstNode & node,
                                 ConstexprValue & out,
                                 const TypePtr & target)
{
  const CppAstNode * payload = &node;
  if(node.kind == CppAstKind::initializer && !node.children.empty()) {
    payload = &node.children[0];
  }

  if(target && hooks_.evaluate_typed_initializer) {
    if(hooks_.evaluate_typed_initializer(*this, *payload, target, out)) {
      return true;
    }
  }

  if(payload->kind == CppAstKind::paren_initializer ||
     payload->kind == CppAstKind::braced_init_list) {
    if(payload->children.size() != 1) {
      return false;
    }
    return eval_expr(payload->children[0], out);
  }

  if(!eval_expr(*payload, out)) {
    return false;
  }
  if(target) {
    ConstexprValue converted;
    if(constexpr_value_cast(out, target, converted)) {
      out = converted;
    }
  }
  return true;
}

bool Evaluator::declare_locals(const vector<LocalDeclaration> & locals,
                               string & error,
                               ConstexprValue * last_value)
{
  for(size_t i = 0; i < locals.size(); ++i) {
    ConstexprValue value;
    if(locals[i].initializer) {
      if(!eval_initializer(*locals[i].initializer, value, locals[i].type)) {
        error = "failed to evaluate local initializer";
        return false;
      }
      if(locals[i].type) {
        ConstexprValue converted;
        if(!constexpr_value_cast(value, locals[i].type, converted)) {
          error = "failed to convert local initializer";
          return false;
        }
        value = converted;
      }
    }
    if(frames_.empty() || frames_.back().scopes.empty()) {
      error = "missing constexpr frame scope";
      return false;
    }
    if(value.kind == ConstexprValue::CV_ARRAY ||
       value.kind == ConstexprValue::CV_AGGREGATE) {
      assign_storage_identity(value, locals[i].name);
    }
    frames_.back().scopes.back()[locals[i].name] = value;
    if(last_value) {
      *last_value = value;
    }
  }
  return true;
}

bool Evaluator::eval_condition_node(const CppAstNode & node, bool & truthy, string & error)
{
  if(node.kind != CppAstKind::condition || node.children.size() != 1) {
    error = "unsupported constexpr condition";
    return false;
  }

  const CppAstNode & payload = node.children[0];
  if(payload.kind == CppAstKind::condition_declaration) {
    if(payload.children.size() != 3 || !hooks_.parse_local_declaration) {
      error = "unsupported constexpr condition declaration";
      return false;
    }

    CppAstNode synthetic_decl;
    synthetic_decl.kind = CppAstKind::simple_declaration;
    synthetic_decl.children.push_back(payload.children[0]);
    CppAstNode init_list;
    init_list.kind = CppAstKind::init_declarator_list;
    CppAstNode init_decl;
    init_decl.kind = CppAstKind::init_declarator;
    init_decl.children.push_back(payload.children[1]);
    init_decl.children.push_back(payload.children[2]);
    init_list.children.push_back(init_decl);
    synthetic_decl.children.push_back(init_list);

    vector<LocalDeclaration> locals;
    if(!hooks_.parse_local_declaration(synthetic_decl, locals, error) || locals.size() != 1) {
      if(error.empty()) {
        error = "failed to parse constexpr condition declaration";
      }
      return false;
    }

    ConstexprValue value;
    if(!declare_locals(locals, error, &value)) {
      return false;
    }
    if(!constexpr_value_truthy(value, truthy)) {
      error = "failed to evaluate constexpr condition declaration";
      return false;
    }
    return true;
  }

  ConstexprValue value;
  if(!eval_initializer(payload, value, make_fundamental(FT_BOOL)) ||
     !constexpr_value_truthy(value, truthy)) {
    error = "failed to evaluate constexpr condition";
    return false;
  }
  return true;
}

bool Evaluator::eval_expr(const CppAstNode & node, ConstexprValue & out)
{
  return eval_expr_inner(node, out);
}

bool Evaluator::eval_condition_expr(const CppAstNode & node, bool & truthy)
{
  ConstexprValue value;
  if(eval_expr(node, value) && constexpr_value_truthy(value, truthy)) {
    return true;
  }
  if(!hooks_.evaluate_typed_initializer ||
     !hooks_.evaluate_typed_initializer(*this,
                                        node,
                                        make_fundamental(FT_BOOL),
                                        value)) {
    return false;
  }
  return constexpr_value_truthy(value, truthy);
}

bool Evaluator::eval_discarded_expr(const CppAstNode & node)
{
  if(node.kind == CppAstKind::parenthesized_expression && node.children.size() == 1) {
    return eval_discarded_expr(node.children[0]);
  }

  if(node.kind == CppAstKind::cast_expression && node.children.size() == 2 &&
     node.children[0].kind == CppAstKind::type_id) {
    TypePtr target;
    if(hooks_.parse_type_id &&
       hooks_.parse_type_id(node.children[0], target) &&
       is_void_type(target)) {
      return eval_discarded_expr(node.children[1]);
    }
  }

  if(node.kind == CppAstKind::binary_expression && node.children.size() == 2 &&
     node.has_token && node.simple_type == OP_COMMA) {
    return eval_discarded_expr(node.children[0]) &&
           eval_discarded_expr(node.children[1]);
  }

  if(node.kind == CppAstKind::conditional_expression &&
     node.children.size() == 3) {
    bool truthy = false;
    if(!eval_condition_expr(node.children[0], truthy)) {
      return false;
    }
    return eval_discarded_expr(node.children[truthy ? 1 : 2]);
  }

  ConstexprValue ignored;
  return eval_expr(node, ignored);
}

bool Evaluator::eval_expr_inner(const CppAstNode & node, ConstexprValue & out)
{
  if(parse_literal_value(node, out)) {
    return true;
  }

  if(is_this_expression(node) && current_this_object(out)) {
    return true;
  }

  if(node.kind == CppAstKind::id_expression) {
    if(lookup_value(node.value, &node, out)) {
      return true;
    }
    return hooks_.evaluate_special_expression &&
           hooks_.evaluate_special_expression(*this, node, out);
  }

  if(node.kind == CppAstKind::parenthesized_expression && node.children.size() == 1) {
    return eval_expr(node.children[0], out);
  }

  if(node.kind == CppAstKind::unary_expression && node.children.size() == 1 && node.has_token) {
    if(node.simple_type == OP_STAR &&
       is_this_expression(node.children[0]) &&
       current_this_object(out)) {
      return true;
    }
    if(hooks_.evaluate_special_expression &&
       node.simple_type == OP_AMP &&
       hooks_.evaluate_special_expression(*this, node, out)) {
      return true;
    }
    if((node.simple_type == OP_INC || node.simple_type == OP_DEC) &&
       node.children[0].kind == CppAstKind::id_expression) {
      ConstexprValue current;
      if(!lookup_value(node.children[0].value, &node.children[0], current)) {
        return false;
      }
      ConstexprValue one = make_integral_value(1, make_fundamental(FT_INT));
      ConstexprValue updated;
      if(!constexpr_value_apply_binary(node.simple_type == OP_INC ? OP_PLUS : OP_MINUS,
                                       current,
                                       one,
                                       updated) ||
         !assign_local(node.children[0].value, updated)) {
        return false;
      }
      out = updated;
      return true;
    }
    ConstexprValue operand;
    if(!eval_expr(node.children[0], operand)) {
      if(node.simple_type == OP_AMP &&
         hooks_.evaluate_special_expression &&
         hooks_.evaluate_special_expression(*this, node, out)) {
        return true;
      }
      return false;
    }
    if(node.simple_type == OP_STAR &&
       operand.kind == ConstexprValue::CV_POINTER &&
       operand.pointer_offset < operand.array_elements.size()) {
      out = operand.array_elements[operand.pointer_offset];
      return true;
    }
    if(node.simple_type == OP_STAR &&
       operand.kind == ConstexprValue::CV_POINTER &&
       !operand.storage_identity.empty()) {
      ConstexprValue storage;
      if(lookup_value(operand.storage_identity, &node.children[0], storage) &&
         array_element_value(storage, operand.pointer_offset, out)) {
        return true;
      }
    }
    if(constexpr_value_apply_unary(node.simple_type, operand, out)) {
      return true;
    }
    if(hooks_.evaluate_special_expression &&
       hooks_.evaluate_special_expression(*this, node, out)) {
      return true;
    }
    return false;
  }

  if(node.kind == CppAstKind::binary_expression && node.children.size() == 2 && node.has_token) {
    if(node.simple_type == OP_COMMA) {
      if(!eval_discarded_expr(node.children[0])) {
        return false;
      }
      return eval_expr(node.children[1], out);
    }

    if(node.simple_type == OP_LAND || node.simple_type == OP_LOR) {
      ConstexprValue lhs;
      if(!eval_expr(node.children[0], lhs)) {
        return false;
      }
      if(type_can_select_overloaded_operator(lhs.type) &&
         hooks_.evaluate_special_expression &&
         hooks_.evaluate_special_expression(*this, node, out)) {
        return true;
      }
      bool lhs_truthy = false;
      if(!constexpr_value_truthy(lhs, lhs_truthy)) {
        return false;
      }
      if(node.simple_type == OP_LAND && !lhs_truthy) {
        if(may_select_overloaded_operator(node.children[1]) &&
           hooks_.evaluate_special_expression &&
           hooks_.evaluate_special_expression(*this, node, out)) {
          return true;
        }
        out = make_integral_value(0, make_fundamental(FT_BOOL));
        return true;
      }
      if(node.simple_type == OP_LOR && lhs_truthy) {
        if(may_select_overloaded_operator(node.children[1]) &&
           hooks_.evaluate_special_expression &&
           hooks_.evaluate_special_expression(*this, node, out)) {
          return true;
        }
        out = make_integral_value(1, make_fundamental(FT_BOOL));
        return true;
      }
      ConstexprValue rhs;
      if(!eval_expr(node.children[1], rhs)) {
        return false;
      }
      if(type_can_select_overloaded_operator(rhs.type) &&
         hooks_.evaluate_special_expression &&
         hooks_.evaluate_special_expression(*this, node, out)) {
        return true;
      }
      bool rhs_truthy = false;
      if(!constexpr_value_truthy(rhs, rhs_truthy)) {
        return false;
      }
      out = make_integral_value(rhs_truthy ? 1 : 0, make_fundamental(FT_BOOL));
      return true;
    }

    ConstexprValue lhs;
    ConstexprValue rhs;
    if(!eval_expr(node.children[0], lhs) || !eval_expr(node.children[1], rhs)) {
      return false;
    }
    if(constexpr_value_apply_binary(node.simple_type, lhs, rhs, out)) {
      return true;
    }
    if((lhs.kind == ConstexprValue::CV_ADDRESSABLE ||
        rhs.kind == ConstexprValue::CV_ADDRESSABLE) &&
       !type_can_select_overloaded_operator(lhs.type) &&
       !type_can_select_overloaded_operator(rhs.type)) {
      return false;
    }
    if(hooks_.evaluate_special_expression &&
       hooks_.evaluate_special_expression(*this, node, out)) {
      return true;
    }
    return false;
  }

  if(node.kind == CppAstKind::conditional_expression && node.children.size() == 3) {
    bool truthy = false;
    if(!eval_condition_expr(node.children[0], truthy)) {
      return false;
    }
    return eval_expr(node.children[truthy ? 1 : 2], out);
  }

  if(node.kind == CppAstKind::cast_expression && node.children.size() == 2 &&
     node.children[0].kind == CppAstKind::type_id) {
    ConstexprValue value;
    TypePtr target;
    if(!hooks_.parse_type_id ||
       !hooks_.parse_type_id(node.children[0], target)) {
      return false;
    }
    if(eval_expr(node.children[1], value) &&
       constexpr_value_cast(value, target, out)) {
      return true;
    }
    return hooks_.evaluate_typed_initializer &&
           hooks_.evaluate_typed_initializer(*this,
                                              node.children[1],
                                              target,
                                              out);
  }

  if(node.kind == CppAstKind::assignment_expression && node.children.size() == 2 && node.has_token) {
    if(node.children[0].kind != CppAstKind::id_expression) {
      return false;
    }
    ConstexprValue current;
    const bool have_current =
        lookup_value(node.children[0].value, &node.children[0], current);
    ConstexprValue rhs;
    if(have_current &&
       (node.children[1].kind == CppAstKind::initializer ||
        node.children[1].kind == CppAstKind::paren_initializer ||
        node.children[1].kind == CppAstKind::paren_argument_list ||
        node.children[1].kind == CppAstKind::braced_init_list)) {
      if(!eval_initializer(node.children[1], rhs, current.type)) {
        return false;
      }
    } else if(!eval_expr(node.children[1], rhs)) {
      return false;
    }
    ConstexprValue stored = rhs;
    if((node.simple_type == OP_ASS || node.simple_type == OP_PLUSASS ||
        node.simple_type == OP_MINUSASS || node.simple_type == OP_STARASS ||
        node.simple_type == OP_DIVASS || node.simple_type == OP_MODASS) &&
       have_current &&
       node.simple_type != OP_ASS) {
      ETokenType binary_op = OP_PLUS;
      switch(node.simple_type) {
      case OP_PLUSASS: binary_op = OP_PLUS; break;
      case OP_MINUSASS: binary_op = OP_MINUS; break;
      case OP_STARASS: binary_op = OP_STAR; break;
      case OP_DIVASS: binary_op = OP_DIV; break;
      case OP_MODASS: binary_op = OP_MOD; break;
      default: break;
      }
      if(!constexpr_value_apply_binary(binary_op, current, rhs, stored)) {
        return false;
      }
    }
    if(!assign_local(node.children[0].value, stored)) {
      return false;
    }
    out = stored;
    return true;
  }

  if(node.kind == CppAstKind::sizeof_expression && node.children.size() == 1) {
    const CppAstNode & payload = node.children[0];
    if(payload.kind == CppAstKind::type_id) {
      TypePtr type;
      if(!hooks_.parse_type_id || !hooks_.parse_type_id(payload, type)) {
        CppAstNode recovered_operand;
        size_t recovered_size = 0;
        if(!cppast_recover_sizeof_type_id_expression_operand(
               payload,
               recovered_operand) ||
           !hooks_.evaluate_sizeof_operand ||
           !hooks_.evaluate_sizeof_operand(recovered_operand, recovered_size)) {
          return false;
        }
        out = make_integral_value(static_cast<long long>(recovered_size),
                                  make_fundamental(FT_UNSIGNED_LONG_INT));
        return true;
      }
      if(hooks_.record_sizeof_type_id) {
        hooks_.record_sizeof_type_id(payload, type);
      }
      TypePtr sizeof_type = sizeof_operand_type(type);
      if(!type_is_valid_sizeof_operand(sizeof_type)) {
        return false;
      }
      out = make_integral_value(static_cast<long long>(type_size(sizeof_type)),
                                make_fundamental(FT_UNSIGNED_LONG_INT));
      return true;
    }
    size_t value = 0;
    if(hooks_.evaluate_sizeof_operand && hooks_.evaluate_sizeof_operand(payload, value)) {
      out = make_integral_value(static_cast<long long>(value), make_fundamental(FT_UNSIGNED_LONG_INT));
      return true;
    }
    return false;
  }

  if(node.kind == CppAstKind::sizeof_pack_expression && node.children.size() == 1 &&
     node.children[0].kind == CppAstKind::identifier) {
    size_t pack_size = 0;
    if(!hooks_.lookup_pack_size || !hooks_.lookup_pack_size(node.children[0].value, pack_size)) {
      return false;
    }
    out = make_integral_value(static_cast<long long>(pack_size), make_fundamental(FT_UNSIGNED_LONG_INT));
    return true;
  }

  if(node.kind == CppAstKind::type_trait_expression && node.has_token &&
     node.simple_type == KW_ALIGNOF && node.children.size() == 1 &&
     node.children[0].kind == CppAstKind::type_id) {
    TypePtr type;
    if(!hooks_.parse_type_id || !hooks_.parse_type_id(node.children[0], type)) {
      return false;
    }
    TypePtr alignof_type = remove_reference_type(type);
    if(!alignof_type) {
      alignof_type = type;
    }
    out = make_integral_value(static_cast<long long>(type_alignment(alignof_type)),
                              make_fundamental(FT_UNSIGNED_LONG_INT));
    return true;
  }

  if(node.kind == CppAstKind::call_expression) {
    if(hooks_.evaluate_special_expression &&
       hooks_.evaluate_special_expression(*this, node, out)) {
      return true;
    }

    const CppAstNode * callee = &node.children[0];
    if(callee->kind == CppAstKind::parenthesized_expression &&
       callee->children.size() == 1) {
      callee = &callee->children[0];
    }
    const CppAstNode * argument_list = find_child_kind(node, CppAstKind::argument_list);
    if(!argument_list) {
      argument_list = find_child_kind(node, CppAstKind::paren_argument_list);
    }
    const bool single_braced_argument =
        argument_list &&
        argument_list->children.size() == 1 &&
        argument_list->children[0].kind == CppAstKind::braced_init_list;
    const auto evaluate_functional_type_initializer =
        [&](bool early_braced_scalar_only) -> bool
    {
      if(callee->kind != CppAstKind::id_expression ||
         !hooks_.lookup_type ||
         !hooks_.evaluate_typed_initializer) {
        return false;
      }
      if(early_braced_scalar_only && !single_braced_argument) {
        return false;
      }
      TypePtr target =
          hooks_.lookup_type_node ? hooks_.lookup_type_node(*callee) :
                                    hooks_.lookup_type(callee->value);
      if(!target) {
        return false;
      }
      if(early_braced_scalar_only) {
        TypePtr target_base = strip_top_level_cv(remove_reference_type(target));
        const bool target_is_named_enum =
            target_base &&
            target_base->kind == Type::TK_NAMED &&
            target_base->named_key.compare(0, 5, "enum ") == 0;
        if(!target_base ||
           (target_base->kind == Type::TK_NAMED && !target_is_named_enum) ||
           target_base->kind == Type::TK_ARRAY ||
           target_base->kind == Type::TK_FUNCTION) {
          return false;
        }
      }
      const CppAstNode * init_node = static_cast<const CppAstNode *>(&node);
      if(argument_list) {
        init_node = argument_list;
        if(single_braced_argument) {
          init_node = &argument_list->children[0];
        }
      }
      return hooks_.evaluate_typed_initializer(*this, *init_node, target, out);
    };
    if(evaluate_functional_type_initializer(true)) {
      return true;
    }
    if(single_braced_argument && evaluate_functional_type_initializer(false)) {
      return true;
    }

    vector<ConstexprValue> args;
    const bool zero_arg_braced_type_init =
        argument_list &&
        argument_list->children.size() == 1 &&
        argument_list->children[0].kind == CppAstKind::braced_init_list &&
        argument_list->children[0].children.empty();
    if(argument_list && !zero_arg_braced_type_init) {
      for(size_t i = 0; i < argument_list->children.size(); ++i) {
        ConstexprValue value;
        if(!eval_expr(argument_list->children[i], value)) {
          return false;
        }
        args.push_back(value);
      }
    }
    if(hooks_.evaluate_call &&
       hooks_.evaluate_call(*this, node, args, out)) {
      return true;
    }
    if(evaluate_functional_type_initializer(false)) {
      return true;
    }
    if(callee->kind == CppAstKind::id_expression &&
       hooks_.lookup_type && args.size() == 1) {
      TypePtr target =
          hooks_.lookup_type_node ? hooks_.lookup_type_node(*callee) :
                                    hooks_.lookup_type(callee->value);
      if(target && constexpr_value_cast(args[0], target, out)) {
        return true;
      }
    }
    return false;
  }

  if(hooks_.evaluate_special_expression) {
    return hooks_.evaluate_special_expression(*this, node, out);
  }

  return false;
}

StatementResult Evaluator::exec_stmt(const CppAstNode & node, const TypePtr & return_type)
{
  StatementResult result;

  if(node.kind == CppAstKind::compound_statement) {
    push_scope();
    for(size_t i = 0; i < node.children.size(); ++i) {
      StatementResult child = exec_stmt(node.children[i], return_type);
      if(child.kind != StatementResult::SR_FALLTHROUGH) {
        pop_scope();
        return child;
      }
    }
    pop_scope();
    return result;
  }

  if(node.kind == CppAstKind::return_statement) {
    result.kind = StatementResult::SR_RETURN;
    if(node.children.empty()) {
      return result;
    }
    if(return_type && !is_void_type(return_type)) {
      if(!eval_initializer(node.children[0], result.value, return_type)) {
        result.kind = StatementResult::SR_FAIL;
        result.error = "failed to evaluate return expression";
        return result;
      }
    } else if(!eval_expr(node.children[0], result.value)) {
      result.kind = StatementResult::SR_FAIL;
      result.error = "failed to evaluate return expression";
      return result;
    }
    if(return_type && !is_void_type(return_type)) {
      ConstexprValue converted;
      if(!constexpr_value_cast(result.value, return_type, converted)) {
        result.kind = StatementResult::SR_FAIL;
        result.error = "failed to convert return expression";
        return result;
      }
      if(is_reference_type(strip_top_level_cv(return_type)) &&
         converted.storage_identity.empty()) {
        converted.storage_identity = result.value.storage_identity;
        converted.pointer_offset = result.value.pointer_offset;
        converted.array_elements = result.value.array_elements;
      }
      result.value = converted;
    }
    return result;
  }

  if(node.kind == CppAstKind::expression_statement) {
    if(node.children.empty()) {
      return result;
    }
    if(!eval_expr(node.children[0], result.value)) {
      result.kind = StatementResult::SR_FAIL;
      result.error = "failed to evaluate expression statement";
    }
    result.kind = result.kind == StatementResult::SR_FAIL ? StatementResult::SR_FAIL
                                                          : StatementResult::SR_FALLTHROUGH;
    return result;
  }

  if(node.kind == CppAstKind::simple_declaration) {
    vector<LocalDeclaration> locals;
    string error;
    if(!hooks_.parse_local_declaration ||
       !hooks_.parse_local_declaration(node, locals, error)) {
      result.kind = StatementResult::SR_FAIL;
      result.error = error.empty() ? "failed to parse local declaration" : error;
      return result;
    }
    if(!declare_locals(locals, result.error)) {
      result.kind = StatementResult::SR_FAIL;
      return result;
    }
    return result;
  }

  if(node.kind == CppAstKind::using_directive ||
     node.kind == CppAstKind::using_declaration ||
     node.kind == CppAstKind::namespace_alias_definition ||
     node.kind == CppAstKind::alias_declaration) {
    string error;
    if(!hooks_.process_semantic_declaration ||
       !hooks_.process_semantic_declaration(node, error)) {
      result.kind = StatementResult::SR_FAIL;
      result.error = error.empty() ? "failed to process constexpr semantic declaration" : error;
    }
    return result;
  }

  if(node.kind == CppAstKind::for_init_statement) {
    if(node.children.empty()) {
      return result;
    }
    if(node.children.size() != 1) {
      result.kind = StatementResult::SR_FAIL;
      result.error = "unsupported constexpr for-init-statement";
      return result;
    }
    return exec_stmt(node.children[0], return_type);
  }

  if(node.kind == CppAstKind::if_statement) {
    push_scope();
    if(const CppAstNode * init = find_child_kind(node, CppAstKind::for_init_statement)) {
      StatementResult init_result = exec_stmt(*init, return_type);
      if(init_result.kind != StatementResult::SR_FALLTHROUGH) {
        pop_scope();
        return init_result;
      }
    }

    const CppAstNode * condition = find_child_kind(node, CppAstKind::condition);
    const CppAstNode * then_branch = find_child_kind(node, CppAstKind::then_node);
    const CppAstNode * else_branch = find_child_kind(node, CppAstKind::else_node);
    if(!condition || !then_branch || then_branch->children.size() != 1) {
      pop_scope();
      result.kind = StatementResult::SR_FAIL;
      result.error = "unsupported constexpr if-statement";
      return result;
    }

    bool truthy = false;
    if(!eval_condition_node(*condition, truthy, result.error)) {
      pop_scope();
      result.kind = StatementResult::SR_FAIL;
      if(result.error.empty()) {
        result.error = "failed to evaluate constexpr if condition";
      }
      return result;
    }

    StatementResult branch_result;
    if(truthy) {
      branch_result = exec_stmt(then_branch->children[0], return_type);
    } else if(else_branch && else_branch->children.size() == 1) {
      branch_result = exec_stmt(else_branch->children[0], return_type);
    }
    pop_scope();
    return branch_result;
  }

  if(node.kind == CppAstKind::while_statement) {
    if(node.children.size() != 2 || node.children[0].kind != CppAstKind::condition) {
      result.kind = StatementResult::SR_FAIL;
      result.error = "unsupported constexpr while-statement";
      return result;
    }
    for(size_t iteration = 0; iteration < kMaxConstexprLoopIterations; ++iteration) {
      push_scope();
      bool truthy = false;
      if(!eval_condition_node(node.children[0], truthy, result.error)) {
        pop_scope();
        result.kind = StatementResult::SR_FAIL;
        return result;
      }
      if(!truthy) {
        pop_scope();
        return StatementResult();
      }
      StatementResult body = exec_stmt(node.children[1], return_type);
      pop_scope();
      if(body.kind == StatementResult::SR_RETURN ||
         body.kind == StatementResult::SR_FAIL) {
        return body;
      }
      if(body.kind == StatementResult::SR_BREAK) {
        return StatementResult();
      }
      continue;
    }
    result.kind = StatementResult::SR_FAIL;
    result.error = "constexpr while iteration limit exceeded";
    return result;
  }

  if(node.kind == CppAstKind::do_statement) {
    if(node.children.size() != 2 || node.children[1].kind != CppAstKind::condition) {
      result.kind = StatementResult::SR_FAIL;
      result.error = "unsupported constexpr do-statement";
      return result;
    }
    for(size_t iteration = 0; iteration < kMaxConstexprLoopIterations; ++iteration) {
      StatementResult body = exec_stmt(node.children[0], return_type);
      if(body.kind == StatementResult::SR_RETURN ||
         body.kind == StatementResult::SR_FAIL) {
        return body;
      }
      if(body.kind == StatementResult::SR_BREAK) {
        return StatementResult();
      }
      bool truthy = false;
      if(!eval_condition_node(node.children[1], truthy, result.error)) {
        result.kind = StatementResult::SR_FAIL;
        return result;
      }
      if(!truthy) {
        return StatementResult();
      }
    }
    result.kind = StatementResult::SR_FAIL;
    result.error = "constexpr do iteration limit exceeded";
    return result;
  }

  if(node.kind == CppAstKind::for_statement) {
    if(node.children.empty() || node.children[0].kind != CppAstKind::for_init_statement) {
      result.kind = StatementResult::SR_FAIL;
      result.error = "unsupported constexpr for-statement";
      return result;
    }

    push_scope();
    StatementResult init_result = exec_stmt(node.children[0], return_type);
    if(init_result.kind != StatementResult::SR_FALLTHROUGH) {
      pop_scope();
      if(init_result.kind == StatementResult::SR_FAIL) {
        return init_result;
      }
      result.kind = StatementResult::SR_FAIL;
      result.error = "constexpr for-init produced non-fallthrough control flow";
      return result;
    }

    const CppAstNode * condition = find_child_kind(node, CppAstKind::condition);
    const CppAstNode * iteration = find_child_kind(node, CppAstKind::iteration);
    const CppAstNode & body = node.children.back();
    for(size_t loop_iteration = 0; loop_iteration < kMaxConstexprLoopIterations; ++loop_iteration) {
      push_scope();
      if(condition) {
        bool truthy = false;
        if(!eval_condition_node(*condition, truthy, result.error)) {
          pop_scope();
          pop_scope();
          result.kind = StatementResult::SR_FAIL;
          return result;
        }
        if(!truthy) {
          pop_scope();
          pop_scope();
          return StatementResult();
        }
      }

      StatementResult body_result = exec_stmt(body, return_type);
      if(body_result.kind == StatementResult::SR_RETURN ||
         body_result.kind == StatementResult::SR_FAIL) {
        pop_scope();
        pop_scope();
        return body_result;
      }

      if(body_result.kind != StatementResult::SR_BREAK && iteration && iteration->children.size() == 1) {
        ConstexprValue ignored;
        if(!eval_expr(iteration->children[0], ignored)) {
          pop_scope();
          pop_scope();
          result.kind = StatementResult::SR_FAIL;
          result.error = "failed to evaluate constexpr for iteration";
          return result;
        }
      }

      pop_scope();

      if(body_result.kind == StatementResult::SR_BREAK) {
        pop_scope();
        return StatementResult();
      }
    }
    pop_scope();
    result.kind = StatementResult::SR_FAIL;
    result.error = "constexpr for iteration limit exceeded";
    return result;
  }

  if(node.kind == CppAstKind::break_statement) {
    result.kind = StatementResult::SR_BREAK;
    return result;
  }

  if(node.kind == CppAstKind::continue_statement) {
    result.kind = StatementResult::SR_CONTINUE;
    return result;
  }

  if(node.kind == CppAstKind::static_assert_declaration && !node.children.empty()) {
    ConstexprValue condition;
    bool truthy = false;
    if(!eval_expr(node.children[0], condition) ||
       !constexpr_value_truthy(condition, truthy) ||
       !truthy) {
      result.kind = StatementResult::SR_FAIL;
      result.error = "constexpr static_assert failed";
      return result;
    }
    return result;
  }

  result.kind = StatementResult::SR_FAIL;
  result.error = "unsupported constexpr statement";
  return result;
}

bool Evaluator::call(const FunctionInfo & function,
                     const vector<ConstexprValue> & args,
                     ConstexprValue & out,
                     const Hooks * override_hooks)
{
  Hooks saved_hooks;
  const bool use_override_hooks = override_hooks != nullptr;
  if(use_override_hooks) {
    saved_hooks = hooks_;
    hooks_ = *override_hooks;
  }

  if(!function.body || function.variadic ||
     function.params.size() != args.size() ||
     call_depth_ >= kMaxConstexprCallDepth) {
    if(use_override_hooks) {
      hooks_ = saved_hooks;
    }
    return false;
  }

  ++call_depth_;
  frames_.push_back(Frame());
  if(function.is_method) {
    ConstexprValue this_object = function.implicit_object;
    if(!function.has_implicit_object && !current_this_object(this_object)) {
      frames_.pop_back();
      --call_depth_;
      if(use_override_hooks) {
        hooks_ = saved_hooks;
      }
      return false;
    }

    frames_.back().has_this_object = true;
    frames_.back().this_object = this_object;
    push_scope();

    std::function<void(const ConstexprValue &)> append_visible_members;
    append_visible_members =
        [this, &append_visible_members](const ConstexprValue & value) -> void
        {
          if(value.kind != ConstexprValue::CV_AGGREGATE || frames_.empty() ||
             frames_.back().scopes.empty()) {
            return;
          }
          for(size_t i = 0; i < value.aggregate_members.size(); ++i) {
            const bool is_base =
                i < value.aggregate_member_is_base.size() &&
                value.aggregate_member_is_base[i];
            if(is_base) {
              ConstexprValue base = value.aggregate_members[i].second;
              if(!value.storage_identity.empty()) {
                assign_storage_identity(base,
                                        value.storage_identity + "." +
                                            value.aggregate_members[i].first);
              }
              append_visible_members(base);
            }
          }
          for(size_t i = 0; i < value.aggregate_members.size(); ++i) {
            const bool is_base =
                i < value.aggregate_member_is_base.size() &&
                value.aggregate_member_is_base[i];
            if(!is_base) {
              ConstexprValue member = value.aggregate_members[i].second;
              if(!value.storage_identity.empty()) {
                assign_storage_identity(member,
                                        value.storage_identity + "." +
                                            value.aggregate_members[i].first);
              }
              frames_.back().scopes.back()[value.aggregate_members[i].first] = member;
            }
          }
        };
    append_visible_members(this_object);
  }
  push_scope();
  for(size_t i = 0; i < args.size(); ++i) {
    ConstexprValue value = args[i];
    if(function.params[i].second) {
      ConstexprValue converted;
      if(!constexpr_value_cast(value, function.params[i].second, converted)) {
        pop_scope();
        frames_.pop_back();
        --call_depth_;
        if(use_override_hooks) {
          hooks_ = saved_hooks;
        }
        return false;
      }
      value = converted;
    }
    if((value.kind == ConstexprValue::CV_ARRAY ||
        value.kind == ConstexprValue::CV_AGGREGATE) &&
       function.params[i].second &&
       !is_reference_type(strip_top_level_cv(function.params[i].second))) {
      assign_storage_identity(value, function.params[i].first);
    }
    frames_.back().scopes.back()[function.params[i].first] = value;
  }

  StatementResult result = exec_stmt(*function.body, function.return_type);
  pop_scope();
  if(function.is_method) {
    pop_scope();
  }
  frames_.pop_back();
  --call_depth_;
  if(result.kind != StatementResult::SR_RETURN) {
    if(use_override_hooks) {
      hooks_ = saved_hooks;
    }
    return false;
  }
  out = result.value;
  const bool ok =
      out.kind != ConstexprValue::CV_INVALID || is_void_type(function.return_type);
  if(use_override_hooks) {
    hooks_ = saved_hooks;
  }
  return ok;
}

}  // namespace constant_eval
