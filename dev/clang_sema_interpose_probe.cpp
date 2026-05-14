#include "clang/AST/DeclTemplate.h"
#include "clang/AST/Expr.h"
#include "clang/AST/PrettyPrinter.h"
#include "clang/Basic/SourceManager.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Parse/ParseAST.h"
#include "clang/Sema/Overload.h"
#include "clang/Sema/Sema.h"
#include "llvm/ADT/ArrayRef.h"

#include <dlfcn.h>
#include <unistd.h>

#include <cstdlib>
#include <sstream>
#include <string>

namespace {

using namespace clang;

struct InterposeConfig {
  std::string file_filter;
  std::string symbol_filter;
};

const InterposeConfig &config() {
  static const InterposeConfig cfg = []() {
    InterposeConfig out;
    if (const char *value = std::getenv("CLANG_SEMA_INTERPOSE_FILE")) {
      out.file_filter = value;
    }
    if (const char *value = std::getenv("CLANG_SEMA_INTERPOSE_SYMBOL")) {
      out.symbol_filter = value;
    }
    return out;
  }();
  return cfg;
}

bool passesFilter(const std::string &location, const std::string &symbol) {
  const InterposeConfig &cfg = config();
  if (!cfg.file_filter.empty() &&
      location.find(cfg.file_filter) == std::string::npos) {
    return false;
  }
  if (!cfg.symbol_filter.empty() &&
      symbol.find(cfg.symbol_filter) == std::string::npos) {
    return false;
  }
  return true;
}

void emitLine(const std::string &message) {
  std::string line = "[clang.interpose] " + message + "\n";
  (void)!::write(2, line.data(), line.size());
}

__attribute__((constructor)) void interposeProbeLoaded() {
  emitLine("loaded");
}

std::string renderLocation(SourceManager &source_manager, SourceLocation loc) {
  if (loc.isInvalid()) {
    return "<invalid>";
  }
  PresumedLoc presumed =
      source_manager.getPresumedLoc(source_manager.getExpansionLoc(loc));
  if (!presumed.isValid()) {
    return "<invalid>";
  }
  std::ostringstream out;
  out << presumed.getFilename() << ':' << presumed.getLine() << ':'
      << presumed.getColumn();
  return out.str();
}

std::string renderNamedDecl(const NamedDecl *decl) {
  if (decl == nullptr) {
    return "<null>";
  }
  std::string name = decl->getQualifiedNameAsString();
  if (!name.empty()) {
    return name;
  }
  PrintingPolicy policy(decl->getASTContext().getLangOpts());
  std::string rendered;
  llvm::raw_string_ostream out(rendered);
  decl->printQualifiedName(out, policy);
  out.flush();
  return rendered.empty() ? std::string("<anonymous>") : rendered;
}

std::string renderCallSiteName(Expr *callee) {
  if (callee == nullptr) {
    return "<null>";
  }
  Expr *stripped = callee->IgnoreParenImpCasts();
  if (auto *decl_ref = dyn_cast<DeclRefExpr>(stripped)) {
    return renderNamedDecl(decl_ref->getDecl());
  }
  if (auto *member = dyn_cast<MemberExpr>(stripped)) {
    return renderNamedDecl(member->getMemberDecl());
  }
  return stripped->getStmtClassName();
}

template <typename FunctionPointer>
FunctionPointer resolveOriginal(const char *symbol) {
  void *resolved = dlsym(RTLD_NEXT, symbol);
  return reinterpret_cast<FunctionPointer>(resolved);
}

using AddTemplateOverloadCandidateFn = void (*)(
    Sema *, FunctionTemplateDecl *, DeclAccessPair, TemplateArgumentListInfo *,
    llvm::ArrayRef<Expr *>, OverloadCandidateSet &, bool, bool, bool,
    CallExpr::ADLCallKind, OverloadCandidateParamOrder, bool);

using BuildOverloadedCallExprFn =
    ExprResult (*)(Sema *, Scope *, Expr *, UnresolvedLookupExpr *,
                   SourceLocation, llvm::MutableArrayRef<Expr *>,
                   SourceLocation, Expr *, bool, bool);

using BuildOverloadedCallSetFn =
    bool (*)(Sema *, Scope *, Expr *, UnresolvedLookupExpr *,
             llvm::MutableArrayRef<Expr *>, SourceLocation,
             OverloadCandidateSet *, ExprResult *);
using ExecuteCompilerInvocationFn = bool (*)(CompilerInstance *);
using ParseASTFn = void (*)(Sema &, bool, bool);

constexpr const char *kAddTemplateOverloadCandidateSymbol =
    "_ZN5clang4Sema28AddTemplateOverloadCandidateEPNS_20FunctionTemplateDeclENS_14DeclAccessPairEPNS_24TemplateArgumentListInfoEN4llvm8ArrayRefIPNS_4ExprEEERNS_20OverloadCandidateSetEbbbNS_8CallExpr11ADLCallKindENS_27OverloadCandidateParamOrderEb";
constexpr const char *kBuildOverloadedCallExprSymbol =
    "_ZN5clang4Sema23BuildOverloadedCallExprEPNS_5ScopeEPNS_4ExprEPNS_20UnresolvedLookupExprENS_14SourceLocationEN4llvm15MutableArrayRefIS4_EES7_S4_bb";
constexpr const char *kBuildOverloadedCallSetSymbol =
    "_ZN5clang4Sema22buildOverloadedCallSetEPNS_5ScopeEPNS_4ExprEPNS_20UnresolvedLookupExprEN4llvm15MutableArrayRefIS4_EENS_14SourceLocationEPNS_20OverloadCandidateSetEPNS_12ActionResultIS4_Lb1EEE";
constexpr const char *kExecuteCompilerInvocationSymbol =
    "_ZN5clang25ExecuteCompilerInvocationEPNS_16CompilerInstanceE";
constexpr const char *kParseASTSymbol = "_ZN5clang8ParseASTERNS_4SemaEbb";

} // namespace

