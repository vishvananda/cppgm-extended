#include "lowir/intro/exercises.h"
#include "lowir/model/program.h"
#include "lowir/model/operand_view.h"
#include "support/driver_errors.h"
#include "support/exception_types.h"
#include "support/tool_help_text.h"

#include <cstdlib>
#include <iostream>
#include <string>
#include <vector>

namespace {

void canonical_literal(lowir_model::Operand & operand)
{
  if(operand.has_int_value || operand.has_float_bits)
    operand.has_spelling = false;
}

void canonicalize_output(lowir_model::Program & program)
{
  for(std::size_t i = 0; i < program.globals.size(); ++i) {
    lowir_model::GlobalDefinition & global = program.globals[i];
    canonical_literal(global.init_operand);
    for(std::size_t j = 0; j < global.data_items.size(); ++j)
      canonical_literal(global.data_items[j].literal_operand);
  }
  for(std::size_t f = 0; f < program.functions.size(); ++f) {
    lowir_model::Function & function = program.functions[f];
    function.metadata.inferred_legacy_role = false;
    for(std::size_t b = 0; b < function.blocks.size(); ++b)
      for(std::size_t i = 0; i < function.blocks[b].instructions.size(); ++i) {
        lowir_model::Instruction & ins = function.blocks[b].instructions[i];
        for(std::size_t j = 0; j < lowir_model::operand_view::all_operand_count(ins); ++j)
          canonical_literal(lowir_model::operand_view::all_operand_at(ins, j));
      }
  }
}

int run_lowir(int argc, char ** argv)
{
  std::string output;
  std::string exercise;
  std::vector<std::string> inputs;
  for(int i = 1; i < argc; ++i) {
    const std::string arg(argv[i]);
    if(arg == "--help" || arg == "-h") {
      std::cout << lowir_help_text();
      return EXIT_SUCCESS;
    }
    if(arg == "-o" || arg == "--exercise") {
      if(i + 1 == argc)
        cppgm::driver_errors::ThrowInvocation("missing value after " + arg);
      std::string & value = arg == "-o" ? output : exercise;
      if(!value.empty())
        cppgm::driver_errors::ThrowInvocation("repeated option: " + arg);
      value = argv[++i];
    } else if(!arg.empty() && arg[0] == '-') {
      cppgm::driver_errors::ThrowInvocation("unknown option: " + arg);
    } else {
      inputs.push_back(arg);
    }
  }
  if(output.empty() || (inputs.empty() == exercise.empty()))
    cppgm::driver_errors::ThrowInvocation("expected input files or one exercise");
  if(!exercise.empty() && exercise != "sum" && exercise != "swap" &&
     exercise != "call")
    cppgm::driver_errors::ThrowInvocation("unknown LowIR exercise: " + exercise);

  lowir_model::Program program = exercise.empty() ?
    lowir_model::parse_lowir_program_files(inputs, lowir_model::LEP_ALLOW_HELPERS_ONLY) :
    lowir_intro::BuildExercise(exercise);
  canonicalize_output(program);
  lowir_model::write_lowir_program_file(output, program);
  return EXIT_SUCCESS;
}

}  // namespace

int main(int argc, char ** argv)
{
  try {
    return run_lowir(argc, argv);
  } catch(const CompilerError & error) {
    std::cerr << "ERROR: " << error.what() << std::endl;
    return EXIT_FAILURE;
  } catch(const std::exception & error) {
    std::cerr << "ERROR: " << error.what() << std::endl;
    return EXIT_FAILURE;
  }
}
