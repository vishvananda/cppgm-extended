#pragma once

#include <cstdint>

// Whether `text` is a ud-suffix ([lex.ext] 2.14.8) that a program may use:
// an identifier beginning with an underscore. Literal suffix identifiers
// that do not begin with an underscore are reserved for future
// standardization ([usrlit.suffix] 17.6.4.3.5), so a literal using one is
// diagnosed as ill-formed -- but by the compiler's dialect (`reserved`),
// where any identifier is one: the implementation's library declares
// literal operators of reserved suffixes (C++14's `s`, `if`, `h` ...),
// which lookup then finds or not.
bool IsUserDefinedSuffix(const char* text, uint32_t length, bool reserved = false);
