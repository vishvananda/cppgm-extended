#pragma once

#include "template_api.h"

namespace template_selection {
struct ClassSpecializationSelection;
struct VariableSpecializationSelection;
}

namespace template_api {

ClassSpecializationSelection to_api_class_specialization_selection(
    const template_selection::ClassSpecializationSelection & selection);

VariableSpecializationSelection to_api_variable_specialization_selection(
    const template_selection::VariableSpecializationSelection & selection);

}  // namespace template_api
