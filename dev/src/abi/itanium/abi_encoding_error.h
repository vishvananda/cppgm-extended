#pragma once

#include <stdexcept>
#include <string>

namespace abi_mangle {

// A target the Itanium grammar cannot encode (AbiEncoder::encode): a node
// of the wrong kind where a production needs a type, name or expression, or
// a name nested beyond the encoder's stack budget. abimangle's fact files
// can state such a target, so it is the input's error.
class EncodingError : public std::runtime_error
{
public:
	explicit EncodingError(const char* message) : std::runtime_error(message) {}
	explicit EncodingError(const std::string& message) : std::runtime_error(message) {}
};

} // namespace abi_mangle
