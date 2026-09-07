#include "lowir/intro/exercises.h"

namespace lowir_intro {
namespace {

using namespace lowir_model;

Operand integer(long long value)
{
  Operand operand;
  operand.has_int_value = true;
  operand.int_value = value;
  operand.literal_type = builtin_lowir_type(LTK_I64);
  return operand;
}

Operand label(BlockId id)
{
  Operand operand;
  operand.kind = Operand::OP_LABEL;
  operand.block = id;
  return operand;
}

Instruction instruction(Instruction::Kind kind, LowTypeKind type,
                        Operand first = Operand(), Operand second = Operand())
{
  Instruction ins;
  ins.kind = kind;
  ins.type = builtin_lowir_type(type);
  ins.first = first;
  ins.second = second;
  return ins;
}

// Small construction helpers for these exercises. Students choose their own
// model and builder interface; the serialized function boundary is the contract.
struct Builder
{
  Program program;
  Function function;
  BlockId current;

  Builder(const char * name, LowTypeKind result)
  {
    function.symbol = append_lowir_symbol(program, name);
    function.return_type = builtin_lowir_type(result);
    current = block("entry");
  }

  BlockId block(const char * name)
  {
    Block value;
    value.id = allocate_lowir_block_id(function, program.strings.intern(name));
    function.blocks.push_back(value);
    return value.id;
  }

  Operand parameter(const char * name, LowTypeKind type)
  {
    Parameter param;
    param.name = program.strings.intern(name);
    param.type = builtin_lowir_type(type);
    param.value = append_lowir_value(function, param.name, param.type);
    function.params.push_back(param);
    Operand operand;
    operand.kind = Operand::OP_TEMP;
    operand.value = param.value;
    return operand;
  }

  Operand slot(const char * name)
  {
    Operand operand;
    operand.kind = Operand::OP_SLOT;
    operand.slot = append_lowir_slot(function, program.strings.intern(name),
                                     builtin_lowir_type(LTK_I64));
    return operand;
  }

  void emit(const Instruction & ins)
  {
    function.blocks[current].instructions.push_back(ins);
  }

  Operand result(Instruction ins)
  {
    const LowType type = ins.kind == Instruction::IK_CMP ?
      builtin_lowir_type(LTK_I1) : ins.type;
    ins.dest = append_lowir_fresh_generated_value(function, type);
    emit(ins);
    Operand operand;
    operand.kind = Operand::OP_TEMP;
    operand.value = ins.dest;
    return operand;
  }

  Operand binary(LowOperation::Kind op, Operand left, Operand right)
  {
    Instruction ins = instruction(Instruction::IK_BINARY, LTK_I64, left, right);
    ins.op = op;
    return result(ins);
  }

  Program finish()
  {
    program.functions.push_back(function);
    return program;
  }
};

Program build_sum()
{
  Builder b("sum_to", LTK_I64);
  const Operand n = b.parameter("n", LTK_I64);
  const Operand i = b.slot("i");
  const Operand total = b.slot("total");
  const BlockId condition = b.block("condition");
  const BlockId body = b.block("body");
  const BlockId done = b.block("done");
  b.emit(instruction(Instruction::IK_STORE, LTK_I64, integer(1), i));
  b.emit(instruction(Instruction::IK_STORE, LTK_I64, integer(0), total));
  b.emit(instruction(Instruction::IK_JUMP, LTK_VOID, label(condition)));
  b.current = condition;
  const Operand index = b.result(instruction(Instruction::IK_LOAD, LTK_I64, i));
  Instruction compare = instruction(Instruction::IK_CMP, LTK_I64, index, n);
  compare.op = LowOperation::LOP_LE;
  Instruction branch = instruction(Instruction::IK_BRANCH, LTK_VOID,
                                    b.result(compare), label(body));
  branch.third = label(done);
  b.emit(branch);
  b.current = body;
  const Operand old_total = b.result(instruction(Instruction::IK_LOAD, LTK_I64, total));
  const Operand next_total = b.binary(LowOperation::LOP_ADD, old_total, index);
  b.emit(instruction(Instruction::IK_STORE, LTK_I64, next_total, total));
  const Operand next_index = b.binary(LowOperation::LOP_ADD, index, integer(1));
  b.emit(instruction(Instruction::IK_STORE, LTK_I64, next_index, i));
  b.emit(instruction(Instruction::IK_JUMP, LTK_VOID, label(condition)));
  b.current = done;
  const Operand value = b.result(instruction(Instruction::IK_LOAD, LTK_I64, total));
  b.emit(instruction(Instruction::IK_RETURN, LTK_I64, value));
  return b.finish();
}

Program build_swap()
{
  Builder b("swap_values", LTK_VOID);
  const Operand left = b.parameter("left", LTK_PTR);
  const Operand right = b.parameter("right", LTK_PTR);
  const Operand a = b.result(instruction(Instruction::IK_LOAD, LTK_I64, left));
  const Operand c = b.result(instruction(Instruction::IK_LOAD, LTK_I64, right));
  b.emit(instruction(Instruction::IK_STORE, LTK_I64, c, left));
  b.emit(instruction(Instruction::IK_STORE, LTK_I64, a, right));
  b.emit(instruction(Instruction::IK_RETURN, LTK_VOID));
  return b.finish();
}

Program build_call()
{
  Builder b("call_twice", LTK_I64);
  const Operand callee = b.parameter("fn", LTK_PTR);
  const Operand x = b.parameter("x", LTK_I64);
  Parameter param;
  param.name = b.program.strings.intern("x");
  param.type = builtin_lowir_type(LTK_I64);
  Instruction call = instruction(Instruction::IK_CALL, LTK_I64, callee);
  call.has_call_signature = true;
  call.call_params.push_back(param);
  call.call_return_type = param.type;
  call.args.push_back(x);
  const Operand first = b.result(call);
  call.args[0] = first;
  const Operand second = b.result(call);
  b.emit(instruction(Instruction::IK_RETURN, LTK_I64, second));
  return b.finish();
}

}  // namespace

lowir_model::Program BuildExercise(const std::string & name)
{
  if(name == "sum") return build_sum();
  if(name == "swap") return build_swap();
  if(name == "call") return build_call();
  lowir_model::ThrowLowirInternalError("unknown introductory LowIR exercise");
}

}  // namespace lowir_intro
