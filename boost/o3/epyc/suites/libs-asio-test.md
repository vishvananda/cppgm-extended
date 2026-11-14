# libs/asio/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| any_completion_executor.test | RUN | 0.643161 | 3.769002 | 3.464443 | 0.002432 | 0.002578 | 0.002474 |
| any_completion_executor_select.test | RUN | 0.650150 | 3.761135 | 3.694274 | 0.002442 | 0.002451 | 0.002270 |
| any_completion_handler.test | RUN | 0.692732 | 3.343710 | 3.299294 | 0.001731 | 0.001824 | 0.001795 |
| any_completion_handler_select.test | RUN | 0.711801 | 3.472011 | 3.311005 | 0.001814 | 0.002136 | 0.001749 |
| any_io_executor.test | RUN | 0.654577 | 3.993708 | 3.805704 | 0.002164 | 0.002468 | 0.002334 |
| any_io_executor_select.test | RUN | 0.673553 | 3.965542 | 3.992318 | 0.002088 | 0.002462 | 0.002433 |
| append.test | RUN | 0.605804 | 3.176882 | 2.820614 | 0.001813 | 0.001938 | 0.001879 |
| append_select.test | RUN | 0.613969 | 3.259972 | 2.888424 | 0.001758 | 0.001964 | 0.001650 |
| as_tuple.test | RUN | 0.786076 | 3.849718 | 4.076140 | 0.001982 | 0.001972 | 0.002072 |
| as_tuple_select.test | RUN | 0.781061 | 3.817408 | 4.060971 | 0.001638 | 0.001941 | 0.001885 |
| associated_allocator.test | LINK | 0.164750 | 0.346274 | 0.437270 | — | — | — |
| associated_allocator_select.test | LINK | 0.174532 | 0.324102 | 0.421637 | — | — | — |
| associated_cancellation_slot.test | LINK | 0.183375 | 0.360158 | 0.470852 | — | — | — |
| associated_cancellation_slot_select.test | LINK | 0.183553 | 0.350985 | 0.475467 | — | — | — |
| associated_executor.test | LINK | 0.257994 | 0.622328 | 0.789702 | — | — | — |
| associated_executor_select.test | LINK | 0.255270 | 0.628748 | 0.798452 | — | — | — |
| associated_immediate_executor.test | LINK | 0.240942 | 0.579150 | 0.726594 | — | — | — |
| associated_immediate_executor_select.test | LINK | 0.240682 | 0.581246 | 0.723328 | — | — | — |
| associator.test | LINK | 0.132674 | 0.282633 | 0.383651 | — | — | — |
| associator_select.test | LINK | 0.138614 | 0.282137 | 0.363069 | — | — | — |
| awaitable.test | LINK | 0.139013 | 0.294628 | 0.364922 | — | — | — |
| awaitable_select.test | LINK | 0.136400 | 0.289003 | 0.359379 | — | — | — |
| basic_datagram_socket.test | LINK | 0.404198 | 0.881276 | 1.423816 | — | — | — |
| basic_datagram_socket_select.test | LINK | 0.411178 | 0.907353 | 1.474700 | — | — | — |
| basic_deadline_timer.test | LINK | 0.504913 | 1.067347 | 1.627834 | — | — | — |
| basic_deadline_timer_select.test | LINK | 0.524422 | 1.095778 | 1.693348 | — | — | — |
| basic_file.test | LINK | 0.137232 | 0.281648 | 0.386292 | — | — | — |
| basic_file_select.test | LINK | 0.135522 | 0.291713 | 0.364258 | — | — | — |
| basic_random_access_file.test | LINK | 0.140231 | 0.288438 | 0.370972 | — | — | — |
| basic_random_access_file_select.test | LINK | 0.137547 | 0.293212 | 0.379063 | — | — | — |
| basic_raw_socket.test | LINK | 0.418726 | 0.902451 | 1.422094 | — | — | — |
| basic_raw_socket_select.test | LINK | 0.432531 | 0.926191 | 1.465558 | — | — | — |
| basic_readable_pipe.test | LINK | 0.404599 | 0.865773 | 1.237333 | — | — | — |
| basic_readable_pipe_select.test | LINK | 0.412560 | 0.909828 | 1.506342 | — | — | — |
| basic_seq_packet_socket.test | LINK | 0.416689 | 0.892753 | 1.433001 | — | — | — |
| basic_seq_packet_socket_select.test | LINK | 0.420535 | 0.917765 | 1.456347 | — | — | — |
| basic_signal_set.test | LINK | 0.382331 | 0.821513 | 1.149175 | — | — | — |
| basic_signal_set_select.test | LINK | 0.413417 | 0.875673 | 1.393361 | — | — | — |
| basic_socket_acceptor.test | LINK | 0.415962 | 0.899603 | 1.408949 | — | — | — |
| basic_socket_acceptor_select.test | LINK | 0.429751 | 0.923197 | 1.474427 | — | — | — |
| basic_stream_file.test | LINK | 0.136295 | 0.279719 | 0.396554 | — | — | — |
| basic_stream_file_select.test | LINK | 0.144754 | 0.284186 | 0.394501 | — | — | — |
| basic_stream_socket.test | LINK | 0.418650 | 0.895860 | 1.435192 | — | — | — |
| basic_stream_socket_select.test | LINK | 0.440345 | 0.941800 | 1.440374 | — | — | — |
| basic_streambuf.test | LINK | 0.203701 | 0.402878 | 0.585338 | — | — | — |
| basic_streambuf_select.test | LINK | 0.205604 | 0.411304 | 0.558810 | — | — | — |
| basic_waitable_timer.test | LINK | 0.385065 | 0.834008 | 1.345188 | — | — | — |
| basic_waitable_timer_select.test | LINK | 0.404157 | 0.866397 | 1.368831 | — | — | — |
| basic_writable_pipe.test | LINK | 0.400304 | 0.865291 | 1.251671 | — | — | — |
| basic_writable_pipe_select.test | LINK | 0.431510 | 0.920327 | 1.546961 | — | — | — |
| bind_allocator.test | RUN | 0.644734 | 3.281418 | 3.202228 | 0.002174 | 0.002392 | 0.002278 |
| bind_allocator_select.test | RUN | 0.676262 | 3.286862 | 3.156086 | 0.002364 | 0.001845 | 0.002042 |
| bind_cancellation_slot.test | RUN | 0.661549 | 3.284319 | 3.204534 | 0.001609 | 0.001558 | 0.001500 |
| bind_cancellation_slot_select.test | RUN | 0.678558 | 3.305269 | 3.094477 | 0.001496 | 0.001536 | 0.001517 |
| bind_executor.test | RUN | 0.442834 | 2.285607 | 1.936295 | 0.002080 | 0.002163 | 0.002188 |
| bind_executor_select.test | RUN | 0.450231 | 2.301916 | 1.942650 | 0.001969 | 0.002012 | 0.001968 |
| bind_immediate_executor.test | RUN | 0.339007 | 1.875585 | 1.229900 | 0.001592 | 0.001410 | 0.001386 |
| bind_immediate_executor_select.test | RUN | 0.351948 | 1.832801 | 1.369412 | 0.001431 | 0.001351 | 0.001411 |
| buffer.test | RUN | 0.206432 | 0.787953 | 0.769954 | 0.001364 | 0.001085 | 0.001381 |
| buffer_select.test | RUN | 0.203204 | 0.776496 | 0.768649 | 0.001362 | 0.001366 | 0.001352 |
| buffer_registration.test | LINK | 0.167619 | 0.400005 | 0.451532 | — | — | — |
| buffer_registration_select.test | LINK | 0.167550 | 0.403238 | 0.455793 | — | — | — |
| buffered_read_stream.test | RUN | 0.702561 | 4.933149 | 4.507954 | 0.001730 | 0.002302 | 0.002422 |
| buffered_read_stream_select.test | RUN | 0.712849 | 5.060705 | 4.724791 | 0.001686 | 0.002230 | 0.002348 |
| buffered_stream.test | RUN | 1.026932 | 7.665435 | 7.845790 | 0.002136 | 0.002216 | 0.002406 |
| buffered_stream_select.test | RUN | 1.051030 | 7.688349 | 7.555772 | 0.002340 | 0.002406 | 0.002396 |
| buffered_write_stream.test | RUN | 1.154092 | 6.695764 | 6.625991 | 0.001834 | 0.002276 | 0.002126 |
| buffered_write_stream_select.test | RUN | 1.173360 | 6.840214 | 6.302124 | 0.002346 | 0.002442 | 0.002272 |
| buffers_iterator.test | RUN | 0.386303 | 0.637709 | 1.112500 | 0.001636 | 0.001870 | 0.001656 |
| buffers_iterator_select.test | RUN | 0.379142 | 0.658952 | 1.097454 | 0.001778 | 0.001645 | 0.001786 |
| cancel_after.test | RUN | 0.919644 | 5.234332 | 4.855090 | 0.003923 | 0.003947 | 0.003835 |
| cancel_after_select.test | RUN | 0.948793 | 5.219723 | 4.861476 | 0.003435 | 0.003771 | 0.003920 |
| cancel_at.test | RUN | 0.928457 | 4.678262 | 4.637295 | 0.003580 | 0.003565 | 0.003568 |
| cancel_at_select.test | RUN | 0.870480 | 4.606614 | 4.600830 | 0.003390 | 0.003577 | 0.003351 |
| cancellation_signal.test | LINK | 0.132995 | 0.244608 | 0.292589 | — | — | — |
| cancellation_signal_select.test | LINK | 0.146857 | 0.251872 | 0.295919 | — | — | — |
| cancellation_state.test | LINK | 0.156812 | 0.253363 | 0.353426 | — | — | — |
| cancellation_state_select.test | LINK | 0.176415 | 0.251515 | 0.489804 | — | — | — |
| cancellation_type.test | LINK | 0.133247 | 0.197770 | 0.393659 | — | — | — |
| cancellation_type_select.test | LINK | 0.136173 | 0.198327 | 0.354574 | — | — | — |
| co_composed.test | RUN | 0.134739 | 0.198131 | 0.365292 | 0.001588 | 0.001376 | 0.001705 |
| co_composed_select.test | RUN | 0.135858 | 0.193432 | 0.387707 | 0.001576 | 0.001571 | 0.001495 |
| co_spawn.test | LINK | 0.134190 | 0.245953 | 0.378322 | — | — | — |
| co_spawn_select.test | LINK | 0.136226 | 0.279797 | 0.356541 | — | — | — |
| completion_condition.test | LINK | 0.170769 | 0.398740 | 0.495709 | — | — | — |
| completion_condition_select.test | LINK | 0.176250 | 0.435047 | 0.524817 | — | — | — |
| compose.test | RUN | 0.781362 | 4.100278 | 3.994646 | 0.002384 | 0.002317 | 0.002248 |
| compose_select.test | RUN | 0.800485 | 4.065746 | 3.938755 | 0.002199 | 0.002020 | 0.002203 |
| composed.test | RUN | 0.772817 | 4.066857 | 3.868511 | 0.001896 | 0.002077 | 0.002014 |
| composed_select.test | RUN | 0.759154 | 4.004632 | 3.760016 | 0.001944 | 0.001938 | 0.001824 |
| config.test | RUN | 0.392025 | 2.610584 | 1.682008 | 0.001904 | 0.001934 | 0.001811 |
| config_select.test | RUN | 0.429262 | 2.792291 | 1.935611 | 0.001500 | 0.001911 | 0.001753 |
| connect.test | LINK | 1.257912 | 8.852312 | 8.176395 | — | — | — |
| connect_select.test | LINK | 1.268153 | 8.905876 | 8.243943 | — | — | — |
| connect_pipe.test | RUN | 0.692114 | 3.285365 | 3.067658 | 0.001908 | 0.001990 | 0.001921 |
| connect_pipe_select.test | RUN | 0.712167 | 3.418751 | 3.290210 | 0.001668 | 0.002046 | 0.001830 |
| consign.test | RUN | 0.591509 | 3.238901 | 2.750548 | 0.001887 | 0.001958 | 0.001813 |
| consign_select.test | RUN | 0.603662 | 3.298834 | 2.696310 | 0.001741 | 0.002012 | 0.001614 |
| coroutine.test | LINK | 0.138979 | 0.329766 | 0.411396 | — | — | — |
| coroutine_select.test | LINK | 0.140776 | 0.351586 | 0.395093 | — | — | — |
| deadline_timer.test | RUN | 0.837412 | 4.235105 | 4.069196 | 0.003184 | 0.003209 | 0.002743 |
| deadline_timer_select.test | RUN | 0.862377 | 4.151457 | 4.013584 | 0.002834 | 0.002873 | 0.003188 |
| deferred.test | RUN | 0.155229 | 0.308754 | 0.386585 | 0.001760 | 0.001855 | 0.001701 |
| deferred_select.test | RUN | 0.158334 | 0.306417 | 0.379596 | 0.001519 | 0.001956 | 0.001778 |
| detached.test | LINK | 0.164651 | 0.331662 | 0.431531 | — | — | — |
| detached_select.test | LINK | 0.170581 | 0.340511 | 0.439643 | — | — | — |
| disposition.test | RUN | 0.152663 | 0.791132 | 0.807789 | 0.001470 | 0.001916 | 0.001879 |
| disposition_select.test | RUN | 0.154552 | 0.799696 | 0.803702 | 0.001364 | 0.001955 | 0.001815 |
| error.test | RUN | 0.144742 | 0.788350 | 0.710276 | 0.001452 | 0.001863 | 0.001600 |
| error_select.test | RUN | 0.146302 | 0.781473 | 0.710532 | 0.001353 | 0.001904 | 0.001823 |
| file_base.test | LINK | 0.180028 | 0.660759 | 0.802558 | — | — | — |
| file_base_select.test | LINK | 0.180876 | 0.647886 | 0.719606 | — | — | — |
| fwd_first.test | LINK | 0.408788 | 1.012919 | 1.395074 | — | — | — |
| fwd_last.test | LINK | 0.408082 | 0.948776 | 1.439436 | — | — | — |
| generic_basic_endpoint.test | LINK | 0.158591 | 0.391968 | 0.522236 | — | — | — |
| generic_basic_endpoint_select.test | LINK | 0.160612 | 0.389173 | 0.505767 | — | — | — |
| generic_datagram_protocol.test | LINK | 0.634726 | 3.348671 | 2.702274 | — | — | — |
| generic_datagram_protocol_select.test | LINK | 0.630139 | 3.439130 | 2.702508 | — | — | — |
| generic_raw_protocol.test | LINK | 0.638631 | 3.347458 | 2.718247 | — | — | — |
| generic_raw_protocol_select.test | LINK | 0.633381 | 3.444815 | 2.711251 | — | — | — |
| generic_seq_packet_protocol.test | LINK | 0.547007 | 2.832865 | 2.337252 | — | — | — |
| generic_seq_packet_protocol_select.test | LINK | 0.545293 | 2.856655 | 2.369933 | — | — | — |
| generic_stream_protocol.test | LINK | 0.585385 | 3.075684 | 2.526379 | — | — | — |
| generic_stream_protocol_select.test | LINK | 0.597066 | 3.125902 | 2.538804 | — | — | — |
| high_resolution_timer.test | LINK | 0.266907 | 0.577636 | 0.791301 | — | — | — |
| high_resolution_timer_select.test | LINK | 0.271622 | 0.600592 | 0.806475 | — | — | — |
| immediate.test | LINK | 0.182932 | 0.446630 | 0.490871 | — | — | — |
| immediate_select.test | LINK | 0.186281 | 0.443352 | 0.489610 | — | — | — |
| inline_executor.test | RUN | 0.221663 | 0.685653 | 0.729078 | 0.001483 | 0.001494 | 0.001388 |
| inline_executor_select.test | RUN | 0.221524 | 0.689945 | 0.732839 | 0.001385 | 0.001479 | 0.001408 |
| inline_or_executor.test | RUN | 0.348969 | 2.016543 | 1.392015 | 0.001486 | 0.001502 | 0.001418 |
| inline_or_executor_select.test | RUN | 0.361554 | 2.063111 | 1.538744 | 0.001473 | 0.001504 | 0.001447 |
| io_context.test | RUN | 0.509464 | 3.037282 | 2.429215 | 0.002504 | 0.002339 | 0.002377 |
| io_context_select.test | RUN | 0.514814 | 2.962845 | 2.446161 | 0.002278 | 0.002181 | 0.002202 |
| io_context_strand.test | RUN | 0.508830 | 2.539268 | 2.106644 | 0.002506 | 0.002380 | 0.002315 |
| io_context_strand_select.test | RUN | 0.511238 | 2.515797 | 2.123193 | 0.002186 | 0.002239 | 0.002069 |
| ip_address.test | LINK | 0.192165 | 0.600834 | 0.613915 | — | — | — |
| ip_address_select.test | LINK | 0.190205 | 0.598284 | 0.618578 | — | — | — |
| ip_address_v4.test | LINK | 0.195484 | 0.669274 | 0.665470 | — | — | — |
| ip_address_v4_select.test | LINK | 0.195997 | 0.663023 | 0.666493 | — | — | — |
| ip_address_v6.test | LINK | 0.220351 | 0.831563 | 0.819488 | — | — | — |
| ip_address_v6_select.test | LINK | 0.222982 | 0.831596 | 0.817737 | — | — | — |
| ip_basic_endpoint.test | LINK | 0.177245 | 0.406999 | 0.529663 | — | — | — |
| ip_basic_endpoint_select.test | LINK | 0.180067 | 0.402116 | 0.527257 | — | — | — |
| ip_basic_resolver.test | LINK | 0.275583 | 0.605415 | 1.207269 | — | — | — |
| ip_basic_resolver_select.test | LINK | 0.285622 | 0.619364 | 0.961777 | — | — | — |
| ip_basic_resolver_entry.test | LINK | 0.098575 | 0.201901 | 0.395117 | — | — | — |
| ip_basic_resolver_entry_select.test | LINK | 0.097139 | 0.198161 | 0.393728 | — | — | — |
| ip_basic_resolver_iterator.test | LINK | 0.170495 | 0.408992 | 0.884758 | — | — | — |
| ip_basic_resolver_iterator_select.test | LINK | 0.170057 | 0.410623 | 0.885373 | — | — | — |
| ip_basic_resolver_query.test | LINK | 0.159165 | 0.400982 | 0.873668 | — | — | — |
| ip_basic_resolver_query_select.test | LINK | 0.162882 | 0.400268 | 0.882111 | — | — | — |
| ip_host_name.test | RUN | 0.170886 | 0.512554 | 0.953102 | 0.001353 | 0.001427 | 0.001788 |
| ip_host_name_select.test | RUN | 0.169327 | 0.511110 | 0.949526 | 0.001096 | 0.001308 | 0.001783 |
| ip_icmp.test | RUN | 0.766444 | 6.689282 | 5.915710 | 0.001451 | 0.002252 | 0.001688 |
| ip_icmp_select.test | RUN | 0.771749 | 6.928735 | 5.757316 | 0.001350 | 0.001978 | 0.001663 |
| ip_multicast.test | RUN | 0.458566 | 3.504234 | 3.330128 | 0.001546 | 0.002108 | 0.001913 |
| ip_multicast_select.test | RUN | 0.469087 | 3.563371 | 3.341943 | 0.001559 | 0.002085 | 0.001833 |
| ip_resolver_query_base.test | LINK | 0.110636 | 0.336443 | 0.396455 | — | — | — |
| ip_resolver_query_base_select.test | LINK | 0.113060 | 0.325532 | 0.389882 | — | — | — |
| ip_tcp.test | RUN | 1.109911 | 10.259871 | 8.996353 | 0.001999 | 0.002734 | 0.002568 |
| ip_tcp_select.test | RUN | 1.128142 | 10.684100 | 8.646946 | 0.001898 | 0.002722 | 0.002743 |
| ip_udp.test | RUN | 0.817656 | 7.238383 | 6.207058 | 0.001536 | 0.002021 | 0.002010 |
| ip_udp_select.test | RUN | 0.826506 | 7.388981 | 6.059222 | 0.001552 | 0.002250 | 0.002070 |
| ip_unicast.test | RUN | 0.424334 | 3.175419 | 2.890993 | 0.001516 | 0.002048 | 0.001776 |
| ip_unicast_select.test | RUN | 0.430550 | 3.239027 | 2.910636 | 0.001537 | 0.002070 | 0.001885 |
| ip_v6_only.test | RUN | 0.435831 | 3.259927 | 3.045454 | 0.001462 | 0.002172 | 0.001766 |
| ip_v6_only_select.test | RUN | 0.441471 | 3.423482 | 3.034870 | 0.001519 | 0.001776 | 0.001357 |
| is_read_buffered.test | RUN | 0.314005 | 1.036229 | 1.663628 | 0.001413 | 0.002010 | 0.001430 |
| is_read_buffered_select.test | RUN | 0.322573 | 1.081212 | 1.667424 | 0.001389 | 0.001759 | 0.001674 |
| is_write_buffered.test | RUN | 0.312828 | 1.044089 | 1.680078 | 0.001452 | 0.001976 | 0.001555 |
| is_write_buffered_select.test | RUN | 0.324314 | 1.058819 | 1.717370 | 0.001448 | 0.001962 | 0.001792 |
| local_basic_endpoint.test | LINK | 0.165519 | 0.568008 | 0.869419 | — | — | — |
| local_basic_endpoint_select.test | LINK | 0.167715 | 0.573135 | 0.861849 | — | — | — |
| local_connect_pair.test | LINK | 0.409364 | 3.262251 | 2.828458 | — | — | — |
| local_connect_pair_select.test | LINK | 0.413901 | 3.200157 | 2.817985 | — | — | — |
| local_datagram_protocol.test | LINK | 0.550632 | 4.166591 | 3.851837 | — | — | — |
| local_datagram_protocol_select.test | LINK | 0.599668 | 4.278631 | 3.794740 | — | — | — |
| local_seq_packet_protocol.test | LINK | 0.820699 | 4.108492 | 3.820758 | — | — | — |
| local_seq_packet_protocol_select.test | LINK | 0.818145 | 4.122477 | 3.824937 | — | — | — |
| local_stream_protocol.test | LINK | 0.766596 | 3.862963 | 3.648788 | — | — | — |
| local_stream_protocol_select.test | LINK | 0.782249 | 3.984524 | 3.595240 | — | — | — |
| placeholders.test | LINK | 0.147039 | 0.325260 | 0.402113 | — | — | — |
| placeholders_select.test | LINK | 0.149154 | 0.309659 | 0.401569 | — | — | — |
| posix_basic_descriptor.test | LINK | 0.394411 | 0.861821 | 1.252241 | — | — | — |
| posix_basic_descriptor_select.test | LINK | 0.410093 | 0.914225 | 1.517621 | — | — | — |
| posix_basic_stream_descriptor.test | LINK | 0.402893 | 0.882554 | 1.256318 | — | — | — |
| posix_basic_stream_descriptor_select.test | LINK | 0.418406 | 0.912665 | 1.503453 | — | — | — |
| posix_descriptor_base.test | LINK | 0.168541 | 0.330784 | 0.389290 | — | — | — |
| posix_descriptor_base_select.test | LINK | 0.165651 | 0.317543 | 0.386120 | — | — | — |
| posix_stream_descriptor.test | LINK | 0.776144 | 3.991836 | 3.600161 | — | — | — |
| posix_stream_descriptor_select.test | LINK | 0.805730 | 4.041905 | 3.699727 | — | — | — |
| prepend.test | RUN | 0.608071 | 3.272445 | 2.870273 | 0.001836 | 0.001804 | 0.001874 |
| prepend_select.test | RUN | 0.617045 | 3.307431 | 2.849439 | 0.001826 | 0.001768 | 0.001778 |
| random_access_file.test | LINK | 0.353588 | 0.799733 | 0.981492 | — | — | — |
| random_access_file_select.test | LINK | 0.379308 | 0.865777 | 1.261957 | — | — | — |
| read.test | RUN | 1.848302 | 21.102497 | 16.339492 | 0.002500 | 0.002006 | 0.002478 |
| read_select.test | RUN | 1.866988 | 20.976094 | 16.555766 | 0.002358 | 0.002727 | 0.002561 |
| read_at.test | RUN | 1.843219 | 30.218251 | 22.345409 | 0.002701 | 0.002854 | 0.002714 |
| read_at_select.test | RUN | 1.867022 | 30.261727 | 22.367632 | 0.002887 | 0.003088 | 0.002767 |
| read_until.test | RUN | 1.003923 | 7.911554 | 6.105502 | 0.001980 | 0.002096 | 0.002104 |
| read_until_select.test | RUN | 1.018798 | 7.937507 | 6.268479 | 0.002134 | 0.002197 | 0.001694 |
| readable_pipe.test | LINK | 0.639385 | 3.340537 | 2.879348 | — | — | — |
| readable_pipe_select.test | LINK | 0.664832 | 3.580947 | 3.060480 | — | — | — |
| recycling_allocator.test | RUN | 0.198386 | 0.494356 | 0.594719 | 0.001728 | 0.001924 | 0.001637 |
| recycling_allocator_select.test | RUN | 0.193564 | 0.501054 | 0.594332 | 0.001620 | 0.001929 | 0.001758 |
| redirect_disposition.test | LINK | 0.822152 | 4.607260 | 4.332926 | — | — | — |
| redirect_disposition_select.test | LINK | 0.818051 | 4.627235 | 4.323373 | — | — | — |
| redirect_error.test | LINK | 0.732671 | 3.847879 | 3.872724 | — | — | — |
| redirect_error_select.test | LINK | 0.733737 | 3.825028 | 3.836240 | — | — | — |
| registered_buffer.test | LINK | 0.191028 | 0.408109 | 0.548274 | — | — | — |
| registered_buffer_select.test | LINK | 0.194438 | 0.411840 | 0.564520 | — | — | — |
| signal_set.test | RUN | 0.610101 | 3.177321 | 2.660718 | 0.001676 | 0.002062 | 0.001734 |
| signal_set_select.test | RUN | 0.624868 | 3.296443 | 2.911537 | 0.001673 | 0.001898 | 0.001830 |
| signal_set_base.test | LINK | 0.165904 | 0.321758 | 0.393126 | — | — | — |
| signal_set_base_select.test | LINK | 0.161030 | 0.329152 | 0.371534 | — | — | — |
| socket_base.test | RUN | 0.753826 | 5.066085 | 4.053285 | 0.002032 | 0.001934 | 0.001960 |
| socket_base_select.test | RUN | 0.765146 | 4.844279 | 4.001111 | 0.001835 | 0.002098 | 0.002060 |
| spawn.test | RUN | 1.225348 | 7.757010 | 6.848471 | 0.002108 | 0.002193 | 0.002254 |
| spawn_select.test | RUN | 1.241014 | 7.693223 | 6.843078 | 0.001987 | 0.002242 | 0.002140 |
| static_thread_pool.test | RUN | 0.349401 | 0.744716 | 0.954889 | 0.001930 | 0.001880 | 0.001766 |
| static_thread_pool_select.test | RUN | 0.362110 | 0.817133 | 1.244681 | 0.001655 | 0.001820 | 0.001828 |
| steady_timer.test | LINK | 0.383921 | 0.873947 | 1.354891 | — | — | — |
| steady_timer_select.test | LINK | 0.389559 | 0.906635 | 1.361750 | — | — | — |
| strand.test | RUN | 0.787818 | 4.427705 | 4.250369 | 0.002466 | 0.002728 | 0.002370 |
| strand_select.test | RUN | 0.808867 | 4.387433 | 4.289069 | 0.002059 | 0.002697 | 0.002660 |
| stream_file.test | LINK | 0.357051 | 0.828914 | 0.968645 | — | — | — |
| stream_file_select.test | LINK | 0.367770 | 0.855908 | 1.278093 | — | — | — |
| streambuf.test | RUN | 0.203161 | 0.626652 | 0.740293 | 0.001427 | 0.001482 | 0.001863 |
| streambuf_select.test | RUN | 0.183154 | 0.458946 | 0.660511 | 0.001308 | 0.001504 | 0.001300 |
| system_timer.test | LINK | 0.505826 | 2.868137 | 2.643144 | — | — | — |
| system_timer_select.test | LINK | 0.519344 | 2.805808 | 2.390510 | — | — | — |
| system_context.test | LINK | 0.233847 | 0.512383 | 0.600357 | — | — | — |
| system_context_select.test | LINK | 0.249359 | 0.553255 | 0.745788 | — | — | — |
| system_executor.test | LINK | 0.309084 | 1.643320 | 1.128850 | — | — | — |
| system_executor_select.test | LINK | 0.317255 | 1.605456 | 1.259643 | — | — | — |
| this_coro.test | LINK | 0.096053 | 0.203693 | 0.233157 | — | — | — |
| this_coro_select.test | LINK | 0.093768 | 0.204251 | 0.236478 | — | — | — |
| thread_pool.test | RUN | 0.361030 | 2.168677 | 1.440733 | 0.002671 | 0.003460 | 0.002588 |
| thread_pool_select.test | RUN | 0.374343 | 2.327001 | 1.573229 | 0.002719 | 0.002849 | 0.002465 |
| time_traits.test | LINK | 0.213410 | 0.582688 | 0.473158 | — | — | — |
| time_traits_select.test | LINK | 0.208690 | 0.603294 | 0.475405 | — | — | — |
| ts_buffer.test | LINK | 0.199939 | 0.660184 | 0.503423 | — | — | — |
| ts_buffer_select.test | LINK | 0.196973 | 0.665850 | 0.537012 | — | — | — |
| ts_executor.test | LINK | 0.276997 | 0.918963 | 1.024130 | — | — | — |
| ts_executor_select.test | LINK | 0.289005 | 0.973120 | 1.476208 | — | — | — |
| ts_internet.test | LINK | 0.310900 | 1.012890 | 1.622349 | — | — | — |
| ts_internet_select.test | LINK | 0.318107 | 1.033454 | 1.655496 | — | — | — |
| ts_io_context.test | LINK | 0.241800 | 0.791711 | 1.015592 | — | — | — |
| ts_io_context_select.test | LINK | 0.255196 | 0.817666 | 1.245317 | — | — | — |
| ts_net.test | LINK | 0.356354 | 1.213078 | 2.002749 | — | — | — |
| ts_net_select.test | LINK | 0.365380 | 1.239209 | 2.060723 | — | — | — |
| ts_netfwd.test | LINK | 0.353990 | 1.214718 | 2.017859 | — | — | — |
| ts_netfwd_select.test | LINK | 0.368904 | 1.253801 | 2.032315 | — | — | — |
| ts_socket.test | LINK | 0.296880 | 0.954142 | 1.541218 | — | — | — |
| ts_socket_select.test | LINK | 0.304474 | 0.963974 | 1.576916 | — | — | — |
| ts_timer.test | LINK | 0.263262 | 0.850467 | 1.367092 | — | — | — |
| ts_timer_select.test | LINK | 0.267535 | 0.878731 | 1.321881 | — | — | — |
| use_awaitable.test | LINK | 0.094648 | 0.293005 | 0.348829 | — | — | — |
| use_awaitable_select.test | LINK | 0.094891 | 0.313108 | 0.390338 | — | — | — |
| wait_traits.test | LINK | 0.093014 | 0.308777 | 0.352897 | — | — | — |
| wait_traits_select.test | LINK | 0.095261 | 0.318068 | 0.358242 | — | — | — |
| windows_basic_object_handle.test | LINK | 0.093189 | 0.317057 | 0.381164 | — | — | — |
| windows_basic_object_handle_select.test | LINK | 0.092359 | 0.282330 | 0.385230 | — | — | — |
| windows_basic_overlapped_handle.test | LINK | 0.093758 | 0.280749 | 0.350939 | — | — | — |
| windows_basic_overlapped_handle_select.test | LINK | 0.095799 | 0.289337 | 0.360536 | — | — | — |
| windows_basic_random_access_handle.test | LINK | 0.094579 | 0.291049 | 0.384595 | — | — | — |
| windows_basic_random_access_handle_select.test | LINK | 0.095498 | 0.288941 | 0.348768 | — | — | — |
| windows_basic_stream_handle.test | LINK | 0.094494 | 0.290810 | 0.387307 | — | — | — |
| windows_basic_stream_handle_select.test | LINK | 0.109663 | 0.304082 | 0.359872 | — | — | — |
| windows_object_handle.test | LINK | 0.272976 | 0.787029 | 1.013845 | — | — | — |
| windows_object_handle_select.test | LINK | 0.280842 | 0.843849 | 1.261538 | — | — | — |
| windows_overlapped_ptr.test | LINK | 0.371181 | 0.800094 | 1.104769 | — | — | — |
| windows_overlapped_ptr_select.test | LINK | 0.389328 | 0.892927 | 1.367705 | — | — | — |
| windows_random_access_handle.test | LINK | 0.351534 | 0.769976 | 1.001852 | — | — | — |
| windows_random_access_handle_select.test | LINK | 0.380285 | 0.817432 | 1.270474 | — | — | — |
| windows_stream_handle.test | LINK | 0.362216 | 0.777169 | 0.985782 | — | — | — |
| windows_stream_handle_select.test | LINK | 0.371456 | 0.842426 | 1.275756 | — | — | — |
| writable_pipe.test | LINK | 0.624743 | 3.250886 | 2.971490 | — | — | — |
| writable_pipe_select.test | LINK | 0.664575 | 3.357947 | 3.173081 | — | — | — |
| write.test | RUN | 1.873672 | 16.917272 | 16.495690 | 0.002600 | 0.002701 | 0.002649 |
| write_select.test | RUN | 1.891217 | 16.893135 | 16.545830 | 0.002464 | 0.002774 | 0.002377 |
| write_at.test | RUN | 1.915103 | 23.942451 | 20.921232 | 0.002707 | 0.002693 | 0.002674 |
| write_at_select.test | RUN | 1.918881 | 23.754834 | 21.509152 | 0.002809 | 0.002820 | 0.002649 |
| any_executor.test | RUN | 0.769367 | 7.870885 | 6.404606 | 0.002413 | 0.002526 | 0.002544 |
| blocking.test | RUN | 1.903702 | 9.561839 | 13.466615 | 0.007371 | 0.007444 | 0.007525 |
| blocking_adaptation.test | RUN | 0.918747 | 4.216857 | 6.076347 | 0.004038 | 0.004325 | 0.003980 |
| context_as.test | RUN | 0.504642 | 2.964359 | 2.058368 | 0.001905 | 0.001984 | 0.001872 |
| executor.test | RUN | 0.140490 | 0.297645 | 0.400829 | 0.001608 | 0.001940 | 0.001749 |
| inline_exception_handling.test | RUN | 1.696051 | 9.368944 | 12.615857 | 0.005177 | 0.005278 | 0.005171 |
| invocable_archetype.test | RUN | 0.134368 | 0.303032 | 0.383972 | 0.001731 | 0.002003 | 0.001343 |
| mapping.test | RUN | 1.815377 | 9.012200 | 13.583543 | 0.005357 | 0.005266 | 0.005272 |
| outstanding_work.test | RUN | 0.866877 | 3.835723 | 5.939644 | 0.003881 | 0.003753 | 0.004122 |
| prefer_only.test | RUN | 0.281596 | 1.650259 | 1.377767 | 0.001716 | 0.001885 | 0.001822 |
| relationship.test | RUN | 0.670144 | 3.916758 | 5.784061 | 0.002886 | 0.004164 | 0.002902 |

