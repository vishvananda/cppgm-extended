#include "native_format.h"

#include <stdexcept>

using namespace std;

#include "elf_writer.h"
#include "macho_writer.h"

namespace {

void write_linux_x86_64_executable(const string & outfile,
                                   const vector<unsigned char> & payload,
                                   uint64_t entry_offset,
                                   uint64_t base_vaddr)
{
  write_elf_x86_64_executable(outfile, payload, entry_offset, base_vaddr);
}

void write_macos_x86_64_executable(const string & outfile,
                                   const vector<unsigned char> & payload,
                                   uint64_t entry_offset,
                                   uint64_t base_vaddr)
{
  write_macho_x86_64_executable(outfile, payload, entry_offset, base_vaddr);
}

const native_format::Hooks kLinuxHooks = {
    cy86_internal::NT_LINUX,
    "linux",
    0x400000ULL,
    60ULL,
    &write_linux_x86_64_executable};

const native_format::Hooks kMacosHooks = {
    cy86_internal::NT_MACOS,
    "macos",
    0x100000000ULL,
    0x2000001ULL,
    &write_macos_x86_64_executable};

}  // namespace

namespace native_format {

const Hooks & hooks_for_target(cy86_internal::NativeTarget target)
{
  switch(target) {
    case cy86_internal::NT_LINUX:
      return kLinuxHooks;
    case cy86_internal::NT_MACOS:
      return kMacosHooks;
  }
  throw logic_error("unknown native target");
}

const Hooks & hooks_for_target_text(const string & target)
{
  if(target == "linux") {
    return kLinuxHooks;
  }
  if(target == "macos" || target == "osx" || target == "darwin") {
    return kMacosHooks;
  }
  throw logic_error("unknown object target " + target);
}

}  // namespace native_format
