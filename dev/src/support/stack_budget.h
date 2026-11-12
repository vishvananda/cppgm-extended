#pragma once

// A recursion budget measured in stack bytes: half the stack limit (the
// rest is left for callers and the runtime), at most 256 MiB, 4 MiB when
// the limit is unknown. Recursive phases record their base address and
// reject input nested beyond the budget instead of overflowing the stack.

#include <cstdint>

#ifndef __wasi__
#include <sys/resource.h>
#endif

inline std::uintptr_t stack_budget_bytes()
{
#ifdef __wasi__
	// The experimental WASI build reserves an 8 MiB linear-memory stack.
	// Engine call-stack limits are qualified separately by the runner.
	return 4u << 20;
#else
	const std::uintptr_t kFallback = 4u << 20;
	const std::uintptr_t kCap = 256u << 20;
	struct rlimit limit;
	if (getrlimit(RLIMIT_STACK, &limit) != 0)
		return kFallback;
	if (limit.rlim_cur == RLIM_INFINITY || limit.rlim_cur / 2 > kCap)
		return kCap;
	return static_cast<std::uintptr_t>(limit.rlim_cur / 2);
#endif
}

// Bytes of stack between `base` (an address recorded higher in the same
// thread's call chain) and `here`. Both arguments are addresses: no bytes
// of the stack markers are read.
inline std::uintptr_t stack_bytes_used(std::uintptr_t base, std::uintptr_t here)
{
	return base > here ? base - here : here - base;
}
