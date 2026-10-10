#include "support/unicode/utf8.h"

namespace {

bool IsContinuationByte(unsigned char byte)
{
	return (byte & 0xC0) == 0x80;
}

Utf8Decoded Failure(Utf8DecodeStatus status)
{
	Utf8Decoded result;
	result.status = status;
	result.code_point = 0;
	result.length = 0;
	return result;
}

} // namespace

Utf8Decoded DecodeUtf8(const unsigned char* bytes, std::size_t available)
{
	const unsigned char lead = bytes[0];
	if (lead < 0x80)
	{
		Utf8Decoded result = { Utf8DecodeStatus::Ok, lead, 1 };
		return result;
	}

	unsigned length;
	char32_t value;
	if (lead >= 0xC2 && lead <= 0xDF)
	{
		length = 2;
		value = lead & 0x1F;
	}
	else if (lead >= 0xE0 && lead <= 0xEF)
	{
		length = 3;
		value = lead & 0x0F;
	}
	else if (lead >= 0xF0 && lead <= 0xF4)
	{
		length = 4;
		value = lead & 0x07;
	}
	else
	{
		// 0x80-0xBF are continuation bytes, 0xC0/0xC1 only start overlong
		// forms and 0xF5-0xFF would encode values above U+10FFFF.
		return Failure(Utf8DecodeStatus::InvalidLeadingByte);
	}

	if (available < length)
	{
		for (std::size_t i = 1; i < available; ++i)
		{
			if (!IsContinuationByte(bytes[i]))
				return Failure(Utf8DecodeStatus::InvalidContinuationByte);
		}
		return Failure(Utf8DecodeStatus::Truncated);
	}

	for (unsigned i = 1; i < length; ++i)
	{
		if (!IsContinuationByte(bytes[i]))
			return Failure(Utf8DecodeStatus::InvalidContinuationByte);
		value = (value << 6) | (bytes[i] & 0x3F);
	}

	static const char32_t kMinimumForLength[5] = { 0, 0, 0x80, 0x800, 0x10000 };
	if (value < kMinimumForLength[length] || value > 0x10FFFF || (value >= 0xD800 && value <= 0xDFFF))
	{
		return Failure(Utf8DecodeStatus::InvalidScalarValue);
	}

	Utf8Decoded result = { Utf8DecodeStatus::Ok, value, length };
	return result;
}

const char* DescribeUtf8DecodeStatus(Utf8DecodeStatus status)
{
	switch (status)
	{
	case Utf8DecodeStatus::Ok:
		return "valid UTF-8";
	case Utf8DecodeStatus::InvalidLeadingByte:
		return "invalid UTF-8 leading byte";
	case Utf8DecodeStatus::InvalidContinuationByte:
		return "invalid UTF-8 continuation byte";
	case Utf8DecodeStatus::Truncated:
		return "truncated UTF-8 character";
	case Utf8DecodeStatus::InvalidScalarValue:
		return "invalid UTF-8 scalar value";
	}
	return "invalid UTF-8";
}

void AppendUtf8(std::string& out, char32_t code_point)
{
	if (code_point < 0x80)
	{
		out.push_back(static_cast<char>(code_point));
	}
	else if (code_point < 0x800)
	{
		out.push_back(static_cast<char>(0xC0 | (code_point >> 6)));
		out.push_back(static_cast<char>(0x80 | (code_point & 0x3F)));
	}
	else if (code_point < 0x10000)
	{
		out.push_back(static_cast<char>(0xE0 | (code_point >> 12)));
		out.push_back(static_cast<char>(0x80 | ((code_point >> 6) & 0x3F)));
		out.push_back(static_cast<char>(0x80 | (code_point & 0x3F)));
	}
	else
	{
		out.push_back(static_cast<char>(0xF0 | (code_point >> 18)));
		out.push_back(static_cast<char>(0x80 | ((code_point >> 12) & 0x3F)));
		out.push_back(static_cast<char>(0x80 | ((code_point >> 6) & 0x3F)));
		out.push_back(static_cast<char>(0x80 | (code_point & 0x3F)));
	}
}
