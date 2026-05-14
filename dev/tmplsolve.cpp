#include <iostream>
#include <string>

#include "template_kernel.h"

int main(int argc, char ** argv)
{
  if(argc != 4 || std::string(argv[1]) != "-o") {
    std::cerr << "usage: tmplsolve -o <outfile> <input.tkq>\n";
    return 1;
  }

  std::string error;
  if(!template_kernel::run_file(argv[3], argv[2], error)) {
    if(!error.empty()) {
      std::cerr << error << "\n";
    }
    return 1;
  }

  return 0;
}
