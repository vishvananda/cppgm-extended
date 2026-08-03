#include "lowir_driver_frontend.h"

#include "cli_batch_frontend.h"

#include <cstdlib>
#include <fstream>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

using namespace std;

#include "lowir_machine_ir.h"
#include "lowir_object_backend.h"
#include "lowir_optimizer.h"
#include "lowir_tool_cli.h"
#include "machine_ir_optimizer.h"
#include "machine_linker.h"
#include "mir_model.h"
#include "optimization_level.h"

namespace {

int run_lowir2native_impl(const vector<string> & args)
{
  string output_target;
  string outfile;
  string machine_ir_file;
  int optimization_level = 0;
  vector<string> srcfiles;

  for(size_t i = 0; i < args.size(); ++i) {
    if(parse_optimization_level_arg(args[i], optimization_level)) {
      continue;
    }
    if(args[i] == "--target") {
      if(i + 1 >= args.size()) {
        throw logic_error("missing target after --target");
      }
      output_target = args[++i];
      continue;
    }
    if(args[i] == "--dump-machine-ir" || args[i] == "--dump-native-plan") {
      if(i + 1 >= args.size()) {
        throw logic_error("missing output file after --dump-machine-ir");
      }
      machine_ir_file = args[++i];
      continue;
    }
    if(args[i] == "-o") {
      if(i + 1 >= args.size()) {
        throw logic_error("missing output file after -o");
      }
      outfile = args[++i];
      continue;
    }
    srcfiles.push_back(args[i]);
  }

  if((outfile.empty() && machine_ir_file.empty()) || srcfiles.empty()) {
    throw logic_error("invalid usage");
  }
  optimization_level = normalize_optimization_level(optimization_level);

  lowir_model::LowirProgram program =
      inline_required_lowir_calls(lowir_model::parse_lowir_program_files(srcfiles));

  if(!machine_ir_file.empty()) {
    ofstream mir(machine_ir_file.c_str());
    if(!mir) {
      throw logic_error("unable to open machine IR file");
    }
    const mir_model::MirProgram machine_program =
        optimize_machine_ir_program(build_lowir_machine_ir(program, output_target),
                                    optimization_level);
    mir << mir_model::serialize_mir_program(machine_program);
  }

  if(!outfile.empty()) {
    const machine_object::ObjectFile object =
        build_machine_object(std::move(program),
                            output_target,
                            false,
                            false,
                            0,
                            optimization_level,
                            true);
    vector<machine_object::ObjectFile> objects(1, object);
    link_machine_objects_to_native(objects, outfile, string());
  }

  return EXIT_SUCCESS;
}

}  // namespace

int run_lowir2native_frontend(int argc, char ** argv)
{
  return run_cli_frontend(argc, argv, run_lowir2native_impl);
}
