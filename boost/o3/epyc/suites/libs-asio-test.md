# libs/asio/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| any_completion_executor.test | RUN | 0.858966 | 5.189687 | 5.218167 | 0.002872 | 0.002934 | 0.003046 |
| any_completion_executor_select.test | RUN | 0.880916 | 5.142749 | 5.515901 | 0.003409 | 0.002739 | 0.002854 |
| any_completion_handler.test | RUN | 1.006967 | 4.440125 | 4.703448 | 0.002231 | 0.002461 | 0.002261 |
| any_completion_handler_select.test | RUN | 0.977697 | 4.800269 | 5.168802 | 0.002318 | 0.002512 | 0.002264 |
| any_io_executor.test | RUN | 0.895555 | 5.514496 | 5.671952 | 0.002987 | 0.002693 | 0.002655 |
| any_io_executor_select.test | RUN | 0.952012 | 5.398259 | 5.953495 | 0.002537 | 0.003068 | 0.003142 |
| append.test | RUN | 0.867808 | 4.279913 | 4.290751 | 0.001924 | 0.002352 | 0.002663 |
| append_select.test | RUN | 0.852306 | 4.374182 | 4.230809 | 0.002203 | 0.002422 | 0.002310 |
| as_tuple.test | RUN | 1.071492 | 5.179895 | 5.888326 | 0.002532 | 0.002558 | 0.002218 |
| as_tuple_select.test | RUN | 1.052330 | 5.140637 | 5.920316 | 0.002395 | 0.002839 | 0.002214 |
| associated_allocator.test | LINK | 0.234581 | 0.502750 | 0.728144 | — | — | — |
| associated_allocator_select.test | LINK | 0.241753 | 0.494523 | 0.742874 | — | — | — |
| associated_cancellation_slot.test | LINK | 0.252566 | 0.534135 | 0.774068 | — | — | — |
| associated_cancellation_slot_select.test | LINK | 0.261999 | 0.552683 | 0.801500 | — | — | — |
| associated_executor.test | LINK | 0.361810 | 0.893800 | 1.286190 | — | — | — |
| associated_executor_select.test | LINK | 0.372942 | 0.927364 | 1.276627 | — | — | — |
| associated_immediate_executor.test | LINK | 0.342074 | 0.826315 | 1.103768 | — | — | — |
| associated_immediate_executor_select.test | LINK | 0.349626 | 0.836411 | 1.089010 | — | — | — |
| associator.test | LINK | 0.200903 | 0.380657 | 0.599979 | — | — | — |
| associator_select.test | LINK | 0.194553 | 0.397758 | 0.599631 | — | — | — |
| awaitable.test | LINK | 0.190961 | 0.402986 | 0.594274 | — | — | — |
| awaitable_select.test | LINK | 0.202632 | 0.403849 | 0.605417 | — | — | — |
| basic_datagram_socket.test | LINK | 0.560664 | 1.242257 | 2.310350 | — | — | — |
| basic_datagram_socket_select.test | LINK | 0.571468 | 1.276412 | 2.303911 | — | — | — |
| basic_deadline_timer.test | LINK | 0.674888 | 1.428410 | 2.671632 | — | — | — |
| basic_deadline_timer_select.test | LINK | 0.699081 | 1.498032 | 2.714058 | — | — | — |
| basic_file.test | LINK | 0.184913 | 0.398467 | 0.589841 | — | — | — |
| basic_file_select.test | LINK | 0.181813 | 0.410844 | 0.576569 | — | — | — |
| basic_random_access_file.test | LINK | 0.190414 | 0.397587 | 0.569674 | — | — | — |
| basic_random_access_file_select.test | LINK | 0.184956 | 0.381080 | 0.606238 | — | — | — |
| basic_raw_socket.test | LINK | 0.567611 | 1.215710 | 2.327085 | — | — | — |
| basic_raw_socket_select.test | LINK | 0.590865 | 1.246406 | 2.377836 | — | — | — |
| basic_readable_pipe.test | LINK | 0.552743 | 1.172942 | 1.941983 | — | — | — |
| basic_readable_pipe_select.test | LINK | 0.572516 | 1.265542 | 2.342548 | — | — | — |
| basic_seq_packet_socket.test | LINK | 0.574944 | 1.229660 | 2.311281 | — | — | — |
| basic_seq_packet_socket_select.test | LINK | 0.589439 | 1.250976 | 2.351219 | — | — | — |
| basic_signal_set.test | LINK | 0.523492 | 1.128539 | 1.708965 | — | — | — |
| basic_signal_set_select.test | LINK | 0.581000 | 1.229168 | 2.164458 | — | — | — |
| basic_socket_acceptor.test | LINK | 0.590511 | 1.242204 | 2.263562 | — | — | — |
| basic_socket_acceptor_select.test | LINK | 0.583053 | 1.275055 | 2.375078 | — | — | — |
| basic_stream_file.test | LINK | 0.197869 | 0.391142 | 0.613683 | — | — | — |
| basic_stream_file_select.test | LINK | 0.191055 | 0.384623 | 0.614880 | — | — | — |
| basic_stream_socket.test | LINK | 0.569154 | 1.246513 | 2.216280 | — | — | — |
| basic_stream_socket_select.test | LINK | 0.579527 | 1.289299 | 2.325460 | — | — | — |
| basic_streambuf.test | LINK | 0.288830 | 0.571718 | 0.895368 | — | — | — |
| basic_streambuf_select.test | LINK | 0.290820 | 0.595814 | 0.859508 | — | — | — |
| basic_waitable_timer.test | LINK | 0.548656 | 1.160627 | 2.195795 | — | — | — |
| basic_waitable_timer_select.test | LINK | 0.540169 | 1.208962 | 2.205362 | — | — | — |
| basic_writable_pipe.test | LINK | 0.536905 | 1.201506 | 1.961069 | — | — | — |
| basic_writable_pipe_select.test | LINK | 0.581345 | 1.263043 | 2.459504 | — | — | — |
| bind_allocator.test | RUN | 0.912566 | 4.350694 | 4.801101 | 0.002552 | 0.002835 | 0.002599 |
| bind_allocator_select.test | RUN | 0.914203 | 4.433741 | 4.689700 | 0.002356 | 0.002417 | 0.002415 |
| bind_cancellation_slot.test | RUN | 0.918804 | 4.462294 | 4.742311 | 0.001684 | 0.001729 | 0.001646 |
| bind_cancellation_slot_select.test | RUN | 0.894955 | 4.411708 | 4.564210 | 0.001802 | 0.001672 | 0.001560 |
| bind_executor.test | RUN | 0.580861 | 2.982882 | 2.698342 | 0.002374 | 0.003459 | 0.002859 |
| bind_executor_select.test | RUN | 0.581441 | 3.212404 | 3.024724 | 0.002470 | 0.002904 | 0.002676 |
| bind_immediate_executor.test | RUN | 0.426726 | 2.636755 | 1.964295 | 0.001701 | 0.001536 | 0.001480 |
| bind_immediate_executor_select.test | RUN | 0.438848 | 2.545141 | 2.098033 | 0.001668 | 0.001663 | 0.001596 |
| buffer.test | RUN | 0.262650 | 1.029044 | 1.125623 | 0.001680 | 0.001562 | 0.001530 |
| buffer_select.test | RUN | 0.258955 | 1.021141 | 1.132775 | 0.001502 | 0.001452 | 0.001421 |
| buffer_registration.test | LINK | 0.211761 | 0.526390 | 0.662125 | — | — | — |
| buffer_registration_select.test | LINK | 0.212690 | 0.519464 | 0.669689 | — | — | — |
| buffered_read_stream.test | RUN | 0.915399 | 7.008073 | 6.850888 | 0.002104 | 0.003106 | 0.002814 |
| buffered_read_stream_select.test | RUN | 0.918303 | 6.865733 | 7.482097 | 0.001888 | 0.002666 | 0.002655 |
| buffered_stream.test | RUN | 1.654017 | 10.420543 | 11.118271 | 0.002661 | 0.002776 | 0.002792 |
| buffered_stream_select.test | RUN | 1.727544 | 10.452424 | 10.887698 | 0.002649 | 0.002832 | 0.002644 |
| buffered_write_stream.test | RUN | 1.601603 | 8.973126 | 9.563597 | 0.002650 | 0.003394 | 0.002760 |
| buffered_write_stream_select.test | RUN | 1.605176 | 9.129467 | 8.973440 | 0.002760 | 0.003016 | 0.002782 |
| buffers_iterator.test | RUN | 0.522834 | 0.911693 | 1.666165 | 0.002203 | 0.002258 | 0.001995 |
| buffers_iterator_select.test | RUN | 0.499914 | 0.915604 | 1.647452 | 0.002468 | 0.002100 | 0.002181 |
| cancel_after.test | RUN | 1.367719 | 7.018455 | 7.056876 | 0.004138 | 0.004557 | 0.004382 |
| cancel_after_select.test | RUN | 1.401916 | 6.961850 | 6.972753 | 0.003871 | 0.004419 | 0.003534 |
| cancel_at.test | RUN | 1.296650 | 5.803537 | 6.293512 | 0.003633 | 0.004116 | 0.003559 |
| cancel_at_select.test | RUN | 1.243899 | 5.695879 | 6.132473 | 0.003663 | 0.003913 | 0.003581 |
| cancellation_signal.test | LINK | 0.151689 | 0.315514 | 0.429128 | — | — | — |
| cancellation_signal_select.test | LINK | 0.162976 | 0.326754 | 0.437914 | — | — | — |
| cancellation_state.test | LINK | 0.217334 | 0.330698 | 0.450093 | — | — | — |
| cancellation_state_select.test | LINK | 0.252216 | 0.318995 | 0.435477 | — | — | — |
| cancellation_type.test | LINK | 0.195766 | 0.250867 | 0.347011 | — | — | — |
| cancellation_type_select.test | LINK | 0.186785 | 0.245695 | 0.462991 | — | — | — |
| co_composed.test | RUN | 0.190062 | 0.245997 | 0.558946 | 0.002088 | 0.001534 | 0.001728 |
| co_composed_select.test | RUN | 0.186249 | 0.247932 | 0.557808 | 0.001878 | 0.001542 | 0.001996 |
| co_spawn.test | LINK | 0.187763 | 0.248224 | 0.570630 | — | — | — |
| co_spawn_select.test | LINK | 0.187452 | 0.249449 | 0.541037 | — | — | — |
| completion_condition.test | LINK | 0.239703 | 0.418631 | 0.783163 | — | — | — |
| completion_condition_select.test | LINK | 0.235758 | 0.572710 | 0.776398 | — | — | — |
| compose.test | RUN | 1.056844 | 5.408203 | 5.913036 | 0.002560 | 0.002514 | 0.002334 |
| compose_select.test | RUN | 1.075750 | 5.458860 | 5.801954 | 0.002349 | 0.002676 | 0.002576 |
| composed.test | RUN | 1.046340 | 5.438377 | 5.662114 | 0.001920 | 0.002622 | 0.002102 |
| composed_select.test | RUN | 1.010463 | 5.274862 | 5.621336 | 0.002208 | 0.002228 | 0.002326 |
| config.test | RUN | 0.460384 | 3.635275 | 2.460214 | 0.002262 | 0.002145 | 0.002252 |
| config_select.test | RUN | 0.532462 | 3.702600 | 3.106571 | 0.002274 | 0.002198 | 0.002080 |
| connect.test | LINK | 1.705731 | 11.495073 | 12.262956 | — | — | — |
| connect_select.test | LINK | 1.710990 | 11.449497 | 12.149742 | — | — | — |
| connect_pipe.test | RUN | 0.967690 | 4.380589 | 4.596157 | 0.002178 | 0.003030 | 0.002365 |
| connect_pipe_select.test | RUN | 0.962803 | 4.552373 | 4.965436 | 0.002173 | 0.002443 | 0.002455 |
| consign.test | RUN | 0.819656 | 4.210528 | 4.098419 | 0.002150 | 0.002464 | 0.002173 |
| consign_select.test | RUN | 0.822741 | 4.346681 | 4.040197 | 0.002171 | 0.002382 | 0.002155 |
| coroutine.test | LINK | 0.193138 | 0.455716 | 0.637112 | — | — | — |
| coroutine_select.test | LINK | 0.210652 | 0.463367 | 0.632035 | — | — | — |
| deadline_timer.test | RUN | 1.199393 | 5.604012 | 6.034514 | 0.003599 | 0.003344 | 0.003961 |
| deadline_timer_select.test | RUN | 1.207416 | 5.496626 | 6.074846 | 0.003235 | 0.003974 | 0.003516 |
| deferred.test | RUN | 0.203368 | 0.449966 | 0.624150 | 0.002081 | 0.002432 | 0.001996 |
| deferred_select.test | RUN | 0.207907 | 0.441543 | 0.591282 | 0.001930 | 0.002275 | 0.002123 |
| detached.test | LINK | 0.231619 | 0.489544 | 0.690521 | — | — | — |
| detached_select.test | LINK | 0.251744 | 0.502218 | 0.688878 | — | — | — |
| disposition.test | RUN | 0.237495 | 1.110183 | 1.250724 | 0.001540 | 0.002294 | 0.002173 |
| disposition_select.test | RUN | 0.201521 | 1.105957 | 1.236811 | 0.001632 | 0.003118 | 0.002213 |
| error.test | RUN | 0.187473 | 1.087921 | 1.096886 | 0.001542 | 0.002236 | 0.002116 |
| error_select.test | RUN | 0.187798 | 1.064894 | 1.067017 | 0.001703 | 0.002267 | 0.002169 |
| file_base.test | LINK | 0.235684 | 0.883314 | 1.177879 | — | — | — |
| file_base_select.test | LINK | 0.249959 | 0.775084 | 1.026389 | — | — | — |
| fwd_first.test | LINK | 0.554609 | 1.271709 | 2.073615 | — | — | — |
| fwd_last.test | LINK | 0.554441 | 1.203271 | 2.067910 | — | — | — |
| generic_basic_endpoint.test | LINK | 0.207134 | 0.489871 | 0.759145 | — | — | — |
| generic_basic_endpoint_select.test | LINK | 0.202200 | 0.491493 | 0.760214 | — | — | — |
| generic_datagram_protocol.test | LINK | 0.798971 | 4.074814 | 3.775861 | — | — | — |
| generic_datagram_protocol_select.test | LINK | 0.815935 | 4.334840 | 3.769769 | — | — | — |
| generic_raw_protocol.test | LINK | 0.799553 | 4.118543 | 3.831347 | — | — | — |
| generic_raw_protocol_select.test | LINK | 0.795335 | 4.228219 | 3.807538 | — | — | — |
| generic_seq_packet_protocol.test | LINK | 0.678356 | 3.528941 | 3.325184 | — | — | — |
| generic_seq_packet_protocol_select.test | LINK | 0.692895 | 3.551236 | 3.259326 | — | — | — |
| generic_stream_protocol.test | LINK | 0.744943 | 3.772459 | 3.557444 | — | — | — |
| generic_stream_protocol_select.test | LINK | 0.739570 | 3.853062 | 3.459423 | — | — | — |
| high_resolution_timer.test | LINK | 0.339856 | 0.748398 | 1.166502 | — | — | — |
| high_resolution_timer_select.test | LINK | 0.342967 | 0.776507 | 1.160777 | — | — | — |
| immediate.test | LINK | 0.235836 | 0.566225 | 0.719241 | — | — | — |
| immediate_select.test | LINK | 0.231615 | 0.579718 | 0.722854 | — | — | — |
| inline_executor.test | RUN | 0.293935 | 0.872791 | 1.059647 | 0.001648 | 0.001610 | 0.001473 |
| inline_executor_select.test | RUN | 0.288876 | 0.863788 | 1.018932 | 0.002078 | 0.001604 | 0.001324 |
| inline_or_executor.test | RUN | 0.448546 | 2.515890 | 1.978960 | 0.001593 | 0.001597 | 0.001530 |
| inline_or_executor_select.test | RUN | 0.462007 | 2.537855 | 2.135776 | 0.001595 | 0.001618 | 0.001480 |
| io_context.test | RUN | 0.646637 | 3.764983 | 3.390276 | 0.002630 | 0.003072 | 0.002469 |
| io_context_select.test | RUN | 0.654405 | 3.682582 | 3.399846 | 0.002583 | 0.002874 | 0.002358 |
| io_context_strand.test | RUN | 0.632923 | 4.277837 | 4.225918 | 0.002589 | 0.003244 | 0.003196 |
| io_context_strand_select.test | RUN | 0.630004 | 4.058265 | 4.204982 | 0.002482 | 0.003219 | 0.002940 |
| ip_address.test | LINK | 0.237015 | 1.217384 | 1.695137 | — | — | — |
| ip_address_select.test | LINK | 0.248513 | 1.187461 | 1.715695 | — | — | — |
| ip_address_v4.test | LINK | 0.263601 | 1.280342 | 1.560085 | — | — | — |
| ip_address_v4_select.test | LINK | 0.244934 | 1.303453 | 1.473677 | — | — | — |
| ip_address_v6.test | LINK | 0.273971 | 1.178997 | 1.231633 | — | — | — |
| ip_address_v6_select.test | LINK | 0.289117 | 1.043830 | 1.229126 | — | — | — |
| ip_basic_endpoint.test | LINK | 0.207665 | 0.528040 | 0.795438 | — | — | — |
| ip_basic_endpoint_select.test | LINK | 0.222481 | 0.521118 | 0.803017 | — | — | — |
| ip_basic_resolver.test | LINK | 0.340871 | 0.756082 | 1.155425 | — | — | — |
| ip_basic_resolver_select.test | LINK | 0.342548 | 0.788387 | 1.221153 | — | — | — |
| ip_basic_resolver_entry.test | LINK | 0.120086 | 0.250512 | 0.333209 | — | — | — |
| ip_basic_resolver_entry_select.test | LINK | 0.116764 | 0.249919 | 0.342678 | — | — | — |
| ip_basic_resolver_iterator.test | LINK | 0.218385 | 0.526884 | 0.784410 | — | — | — |
| ip_basic_resolver_iterator_select.test | LINK | 0.207684 | 0.512955 | 0.788429 | — | — | — |
| ip_basic_resolver_query.test | LINK | 0.201535 | 0.495710 | 0.756984 | — | — | — |
| ip_basic_resolver_query_select.test | LINK | 0.199406 | 0.496228 | 0.756859 | — | — | — |
| ip_host_name.test | RUN | 0.216991 | 0.661357 | 0.838846 | 0.001828 | 0.001586 | 0.001454 |
| ip_host_name_select.test | RUN | 0.223523 | 0.649164 | 0.846716 | 0.001504 | 0.001594 | 0.001393 |
| ip_icmp.test | RUN | 0.974356 | 7.552286 | 8.342423 | 0.001639 | 0.002164 | 0.002137 |
| ip_icmp_select.test | RUN | 0.982727 | 7.973147 | 8.185183 | 0.002032 | 0.002622 | 0.001867 |
| ip_multicast.test | RUN | 0.593238 | 4.567574 | 5.044577 | 0.001723 | 0.002298 | 0.002288 |
| ip_multicast_select.test | RUN | 0.604104 | 4.568728 | 5.098293 | 0.001752 | 0.002243 | 0.002212 |
| ip_resolver_query_base.test | LINK | 0.137025 | 0.447615 | 0.613834 | — | — | — |
| ip_resolver_query_base_select.test | LINK | 0.139944 | 0.421507 | 0.589055 | — | — | — |
| ip_tcp.test | RUN | 1.482514 | 13.071162 | 12.695134 | 0.002232 | 0.003144 | 0.003132 |
| ip_tcp_select.test | RUN | 1.458022 | 13.649064 | 12.300835 | 0.002241 | 0.002917 | 0.002694 |
| ip_udp.test | RUN | 1.022320 | 9.339343 | 8.867325 | 0.001736 | 0.002293 | 0.002517 |
| ip_udp_select.test | RUN | 1.020852 | 9.556121 | 8.671342 | 0.002047 | 0.002420 | 0.001975 |
| ip_unicast.test | RUN | 0.826917 | 4.186452 | 4.328174 | 0.002150 | 0.002324 | 0.002275 |
| ip_unicast_select.test | RUN | 0.875196 | 4.258249 | 4.284967 | 0.002228 | 0.002468 | 0.002286 |
| ip_v6_only.test | RUN | 0.873836 | 4.333745 | 4.620303 | 0.002186 | 0.002038 | 0.002056 |
| ip_v6_only_select.test | RUN | 0.879389 | 4.509160 | 4.650060 | 0.002086 | 0.002414 | 0.002052 |
| is_read_buffered.test | RUN | 0.636337 | 1.396814 | 2.658691 | 0.002188 | 0.002280 | 0.001885 |
| is_read_buffered_select.test | RUN | 0.651430 | 1.451177 | 2.645358 | 0.002067 | 0.002309 | 0.002070 |
| is_write_buffered.test | RUN | 0.635005 | 1.398128 | 2.674582 | 0.001907 | 0.002025 | 0.001873 |
| is_write_buffered_select.test | RUN | 0.665274 | 1.467478 | 2.703030 | 0.001894 | 0.002003 | 0.001850 |
| local_basic_endpoint.test | LINK | 0.323579 | 0.777358 | 1.440472 | — | — | — |
| local_basic_endpoint_select.test | LINK | 0.334849 | 0.791573 | 1.384341 | — | — | — |
| local_connect_pair.test | LINK | 0.839624 | 4.235263 | 4.292383 | — | — | — |
| local_connect_pair_select.test | LINK | 0.874983 | 4.248647 | 4.174426 | — | — | — |
| local_datagram_protocol.test | LINK | 1.094391 | 5.438001 | 5.642912 | — | — | — |
| local_datagram_protocol_select.test | LINK | 1.089402 | 5.494271 | 5.580042 | — | — | — |
| local_seq_packet_protocol.test | LINK | 1.088060 | 5.390961 | 5.699435 | — | — | — |
| local_seq_packet_protocol_select.test | LINK | 1.084871 | 5.535169 | 5.657114 | — | — | — |
| local_stream_protocol.test | LINK | 1.033817 | 5.107080 | 5.373592 | — | — | — |
| local_stream_protocol_select.test | LINK | 1.070473 | 5.163915 | 5.311921 | — | — | — |
| placeholders.test | LINK | 0.201309 | 0.429218 | 0.624438 | — | — | — |
| placeholders_select.test | LINK | 0.202202 | 0.425715 | 0.622676 | — | — | — |
| posix_basic_descriptor.test | LINK | 0.546982 | 1.177762 | 1.977033 | — | — | — |
| posix_basic_descriptor_select.test | LINK | 0.579333 | 1.255564 | 2.517671 | — | — | — |
| posix_basic_stream_descriptor.test | LINK | 0.555222 | 1.189542 | 1.959679 | — | — | — |
| posix_basic_stream_descriptor_select.test | LINK | 0.584990 | 1.287466 | 2.388926 | — | — | — |
| posix_descriptor_base.test | LINK | 0.230462 | 0.468260 | 0.632669 | — | — | — |
| posix_descriptor_base_select.test | LINK | 0.229930 | 0.440379 | 0.589904 | — | — | — |
| posix_stream_descriptor.test | LINK | 1.059769 | 5.220284 | 5.291217 | — | — | — |
| posix_stream_descriptor_select.test | LINK | 1.080179 | 5.281038 | 5.547258 | — | — | — |
| prepend.test | RUN | 0.818381 | 4.269558 | 4.378890 | 0.002084 | 0.003196 | 0.002090 |
| prepend_select.test | RUN | 0.829110 | 4.363367 | 4.315375 | 0.002259 | 0.002370 | 0.002196 |
| random_access_file.test | LINK | 0.517272 | 1.049741 | 1.529039 | — | — | — |
| random_access_file_select.test | LINK | 0.523804 | 1.120477 | 2.015489 | — | — | — |
| read.test | RUN | 2.568776 | 25.894620 | 23.156811 | 0.004124 | 0.003141 | 0.003277 |
| read_select.test | RUN | 2.577358 | 25.800310 | 23.728752 | 0.003089 | 0.003396 | 0.002883 |
| read_at.test | RUN | 2.566098 | 36.653500 | 32.093563 | 0.003821 | 0.003972 | 0.003724 |
| read_at_select.test | RUN | 2.604961 | 36.425495 | 32.785002 | 0.004152 | 0.003888 | 0.003678 |
| read_until.test | RUN | 1.356355 | 10.062535 | 8.496069 | 0.002242 | 0.002331 | 0.002428 |
| read_until_select.test | RUN | 1.388105 | 10.104074 | 9.093787 | 0.002568 | 0.002461 | 0.002709 |
| readable_pipe.test | LINK | 0.872867 | 4.269927 | 4.123413 | — | — | — |
| readable_pipe_select.test | LINK | 0.861421 | 4.497161 | 4.507935 | — | — | — |
| recycling_allocator.test | RUN | 0.287285 | 0.659114 | 0.951806 | 0.001950 | 0.002160 | 0.002112 |
| recycling_allocator_select.test | RUN | 0.286565 | 0.668801 | 0.899968 | 0.002036 | 0.001990 | 0.002516 |
| redirect_disposition.test | LINK | 1.108767 | 5.950347 | 6.604423 | — | — | — |
| redirect_disposition_select.test | LINK | 1.132046 | 5.944958 | 6.475132 | — | — | — |
| redirect_error.test | LINK | 0.993476 | 5.017941 | 5.677086 | — | — | — |
| redirect_error_select.test | LINK | 1.005859 | 5.083933 | 5.720626 | — | — | — |
| registered_buffer.test | LINK | 0.291101 | 0.572193 | 0.879312 | — | — | — |
| registered_buffer_select.test | LINK | 0.307991 | 0.546114 | 0.819839 | — | — | — |
| signal_set.test | RUN | 0.834330 | 4.219111 | 3.904814 | 0.002264 | 0.002168 | 0.002030 |
| signal_set_select.test | RUN | 0.876588 | 4.361874 | 4.342242 | 0.002100 | 0.002424 | 0.002135 |
| signal_set_base.test | LINK | 0.230576 | 0.462508 | 0.643717 | — | — | — |
| signal_set_base_select.test | LINK | 0.238599 | 0.469273 | 0.650206 | — | — | — |
| socket_base.test | RUN | 1.053160 | 6.373131 | 6.160343 | 0.002487 | 0.002544 | 0.002861 |
| socket_base_select.test | RUN | 1.067144 | 6.158985 | 6.212651 | 0.002392 | 0.002919 | 0.002416 |
| spawn.test | RUN | 1.655056 | 9.912990 | 9.795335 | 0.002544 | 0.002505 | 0.002486 |
| spawn_select.test | RUN | 1.681225 | 9.847128 | 9.758640 | 0.002500 | 0.002438 | 0.002696 |
| static_thread_pool.test | RUN | 0.489903 | 1.046849 | 1.496106 | 0.002816 | 0.002580 | 0.002192 |
| static_thread_pool_select.test | RUN | 0.506201 | 1.117093 | 1.980069 | 0.002340 | 0.002416 | 0.002135 |
| steady_timer.test | LINK | 0.540501 | 1.179137 | 2.181643 | — | — | — |
| steady_timer_select.test | LINK | 0.559299 | 1.205577 | 2.179383 | — | — | — |
| strand.test | RUN | 1.097180 | 5.789159 | 6.342603 | 0.002698 | 0.003384 | 0.003616 |
| strand_select.test | RUN | 1.112609 | 5.792305 | 6.157696 | 0.002620 | 0.002459 | 0.003042 |
| stream_file.test | LINK | 0.508304 | 1.067683 | 1.581318 | — | — | — |
| stream_file_select.test | LINK | 0.531747 | 1.132011 | 2.107104 | — | — | — |
| streambuf.test | RUN | 0.228525 | 0.853810 | 1.118466 | 0.001574 | 0.002058 | 0.002195 |
| streambuf_select.test | RUN | 0.192025 | 0.617327 | 1.061490 | 0.001523 | 0.001972 | 0.002072 |
| system_timer.test | LINK | 0.631534 | 3.487110 | 3.890426 | — | — | — |
| system_timer_select.test | LINK | 0.637463 | 3.454803 | 3.383287 | — | — | — |
| system_context.test | LINK | 0.314845 | 0.642222 | 0.858656 | — | — | — |
| system_context_select.test | LINK | 0.342348 | 0.695534 | 1.087567 | — | — | — |
| system_executor.test | LINK | 0.391781 | 2.018942 | 1.573581 | — | — | — |
| system_executor_select.test | LINK | 0.403674 | 1.954308 | 1.785961 | — | — | — |
| this_coro.test | LINK | 0.117922 | 0.247070 | 0.345851 | — | — | — |
| this_coro_select.test | LINK | 0.121318 | 0.249156 | 0.344404 | — | — | — |
| thread_pool.test | RUN | 0.501964 | 3.468625 | 2.404954 | 0.003250 | 0.003851 | 0.003975 |
| thread_pool_select.test | RUN | 0.492568 | 3.823235 | 2.619777 | 0.002982 | 0.004000 | 0.003856 |
| time_traits.test | LINK | 0.276780 | 0.891022 | 1.218343 | — | — | — |
| time_traits_select.test | LINK | 0.266109 | 0.827000 | 1.259490 | — | — | — |
| ts_buffer.test | LINK | 0.245234 | 0.911111 | 1.268394 | — | — | — |
| ts_buffer_select.test | LINK | 0.251284 | 0.903931 | 1.259223 | — | — | — |
| ts_executor.test | LINK | 0.359258 | 1.245850 | 1.983182 | — | — | — |
| ts_executor_select.test | LINK | 0.366559 | 1.342967 | 2.502560 | — | — | — |
| ts_internet.test | LINK | 0.384644 | 1.382569 | 2.594988 | — | — | — |
| ts_internet_select.test | LINK | 0.393976 | 1.431140 | 2.668404 | — | — | — |
| ts_io_context.test | LINK | 0.307936 | 1.063191 | 1.493858 | — | — | — |
| ts_io_context_select.test | LINK | 0.323623 | 1.131811 | 2.087721 | — | — | — |
| ts_net.test | LINK | 0.464965 | 1.663727 | 3.165020 | — | — | — |
| ts_net_select.test | LINK | 0.485967 | 1.721777 | 3.193896 | — | — | — |
| ts_netfwd.test | LINK | 0.481230 | 1.655486 | 3.190703 | — | — | — |
| ts_netfwd_select.test | LINK | 0.484668 | 1.701172 | 3.250558 | — | — | — |
| ts_socket.test | LINK | 0.389519 | 1.291679 | 2.456745 | — | — | — |
| ts_socket_select.test | LINK | 0.393953 | 1.357583 | 2.440807 | — | — | — |
| ts_timer.test | LINK | 0.342182 | 1.166194 | 2.161212 | — | — | — |
| ts_timer_select.test | LINK | 0.346517 | 1.239026 | 2.170807 | — | — | — |
| use_awaitable.test | LINK | 0.172771 | 0.382171 | 0.586714 | — | — | — |
| use_awaitable_select.test | LINK | 0.188184 | 0.407824 | 0.605976 | — | — | — |
| wait_traits.test | LINK | 0.182237 | 0.381260 | 0.580875 | — | — | — |
| wait_traits_select.test | LINK | 0.185042 | 0.398862 | 0.557371 | — | — | — |
| windows_basic_object_handle.test | LINK | 0.186810 | 0.395244 | 0.584072 | — | — | — |
| windows_basic_object_handle_select.test | LINK | 0.185366 | 0.396492 | 0.605118 | — | — | — |
| windows_basic_overlapped_handle.test | LINK | 0.192471 | 0.386623 | 0.548900 | — | — | — |
| windows_basic_overlapped_handle_select.test | LINK | 0.187339 | 0.397307 | 0.565462 | — | — | — |
| windows_basic_random_access_handle.test | LINK | 0.189431 | 0.396888 | 0.590275 | — | — | — |
| windows_basic_random_access_handle_select.test | LINK | 0.189083 | 0.396874 | 0.602974 | — | — | — |
| windows_basic_stream_handle.test | LINK | 0.195128 | 0.378660 | 0.560634 | — | — | — |
| windows_basic_stream_handle_select.test | LINK | 0.188579 | 0.391750 | 0.557755 | — | — | — |
| windows_object_handle.test | LINK | 0.504421 | 1.050954 | 1.548675 | — | — | — |
| windows_object_handle_select.test | LINK | 0.518590 | 1.117604 | 2.053942 | — | — | — |
| windows_overlapped_ptr.test | LINK | 0.523820 | 1.110766 | 1.684870 | — | — | — |
| windows_overlapped_ptr_select.test | LINK | 0.550972 | 1.190063 | 2.165140 | — | — | — |
| windows_random_access_handle.test | LINK | 0.524822 | 1.069143 | 1.444546 | — | — | — |
| windows_random_access_handle_select.test | LINK | 0.521683 | 1.137460 | 2.054857 | — | — | — |
| windows_stream_handle.test | LINK | 0.512212 | 1.055478 | 1.584269 | — | — | — |
| windows_stream_handle_select.test | LINK | 0.543577 | 1.161906 | 2.054196 | — | — | — |
| writable_pipe.test | LINK | 0.875630 | 4.299691 | 4.364955 | — | — | — |
| writable_pipe_select.test | LINK | 0.902863 | 4.479871 | 4.772619 | — | — | — |
| write.test | RUN | 2.526294 | 21.769614 | 23.433119 | 0.003259 | 0.003291 | 0.003334 |
| write_select.test | RUN | 2.543929 | 21.986742 | 23.785886 | 0.003090 | 0.003141 | 0.003318 |
| write_at.test | RUN | 2.582332 | 30.365390 | 29.329332 | 0.003403 | 0.003109 | 0.003250 |
| write_at_select.test | RUN | 2.621212 | 30.507937 | 29.889741 | 0.004386 | 0.003290 | 0.003382 |
| any_executor.test | RUN | 1.073499 | 10.000623 | 9.961150 | 0.003304 | 0.002966 | 0.002918 |
| blocking.test | RUN | 2.766138 | 12.452450 | 20.204376 | 0.009443 | 0.009082 | 0.009023 |
| blocking_adaptation.test | RUN | 1.367718 | 5.480571 | 9.366023 | 0.003930 | 0.005218 | 0.004678 |
| context_as.test | RUN | 0.740357 | 3.898156 | 3.116859 | 0.002240 | 0.002558 | 0.002146 |
| executor.test | RUN | 0.203772 | 0.402124 | 0.619261 | 0.002401 | 0.002280 | 0.002664 |
| inline_exception_handling.test | RUN | 2.564477 | 12.130192 | 20.625288 | 0.005796 | 0.005750 | 0.007659 |
| invocable_archetype.test | RUN | 0.189052 | 0.412762 | 0.599728 | 0.002074 | 0.002238 | 0.002006 |
| mapping.test | RUN | 2.501778 | 11.421570 | 18.900151 | 0.006192 | 0.006575 | 0.006060 |
| outstanding_work.test | RUN | 1.228964 | 5.154292 | 8.970452 | 0.004764 | 0.004266 | 0.004710 |
| prefer_only.test | RUN | 0.387518 | 2.090552 | 2.047556 | 0.002098 | 0.002360 | 0.002129 |
| relationship.test | RUN | 1.048133 | 5.132050 | 8.710579 | 0.003386 | 0.005611 | 0.003279 |

