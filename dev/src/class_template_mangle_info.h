#pragma once

#include <map>
#include <memory>
#include <string>
#include <vector>

#include "template_model.h"

namespace cpp_decl {

class ClassTemplateMangleArguments
{
public:
  using const_iterator =
      std::vector<template_model::TemplateArgument>::const_iterator;
  using iterator = std::vector<template_model::TemplateArgument>::iterator;

  ClassTemplateMangleArguments()
      : view_(&owned_)
  {
  }

  ClassTemplateMangleArguments(const ClassTemplateMangleArguments & other)
      : owned_(other.values()),
        view_(&owned_)
  {
  }

  ClassTemplateMangleArguments & operator=(
      const ClassTemplateMangleArguments & other)
  {
    if(this != &other) {
      shared_.reset();
      owned_ = other.values();
      view_ = &owned_;
    }
    return *this;
  }

  ClassTemplateMangleArguments & operator=(
      const std::vector<template_model::TemplateArgument> & arguments)
  {
    shared_.reset();
    owned_ = arguments;
    view_ = &owned_;
    return *this;
  }

  operator const std::vector<template_model::TemplateArgument> &() const
  {
    return values();
  }

  bool empty() const
  {
    return values().empty();
  }

  std::size_t size() const
  {
    return values().size();
  }

  const template_model::TemplateArgument & operator[](std::size_t index) const
  {
    return values()[index];
  }

  template_model::TemplateArgument & operator[](std::size_t index)
  {
    materialize();
    return owned_[index];
  }

  const_iterator begin() const
  {
    return values().begin();
  }

  const_iterator end() const
  {
    return values().end();
  }

  iterator begin()
  {
    materialize();
    return owned_.begin();
  }

  iterator end()
  {
    materialize();
    return owned_.end();
  }

  void swap(std::vector<template_model::TemplateArgument> & arguments)
  {
    materialize();
    owned_.swap(arguments);
  }

  std::vector<template_model::TemplateArgument> & mutable_values()
  {
    materialize();
    return owned_;
  }

  const std::vector<template_model::TemplateArgument> & const_values() const
  {
    return values();
  }

  bool shares(
      const std::shared_ptr<std::vector<template_model::TemplateArgument> > &
          arguments) const
  {
    return shared_ && shared_.get() == arguments.get();
  }

  void share(
      const std::shared_ptr<std::vector<template_model::TemplateArgument> > &
          arguments)
  {
    std::vector<template_model::TemplateArgument>().swap(owned_);
    shared_ = arguments;
    view_ = shared_.get();
  }

private:
  const std::vector<template_model::TemplateArgument> & values() const
  {
    return *view_;
  }

  void materialize()
  {
    if(view_ == &owned_) {
      return;
    }
    owned_ = *view_;
    shared_.reset();
    view_ = &owned_;
  }

  std::vector<template_model::TemplateArgument> owned_;
  std::shared_ptr<const std::vector<template_model::TemplateArgument> > shared_;
  const std::vector<template_model::TemplateArgument> * view_;
};

struct ClassTemplateSpecializationMangleInfo
{
  void * class_template_decl = nullptr;
  QualifiedName template_name_syntax;
  std::string template_scope_prefix;
  std::string template_name;
  std::vector<template_model::TemplateParameterInfo> template_parameters;
  std::vector<template_model::TemplateParameterInfo> mangle_parameters;
  std::vector<template_model::TemplateArgument> mangle_arguments;
  ClassTemplateMangleArguments arguments;
  std::vector<TemplateArgumentSyntax> argument_syntaxes;
  std::map<std::string, std::size_t> pack_sizes;
  bool force_structured_mangling = false;
};

inline TypePtr named_mangle_base(const TypePtr & type)
{
  TypePtr base = strip_top_level_cv(type);
  return base && base->kind == Type::TK_NAMED ? base : TypePtr();
}

inline void set_named_type_class_template_specialization_mangle_info(
    const TypePtr & type,
    const std::shared_ptr<ClassTemplateSpecializationMangleInfo> & info)
{
  TypePtr base = named_mangle_base(type);
  if(base) {
    base->mutable_named_rare_metadata()
        .named_class_template_specialization_mangle_info = info;
  }
}

inline std::shared_ptr<ClassTemplateSpecializationMangleInfo>
named_type_class_template_specialization_mangle_info(const TypePtr & type)
{
  TypePtr base = named_mangle_base(type);
  return base ?
      base->named_rare().named_class_template_specialization_mangle_info :
                std::shared_ptr<ClassTemplateSpecializationMangleInfo>();
}

inline std::shared_ptr<const ClassTemplateSpecializationMangleInfo>
named_type_class_template_specialization_mangle_info_const(const TypePtr & type)
{
  return named_type_class_template_specialization_mangle_info(type);
}

}  // namespace cpp_decl
