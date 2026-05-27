#pragma once

#include <string>
#include <vector>

namespace abi_mangle {

enum AbiFactLineKind
{
  ABI_FACT_LEGACY,
  ABI_FACT_LET_TYPE,
  ABI_FACT_LET_ARG,
  ABI_FACT_LET_EXPR,
  ABI_FACT_LET_CONTEXT,
  ABI_FACT_LET_ENTITY,
  ABI_FACT_RESULT_TYPE,
  ABI_FACT_RESULT_FUNCTION,
  ABI_FACT_PARAM
};

struct Invocation
{
  std::string outfile;
  std::vector<std::string> inputs;
};

struct AbiFactLine
{
  AbiFactLineKind kind = ABI_FACT_LEGACY;
  std::string id;
  std::string op;
  std::vector<std::string> operands;
};

struct AbiFactCase
{
  std::string label;
  std::vector<AbiFactLine> lines;
};

struct AbiFactFile
{
  std::vector<AbiFactCase> cases;
};

AbiFactFile parse_fact_text(const std::string & text);
std::string serialize_fact_file(const AbiFactFile & file);
std::string mangle_fact_file(const AbiFactFile & file);
std::string mangle_fact_files(const std::vector<std::string> & input_paths);

}  // namespace abi_mangle
