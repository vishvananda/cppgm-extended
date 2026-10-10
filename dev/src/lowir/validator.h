#pragma once

#include <string>

#include "lowir/program.h"

namespace lowir {

// A validity violation, rendered only when reported.
struct Diagnostic
{
	const char* message = nullptr;
	// A name the message is about (a symbol, temporary, block or slot).
	NameId detail = 0;
	// The function concerned, if any, and the instruction within it.
	SymbolId function = kNoId;
	InstId inst = kNoId;
};

// Checks a complete program against the LowIR structural and type rules
// (lowir.md "Structural Well-Formedness", the phi, call-boundary, return,
// conversion and metadata constraints). Full validation is for external
// LowIR and audit builds; producers need not revalidate unchanged
// programs. Returns false with the first violation found.
bool validate_program(const Program& program, Diagnostic& diagnostic);

// validate_program for a program built through FunctionBuilder (the
// exercises, audit builds of later producers). It first checks what the
// text reader's grammar guarantees for its programs: every id is in range,
// each operand has a kind its position allows, instruction fields are in
// range, exactly the value instructions define a result temporary of
// their result type, and (unless `spellings` is false: a producer that
// names its entities only when it finishes, as the optimizer's passes do)
// the parameters of a signature and the temporaries, slots and blocks of
// a body print as distinct names.
bool validate_constructed_program(const Program& program, Diagnostic& diagnostic, bool spellings = true);

// "message: detail in function @name".
std::string describe(const Program& program, const Diagnostic& diagnostic);

// In a program without an explicit entry role, gives the legacy spellings
// @main, @__cppgm_init and @__cppgm_fini the entry, init and fini roles
// when they are function definitions without a role and no symbol has that
// role explicitly (lowir.md "Reserved Runtime Hooks"); the writer then
// prints the roles explicitly.
void apply_legacy_roles(Program& program);

} // namespace lowir
