# libs/asio/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| any_completion_executor.test | RUN | 0.509043 | 2.902840 | 2.904253 | 0.002336 | 0.002354 | 0.002605 |
| any_completion_executor_select.test | RUN | 0.548714 | 2.923370 | 3.082949 | 0.002012 | 0.002115 | 0.002543 |
| any_completion_handler.test | RUN | 0.592294 | 2.686168 | 2.692746 | 0.001740 | 0.001857 | 0.002041 |
| any_completion_handler_select.test | RUN | 0.606901 | 2.763919 | 2.941196 | 0.001641 | 0.001861 | 0.001922 |
| any_io_executor.test | RUN | 0.546051 | 3.107198 | 3.208639 | 0.002046 | 0.002066 | 0.002839 |
| any_io_executor_select.test | RUN | 0.562291 | 3.089443 | 3.299807 | 0.002022 | 0.002233 | 0.002368 |
| append.test | RUN | 0.512743 | 2.519355 | 2.363223 | 0.001664 | 0.001921 | 0.001807 |
| append_select.test | RUN | 0.505796 | 2.589394 | 2.321340 | 0.001874 | 0.001842 | 0.001956 |
| as_tuple.test | RUN | 0.658188 | 3.129685 | 3.428360 | 0.001976 | 0.001622 | 0.002835 |
| as_tuple_select.test | RUN | 0.672632 | 3.053547 | 3.426315 | 0.001688 | 0.001588 | 0.001947 |
| associated_allocator.test | LINK | 0.135262 | 0.261546 | 0.350796 | — | — | — |
| associated_allocator_select.test | LINK | 0.148508 | 0.272330 | 0.345197 | — | — | — |
| associated_cancellation_slot.test | LINK | 0.158538 | 0.298763 | 0.367635 | — | — | — |
| associated_cancellation_slot_select.test | LINK | 0.164412 | 0.300366 | 0.404770 | — | — | — |
| associated_executor.test | LINK | 0.221976 | 0.504940 | 0.621934 | — | — | — |
| associated_executor_select.test | LINK | 0.220683 | 0.511519 | 0.614121 | — | — | — |
| associated_immediate_executor.test | LINK | 0.204982 | 0.464442 | 0.552601 | — | — | — |
| associated_immediate_executor_select.test | LINK | 0.207267 | 0.471199 | 0.588225 | — | — | — |
| associator.test | LINK | 0.115132 | 0.207665 | 0.335545 | — | — | — |
| associator_select.test | LINK | 0.112333 | 0.210655 | 0.312040 | — | — | — |
| awaitable.test | LINK | 0.112160 | 0.214102 | 0.270745 | — | — | — |
| awaitable_select.test | LINK | 0.118795 | 0.208157 | 0.289303 | — | — | — |
| basic_datagram_socket.test | LINK | 0.341916 | 0.669247 | 1.136475 | — | — | — |
| basic_datagram_socket_select.test | LINK | 0.346671 | 0.687833 | 1.200422 | — | — | — |
| basic_deadline_timer.test | LINK | 0.412933 | 0.802952 | 1.328368 | — | — | — |
| basic_deadline_timer_select.test | LINK | 0.431640 | 0.825119 | 1.362173 | — | — | — |
| basic_file.test | LINK | 0.117286 | 0.222537 | 0.305661 | — | — | — |
| basic_file_select.test | LINK | 0.116632 | 0.218987 | 0.288434 | — | — | — |
| basic_random_access_file.test | LINK | 0.121091 | 0.219667 | 0.281467 | — | — | — |
| basic_random_access_file_select.test | LINK | 0.123002 | 0.220358 | 0.300768 | — | — | — |
| basic_raw_socket.test | LINK | 0.354110 | 0.697111 | 1.066542 | — | — | — |
| basic_raw_socket_select.test | LINK | 0.358968 | 0.717556 | 1.079977 | — | — | — |
| basic_readable_pipe.test | LINK | 0.335465 | 0.640657 | 0.897941 | — | — | — |
| basic_readable_pipe_select.test | LINK | 0.360817 | 0.688310 | 1.102732 | — | — | — |
| basic_seq_packet_socket.test | LINK | 0.331414 | 0.697078 | 1.074546 | — | — | — |
| basic_seq_packet_socket_select.test | LINK | 0.338596 | 0.712951 | 1.094880 | — | — | — |
| basic_signal_set.test | LINK | 0.306898 | 0.652841 | 0.852876 | — | — | — |
| basic_signal_set_select.test | LINK | 0.333408 | 0.688948 | 1.092098 | — | — | — |
| basic_socket_acceptor.test | LINK | 0.342782 | 0.684136 | 1.043660 | — | — | — |
| basic_socket_acceptor_select.test | LINK | 0.352588 | 0.693342 | 1.094738 | — | — | — |
| basic_stream_file.test | LINK | 0.112551 | 0.223474 | 0.296691 | — | — | — |
| basic_stream_file_select.test | LINK | 0.114736 | 0.220490 | 0.290268 | — | — | — |
| basic_stream_socket.test | LINK | 0.345521 | 0.661280 | 1.069741 | — | — | — |
| basic_stream_socket_select.test | LINK | 0.359061 | 0.678041 | 1.067661 | — | — | — |
| basic_streambuf.test | LINK | 0.176439 | 0.309082 | 0.413119 | — | — | — |
| basic_streambuf_select.test | LINK | 0.173271 | 0.313327 | 0.422073 | — | — | — |
| basic_waitable_timer.test | LINK | 0.319112 | 0.618556 | 1.007976 | — | — | — |
| basic_waitable_timer_select.test | LINK | 0.322832 | 0.632731 | 1.031003 | — | — | — |
| basic_writable_pipe.test | LINK | 0.321687 | 0.622656 | 0.946907 | — | — | — |
| basic_writable_pipe_select.test | LINK | 0.349931 | 0.687504 | 1.134722 | — | — | — |
| bind_allocator.test | RUN | 0.553407 | 2.600980 | 2.595489 | 0.002356 | 0.002128 | 0.002055 |
| bind_allocator_select.test | RUN | 0.574442 | 2.659149 | 2.581560 | 0.001688 | 0.001634 | 0.002914 |
| bind_cancellation_slot.test | RUN | 0.558703 | 2.662215 | 2.662117 | 0.001594 | 0.001508 | 0.001538 |
| bind_cancellation_slot_select.test | RUN | 0.578706 | 2.651247 | 2.572503 | 0.001527 | 0.001416 | 0.001481 |
| bind_executor.test | RUN | 0.405497 | 1.866387 | 1.642346 | 0.002204 | 0.001956 | 0.002225 |
| bind_executor_select.test | RUN | 0.405189 | 1.858925 | 1.628754 | 0.001887 | 0.001896 | 0.002080 |
| bind_immediate_executor.test | RUN | 0.295210 | 1.567659 | 1.206766 | 0.001449 | 0.001358 | 0.001392 |
| bind_immediate_executor_select.test | RUN | 0.304048 | 1.525069 | 1.321096 | 0.001296 | 0.001421 | 0.001492 |
| buffer.test | RUN | 0.167837 | 0.674116 | 0.608974 | 0.001434 | 0.001488 | 0.001389 |
| buffer_select.test | RUN | 0.169228 | 0.686060 | 0.598943 | 0.001233 | 0.001372 | 0.001371 |
| buffer_registration.test | LINK | 0.149676 | 0.324947 | 0.371349 | — | — | — |
| buffer_registration_select.test | LINK | 0.144353 | 0.314473 | 0.361119 | — | — | — |
| buffered_read_stream.test | RUN | 0.607587 | 4.059088 | 3.931406 | 0.001834 | 0.002276 | 0.001964 |
| buffered_read_stream_select.test | RUN | 0.604933 | 4.146216 | 3.768713 | 0.002859 | 0.002276 | 0.002293 |
| buffered_stream.test | RUN | 0.767720 | 6.841335 | 6.580591 | 0.002022 | 0.002334 | 0.002342 |
| buffered_stream_select.test | RUN | 0.808412 | 6.856124 | 6.367413 | 0.002122 | 0.002392 | 0.002160 |
| buffered_write_stream.test | RUN | 0.978970 | 5.973205 | 5.835920 | 0.002381 | 0.002383 | 0.002495 |
| buffered_write_stream_select.test | RUN | 1.021539 | 6.077940 | 5.580702 | 0.002348 | 0.002800 | 0.002118 |
| buffers_iterator.test | RUN | 0.304836 | 0.529166 | 0.880419 | 0.001659 | 0.001666 | 0.001874 |
| buffers_iterator_select.test | RUN | 0.302501 | 0.507274 | 0.868400 | 0.001628 | 0.001715 | 0.001269 |
| cancel_after.test | RUN | 0.805393 | 4.551605 | 4.305749 | 0.003723 | 0.003954 | 0.004277 |
| cancel_after_select.test | RUN | 0.819579 | 4.534984 | 4.251827 | 0.003648 | 0.003856 | 0.004574 |
| cancel_at.test | RUN | 0.700075 | 4.164106 | 4.089391 | 0.003181 | 0.003711 | 0.004082 |
| cancel_at_select.test | RUN | 0.652756 | 4.036900 | 4.053065 | 0.002984 | 0.003594 | 0.004052 |
| cancellation_signal.test | LINK | 0.101691 | 0.193947 | 0.246231 | — | — | — |
| cancellation_signal_select.test | LINK | 0.103446 | 0.201124 | 0.246750 | — | — | — |
| cancellation_state.test | LINK | 0.104444 | 0.200022 | 0.359661 | — | — | — |
| cancellation_state_select.test | LINK | 0.137175 | 0.209183 | 0.328997 | — | — | — |
| cancellation_type.test | LINK | 0.114018 | 0.156897 | 0.300878 | — | — | — |
| cancellation_type_select.test | LINK | 0.112263 | 0.155923 | 0.286756 | — | — | — |
| co_composed.test | RUN | 0.119046 | 0.168533 | 0.291370 | 0.001556 | 0.001527 | 0.001368 |
| co_composed_select.test | RUN | 0.118935 | 0.197742 | 0.277526 | 0.001490 | 0.001735 | 0.001351 |
| co_spawn.test | LINK | 0.114047 | 0.190977 | 0.292739 | — | — | — |
| co_spawn_select.test | LINK | 0.116343 | 0.221187 | 0.277977 | — | — | — |
| completion_condition.test | LINK | 0.142434 | 0.344262 | 0.382010 | — | — | — |
| completion_condition_select.test | LINK | 0.145297 | 0.348043 | 0.387988 | — | — | — |
| compose.test | RUN | 0.678325 | 3.442561 | 3.248456 | 0.002201 | 0.002186 | 0.002331 |
| compose_select.test | RUN | 0.702111 | 3.344054 | 3.230850 | 0.001722 | 0.002160 | 0.002283 |
| composed.test | RUN | 0.677219 | 3.368192 | 3.070688 | 0.001870 | 0.001884 | 0.002132 |
| composed_select.test | RUN | 0.681130 | 3.313901 | 3.106543 | 0.001757 | 0.001845 | 0.001926 |
| config.test | RUN | 0.319533 | 2.139337 | 1.315072 | 0.001806 | 0.002038 | 0.001964 |
| config_select.test | RUN | 0.368813 | 2.292086 | 1.541978 | 0.001866 | 0.001882 | 0.001875 |
| connect.test | LINK | 1.067372 | 7.223813 | 6.884429 | — | — | — |
| connect_select.test | LINK | 1.071988 | 7.244255 | 6.862970 | — | — | — |
| connect_pipe.test | RUN | 0.605119 | 2.799154 | 2.456920 | 0.001850 | 0.001937 | 0.002006 |
| connect_pipe_select.test | RUN | 0.614872 | 2.897107 | 2.625874 | 0.001561 | 0.001908 | 0.001934 |
| consign.test | RUN | 0.505810 | 2.639772 | 2.280601 | 0.002084 | 0.001612 | 0.001997 |
| consign_select.test | RUN | 0.518596 | 2.726412 | 2.258842 | 0.001714 | 0.001938 | 0.001846 |
| coroutine.test | LINK | 0.117293 | 0.276468 | 0.359957 | — | — | — |
| coroutine_select.test | LINK | 0.124499 | 0.271511 | 0.321990 | — | — | — |
| deadline_timer.test | RUN | 0.770365 | 3.629879 | 3.321826 | 0.003233 | 0.003264 | 0.003653 |
| deadline_timer_select.test | RUN | 0.774584 | 3.611794 | 3.326222 | 0.002854 | 0.003227 | 0.003247 |
| deferred.test | RUN | 0.128022 | 0.307162 | 0.346369 | 0.001560 | 0.001818 | 0.001820 |
| deferred_select.test | RUN | 0.125806 | 0.255615 | 0.313744 | 0.001411 | 0.001730 | 0.001748 |
| detached.test | LINK | 0.154078 | 0.283003 | 0.343308 | — | — | — |
| detached_select.test | LINK | 0.184642 | 0.281920 | 0.383745 | — | — | — |
| disposition.test | RUN | 0.138771 | 0.672211 | 0.625364 | 0.001412 | 0.001905 | 0.001902 |
| disposition_select.test | RUN | 0.140696 | 0.664140 | 0.643519 | 0.001308 | 0.001932 | 0.001914 |
| error.test | RUN | 0.124888 | 0.655855 | 0.567217 | 0.001283 | 0.001958 | 0.001970 |
| error_select.test | RUN | 0.126423 | 0.654556 | 0.576412 | 0.001228 | 0.001855 | 0.002029 |
| file_base.test | LINK | 0.159448 | 0.553339 | 0.586249 | — | — | — |
| file_base_select.test | LINK | 0.157928 | 0.526316 | 0.471964 | — | — | — |
| fwd_first.test | LINK | 0.341325 | 0.791008 | 1.126085 | — | — | — |
| fwd_last.test | LINK | 0.350542 | 0.742050 | 1.113757 | — | — | — |
| generic_basic_endpoint.test | LINK | 0.136107 | 0.315250 | 0.414254 | — | — | — |
| generic_basic_endpoint_select.test | LINK | 0.141228 | 0.299290 | 0.407113 | — | — | — |
| generic_datagram_protocol.test | LINK | 0.547033 | 2.611050 | 2.360997 | — | — | — |
| generic_datagram_protocol_select.test | LINK | 0.543059 | 2.724261 | 2.275302 | — | — | — |
| generic_raw_protocol.test | LINK | 0.533822 | 2.836248 | 2.265176 | — | — | — |
| generic_raw_protocol_select.test | LINK | 0.527430 | 2.916535 | 2.304157 | — | — | — |
| generic_seq_packet_protocol.test | LINK | 0.470464 | 2.375725 | 1.950156 | — | — | — |
| generic_seq_packet_protocol_select.test | LINK | 0.472743 | 2.463099 | 1.935662 | — | — | — |
| generic_stream_protocol.test | LINK | 0.521413 | 2.522854 | 2.184584 | — | — | — |
| generic_stream_protocol_select.test | LINK | 0.501869 | 2.587594 | 2.280392 | — | — | — |
| high_resolution_timer.test | LINK | 0.217791 | 0.443342 | 0.664955 | — | — | — |
| high_resolution_timer_select.test | LINK | 0.224803 | 0.458931 | 0.683114 | — | — | — |
| immediate.test | LINK | 0.156452 | 0.350733 | 0.421753 | — | — | — |
| immediate_select.test | LINK | 0.156630 | 0.351337 | 0.421688 | — | — | — |
| inline_executor.test | RUN | 0.194539 | 0.556585 | 0.611508 | 0.001375 | 0.001389 | 0.001498 |
| inline_executor_select.test | RUN | 0.189774 | 0.605024 | 0.621267 | 0.001302 | 0.001490 | 0.001447 |
| inline_or_executor.test | RUN | 0.292804 | 1.641408 | 1.205938 | 0.001374 | 0.001499 | 0.001507 |
| inline_or_executor_select.test | RUN | 0.306313 | 1.652543 | 1.339879 | 0.001284 | 0.001486 | 0.001565 |
| io_context.test | RUN | 0.435612 | 2.495743 | 2.150878 | 0.002328 | 0.002213 | 0.002342 |
| io_context_select.test | RUN | 0.449115 | 2.431414 | 2.127251 | 0.002109 | 0.002234 | 0.002448 |
| io_context_strand.test | RUN | 0.446696 | 2.151771 | 1.755626 | 0.002395 | 0.002175 | 0.002538 |
| io_context_strand_select.test | RUN | 0.459827 | 2.154046 | 1.755919 | 0.002060 | 0.002343 | 0.002178 |
| ip_address.test | LINK | 0.172946 | 0.510458 | 0.496997 | — | — | — |
| ip_address_select.test | LINK | 0.173417 | 0.508470 | 0.493649 | — | — | — |
| ip_address_v4.test | LINK | 0.177209 | 0.512467 | 0.741556 | — | — | — |
| ip_address_v4_select.test | LINK | 0.181320 | 0.516188 | 0.788446 | — | — | — |
| ip_address_v6.test | LINK | 0.199947 | 0.651964 | 1.041653 | — | — | — |
| ip_address_v6_select.test | LINK | 0.200544 | 0.669906 | 1.049386 | — | — | — |
| ip_basic_endpoint.test | LINK | 0.156301 | 0.473630 | 0.666865 | — | — | — |
| ip_basic_endpoint_select.test | LINK | 0.156348 | 0.472244 | 0.662470 | — | — | — |
| ip_basic_resolver.test | LINK | 0.241938 | 0.707572 | 1.009682 | — | — | — |
| ip_basic_resolver_select.test | LINK | 0.249324 | 0.708303 | 1.034659 | — | — | — |
| ip_basic_resolver_entry.test | LINK | 0.090877 | 0.236825 | 0.282959 | — | — | — |
| ip_basic_resolver_entry_select.test | LINK | 0.087297 | 0.230419 | 0.281194 | — | — | — |
| ip_basic_resolver_iterator.test | LINK | 0.156522 | 0.460179 | 0.629729 | — | — | — |
| ip_basic_resolver_iterator_select.test | LINK | 0.154774 | 0.457360 | 0.631411 | — | — | — |
| ip_basic_resolver_query.test | LINK | 0.153498 | 0.455724 | 0.634404 | — | — | — |
| ip_basic_resolver_query_select.test | LINK | 0.146750 | 0.465111 | 0.603677 | — | — | — |
| ip_host_name.test | RUN | 0.154789 | 0.590745 | 0.695078 | 0.001345 | 0.001663 | 0.001408 |
| ip_host_name_select.test | RUN | 0.157970 | 0.599889 | 0.687520 | 0.001454 | 0.001676 | 0.001700 |
| ip_icmp.test | RUN | 0.697588 | 5.653532 | 4.773434 | 0.001424 | 0.001886 | 0.001758 |
| ip_icmp_select.test | RUN | 0.702087 | 5.818258 | 4.614640 | 0.001494 | 0.001887 | 0.001448 |
| ip_multicast.test | RUN | 0.417289 | 2.845360 | 2.595901 | 0.001668 | 0.001978 | 0.001554 |
| ip_multicast_select.test | RUN | 0.441294 | 2.834286 | 2.570343 | 0.001520 | 0.001626 | 0.001834 |
| ip_resolver_query_base.test | LINK | 0.098627 | 0.248008 | 0.291184 | — | — | — |
| ip_resolver_query_base_select.test | LINK | 0.098088 | 0.250762 | 0.292140 | — | — | — |
| ip_tcp.test | RUN | 0.965519 | 8.388125 | 7.670965 | 0.002106 | 0.002725 | 0.002610 |
| ip_tcp_select.test | RUN | 0.994495 | 8.806609 | 7.467853 | 0.002001 | 0.002698 | 0.002438 |
| ip_udp.test | RUN | 0.708852 | 6.129765 | 5.233908 | 0.001684 | 0.001810 | 0.001928 |
| ip_udp_select.test | RUN | 0.711166 | 6.186378 | 5.088150 | 0.001670 | 0.001945 | 0.002038 |
| ip_unicast.test | RUN | 0.395601 | 2.659139 | 2.422186 | 0.001560 | 0.001878 | 0.001844 |
| ip_unicast_select.test | RUN | 0.393455 | 2.656431 | 2.380992 | 0.001548 | 0.001860 | 0.001571 |
| ip_v6_only.test | RUN | 0.396577 | 2.672017 | 2.588120 | 0.001542 | 0.001934 | 0.001660 |
| ip_v6_only_select.test | RUN | 0.403787 | 2.778257 | 2.538670 | 0.001572 | 0.001511 | 0.001698 |
| is_read_buffered.test | RUN | 0.273581 | 0.811973 | 1.291410 | 0.001448 | 0.001608 | 0.001668 |
| is_read_buffered_select.test | RUN | 0.278218 | 0.800537 | 1.328459 | 0.001421 | 0.001823 | 0.001496 |
| is_write_buffered.test | RUN | 0.280256 | 0.800602 | 1.200952 | 0.001559 | 0.001716 | 0.001734 |
| is_write_buffered_select.test | RUN | 0.293036 | 0.811204 | 1.239334 | 0.001458 | 0.001571 | 0.001730 |
| local_basic_endpoint.test | LINK | 0.149077 | 0.456547 | 0.638086 | — | — | — |
| local_basic_endpoint_select.test | LINK | 0.143829 | 0.455615 | 0.640401 | — | — | — |
| local_connect_pair.test | LINK | 0.357219 | 2.551897 | 2.137397 | — | — | — |
| local_connect_pair_select.test | LINK | 0.357729 | 2.543882 | 2.117671 | — | — | — |
| local_datagram_protocol.test | LINK | 0.485978 | 3.400697 | 3.031513 | — | — | — |
| local_datagram_protocol_select.test | LINK | 0.502673 | 3.421294 | 2.991070 | — | — | — |
| local_seq_packet_protocol.test | LINK | 0.484699 | 3.403560 | 2.970761 | — | — | — |
| local_seq_packet_protocol_select.test | LINK | 0.488195 | 3.446419 | 2.955828 | — | — | — |
| local_stream_protocol.test | LINK | 0.469673 | 3.284361 | 2.789147 | — | — | — |
| local_stream_protocol_select.test | LINK | 0.498694 | 3.313777 | 2.769476 | — | — | — |
| placeholders.test | LINK | 0.089013 | 0.239834 | 0.312665 | — | — | — |
| placeholders_select.test | LINK | 0.089013 | 0.241508 | 0.310570 | — | — | — |
| posix_basic_descriptor.test | LINK | 0.238449 | 0.685287 | 0.904709 | — | — | — |
| posix_basic_descriptor_select.test | LINK | 0.249080 | 0.735487 | 1.096200 | — | — | — |
| posix_basic_stream_descriptor.test | LINK | 0.305293 | 0.676153 | 0.882156 | — | — | — |
| posix_basic_stream_descriptor_select.test | LINK | 0.376563 | 0.703119 | 1.079865 | — | — | — |
| posix_descriptor_base.test | LINK | 0.143464 | 0.262137 | 0.304131 | — | — | — |
| posix_descriptor_base_select.test | LINK | 0.142192 | 0.260445 | 0.296871 | — | — | — |
| posix_stream_descriptor.test | LINK | 0.701286 | 3.188351 | 2.919784 | — | — | — |
| posix_stream_descriptor_select.test | LINK | 0.721881 | 3.227029 | 2.912959 | — | — | — |
| prepend.test | RUN | 0.540563 | 2.598053 | 2.227259 | 0.001834 | 0.001862 | 0.001947 |
| prepend_select.test | RUN | 0.541372 | 2.635810 | 2.229893 | 0.001644 | 0.001838 | 0.001938 |
| random_access_file.test | LINK | 0.326350 | 0.656647 | 0.743514 | — | — | — |
| random_access_file_select.test | LINK | 0.340639 | 0.702689 | 0.919742 | — | — | — |
| read.test | RUN | 1.613796 | 16.321874 | 13.026213 | 0.002578 | 0.002340 | 0.002365 |
| read_select.test | RUN | 1.639436 | 16.290698 | 13.080281 | 0.002490 | 0.002516 | 0.002442 |
| read_at.test | RUN | 1.608789 | 23.721613 | 18.499575 | 0.002713 | 0.002527 | 0.002671 |
| read_at_select.test | RUN | 1.638707 | 23.490648 | 18.657215 | 0.002902 | 0.002747 | 0.002398 |
| read_until.test | RUN | 0.888834 | 6.220921 | 4.789801 | 0.002227 | 0.002162 | 0.002046 |
| read_until_select.test | RUN | 0.912881 | 6.220960 | 4.945077 | 0.002030 | 0.001714 | 0.002068 |
| readable_pipe.test | LINK | 0.589453 | 2.662460 | 2.280875 | — | — | — |
| readable_pipe_select.test | LINK | 0.604985 | 2.776383 | 2.466683 | — | — | — |
| recycling_allocator.test | RUN | 0.174659 | 0.402715 | 0.460848 | 0.002154 | 0.001775 | 0.001788 |
| recycling_allocator_select.test | RUN | 0.178074 | 0.407328 | 0.474835 | 0.001643 | 0.001644 | 0.001850 |
| redirect_disposition.test | LINK | 0.742699 | 3.774332 | 3.598044 | — | — | — |
| redirect_disposition_select.test | LINK | 0.743184 | 3.777695 | 3.554481 | — | — | — |
| redirect_error.test | LINK | 0.664360 | 3.197962 | 3.062372 | — | — | — |
| redirect_error_select.test | LINK | 0.672152 | 3.179888 | 3.083670 | — | — | — |
| registered_buffer.test | LINK | 0.178361 | 0.317169 | 0.415240 | — | — | — |
| registered_buffer_select.test | LINK | 0.187941 | 0.304834 | 0.394280 | — | — | — |
| signal_set.test | RUN | 0.563069 | 2.615378 | 2.114633 | 0.001784 | 0.001804 | 0.001882 |
| signal_set_select.test | RUN | 0.586828 | 2.707670 | 2.303360 | 0.001962 | 0.001765 | 0.001836 |
| signal_set_base.test | LINK | 0.142234 | 0.244205 | 0.297441 | — | — | — |
| signal_set_base_select.test | LINK | 0.146179 | 0.249676 | 0.296823 | — | — | — |
| socket_base.test | RUN | 0.663659 | 4.057224 | 3.331276 | 0.002135 | 0.002288 | 0.002086 |
| socket_base_select.test | RUN | 0.683683 | 3.985189 | 3.336851 | 0.002040 | 0.002498 | 0.001988 |
| spawn.test | RUN | 1.113314 | 6.249768 | 5.702313 | 0.002102 | 0.004927 | 0.001998 |
| spawn_select.test | RUN | 1.109417 | 6.220666 | 5.724830 | 0.001995 | 0.002084 | 0.002003 |
| static_thread_pool.test | RUN | 0.304060 | 0.602192 | 0.689656 | 0.001769 | 0.001427 | 0.001586 |
| static_thread_pool_select.test | RUN | 0.315080 | 0.624074 | 0.870706 | 0.001758 | 0.001589 | 0.001834 |
| steady_timer.test | LINK | 0.341899 | 0.654200 | 1.051035 | — | — | — |
| steady_timer_select.test | LINK | 0.358049 | 0.681256 | 1.055505 | — | — | — |
| strand.test | RUN | 0.719633 | 3.434659 | 3.252026 | 0.002327 | 0.002410 | 0.002489 |
| strand_select.test | RUN | 0.744800 | 3.395848 | 3.277019 | 0.002433 | 0.002315 | 0.002650 |
| stream_file.test | LINK | 0.330692 | 0.608556 | 0.710464 | — | — | — |
| stream_file_select.test | LINK | 0.344842 | 0.599219 | 0.908122 | — | — | — |
| streambuf.test | RUN | 0.187283 | 0.352556 | 0.500127 | 0.001437 | 0.001307 | 0.001628 |
| streambuf_select.test | RUN | 0.163555 | 0.340667 | 0.429177 | 0.001468 | 0.001343 | 0.001305 |
| system_timer.test | LINK | 0.469492 | 2.302576 | 2.022062 | — | — | — |
| system_timer_select.test | LINK | 0.473398 | 2.302902 | 1.867948 | — | — | — |
| system_context.test | LINK | 0.216957 | 0.393979 | 0.434583 | — | — | — |
| system_context_select.test | LINK | 0.218585 | 0.433711 | 0.549788 | — | — | — |
| system_executor.test | LINK | 0.271317 | 1.334948 | 0.912606 | — | — | — |
| system_executor_select.test | LINK | 0.283967 | 1.291676 | 0.998801 | — | — | — |
| this_coro.test | LINK | 0.085841 | 0.151960 | 0.177974 | — | — | — |
| this_coro_select.test | LINK | 0.084665 | 0.152379 | 0.170060 | — | — | — |
| thread_pool.test | RUN | 0.335236 | 1.680387 | 1.168696 | 0.002389 | 0.002490 | 0.002419 |
| thread_pool_select.test | RUN | 0.346308 | 1.679974 | 1.238178 | 0.002521 | 0.002394 | 0.002445 |
| time_traits.test | LINK | 0.178637 | 0.312196 | 0.352802 | — | — | — |
| time_traits_select.test | LINK | 0.179229 | 0.324849 | 0.349623 | — | — | — |
| ts_buffer.test | LINK | 0.168758 | 0.359306 | 0.385740 | — | — | — |
| ts_buffer_select.test | LINK | 0.173945 | 0.372287 | 0.374938 | — | — | — |
| ts_executor.test | LINK | 0.245239 | 0.725934 | 0.596444 | — | — | — |
| ts_executor_select.test | LINK | 0.257895 | 0.804574 | 0.723046 | — | — | — |
| ts_internet.test | LINK | 0.263445 | 0.805293 | 0.732402 | — | — | — |
| ts_internet_select.test | LINK | 0.271370 | 0.847629 | 0.787629 | — | — | — |
| ts_io_context.test | LINK | 0.223737 | 0.629265 | 0.696094 | — | — | — |
| ts_io_context_select.test | LINK | 0.221818 | 0.664641 | 0.899740 | — | — | — |
| ts_net.test | LINK | 0.312201 | 0.987347 | 1.397930 | — | — | — |
| ts_net_select.test | LINK | 0.334345 | 0.999587 | 1.478416 | — | — | — |
| ts_netfwd.test | LINK | 0.331447 | 0.995210 | 1.474099 | — | — | — |
| ts_netfwd_select.test | LINK | 0.361574 | 1.016183 | 1.470683 | — | — | — |
| ts_socket.test | LINK | 0.281915 | 0.779177 | 1.085071 | — | — | — |
| ts_socket_select.test | LINK | 0.291275 | 0.803310 | 1.130599 | — | — | — |
| ts_timer.test | LINK | 0.244513 | 0.686867 | 0.959560 | — | — | — |
| ts_timer_select.test | LINK | 0.258508 | 0.706207 | 0.973163 | — | — | — |
| use_awaitable.test | LINK | 0.087401 | 0.231581 | 0.274953 | — | — | — |
| use_awaitable_select.test | LINK | 0.084966 | 0.235461 | 0.267025 | — | — | — |
| wait_traits.test | LINK | 0.087504 | 0.233651 | 0.265779 | — | — | — |
| wait_traits_select.test | LINK | 0.087887 | 0.236609 | 0.269643 | — | — | — |
| windows_basic_object_handle.test | LINK | 0.085267 | 0.229073 | 0.267289 | — | — | — |
| windows_basic_object_handle_select.test | LINK | 0.083618 | 0.235573 | 0.262831 | — | — | — |
| windows_basic_overlapped_handle.test | LINK | 0.085469 | 0.234486 | 0.266262 | — | — | — |
| windows_basic_overlapped_handle_select.test | LINK | 0.085606 | 0.232491 | 0.259608 | — | — | — |
| windows_basic_random_access_handle.test | LINK | 0.087627 | 0.229111 | 0.271227 | — | — | — |
| windows_basic_random_access_handle_select.test | LINK | 0.090418 | 0.229284 | 0.272298 | — | — | — |
| windows_basic_stream_handle.test | LINK | 0.091025 | 0.226325 | 0.265578 | — | — | — |
| windows_basic_stream_handle_select.test | LINK | 0.089180 | 0.227849 | 0.272425 | — | — | — |
| windows_object_handle.test | LINK | 0.277709 | 0.627976 | 0.720713 | — | — | — |
| windows_object_handle_select.test | LINK | 0.299634 | 0.672172 | 0.903596 | — | — | — |
| windows_overlapped_ptr.test | LINK | 0.265752 | 0.670382 | 0.817945 | — | — | — |
| windows_overlapped_ptr_select.test | LINK | 0.270019 | 0.719085 | 1.027615 | — | — | — |
| windows_random_access_handle.test | LINK | 0.331601 | 0.640258 | 0.719420 | — | — | — |
| windows_random_access_handle_select.test | LINK | 0.344026 | 0.672400 | 0.934808 | — | — | — |
| windows_stream_handle.test | LINK | 0.333021 | 0.622482 | 0.746534 | — | — | — |
| windows_stream_handle_select.test | LINK | 0.360569 | 0.677448 | 0.927354 | — | — | — |
| writable_pipe.test | LINK | 0.594153 | 2.617363 | 2.385953 | — | — | — |
| writable_pipe_select.test | LINK | 0.611422 | 2.720982 | 2.532572 | — | — | — |
| write.test | RUN | 1.641146 | 13.300838 | 13.602093 | 0.002370 | 0.002289 | 0.002591 |
| write_select.test | RUN | 1.661125 | 13.383210 | 13.681658 | 0.002452 | 0.002422 | 0.002526 |
| write_at.test | RUN | 1.618015 | 20.788298 | 18.218699 | 0.002687 | 0.002640 | 0.002563 |
| write_at_select.test | RUN | 1.630550 | 20.700132 | 18.349924 | 0.002237 | 0.002585 | 0.002666 |
| any_executor.test | RUN | 0.637513 | 7.008315 | 5.568936 | 0.002401 | 0.002305 | 0.002489 |
| blocking.test | RUN | 1.423763 | 7.938195 | 10.350000 | 0.006165 | 0.006108 | 0.006615 |
| blocking_adaptation.test | RUN | 0.715902 | 3.377139 | 4.794665 | 0.003741 | 0.003559 | 0.003804 |
| context_as.test | RUN | 0.432821 | 2.397115 | 1.724260 | 0.002034 | 0.001988 | 0.002007 |
| executor.test | RUN | 0.122336 | 0.239974 | 0.303941 | 0.001757 | 0.001801 | 0.001772 |
| inline_exception_handling.test | RUN | 1.393199 | 7.011235 | 9.291964 | 0.004825 | 0.004326 | 0.004455 |
| invocable_archetype.test | RUN | 0.119440 | 0.249387 | 0.294797 | 0.001763 | 0.001574 | 0.001818 |
| mapping.test | RUN | 1.308336 | 7.068521 | 9.821079 | 0.004857 | 0.004342 | 0.004685 |
| outstanding_work.test | RUN | 0.650174 | 3.176111 | 4.510209 | 0.003817 | 0.003749 | 0.003667 |
| prefer_only.test | RUN | 0.229853 | 1.321391 | 1.007820 | 0.001808 | 0.001787 | 0.001752 |
| relationship.test | RUN | 0.597969 | 3.074631 | 4.390938 | 0.002640 | 0.003730 | 0.003726 |

