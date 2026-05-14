#include "macho_writer.h"

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
const std::uint32_t MH_MAGIC_64 = 0xFEEDFACF;
const std::uint32_t CPU_TYPE_X86_64 = 0x01000007;
const std::uint32_t CPU_SUBTYPE_X86_64_ALL = 3;
const std::uint32_t MH_EXECUTE = 2;
const std::uint32_t LC_SEGMENT_64 = 0x19;
const std::uint32_t LC_UNIXTHREAD = 0x5;
const std::uint32_t X86_THREAD_STATE64 = 4;
const std::uint32_t X86_THREAD_STATE64_COUNT = 42;
const std::uint32_t VM_PROT_READ = 1;
const std::uint32_t VM_PROT_WRITE = 2;
const std::uint32_t VM_PROT_EXECUTE = 4;

std::uint64_t round_up(std::uint64_t value, std::uint64_t align)
{
  std::uint64_t rem = value % align;
  return rem == 0 ? value : value + (align - rem);
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

void append_fixed_string(vector<unsigned char> & out,
                         const char * value,
                         size_t width)
{
  for(size_t i = 0; i < width; ++i) {
    out.push_back(value[i] == '\0' ? 0
                                   : static_cast<unsigned char>(value[i]));
    if(value[i] == '\0') {
      for(size_t j = i + 1; j < width; ++j) {
        out.push_back(0);
      }
      return;
    }
  }
}

void append_mach_header(vector<unsigned char> & out,
                        std::uint32_t ncmds,
                        std::uint32_t sizeofcmds,
                        std::uint32_t flags)
{
  append_u32(out, MH_MAGIC_64);
  append_u32(out, CPU_TYPE_X86_64);
  append_u32(out, CPU_SUBTYPE_X86_64_ALL);
  append_u32(out, MH_EXECUTE);
  append_u32(out, ncmds);
  append_u32(out, sizeofcmds);
  append_u32(out, flags);
  append_u32(out, 0);
}

void append_segment_command(vector<unsigned char> & out,
                            const char * segname,
                            std::uint64_t vmaddr,
                            std::uint64_t vmsize,
                            std::uint64_t fileoff,
                            std::uint64_t filesize,
                            std::uint32_t maxprot,
                            std::uint32_t initprot,
                            std::uint32_t nsects,
                            std::uint32_t flags)
{
  append_u32(out, LC_SEGMENT_64);
  append_u32(out, 72 + nsects * 80);
  append_fixed_string(out, segname, 16);
  append_u64(out, vmaddr);
  append_u64(out, vmsize);
  append_u64(out, fileoff);
  append_u64(out, filesize);
  append_u32(out, maxprot);
  append_u32(out, initprot);
  append_u32(out, nsects);
  append_u32(out, flags);
}

void append_section_command(vector<unsigned char> & out,
                            const char * sectname,
                            const char * segname,
                            std::uint64_t addr,
                            std::uint64_t size,
                            std::uint32_t offset,
                            std::uint32_t align,
                            std::uint32_t flags)
{
  append_fixed_string(out, sectname, 16);
  append_fixed_string(out, segname, 16);
  append_u64(out, addr);
  append_u64(out, size);
  append_u32(out, offset);
  append_u32(out, align);
  append_u32(out, 0);
  append_u32(out, 0);
  append_u32(out, flags);
  append_u32(out, 0);
  append_u32(out, 0);
  append_u32(out, 0);
}

void append_unixthread_command(vector<unsigned char> & out,
                               std::uint64_t entry_vaddr)
{
  append_u32(out, LC_UNIXTHREAD);
  append_u32(out, 184);
  append_u32(out, X86_THREAD_STATE64);
  append_u32(out, X86_THREAD_STATE64_COUNT);

  const std::uint64_t regs[] =
  {
    0,    // rax
    0,    // rbx
    0,    // rcx
    0,    // rdx
    0,    // rdi
    0,    // rsi
    0,    // rbp
    0,    // rsp (kernel fills the UNIX process stack)
    0,    // r8
    0,    // r9
    0,    // r10
    0,    // r11
    0,    // r12
    0,    // r13
    0,    // r14
    0,    // r15
    entry_vaddr,
    0x202,  // rflags
    0x2B,   // cs
    0,      // fs
    0       // gs
  };

  for(size_t i = 0; i < sizeof(regs) / sizeof(regs[0]); ++i) {
    append_u64(out, regs[i]);
  }
}

}  // namespace

void write_macho_x86_64_executable(const std::string & outfile,
                                   const std::vector<unsigned char> & payload,
                                   std::uint64_t entry_offset,
                                   std::uint64_t base_vaddr)
{
  vector<unsigned char> image;
  const std::uint32_t ncmds = 3;
  const std::uint32_t sizeofcmds = 72 + 152 + 184;
  const std::uint64_t file_size = kPayloadOffset + payload.size();
  const std::uint64_t vm_size = round_up(file_size, kPageSize);

  append_mach_header(image, ncmds, sizeofcmds, 0);
  append_segment_command(image,
                         "__PAGEZERO",
                         0,
                         base_vaddr,
                         0,
                         0,
                         0,
                         0,
                         0,
                         0);
  append_segment_command(image,
                         "__TEXT",
                         base_vaddr,
                         vm_size,
                         0,
                         file_size,
                         VM_PROT_READ | VM_PROT_WRITE | VM_PROT_EXECUTE,
                         VM_PROT_READ | VM_PROT_WRITE | VM_PROT_EXECUTE,
                         1,
                         0);
  append_section_command(image,
                         "__text",
                         "__TEXT",
                         base_vaddr + kPayloadOffset,
                         payload.size(),
                         static_cast<std::uint32_t>(kPayloadOffset),
                         0,
                         0);
  append_unixthread_command(image, base_vaddr + kPayloadOffset + entry_offset);

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
    throw logic_error("unable to write Mach-O executable: " + outfile);
  }

  if(chmod(outfile.c_str(), 0755) != 0) {
    remove(outfile.c_str());
    throw logic_error("chmod failed for Mach-O executable: " + outfile
                      + ": " + strerror(errno));
  }
}
