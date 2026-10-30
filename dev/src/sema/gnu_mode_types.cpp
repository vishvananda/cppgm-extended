#include "sema/analyzer.h"

#include <cstring>

#include "lex/identifier_table.h"

namespace sema {

const Type* Analyzer::declaration_mode_type(
	const Type* type, const Specifiers* specifiers, const syntax::Attribute* attributes)
{
	if (specifiers != nullptr)
		for (const syntax::DeclSpecifier& specifier : specifiers->mode_specifiers)
			if (specifier.kind == syntax::DeclSpecKind::Mode)
				type = mode_type(type, *specifier.mode, specifier.location);
	for (const syntax::Attribute* attribute = attributes; attribute != nullptr; attribute = attribute->next)
		if (attribute->kind == syntax::AttributeKind::Mode)
			type = mode_type(type, *attribute->mode, attribute->location);
	return type;
}

const Type* Analyzer::mode_type(const Type* type, const Identifier& mode, uint32_t location)
{
	// A machine mode changes the complete declared type, not a pointer's
	// pointee or an array's element. The target is System V AMD64: word,
	// pointer and unwind_word are DI, whose canonical integer type is long.
	if (type == nullptr)
		return type;
	const char* spelling = mode.spelling;
	uint32_t length = mode.length;
	if (length > 4 && spelling[0] == '_' && spelling[1] == '_' && spelling[length - 2] == '_' &&
		spelling[length - 1] == '_')
	{
		spelling += 2;
		length -= 4;
	}
	struct Mode
	{
		const char* name;
		FundamentalType signed_type;
		FundamentalType unsigned_type;
		bool complex;
	};
	static const Mode modes[] = { { "QI", FundamentalType::SignedChar, FundamentalType::UnsignedChar, false },
		{ "HI", FundamentalType::ShortInt, FundamentalType::UnsignedShortInt, false },
		{ "SI", FundamentalType::Int, FundamentalType::UnsignedInt, false },
		{ "DI", FundamentalType::LongInt, FundamentalType::UnsignedLongInt, false },
		{ "TI", FundamentalType::Int128, FundamentalType::UnsignedInt128, false },
		{ "byte", FundamentalType::SignedChar, FundamentalType::UnsignedChar, false },
		{ "word", FundamentalType::LongInt, FundamentalType::UnsignedLongInt, false },
		{ "pointer", FundamentalType::LongInt, FundamentalType::UnsignedLongInt, false },
		{ "unwind_word", FundamentalType::LongInt, FundamentalType::UnsignedLongInt, false },
		{ "HF", FundamentalType::Float16, FundamentalType::Float16, false },
		{ "SF", FundamentalType::Float, FundamentalType::Float, false },
		{ "DF", FundamentalType::Double, FundamentalType::Double, false },
		{ "XF", FundamentalType::LongDouble, FundamentalType::LongDouble, false },
		{ "TF", FundamentalType::Float128, FundamentalType::Float128, false },
		{ "SC", FundamentalType::Float, FundamentalType::Float, true },
		{ "DC", FundamentalType::Double, FundamentalType::Double, true },
		{ "XC", FundamentalType::LongDouble, FundamentalType::LongDouble, true },
		{ "TC", FundamentalType::Float128, FundamentalType::Float128, true } };
	const Mode* selected = nullptr;
	for (const Mode& candidate : modes)
	{
		if (std::strlen(candidate.name) == length && std::memcmp(candidate.name, spelling, length) == 0)
		{
			selected = &candidate;
			break;
		}
	}
	if (selected == nullptr)
		error(location, "an unknown or unsupported scalar machine mode");
	return apply_mode(type, selected->signed_type, location, selected->complex);
}

void Analyzer::filter_mode_alias(Specifiers& specifiers)
{
	if (!dependent_type(specifiers.type))
		return;
	// GCC ignores a mode on an unassociated dependent RHS, but retains
	// an alias-name's attribute. The parser appended the latter facts.
	const syntax::Slice<syntax::DeclSpecifier> written = specifiers.mode_specifiers;
	for (uint32_t i = 0; i < written.size; ++i)
	{
		if (written[i].kind == syntax::DeclSpecKind::Mode && written[i].keyword == TokenKind::KW_USING)
		{
			specifiers.mode_specifiers = syntax::Slice<syntax::DeclSpecifier>{ written.data + i, written.size - i };
			return;
		}
	}
	specifiers.mode_specifiers = syntax::Slice<syntax::DeclSpecifier>{ nullptr, 0 };
}

const Type* Analyzer::apply_mode(const Type* type, FundamentalType target, uint32_t location, bool complex_mode)
{
	const Type* base = type->unqualified;
	const bool integer_mode = !IsFloatingFundamental(target);
	if (base->kind == TypeKind::Pointer && integer_mode && FundamentalTypeSize(target) == 8)
		return type;
	if (base->kind == TypeKind::TemplateParameter && dependent_type(base))
		return types_.qualified(types_.mode(base, target, complex_mode), type->cv);
	if (complex_mode)
	{
		if (base->kind != TypeKind::Complex)
			error(location, "a complex machine mode applied to an inappropriate type");
		base = base->element->unqualified;
	}
	IntegralKind integer = IntegralKind();
	const bool integral = base->kind == TypeKind::Fundamental && base->fundamental != FundamentalType::Bool &&
		IntegralKindOf(base, integer);
	const bool floating = base->kind == TypeKind::Fundamental && IsFloatingFundamental(base->fundamental);
	if (integer_mode ? !integral : !floating)
		error(location, "a scalar machine mode applied to an inappropriate type");
	FundamentalType result = target;
	if (integer_mode && !integer.is_signed)
	{
		switch (target)
		{
		case FundamentalType::SignedChar:
			result = FundamentalType::UnsignedChar;
			break;
		case FundamentalType::ShortInt:
			result = FundamentalType::UnsignedShortInt;
			break;
		case FundamentalType::Int:
			result = FundamentalType::UnsignedInt;
			break;
		case FundamentalType::LongInt:
			result = FundamentalType::UnsignedLongInt;
			break;
		case FundamentalType::Int128:
			result = FundamentalType::UnsignedInt128;
			break;
		default:
			break;
		}
	}
	const Type* representation = types_.fundamental(result);
	return types_.qualified(complex_mode ? types_.complex(representation) : representation, type->cv);
}

} // namespace sema
