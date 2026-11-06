// Native LLVM 21 expectation discovery for the external correctness adapter.
// Uses Clang's public verifier and preprocessor APIs; no C++ AST is parsed and
// no candidate diagnostic text is matched. See NOTICE for compiler attribution.
#include "clang/Basic/Diagnostic.h"
#include "clang/Basic/DiagnosticFrontend.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/CompilerInvocation.h"
#include "clang/Frontend/VerifyDiagnosticConsumer.h"
#include "clang/Lex/Lexer.h"
#include "clang/Lex/PPCallbacks.h"
#include "clang/Lex/Preprocessor.h"
#include "llvm/ADT/SmallString.h"
#include "llvm/Support/JSON.h"
#include "llvm/Support/MemoryBuffer.h"
#include "llvm/Support/SHA256.h"
#include "llvm/Support/VirtualFileSystem.h"
#include "llvm/Support/raw_ostream.h"
#include <memory>
#include <set>
#include <string>
#include <vector>

using namespace clang;

namespace {

int blocked(llvm::StringRef reason)
{
	llvm::outs() << llvm::json::Value(llvm::json::Object{ { "status", "harness-blocked" }, { "reason", reason } })
				 << '\n';
	return 2;
}

class Expectations : public DiagnosticConsumer
{
public:
	bool errors = false;
	bool warnings = false;
	bool malformed = false;
	llvm::json::Array rejected;
	void HandleDiagnostic(DiagnosticsEngine::Level, const Diagnostic& diagnostic) override
	{
		// With no supplied events, native CheckResults reports required
		// expectations using this typed ID and its original severity argument.
		// Any parser error, unexpected diagnostic or missing directive blocks.
		if (diagnostic.getID() != diag::err_verify_inconsistent_diags || diagnostic.getNumArgs() != 5 ||
			diagnostic.getArgKind(2) != DiagnosticsEngine::ak_c_string ||
			(diagnostic.getArgKind(3) != DiagnosticsEngine::ak_uint &&
				diagnostic.getArgKind(3) != DiagnosticsEngine::ak_sint) ||
			(diagnostic.getArgKind(3) == DiagnosticsEngine::ak_uint ? diagnostic.getArgUInt(3) != 0
																	: diagnostic.getArgSInt(3) != 0))
		{
			malformed = true;
			llvm::json::Array kinds;
			for (unsigned i = 0; i < diagnostic.getNumArgs(); ++i)
				kinds.push_back(static_cast<unsigned>(diagnostic.getArgKind(i)));
			rejected.push_back(
				llvm::json::Object{ { "id", diagnostic.getID() }, { "argument_kinds", std::move(kinds) } });
			return;
		}
		llvm::StringRef kind(diagnostic.getArgCStr(2));
		errors |= kind == "error";
		warnings |= kind == "warning";
	}
};

class Comments : public CommentHandler
{
	VerifyDiagnosticConsumer& verifier;
	DiagnosticsEngine& engine;

public:
	Comments(VerifyDiagnosticConsumer& verifier, DiagnosticsEngine& engine) : verifier(verifier), engine(engine) {}
	bool HandleComment(Preprocessor& preprocessor, SourceRange range) override
	{
		DiagnosticsEngine& saved = preprocessor.getDiagnostics();
		preprocessor.setDiagnostics(engine);
		const bool result = verifier.HandleComment(preprocessor, range);
		preprocessor.setDiagnostics(saved);
		return result;
	}
};

class Files : public PPCallbacks
{
	SourceManager& manager;
	std::set<std::string>& files;
	bool& pragmas;
	llvm::json::Array& pragmaEvents;

public:
	Files(SourceManager& manager, std::set<std::string>& files, bool& pragmas, llvm::json::Array& pragmaEvents)
		: manager(manager), files(files), pragmas(pragmas), pragmaEvents(pragmaEvents)
	{}
	void PragmaDirective(SourceLocation location, PragmaIntroducerKind) override
	{
		location = manager.getFileLoc(location);
		if (location.isInvalid())
		{
			pragmas = true;
			return;
		}
		const auto file = manager.getFileID(location);
		const auto buffer = manager.getBufferData(file);
		const size_t offset = manager.getFileOffset(location);
		const size_t previous = buffer.rfind('\n', offset);
		const size_t start = previous == llvm::StringRef::npos ? 0 : previous + 1;
		const size_t end = buffer.find('\n', offset);
		const bool physical = bool(manager.getFileEntryRefForID(file));
		const auto text = buffer.slice(start, end).trim();
		// Clang's predefines push these values before source preprocessing.
		// A later matching source pragma still requires policy equivalence.
		// Physical source pragmas never receive this exemption.
		const bool infrastructure = !physical &&
			llvm::StringRef(manager.getPresumedLoc(location).getFilename()) == "<built-in>" &&
			(text == "#pragma push_macro(\"__GCC_DESTRUCTIVE_SIZE\")" ||
				text == "#pragma push_macro(\"__GCC_CONSTRUCTIVE_SIZE\")");
		pragmas |= !infrastructure;
		pragmaEvents.push_back(
			llvm::json::Object{ { "physical_file", physical }, { "infrastructure_only", infrastructure },
				{ "file", manager.getPresumedLoc(location).getFilename() }, { "text", text } });
	}
	void FileChanged(SourceLocation location, FileChangeReason, SrcMgr::CharacteristicKind, FileID) override
	{
		if (location.isValid())
			if (auto file = manager.getFileEntryRefForID(manager.getFileID(manager.getFileLoc(location))))
				files.insert(file->getName().str());
	}
};

std::string tokenHash(const std::vector<std::string>& tokens)
{
	llvm::SHA256 hash;
	for (const auto& token : tokens)
	{
		hash.update(std::to_string(token.size()));
		hash.update(":");
		hash.update(token);
	}
	const auto result = hash.final();
	static const char digits[] = "0123456789abcdef";
	std::string text;
	for (unsigned char byte : result)
	{
		text.push_back(digits[byte >> 4]);
		text.push_back(digits[byte & 15]);
	}
	return text;
}

bool streamTokens(
	llvm::StringRef input, const LangOptions& language, SourceManager& manager, std::vector<std::string>& tokens)
{
	// Decode only the source-spelling portion of the documented PA4 records.
	// Clang's raw lexer handles strings, raw-string newlines and UDL suffixes.
	// Literal value/type metadata is retained in the original stream, not used
	// to decide branch/token equivalence or language correctness.
	if (!input.consume_front("preproc 1\n") || !input.starts_with("sof "))
		return false;
	const auto first = input.find('\n');
	if (first == llvm::StringRef::npos)
		return false;
	input = input.drop_front(first + 1);
	const auto file = manager.createFileID(llvm::MemoryBuffer::getMemBufferCopy(input, "<cppgm-stream>"));
	const char* begin = input.begin();
	const char* end = input.end();
	const char* position = begin;
	while (position != end)
	{
		llvm::StringRef rest(position, end - position);
		if (rest.consume_front("eof\n"))
			return rest.trim().empty();
		bool literal = false;
		bool identifier = false;
		if (rest.consume_front("simple "))
		{
		}
		else if (rest.consume_front("identifier "))
			identifier = true;
		else if (rest.consume_front("literal ") || rest.consume_front("user-defined-literal "))
			literal = true;
		else
			return false;
		const size_t offset = rest.begin() - begin;
		Lexer lexer(manager.getLocForStartOfFile(file), language, begin, begin + offset, end);
		Token token;
		lexer.LexFromRawLexer(token);
		if (token.is(tok::eof) || token.is(tok::unknown) || token.is(tok::comment))
			return false;
		for (;;)
		{
			tokens.push_back(Lexer::getSpelling(token, manager, language));
			position = begin + manager.getFileOffset(token.getLocation()) + token.getLength();
			if (!literal || !isStringLiteral(token.getKind()))
				break;
			Token next;
			lexer.LexFromRawLexer(next);
			if (!isStringLiteral(next.getKind()))
				break;
			token = next;
		}
		rest = llvm::StringRef(position, end - position);
		const auto newline = rest.find('\n');
		if (newline == llvm::StringRef::npos)
			return false;
		const auto metadata = rest.take_front(newline).trim();
		if (identifier ? !metadata.empty() : metadata.empty())
			return false;
		position += newline + 1;
	}
	return false;
}

} // namespace

