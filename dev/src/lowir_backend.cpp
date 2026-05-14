#include "lowir_backend.h"

#include <sstream>
#include <string>
#include <vector>

using namespace std;

#include "cy86_internal.h"
#include "lowir_cy86_backend.h"

namespace {

string literal_text(const cy86_internal::LiteralValue & literal)
{
  if(!literal.source.empty()) {
    return literal.source;
  }
  return to_string(cy86_internal::scalar_from_bytes<long long>(
      literal.data, literal.data.size()));
}

string address_expr_text(const cy86_internal::AddressExpr & expr)
{
  string base;
  if(expr.base_kind == cy86_internal::EB_LITERAL) {
    base = literal_text(expr.literal);
  } else {
    base = expr.name;
  }
  if(!expr.has_offset) {
    return base;
  }
  return base + (expr.subtract_offset ? "-" : "+") + literal_text(expr.offset);
}

string operand_text(const cy86_internal::Operand & operand)
{
  switch(operand.kind) {
    case cy86_internal::OPERAND_REGISTER:
      return operand.reg;
    case cy86_internal::OPERAND_IMMEDIATE:
      return address_expr_text(operand.expr);
    case cy86_internal::OPERAND_MEMORY:
      return "[" + address_expr_text(operand.expr) + "]";
  }
  return "";
}

string statement_text(const cy86_internal::Statement & statement)
{
  ostringstream out;
  for(size_t i = 0; i < statement.labels.size(); ++i) {
    out << statement.labels[i] << ":\n";
  }
  out << "\t" << statement.opcode;
  for(size_t i = 0; i < statement.operands.size(); ++i) {
    out << (i == 0 ? " " : " ") << operand_text(statement.operands[i]);
  }
  out << ";\n";
  return out.str();
}

}  // namespace

string translate_lowir_to_cy86(const vector<string> & srcfiles)
{
  const cy86_internal::Program program = build_lowir_cy86_program(srcfiles);
  ostringstream out;
  for(size_t i = 0; i < program.statements.size(); ++i) {
    if(i != 0 &&
       !program.statements[i].labels.empty() &&
       (program.statements[i - 1].opcode == "ret" ||
        cy86_internal::starts_with(program.statements[i - 1].opcode, "data") ||
        cy86_internal::starts_with(program.statements[i - 1].opcode, "syscall"))) {
      out << "\n";
    }
    out << statement_text(program.statements[i]);
  }
  return out.str();
}
