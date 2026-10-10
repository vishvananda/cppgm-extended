#include "types/fundamental_type.h"

const char* FundamentalTypeName(FundamentalType type)
{
	switch (type)
	{
	case FundamentalType::SignedChar:
		return "signed char";
	case FundamentalType::ShortInt:
		return "short int";
	case FundamentalType::Int:
		return "int";
	case FundamentalType::LongInt:
		return "long int";
	case FundamentalType::LongLongInt:
		return "long long int";
	case FundamentalType::UnsignedChar:
		return "unsigned char";
	case FundamentalType::UnsignedShortInt:
		return "unsigned short int";
	case FundamentalType::UnsignedInt:
		return "unsigned int";
	case FundamentalType::UnsignedLongInt:
		return "unsigned long int";
	case FundamentalType::UnsignedLongLongInt:
		return "unsigned long long int";
	case FundamentalType::WcharT:
		return "wchar_t";
	case FundamentalType::Char:
		return "char";
	case FundamentalType::Char16T:
		return "char16_t";
	case FundamentalType::Char32T:
		return "char32_t";
	case FundamentalType::Bool:
		return "bool";
	case FundamentalType::Float:
		return "float";
	case FundamentalType::Double:
		return "double";
	case FundamentalType::LongDouble:
		return "long double";
	case FundamentalType::Void:
		return "void";
	case FundamentalType::NullptrT:
		return "nullptr_t";
	case FundamentalType::Int128:
		return "__int128";
	case FundamentalType::UnsignedInt128:
		return "unsigned __int128";
	case FundamentalType::Float16:
		return "_Float16";
	case FundamentalType::Float32:
		return "_Float32";
	case FundamentalType::Float64:
		return "_Float64";
	case FundamentalType::Float128:
		return "_Float128";
	case FundamentalType::Float32x:
		return "_Float32x";
	case FundamentalType::Float64x:
		return "_Float64x";
	case FundamentalType::GnuFloat128:
		return "__float128";
	case FundamentalType::BFloat16:
		return "__bf16";
	}
	return "?";
}

unsigned FundamentalTypeSize(FundamentalType type)
{
	switch (type)
	{
	case FundamentalType::SignedChar:
	case FundamentalType::UnsignedChar:
	case FundamentalType::Char:
	case FundamentalType::Bool:
		return 1;
	case FundamentalType::ShortInt:
	case FundamentalType::UnsignedShortInt:
	case FundamentalType::Char16T:
	case FundamentalType::Float16:
	case FundamentalType::BFloat16:
		return 2;
	case FundamentalType::Int:
	case FundamentalType::UnsignedInt:
	case FundamentalType::WcharT:
	case FundamentalType::Char32T:
	case FundamentalType::Float:
	case FundamentalType::Float32:
		return 4;
	case FundamentalType::LongInt:
	case FundamentalType::LongLongInt:
	case FundamentalType::UnsignedLongInt:
	case FundamentalType::UnsignedLongLongInt:
	case FundamentalType::Double:
	case FundamentalType::NullptrT:
	case FundamentalType::Float64:
	case FundamentalType::Float32x:
		return 8;
	case FundamentalType::LongDouble:
	case FundamentalType::Int128:
	case FundamentalType::UnsignedInt128:
	case FundamentalType::Float128:
	case FundamentalType::Float64x:
	case FundamentalType::GnuFloat128:
		return 16;
	case FundamentalType::Void:
		return 0;
	}
	return 0;
}

bool IsFloatingFundamental(FundamentalType type)
{
	return FloatingRank(type) != 0;
}

FundamentalType FloatingRepresentation(FundamentalType type)
{
	switch (type)
	{
	case FundamentalType::Float32:
		return FundamentalType::Float;
	case FundamentalType::Float64:
	case FundamentalType::Float32x:
		return FundamentalType::Double;
	case FundamentalType::Float64x:
		return FundamentalType::LongDouble;
	case FundamentalType::GnuFloat128:
		return FundamentalType::Float128;
	default:
		return type;
	}
}

unsigned FloatingRank(FundamentalType type)
{
	switch (type)
	{
	case FundamentalType::BFloat16:
		return 1;
	case FundamentalType::Float16:
		return 2;
	case FundamentalType::Float:
	case FundamentalType::Float32:
		return 3;
	case FundamentalType::Double:
	case FundamentalType::Float64:
	case FundamentalType::Float32x:
		return 4;
	case FundamentalType::LongDouble:
	case FundamentalType::Float64x:
		return 5;
	case FundamentalType::Float128:
	case FundamentalType::GnuFloat128:
		return 6;
	default:
		return 0;
	}
}
