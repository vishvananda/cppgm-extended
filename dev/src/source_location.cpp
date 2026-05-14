#include "source_location.h"

#include <stdexcept>

using namespace std;

SourceLocationTable::SourceLocationTable()
{
  files.push_back(string());
  locations.push_back(SourceLocation{0, 0, 0});
}

uint16_t SourceLocationTable::add_file(const string & path)
{
  map<string, uint16_t>::const_iterator found = file_indices_.find(path);
  if(found != file_indices_.end()) {
    return found->second;
  }

  if(files.size() >= 0x10000) {
    throw logic_error("too many source files for location table");
  }

  const uint16_t index = static_cast<uint16_t>(files.size());
  files.push_back(path);
  file_indices_[path] = index;
  return index;
}

uint32_t SourceLocationTable::add(uint16_t file_index, uint32_t line, uint32_t column)
{
  if(file_index == 0 || line == 0 || column == 0) {
    return 0;
  }

  if(!locations.empty()) {
    const SourceLocation & previous = locations.back();
    if(previous.file_index == file_index &&
       previous.line == line &&
       previous.column == column) {
      return static_cast<uint32_t>(locations.size() - 1);
    }
  }

  if(locations.size() >= 0x100000000ULL) {
    throw logic_error("too many source locations for location table");
  }

  const uint32_t index = static_cast<uint32_t>(locations.size());
  locations.push_back(SourceLocation{file_index, line, column});
  return index;
}

string SourceLocationTable::describe(uint32_t location_id) const
{
  if(location_id == 0 || location_id >= locations.size()) {
    return "<unknown>";
  }

  const SourceLocation & location = locations[location_id];
  if(location.file_index == 0 || location.file_index >= files.size()) {
    return "<unknown>";
  }

  const string line = to_string(location.line);
  const string column = to_string(location.column);
  const string & file = files[location.file_index];
  string out;
  out.reserve(file.size() + line.size() + column.size() + 2);
  out += file;
  out += ':';
  out += line;
  out += ':';
  out += column;
  return out;
}
