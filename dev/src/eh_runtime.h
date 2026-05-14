#pragma once

#include <string>

namespace eh_runtime {

extern const char kEhTopSymbol[];
extern const char kEhValueSymbol[];
extern const char kEhTypeSymbol[];
extern const char kEhUnhandledSymbol[];
extern const char kPureVirtualSymbol[];
extern const char kEhTopObjectSymbol[];
extern const char kEhValueObjectSymbol[];
extern const char kEhTypeObjectSymbol[];
extern const char kEhUnhandledObjectSymbol[];

bool is_reserved_symbol(const std::string & name);

}  // namespace eh_runtime
