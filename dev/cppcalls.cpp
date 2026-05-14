// (C) 2013 CPPGM Foundation www.cppgm.org.  All rights reserved.

#include "cpp_batch_frontend.h"
#include "cpp_text_generators.h"

int main(int argc, char** argv)
{
  return run_cpp_text_frontend(
      argc,
      argv,
      [](const std::vector<std::string> & srcfiles) {
        return generate_calls_translation_units(srcfiles);
      });
}
