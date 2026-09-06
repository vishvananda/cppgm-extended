#pragma once

#include <stdexcept>

// The student scaffolds of the assignment export throw this from every stage
// the student has not written yet, and exit with the status below; the
// harness reads that status as "not implemented" rather than "wrong".
static constexpr int CPPGM_EXIT_NOT_IMPLEMENTED = 86;

class NotImplementedException : public std::logic_error
{
public:
	NotImplementedException() : std::logic_error("not yet implemented") {}
};
