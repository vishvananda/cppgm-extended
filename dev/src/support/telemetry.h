#pragma once

#include <cstdint>

// Telemetry (spec.md section 9): the work counters, sizes and phase clocks
// that `--stats` and `cppgm++ --stats-functions` report. They only observe
// work the compiler does anyway. A build with -DCPPGM_TELEMETRY_ENABLED=0
// compiles them out: each counter below keeps nothing, the reports are not
// built, and the tools reject the statistics options ("statistics are
// unavailable in the telemetry-off build"). Its output is otherwise the
// default build's.
#ifndef CPPGM_TELEMETRY_ENABLED
#define CPPGM_TELEMETRY_ENABLED 1
#endif

namespace telemetry {

const bool kEnabled = CPPGM_TELEMETRY_ENABLED != 0;

// The message of a statistics option given to a telemetry-off build.
const char* const kUnavailable = "statistics are unavailable in the telemetry-off build";

#if CPPGM_TELEMETRY_ENABLED

typedef uint64_t Counter;
typedef uint32_t SmallCounter;
typedef double Milliseconds;

// Raises a high-water mark to `value`.
template<typename Peak, typename Value>
inline void note_peak(Peak& peak, Value value)
{
	if (value > peak)
		peak = value;
}

#else

// A counter that counts nothing. It has no value: code that reads one is
// not telemetry, and does not compile here.
template<typename T>
class NullCounter
{
public:
	NullCounter() {}
	NullCounter(T) {}
	NullCounter& operator=(T) { return *this; }
	NullCounter& operator++() { return *this; }
	NullCounter& operator++(int) { return *this; }
	NullCounter& operator--() { return *this; }
	NullCounter& operator+=(T) { return *this; }
	NullCounter& operator+=(const NullCounter&) { return *this; }
	NullCounter& operator-=(T) { return *this; }
};

typedef NullCounter<uint64_t> Counter;
typedef NullCounter<uint32_t> SmallCounter;
typedef NullCounter<double> Milliseconds;

template<typename T, typename Value>
inline void note_peak(NullCounter<T>&, Value)
{}

#endif

} // namespace telemetry
