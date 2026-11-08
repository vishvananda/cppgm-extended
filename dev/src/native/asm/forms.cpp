#include "native/asm/forms.h"

#include <cstring>

namespace native {
namespace asm_detail {
namespace {

using E = Encoding;

const Form* find(const std::string& mnemonic)
{
	std::size_t count = 0;
	const Form* forms = x86_asm_model::forms(count);
	for (std::size_t i = 0; i < count; ++i)
		if (mnemonic == forms[i].mnemonic)
			return forms + i;
	return nullptr;
}

} // namespace

const Form* assembly_form(const std::string& mnemonic, uint16_t& suffix)
{
	suffix = 0;
	if (const Form* form = find(mnemonic))
		return form;
	if (mnemonic.size() < 2)
		return nullptr;
	const char last = mnemonic.back();
	suffix = last == 'b' ? 8 : last == 'w' ? 16 : last == 'l' ? 32 : last == 'q' ? 64 : 0;
	if (suffix == 0)
		return nullptr;
	const Form* form = find(mnemonic.substr(0, mnemonic.size() - 1));
	return form != nullptr && form->encoding >= E::Move && form->encoding <= E::Alu ? form : nullptr;
}

} // namespace asm_detail
} // namespace native
