#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include "cy86_internal.h"

namespace native_format {

struct Hooks
{
  Hooks()
  {
  }

  Hooks(cy86_internal::NativeTarget target_in,
        const char * target_name_in,
        std::uint64_t base_vaddr_in,
        std::uint64_t exit_syscall_number_in,
        void (*write_x86_64_executable_in)(const std::string & outfile,
                                           const std::vector<unsigned char> & payload,
                                           std::uint64_t entry_offset,
                                           std::uint64_t base_vaddr))
      : target(target_in),
        target_name(target_name_in),
        base_vaddr(base_vaddr_in),
        exit_syscall_number(exit_syscall_number_in),
        write_x86_64_executable(write_x86_64_executable_in)
  {
  }

  cy86_internal::NativeTarget target = cy86_internal::NT_LINUX;
  const char * target_name = "linux";
  std::uint64_t base_vaddr = 0;
  std::uint64_t exit_syscall_number = 0;
  void (*write_x86_64_executable)(const std::string & outfile,
                                  const std::vector<unsigned char> & payload,
                                  std::uint64_t entry_offset,
                                  std::uint64_t base_vaddr) = nullptr;
};

const Hooks & hooks_for_target(cy86_internal::NativeTarget target);
const Hooks & hooks_for_target_text(const std::string & target);

}  // namespace native_format