int main(int argc, char** argv)
{
	if (argc != 2)
		return blocked("usage: upstream_clang_expectations context.json");
	auto buffer = llvm::MemoryBuffer::getFile(argv[1]);
	if (!buffer)
		return blocked("context cannot be read");
	auto parsed = llvm::json::parse((*buffer)->getBuffer());
	if (!parsed)
	{
		llvm::consumeError(parsed.takeError());
		return blocked("invalid context JSON");
	}
	const auto* context = parsed->getAsObject();
	const auto* input = context ? context->getArray("cc1_args") : nullptr;
	if (!input)
		return blocked("cc1_args array required");
	std::vector<std::string> storage;
	for (const auto& value : *input)
	{
		auto text = value.getAsString();
		if (!text)
			return blocked("non-string cc1 argument");
		storage.push_back(text->str());
	}
	std::vector<const char*> args;
	for (const auto& argument : storage)
		args.push_back(argument.c_str());
	DiagnosticOptions options;
	llvm::IntrusiveRefCntPtr<DiagnosticIDs> ids(new DiagnosticIDs());
	auto* parseDiagnostics = new DiagnosticConsumer();
	DiagnosticsEngine parser(ids, options, parseDiagnostics);
	auto invocation = std::make_shared<CompilerInvocation>();
	if (!CompilerInvocation::CreateFromArgs(*invocation, args, parser) || parseDiagnostics->getNumErrors())
		return blocked("native cc1 context cannot be parsed");
	DiagnosticOptions verifyOptions = invocation->getDiagnosticOpts();
	const bool discover = verifyOptions.VerifyDiagnostics && !verifyOptions.VerifyPrefixes.empty();
	if (invocation->getFrontendOpts().Inputs.size() != 1 ||
		(!discover && !context->getBoolean("allow_positive_without_verify").value_or(false)))
		return blocked("one source and explicit native verify prefixes or positive-host mode required");
	invocation->getDiagnosticOpts().VerifyDiagnostics = false;
	CompilerInstance compiler(invocation);
	auto* preprocessDiagnostics = new DiagnosticConsumer();
	compiler.createDiagnostics(*llvm::vfs::getRealFileSystem(), preprocessDiagnostics);
	compiler.createFileManager();
	compiler.createSourceManager(compiler.getFileManager());
	if (!compiler.createTarget())
		return blocked("native target cannot be created");
	compiler.createPreprocessor(TU_Complete);
	if (!compiler.InitializeSourceManager(compiler.getFrontendOpts().Inputs.front()))
		return blocked("source cannot be initialized");
	SourceManager& manager = compiler.getSourceManager();
	Preprocessor& preprocessor = compiler.getPreprocessor();
	std::set<std::string> files;
	bool pragmas = false;
	llvm::json::Array pragmaEvents;
	preprocessor.addPPCallbacks(std::make_unique<Files>(manager, files, pragmas, pragmaEvents));
	Expectations expected;
	DiagnosticsEngine engine(ids, verifyOptions, &expected, false);
	engine.setSourceManager(&manager);
	engine.setErrorLimit(0);
	std::unique_ptr<VerifyDiagnosticConsumer> verifier;
	std::unique_ptr<Comments> comments;
	if (discover)
	{
		verifier = std::make_unique<VerifyDiagnosticConsumer>(engine);
		engine.setClient(verifier.get(), false);
		verifier->BeginSourceFile(compiler.getLangOpts(), &preprocessor);
		comments = std::make_unique<Comments>(*verifier, engine);
		preprocessor.removeCommentHandler(verifier.get());
		preprocessor.addCommentHandler(comments.get());
	}
	preprocessor.EnterMainSourceFile();
	std::vector<std::string> hostTokens;
	std::set<std::string> policyTokens;
	Token token;
	for (;;)
	{
		preprocessor.Lex(token);
		if (token.is(tok::eof))
			break;
		switch (token.getKind())
		{
		case tok::kw_throw:
			policyTokens.insert("throw");
			break;
		case tok::kw_try:
			policyTokens.insert("try");
			break;
		case tok::kw_catch:
			policyTokens.insert("catch");
			break;
		case tok::kw_typeid:
			policyTokens.insert("typeid");
			break;
		case tok::kw_dynamic_cast:
			policyTokens.insert("dynamic_cast");
			break;
		default:
			break;
		}
		hostTokens.push_back(preprocessor.getSpelling(token));
	}
	if (discover)
	{
		preprocessor.removeCommentHandler(comments.get());
		preprocessor.addCommentHandler(verifier.get());
		verifier->EndSourceFile();
		engine.setClient(nullptr, false);
		verifier.reset();
	}
	if (preprocessDiagnostics->getNumErrors())
		return blocked("native preprocessing failed; no complete expectation projection");
	if (expected.malformed)
	{
		llvm::outs() << llvm::json::Value(llvm::json::Object{ { "status", "harness-blocked" },
							{ "reason", "native verifier rejected expectation metadata" },
							{ "native_diagnostics", std::move(expected.rejected) } })
					 << '\n';
		return 2;
	}
	llvm::json::Array included;
	for (const auto& file : files)
		included.push_back(file);
	llvm::json::Object result{ { "status", "expectations-discovered" }, { "expected_error", expected.errors },
		{ "expected_warning", expected.warnings }, { "host_token_sha256", tokenHash(hostTokens) },
		{ "host_token_count", hostTokens.size() }, { "preprocessed_files", std::move(included) },
		{ "native_ast_analysis", false }, { "candidate_diagnostic_matching", false },
		{ "expectation_source", discover ? "native-active-verifier-directives" : "positive-host-baseline" },
		{ "native_cxx_exceptions", bool(compiler.getLangOpts().CXXExceptions) },
		{ "native_rtti", bool(compiler.getLangOpts().RTTI) } };
	result["active_pragma"] = pragmas;
	result["native_pragmas"] = std::move(pragmaEvents);
	llvm::json::Array policies;
	for (const auto& word : policyTokens)
		policies.push_back(word);
	result["language_policy_tokens"] = std::move(policies);
	if (auto path = context->getString("cppgm_stream"))
	{
		auto stream = llvm::MemoryBuffer::getFile(*path);
		std::vector<std::string> candidateTokens;
		if (!stream || !streamTokens((*stream)->getBuffer(), compiler.getLangOpts(), manager, candidateTokens))
			return blocked("candidate PA4 token stream cannot be decoded");
		result["candidate_token_sha256"] = tokenHash(candidateTokens);
		result["candidate_token_count"] = candidateTokens.size();
		result["same_preprocessed_tokens"] = candidateTokens == hostTokens;
	}
	llvm::outs() << llvm::json::Value(std::move(result)) << '\n';
	return 0;
}
