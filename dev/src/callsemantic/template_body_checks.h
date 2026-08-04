#pragma once

#include <set>
#include <string>
#include <vector>

#include "cppast_ast.h"
#include "semantic_model.h"
#include "template_model.h"

class SemanticContext;

namespace callsemantic {

bool declarator_declared_identifier(const CppAstNode & node,
                                    std::string & out);

std::set<std::string> template_parameter_names(
    const std::vector<template_model::TemplateParameterInfo> & parameters);

bool template_body_has_invalid_nondependent_id_expression(
    SemanticContext & ctx,
    semantic_model::Scope & scope,
    const CppAstNode & node,
    const std::set<std::string> & visible_names,
    const std::set<std::string> & type_parameter_names,
    const CppAstNode *& offending_node,
    std::string & offending_name);

bool class_member_body_has_invalid_nondependent_lookup(
    SemanticContext & ctx,
    semantic_model::Scope & scope,
    const CppAstNode & class_node,
    const std::vector<template_model::TemplateParameterInfo> & parameters,
    const CppAstNode *& offending_node,
    std::string & offending_name);

bool function_template_body_has_invalid_nondependent_lookup(
    SemanticContext & ctx,
    semantic_model::Scope & scope,
    const CppAstNode & declarator,
    const CppAstNode & body,
    const std::vector<template_model::TemplateParameterInfo> & parameters,
    const CppAstNode *& offending_node,
    std::string & offending_name);

bool function_template_signature_has_invalid_nondependent_lookup(
    SemanticContext & ctx,
    semantic_model::Scope & scope,
    const CppAstNode & declarator,
    const CppAstNode & result_type_pattern,
    const std::vector<template_model::TemplateParameterInfo> & parameters,
    const CppAstNode *& offending_node,
    std::string & offending_name);

bool subtree_alias_redeclares_template_parameter(
    const CppAstNode & node,
    const std::set<std::string> & parameter_names,
    const CppAstNode *& offending_node,
    std::string & offending_name);

bool class_member_redeclares_template_parameter(
    const CppAstNode & class_node,
    const std::vector<template_model::TemplateParameterInfo> & parameters,
    const CppAstNode *& offending_node,
    std::string & offending_name);

}  // namespace callsemantic