## Native runtime replay

Child CPU of successful native tests, with process startup included; GDB and expected-failure executions are correctness-only. Two warmups per compiler, four A/A observations per reference and mirrored ABBA/BAAB blocks. Short tests repeat up to ten times, identically across compilers; CPU is divided by repetitions before summing tests. Executable hashes and output checks stay outside process timers. All runs must pass their original native oracle. Output hashes record equality or variation across the complete campaign; test reports may print times or addresses.

| Reference | CPU speedup ↑ | Paired blocks | A/A speedup |
|---|---:|---|---:|
| gcc | 0.989× | 0.981×, 0.997× | 1.021× |
| clang | 1.001× | 0.998×, 1.004× | 1.009× |

† A/A drift or paired-block spread exceeds 5%; small differences are unresolved. 1375 native tests in the full cohort; 1359 have byte-identical output throughout. [Raw replay evidence in CI artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43558051632).

| Test | Repeats | CPU cppgm | GCC | Clang | Output check |
|---|---:|---:|---:|---:|---|
| any_completion_executor.test | 3 | 0.001483s | 0.001470s | 0.001432s | byte equality observed |
| any_completion_executor_select.test | 4 | 0.001466s | 0.001442s | 0.001420s | byte equality observed |
| any_completion_handler.test | 10 | 0.001190s | 0.001165s | 0.001165s | byte equality observed |
| any_completion_handler_select.test | 9 | 0.001182s | 0.001176s | 0.001140s | byte equality observed |
| any_io_executor.test | 5 | 0.001424s | 0.001443s | 0.001411s | byte equality observed |
| any_io_executor_select.test | 6 | 0.001440s | 0.001426s | 0.001414s | byte equality observed |
| append.test | 10 | 0.001142s | 0.001118s | 0.001094s | byte equality observed |
| append_select.test | 10 | 0.001116s | 0.001100s | 0.001067s | byte equality observed |
| as_tuple.test | 8 | 0.001197s | 0.001180s | 0.001172s | byte equality observed |
| as_tuple_select.test | 5 | 0.001130s | 0.001138s | 0.001092s | byte equality observed |
| bind_allocator.test | 1 | 0.001619s | 0.001583s | 0.001568s | byte equality observed |
| bind_allocator_select.test | 1 | 0.001309s | 0.001302s | 0.001275s | byte equality observed |
| bind_cancellation_slot.test | 10 | 0.001103s | 0.001099s | 0.001039s | byte equality observed |
| bind_cancellation_slot_select.test | 10 | 0.000976s | 0.000983s | 0.000935s | byte equality observed |
| bind_executor.test | 1 | 0.001576s | 0.001633s | 0.001550s | byte equality observed |
| bind_executor_select.test | 1 | 0.001972s | 0.001967s | 0.001904s | byte equality observed |
| bind_immediate_executor.test | 10 | 0.001014s | 0.000995s | 0.001017s | byte equality observed |
| bind_immediate_executor_select.test | 10 | 0.000987s | 0.000985s | 0.000987s | byte equality observed |
| buffer.test | 10 | 0.000931s | 0.000937s | 0.000930s | byte equality observed |
| buffer_select.test | 10 | 0.000938s | 0.000910s | 0.000949s | byte equality observed |
| buffered_read_stream.test | 10 | 0.001222s | 0.001191s | 0.001219s | byte equality observed |
| buffered_read_stream_select.test | 10 | 0.001195s | 0.001159s | 0.001213s | byte equality observed |
| buffered_stream.test | 10 | 0.001213s | 0.001209s | 0.001179s | byte equality observed |
| buffered_stream_select.test | 10 | 0.001194s | 0.001160s | 0.001202s | byte equality observed |
| buffered_write_stream.test | 10 | 0.001223s | 0.001192s | 0.001250s | byte equality observed |
| buffered_write_stream_select.test | 10 | 0.001194s | 0.001125s | 0.001145s | byte equality observed |
| buffers_iterator.test | 10 | 0.000932s | 0.000926s | 0.000963s | byte equality observed |
| buffers_iterator_select.test | 10 | 0.000892s | 0.000888s | 0.000896s | byte equality observed |
| cancel_after.test | 1 | 0.003409s | 0.003121s | 0.003128s | byte equality observed |
| cancel_after_select.test | 1 | 0.002560s | 0.002653s | 0.002429s | byte equality observed |
| cancel_at.test | 1 | 0.003537s | 0.003659s | 0.003457s | byte equality observed |
| cancel_at_select.test | 1 | 0.003782s | 0.003626s | 0.003291s | byte equality observed |
| co_composed.test | 10 | 0.000939s | 0.000922s | 0.000953s | byte equality observed |
| co_composed_select.test | 7 | 0.000915s | 0.000909s | 0.000916s | byte equality observed |
| compose.test | 1 | 0.001578s | 0.001647s | 0.001513s | byte equality observed |
| compose_select.test | 1 | 0.001295s | 0.001293s | 0.001243s | byte equality observed |
| composed.test | 1 | 0.002059s | 0.002004s | 0.001914s | byte equality observed |
| composed_select.test | 1 | 0.001393s | 0.001570s | 0.001383s | byte equality observed |
| config.test | 10 | 0.001053s | 0.001009s | 0.000992s | byte equality observed |
| config_select.test | 10 | 0.001022s | 0.000999s | 0.000961s | byte equality observed |
| connect_pipe.test | 9 | 0.001047s | 0.001021s | 0.000986s | byte equality observed |
| connect_pipe_select.test | 10 | 0.001011s | 0.000998s | 0.000985s | byte equality observed |
| consign.test | 10 | 0.001013s | 0.000988s | 0.000982s | byte equality observed |
| consign_select.test | 10 | 0.000968s | 0.000961s | 0.000943s | byte equality observed |
| deadline_timer.test | 1 | 0.002193s | 0.002215s | 0.002254s | byte equality observed |
| deadline_timer_select.test | 1 | 0.002073s | 0.002049s | 0.001976s | byte equality observed |
| deferred.test | 10 | 0.000927s | 0.000937s | 0.000909s | byte equality observed |
| deferred_select.test | 10 | 0.000874s | 0.000878s | 0.000875s | byte equality observed |
| disposition.test | 10 | 0.000937s | 0.000938s | 0.000921s | byte equality observed |
| disposition_select.test | 10 | 0.000930s | 0.000930s | 0.000916s | byte equality observed |
| error.test | 10 | 0.000896s | 0.000902s | 0.000885s | byte equality observed |
| error_select.test | 10 | 0.000894s | 0.000899s | 0.000885s | byte equality observed |
| inline_executor.test | 10 | 0.000908s | 0.000914s | 0.000898s | byte equality observed |
| inline_executor_select.test | 10 | 0.000893s | 0.000918s | 0.000894s | byte equality observed |
| inline_or_executor.test | 10 | 0.000907s | 0.000924s | 0.000894s | byte equality observed |
| inline_or_executor_select.test | 10 | 0.000898s | 0.000923s | 0.000896s | byte equality observed |
| io_context.test | 1 | 0.001768s | 0.001856s | 0.001824s | byte equality observed |
| io_context_select.test | 1 | 0.002167s | 0.002174s | 0.002194s | byte equality observed |
| io_context_strand.test | 1 | 0.002099s | 0.002316s | 0.002229s | byte equality observed |
| io_context_strand_select.test | 1 | 0.001925s | 0.001978s | 0.001945s | byte equality observed |
| ip_host_name.test | 10 | 0.000925s | 0.001099s | 0.000947s | byte equality observed |
| ip_host_name_select.test | 10 | 0.000867s | 0.001153s | 0.000878s | byte equality observed |
| ip_icmp.test | 10 | 0.000894s | 0.001073s | 0.000904s | byte equality observed |
| ip_icmp_select.test | 10 | 0.000892s | 0.001269s | 0.000893s | byte equality observed |
| ip_multicast.test | 10 | 0.000960s | 0.001031s | 0.001005s | byte equality observed |
| ip_multicast_select.test | 10 | 0.000936s | 0.000954s | 0.000972s | byte equality observed |
| ip_tcp.test | 10 | 0.001326s | 0.001357s | 0.001376s | byte equality observed |
| ip_tcp_select.test | 10 | 0.001275s | 0.001263s | 0.001305s | byte equality observed |
| ip_udp.test | 10 | 0.000985s | 0.000986s | 0.001015s | byte equality observed |
| ip_udp_select.test | 10 | 0.000983s | 0.001014s | 0.001001s | byte equality observed |
| ip_unicast.test | 10 | 0.000952s | 0.001009s | 0.000973s | byte equality observed |
| ip_unicast_select.test | 10 | 0.000932s | 0.000956s | 0.000957s | byte equality observed |
| ip_v6_only.test | 10 | 0.000963s | 0.000993s | 0.000983s | byte equality observed |
| ip_v6_only_select.test | 10 | 0.000950s | 0.000982s | 0.000986s | byte equality observed |
| is_read_buffered.test | 10 | 0.000885s | 0.000920s | 0.000907s | byte equality observed |
| is_read_buffered_select.test | 10 | 0.000885s | 0.000919s | 0.000902s | byte equality observed |
| is_write_buffered.test | 10 | 0.000878s | 0.000918s | 0.000934s | byte equality observed |
| is_write_buffered_select.test | 10 | 0.000877s | 0.000916s | 0.000930s | byte equality observed |
| prepend.test | 10 | 0.000965s | 0.000988s | 0.001005s | byte equality observed |
| prepend_select.test | 10 | 0.000948s | 0.000972s | 0.000975s | byte equality observed |
| read.test | 10 | 0.001309s | 0.001326s | 0.001332s | byte equality observed |
| read_at.test | 10 | 0.001532s | 0.001584s | 0.001472s | byte equality observed |
| read_at_select.test | 10 | 0.001544s | 0.001578s | 0.001482s | byte equality observed |
| read_select.test | 10 | 0.001327s | 0.001313s | 0.001346s | byte equality observed |
| read_until.test | 10 | 0.001090s | 0.001106s | 0.001129s | byte equality observed |
| read_until_select.test | 10 | 0.001105s | 0.001075s | 0.001109s | byte equality observed |
| recycling_allocator.test | 10 | 0.000923s | 0.000942s | 0.000953s | byte equality observed |
| recycling_allocator_select.test | 10 | 0.000919s | 0.000940s | 0.000955s | byte equality observed |
| signal_set.test | 10 | 0.000937s | 0.000923s | 0.000958s | byte equality observed |
| signal_set_select.test | 10 | 0.000945s | 0.000931s | 0.000955s | byte equality observed |
| socket_base.test | 10 | 0.001028s | 0.001066s | 0.001083s | byte equality observed |
| socket_base_select.test | 10 | 0.001009s | 0.001009s | 0.001060s | byte equality observed |
| spawn.test | 10 | 0.001131s | 0.001128s | 0.001135s | byte equality observed |
| spawn_select.test | 10 | 0.001114s | 0.001108s | 0.001130s | byte equality observed |
| static_thread_pool.test | 10 | 0.000916s | 0.000918s | 0.000946s | byte equality observed |
| static_thread_pool_select.test | 10 | 0.000892s | 0.000887s | 0.000936s | byte equality observed |
| strand.test | 1 | 0.001802s | 0.001939s | 0.001803s | byte equality observed |
| strand_select.test | 1 | 0.001497s | 0.001454s | 0.001514s | byte equality observed |
| streambuf.test | 10 | 0.000977s | 0.000964s | 0.000997s | byte equality observed |
| streambuf_select.test | 10 | 0.000949s | 0.000962s | 0.000971s | byte equality observed |
| thread_pool.test | 10 | 0.001914s | 0.001920s | 0.002015s | byte equality observed |
| thread_pool_select.test | 10 | 0.001889s | 0.001908s | 0.001980s | byte equality observed |
| write.test | 10 | 0.001351s | 0.001319s | 0.001371s | byte equality observed |
| write_at.test | 10 | 0.001464s | 0.001431s | 0.001458s | byte equality observed |
| write_at_select.test | 10 | 0.001424s | 0.001412s | 0.001401s | byte equality observed |
| write_select.test | 10 | 0.001401s | 0.001336s | 0.001356s | byte equality observed |

[Raw outcomes, commands and timing observations in CI artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43558051632).

[Comparison records](../results.json) · [Host and toolchains](../metadata.json)
