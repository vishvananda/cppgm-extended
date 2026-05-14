#pragma once
#include <stdexcept>

static constexpr int CPPGM_EXIT_NOT_IMPLEMENTED = 86;

class NotImplementedException : public std::logic_error
{
public:
    NotImplementedException() : std::logic_error("not yet implemented") {}
};
