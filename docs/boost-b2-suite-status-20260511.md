# Boost B2 Suite Status - 2026-05-11

- Boost root: `/Users/vishvananda/boost_1_91_0`
- Compiler worktree: `/Users/vishvananda/cppgm-boost-b2-frontier`
- Command pattern: `JOBS=12 ./run-cppgm-b2.sh -a <suite>`
- Raw logs: `/tmp/boost-suite-survey-20260511-j12`
- Suite inventory: 147 top-level Boost test suites with Jamfiles; `libs/parameter_python/test` is excluded to match the tracked inventory.
- Generated: `2026-05-11T23:48:01`

## Summary

- Completed suites: 147 / 147
- Passing: 54
- Mixed: 67
- Failing: 24
- Setup fail: 2
- Missing/incomplete: 0

## Incremental Replay Notes

- 2026-05-12: `libs/function_types/test` was rerun after the inherited
  qualified member-template fix. B2 updated 81 targets and left 4 compile
  failures: `mem_func_ptr_cv2`, `fast_mem_fn_example`, `interface_example`,
  and `interpreter_example`. The `function_type`, `function_pointer`,
  `function_reference`, `transformation`, `variadic_function_synthesis`,
  `components_seq`, `cc_preprocessing`, `preprocessing_mode`,
  `partial_arity_preprocessing`, and `mem_func_ptr_cv1` targets now pass.
- 2026-05-12: `libs/function_types/test` was rerun after the cv-reference
  partial-specialization selector fix. B2 updated 83 targets and left 3 compile
  failures: `fast_mem_fn_example`, `interface_example`, and
  `interpreter_example`. `mem_func_ptr_cv2` now passes. `interface_example`
  is at a separate Boost.MPL `size<Seq>::value` frontier for
  `concat_view`/`transform_view`; `interpreter_example` still fails while
  collecting the out-of-class partial `interpreter::invoker<Function, To, To>`.
- 2026-05-12: `libs/function_types/test` was rerun after the repeated
  template-argument constraint partial-specialization ordering fix. B2 updated
  85 targets and left 2 compile failures: `fast_mem_fn_example` and
  `interpreter_example`. `interface_example` now passes after the
  Boost.MPL `iter_fold_impl<-1, Last, Last, ...>` termination partial is
  selected over the recursive `First, Last` partial.
- 2026-05-12: `libs/function_types/test//interpreter_example` was rerun after
  out-of-class member class-template partial-specialization support. The target
  advanced past collecting `interpreter::invoker<Function, To, To>` and now
  fails at a separate `boost::bind` qualified lookup/call surface inside
  `interpreter::register_function`. A full `libs/function_types/test` rerun
  still leaves the same 2 compile failures: `fast_mem_fn_example` and
  `interpreter_example`.
- 2026-05-12: `libs/function_types/test//interpreter_example` was rerun after
  preserving nested class-template definitions across non-template class
  reference-member reset. The target advanced past the `boost::bind` address
  lookup and now fails while resolving the recursive
  `interpreter::invoker<Function, next_iter_type, To>::apply` qualified static
  member-template call. `libs/function_types/test` is still expected to have 2
  compile failures until that next frontier and `fast_mem_fn_example` are fixed.
- 2026-05-12: `libs/function_types/test` was rerun after narrowing
  reference-reset class-template preservation and fixing nested partial
  specialization owner scopes. B2 updated 87 targets and left 1 compile
  failure: `fast_mem_fn_example`. `interface_example` is green again after the
  reset preservation no longer keeps ordinary in-class helper templates from
  reference-member collection, and `interpreter_example` now passes after
  `interpreter::invoker<Function, To, To>` is instantiated in the nested
  `interpreter` scope and recursive qualified `apply` calls use that owner.
- 2026-05-12: `libs/function_types/test//fast_mem_fn_example` was rerun after
  member-function-pointer pack partial-specialization deduction was added for
  the `enable_if<is_member_function_pointer<MFPT>, maker<MFPT>>::type` return
  path. The target advanced past the retained dependent `make_fast_mem_fn`
  return and now fails at a separate class member object completion frontier:
  `unsupported class member object name=fnc_criterion` in
  `test_compare<example::fast_mem_fn<int (test::*)() const, ...>,
  std::__1::greater<int>>`. `libs/function_types/test` is still expected to
  have 1 compile failure until that next frontier is fixed.
- 2026-05-12: `libs/function_types/test//fast_mem_fn_example` was rerun after
  preserving member-function-pointer non-type partial-specialization arguments
  and materializing those template-argument member pointers in expression
  analysis. The focused target now passes. A full `libs/function_types/test`
  rerun updated 87 targets and leaves one unexpected compile failure:
  `is_cv_pointer`, which fails with `unsupported enumerator value` while
  analyzing a Boost.MPL assertion expression. The expected negative
  decomposition tests still fail as expected and are reported passing by B2.
- 2026-05-12: `libs/function_types/test` was rerun after top-level cv
  partial-specialization ordering was added for function-pointer and
  member-function-pointer wrapper patterns. B2 updated 89 targets and the
  full suite now passes; the decomposition negative tests still fail as
  expected and are reported passing by B2.
- 2026-05-12: `libs/functional/test` was rerun after indirect member-pointer
  calls learned to infer and pass reference-parameter hidden virtual-base
  arguments from the callable function type. B2 updated `function_test.o`,
  linked `function_test`, and `function_test` now passes. The reducer was
  regressed as PA29 because the underlying bug is non-hosted virtual-base
  hidden-argument lowering; the Boost surface happened to use `std::ostream&`.
- 2026-05-12: `libs/fusion/test//copy` was rerun after semantic
  partial-specialization matching learned to resolve sibling namespace
  template-id patterns such as `detail::index_sequence<I...>` from inside
  `boost::fusion::vector_detail`. The target advanced past the incomplete
  `vector_data` base failure and now fails at the next frontier:
  unsupported `boost::fusion::vector<int, short, double> ==
  boost::fusion::list<int, short, double>` comparison overload resolution.
- 2026-05-12: `libs/fusion/test//copy` was rerun after function-template ADL
  started keeping namespace using-declaration imports, matching the
  `boost::fusion::using operators::operator==` shape. The target advanced past
  the `vector == list` comparison and now fails at a separate iterator deref
  frontier: `unknown function operator*` inside
  `boost::fusion::detail::sequence_copy<...>::call`.
- 2026-05-12: `libs/fusion/test//copy` was rerun after rvalue-reference
  pointer expressions started participating in derived-to-base pointer
  conversions. The target now compiles and links, then fails at runtime in
  `libs/fusion/test/algorithm/copy.cpp:21` with `v == l`, making copy/equality
  runtime behavior the next Fusion frontier.
- 2026-05-12: `libs/fusion/test//copy` was rerun after reference data members
  stopped being rewritten as base-subobject access when the field name matches
  the substituted class-template name, matching `cons_iterator<cons<...>>::cons`.
  The `copy` target now passes.
- 2026-05-12: `libs/fusion/test//count_if` was rerun after dependent
  comma-`decltype` operands started remaining dependent through expression
  SFINAE substitution. The target advanced past the false
  `is_native_fusion_sequence` classification of an MPL vector and now fails at
  the next frontier: unsupported local typedef declaration parsing for
  `typedef result_of::begin<Sequence>::type first;` in
  `boost::fusion::detail::count_if`.
- 2026-05-12: the first reducer layer under that `count_if` local typedef was
  fixed: non-type template argument evaluation now completes
  `sizeof(call-expression)` result class layouts before calling `type_size`.
  The focused no-STL reducers pass, but the Boost target still fails in
  `result_of::begin<Sequence>::type`; the active frontier is now the later
  `begin_impl<mpl_sequence_tag>` explicit-specialization redeclaration after
  the primary template definition.
- 2026-05-12: `libs/fusion/test//count_if` was rerun after explicit
  class-template specialization collection stopped replacing an existing
  definition with a later forward redeclaration. The target now passes.
- 2026-05-12: `libs/fusion/test//deque_nest` and
  `libs/fusion/test//list_nest` were rerun after qualified explicit
  class-template specializations started resolving their qualified primary
  template semantically. Both targets now pass. `libs/fusion/test//vector_nest`
  now reaches a separate operator-resolution frontier in
  `libs/fusion/test/sequence/fixture.hpp`.
- 2026-05-12: `libs/fusion/test//vector_nest` was rerun after empty base-class
  pack expansions started distinguishing zero expansions from unresolved pack
  expansions. It now advances past `boost::is_complete<vector<vector<> > >`
  and the missing Fusion equality overload, then fails at the separate
  reference-member initializer frontier for
  `vector_detail::store<0, vector<> const &>::store`.
- 2026-05-12: `libs/fusion/test//vector_nest` was rerun after allowing
  reference-member storage binding from xvalues when the reference target is
  allowed to bind them. It now advances past
  `vector_detail::store<0, vector<> const &>::store` and fails at the separate
  deleted-assignment frontier in `fixture.hpp` for `expected = source` on
  `vector<vector<> >`.
- 2026-05-12: `libs/fusion/test//vector_nest` was rerun after function-template
  specializations stopped participating in copy/move special-member
  classification. It now advances past the deleted `operator=` frontier and
  fails at the separate `assign_sequence` fallback pack expansion:
  `int nofold[] = { (...), 0 }` reports too many array initializer elements.
- 2026-05-12: `libs/fusion/test//vector_nest` was rerun after unknown-bound
  array deduction started expanding braced initializer pack expansions before
  counting elements. The target now passes.
- 2026-05-12: `libs/fusion/test//find` was rerun after instantiated
  function-template parameter lists started applying the same array/function
  parameter adjustment as function types. It advances past the
  `basic_string<char const *>` delimiter path and now fails at a separate
  qualified `boost::fusion::find<char>` lookup frontier.
- 2026-05-12: `libs/fusion/test//find` was rerun after recursive static value
  dependency checks were guarded. The target advances past the Boost.MPL
  `template_arity`/`if_` recursion that previously segfaulted during the first
  `boost::fusion::find<char>` candidate and now fails later at
  `libs/fusion/test/algorithm/find.cpp:49` for the set case.
- 2026-05-12: `libs/fusion/test//find` was rerun after instantiated-template
  output stopped completing tracked class instantiations that have no required
  output. The target now passes, including the set case that previously forced
  `boost::fusion::result_of::key_of<mpl_::arg<1>>` during Boost.MPL
  `apply1`/`or_` probing.
- 2026-05-12: `libs/fusion/test//for_each` was rerun after floating
  increment/decrement and callable-object member-template hiding were fixed.
  The target advances past both failures and now stops at the separate
  `operator<<` overload-ranking frontier for `mpl_::integral_c<int, 2>`.
- 2026-05-12: `libs/fusion/test//for_each` was rerun after user-defined
  conversion overload ranking started comparing the second standard conversion
  rank. The target advances past `operator<<` on `mpl_::integral_c<int, 2>`
  and now stops in Fusion sequence I/O setup at `ios_base::pword(...)`, where
  `get_xalloc_index<Tag>` is rejected while analyzing `xalloc.value`.