## Native runtime replay

Child CPU of successful native tests, with process startup included; GDB and expected-failure executions are correctness-only. Two warmups per compiler, four A/A observations per reference and mirrored ABBA/BAAB blocks. Short tests repeat up to ten times, identically across compilers; CPU is divided by repetitions before summing tests. Executable hashes and output checks stay outside process timers. All runs must pass their original native oracle. Output hashes record equality or variation across the complete campaign; test reports may print times or addresses.

| Reference | CPU speedup ↑ | Paired blocks | A/A speedup |
|---|---:|---|---:|
| gcc | 0.985× | 0.991×, 0.979× | 1.008× |
| clang | 0.977× | 0.974×, 0.980× | 1.003× |

† A/A drift or paired-block spread exceeds 5%; small differences are unresolved. 1375 native tests in the full cohort; 1363 have byte-identical output throughout. [Raw replay evidence in CI artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43601267198).

| Test | Repeats | CPU cppgm | GCC | Clang | Output check |
|---|---:|---:|---:|---:|---|
| any_completion_executor.test | 3 | 0.001671s | 0.001639s | 0.001608s | byte equality observed |
| any_completion_executor_select.test | 4 | 0.001648s | 0.001589s | 0.001610s | byte equality observed |
| any_completion_handler.test | 9 | 0.001303s | 0.001277s | 0.001295s | byte equality observed |
| any_completion_handler_select.test | 8 | 0.001300s | 0.001284s | 0.001290s | byte equality observed |
| any_io_executor.test | 5 | 0.001611s | 0.001597s | 0.001602s | byte equality observed |
| any_io_executor_select.test | 4 | 0.001645s | 0.001586s | 0.001587s | byte equality observed |
| append.test | 10 | 0.001241s | 0.001236s | 0.001221s | byte equality observed |
| append_select.test | 7 | 0.001224s | 0.001202s | 0.001164s | byte equality observed |
| as_tuple.test | 8 | 0.001334s | 0.001300s | 0.001279s | byte equality observed |
| as_tuple_select.test | 8 | 0.001267s | 0.001229s | 0.001230s | byte equality observed |
| bind_allocator.test | 1 | 0.001661s | 0.001677s | 0.001652s | byte equality observed |
| bind_allocator_select.test | 1 | 0.001463s | 0.001413s | 0.001313s | byte equality observed |
| bind_cancellation_slot.test | 10 | 0.001198s | 0.001180s | 0.001187s | byte equality observed |
| bind_cancellation_slot_select.test | 10 | 0.001075s | 0.001075s | 0.001064s | byte equality observed |
| bind_executor.test | 1 | 0.001752s | 0.001669s | 0.001660s | byte equality observed |
| bind_executor_select.test | 1 | 0.002018s | 0.001992s | 0.002000s | byte equality observed |
| bind_immediate_executor.test | 10 | 0.001129s | 0.001137s | 0.001135s | byte equality observed |
| bind_immediate_executor_select.test | 10 | 0.001076s | 0.001100s | 0.001061s | byte equality observed |
| buffer.test | 10 | 0.001066s | 0.001100s | 0.001051s | byte equality observed |
| buffer_select.test | 10 | 0.001058s | 0.001062s | 0.001041s | byte equality observed |
| buffered_read_stream.test | 10 | 0.001372s | 0.001354s | 0.001351s | byte equality observed |
| buffered_read_stream_select.test | 10 | 0.001323s | 0.001332s | 0.001305s | byte equality observed |
| buffered_stream.test | 10 | 0.001321s | 0.001316s | 0.001326s | byte equality observed |
| buffered_stream_select.test | 10 | 0.001327s | 0.001289s | 0.001355s | byte equality observed |
| buffered_write_stream.test | 10 | 0.001332s | 0.001307s | 0.001338s | byte equality observed |
| buffered_write_stream_select.test | 10 | 0.001309s | 0.001322s | 0.001378s | byte equality observed |
| buffers_iterator.test | 10 | 0.001046s | 0.001025s | 0.001011s | byte equality observed |
| buffers_iterator_select.test | 10 | 0.001005s | 0.000993s | 0.000985s | byte equality observed |
| cancel_after.test | 1 | 0.003459s | 0.003305s | 0.003438s | byte equality observed |
| cancel_after_select.test | 1 | 0.002874s | 0.002790s | 0.002778s | byte equality observed |
| cancel_at.test | 1 | 0.003816s | 0.003749s | 0.003507s | byte equality observed |
| cancel_at_select.test | 1 | 0.003487s | 0.003416s | 0.003553s | byte equality observed |
| co_composed.test | 10 | 0.001055s | 0.001025s | 0.001063s | byte equality observed |
| co_composed_select.test | 10 | 0.001011s | 0.001005s | 0.001025s | byte equality observed |
| compose.test | 1 | 0.001713s | 0.001663s | 0.001726s | byte equality observed |
| compose_select.test | 1 | 0.001436s | 0.001404s | 0.001393s | byte equality observed |
| composed.test | 1 | 0.002105s | 0.002042s | 0.002080s | byte equality observed |
| composed_select.test | 1 | 0.001628s | 0.001544s | 0.001482s | byte equality observed |
| config.test | 10 | 0.001103s | 0.001098s | 0.001112s | byte equality observed |
| config_select.test | 10 | 0.001098s | 0.001125s | 0.001062s | byte equality observed |
| connect_pipe.test | 10 | 0.001104s | 0.001106s | 0.001093s | byte equality observed |
| connect_pipe_select.test | 10 | 0.001119s | 0.001160s | 0.001081s | byte equality observed |
| consign.test | 10 | 0.001131s | 0.001091s | 0.001082s | byte equality observed |
| consign_select.test | 10 | 0.001053s | 0.001068s | 0.001055s | byte equality observed |
| deadline_timer.test | 1 | 0.002355s | 0.002286s | 0.002335s | byte equality observed |
| deadline_timer_select.test | 1 | 0.002177s | 0.002144s | 0.002106s | byte equality observed |
| deferred.test | 10 | 0.001008s | 0.000985s | 0.001010s | byte equality observed |
| deferred_select.test | 10 | 0.000967s | 0.000938s | 0.000956s | byte equality observed |
| disposition.test | 10 | 0.001029s | 0.001022s | 0.001025s | byte equality observed |
| disposition_select.test | 10 | 0.001024s | 0.001020s | 0.001022s | byte equality observed |
| error.test | 10 | 0.000993s | 0.000984s | 0.000981s | byte equality observed |
| error_select.test | 10 | 0.000996s | 0.000988s | 0.000989s | byte equality observed |
| inline_executor.test | 10 | 0.001009s | 0.000989s | 0.001006s | byte equality observed |
| inline_executor_select.test | 10 | 0.001010s | 0.000994s | 0.001001s | byte equality observed |
| inline_or_executor.test | 10 | 0.001017s | 0.001006s | 0.000999s | byte equality observed |
| inline_or_executor_select.test | 10 | 0.001012s | 0.000989s | 0.000994s | byte equality observed |
| io_context.test | 1 | 0.001890s | 0.001894s | 0.001927s | byte equality observed |
| io_context_select.test | 1 | 0.002230s | 0.002300s | 0.002239s | byte equality observed |
| io_context_strand.test | 1 | 0.002268s | 0.002254s | 0.002271s | byte equality observed |
| io_context_strand_select.test | 1 | 0.002023s | 0.001981s | 0.002035s | byte equality observed |
| ip_host_name.test | 10 | 0.001020s | 0.001001s | 0.001009s | byte equality observed |
| ip_host_name_select.test | 10 | 0.000971s | 0.000956s | 0.000966s | byte equality observed |
| ip_icmp.test | 10 | 0.001017s | 0.001007s | 0.000982s | byte equality observed |
| ip_icmp_select.test | 10 | 0.001010s | 0.001013s | 0.000981s | byte equality observed |
| ip_multicast.test | 10 | 0.001091s | 0.001083s | 0.001076s | byte equality observed |
| ip_multicast_select.test | 10 | 0.001068s | 0.001058s | 0.001055s | byte equality observed |
| ip_tcp.test | 10 | 0.001486s | 0.001456s | 0.001459s | byte equality observed |
| ip_tcp_select.test | 10 | 0.001423s | 0.001390s | 0.001396s | byte equality observed |
| ip_udp.test | 10 | 0.001117s | 0.001087s | 0.001074s | byte equality observed |
| ip_udp_select.test | 10 | 0.001105s | 0.001077s | 0.001075s | byte equality observed |
| ip_unicast.test | 10 | 0.001072s | 0.001058s | 0.001059s | byte equality observed |
| ip_unicast_select.test | 10 | 0.001054s | 0.001036s | 0.001037s | byte equality observed |
| ip_v6_only.test | 10 | 0.001061s | 0.001038s | 0.001047s | byte equality observed |
| ip_v6_only_select.test | 10 | 0.001057s | 0.001054s | 0.001048s | byte equality observed |
| is_read_buffered.test | 10 | 0.000991s | 0.000995s | 0.000962s | byte equality observed |
| is_read_buffered_select.test | 10 | 0.001003s | 0.000974s | 0.000962s | byte equality observed |
| is_write_buffered.test | 10 | 0.001017s | 0.001002s | 0.000971s | byte equality observed |
| is_write_buffered_select.test | 10 | 0.001028s | 0.001031s | 0.000970s | byte equality observed |
| prepend.test | 10 | 0.001103s | 0.001059s | 0.001059s | byte equality observed |
| prepend_select.test | 10 | 0.001087s | 0.001065s | 0.001070s | byte equality observed |
| read.test | 10 | 0.001472s | 0.001452s | 0.001438s | byte equality observed |
| read_at.test | 10 | 0.001874s | 0.001833s | 0.001680s | byte equality observed |
| read_at_select.test | 10 | 0.001822s | 0.001846s | 0.001670s | byte equality observed |
| read_select.test | 10 | 0.001466s | 0.001446s | 0.001432s | byte equality observed |
| read_until.test | 10 | 0.001251s | 0.001223s | 0.001213s | byte equality observed |
| read_until_select.test | 10 | 0.001244s | 0.001219s | 0.001200s | byte equality observed |
| recycling_allocator.test | 10 | 0.001033s | 0.001012s | 0.001032s | byte equality observed |
| recycling_allocator_select.test | 10 | 0.001024s | 0.001061s | 0.001052s | byte equality observed |
| signal_set.test | 10 | 0.001058s | 0.001084s | 0.001060s | byte equality observed |
| signal_set_select.test | 10 | 0.001071s | 0.001068s | 0.001043s | byte equality observed |
| socket_base.test | 10 | 0.001167s | 0.001161s | 0.001149s | byte equality observed |
| socket_base_select.test | 10 | 0.001151s | 0.001155s | 0.001155s | byte equality observed |
| spawn.test | 10 | 0.001269s | 0.001252s | 0.001267s | byte equality observed |
| spawn_select.test | 10 | 0.001248s | 0.001228s | 0.001250s | byte equality observed |
| static_thread_pool.test | 10 | 0.001041s | 0.001026s | 0.001021s | byte equality observed |
| static_thread_pool_select.test | 10 | 0.001016s | 0.000970s | 0.000989s | byte equality observed |
| strand.test | 1 | 0.001978s | 0.001987s | 0.001971s | byte equality observed |
| strand_select.test | 1 | 0.001648s | 0.001664s | 0.001646s | byte equality observed |
| streambuf.test | 10 | 0.001072s | 0.001044s | 0.001053s | byte equality observed |
| streambuf_select.test | 10 | 0.001063s | 0.001059s | 0.001048s | byte equality observed |
| thread_pool.test | 10 | 0.002214s | 0.002205s | 0.002217s | byte equality observed |
| thread_pool_select.test | 10 | 0.002227s | 0.002198s | 0.002237s | byte equality observed |
| write.test | 10 | 0.001581s | 0.001493s | 0.001485s | byte equality observed |
| write_at.test | 10 | 0.001651s | 0.001625s | 0.001568s | byte equality observed |
| write_at_select.test | 10 | 0.001652s | 0.001599s | 0.001537s | byte equality observed |
| write_select.test | 10 | 0.001512s | 0.001488s | 0.001454s | byte equality observed |

[Raw outcomes, commands and timing observations in CI artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43601267198).

[Comparison records](../results.json) · [Host and toolchains](../metadata.json)
