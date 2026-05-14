# Boost B2 Reducer Inventory, 2026-05-11

This records the temporary reducers collected while driving the Boost B2
frontier. The source files currently live in:

```text
/tmp/cppgm-boost-reducers-20260511/
```

This document is an inventory, not a replacement for the source files. Promote
confirmed reducers into the earliest appropriate PA regression tests before
cleaning the temp directory.

Status meanings:

- `confirmed-positive`: clang accepted the reduced shape and cppgm rejected it,
  or the reducer was captured from a known Boost failure and confirmed during
  the frontier run.
- `confirmed-positive, Boost headers`: still depends on Boost headers and
  should be reduced further if possible before becoming a regression.
- `negative/control or now-passing`: useful control or intermediate reducer;
  artifacts indicate the shape passed after reduction or after later fixes.
- `exploratory/unclassified`: keep for reference, but retest before using it as
  evidence for a compiler bug.

Summary: 742 reducer source files observed in the temp directory. The detailed
status counts were last fully recounted at 144 files and are now updated
opportunistically as rows are added during frontier replay.

| File | Status | Surface |
| --- | --- | --- |
| `adl_using_operator_template_simple.cpp` | confirmed-positive | minimal PA18 regression for ADL finding a function-template operator imported into the associated namespace by a namespace using-declaration |
| `aggregate_array_member_string_init_nostl.cpp` | confirmed-positive | array aggregate member initialized from string literal |
| `boost_container_pmr_construct.cpp` | exploratory/unclassified | Boost.Container PMR construct exploration; Boost-header repro, not minimized to non-STL |
| `boost_container_pmr_construct_suffix.cpp` | exploratory/unclassified | Boost.Container allocator propagation suffix exploration; includes local Boost test helper |
| `boost_container_pmr_construct_suffix_10.cpp` | exploratory/unclassified | Boost.Container allocator propagation suffix arity exploration; includes local Boost test helper |
| `boost_container_pmr_construct_suffix_tag10.cpp` | exploratory/unclassified | Boost.Container allocator propagation suffix tag exploration; includes local Boost test helper |
| `boost_enable_if_member_pointer_return_nostl.cpp` | confirmed-positive, Boost headers | Boost.FunctionTypes `fast_mem_fn_example` enable_if member-function-pointer return path; after member-function-pointer pack deduction was fixed this advanced to the member-function-pointer non-type partial-specialization argument frontier |
| `boost_function_type_synthesis_probe.cpp` | confirmed-positive, Boost headers | Boost.FunctionTypes synthesis probe for `function_type`, `function_pointer`, and `member_function_pointer`; failed through Boost.MPL `size<Seq>` before inherited member-template qualifier lookup was fixed |
| `boost_heap_detail_comparison_template_bool.cpp` | negative/control or now-passing | clang and cppgm artifacts indicate this shape passed after reduction or after fixes |
| `boost_heap_detail_stable_heap_template_bool.cpp` | confirmed-positive, Boost headers | Boost.MPL `template_arity` NTTP evaluation while including stable_heap detail |
| `boost_heap_detail_utils_template_bool.cpp` | negative/control or now-passing | clang and cppgm artifacts indicate this shape passed after reduction or after fixes |
| `boost_heap_include_template_bool_function.cpp` | confirmed-positive, Boost headers | `template<bool>` function after Boost.Heap/Test includes; parser declaration recovery |
| `boost_heap_only_template_bool_function.cpp` | confirmed-positive, Boost headers | `template<bool>` function after Boost.Heap include; parser declaration recovery |
| `boost_interface_concat_transform_size_probe.cpp` | confirmed-positive, Boost headers | Boost.FunctionTypes `interface_example` failed through Boost.MPL `size<concat_view<void, transform_view<vector<void*, int>, param_type<_>>>>::value` before repeated direct-parameter partial-specialization constraints were ordered |
| `boost_mpl_sequence_tag_probe.cpp` | negative/control or now-passing | Boost.MPL `sequence_tag<vector<...>>::type` control; verified the tag path resolved before the inherited `size_impl<Tag>::apply<Seq>` lookup failed |
| `boost_mpl_size_probe.cpp` | confirmed-positive, Boost headers | Boost.MPL `size<vector<int,int,int>>::value` failed because `size_impl<vector_tag>` inherits the `apply<Sequence>` member class template from `O1_size_impl<vector_tag>` |
| `boost_mem_func_ptr_cv2_probe.cpp` | confirmed-positive, Boost headers | Boost.FunctionTypes `member_function_pointer<vector<int, C const volatile &, int>>::type` failed through cv-reference partial-specialization ordering before the PA21 selector fix |
| `boost_optional_disjunction_base_probe.cpp` | confirmed-positive, Boost headers | Boost.Optional `has_dedicated_constructor` / Boost.TypeTraits `disjunction` probe where a concrete `conditional<bool(T::value), T, disjunction<U...>>::type` base must complete after a substituted `||` trait expression |
| `boost_test_include_template_bool_function.cpp` | negative/control or now-passing | clang and cppgm artifacts indicate this shape passed after reduction or after fixes |
| `builtin_bswap_cli.cpp` | exploratory/unclassified | CLI smoke for `__builtin_bswap32` |
| `builtin_offsetof.cpp` | confirmed-positive | `__builtin_offsetof` lowering/parsing surface |
| `builtin_popcountg_nostl.cpp` | confirmed-positive | Clang-compatible `__builtin_popcountg` availability, constant evaluation, overload analysis, and lowering surface |
| `char_array_braced_string_literal_nostl.cpp` | confirmed-positive | braced string literal initializer for `const char[]`, from Boost.Regex `const char e[] = { s }` macro expansion |
| `char_traits_specialization_inherited_alias_no_inline.cpp` | confirmed-positive | full specialization inheriting a class-template base alias, then using that alias in an out-of-class static member definition |
| `char_traits_specialization_inherited_alias_out_of_class.cpp` | confirmed-positive | libc++ `char_traits<char16_t>`-style inherited alias in an inline-namespace shape; failed before inherited template-parameter bindings stopped shadowing real aliases |
| `child_c_constref_min.cpp` | confirmed-positive | no-STL Boost.Proto `child_c<Expr const &, 1>` reducer where transformed partial-specialization arguments must resolve both reference/cv type text and a fixed non-type pattern |
| `child_c_ref_min.cpp` | negative/control or now-passing | lvalue-reference control for the Boost.Proto `child_c<Expr &, 1>` partial-specialization ordering set |
| `class_array_new_nostl.cpp` | confirmed-positive | `new[]`/`delete[]` class array construction/destruction |
| `const_prvalue_iterator_next_assign_nostl.cpp` | negative/control or now-passing | control for chained iterator `next`/deref assignment where `begin` returns a const prvalue iterator; did not reproduce the Fusion `cons_iterator` reference-field bug |
| `crtp_postfix_decrement_friend_nostl.cpp` | negative/control or now-passing | clang and cppgm artifacts indicate this shape passed after reduction or after fixes |
| `cv_lvalue_ref_alias_template_param_cv_fallback_nostl.cpp` | confirmed-positive | no-STL Boost.Xpressive `regex_search` reducer where `Range&` and `Range const&` function templates instantiate to equal parameter types through an alias-template argument, requiring source reference-pattern cv preference to select the `const&` overload |
| `cv_reference_partial_specialization_order_control_nostl.cpp` | negative/control or now-passing | same cv-reference partial-specialization set with the most-specific partial declared before the second broad partial; control showing comparison itself already recognized the most-specific candidate |
| `cv_reference_partial_specialization_ordering_nostl.cpp` | confirmed-positive | no-STL reducer for `T const &`, `T volatile &`, and later `T const volatile &` partials where early ambiguity prevented selecting the later most-specific partial |
| `fast_mem_fn_member_object_explicit_nostl.cpp` | confirmed-positive | no-STL Boost.FunctionTypes `fast_mem_fn` reducer where a class partial specialization has a member-function-pointer non-type parameter whose type depends on an earlier template parameter, and the selected member template body calls `(value.*MemberFunction)()` |
| `fast_mem_fn_member_object_maker_nostl.cpp` | confirmed-positive | no-STL Boost.FunctionTypes `FAST_MEM_FN` maker-shape reducer; the member template `make_fast_mem_fn<Callee>` must re-resolve the textual member-function pointer argument before selecting and instantiating the `fast_mem_fn<MFPT, Callee, 1>` partial |
| `date_time_operator_chain_period_nostl.cpp` | exploratory/unclassified | DateTime operator-chain reduction attempt |
| `default_nttp_dependent_trait_value_nostl.cpp` | negative/control or now-passing | clang and cppgm artifacts indicate this shape passed after reduction or after fixes |
| `default_nttp_qualified_function_lookup.cpp` | confirmed-positive | no-STL Boost.Proto reducer where a default non-type template argument evaluates `sizeof(proto::detail::default_test((domain_<D0>*)0, ...))` and qualified namespace function lookup must use the default argument's declaration-time location instead of a synthetic parsed call node |
| `default_nttp_sfinae_trait_value_nostl.cpp` | negative/control or now-passing | clang and cppgm artifacts indicate this shape passed after reduction or after fixes |
| `dependent_alignas_value_nostl.cpp` | exploratory/unclassified | dependent `alignas` value exploration |
| `dependent_conversion_operator_noexcept_nostl.cpp` | negative/control or now-passing | clang and cppgm artifacts indicate this shape passed after reduction or after fixes |
| `dependent_nested_impl_result_type_probe.cpp` | confirmed-positive | no-STL Boost.Xpressive `as_matcher::impl` reducer where current-class member-type lookup during reference-member collection must fall through to base typedefs such as `impl::data` |
| `dependent_nested_static_value_nttp_nostl.cpp` | exploratory/unclassified | dependent nested static value NTTP exploration |
| `dependent_not_bool_nttp_nostl.cpp` | negative/control or now-passing | clang and cppgm artifacts indicate this shape passed after reduction or after fixes |
| `dependent_nttp_sum_integral_constant_nostl.cpp` | negative/control or now-passing | clang and cppgm artifacts indicate this shape passed after reduction or after fixes |
| `dependent_size_t_braced_nttp.cpp` | exploratory/unclassified | STL/`<cstddef>` version of dependent `size_t` NTTP reducer |
| `dependent_size_t_braced_nttp_nostl.cpp` | confirmed-positive | dependent `size_t` non-type template argument from braced/array-size shape |
| `empty_compile_cli.cpp` | exploratory/unclassified | empty compiler smoke/control |
| `enable_if_negated_or_overload_min_nostl.cpp` | confirmed-positive | negated trait expression in `enable_if` overload ordering |
| `enable_if_negated_trait_min_nostl.cpp` | exploratory/unclassified | intermediate `enable_if` negated trait reducer |
| `enable_if_negated_trait_overload.cpp` | confirmed-positive | STL version of Boost.Bind/member-function `enable_if` negated `is_same || is_base_of` overload reducer |
| `enable_if_negated_trait_overload_min_nostl.cpp` | exploratory/unclassified | intermediate no-STL `enable_if` negated overload reducer |
| `enable_if_negated_trait_overload_nostl.cpp` | confirmed-positive | no-STL Boost.Bind/member-function `enable_if` negated trait overload reducer |
| `enum_class_native_macro_nttp_nostl.cpp` | negative/control or now-passing | clang and cppgm artifacts indicate this shape passed after reduction or after fixes |
| `enum_sizeof_call_nostl.cpp` | negative/control or now-passing | clang and cppgm artifacts indicate this shape passed after reduction or after fixes |
| `enum_sizeof_template_call_nostl.cpp` | negative/control or now-passing | clang and cppgm artifacts indicate this shape passed after reduction or after fixes |
| `explicit_function_template_id_call_nostl.cpp` | exploratory/unclassified | qualified/explicit function template-id call exploration |
| `functional_bind2nd_ostream_memfun_reference_member.cpp` | confirmed-positive | Boost.Functional `bind2nd(mem_fun(...), std::cout)` shape; crashed before indirect member-pointer calls passed hidden virtual-base arguments for `std::ostream&` |
| `functional_bind2nd_ostream_reference_member.cpp` | negative/control or now-passing | `std::ostream&` binder/reference-member control without the indirect pointer-to-member call |
| `functional_bind2nd_reference_member_global_nostl.cpp` | negative/control or now-passing | no-STL global reference-member binder control; did not require hidden virtual-base reference parameters |
| `functional_bind2nd_reference_member_nostl.cpp` | negative/control or now-passing | no-STL binder/reference-member control; did not require hidden virtual-base reference parameters |
| `functional_bind2nd_virtual_reference_member_nostl.cpp` | negative/control or now-passing | no-STL virtual-method/reference-member control; showed ordinary virtual calls were not the Boost.Functional crash surface |
| `function_type_synthesis_reduce.cpp` | negative/control or now-passing | direct function-type synthesis control; simple dependent typedef/function-pointer shape passed without the Boost.MPL `size<>` qualifier path |
| `function_type_nttp_helper_nostl.cpp` | confirmed-positive | function type decomposition used as NTTP/helper trait |
| `forward_class_static_member_match_nostl.cpp` | confirmed-positive | no-STL Boost.Xpressive `this->Matcher::match(...)` reducer with a forward-declared matcher; qualified syntax must preserve preselected member-template candidates |
| `forward_primary_partial_switch_value_nostl.cpp` | confirmed-positive | no-STL Boost.Proto/Xpressive reducer where a forward-declared primary class template still needs partial-specialization selection in dependent recursive grammar matching |
| `forward_template_reference_member_alias_nostl.cpp` | confirmed-positive | no-STL Boost.Proto reducer where a class template specialization is named while the primary is only forward-declared, then later member collection must refresh the reference to the completed template definition before resolving `typename Grammar::proto_grammar` |
| `fusion_copy_equal_to_probe.cpp` | negative/control or now-passing | Boost.Fusion-header control showing `result_of::equal_to` recursion termination chose the expected true/false values while reducing `copy` runtime failure |
| `fusion_copy_recursive_reference_iterator_nostl.cpp` | negative/control or now-passing | no-STL recursive copy control with reference-member iterators and nested `next`; passed before the final same-template-name reducer isolated the failing surface |
| `fusion_copy_reference_iterator_nostl.cpp` | negative/control or now-passing | no-STL direct copy control with reference-member iterators; passed while reducing the Fusion list assignment failure |
| `fusion_copy_runtime_probe.cpp` | confirmed-positive, Boost headers | Boost.Fusion `copy(v, l)` bitmask probe showing the first list element copied but second/third element writes failed before the reference-member same-template-name fix |
| `fusion_list_at_c_probe.cpp` | negative/control or now-passing | Boost.Fusion-header control showing constructed `list<int, short, double>` storage and `at_c` reads were correct |
| `fusion_list_deref_assign_probe.cpp` | confirmed-positive, Boost headers | Boost.Fusion-header reducer where assigning through `*next(begin(list))` and `*next(next(begin(list)))` failed because `cons_iterator<cons<...>>::cons` skipped reference-field loading |
| `fusion_tag_of_incomplete_typedef_nostl.cpp` | exploratory/unclassified | Fusion `tag_of` incomplete typedef exploration |
| `fusion_sibling_namespace_partial_min.cpp` | confirmed-positive | minimal no-STL Boost.Fusion shape where a partial specialization in `outer::impl` names sibling namespace `detail::seq<I...>` in its template-id pattern |
| `fusion_sequence_operator_equal_direct.cpp` | negative/control or now-passing | control showing the Fusion `operator==` function template is viable when declared directly in the associated namespace |
| `fusion_sequence_operator_equal_explicit.cpp` | negative/control or now-passing | control showing an explicit `boost::fusion::operators::operator==` call resolves the nested operator template |
| `fusion_sequence_operator_equal_local_using.cpp` | negative/control or now-passing | control showing local ordinary lookup through `using boost::fusion::operator==` finds the imported operator template |
| `fusion_sequence_operator_equal_min.cpp` | confirmed-positive | Boost.Fusion-shaped reducer where ADL must find `operator==` imported from `boost::fusion::operators` into `boost::fusion` by a namespace using-declaration |
| `fusion_sequence_operator_equal_qualified_using.cpp` | negative/control or now-passing | control showing qualified `boost::fusion::operator==` lookup sees the namespace using-declaration import |
| `fusion_iterator_base_operator_deref_min.cpp` | negative/control or now-passing | control showing a simple `iterator_base<Iterator>` operator dereference template can deduce through the CRTP base |
| `fusion_iterator_base_operator_deref_result.cpp` | negative/control or now-passing | control adding a simple `result_of::deref<Iterator>::type` return to the Fusion iterator dereference shape |
| `fusion_iterator_deref_tag_dispatch.cpp` | negative/control or now-passing | control adding Fusion-style tag-dispatched deref implementation without the decltype value-at base |
| `fusion_iterator_deref_decltype_value_at.cpp` | confirmed-positive | Boost.Fusion iterator deref reducer where `result_of::deref<Iterator>::type` depends on a decltype call to `value_at_impl<N::value>(declval<Sequence*>())`; failed before rvalue-reference pointer derived-to-base conversion was accepted |
| `fusion_iterator_deref_decltype_value_at_explicit.cpp` | confirmed-positive | explicit-qualified `boost::fusion::operator*` version of the decltype value-at deref reducer, confirming the failure was substitution/conversion rather than ADL |
| `fusion_count_if_sequence_probe.cpp` | confirmed-positive, Boost headers | Boost.Fusion `count_if` probe advanced by the dependent comma-`decltype` SFINAE fix; still open at the next frontier, where the B2 target reaches an unsupported local typedef declaration in `boost::fusion::detail::count_if` |
| `fusion_value_at_decltype_base_min.cpp` | confirmed-positive | minimized decltype base-class form for `value_at_impl<N::value>(declval<Sequence*>())`, useful for reproducing the exact Boost.Fusion unevaluated call path |
| `fusion_value_at_decltype_only.cpp` | exploratory/unclassified | early decltype-only reduction attempt; local/namespace typedef parsing hit unrelated frontend limits before the final base-class reducer was isolated |
| `fusion_mpl_sequence_tag_probe.cpp` | confirmed-positive, Boost headers | Boost.Fusion `is_native_fusion_sequence` probe failed before dependent comma-`decltype` operands in `is_convertible` were preserved through substitution; now advances past the false native-sequence classification |
| `fusion_vector_deref_probe.cpp` | negative/control or now-passing | Boost.Fusion-header control showing source vector `begin`/`next`/deref reads were correct while isolating the list-side copy failure |
| `fusion_vector_crtp_base_only.cpp` | negative/control or now-passing | Fusion vector base-only control while reducing the `vector_data` partial-specialization failure |
| `fusion_vector_make_index_sequence_base.cpp` | negative/control or now-passing | global-namespace Fusion vector control with `make_index_sequence`; did not reproduce the sibling-namespace partial-specialization mismatch |
| `fusion_vector_namespace_partial_match.cpp` | confirmed-positive | Boost.Fusion-shaped `boost::fusion::vector_detail::vector_data<detail::index_sequence<I...>, T...>` reducer; failed before semantic template-name lookup matched the sibling namespace pattern to the actual class-template entity |
| `gnu_variadic_comma_paste_nostl.cpp` | exploratory/unclassified | GNU variadic comma paste preprocessor exploration |
| `guarded_static_reference_initializer_nostl.cpp` | exploratory/unclassified | guarded static reference initializer exploration |
| `hash2_enable_if_sizeof_function_template_nostl.cpp` | negative/control or now-passing | clang and cppgm artifacts indicate this shape passed after reduction or after fixes |
| `inherited_cdr_iterator_next_assign_nostl.cpp` | negative/control or now-passing | no-STL control with inherited `cdr` subobject and reference-member iterators; passed before the same-template-name reference-field bug was isolated |
| `integral_constant_dependent_bool_expr.cpp` | exploratory/unclassified | Boost.Bind `integral_constant` dependent bool expression, STL version |
| `integral_constant_dependent_bool_negated.cpp` | exploratory/unclassified | Boost.Bind `integral_constant` negated dependent bool expression, STL version |
| `intermediate_type_transform_value_nttp_nostl.cpp` | confirmed-positive | no-STL Boost.Proto reducer where `type_transform<T>` is an intermediate qualifier in `type_transform<T>::type::value`, so failed trait-style probing must fall through to qualified-type static value evaluation |
| `is_aggregate_integral_constant_mangle_nostl.cpp` | confirmed-positive | `__is_aggregate` inside `integral_constant`; mangling/lowering surface |
| `is_cv_pointer_boost_components.cpp` | exploratory/unclassified | Boost.FunctionTypes `components<func_c_ptr>` probe; useful for reproducing the cppgm ambiguity, but the local expected numeric bits were only diagnostic |
| `is_cv_pointer_function_pointer_partial.cpp` | confirmed-positive | no-STL reducer for a top-level const function pointer matching both `R (*)()` and `T * const` class partial specializations |
| `is_cv_pointer_function_pointer_partial_select.cpp` | confirmed-positive | no-STL selection reducer proving clang chooses the top-level cv wrapper partial over the unqualified function-pointer partial for `void (* const)()` |
| `is_cv_pointer_function_pointer_partial_static.cpp` | confirmed-positive | minimal static-assert form of the Boost.FunctionTypes `is_cv_pointer` top-level cv function-pointer ambiguity |
| `is_cv_pointer_partial_min.cpp` | negative/control or now-passing | simple `T * const` wrapper over `T *` control that passed before the function-pointer partial was introduced |
| `is_convertible_decltype_unrelated_class_nostl.cpp` | confirmed-positive | minimized PA22 expression-SFINAE reducer for `decltype(test_aux<To1>(declval<From1>()), one())`; failed before comma-`decltype` prefix dependency was preserved, making unrelated classes appear convertible |
| `iter_fold_repeated_parameter_partial_min.cpp` | confirmed-positive | no-STL reducer for `iter_fold_impl<-1, Last, Last, ...>` ordering over the recursive `iter_fold_impl<-1, First, Last, ...>` partial when the first and last iterators are the same |
| `libcpp_char_traits_char16_include.cpp` | confirmed-positive, Boost headers | direct libc++ `__string/char_traits.h` include for `std::char_traits<char16_t>::compare`; failed through the same inherited-alias/member-definition surface as the no-STL reducers |
| `local_template_id_variable_decl_nostl.cpp` | negative/control or now-passing | local variable declaration with template-id type; control for Assign local declaration failures |
| `local_typedef_template_member_type_nostl.cpp` | negative/control or now-passing | clang and cppgm artifacts indicate this shape passed after reduction or after fixes |
| `member_function_cv_equal_min_nostl.cpp` | negative/control or now-passing | direct member-function pointer cv type equality control for the FunctionTypes `mem_func_ptr_cv2` failure |
| `member_class_partial_specialization_out_of_class_min.cpp` | confirmed-positive | no-STL reducer for a member class template partially specialized out of class as `outer::inner<T, Last, Last>` |
| `member_class_template_iter_recursive_apply_nested_parser_min.cpp` | confirmed-positive | no-STL reducer for Boost.FunctionTypes `interpreter::invoker<Function,next_iter_type,To>::apply`, where a selected out-of-class nested partial specialization must keep the `interpreter` owner scope for member-template collection and recursive qualified calls |
| `member_class_template_reference_reset_nostl.cpp` | confirmed-positive | no-STL reducer for an out-of-class nested class-template definition on a non-template owner surviving reference-member to full-member collection reset |
| `member_direct_ostream_reference_call.cpp` | negative/control or now-passing | direct `Person::print(std::ostream&)` control; emitted the hidden virtual-base argument correctly before the indirect member-pointer fix |
| `local_typedef_after_using_declaration_nostl.cpp` | negative/control or now-passing | local typedef after `using boost::array`; control for Assign local typedef failures |
| `local_typedef_using_namespace_template_id_nostl.cpp` | negative/control or now-passing | local typedef after `using namespace boost`; control for Assign local typedef failures |
| `member_function_enable_if_negated_trait.cpp` | confirmed-positive | member-function version of Boost.Bind `enable_if` negated `is_same || is_base_of` trait |
| `member_pointer_ostream_reference_call.cpp` | confirmed-positive | minimal Boost.Functional crash reducer: indirect pointer-to-member call with `std::ostream&` omitted the hidden virtual-base argument before the PA29 lowering fix |
| `mp_defer_alias_template_pack_nostl.cpp` | negative/control or now-passing | clang and cppgm artifacts indicate this shape passed after reduction or after fixes |
| `mpl_assert_call_expression_enum_nostl.cpp` | exploratory/unclassified | MPL assert enum/call-expression exploration |
| `mpl_assert_enum_function_pointer_trait_nostl.cpp` | negative/control or now-passing | MPL assert enum with function-pointer trait argument; control for FunctionTypes enum failures |
| `mpl_assert_enum_sizeof_call_nostl.cpp` | negative/control or now-passing | clang and cppgm artifacts indicate this shape passed after reduction or after fixes |
| `mpl_assert_enum_template_predicate_nostl.cpp` | negative/control or now-passing | clang and cppgm artifacts indicate this shape passed after reduction or after fixes |
| `mpl_assert_sizeof_call_enum_nostl.cpp` | exploratory/unclassified | MPL `sizeof`/assert enum exploration |
| `mpl_size_base_reduce.cpp` | negative/control or now-passing | control for direct `size_impl<Tag>::apply<Seq>` member template declared in the selected specialization |
| `mpl_size_dependent_base_reduce.cpp` | negative/control or now-passing | control for a direct member template deriving from `Vector::size` |
| `mpl_size_direct_apply_direct_reduce.cpp` | negative/control or now-passing | direct `size_impl<Tag>::apply<Seq>::type` control proving the failure required inherited member-template lookup |
| `mpl_size_inherited_apply_direct_reduce.cpp` | confirmed-positive | minimal non-STL reducer for `typename size_impl<typename sequence_tag<vector>::type>::template apply<vector>::type` where `apply` is inherited from a concrete base specialization |
| `mpl_size_inherited_apply_reduce.cpp` | confirmed-positive | same inherited `apply<Sequence>` shape through an `msvc_eti_base<...>::type` base, matching Boost.MPL `size<>` more closely |
| `mpl_size_inherited_vector_size_reduce.cpp` | negative/control or now-passing | control showing inherited `Vector::size` itself resolved before the inherited `apply` qualifier was introduced |
| `mpl_size_sequence_tag_base_reduce.cpp` | negative/control or now-passing | control for `sequence_tag<Sequence>::type` inherited from a base |
| `mpl_size_vector_wrapper_reduce.cpp` | negative/control or now-passing | control for the Boost.MPL vector wrapper/tag shape without inherited member-template lookup |
| `multiple_inheritance_final_overrider_nostl.cpp` | confirmed-positive | ambiguous final overrider reported for valid multiple inheritance override shape |
| `namespace_alias_using_directive_nostl.cpp` | confirmed-positive | namespace alias target found through a using-directive, from Boost.Xpressive `namespace tag = proto::tag` |
| `nonprimary_embedded_class_inline_var.cpp` | confirmed-positive | no-STL Boost.Xpressive `sequence_stack` reducer where native compile's nonprimary static-storage probe must not collect an included header's embedded namespace-scope class definition before normal declaration collection |
| `namespace_scope_member_type_typedef_nostl.cpp` | exploratory/unclassified | namespace-scope member type typedef exploration |
| `namespaced_disjunction_conditional_or_identity_short.cpp` | confirmed-positive | shrunken no-STL Boost.Optional/TypeTraits reducer where both operands of a substituted `||` expression use dependent-looking member-type syntax before feeding `conditional<bool(T::value), T, disjunction<U...>>::type` |
| `nested_braced_array_init_nostl.cpp` | exploratory/unclassified | nested braced array init control/exploration |
| `nested_aggregate_reference_member_copy.cpp` | confirmed-positive | no-STL reducer for aggregate appertainment preferring direct copy-initialization of a nested aggregate whose members are references before brace-eliding following clauses |
| `nested_out_of_class_conversion_operator_nostl.cpp` | confirmed-positive | out-of-class conversion operator declared in nested namespace |
| `nested_type_static_member_nttp_base_nostl.cpp` | negative/control or now-passing | clang and cppgm artifacts indicate this shape passed after reduction or after fixes |
| `nondependent_template_member_type_base_nostl.cpp` | exploratory/unclassified | nondependent template member type base exploration |
| `operator_chain_min_nostl.cpp` | exploratory/unclassified | DateTime/operator-chain minimal attempt |
| `operator_template_single_arg_min_nostl.cpp` | exploratory/unclassified | operator template single-arg minimal attempt |
| `operator_template_single_arg_without_chain_trait_nostl.cpp` | exploratory/unclassified | operator template single-arg without chain trait attempt |
| `out_of_class_conversion_operator_nostl.cpp` | exploratory/unclassified | simple out-of-class conversion operator control for nested version |
| `out_of_class_template_constructor_nostl.cpp` | negative/control or now-passing | clang and cppgm artifacts indicate this shape passed after reduction or after fixes |
| `partial_specialization_default_same_arg_nostl.cpp` | exploratory/unclassified | partial specialization default/same-arg exploration |
| `partial_specialization_default_function_type_no_eager.cpp` | confirmed-positive | no-STL Boost.Xpressive/Boost.Proto reducer where a class partial specialization omits a defaulted function-type template argument and must not instantiate the default return type while collecting the partial |
| `partial_specialization_dependent_base_param_nostl.cpp` | exploratory/unclassified | partial specialization dependent base parameter exploration |
| `partial_specialization_dependent_class_base_param_nostl.cpp` | exploratory/unclassified | partial specialization dependent class base parameter exploration |
| `partial_specialization_dependent_default_param_nostl.cpp` | exploratory/unclassified | partial specialization dependent default parameter exploration |
| `partial_specialization_nested_default_param_nostl.cpp` | exploratory/unclassified | partial specialization nested default parameter exploration |
| `partial_specialization_nested_dependent_base_param_nostl.cpp` | exploratory/unclassified | partial specialization nested dependent base parameter exploration |
| `partial_specialization_nested_false_default_nostl.cpp` | exploratory/unclassified | partial specialization nested false/default exploration |
| `partial_specialization_nested_false_global_box_nostl.cpp` | exploratory/unclassified | partial specialization nested false global-box exploration |
| `partial_specialization_nested_false_min_nostl.cpp` | exploratory/unclassified | partial specialization nested false minimal exploration |
| `partial_specialization_nested_false_nontemplate_box_nostl.cpp` | exploratory/unclassified | partial specialization nested false non-template-box exploration |
| `partial_specialization_nested_false_object_nostl.cpp` | confirmed-positive | partial specialization with nested `false_t`/default argument |
| `partial_specialization_nested_type_default_nostl.cpp` | exploratory/unclassified | partial specialization nested type/default exploration |
| `partial_specialization_same_arg_extra_param_nostl.cpp` | exploratory/unclassified | partial specialization same-arg extra-param exploration |
| `partial_specialization_same_arg_min_nostl.cpp` | exploratory/unclassified | partial specialization same-arg minimal exploration |
| `partial_order_cv_concrete_probe.cpp` | negative/control or now-passing | explicit-specialization control for cv-qualified concrete type ordering |
| `partial_order_cv_probe.cpp` | exploratory/unclassified | combined clang/cppgm probe for function-pointer, member-function-pointer, and plain pointer partial ordering with top-level cv |
| `partial_order_cv_probe_yz.cpp` | negative/control or now-passing | control showing existing ordering already handled cv-qualified function-pointer and unqualified function-pointer-vs-pointer cases |
| `partial_order_cv_subset_probe.cpp` | negative/control or now-passing | control for exact `const volatile` top-level cv partial selection when all cv wrapper partials exist |
| `partial_order_cv_subset_viability.cpp` | negative/control or now-passing | clang/cppgm control showing `T * const` alone does not match `T * const volatile` |
| `partial_order_top_vs_pointee_cv.cpp` | negative/control or now-passing | control where the exact `T const * const` partial beats separate top-level and pointee cv partials |
| `partial_order_top_vs_pointee_cv_no_both.cpp` | confirmed-positive | no-STL reducer where clang chooses the top-level cv wrapper partial over a pointee-cv partial for `int const * const` |
| `prefix_increment_deref_dependent_return_nostl.cpp` | negative/control or now-passing | prefix increment on dereferenced function-template result with dependent return type; control for Boost.Exception failures |
| `prefix_increment_deref_error_info_typedef_nostl.cpp` | negative/control or now-passing | Boost.Exception-like `error_info` typedef plus prefix increment control |
| `prefix_increment_deref_qualified_dependent_return_nostl.cpp` | negative/control or now-passing | qualified function-template dependent return plus prefix increment control |
| `prefix_increment_deref_template_call_nostl.cpp` | negative/control or now-passing | simple `++*get_value<T>()` control for Boost.Exception failures |
| `qualified_declval_builtin_same_partial_nostl.cpp` | exploratory/unclassified | qualified `declval` builtin/same partial-specialization exploration |
| `qualified_declval_dependent_and_min_nostl.cpp` | exploratory/unclassified | qualified `declval` dependent-and exploration |
| `qualified_declval_dependent_true_and_same_partial_nostl.cpp` | exploratory/unclassified | qualified `declval` dependent true/same partial exploration |
| `qualified_declval_partial_specialization_nostl.cpp` | exploratory/unclassified | qualified `declval` partial-specialization exploration |
| `qualified_declval_trait_class_min_nostl.cpp` | confirmed-positive | qualified `declval` in trait/SFINAE expression |
| `qualified_declval_unsigned_and_same_partial_nostl.cpp` | exploratory/unclassified | qualified `declval` unsigned/same partial exploration |
| `qualified_enable_if_integral_constant_nonvariadic_trait_nostl.cpp` | negative/control or now-passing | non-variadic qualified enable_if trait control that passed while isolating the FunctionTypes member-function-pointer pack failure |
| `qualified_enable_if_variadic_bool_constant_trait_nostl.cpp` | confirmed-positive | no-STL Boost.FunctionTypes-style enable_if trait reducer where a variadic partial specialization must bind an empty function-parameter pack in `Ret (C::*)(Args...) const` |
| `qualified_enable_if_variadic_member_trait_nostl.cpp` | confirmed-positive | minimal no-STL reducer for `enable_if<is_member_function_pointer<MFPT>::value, maker<MFPT>>::type` retaining a dependent return type until member-function-pointer pack partial deduction succeeds |
| `qualified_static_member_function_nttp_nostl.cpp` | confirmed-positive | qualified static member function in NTTP/concept requirement surface |
| `random_mt.cpp` | exploratory/unclassified | `std::random` mt19937/uniform distribution Boost.Random-like surface |
| `random_urng.cpp` | exploratory/unclassified | `std::random` URNG/uniform distribution surface |
| `random_urng_inline_namespace_nostl.cpp` | exploratory/unclassified | random URNG trait reduction with inline namespace |
| `random_urng_libcpp_namespace_nostl.cpp` | exploratory/unclassified | random URNG trait reduction in libc++ namespace shape |
| `random_urng_libcpp_traits_nostl.cpp` | exploratory/unclassified | random URNG trait reduction for libc++ type traits |
| `random_urng_namespace_nostl.cpp` | exploratory/unclassified | random URNG trait namespace reduction |
| `random_urng_namespace_unqualified_declval_nostl.cpp` | exploratory/unclassified | random URNG trait plus unqualified `declval` exploration |
| `random_urng_nostl.cpp` | exploratory/unclassified | random URNG trait no-STL baseline |
| `reference_reset_recollects_inclass_template_nostl.cpp` | confirmed-positive | no-STL reducer for ordinary in-class member class templates being recollected during full member collection after a reference-member reset |
| `reference_member_same_template_name_min.cpp` | confirmed-positive | minimized PA18 reducer for a reference data member named `cons` whose substituted type is `cons<...>`; failed before member access stopped rewriting reference fields as base subobjects |
| `rvalue_pointer_derived_template_call_min.cpp` | confirmed-positive | minimized PA19 regression for an rvalue-reference pointer expression passed to an explicit non-type function template, requiring derived-to-base pointer conversion plus added pointee cv |
| `rvalue_pointer_derived_template_call_typedef.cpp` | confirmed-positive | witness-stable PA19 version of the rvalue-reference pointer conversion reducer promoted to `pa19/tests/spec/200-rvalue-pointer-derived-template-call.t` |
| `static_assert_array_template_size_nostl.cpp` | negative/control or now-passing | clang and cppgm artifacts indicate this shape passed after reduction or after fixes |
| `static_assert_boost_array_size.cpp` | confirmed-positive, Boost headers | Boost.Array temporary/member `static_assert` evaluation |
| `static_assert_static_member_size_nostl.cpp` | negative/control or now-passing | clang and cppgm artifacts indicate this shape passed after reduction or after fixes |
| `static_assert_temporary_member_size_nostl.cpp` | negative/control or now-passing | clang and cppgm artifacts indicate this shape passed after reduction or after fixes |
| `static_const_member_address.cpp` | exploratory/unclassified | STL-free static const member address exploration |
| `static_const_member_address_nontemplate.cpp` | exploratory/unclassified | non-template static const member address control |
| `static_constexpr_conditional_address.cpp` | exploratory/unclassified | conditional static constexpr address exploration |
| `static_constexpr_numeric_limits_address.cpp` | exploratory/unclassified | numeric_limits-like static constexpr address exploration |
| `static_constexpr_qualified_template_address.cpp` | exploratory/unclassified | qualified template static constexpr address exploration |
| `static_member_template_dependent_qualified_no_forward.cpp` | confirmed-positive | no-STL Boost.Xpressive qualified static member-template call reducer without the forward-declaration noise from the original reducer |
| `static_member_template_object.cpp` | negative/control or now-passing | direct object-call control for the static member-template call surface |
| `static_member_address_qualified_nostl.cpp` | confirmed-positive | qualified static data member address/ODR-use surface |
| `template_bool_function_nostl.cpp` | negative/control or now-passing | clang and cppgm artifacts indicate this shape passed after reduction or after fixes |
| `template_bool_param_name_after_stable_template_nostl.cpp` | negative/control or now-passing | clang and cppgm artifacts indicate this shape passed after reduction or after fixes |
| `template_bool_three_parameters_function_nostl.cpp` | negative/control or now-passing | clang and cppgm artifacts indicate this shape passed after reduction or after fixes |
| `template_template_arity_incomplete_partial_nostl.cpp` | confirmed-positive | no-STL Boost.Proto `template_arity` reducer where template-template deduction against `F<T>` must treat mismatched arity as deduction failure without completing `tag_of<T>` while a reference partial specialization and incomplete argument are present |
| `token_paste_variadic_multi_token_nostl.cpp` | exploratory/unclassified | token-paste multi-token preprocessor exploration |
| `type_name_pack_expansion_array_init_nostl.cpp` | confirmed-positive | pack expansion in braced array initializer |
| `typeid_type_id_nostl.cpp` | exploratory/unclassified | `typeid(type-id)` parsing/semantic exploration |
| `unqualified_declval_dependent_and_min_nostl.cpp` | exploratory/unclassified | unqualified `declval` dependent-and exploration |
| `unqualified_declval_trait_class_min_nostl.cpp` | exploratory/unclassified | unqualified `declval` trait class exploration |
| `using_adl_function_template_nostl.cpp` | confirmed-positive | using-declaration plus ADL function template lookup |
| `using_inherited_alias_operator_template_nostl.cpp` | confirmed-positive | no-STL Boost.Xpressive/Boost.Proto reducer where `using proto_extends::operator=` resolves `proto_extends` through an inherited type alias while collecting derived class reference members |
| `using_std_min_macro_guard_nostl.cpp` | negative/control or now-passing | clang and cppgm artifacts indicate this shape passed after reduction or after fixes |
| `value_init_class_template_temporary_nostl.cpp` | negative/control or now-passing | clang and cppgm artifacts indicate this shape passed after reduction or after fixes |
| `variadic_dependent_base_partial_nostl.cpp` | exploratory/unclassified | variadic dependent base partial-specialization exploration |
| `variadic_member_trait_enable_if_type_nostl.cpp` | exploratory/unclassified | namespace typedef variant from the member-function-pointer pack deduction investigation; blocked before it became a confirmed reducer |
| `variadic_member_trait_enable_if_type_notypename_nostl.cpp` | exploratory/unclassified | no-`typename` variant from the member-function-pointer pack deduction investigation; blocked before it became a confirmed reducer |
| `variadic_member_trait_value_nostl.cpp` | negative/control or now-passing | direct trait-value control for variadic member-function-pointer partials; did not prove the Boost failure by itself |
| `variadic_member_trait_value_selected_nostl.cpp` | exploratory/unclassified | selected-value variant from the member-function-pointer pack deduction investigation; hit an unrelated strict fallback before confirmation |
| `is_complete_inherited_type_sizeof_nostl.cpp` | confirmed-positive | PA19 reducer for `sizeof(call-expression)` returning a class-template specialization whose layout must be completed before non-type template argument evaluation |
| `mpl_if_is_complete_sizeof_nostl.cpp` | confirmed-positive | larger MPL `if_`/`not_` reducer that depends on the same completed `sizeof(check<T>(0))` result |
| `local_typedef_mpl_nested_type_wknd_nostl.cpp` | confirmed-positive | Boost.Fusion-shaped local typedef reducer that first exposed the `is_complete<T>` `sizeof(call-expression)` layout hole |
| `fusion_mpl_begin_type_probe.cpp` | confirmed-positive, Boost headers | Boost-header probe for `boost::fusion::result_of::begin<mpl::vector_c<...>>::type`; passed after explicit-specialization redeclarations stopped replacing earlier definitions |
| `fusion_begin_impl_apply_after_begin_hpp_probe.cpp` | confirmed-positive, Boost headers | Boost-header probe showing `boost/fusion/sequence/intrinsic/begin.hpp` caused the later primary `begin_impl<Tag>` definition and redeclared `begin_impl<mpl_sequence_tag>` to hide the earlier MPL specialization definition |
| `fusion_begin_impl_mpl_tag_after_begin_hpp_probe.cpp` | confirmed-positive, Boost headers | narrower Boost-header probe for `boost::fusion::extension::begin_impl<boost::fusion::mpl_sequence_tag>::apply<Sequence>::type` after including both MPL adaptation and the generic begin intrinsic |
| `explicit_specialization_before_primary_definition_nostl.cpp` | confirmed-positive | PA19 no-STL reducer for an explicit class-template specialization definition declared before the primary definition, followed by a later specialization forward redeclaration that must not erase the definition |
| `qualified_explicit_specialization_member_class_template_nostl.cpp` | confirmed-positive | PA19 no-STL reducer for `template<> struct access::struct_member<T, 0>` specializing a member class template through a qualified template-id |
| `empty_base_pack_expansion_sizeof_nostl.cpp` | confirmed-positive | PA18 no-STL reducer for an empty base-class pack expansion such as `struct data : store<T>... {}`; failed because a zero-length expansion left the dependent `store<T>` base in `data<>`, making `sizeof(tuple<tuple<>>)` unevaluated |
| `reference_member_forward_xvalue_nostl_probe.cpp` | confirmed-positive | PA16 no-STL reducer for a `const T&` reference member initialized from an xvalue returned by a forwarding helper; failed because reference-member storage binding accepted xvalues only for rvalue-reference members |
| `member_template_move_assignment_does_not_delete_copy_assign_min.cpp` | confirmed-positive | PA18 no-STL reducer for a member `operator=` template instantiated with an rvalue before const-lvalue assignment; failed because the function-template specialization was classified as a real move assignment and deleted the implicit copy assignment |
| `fusion_vector_nested_fixture_full_assign_probe.cpp` | confirmed-positive, Boost headers | Boost.Fusion fixture-shaped probe for the same member-template assignment special-member classification issue; after this fix it advances to the `assign_sequence` array-initializer pack frontier |
| `array_unknown_bound_pack_initializer_trailing_element_smallest.cpp` | confirmed-positive | PA18 no-STL reducer for unknown-bound array deduction from a braced initializer containing a pack expansion plus trailing element, `int values[] = { I..., 0 }`; failed because the pack expansion counted as one raw initializer child |
| `array_unknown_bound_pack_initializer_trailing_element_func_min.cpp` | confirmed-positive, superseded | Earlier no-STL function reducer for the same unknown-bound array pack-expansion issue, using an unsigned non-type pack and `static_cast<int>(I)...` |
| `array_unknown_bound_pack_initializer_trailing_element_min.cpp` | confirmed-positive, superseded | Earlier class-template form of the same unknown-bound array pack-expansion issue |
| `string_literal_overload_array_vs_by_value_nostl.cpp` | confirmed-positive | PA18 no-STL reducer for a string literal choosing `template<class Char> f(Char)` over `template<class Char> f(Char const[])`; failed by instantiating a Boost-like `basic_string<char const *>` path because function-template instantiated parameter lists kept raw array parameter types |
| `basic_string_traits_char_type_static_assert_nostl.cpp` | control/now-passing | Direct `is_same<CharT, typename Traits::char_type>` static-assert control; showed the static assertion itself was not the failure without the overload-selection surface |
| `traits_type_alias_char_type_static_assert_nostl.cpp` | control/now-passing | Alias-through-`traits_type` variant of the same static-assert control; also passed outside the overload-selection surface |
| `array_function_parameter_string_literal_nontemplate.cpp` | control/now-passing | Non-template array-parameter/string-literal call control kept beside the template overload reducer |
| `boost_mpl_if_placeholder_apply_filter_min.cpp` | confirmed-positive, Boost headers | Boost.Fusion `find`/Boost.MPL `if_` probe that segfaulted before guarding recursive static value dependency checks; after the guard it advances to a separate `make_selected().value == 7` member/comparison frontier |
| `mpl_placeholder_apply_filter_if_no_stl.cpp` | control/now-passing | no-STL placeholder/apply-filter reduction attempt for the same Fusion `find` path; clang and cppgm pass it, so it was not promoted as a PA regression |
| `boost_mpl_or_apply_filter_key_of_min.cpp` | confirmed-positive, Boost headers | Boost.Fusion `find`/Boost.MPL `apply1` + `or_` reducer where the final instantiated-template output sweep completed the unrequired placeholder-bearing `key_of<mpl_::arg<1>>` instantiation; no portable no-STL PA regression was promoted because local reductions either did not track the class for output or forced an invalid arity probe under clang |
| `fusion_find_set_boost_min.cpp` | confirmed-positive, Boost headers | Larger Boost.Fusion `set<int, char, double>` `find<char>` reducer for the same unrequired `key_of<mpl_::arg<1>>` output-completion frontier; passes after the output-sweep guard |
| `floating_inc_dec_nostl.cpp` | confirmed-positive | PA12 no-STL reducer for prefix and postfix increment/decrement on a modifiable `double` lvalue; failed before floating scalar inc/dec operands were accepted |
| `member_call_template_hides_inherited_instantiation_nostl.cpp` | confirmed-positive | PA18 no-STL reducer where a direct derived member function template `operator()` must hide an inherited materialized base `operator()<T>` for callable-object overload resolution |
| `fusion_for_each_mutable_increment_boost_min.cpp` | confirmed-positive, Boost headers | Boost.Fusion `for_each` reducer combining floating increment and a derived mutable functor; after both fixes it advances to the separate stream insertion overload-ranking frontier in the full target |
| `user_defined_conversion_second_rank_min.cpp` | confirmed-positive | PA15 no-STL reducer where overload resolution must compare the standard conversion after a shared `operator int()` user-defined conversion, selecting `pick(int)` over `pick(bool)`, `pick(short)`, and `pick(long)` |
| `static_member_function_object_access_pointer_arg_min.cpp` | confirmed-positive | PA15 no-STL reducer for naming a static member function through an object expression, `s.xalloc`, and passing it as a function pointer argument |
| `template_template_head_partial_order_min.cpp` | confirmed-positive | PA21 no-STL reducer from Boost.MPL `lambda` where `lambda<bind1<F, T1>, Tag, int_<2>>` must be ordered before the generic template-template pattern `lambda<F<T1, T2>, Tag, int_<2>>` |
| `function_template_disable_if_const_ref_overload_min.cpp` | confirmed-positive | PA22 no-STL reducer for preserving `T&` versus `T const&` function-template overloads during redeclaration matching when the `T&` overload has a dependent disable-if return type |
| `qualified_forward_boost_lazy_end_nostl.cpp` | confirmed-positive | no-STL Boost.Fusion-shaped reducer for `end(segments(seq))` where the non-const lazy-disable overload must not absorb the const overload before a class-template member body is instantiated |
| `boost_segmented_end_impl_include_order_min.cpp` | confirmed-positive, Boost headers | Boost.Fusion include-order reducer for `detail::segmented_end_impl` seeing only intrinsic forward declarations before `end.hpp` defines the lazy-enable overloads |
| `iostream_shape_pointer_base_adjust_min.cpp` | confirmed-positive | PA29 no-STL reducer for hidden virtual-base argument lowering through a reference-to-pointer parameter in an iostream-shaped virtual-inheritance diamond |
| `unused_static_member_template_return_type_min.cpp` | confirmed-positive | PA18 no-STL reducer for a class-template static member overload whose unused return type would complete an invalid `mpl_iterator<T>`-style dependent class if output reparses the static member instead of using its existing binding |
| `tail_helper_macro_rescan_min.cpp` | confirmed-positive | PA4 no-STL macro reducer for Boost.Fusion-style alternating filler helper macros where a replacement-list tail helper such as `FILLER_1` must not inherit the older helper's unavailable-name set before it consumes the next parenthesized argument |
| `inherited_member_template_bool_value_direct_nostl.cpp` | confirmed-positive | PA19 no-STL reducer for evaluating a boolean non-type template argument through an inherited member class template, `impl<derived_tag>::template apply<int>::type::value` |
| `inherited_is_sequence_lazy_begin_nostl.cpp` | confirmed-positive | no-STL Boost.Fusion-shaped reducer where `traits::is_sequence<T>::value` depends on `is_sequence_impl<assoc_struct_tag>` inheriting an `apply<T>` member class template from `is_sequence_impl<struct_tag>` before a lazy `begin` overload is viable |
| `fusion_assoc_struct_begin_headers_min.cpp` | confirmed-positive, Boost headers | Boost.Fusion adapted-associative-struct reducer for `boost::fusion::begin(p)` after `BOOST_FUSION_ADAPT_ASSOC_STRUCT`; failed before inherited member-template lookup participated in non-type argument evaluation |
| `fusion_as_map_assoc_headers_min.cpp` | confirmed-positive, Boost headers | Boost.Fusion `as_map` associative-struct reducer that reproduced the full `libs/fusion/test//as_map_assoc` semantic frontier after the macro filler paste issue was fixed |
| `builtin_is_convertible_array_to_class_nostl.cpp` | confirmed-positive | PA33 no-STL reducer for builtin conversion traits where a non-reference array source such as `char[6]` must decay to `char const *` and then satisfy a class constructor target |
| `fusion_count_convert_bitmask.cpp` | confirmed-positive, Boost headers | Boost.TypeTraits/std::string bitmask reducer for the Fusion `count` runtime failure; before the fix every literal-array conversion bit failed while pointer-source conversion bits passed |
| `fusion_count_string_min.cpp` | confirmed-positive, Boost headers | Minimal Boost.Fusion runtime reducer for `boost::fusion::count(vector<int, std::string, double>, "hello")`; passes after array-source builtin conversion traits decay correctly |
| `fusion_count_string_convert_probe.cpp` | confirmed-positive, superseded | Early Boost.TypeTraits/std::string probe for the same `char[6]` conversion-trait failure; superseded by the bitmask reducer |
| `fusion_count_string_real_order.cpp` | exploratory/control | Real include-order and all-count-cases probe used while isolating the `count` failure from the B2 target |
| `fusion_count_original_trim.cpp` | exploratory/control | Source-shaped copy of Boost.Fusion `algorithm/count.cpp` used to verify the wrapper had to be invoked with `-c` for cppgm instead of host clang |
| `class_conversion_pointer_equality_nostl.cpp` | confirmed-positive | PA15 no-STL reducer for same-class operands with `operator char const*()` using builtin pointer equality after overload resolution finds no viable user-defined `operator==`; reproduced the Boost.Fusion `filter_if` comparison failure |
| `qualified_template_call_global_same_name_nostl.cpp` | confirmed-positive | PA18 no-STL reducer for a function template with return type `result_of::remove<Seq const, T>::type` called as `fusion::remove<X>(value)` while an unrelated ordinary `::remove` exists; failed because qualified type lookup collapsed the dependent `result_of::remove<...>::type` owner into opaque text instead of preserving structured owner/member metadata |
| `fusion_remove_result_of_same_name_nostl.cpp` | control/now-passing | Same dependent return and namespace-alias call shape without the unrelated global `remove`; passed before the PA18 qualified-member metadata fix and showed ordinary structured dependent class-template owner substitution already worked |
| `qualified_template_call_global_same_name_int_return_nostl.cpp` | control/now-passing | Same qualified explicit function-template call with a plain `int` return type; isolated the failure to the dependent qualified return type rather than the call lookup |
| `qualified_nested_template_type_global_function_same_name_nostl.cpp` | control/now-passing | Direct `typedef boost::fusion::result_of::remove<int const, X>::type` control with an unrelated global `remove`; showed concrete qualified nested template type lookup already worked |
| `function_template_return_qualified_template_global_same_name_nondep_nostl.cpp` | control/now-passing | Function-template return type using a qualified result template with non-dependent template arguments; showed the failure required substituting the function-template parameters in the dependent owner |
| `qualified_explicit_function_template_type_arg_nostl.cpp` | control/now-passing | Baseline qualified explicit function-template type-argument call control for `fusion::remove<X>(value)` without dependent return-type owner substitution |
| `namespace_alias_explicit_function_template_type_arg_nostl.cpp` | control/now-passing | Namespace-alias variant of the explicit function-template type-argument call control |
| `qualified_explicit_template_call_inside_shift_nostl.cpp` | control/now-passing | Qualified explicit template call nested inside a stream-shift expression; ruled out the surrounding `operator<<` syntax as the cause of the Boost.Fusion `remove` failure |
| `local_typedef_template_id_decl_spec_nostl.cpp` | control/now-passing | Simple local `typedef n::vector<int, char> vector_type` control; showed ordinary template-id decl-specifier parsing was not the Boost.Fusion `remove` frontier |
| `local_using_template_typedef_nostl.cpp` | control/now-passing | Local `using boost::mpl::vector; typedef vector<int, char> mpl_vec` control without a competing using-directive import |
| `local_using_variadic_template_typedef_nostl.cpp` | control/now-passing | Five-argument local using-declaration typedef control matching the MPL arity without the competing Fusion `vector` import |
| `local_using_defaulted_template_typedef_nostl.cpp` | control/now-passing | Defaulted-template-parameter local using-declaration typedef control; passed before the lookup fix |
| `mpl_vector_using_local_typedef_header.cpp` | control/now-passing, Boost headers | `boost::mpl::vector` local using-declaration typedef with only MPL headers; passed before the lookup fix |
| `mpl_vector_using_local_typedef_fusion_adapted.cpp` | control/now-passing, Boost headers | MPL vector typedef with Fusion MPL adaptation included but no competing `using namespace boost::fusion`; passed before the lookup fix |
| `mpl_vector_using_local_typedef_fusion_vector.cpp` | control/now-passing, Boost headers | MPL vector typedef with Fusion vector headers but no competing Fusion using-directive import; passed before the lookup fix |
| `mpl_vector_using_local_typedef_fusion_remove_header.cpp` | control/now-passing, Boost headers | MPL vector typedef with Fusion remove headers but no competing Fusion using-directive import; passed before the lookup fix |
| `fusion_remove_mpl_vector_typedef_no_fusion_using.cpp` | control/now-passing, Boost headers | Full Fusion remove/MPL include set without `using namespace boost::fusion`; passed before the lookup fix and isolated the failure to same-block using-directive interaction |
| `fusion_remove_mpl_vector_typedef_header.cpp` | confirmed-positive, Boost headers | Boost.Fusion `remove`-shaped header reducer with a prior `fusion::vector` object and `using namespace boost::fusion`; failed at `typedef vector<Y,char,long,X,bool> mpl_vec` before block-scope direct using-declarations hid using-directive imports |
| `fusion_remove_mpl_vector_typedef_headers_only.cpp` | confirmed-positive, Boost headers | Minimal Boost-header reducer for the same local `using namespace boost::fusion; using boost::mpl::vector; typedef vector<...>` lookup collision, with no preceding Fusion object/call needed |
| `local_using_decl_over_using_directive_same_template_nostl.cpp` | confirmed-positive | PA18 no-STL reducer where a block-scope `using mpl::vector` must hide the same-name `fusion::vector` made visible by a block-scope using-directive during class-template lookup |
| `nonmember_template_compound_assignment_const_lhs_nostl.cpp` | confirmed-positive | PA18 no-STL reducer for a `const` class lvalue selecting a non-member function-template `operator+=`; failed before non-member compound-assignment overload resolution ran before builtin modifiable-lvalue checks |
| `nonmember_template_compound_assignment_nonconst_lhs_nostl.cpp` | control/now-passing | Non-const variant of the compound-assignment operator-template reducer; passed before the fix and showed the failure was the premature const-lhs builtin assignment rejection |
| `fusion_cons_lambda_expr_only_boost.cpp` | confirmed-positive, Boost headers | Minimal Boost.Lambda expression reducer for `(boost::lambda::_1 += ' ')`; failed before Boost.Lambda's non-member `operator+=` template was considered for the const placeholder object |
| `fusion_cons_lambda_for_each_boost.cpp` | confirmed-positive, Boost headers | Boost.Fusion `cons` reducer for `for_each(ns, boost::lambda::_1 += ' ')`; failed at the same Boost.Lambda compound-assignment argument expression as the full `libs/fusion/test//cons` target |
| `fusion_cons_lambda_for_each_qualified_boost.cpp` | confirmed-positive, Boost headers | Qualified-call control for the Fusion `cons` reducer; showed the frontier was the Boost.Lambda `operator+=` argument expression rather than unqualified `for_each` lookup |
| `fusion_cons_functor_for_each_boost.cpp` | control/now-passing, Boost headers | Same Fusion `cons` sequence with a normal templated functor argument; passed before the fix and isolated the failure to Boost.Lambda's compound-assignment operator template |
| `nontype_braced_size_t_member_value_alias_nostl.cpp` | confirmed-positive | PA19 no-STL reducer for namespace-qualified braced functional casts in non-type template arguments, `std::size_t{N::value}`; failed before template-argument fragment parsing preserved expression syntax for qualified-id `{...}` |
| `nontype_cstyle_size_t_member_value_alias_nostl.cpp` | control/now-passing | C-style cast control for the same dependent `N::value` alias-template path; passed before the qualified braced-cast parser fix and isolated the failure to fragment classification rather than non-type value lookup |
| `alias_integral_constant_value_type_arg_nostl.cpp` | confirmed-positive | PA21 no-STL reducer for substituting a type argument through structured qualified-name expression syntax, `size<L>::value`, inside an alias-template non-type argument; failed before qualified-name syntax was substituted alongside expression text |
| `alias_integral_constant_value_nostl.cpp` | control/now-passing | Simpler alias-template boolean non-type argument control that passed before the structured qualified-name substitution fix |
| `mp11_slice_drop_nostl.cpp` | confirmed-positive, unfixed | no-STL Boost.MP11-shaped slice/drop reducer that still fails after the `size<L>::value` qualified-name fix; it reaches the next frontier around `decltype(f(U*..., identity<W>*...))` and evaluates `(1 <= mp_size<boost::mp11::mp_list<int, char>>::value)` |
| `mp11_slice_braced_size_t_probe.cpp` | confirmed-positive, Boost headers, unfixed | Boost.MP11 `mp_slice` header probe that advances past the earlier `std::size_t{J::value}` text-evaluation diagnostic but still times out in alias-template/partial-specialization resolution |
| `alias_value_type_arg_drop_decltype_nostl.cpp` | confirmed-positive, unfixed | smaller no-STL drop/declaration-type probe that isolates the remaining MP11 slice issue to alias-template instantiation through a `decltype(f(...))` helper shape; clang accepts it while cppgm still reports unsupported alias-template instantiation |
| `xpressive_marker_optional_alt_probe.cpp` | confirmed-positive, Boost headers | Boost.Xpressive marker assignment plus optional/alternate expression probe that reproduces `as_marker::impl::operator()` nested aggregate initialization |
| `xpressive_range_global_char_ptr_regex.cpp` | confirmed-positive, Boost headers | 5-line Boost.Xpressive/Boost.Range reducer for the diagnostic-free mangling crash caused by shallow-copied `TemplateParameterInfo::owned_syntax`; before the deep-copy fix it crashes with `SIGSEGV`, and after the fix it advances to the libc++ `std::__1::__count_bool` alias-declaration frontier |
| `xpressive_regex_search_const_string_probe.cpp` | confirmed-positive, Boost headers | Boost.Xpressive `regex_search(str, what, rex)` probe where `BidiRange&` and `BidiRange const&` range overloads both instantiate to `std::string const&`; after source reference-pattern cv preference is fixed this advances to a separate Boost.Optional `disjunction` base-completion frontier |
| `nested_member_template_argument_mangling.cpp` | confirmed-positive | no-STL reducer for nested member class-template specializations where the default `T&` argument and explicit `T const&` argument must both survive typed owner replay into Itanium object symbols; grouped into PA21 `416-local-qualified-argument-replay.t` |
