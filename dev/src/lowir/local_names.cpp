#include "lowir/local_names.h"

namespace lowir {

namespace {

// The course's spellings, by kind (its lowering's block labels and
// generated slot prefixes), then this implementation's.
const char* const kBlockStems[] = { "",
	// Statements.
	"if_then", "if_else", "if_end", "while_cond", "while_body", "while_end", "do_body", "do_cond", "do_end", "for_cond",
	"for_body", "for_iter", "for_end", "switch_dispatch", "switch_case", "switch_default", "switch_end", "goto",
	"block", "unreachable", "cond_true_cleanup", "cond_false_cleanup",
	// Expressions.
	"land_rhs", "land_short", "land_end", "lor_rhs", "lor_short", "lor_end", "cond_then", "cond_else", "cond_end",
	"condaddr_then", "condaddr_else", "condaddr_end", "condobj_then", "condobj_else", "condobj_end",
	"discard_cond_then", "discard_cond_else", "discard_cond_end", "basecast_null", "basecast_adjust", "basecast_end",
	"member_pointer_virtual", "member_pointer_direct", "member_pointer_end", "null_member_cond", "null_member_body",
	"null_member_end", "dyn_cast_scan", "dyn_cast_found", "dyn_cast_fail", "dyn_cast_end", "typeid_scan", "typeid_fail",
	"atomic_retry", "atomic_done",
	// Dynamic allocation.
	"new_init", "new_end", "array_new_length_valid", "array_new_length_invalid", "array_new_ctor_cond",
	"array_new_ctor_body", "array_new_ctor_end", "array_new_ctor_cleanup", "array_new_ctor_cont",
	"array_new_ctor_resume", "array_new_ctor_cleanup_cond", "array_new_ctor_cleanup_body", "zeroinit_cond",
	"zeroinit_body", "zeroinit_end", "array_dtor_cond", "array_dtor_body", "array_dtor_end", "delete_nonnull",
	"delete_end", "array_delete_nonnull", "array_delete_end", "array_delete_dtor_cond", "array_delete_dtor_body",
	"array_delete_dtor_end",
	// Objects and their lifetimes.
	"conditional_temporary_destroy", "conditional_temporary_done", "ref_destroy", "ref_destroy_done",
	"constructor_array_cond", "constructor_array_body", "constructor_array_end", "constructor_array_cleanup",
	"constructor_array_cleanup_body", "constructor_array_resume", "destructor_array_cond", "destructor_array_body",
	"destructor_array_end", "destructor_array_cleanup", "destructor_array_cleanup_body", "destructor_array_resume",
	"copy_array_cond", "copy_array_body", "copy_array_end", "copy_array_cleanup", "assign_array_cond",
	"assign_array_body", "assign_array_end", "construction_array_cleanup", "construction_array_cleanup_body",
	"construction_array_cleanup_end", "initlist_constructor_cleanup", "initlist_constructor_end",
	"initlist_constructor_cleanup_condition", "initlist_constructor_cleanup_body", "initlist_constructor_resume",
	"local_static_ready", "local_static_init", "local_static_ctor_run", "local_static_ctor_done",
	"local_static_destroy", "local_static_fini_next",
	// Cleanups and exception handling.
	"call_unwind_dispatch", "call_unwind_end", "throw_alloc_unwind_end", "cleanup_action", "cleanup_resume",
	"construction_cleanup", "return_cleanup_action", "return_cleanup_terminal", "return_cleanup_exit",
	"conditional_cleanup_dispatch", "conditional_cleanup_route", "conditional_cleanup_resume",
	"conditional_cleanup_landing", "lexical_cleanup_dispatch", "lexical_cleanup_terminate", "lexical_unwind", "try_end",
	"catch_dispatch", "catch_entry", "catch_body", "catch_next", "catch_cleanup", "function_exception_landing",
	"function_exception_action", "function_exception_resume", "constructor_cleanup", "constructor_cleanup_entry",
	"destructor_cleanup", "destructor_cleanup_entry", "destructor_end", "destructor_suffix_cleanup",
	"destructor_suffix_next", "destructor_return_epilogue", "return_null", "adjust_return",
	// This implementation's constructs.
	"complex_nan", "complex_recompute", "complex_by_d", "complex_by_c", "complex_end", "vector_take", "vector_join",
	"vector_unordered",
	// The optimizer's.
	"inline_entry", "inline_cont", "inline_copy", "split", "thread", "loop_preheader", "loop_copy", "loop_exit",
	"fill_fast", "fill_sized", "fill_same", "wide_loop", "wide_tail", "unrolled", "dead" };

static_assert(
	sizeof kBlockStems / sizeof kBlockStems[0] == static_cast<size_t>(BlockKind::Count), "one stem per block kind");

const char* const kSlotStems[] = { "", "cond", "land", "lor", "condaddr", "argobj", "retobj", "retref", "arg", "argarr",
	"call", "callobj", "refcall", "lifetime", "refarg", "tmpobj", "tmpref", "arraytmp", "baseobj", "classvalue",
	"stmtobj", "catch", "function_exception", "throw_alloc", "initlist", "discard", "discardarr", "cleanup_return",
	"basecast", "member_pointer_callee", "dyn_cast", "array_new_index", "array_new_size", "zeroinit_offset",
	"array_dtor_index", "array_delete_index", "constructor_array_index", "destructor_array_index",
	"destructor_progress", "copy_array_index", "assign_array_index", "initlist_constructor_index", "atomic_arg",
	"atomic_expected", "atomic_result", "catch_pointer", "catch_value", "floating_signbit", "null_member_element",
	"null_member_index", "__anonymous_union_storage", "complex", "vector", "soft_float", "bits", "asm",
	// The optimizer's.
	"retmerge", "retmergeobj", "aggregate", "field", "phi", "demote", "wide", "inlined", "", "", "" };

static_assert(
	sizeof kSlotStems / sizeof kSlotStems[0] == static_cast<size_t>(SlotKind::Count), "one stem per slot kind");

// Whether a name is spelled as a generated temporary: `t` and a decimal
// without leading zeros.
bool temporary_spelling(const NameTable& names, NameId name)
{
	const char* text = names.spelling(name);
	const uint32_t length = names.length(name);
	if (length < 2 || text[0] != 't' || text[1] == '0')
		return false;
	for (uint32_t i = 1; i < length; ++i)
		if (text[i] < '0' || text[i] > '9')
			return false;
	return true;
}

void append_number(std::string& out, uint64_t value)
{
	char digits[24];
	int count = 0;
	do
	{
		digits[count++] = static_cast<char>('0' + value % 10);
		value /= 10;
	} while (value != 0);
	while (count > 0)
		out.push_back(digits[--count]);
}

} // namespace

const char* block_kind_stem(BlockKind kind)
{
	return kBlockStems[static_cast<size_t>(kind)];
}

const char* slot_kind_stem(SlotKind kind)
{
	return kSlotStems[static_cast<size_t>(kind)];
}

void LocalNames::collect(Taken& taken, NameId name)
{
	// Only an explicit name ending in a digit or an underscore can equal a
	// generated spelling (or one made free of them).
	const uint32_t length = program_.names.length(name);
	const char last = length == 0 ? '\0' : program_.names.spelling(name)[length - 1];
	if (!((last >= '0' && last <= '9') || last == '_'))
		return;
	if (!taken.any)
	{
		taken.any = true;
		taken.names.clear();
	}
	if (taken.names.find(name) == nullptr)
		taken.names.insert(name, 1);
}

bool LocalNames::taken(Taken& taken, const std::string& spelling)
{
	if (!taken.any)
		return false;
	const NameId name = program_.names.find(spelling.data(), static_cast<uint32_t>(spelling.size()));
	return name != 0 && taken.names.find(name) != nullptr;
}

void LocalNames::unique(std::string& out, Taken& taken)
{
	while (this->taken(taken, scratch_))
		scratch_.push_back('_');
	out += scratch_;
}

void LocalNames::reset(const FunctionBody& body)
{
	body_ = &body;
	values_.any = blocks_.any = false;
	for (const Temp& temp : body.temps)
		if (temp.name != 0 && temp.kind == TempKind::Plain)
			collect(values_, temp.name);
	for (const Slot& slot : body.slots)
		if (slot.name != 0 && slot.kind == SlotKind::Plain)
			collect(values_, slot.name);
	for (const Block& block : body.blocks)
		if (block.name != 0 && block.kind == BlockKind::Plain)
			collect(blocks_, block.name);
	ordinals(body);
}

void LocalNames::ordinals(const FunctionBody& body)
{
	// Generated slots count in order, skipping an ordinal whose spelling
	// an explicit name has (the course's generated names); a shadow is its
	// name's n-th slot (the first is the shadowed one); range-for
	// variables count apart.
	slot_ordinals_.assign(body.slots.size(), 0);
	uint32_t generated = 0, range_for = 0;
	bool shadows = false;
	for (SlotId s = 0; s < body.slots.size(); ++s)
	{
		const Slot& slot = body.slots[s];
		if (slot.kind == SlotKind::Plain)
			continue;
		if (slot.kind <= SlotKind::LastGenerated)
		{
			for (;;)
			{
				slot_ordinals_[s] = ++generated;
				if (!values_.any)
					break;
				if (slot.name != 0)
					scratch_.assign(program_.names.spelling(slot.name), program_.names.length(slot.name));
				else
					scratch_ = slot_kind_stem(slot.kind);
				scratch_ += "__";
				append_number(scratch_, generated);
				if (!taken(values_, scratch_))
					break;
			}
		}
		else if (slot.kind == SlotKind::RangeFor)
			slot_ordinals_[s] = ++range_for;
		else if (slot.kind == SlotKind::Shadow)
			shadows = true;
	}
	if (shadows)
	{
		FlatMap<NameId, uint32_t, NameHash> counts;
		for (SlotId s = 0; s < body.slots.size(); ++s)
		{
			const Slot& slot = body.slots[s];
			if (slot.name == 0 || (slot.kind != SlotKind::Plain && slot.kind != SlotKind::Shadow))
				continue;
			uint32_t* count = counts.find(slot.name);
			if (count == nullptr)
				counts.insert(slot.name, 1);
			else
				++*count;
			if (slot.kind == SlotKind::Shadow)
				slot_ordinals_[s] = count == nullptr ? 1 : *count;
		}
	}
	// Temporaries: `t<n>` by position unless an explicit name has such a
	// spelling, whose ordinal they then skip.
	temp_ordinals_.clear();
	if (!values_.any)
		return;
	bool reserved = false;
	for (uint32_t t = 0; t < body.temps.size() && !reserved; ++t)
		reserved = body.temps[t].name != 0 && body.temps[t].kind == TempKind::Plain &&
			temporary_spelling(program_.names, body.temps[t].name);
	for (SlotId s = 0; s < body.slots.size() && !reserved; ++s)
		reserved = body.slots[s].kind == SlotKind::Plain && body.slots[s].name != 0 &&
			temporary_spelling(program_.names, body.slots[s].name);
	if (!reserved)
		return;
	temp_ordinals_.assign(body.temps.size(), 0);
	uint32_t ordinal = 0;
	for (uint32_t t = body.parameter_count; t < body.temps.size(); ++t)
	{
		for (;;)
		{
			++ordinal;
			scratch_ = "t";
			append_number(scratch_, ordinal);
			if (!taken(values_, scratch_))
				break;
		}
		temp_ordinals_[t] = ordinal;
	}
}

void LocalNames::boundary_parameter(std::string& out, const Signature& signature, uint32_t index)
{
	const Parameter* parameters = program_.parameters_of(signature);
	if (parameters[index].name != 0)
	{
		out.append(program_.names.spelling(parameters[index].name), program_.names.length(parameters[index].name));
		return;
	}
	// Free of the boundary's other parameters' names.
	std::string spelling = "arg";
	append_number(spelling, index);
	for (bool taken = true; taken;)
	{
		taken = false;
		for (uint32_t i = 0; i < signature.parameter_count && !taken; ++i)
			taken = parameters[i].name != 0 && program_.names.length(parameters[i].name) == spelling.size() &&
				spelling.compare(0, spelling.size(), program_.names.spelling(parameters[i].name)) == 0;
		if (taken)
			spelling.push_back('_');
	}
	out += spelling;
}

void LocalNames::temp(std::string& out, TempId temp)
{
	const Temp& entity = body_->temps[temp];
	if (entity.name != 0 && entity.kind == TempKind::Plain)
	{
		out.append(program_.names.spelling(entity.name), program_.names.length(entity.name));
		return;
	}
	if (temp < body_->parameter_count)
	{
		scratch_ = "__param";
		append_number(scratch_, temp);
		unique(out, values_);
		return;
	}
	const uint32_t ordinal = temp_ordinals_.empty() ? temp - body_->parameter_count + 1 : temp_ordinals_[temp];
	if (entity.name == 0)
	{
		out.push_back('t');
		append_number(out, ordinal);
		return;
	}
	scratch_.assign(program_.names.spelling(entity.name), program_.names.length(entity.name));
	scratch_.push_back('_');
	append_number(scratch_, ordinal);
	unique(out, values_);
}

void LocalNames::slot(std::string& out, SlotId id)
{
	const Slot& slot = body_->slots[id];
	if (slot.kind == SlotKind::Plain && slot.name != 0)
	{
		out.append(program_.names.spelling(slot.name), program_.names.length(slot.name));
		return;
	}
	if (slot.kind == SlotKind::Plain)
	{
		scratch_.clear();
		append_number(scratch_, id);
		unique(out, values_);
		return;
	}
	if (slot.kind <= SlotKind::LastGenerated)
	{
		// (An inlined copy of a named slot derives from its name.)
		if (slot.name != 0)
			scratch_.assign(program_.names.spelling(slot.name), program_.names.length(slot.name));
		else
			scratch_ = slot_kind_stem(slot.kind);
		scratch_ += "__";
	}
	else if (slot.kind == SlotKind::Parameter)
		scratch_ = "__param";
	else
	{
		scratch_.assign(program_.names.spelling(slot.name), program_.names.length(slot.name));
		if (slot.kind == SlotKind::Shadow)
			scratch_ += "__shadow";
	}
	append_number(scratch_, slot.kind == SlotKind::Parameter ? slot.index : slot_ordinals_[id]);
	unique(out, values_);
}

void LocalNames::block(std::string& out, BlockId id)
{
	const Block& block = body_->blocks[id];
	if (block.kind == BlockKind::Plain && block.name != 0)
	{
		out.append(program_.names.spelling(block.name), program_.names.length(block.name));
		return;
	}
	// (An inlined copy of a named block derives from its name.)
	if (block.name != 0)
		scratch_.assign(program_.names.spelling(block.name), program_.names.length(block.name));
	else
		scratch_ = block_kind_stem(block.kind);
	if (block.kind != BlockKind::Plain)
		scratch_.push_back('_');
	append_number(scratch_, id);
	unique(out, blocks_);
}

} // namespace lowir
