#pragma once

#include <string>
#include <vector>

#include "cppast_ast.h"
#include "cpp_decl_model.h"
#include "semantic_model.h"
#include "template_model.h"

class SemanticContext;

namespace semantic_template_variable {

const semantic_model::ValueBinding * acquire_variable_template_binding(
    SemanticContext & ctx,
    semantic_model::VariableTemplateDecl & decl,
    const std::vector<template_model::TemplateArgument> & arguments,
    semantic_model::Scope & source_use_scope,
    const std::string & source_use_location = std::string());

const semantic_model::ValueBinding * acquire_variable_template_binding_for_source_use(
    SemanticContext & ctx,
    semantic_model::VariableTemplateDecl & decl,
    const std::vector<template_model::TemplateArgument> & arguments,
    semantic_model::Scope & source_use_scope,
    const CppAstNode & source_node,
    const std::string & template_name);

const semantic_model::ValueBinding *
acquire_variable_template_binding_for_template_id_source_use(
    SemanticContext & ctx,
    semantic_model::VariableTemplateDecl & decl,
    semantic_model::Scope & source_use_scope,
    const CppAstNode & source_node,
    const cpp_decl::TemplateIdSyntax & template_id);

const semantic_model::ValueBinding *
acquire_member_variable_template_binding_for_template_id_source_use(
    SemanticContext & ctx,
    semantic_model::VariableTemplateDecl & decl,
    semantic_model::ClassInfo & owner,
    semantic_model::Scope & source_use_scope,
    const CppAstNode & source_node,
    const cpp_decl::TemplateIdSyntax & template_id);

}  // namespace semantic_template_variable
