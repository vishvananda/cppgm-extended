#pragma once

#include <string>
#include <vector>

namespace machine_object { struct ObjectFile; }

void link_machine_objects_to_native(const std::vector<std::string> & objfiles,
                                    const std::string & outfile,
                                    const std::string & mapfile);

void link_machine_objects_to_native(const std::vector<machine_object::ObjectFile> & objects,
                                    const std::string & outfile,
                                    const std::string & mapfile);

void link_exception_objects_to_native(const std::vector<std::string> & objfiles,
                                      const std::string & outfile,
                                      const std::string & mapfile);

void link_exception_objects_to_native(const std::vector<machine_object::ObjectFile> & objects,
                                      const std::string & outfile,
                                      const std::string & mapfile);
