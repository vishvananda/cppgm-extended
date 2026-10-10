// (C) 2013 CPPGM Foundation www.cppgm.org.  All rights reserved.
// Revised for cppgm-extended; see NOTICE.

#include <iostream>
#include <stdexcept>
#include <string>

using namespace std;

#include "preprocess/expr/controlling_expression.h"
#include "preprocess/lexer/pp_lexer.h"
#include "preprocess/source/source_buffer.h"
#include "support/internal_error.h"

// ppexpr: translation phases 1-3 of a C++ source file read from standard
// input, then the controlling expression on each logical line evaluated as
// in conditional inclusion ([cpp.cond] 16.1).

// The tool has no macro table: `defined X` is true iff the first UTF-8
// code unit of X is odd (the handout's mock `defined`).
bool MockIsDefinedIdentifier(const char* identifier, uint32_t length)
{
	return length != 0 && (static_cast<unsigned char>(identifier[0]) & 1) != 0;
}

class MockMacroDefinitions final : public MacroDefinitionQuery
{
public:
	bool is_defined(const char* spelling, uint32_t length) const override
	{
		return MockIsDefinedIdentifier(spelling, length);
	}
};

// One output line per non-empty logical line: a decimal value, suffixed `u`
// when unsigned, or `error`; then `eof`. Output is buffered.
class ResultWriter
{
public:
	explicit ResultWriter(ostream& out) : out_(out) { buffer_.reserve(kFlushSize + 64); }

	void write(const ControllingExpressionResult& result)
	{
		if (result.status == ControllingExpressionStatus::Invalid)
			buffer_ += "error\n";
		else if (result.status == ControllingExpressionStatus::Valid)
			write_value(result.value);
		if (buffer_.size() >= kFlushSize)
			flush();
	}

	void write_end()
	{
		buffer_ += "eof\n";
		flush();
	}

	void flush()
	{
		out_.write(buffer_.data(), buffer_.size());
		out_.flush();
		buffer_.clear();
	}

private:
	static const size_t kFlushSize = 64 * 1024;

	void write_value(const PPValue& value)
	{
		const bool negative = !value.is_unsigned && (value.bits >> 63) != 0;
		uint64_t magnitude = negative ? 0 - value.bits : value.bits;
		char digits[24];
		char* const end = digits + sizeof digits;
		char* first = end;
		do
		{
			*--first = static_cast<char>('0' + magnitude % 10);
			magnitude /= 10;
		} while (magnitude != 0);
		if (negative)
			*--first = '-';
		buffer_.append(first, end);
		if (value.is_unsigned)
			buffer_ += 'u';
		buffer_ += '\n';
	}

	ostream& out_;
	string buffer_;
};

bool HasBatchStdinArg(int argc, char** argv)
{
	for (int i = 1; i < argc; i++)
	{
		if (string(argv[i]) == "--batch-stdin")
			return true;
	}
	return false;
}

// Batch requests are served by the linked test runner; a build without it
// cannot run them.
int RunNotImplementedBatchMode()
{
	string line;
	while (getline(cin, line))
	{
		(void)line;
		cout << "EXIT_NOT_IMPLEMENTED" << endl;
	}
	return EXIT_SUCCESS;
}

int main(int argc, char** argv)
{
	try
	{
		if (HasBatchStdinArg(argc, argv))
			return RunNotImplementedBatchMode();

		SourceBuffer source = SourceBuffer::ReadFrom(cin);
		PPLexer lexer(source);
		MockMacroDefinitions macros;
		ControllingExpressionEvaluator evaluator(macros);
		ResultWriter output(cout);
		try
		{
			for (;;)
			{
				const ControllingExpressionResult result = evaluator.evaluate(lexer);
				output.write(result);
				if (result.end_of_file)
					break;
			}
		}
		catch (const SourceError& e)
		{
			// Phases 1-3 failed; the results of earlier lines are kept.
			output.flush();
			cerr << "ERROR: <stdin>:" << FormatSourceError(source, e) << endl;
			return EXIT_FAILURE;
		}
		output.write_end();
		return EXIT_SUCCESS;
	}
	catch (const InternalError& e)
	{
		cerr << "internal compiler error: " << e.what() << endl;
		return CPPGM_EXIT_INTERNAL_ERROR;
	}
	catch (const exception& e)
	{
		cerr << "ERROR: " << e.what() << endl;
		return EXIT_FAILURE;
	}
}
