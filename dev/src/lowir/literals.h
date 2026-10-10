#pragma once

#include <cstdint>
#include <string>

#include "lowir/program.h"

// LowIR scalar literals: scanning numeric spellings into Literal bits and
// writing Literal bits back as canonical spellings (lowir.md "Constants and
// Copies"). A literal's kind (integer or floating) is part of the program:
// `3` stays an integer literal in a floating context.
namespace lowir {

// True if a numeric spelling is floating: it has a '.', an exponent (a 'p'
// for hexadecimal) or, when decimal, a floating suffix (`1f`).
bool is_floating_spelling(const char* text, uint32_t length);

// Scans a decimal, octal (leading 0) or hexadecimal integer spelling, negated
// if `negative`, as 128-bit two's complement modulo 2^128; unless `wide` (an
// i128 context), the value is then wrapped to 64 bits and sign extended.
// Returns false for a malformed spelling.
bool scan_integer_literal(const char* text, uint32_t length, bool negative, bool wide, Literal& bits);

// Scans a floating spelling with an optional f/F (f32) or l/L (f80) suffix.
// The literal's kind is `context` if that is a floating kind, otherwise the
// suffix's kind (f64 without one); the value is rounded once, to that kind.
// Returns false for a malformed spelling or a value beyond the kind's range.
// `scratch` receives the NUL-terminated copy the C library scanner needs.
bool scan_float_literal(const char* text, uint32_t length, bool negative, TypeKind context, TypeKind& kind,
	Literal& bits, std::string& scratch);

enum class SpecialFloat : uint8_t
{
	Infinity,
	QuietNaN,
	SignalingNaN
};

Literal special_float(TypeKind kind, SpecialFloat which, bool negative);

// Object mode (Program::object_facts): the NaN `which` whose fraction bits
// below the quiet bit are `payload` in place of the default ones (an f80
// keeps its integer bit), as `nan(<payload>)` and `snan(<payload>)` spell
// it. False if the payload does not fit below the quiet bit, or leaves a
// signaling NaN no fraction bit (an infinity).
bool nan_with_payload(TypeKind kind, SpecialFloat which, bool negative, uint64_t payload, Literal& bits);

// The bits of `value` rounded to a floating kind.
Literal float_literal(TypeKind kind, long double value);

// The value a floating literal's bits denote (exact in long double).
long double float_value(TypeKind kind, const Literal& bits);

// The floating literal of an integer literal's value (`negative_zero`: a
// zero written `-0`), rounded once to `kind`: an integer literal in a
// floating context is its value.
Literal integer_as_float(TypeKind kind, const Literal& bits, bool negative_zero = false);

// A literal is its value, not its spelling (lowir.md "What The Comparison
// Absorbs"): each value has one spelling in each context.
//
// An integer literal is its decimal value as an operand of `context`:
// folded to an integer kind's width, signed for iN and unsigned for uN, i1
// and ptr (`i32 -1`, `u32 4294967295`); with no context (Void: a call's or
// a switch's operand, whose use decides) the 128 bits stored, signed.
void append_integer_literal(std::string& out, const Literal& bits, TypeKind context = TypeKind::Void);

// A floating literal in the course's spelling (its lowir writer): 20
// significant digits of the value with trailing zeros dropped (21 for an
// f80 that needs them to read back), `8`, `-0`, `0.10000000000000000555`,
// `1.0000000000000000199e+30`, with an `f` (f32) or `L` (f80) suffix; a
// subnormal value as exact hexadecimal floating text; `inf`, `-inf`, `nan`,
// `-nan`, `snan`, `-snan` (an f32 or f80 one with its literal suffix,
// `INFINITYf`, `nanL`, as the course's LowIR writes them). `marked` (no type
// gives the kind: an argument to `...`, or of a call whose boundary is not
// known yet) keeps an f64 floating there: a spelling without a fraction or
// exponent gets `.0`. With `exact` (object mode) a NaN reads back bit for
// bit where the course's `nan` keeps only its sign and quiet bit: fraction
// bits below the quiet bit other than its spelling's default ones are
// stated in hexadecimal, `nan(0x55)`, `-snanf(0x2)` (nan_with_payload).
void append_float_literal(
	std::string& out, TypeKind kind, const Literal& bits, bool exact = false, bool marked = false);

} // namespace lowir
