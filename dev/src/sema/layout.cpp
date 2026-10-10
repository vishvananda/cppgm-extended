#include "sema/analyzer.h"

#include <algorithm>

namespace sema {

namespace {

uint64_t RoundUp(uint64_t value, uint64_t alignment)
{
	return (value + alignment - 1) / alignment * alignment;
}

// The class of the objects a member of this type holds (itself or its
// array elements) and how many there are; null if not a class.
ClassEntity* ElementClass(const Type* type, uint64_t& count)
{
	count = 1;
	type = type->unqualified;
	while (type->kind == TypeKind::Array)
	{
		count *= type->bound;
		type = type->element->unqualified;
	}
	return type->kind == TypeKind::Class ? static_cast<ClassEntity*>(type->entity) : nullptr;
}

// GNU packed packs a member of this class (or array of it): GCC leaves
// one of a class that is no POD for the purpose of layout and not packed
// itself unpacked.
bool PackableElement(const ClassEntity* element)
{
	return !element->non_pod || element->packed;
}

// The subobjects one class layout may visit to place its empty bases and
// members; past it the layout is rejected rather than searched on.
const uint64_t kLayoutWork = 1u << 20;

// The first of the elements from index `i` of an array of `size`-byte
// objects at `start` that does not lie wholly in the gap [begin, end) (an
// unbounded one when `end` is UINT64_MAX), or UINT64_MAX.
uint64_t FirstOutsideGap(uint64_t start, uint64_t size, uint64_t i, uint64_t begin, uint64_t end)
{
	const uint64_t at = start + i * size;
	if (at < begin || (end != UINT64_MAX && at + size > end))
		return i;
	return end == UINT64_MAX ? UINT64_MAX : (end - start) / size;
}

// A class object's data size as a potentially-overlapping member, where
// later members may start (Itanium 2.4 II.2: the larger of its non-virtual
// size and its data size): its virtual bases' data counts.
uint64_t OverlappingDataSize(const ClassEntity* cls)
{
	uint64_t size = cls->nvsize;
	for (uint32_t i = 0; i < VirtualBaseCount(cls); ++i)
	{
		const BaseClass* base = VirtualBases(cls)[i].base;
		const ClassEntity* base_class = static_cast<const ClassEntity*>(base->base);
		if (!base_class->empty)
			size = std::max(size, base->offset + base_class->nvsize);
	}
	return size;
}

} // namespace

bool Analyzer::object_layout(const Type* type, uint64_t& size, uint64_t& alignment)
{
	if (type->kind == TypeKind::Function || IsVoid(type) || (type->kind == TypeKind::Array && type->bound == 0))
		return false;
	const ClassEntity* entity = ClassOf(type);
	if (entity != nullptr && !require_complete(entity))
		return false;
	return type_size(type, size, alignment, 0);
}

bool Analyzer::member_offset(const VariableEntity* member, uint64_t& offset)
{
	if (member->data_member == nullptr || member->owner == nullptr || member->owner->entity == nullptr)
		return false;
	uint64_t size = 0, alignment = 0;
	if (!object_layout(member->owner->entity->type, size, alignment))
		return false;
	offset = member->data_member->offset / 8;
	return true;
}

void Analyzer::record_alias_alignment(const Specifiers& specifiers, const Binding* declared, uint64_t own)
{
	// GNU aligned on a typedef sets its alignment, weaker or stronger than
	// its type's (GCC); a typedef of such a typedef, or a variable or data
	// member declared with it, takes it when its type is the typedef's; a
	// variable declared a pointer to it records its pointee's. A stronger
	// alignment is the variable's requested one (its storage's).
	const Type* type = declared->type;
	const uint64_t aligned = own != 0 ? own : specifiers.alias_alignment;
	if (declared->kind == BindingKind::TypeAlias)
	{
		if (own != 0 || type == specifiers.type)
		{
			alias_alignments_.set(declared, static_cast<uint32_t>(aligned));
			const_cast<Binding*>(declared)->aligned_alias = true;
		}
		return;
	}
	if (declared->entity == nullptr || declared->entity->kind != EntityKind::Variable || type == nullptr)
		return;
	VariableEntity* variable = static_cast<VariableEntity*>(declared->entity);
	if (variable->type->kind == TypeKind::Pointer && variable->type->element == specifiers.type)
	{
		aligned_variables_.set(reinterpret_cast<const char*>(variable) + 1, static_cast<uint32_t>(aligned));
		return;
	}
	if (variable->type != specifiers.type)
		return;
	aligned_variables_.set(variable, static_cast<uint32_t>(aligned));
	uint64_t size = 0, natural = 0;
	if (object_layout(variable->type, size, natural) && aligned > natural)
		request_alignment(variable, aligned, nullptr);
}

uint64_t Analyzer::declared_alignment(const void* declared, uint64_t natural) const
{
	// A typedef's binding, or a variable or data member declared with an
	// aligned typedef: its alignment (see record_alias_alignment), else
	// `natural`.
	const uint32_t aligned = alias_alignments_.find(declared);
	return aligned != PointerMap::kMissing ? aligned : natural;
}

uint64_t Analyzer::declared_alignment(const DataMember* member, uint64_t natural) const
{
	const uint32_t aligned = aligned_variables_.size() != 0 ? aligned_variables_.find(member->variable)
															: PointerMap::kMissing;
	return aligned != PointerMap::kMissing ? aligned : natural;
}

uint32_t Analyzer::alias_alignment_of(const syntax::Expr& expr, bool pointee) const
{
	// GNU __alignof__ of an expression: the aligned typedef's alignment of
	// the variable or member it names, of `*p` through a pointer declared to
	// one, of `*&x` (0: none, the type's).
	const syntax::Expr* e = &expr;
	while (e->kind == syntax::ExprKind::Paren)
		e = static_cast<const syntax::ParenExpr*>(e)->inner;
	if (e->kind == syntax::ExprKind::Unary)
	{
		const syntax::UnaryExpr& unary = static_cast<const syntax::UnaryExpr&>(*e);
		if (unary.op == TokenKind::OP_STAR && !pointee)
			return alias_alignment_of(*unary.operand, true);
		if (unary.op == TokenKind::OP_AMP && pointee)
			return alias_alignment_of(*unary.operand, false);
		return 0;
	}
	const Entity* entity = e->facts != nullptr ? e->facts->entity : nullptr;
	if ((e->kind != syntax::ExprKind::Id && e->kind != syntax::ExprKind::Member) || entity == nullptr ||
		entity->kind != EntityKind::Variable)
		return 0;
	const uint32_t aligned = aligned_variables_.find(
		pointee ? reinterpret_cast<const char*>(entity) + 1 : static_cast<const void*>(entity));
	return aligned != PointerMap::kMissing ? aligned : 0;
}

bool Analyzer::member_size(const Type* type, uint64_t& size, uint64_t& alignment, uint32_t location)
{
	// A non-static data member's: GNU's flexible array member of unknown
	// bound (C99's, and a zero-length array, see array_bound) takes no
	// space, aligned as its element (hosted compilations).
	if (hosted_ && type->kind == TypeKind::Array && type->bound == 0 && type->owner == nullptr && template_depth_ == 0)
	{
		if (!type_size(type->element, size, alignment, location))
			return false;
		size = 0;
		return true;
	}
	return type_size(type, size, alignment, location);
}

bool Analyzer::type_size(const Type* type, uint64_t& size, uint64_t& alignment, uint32_t location)
{
	check_stack(location);
	if ((type->cv & kAtomic) != 0)
	{
		// An atomic type of at most 16 bytes takes its size rounded up to a
		// power of two and that alignment (Clang's x86-64 rule), so the
		// processor's atomic instructions cover it.
		if (!type_size(type->unqualified, size, alignment, location))
			return false;
		if (size <= 16)
		{
			uint64_t rounded = 1;
			while (rounded < size)
				rounded *= 2;
			size = alignment = rounded;
		}
		return true;
	}
	type = type->unqualified;
	switch (type->kind)
	{
	case TypeKind::Fundamental:
		if (type->fundamental == FundamentalType::Void)
			error(location, "size of void");
		size = FundamentalTypeSize(type->fundamental);
		alignment = size;
		return true;
	case TypeKind::Pointer:
	case TypeKind::BlockPointer:
		size = alignment = 8;
		return true;
	case TypeKind::MemberPointer:
		// Itanium: a data member pointer is an offset, a member function
		// pointer a (pointer, adjustment) pair.
		size = type->element->kind == TypeKind::Function ? 16 : 8;
		alignment = 8;
		return true;
	case TypeKind::LValueReference:
	case TypeKind::RValueReference:
		// As a member: the address it binds.
		size = alignment = 8;
		return true;
	case TypeKind::Array:
	{
		if (type->bound == 0)
		{
			// In a template an unknown bound may be a dependent one.
			if (template_depth_ != 0)
				return false;
			error(location, "size of an array of unknown bound");
		}
		uint64_t element = 0;
		if (!type_size(type->element, element, alignment, location))
			return false;
		if (element != 0 && type->bound > UINT64_MAX / element)
			error(location, "array too large");
		size = element * type->bound;
		return true;
	}
	case TypeKind::Complex:
		if (!type_size(type->element, size, alignment, location))
			return false;
		size *= 2;
		return true;
	case TypeKind::Vector:
	{
		// The lanes' total rounded up to a power of two, aligned as big (GCC
		// and Clang); a bool vector's lanes bits (Clang's).
		if (type->owner != nullptr)
			return false;
		size = type->element->fundamental == FundamentalType::Bool
			? (type->bound + 7) / 8
			: FundamentalTypeSize(type->element->fundamental) * type->bound;
		uint64_t rounded = 1;
		while (rounded < size)
			rounded <<= 1;
		size = alignment = rounded;
		return true;
	}
	case TypeKind::Enum:
	{
		const EnumEntity* entity = static_cast<const EnumEntity*>(type->entity);
		if (entity->underlying == nullptr)
			error(location, "size of an incomplete enumeration");
		return type_size(entity->underlying, size, alignment, location);
	}
	case TypeKind::Class:
		break;
	case TypeKind::Function:
		error(location, "size of a function type");
	default:
		// A dependent size waits for instantiation.
		if (template_depth_ != 0)
			return false;
		error(location, "size of a dependent type is not supported here");
	}
	ClassEntity* entity = static_cast<ClassEntity*>(type->entity);
	if (template_depth_ != 0 && dependent_type(type))
		return false;
	// A substitution's failure (14.8.2p8), not memoized: the class may be
	// completed later.
	if (!require_complete(entity))
		return substitution_error(location, "size of an incomplete class", true);
	if (entity->layout != FactState::Done && !lay_out(entity, location))
		return false;
	size = entity->size;
	alignment = entity->alignment;
	return true;
}

bool Analyzer::lay_out(ClassEntity* root, uint32_t location)
{
	// Lays out `root` after the classes its layout reads (its bases and its
	// members' classes), depth first with an explicit stack, so chains of
	// bases or members of any length take no call depth. Each class is laid
	// out once; one met again while its own layout waits contains itself.
	// A dependent member type (in a template) leaves the waiting classes
	// not started.
	if (root->layout == FactState::InProgress)
		error(location, "class contains itself");
	const size_t first = layout_frames_.size();
	root->layout = FactState::InProgress;
	layout_frames_.push_back(LayoutFrame{ root, root->bases, root->members });
	while (layout_frames_.size() > first)
	{
		LayoutFrame& frame = layout_frames_.back();
		ClassEntity* next = nullptr;
		while (next == nullptr && frame.base != nullptr)
		{
			next = static_cast<ClassEntity*>(frame.base->base);
			frame.base = frame.base->next;
			if (next->layout == FactState::Done)
				next = nullptr;
		}
		while (next == nullptr && frame.member != nullptr)
		{
			uint64_t count = 0;
			next = ElementClass(frame.member->variable->type, count);
			frame.member = frame.member->next;
			if (next != nullptr && next->layout == FactState::Done)
				next = nullptr;
		}
		if (next != nullptr)
		{
			if (!next->complete)
				error(location, "size of an incomplete class");
			if (next->layout == FactState::InProgress)
				error(location, "class contains itself");
			next->layout = FactState::InProgress;
			layout_frames_.push_back(LayoutFrame{ next, next->bases, next->members });
			continue;
		}
		ClassEntity* entity = frame.entity;
		layout_frames_.pop_back();
		if (!class_layout(entity, location))
		{
			entity->layout = FactState::NotStarted;
			for (size_t i = first; i < layout_frames_.size(); ++i)
				layout_frames_[i].entity->layout = FactState::NotStarted;
			layout_frames_.resize(first);
			return false;
		}
		entity->layout = FactState::Done;
	}
	return true;
}

bool Analyzer::class_layout(ClassEntity* entity, uint32_t location)
{
	// The Itanium C++ ABI's layout (2.4) of a class without virtual bases:
	// the virtual table pointer unless a primary base (the first
	// polymorphic one) provides it; the primary base, then the other bases
	// in declaration order, each at the data size so far (an empty base
	// first at offset 0), moved on by its alignment while an empty class
	// subobject would share its type and offset with another; then the
	// members likewise, bit-fields packed without straddling a unit of
	// their type (SysV x86-64). A base's tail padding is reused unless it
	// is a POD for the purpose of layout. Base and member offsets are
	// recorded with the size.
	//
	// The GCC extensions, as GCC lays them out: #pragma pack(N) caps the
	// alignment of non-empty bases and of members, even one an
	// alignment-specifier requests; GNU packed gives members alignment 1
	// unless their own alignment-specifier requests one, and leaves bases
	// alone. Under either, bit-fields are placed at the next bit and a
	// named one aligns the class by its type's alignment capped likewise;
	// a zero-width bit-field still aligns the next member to its type.
	//
	// lay_out() has laid out the base and member classes, so the sizes read
	// below are finished facts.
	const bool is_union = entity->key == TokenKind::KW_UNION;
	const uint64_t pack = entity->pack;
	const bool packed = entity->packed;
	bool packs_all = true;
	bool pod = !entity->non_pod && entity->bases == nullptr && !entity->polymorphic;
	bool contains_empty = entity->empty;
	bool volatile_or_union = is_union;
	uint64_t empty_base_limit = 0;
	const BaseClass* primary = nullptr;
	for (const BaseClass* base = entity->bases; base != nullptr; base = base->next)
	{
		const ClassEntity* base_class = static_cast<const ClassEntity*>(base->base);
		uint64_t size = 0, alignment = 1;
		if (!type_size(base_class->type, size, alignment, location))
			return false;
		contains_empty = contains_empty || base_class->contains_empty;
		volatile_or_union = volatile_or_union || base_class->contains_volatile_or_union;
		if (base_class->empty)
			empty_base_limit = std::max(empty_base_limit, base_class->size);
	}
	// Itanium 2.4 II.3: the base sharing the table pointer (possibly a
	// nearly empty virtual one).
	if (entity->dynamic)
		primary = layout_primary(entity);
	for (uint32_t i = 0; i < VirtualBaseCount(entity); ++i)
	{
		const ClassEntity* base_class = static_cast<const ClassEntity*>(VirtualBases(entity)[i].base->base);
		if (base_class->empty)
			empty_base_limit = std::max(empty_base_limit, base_class->size);
	}
	for (const DataMember* member = entity->members; member != nullptr; member = member->next)
	{
		uint64_t size = 0, alignment = 1;
		if (!member_size(member->variable->type, size, alignment, location))
			return false;
		uint64_t count = 0;
		const Type* scalar = member->variable->type;
		while (scalar->kind == TypeKind::Array)
			scalar = scalar->element;
		volatile_or_union = volatile_or_union || (scalar->cv & kVolatile) != 0;
		if (const ClassEntity* element = ElementClass(member->variable->type, count))
		{
			pod = pod && !element->non_pod;
			contains_empty = contains_empty || (element->contains_empty && count != 0);
			volatile_or_union = volatile_or_union || element->contains_volatile_or_union;
			packs_all = packs_all && PackableElement(element);
			// An empty potentially-overlapping member is tried at 0 too.
			if (member->overlapping && element->empty && !is_union)
				empty_base_limit = std::max(empty_base_limit, element->size);
		}
	}
	start_layout(entity, empty_base_limit, location);
	const size_t bit_first = bit_runs_.size();
	uint64_t data_bits = 0;
	uint64_t size = 0;
	uint64_t alignment = 1;
	place_table_pointer(entity, primary, data_bits, size, alignment, location);
	for (int pass = 0; pass < 2; ++pass)
	{
		for (BaseClass* base = entity->bases; base != nullptr; base = base->next)
		{
			if ((base == primary) != (pass == 0) || base->is_virtual)
				continue;
			const ClassEntity* base_class = static_cast<const ClassEntity*>(base->base);
			const uint64_t data_size = RoundUp(data_bits, 8) / 8;
			uint64_t offset = 0;
			// #pragma pack caps a non-empty base's alignment; a conflict
			// still moves a base on by its own.
			uint64_t base_alignment = base_class->alignment;
			if (base_class->empty)
			{
				// No data: the data size stays; past offset 0 the search
				// starts at it rounded up to the base's alignment (GCC, Clang).
				if (empty_subobjects(base_class, 0, EmptyWalk::Conflict, location))
				{
					offset = RoundUp(data_size, base_class->alignment);
					while (empty_subobjects(base_class, offset, EmptyWalk::Conflict, location))
						offset += base_class->alignment;
				}
				size = std::max(size, offset + base_class->size);
			}
			else
			{
				// A base is placed by its non-virtual size and alignment
				// (Itanium 2.4 II.3: its virtual bases are the complete
				// object's, but for those it claims).
				base_alignment = NonVirtualAlignment(base_class);
				if (pack != 0)
					base_alignment = std::min(base_alignment, pack);
				offset = RoundUp(data_size, base_alignment);
				while (base_conflict(base, base_class, offset, location))
					offset += NonVirtualAlignment(base_class);
				data_bits = (offset + base_class->nvsize) * 8;
				size = std::max(size, offset + base_class->nvsize);
			}
			alignment = std::max(alignment, base_alignment);
			base->offset = offset;
			check_class_size(size, location);
			// A non-empty base lies below the data size, where later
			// non-empty components cannot start, so of its empty subobjects
			// only those an empty component placed at offset 0 can meet matter.
			const EmptyWalk walk = base_class->empty ? EmptyWalk::Record : EmptyWalk::RecordReachable;
			empty_record_tail_ = RoundUp(data_bits, 8) / 8;
			empty_subobjects(base_class, offset, walk, location);
			if (layout_claims_)
				record_claimed(base, offset, walk, location);
		}
	}
	uint64_t union_size = 0;
	for (DataMember* member = entity->members; member != nullptr; member = member->next)
	{
		uint64_t member_size = 0, natural = 1;
		this->member_size(member->variable->type, member_size, natural, location);
		check_class_size(member_size | size, location);
		uint64_t count = 0;
		const ClassEntity* element = ElementClass(member->variable->type, count);
		// (A member packed itself is packed whatever its class: GCC.)
		const bool packed_member = member->packed || (packed && (element == nullptr || PackableElement(element)));
		const uint64_t requested = member->variable->kind == EntityKind::Variable
			? static_cast<const VariableEntity*>(member->variable)->alignment_request
			: 0;
		// A bit-field wider than its type is placed and aligned as the
		// largest integral type that fits in its width, and aligns its class
		// even unnamed (Itanium 2.4 II.1b, as GCC and Clang lay it out,
		// keeping such a class a POD); another unnamed bit-field does not
		// align its class.
		const bool wide = member->bit_field && member->width > member_size * 8;
		uint64_t member_alignment = member->bit_field ? natural : declared_alignment(member, natural);
		if (wide)
		{
			member_alignment = 8;
			while (member_alignment * 8 > member->width)
				member_alignment /= 2;
		}
		// 7.6.2: a member's alignment-specifier strengthens its alignment;
		// GNU packed makes it the requested one or 1, and #pragma pack caps
		// either.
		if (packed_member)
			member_alignment = requested != 0 ? requested : 1;
		else
			member_alignment = std::max(member_alignment, requested);
		if (pack != 0)
			member_alignment = std::min(member_alignment, pack);
		// The alignment it gives its class: a named or wide bit-field its
		// type's, #pragma pack capping it (else GNU packed), a wide one also
		// its own.
		uint64_t class_alignment = member_alignment;
		if (member->bit_field)
		{
			class_alignment = pack != 0 ? std::min(natural, pack) : packed_member ? 1 : natural;
			if (wide)
				class_alignment = std::max(class_alignment, member_alignment);
			else if (member->variable->name == nullptr)
				class_alignment = 1;
		}
		if (is_union)
		{
			const uint64_t member_bytes = member->bit_field ? RoundUp(member->width, 8) / 8 : member_size;
			union_size = std::max(union_size, member_bytes);
			alignment = std::max(alignment, class_alignment);
			member->offset = 0;
			// A union's bit-field is read as an object of its type.
			member->unit_offset = 0;
			member->unit_bytes = static_cast<uint8_t>(member_size);
			continue;
		}
		if (member->bit_field)
		{
			const uint64_t unit = natural * 8;
			uint64_t start = data_bits;
			if (member->width == 0)
				start = RoundUp(start, unit);
			else if (wide)
				start = RoundUp(start, member_alignment * 8);
			else if (pack == 0 && !packed_member && start / unit != (start + member->width - 1) / unit)
				start = RoundUp(start, unit);
			// A zero-width bit-field ends its run where it aligns; another
			// joins the run open since the data size (assign_bit_units).
			if (member->width == 0)
				end_bit_run(bit_first, start / 8);
			else
			{
				begin_bit_run(bit_first, RoundUp(data_bits, 8) / 8);
				bit_run_fields_.push_back(BitRunField{ member, start, std::min(member->width, member_size * 8),
					member_size, 0, 0, 0, pack != 0 || packed_member, false });
				bit_runs_.back().last = bit_run_fields_.size();
			}
			member->offset = start;
			data_bits = start + member->width;
			size = std::max(size, RoundUp(data_bits, 8) / 8);
			alignment = std::max(alignment, class_alignment);
			continue;
		}
		if (member->overlapping)
		{
			// An empty one aligns by its alignment-specifier, else its type
			// (GCC: neither GNU packed nor #pragma pack caps it).
			if (ClassOf(member->variable->type)->empty)
				member_alignment = requested != 0 ? member_alignment : natural;
			place_overlapping_member(member, member_alignment, natural, data_bits, location);
			if (!ClassOf(member->variable->type)->empty)
				end_bit_run(bit_first, member->offset / 8);
			size = std::max(size, member->offset / 8 + member_size);
			alignment = std::max(alignment, member_alignment);
			continue;
		}
		// A conflicting empty class member moves on by its type's alignment.
		uint64_t offset = RoundUp(RoundUp(data_bits, 8) / 8, member_alignment);
		while (member_conflict(member->variable->type, offset, location))
			offset = RoundUp(offset + natural, member_alignment);
		member->offset = offset * 8;
		end_bit_run(bit_first, offset);
		data_bits = (offset + member_size) * 8;
		size = std::max(size, offset + member_size);
		alignment = std::max(alignment, member_alignment);
		record_member(member->variable->type, offset, offset + member_size, location);
	}
	check_class_size(size, location);
	const uint64_t nvsize = is_union ? union_size : size;
	const uint64_t member_data = RoundUp(data_bits, 8) / 8;
	const uint64_t nvalign = alignment;
	allocate_virtual_bases(entity, data_bits, size, alignment, location);
	check_class_size(size, location);
	// 7.6.2p4-p5: the class's own alignment-specifier, which may not be
	// weaker than its natural alignment (GNU aligned only strengthens it).
	if (entity->alignment_request != 0)
	{
		if (entity->alignment_request >= alignment)
			alignment = entity->alignment_request;
		else if (!entity->alignment_request_gnu)
			error(location, "a class alignment-specifier weaker than its natural alignment");
	}
	entity->size = RoundUp(std::max<uint64_t>(is_union ? union_size : size, 1), alignment);
	entity->alignment = alignment;
	// The last run may reach the class's end when no derived class can
	// reuse its tail padding (a POD for the purpose of layout), else its
	// non-virtual data size.
	if (bit_runs_.size() != bit_first)
		assign_bit_units(bit_first, pod ? entity->size : member_data, location);
	entity->nvsize = pod ? entity->size : nvsize;
	if (VirtualBaseCount(entity) != 0)
		entity->virtuals->nvalign = nvalign;
	entity->non_pod = !pod;
	entity->contains_empty = contains_empty;
	entity->contains_volatile_or_union = volatile_or_union;
	entity->packed = packed && packs_all;
	return true;
}

namespace {

// The narrowest 1-, 2-, 4- or 8-byte window holding bytes [first, last)
// within [lo, hi): naturally aligned if one is, else starting at `first`,
// else ending at `hi`.
bool NarrowStorageWindow(uint64_t first, uint64_t last, uint64_t lo, uint64_t hi, uint64_t& begin, uint64_t& bytes)
{
	for (uint64_t width = 1; width <= 8; width *= 2)
	{
		if (width < last - first || width > hi - lo)
			continue;
		const uint64_t aligned = first / width * width;
		if (aligned >= lo && aligned + width <= hi && aligned + width >= last)
			begin = aligned;
		else if (first + width <= hi)
			begin = first;
		else if (hi - width >= lo && hi - width <= first)
			begin = hi - width;
		else
			continue;
		bytes = width;
		return true;
	}
	return false;
}

} // namespace

void Analyzer::begin_bit_run(size_t first, uint64_t first_byte)
{
	if (bit_runs_.size() > first && bit_runs_.back().open)
		return;
	const size_t at = bit_run_fields_.size();
	bit_runs_.push_back(BitRun{ at, at, first_byte, 0, true });
}

void Analyzer::end_bit_run(size_t first, uint64_t limit)
{
	if (bit_runs_.size() <= first || !bit_runs_.back().open)
		return;
	bit_runs_.back().hi = limit;
	bit_runs_.back().open = false;
}

void Analyzer::assign_bit_units(size_t first, uint64_t open_limit, uint32_t location)
{
	// The storage units of the class's bit-field runs (a run: adjacent
	// bit-fields, ended by another non-empty member or a zero-width
	// bit-field), each run within the bytes [lo, hi) it may touch: from the
	// data size where it began to the next component (the last run: to
	// `open_limit`). The naturally aligned objects of the fields' declared
	// types holding them (SysV) nest, so the largest ones partition the run
	// into segments; a segment's unit reaching outside the run is narrowed
	// to the narrowest integer holding its fields there, else each field
	// takes such a unit of its own (else its type's object, shared). A
	// packed run's fields each take their own; one no access within the
	// class holds is read and written piecewise (64-bit unit, or 128-bit
	// past 8 bytes, as GCC does).
	const size_t field_first = bit_runs_[first].first;
	for (size_t r = first; r < bit_runs_.size(); ++r)
	{
		BitRun& run = bit_runs_[r];
		if (run.open)
		{
			run.hi = open_limit;
			run.open = false;
		}
		if (run.first == run.last)
			continue;
		if (bit_run_fields_[run.first].packed)
		{
			assign_bit_fields(run.first, run.last, run.lo, run.hi, location);
			continue;
		}
		// The segments: the fields of each largest object (first, past
		// last), its first byte and size.
		struct Segment
		{
			size_t first, last;
			uint64_t begin, bytes;
		};
		std::vector<Segment> list;
		for (size_t i = run.first; i < run.last; ++i)
		{
			const uint64_t bytes = bit_run_fields_[i].storage;
			const uint64_t begin = bit_run_fields_[i].bit / 8 / bytes * bytes;
			size_t from = i;
			while (
				!list.empty() && begin <= list.back().begin && list.back().begin + list.back().bytes <= begin + bytes)
			{
				from = list.back().first;
				list.pop_back();
			}
			if (!list.empty() && list.back().begin <= begin && begin + bytes <= list.back().begin + list.back().bytes)
			{
				list.back().last = i + 1;
				continue;
			}
			list.push_back(Segment{ from, i + 1, begin, bytes });
		}
		for (const Segment& segment : list)
			assign_bit_segment(run, segment.first, segment.last, segment.begin, segment.bytes, location);
	}
	for (size_t i = field_first; i < bit_run_fields_.size(); ++i)
	{
		const BitRunField& field = bit_run_fields_[i];
		DataMember* member = field.member;
		member->unit_offset = field.unit_begin;
		member->unit_bytes = static_cast<uint8_t>(field.unit_bytes);
		member->unit_split = static_cast<uint8_t>(field.split);
		member->unit_shared = field.shared;
	}
	bit_runs_.resize(first);
	bit_run_fields_.resize(field_first);
}

void Analyzer::assign_bit_segment(
	const BitRun& run, size_t first, size_t last, uint64_t begin, uint64_t bytes, uint32_t location)
{
	uint64_t unit_begin = begin, unit_bytes = bytes;
	if (begin < run.lo || begin + bytes > run.hi)
	{
		const uint64_t lo = std::max(run.lo, begin);
		const uint64_t hi = std::min(run.hi, begin + bytes);
		uint64_t data_first = UINT64_MAX, data_last = 0;
		for (size_t i = first; i < last; ++i)
		{
			data_first = std::min(data_first, bit_run_fields_[i].bit / 8);
			data_last = std::max(data_last, RoundUp(bit_run_fields_[i].bit + bit_run_fields_[i].width, 8) / 8);
		}
		if (!NarrowStorageWindow(data_first, data_last, lo, hi, unit_begin, unit_bytes))
		{
			assign_bit_fields(first, last, lo, hi, location);
			return;
		}
	}
	for (size_t i = first; i < last; ++i)
	{
		bit_run_fields_[i].unit_begin = unit_begin;
		bit_run_fields_[i].unit_bytes = unit_bytes;
	}
}

void Analyzer::assign_bit_fields(size_t first, size_t last, uint64_t lo, uint64_t hi, uint32_t location)
{
	// Each field its own unit; units of neighbours that overlap without
	// coinciding are shared.
	for (size_t i = first; i < last; ++i)
	{
		BitRunField& field = bit_run_fields_[i];
		const uint64_t begin = field.bit / 8;
		const uint64_t end = RoundUp(field.bit + field.width, 8) / 8;
		if (NarrowStorageWindow(begin, end, lo, hi, field.unit_begin, field.unit_bytes))
			continue;
		if (field.packed)
		{
			if (end - begin > 16)
				error(location, "a packed bit-field spanning more than 16 bytes");
			field.unit_begin = begin;
			field.unit_bytes = end - begin <= 8 ? 8 : 16;
			field.split = end - begin;
			field.shared = true;
			continue;
		}
		field.unit_bytes = field.storage;
		field.unit_begin = begin / field.unit_bytes * field.unit_bytes;
		field.shared = true;
	}
	for (size_t i = first; i + 1 < last; ++i)
	{
		BitRunField& left = bit_run_fields_[i];
		BitRunField& right = bit_run_fields_[i + 1];
		const bool same = left.unit_begin == right.unit_begin && left.unit_bytes == right.unit_bytes;
		if (!same && right.unit_begin < left.unit_begin + left.unit_bytes)
			left.shared = right.shared = true;
	}
}

void Analyzer::place_overlapping_member(
	DataMember* member, uint64_t member_alignment, uint64_t natural, uint64_t& data_bits, uint32_t location)
{
	// A potentially-overlapping member is placed as a base is (Itanium 2.4
	// II.2-3): an empty one at offset 0 unless one of its empty subobjects
	// would share its type and offset with one placed before (an earlier
	// base's, member's or array element's), then from the data size
	// rounded up to its type's alignment on by `member_alignment` (as GCC
	// places it), leaving the data size; a non-empty one at the data size
	// by its alignment, moved on likewise, the data size then reaching its
	// own data size (its tail padding reusable).
	const ClassEntity* cls = ClassOf(member->variable->type);
	const uint64_t data_size = RoundUp(data_bits, 8) / 8;
	uint64_t offset = 0;
	if (cls->empty)
	{
		if (empty_subobjects(cls, 0, EmptyWalk::Conflict, location, true))
		{
			offset = RoundUp(data_size, natural);
			while (empty_subobjects(cls, offset, EmptyWalk::Conflict, location, true))
				offset += member_alignment;
		}
		empty_subobjects(cls, offset, EmptyWalk::Record, location, true);
	}
	else
	{
		offset = RoundUp(data_size, member_alignment);
		while (member_conflict(member->variable->type, offset, location))
			offset = RoundUp(offset + natural, member_alignment);
		const uint64_t end = offset + OverlappingDataSize(cls);
		data_bits = end * 8;
		record_member(member->variable->type, offset, end, location);
	}
	member->offset = offset * 8;
}

void Analyzer::record_member(const Type* type, uint64_t offset, uint64_t tail, uint32_t location)
{
	// A member's empty class subobjects (its array elements' in order)
	// placed at `offset`, its data ending at `tail`, that a later component
	// can meet (see EmptyWalk::RecordReachable): elements wholly between
	// the record limit and the tail are skipped, not visited.
	uint64_t count = 0;
	const ClassEntity* element = ElementClass(type, count);
	if (element == nullptr || !element->contains_empty)
		return;
	empty_record_tail_ = tail;
	for (uint64_t i = FirstOutsideGap(offset, element->size, 0, empty_record_limit_, tail); i < count;
		i = FirstOutsideGap(offset, element->size, i + 1, empty_record_limit_, tail))
		empty_subobjects(element, offset + i * element->size, EmptyWalk::RecordReachable, location, true);
}

void Analyzer::start_layout(const ClassEntity* entity, uint64_t empty_base_limit, uint32_t location)
{
	// No empty subobject placed yet. A class's indirect primary bases
	// lie with the subobjects that claim them (claim_primaries): placed and
	// recorded with the placement deciding where those are (plan_claims).
	if (empty_count_ != 0)
		std::fill(empty_subobjects_.begin(), empty_subobjects_.end(), EmptySubobject{ nullptr, 0 });
	empty_count_ = 0;
	max_empty_offset_ = 0;
	empty_record_limit_ = empty_base_limit;
	layout_budget_ = kLayoutWork;
	conflict_root_ = nullptr;
	layout_claims_ = entity->dynamic && VirtualBaseCount(entity) != 0 && has_indirect_primaries(entity) &&
		indirect_primaries(entity);
	if (layout_claims_)
	{
		claim_primaries(entity, location);
		plan_claims(entity, layout_primary(entity));
	}
}

bool Analyzer::base_conflict(const BaseClass* base, const ClassEntity* base_class, uint64_t offset, uint32_t location)
{
	return empty_subobjects(base_class, offset, EmptyWalk::Conflict, location) ||
		(layout_claims_ && claimed_conflict(base, offset, location));
}

void Analyzer::place_table_pointer(const ClassEntity* entity, const BaseClass* primary, uint64_t& data_bits,
	uint64_t& size, uint64_t& alignment, uint32_t location)
{
	// The table pointer at offset 0, unless a primary base provides it: a
	// virtual one is placed first, at offset 0 (its virtual bases are the
	// class's, allocated with them unless primary too); a non-virtual one is
	// the first base placed.
	if (entity->dynamic && primary == nullptr)
	{
		data_bits = 64;
		size = alignment = 8;
	}
	else if (primary != nullptr && primary->is_virtual)
	{
		const ClassEntity* base_class = static_cast<const ClassEntity*>(primary->base);
		const_cast<BaseClass*>(primary)->offset = 0;
		data_bits = base_class->nvsize * 8;
		size = base_class->nvsize;
		alignment = NonVirtualAlignment(base_class);
		empty_subobjects(base_class, 0, EmptyWalk::Record, location);
		if (layout_claims_)
			record_claimed(primary, 0, EmptyWalk::Record, location);
	}
}

void Analyzer::allocate_virtual_bases(
	ClassEntity* entity, uint64_t& data_bits, uint64_t& size, uint64_t& alignment, uint32_t location)
{
	// Itanium 2.4 IV: the virtual bases in inheritance graph order, each by
	// its non-virtual size at the data size rounded to its non-virtual
	// alignment, an empty one at offset 0 (else at the data size); one
	// whose empty subobjects would meet another of their type and offset
	// moves on by its alignment. Primary bases are not
	// allocated again: the class's own virtual one is at offset 0, an
	// indirect one where the subobject claiming it is (claim_primaries).
	const BaseClass* primary = entity->dynamic ? layout_primary(entity) : nullptr;
	const bool indirect = layout_claims_;
	for (uint32_t i = 0; i < VirtualBaseCount(entity); ++i)
	{
		BaseClass* base = VirtualBases(entity)[i].base;
		const ClassEntity* base_class = static_cast<const ClassEntity*>(base->base);
		if (base == primary || (indirect && indirect_primary_set_.find(base_class) != PointerMap::kMissing))
			continue;
		const uint64_t data_size = RoundUp(data_bits, 8) / 8;
		uint64_t offset = 0;
		if (base_class->empty)
		{
			if (empty_subobjects(base_class, 0, EmptyWalk::Conflict, location))
			{
				offset = RoundUp(data_size, base_class->alignment);
				while (empty_subobjects(base_class, offset, EmptyWalk::Conflict, location))
					offset += base_class->alignment;
			}
			size = std::max(size, offset + base_class->size);
			alignment = std::max(alignment, base_class->alignment);
		}
		else
		{
			offset = RoundUp(data_size, NonVirtualAlignment(base_class));
			while (base_conflict(base, base_class, offset, location))
				offset += NonVirtualAlignment(base_class);
			data_bits = (offset + base_class->nvsize) * 8;
			size = std::max(size, offset + base_class->nvsize);
			alignment = std::max(alignment, NonVirtualAlignment(base_class));
		}
		base->offset = offset;
		empty_subobjects(base_class, offset, EmptyWalk::Record, location);
		if (indirect)
			record_claimed(base, offset, EmptyWalk::Record, location);
	}
}

void Analyzer::spend_layout_work(uint32_t location)
{
	if (layout_budget_ == 0)
		error(location, "class layout needs more work than this implementation allows");
	--layout_budget_;
}

bool Analyzer::place_empty_subobject(const ClassEntity* entity, uint64_t offset, bool insert)
{
	// Whether (entity, offset) is in the set; with `insert`, it is then.
	if (insert && (empty_count_ + 1) * 2 > empty_subobjects_.size())
	{
		std::vector<EmptySubobject> old(
			std::max<size_t>(64, empty_subobjects_.size() * 2), EmptySubobject{ nullptr, 0 });
		old.swap(empty_subobjects_);
		empty_count_ = 0;
		for (const EmptySubobject& placed : old)
		{
			if (placed.type != nullptr)
				place_empty_subobject(placed.type, placed.offset, true);
		}
	}
	if (empty_subobjects_.empty())
		return false;
	const uint64_t mask = empty_subobjects_.size() - 1;
	uint64_t key = reinterpret_cast<uintptr_t>(entity) ^ (offset * 0x9e3779b97f4a7c15ull);
	key ^= key >> 29;
	for (uint64_t index = (key * 0xbf58476d1ce4e5b9ull >> 20) & mask;; index = (index + 1) & mask)
	{
		EmptySubobject& slot = empty_subobjects_[index];
		if (slot.type == entity && slot.offset == offset)
			return true;
		if (slot.type == nullptr)
		{
			if (insert)
			{
				slot = EmptySubobject{ entity, offset };
				++empty_count_;
			}
			return false;
		}
	}
}

bool Analyzer::empty_subobjects(
	const ClassEntity* root, uint64_t offset, EmptyWalk walk, uint32_t location, bool complete)
{
	// The empty class subobjects of `root` placed at `offset`, found with an
	// explicit stack: checked for one with the type and offset of one placed
	// before (Itanium 2.4 II.2), or recorded. Subobjects lie within their
	// object's bytes, so the search skips an object lying wholly in a gap:
	// past the largest recorded offset for a conflict, between
	// empty_record_limit_ and empty_record_tail_ for RecordReachable.
	uint64_t gap_begin = UINT64_MAX, gap_end = UINT64_MAX;
	if (walk == EmptyWalk::Conflict)
	{
		if (empty_count_ == 0)
			return false;
		// A class moved on by a conflict usually meets the same subobject
		// at the next offset: that one first.
		if (conflict_root_ == root && conflict_hint_.complete == complete &&
			place_empty_subobject(conflict_hint_.type, offset + conflict_hint_.offset, false))
			return true;
		gap_begin = max_empty_offset_ + 1;
	}
	else if (walk == EmptyWalk::RecordReachable)
	{
		gap_begin = empty_record_limit_;
		gap_end = empty_record_tail_;
	}
	const size_t first = empty_work_.size();
	empty_work_.push_back(EmptySubobject{ root, offset, complete });
	while (empty_work_.size() > first)
	{
		const EmptySubobject item = empty_work_.back();
		empty_work_.pop_back();
		const ClassEntity* entity = item.type;
		if (!entity->contains_empty ||
			(item.offset >= gap_begin && (gap_end == UINT64_MAX || item.offset + entity->size <= gap_end)))
			continue;
		if (entity->empty)
		{
			if (walk == EmptyWalk::Conflict)
			{
				if (place_empty_subobject(entity, item.offset, false))
				{
					empty_work_.resize(first);
					conflict_root_ = root;
					conflict_hint_ = EmptySubobject{ entity, item.offset - offset, complete };
					return true;
				}
			}
			else
			{
				place_empty_subobject(entity, item.offset, true);
				max_empty_offset_ = std::max(max_empty_offset_, item.offset);
			}
		}
		// A base subobject's virtual bases are its complete object's (a
		// complete object's are its own, at their offsets).
		for (const BaseClass* base = entity->bases; base != nullptr; base = base->next)
		{
			if (base->is_virtual)
				continue;
			spend_layout_work(location);
			empty_work_.push_back(
				EmptySubobject{ static_cast<const ClassEntity*>(base->base), item.offset + base->offset, false });
		}
		for (uint32_t i = 0; item.complete && i < VirtualBaseCount(entity); ++i)
		{
			const BaseClass* base = VirtualBases(entity)[i].base;
			spend_layout_work(location);
			empty_work_.push_back(
				EmptySubobject{ static_cast<const ClassEntity*>(base->base), item.offset + base->offset, false });
		}
		for (const DataMember* member = entity->members; member != nullptr; member = member->next)
		{
			uint64_t count = 0;
			const ClassEntity* element = member->bit_field ? nullptr : ElementClass(member->variable->type, count);
			if (element == nullptr || !element->contains_empty)
				continue;
			const uint64_t start = item.offset + member->offset / 8;
			for (uint64_t i = FirstOutsideGap(start, element->size, 0, gap_begin, gap_end); i < count;
				i = FirstOutsideGap(start, element->size, i + 1, gap_begin, gap_end))
			{
				spend_layout_work(location);
				empty_work_.push_back(EmptySubobject{ element, start + i * element->size, true });
			}
		}
	}
	return false;
}

bool Analyzer::member_conflict(const Type* type, uint64_t offset, uint32_t location)
{
	// A member's class objects (array elements in order) placed at `offset`.
	uint64_t count = 0;
	const ClassEntity* element = ElementClass(type, count);
	if (element == nullptr || !element->contains_empty)
		return false;
	for (uint64_t i = 0; i < count; ++i)
	{
		const uint64_t at = offset + i * element->size;
		if (at > max_empty_offset_)
			break;
		if (empty_subobjects(element, at, EmptyWalk::Conflict, location, true))
			return true;
	}
	return false;
}

} // namespace sema
