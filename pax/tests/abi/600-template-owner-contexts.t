case nested_helper_owner
let-type T template-param 0
let-arg T_arg type T
let-type Alloc template ns::Alloc T_arg
let-arg Alloc_arg type Alloc
let-type Owner template ns::Outer T_arg Alloc_arg
type member Owner _Guard_alloc

case function_local_class_template_arg
let-context make function ns::make
let-type Local local-type make Local 0
let-arg Local_arg type Local
type template ns::Wrap Local_arg

case inline_namespace_basic_string_param
let-type Char template-param 0
let-arg Char_arg type Char
let-type Traits template std::char_traits Char_arg
let-arg Traits_arg type Traits
let-type Alloc template std::allocator Char_arg
let-arg Alloc_arg type Alloc
let-type String template std::__cxx11::basic_string Char_arg Traits_arg Alloc_arg
function path std::getline Char_arg
param ref String
