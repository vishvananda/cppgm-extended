#include "machine_linker.h"

#include <algorithm>
#include <cstdlib>
#include <cstdint>
#include <fstream>
#include <map>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

using namespace std;

#include "cy86_internal.h"
#include "eh_runtime.h"
#include "lowir_object_backend.h"
#include "machine_object.h"
#include "machine_ir.h"
#include "native_format.h"
#include "x86_assembler.h"

namespace {

using machine_object::ObjectFile;
using machine_object::Relocation;
using machine_object::Symbol;
namespace mir = machine_ir;

struct LinkedSymbol
{
  string name;
  Symbol::Binding binding = Symbol::SB_GLOBAL;
  Symbol::Section section = Symbol::SS_CODE;
  size_t offset = 0;
  uint64_t vaddr = 0;
};

struct ObjectPlacement
{
  size_t code_offset = 0;
  size_t data_offset = 0;
  vector<size_t> extra_offsets;
};

struct StartupFixup
{
  Relocation::Kind kind = Relocation::RK_BRANCH32;
  size_t patch_offset = 0;
  size_t object_index = 0;
  string symbol;
};

struct StartupSymbolRef
{
  size_t object_index = 0;
  string symbol;
};

struct LinkLayout
{
  string target;
  size_t startup_size = 0;
  size_t code_size = 0;
  size_t data_size = 0;
  uint64_t image_base = 0;
  vector<ObjectPlacement> placements;
  map<string, LinkedSymbol> globals;
  map<string, size_t> got_slots;
  StartupSymbolRef main_symbol;
  vector<StartupSymbolRef> init_symbols;
  vector<StartupSymbolRef> fini_symbols;
};

int binding_precedence(Symbol::Binding binding)
{
  switch(binding) {
    case Symbol::SB_GLOBAL:
      return 2;
    case Symbol::SB_WEAK:
      return 1;
    case Symbol::SB_LOCAL:
      return 0;
  }
  return 0;
}

size_t extra_section_alignment(const machine_object::ExtraSection & section)
{
  if(section.macho_align_pow2 >= sizeof(size_t) * 8) {
    throw logic_error("extra section alignment too large");
  }
  return size_t(1) << section.macho_align_pow2;
}

size_t find_extra_section_index(const ObjectFile & object,
                               const string & extra_section_key)
{
  for(size_t i = 0; i < object.extra_sections.size(); ++i) {
    const string key = object.extra_sections[i].segment_name + "," +
        object.extra_sections[i].section_name;
    if(key == extra_section_key) {
      return i;
    }
  }
  throw logic_error("missing extra section " + extra_section_key);
}

size_t section_base_offset(const vector<ObjectFile> & objects,
                           const LinkLayout & layout,
                           size_t object_index,
                           Symbol::Section section,
                           const string & extra_section_key)
{
  if(section == Symbol::SS_CODE) {
    return layout.placements[object_index].code_offset;
  }
  if(section == Symbol::SS_DATA) {
    return layout.placements[object_index].data_offset;
  }
  if(section == Symbol::SS_EXTRA) {
    const size_t extra_index =
        find_extra_section_index(objects[object_index], extra_section_key);
    return layout.placements[object_index].extra_offsets[extra_index];
  }
  throw logic_error("unplaced section kind");
}

uint64_t section_base_vaddr(const vector<ObjectFile> & objects,
                            const LinkLayout & layout,
                            size_t object_index,
                            Symbol::Section section,
                            const string & extra_section_key)
{
  return layout.image_base +
      section_base_offset(objects, layout, object_index, section, extra_section_key);
}

ObjectFile build_exception_runtime_object(const string & target)
{
  mir::Program runtime;
  runtime.target = target;

  mir::GlobalDefinition top;
  top.name = eh_runtime::kEhTopSymbol;
  top.storage_kind = mir::GlobalDefinition::GS_SCALAR;
  top.type = "ptr";
  top.init_kind = mir::GlobalDefinition::GI_ZERO;
  runtime.globals.push_back(top);

  mir::GlobalDefinition value;
  value.name = eh_runtime::kEhValueSymbol;
  value.storage_kind = mir::GlobalDefinition::GS_SCALAR;
  value.type = "ptr";
  value.init_kind = mir::GlobalDefinition::GI_ZERO;
  runtime.globals.push_back(value);

  mir::GlobalDefinition type;
  type.name = eh_runtime::kEhTypeSymbol;
  type.storage_kind = mir::GlobalDefinition::GS_SCALAR;
  type.type = "ptr";
  type.init_kind = mir::GlobalDefinition::GI_ZERO;
  runtime.globals.push_back(type);

  mir::Function unhandled;
  unhandled.name = eh_runtime::kEhUnhandledSymbol;
  unhandled.return_type = "void";
  mir::ParamBinding param;
  param.name = "%code";
  param.reg = XR_RDI;
  param.type = "i64";
  unhandled.params.push_back(param);
  mir::Block entry;
  entry.label = "^entry";
  mir::Instruction exit_inst;
  exit_inst.opcode = mir::Instruction::MI_EXIT;
  entry.instructions.push_back(exit_inst);
  unhandled.blocks.push_back(entry);
  runtime.functions.push_back(unhandled);

  return build_machine_object(runtime);
}

uint64_t native_payload_offset()
{
  return 0x1000ULL;
}

uint64_t native_image_base(const native_format::Hooks & format_hooks)
{
  return format_hooks.base_vaddr + native_payload_offset();
}

size_t align_up(size_t value, size_t alignment)
{
  if(alignment == 0) {
    return value;
  }
  const size_t mask = alignment - 1;
  return (value + mask) & ~mask;
}

string link_map_symbol_name(const string & name)
{
  if(name == eh_runtime::kEhTopObjectSymbol) return eh_runtime::kEhTopSymbol;
  if(name == eh_runtime::kEhValueObjectSymbol) return eh_runtime::kEhValueSymbol;
  if(name == eh_runtime::kEhTypeObjectSymbol) return eh_runtime::kEhTypeSymbol;
  if(name == eh_runtime::kEhUnhandledObjectSymbol) return eh_runtime::kEhUnhandledSymbol;
  return name;
}

const char * section_display_name(Symbol::Section section)
{
  switch(section) {
    case Symbol::SS_CODE:
      return "code";
    case Symbol::SS_DATA:
      return "data";
    case Symbol::SS_EXTRA:
      return "extra";
    case Symbol::SS_UNDEFINED:
      return "undef";
  }
  return "unknown";
}

void patch_rel32(vector<unsigned char> & bytes,
                 size_t patch_offset,
                 uint64_t base_vaddr,
                 uint64_t target_vaddr)
{
  const int64_t disp = static_cast<int64_t>(target_vaddr) -
      static_cast<int64_t>(base_vaddr + patch_offset + 4);
  if(disp < INT32_MIN || disp > INT32_MAX) {
    throw logic_error("rel32 target out of range");
  }
  const uint32_t patch = static_cast<uint32_t>(static_cast<int32_t>(disp));
  bytes[patch_offset + 0] = static_cast<unsigned char>(patch & 0xFF);
  bytes[patch_offset + 1] = static_cast<unsigned char>((patch >> 8) & 0xFF);
  bytes[patch_offset + 2] = static_cast<unsigned char>((patch >> 16) & 0xFF);
  bytes[patch_offset + 3] = static_cast<unsigned char>((patch >> 24) & 0xFF);
}

void patch_abs64(vector<unsigned char> & bytes,
                 size_t patch_offset,
                 uint64_t value)
{
  for(size_t i = 0; i < 8; ++i) {
    bytes[patch_offset + i] = static_cast<unsigned char>((value >> (i * 8)) & 0xFF);
  }
}

size_t emit_startup_stub(const native_format::Hooks & format_hooks,
                         const StartupSymbolRef & main_symbol,
                         const vector<StartupSymbolRef> & init_symbols,
                         const vector<StartupSymbolRef> & fini_symbols,
                         vector<unsigned char> & startup,
                         vector<StartupFixup> & fixups)
{
  X86Assembler out;
  for(size_t i = 0; i < init_symbols.size(); ++i) {
    StartupFixup fixup;
    fixup.kind = Relocation::RK_BRANCH32;
    fixup.patch_offset = out.emit_call_rel32_placeholder();
    fixup.object_index = init_symbols[i].object_index;
    fixup.symbol = init_symbols[i].symbol;
    fixups.push_back(fixup);
  }
  {
    StartupFixup fixup;
    fixup.kind = Relocation::RK_BRANCH32;
    fixup.patch_offset = out.emit_call_rel32_placeholder();
    fixup.object_index = main_symbol.object_index;
    fixup.symbol = main_symbol.symbol;
    fixups.push_back(fixup);
  }
  if(!fini_symbols.empty()) {
    out.emit_mov_r64_r64(XR_R12, XR_RAX);
    for(size_t i = fini_symbols.size(); i-- > 0;) {
      StartupFixup fixup;
      fixup.kind = Relocation::RK_BRANCH32;
      fixup.patch_offset = out.emit_call_rel32_placeholder();
      fixup.object_index = fini_symbols[i].object_index;
      fixup.symbol = fini_symbols[i].symbol;
      fixups.push_back(fixup);
    }
    out.emit_mov_r64_r64(XR_RDI, XR_R12);
  } else {
    out.emit_mov_r64_r64(XR_RDI, XR_RAX);
  }
  out.emit_mov_r64_imm64(XR_RAX, format_hooks.exit_syscall_number);
  out.emit_syscall();
  out.emit_ud2();
  startup = out.bytes();
  return startup.size();
}

LinkLayout layout_objects(const vector<ObjectFile> & objects)
{
  if(objects.empty()) {
    throw logic_error("no object files");
  }
  LinkLayout layout;
  layout.target = objects[0].target;
  const native_format::Hooks & format_hooks =
      native_format::hooks_for_target_text(layout.target);
  layout.image_base = native_image_base(format_hooks);
  layout.placements.resize(objects.size());

  bool has_main = false;
  for(size_t i = 0; i < objects.size(); ++i) {
    if(objects[i].target != layout.target) {
      throw logic_error("mismatched object targets");
    }
    for(size_t si = 0; si < objects[i].symbols.size(); ++si) {
      const Symbol & symbol = objects[i].symbols[si];
      if(symbol.section == Symbol::SS_UNDEFINED) {
        if(symbol.name == "@main" || symbol.name == "main") {
          has_main = true;
          layout.main_symbol.object_index = i;
          layout.main_symbol.symbol = symbol.name;
        }
        continue;
      }
      if(symbol.binding != Symbol::SB_LOCAL) {
        map<string, LinkedSymbol>::iterator found = layout.globals.find(symbol.name);
        if(found != layout.globals.end()) {
          const int existing = binding_precedence(found->second.binding);
          const int incoming = binding_precedence(symbol.binding);
          if(existing == binding_precedence(Symbol::SB_GLOBAL) &&
             incoming == binding_precedence(Symbol::SB_GLOBAL)) {
            throw logic_error("duplicate global symbol " + symbol.name);
          }
          if(existing >= incoming) {
            goto symbol_done;
          }
        }
        LinkedSymbol linked;
        linked.name = symbol.name;
        linked.binding = symbol.binding;
        linked.section = symbol.section;
        layout.globals[symbol.name] = linked;
      }
    symbol_done:
      if(symbol.name == "@__cppgm_init") {
        StartupSymbolRef init_ref;
        init_ref.object_index = i;
        init_ref.symbol = symbol.name;
        layout.init_symbols.push_back(init_ref);
      }
      if(symbol.name == "@__cppgm_fini") {
        StartupSymbolRef fini_ref;
        fini_ref.object_index = i;
        fini_ref.symbol = symbol.name;
        layout.fini_symbols.push_back(fini_ref);
      }
      if(symbol.name == "@main" || symbol.name == "main") {
        has_main = true;
        layout.main_symbol.object_index = i;
        layout.main_symbol.symbol = symbol.name;
      }
    }
  }
  if(!has_main) {
    throw logic_error("missing main");
  }

  vector<unsigned char> startup;
  vector<StartupFixup> startup_fixups;
  layout.startup_size = emit_startup_stub(format_hooks,
                                          layout.main_symbol,
                                          layout.init_symbols,
                                          layout.fini_symbols,
                                          startup, startup_fixups);

  size_t code_offset = layout.startup_size;
  for(size_t i = 0; i < objects.size(); ++i) {
    code_offset = align_up(code_offset, 16);
    layout.placements[i].code_offset = code_offset;
    code_offset += objects[i].code.size();
  }
  layout.code_size = code_offset;

  size_t data_offset = align_up(layout.code_size, 16);
  for(size_t i = 0; i < objects.size(); ++i) {
    data_offset = align_up(data_offset, 16);
    layout.placements[i].data_offset = data_offset;
    data_offset += objects[i].data.size();
  }
  size_t got_offset = align_up(data_offset, 8);
  for(size_t oi = 0; oi < objects.size(); ++oi) {
    for(size_t ri = 0; ri < objects[oi].relocations.size(); ++ri) {
      const Relocation & reloc = objects[oi].relocations[ri];
      if(reloc.kind != Relocation::RK_INDIRECT_REL32) {
        continue;
      }
      if(layout.got_slots.count(reloc.symbol) != 0) {
        continue;
      }
      layout.got_slots[reloc.symbol] = got_offset;
      got_offset += 8;
    }
  }
  size_t extra_offset = align_up(got_offset, 16);
  for(size_t i = 0; i < objects.size(); ++i) {
    layout.placements[i].extra_offsets.resize(objects[i].extra_sections.size(), 0);
    for(size_t si = 0; si < objects[i].extra_sections.size(); ++si) {
      extra_offset = align_up(extra_offset,
                              extra_section_alignment(objects[i].extra_sections[si]));
      layout.placements[i].extra_offsets[si] = extra_offset;
      extra_offset += objects[i].extra_sections[si].bytes.size();
    }
  }
  layout.data_size = extra_offset;

  for(size_t i = 0; i < objects.size(); ++i) {
    for(size_t si = 0; si < objects[i].symbols.size(); ++si) {
      const Symbol & symbol = objects[i].symbols[si];
      if(symbol.binding == Symbol::SB_LOCAL ||
         symbol.section == Symbol::SS_UNDEFINED) {
        continue;
      }
      map<string, LinkedSymbol>::iterator found = layout.globals.find(symbol.name);
      if(found == layout.globals.end() || found->second.binding != symbol.binding) {
        continue;
      }
      LinkedSymbol & linked = found->second;
      linked.offset = section_base_offset(objects,
                                          layout,
                                          i,
                                          symbol.section,
                                          symbol.extra_section) + symbol.offset;
      linked.vaddr = layout.image_base + linked.offset;
    }
  }
  return layout;
}

uint64_t resolve_symbol(const vector<ObjectFile> & objects,
                        const LinkLayout & layout,
                        size_t object_index,
                        const string & name)
{
  map<string, LinkedSymbol>::const_iterator global = layout.globals.find(name);
  if(global != layout.globals.end()) {
    return global->second.vaddr;
  }
  const ObjectFile & object = objects[object_index];
  for(size_t i = 0; i < object.symbols.size(); ++i) {
    const Symbol & symbol = object.symbols[i];
    if(symbol.binding == Symbol::SB_LOCAL && symbol.name == name) {
      return section_base_vaddr(objects,
                                layout,
                                object_index,
                                symbol.section,
                                symbol.extra_section) + symbol.offset;
    }
  }
  throw logic_error("unresolved symbol " + name);
}

uint64_t resolve_extra_relocation_target(const vector<ObjectFile> & objects,
                                         const LinkLayout & layout,
                                         size_t object_index,
                                         const machine_object::ExtraRelocation & reloc)
{
  if(reloc.target_kind == machine_object::ExtraRelocation::TK_SYMBOL) {
    return resolve_symbol(objects, layout, object_index, reloc.symbol) + reloc.addend;
  }
  if(reloc.target_kind == machine_object::ExtraRelocation::TK_CODE) {
    return section_base_vaddr(objects, layout, object_index, Symbol::SS_CODE, string()) +
        reloc.addend;
  }
  if(reloc.target_kind == machine_object::ExtraRelocation::TK_DATA) {
    return section_base_vaddr(objects, layout, object_index, Symbol::SS_DATA, string()) +
        reloc.addend;
  }
  if(reloc.target_kind == machine_object::ExtraRelocation::TK_EXTRA) {
    return section_base_vaddr(objects,
                              layout,
                              object_index,
                              Symbol::SS_EXTRA,
                              reloc.target_extra_section) + reloc.addend;
  }
  throw logic_error("unsupported extra relocation target kind");
}

string build_link_map(const vector<ObjectFile> & objects,
                      const LinkLayout & layout)
{
  struct LinkMapSymbolLine
  {
    string display_name;
    const LinkedSymbol * symbol;
  };

  ostringstream out;
  out << "link_map x86_64 " << layout.target << "\n";
  out << "startup_size " << layout.startup_size << "\n";
  out << "code_size " << layout.code_size << "\n";
  out << "data_size " << layout.data_size << "\n";
  for(size_t i = 0; i < objects.size(); ++i) {
    out << "object " << i
        << " code_offset " << layout.placements[i].code_offset
        << " data_offset " << layout.placements[i].data_offset << "\n";
  }
  vector<LinkMapSymbolLine> symbols;
  for(map<string, LinkedSymbol>::const_iterator it = layout.globals.begin();
      it != layout.globals.end(); ++it) {
    LinkMapSymbolLine line;
    line.display_name = link_map_symbol_name(it->first);
    line.symbol = &it->second;
    symbols.push_back(line);
  }
  sort(symbols.begin(), symbols.end(),
       [](const LinkMapSymbolLine & a, const LinkMapSymbolLine & b)
       {
         if(a.display_name != b.display_name) {
           return a.display_name < b.display_name;
         }
         if(a.symbol->section != b.symbol->section) {
           return a.symbol->section < b.symbol->section;
         }
         return a.symbol->offset < b.symbol->offset;
       });
  for(size_t i = 0; i < symbols.size(); ++i) {
    out << "symbol " << symbols[i].display_name << ' '
        << section_display_name(symbols[i].symbol->section) << ' '
        << symbols[i].symbol->offset << "\n";
  }
  return out.str();
}

string build_link_map_slice(const vector<ObjectFile> & objects,
                            const LinkLayout & layout,
                            size_t object_start)
{
  struct LinkMapSymbolLine
  {
    string display_name;
    const LinkedSymbol * symbol;
  };

  ostringstream out;
  out << "link_map x86_64 " << layout.target << "\n";
  out << "startup_size " << layout.startup_size << "\n";
  out << "code_size " << layout.code_size << "\n";
  out << "data_size " << layout.data_size << "\n";
  for(size_t i = object_start; i < objects.size(); ++i) {
    out << "object " << (i - object_start)
        << " code_offset " << layout.placements[i].code_offset
        << " data_offset " << layout.placements[i].data_offset << "\n";
  }
  vector<LinkMapSymbolLine> symbols;
  for(map<string, LinkedSymbol>::const_iterator it = layout.globals.begin();
      it != layout.globals.end(); ++it) {
    LinkMapSymbolLine line;
    line.display_name = link_map_symbol_name(it->first);
    line.symbol = &it->second;
    symbols.push_back(line);
  }
  sort(symbols.begin(), symbols.end(),
       [](const LinkMapSymbolLine & a, const LinkMapSymbolLine & b)
       {
         if(a.display_name != b.display_name) {
           return a.display_name < b.display_name;
         }
         if(a.symbol->section != b.symbol->section) {
           return a.symbol->section < b.symbol->section;
         }
         return a.symbol->offset < b.symbol->offset;
       });
  for(size_t i = 0; i < symbols.size(); ++i) {
    out << "symbol " << symbols[i].display_name << ' '
        << section_display_name(symbols[i].symbol->section) << ' '
        << symbols[i].symbol->offset << "\n";
  }
  return out.str();
}

vector<unsigned char> link_payload(const vector<ObjectFile> & objects,
                                   const LinkLayout & layout)
{
  const native_format::Hooks & format_hooks =
      native_format::hooks_for_target_text(layout.target);
  vector<unsigned char> startup;
  vector<StartupFixup> startup_fixups;
  emit_startup_stub(format_hooks,
                    layout.main_symbol,
                    layout.init_symbols,
                    layout.fini_symbols,
                    startup,
                    startup_fixups);

  vector<unsigned char> payload(layout.data_size, 0);
  copy(startup.begin(), startup.end(), payload.begin());

  for(size_t i = 0; i < objects.size(); ++i) {
    copy(objects[i].code.begin(), objects[i].code.end(),
         payload.begin() + layout.placements[i].code_offset);
    copy(objects[i].data.begin(), objects[i].data.end(),
         payload.begin() + layout.placements[i].data_offset);
    for(size_t si = 0; si < objects[i].extra_sections.size(); ++si) {
      copy(objects[i].extra_sections[si].bytes.begin(),
           objects[i].extra_sections[si].bytes.end(),
           payload.begin() + layout.placements[i].extra_offsets[si]);
    }
  }

  for(size_t i = 0; i < startup_fixups.size(); ++i) {
    patch_rel32(payload,
                startup_fixups[i].patch_offset,
                layout.image_base,
                resolve_symbol(objects,
                               layout,
                               startup_fixups[i].object_index,
                               startup_fixups[i].symbol));
  }

  for(size_t oi = 0; oi < objects.size(); ++oi) {
    for(size_t ri = 0; ri < objects[oi].relocations.size(); ++ri) {
      const Relocation & reloc = objects[oi].relocations[ri];
      if(reloc.section == Symbol::SS_EXTRA) {
        throw logic_error("main relocation stream cannot target unnamed extra section");
      }
      const size_t base_offset = reloc.section == Symbol::SS_CODE
          ? layout.placements[oi].code_offset
          : layout.placements[oi].data_offset;
      const size_t patch_offset = base_offset + reloc.offset;
      const uint64_t target = resolve_symbol(objects, layout, oi, reloc.symbol);
      if(reloc.kind == Relocation::RK_BRANCH32 ||
         reloc.kind == Relocation::RK_PCREL32) {
        patch_rel32(payload, patch_offset, layout.image_base, target);
      } else if(reloc.kind == Relocation::RK_TLV_REL32) {
        throw logic_error(
            "direct native linker does not support Mach-O TLV relocations; "
            "rebuild objects with the direct-native thread_local ABI");
      } else if(reloc.kind == Relocation::RK_INDIRECT_REL32) {
        map<string, size_t>::const_iterator got = layout.got_slots.find(reloc.symbol);
        if(got == layout.got_slots.end()) {
          throw logic_error("missing GOT slot for " + reloc.symbol);
        }
        patch_rel32(payload,
                    patch_offset,
                    layout.image_base,
                    layout.image_base + got->second);
      } else {
        patch_abs64(payload, patch_offset, target);
      }
    }
    for(size_t si = 0; si < objects[oi].extra_sections.size(); ++si) {
      const machine_object::ExtraSection & section = objects[oi].extra_sections[si];
      const size_t base_offset = layout.placements[oi].extra_offsets[si];
      for(size_t ri = 0; ri < section.relocations.size(); ++ri) {
        const machine_object::ExtraRelocation & reloc = section.relocations[ri];
        const size_t patch_offset = base_offset + reloc.offset;
        if(reloc.kind == machine_object::ExtraRelocation::RK_INDIRECT_REL32) {
          if(reloc.target_kind != machine_object::ExtraRelocation::TK_SYMBOL) {
            throw logic_error("indirect extra relocation requires symbol target");
          }
          map<string, size_t>::const_iterator got = layout.got_slots.find(reloc.symbol);
          if(got == layout.got_slots.end()) {
            throw logic_error("missing GOT slot for " + reloc.symbol);
          }
          patch_rel32(payload,
                      patch_offset,
                      layout.image_base,
                      layout.image_base + got->second);
          continue;
        }
        const uint64_t target =
            resolve_extra_relocation_target(objects, layout, oi, reloc);
        if(reloc.kind == machine_object::ExtraRelocation::RK_BRANCH32 ||
           reloc.kind == machine_object::ExtraRelocation::RK_PCREL32) {
          patch_rel32(payload, patch_offset, layout.image_base, target);
        } else {
          patch_abs64(payload, patch_offset, target);
        }
      }
    }
  }

  for(map<string, size_t>::const_iterator it = layout.got_slots.begin();
      it != layout.got_slots.end(); ++it) {
    patch_abs64(payload,
                it->second,
                resolve_symbol(objects, layout, 0, it->first));
  }

  return payload;
}

string absolute_path(const string & path)
{
  if(!path.empty() && path[0] == '/') {
    return path;
  }
  char * resolved = realpath(path.c_str(), NULL);
  if(resolved) {
    string result(resolved);
    free(resolved);
    return result;
  }
  return path;
}

}  // namespace

