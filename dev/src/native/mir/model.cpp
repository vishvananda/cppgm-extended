#include "native/mir/model.h"
#include "native/errors.h"

#include <fstream>
#include <iomanip>
#include <sstream>

namespace mir_model {

Program::Program()
{
}

const std::string & mir_block_label(const MirProgram & program,
                                    const MirFunction & function,
                                    lowir_model::BlockId block)
{
  const std::uint32_t index = block;
  if(!block.valid() || index >= function.block_labels.size())
    native_errors::ThrowInternal("invalid MIR block identity");
  return mir_string(program, function.block_labels[index]);
}

const std::string & mir_symbol_name(const MirProgram & program,
                                    lowir_model::SymbolId symbol)
{
  const std::uint32_t index = symbol;
  if(!symbol.valid() || index >= program.symbol_names.size())
    native_errors::ThrowInternal("invalid MIR symbol identity");
  return mir_string(program, program.symbol_names[index]);
}

const std::string & mir_literal_spelling(const MirProgram & program,
                                         lowir_model::StringId literal)
{
  return mir_string(program, literal);
}

const std::string & mir_string(const MirProgram & program,
                               lowir_model::StringId string)
{
  if(!program.strings.valid() || !string.valid())
    native_errors::ThrowInternal("invalid MIR string identity");
  return program.strings.get(string);
}

std::string mir_presentation_name(
    const MirProgram & program, lowir_model::PresentationName name)
{
  if(!name.valid())
    native_errors::ThrowInternal("invalid MIR presentation identity");
  if(name.generated())
    return "%t" + std::to_string(name.generated_ordinal());
  if(name.fixed())
    return lowir_model::fixed_presentation_name_text(name.fixed_name());
  return "%" + mir_string(program, name.spelling());
}

}  // namespace mir_model
