#include "target/target.h"

namespace target {

bool parse(const std::string& text, Target& result)
{
	if (text == spelling(Target::Wasm32))
	{
		result = Target::Wasm32;
		return true;
	}
	if (text == spelling(Target::Wasi))
	{
		result = Target::Wasi;
		return true;
	}
	const std::string darwin = "x86_64-apple-darwin";
	const std::string macos = "x86_64-apple-macos";
	if (text == "macos" || text == "darwin" || text == "osx" || text.compare(0, darwin.size(), darwin) == 0 ||
		text.compare(0, macos.size(), macos) == 0)
	{
		result = Target::MacOS;
		return true;
	}
	if (text == "linux")
	{
		result = Target::Linux;
		return true;
	}
	if (text.compare(0, 7, "x86_64-") != 0)
		return false;
	const size_t at = text.find("linux", 7);
	const bool vendor = at == 7 || (at != std::string::npos && text[at - 1] == '-' && text.find('-', 7) == at - 1);
	if (!vendor || (at + 5 != text.size() && text[at + 5] != '-'))
		return false;
	result = Target::Linux;
	return true;
}

} // namespace target
