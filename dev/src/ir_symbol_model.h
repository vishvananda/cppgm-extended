#pragma once

// The LowIR metadata vocabulary, shared by the LowIR and machine-IR
// scaffolds (lowir_model.h, native/mir/registers.h, mir_model.h).  The
// course solution builds on these headers exactly as they are shipped.
//
// Every named value the LowIR text admits in metadata (pa8/lowir.md
// "Program Structure", "Functions", "Memory and Addressing" and "Object ABI
// Conventions") is an enumerator here, with its spelling and a parser.  A
// model that records one of these facts stores the enumerator, never the
// text.  Names (`object=`, `section=`, `tls_for=`) are not vocabulary: keep
// them however your model keeps names.
//
// Conventions of all three headers:
// - spelling(value) returns the text form; the default enumerator of an
//   optional fact (value 0) is spelled "", because the text omits it;
// - from_spelling(text, length, &value) accepts only a non-default spelling and
//   returns false for anything else;
// - k<Name>Count is the number of enumerators;
// - every enum is one byte, so a model can store it compactly; print its
//   text with spelling(), or its number with static_cast<unsigned>(value):
//   `std::cout << value` would write the byte as a character.

#include <cstddef>
#include <cstdint>
#include <cstring>
#include <string>

namespace ir_model {

namespace detail {

inline bool same_spelling(const char * spelling, const char * text, std::size_t length)
{
  return std::strlen(spelling) == length && std::memcmp(spelling, text, length) == 0;
}

// Finds `text` among the spellings of enumerators [first, count) of E (the
// enumerator's own spelling() overload is found by argument-dependent
// lookup, so the vocabularies of all three headers share this).
template <typename E>
bool find_spelling(const char * text, std::size_t length, unsigned first, unsigned count, E * value)
{
  for(unsigned i = first; i < count; ++i) {
    if(same_spelling(spelling(static_cast<E>(i)), text, length)) {
      *value = static_cast<E>(i);
      return true;
    }
  }
  return false;
}

}  // namespace detail

// ---------------------------------------------------------------------------
// Symbol metadata (functions and globals)

enum SymbolRole : std::uint8_t
{
  SR_NONE,
  SR_ENTRY,
  SR_INIT,
  SR_FINI,
  SR_EH_ALLOCATE_EXCEPTION,
  SR_EH_BEGIN_CATCH,
  SR_EH_END_CATCH,
  SR_EH_RETHROW,
  SR_EH_THROW,
  SR_EH_PERSONALITY,
  SR_EH_RESUME,
  SR_ALLOCATE_MEMORY,
  SR_FREE_MEMORY,
  SR_TERMINATE,
  SR_PURE_VIRTUAL,
  SR_DYNAMIC_CAST,
  SR_BAD_CAST,
  SR_BAD_TYPEID,
  SR_RTTI_CLASS,
  SR_RTTI_SI,
  SR_RTTI_VMI,
  SR_RTTI_DATA,
  SR_BAD_ARRAY_NEW_LENGTH,
  SR_RTTI_POINTER
};

const unsigned kSymbolRoleCount = SR_RTTI_POINTER + 1;

enum LanguageLinkageMode : std::uint8_t
{
  LLM_DEFAULT,
  LLM_C
};

const unsigned kLanguageLinkageModeCount = LLM_C + 1;

enum SymbolBindingMode : std::uint8_t
{
  SBM_DEFAULT,
  SBM_INTERNAL,
  SBM_STRONG,
  SBM_WEAK
};

const unsigned kSymbolBindingModeCount = SBM_WEAK + 1;

enum GlobalStorageMode : std::uint8_t
{
  GSM_DEFAULT,
  GSM_READONLY,
  GSM_THREAD_LOCAL
};

const unsigned kGlobalStorageModeCount = GSM_THREAD_LOCAL + 1;

// The yes-valued symbol flags (`keep_alias=yes`, ...), as bits.
// SF_REPLACEABLE (`replaceable=yes`) marks a GNU weak definition and
// requires binding=weak.
enum SymbolFlag : std::uint8_t
{
  SF_KEEP_ALIAS = 1,
  SF_PREFER_LOCAL = 2,
  SF_OBJECT_ROOT = 4,
  SF_FORCE_INLINE = 8,
  SF_INLINE_HINT = 16,
  SF_NO_INLINE = 32,
  SF_REPLACEABLE = 64
};

// ---------------------------------------------------------------------------
// Boundary metadata (functions and call signatures)

enum CallArityMode : std::uint8_t
{
  CAM_FIXED,
  CAM_VARIADIC
};

const unsigned kCallArityModeCount = CAM_VARIADIC + 1;

enum CallEffectsMode : std::uint8_t
{
  CFXM_DEFAULT,
  CFXM_READNONE,
  CFXM_READONLY
};

const unsigned kCallEffectsModeCount = CFXM_READONLY + 1;

enum CallUnwindMode : std::uint8_t
{
  CUM_DEFAULT,
  CUM_NO
};

const unsigned kCallUnwindModeCount = CUM_NO + 1;

enum CallReturnMode : std::uint8_t
{
  CRM_DEFAULT,
  CRM_NORETURN
};

const unsigned kCallReturnModeCount = CRM_NORETURN + 1;

enum CallQueryMode : std::uint8_t
{
  CQM_DEFAULT,
  CQM_STABLE_PREFIX
};

const unsigned kCallQueryModeCount = CQM_STABLE_PREFIX + 1;

// ---------------------------------------------------------------------------
// Parameter and instruction metadata

enum ParamPassingMode : std::uint8_t
{
  PPM_DIRECT,
  PPM_INDIRECT_RESULT,
  PPM_BY_ADDRESS
};

const unsigned kParamPassingModeCount = PPM_BY_ADDRESS + 1;

enum ParamAliasMode : std::uint8_t
{
  PALM_DEFAULT,
  PALM_NOALIAS
};

const unsigned kParamAliasModeCount = PALM_NOALIAS + 1;

// `index ... [projection=...]`.
enum IndexProjectionKind : std::uint8_t
{
  IPK_NONE,
  IPK_ARRAY_ELEMENT,
  IPK_FIELD
};

const unsigned kIndexProjectionKindCount = IPK_FIELD + 1;

// ---------------------------------------------------------------------------
// Object classes (`classes=`, `result_classes=`; pa8/lowir.md "Object ABI
// Conventions")
//
// An `obj<bytes x align>` value carries no field types, so a boundary that
// passes one directly may state its x86-64 psABI classification (3.2.3) in
// one byte: 0 states none (each eightbyte is INTEGER); otherwise
// OC_STATED with the classes of eightbytes 0 and 1 in bits 0-1 and 2-3
// (EightbyteClass), or OC_STATED with OC_MEMORY.  OC_X87, OC_COMPLEX_X87
// and EB_SSEUP occur only in host-object LowIR (the object dialect
// `cppgm++ -c` reads), never in course LowIR.

enum EightbyteClass : std::uint8_t
{
  EB_NONE = 0,
  EB_INTEGER = 1,
  EB_SSE = 2,
  EB_SSEUP = 3
};

const unsigned kEightbyteClassCount = EB_SSEUP + 1;

enum ObjectClassBits : std::uint8_t
{
  OC_MEMORY = 0x10,
  OC_X87 = 0x20,
  OC_COMPLEX_X87 = 0x40,
  OC_STATED = 0x80
};

// The class of eightbyte 0 or 1; EB_NONE past them.
inline EightbyteClass eightbyte_class(std::uint8_t classes, unsigned eightbyte)
{
  return eightbyte < 2 ? static_cast<EightbyteClass>((classes >> (2 * eightbyte)) & 3u) : EB_NONE;
}

// ---------------------------------------------------------------------------
// Metadata keys

enum class MetadataKey : std::uint8_t
{
  // Boundary metadata, on functions and call signatures.
  Arity,
  Effects,
  Unwind,
  Return,
  Query,
  ResultClasses,
  // Symbol metadata.
  Storage,
  Section,
  Role,
  Linkage,
  Binding,
  Object,
  TlsFor,
  KeepAlias,
  PreferLocal,
  ObjectRoot,
  ForceInline,
  InlineHint,
  NoInline,
  Replaceable,
  // Parameter metadata.
  Pass,
  Alias,
  ObjectBytes,
  Classes,
  // Instruction metadata (`call ... [elision=copy, classes=1:sse]`,
  // `va_arg ... [classes=sse]`, `index ... [projection=field]`).
  Elision,
  Projection
};

const unsigned kMetadataKeyCount = static_cast<unsigned>(MetadataKey::Projection) + 1;

inline bool is_boundary_key(MetadataKey key)
{
  return key <= MetadataKey::ResultClasses;
}

inline bool is_symbol_key(MetadataKey key)
{
  return key >= MetadataKey::Storage && key <= MetadataKey::Replaceable;
}

inline bool is_parameter_key(MetadataKey key)
{
  return key >= MetadataKey::Pass && key <= MetadataKey::Classes;
}

// The key of a yes-valued symbol flag, and the flag of such a key (0 for
// any other key).
inline MetadataKey symbol_flag_key(SymbolFlag flag)
{
  return flag == SF_REPLACEABLE ? MetadataKey::Replaceable
    : flag == SF_KEEP_ALIAS ? MetadataKey::KeepAlias
    : flag == SF_PREFER_LOCAL ? MetadataKey::PreferLocal
    : flag == SF_OBJECT_ROOT ? MetadataKey::ObjectRoot
    : flag == SF_FORCE_INLINE ? MetadataKey::ForceInline
    : flag == SF_INLINE_HINT ? MetadataKey::InlineHint
    : MetadataKey::NoInline;
}

inline std::uint8_t symbol_flag_of(MetadataKey key)
{
  return key == MetadataKey::Replaceable ? SF_REPLACEABLE
    : key >= MetadataKey::KeepAlias && key <= MetadataKey::NoInline
    ? static_cast<std::uint8_t>(1u << (static_cast<unsigned>(key) - static_cast<unsigned>(MetadataKey::KeepAlias)))
    : 0;
}

// The one value of a symbol flag (`keep_alias=yes`) and of `elision=`.
inline const char * metadata_yes_spelling() { return "yes"; }
inline const char * elision_copy_spelling() { return "copy"; }

// ---------------------------------------------------------------------------
// Spellings

inline const char * spelling(SymbolRole value)
{
  static const char * const kRoles[kSymbolRoleCount] = { "", "entry", "init", "fini",
    "eh_allocate_exception", "eh_begin_catch", "eh_end_catch", "eh_rethrow", "eh_throw",
    "eh_personality", "eh_resume", "allocate_memory", "free_memory", "terminate", "pure_virtual",
    "dynamic_cast", "bad_cast", "bad_typeid", "rtti_class", "rtti_si", "rtti_vmi", "rtti_data",
    "bad_array_new_length", "rtti_pointer" };
  return kRoles[value];
}

inline const char * spelling(LanguageLinkageMode value)
{
  static const char * const kLinkages[kLanguageLinkageModeCount] = { "", "c" };
  return kLinkages[value];
}

inline const char * spelling(SymbolBindingMode value)
{
  static const char * const kBindings[kSymbolBindingModeCount] = { "", "internal", "strong", "weak" };
  return kBindings[value];
}

inline const char * spelling(GlobalStorageMode value)
{
  static const char * const kStorages[kGlobalStorageModeCount] = { "", "readonly", "thread_local" };
  return kStorages[value];
}

inline const char * spelling(CallArityMode value)
{
  static const char * const kArities[kCallArityModeCount] = { "", "variadic" };
  return kArities[value];
}

inline const char * spelling(CallEffectsMode value)
{
  static const char * const kEffects[kCallEffectsModeCount] = { "", "readnone", "readonly" };
  return kEffects[value];
}

inline const char * spelling(CallUnwindMode value)
{
  static const char * const kUnwinds[kCallUnwindModeCount] = { "", "no" };
  return kUnwinds[value];
}

inline const char * spelling(CallReturnMode value)
{
  static const char * const kReturns[kCallReturnModeCount] = { "", "noreturn" };
  return kReturns[value];
}

inline const char * spelling(CallQueryMode value)
{
  static const char * const kQueries[kCallQueryModeCount] = { "", "stable_prefix" };
  return kQueries[value];
}

inline const char * spelling(ParamPassingMode value)
{
  static const char * const kPasses[kParamPassingModeCount] = { "", "indirect_result", "by_address" };
  return kPasses[value];
}

inline const char * spelling(ParamAliasMode value)
{
  static const char * const kAliases[kParamAliasModeCount] = { "", "noalias" };
  return kAliases[value];
}

inline const char * spelling(IndexProjectionKind value)
{
  static const char * const kProjections[kIndexProjectionKindCount] = { "", "array_element", "field" };
  return kProjections[value];
}

// An eightbyte's class word; EB_NONE is spelled "none" because it names
// padding, not an omitted fact.
inline const char * spelling(EightbyteClass value)
{
  static const char * const kEightbytes[kEightbyteClassCount] = { "none", "integer", "sse", "sseup" };
  return kEightbytes[value];
}

inline const char * spelling(MetadataKey key)
{
  static const char * const kKeys[kMetadataKeyCount] = { "arity", "effects", "unwind", "return",
    "query", "result_classes", "storage", "section", "role", "linkage", "binding", "object",
    "tls_for", "keep_alias", "prefer_local", "object_root", "force_inline", "inline_hint",
    "no_inline", "replaceable", "pass", "alias", "object_bytes", "classes", "elision",
    "projection" };
  return kKeys[static_cast<unsigned>(key)];
}

// ---------------------------------------------------------------------------
// Parsers

inline bool from_spelling(const char * text, std::size_t length, SymbolRole * value)
{
  return detail::find_spelling(text, length, 1, kSymbolRoleCount, value);
}

inline bool from_spelling(const char * text, std::size_t length, LanguageLinkageMode * value)
{
  return detail::find_spelling(text, length, 1, kLanguageLinkageModeCount, value);
}

inline bool from_spelling(const char * text, std::size_t length, SymbolBindingMode * value)
{
  return detail::find_spelling(text, length, 1, kSymbolBindingModeCount, value);
}

inline bool from_spelling(const char * text, std::size_t length, GlobalStorageMode * value)
{
  return detail::find_spelling(text, length, 1, kGlobalStorageModeCount, value);
}

inline bool from_spelling(const char * text, std::size_t length, CallArityMode * value)
{
  return detail::find_spelling(text, length, 1, kCallArityModeCount, value);
}

inline bool from_spelling(const char * text, std::size_t length, CallEffectsMode * value)
{
  return detail::find_spelling(text, length, 1, kCallEffectsModeCount, value);
}

inline bool from_spelling(const char * text, std::size_t length, CallUnwindMode * value)
{
  return detail::find_spelling(text, length, 1, kCallUnwindModeCount, value);
}

inline bool from_spelling(const char * text, std::size_t length, CallReturnMode * value)
{
  return detail::find_spelling(text, length, 1, kCallReturnModeCount, value);
}

inline bool from_spelling(const char * text, std::size_t length, CallQueryMode * value)
{
  return detail::find_spelling(text, length, 1, kCallQueryModeCount, value);
}

inline bool from_spelling(const char * text, std::size_t length, ParamPassingMode * value)
{
  return detail::find_spelling(text, length, 1, kParamPassingModeCount, value);
}

inline bool from_spelling(const char * text, std::size_t length, ParamAliasMode * value)
{
  return detail::find_spelling(text, length, 1, kParamAliasModeCount, value);
}

inline bool from_spelling(const char * text, std::size_t length, IndexProjectionKind * value)
{
  return detail::find_spelling(text, length, 1, kIndexProjectionKindCount, value);
}

inline bool from_spelling(const char * text, std::size_t length, MetadataKey * key)
{
  return detail::find_spelling(text, length, 0, kMetadataKeyCount, key);
}

// ---------------------------------------------------------------------------
// Role facts (pa8/lowir.md "Reserved Runtime Hooks")

// The rtti_* roles belong to globals; the others to functions.
inline bool is_global_role(SymbolRole role)
{
  return (role >= SR_RTTI_CLASS && role <= SR_RTTI_DATA) || role == SR_RTTI_POINTER;
}

// Every role but rtti_data belongs to at most one symbol of a program.
inline bool is_singleton_role(SymbolRole role)
{
  return role != SR_NONE && role != SR_RTTI_DATA;
}

// The legacy symbol spelling that implies a role when no role metadata is
// present (`@main`, `@__cppgm_init`, `@__cppgm_fini`), or null.
inline const char * legacy_role_symbol(SymbolRole role)
{
  return role == SR_ENTRY ? "main" : role == SR_INIT ? "__cppgm_init" : role == SR_FINI ? "__cppgm_fini" : 0;
}

// ---------------------------------------------------------------------------
// Object class words

// Appends the `classes=` word of `classes` for an object of `bytes` bytes
// (0: unknown): `memory`, or the class of each eightbyte joined by `_`
// (two when the size is unknown); in host objects also `x87` and
// `complex_x87`, and any other value as its number.
inline void append_object_classes(std::string & out, std::uint8_t classes, std::uint64_t bytes)
{
  if(classes == (OC_STATED | OC_MEMORY)) {
    out += "memory";
    return;
  }
  if(classes == (OC_STATED | OC_X87)) {
    out += "x87";
    return;
  }
  if(classes == (OC_STATED | OC_COMPLEX_X87)) {
    out += "complex_x87";
    return;
  }
  if((classes & ~(OC_STATED | 15)) != 0 || (classes & OC_STATED) == 0) {
    out += std::to_string(static_cast<unsigned>(classes));
    return;
  }
  out += spelling(eightbyte_class(classes, 0));
  if(bytes > 8 || bytes == 0) {
    out += '_';
    out += spelling(eightbyte_class(classes, 1));
  }
}

// The value of a `classes=` word naming exactly the eightbytes of `bytes`
// (two if 0); the host-object words (`x87`, `complex_x87`, `sseup`) only
// with `extensions`.  0 if the word is not one.
inline std::uint8_t object_classes_from_spelling(const char * text, std::size_t length, std::uint64_t bytes,
                                         bool extensions)
{
  if(detail::same_spelling("memory", text, length)) {
    return OC_STATED | OC_MEMORY;
  }
  if(extensions && detail::same_spelling("x87", text, length)) {
    return OC_STATED | OC_X87;
  }
  if(extensions && detail::same_spelling("complex_x87", text, length)) {
    return OC_STATED | OC_COMPLEX_X87;
  }
  const unsigned eightbytes = bytes == 0 || bytes > 16 ? 2u : static_cast<unsigned>((bytes + 7) / 8);
  std::uint8_t classes = OC_STATED;
  std::size_t at = 0;
  for(unsigned k = 0; k < eightbytes; ++k) {
    std::size_t end = at;
    while(end < length && text[end] != '_') {
      ++end;
    }
    EightbyteClass found = EB_NONE;
    if(!detail::find_spelling(text + at, end - at, 0, extensions ? kEightbyteClassCount : static_cast<unsigned>(EB_SSEUP), &found) ||
       (end == length) != (k + 1 == eightbytes)) {
      return 0;
    }
    classes = static_cast<std::uint8_t>(classes | found << (2 * k));
    at = end + 1;
  }
  return classes;
}

// ---------------------------------------------------------------------------
// Records: the metadata facts as typed values.  They hold no names, so any
// model can embed them next to its own name representation.

struct SymbolFacts
{
  SymbolRole role = SR_NONE;
  LanguageLinkageMode linkage = LLM_DEFAULT;
  SymbolBindingMode binding = SBM_DEFAULT;
  GlobalStorageMode storage = GSM_DEFAULT;
  std::uint8_t flags = 0;  // SymbolFlag bits
};

struct FunctionBoundaryMetadata
{
  CallArityMode arity = CAM_FIXED;
  CallEffectsMode effects = CFXM_DEFAULT;
  CallUnwindMode unwind = CUM_DEFAULT;
  CallReturnMode returns = CRM_DEFAULT;
  CallQueryMode query = CQM_DEFAULT;
  std::uint8_t result_classes = 0;  // ObjectClassBits of a direct object result
};

struct ParameterMetadata
{
  ParamPassingMode pass = PPM_DIRECT;
  ParamAliasMode alias = PALM_DEFAULT;
  std::uint8_t classes = 0;        // ObjectClassBits of a direct object parameter
  std::uint64_t object_bytes = 0;  // `object_bytes=`; 0 when absent
};

}  // namespace ir_model