## Native runtime replay

Child CPU of successful native tests, with process startup included; GDB and expected-failure executions are correctness-only. Two warmups per compiler, four A/A observations per reference and mirrored ABBA/BAAB blocks. Short tests repeat up to ten times, identically across compilers; CPU is divided by repetitions before summing tests. Executable hashes and output checks stay outside process timers. All runs must pass their original native oracle. Output hashes record equality or variation across the complete campaign; test reports may print times or addresses.

| Reference | CPU speedup ↑ | Paired blocks | A/A speedup |
|---|---:|---|---:|
| gcc | 1.003× | 0.998×, 1.008× | 0.996× |
| clang | 0.983× | 0.990×, 0.975× | 1.005× |

† A/A drift or paired-block spread exceeds 5%; small differences are unresolved. 1375 native tests in the full cohort; 1360 have byte-identical output throughout. [Raw replay evidence in CI artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43133898266).

| Test | Repeats | CPU cppgm | GCC | Clang | Output check |
|---|---:|---:|---:|---:|---|
| any_completion_executor.test | 3 | 0.002074s | 0.002085s | 0.002012s | byte equality observed |
| any_completion_executor_select.test | 4 | 0.002083s | 0.002087s | 0.001995s | byte equality observed |
| any_completion_handler.test | 8 | 0.001633s | 0.001702s | 0.001586s | byte equality observed |
| any_completion_handler_select.test | 8 | 0.001609s | 0.001652s | 0.001600s | byte equality observed |
| any_io_executor.test | 4 | 0.002034s | 0.002044s | 0.002039s | byte equality observed |
| any_io_executor_select.test | 5 | 0.002030s | 0.002027s | 0.001982s | byte equality observed |
| append.test | 10 | 0.001568s | 0.001615s | 0.001546s | byte equality observed |
| append_select.test | 9 | 0.001526s | 0.001554s | 0.001504s | byte equality observed |
| as_tuple.test | 5 | 0.001718s | 0.001718s | 0.001689s | byte equality observed |
| as_tuple_select.test | 8 | 0.001542s | 0.001563s | 0.001546s | byte equality observed |
| bind_allocator.test | 1 | 0.002061s | 0.002041s | 0.001984s | byte equality observed |
| bind_allocator_select.test | 1 | 0.001710s | 0.001596s | 0.001709s | byte equality observed |
| bind_cancellation_slot.test | 9 | 0.001420s | 0.001427s | 0.001441s | byte equality observed |
| bind_cancellation_slot_select.test | 10 | 0.001311s | 0.001306s | 0.001314s | byte equality observed |
| bind_executor.test | 1 | 0.002022s | 0.001937s | 0.001933s | byte equality observed |
| bind_executor_select.test | 1 | 0.002258s | 0.002354s | 0.002332s | byte equality observed |
| bind_immediate_executor.test | 9 | 0.001337s | 0.001420s | 0.001305s | byte equality observed |
| bind_immediate_executor_select.test | 10 | 0.001393s | 0.001422s | 0.001361s | byte equality observed |
| buffer.test | 10 | 0.001335s | 0.001353s | 0.001327s | byte equality observed |
| buffer_select.test | 10 | 0.001298s | 0.001369s | 0.001334s | byte equality observed |
| buffered_read_stream.test | 9 | 0.001709s | 0.001707s | 0.001698s | byte equality observed |
| buffered_read_stream_select.test | 10 | 0.001675s | 0.001648s | 0.001631s | byte equality observed |
| buffered_stream.test | 10 | 0.001716s | 0.001723s | 0.001700s | byte equality observed |
| buffered_stream_select.test | 6 | 0.001659s | 0.001759s | 0.001663s | byte equality observed |
| buffered_write_stream.test | 7 | 0.001761s | 0.001723s | 0.001682s | byte equality observed |
| buffered_write_stream_select.test | 10 | 0.001664s | 0.001657s | 0.001658s | byte equality observed |
| buffers_iterator.test | 10 | 0.001329s | 0.001290s | 0.001293s | byte equality observed |
| buffers_iterator_select.test | 10 | 0.001226s | 0.001255s | 0.001205s | byte equality observed |
| cancel_after.test | 1 | 0.003556s | 0.004146s | 0.003724s | byte equality observed |
| cancel_after_select.test | 1 | 0.003148s | 0.003444s | 0.003149s | byte equality observed |
| cancel_at.test | 1 | 0.003816s | 0.004150s | 0.003518s | byte equality observed |
| cancel_at_select.test | 1 | 0.003873s | 0.003952s | 0.003380s | byte equality observed |
| co_composed.test | 9 | 0.001240s | 0.001260s | 0.001241s | byte equality observed |
| co_composed_select.test | 10 | 0.001204s | 0.001226s | 0.001200s | byte equality observed |
| compose.test | 1 | 0.002055s | 0.002051s | 0.001894s | byte equality observed |
| compose_select.test | 1 | 0.001663s | 0.001711s | 0.001598s | byte equality observed |
| composed.test | 1 | 0.002214s | 0.002381s | 0.002001s | byte equality observed |
| composed_select.test | 1 | 0.001751s | 0.001897s | 0.001682s | byte equality observed |
| config.test | 7 | 0.001321s | 0.001341s | 0.001295s | byte equality observed |
| config_select.test | 8 | 0.001397s | 0.001391s | 0.001392s | byte equality observed |
| connect_pipe.test | 10 | 0.001423s | 0.001447s | 0.001410s | byte equality observed |
| connect_pipe_select.test | 8 | 0.001434s | 0.001394s | 0.001397s | byte equality observed |
| consign.test | 10 | 0.001397s | 0.001425s | 0.001391s | byte equality observed |
| consign_select.test | 10 | 0.001364s | 0.001333s | 0.001297s | byte equality observed |
| deadline_timer.test | 1 | 0.002921s | 0.002915s | 0.002877s | byte equality observed |
| deadline_timer_select.test | 1 | 0.002616s | 0.002537s | 0.002508s | byte equality observed |
| deferred.test | 10 | 0.001219s | 0.001216s | 0.001219s | byte equality observed |
| deferred_select.test | 10 | 0.001179s | 0.001182s | 0.001178s | byte equality observed |
| disposition.test | 10 | 0.001250s | 0.001253s | 0.001249s | byte equality observed |
| disposition_select.test | 10 | 0.001249s | 0.001257s | 0.001251s | byte equality observed |
| error.test | 10 | 0.001217s | 0.001223s | 0.001215s | byte equality observed |
| error_select.test | 10 | 0.001215s | 0.001217s | 0.001217s | byte equality observed |
| inline_executor.test | 10 | 0.001222s | 0.001214s | 0.001213s | byte equality observed |
| inline_executor_select.test | 10 | 0.001234s | 0.001233s | 0.001217s | byte equality observed |
| inline_or_executor.test | 10 | 0.001236s | 0.001258s | 0.001216s | byte equality observed |
| inline_or_executor_select.test | 10 | 0.001239s | 0.001250s | 0.001211s | byte equality observed |
| io_context.test | 1 | 0.002313s | 0.002357s | 0.002325s | byte equality observed |
| io_context_select.test | 1 | 0.002766s | 0.002680s | 0.002654s | byte equality observed |
| io_context_strand.test | 1 | 0.002678s | 0.002676s | 0.002648s | byte equality observed |
| io_context_strand_select.test | 1 | 0.002394s | 0.002410s | 0.002246s | byte equality observed |
| ip_host_name.test | 10 | 0.001236s | 0.001236s | 0.001214s | byte equality observed |
| ip_host_name_select.test | 10 | 0.001187s | 0.001197s | 0.001168s | byte equality observed |
| ip_icmp.test | 10 | 0.001234s | 0.001246s | 0.001198s | byte equality observed |
| ip_icmp_select.test | 10 | 0.001233s | 0.001242s | 0.001190s | byte equality observed |
| ip_multicast.test | 10 | 0.001308s | 0.001339s | 0.001314s | byte equality observed |
| ip_multicast_select.test | 10 | 0.001304s | 0.001316s | 0.001283s | byte equality observed |
| ip_tcp.test | 10 | 0.001820s | 0.001836s | 0.001815s | byte equality observed |
| ip_tcp_select.test | 10 | 0.001734s | 0.001837s | 0.001737s | byte equality observed |
| ip_udp.test | 10 | 0.001329s | 0.001426s | 0.001327s | byte equality observed |
| ip_udp_select.test | 10 | 0.001326s | 0.001439s | 0.001315s | byte equality observed |
| ip_unicast.test | 10 | 0.001293s | 0.001441s | 0.001271s | byte equality observed |
| ip_unicast_select.test | 10 | 0.001265s | 0.001378s | 0.001247s | byte equality observed |
| ip_v6_only.test | 10 | 0.001278s | 0.001425s | 0.001277s | byte equality observed |
| ip_v6_only_select.test | 10 | 0.001260s | 0.001428s | 0.001269s | byte equality observed |
| is_read_buffered.test | 10 | 0.001170s | 0.001332s | 0.001254s | byte equality observed |
| is_read_buffered_select.test | 10 | 0.001178s | 0.001312s | 0.001313s | byte equality observed |
| is_write_buffered.test | 10 | 0.001196s | 0.001349s | 0.001281s | byte equality observed |
| is_write_buffered_select.test | 10 | 0.001201s | 0.001283s | 0.001285s | byte equality observed |
| prepend.test | 10 | 0.001396s | 0.001415s | 0.001395s | byte equality observed |
| prepend_select.test | 10 | 0.001356s | 0.001377s | 0.001349s | byte equality observed |
| read.test | 10 | 0.001893s | 0.001785s | 0.001755s | byte equality observed |
| read_at.test | 8 | 0.002313s | 0.002282s | 0.002169s | byte equality observed |
| read_at_select.test | 8 | 0.002308s | 0.002287s | 0.002223s | byte equality observed |
| read_select.test | 10 | 0.001916s | 0.001800s | 0.001798s | byte equality observed |
| read_until.test | 10 | 0.001535s | 0.001504s | 0.001527s | byte equality observed |
| read_until_select.test | 10 | 0.001596s | 0.001498s | 0.001520s | byte equality observed |
| recycling_allocator.test | 10 | 0.001298s | 0.001315s | 0.001302s | byte equality observed |
| recycling_allocator_select.test | 10 | 0.001307s | 0.001289s | 0.001336s | byte equality observed |
| signal_set.test | 10 | 0.001372s | 0.001309s | 0.001387s | byte equality observed |
| signal_set_select.test | 10 | 0.001378s | 0.001323s | 0.001263s | byte equality observed |
| socket_base.test | 10 | 0.001478s | 0.001503s | 0.001466s | byte equality observed |
| socket_base_select.test | 10 | 0.001483s | 0.001448s | 0.001410s | byte equality observed |
| spawn.test | 10 | 0.001573s | 0.001568s | 0.001517s | byte equality observed |
| spawn_select.test | 10 | 0.001576s | 0.001573s | 0.001546s | byte equality observed |
| static_thread_pool.test | 10 | 0.001297s | 0.001320s | 0.001303s | byte equality observed |
| static_thread_pool_select.test | 10 | 0.001303s | 0.001208s | 0.001254s | byte equality observed |
| strand.test | 1 | 0.002489s | 0.002646s | 0.002433s | byte equality observed |
| strand_select.test | 1 | 0.002074s | 0.002103s | 0.001988s | byte equality observed |
| streambuf.test | 10 | 0.001358s | 0.001344s | 0.001303s | byte equality observed |
| streambuf_select.test | 10 | 0.001335s | 0.001348s | 0.001351s | byte equality observed |
| thread_pool.test | 8 | 0.003086s | 0.003025s | 0.003035s | byte equality observed |
| thread_pool_select.test | 8 | 0.003027s | 0.003049s | 0.002982s | byte equality observed |
| write.test | 10 | 0.001985s | 0.001942s | 0.001902s | byte equality observed |
| write_at.test | 9 | 0.002212s | 0.002053s | 0.001911s | byte equality observed |
| write_at_select.test | 9 | 0.002156s | 0.002047s | 0.001933s | byte equality observed |
| write_select.test | 9 | 0.001990s | 0.001851s | 0.001871s | byte equality observed |

[Raw outcomes, commands and timing observations in CI artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43133898266).

[Comparison records](../results.json) · [Host and toolchains](../metadata.json)
