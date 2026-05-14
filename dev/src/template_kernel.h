#pragma once

#include <string>

namespace template_kernel {

bool run_file(const std::string & input_path,
              const std::string & output_path,
              std::string & error);

}  // namespace template_kernel