void link_machine_objects_to_native(const vector<string> & objfiles,
                                    const string & outfile,
                                    const string & mapfile)
{
  vector<ObjectFile> objects;
  for(size_t i = 0; i < objfiles.size(); ++i) {
    objects.push_back(machine_object::parse_object_file(objfiles[i]));
  }
  link_machine_objects_to_native(objects, outfile, mapfile);
}

void link_machine_objects_to_native(const vector<ObjectFile> & objects,
                                    const string & outfile,
                                    const string & mapfile)
{
  const LinkLayout layout = layout_objects(objects);
  const vector<unsigned char> payload = link_payload(objects, layout);

  if(!mapfile.empty()) {
    ofstream out(mapfile.c_str());
    if(!out) {
      throw logic_error("unable to open link map file");
    }
    out << build_link_map(objects, layout);
  }

  const native_format::Hooks & format_hooks =
      native_format::hooks_for_target_text(layout.target);
  const string outfile_host = absolute_path(outfile);
  format_hooks.write_x86_64_executable(outfile_host,
                                       payload,
                                       0,
                                       format_hooks.base_vaddr);
}

void link_exception_objects_to_native(const vector<string> & objfiles,
                                      const string & outfile,
                                      const string & mapfile)
{
  vector<ObjectFile> objects;
  for(size_t i = 0; i < objfiles.size(); ++i) {
    objects.push_back(machine_object::parse_object_file(objfiles[i]));
  }
  link_exception_objects_to_native(objects, outfile, mapfile);
}

void link_exception_objects_to_native(const vector<ObjectFile> & objects,
                                      const string & outfile,
                                      const string & mapfile)
{
  if(objects.empty()) {
    throw logic_error("no object files");
  }
  vector<ObjectFile> with_runtime;
  with_runtime.push_back(build_exception_runtime_object(objects[0].target));
  with_runtime.insert(with_runtime.end(), objects.begin(), objects.end());
  const LinkLayout layout = layout_objects(with_runtime);
  const vector<unsigned char> payload = link_payload(with_runtime, layout);

  if(!mapfile.empty()) {
    ofstream out(mapfile.c_str());
    if(!out) {
      throw logic_error("unable to open link map file");
    }
    out << build_link_map_slice(with_runtime, layout, 1);
  }

  const native_format::Hooks & format_hooks =
      native_format::hooks_for_target_text(layout.target);
  const string outfile_host = absolute_path(outfile);
  format_hooks.write_x86_64_executable(outfile_host,
                                       payload,
                                       0,
                                       format_hooks.base_vaddr);
}
