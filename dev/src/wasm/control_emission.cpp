#include "wasm/selector.h"

#include "wasm/exceptions.h"

namespace wasm {

using namespace lowir;

bool Selector::branch_label(uint32_t target)
{
	for (auto label = labels_.rbegin(); label != labels_.rend(); ++label)
		if (label->target == target)
		{
			out_.emit(Op::Br, { nesting_ - 1 - label->depth });
			return true;
		}
	return false;
}

void Selector::open_label(Op op, BlockId target)
{
	out_.emit(op, { block_type(ValueType::Void) });
	labels_.push_back(Label{ target, nesting_++ });
}

void Selector::close_label()
{
	out_.emit(Op::End);
	labels_.pop_back();
	--nesting_;
}

void Selector::region_child(uint32_t child)
{
	if (child < control_.block_count)
		block(child);
	else
	{
		const uint32_t id = child - control_.block_count + 1;
		if (control_.regions[id].dispatch)
			dispatch_region(id);
		else
		{
			open_label(Op::Loop, control_.regions[id].header);
			region(id);
			close_label();
		}
	}
}

void Selector::dispatch_region(uint32_t id)
{
	const auto& children = control_.regions[id].children;
	const uint32_t count = static_cast<uint32_t>(children.size());
	dispatch_labels_.push_back(Label{ id, nesting_ });
	open_label(Op::Loop, kNoId);
	for (uint32_t i = 0; i <= count; ++i)
		open_label(Op::Block, kNoId);
	out_.emit(Op::LocalGet, { region_locals_[id] });
	out_.emit(Op::BrTable);
	for (uint32_t i = 0; i <= count; ++i)
		out_.operands.push_back(i);
	out_.instructions.back().operands.count = count + 1;
	for (uint32_t child : children)
	{
		close_label();
		region_child(child);
	}
	close_label();
	out_.emit(Op::Unreachable);
	close_label();
	dispatch_labels_.pop_back();
	out_.emit(Op::Unreachable);
}

void Selector::region(uint32_t id)
{
	const auto& children = control_.regions[id].children;
	auto entry = [&](uint32_t child) {
		return child >= control_.block_count && control_.regions[child - control_.block_count + 1].dispatch
			? child
			: control_.entry(child);
	};
	for (uint32_t i = static_cast<uint32_t>(children.size()); i > 1; --i)
		open_label(Op::Block, entry(children[i - 1]));
	for (uint32_t i = 0; i < children.size(); ++i)
	{
		if (i != 0)
			close_label();
		region_child(children[i]);
	}
}

bool Selector::structured()
{
	const opt::Cfg* edges = exceptions_ != nullptr && exceptions_->active ? &exceptions_->control : nullptr;
	if (!structure_control(program_, body_, control_, edges))
		return false;
	if (!control_.dispatch_region.empty())
	{
		region_locals_.assign(control_.regions.size(), kNoId);
		for (uint32_t i = 0; i < control_.regions.size(); ++i)
			if (control_.regions[i].dispatch)
				region_locals_[i] = add_local(ValueType::I32);
	}
	region(0);
	out_.emit(Op::Unreachable);
	return true;
}

} // namespace wasm
