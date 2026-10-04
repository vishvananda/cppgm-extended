#!/usr/bin/env python3
"""Exercise collision/growth boundaries and the allocation-free small case."""

import os
from pathlib import Path
import shlex
import subprocess
import tempfile
import unittest


ROOT = Path(__file__).resolve().parents[2]
SOURCE = r'''
#include "support/containers/flat_hash_set.h"
#include <cassert>
#include <cstdlib>
#include <limits>
#include <new>

static unsigned allocations = 0;
void* operator new(std::size_t size)
{
    ++allocations;
    void* result = std::malloc(size ? size : 1);
    if (!result) throw std::bad_alloc();
    return result;
}
void operator delete(void* pointer) noexcept { std::free(pointer); }

struct CollidingHash
{
    std::size_t operator()(unsigned) const { return 0; }
};

int main()
{
    const unsigned start = allocations;
    cppgm::detail::FlatHashSet<unsigned, CollidingHash> visited;
    for (unsigned i = 0; i < 10; ++i) assert(visited.Insert(i));
    for (unsigned i = 0; i < 10; ++i) assert(!visited.Insert(i));
    assert(allocations == start);

    // All keys collide: preserve the inline keys through repeated growth,
    // including zero and the maximum identity (neither is a sentinel).
    for (unsigned i = 10; i < 2048; ++i) assert(visited.Insert(i));
    const unsigned maximum = std::numeric_limits<unsigned>::max();
    assert(visited.Insert(maximum));
    assert(allocations > start && allocations - start <= 9);
    const unsigned grown = allocations;
    for (unsigned i = 2048; i != 0; --i) assert(!visited.Insert(i - 1));
    assert(!visited.Insert(maximum));
    assert(allocations == grown);

    // Queries own their state: a second set must accept the same identities.
    cppgm::detail::FlatHashSet<unsigned> next;
    assert(next.Insert(0));
    assert(next.Insert(maximum));
    assert(!next.Insert(0));
    assert(!next.Insert(maximum));
    assert(allocations == grown);
}
'''


class FlatHashSetTests(unittest.TestCase):
    def test_collision_growth_and_inline_allocation(self):
        compiler = shlex.split(os.environ.get("CPPGM_HOST_CXX", "g++"))
        flags = shlex.split(os.environ.get("CPPGM_STDLIB_FLAGS", ""))
        with tempfile.TemporaryDirectory(prefix="flat-hash-set.") as temp:
            source = Path(temp) / "test.cpp"
            binary = Path(temp) / "test"
            source.write_text(SOURCE)
            subprocess.run(compiler + flags + ["-std=c++11", "-O2", "-Wall",
                           "-I", str(ROOT / "dev/src"), str(source),
                           "-o", str(binary)], check=True)
            subprocess.run([str(binary)], check=True)


if __name__ == "__main__":
    unittest.main()
