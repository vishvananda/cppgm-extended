#include "lowir_driver_frontend.h"

#include "cli_batch_frontend.h"

#include <cstdlib>
#include <fstream>
#include <stdexcept>
#include <string>
#include <vector>

using namespace std;

#include "lowir_internal.h"
#include "lowir_machine_ir.h"
#include "lowir_object_backend.h"
#include "lowir_tool_cli.h"
#include "machine_ir.h"
#include "machine_ir_optimizer.h"
#include "machine_linker.h"
#include "optimization_level.h"

namespace {

int run_lowir_link_frontend(const vector<string> & args, bool exception_link)
{
  const LowIRToolInvocation invocation = parse_lowir_tool_invocation(args);
  if(invocation.compile_only) {
    write_lowir_object_file(invocation.inputs,
                            invocation.outfile,
                            invocation.output_target);
  } else if(exception_link) {
    link_exception_objects_to_native(invocation.inputs,
                                     invocation.outfile,
                                     invocation.mapfile);
  } else {
    link_machine_objects_to_native(invocation.inputs,
                                   invocation.outfile,
                                   invocation.mapfile);
  }
  return EXIT_SUCCESS;
}

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

  const lowir_internal::Program program = lowir_internal::parse_program(srcfiles);

  if(!machine_ir_file.empty()) {
    ofstream mir(machine_ir_file.c_str());
    if(!mir) {
      throw logic_error("unable to open machine IR file");
    }
    const machine_ir::Program machine_program =
        optimize_machine_ir_program(build_lowir_machine_ir(program, output_target),
                                    optimization_level);
    mir << machine_ir::dump_program(machine_program);
  }

  if(!outfile.empty()) {
    const machine_object::ObjectFile object =
        build_machine_object(program,
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

int run_cpplink_frontend(int argc, char ** argv)
{
  return run_cli_frontend(
      argc,
      argv,
      [](const vector<string> & args) {
        return run_lowir_link_frontend(args, false);
      });
}

int run_cppeh_frontend(int argc, char ** argv)
{
  return run_cli_frontend(
      argc,
      argv,
      [](const vector<string> & args) {
        return run_lowir_link_frontend(args, true);
      });
}

int run_lowir2native_frontend(int argc, char ** argv)
{
  return run_cli_frontend(argc, argv, run_lowir2native_impl);
}
