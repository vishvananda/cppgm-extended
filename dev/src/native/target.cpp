#include "native/target.h"

namespace native {

bool parse_target(const std::string& text, Target& result)
{
	return target::parse(text, result);
}

} // namespace native
