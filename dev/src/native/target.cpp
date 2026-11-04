#include "native/target.h"

namespace native {

bool parse_target(const std::string& text, Target& target)
{
	const std::string darwin = "x86_64-apple-darwin";
	const std::string macos = "x86_64-apple-macos";
	if (text == "macos" || text == "darwin" || text == "osx" || text.compare(0, darwin.size(), darwin) == 0 ||
		text.compare(0, macos.size(), macos) == 0)
	{
		target = Target::MacOS;
		return true;
	}
	if (text == "linux")
	{
		target = Target::Linux;
		return true;
	}
	if (text.compare(0, 7, "x86_64-") != 0)
		return false;
	const size_t at = text.find("linux", 7);
	const bool vendor = at == 7 || (at != std::string::npos && text[at - 1] == '-' && text.find('-', 7) == at - 1);
	if (!vendor || (at + 5 != text.size() && text[at + 5] != '-'))
		return false;
	target = Target::Linux;
	return true;
}

} // namespace native
