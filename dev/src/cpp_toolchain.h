#pragma once

#include <string>
#include <vector>

#include "callsemantic.h"
#include "cpp_preprocess_options.h"
#include "lowir_internal.h"
#include "machine_object.h"
#include "template_witness.h"

enum HostToolchainFamily
{
  HOST_TOOLCHAIN_UNKNOWN,
  HOST_TOOLCHAIN_CLANG,
  HOST_TOOLCHAIN_GNU
};

std::vector<CallSemNode> analyze_cpp_sources(
    const std::vector<std::string> & srcfiles,
    const CppPreprocessOptions & options = CppPreprocessOptions(),
    bool expand_output_closure = false,
    std::vector<std::string> * dependency_files = nullptr,
    std::vector<template_api::TemplateWitnessSession> * witness_sessions = nullptr,
    bool exact_source_locations = true);
std::string generate_lowir_from_translation_units(
    const std::vector<CallSemNode> & translation_units,
    int optimization_level = 0,
    int debug_info_level = 0);
std::string generate_lowir_from_cpp_sources(
    const std::vector<std::string> & srcfiles,
    const CppPreprocessOptions & options = CppPreprocessOptions(),
    int optimization_level = 0,
    int debug_info_level = 0);
lowir_internal::Program build_lowir_program_from_cpp_sources(
    const std::vector<std::string> & srcfiles,
    const CppPreprocessOptions & options = CppPreprocessOptions(),
    int debug_info_level = 0);
lowir_internal::Program prepare_object_lowir_program(
    lowir_internal::Program program,
    int optimization_level = 0,
    int debug_info_level = 0);
machine_object::ObjectFile build_cpp_object_file(const std::vector<std::string> & srcfiles,
                                                 const CppPreprocessOptions & options,
                                                 const std::string & output_target,
                                                 int optimization_level = 0,
                                                 int debug_info_level = 0);
machine_object::ObjectFile build_cpp_lowir_object_file(
    const std::vector<std::string> & srcfiles,
    const std::string & output_target,
    int optimization_level = 0,
    int debug_info_level = 0);
void set_cpp_tool_program_path(const std::string & path);
std::string cpp_tool_program_path();
HostToolchainFamily host_toolchain_family();
bool can_use_host_toolchain_for_output_target(const std::string & output_target);
void run_host_cpp_query(const std::vector<std::string> & args);
void write_cpp_object_file(const std::vector<std::string> & srcfiles,
                           const CppPreprocessOptions & options,
                           const std::string & outfile,
                           const std::string & output_target,
                           int optimization_level = 0,
                           int debug_info_level = 0,
                           std::vector<std::string> * dependency_files = nullptr);
void write_cpp_lowir_object_file(const std::vector<std::string> & srcfiles,
                                 const std::string & outfile,
                                 const std::string & output_target,
                                 int optimization_level = 0,
                                 int debug_info_level = 0,
                                 std::vector<std::string> * dependency_files = nullptr);
bool link_host_objects_to_native(const std::vector<std::string> & objfiles,
                                 const std::string & outfile,
                                 const std::string & output_target,
                                 int debug_info_level = 0,
                                 const std::string & stdlib_flag = std::string());
int run_cpp_to_lowir_frontend(int argc, char ** argv);
