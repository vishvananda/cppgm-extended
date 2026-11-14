# libs/asio/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| any_completion_executor.test | RUN | 0.816203 | 5.044172 | 5.230684 | 0.002509 | 0.002882 | 0.002702 |
| any_completion_executor_select.test | RUN | 0.877390 | 5.021211 | 5.534218 | 0.002578 | 0.002678 | 0.002602 |
| any_completion_handler.test | RUN | 0.923907 | 4.438716 | 4.812854 | 0.001936 | 0.002214 | 0.002121 |
| any_completion_handler_select.test | RUN | 0.970510 | 4.612548 | 5.246687 | 0.002194 | 0.002014 | 0.002224 |
| any_io_executor.test | RUN | 0.914779 | 5.423376 | 5.520286 | 0.002571 | 0.002873 | 0.002756 |
| any_io_executor_select.test | RUN | 0.788840 | 5.343223 | 5.926184 | 0.002575 | 0.002880 | 0.002652 |
| append.test | RUN | 0.782703 | 4.204085 | 4.342926 | 0.002017 | 0.001783 | 0.002233 |
| append_select.test | RUN | 0.830687 | 4.273799 | 4.323954 | 0.001926 | 0.002134 | 0.002184 |
| as_tuple.test | RUN | 1.017965 | 5.065407 | 5.864600 | 0.002456 | 0.002598 | 0.002384 |
| as_tuple_select.test | RUN | 1.036850 | 5.075479 | 5.799191 | 0.002076 | 0.002438 | 0.002338 |
| associated_allocator.test | LINK | 0.227195 | 0.472483 | 0.740334 | — | — | — |
| associated_allocator_select.test | LINK | 0.233813 | 0.487218 | 0.710247 | — | — | — |
| associated_cancellation_slot.test | LINK | 0.242599 | 0.529761 | 0.742534 | — | — | — |
| associated_cancellation_slot_select.test | LINK | 0.234795 | 0.532339 | 0.736075 | — | — | — |
| associated_executor.test | LINK | 0.354773 | 0.884984 | 1.242220 | — | — | — |
| associated_executor_select.test | LINK | 0.350769 | 0.895880 | 1.245493 | — | — | — |
| associated_immediate_executor.test | LINK | 0.327892 | 0.805179 | 1.130136 | — | — | — |
| associated_immediate_executor_select.test | LINK | 0.337893 | 0.821995 | 1.121929 | — | — | — |
| associator.test | LINK | 0.181637 | 0.382587 | 0.575487 | — | — | — |
| associator_select.test | LINK | 0.183913 | 0.389699 | 0.603634 | — | — | — |
| awaitable.test | LINK | 0.181469 | 0.384164 | 0.549646 | — | — | — |
| awaitable_select.test | LINK | 0.183238 | 0.394062 | 0.588159 | — | — | — |
| basic_datagram_socket.test | LINK | 0.547415 | 1.223504 | 2.294368 | — | — | — |
| basic_datagram_socket_select.test | LINK | 0.551889 | 1.244564 | 2.360929 | — | — | — |
| basic_deadline_timer.test | LINK | 0.651219 | 1.437332 | 2.663650 | — | — | — |
| basic_deadline_timer_select.test | LINK | 0.657941 | 1.509783 | 2.666131 | — | — | — |
| basic_file.test | LINK | 0.177796 | 0.385280 | 0.581341 | — | — | — |
| basic_file_select.test | LINK | 0.180792 | 0.415050 | 0.577428 | — | — | — |
| basic_random_access_file.test | LINK | 0.181111 | 0.394807 | 0.572061 | — | — | — |
| basic_random_access_file_select.test | LINK | 0.178967 | 0.402085 | 0.575701 | — | — | — |
| basic_raw_socket.test | LINK | 0.549311 | 1.220481 | 2.290049 | — | — | — |
| basic_raw_socket_select.test | LINK | 0.559306 | 1.249921 | 2.362519 | — | — | — |
| basic_readable_pipe.test | LINK | 0.537661 | 1.161920 | 1.902402 | — | — | — |
| basic_readable_pipe_select.test | LINK | 0.556079 | 1.269397 | 2.486684 | — | — | — |
| basic_seq_packet_socket.test | LINK | 0.544243 | 1.222030 | 2.242904 | — | — | — |
| basic_seq_packet_socket_select.test | LINK | 0.555272 | 1.255569 | 2.402718 | — | — | — |
| basic_signal_set.test | LINK | 0.506342 | 1.116153 | 1.716090 | — | — | — |
| basic_signal_set_select.test | LINK | 0.541151 | 1.215833 | 2.245946 | — | — | — |
| basic_socket_acceptor.test | LINK | 0.539348 | 1.231097 | 2.326308 | — | — | — |
| basic_socket_acceptor_select.test | LINK | 0.554560 | 1.266899 | 2.395152 | — | — | — |
| basic_stream_file.test | LINK | 0.178209 | 0.378858 | 0.576563 | — | — | — |
| basic_stream_file_select.test | LINK | 0.181749 | 0.384033 | 0.562215 | — | — | — |
| basic_stream_socket.test | LINK | 0.545138 | 1.216687 | 2.322168 | — | — | — |
| basic_stream_socket_select.test | LINK | 0.571742 | 1.270583 | 2.328355 | — | — | — |
| basic_streambuf.test | LINK | 0.274311 | 0.563182 | 0.884020 | — | — | — |
| basic_streambuf_select.test | LINK | 0.275008 | 0.570113 | 0.885623 | — | — | — |
| basic_waitable_timer.test | LINK | 0.512653 | 1.139636 | 2.127282 | — | — | — |
| basic_waitable_timer_select.test | LINK | 0.516166 | 1.176461 | 2.209857 | — | — | — |
| basic_writable_pipe.test | LINK | 0.529625 | 1.178508 | 1.930440 | — | — | — |
| basic_writable_pipe_select.test | LINK | 0.560897 | 1.246237 | 2.436964 | — | — | — |
| bind_allocator.test | RUN | 0.847737 | 4.300422 | 4.700439 | 0.002078 | 0.002588 | 0.002596 |
| bind_allocator_select.test | RUN | 0.890774 | 4.395377 | 4.855991 | 0.001880 | 0.002369 | 0.002118 |
| bind_cancellation_slot.test | RUN | 0.862462 | 4.364964 | 4.733604 | 0.002154 | 0.001610 | 0.001486 |
| bind_cancellation_slot_select.test | RUN | 0.897880 | 4.330071 | 4.499647 | 0.001546 | 0.001624 | 0.001532 |
| bind_executor.test | RUN | 0.544798 | 2.965023 | 2.698446 | 0.002197 | 0.002675 | 0.002958 |
| bind_executor_select.test | RUN | 0.553908 | 2.980612 | 2.824217 | 0.002060 | 0.002824 | 0.002058 |
| bind_immediate_executor.test | RUN | 0.415727 | 2.536114 | 1.981323 | 0.001582 | 0.001607 | 0.001473 |
| bind_immediate_executor_select.test | RUN | 0.430655 | 2.435172 | 2.161217 | 0.001443 | 0.001558 | 0.001466 |
| buffer.test | RUN | 0.254923 | 1.008143 | 1.140059 | 0.001492 | 0.001471 | 0.001392 |
| buffer_select.test | RUN | 0.288192 | 1.014632 | 1.127661 | 0.001448 | 0.001483 | 0.001395 |
| buffer_registration.test | LINK | 0.205240 | 0.518147 | 0.684084 | — | — | — |
| buffer_registration_select.test | LINK | 0.209177 | 0.517738 | 0.652159 | — | — | — |
| buffered_read_stream.test | RUN | 0.891051 | 6.704758 | 7.166182 | 0.001903 | 0.002799 | 0.002832 |
| buffered_read_stream_select.test | RUN | 0.900323 | 6.909230 | 7.285132 | 0.001868 | 0.002659 | 0.002694 |
| buffered_stream.test | RUN | 1.613971 | 10.084375 | 11.078580 | 0.002670 | 0.002834 | 0.002449 |
| buffered_stream_select.test | RUN | 1.774967 | 10.230713 | 10.964727 | 0.002396 | 0.002680 | 0.002757 |
| buffered_write_stream.test | RUN | 1.581208 | 8.846201 | 9.778648 | 0.002393 | 0.002913 | 0.002592 |
| buffered_write_stream_select.test | RUN | 1.611178 | 8.942596 | 9.129561 | 0.002346 | 0.002550 | 0.002643 |
| buffers_iterator.test | RUN | 0.510445 | 0.906442 | 1.660982 | 0.001920 | 0.002038 | 0.001900 |
| buffers_iterator_select.test | RUN | 0.495659 | 0.899394 | 1.677538 | 0.002064 | 0.002086 | 0.002118 |
| cancel_after.test | RUN | 1.240068 | 6.915107 | 7.123820 | 0.003729 | 0.004237 | 0.004210 |
| cancel_after_select.test | RUN | 1.264043 | 6.903408 | 7.131483 | 0.003366 | 0.004158 | 0.004026 |
| cancel_at.test | RUN | 1.203524 | 5.673186 | 6.013477 | 0.003477 | 0.003800 | 0.003638 |
| cancel_at_select.test | RUN | 1.118029 | 5.527992 | 6.117620 | 0.003516 | 0.003421 | 0.003445 |
| cancellation_signal.test | LINK | 0.148234 | 0.308598 | 0.423847 | — | — | — |
| cancellation_signal_select.test | LINK | 0.154717 | 0.311881 | 0.437597 | — | — | — |
| cancellation_state.test | LINK | 0.223740 | 0.310552 | 0.433786 | — | — | — |
| cancellation_state_select.test | LINK | 0.234166 | 0.311212 | 0.436246 | — | — | — |
| cancellation_type.test | LINK | 0.128797 | 0.243543 | 0.345969 | — | — | — |
| cancellation_type_select.test | LINK | 0.179475 | 0.247970 | 0.350191 | — | — | — |
| co_composed.test | RUN | 0.180381 | 0.243547 | 0.439952 | 0.001951 | 0.001461 | 0.002074 |
| co_composed_select.test | RUN | 0.176270 | 0.246381 | 0.496136 | 0.001793 | 0.001525 | 0.001958 |
| co_spawn.test | LINK | 0.182014 | 0.247661 | 0.558418 | — | — | — |
| co_spawn_select.test | LINK | 0.181359 | 0.246508 | 0.562291 | — | — | — |
| completion_condition.test | LINK | 0.233669 | 0.391838 | 0.774118 | — | — | — |
| completion_condition_select.test | LINK | 0.226736 | 0.576746 | 0.767562 | — | — | — |
| compose.test | RUN | 1.053613 | 5.426035 | 5.946716 | 0.002457 | 0.002287 | 0.001795 |
| compose_select.test | RUN | 1.081860 | 5.297215 | 5.905501 | 0.002344 | 0.002399 | 0.002328 |
| composed.test | RUN | 0.986838 | 5.343339 | 5.837840 | 0.002016 | 0.002079 | 0.002736 |
| composed_select.test | RUN | 1.006577 | 5.172720 | 5.778973 | 0.001846 | 0.002297 | 0.002142 |
| config.test | RUN | 0.451649 | 3.651505 | 2.563561 | 0.002108 | 0.002272 | 0.002198 |
| config_select.test | RUN | 0.602455 | 3.741025 | 3.050264 | 0.002269 | 0.002305 | 0.002248 |
| connect.test | LINK | 1.660059 | 11.225720 | 11.942894 | — | — | — |
| connect_select.test | LINK | 1.693107 | 11.468533 | 11.985105 | — | — | — |
| connect_pipe.test | RUN | 0.950029 | 4.324999 | 4.728435 | 0.002105 | 0.002244 | 0.002284 |
| connect_pipe_select.test | RUN | 0.939598 | 4.484863 | 5.017374 | 0.001922 | 0.002276 | 0.002336 |
| consign.test | RUN | 0.792228 | 4.203259 | 4.358815 | 0.002083 | 0.001978 | 0.002349 |
| consign_select.test | RUN | 0.801623 | 4.331456 | 4.364507 | 0.002664 | 0.002298 | 0.001929 |
| coroutine.test | LINK | 0.203951 | 0.465574 | 0.612201 | — | — | — |
| coroutine_select.test | LINK | 0.190807 | 0.454224 | 0.615917 | — | — | — |
| deadline_timer.test | RUN | 1.144399 | 5.534405 | 5.965824 | 0.003665 | 0.004013 | 0.004337 |
| deadline_timer_select.test | RUN | 1.158455 | 5.453976 | 6.083004 | 0.003562 | 0.003380 | 0.003549 |
| deferred.test | RUN | 0.203321 | 0.428682 | 0.664390 | 0.002098 | 0.002165 | 0.001981 |
| deferred_select.test | RUN | 0.210448 | 0.444066 | 0.640681 | 0.001971 | 0.002225 | 0.001885 |
| detached.test | LINK | 0.225869 | 0.493283 | 0.749608 | — | — | — |
| detached_select.test | LINK | 0.221314 | 0.511682 | 0.723880 | — | — | — |
| disposition.test | RUN | 0.235425 | 1.122026 | 1.256176 | 0.001569 | 0.002122 | 0.002030 |
| disposition_select.test | RUN | 0.197064 | 1.105972 | 1.247104 | 0.001553 | 0.002300 | 0.002043 |
| error.test | RUN | 0.180120 | 1.056117 | 1.113829 | 0.001555 | 0.002121 | 0.002155 |
| error_select.test | RUN | 0.184069 | 1.036287 | 1.157865 | 0.001542 | 0.002032 | 0.002015 |
| file_base.test | LINK | 0.225831 | 0.883929 | 1.029502 | — | — | — |
| file_base_select.test | LINK | 0.225164 | 0.716811 | 1.174746 | — | — | — |
| fwd_first.test | LINK | 0.512358 | 1.196102 | 2.023111 | — | — | — |
| fwd_last.test | LINK | 0.506376 | 1.182419 | 2.100548 | — | — | — |
| generic_basic_endpoint.test | LINK | 0.197491 | 0.487607 | 0.739276 | — | — | — |
| generic_basic_endpoint_select.test | LINK | 0.201423 | 0.482005 | 0.733660 | — | — | — |
| generic_datagram_protocol.test | LINK | 0.786205 | 4.130537 | 3.848550 | — | — | — |
| generic_datagram_protocol_select.test | LINK | 0.783236 | 4.206124 | 3.797729 | — | — | — |
| generic_raw_protocol.test | LINK | 0.772666 | 4.089849 | 3.874411 | — | — | — |
| generic_raw_protocol_select.test | LINK | 0.787339 | 4.200767 | 3.866930 | — | — | — |
| generic_seq_packet_protocol.test | LINK | 0.671518 | 3.466865 | 3.290093 | — | — | — |
| generic_seq_packet_protocol_select.test | LINK | 0.672060 | 3.508382 | 3.339663 | — | — | — |
| generic_stream_protocol.test | LINK | 0.721354 | 3.801747 | 3.617137 | — | — | — |
| generic_stream_protocol_select.test | LINK | 0.724172 | 3.906796 | 3.622680 | — | — | — |
| high_resolution_timer.test | LINK | 0.323666 | 0.750436 | 1.199648 | — | — | — |
| high_resolution_timer_select.test | LINK | 0.333660 | 0.750784 | 1.214277 | — | — | — |
| immediate.test | LINK | 0.229285 | 0.567845 | 0.749227 | — | — | — |
| immediate_select.test | LINK | 0.228410 | 0.564931 | 0.741811 | — | — | — |
| inline_executor.test | RUN | 0.274189 | 0.861014 | 1.065276 | 0.001544 | 0.001542 | 0.001516 |
| inline_executor_select.test | RUN | 0.281571 | 0.852361 | 1.100063 | 0.001511 | 0.001561 | 0.001616 |
| inline_or_executor.test | RUN | 0.435229 | 2.441839 | 2.042319 | 0.001522 | 0.001553 | 0.001630 |
| inline_or_executor_select.test | RUN | 0.448802 | 2.492116 | 2.274601 | 0.001513 | 0.001525 | 0.001582 |
| io_context.test | RUN | 0.626655 | 3.705552 | 3.518816 | 0.002586 | 0.002502 | 0.002800 |
| io_context_select.test | RUN | 0.636620 | 3.602016 | 3.604386 | 0.002388 | 0.002592 | 0.002831 |
| io_context_strand.test | RUN | 0.610948 | 4.174883 | 3.652600 | 0.002453 | 0.003092 | 0.002773 |
| io_context_strand_select.test | RUN | 0.625723 | 4.277841 | 4.320728 | 0.002340 | 0.003219 | 0.003224 |
| ip_address.test | LINK | 0.230603 | 1.205962 | 1.765869 | — | — | — |
| ip_address_select.test | LINK | 0.234376 | 1.210032 | 1.581824 | — | — | — |
| ip_address_v4.test | LINK | 0.234634 | 1.262322 | 1.450409 | — | — | — |
| ip_address_v4_select.test | LINK | 0.237869 | 1.273400 | 1.083945 | — | — | — |
| ip_address_v6.test | LINK | 0.266464 | 1.178174 | 1.269033 | — | — | — |
| ip_address_v6_select.test | LINK | 0.268590 | 1.185660 | 1.246506 | — | — | — |
| ip_basic_endpoint.test | LINK | 0.206039 | 0.524882 | 0.827414 | — | — | — |
| ip_basic_endpoint_select.test | LINK | 0.202961 | 0.515810 | 0.820244 | — | — | — |
| ip_basic_resolver.test | LINK | 0.333998 | 0.746237 | 1.236268 | — | — | — |
| ip_basic_resolver_select.test | LINK | 0.326393 | 0.761975 | 1.256915 | — | — | — |
| ip_basic_resolver_entry.test | LINK | 0.116792 | 0.249876 | 0.352938 | — | — | — |
| ip_basic_resolver_entry_select.test | LINK | 0.112286 | 0.253748 | 0.348969 | — | — | — |
| ip_basic_resolver_iterator.test | LINK | 0.208775 | 0.502135 | 0.809934 | — | — | — |
| ip_basic_resolver_iterator_select.test | LINK | 0.207108 | 0.514117 | 0.801352 | — | — | — |
| ip_basic_resolver_query.test | LINK | 0.193947 | 0.484674 | 0.777555 | — | — | — |
| ip_basic_resolver_query_select.test | LINK | 0.195868 | 0.486118 | 0.767767 | — | — | — |
| ip_host_name.test | RUN | 0.209565 | 0.633154 | 1.500854 | 0.001447 | 0.001498 | 0.002214 |
| ip_host_name_select.test | RUN | 0.211292 | 0.643155 | 0.884459 | 0.001433 | 0.001540 | 0.001488 |
| ip_icmp.test | RUN | 0.965561 | 7.151691 | 8.931037 | 0.001503 | 0.002194 | 0.002208 |
| ip_icmp_select.test | RUN | 0.943428 | 7.808701 | 8.500774 | 0.001510 | 0.002074 | 0.002120 |
| ip_multicast.test | RUN | 0.575165 | 4.496742 | 5.195876 | 0.001599 | 0.002195 | 0.002429 |
| ip_multicast_select.test | RUN | 0.577263 | 4.481670 | 5.060557 | 0.001795 | 0.002122 | 0.002359 |
| ip_resolver_query_base.test | LINK | 0.142155 | 0.429427 | 0.609907 | — | — | — |
| ip_resolver_query_base_select.test | LINK | 0.138777 | 0.439050 | 0.619445 | — | — | — |
| ip_tcp.test | RUN | 1.409664 | 12.886547 | 13.012843 | 0.002125 | 0.003144 | 0.003384 |
| ip_tcp_select.test | RUN | 1.421456 | 13.443919 | 12.550537 | 0.002128 | 0.002857 | 0.003004 |
| ip_udp.test | RUN | 1.011348 | 9.220522 | 8.980607 | 0.001667 | 0.002394 | 0.002286 |
| ip_udp_select.test | RUN | 1.022121 | 9.471100 | 8.674256 | 0.001649 | 0.002352 | 0.002293 |
| ip_unicast.test | RUN | 0.519230 | 4.125599 | 4.481579 | 0.001557 | 0.002159 | 0.002326 |
| ip_unicast_select.test | RUN | 0.594392 | 4.262356 | 4.472755 | 0.002139 | 0.002340 | 0.002202 |
| ip_v6_only.test | RUN | 0.864623 | 4.307010 | 4.605730 | 0.002180 | 0.002316 | 0.002252 |
| ip_v6_only_select.test | RUN | 0.860407 | 4.431620 | 4.657194 | 0.001989 | 0.002129 | 0.002256 |
| is_read_buffered.test | RUN | 0.602262 | 1.425179 | 2.594971 | 0.002163 | 0.002160 | 0.002058 |
| is_read_buffered_select.test | RUN | 0.628823 | 1.474101 | 2.677514 | 0.001962 | 0.002120 | 0.002294 |
| is_write_buffered.test | RUN | 0.613642 | 1.425633 | 2.604750 | 0.001888 | 0.002090 | 0.002154 |
| is_write_buffered_select.test | RUN | 0.615820 | 1.406065 | 2.709516 | 0.001940 | 0.002234 | 0.002159 |
| local_basic_endpoint.test | LINK | 0.310386 | 0.768012 | 1.471418 | — | — | — |
| local_basic_endpoint_select.test | LINK | 0.327867 | 0.755418 | 1.456662 | — | — | — |
| local_connect_pair.test | LINK | 0.784255 | 4.238400 | 4.348556 | — | — | — |
| local_connect_pair_select.test | LINK | 0.820161 | 4.172329 | 4.308194 | — | — | — |
| local_datagram_protocol.test | LINK | 1.040455 | 5.373812 | 5.813819 | — | — | — |
| local_datagram_protocol_select.test | LINK | 1.090153 | 5.501089 | 5.727765 | — | — | — |
| local_seq_packet_protocol.test | LINK | 1.075655 | 5.301759 | 5.688982 | — | — | — |
| local_seq_packet_protocol_select.test | LINK | 1.066627 | 5.468844 | 5.634808 | — | — | — |
| local_stream_protocol.test | LINK | 1.013451 | 5.105153 | 5.502779 | — | — | — |
| local_stream_protocol_select.test | LINK | 1.029084 | 5.145018 | 5.475554 | — | — | — |
| placeholders.test | LINK | 0.196496 | 0.432658 | 0.585066 | — | — | — |
| placeholders_select.test | LINK | 0.198621 | 0.417991 | 0.621072 | — | — | — |
| posix_basic_descriptor.test | LINK | 0.530213 | 1.148650 | 2.006309 | — | — | — |
| posix_basic_descriptor_select.test | LINK | 0.544583 | 1.237683 | 2.498401 | — | — | — |
| posix_basic_stream_descriptor.test | LINK | 0.536825 | 1.200505 | 1.983014 | — | — | — |
| posix_basic_stream_descriptor_select.test | LINK | 0.554110 | 1.223757 | 2.552562 | — | — | — |
| posix_descriptor_base.test | LINK | 0.218837 | 0.463644 | 0.674494 | — | — | — |
| posix_descriptor_base_select.test | LINK | 0.214514 | 0.433139 | 0.629715 | — | — | — |
| posix_stream_descriptor.test | LINK | 1.026752 | 5.197831 | 5.440168 | — | — | — |
| posix_stream_descriptor_select.test | LINK | 1.051911 | 5.287320 | 5.656382 | — | — | — |
| prepend.test | RUN | 0.799278 | 4.317754 | 4.502802 | 0.001914 | 0.002332 | 0.002039 |
| prepend_select.test | RUN | 0.804697 | 4.362831 | 4.458571 | 0.001848 | 0.002262 | 0.002008 |
| random_access_file.test | LINK | 0.476386 | 1.039983 | 1.588352 | — | — | — |
| random_access_file_select.test | LINK | 0.496166 | 1.117860 | 2.068825 | — | — | — |
| read.test | RUN | 2.483398 | 25.728924 | 24.066699 | 0.002962 | 0.002818 | 0.002700 |
| read_select.test | RUN | 2.529572 | 25.567103 | 24.791663 | 0.002616 | 0.002769 | 0.003294 |
| read_at.test | RUN | 2.500717 | 34.949786 | 32.319971 | 0.003560 | 0.003461 | 0.003561 |
| read_at_select.test | RUN | 2.542193 | 34.587015 | 32.578912 | 0.003478 | 0.003525 | 0.003595 |
| read_until.test | RUN | 1.306954 | 10.222803 | 9.606461 | 0.002261 | 0.002204 | 0.002359 |
| read_until_select.test | RUN | 1.313601 | 10.216755 | 9.890026 | 0.002084 | 0.002365 | 0.002463 |
| readable_pipe.test | LINK | 0.824181 | 4.279807 | 4.307860 | — | — | — |
| readable_pipe_select.test | LINK | 0.844317 | 4.479950 | 4.752181 | — | — | — |
| recycling_allocator.test | RUN | 0.262738 | 0.674922 | 0.874694 | 0.001877 | 0.002204 | 0.002160 |
| recycling_allocator_select.test | RUN | 0.255136 | 0.675182 | 0.910668 | 0.001944 | 0.001960 | 0.002227 |
| redirect_disposition.test | LINK | 1.079559 | 6.011101 | 6.733878 | — | — | — |
| redirect_disposition_select.test | LINK | 1.101338 | 6.002197 | 6.807536 | — | — | — |
| redirect_error.test | LINK | 0.977171 | 4.956584 | 5.834449 | — | — | — |
| redirect_error_select.test | LINK | 0.978350 | 4.900127 | 5.889753 | — | — | — |
| registered_buffer.test | LINK | 0.263068 | 0.567730 | 0.913225 | — | — | — |
| registered_buffer_select.test | LINK | 0.264282 | 0.554503 | 0.840183 | — | — | — |
| signal_set.test | RUN | 0.815886 | 4.167712 | 3.998806 | 0.001946 | 0.002222 | 0.002250 |
| signal_set_select.test | RUN | 0.849809 | 4.355533 | 4.571700 | 0.002092 | 0.002051 | 0.002284 |
| signal_set_base.test | LINK | 0.204681 | 0.464440 | 0.662124 | — | — | — |
| signal_set_base_select.test | LINK | 0.207264 | 0.454483 | 0.667704 | — | — | — |
| socket_base.test | RUN | 1.013063 | 6.087819 | 6.497594 | 0.002415 | 0.002289 | 0.002514 |
| socket_base_select.test | RUN | 1.018180 | 5.888509 | 6.430181 | 0.002200 | 0.002372 | 0.002432 |
| spawn.test | RUN | 1.628465 | 9.795289 | 10.337079 | 0.002071 | 0.002544 | 0.002630 |
| spawn_select.test | RUN | 1.647342 | 9.804049 | 10.253411 | 0.002553 | 0.002607 | 0.002470 |
| static_thread_pool.test | RUN | 0.458357 | 1.011084 | 1.527539 | 0.002187 | 0.001878 | 0.002136 |
| static_thread_pool_select.test | RUN | 0.499198 | 1.102438 | 2.055254 | 0.002040 | 0.001565 | 0.002184 |
| steady_timer.test | LINK | 0.516768 | 1.148869 | 2.158962 | — | — | — |
| steady_timer_select.test | LINK | 0.537282 | 1.196381 | 2.226308 | — | — | — |
| strand.test | RUN | 1.062440 | 5.625670 | 6.350387 | 0.002782 | 0.002877 | 0.002959 |
| strand_select.test | RUN | 1.081030 | 5.571165 | 6.335423 | 0.002491 | 0.002524 | 0.003265 |
| stream_file.test | LINK | 0.498440 | 1.029429 | 1.569223 | — | — | — |
| stream_file_select.test | LINK | 0.510384 | 1.138461 | 2.122131 | — | — | — |
| streambuf.test | RUN | 0.268569 | 0.875691 | 1.133334 | 0.001471 | 0.001744 | 0.002040 |
| streambuf_select.test | RUN | 0.214665 | 0.715420 | 1.071809 | 0.001468 | 0.001510 | 0.001590 |
| system_timer.test | LINK | 0.626813 | 3.438976 | 4.211136 | — | — | — |
| system_timer_select.test | LINK | 0.631227 | 3.398387 | 3.486132 | — | — | — |
| system_context.test | LINK | 0.293069 | 0.646163 | 0.891157 | — | — | — |
| system_context_select.test | LINK | 0.309520 | 0.693926 | 1.109382 | — | — | — |
| system_executor.test | LINK | 0.379833 | 1.977673 | 1.617399 | — | — | — |
| system_executor_select.test | LINK | 0.399353 | 1.954246 | 1.788750 | — | — | — |
| this_coro.test | LINK | 0.113559 | 0.244543 | 0.335733 | — | — | — |
| this_coro_select.test | LINK | 0.114806 | 0.247626 | 0.343819 | — | — | — |
| thread_pool.test | RUN | 0.454272 | 3.400550 | 2.770703 | 0.003105 | 0.003744 | 0.003837 |
| thread_pool_select.test | RUN | 0.469543 | 3.595750 | 2.593076 | 0.003276 | 0.004017 | 0.003919 |
| time_traits.test | LINK | 0.260140 | 0.867132 | 1.305974 | — | — | — |
| time_traits_select.test | LINK | 0.254962 | 0.834652 | 1.232240 | — | — | — |
| ts_buffer.test | LINK | 0.248168 | 0.892846 | 1.283263 | — | — | — |
| ts_buffer_select.test | LINK | 0.243193 | 0.895462 | 1.298854 | — | — | — |
| ts_executor.test | LINK | 0.341476 | 1.250650 | 2.005376 | — | — | — |
| ts_executor_select.test | LINK | 0.362418 | 1.318132 | 2.515436 | — | — | — |
| ts_internet.test | LINK | 0.372307 | 1.364721 | 2.524117 | — | — | — |
| ts_internet_select.test | LINK | 0.382265 | 1.410407 | 2.682772 | — | — | — |
| ts_io_context.test | LINK | 0.306614 | 1.040512 | 1.520363 | — | — | — |
| ts_io_context_select.test | LINK | 0.318064 | 1.128256 | 2.046208 | — | — | — |
| ts_net.test | LINK | 0.446751 | 1.640123 | 3.135091 | — | — | — |
| ts_net_select.test | LINK | 0.451675 | 1.678968 | 3.288693 | — | — | — |
| ts_netfwd.test | LINK | 0.445396 | 1.628743 | 3.130429 | — | — | — |
| ts_netfwd_select.test | LINK | 0.457470 | 1.693496 | 3.240030 | — | — | — |
| ts_socket.test | LINK | 0.368420 | 1.302709 | 2.501901 | — | — | — |
| ts_socket_select.test | LINK | 0.376946 | 1.327219 | 2.602882 | — | — | — |
| ts_timer.test | LINK | 0.324119 | 1.174693 | 2.143624 | — | — | — |
| ts_timer_select.test | LINK | 0.330042 | 1.207820 | 2.226434 | — | — | — |
| use_awaitable.test | LINK | 0.114990 | 0.397257 | 0.562028 | — | — | — |
| use_awaitable_select.test | LINK | 0.120095 | 0.391271 | 0.568914 | — | — | — |
| wait_traits.test | LINK | 0.163440 | 0.389783 | 0.553137 | — | — | — |
| wait_traits_select.test | LINK | 0.172348 | 0.388678 | 0.570237 | — | — | — |
| windows_basic_object_handle.test | LINK | 0.182258 | 0.395641 | 0.569377 | — | — | — |
| windows_basic_object_handle_select.test | LINK | 0.180795 | 0.392887 | 0.603018 | — | — | — |
| windows_basic_overlapped_handle.test | LINK | 0.181562 | 0.396560 | 0.606369 | — | — | — |
| windows_basic_overlapped_handle_select.test | LINK | 0.179185 | 0.386782 | 0.573656 | — | — | — |
| windows_basic_random_access_handle.test | LINK | 0.183585 | 0.401188 | 0.574420 | — | — | — |
| windows_basic_random_access_handle_select.test | LINK | 0.181010 | 0.379069 | 0.560301 | — | — | — |
| windows_basic_stream_handle.test | LINK | 0.178303 | 0.387058 | 0.599739 | — | — | — |
| windows_basic_stream_handle_select.test | LINK | 0.185761 | 0.395083 | 0.589191 | — | — | — |
| windows_object_handle.test | LINK | 0.493197 | 1.061157 | 1.496661 | — | — | — |
| windows_object_handle_select.test | LINK | 0.494361 | 1.119867 | 2.024852 | — | — | — |
| windows_overlapped_ptr.test | LINK | 0.515669 | 1.124717 | 1.689333 | — | — | — |
| windows_overlapped_ptr_select.test | LINK | 0.536599 | 1.186099 | 2.203176 | — | — | — |
| windows_random_access_handle.test | LINK | 0.484859 | 1.066309 | 1.541645 | — | — | — |
| windows_random_access_handle_select.test | LINK | 0.501530 | 1.128819 | 2.103117 | — | — | — |
| windows_stream_handle.test | LINK | 0.486482 | 1.052128 | 1.534101 | — | — | — |
| windows_stream_handle_select.test | LINK | 0.524467 | 1.127070 | 1.983465 | — | — | — |
| writable_pipe.test | LINK | 0.862892 | 4.235662 | 4.501232 | — | — | — |
| writable_pipe_select.test | LINK | 0.892332 | 4.384384 | 4.937194 | — | — | — |
| write.test | RUN | 2.471305 | 21.527003 | 23.657677 | 0.002982 | 0.002908 | 0.003085 |
| write_select.test | RUN | 2.495993 | 21.543736 | 23.937784 | 0.002858 | 0.003146 | 0.003105 |
| write_at.test | RUN | 2.593981 | 28.580233 | 29.295932 | 0.002884 | 0.003162 | 0.002973 |
| write_at_select.test | RUN | 2.607670 | 28.635755 | 30.166219 | 0.002994 | 0.003231 | 0.003235 |
| any_executor.test | RUN | 1.028286 | 10.005128 | 9.876194 | 0.002442 | 0.002797 | 0.002755 |
| blocking.test | RUN | 2.726204 | 12.354003 | 20.341829 | 0.008967 | 0.008902 | 0.008645 |
| blocking_adaptation.test | RUN | 1.376571 | 5.587769 | 9.065852 | 0.004522 | 0.005072 | 0.004732 |
| context_as.test | RUN | 0.661345 | 3.823499 | 3.006030 | 0.002396 | 0.002325 | 0.002249 |
| executor.test | RUN | 0.183710 | 0.415078 | 0.625698 | 0.002084 | 0.001992 | 0.002121 |
| inline_exception_handling.test | RUN | 2.582421 | 11.752100 | 19.408909 | 0.005839 | 0.005685 | 0.005796 |
| invocable_archetype.test | RUN | 0.192550 | 0.409564 | 0.609325 | 0.002108 | 0.002144 | 0.002012 |
| mapping.test | RUN | 2.439181 | 11.484040 | 19.833841 | 0.006104 | 0.006104 | 0.005850 |
| outstanding_work.test | RUN | 1.223851 | 5.030372 | 8.999221 | 0.004740 | 0.005077 | 0.005091 |
| prefer_only.test | RUN | 0.385793 | 2.040343 | 2.068078 | 0.002247 | 0.002068 | 0.001949 |
| relationship.test | RUN | 0.990957 | 5.077282 | 7.319876 | 0.003286 | 0.004241 | 0.004653 |