- 2026-05-12: `libs/fusion/test//for_each` was rerun after object-expression
  access to static member functions was allowed to produce a function pointer
  source. The focused target now passes.
- 2026-05-13: `libs/fusion/test//find_if` was rerun after partial
  specialization ordering started preferring a concrete template-id head such
  as `bind1<F, T1>` over a same-arity direct template-template parameter head
  such as `F<T1, T2>`. The focused target now passes.
- 2026-05-13: `libs/fusion/test//segmented_find_if` was rerun after
  function-template redeclaration matching stopped erasing cv nested under
  reference parameters. The focused target now passes; the Boost-shaped
  include-order reducer also passes.
- 2026-05-13: `libs/fusion/test//segmented_fold` was rerun after hidden
  virtual-base helper arguments for reference-to-pointer parameters started
  loading the referenced pointer before computing base offsets. The focused
  target now passes.
- 2026-05-13: `libs/fusion/test//as_map` was rerun after class output started
  indexing static member-function definitions by AST node as well as
  non-static methods. The focused target now passes; the underlying no-STL
  reducer models the unused `convert_iterator<T>::call(T const&, false_tag)`
  overload whose return type should not force `mpl_iterator<T>` completion.
- 2026-05-13: `libs/fusion/test//as_map_assoc` was rerun after macro
  expansion stopped propagating stale unavailable-name state through
  replacement-list tail helper macros. It now advances past the
  `BOOST_FUSION_ADAPT_ASSOC_STRUCT` filler paste failure and stops at a
  separate semantic frontier: `fusion::begin` is not viable for the adapted
  `ns::point` associative-struct path.
- 2026-05-13: `libs/fusion/test//as_map_assoc` was rerun after inherited
  member class-template lookup was added to unqualified template-id resolution
  from class member scopes. The focused target now passes; the promoted PA19
  reducer covers the `impl<derived_tag>::template apply<T>::type::value`
  non-type argument shape that Boost.Fusion uses through
  `traits::is_sequence<T>::value`.
- 2026-05-13: a full `libs/fusion/test` run from the previous head completed
  with 33 failing targets. The first runtime frontier in that run was
  `libs/fusion/test//count`, where `boost::is_convertible<char[6], std::string>`
  was false because `__is_convertible` did not decay non-reference array
  sources. After the PA33 builtin-trait fix, the focused
  `libs/fusion/test//count` target passes. The full Fusion suite should be
  rerun from the new commit to refresh the remaining frontier list.
- 2026-05-13: `libs/fusion/test//filter_if` was rerun after builtin comparison
  conversion probing added a common `void const*` target for class operands
  with object-pointer conversion operators. The focused target now passes. A
  concurrent full `libs/fusion/test` run started before this patch completed
  with 32 failed targets, but it is mixed-state because it overlapped the
  rebuild; use a fresh run from this commit before updating the suite table.
- 2026-05-13: `libs/fusion/test//remove` was rerun after dependent qualified
  member return types preserved structured owner metadata across qualified type
  lookup. The target now advances past `fusion::remove<X>(t)` and stops at the
  next independent frontier: a block-scope `using namespace boost::fusion`
  plus `using boost::mpl::vector` makes the local
  `typedef vector<Y,char,long,X,bool> mpl_vec` look ambiguous to cppgm even
  though the direct block-scope using-declaration should hide the using
  directive import.
- 2026-05-13: `libs/fusion/test//remove` was rerun after non-namespace-scope
  direct declarations started stopping unqualified lookup before same-scope
  using-directive imports. The focused target now passes, including the MPL
  vector local typedef case.
- 2026-05-13: `libs/fusion/test//cons` was rerun after non-member
  compound-assignment operator templates started participating before builtin
  lvalue/modifiability checks. The focused target now passes the
  `for_each(ns, boost::lambda::_1 += ' ')` case; the no-STL PA18 reducer
  covers the const placeholder object selecting a non-member `operator+=`
  template.
- 2026-05-13: the Fusion hash family (`hash`, `list_hash`, `deque_hash`,
  `tuple_hash`, `vector_hash`) was rerun after template-argument fragment
  parsing started treating qualified braced casts such as
  `std::size_t{J::value}` as expression-capable non-type arguments. The old
  `legacy non-type template argument text evaluation` failure is gone, but the
  same five translation units then compiled for more than 15 minutes with
  rising memory in alias-template/partial-specialization resolution. That run
  was stopped; the hash compile-time blowup is the next Fusion frontier.
- 2026-05-13: the next hash-family reduction showed that expression text
  substitution had advanced past `std::size_t{J::value}`, but the structured
  qualified-name syntax for `size<L>::value` still held the original `L`
  qualifier. The PA21 no-STL reducer
  `alias_integral_constant_value_type_arg_nostl.cpp` now passes after
  qualified-name syntax is substituted with the expression text. The remaining
  MP11 slice/drop reducers still fail or time out in the `mp_drop`/`decltype`
  helper path, so the Boost hash family is still blocked at a separate
  alias-template/partial-specialization frontier.

## Suites