using namespace clang;

void interposedAddTemplateOverloadCandidate(
    Sema *self, FunctionTemplateDecl *function_template, DeclAccessPair found,
    TemplateArgumentListInfo *explicit_template_args,
    llvm::ArrayRef<Expr *> args, OverloadCandidateSet &candidate_set,
    bool suppress_user_conversions, bool partial_overloading,
    bool allow_explicit, CallExpr::ADLCallKind adl_call_kind,
    OverloadCandidateParamOrder parameter_order,
    bool aggregate_candidate_deduction)
    asm("_ZN5clang4Sema28AddTemplateOverloadCandidateEPNS_20FunctionTemplateDeclENS_14DeclAccessPairEPNS_24TemplateArgumentListInfoEN4llvm8ArrayRefIPNS_4ExprEEERNS_20OverloadCandidateSetEbbbNS_8CallExpr11ADLCallKindENS_27OverloadCandidateParamOrderEb");
void interposedAddTemplateOverloadCandidate(
    Sema *self, FunctionTemplateDecl *function_template, DeclAccessPair found,
    TemplateArgumentListInfo *explicit_template_args,
    llvm::ArrayRef<Expr *> args, OverloadCandidateSet &candidate_set,
    bool suppress_user_conversions, bool partial_overloading,
    bool allow_explicit, CallExpr::ADLCallKind adl_call_kind,
    OverloadCandidateParamOrder parameter_order,
    bool aggregate_candidate_deduction) {
  static AddTemplateOverloadCandidateFn original =
      resolveOriginal<AddTemplateOverloadCandidateFn>(
          kAddTemplateOverloadCandidateSymbol);
  const std::string location =
      renderLocation(self->getSourceManager(), candidate_set.getLocation());
  const std::string symbol = renderNamedDecl(function_template);
  const std::size_t before = candidate_set.size();
  original(self, function_template, found, explicit_template_args, args,
           candidate_set, suppress_user_conversions, partial_overloading,
           allow_explicit, adl_call_kind, parameter_order,
           aggregate_candidate_deduction);
  const std::size_t after = candidate_set.size();
  if (!passesFilter(location, symbol)) {
    return;
  }
  std::ostringstream out;
  out << "add-template-candidate"
      << " location=" << location << " template=" << symbol
      << " explicit_template_args="
      << (explicit_template_args != nullptr ? "yes" : "no")
      << " arg_count=" << args.size() << " before=" << before
      << " after=" << after;
  emitLine(out.str());
}

bool interposedBuildOverloadedCallSet(
    Sema *self, Scope *scope, Expr *fn, UnresolvedLookupExpr *ule,
    llvm::MutableArrayRef<Expr *> args, SourceLocation rparen_loc,
    OverloadCandidateSet *candidate_set, ExprResult *result)
    asm("_ZN5clang4Sema22buildOverloadedCallSetEPNS_5ScopeEPNS_4ExprEPNS_20UnresolvedLookupExprEN4llvm15MutableArrayRefIS4_EENS_14SourceLocationEPNS_20OverloadCandidateSetEPNS_12ActionResultIS4_Lb1EEE");
