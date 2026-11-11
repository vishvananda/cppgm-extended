# libs/asio/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| any_completion_executor.test | RUN | 0.641094 | 3.875227 | 2.965438 | 0.002582 | 0.002890 | 0.002716 |
| any_completion_executor_select.test | RUN | 0.651029 | 3.795871 | 3.120278 | 0.002386 | 0.002664 | 0.002336 |
| any_completion_handler.test | RUN | 0.696653 | 3.314467 | 2.722461 | 0.002089 | 0.002501 | 0.001886 |
| any_completion_handler_select.test | RUN | 0.704023 | 3.439232 | 2.890876 | 0.002016 | 0.002264 | 0.002078 |
| any_io_executor.test | RUN | 0.637770 | 4.003801 | 3.317274 | 0.002286 | 0.002556 | 0.002614 |
| any_io_executor_select.test | RUN | 0.639143 | 3.863210 | 3.264189 | 0.002103 | 0.002539 | 0.002256 |
| append.test | RUN | 0.589099 | 3.069308 | 2.369482 | 0.001736 | 0.002220 | 0.002064 |
| append_select.test | RUN | 0.590920 | 3.196526 | 2.412861 | 0.001927 | 0.002418 | 0.002076 |
| as_tuple.test | RUN | 0.775073 | 3.757113 | 3.481899 | 0.002088 | 0.002040 | 0.001740 |
| as_tuple_select.test | RUN | 0.786591 | 3.615610 | 3.457271 | 0.001666 | 0.002427 | 0.001852 |
| associated_allocator.test | LINK | 0.173729 | 0.311870 | 0.329885 | — | — | — |
| associated_allocator_select.test | LINK | 0.170793 | 0.337573 | 0.354243 | — | — | — |
| associated_cancellation_slot.test | LINK | 0.191513 | 0.354509 | 0.380070 | — | — | — |
| associated_cancellation_slot_select.test | LINK | 0.195636 | 0.360097 | 0.417398 | — | — | — |
| associated_executor.test | LINK | 0.271708 | 0.608299 | 0.693750 | — | — | — |
| associated_executor_select.test | LINK | 0.266810 | 0.620088 | 0.708636 | — | — | — |
| associated_immediate_executor.test | LINK | 0.252563 | 0.601334 | 0.626561 | — | — | — |
| associated_immediate_executor_select.test | LINK | 0.251318 | 0.566553 | 0.578299 | — | — | — |
| associator.test | LINK | 0.137284 | 0.256163 | 0.298139 | — | — | — |
| associator_select.test | LINK | 0.145972 | 0.260610 | 0.299712 | — | — | — |
| awaitable.test | LINK | 0.146454 | 0.253564 | 0.297584 | — | — | — |
| awaitable_select.test | LINK | 0.144856 | 0.252081 | 0.298990 | — | — | — |
| basic_datagram_socket.test | LINK | 0.412885 | 0.790334 | 1.111626 | — | — | — |
| basic_datagram_socket_select.test | LINK | 0.415900 | 0.813000 | 1.172487 | — | — | — |
| basic_deadline_timer.test | LINK | 0.487733 | 0.947541 | 1.290382 | — | — | — |
| basic_deadline_timer_select.test | LINK | 0.497397 | 1.010873 | 1.313857 | — | — | — |
| basic_file.test | LINK | 0.136421 | 0.282832 | 0.293438 | — | — | — |
| basic_file_select.test | LINK | 0.145392 | 0.261952 | 0.287382 | — | — | — |
| basic_random_access_file.test | LINK | 0.163817 | 0.263691 | 0.297467 | — | — | — |
| basic_random_access_file_select.test | LINK | 0.166725 | 0.255322 | 0.289038 | — | — | — |
| basic_raw_socket.test | LINK | 0.428483 | 0.828009 | 1.084995 | — | — | — |
| basic_raw_socket_select.test | LINK | 0.429000 | 0.845772 | 1.118778 | — | — | — |
| basic_readable_pipe.test | LINK | 0.405290 | 0.792369 | 0.934221 | — | — | — |
| basic_readable_pipe_select.test | LINK | 0.424817 | 0.851042 | 1.170492 | — | — | — |
| basic_seq_packet_socket.test | LINK | 0.422966 | 0.795751 | 1.146333 | — | — | — |
| basic_seq_packet_socket_select.test | LINK | 0.426494 | 0.824619 | 1.168474 | — | — | — |
| basic_signal_set.test | LINK | 0.383731 | 0.757540 | 0.916830 | — | — | — |
| basic_signal_set_select.test | LINK | 0.402490 | 0.789945 | 1.090882 | — | — | — |
| basic_socket_acceptor.test | LINK | 0.414273 | 0.842748 | 1.071402 | — | — | — |
| basic_socket_acceptor_select.test | LINK | 0.420743 | 0.867925 | 1.138936 | — | — | — |
| basic_stream_file.test | LINK | 0.138568 | 0.253233 | 0.296345 | — | — | — |
| basic_stream_file_select.test | LINK | 0.148804 | 0.278669 | 0.299993 | — | — | — |
| basic_stream_socket.test | LINK | 0.421947 | 0.846887 | 1.142183 | — | — | — |
| basic_stream_socket_select.test | LINK | 0.427044 | 0.855735 | 1.180221 | — | — | — |
| basic_streambuf.test | LINK | 0.208935 | 0.362376 | 0.437776 | — | — | — |
| basic_streambuf_select.test | LINK | 0.218538 | 0.371930 | 0.421141 | — | — | — |
| basic_waitable_timer.test | LINK | 0.399573 | 0.748022 | 1.028584 | — | — | — |
| basic_waitable_timer_select.test | LINK | 0.406944 | 0.767687 | 1.032735 | — | — | — |
| basic_writable_pipe.test | LINK | 0.410034 | 0.758250 | 0.933286 | — | — | — |
| basic_writable_pipe_select.test | LINK | 0.436217 | 0.826399 | 1.128340 | — | — | — |
| bind_allocator.test | RUN | 0.664315 | 3.090376 | 2.652718 | 0.002509 | 0.002294 | 0.002357 |
| bind_allocator_select.test | RUN | 0.680557 | 3.136138 | 2.648949 | 0.002244 | 0.002454 | 0.002166 |
| bind_cancellation_slot.test | RUN | 0.664694 | 3.193204 | 2.695116 | 0.001789 | 0.001672 | 0.001683 |
| bind_cancellation_slot_select.test | RUN | 0.678373 | 3.176748 | 2.624571 | 0.001447 | 0.001841 | 0.001565 |
| bind_executor.test | RUN | 0.470313 | 2.290075 | 1.713218 | 0.002375 | 0.002474 | 0.002332 |
| bind_executor_select.test | RUN | 0.475275 | 2.234678 | 1.723879 | 0.002234 | 0.002182 | 0.002203 |
| bind_immediate_executor.test | RUN | 0.364277 | 1.884429 | 1.046519 | 0.001756 | 0.001520 | 0.001406 |
| bind_immediate_executor_select.test | RUN | 0.387523 | 1.847934 | 1.154882 | 0.001439 | 0.001391 | 0.001453 |
| buffer.test | RUN | 0.218605 | 0.733568 | 0.633196 | 0.001552 | 0.001938 | 0.001505 |
| buffer_select.test | RUN | 0.211159 | 0.743546 | 0.637834 | 0.001328 | 0.001452 | 0.001584 |
| buffer_registration.test | LINK | 0.187079 | 0.393094 | 0.393100 | — | — | — |
| buffer_registration_select.test | LINK | 0.189319 | 0.401911 | 0.377689 | — | — | — |
| buffered_read_stream.test | RUN | 0.743677 | 4.860006 | 3.840367 | 0.002125 | 0.002549 | 0.002203 |
| buffered_read_stream_select.test | RUN | 0.736747 | 5.119684 | 3.997574 | 0.002020 | 0.002410 | 0.002388 |
| buffered_stream.test | RUN | 1.180094 | 7.924011 | 6.946380 | 0.002668 | 0.002861 | 0.002484 |
| buffered_stream_select.test | RUN | 1.193194 | 7.788064 | 6.715825 | 0.002514 | 0.002634 | 0.002173 |
| buffered_write_stream.test | RUN | 1.160290 | 6.901585 | 6.098382 | 0.001902 | 0.002661 | 0.002750 |
| buffered_write_stream_select.test | RUN | 1.171700 | 6.875597 | 5.862572 | 0.002211 | 0.002551 | 0.002446 |
| buffers_iterator.test | RUN | 0.295511 | 0.593178 | 0.916111 | 0.001618 | 0.002026 | 0.001960 |
| buffers_iterator_select.test | RUN | 0.293895 | 0.564576 | 0.963606 | 0.001814 | 0.001986 | 0.001788 |
| cancel_after.test | RUN | 1.065341 | 5.196389 | 4.253232 | 0.004265 | 0.005998 | 0.004492 |
| cancel_after_select.test | RUN | 1.072651 | 5.169217 | 4.162467 | 0.004661 | 0.004585 | 0.004060 |
| cancel_at.test | RUN | 1.076812 | 4.886571 | 4.107794 | 0.004292 | 0.004355 | 0.004098 |
| cancel_at_select.test | RUN | 1.073772 | 4.762428 | 4.056939 | 0.003391 | 0.004428 | 0.004021 |
| cancellation_signal.test | LINK | 0.178292 | 0.218035 | 0.240338 | — | — | — |
| cancellation_signal_select.test | LINK | 0.190685 | 0.266110 | 0.303782 | — | — | — |
| cancellation_state.test | LINK | 0.191168 | 0.263976 | 0.373289 | — | — | — |
| cancellation_state_select.test | LINK | 0.192463 | 0.222493 | 0.389203 | — | — | — |
| cancellation_type.test | LINK | 0.150894 | 0.174410 | 0.287574 | — | — | — |
| cancellation_type_select.test | LINK | 0.160448 | 0.184024 | 0.290466 | — | — | — |
| co_composed.test | RUN | 0.160811 | 0.187706 | 0.307448 | 0.001752 | 0.002356 | 0.001877 |
| co_composed_select.test | RUN | 0.160688 | 0.186045 | 0.308101 | 0.001894 | 0.001560 | 0.001519 |
| co_spawn.test | LINK | 0.150419 | 0.183013 | 0.288201 | — | — | — |
| co_spawn_select.test | LINK | 0.147267 | 0.242223 | 0.282391 | — | — | — |
| completion_condition.test | LINK | 0.187313 | 0.387031 | 0.394747 | — | — | — |
| completion_condition_select.test | LINK | 0.190513 | 0.382683 | 0.394761 | — | — | — |
| compose.test | RUN | 0.777688 | 3.873543 | 3.441490 | 0.002701 | 0.002417 | 0.003106 |
| compose_select.test | RUN | 0.784960 | 3.781204 | 3.371872 | 0.002553 | 0.002200 | 0.002437 |
| composed.test | RUN | 0.783541 | 3.794699 | 3.366341 | 0.002084 | 0.002406 | 0.002250 |
| composed_select.test | RUN | 0.781515 | 3.799396 | 3.323411 | 0.002149 | 0.002343 | 0.001787 |
| config.test | RUN | 0.433369 | 2.446883 | 1.492923 | 0.001846 | 0.002231 | 0.001991 |
| config_select.test | RUN | 0.472079 | 2.625404 | 1.674216 | 0.002233 | 0.002260 | 0.001964 |
| connect.test | LINK | 1.238926 | 8.573631 | 7.296502 | — | — | — |
| connect_select.test | LINK | 1.237960 | 8.628257 | 7.302461 | — | — | — |
| connect_pipe.test | RUN | 0.728727 | 3.136108 | 2.571643 | 0.001780 | 0.002355 | 0.001988 |
| connect_pipe_select.test | RUN | 0.730464 | 3.270690 | 2.727902 | 0.002012 | 0.002410 | 0.001969 |
| consign.test | RUN | 0.604550 | 2.988588 | 2.375698 | 0.001888 | 0.002036 | 0.002062 |
| consign_select.test | RUN | 0.591963 | 3.066996 | 2.341993 | 0.001794 | 0.002145 | 0.002032 |
| coroutine.test | LINK | 0.147515 | 0.301170 | 0.376626 | — | — | — |
| coroutine_select.test | LINK | 0.149781 | 0.315307 | 0.338720 | — | — | — |
| deadline_timer.test | RUN | 0.855797 | 4.095891 | 3.376011 | 0.003900 | 0.004057 | 0.003571 |
| deadline_timer_select.test | RUN | 0.869346 | 4.026089 | 3.317152 | 0.003199 | 0.003835 | 0.003705 |
| deferred.test | RUN | 0.161363 | 0.271385 | 0.318508 | 0.001596 | 0.001616 | 0.001889 |
| deferred_select.test | RUN | 0.154228 | 0.267338 | 0.319116 | 0.001822 | 0.001925 | 0.001998 |
| detached.test | LINK | 0.176341 | 0.310121 | 0.354279 | — | — | — |
| detached_select.test | LINK | 0.175320 | 0.311895 | 0.349205 | — | — | — |
| disposition.test | RUN | 0.173122 | 0.700440 | 0.649450 | 0.001426 | 0.001822 | 0.002176 |
| disposition_select.test | RUN | 0.171446 | 0.718339 | 0.660855 | 0.001406 | 0.001880 | 0.002026 |
| error.test | RUN | 0.155028 | 0.757549 | 0.620793 | 0.001492 | 0.001878 | 0.001881 |
| error_select.test | RUN | 0.150667 | 0.760681 | 0.565006 | 0.001349 | 0.001934 | 0.001892 |
| file_base.test | LINK | 0.197774 | 0.600600 | 0.606118 | — | — | — |
| file_base_select.test | LINK | 0.197701 | 0.612983 | 0.555392 | — | — | — |
| fwd_first.test | LINK | 0.467332 | 0.906473 | 1.196066 | — | — | — |
| fwd_last.test | LINK | 0.457405 | 0.873788 | 1.209609 | — | — | — |
| generic_basic_endpoint.test | LINK | 0.171463 | 0.358288 | 0.429904 | — | — | — |
| generic_basic_endpoint_select.test | LINK | 0.172746 | 0.351569 | 0.436129 | — | — | — |
| generic_datagram_protocol.test | LINK | 0.632071 | 3.227230 | 2.484546 | — | — | — |
| generic_datagram_protocol_select.test | LINK | 0.635219 | 3.212004 | 2.561307 | — | — | — |
| generic_raw_protocol.test | LINK | 0.642650 | 3.403477 | 2.461260 | — | — | — |
| generic_raw_protocol_select.test | LINK | 0.623031 | 3.425487 | 2.461093 | — | — | — |
| generic_seq_packet_protocol.test | LINK | 0.554473 | 2.610096 | 2.136702 | — | — | — |
| generic_seq_packet_protocol_select.test | LINK | 0.554717 | 2.672935 | 2.135185 | — | — | — |
| generic_stream_protocol.test | LINK | 0.597470 | 2.908432 | 2.244307 | — | — | — |
| generic_stream_protocol_select.test | LINK | 0.598972 | 3.026985 | 2.244755 | — | — | — |
| high_resolution_timer.test | LINK | 0.285114 | 0.536710 | 0.688295 | — | — | — |
| high_resolution_timer_select.test | LINK | 0.299042 | 0.550334 | 0.732357 | — | — | — |
| immediate.test | LINK | 0.195909 | 0.408985 | 0.421506 | — | — | — |
| immediate_select.test | LINK | 0.196581 | 0.396471 | 0.418621 | — | — | — |
| inline_executor.test | RUN | 0.232370 | 0.652504 | 0.602833 | 0.001572 | 0.001672 | 0.001584 |
| inline_executor_select.test | RUN | 0.230566 | 0.645608 | 0.609471 | 0.001406 | 0.001592 | 0.001487 |
| inline_or_executor.test | RUN | 0.407297 | 1.862769 | 1.236939 | 0.001597 | 0.001675 | 0.001472 |
| inline_or_executor_select.test | RUN | 0.405889 | 1.974444 | 1.332803 | 0.001383 | 0.001586 | 0.001447 |
| io_context.test | RUN | 0.519253 | 2.805989 | 2.114090 | 0.002703 | 0.002413 | 0.002904 |
| io_context_select.test | RUN | 0.526094 | 2.756525 | 2.157497 | 0.002377 | 0.002475 | 0.002365 |
| io_context_strand.test | RUN | 0.505458 | 2.447891 | 1.960050 | 0.002580 | 0.002527 | 0.002626 |
| io_context_strand_select.test | RUN | 0.506771 | 2.396736 | 1.953971 | 0.002271 | 0.002326 | 0.002891 |
| ip_address.test | LINK | 0.211936 | 0.556750 | 0.498887 | — | — | — |
| ip_address_select.test | LINK | 0.218499 | 0.554316 | 0.492210 | — | — | — |
| ip_address_v4.test | LINK | 0.224924 | 0.587628 | 0.548019 | — | — | — |
| ip_address_v4_select.test | LINK | 0.231275 | 0.574266 | 0.563318 | — | — | — |
| ip_address_v6.test | LINK | 0.224883 | 0.791703 | 0.966463 | — | — | — |
| ip_address_v6_select.test | LINK | 0.221413 | 0.757117 | 0.853038 | — | — | — |
| ip_basic_endpoint.test | LINK | 0.185072 | 0.375174 | 0.695264 | — | — | — |
| ip_basic_endpoint_select.test | LINK | 0.183848 | 0.428832 | 0.671423 | — | — | — |
| ip_basic_resolver.test | LINK | 0.295873 | 0.526913 | 0.998837 | — | — | — |
| ip_basic_resolver_select.test | LINK | 0.298755 | 0.531000 | 1.077004 | — | — | — |
| ip_basic_resolver_entry.test | LINK | 0.109858 | 0.170341 | 0.298947 | — | — | — |
| ip_basic_resolver_entry_select.test | LINK | 0.110421 | 0.168412 | 0.293494 | — | — | — |
| ip_basic_resolver_iterator.test | LINK | 0.193673 | 0.350902 | 0.680888 | — | — | — |
| ip_basic_resolver_iterator_select.test | LINK | 0.187699 | 0.357287 | 0.649324 | — | — | — |
| ip_basic_resolver_query.test | LINK | 0.186691 | 0.361057 | 0.619931 | — | — | — |
| ip_basic_resolver_query_select.test | LINK | 0.184328 | 0.352151 | 0.645016 | — | — | — |
| ip_host_name.test | RUN | 0.187111 | 0.483011 | 0.750377 | 0.001516 | 0.001926 | 0.002192 |
| ip_host_name_select.test | RUN | 0.189973 | 0.560345 | 0.727631 | 0.001318 | 0.002018 | 0.001914 |
| ip_icmp.test | RUN | 0.809921 | 6.570957 | 5.179378 | 0.001576 | 0.001858 | 0.001755 |
| ip_icmp_select.test | RUN | 0.814950 | 6.631567 | 5.030100 | 0.001320 | 0.001848 | 0.001533 |
| ip_multicast.test | RUN | 0.491894 | 3.316516 | 2.788407 | 0.001728 | 0.002259 | 0.002216 |
| ip_multicast_select.test | RUN | 0.496382 | 3.263318 | 2.766251 | 0.001694 | 0.002126 | 0.002070 |
| ip_resolver_query_base.test | LINK | 0.118648 | 0.290377 | 0.301527 | — | — | — |
| ip_resolver_query_base_select.test | LINK | 0.117518 | 0.289198 | 0.309869 | — | — | — |
| ip_tcp.test | RUN | 1.214107 | 9.362852 | 7.457474 | 0.002326 | 0.002576 | 0.002632 |
| ip_tcp_select.test | RUN | 1.192190 | 9.698724 | 7.260420 | 0.002192 | 0.002644 | 0.002286 |
| ip_udp.test | RUN | 0.878420 | 6.648070 | 5.008808 | 0.001642 | 0.001768 | 0.002020 |
| ip_udp_select.test | RUN | 0.850790 | 6.743154 | 4.892052 | 0.001766 | 0.001692 | 0.002251 |
| ip_unicast.test | RUN | 0.532225 | 2.799984 | 2.231364 | 0.001824 | 0.002155 | 0.001912 |
| ip_unicast_select.test | RUN | 0.516835 | 2.850759 | 2.235596 | 0.001632 | 0.001908 | 0.001912 |
| ip_v6_only.test | RUN | 0.472989 | 2.921720 | 2.379626 | 0.001644 | 0.001680 | 0.001498 |
| ip_v6_only_select.test | RUN | 0.485927 | 3.126101 | 2.299480 | 0.001642 | 0.002048 | 0.002044 |
| is_read_buffered.test | RUN | 0.337905 | 0.880703 | 1.223309 | 0.001947 | 0.001575 | 0.001926 |
| is_read_buffered_select.test | RUN | 0.351137 | 0.878549 | 1.229239 | 0.001547 | 0.001956 | 0.001726 |
| is_write_buffered.test | RUN | 0.352486 | 0.855915 | 1.173406 | 0.001762 | 0.001856 | 0.001636 |
| is_write_buffered_select.test | RUN | 0.375965 | 0.900491 | 1.173058 | 0.001564 | 0.001920 | 0.001709 |
| local_basic_endpoint.test | LINK | 0.180605 | 0.483449 | 0.619392 | — | — | — |
| local_basic_endpoint_select.test | LINK | 0.202302 | 0.494817 | 0.613349 | — | — | — |
| local_connect_pair.test | LINK | 0.596476 | 2.993426 | 2.284793 | — | — | — |
| local_connect_pair_select.test | LINK | 0.610650 | 2.966283 | 2.271390 | — | — | — |
| local_datagram_protocol.test | LINK | 0.798315 | 4.016180 | 3.280363 | — | — | — |
| local_datagram_protocol_select.test | LINK | 0.805661 | 4.039651 | 3.215337 | — | — | — |
| local_seq_packet_protocol.test | LINK | 0.773969 | 3.913611 | 3.263321 | — | — | — |
| local_seq_packet_protocol_select.test | LINK | 0.765644 | 3.892496 | 3.244550 | — | — | — |
| local_stream_protocol.test | LINK | 0.739680 | 3.650891 | 3.056162 | — | — | — |
| local_stream_protocol_select.test | LINK | 0.742046 | 3.640638 | 3.039563 | — | — | — |
| placeholders.test | LINK | 0.159259 | 0.281851 | 0.309438 | — | — | — |
| placeholders_select.test | LINK | 0.167553 | 0.255689 | 0.313844 | — | — | — |
| posix_basic_descriptor.test | LINK | 0.398757 | 0.724599 | 0.961382 | — | — | — |
| posix_basic_descriptor_select.test | LINK | 0.409292 | 0.792452 | 1.188194 | — | — | — |
| posix_basic_stream_descriptor.test | LINK | 0.399668 | 0.813784 | 1.005400 | — | — | — |
| posix_basic_stream_descriptor_select.test | LINK | 0.419850 | 0.871062 | 1.235212 | — | — | — |
| posix_descriptor_base.test | LINK | 0.177448 | 0.288690 | 0.388120 | — | — | — |
| posix_descriptor_base_select.test | LINK | 0.174333 | 0.302311 | 0.346261 | — | — | — |
| posix_stream_descriptor.test | LINK | 0.750525 | 3.709668 | 3.157177 | — | — | — |
| posix_stream_descriptor_select.test | LINK | 0.757594 | 3.699094 | 3.179996 | — | — | — |
| prepend.test | RUN | 0.609111 | 3.037331 | 2.451882 | 0.002090 | 0.001690 | 0.001947 |
| prepend_select.test | RUN | 0.619029 | 3.021269 | 2.435838 | 0.002230 | 0.001971 | 0.001693 |
| random_access_file.test | LINK | 0.384499 | 0.669859 | 0.772098 | — | — | — |
| random_access_file_select.test | LINK | 0.388582 | 0.725156 | 0.975875 | — | — | — |
| read.test | RUN | 1.786437 | 19.891732 | 14.583342 | 0.002680 | 0.002577 | 0.002378 |
| read_select.test | RUN | 1.811484 | 19.896917 | 14.654805 | 0.002563 | 0.002635 | 0.002411 |
| read_at.test | RUN | 1.820459 | 30.132592 | 20.591453 | 0.003217 | 0.002940 | 0.002715 |
| read_at_select.test | RUN | 1.813400 | 29.885802 | 20.982164 | 0.002950 | 0.002703 | 0.002890 |
| read_until.test | RUN | 1.004004 | 7.214850 | 4.965238 | 0.002341 | 0.002238 | 0.001946 |
| read_until_select.test | RUN | 1.061861 | 7.252219 | 5.160479 | 0.002275 | 0.002154 | 0.002491 |
| readable_pipe.test | LINK | 0.669554 | 2.931820 | 2.498965 | — | — | — |
| readable_pipe_select.test | LINK | 0.670012 | 3.171730 | 2.632502 | — | — | — |
| recycling_allocator.test | RUN | 0.211920 | 0.443577 | 0.478565 | 0.001786 | 0.001956 | 0.001743 |
| recycling_allocator_select.test | RUN | 0.206846 | 0.434335 | 0.479222 | 0.001556 | 0.001895 | 0.001792 |
| redirect_disposition.test | LINK | 0.824485 | 4.155519 | 3.710912 | — | — | — |
| redirect_disposition_select.test | LINK | 0.827035 | 4.085109 | 3.602626 | — | — | — |
| redirect_error.test | LINK | 0.741813 | 3.401107 | 3.145524 | — | — | — |
| redirect_error_select.test | LINK | 0.746803 | 3.439617 | 3.111403 | — | — | — |
| registered_buffer.test | LINK | 0.217397 | 0.357587 | 0.401927 | — | — | — |
| registered_buffer_select.test | LINK | 0.220871 | 0.340174 | 0.397017 | — | — | — |
| signal_set.test | RUN | 0.597766 | 2.813515 | 2.226042 | 0.001983 | 0.001779 | 0.001871 |
| signal_set_select.test | RUN | 0.615484 | 2.904293 | 2.487989 | 0.001556 | 0.002035 | 0.001930 |
| signal_set_base.test | LINK | 0.169413 | 0.270215 | 0.329306 | — | — | — |
| signal_set_base_select.test | LINK | 0.173868 | 0.274384 | 0.316611 | — | — | — |
| socket_base.test | RUN | 0.755714 | 4.842485 | 3.604503 | 0.002292 | 0.002458 | 0.002082 |
| socket_base_select.test | RUN | 0.755819 | 4.646199 | 3.537416 | 0.002018 | 0.001938 | 0.002209 |
| spawn.test | RUN | 1.213816 | 6.975629 | 6.041072 | 0.002256 | 0.002130 | 0.002036 |
| spawn_select.test | RUN | 1.186010 | 6.973843 | 6.036533 | 0.002409 | 0.001958 | 0.002148 |
| static_thread_pool.test | RUN | 0.362494 | 0.643235 | 0.763874 | 0.001505 | 0.001809 | 0.001955 |
| static_thread_pool_select.test | RUN | 0.380183 | 0.671453 | 0.945838 | 0.001686 | 0.001740 | 0.001851 |
| steady_timer.test | LINK | 0.383218 | 0.794597 | 1.121728 | — | — | — |
| steady_timer_select.test | LINK | 0.394840 | 0.827808 | 1.108473 | — | — | — |
| strand.test | RUN | 0.818553 | 3.934664 | 3.578015 | 0.002719 | 0.003205 | 0.002930 |
| strand_select.test | RUN | 0.838198 | 3.994324 | 3.561238 | 0.002456 | 0.003100 | 0.002409 |
| stream_file.test | LINK | 0.372618 | 0.688725 | 0.752197 | — | — | — |
| stream_file_select.test | LINK | 0.393569 | 0.741231 | 1.021788 | — | — | — |
| streambuf.test | RUN | 0.168225 | 0.571972 | 0.576627 | 0.001559 | 0.001394 | 0.001919 |
| streambuf_select.test | RUN | 0.178249 | 0.514165 | 0.481974 | 0.001398 | 0.001310 | 0.001844 |
| system_timer.test | LINK | 0.518041 | 2.636590 | 2.249287 | — | — | — |
| system_timer_select.test | LINK | 0.523366 | 2.582328 | 2.104650 | — | — | — |
| system_context.test | LINK | 0.253802 | 0.507890 | 0.483529 | — | — | — |
| system_context_select.test | LINK | 0.261752 | 0.538745 | 0.604832 | — | — | — |
| system_executor.test | LINK | 0.323471 | 1.483463 | 0.952783 | — | — | — |
| system_executor_select.test | LINK | 0.337940 | 1.412300 | 1.068207 | — | — | — |
| this_coro.test | LINK | 0.101622 | 0.174462 | 0.194026 | — | — | — |
| this_coro_select.test | LINK | 0.102264 | 0.176065 | 0.202809 | — | — | — |
| thread_pool.test | RUN | 0.381645 | 1.891524 | 1.298203 | 0.002510 | 0.002461 | 0.002666 |
| thread_pool_select.test | RUN | 0.390750 | 1.873950 | 1.378123 | 0.002641 | 0.002442 | 0.002596 |
| time_traits.test | LINK | 0.227577 | 0.345451 | 0.377589 | — | — | — |
| time_traits_select.test | LINK | 0.226023 | 0.348483 | 0.380076 | — | — | — |
| ts_buffer.test | LINK | 0.218377 | 0.588692 | 0.415003 | — | — | — |
| ts_buffer_select.test | LINK | 0.216928 | 0.584782 | 0.403574 | — | — | — |
| ts_executor.test | LINK | 0.299659 | 0.829154 | 0.672940 | — | — | — |
| ts_executor_select.test | LINK | 0.313173 | 0.869900 | 0.809478 | — | — | — |
| ts_internet.test | LINK | 0.336074 | 0.864424 | 1.086103 | — | — | — |
| ts_internet_select.test | LINK | 0.343804 | 0.890255 | 1.217251 | — | — | — |
| ts_io_context.test | LINK | 0.291556 | 0.668475 | 0.730189 | — | — | — |
| ts_io_context_select.test | LINK | 0.296313 | 0.714141 | 0.916699 | — | — | — |
| ts_net.test | LINK | 0.399330 | 1.052552 | 1.480911 | — | — | — |
| ts_net_select.test | LINK | 0.408573 | 1.088127 | 1.516679 | — | — | — |
| ts_netfwd.test | LINK | 0.399134 | 1.017538 | 1.508240 | — | — | — |
| ts_netfwd_select.test | LINK | 0.419721 | 1.080284 | 1.513038 | — | — | — |
| ts_socket.test | LINK | 0.341615 | 0.852465 | 1.155337 | — | — | — |
| ts_socket_select.test | LINK | 0.330287 | 0.869557 | 1.191469 | — | — | — |
| ts_timer.test | LINK | 0.285062 | 0.750537 | 0.991550 | — | — | — |
| ts_timer_select.test | LINK | 0.290294 | 0.773288 | 0.996674 | — | — | — |
| use_awaitable.test | LINK | 0.099348 | 0.281862 | 0.269044 | — | — | — |
| use_awaitable_select.test | LINK | 0.104161 | 0.271604 | 0.270375 | — | — | — |
| wait_traits.test | LINK | 0.102861 | 0.276673 | 0.281107 | — | — | — |
| wait_traits_select.test | LINK | 0.101731 | 0.288250 | 0.280613 | — | — | — |
| windows_basic_object_handle.test | LINK | 0.101361 | 0.255441 | 0.275577 | — | — | — |
| windows_basic_object_handle_select.test | LINK | 0.103433 | 0.249356 | 0.281113 | — | — | — |
| windows_basic_overlapped_handle.test | LINK | 0.101576 | 0.244046 | 0.282536 | — | — | — |
| windows_basic_overlapped_handle_select.test | LINK | 0.106125 | 0.249239 | 0.300148 | — | — | — |
| windows_basic_random_access_handle.test | LINK | 0.102367 | 0.240392 | 0.287881 | — | — | — |
| windows_basic_random_access_handle_select.test | LINK | 0.113576 | 0.238321 | 0.286098 | — | — | — |
| windows_basic_stream_handle.test | LINK | 0.138036 | 0.240413 | 0.279662 | — | — | — |
| windows_basic_stream_handle_select.test | LINK | 0.143291 | 0.249787 | 0.261522 | — | — | — |
| windows_object_handle.test | LINK | 0.375080 | 0.692274 | 0.731430 | — | — | — |
| windows_object_handle_select.test | LINK | 0.387743 | 0.735601 | 0.917583 | — | — | — |
| windows_overlapped_ptr.test | LINK | 0.399785 | 0.725081 | 0.793697 | — | — | — |
| windows_overlapped_ptr_select.test | LINK | 0.400763 | 0.782087 | 0.984712 | — | — | — |
| windows_random_access_handle.test | LINK | 0.371568 | 0.698675 | 0.745746 | — | — | — |
| windows_random_access_handle_select.test | LINK | 0.389249 | 0.741228 | 0.941365 | — | — | — |
| windows_stream_handle.test | LINK | 0.365351 | 0.683414 | 0.754928 | — | — | — |
| windows_stream_handle_select.test | LINK | 0.378989 | 0.741244 | 0.944312 | — | — | — |
| writable_pipe.test | LINK | 0.621120 | 2.989924 | 2.431774 | — | — | — |
| writable_pipe_select.test | LINK | 0.637495 | 3.129604 | 2.586976 | — | — | — |
| write.test | RUN | 1.757022 | 16.905149 | 13.669589 | 0.002226 | 0.002578 | 0.002794 |
| write_select.test | RUN | 1.766370 | 16.948008 | 13.725677 | 0.002349 | 0.002678 | 0.002376 |
| write_at.test | RUN | 1.847263 | 26.343069 | 17.971419 | 0.003048 | 0.002607 | 0.002805 |
| write_at_select.test | RUN | 1.874076 | 26.175985 | 18.463581 | 0.004302 | 0.002779 | 0.002706 |
| any_executor.test | RUN | 0.741985 | 8.818284 | 5.655525 | 0.002622 | 0.002545 | 0.002563 |
| blocking.test | RUN | 1.682327 | 9.122924 | 10.282036 | 0.006855 | 0.006050 | 0.006741 |
| blocking_adaptation.test | RUN | 0.900310 | 3.575341 | 4.940314 | 0.003822 | 0.003420 | 0.003895 |
| context_as.test | RUN | 0.556437 | 2.491596 | 1.774503 | 0.002082 | 0.001625 | 0.002072 |
| executor.test | RUN | 0.167805 | 0.247701 | 0.326103 | 0.001910 | 0.001862 | 0.001784 |
| inline_exception_handling.test | RUN | 1.602573 | 7.481630 | 9.894030 | 0.006526 | 0.005054 | 0.004925 |
| invocable_archetype.test | RUN | 0.152653 | 0.253227 | 0.296052 | 0.002220 | 0.002242 | 0.001794 |
| mapping.test | RUN | 1.571808 | 8.251070 | 10.419160 | 0.005428 | 0.005001 | 0.004890 |
| outstanding_work.test | RUN | 0.758326 | 3.705710 | 4.600727 | 0.004356 | 0.003735 | 0.003512 |
| prefer_only.test | RUN | 0.292145 | 1.557787 | 1.088866 | 0.002005 | 0.001915 | 0.001814 |
| relationship.test | RUN | 0.588650 | 3.405298 | 4.649000 | 0.003184 | 0.004064 | 0.003990 |