| # | Suite | Status | Elapsed | Not-passing / setup detail | Raw log |
|---:|---|---|---:|---|---|
| 1 | `libs/accumulators/test` | passing | 7.0s | B2 completed successfully | `/tmp/boost-suite-survey-20260511-j12/libs__accumulators__test.log` |
| 2 | `libs/algorithm/test` | mixed | 191.0s | framework, is_clamped_test, sort_subrange_test, partition_subrange_test, apply_permutation_test, targets, empty_search_test, search_test1, search_test2, search_test3, search_test4, clamp_test, power_test, all_of_test, any_of_test, none_of_test, one_of_test, ordered_test, find_if_not_test1, copy_if_test1, copy_n_test1, iota_test1, is_permutation_test1, partition_point_test1, is_partitioned_test1, partition_copy_test1, equal_test, mismatch_test, for_each_n_test, reduce_test, mclow, inclusive_scan_test, exclusive_scan_test, transform_reduce_test, transform_exclusive_scan_test, hex_test4, hex_test3, hex_test2, transform_inclusive_scan_test, is_palindrome_test (+5 more) | `/tmp/boost-suite-survey-20260511-j12/libs__algorithm__test.log` |
| 3 | `libs/align/test` | mixed | 15.8s | aligned_alloc_test, aligned_delete_test, assume_aligned_test, aligned_allocator_test, aligned_allocator_incomplete_test, alignment_of_test, targets | `/tmp/boost-suite-survey-20260511-j12/libs__align__test.log` |
| 4 | `libs/any/test` | mixed | 38.9s | basic_any_test, any_test, basic_any_test_rv, any_test_rv, any_test_mplif, basic_any_test_rv_no_rtti, any_test_no_rtti, any_test_rv_no_rtti, basic_any_test_no_rtti, basic_any_test_mplif, unique_const_rvalue_construction_failed, emplace, unique_from_any, base, unique_base, move, from_any, targets, unique_emplace, no_rtti_unique_base, no_rtti_unique_move, no_rtti_unique_emplace, no_rtti_unique_from_any | `/tmp/boost-suite-survey-20260511-j12/libs__any__test.log` |
| 5 | `libs/array/test` | mixed | 54.1s | array6, array7, array_hash, array_typedef_test, array_c_array_test, array_size_test, to_array_test, targets | `/tmp/boost-suite-survey-20260511-j12/libs__array__test.log` |
| 6 | `libs/asio/test` | failing | 247.0s | make_x86_64_sysv_macho_gas, jump_x86_64_sysv_macho_gas, ontop_x86_64_sysv_macho_gas, any_completion_executor, any_completion_executor_select, any_io_executor, any_io_executor_select, any_completion_handler, any_completion_handler_select, append, append_select, as_tuple, as_tuple_select, process_cpu_clocks, chrono, thread_clock, associated_cancellation_slot, associated_executor, associated_cancellation_slot_select, associated_allocator, associated_allocator_select, associated_executor_select, associated_immediate_executor, associated_immediate_executor_select, basic_datagram_socket, basic_datagram_socket_select, basic_deadline_timer, basic_deadline_timer_select, associator, associator_select, awaitable, awaitable_select, basic_raw_socket, basic_raw_socket_select, basic_readable_pipe, basic_readable_pipe_select, basic_file, basic_seq_packet_socket, basic_file_select, basic_seq_packet_socket_select (+260 more) | `/tmp/boost-suite-survey-20260511-j12/libs__asio__test.log` |
| 7 | `libs/assert/test` | passing | 33.1s | 18 passed target(s) | `/tmp/boost-suite-survey-20260511-j12/libs__assert__test.log` |
| 8 | `libs/assign/test` | failing | 134.2s | std, framework, array, list_of, static_list_of, ptr_list_of, list_inserter, tuple_list_of, list_of_workaround, multi_index_container, targets, basic, ptr_list_inserter, ptr_map_inserter, email_example, my_vector_example | `/tmp/boost-suite-survey-20260511-j12/libs__assign__test.log` |
| 9 | `libs/atomic/test` | passing | 4.1s | B2 completed successfully | `/tmp/boost-suite-survey-20260511-j12/libs__atomic__test.log` |
| 10 | `libs/beast/test` | passing | 6.6s | B2 completed successfully | `/tmp/boost-suite-survey-20260511-j12/libs__beast__test.log` |
| 11 | `libs/bimap/test` | mixed | 139.4s | test_tagged, test_structured_pair, test_mutant, test_bimap_list_of, test_bimap_vector_of, test_bimap_unordered_multiset_of, test_bimap_set_of, test_bimap_multiset_of, test_bimap_unordered_set_of, test_bimap_ordered, test_bimap_unordered, test_mutant_relation, test_bimap_sequenced, test_bimap_assign, test_bimap_unconstrained, test_bimap_property_map, test_bimap_mutable, test_bimap_range, test_bimap_modify, test_bimap_operator_bracket, test_bimap_lambda, test_bimap_serialization, test_bimap_extra, test_bimap_convenience_header, basic_iarchive, binary_iarchive, basic_oarchive, test_bimap_project, binary_oarchive, polymorphic_iarchive, basic_text_iprimitive, basic_text_oprimitive, text_iarchive, polymorphic_text_iarchive, polymorphic_binary_iarchive, xml_grammar, text_oarchive, polymorphic_xml_iarchive, xml_iarchive, polymorphic_text_oarchive (+13 more) | `/tmp/boost-suite-survey-20260511-j12/libs__bimap__test.log` |
| 12 | `libs/bind/test` | mixed | 10.2s | quick, bind_test, bind_dm_test, bind_eq_test, bind_const_test, bind_cv_test, bind_stateful_test, bind_dm2_test, bind_not_test, bind_rel_test, bind_function_test, bind_lookup_problem_test, bind_rv_sp_test, bind_unary_addr, bind_dm3_test, bind_visit_test, bind_placeholder_test, bind_rvalue_test, bind_and_or_test, bind_void_test, bind_void_dm_test, mem_fn_test, bind_void_mf_test, mem_fn_void_test, mem_fn_eq_test, mem_fn_derived_test, mem_fn_dm_test, bind_fnobj2_test, mem_fn_rv_test, ref_fn_test, bind_fn2_test, bind_mf2_test, bind_eq2_test, bind_ref_test, mem_fn_ref_test, bind_eq3_test, protect_test, mem_fn_unary_addr_test, bind_function2_test, bind_fwd_test (+37 more) | `/tmp/boost-suite-survey-20260511-j12/libs__bind__test.log` |
| 13 | `libs/bloom/test` | failing | 25.5s | test_boost_bloom_hpp, test_capacity, test_insertion, test_visualization, test_comparison, test_fpr, test_construction, test_bulk_operations, test_combination, test_array, targets | `/tmp/boost-suite-survey-20260511-j12/libs__bloom__test.log` |
| 14 | `libs/callable_traits/test` | passing | 5.2s | B2 completed successfully | `/tmp/boost-suite-survey-20260511-j12/libs__callable_traits__test.log` |
| 15 | `libs/charconv/test` | mixed | 78.6s | limits_link_1, limits, test_boost_json_values, github_issue_152, from_chars_string_view, targets | `/tmp/boost-suite-survey-20260511-j12/libs__charconv__test.log` |
| 16 | `libs/chrono/test` | mixed | 148.5s | min_time_point_d, simulated_thread_interface_demo_d, xtime_h, runtime_resolution_h, saturating_h, i_dont_like_the_default_duration_behavior_h, cycle_count_h, simulated_thread_interface_demo_h, min_time_point_h, process_cpu_clocks, chrono, thread_clock, timeval_demo_h, test_clock_d, test_duration_h, explore_limits_h, test_clock_h, miscellaneous_h, test_special_values_h, chrono_unit_test_d, manipulate_clock_object_d, test_thread_clock_d, chrono_unit_test_h, manipulate_clock_object_h, test_thread_clock_h, traits_common_type_time_point_p_l, traits_treat_as_floating_point_p_l, traits_common_type_time_point_p_h, default_ratio_pass_l, types_pass_l, default_ratio_pass_h, ratio_alias_pass_l, ratio_alias_pass_h, types_pass_h, typedefs_pass_l, typedefs_pass_h, duration_cast_pass_h, rounding_h, traits_treat_as_floating_point_p_h, traits_duration_values_p_h (+60 more) | `/tmp/boost-suite-survey-20260511-j12/libs__chrono__test.log` |
| 17 | `libs/circular_buffer/test` | failing | 24.8s | constant_erase_test, soft_iterator_invalidation, base_test, base_test_dbg, space_optimized_test, space_optimized_test_dbg, targets | `/tmp/boost-suite-survey-20260511-j12/libs__circular_buffer__test.log` |
| 18 | `libs/compat/test` | passing | 5.3s | B2 completed successfully | `/tmp/boost-suite-survey-20260511-j12/libs__compat__test.log` |
| 19 | `libs/compute/test` | passing | 4.1s | B2 completed successfully | `/tmp/boost-suite-survey-20260511-j12/libs__compute__test.log` |
| 20 | `libs/concept_check/test` | mixed | 13.2s | old_concept_pass, class_concept_check_test, function_requires_fail, old_concept_function_fail, where, stl_concept_check, stl_concept_covering, targets | `/tmp/boost-suite-survey-20260511-j12/libs__concept_check__test.log` |
| 21 | `libs/config/test` | mixed | 48.9s | config_test_threaded, config_test_no_except, config_test, config_test_no_rtti, config_link_test, targets | `/tmp/boost-suite-survey-20260511-j12/libs__config__test.log` |
| 22 | `libs/container/test` | mixed | 220.9s | string_view_compat_test, string_test, static_vector_options_test, static_vector_test, small_vector_test, stable_vector_test, segtor_options_test, set_test, pmr_small_vector_test, pmr_stable_vector_test, scoped_allocator_usage_test, pmr_slist_test, small_vector_options_test, scoped_allocator_adaptor_test, pmr_list_test, polymorphic_allocator_test, segtor_test, pmr_deque_test, pmr_nest_test, pmr_set_test, pmr_map_test, slist_test, pair_test, new_delete_resource_test, null_iterators_test, node_handle_test, nest_test, map_test, explicit_inst_static_vector_test, explicit_inst_vector_test, flat_set_adaptor_test, flat_set_test, explicit_inst_list_test, explicit_inst_small_vector_test, explicit_inst_stable_vector_test, explicit_inst_string_test, node_allocator_test, explicit_inst_slist_test, explicit_inst_set_test, explicit_inst_map_test (+13 more) | `/tmp/boost-suite-survey-20260511-j12/libs__container__test.log` |
| 23 | `libs/container_hash/test` | mixed | 28.9s | hash_info, check_float_funcs, hash_fwd_test_1, hash_fwd_test_2, hash_number_test, hash_enum_test, hash_pointer_test, hash_function_pointer_test, hash_float_test, hash_long_double_test, hash_string_test, hash_range_test, hash_custom_test, hash_global_namespace_test, hash_friend_test, hash_built_in_array_test, hash_value_array_test, hash_vector_test, hash_list_test, hash_deque_test, hash_set_test, hash_map_test, hash_complex_test, hash_optional_test, hash_variant_test, hash_type_index_test, hash_system_error_test, hash_std_array_test, hash_std_tuple_test, hash_std_smart_ptr_test, link_test, link_ext_test, extensions_hpp_test, hash_no_ext_macro_2, implicit_test, hash_no_ext_macro_1, hash_reference_values, is_range_test, is_contiguous_range_test, is_unordered_range_test (+40 more) | `/tmp/boost-suite-survey-20260511-j12/libs__container_hash__test.log` |
| 24 | `libs/context/test` | passing | 23.9s | B2 completed successfully | `/tmp/boost-suite-survey-20260511-j12/libs__context__test.log` |
| 25 | `libs/contract/test` | passing | 19.8s | B2 completed successfully | `/tmp/boost-suite-survey-20260511-j12/libs__contract__test.log` |
| 26 | `libs/conversion/test` | mixed | 10.4s | polymorphic_cast_test, target, targets | `/tmp/boost-suite-survey-20260511-j12/libs__conversion__test.log` |
| 27 | `libs/convert/test` | passing | 11.3s | B2 completed successfully | `/tmp/boost-suite-survey-20260511-j12/libs__convert__test.log` |
| 28 | `libs/core/test` | mixed | 275.0s | checked_delete_test, addressof_fail_rvalue, ref_ct_test, ref_rv_fail1, eif_constructors, ref_cv_test, ref_test, lightweight_test_test, lightweight_test_test_no_except, lightweight_test_test2, lightweight_test_all_with_test, lightweight_test_lt_le_test, lightweight_test_gt_ge_test, lightweight_test_eq_nullptr, lightweight_test_test3, lightweight_test_test4, lightweight_test_test5, lightweight_test_test6, lightweight_test_all_with_fail, lightweight_test_fail7, lightweight_test_fail7_no_rtti, lightweight_test_fail8, lightweight_test_fail8_no_rtti, lightweight_test_bool, lightweight_test_with_test, lightweight_test_eq_ptr, lightweight_test_fail12, is_same_test, iterator_test, underlying_type, detail_iterator_test, pointer_traits_pointer_test, pointer_traits_rebind_test, pointer_traits_element_type_test, pointer_traits_difference_type_test, empty_value_compile_fail_casting, pointer_traits_sfinae_test, default_allocator_test, allocator_value_type_test, allocator_pointer_test (+61 more) | `/tmp/boost-suite-survey-20260511-j12/libs__core__test.log` |
| 29 | `libs/coroutine/test` | failing | 294.7s | make_x86_64_sysv_macho_gas, jump_x86_64_sysv_macho_gas, ontop_x86_64_sysv_macho_gas, exceptions, value_semantic, test_asymmetric_coroutine, variables_map, convert, test_symmetric_coroutine, framework, targets | `/tmp/boost-suite-survey-20260511-j12/libs__coroutine__test.log` |
| 30 | `libs/coroutine2/test` | passing | 14.4s | B2 completed successfully | `/tmp/boost-suite-survey-20260511-j12/libs__coroutine2__test.log` |
| 31 | `libs/crc/test` | mixed | 28.1s | quick, pr15_test, crc_test2, crc_test, targets | `/tmp/boost-suite-survey-20260511-j12/libs__crc__test.log` |
| 32 | `libs/date_time/test` | mixed | 172.3s | testconstrained_value, testwrapping_int, testgeneric_period, testmisc_std_cfg, testdate_duration, testdate_facet_new, testdate_input_facet, testdate, testdate_iterator, testgreg_day, testgreg_cal, testgreg_serialize_xml, testgenerators, testgreg_year, testgreg_durations, testgreg_month, basic_iarchive, basic_oarchive, binary_iarchive, testformatters, testparse_date, testperiod, polymorphic_iarchive, binary_oarchive, text_iarchive, polymorphic_text_iarchive, basic_text_iprimitive, basic_text_oprimitive, xml_grammar, polymorphic_binary_iarchive, polymorphic_xml_iarchive, text_oarchive, xml_iarchive, polymorphic_text_oarchive, polymorphic_binary_oarchive, polymorphic_xml_oarchive, testgreg_serialize, xml_oarchive, testfiletime_functions, testlocal_adjustor (+31 more) | `/tmp/boost-suite-survey-20260511-j12/libs__date_time__test.log` |
| 33 | `libs/decimal/test` | passing | 6.0s | B2 completed successfully | `/tmp/boost-suite-survey-20260511-j12/libs__decimal__test.log` |
| 34 | `libs/describe/test` | mixed | 70.7s | trailing_comma_test, enumerators_test, pp_for_each_test, empty_enum_test, nested_enum_test, quick, members_test, bases_test, members_test2, members_test3, members_test4, members_test5, has_enumerators_test, members_test6, members_test7, members_test8, overloaded_test, overloaded_test2, class_template_test, has_bases_test, pedantic_enumerators_test, pedantic_bases_test, pedantic_members_test, has_members_test, nested_enum_test2, unnamed_namespace_test, unnamed_namespace_test2, descriptor_by_name_test, descriptor_by_pointer_test, union_test, union_test2, targets | `/tmp/boost-suite-survey-20260511-j12/libs__describe__test.log` |
| 35 | `libs/detail/test` | mixed | 33.4s | container_no_fwd_test, container_fwd_test, numeric_traits_test, test_utf8_codecvt, targets, container_fwd, container_fwd_debug | `/tmp/boost-suite-survey-20260511-j12/libs__detail__test.log` |
| 36 | `libs/dll/test` | passing | 42.7s | B2 completed successfully | `/tmp/boost-suite-survey-20260511-j12/libs__dll__test.log` |
| 37 | `libs/dynamic_bitset/test` | mixed | 39.6s | test_ambiguous_set, test_std_hash, test_boost_hash, targets | `/tmp/boost-suite-survey-20260511-j12/libs__dynamic_bitset__test.log` |
| 38 | `libs/endian/test` | failing | 8.5s | buffer_test_ni, endian_test, buffer_test, endian_test_ni, endian_operations_test, endian_operations_test_ni, endian_in_union_test, conversion_test, conversion_test_ni, intrinsic_test, endian_reverse_test, quick, endian_load_test, endian_reverse_test_ni, endian_load_test_ni, endian_store_test, endian_store_test_ni, endian_ld_st_roundtrip_test, endian_ld_st_roundtrip_test_ni, endian_arithmetic_test, endian_arithmetic_test_ni, deprecated_test, endian_reverse_cx_test, endian_reverse_cx_test_ni, load_convenience_test, float_typedef_test_ni, store_convenience_test, store_convenience_test_ni, load_convenience_test_ni, float_typedef_test, data_test, data_test_ni, endian_hpp_test, endian_hpp_test_ni, endian_reverse_test2, order_test, endian_reverse_test2_ni, is_scoped_enum_test, endian_reverse_test3, endian_reverse_test3_ni (+8 more) | `/tmp/boost-suite-survey-20260511-j12/libs__endian__test.log` |
| 39 | `libs/exception/test` | mixed | 94.4s | 2-throw_exception_no_exceptions_test, 4-throw_exception_no_both_test, copy_exception_no_exceptions_test, enable_error_info_test, to_string_stub_test, cloning_test, no_exceptions_test, diagnostic_information_test, nlohmann_json_test, error_info_lv_const_test, error_info_lv_test, errno_test, error_info_rv_const_test, error_info_rv_test, exception_ptr_test2, exception_fail, throw_exception_test, unknown_exception_test, errinfos_test, libvisibility_test_lib, targets, visibility_test | `/tmp/boost-suite-survey-20260511-j12/libs__exception__test.log` |
| 40 | `libs/fiber/test` | passing | 26.9s | B2 completed successfully | `/tmp/boost-suite-survey-20260511-j12/libs__fiber__test.log` |
| 41 | `libs/filesystem/test` | passing | 18.5s | B2 completed successfully | `/tmp/boost-suite-survey-20260511-j12/libs__filesystem__test.log` |
| 42 | `libs/flyweight/test` | failing | 111.3s | test_assoc_cont_factory, test_basic, test_concurrent_factory, test_custom_factory, test_init, test_intermod_holder, intermod_holder_dll, test_no_locking, basic_oarchive, basic_iarchive, test_multictor, test_no_tracking, test_serialization, binary_iarchive, polymorphic_iarchive, basic_text_iprimitive, basic_text_oprimitive, text_iarchive, polymorphic_text_iarchive, binary_oarchive, polymorphic_binary_iarchive, polymorphic_xml_iarchive, xml_grammar, text_oarchive, xml_iarchive, polymorphic_text_oarchive, polymorphic_binary_oarchive, polymorphic_xml_oarchive, xml_oarchive, test_set_factory, targets | `/tmp/boost-suite-survey-20260511-j12/libs__flyweight__test.log` |
| 43 | `libs/foreach/test` | mixed | 85.0s | array_byref, array_byref_r, array_byval, array_byval_r, targets | `/tmp/boost-suite-survey-20260511-j12/libs__foreach__test.log` |
| 44 | `libs/format/test` | failing | 41.5s | format_test1, format_test1_windows_h, format_test_wstring, format_test_enum, format_test3_windows_h, format_test3, format_test2_windows_h, format_test_exceptions, format_test2, targets | `/tmp/boost-suite-survey-20260511-j12/libs__format__test.log` |
| 45 | `libs/function/test` | mixed | 249.4s | function_n_test, function_test_no_rtti, function_test, contains_test, rvalues_test, issue_42, fn_eq_bind_test, contains_test_no_rtti, targets | `/tmp/boost-suite-survey-20260511-j12/libs__function__test.log` |
| 46 | `libs/function_types/test` | passing | manual rerun | B2 completed successfully on 2026-05-12 | rerun not captured in survey log |
| 47 | `libs/functional/test` | passing | manual rerun | `function_test` passed on 2026-05-12 | rerun not captured in survey log |
| 48 | `libs/fusion/test` | setup-fail | 361.9s | error: No best alternative for libs/fusion/test/make_pair_r-value with <abi>sysv <address-model>64 <architecture>x86 <asynch-exceptions>off <binary-format>mach-o <boost.beast.allow-deprecated>on <boost.beast.separate-compilation>on <boost.cobalt.executor>any_io_executor <boost.cobalt.pmr>std <context-impl>fcontext <coverage>off <cxxstd-dialect>iso <cxxstd>11 <debug-symbols>on <exception-handling>on <extern-c-nothrow>off <inlining>off <known-warnings>hide <link>static <optimization>off <os>MACOSX <pch>on <preserve-test-targets>on <profiling>off <python-debugging>off <rtti>on <runtime-debugging>on <runtime-link>shared <stdlib>native <strip>off <target-os>darwin <testing.execute>on <threadapi>pthread <threading>multi <toolset-gcc:version>cppgm <toolset>gcc <variant>debug <vectorize>off <visibility>hidden <warnings-as-errors>off <warnings>on <x-deduced-platform>x86_64 | `/tmp/boost-suite-survey-20260511-j12/libs__fusion__test.log` |
| 49 | `libs/geometry/test` | passing | 5.0s | B2 completed successfully | `/tmp/boost-suite-survey-20260511-j12/libs__geometry__test.log` |
| 50 | `libs/gil/test` | passing | 4.5s | B2 completed successfully | `/tmp/boost-suite-survey-20260511-j12/libs__gil__test.log` |
| 51 | `libs/graph/test` | mixed | 561.4s | adj_list_cc, adj_matrix_cc, labeled_graph, transitive_closure_test, transitive_closure_test2, finish_edge_bug, index_graph, test_graphs, adj_list_loops, adj_list_edge_list_set, betweenness_centrality_test, dijkstra_cc, bfs_cc, csr_graph_test, bfs, dfs_cc, undirected_dfs, dfs, bellman-test, bidir_remove_edge, dijkstra_no_color_map_compare, bipartite_test, disjoint_set_test, dag_longest_paths, generator_test, filter_graph_vp_test, edge_list_cc, dominator_tree_test, filtered_graph_cc, graph_1, graph_2, graph_3, graph_4, graph_5, graph_6, graph_7, graph_8, graphml, graph_9, graph_concepts (+238 more) | `/tmp/boost-suite-survey-20260511-j12/libs__graph__test.log` |
| 52 | `libs/graph_parallel/test` | setup-fail | 4.6s | error: No best alternative for /Users/vishvananda/boost_1_91_0/libs/graph_parallel/build/boost_graph_parallel with <abi>sysv <address-model>64 <architecture>x86 <asynch-exceptions>off <binary-format>mach-o <boost.beast.allow-deprecated>on <boost.beast.separate-compilation>on <boost.cobalt.executor>any_io_executor <boost.cobalt.pmr>std <context-impl>fcontext <coverage>off <cxxstd-dialect>iso <cxxstd>11 <debug-symbols>on <exception-handling>on <extern-c-nothrow>off <inlining>off <known-warnings>hide <link>static <optimization>off <os>MACOSX <pch>on <preserve-test-targets>on <profiling>off <python-debugging>off <rtti>on <runtime-debugging>on <runtime-link>shared <stdlib>native <strip>off <target-os>darwin <testing.execute>on <threadapi>pthread <threading>multi <toolset-gcc:version>cppgm <toolset>gcc <variant>debug <vectorize>off <visibility>hidden <warnings-as-errors>off <warnings>on <x-deduced-platform>x86_64; error: No best alternative for /Users/vishvananda/boost_1_91_0/libs/mpi/build/boost_mpi with <abi>sysv <address-model>64 <architecture>x86 <asynch-exceptions>off <binary-format>mach-o <boost.beast.allow-deprecated>on <boost.beast.separate-compilation>on <boost.cobalt.executor>any_io_executor <boost.cobalt.pmr>std <context-impl>fcontext <coverage>off <cxxstd-dialect>iso <cxxstd>11 <debug-symbols>on <exception-handling>on <extern-c-nothrow>off <inlining>off <known-warnings>hide <link>static <optimization>off <os>MACOSX <pch>on <preserve-test-targets>on <profiling>off <python-debugging>off <rtti>on <runtime-debugging>on <runtime-link>shared <stdlib>native <strip>off <target-os>darwin <testing.execute>on <threadapi>pthread <threading>multi <toolset-gcc:version>cppgm <toolset>gcc <variant>debug <vectorize>off <visibility>hidden <warnings-as-errors>off <warnings>on <x-deduced-platform>x86_64; error: Unable to find file or target named; error: referred to from project at | `/tmp/boost-suite-survey-20260511-j12/libs__graph_parallel__test.log` |
| 53 | `libs/hana/test` | passing | 5.6s | B2 completed successfully | `/tmp/boost-suite-survey-20260511-j12/libs__hana__test.log` |
| 54 | `libs/hash2/test` | mixed | 163.5s | get_integral_result, get_integral_result_2, get_integral_result_3, get_integral_result_4, get_integral_result_6, is_trivially_equality_comparable, is_endian_independent, is_range, is_unordered_range, get_integral_result_5, is_contiguous_range, detail_write, digest, detail_write_2, detail_has_tag_invoke, is_tuple_like, has_constant_size, append_integer, append_bool, append_byte_sized, append_character, append_floating_point, append_pointer, append_array, append_container, append_string, append_string_view, append_tuple_like, append_tuple_like_2, append_set, is_contiguously_hashable, append_map, append_described, append_described_2, append_described_3, append_described_5, append_described_4, append_tag_invoke, append_tag_invoke_2, hash_append_range (+76 more) | `/tmp/boost-suite-survey-20260511-j12/libs__hash2__test.log` |
| 55 | `libs/heap/test` | failing | 104.0s | skew_heap_test, priority_queue_test, pairing_heap_tests, framework, fibonacci_heap_test, d_ary_heap_test, binomial_heap_test, mutable_heap_test, move_only_types_test, targets | `/tmp/boost-suite-survey-20260511-j12/libs__heap__test.log` |
| 56 | `libs/histogram/test` | passing | 3.8s | B2 completed successfully | `/tmp/boost-suite-survey-20260511-j12/libs__histogram__test.log` |
| 57 | `libs/hof/test` | passing | 3.3s | B2 completed successfully | `/tmp/boost-suite-survey-20260511-j12/libs__hof__test.log` |
| 58 | `libs/icl/test` | failing | 188.0s | process_cpu_clocks, chrono, thread_clock, fastest_icl_interval, framework, fastest_interval_set, fastest_interval_set_infix, fastest_interval_map, fastest_separate_interval_set, interval_map_right_open, fastest_separate_interval_set_infix, fastest_interval_set_mixed, fastest_split_interval_set, fastest_split_interval_set_infix, interval_map_left_open, fastest_split_interval_map, fastest_interval_map_infix, fastest_split_interval_map_infix, fastest_icl_map, fastest_interval_map_mixed, fastest_interval_map_mixed2, fastest_interval_map_infix_mixed, fastest_partial_interval_quantifier, fastest_set_interval_set, fastest_set_icl_set, fastest_total_interval_quantifier, fastest_partial_icl_quantifier, fastest_total_icl_quantifier, fix_icl_after_thread, fix_tickets, test_misc, test_doc_code, test_changing_interval_defaults, test_type_traits, cmp_clang_ttp_passing, ex_boost_party, chrono_interval_map, chrono_interval_map_infix, chrono_interval_map_mixed, chrono_interval_map_mixed2 (+15 more) | `/tmp/boost-suite-survey-20260511-j12/libs__icl__test.log` |
| 59 | `libs/integer/test` | passing | 27.8s | 20 passed target(s) | `/tmp/boost-suite-survey-20260511-j12/libs__integer__test.log` |
| 60 | `libs/interprocess/test` | mixed | 132.6s | xsi_shared_memory_mapping_test, user_buffer_test, winapi_condition_test, upgradable_mutex_test, spin_semaphore_test, unique_ptr_test, vectorstream_test, vector_test, string_test, stable_vector_test, unordered_test, spin_recursive_mutex_test, spin_condition_test, spin_mutex_test, slist_test, shm_named_semaphore_test, shm_named_recursive_mutex_test, shm_named_mutex_test, shm_named_condition_test, shared_memory_test, shared_memory_mapping_test, sharable_mutex_test, shared_ptr_test, semaphore_test, offset_ptr_test, recursive_mutex_test, robust_recursive_emulation_test, robust_emulation_test, private_node_allocator_test, private_adaptive_pool_test, segment_manager_v1_test, segment_manager_test, null_index_test, node_pool_test, set_test, node_allocator_test, named_upgradable_mutex_test, named_sharable_mutex_test, named_semaphore_test, named_recursive_mutex_test (+40 more) | `/tmp/boost-suite-survey-20260511-j12/libs__interprocess__test.log` |
| 61 | `libs/intrusive/test` | mixed | 42.8s | voidptr_key_test, virtual_base_test, treap_set_test, treap_multiset_test, splay_set_test, unordered_set_test, unordered_multiset_test, sg_set_test, pack_options_test, stateful_value_traits_test, splay_multiset_test, sg_multiset_test, slist_test, scary_iterators_test, recursive_test, pointer_traits_test, set_test, parent_from_member_test, callable_with_no_decltype, null_iterator_test, make_functions_test, callable_with_no_variadic, multiset_test, list_test, default_hook_test, function_hook_test, hash_functor_test, custom_bucket_traits_test, container_size_test, bs_set_test, avl_set_test, any_test, avl_multiset_test, bs_multiset_test, targets | `/tmp/boost-suite-survey-20260511-j12/libs__intrusive__test.log` |
| 62 | `libs/io/test` | failing | 4.7s | quoted_test, quoted_fill_test, ios_state_unit_test, ios_state_test, ostream_put_test, nullstream_test, make_ostream_joiner_test, ostream_joiner_test, targets | `/tmp/boost-suite-survey-20260511-j12/libs__io__test.log` |
| 63 | `libs/iostreams/test` | passing | 17.2s | B2 completed successfully | `/tmp/boost-suite-survey-20260511-j12/libs__iostreams__test.log` |
| 64 | `libs/iterator/test` | mixed | 92.4s | zip_iterator_test_std_pair, zip_iterator_test, zip_iterator_test2_fusion_vector, is_iterator, zip_iterator_test2_fusion_list, zip_iterator_test_fusion, zip_iterator_test_std_tuple, pointee, zip_iterator_test2_std_tuple, iterator_archetype_cc, iterator_adaptor_test, transform_iterator_test, indirect_iterator_test, iterator_traits_test, function_input_iterator_test, filter_iterator_test, iterator_facade, reverse_iterator_test, counting_iterator_test, min_category, permutation_iterator_test, minimum_category, shared_iterator_test, targets | `/tmp/boost-suite-survey-20260511-j12/libs__iterator__test.log` |
| 65 | `libs/json/test` | passing | 6.1s | B2 completed successfully | `/tmp/boost-suite-survey-20260511-j12/libs__json__test.log` |
| 66 | `libs/lambda/test` | mixed | 816.9s | bind_tests_simple, bind_tests_simple_f_refs, is_instance_of_test, cast_test, extending_rt_traits, constructor_tests, bll_and_function, member_pointer_test, rvalue_test, phoenix_control_structures, result_of_tests, exception_test, switch_construct, ret_test, operator_tests_simple, bind_tests_advanced, control_structures, algorithm_test, targets | `/tmp/boost-suite-survey-20260511-j12/libs__lambda__test.log` |
| 67 | `libs/lambda2/test` | passing | 4.1s | B2 completed successfully | `/tmp/boost-suite-survey-20260511-j12/libs__lambda2__test.log` |
| 68 | `libs/leaf/test` | mixed | 171.8s | _hpp_error_test, _hpp_on_error_test, _hpp_result_test, _hpp_exception_test, _hpp_context_test, _hpp_handle_errors_test, _hpp_diagnostics_test, _hpp_pred_test, _hpp_leaf_test, BOOST_LEAF_ASSIGN_test, BOOST_LEAF_AUTO_test, BOOST_LEAF_CHECK_test, capture_exception_result_unload_test, capture_exception_result_async_test, capture_exception_async_test, capture_exception_state_test, capture_exception_unload_test, capture_result_state_test, capture_result_async_test, capture_result_unload_test, boost_exception_test, context_activator_test, context_deduction_test, ctx_handle_all_test, ctx_remote_handle_some_test, ctx_handle_some_test, ctx_remote_handle_all_test, diagnostics_test1, diagnostics_test2, diagnostics_test3, diagnostics_test4, diagnostics_test5, e_errno_test, exception_test, error_id_test, error_code_test, function_traits_test, exception_to_result_test, handle_all_other_result_test, handle_some_other_result_test (+64 more) | `/tmp/boost-suite-survey-20260511-j12/libs__leaf__test.log` |
| 69 | `libs/lexical_cast/test` | mixed | 101.7s | lexical_cast_test, implicit_convert, float_types_test, no_exceptions_test, typedefed_wchar_test, iterator_range_test, from_volatile, arrays_test, stream_detection_test, try_lexical_convert, args_to_numbers, variant_to_long_double, generic_stringize, stream_traits_test, targets | `/tmp/boost-suite-survey-20260511-j12/libs__lexical_cast__test.log` |
| 70 | `libs/local_function/test` | mixed | 124.4s | add_inline, add_except_seq, add, add_seq_nova, add_except, add_inline_seq_nova, add_except_seq_nova, add_seq, add_inline_seq, add_default, add_default_seq_nova, add_default_seq, add_params_only, add_params_only_seq, add_params_only_seq_nova, add_this_seq_nova, add_template, add_this, add_this_seq, add_template_seq, add_template_seq_nova, add_typed, add_typed_seq, add_typed_seq_nova, add_with_default, add_with_default_seq, add_with_default_seq_nova, goto, goto_seq, goto_seq_nova, all_decl_seq_nova, all_decl, all_decl_seq, factorial, factorial_seq, factorial_seq_nova, nesting_seq, nesting_seq_nova, nesting, operator (+35 more) | `/tmp/boost-suite-survey-20260511-j12/libs__local_function__test.log` |
| 71 | `libs/locale/test` | passing | 16.7s | B2 completed successfully | `/tmp/boost-suite-survey-20260511-j12/libs__locale__test.log` |
| 72 | `libs/lockfree/test` | passing | 10.2s | B2 completed successfully | `/tmp/boost-suite-survey-20260511-j12/libs__lockfree__test.log` |
| 73 | `libs/log/test` | passing | 34.8s | B2 completed successfully | `/tmp/boost-suite-survey-20260511-j12/libs__log__test.log` |
| 74 | `libs/logic/test` | mixed | 8.8s | tribool_test, tribool_rename_test, tribool_io_test, targets | `/tmp/boost-suite-survey-20260511-j12/libs__logic__test.log` |
| 75 | `libs/math/test` | passing | 10.1s | B2 completed successfully | `/tmp/boost-suite-survey-20260511-j12/libs__math__test.log` |
| 76 | `libs/metaparse/test` | mixed | 285.2s | back_inserter, accept_when, accept, alphanum, always_c, always, at_c, define_error, framework, digit, digit_to_int, digit_val, empty, entire_input, except, fail_at_first_char_expected, fail, fail_tag, first_of, foldl1, foldl, foldl_reject_incomplete1, front_inserter, has_type, all_headers, foldl_reject_incomplete, foldr1, foldr, int_to_digit, foldr_reject_incomplete1, foldr_reject_incomplete, is_char_c, if_, in_range_c, in_range, int_, is_digit, foldl_reject_incomplete_start_with_parser, foldl_start_with_parser, is_error (+58 more) | `/tmp/boost-suite-survey-20260511-j12/libs__metaparse__test.log` |
| 77 | `libs/move/test` | mixed | 25.2s | unique_ptr_nullptr, unique_ptr_observers, unique_ptr_movector, unique_ptr_types, unique_ptr_modifiers, unique_ptr_default_deleter, unique_ptr_ctordtor, unique_ptr_functions, type_traits, unique_ptr_assign, inplace_merge_test, conversion_test, bench_sort, move_if_noexcept, back_move_inserter, bench_merge, adaptive_sort_test, algo_test, adaptive_merge_test, targets | `/tmp/boost-suite-survey-20260511-j12/libs__move__test.log` |
| 78 | `libs/mp11/test` | mixed | 331.3s | mp11, mp_empty, mp_clear, mp_size, mp_front, mp_assign, complex_h, mp_pop_front, mp_second, mp_third, mp_push_front, mp_rename, mp_push_back, mp_append_2, mp_append, mp_append_sf, mp_replace_front, mp_replace_second, mp_replace_third, mp_apply_q, mp_apply_q_sf, mp_is_list, mp_transform_front, mp_list_c, mp_transform_second, mp_transform_third, mp_transform, mp_transform_2, mp_is_value_list, mp_transform_q, mp_transform_q_2, mp_fill_2, mp_transform_sf, mp_transform_if, mp_transform_if_q, mp_filter, mp_count, mp_repeat_2, mp_fill, mp_count_if (+161 more) | `/tmp/boost-suite-survey-20260511-j12/libs__mp11__test.log` |
| 79 | `libs/mpi/test` | passing | 3.4s | B2 completed successfully | `/tmp/boost-suite-survey-20260511-j12/libs__mpi__test.log` |
| 80 | `libs/mpl/test` | mixed | 133.3s | assert, apply_wrap, largest_int, advance, arithmetic, apply, always, back, as_sequence, at, bind, increased_arity, bool, bitwise, comparison, empty, contains, count, copy, copy_if, count_if, deque, equal, eval_if, erase, erase_range, get_tag_def, front, filter_view, has_xxx, fold, for_each, identity, if, inherit, int, char, char_unsigned, insert, is_placeholder (+50 more) | `/tmp/boost-suite-survey-20260511-j12/libs__mpl__test.log` |
| 81 | `libs/mqtt5/test` | passing | 9.8s | B2 completed successfully | `/tmp/boost-suite-survey-20260511-j12/libs__mqtt5__test.log` |
| 82 | `libs/msm/test` | failing | 381.7s | binary_iarchive, Anonymous, basic_oarchive, basic_iarchive, polymorphic_iarchive, binary_oarchive, text_iarchive, basic_text_iprimitive, basic_text_oprimitive, polymorphic_text_iarchive, polymorphic_xml_iarchive, polymorphic_binary_iarchive, xml_grammar, xml_iarchive, text_oarchive, polymorphic_text_oarchive, polymorphic_binary_oarchive, polymorphic_xml_oarchive, xml_oarchive, AnonymousAndGuard, AnonymousEuml, Back11CompositeMachine, CompositeEuml, BigWithFunctors, CompositeMachine, Constructor, EventQueue, framework, Entries, History, KleeneDeferred, ManyDeferTransitions, OrthogonalDeferred, OrthogonalDeferred2, OrthogonalDeferred3, OrthogonalDeferredEuml, Serialize, SerializeSimpleEuml, SerializeWithHistory, SetStates (+36 more) | `/tmp/boost-suite-survey-20260511-j12/libs__msm__test.log` |
| 83 | `libs/multi_array/test` | mixed | 58.8s | access, constructors, storage_order_convert, iterators, compare, slice, assign, index_bases, storage_order, reshape, assign_to_array, stl_interaction, idxgen1, resize, concept_checks, reverse_view, assert, allocators, targets | `/tmp/boost-suite-survey-20260511-j12/libs__multi_array__test.log` |
| 84 | `libs/multi_index/test` | failing | 125.0s | test_comparison, test_capacity, test_iterators, test_key_extractors, test_hash_ops, test_copy_assignment, test_alloc_awareness, test_list_ops, test_composite_key, test_conv_iterators, test_basic, test_mpl_ops, test_modifiers, test_node_handling, test_observers, test_projection, test_serialization, test_range, test_rank_ops, test_safe_mode, test_rearrange, binary_iarchive, basic_iarchive, basic_oarchive, polymorphic_iarchive, binary_oarchive, text_iarchive, polymorphic_text_iarchive, basic_text_iprimitive, basic_text_oprimitive, polymorphic_xml_iarchive, polymorphic_binary_iarchive, xml_grammar, text_oarchive, xml_iarchive, polymorphic_text_oarchive, polymorphic_binary_oarchive, polymorphic_xml_oarchive, xml_oarchive, test_set_ops (+3 more) | `/tmp/boost-suite-survey-20260511-j12/libs__multi_index__test.log` |
| 85 | `libs/multiprecision/test` | passing | 8.5s | B2 completed successfully | `/tmp/boost-suite-survey-20260511-j12/libs__multiprecision__test.log` |
| 86 | `libs/mysql/test` | failing | 97.6s | make_x86_64_sysv_macho_gas, jump_x86_64_sysv_macho_gas, ontop_x86_64_sysv_macho_gas, framework, targets | `/tmp/boost-suite-survey-20260511-j12/libs__mysql__test.log` |
| 87 | `libs/nowide/test` | passing | 13.7s | B2 completed successfully | `/tmp/boost-suite-survey-20260511-j12/libs__nowide__test.log` |
| 88 | `libs/openmethod/test` | passing | 3.2s | B2 completed successfully | `/tmp/boost-suite-survey-20260511-j12/libs__openmethod__test.log` |
| 89 | `libs/optional/test` | passing | 3.8s | B2 completed successfully | `/tmp/boost-suite-survey-20260511-j12/libs__optional__test.log` |
| 90 | `libs/outcome/test` | passing | 4.8s | B2 completed successfully | `/tmp/boost-suite-survey-20260511-j12/libs__outcome__test.log` |
| 91 | `libs/parameter/test` | mixed | 90.6s | maybe, tutorial, efficiency, parameterized_inheritance, singular, preprocessor_eval_cat_no_spec, compose, optional_deduced_sfinae, preprocessor_eval_category, sfinae, evaluate_category_16, evaluate_category, normalized_argument_types, preprocessor, preprocessor_deduced, earwicker, unwrap_cv_reference, mpl, deduced, basics, deduced_dependent_predicate, macros, function_type_tpl_param, ntp, deduced-parameters0, default-expression-evaluation0, building-argumentpacks0, deduced-template-parameters0, fine-grained-name-control0, extracting-parameter-types0, namespaces0, namespaces1, namespaces2, extracting-parameter-types1, namespaces3, lazy-default-computation0, parameter-enabled-constructors0, parameter-enabled-function-call-operators0, lazy-default-computation1, parameter-enabled-member-functions0 (+18 more) | `/tmp/boost-suite-survey-20260511-j12/libs__parameter__test.log` |
| 92 | `libs/parser/test` | mixed | 21.8s | compile_attribute, compile_combining_groups, compile_seq_attribute, compile_all_t, aggr_tuple_assignment, compile_or_attribute, case_fold_generated, github_issues, class_type, hl, merge_separate, no_case, parse_coords_new, parser_action, parse_empty, parser_action_with_params, parser_api, parser_attributes, parser, parser_if_switch, parser_lazy_params, parser_or_permutations_1, parser_or_permutations_2, parser_rule, parser_perm, parser_rule_with_params, parser_quoted_string, parser_seq_permutations_1, parser_seq_permutations_2, parser_symbol_table, replace, search, split, tracing, transform_replace, tuple_aggregate, targets | `/tmp/boost-suite-survey-20260511-j12/libs__parser__test.log` |
| 93 | `libs/pfr/test` | passing | 11.9s | 1 passed target(s) | `/tmp/boost-suite-survey-20260511-j12/libs__pfr__test.log` |
| 94 | `libs/phoenix/test` | mixed | 155.6s | custom_terminal, cmath, primitives_tests, intel_test, arithmetic_tests, if_else_tests, bitwise_tests, misc_binary_tests, comparison_tests, logical_tests, io_tests, self_tests, member, unary_tests, cast_tests, new_delete_tests, function_tests, adapt_function, lazy_argument_tests, lazy_list2_tests, lazy_list3_tests, lazy_list_tests, lazy_make_pair_tests, lazy_templated_struct_tests, lazy_operator_tests, bug5782, bug5715, bind_function_tests, exceptions, bind_function_object_tests, bind_member_function_tests, bind_member_variable_tests, loops_tests, if_tests, switch_tests, container_tests1a, container_tests1b, container_tests2a, container_tests2b, container_tests3a (+160 more) | `/tmp/boost-suite-survey-20260511-j12/libs__phoenix__test.log` |
| 95 | `libs/poly_collection/test` | passing | 13.4s | B2 completed successfully | `/tmp/boost-suite-survey-20260511-j12/libs__poly_collection__test.log` |
| 96 | `libs/polygon/test` | failing | 4.3s | polygon_set_data_test, gtl_boost_unit_test, polygon_rectangle_formation_test, polygon_90_data_test, voronoi_builder_test, polygon_interval_test, polygon_rectangle_test, polygon_point_test, polygon_segment_test, voronoi_geometry_type_test, voronoi_ctypes_test, voronoi_diagram_test, voronoi_predicates_test, voronoi_robust_fpt_test, voronoi_structures_test, targets | `/tmp/boost-suite-survey-20260511-j12/libs__polygon__test.log` |
| 97 | `libs/pool/test` | failing | 52.0s | pool_msvc_compiler_bug_test, binary_iarchive, test_pool_alloc, basic_oarchive, basic_iarchive, polymorphic_iarchive, binary_oarchive, basic_text_iprimitive, basic_text_oprimitive, text_iarchive, polymorphic_text_iarchive, polymorphic_binary_iarchive, polymorphic_xml_iarchive, xml_grammar, xml_iarchive, text_oarchive, polymorphic_text_oarchive, polymorphic_binary_oarchive, polymorphic_xml_oarchive, test_msvc_mem_leak_detect, test_bug_3349, test_bug_4960, test_bug_5526, test_bug_1252, test_bug_2696, test_bug_6701, test_poisoned_macros, xml_oarchive, targets | `/tmp/boost-suite-survey-20260511-j12/libs__pool__test.log` |
| 98 | `libs/preprocessor/test` | mixed | 148.3s | list, seq, targets | `/tmp/boost-suite-survey-20260511-j12/libs__preprocessor__test.log` |
| 99 | `libs/program_options/test` | failing | 155.6s | value_semantic, convert, variables_map, options_description_test, options_description_test_dll, parsers_test, parsers_test_dll, cmdline_test, cmdline_test_dll, variable_map_test, positional_options_test, variable_map_test_dll, positional_options_test_dll, exception_test, unicode_test, unicode_test_dll, exception_test_dll, split_test, optional_test, split_test_dll, optional_test_dll, required_test, required_test_dll, exception_txt_test, options_description_no_rtti_test, exception_txt_test_dll, quick, targets, winmain, winmain_dll, unrecognized_test, unrecognized_test_dll, libboost_program_options | `/tmp/boost-suite-survey-20260511-j12/libs__program_options__test.log` |
| 100 | `libs/property_map/test` | failing | 19.9s | compose_property_map_test, function_property_map_test, transform_value_property_map_test, property_map_cc, dynamic_properties_test, dynamic_properties_no_rtti_test, targets | `/tmp/boost-suite-survey-20260511-j12/libs__property_map__test.log` |
| 101 | `libs/property_tree/test` | failing | 56.1s | test_property_tree, binary_iarchive, basic_oarchive, basic_iarchive, binary_oarchive, polymorphic_iarchive, basic_text_iprimitive, basic_text_oprimitive, text_iarchive, polymorphic_text_iarchive, polymorphic_binary_iarchive, xml_grammar, polymorphic_xml_iarchive, text_oarchive, polymorphic_text_oarchive, xml_iarchive, polymorphic_binary_oarchive, polymorphic_xml_oarchive, test_info_parser, test_json_parser, test_json_parser2, info_grammar_spirit, test_json_parser3, test_ini_parser, test_xml_parser_rapidxml, test_multi_module1, custom_data_type, debug_settings, empty_ptree_trick, speed_test, xml_oarchive, targets, test_rapidxml | `/tmp/boost-suite-survey-20260511-j12/libs__property_tree__test.log` |
| 102 | `libs/proto/test` | failing | 136.8s | calculator, framework, deep_copy, deduce_domain, constrained_ops, cpp-next_bug, examples, display_expr, env_var, lambda, flatten, make_expr, matches, switch, external_transforms, toy_spirit, toy_spirit2, make, noinvoke, mem_ptr, mpl, protect, bug2407, pack_expansion, targets | `/tmp/boost-suite-survey-20260511-j12/libs__proto__test.log` |
| 103 | `libs/ptr_container/test` | mixed | 116.1s | ptr_vector, ptr_map, ptr_map_adapter, tree_test, incomplete_type_test, ptr_list, ptr_deque, ptr_set, ptr_array, framework, serialization, basic_iarchive, basic_oarchive, binary_iarchive, basic_text_iprimitive, basic_text_oprimitive, binary_oarchive, polymorphic_iarchive, text_iarchive, polymorphic_text_iarchive, xml_grammar, polymorphic_binary_iarchive, polymorphic_xml_iarchive, text_oarchive, xml_iarchive, polymorphic_text_oarchive, ptr_unordered_map, polymorphic_binary_oarchive, polymorphic_xml_oarchive, no_exceptions, ptr_unordered_set, ptr_circular_buffer, xml_oarchive, const_element_containers, targets, ptr_inserter, view_example, iterator_test, tut1, indirect_fun | `/tmp/boost-suite-survey-20260511-j12/libs__ptr_container__test.log` |
| 104 | `libs/python/test` | passing | 3.2s | B2 completed successfully | `/tmp/boost-suite-survey-20260511-j12/libs__python__test.log` |
| 105 | `libs/qvm/test` | mixed | 1145.0s | quat_traits_array_test, vec_traits_std_array_test, vec_traits_array_test, math_test, mat_traits_array_test, mat_traits_std_array_test, quat_traits_std_array_test, access_q_test, access_m_test, access_v_test, cmp_vv_test, cross_test, dot_vv_test, div_vs_test, mag_v_test, mag_sqr_v_test, to_string_test, minus_v_test, eq_vv_test, minus_vv_test, mul_vs_test, mul_sv_test, plus_vv_test, scalar_cast_v_test, vec_register_test, vec_index_test, assign_test, div_eq_ms_test, mul_eq_mm_test, mul_eq_ms_test, inverse_m_test, minus_eq_mm_test, minus_m_test, plus_eq_mm_test, mat_index_test, scalar_cast_m_test, div_ms_test, cmp_qq_test, conjugate_test, normalize_q_test (+62 more) | `/tmp/boost-suite-survey-20260511-j12/libs__qvm__test.log` |
| 106 | `libs/random/test` | passing | 5.7s | B2 completed successfully | `/tmp/boost-suite-survey-20260511-j12/libs__random__test.log` |
| 107 | `libs/range/test` | mixed | 733.5s | adjacent_filtered_concept, adjacent_filtered_concept2, adjacent_filtered_concept3, adjacent_filtered_concept4, sliced_concept, framework, sliced_concept2, uniqued_concept, uniqued_concept2, uniqued_concept3, uniqued_concept4, chained, filtered, map, ref_unwrapped, ref_unwrapped_example, replaced, replaced_if, copied, indexed, transformed, ticket_8676_sliced_transformed, indirected, strided2, sliced, strided, type_erased_brackets, type_erased_abstract, type_erased_transformed, filtered_example, type_erased_mix_values, type_erased, uniqued, map_keys_example, map_values_example, copied_example, replaced_example, transformed_example, indexed_example, replaced_if_example (+137 more) | `/tmp/boost-suite-survey-20260511-j12/libs__range__test.log` |
| 108 | `libs/ratio/test` | mixed | 5.0s | ratio_pass, typedefs_pass, ratio_divide_pass, ratio_add_pass, ratio_subtract_pass, ratio_multiply_pass, ratio_io_pass, ratio_gcd_pass, ratio_equal_pass, ratio_less_pass, ratio_not_equal_pass, ratio_less_equal_pass, ratio_greater_equal_pass, ratio_greater_pass, is_evenly_divisible_by, si_physics, display_ex, gcd_lcm, quick, is_ratio, targets | `/tmp/boost-suite-survey-20260511-j12/libs__ratio__test.log` |
| 109 | `libs/rational/test` | mixed | 117.9s | rational_example, rational_test, framework, targets | `/tmp/boost-suite-survey-20260511-j12/libs__rational__test.log` |
| 110 | `libs/redis/test` | passing | 8.4s | B2 completed successfully | `/tmp/boost-suite-survey-20260511-j12/libs__redis__test.log` |
| 111 | `libs/regex/test` | mixed | 438.5s | regex_timer, regex_regress, grep, posix_api, wide_posix_api, range_concept_check, regex_regress_noeh, test_bug_11988, issue153, value_semantic, variables_map, convert, regex_grep_example_1, regex_grep_example_2, regex_split_example_1, regex_search_example, regex_split_example_2, regex_iterator_example, targets, unicode_iterator_test_utf8, unicode_iterator_test_utf16, test_grep | `/tmp/boost-suite-survey-20260511-j12/libs__regex__test.log` |
| 112 | `libs/safe_numerics/test` | passing | 4.2s | B2 completed successfully | `/tmp/boost-suite-survey-20260511-j12/libs__safe_numerics__test.log` |
| 113 | `libs/scope/test` | passing | 4.3s | B2 completed successfully | `/tmp/boost-suite-survey-20260511-j12/libs__scope__test.log` |
| 114 | `libs/scope_exit/test` | mixed | 51.2s | world, world_seq, same_line, same_line_seq, world_seq_nova, same_line_seq_nova, world_checkpoint_seq, world_checkpoint, world_this, world_this_seq, world_tpl, world_tpl_seq, world_this_seq_nova, world_checkpoint_seq_nova, native, emulation, native_tpl, world_tpl_seq_nova, emulation_tpl, native_tu_test, emulation_tu_test, targets | `/tmp/boost-suite-survey-20260511-j12/libs__scope_exit__test.log` |
| 115 | `libs/serialization/test` | mixed | 781.0s | dll_polymorphic_base, dll_polymorphic_derived2, dll_a, basic_oarchive, basic_iarchive, binary_iarchive, basic_text_iprimitive, polymorphic_iarchive, text_iarchive, binary_oarchive, polymorphic_text_iarchive, basic_text_oprimitive, polymorphic_binary_iarchive, xml_grammar, polymorphic_xml_iarchive, text_oarchive, xml_iarchive, polymorphic_text_oarchive, polymorphic_binary_oarchive, polymorphic_xml_oarchive, test_array_text_archive, xml_oarchive, text_wiarchive, test_array_text_warchive, xml_wgrammar, polymorphic_text_wiarchive, xml_wiarchive, polymorphic_xml_wiarchive, text_woarchive, polymorphic_text_woarchive, basic_text_wiprimitive, basic_text_woprimitive, test_array_binary_archive, polymorphic_xml_woarchive, test_array_xml_archive, test_array_xml_warchive, xml_woarchive, test_boost_array_text_archive, test_boost_array_text_warchive, test_boost_array_binary_archive (+306 more) | `/tmp/boost-suite-survey-20260511-j12/libs__serialization__test.log` |
| 116 | `libs/signals2/test` | failing | 138.6s | connection_test, ordering_test, deletion_test, regression_test, dead_slot_test, track_test, signal_test, signal_type_test, move_construct_only_test, shared_connection_block_test, deadlock_regression_test, trackable_test, deconstruct_test, slot_compile_test, signal_n_test, targets | `/tmp/boost-suite-survey-20260511-j12/libs__signals2__test.log` |
| 117 | `libs/smart_ptr/test` | passing | 5.0s | B2 completed successfully | `/tmp/boost-suite-survey-20260511-j12/libs__smart_ptr__test.log` |
| 118 | `libs/sort/test` | failing | 199.8s | integer_sort, float_sort, string_sort, sort_detail, test_pdqsort, framework, posix_api, wide_posix_api, unit_test_parameters, targets | `/tmp/boost-suite-survey-20260511-j12/libs__sort__test.log` |
| 119 | `libs/spirit/test` | passing | 17.9s | B2 completed successfully | `/tmp/boost-suite-survey-20260511-j12/libs__spirit__test.log` |
| 120 | `libs/stacktrace/test` | mixed | 130.0s | libboost_stacktrace_from_exception, addr2line_ho, basic, addr2line, basic_ho, basic_ho_empty, trivial_addr2line_ho, trivial_basic_ho_empty, trivial_basic_ho, basic_ho_no_dbg, basic_lib_threaded, noop_ho, basic_ho_no_dbg_empty, addr2line_ho_no_dbg, noop_lib, addr2line_lib, basic_lib, basic_lib_no_dbg_threaded, noop_ho_no_dbg, from_exception_none_basic_ho, test_void_ptr_cast, from_exception_disabled_basic_ho, test_num_conv, from_exception_basic_ho, noop_lib_no_dbg, addr2line_lib_no_dbg, basic_lib_no_dbg, from_exception_disabled_none_ho, addr2line_throwing_st, noop_throwing_st, basic_throwing_st, addr2line_throwing_st_no_dbg, noop_throwing_st_no_dbg, basic_throwing_st_no_dbg, targets, libboost_stacktrace_addr2line, libboost_stacktrace_basic, trivial_addr2line_lib, trivial_basic_lib, from_exception_none_basic (+19 more) | `/tmp/boost-suite-survey-20260511-j12/libs__stacktrace__test.log` |
| 121 | `libs/statechart/test` | passing | 19.7s | B2 completed successfully | `/tmp/boost-suite-survey-20260511-j12/libs__statechart__test.log` |
| 122 | `libs/static_assert/test` | mixed | 6.0s | static_assert_example_2, target, targets | `/tmp/boost-suite-survey-20260511-j12/libs__static_assert__test.log` |
| 123 | `libs/static_string/test` | passing | 3.1s | B2 completed successfully | `/tmp/boost-suite-survey-20260511-j12/libs__static_string__test.log` |
| 124 | `libs/stl_interfaces/test` | passing | 3.1s | B2 completed successfully | `/tmp/boost-suite-survey-20260511-j12/libs__stl_interfaces__test.log` |
| 125 | `libs/system/test` | mixed | 430.2s | error_code_user_test_utf8, error_code_user_test_shared, error_code_test, error_code_test_utf8, error_code_user_test_static, error_code_user_test_nthr, error_code_test_nthr, error_code_user_test, error_code_test_shared, error_code_test_no_ansi, error_code_test_static, error_code_user_test_no_ansi, throw_test, system_error_test_static, initialization_test_static, system_error_test, throw_test_shared, system_error_test_shared, initialization_test_shared, initialization_test, system_error_test_utf8, system_error_test_nthr, system_error_test_no_ansi, single_instance_2, single_instance_1, std_single_instance_1, std_single_instance_2, initialization_test_no_ansi, initialization_test_utf8, initialization_test_nthr, header_only_test, header_only_test_shared, header_only_test_static, header_only_test_no_ansi, std_interop_test, header_only_test_utf8, std_interop_test_static, header_only_test_nthr, std_interop_test_shared, std_interop_test_no_ansi (+168 more) | `/tmp/boost-suite-survey-20260511-j12/libs__system__test.log` |
| 126 | `libs/test/test` | mixed | 895.3s | algorithm-test, basic_cstring-test, class_properties-test, foreach-test, named_params-test, string_cast-test, token_iterator-test, single-header-test, single-header-custom-init-test, single-header-custom-main-test, static-library-test, static-library-custom-init-test, shared-library-test, framework, shared-library-custom-init-test, shared-library-custom-main-test, header-only-over-multiple-files, log-formatter-test, run-by-name-or-label-test, version-uses-module-name, test-macro-global-fixture, log-count-skipped-test, dataset-size, assertion-construction-test, boost_check_equal-str-test, collection-comparison-test, fp-comparisons-test, fp-no-comparison-for-incomplete-types-test, fp-relational-operator, output_test_stream-test, windows-headers-test, tools-debuggable-test, tools-under-debugger-test, nullptr-support-test, user-defined-types-logging-customization-points, test-fixture-detect-setup-teardown, test-fixture-detect-setup-teardown-cpp11, test-with-precondition, parameterized_test-test, test_case_template-with-tuples-test (+84 more) | `/tmp/boost-suite-survey-20260511-j12/libs__test__test.log` |
| 127 | `libs/thread/test` | mixed | 14.9s | test_10963_c, ex_starvephil, ex_condition, string_trim_test, string_to_unsigned_test, targets | `/tmp/boost-suite-survey-20260511-j12/libs__thread__test.log` |
| 128 | `libs/throw_exception/test` | mixed | 44.2s | throw_exception_test3, throw_from_library_static, throw_from_library_shared, throw_exception_nx_test, throw_exception_test4, throw_exception_nx_test2, throw_exception_test5, make_exception_ptr_test, make_exception_ptr_test2, throw_with_location_nx_test, throw_with_location_test4, targets | `/tmp/boost-suite-survey-20260511-j12/libs__throw_exception__test.log` |
| 129 | `libs/timer/test` | mixed | 44.8s | timex, process_cpu_clocks, chrono, thread_clock, chrono_conflict_test, targets | `/tmp/boost-suite-survey-20260511-j12/libs__timer__test.log` |
| 130 | `libs/tokenizer/test` | passing | 20.7s | 6 passed target(s) | `/tmp/boost-suite-survey-20260511-j12/libs__tokenizer__test.log` |
| 131 | `libs/tti/test` | mixed | 274.1s | test_has_member_cv_compile, test_has_member_compile, test_has_mem_data_compile, test_has_mem_fun_compile, test_has_mem_fun_cv_compile, test_has_member_cv, test_has_member, test_has_static_mem_data_compile, test_has_mem_data2, test_has_mem_fun, test_has_mem_fun_cv, test_has_static_mem_data, test_has_data_compile, test_has_static_mem_fun_compile, test_has_static_member_compile, test_has_data, test_has_fun_compile, test_has_mem_fun_template_compile, test_has_member_template_compile, test_has_static_member, test_has_fun, test_has_mem_fun_template_cv_compile, test_has_mem_fun_template, test_has_mem_fun_template_nov, test_has_member_template, test_has_member_template_nov, test_has_member_template_cv_compile, test_has_mem_fun_template_cv, test_has_mem_fun_template_cv_nov, test_has_member_template_cv, test_has_member_template_cv_nov, test_has_static_member_template_compile, test_has_static_mem_fun_template_compile, test_has_template_compile, test_has_fun_template_compile, test_has_template_cp_compile, test_has_template, test_has_fun_template, test_has_fun_template_nov, test_has_type_compile (+13 more) | `/tmp/boost-suite-survey-20260511-j12/libs__tti__test.log` |
| 132 | `libs/tuple/test` | mixed | 12.8s | tuple_test_bench, std_tuple_element, std_tuple_size, io_test, targets | `/tmp/boost-suite-survey-20260511-j12/libs__tuple__test.log` |
| 133 | `libs/type_index/test` | mixed | 78.6s | test_lib_nortti_compat-obj, test_lib_anonymous_nortti-obj, test_lib_nortti-obj, track_13621, type_index_constexpr_test, type_index_test_no_rtti, compare_ctti_stl, ctti_print_name, test_lib_rtti_compat-obj, testing_crossmodule_no_rtti, type_index_runtime_cast_test, type_index_test_ctti_alignment, link_fail_nortti_rtti, testing_crossmodule_nortti_rtti_compat, testing_crossmodule_anonymous, testing_crossmodule_rtti_nortti_compat, runtime_cast, user_defined_typeinfo, user_defined_typeinfo_no_rtti, table_of_names, runtime_cast_no_rtti, inheritance_no_rtti, registry_no_rtti, registry, exact_types_match_no_rtti, demangled_names_no_rtti, targets, link_fail_rtti_nortti | `/tmp/boost-suite-survey-20260511-j12/libs__type_index__test.log` |
| 134 | `libs/type_traits/test` | mixed | 1217.7s | type_traits_test, tricky_incomplete_type_test, type_with_alignment_test, tricky_function_type_test, tricky_partial_spec_test, promote_mpl_test, promote_enum_test, promote_enum_msvc_bug_test, promote_basic_test, mpl_interop_test3, make_unsigned_test, make_signed_test, is_volatile_test, is_virtual_base_of_test, remove_volatile_test, is_unbounded_array_test, remove_const_test, remove_cv_test, remove_all_extents_test, is_swappable_test, is_nothrow_swappable_test, is_member_obj_test, is_member_func_test, is_function_test, is_pod_test, is_list_constructible_test, is_nothrow_move_constructible_test, is_nothrow_move_assignable_test, is_const_test, is_constructible_test, is_bounded_array_test, is_complete_test, is_array_test, is_destructible_test, is_base_of_test, is_convertible_test, is_default_constr_test, is_copy_assignable_test, has_trivial_copy_test, is_assignable_test (+75 more) | `/tmp/boost-suite-survey-20260511-j12/libs__type_traits__test.log` |
| 135 | `libs/typeof/test` | mixed | 13.6s | function_binding_native, function_binding_emulation, targets | `/tmp/boost-suite-survey-20260511-j12/libs__typeof__test.log` |
| 136 | `libs/units/test` | passing | 4.8s | B2 completed successfully | `/tmp/boost-suite-survey-20260511-j12/libs__units__test.log` |
| 137 | `libs/unordered/test` | mixed | 63.2s | tl_unordered_map_hpp, tl_unordered_set_hpp, compile_map, bucket_tests, allocator_traits, at_tests, assign_tests, binary_iarchive, basic_oarchive, basic_iarchive, polymorphic_iarchive, binary_oarchive, text_iarchive, basic_text_iprimitive, basic_text_oprimitive, polymorphic_text_iarchive, xml_grammar, polymorphic_binary_iarchive, polymorphic_xml_iarchive, visualization_tests, quick, compile_set, constructor_tests, contains_tests, copy_tests, deduction_tests, text_oarchive, emplace_smf_tests, emplace_tests, equality_tests, equivalent_keys_tests, erase_equiv_tests, erase_if, erase_tests, explicit_alloc_ctor_tests, extract_tests, find_tests, fwd_map_test, fwd_set_test, incomplete_test (+134 more) | `/tmp/boost-suite-survey-20260511-j12/libs__unordered__test.log` |
| 138 | `libs/url/test` | failing | 20.6s | test_suite, target, test_test_suite, targets | `/tmp/boost-suite-survey-20260511-j12/libs__url__test.log` |
| 139 | `libs/utility/test` | mixed | 43.5s | result_of_test, binary_test, iterators_test, operators_test, value_init_test, call_traits_test, value_init_test2, string_view_test1, initialized_test_fail1, value_init_workaround_test, targets | `/tmp/boost-suite-survey-20260511-j12/libs__utility__test.log` |
| 140 | `libs/uuid/test` | mixed | 14.3s | dlmalloc, global_resource, monotonic_buffer_resource, pool_resource, synchronized_pool_resource, unsynchronized_pool_resource, archive_exception, basic_archive, basic_iarchive, basic_iserializer, basic_oarchive, test_uuid, test_include1, test_uuid_no_simd, test_uuid_3, test_uuid_2, test_attribute_packed, test_alignment_2, test_pragma_pack, test_data, test_comparison, test_alignment, test_comparison_no_simd, test_io, test_io_2_no_simd, test_io_no_simd, test_io_2, test_to_chars_no_simd, test_to_chars_2, test_to_chars, test_to_chars_2_no_simd, test_to_chars_3, test_from_chars, test_from_chars_2_no_simd, test_from_chars_no_simd, test_from_chars_2, test_uuid_from_string_2, test_uuid_from_string, test_uuid_clock, test_uuid_from_string_3 (+81 more) | `/tmp/boost-suite-survey-20260511-j12/libs__uuid__test.log` |
| 141 | `libs/variant/test` | mixed | 74.8s | variant_test3, variant_test1, variant_test4, variant_test2, variant_test5, variant_test6, variant_test8, variant_test9, variant_test7, variant_test3_no_rtti, variant_reference_test, recursive_variant_test, variant_visit_test, variant_comparison_test, variant_get_test, fusion_interop, variant_polymorphic_get_test, variant_multivisit_test, rvalue_test, variant_nonempty_check, hash_variant_test, variant_noexcept_test, issue42, variant_swap_test, hash_recursive_variant_test, auto_visitors, overload_selection, variant_over_joint_view_test, variant_no_rtti_test, const_ref_apply_visitor, targets | `/tmp/boost-suite-survey-20260511-j12/libs__variant__test.log` |
| 142 | `libs/variant2/test` | failing | 131.4s | variant_holds_alternative_cx, variant_default_construct_cx, quick, variant_get_by_type_cx, variant_get_by_index_cx, variant_holds_alternative, variant_size, variant_default_construct, variant_get_by_index, variant_get_by_type, variant_copy_construct, variant_alternative, variant_move_construct_cx, variant_copy_construct_cx, variant_value_construct_cx, variant_in_place_index_construct_cx, variant_in_place_type_construct_cx, variant_copy_assign_cx, variant_value_construct, variant_move_construct, variant_in_place_index_construct, variant_in_place_type_construct, variant_move_assign, variant_copy_assign, variant_move_assign_cx, variant_value_assign_cx, variant_emplace_index_cx, variant_emplace_type_cx, variant_value_assign, variant_emplace_index, variant_eq_ne_cx, variant_emplace_type, variant_swap, variant_lt_gt_cx, variant_eq_ne, variant_lt_gt, variant_visit, variant_destroy, variant_valueless, variant_subset (+43 more) | `/tmp/boost-suite-survey-20260511-j12/libs__variant2__test.log` |
| 143 | `libs/vmd/test` | mixed | 521.2s | test_seq_to_list, target, targets | `/tmp/boost-suite-survey-20260511-j12/libs__vmd__test.log` |
| 144 | `libs/wave/test` | passing | 4.4s | B2 completed successfully | `/tmp/boost-suite-survey-20260511-j12/libs__wave__test.log` |
| 145 | `libs/winapi/test` | passing | 4.1s | B2 completed successfully | `/tmp/boost-suite-survey-20260511-j12/libs__winapi__test.log` |
| 146 | `libs/xpressive/test` | mixed | 174.8s | regress, regress_u, c_traits, c_traits_u, test1, test2, framework, test3, test4, test5, test6, test7, test8, test9, test10, test11, test1u, test2u, test3u, test6u, test4u, test5u, test7u, test8u, test9u, test10u, test11u, misc1, misc2, test_format, test_static, test_non_char, test_actions, test_cycles, test_assert, test_assert_with_placeholder, test_symbols, test_dynamic_grammar, test_skip, test_dynamic (+11 more) | `/tmp/boost-suite-survey-20260511-j12/libs__xpressive__test.log` |
| 147 | `libs/yap/test` | passing | 4.7s | B2 completed successfully | `/tmp/boost-suite-survey-20260511-j12/libs__yap__test.log` |
