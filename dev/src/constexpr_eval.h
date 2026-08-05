#pragma once

#include <functional>
#include <map>
#include <string>
#include <vector>

#include "constant_value.h"
#include "cppast_ast.h"

namespace constant_eval {

struct LocalDeclaration
{
  std::string name;
  cpp_decl::TypePtr type;
  const CppAstNode * initializer = nullptr;
};

struct FunctionInfo
{
  std::string name;
  cpp_decl::TypePtr return_type;
  std::vector<std::pair<std::string, cpp_decl::TypePtr> > params;
  const CppAstNode * body = nullptr;
  bool variadic = false;
  bool is_method = false;
  bool has_implicit_object = false;
  ConstexprValue implicit_object;
};

struct StatementResult
{
  enum Kind
  {
    SR_FALLTHROUGH,
    SR_BREAK,
    SR_CONTINUE,
    SR_RETURN,
    SR_FAIL
  };

  Kind kind = SR_FALLTHROUGH;
  ConstexprValue value;
  std::string error;
};

class Evaluator;

struct Hooks
{
  std::function<bool(const std::string &, const CppAstNode *, ConstexprValue &)>
      lookup_external_value;
  std::function<cpp_decl::TypePtr(const std::string &)> lookup_type;
  std::function<cpp_decl::TypePtr(const CppAstNode &)> lookup_type_node;
  std::function<bool(const CppAstNode &, cpp_decl::TypePtr &)> parse_type_id;
  std::function<bool(const CppAstNode &, std::size_t &)> evaluate_sizeof_operand;
  std::function<bool(const CppAstNode &, std::size_t &)> evaluate_alignof_operand;
  std::function<bool(const std::string &, std::size_t &)> lookup_pack_size;
  std::function<bool(Evaluator &, const CppAstNode &, ConstexprValue &)> evaluate_special_expression;
  bool supports_overloaded_operator_operand_probe = false;
  std::function<bool(Evaluator &,
                     const CppAstNode &,
                     const cpp_decl::TypePtr &,
                     ConstexprValue &)> evaluate_typed_initializer;
  std::function<bool(Evaluator &,
                     const CppAstNode &,
                     const std::vector<ConstexprValue> &,
                     ConstexprValue &)> evaluate_call;
  std::function<bool(const CppAstNode &,
                     std::vector<LocalDeclaration> &,
                     std::string &)> parse_local_declaration;
  std::function<bool(const CppAstNode &, std::string &)>
      process_semantic_declaration;
};

class Evaluator
{
public:
  explicit Evaluator(Hooks hooks);

  bool eval_expr(const CppAstNode & node, ConstexprValue & out);
  bool eval_initializer(const CppAstNode & node,
                        ConstexprValue & out,
                        const cpp_decl::TypePtr & target = cpp_decl::TypePtr());
  bool call(const FunctionInfo & function,
            const std::vector<ConstexprValue> & args,
            ConstexprValue & out,
            const Hooks * override_hooks = nullptr);
  bool current_this_object(ConstexprValue & out) const;
  bool probing_overloaded_operator_operand() const;

private:
  struct Frame
  {
    std::vector<std::map<std::string, ConstexprValue> > scopes;
    bool has_this_object = false;
    ConstexprValue this_object;
  };

  Hooks hooks_;
  std::vector<Frame> frames_;
  std::size_t call_depth_ = 0;
  std::size_t next_temporary_identity_ = 0;
  bool overloaded_operator_operand_probe_ = false;

  bool eval_expr_inner(const CppAstNode & node, ConstexprValue & out);
  bool eval_discarded_expr(const CppAstNode & node);
  bool eval_condition_expr(const CppAstNode & node, bool & truthy);
  bool may_select_overloaded_operator(const CppAstNode & node);
  bool declare_locals(const std::vector<LocalDeclaration> & locals,
                      std::string & error,
                      ConstexprValue * last_value = nullptr);
  bool eval_condition_node(const CppAstNode & node, bool & truthy, std::string & error);
  bool lookup_value(const std::string & name,
                    const CppAstNode * node,
                    ConstexprValue & out) const;
  bool assign_local(const std::string & name, const ConstexprValue & value);
  void push_scope();
  void pop_scope();
  StatementResult exec_stmt(const CppAstNode & node, const cpp_decl::TypePtr & return_type);
};

}  // namespace constant_eval
