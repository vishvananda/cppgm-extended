#include "native/select/for_each_inst.h"
#include "native/select/function_selector.h"

// The selection of `vector`. The operands of an operation are XMM registers: a
// value in its home reloads into the vector scratch, xmm15 and, for a second
// operand, xmm14. The destructive first operand of a two-operand form may stay
// in memory, since the emitter copies it into the destination.
namespace native {

using lowir::Instruction;
using lowir::Type;
using lowir::TypeKind;
using lowir::Value;
using lowir::VectorOp;
using lowir::VectorShape;

namespace {

const Reg kX14 = static_cast<Reg>(XMM0 + 14);
const Reg kX15 = static_cast<Reg>(XMM0 + 15);

Cond test_cond(VectorOp op) {
  return op == VectorOp::Ptestz ? CC_E : op == VectorOp::Ptestc ? CC_B : CC_A;
}

} // namespace

// Exact bit construction only: no floating arithmetic or memory reads.
bool FunctionSelector::vector_constant(const Instruction &inst,
                                       lowir::Literal &out,
                                       unsigned depth) const {
  if (depth >= 6 || inst.op != lowir::Opcode::Vector ||
      inst.type.kind() != TypeKind::V128)
    return false;
  const VectorOp op = static_cast<VectorOp>(inst.sub);
  out = lowir::Literal{};
  if (op == VectorOp::Pzero)
    return true;
  if (op == VectorOp::Fromi64 || op == VectorOp::Fromi32) {
    const Value av = resolve(inst.ops[0]);
    if (av.kind != lowir::ValueKind::Integer)
      return false;
    out.low = op == VectorOp::Fromi32 ? literal(av).low & 0xffffffffull
                                      : literal(av).low;
    return true;
  }
  if (op != VectorOp::Punpcklqdq && op != VectorOp::Pshufd)
    return false;
  auto known = [&](Value v, lowir::Literal &bits) {
    v = resolve(v);
    return v.kind == lowir::ValueKind::Temp &&
           v.index >= body_->parameter_count &&
           body_->temps[v.index].def != lowir::kNoId &&
           vector_constant(body_->instructions[body_->temps[v.index].def], bits,
                           depth + 1);
  };
  lowir::Literal a{}, b{};
  if (!known(inst.ops[0], a))
    return false;
  if (op == VectorOp::Punpcklqdq) {
    if (!known(inst.ops[1], b))
      return false;
    out.low = a.low;
    out.high = b.low;
    return true;
  }
  const Value cv = resolve(inst.ops[2]);
  if (cv.kind != lowir::ValueKind::Integer)
    return false;
  const uint64_t control = literal(cv).low;
  for (unsigned k = 0; k < 4; ++k) {
    const unsigned lane = (control >> (2 * k)) & 3;
    const uint64_t bits =
        ((lane < 2 ? a.low : a.high) >> (32 * (lane & 1))) & 0xffffffffull;
    (k < 2 ? out.low : out.high) |= bits << (32 * (k & 1));
  }
  return true;
}

void FunctionSelector::find_vector_constants() {
  if (level_ < 1)
    return;
  for_each_inst([&](uint32_t id, uint32_t b) {
    const Instruction &inst = body_->instructions[id];
    lowir::Literal bits{};
    if (!live(id) || !vector_constant(inst, bits))
      return;
    // Construction inputs with no other reader emit nothing. Keep a shared
    // value in its register for its own readers rather than rematerializing it.
    for (unsigned k = 0; k < 2; ++k) {
      const Value v = resolve(inst.ops[k]);
      if (v.kind != lowir::ValueKind::Temp ||
          v.index < body_->parameter_count ||
          body_->temps[v.index].type.kind() != TypeKind::V128)
        continue;
      TempInfo &t = temps_[v.index];
      const unsigned reads = 1 + (resolve(inst.ops[1 - k]) == v ? 1 : 0);
      if (t.uses == reads && t.user == id && t.block == b &&
          !(t.flags & TF_GLOBAL))
        t.flags |= TF_ROTATED;
    }
  });
}

Operand FunctionSelector::vector_register(Value value, Reg scratch) {
  const Type type = Type::scalar(TypeKind::V128);
  const Operand op = rm_operand(value, type, scratch);
  if (op.kind == OperandKind::Reg)
    return op;
  load_into(scratch, op, TypeKind::V128);
  return Operand::make_reg(scratch);
}

void FunctionSelector::emit_vector_two_operand(const Instruction &inst) {
  const VectorOp op = static_cast<VectorOp>(inst.sub);
  const bool blend = lowir::shape_of(op) == VectorShape::Blend;
  if (blend)
    load_into(XMM0, rm_operand(inst.ops[2], Type::scalar(TypeKind::V128), kX15),
              TypeKind::V128);
  const Operand b = vector_register(inst.ops[1], kX15);
  Operand a = rm_operand(inst.ops[0], Type::scalar(TypeKind::V128), kX14);
  if (a.kind != OperandKind::Reg && a.kind != OperandKind::Mem)
    a = vector_register(inst.ops[0], kX14);
  const uint32_t exclude =
      (lowir::vector_commutes(op) ? 0 : 1u << b.reg) | (blend ? 1u << XMM0 : 0);
  const Reg d = result_reg(
      inst, dying_reg(inst.ops[0]),
      lowir::vector_commutes(op) ? dying_reg(inst.ops[1]) : kNoReg, exclude);
  // A blend's result may be planned in xmm0, which holds its mask.
  const Reg work = blend && d == XMM0 ? kX14 : d;
  Inst &mir = emit3(Op::Vector, TypeKind::V128, Operand::make_reg(work), a, b);
  mir.sub = inst.sub;
  if (lowir::shape_of(op) == VectorShape::BinaryImmediate)
    mir.aux = static_cast<uint32_t>(literal(resolve(inst.ops[2])).low);
  if (work != d)
    emit2(Op::FMov, TypeKind::V128, Operand::make_reg(d),
          Operand::make_reg(work));
  finish_result(inst, d);
}

// Whether the sole consumer reads only the scalar source lanes: other
// lanes need not be cleared before that consumer.
bool FunctionSelector::low_lane_only(const Instruction &inst) const {
  const auto single_user = [&](const Instruction &def) -> const Instruction * {
    const auto &t = temps_[def.result];
    return t.uses == 1 && t.user != lowir::kNoId ? &body_->instructions[t.user]
                                                 : nullptr;
  };
  const Instruction *lanes = single_user(inst);
  if (lanes == nullptr || lanes->op != lowir::Opcode::Vector)
    return false;
  const VectorOp op = static_cast<VectorOp>(lanes->sub);
  const VectorOp from = static_cast<VectorOp>(inst.sub);
  if (from != VectorOp::Fromf32 && from != VectorOp::Fromf64)
    return false;
  if (op == VectorOp::Pshufd) {
    const uint32_t lanes_read =
        static_cast<uint32_t>(literal(resolve(lanes->ops[2])).low) & 0xff;
    // Each two-bit selector names a dword of the source. The scalar
    // supplies one dword (f32) or two (f64); no other lane is observed.
    return (from == VectorOp::Fromf32 && lanes_read == 0) ||
           (from == VectorOp::Fromf64 && (lanes_read & 0xaa) == 0);
  }
  if (op == VectorOp::Unpcklpd)
    return from == VectorOp::Fromf64; // the whole low qword is read
  if (op != VectorOp::Minps && op != VectorOp::Minpd && op != VectorOp::Maxps &&
      op != VectorOp::Maxpd)
    return false;
  const Instruction *read = single_user(*lanes);
  if (read == nullptr || read->op != lowir::Opcode::Vector)
    return false;
  const VectorOp to = static_cast<VectorOp>(read->sub);
  const bool wide_operation = op == VectorOp::Minpd || op == VectorOp::Maxpd;
  return (to == VectorOp::Tof32 || to == VectorOp::Tof64) &&
         (from == VectorOp::Fromf64 ||
          (!wide_operation && to == VectorOp::Tof32));
}

void FunctionSelector::emit_vector_from_scalar(const Instruction &inst) {
  const VectorOp op = static_cast<VectorOp>(inst.sub);
  const TypeKind kind = lowir::vector_scalar_kind(op);
  const Operand src =
      Type::scalar(kind).is_float()
          ? rm_operand(inst.ops[0], Type::scalar(kind), kSelectScratch)
          : reg_operand(inst.ops[0], Type::scalar(kind), kSelectScratch);
  const Reg d = result_reg(inst, dying_reg(inst.ops[0]));
  // A load or a literal already clears the other lanes.
  if (src.kind == OperandKind::Reg && !(level_ >= 1 && low_lane_only(inst)))
    emit2(Op::Vector, TypeKind::V128, Operand::make_reg(d), src).sub = inst.sub;
  else
    load_into(d, src, kind);
  finish_result(inst, d);
}

void FunctionSelector::emit_vector_to_scalar(const Instruction &inst) {
  const VectorOp op = static_cast<VectorOp>(inst.sub);
  const TypeKind kind = lowir::vector_scalar_kind(op);
  if (Type::scalar(kind).is_float()) {
    const Operand src = vector_register(inst.ops[0], kX15);
    const Reg d = result_reg(inst, dying_reg(inst.ops[0]));
    load_into(d, src, TypeKind::V128);
    finish_result(inst, d);
    return;
  }
  const Operand src = vector_register(inst.ops[0], kX15);
  const Reg d = result_reg(inst);
  emit2(Op::Vector, TypeKind::V128, Operand::make_reg(d), src).sub = inst.sub;
  normalize(d, kind);
  finish_result(inst, d);
}

void FunctionSelector::emit_vector(const Instruction &inst) {
  const VectorOp op = static_cast<VectorOp>(inst.sub);
  const VectorShape shape = lowir::shape_of(op);

  lowir::Literal bits{};
  if (level_ >= 1 && vector_constant(inst, bits)) {
    const Reg d = result_reg(inst);
    emit2(Op::FMov, TypeKind::V128, Operand::make_reg(d),
          Operand::make_fimm(TypeKind::V128, bits));
    finish_result(inst, d);
    return;
  }
  switch (shape) {
  case VectorShape::Binary:
  case VectorShape::BinaryImmediate:
  case VectorShape::Blend:
    emit_vector_two_operand(inst);
    break;
  case VectorShape::FromScalar:
    emit_vector_from_scalar(inst);
    break;
  case VectorShape::ToScalar:
    emit_vector_to_scalar(inst);
    break;
  case VectorShape::Zero: {
    const Reg d = result_reg(inst);
    emit1(Op::Vector, TypeKind::V128, Operand::make_reg(d)).sub = inst.sub;
    finish_result(inst, d);
    break;
  }
  case VectorShape::Unary:
  case VectorShape::UnaryImmediate:
  case VectorShape::Mask: {
    const Operand a = vector_register(inst.ops[0], kX15);
    const bool immediate = shape == VectorShape::UnaryImmediate;
    const Reg d = shape == VectorShape::Mask
                      ? result_reg(inst)
                      : result_reg(inst, dying_reg(inst.ops[0]));
    Inst &mir = emit2(Op::Vector, TypeKind::V128, Operand::make_reg(d), a);
    mir.sub = inst.sub;
    if (immediate)
      mir.aux = static_cast<uint32_t>(literal(resolve(inst.ops[2])).low);
    finish_result(inst, d);
    break;
  }
  case VectorShape::Test: {
    const Operand a = vector_register(inst.ops[0], kX14);
    const Operand b = vector_register(inst.ops[1], kX15);
    const Reg d = result_reg(inst);
    emit2(Op::Vector, TypeKind::V128, a, b).sub = inst.sub;
    Inst &set = emit1(Op::Setcc, TypeKind::Void, Operand::make_reg(d));
    set.sub = test_cond(op);
    emit2(Op::Movzx, TypeKind::Void, Operand::make_reg(d),
          Operand::make_reg(d));
    finish_result(inst, d);
    break;
  }
  }
}

} // namespace native