## Native runtime replay

Child CPU of successful native tests, with process startup included; GDB and expected-failure executions are correctness-only. Two warmups per compiler, four A/A observations per reference and mirrored ABBA/BAAB blocks. Short tests repeat up to ten times, identically across compilers; CPU is divided by repetitions before summing tests. Executable hashes and output checks stay outside process timers. All runs must pass their original native oracle. Output hashes record equality or variation across the complete campaign; test reports may print times or addresses.

| Reference | CPU speedup ↑ | Paired blocks | A/A speedup |
|---|---:|---|---:|
| gcc | 0.986× | 0.982×, 0.990× | 1.000× |
| clang | 0.987× | 0.988×, 0.987× | 0.997× |

† A/A drift or paired-block spread exceeds 5%; small differences are unresolved. 1375 native tests in the full cohort; 1360 have byte-identical output throughout. [Raw replay evidence in CI artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43609274616).

| Test | Repeats | CPU cppgm | GCC | Clang | Output check |
|---|---:|---:|---:|---:|---|
| any_completion_executor.test | 3 | 0.002010s | 0.002003s | 0.002063s | byte equality observed |
| any_completion_executor_select.test | 3 | 0.002017s | 0.002018s | 0.002057s | byte equality observed |
| any_completion_handler.test | 9 | 0.001575s | 0.001579s | 0.001590s | byte equality observed |
| any_completion_handler_select.test | 8 | 0.001573s | 0.001573s | 0.001614s | byte equality observed |
| any_io_executor.test | 5 | 0.001966s | 0.002012s | 0.002011s | byte equality observed |
| any_io_executor_select.test | 3 | 0.001967s | 0.002016s | 0.001967s | byte equality observed |
| append.test | 6 | 0.001517s | 0.001501s | 0.001526s | byte equality observed |
| append_select.test | 8 | 0.001483s | 0.001464s | 0.001510s | byte equality observed |
| as_tuple.test | 4 | 0.001663s | 0.001681s | 0.001787s | byte equality observed |
| as_tuple_select.test | 9 | 0.001509s | 0.001397s | 0.001350s | byte equality observed |
| bind_allocator.test | 1 | 0.001928s | 0.001895s | 0.001978s | byte equality observed |
| bind_allocator_select.test | 1 | 0.001658s | 0.001571s | 0.001484s | byte equality observed |
| bind_cancellation_slot.test | 10 | 0.001379s | 0.001390s | 0.001374s | byte equality observed |
| bind_cancellation_slot_select.test | 10 | 0.001276s | 0.001293s | 0.001270s | byte equality observed |
| bind_executor.test | 1 | 0.001997s | 0.001845s | 0.001926s | byte equality observed |
| bind_executor_select.test | 1 | 0.002226s | 0.001886s | 0.002144s | byte equality observed |
| bind_immediate_executor.test | 10 | 0.001280s | 0.001275s | 0.001249s | byte equality observed |
| bind_immediate_executor_select.test | 10 | 0.001354s | 0.001350s | 0.001328s | byte equality observed |
| buffer.test | 10 | 0.001316s | 0.001337s | 0.001346s | byte equality observed |
| buffer_select.test | 10 | 0.001283s | 0.001255s | 0.001241s | byte equality observed |
| buffered_read_stream.test | 10 | 0.001673s | 0.001656s | 0.001663s | byte equality observed |
| buffered_read_stream_select.test | 10 | 0.001638s | 0.001664s | 0.001621s | byte equality observed |
| buffered_stream.test | 10 | 0.001630s | 0.001629s | 0.001621s | byte equality observed |
| buffered_stream_select.test | 10 | 0.001634s | 0.001634s | 0.001595s | byte equality observed |
| buffered_write_stream.test | 10 | 0.001656s | 0.001623s | 0.001620s | byte equality observed |
| buffered_write_stream_select.test | 10 | 0.001631s | 0.001643s | 0.001628s | byte equality observed |
| buffers_iterator.test | 10 | 0.001250s | 0.001227s | 0.001289s | byte equality observed |
| buffers_iterator_select.test | 10 | 0.001186s | 0.001151s | 0.001179s | byte equality observed |
| cancel_after.test | 1 | 0.003462s | 0.003236s | 0.003279s | byte equality observed |
| cancel_after_select.test | 1 | 0.003002s | 0.002869s | 0.002948s | byte equality observed |
| cancel_at.test | 1 | 0.003691s | 0.003488s | 0.003564s | byte equality observed |
| cancel_at_select.test | 1 | 0.003545s | 0.003319s | 0.003317s | byte equality observed |
| co_composed.test | 10 | 0.001201s | 0.001187s | 0.001216s | byte equality observed |
| co_composed_select.test | 10 | 0.001173s | 0.001151s | 0.001175s | byte equality observed |
| compose.test | 1 | 0.002033s | 0.001870s | 0.002000s | byte equality observed |
| compose_select.test | 1 | 0.001652s | 0.001562s | 0.001545s | byte equality observed |
| composed.test | 1 | 0.002221s | 0.002102s | 0.002200s | byte equality observed |
| composed_select.test | 1 | 0.001765s | 0.001740s | 0.001723s | byte equality observed |
| config.test | 10 | 0.001309s | 0.001274s | 0.001295s | byte equality observed |
| config_select.test | 10 | 0.001368s | 0.001333s | 0.001334s | byte equality observed |
| connect_pipe.test | 7 | 0.001408s | 0.001339s | 0.001352s | byte equality observed |
| connect_pipe_select.test | 8 | 0.001340s | 0.001377s | 0.001317s | byte equality observed |
| consign.test | 10 | 0.001353s | 0.001383s | 0.001338s | byte equality observed |
| consign_select.test | 10 | 0.001262s | 0.001260s | 0.001230s | byte equality observed |
| deadline_timer.test | 1 | 0.002750s | 0.002726s | 0.002752s | byte equality observed |
| deadline_timer_select.test | 1 | 0.002509s | 0.002484s | 0.002457s | byte equality observed |
| deferred.test | 10 | 0.001185s | 0.001189s | 0.001188s | byte equality observed |
| deferred_select.test | 10 | 0.001131s | 0.001148s | 0.001124s | byte equality observed |
| disposition.test | 10 | 0.001197s | 0.001215s | 0.001191s | byte equality observed |
| disposition_select.test | 10 | 0.001211s | 0.001204s | 0.001210s | byte equality observed |
| error.test | 10 | 0.001193s | 0.001186s | 0.001173s | byte equality observed |
| error_select.test | 10 | 0.001187s | 0.001186s | 0.001171s | byte equality observed |
| inline_executor.test | 10 | 0.001204s | 0.001202s | 0.001195s | byte equality observed |
| inline_executor_select.test | 10 | 0.001196s | 0.001193s | 0.001190s | byte equality observed |
| inline_or_executor.test | 10 | 0.001207s | 0.001204s | 0.001199s | byte equality observed |
| inline_or_executor_select.test | 10 | 0.001201s | 0.001205s | 0.001180s | byte equality observed |
| io_context.test | 1 | 0.002272s | 0.002159s | 0.002265s | byte equality observed |
| io_context_select.test | 1 | 0.002497s | 0.002434s | 0.002541s | byte equality observed |
| io_context_strand.test | 1 | 0.002595s | 0.002562s | 0.002641s | byte equality observed |
| io_context_strand_select.test | 1 | 0.002293s | 0.002158s | 0.002293s | byte equality observed |
| ip_host_name.test | 10 | 0.001205s | 0.001202s | 0.001212s | byte equality observed |
| ip_host_name_select.test | 10 | 0.001170s | 0.001159s | 0.001162s | byte equality observed |
| ip_icmp.test | 10 | 0.001209s | 0.001214s | 0.001182s | byte equality observed |
| ip_icmp_select.test | 10 | 0.001207s | 0.001207s | 0.001191s | byte equality observed |
| ip_multicast.test | 10 | 0.001289s | 0.001279s | 0.001293s | byte equality observed |
| ip_multicast_select.test | 10 | 0.001263s | 0.001258s | 0.001262s | byte equality observed |
| ip_tcp.test | 10 | 0.001801s | 0.001776s | 0.001801s | byte equality observed |
| ip_tcp_select.test | 10 | 0.001716s | 0.001730s | 0.001711s | byte equality observed |
| ip_udp.test | 10 | 0.001312s | 0.001405s | 0.001302s | byte equality observed |
| ip_udp_select.test | 10 | 0.001312s | 0.001386s | 0.001287s | byte equality observed |
| ip_unicast.test | 10 | 0.001257s | 0.001352s | 0.001241s | byte equality observed |
| ip_unicast_select.test | 10 | 0.001248s | 0.001363s | 0.001205s | byte equality observed |
| ip_v6_only.test | 10 | 0.001273s | 0.001342s | 0.001234s | byte equality observed |
| ip_v6_only_select.test | 10 | 0.001313s | 0.001384s | 0.001199s | byte equality observed |
| is_read_buffered.test | 10 | 0.001292s | 0.001257s | 0.001269s | byte equality observed |
| is_read_buffered_select.test | 10 | 0.001306s | 0.001260s | 0.001293s | byte equality observed |
| is_write_buffered.test | 10 | 0.001283s | 0.001251s | 0.001283s | byte equality observed |
| is_write_buffered_select.test | 10 | 0.001299s | 0.001278s | 0.001313s | byte equality observed |
| prepend.test | 10 | 0.001362s | 0.001368s | 0.001381s | byte equality observed |
| prepend_select.test | 10 | 0.001291s | 0.001287s | 0.001317s | byte equality observed |
| read.test | 10 | 0.001801s | 0.001771s | 0.001758s | byte equality observed |
| read_at.test | 9 | 0.002227s | 0.002315s | 0.002192s | byte equality observed |
| read_at_select.test | 9 | 0.002192s | 0.002239s | 0.002231s | byte equality observed |
| read_select.test | 10 | 0.001812s | 0.001758s | 0.001742s | byte equality observed |
| read_until.test | 10 | 0.001522s | 0.001469s | 0.001475s | byte equality observed |
| read_until_select.test | 10 | 0.001514s | 0.001442s | 0.001492s | byte equality observed |
| recycling_allocator.test | 10 | 0.001248s | 0.001293s | 0.001298s | byte equality observed |
| recycling_allocator_select.test | 10 | 0.001281s | 0.001305s | 0.001271s | byte equality observed |
| signal_set.test | 10 | 0.001321s | 0.001301s | 0.001298s | byte equality observed |
| signal_set_select.test | 10 | 0.001282s | 0.001271s | 0.001301s | byte equality observed |
| socket_base.test | 10 | 0.001402s | 0.001405s | 0.001412s | byte equality observed |
| socket_base_select.test | 10 | 0.001465s | 0.001392s | 0.001423s | byte equality observed |
| spawn.test | 10 | 0.001537s | 0.001555s | 0.001504s | byte equality observed |
| spawn_select.test | 10 | 0.001548s | 0.001554s | 0.001528s | byte equality observed |
| static_thread_pool.test | 10 | 0.001260s | 0.001243s | 0.001286s | byte equality observed |
| static_thread_pool_select.test | 10 | 0.001241s | 0.001178s | 0.001261s | byte equality observed |
| strand.test | 1 | 0.002332s | 0.002334s | 0.002499s | byte equality observed |
| strand_select.test | 1 | 0.002041s | 0.002036s | 0.002104s | byte equality observed |
| streambuf.test | 10 | 0.001303s | 0.001285s | 0.001262s | byte equality observed |
| streambuf_select.test | 10 | 0.001271s | 0.001259s | 0.001292s | byte equality observed |
| thread_pool.test | 8 | 0.002984s | 0.002916s | 0.002913s | byte equality observed |
| thread_pool_select.test | 7 | 0.002918s | 0.002902s | 0.002919s | byte equality observed |
| write.test | 10 | 0.001862s | 0.001817s | 0.001809s | byte equality observed |
| write_at.test | 10 | 0.002024s | 0.002004s | 0.001982s | byte equality observed |
| write_at_select.test | 10 | 0.002015s | 0.002024s | 0.001967s | byte equality observed |
| write_select.test | 10 | 0.001862s | 0.001821s | 0.001873s | byte equality observed |

[Raw outcomes, commands and timing observations in CI artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43609274616).

[Comparison records](../results.json) · [Host and toolchains](../metadata.json)