## Native runtime replay

Child CPU of successful native tests, with process startup included; GDB and expected-failure executions are correctness-only. Two warmups per compiler, four A/A observations per reference and mirrored ABBA/BAAB blocks. Short tests repeat up to ten times, identically across compilers; CPU is divided by repetitions before summing tests. Executable hashes and output checks stay outside process timers. All runs must pass their original native oracle. Output hashes record equality or variation across the complete campaign; test reports may print times or addresses.

| Reference | CPU speedup ↑ | Paired blocks | A/A speedup |
|---|---:|---|---:|
| gcc | 1.010×† | 1.027×, 0.992× | 1.053× |
| clang | 0.985× | 0.978×, 0.992× | 1.008× |

† A/A drift or paired-block spread exceeds 5%; small differences are unresolved. 1375 native tests in the full cohort; 1361 have byte-identical output throughout. [Manifest](https://github.com/vishvananda/cppgm-extended/actions/runs/42951039903), [paired observations](https://github.com/vishvananda/cppgm-extended/actions/runs/42951039903) and [per-test summaries](https://github.com/vishvananda/cppgm-extended/actions/runs/42951039903).

| Test | Repeats | CPU cppgm | GCC | Clang | Output check |
|---|---:|---:|---:|---:|---|
| any_completion_executor.test | 4 | 0.001555s | 0.001569s | 0.001590s | byte equality observed |
| any_completion_executor_select.test | 3 | 0.001595s | 0.001569s | 0.001592s | byte equality observed |
| any_completion_handler.test | 9 | 0.001275s | 0.001263s | 0.001277s | byte equality observed |
| any_completion_handler_select.test | 8 | 0.001280s | 0.001241s | 0.001241s | byte equality observed |
| any_io_executor.test | 5 | 0.001564s | 0.001597s | 0.001580s | byte equality observed |
| any_io_executor_select.test | 5 | 0.001535s | 0.001589s | 0.001571s | byte equality observed |
| append.test | 10 | 0.001220s | 0.001188s | 0.001180s | byte equality observed |
| append_select.test | 10 | 0.001179s | 0.001171s | 0.001164s | byte equality observed |
| as_tuple.test | 8 | 0.001280s | 0.001267s | 0.001308s | byte equality observed |
| as_tuple_select.test | 8 | 0.001211s | 0.001202s | 0.001235s | byte equality observed |
| bind_allocator.test | 1 | 0.001724s | 0.001652s | 0.001799s | byte equality observed |
| bind_allocator_select.test | 1 | 0.001455s | 0.001462s | 0.001451s | byte equality observed |
| bind_cancellation_slot.test | 10 | 0.001180s | 0.001194s | 0.001205s | byte equality observed |
| bind_cancellation_slot_select.test | 10 | 0.001033s | 0.001031s | 0.001056s | byte equality observed |
| bind_executor.test | 1 | 0.001773s | 0.001774s | 0.001796s | byte equality observed |
| bind_executor_select.test | 1 | 0.002031s | 0.001988s | 0.002101s | byte equality observed |
| bind_immediate_executor.test | 10 | 0.001093s | 0.001023s | 0.001082s | byte equality observed |
| bind_immediate_executor_select.test | 10 | 0.001052s | 0.000991s | 0.001050s | byte equality observed |
| buffer.test | 10 | 0.001000s | 0.000936s | 0.001018s | byte equality observed |
| buffer_select.test | 10 | 0.000985s | 0.000966s | 0.001001s | byte equality observed |
| buffered_read_stream.test | 10 | 0.001308s | 0.001266s | 0.001332s | byte equality observed |
| buffered_read_stream_select.test | 10 | 0.001262s | 0.001236s | 0.001328s | byte equality observed |
| buffered_stream.test | 10 | 0.001290s | 0.001201s | 0.001310s | byte equality observed |
| buffered_stream_select.test | 10 | 0.001282s | 0.001196s | 0.001298s | byte equality observed |
| buffered_write_stream.test | 10 | 0.001297s | 0.001225s | 0.001298s | byte equality observed |
| buffered_write_stream_select.test | 10 | 0.001275s | 0.001203s | 0.001311s | byte equality observed |
| buffers_iterator.test | 10 | 0.000994s | 0.000970s | 0.001011s | byte equality observed |
| buffers_iterator_select.test | 10 | 0.000980s | 0.000897s | 0.001006s | byte equality observed |
| cancel_after.test | 1 | 0.003914s | 0.003677s | 0.003563s | byte equality observed |
| cancel_after_select.test | 1 | 0.003050s | 0.003042s | 0.003078s | byte equality observed |
| cancel_at.test | 1 | 0.004074s | 0.004123s | 0.004176s | byte equality observed |
| cancel_at_select.test | 1 | 0.004022s | 0.003719s | 0.004272s | byte equality observed |
| co_composed.test | 10 | 0.001029s | 0.001007s | 0.001054s | byte equality observed |
| co_composed_select.test | 10 | 0.000967s | 0.000935s | 0.001010s | byte equality observed |
| compose.test | 1 | 0.001738s | 0.001598s | 0.001732s | byte equality observed |
| compose_select.test | 1 | 0.001431s | 0.001440s | 0.001452s | byte equality observed |
| composed.test | 1 | 0.002173s | 0.002097s | 0.002203s | byte equality observed |
| composed_select.test | 1 | 0.001600s | 0.001621s | 0.001816s | byte equality observed |
| config.test | 10 | 0.001128s | 0.001094s | 0.001109s | byte equality observed |
| config_select.test | 10 | 0.001062s | 0.001092s | 0.001017s | byte equality observed |
| connect_pipe.test | 10 | 0.001085s | 0.001073s | 0.001063s | byte equality observed |
| connect_pipe_select.test | 10 | 0.001070s | 0.001065s | 0.001057s | byte equality observed |
| consign.test | 10 | 0.001055s | 0.001062s | 0.001055s | byte equality observed |
| consign_select.test | 10 | 0.001055s | 0.001009s | 0.001022s | byte equality observed |
| deadline_timer.test | 1 | 0.002479s | 0.002346s | 0.002548s | byte equality observed |
| deadline_timer_select.test | 1 | 0.002342s | 0.002204s | 0.002373s | byte equality observed |
| deferred.test | 10 | 0.000968s | 0.000958s | 0.000965s | byte equality observed |
| deferred_select.test | 10 | 0.000911s | 0.000924s | 0.000914s | byte equality observed |
| disposition.test | 10 | 0.000988s | 0.000983s | 0.000988s | byte equality observed |
| disposition_select.test | 10 | 0.000983s | 0.000964s | 0.001076s | byte equality observed |
| error.test | 10 | 0.000953s | 0.000927s | 0.000934s | byte equality observed |
| error_select.test | 10 | 0.000943s | 0.000933s | 0.000941s | byte equality observed |
| inline_executor.test | 10 | 0.000953s | 0.000963s | 0.000989s | byte equality observed |
| inline_executor_select.test | 10 | 0.000950s | 0.000970s | 0.001015s | byte equality observed |
| inline_or_executor.test | 10 | 0.000967s | 0.000971s | 0.001094s | byte equality observed |
| inline_or_executor_select.test | 10 | 0.000953s | 0.000971s | 0.000986s | byte equality observed |
| io_context.test | 1 | 0.001968s | 0.002008s | 0.001913s | byte equality observed |
| io_context_select.test | 1 | 0.002352s | 0.002309s | 0.002433s | byte equality observed |
| io_context_strand.test | 1 | 0.002382s | 0.002385s | 0.002397s | byte equality observed |
| io_context_strand_select.test | 1 | 0.002092s | 0.002054s | 0.002066s | byte equality observed |
| ip_host_name.test | 10 | 0.000941s | 0.000980s | 0.000985s | byte equality observed |
| ip_host_name_select.test | 10 | 0.000936s | 0.000948s | 0.000925s | byte equality observed |
| ip_icmp.test | 10 | 0.000950s | 0.000984s | 0.000941s | byte equality observed |
| ip_icmp_select.test | 10 | 0.000941s | 0.000984s | 0.000977s | byte equality observed |
| ip_multicast.test | 10 | 0.001016s | 0.001083s | 0.001040s | byte equality observed |
| ip_multicast_select.test | 10 | 0.000995s | 0.001034s | 0.000980s | byte equality observed |
| ip_tcp.test | 10 | 0.001428s | 0.001443s | 0.001403s | byte equality observed |
| ip_tcp_select.test | 10 | 0.001362s | 0.001377s | 0.001300s | byte equality observed |
| ip_udp.test | 10 | 0.001068s | 0.001053s | 0.001034s | byte equality observed |
| ip_udp_select.test | 10 | 0.001033s | 0.001050s | 0.001038s | byte equality observed |
| ip_unicast.test | 10 | 0.001011s | 0.001055s | 0.001021s | byte equality observed |
| ip_unicast_select.test | 10 | 0.000976s | 0.001061s | 0.000969s | byte equality observed |
| ip_v6_only.test | 10 | 0.000989s | 0.001049s | 0.001007s | byte equality observed |
| ip_v6_only_select.test | 10 | 0.000980s | 0.001056s | 0.001057s | byte equality observed |
| is_read_buffered.test | 10 | 0.000894s | 0.000976s | 0.000973s | byte equality observed |
| is_read_buffered_select.test | 10 | 0.000900s | 0.000964s | 0.000947s | byte equality observed |
| is_write_buffered.test | 10 | 0.000933s | 0.000972s | 0.000953s | byte equality observed |
| is_write_buffered_select.test | 10 | 0.000961s | 0.000969s | 0.000967s | byte equality observed |
| prepend.test | 10 | 0.001038s | 0.001063s | 0.001089s | byte equality observed |
| prepend_select.test | 10 | 0.001029s | 0.001044s | 0.001065s | byte equality observed |
| read.test | 10 | 0.001501s | 0.001377s | 0.001386s | byte equality observed |
| read_at.test | 10 | 0.001743s | 0.001648s | 0.001550s | byte equality observed |
| read_at_select.test | 10 | 0.001745s | 0.001661s | 0.001560s | byte equality observed |
| read_select.test | 10 | 0.001476s | 0.001393s | 0.001412s | byte equality observed |
| read_until.test | 10 | 0.001209s | 0.001133s | 0.001156s | byte equality observed |
| read_until_select.test | 10 | 0.001205s | 0.001176s | 0.001148s | byte equality observed |
| recycling_allocator.test | 10 | 0.000999s | 0.000988s | 0.000968s | byte equality observed |
| recycling_allocator_select.test | 10 | 0.000953s | 0.000964s | 0.000979s | byte equality observed |
| signal_set.test | 10 | 0.000984s | 0.000996s | 0.001014s | byte equality observed |
| signal_set_select.test | 10 | 0.001007s | 0.001034s | 0.001002s | byte equality observed |
| socket_base.test | 10 | 0.001129s | 0.001111s | 0.001113s | byte equality observed |
| socket_base_select.test | 10 | 0.001090s | 0.001105s | 0.001062s | byte equality observed |
| spawn.test | 10 | 0.001207s | 0.001171s | 0.001205s | byte equality observed |
| spawn_select.test | 10 | 0.001219s | 0.001173s | 0.001158s | byte equality observed |
| static_thread_pool.test | 10 | 0.000996s | 0.000956s | 0.001005s | byte equality observed |
| static_thread_pool_select.test | 10 | 0.000942s | 0.000944s | 0.000978s | byte equality observed |
| strand.test | 1 | 0.001972s | 0.001990s | 0.001993s | byte equality observed |
| strand_select.test | 1 | 0.001682s | 0.001648s | 0.001690s | byte equality observed |
| streambuf.test | 10 | 0.001007s | 0.001007s | 0.001039s | byte equality observed |
| streambuf_select.test | 10 | 0.000995s | 0.001021s | 0.001028s | byte equality observed |
| thread_pool.test | 10 | 0.002013s | 0.002032s | 0.002200s | byte equality observed |
| thread_pool_select.test | 10 | 0.001992s | 0.001995s | 0.002195s | byte equality observed |
| write.test | 10 | 0.001539s | 0.001394s | 0.001437s | byte equality observed |
| write_at.test | 10 | 0.001635s | 0.001547s | 0.001577s | byte equality observed |
| write_at_select.test | 10 | 0.001643s | 0.001537s | 0.001623s | byte equality observed |
| write_select.test | 10 | 0.001516s | 0.001417s | 0.001492s | byte equality observed |

Compiler command and timing observations: [cppgm](https://github.com/vishvananda/cppgm-extended/actions/runs/42951039903) · [gcc](https://github.com/vishvananda/cppgm-extended/actions/runs/42951039903) · [clang](https://github.com/vishvananda/cppgm-extended/actions/runs/42951039903).

[Reviewed outcomes and per-test records](../results.json) · [Host and toolchains](../metadata.json)
