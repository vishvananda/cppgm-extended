#include "native/asm/forms.h"

#include <cstring>

namespace native {
namespace asm_detail {
int branch_condition(const std::string& mnemonic)
{
	for (const x86_asm_model::Branch& branch : x86_asm_model::branches)
		if (mnemonic == branch.name)
			return branch.condition;
	return -1;
}

namespace {

using E = Encoding;

const Form* find(const std::string& mnemonic, uint32_t operand_count)
{
	std::size_t count = 0;
	const Form* forms = x86_asm_model::forms(count);
	for (std::size_t i = 0; i < count; ++i)
		if (mnemonic == forms[i].mnemonic && (forms[i].encoding != E::String || operand_count == 0) &&
			(forms[i].encoding != E::VectorRM || operand_count != 0))
			return forms + i;
	return nullptr;
}

} // namespace

const Form* assembly_form(const std::string& mnemonic, uint16_t& suffix, uint32_t operand_count)
{
	suffix = 0;
	if (const Form* form = find(mnemonic, operand_count))
		return form;
	if (mnemonic.size() < 2)
		return nullptr;
	const char last = mnemonic.back();
	suffix = last == 'b' ? 8 : last == 'w' ? 16 : last == 'l' ? 32 : last == 'q' ? 64 : 0;
	if (suffix == 0)
		return nullptr;
	const Form* form = find(mnemonic.substr(0, mnemonic.size() - 1), operand_count);
	return form != nullptr && form->encoding >= E::Move &&
			(form->encoding <= E::Alu || (form->encoding >= E::Unary && form->encoding <= E::Atomic) ||
				form->encoding == E::Control ||
				(form->encoding >= E::ConditionMove && form->encoding <= E::DoubleShift))
		? form
		: nullptr;
}

} // namespace asm_detail
} // namespace native
