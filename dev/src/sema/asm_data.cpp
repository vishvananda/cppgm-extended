#include "sema/asm_data.h"

#include <cstdlib>

namespace sema {
namespace {

bool MetadataName(const std::string& name)
{
	if (name.empty() || (name[0] >= '0' && name[0] <= '9'))
		return false;
	for (char c : name)
	{
		if (!((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') || c == '_' || c == '.'))
			return false;
	}
	// GAS gives its predefined loader/unwind sections implicit properties;
	// these are not custom nonloaded sections, even with an empty flag list.
	const char* const reserved[] = { ".text", ".data", ".bss", ".rodata", ".init", ".fini", ".init_array",
		".fini_array", ".preinit_array", ".tdata", ".tbss", ".ctors", ".dtors", ".eh_frame", ".eh_frame_hdr",
		".gcc_except_table", ".comment", ".dynamic", ".dynsym", ".dynstr", ".interp", ".hash", ".got", ".got.plt",
		".plt", ".rela.dyn", ".rela.plt", ".symtab", ".strtab", ".shstrtab" };
	for (const char* section : reserved)
	{
		if (name == section)
			return false;
	}
	return name.compare(0, 5, ".gnu.") != 0;
}

class DataLifter
{
public:
	DataLifter(const std::string& text, std::vector<AsmDataImage>& sections)
		: text_(text), sections_(sections), pos_(0), section_(-1)
	{}
	bool run();

private:
	void space();
	bool take(char c);
	std::string word();
	bool quoted(std::string& result);
	bool number(long& result);
	bool section(bool push);
	bool data(bool terminate);
	bool bytes();
	bool end();
	const std::string& text_;
	std::vector<AsmDataImage>& sections_;
	size_t pos_;
	int32_t section_;
	std::vector<int32_t> stack_;
};

void DataLifter::space()
{
	while (pos_ < text_.size() && (text_[pos_] == ' ' || text_[pos_] == '\t' || text_[pos_] == '\r'))
		++pos_;
}

bool DataLifter::take(char c)
{
	space();
	if (pos_ == text_.size() || text_[pos_] != c)
		return false;
	++pos_;
	return true;
}

std::string DataLifter::word()
{
	space();
	const size_t start = pos_;
	while (pos_ < text_.size())
	{
		const char c = text_[pos_];
		if (c == ' ' || c == '\t' || c == '\r' || c == '\n' || c == ';' || c == ',' || c == '#')
			break;
		++pos_;
	}
	return text_.substr(start, pos_ - start);
}

bool DataLifter::quoted(std::string& result)
{
	if (!take('"'))
		return false;
	result.clear();
	while (pos_ < text_.size())
	{
		unsigned char c = text_[pos_++];
		if (c == '"')
			return true;
		if (c == '\n' || c == 0)
			return false;
		if (c == '\\')
		{
			if (pos_ == text_.size())
				return false;
			c = text_[pos_++];
			if (c >= '0' && c <= '7')
			{
				unsigned value = c - '0';
				for (unsigned n = 1; n < 3 && pos_ < text_.size() && text_[pos_] >= '0' && text_[pos_] <= '7'; ++n)
					value = value * 8 + static_cast<unsigned>(text_[pos_++] - '0');
				c = static_cast<unsigned char>(value);
			}
			else if (c == 'x')
			{
				const size_t first = pos_;
				unsigned value = 0;
				while (pos_ < text_.size())
				{
					const char digit = text_[pos_];
					const unsigned part = digit >= '0' && digit <= '9' ? static_cast<unsigned>(digit - '0')
						: digit >= 'a' && digit <= 'f'                 ? static_cast<unsigned>(digit - 'a' + 10)
						: digit >= 'A' && digit <= 'F'                 ? static_cast<unsigned>(digit - 'A' + 10)
																	   : 16;
					if (part == 16)
						break;
					value = value * 16 + part;
					++pos_;
				}
				if (pos_ == first)
					return false;
				c = static_cast<unsigned char>(value);
			}
			else
			{
				switch (c)
				{
				case 'b':
					c = '\b';
					break;
				case 'f':
					c = '\f';
					break;
				case 'n':
					c = '\n';
					break;
				case 'r':
					c = '\r';
					break;
				case 't':
					c = '\t';
					break;
				case '\\':
				case '"':
					break;
				default:
					return false;
				}
			}
		}
		result.push_back(static_cast<char>(c));
	}
	return false;
}

bool DataLifter::number(long& result)
{
	space();
	const char* first = text_.c_str() + pos_;
	char* last = nullptr;
	result = std::strtol(first, &last, 0);
	if (first == last)
		return false;
	pos_ += static_cast<size_t>(last - first);
	return true;
}

bool DataLifter::section(bool push)
{
	std::string name, flags;
	space();
	if (pos_ < text_.size() && text_[pos_] == '"')
	{
		if (!quoted(name))
			return false;
	}
	else
		name = word();
	if (!MetadataName(name))
		return false;
	if (take(','))
	{
		if (!quoted(flags) || (!flags.empty() && flags != "MS" && flags != "SM"))
			return false;
		if (take(','))
		{
			const std::string type = word();
			if (type != "@progbits" && type != "%progbits")
				return false;
			if (take(','))
			{
				long size = 0;
				if (flags.empty() || !number(size) || size != 1)
					return false;
			}
			else if (!flags.empty())
				return false;
		}
		else if (!flags.empty())
			return false;
	}
	if (push)
		stack_.push_back(section_);
	section_ = static_cast<int32_t>(sections_.size());
	sections_.push_back(AsmDataImage{ name, std::vector<uint8_t>(), !flags.empty() });
	return true;
}

bool DataLifter::data(bool terminate)
{
	if (section_ < 0)
		return false;
	do
	{
		std::string text;
		if (!quoted(text))
			return false;
		std::vector<uint8_t>& bytes = sections_[section_].bytes;
		bytes.insert(bytes.end(), text.begin(), text.end());
		if (terminate)
			bytes.push_back(0);
	} while (take(','));
	return true;
}

bool DataLifter::bytes()
{
	if (section_ < 0)
		return false;
	do
	{
		long value = 0;
		if (!number(value) || value < -128 || value > 255)
			return false;
		sections_[section_].bytes.push_back(static_cast<uint8_t>(value));
	} while (take(','));
	return true;
}

bool DataLifter::end()
{
	space();
	if (pos_ < text_.size() && text_[pos_] == '#')
		while (pos_ < text_.size() && text_[pos_] != '\n')
			++pos_;
	return pos_ == text_.size() || take('\n') || take(';');
}

bool DataLifter::run()
{
	while (pos_ < text_.size())
	{
		space();
		if (pos_ == text_.size())
			break;
		if (text_[pos_] == '\n' || text_[pos_] == ';' || text_[pos_] == '#')
		{
			if (!end())
				return false;
			continue;
		}
		const std::string directive = word();
		bool ok = false;
		if (directive == ".pushsection" || directive == ".section")
			ok = section(directive == ".pushsection");
		else if (directive == ".popsection" && !stack_.empty())
		{
			section_ = stack_.back();
			stack_.pop_back();
			ok = true;
		}
		else if (directive == ".ascii" || directive == ".asciz" || directive == ".string")
			ok = data(directive != ".ascii");
		else if (directive == ".byte")
			ok = bytes();
		if (!ok || !end())
			return false;
	}
	// The compiler's surrounding code assumes its section is unchanged.
	// A basic asm definition must restore it before that code resumes.
	return stack_.empty() && section_ == -1;
}

} // namespace

bool LiftAsmData(const std::string& text, std::vector<AsmDataImage>& sections, std::string& error)
{
	if (DataLifter(text, sections).run())
		return true;
	error = "namespace asm outside custom nonloaded PROGBITS section/data directives";
	return false;
}

} // namespace sema
