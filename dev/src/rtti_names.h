#ifndef INCLUDED_RTTI_NAMES_H
#define INCLUDED_RTTI_NAMES_H

#include <string>

#include "cpp_decl_model.h"

std::string rtti_symbol_for_display_name(const std::string & name);
std::string rtti_symbol_for_type(const cpp_decl::TypePtr & type);

#endif
