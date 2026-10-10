#pragma once

#include <cstdint>
#include <stdexcept>

// A preprocessing error ([cpp] 16): the translation unit is ill-formed and
// preprocessing stops. The location is a translation-unit location (see
// preprocess/source/source_files.h); the message is a static description,
// rendered with the location only when the error is reported.
class PreprocessError : public std::runtime_error
{
public:
	PreprocessError(uint32_t location, const char* message) : std::runtime_error(message), location_(location) {}

	uint32_t location() const { return location_; }

private:
	uint32_t location_;
};
