#include "native/asm/unwind_rules.h"

#include "native/backend_error.h"

#include "native/object/dwarf_bytes.h"
#include <climits>

namespace native {
namespace {
[[noreturn]] void bad(const AssemblyCfiEvent& event, const char* message)
{
	throw BackendError("assembly CFI line " + std::to_string(event.line) + ": " + message);
}
int64_t difference(const AssemblyCfiEvent& event, int64_t a, int64_t b)
{
	if ((b > 0 && a < INT64_MIN + b) || (b < 0 && a > INT64_MAX + b))
		bad(event, "CFI offset arithmetic exceeds 64 bits");
	return a - b;
}
int64_t factor(const AssemblyCfiEvent& event, int64_t offset)
{
	if (offset % 8 != 0)
		bad(event, "CFI saved offset must be a multiple of eight");
	return offset / -8;
}
} // namespace
AssemblyCfiRules::AssemblyCfiRules(bool simple)
{
	state_.offset_known = !simple;
}
void AssemblyCfiRules::advance(uint32_t pc)
{
	if (pc < pc_)
		throw BackendError("assembly CFI line " + std::to_string(1) + ": " + "CFI locations are not ordered");
	const uint32_t delta = pc - pc_;
	DwarfBytes out(bytes_);
	if (delta < 64)
	{
		if (delta != 0)
			out.fixed(0x40 + delta, 1);
	}
	else if (delta <= 255)
	{
		out.fixed(0x02, 1);
		out.fixed(delta, 1);
	}
	else if (delta <= 65535)
	{
		out.fixed(0x03, 1);
		out.fixed(delta, 2);
	}
	else
	{
		out.fixed(0x04, 1);
		out.fixed(delta, 4);
	}
	pc_ = pc;
}
void AssemblyCfiRules::cfa(const AssemblyCfiEvent& event)
{
	DwarfBytes out(bytes_);
	if (event.op == AssemblyCfiOp::DefRegister)
	{
		state_.reg = event.reg;
		out.fixed(0x0d, 1);
		out.uleb(event.reg);
		return;
	}
	if (event.op == AssemblyCfiOp::Adjust)
	{
		if (!state_.offset_known)
			bad(event, "relative CFI adjustment has no known CFA offset");
		if (event.value == INT64_MIN)
		{
			if (state_.offset < 0)
				bad(event, "CFI offset arithmetic exceeds 64 bits");
			state_.offset = static_cast<int64_t>(uint64_t(state_.offset) + uint64_t(event.value));
		}
		else
			state_.offset = difference(event, state_.offset, -event.value);
	}
	else
	{
		state_.offset = event.value;
		state_.offset_known = true;
	}
	const bool complete = event.op == AssemblyCfiOp::DefCfa;
	if (complete)
		state_.reg = event.reg;
	if (state_.offset >= 0)
	{
		out.fixed(complete ? 0x0c : 0x0e, 1);
		if (complete)
			out.uleb(state_.reg);
		out.uleb(static_cast<uint64_t>(state_.offset));
	}
	else
	{
		out.fixed(complete ? 0x12 : 0x13, 1);
		if (complete)
			out.uleb(state_.reg);
		out.sleb(factor(event, state_.offset));
	}
}
void AssemblyCfiRules::saved(const AssemblyCfiEvent& event)
{
	int64_t offset = event.value;
	if (event.op == AssemblyCfiOp::RelOffset)
	{
		if (!state_.offset_known)
			bad(event, "relative saved register has no known CFA offset");
		offset = difference(event, offset, state_.offset);
	}
	const int64_t units = factor(event, offset);
	DwarfBytes out(bytes_);
	const bool value = event.op == AssemblyCfiOp::ValOffset;
	if (units >= 0)
	{
		if (!value && event.reg < 64)
			out.fixed(0x80 + event.reg, 1);
		else
		{
			out.fixed(value ? 0x14 : 0x05, 1);
			out.uleb(event.reg);
		}
		out.uleb(static_cast<uint64_t>(units));
	}
	else
	{
		out.fixed(value ? 0x15 : 0x11, 1);
		out.uleb(event.reg);
		out.sleb(units);
	}
}
void AssemblyCfiRules::apply(const AssemblyCfiEvent& event, uint32_t pc)
{
	advance(pc);
	DwarfBytes out(bytes_);
	switch (event.op)
	{
	case AssemblyCfiOp::DefCfa:
	case AssemblyCfiOp::DefRegister:
	case AssemblyCfiOp::DefOffset:
	case AssemblyCfiOp::Adjust:
		cfa(event);
		break;
	case AssemblyCfiOp::Offset:
	case AssemblyCfiOp::RelOffset:
	case AssemblyCfiOp::ValOffset:
		saved(event);
		break;
	case AssemblyCfiOp::Register:
		out.fixed(0x09, 1);
		out.uleb(event.reg);
		out.uleb(event.second);
		break;
	case AssemblyCfiOp::Restore:
		if (event.reg < 64)
			out.fixed(0xc0 + event.reg, 1);
		else
		{
			out.fixed(0x06, 1);
			out.uleb(event.reg);
		}
		break;
	case AssemblyCfiOp::Undefined:
	case AssemblyCfiOp::Same:
		out.fixed(event.op == AssemblyCfiOp::Undefined ? 0x07 : 0x08, 1);
		out.uleb(event.reg);
		break;
	case AssemblyCfiOp::Remember:
		if (stack_.size() >= 64)
			bad(event, "CFI state stack exceeds limit");
		stack_.push_back(state_);
		out.fixed(0x0a, 1);
		break;
	case AssemblyCfiOp::RestoreState:
		if (stack_.empty())
			bad(event, "CFI restore has no remembered state");
		state_ = stack_.back();
		stack_.pop_back();
		out.fixed(0x0b, 1);
		break;
	case AssemblyCfiOp::Escape:
		// GNU escape bytes do not change the assembler's logical CFA state.
		bytes_.insert(bytes_.end(), event.bytes.begin(), event.bytes.end());
		break;
	default:
		bad(event, "CFI metadata cannot appear in the frame rule stream");
	}
}
} // namespace native
