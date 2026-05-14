#pragma once

#include <cstddef>
#include <map>
#include <string>
#include <vector>

#include "machine_ir.h"
#include "machine_object.h"

namespace host_eh_object_sections {

struct HostEhCallSite
{
  std::size_t start = 0;
  std::size_t length = 0;
  std::size_t landingpad_offset = 0;
  std::string landingpad_symbol;
};

struct HostEhFunctionInfo
{
  std::string function_name;
  std::size_t function_offset = 0;
  std::size_t function_size = 0;
  std::vector<HostEhCallSite> call_sites;
};

struct HostEhFunctionLayout
{
  std::map<std::string, std::size_t> block_offsets;
  std::size_t size = 0;
};

struct HostEhObjectLayout
{
  std::map<std::string, std::size_t> function_offsets;
  std::map<std::string, HostEhFunctionLayout> function_layouts;
};

void append_host_eh_sections(const machine_ir::Program & program,
                             const HostEhObjectLayout & layout,
                             const std::vector<HostEhFunctionInfo> & host_eh_functions,
                             machine_object::ObjectFile & object);

}  // namespace host_eh_object_sections
