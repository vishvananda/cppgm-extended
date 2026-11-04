#include "native/select/selector.h"

#include "native/select/function_selector.h"

namespace native {

Selector::Selector(const lowir::Program& program) : program_(program), state_(new FunctionSelector(program, stats_))
{}

Selector::~Selector()
{
	delete state_;
}

void Selector::select(lowir::SymbolId function, Function& out, uint32_t extra_preserved_gprs)
{
	state_->select(function, out, extra_preserved_gprs);
}

void Selector::set_eh_model(EhModel model)
{
	state_->set_eh_model(model);
}

void Selector::set_position_independent(bool enabled)
{
	state_->set_position_independent(enabled);
}

void Selector::set_optimization_level(int level)
{
	state_->set_optimization_level(level);
}

} // namespace native