bool interposedBuildOverloadedCallSet(
    Sema *self, Scope *scope, Expr *fn, UnresolvedLookupExpr *ule,
    llvm::MutableArrayRef<Expr *> args, SourceLocation rparen_loc,
    OverloadCandidateSet *candidate_set, ExprResult *result) {
  static BuildOverloadedCallSetFn original =
      resolveOriginal<BuildOverloadedCallSetFn>(kBuildOverloadedCallSetSymbol);
  const bool had_set = candidate_set != nullptr;
  const std::string location = renderLocation(
      self->getSourceManager(), ule ? ule->getNameLoc() : fn->getExprLoc());
  const std::string symbol = renderCallSiteName(fn);
  const std::size_t before = had_set ? candidate_set->size() : 0;
  const bool handled =
      original(self, scope, fn, ule, args, rparen_loc, candidate_set, result);
  const std::size_t after = had_set ? candidate_set->size() : 0;
  if (!passesFilter(location, symbol)) {
    return handled;
  }
  std::ostringstream out;
  out << "build-overloaded-call-set"
      << " location=" << location << " callee=" << symbol
      << " handled=" << (handled ? "yes" : "no") << " arg_count=" << args.size()
      << " candidates_before=" << before << " candidates_after=" << after;
  emitLine(out.str());
  return handled;
}

ExprResult interposedBuildOverloadedCallExpr(
    Sema *self, Scope *scope, Expr *fn, UnresolvedLookupExpr *ule,
    SourceLocation lparen_loc, llvm::MutableArrayRef<Expr *> args,
    SourceLocation rparen_loc, Expr *exec_config, bool allow_typo_correction,
    bool callees_address_is_taken)
    asm("_ZN5clang4Sema23BuildOverloadedCallExprEPNS_5ScopeEPNS_4ExprEPNS_20UnresolvedLookupExprENS_14SourceLocationEN4llvm15MutableArrayRefIS4_EES7_S4_bb");
ExprResult interposedBuildOverloadedCallExpr(
    Sema *self, Scope *scope, Expr *fn, UnresolvedLookupExpr *ule,
    SourceLocation lparen_loc, llvm::MutableArrayRef<Expr *> args,
    SourceLocation rparen_loc, Expr *exec_config, bool allow_typo_correction,
    bool callees_address_is_taken) {
  static BuildOverloadedCallExprFn original =
      resolveOriginal<BuildOverloadedCallExprFn>(
          kBuildOverloadedCallExprSymbol);
  const std::string location = renderLocation(
      self->getSourceManager(), ule ? ule->getNameLoc() : fn->getExprLoc());
  const std::string symbol = renderCallSiteName(fn);
  ExprResult result =
      original(self, scope, fn, ule, lparen_loc, args, rparen_loc, exec_config,
               allow_typo_correction, callees_address_is_taken);
  if (!passesFilter(location, symbol)) {
    return result;
  }

  std::string selected = "<error>";
  if (result.isUsable()) {
    if (auto *call = dyn_cast<CallExpr>(result.get()->IgnoreImplicit())) {
      if (FunctionDecl *callee = call->getDirectCallee()) {
        selected = renderNamedDecl(callee);
      } else {
        selected = "<non-direct>";
      }
    } else {
      selected = result.get()->getStmtClassName();
    }
  }

  std::ostringstream out;
  out << "build-overloaded-call-expr"
      << " location=" << location << " callee=" << symbol
      << " arg_count=" << args.size() << " result=" << selected;
  emitLine(out.str());
  return result;
}

bool interposedExecuteCompilerInvocation(CompilerInstance *compiler)
    asm("_ZN5clang25ExecuteCompilerInvocationEPNS_16CompilerInstanceE");
bool interposedExecuteCompilerInvocation(CompilerInstance *compiler) {
  static ExecuteCompilerInvocationFn original =
      resolveOriginal<ExecuteCompilerInvocationFn>(
          kExecuteCompilerInvocationSymbol);
  emitLine("execute-compiler-invocation-begin");
  const bool result = original(compiler);
  emitLine(std::string("execute-compiler-invocation-end result=") +
           (result ? "true" : "false"));
  return result;
}

void interposedParseAST(Sema &sema, bool print_stats, bool skip_function_bodies)
    asm("_ZN5clang8ParseASTERNS_4SemaEbb");
void interposedParseAST(Sema &sema, bool print_stats,
                        bool skip_function_bodies) {
  static ParseASTFn original = resolveOriginal<ParseASTFn>(kParseASTSymbol);
  emitLine(std::string("parse-ast-begin") +
           " print_stats=" + (print_stats ? "yes" : "no") +
           " skip_function_bodies=" +
           (skip_function_bodies ? "yes" : "no"));
  original(sema, print_stats, skip_function_bodies);
  emitLine("parse-ast-end");
}
