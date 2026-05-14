#include "elf_writer.h"

#include <cerrno>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <stdexcept>

#include <sys/stat.h>

using namespace std;

namespace {

const std::uint64_t kPageSize = 0x1000;
const std::uint64_t kPayloadOffset = 0x1000;

void append_u16(vector<unsigned char> & out, std::uint16_t value)
{
  out.push_back(static_cast<unsigned char>(value & 0xFF));
  out.push_back(static_cast<unsigned char>((value >> 8) & 0xFF));
}

void append_u32(vector<unsigned char> & out, std::uint32_t value)
{
  for(size_t i = 0; i < 4; ++i) {
    out.push_back(static_cast<unsigned char>((value >> (8 * i)) & 0xFF));
  }
}

void append_u64(vector<unsigned char> & out, std::uint64_t value)
{
  for(size_t i = 0; i < 8; ++i) {
    out.push_back(static_cast<unsigned char>((value >> (8 * i)) & 0xFF));
  }
}

void append_elf_header(vector<unsigned char> & out,
                       std::uint64_t entry_vaddr,
                       std::uint64_t file_size,
                       std::uint64_t base_vaddr)
{
  out.push_back(0x7F);
  out.push_back('E');
  out.push_back('L');
  out.push_back('F');
  out.push_back(2);
  out.push_back(1);
  out.push_back(1);
  out.push_back(0);
  out.push_back(0);
  out.resize(16, 0);

  append_u16(out, 2);
  append_u16(out, 62);
  append_u32(out, 1);
  append_u64(out, entry_vaddr);
  append_u64(out, 64);
  append_u64(out, 0);
  append_u32(out, 0);
  append_u16(out, 64);
  append_u16(out, 56);
  append_u16(out, 1);
  append_u16(out, 0);
  append_u16(out, 0);
  append_u16(out, 0);

  append_u32(out, 1);
  append_u32(out, 7);
  append_u64(out, 0);
  append_u64(out, base_vaddr);
  append_u64(out, base_vaddr);
  append_u64(out, file_size);
  append_u64(out, file_size);
  append_u64(out, kPageSize);
}

}  // namespace

void write_elf_x86_64_executable(const std::string & outfile,
                                 const std::vector<unsigned char> & payload,
                                 std::uint64_t entry_offset,
                                 std::uint64_t base_vaddr)
{
  vector<unsigned char> image;
  std::uint64_t file_size = kPayloadOffset + payload.size();
  append_elf_header(image,
                    base_vaddr + kPayloadOffset + entry_offset,
                    file_size,
                    base_vaddr);
  image.resize(kPayloadOffset, 0);
  image.insert(image.end(), payload.begin(), payload.end());

  ofstream out(outfile.c_str(), ios::binary | ios::trunc);
  if(!out) {
    throw logic_error("unable to open output file: " + outfile);
  }
  out.write(reinterpret_cast<const char *>(image.data()),
            static_cast<std::streamsize>(image.size()));
  out.close();
  if(!out) {
    remove(outfile.c_str());
    throw logic_error("unable to write ELF executable: " + outfile);
  }

  if(chmod(outfile.c_str(), 0755) != 0) {
    remove(outfile.c_str());
    throw logic_error("chmod failed for ELF executable: " + outfile
                      + ": " + strerror(errno));
  }
}
