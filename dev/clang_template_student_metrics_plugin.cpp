#include "clang/AST/ASTConsumer.h"
#include "clang/AST/DeclTemplate.h"
#include "clang/AST/Expr.h"
#include "clang/AST/PrettyPrinter.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/AST/Type.h"
#include "clang/AST/TypeLoc.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/FrontendPluginRegistry.h"
#include "llvm/Support/FormatVariadic.h"
#include "llvm/Support/JSON.h"
#include "llvm/Support/raw_ostream.h"

#include <algorithm>
#include <string>
#include <utility>
#include <vector>

namespace {

using namespace clang;

struct ObservedEvent {
  unsigned offset = 0;
  llvm::json::Object payload;
};

class TemplateStudentMetricsVisitor
    : public RecursiveASTVisitor<TemplateStudentMetricsVisitor> {
public:
  TemplateStudentMetricsVisitor(ASTContext &context, bool main_file_only)
      : context_(context), source_manager_(context.getSourceManager()),
        policy_(context.getLangOpts()), main_file_only_(main_file_only) {}

  bool VisitTemplateSpecializationTypeLoc(TemplateSpecializationTypeLoc type_loc) {
    SourceLocation loc = type_loc.getTemplateNameLoc();
    if (!shouldEmit(loc)) {
      return true;
    }

    const TemplateSpecializationType *type = type_loc.getTypePtr();
    if (type == nullptr) {
      return true;
    }

    if (type->isTypeAlias()) {
      TypeAliasTemplateDecl *alias_template =
          dyn_cast_or_null<TypeAliasTemplateDecl>(
              type->getTemplateName().getAsTemplateDecl());
      if (alias_template == nullptr) {
        return true;
      }

      llvm::json::Object event;
      event["kind"] = "alias_use";
      event["location"] = renderLocation(loc);
      event["template"] = renderNamedDecl(alias_template);
      event["bindings"] = buildBindings(alias_template->getTemplateParameters(),
                                        type->template_arguments(),
                                        type_loc.getNumArgs());
      event["expanded_to"] = renderQualType(type->getAliasedType());
      pushEvent(loc, std::move(event));
      ++alias_use_count_;
      return true;
    }

    CXXRecordDecl *record_decl = type->getAsCXXRecordDecl();
    ClassTemplateSpecializationDecl *specialization =
        dyn_cast_or_null<ClassTemplateSpecializationDecl>(record_decl);
    if (specialization == nullptr) {
      return true;
    }

    llvm::json::Object event;
    event["kind"] = "class_use";
    event["location"] = renderLocation(loc);
    event["template"] = renderNamedDecl(specialization->getSpecializedTemplate());
    event["selection"] = renderClassSelection(*specialization);
    event["selected_decl_location"] = renderSelectedClassLocation(*specialization);
    event["resolved_args"] = renderTemplateArgs(specialization->getTemplateArgs());
    event["bindings"] = buildBindings(
        specialization->getSpecializedTemplate()->getTemplateParameters(),
        specialization->getTemplateArgs().asArray(),
        type_loc.getNumArgs());

    llvm::PointerUnion<ClassTemplateDecl *,
                       ClassTemplatePartialSpecializationDecl *>
        selected = specialization->getSpecializedTemplateOrPartial();
    if (isa<ClassTemplatePartialSpecializationDecl *>(selected)) {
      auto *partial = cast<ClassTemplatePartialSpecializationDecl *>(selected);
      event["specialization_bindings"] = buildBindings(
          partial->getTemplateParameters(),
          specialization->getTemplateInstantiationArgs().asArray(), 0);
    }

    pushEvent(loc, std::move(event));
    ++class_use_count_;
    return true;
  }

  bool VisitDeclRefExpr(DeclRefExpr *expr) {
    if (expr == nullptr || !shouldEmit(expr->getLocation())) {
      return true;
    }

    VarTemplateSpecializationDecl *specialization =
        dyn_cast<VarTemplateSpecializationDecl>(expr->getDecl());
    if (specialization == nullptr) {
      return true;
    }

    llvm::json::Object event;
    event["kind"] = "variable_use";
    event["location"] = renderLocation(expr->getLocation());
    event["template"] = renderNamedDecl(specialization->getSpecializedTemplate());
    event["selection"] = renderVariableSelection(*specialization);
    event["selected_decl_location"] =
        renderSelectedVariableLocation(*specialization);
    event["resolved_args"] = renderTemplateArgs(specialization->getTemplateArgs());
    event["bindings"] = buildBindings(
        specialization->getSpecializedTemplate()->getTemplateParameters(),
        specialization->getTemplateArgs().asArray(),
        expr->hasExplicitTemplateArgs() ? expr->getNumTemplateArgs() : 0);

    llvm::PointerUnion<VarTemplateDecl *, VarTemplatePartialSpecializationDecl *>
        selected = specialization->getSpecializedTemplateOrPartial();
    if (isa<VarTemplatePartialSpecializationDecl *>(selected)) {
      auto *partial = cast<VarTemplatePartialSpecializationDecl *>(selected);
      event["specialization_bindings"] = buildBindings(
          partial->getTemplateParameters(),
          specialization->getTemplateInstantiationArgs().asArray(), 0);
    }

    pushEvent(expr->getLocation(), std::move(event));
    ++variable_use_count_;
    return true;
  }

  bool VisitCallExpr(CallExpr *expr) {
    if (expr == nullptr || !shouldEmit(expr->getExprLoc())) {
      return true;
    }

    FunctionDecl *callee = expr->getDirectCallee();
    if (callee == nullptr) {
      return true;
    }

    FunctionTemplateSpecializationInfo *specialization =
        callee->getTemplateSpecializationInfo();
    if (specialization == nullptr) {
      return true;
    }

    llvm::json::Object event;
    event["kind"] = "function_call";
    event["location"] = renderLocation(expr->getExprLoc());
    event["template"] = renderNamedDecl(specialization->getTemplate());
    event["selected"] = renderNamedDecl(callee);
    event["selected_decl_location"] = renderLocation(callee->getLocation());
    event["selection"] = specialization->isExplicitSpecialization()
                             ? "explicit_specialization"
                             : "instantiation";
    event["bindings"] = buildBindings(
        specialization->getTemplate()->getTemplateParameters(),
        specialization->TemplateArguments->asArray(),
        explicitTemplateArgCount(expr->getCallee()));

    pushEvent(expr->getExprLoc(), std::move(event));
    ++function_call_count_;
    return true;
  }

  void emit(StringRef input_file) {
    std::sort(events_.begin(), events_.end(),
              [](const ObservedEvent &lhs, const ObservedEvent &rhs) {
                if (lhs.offset != rhs.offset) {
                  return lhs.offset < rhs.offset;
                }
                return lhs.payload.size() < rhs.payload.size();
              });

    llvm::json::Array events;
    for (ObservedEvent &event : events_) {
      events.emplace_back(std::move(event.payload));
    }

    llvm::json::Object binding_counts;
    binding_counts["explicit"] = explicit_binding_count_;
    binding_counts["deduced"] = deduced_binding_count_;
    binding_counts["defaulted"] = defaulted_binding_count_;

    llvm::json::Object summary;
    summary["class_uses"] = class_use_count_;
    summary["alias_uses"] = alias_use_count_;
    summary["variable_uses"] = variable_use_count_;
    summary["function_calls"] = function_call_count_;
    summary["binding_counts"] = std::move(binding_counts);

    llvm::json::Object root;
    root["plugin"] = "student-template-metrics";
    root["main_file_only"] = main_file_only_;
    root["input"] = input_file.str();
    root["summary"] = std::move(summary);
    root["events"] = std::move(events);

    llvm::outs() << llvm::formatv("{0:2}\n", llvm::json::Value(std::move(root)));
  }

private:
  ASTContext &context_;
  SourceManager &source_manager_;
  PrintingPolicy policy_;
  bool main_file_only_ = true;
  std::vector<ObservedEvent> events_;

  unsigned class_use_count_ = 0;
  unsigned alias_use_count_ = 0;
  unsigned variable_use_count_ = 0;
  unsigned function_call_count_ = 0;
  unsigned explicit_binding_count_ = 0;
  unsigned deduced_binding_count_ = 0;
  unsigned defaulted_binding_count_ = 0;

  bool shouldEmit(SourceLocation loc) const {
    if (loc.isInvalid()) {
      return false;
    }
    SourceLocation expansion = source_manager_.getExpansionLoc(loc);
    if (expansion.isInvalid()) {
      return false;
    }
    return !main_file_only_ || source_manager_.isWrittenInMainFile(expansion);
  }

  void pushEvent(SourceLocation loc, llvm::json::Object payload) {
    SourceLocation expansion = source_manager_.getExpansionLoc(loc);
    ObservedEvent event;
    event.offset = source_manager_.getFileOffset(expansion);
    event.payload = std::move(payload);
    events_.push_back(std::move(event));
  }

  std::string renderLocation(SourceLocation loc) const {
    PresumedLoc presumed = source_manager_.getPresumedLoc(
        source_manager_.getExpansionLoc(loc));
    if (!presumed.isValid()) {
      return "<invalid>";
    }
    return llvm::formatv("{0}:{1}:{2}", presumed.getFilename(),
                         presumed.getLine(), presumed.getColumn())
        .str();
  }

  std::string renderNamedDecl(const NamedDecl *decl) const {
    if (decl == nullptr) {
      return "<null>";
    }
    if (!decl->getQualifiedNameAsString().empty()) {
      return decl->getQualifiedNameAsString();
    }
    std::string rendered;
    llvm::raw_string_ostream out(rendered);
    decl->printQualifiedName(out, policy_);
    return out.str();
  }

  std::string renderQualType(QualType type) const {
    std::string rendered;
    llvm::raw_string_ostream out(rendered);
    type.print(out, policy_);
    return out.str();
  }

  std::string renderTemplateArg(const TemplateArgument &argument) const {
    std::string rendered;
    llvm::raw_string_ostream out(rendered);
    argument.print(policy_, out, true);
    return out.str();
  }

  llvm::json::Array renderTemplateArgs(const TemplateArgumentList &arguments) {
    return renderTemplateArgs(arguments.asArray());
  }

  llvm::json::Array renderTemplateArgs(ArrayRef<TemplateArgument> arguments) {
    llvm::json::Array values;
    for (const TemplateArgument &argument : arguments) {
      values.emplace_back(renderTemplateArg(argument));
    }
    return values;
  }

  llvm::json::Array buildBindings(const TemplateParameterList *parameters,
                                  ArrayRef<TemplateArgument> arguments,
                                  unsigned explicit_arg_count) {
    llvm::json::Array bindings;
    if (parameters == nullptr) {
      return bindings;
    }

    unsigned limit = std::min<unsigned>(parameters->size(), arguments.size());
    for (unsigned index = 0; index < limit; ++index) {
      const NamedDecl *parameter = parameters->getParam(index);
      const TemplateArgument &argument = arguments[index];
      std::string source = bindingSource(argument, index, explicit_arg_count);
      bindings.emplace_back(bindingObject(parameterName(parameter, index),
                                          renderTemplateArg(argument), source));
      bumpBindingCount(source);
    }
    return bindings;
  }

  std::string parameterName(const NamedDecl *parameter, unsigned index) const {
    if (parameter == nullptr) {
      return llvm::formatv("${0}", index).str();
    }
    std::string name = parameter->getNameAsString();
    if (!name.empty()) {
      return name;
    }
    return llvm::formatv("${0}", index).str();
  }

  std::string bindingSource(const TemplateArgument &argument, unsigned index,
                            unsigned explicit_arg_count) const {
    if (argument.getIsDefaulted()) {
      return "defaulted";
    }
    if (index < explicit_arg_count) {
      return "explicit";
    }
    return "deduced";
  }

  llvm::json::Object bindingObject(std::string parameter, std::string argument,
                                   std::string source) const {
    llvm::json::Object binding;
    binding["param"] = std::move(parameter);
    binding["arg"] = std::move(argument);
    binding["source"] = std::move(source);
    return binding;
  }

  void bumpBindingCount(const std::string &source) {
    if (source == "explicit") {
      ++explicit_binding_count_;
    } else if (source == "defaulted") {
      ++defaulted_binding_count_;
    } else {
      ++deduced_binding_count_;
    }
  }

  std::string renderClassSelection(
      const ClassTemplateSpecializationDecl &specialization) const {
    if (specialization.isExplicitSpecialization()) {
      return "explicit";
    }
    auto selected = specialization.getSpecializedTemplateOrPartial();
    if (isa<ClassTemplatePartialSpecializationDecl *>(selected)) {
      return "partial";
    }
    return "primary";
  }

  std::string renderSelectedClassLocation(
      const ClassTemplateSpecializationDecl &specialization) const {
    if (specialization.isExplicitSpecialization()) {
      return renderLocation(specialization.getLocation());
    }
    auto selected = specialization.getSpecializedTemplateOrPartial();
    if (isa<ClassTemplatePartialSpecializationDecl *>(selected)) {
      return renderLocation(
          cast<ClassTemplatePartialSpecializationDecl *>(selected)->getLocation());
    }
    return renderLocation(cast<ClassTemplateDecl *>(selected)->getLocation());
  }

  std::string renderVariableSelection(
      const VarTemplateSpecializationDecl &specialization) const {
    if (specialization.isExplicitSpecialization()) {
      return "explicit";
    }
    auto selected = specialization.getSpecializedTemplateOrPartial();
    if (isa<VarTemplatePartialSpecializationDecl *>(selected)) {
      return "partial";
    }
    return "primary";
  }

  std::string renderSelectedVariableLocation(
      const VarTemplateSpecializationDecl &specialization) const {
    if (specialization.isExplicitSpecialization()) {
      return renderLocation(specialization.getLocation());
    }
    auto selected = specialization.getSpecializedTemplateOrPartial();
    if (isa<VarTemplatePartialSpecializationDecl *>(selected)) {
      return renderLocation(
          cast<VarTemplatePartialSpecializationDecl *>(selected)->getLocation());
    }
    return renderLocation(cast<VarTemplateDecl *>(selected)->getLocation());
  }

  unsigned explicitTemplateArgCount(Expr *callee) const {
    if (callee == nullptr) {
      return 0;
    }

    Expr *stripped = callee->IgnoreParenImpCasts();
    if (auto *decl_ref = dyn_cast<DeclRefExpr>(stripped)) {
      return decl_ref->hasExplicitTemplateArgs() ? decl_ref->getNumTemplateArgs()
                                                 : 0;
    }
    if (auto *member = dyn_cast<MemberExpr>(stripped)) {
      return member->hasExplicitTemplateArgs() ? member->getNumTemplateArgs()
                                               : 0;
    }
    return 0;
  }
};

class TemplateStudentMetricsConsumer : public ASTConsumer {
public:
  TemplateStudentMetricsConsumer(ASTContext &context, bool main_file_only,
                                 StringRef input_file)
      : visitor_(context, main_file_only), input_file_(input_file.str()) {}

  void HandleTranslationUnit(ASTContext &context) override {
    visitor_.TraverseDecl(context.getTranslationUnitDecl());
    visitor_.emit(input_file_);
  }

private:
  TemplateStudentMetricsVisitor visitor_;
  std::string input_file_;
};

class TemplateStudentMetricsAction : public PluginASTAction {
public:
  std::unique_ptr<ASTConsumer> CreateASTConsumer(CompilerInstance &compiler,
                                                 llvm::StringRef input) override {
    return std::make_unique<TemplateStudentMetricsConsumer>(
        compiler.getASTContext(), main_file_only_, input);
  }

  bool ParseArgs(const CompilerInstance &compiler,
                 const std::vector<std::string> &args) override {
    for (const std::string &arg : args) {
      if (arg == "all-files") {
        main_file_only_ = false;
        continue;
      }
      llvm::errs() << "student-template-metrics: unknown argument '" << arg
                   << "'\n";
      return false;
    }
    return true;
  }

  ActionType getActionType() override { return AddAfterMainAction; }

private:
  bool main_file_only_ = true;
};

} // namespace

static FrontendPluginRegistry::Add<TemplateStudentMetricsAction> X(
    "student-template-metrics",
    "Emit student-facing template observations as JSON");
