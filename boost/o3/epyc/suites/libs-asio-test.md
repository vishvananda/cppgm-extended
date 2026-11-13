# libs/asio/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| any_completion_executor.test | RUN | 0.475461 | 2.752505 | 2.426779 | 0.002132 | 0.001972 | 0.002102 |
| any_completion_executor_select.test | RUN | 0.459668 | 2.769697 | 2.548438 | 0.002126 | 0.002115 | 0.002017 |
| any_completion_handler.test | RUN | 0.514255 | 2.606771 | 2.253374 | 0.001395 | 0.001595 | 0.001476 |
| any_completion_handler_select.test | RUN | 0.555068 | 2.649881 | 2.396741 | 0.001592 | 0.001790 | 0.001602 |
| any_io_executor.test | RUN | 0.477696 | 2.997658 | 2.609650 | 0.001770 | 0.002186 | 0.001989 |
| any_io_executor_select.test | RUN | 0.490297 | 2.942586 | 2.708560 | 0.001556 | 0.002046 | 0.002010 |
| append.test | RUN | 0.452118 | 2.446671 | 1.982233 | 0.001614 | 0.001721 | 0.001624 |
| append_select.test | RUN | 0.459375 | 2.480149 | 1.931769 | 0.001455 | 0.001475 | 0.001559 |
| as_tuple.test | RUN | 0.586937 | 2.972880 | 2.753192 | 0.001565 | 0.001438 | 0.001423 |
| as_tuple_select.test | RUN | 0.592306 | 2.997691 | 2.749857 | 0.001360 | 0.001770 | 0.001508 |
| associated_allocator.test | LINK | 0.130901 | 0.235448 | 0.294671 | — | — | — |
| associated_allocator_select.test | LINK | 0.125103 | 0.240308 | 0.296886 | — | — | — |
| associated_cancellation_slot.test | LINK | 0.134257 | 0.258849 | 0.323239 | — | — | — |
| associated_cancellation_slot_select.test | LINK | 0.133747 | 0.261719 | 0.319324 | — | — | — |
| associated_executor.test | LINK | 0.194118 | 0.449922 | 0.553653 | — | — | — |
| associated_executor_select.test | LINK | 0.192688 | 0.458897 | 0.558073 | — | — | — |
| associated_immediate_executor.test | LINK | 0.178906 | 0.418662 | 0.471092 | — | — | — |
| associated_immediate_executor_select.test | LINK | 0.181669 | 0.432515 | 0.487150 | — | — | — |
| associator.test | LINK | 0.101003 | 0.200729 | 0.241032 | — | — | — |
| associator_select.test | LINK | 0.103287 | 0.209362 | 0.239517 | — | — | — |
| awaitable.test | LINK | 0.102729 | 0.201053 | 0.246906 | — | — | — |
| awaitable_select.test | LINK | 0.102763 | 0.198628 | 0.238624 | — | — | — |
| basic_datagram_socket.test | LINK | 0.298066 | 0.639925 | 0.913442 | — | — | — |
| basic_datagram_socket_select.test | LINK | 0.307475 | 0.646053 | 0.936803 | — | — | — |
| basic_deadline_timer.test | LINK | 0.362185 | 0.768060 | 1.034644 | — | — | — |
| basic_deadline_timer_select.test | LINK | 0.367321 | 0.777798 | 1.054826 | — | — | — |
| basic_file.test | LINK | 0.099419 | 0.209133 | 0.243249 | — | — | — |
| basic_file_select.test | LINK | 0.101105 | 0.211606 | 0.244205 | — | — | — |
| basic_random_access_file.test | LINK | 0.100492 | 0.206087 | 0.249405 | — | — | — |
| basic_random_access_file_select.test | LINK | 0.098891 | 0.201005 | 0.252017 | — | — | — |
| basic_raw_socket.test | LINK | 0.309429 | 0.643217 | 0.906076 | — | — | — |
| basic_raw_socket_select.test | LINK | 0.305916 | 0.658402 | 0.928075 | — | — | — |
| basic_readable_pipe.test | LINK | 0.295785 | 0.620774 | 0.784718 | — | — | — |
| basic_readable_pipe_select.test | LINK | 0.308814 | 0.648809 | 0.947269 | — | — | — |
| basic_seq_packet_socket.test | LINK | 0.301186 | 0.640728 | 0.917986 | — | — | — |
| basic_seq_packet_socket_select.test | LINK | 0.309154 | 0.649762 | 0.924465 | — | — | — |
| basic_signal_set.test | LINK | 0.279813 | 0.595813 | 0.717402 | — | — | — |
| basic_signal_set_select.test | LINK | 0.292260 | 0.629044 | 0.859652 | — | — | — |
| basic_socket_acceptor.test | LINK | 0.300969 | 0.645167 | 0.938467 | — | — | — |
| basic_socket_acceptor_select.test | LINK | 0.310536 | 0.669519 | 0.942751 | — | — | — |
| basic_stream_file.test | LINK | 0.100070 | 0.201418 | 0.234222 | — | — | — |
| basic_stream_file_select.test | LINK | 0.102260 | 0.194723 | 0.234552 | — | — | — |
| basic_stream_socket.test | LINK | 0.299421 | 0.636906 | 0.893869 | — | — | — |
| basic_stream_socket_select.test | LINK | 0.311357 | 0.671542 | 0.922228 | — | — | — |
| basic_streambuf.test | LINK | 0.147084 | 0.287363 | 0.355980 | — | — | — |
| basic_streambuf_select.test | LINK | 0.144605 | 0.286500 | 0.356794 | — | — | — |
| basic_waitable_timer.test | LINK | 0.284490 | 0.600596 | 0.877515 | — | — | — |
| basic_waitable_timer_select.test | LINK | 0.284682 | 0.620772 | 0.926395 | — | — | — |
| basic_writable_pipe.test | LINK | 0.298372 | 0.617679 | 0.820740 | — | — | — |
| basic_writable_pipe_select.test | LINK | 0.313932 | 0.663215 | 1.003750 | — | — | — |
| bind_allocator.test | RUN | 0.478815 | 2.483601 | 2.228876 | 0.001720 | 0.001864 | 0.001624 |
| bind_allocator_select.test | RUN | 0.495178 | 2.465340 | 2.213107 | 0.001173 | 0.001519 | 0.001812 |
| bind_cancellation_slot.test | RUN | 0.490372 | 2.495880 | 2.266546 | 0.001329 | 0.001249 | 0.001267 |
| bind_cancellation_slot_select.test | RUN | 0.505835 | 2.527461 | 2.158241 | 0.001193 | 0.001269 | 0.001221 |
| bind_executor.test | RUN | 0.333750 | 1.739459 | 1.356080 | 0.001811 | 0.001807 | 0.001818 |
| bind_executor_select.test | RUN | 0.344472 | 1.733296 | 1.368299 | 0.001496 | 0.001677 | 0.001599 |
| bind_immediate_executor.test | RUN | 0.257347 | 1.397670 | 0.850876 | 0.001179 | 0.001200 | 0.001145 |
| bind_immediate_executor_select.test | RUN | 0.259543 | 1.395650 | 0.933669 | 0.001022 | 0.001203 | 0.001121 |
| buffer.test | RUN | 0.152881 | 0.570793 | 0.491580 | 0.001124 | 0.001197 | 0.001151 |
| buffer_select.test | RUN | 0.151791 | 0.569632 | 0.489600 | 0.001069 | 0.001126 | 0.001103 |
| buffer_registration.test | LINK | 0.124794 | 0.301622 | 0.296815 | — | — | — |
| buffer_registration_select.test | LINK | 0.124670 | 0.300581 | 0.299309 | — | — | — |
| buffered_read_stream.test | RUN | 0.539767 | 3.594330 | 2.863341 | 0.001473 | 0.002060 | 0.001761 |
| buffered_read_stream_select.test | RUN | 0.540244 | 3.660546 | 2.732916 | 0.001481 | 0.002086 | 0.001826 |
| buffered_stream.test | RUN | 0.681933 | 5.986059 | 5.457195 | 0.001386 | 0.001911 | 0.002074 |
| buffered_stream_select.test | RUN | 0.683728 | 6.029410 | 5.318806 | 0.001454 | 0.001902 | 0.001866 |
| buffered_write_stream.test | RUN | 0.802965 | 5.177301 | 4.715895 | 0.002050 | 0.001922 | 0.001971 |
| buffered_write_stream_select.test | RUN | 0.809599 | 5.163743 | 4.563673 | 0.001798 | 0.001939 | 0.001825 |
| buffers_iterator.test | RUN | 0.267822 | 0.458661 | 0.716815 | 0.001550 | 0.001458 | 0.001316 |
| buffers_iterator_select.test | RUN | 0.268855 | 0.462980 | 0.715304 | 0.001383 | 0.001438 | 0.001403 |
| cancel_after.test | RUN | 0.706530 | 3.962989 | 3.385197 | 0.002520 | 0.002464 | 0.002482 |
| cancel_after_select.test | RUN | 0.714787 | 3.967325 | 3.337521 | 0.002304 | 0.002550 | 0.002181 |
| cancel_at.test | RUN | 0.647622 | 3.732882 | 3.348245 | 0.001902 | 0.002275 | 0.002247 |
| cancel_at_select.test | RUN | 0.636528 | 3.673594 | 3.282519 | 0.002084 | 0.002220 | 0.001900 |
| cancellation_signal.test | LINK | 0.091521 | 0.178677 | 0.220760 | — | — | — |
| cancellation_signal_select.test | LINK | 0.093377 | 0.177212 | 0.228966 | — | — | — |
| cancellation_state.test | LINK | 0.130158 | 0.177698 | 0.300729 | — | — | — |
| cancellation_state_select.test | LINK | 0.135238 | 0.177530 | 0.302146 | — | — | — |
| cancellation_type.test | LINK | 0.098659 | 0.192010 | 0.246516 | — | — | — |
| cancellation_type_select.test | LINK | 0.099728 | 0.195338 | 0.237772 | — | — | — |
| co_composed.test | RUN | 0.097563 | 0.206347 | 0.238123 | 0.001354 | 0.001411 | 0.001573 |
| co_composed_select.test | RUN | 0.098933 | 0.201232 | 0.247569 | 0.001298 | 0.001256 | 0.001153 |
| co_spawn.test | LINK | 0.100378 | 0.204820 | 0.237862 | — | — | — |
| co_spawn_select.test | LINK | 0.103246 | 0.205349 | 0.243609 | — | — | — |
| completion_condition.test | LINK | 0.126917 | 0.313909 | 0.326146 | — | — | — |
| completion_condition_select.test | LINK | 0.126042 | 0.311775 | 0.328022 | — | — | — |
| compose.test | RUN | 0.580144 | 3.013069 | 2.697822 | 0.001696 | 0.001415 | 0.001916 |
| compose_select.test | RUN | 0.588847 | 2.984409 | 2.730348 | 0.001636 | 0.001751 | 0.001896 |
| composed.test | RUN | 0.551107 | 2.969046 | 2.653664 | 0.001732 | 0.001518 | 0.001716 |
| composed_select.test | RUN | 0.552437 | 2.935615 | 2.622905 | 0.001294 | 0.001669 | 0.001310 |
| config.test | RUN | 0.270332 | 1.965212 | 1.148662 | 0.001632 | 0.001740 | 0.001657 |
| config_select.test | RUN | 0.288184 | 2.010040 | 1.332170 | 0.001477 | 0.001451 | 0.001498 |
| connect.test | LINK | 0.930621 | 6.390430 | 5.919663 | — | — | — |
| connect_select.test | LINK | 0.952216 | 6.433161 | 5.881657 | — | — | — |
| connect_pipe.test | RUN | 0.528355 | 2.495458 | 2.164801 | 0.001717 | 0.001750 | 0.001661 |
| connect_pipe_select.test | RUN | 0.546829 | 2.564370 | 2.290275 | 0.001477 | 0.001674 | 0.001604 |
| consign.test | RUN | 0.439448 | 2.333851 | 1.996028 | 0.001622 | 0.001256 | 0.001622 |
| consign_select.test | RUN | 0.454797 | 2.338182 | 1.951284 | 0.001632 | 0.001402 | 0.001642 |
| coroutine.test | LINK | 0.104777 | 0.243107 | 0.290186 | — | — | — |
| coroutine_select.test | LINK | 0.108585 | 0.240863 | 0.273511 | — | — | — |
| deadline_timer.test | RUN | 0.642791 | 3.112496 | 2.877546 | 0.002628 | 0.002554 | 0.002523 |
| deadline_timer_select.test | RUN | 0.652357 | 3.081058 | 2.857142 | 0.002541 | 0.002712 | 0.002558 |
| deferred.test | RUN | 0.116590 | 0.210553 | 0.265233 | 0.001510 | 0.001273 | 0.001545 |
| deferred_select.test | RUN | 0.114662 | 0.208699 | 0.265934 | 0.001481 | 0.001274 | 0.001479 |
| detached.test | LINK | 0.125223 | 0.239178 | 0.294167 | — | — | — |
| detached_select.test | LINK | 0.129899 | 0.234777 | 0.305126 | — | — | — |
| disposition.test | RUN | 0.120376 | 0.551905 | 0.572636 | 0.001252 | 0.001563 | 0.001626 |
| disposition_select.test | RUN | 0.117128 | 0.551607 | 0.551161 | 0.001163 | 0.001336 | 0.001552 |
| error.test | RUN | 0.109923 | 0.561326 | 0.501488 | 0.001211 | 0.001269 | 0.001570 |
| error_select.test | RUN | 0.111009 | 0.566144 | 0.498691 | 0.001139 | 0.001558 | 0.001642 |
| file_base.test | LINK | 0.136338 | 0.471933 | 0.551971 | — | — | — |
| file_base_select.test | LINK | 0.138175 | 0.478580 | 0.472709 | — | — | — |
| fwd_first.test | LINK | 0.305022 | 0.759292 | 0.963104 | — | — | — |
| fwd_last.test | LINK | 0.306404 | 0.751206 | 0.967587 | — | — | — |
| generic_basic_endpoint.test | LINK | 0.119495 | 0.281022 | 0.348733 | — | — | — |
| generic_basic_endpoint_select.test | LINK | 0.119741 | 0.273969 | 0.346388 | — | — | — |
| generic_datagram_protocol.test | LINK | 0.469190 | 2.520928 | 1.989622 | — | — | — |
| generic_datagram_protocol_select.test | LINK | 0.468042 | 2.593286 | 1.989755 | — | — | — |
| generic_raw_protocol.test | LINK | 0.471571 | 2.475260 | 1.993614 | — | — | — |
| generic_raw_protocol_select.test | LINK | 0.468832 | 2.583053 | 1.943901 | — | — | — |
| generic_seq_packet_protocol.test | LINK | 0.416505 | 2.087721 | 1.710517 | — | — | — |
| generic_seq_packet_protocol_select.test | LINK | 0.420881 | 2.117385 | 1.695918 | — | — | — |
| generic_stream_protocol.test | LINK | 0.440748 | 2.259245 | 1.841647 | — | — | — |
| generic_stream_protocol_select.test | LINK | 0.443196 | 2.347539 | 1.800006 | — | — | — |
| high_resolution_timer.test | LINK | 0.193372 | 0.413663 | 0.537456 | — | — | — |
| high_resolution_timer_select.test | LINK | 0.199565 | 0.429208 | 0.545123 | — | — | — |
| immediate.test | LINK | 0.136787 | 0.318230 | 0.317604 | — | — | — |
| immediate_select.test | LINK | 0.138362 | 0.319026 | 0.314173 | — | — | — |
| inline_executor.test | RUN | 0.164708 | 0.491775 | 0.485151 | 0.001231 | 0.001217 | 0.001111 |
| inline_executor_select.test | RUN | 0.168256 | 0.501122 | 0.489493 | 0.001145 | 0.001204 | 0.001110 |
| inline_or_executor.test | RUN | 0.262058 | 1.447826 | 0.982678 | 0.001240 | 0.001175 | 0.001166 |
| inline_or_executor_select.test | RUN | 0.273142 | 1.503599 | 1.066678 | 0.001157 | 0.001209 | 0.001156 |
| io_context.test | RUN | 0.387966 | 2.214941 | 1.707709 | 0.001823 | 0.001793 | 0.001788 |
| io_context_select.test | RUN | 0.385098 | 2.171814 | 1.679079 | 0.001742 | 0.001866 | 0.001654 |
| io_context_strand.test | RUN | 0.371667 | 1.886333 | 1.500206 | 0.001892 | 0.001717 | 0.001752 |
| io_context_strand_select.test | RUN | 0.374860 | 1.863794 | 1.494088 | 0.001574 | 0.001744 | 0.001624 |
| ip_address.test | LINK | 0.143041 | 0.464586 | 0.436762 | — | — | — |
| ip_address_select.test | LINK | 0.141511 | 0.617561 | 0.428787 | — | — | — |
| ip_address_v4.test | LINK | 0.143728 | 0.673010 | 0.453120 | — | — | — |
| ip_address_v4_select.test | LINK | 0.142627 | 0.680918 | 0.450614 | — | — | — |
| ip_address_v6.test | LINK | 0.158691 | 0.904063 | 0.561067 | — | — | — |
| ip_address_v6_select.test | LINK | 0.158976 | 0.897176 | 0.557374 | — | — | — |
| ip_basic_endpoint.test | LINK | 0.125437 | 0.429651 | 0.355949 | — | — | — |
| ip_basic_endpoint_select.test | LINK | 0.125899 | 0.433674 | 0.357165 | — | — | — |
| ip_basic_resolver.test | LINK | 0.197420 | 0.621672 | 0.734342 | — | — | — |
| ip_basic_resolver_select.test | LINK | 0.202696 | 0.629686 | 0.780992 | — | — | — |
| ip_basic_resolver_entry.test | LINK | 0.071088 | 0.203781 | 0.250589 | — | — | — |
| ip_basic_resolver_entry_select.test | LINK | 0.072303 | 0.208357 | 0.257957 | — | — | — |
| ip_basic_resolver_iterator.test | LINK | 0.126848 | 0.416965 | 0.585631 | — | — | — |
| ip_basic_resolver_iterator_select.test | LINK | 0.128036 | 0.419820 | 0.601893 | — | — | — |
| ip_basic_resolver_query.test | LINK | 0.120799 | 0.397397 | 0.552459 | — | — | — |
| ip_basic_resolver_query_select.test | LINK | 0.122381 | 0.403750 | 0.556388 | — | — | — |
| ip_host_name.test | RUN | 0.128731 | 0.549406 | 0.649027 | 0.001308 | 0.001611 | 0.001457 |
| ip_host_name_select.test | RUN | 0.132327 | 0.545100 | 0.622484 | 0.001022 | 0.001581 | 0.001477 |
| ip_icmp.test | RUN | 0.580129 | 5.133317 | 4.273441 | 0.001209 | 0.001606 | 0.001471 |
| ip_icmp_select.test | RUN | 0.580425 | 5.248909 | 4.209667 | 0.001070 | 0.001660 | 0.001630 |
| ip_multicast.test | RUN | 0.344925 | 2.550506 | 2.330685 | 0.001289 | 0.001733 | 0.001734 |
| ip_multicast_select.test | RUN | 0.353413 | 2.548270 | 2.347762 | 0.001181 | 0.001732 | 0.001466 |
| ip_resolver_query_base.test | LINK | 0.085627 | 0.222734 | 0.276944 | — | — | — |
| ip_resolver_query_base_select.test | LINK | 0.083193 | 0.221680 | 0.269373 | — | — | — |
| ip_tcp.test | RUN | 0.847013 | 7.597452 | 6.774957 | 0.001717 | 0.002133 | 0.002381 |
| ip_tcp_select.test | RUN | 0.852722 | 8.018464 | 6.440270 | 0.001579 | 0.002223 | 0.002284 |
| ip_udp.test | RUN | 0.609770 | 5.451360 | 4.555118 | 0.001340 | 0.001702 | 0.001792 |
| ip_udp_select.test | RUN | 0.612793 | 5.638244 | 4.385612 | 0.001249 | 0.001840 | 0.001660 |
| ip_unicast.test | RUN | 0.314921 | 2.360290 | 2.080363 | 0.001223 | 0.001416 | 0.001750 |
| ip_unicast_select.test | RUN | 0.321065 | 2.391199 | 2.006444 | 0.001187 | 0.001598 | 0.001567 |
| ip_v6_only.test | RUN | 0.328514 | 2.426447 | 2.153927 | 0.001270 | 0.001664 | 0.001645 |
| ip_v6_only_select.test | RUN | 0.332424 | 2.514457 | 2.123516 | 0.001156 | 0.001708 | 0.001652 |
| is_read_buffered.test | RUN | 0.227265 | 0.748566 | 1.155247 | 0.001123 | 0.001599 | 0.001434 |
| is_read_buffered_select.test | RUN | 0.231659 | 0.737900 | 1.152423 | 0.001066 | 0.001371 | 0.001451 |
| is_write_buffered.test | RUN | 0.228147 | 0.742890 | 1.090556 | 0.001136 | 0.001498 | 0.001440 |
| is_write_buffered_select.test | RUN | 0.235865 | 0.734632 | 1.122782 | 0.001114 | 0.001284 | 0.001530 |
| local_basic_endpoint.test | LINK | 0.121094 | 0.413822 | 0.563702 | — | — | — |
| local_basic_endpoint_select.test | LINK | 0.122956 | 0.401117 | 0.552855 | — | — | — |
| local_connect_pair.test | LINK | 0.308203 | 2.475687 | 1.955617 | — | — | — |
| local_connect_pair_select.test | LINK | 0.309199 | 2.502731 | 1.923408 | — | — | — |
| local_datagram_protocol.test | LINK | 0.412435 | 3.465186 | 2.791332 | — | — | — |
| local_datagram_protocol_select.test | LINK | 0.411347 | 3.457249 | 2.738942 | — | — | — |
| local_seq_packet_protocol.test | LINK | 0.415350 | 3.864889 | 2.777183 | — | — | — |
| local_seq_packet_protocol_select.test | LINK | 0.410004 | 3.895092 | 2.782685 | — | — | — |
| local_stream_protocol.test | LINK | 0.385961 | 3.819182 | 2.670509 | — | — | — |
| local_stream_protocol_select.test | LINK | 0.400656 | 3.816005 | 2.651831 | — | — | — |
| placeholders.test | LINK | 0.079333 | 0.294522 | 0.296191 | — | — | — |
| placeholders_select.test | LINK | 0.078848 | 0.294328 | 0.295926 | — | — | — |
| posix_basic_descriptor.test | LINK | 0.201680 | 0.783407 | 0.847683 | — | — | — |
| posix_basic_descriptor_select.test | LINK | 0.211888 | 0.805323 | 1.018269 | — | — | — |
| posix_basic_stream_descriptor.test | LINK | 0.199489 | 0.756894 | 0.801906 | — | — | — |
| posix_basic_stream_descriptor_select.test | LINK | 0.212564 | 0.817961 | 1.009657 | — | — | — |
| posix_descriptor_base.test | LINK | 0.088654 | 0.295450 | 0.279103 | — | — | — |
| posix_descriptor_base_select.test | LINK | 0.086003 | 0.289044 | 0.272145 | — | — | — |
| posix_stream_descriptor.test | LINK | 0.388490 | 3.659388 | 2.620024 | — | — | — |
| posix_stream_descriptor_select.test | LINK | 0.399028 | 3.664123 | 2.662513 | — | — | — |
| prepend.test | RUN | 0.305911 | 3.024603 | 2.045489 | 0.001204 | 0.002074 | 0.001693 |
| prepend_select.test | RUN | 0.313594 | 2.997020 | 2.075108 | 0.001193 | 0.001942 | 0.001548 |
| random_access_file.test | LINK | 0.186796 | 0.685019 | 0.680014 | — | — | — |
| random_access_file_select.test | LINK | 0.191723 | 0.715619 | 0.845888 | — | — | — |
| read.test | RUN | 0.902743 | 17.928715 | 11.635416 | 0.001608 | 0.002518 | 0.002014 |
| read_select.test | RUN | 0.914069 | 17.751330 | 11.609336 | 0.001774 | 0.002551 | 0.002120 |
| read_at.test | RUN | 1.339352 | 25.012145 | 15.869051 | 0.002271 | 0.002989 | 0.002396 |
| read_at_select.test | RUN | 1.362748 | 24.919160 | 16.111795 | 0.002269 | 0.002970 | 0.002245 |
| read_until.test | RUN | 0.753102 | 6.990262 | 4.434582 | 0.001696 | 0.002192 | 0.001967 |
| read_until_select.test | RUN | 0.744868 | 6.931563 | 4.499444 | 0.001738 | 0.002016 | 0.001783 |
| readable_pipe.test | LINK | 0.472244 | 2.975737 | 2.091995 | — | — | — |
| readable_pipe_select.test | LINK | 0.485550 | 3.142039 | 2.231379 | — | — | — |
| recycling_allocator.test | RUN | 0.148284 | 0.448507 | 0.425032 | 0.001398 | 0.001874 | 0.001542 |
| recycling_allocator_select.test | RUN | 0.144937 | 0.457228 | 0.415458 | 0.001577 | 0.001788 | 0.001587 |
| redirect_disposition.test | LINK | 0.605803 | 4.204764 | 3.258756 | — | — | — |
| redirect_disposition_select.test | LINK | 0.625904 | 4.193501 | 3.271579 | — | — | — |
| redirect_error.test | LINK | 0.559687 | 3.487707 | 2.774942 | — | — | — |
| redirect_error_select.test | LINK | 0.550209 | 3.499526 | 2.781239 | — | — | — |
| registered_buffer.test | LINK | 0.143367 | 0.358468 | 0.382221 | — | — | — |
| registered_buffer_select.test | LINK | 0.148125 | 0.360892 | 0.394113 | — | — | — |
| signal_set.test | RUN | 0.454463 | 2.942486 | 1.986214 | 0.001520 | 0.001882 | 0.001631 |
| signal_set_select.test | RUN | 0.476960 | 3.051216 | 2.174104 | 0.001608 | 0.001929 | 0.001683 |
| signal_set_base.test | LINK | 0.119428 | 0.293159 | 0.264255 | — | — | — |
| signal_set_base_select.test | LINK | 0.126348 | 0.289750 | 0.265954 | — | — | — |
| socket_base.test | RUN | 0.549890 | 4.308379 | 2.914265 | 0.001769 | 0.002080 | 0.001650 |
| socket_base_select.test | RUN | 0.563761 | 4.233147 | 2.886493 | 0.001749 | 0.002322 | 0.001747 |
| spawn.test | RUN | 0.913306 | 7.114730 | 5.138660 | 0.001687 | 0.002182 | 0.001894 |
| spawn_select.test | RUN | 0.934637 | 7.150066 | 5.160204 | 0.001795 | 0.002307 | 0.001920 |
| static_thread_pool.test | RUN | 0.264138 | 0.684730 | 0.677773 | 0.001420 | 0.001876 | 0.001631 |
| static_thread_pool_select.test | RUN | 0.275608 | 0.723657 | 0.837773 | 0.001372 | 0.001842 | 0.001637 |
| steady_timer.test | LINK | 0.287424 | 0.809130 | 0.906309 | — | — | — |
| steady_timer_select.test | LINK | 0.285393 | 0.806543 | 0.910101 | — | — | — |
| strand.test | RUN | 0.615918 | 4.041851 | 3.063869 | 0.002156 | 0.002556 | 0.002371 |
| strand_select.test | RUN | 0.625251 | 3.986521 | 3.063004 | 0.002355 | 0.002320 | 0.002155 |
| stream_file.test | LINK | 0.262005 | 0.715742 | 0.680344 | — | — | — |
| stream_file_select.test | LINK | 0.275336 | 0.739485 | 0.842820 | — | — | — |
| streambuf.test | RUN | 0.143784 | 0.493495 | 0.485503 | 0.001197 | 0.001409 | 0.001637 |
| streambuf_select.test | RUN | 0.120511 | 0.425117 | 0.388242 | 0.001123 | 0.001450 | 0.001149 |
| system_timer.test | LINK | 0.378862 | 2.679868 | 1.879558 | — | — | — |
| system_timer_select.test | LINK | 0.389028 | 2.615831 | 1.770437 | — | — | — |
| system_context.test | LINK | 0.176368 | 0.458949 | 0.423192 | — | — | — |
| system_context_select.test | LINK | 0.187529 | 0.487814 | 0.519950 | — | — | — |
| system_executor.test | LINK | 0.232207 | 1.469847 | 0.821013 | — | — | — |
| system_executor_select.test | LINK | 0.242732 | 1.463133 | 0.896814 | — | — | — |
| this_coro.test | LINK | 0.073371 | 0.174392 | 0.162612 | — | — | — |
| this_coro_select.test | LINK | 0.071742 | 0.176198 | 0.168090 | — | — | — |
| thread_pool.test | RUN | 0.277071 | 1.900945 | 1.072060 | 0.002168 | 0.002665 | 0.002214 |
| thread_pool_select.test | RUN | 0.283005 | 1.926191 | 1.134848 | 0.001937 | 0.002617 | 0.002194 |
| time_traits.test | LINK | 0.153245 | 0.451238 | 0.335963 | — | — | — |
| time_traits_select.test | LINK | 0.155615 | 0.515802 | 0.336028 | — | — | — |
| ts_buffer.test | LINK | 0.148496 | 0.566616 | 0.348991 | — | — | — |
| ts_buffer_select.test | LINK | 0.150919 | 0.599238 | 0.354440 | — | — | — |
| ts_executor.test | LINK | 0.206779 | 0.808133 | 0.552013 | — | — | — |
| ts_executor_select.test | LINK | 0.218736 | 0.872873 | 0.657995 | — | — | — |
| ts_internet.test | LINK | 0.224501 | 0.868489 | 0.674592 | — | — | — |
| ts_internet_select.test | LINK | 0.231210 | 0.918855 | 0.689860 | — | — | — |
| ts_io_context.test | LINK | 0.180392 | 0.671071 | 0.417248 | — | — | — |
| ts_io_context_select.test | LINK | 0.194365 | 0.733489 | 0.702729 | — | — | — |
| ts_net.test | LINK | 0.267511 | 1.064518 | 1.231292 | — | — | — |
| ts_net_select.test | LINK | 0.275401 | 1.063006 | 1.356017 | — | — | — |
| ts_netfwd.test | LINK | 0.261454 | 1.071782 | 1.317768 | — | — | — |
| ts_netfwd_select.test | LINK | 0.277412 | 1.094337 | 1.373133 | — | — | — |
| ts_socket.test | LINK | 0.219631 | 0.818863 | 1.013209 | — | — | — |
| ts_socket_select.test | LINK | 0.228719 | 0.868258 | 1.058829 | — | — | — |
| ts_timer.test | LINK | 0.194937 | 0.755529 | 0.874386 | — | — | — |
| ts_timer_select.test | LINK | 0.203772 | 0.764828 | 0.926421 | — | — | — |
| use_awaitable.test | LINK | 0.071827 | 0.258952 | 0.245560 | — | — | — |
| use_awaitable_select.test | LINK | 0.073205 | 0.248184 | 0.244581 | — | — | — |
| wait_traits.test | LINK | 0.072167 | 0.255065 | 0.261018 | — | — | — |
| wait_traits_select.test | LINK | 0.072529 | 0.261059 | 0.246230 | — | — | — |
| windows_basic_object_handle.test | LINK | 0.073114 | 0.249782 | 0.258264 | — | — | — |
| windows_basic_object_handle_select.test | LINK | 0.072325 | 0.249221 | 0.247818 | — | — | — |
| windows_basic_overlapped_handle.test | LINK | 0.072178 | 0.250555 | 0.256363 | — | — | — |
| windows_basic_overlapped_handle_select.test | LINK | 0.071859 | 0.249510 | 0.251210 | — | — | — |
| windows_basic_random_access_handle.test | LINK | 0.072191 | 0.249537 | 0.257518 | — | — | — |
| windows_basic_random_access_handle_select.test | LINK | 0.071944 | 0.250494 | 0.247063 | — | — | — |
| windows_basic_stream_handle.test | LINK | 0.071800 | 0.251642 | 0.254977 | — | — | — |
| windows_basic_stream_handle_select.test | LINK | 0.072004 | 0.248465 | 0.255850 | — | — | — |
| windows_object_handle.test | LINK | 0.186008 | 0.683573 | 0.677335 | — | — | — |
| windows_object_handle_select.test | LINK | 0.193998 | 0.730480 | 0.842258 | — | — | — |
| windows_overlapped_ptr.test | LINK | 0.193892 | 0.711123 | 0.733908 | — | — | — |
| windows_overlapped_ptr_select.test | LINK | 0.199703 | 0.748041 | 0.912996 | — | — | — |
| windows_random_access_handle.test | LINK | 0.190418 | 0.684403 | 0.678304 | — | — | — |
| windows_random_access_handle_select.test | LINK | 0.194848 | 0.735502 | 0.850025 | — | — | — |
| windows_stream_handle.test | LINK | 0.189697 | 0.681363 | 0.676763 | — | — | — |
| windows_stream_handle_select.test | LINK | 0.195216 | 0.727631 | 0.856820 | — | — | — |
| writable_pipe.test | LINK | 0.334214 | 2.958214 | 2.110081 | — | — | — |
| writable_pipe_select.test | LINK | 0.341473 | 3.082790 | 2.243911 | — | — | — |
| write.test | RUN | 1.176652 | 14.974217 | 11.562392 | 0.002074 | 0.002496 | 0.002246 |
| write_select.test | RUN | 1.258176 | 14.901396 | 11.774549 | 0.002242 | 0.002500 | 0.002258 |
| write_at.test | RUN | 1.401192 | 20.940688 | 14.584823 | 0.002124 | 0.002608 | 0.002203 |
| write_at_select.test | RUN | 1.417532 | 20.692367 | 14.634039 | 0.002167 | 0.002673 | 0.002120 |
| any_executor.test | RUN | 0.564073 | 6.766973 | 4.497253 | 0.002174 | 0.002389 | 0.001908 |
| blocking.test | RUN | 1.276536 | 8.985225 | 9.190423 | 0.006039 | 0.006930 | 0.006326 |
| blocking_adaptation.test | RUN | 0.634525 | 3.721304 | 4.190992 | 0.003361 | 0.004195 | 0.003171 |
| context_as.test | RUN | 0.377215 | 2.648613 | 1.453470 | 0.001730 | 0.002136 | 0.001561 |
| executor.test | RUN | 0.105175 | 0.265661 | 0.252587 | 0.001658 | 0.001741 | 0.001294 |
| inline_exception_handling.test | RUN | 1.210434 | 7.802865 | 9.252526 | 0.004453 | 0.005118 | 0.004262 |
| invocable_archetype.test | RUN | 0.104084 | 0.266449 | 0.263432 | 0.001576 | 0.001486 | 0.001496 |
| mapping.test | RUN | 1.180584 | 8.387054 | 9.206269 | 0.004231 | 0.005212 | 0.004361 |
| outstanding_work.test | RUN | 0.564759 | 3.570587 | 4.083287 | 0.003234 | 0.004100 | 0.003438 |
| prefer_only.test | RUN | 0.200461 | 1.365739 | 0.927160 | 0.001661 | 0.001882 | 0.001501 |
| relationship.test | RUN | 0.475220 | 3.425972 | 3.223194 | 0.002483 | 0.003978 | 0.002432 |

## Native runtime replay

Child CPU of successful native tests, with process startup included; GDB and expected-failure executions are correctness-only. Two warmups per compiler, four A/A observations per reference and mirrored ABBA/BAAB blocks. Short tests repeat up to ten times, identically across compilers; CPU is divided by repetitions before summing tests. Executable hashes and output checks stay outside process timers. All runs must pass their original native oracle. Output hashes record equality or variation across the complete campaign; test reports may print times or addresses.

| Reference | CPU speedup ↑ | Paired blocks | A/A speedup |
|---|---:|---|---:|
| gcc | 0.999× | 1.006×, 0.991× | 0.998× |
| clang | 0.994× | 0.994×, 0.994× | 0.991× |

† A/A drift or paired-block spread exceeds 5%; small differences are unresolved. 1375 native tests in the full cohort; 1364 have byte-identical output throughout. [Raw replay evidence in CI artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43493409407).

| Test | Repeats | CPU cppgm | GCC | Clang | Output check |
|---|---:|---:|---:|---:|---|
| any_completion_executor.test | 2 | 0.001358s | 0.001396s | 0.001361s | byte equality observed |
| any_completion_executor_select.test | 3 | 0.001322s | 0.001374s | 0.001347s | byte equality observed |
| any_completion_handler.test | 7 | 0.001087s | 0.001084s | 0.001095s | byte equality observed |
| any_completion_handler_select.test | 10 | 0.001071s | 0.001083s | 0.001081s | byte equality observed |
| any_io_executor.test | 7 | 0.001316s | 0.001317s | 0.001335s | byte equality observed |
| any_io_executor_select.test | 8 | 0.001331s | 0.001325s | 0.001347s | byte equality observed |
| append.test | 10 | 0.001022s | 0.001022s | 0.001027s | byte equality observed |
| append_select.test | 9 | 0.001023s | 0.001022s | 0.001019s | byte equality observed |
| as_tuple.test | 10 | 0.001101s | 0.001098s | 0.001092s | byte equality observed |
| as_tuple_select.test | 10 | 0.001019s | 0.001005s | 0.001048s | byte equality observed |
| bind_allocator.test | 1 | 0.001198s | 0.001140s | 0.001164s | byte equality observed |
| bind_allocator_select.test | 1 | 0.001082s | 0.001074s | 0.001106s | byte equality observed |
| bind_cancellation_slot.test | 10 | 0.000954s | 0.000931s | 0.000915s | byte equality observed |
| bind_cancellation_slot_select.test | 10 | 0.000896s | 0.000885s | 0.000888s | byte equality observed |
| bind_executor.test | 1 | 0.001133s | 0.001126s | 0.001133s | byte equality observed |
| bind_executor_select.test | 1 | 0.001105s | 0.001082s | 0.001086s | byte equality observed |
| bind_immediate_executor.test | 10 | 0.000897s | 0.000867s | 0.000878s | byte equality observed |
| bind_immediate_executor_select.test | 10 | 0.000903s | 0.000914s | 0.000900s | byte equality observed |
| buffer.test | 10 | 0.000886s | 0.000869s | 0.000889s | byte equality observed |
| buffer_select.test | 10 | 0.000878s | 0.000884s | 0.000866s | byte equality observed |
| buffered_read_stream.test | 10 | 0.001141s | 0.001141s | 0.001145s | byte equality observed |
| buffered_read_stream_select.test | 9 | 0.001135s | 0.001102s | 0.001118s | byte equality observed |
| buffered_stream.test | 10 | 0.001145s | 0.001142s | 0.001141s | byte equality observed |
| buffered_stream_select.test | 10 | 0.001126s | 0.001106s | 0.001102s | byte equality observed |
| buffered_write_stream.test | 10 | 0.001126s | 0.001132s | 0.001143s | byte equality observed |
| buffered_write_stream_select.test | 10 | 0.001121s | 0.001113s | 0.001132s | byte equality observed |
| buffers_iterator.test | 10 | 0.000871s | 0.000849s | 0.000856s | byte equality observed |
| buffers_iterator_select.test | 10 | 0.000840s | 0.000834s | 0.000846s | byte equality observed |
| cancel_after.test | 1 | 0.001700s | 0.001718s | 0.001713s | byte equality observed |
| cancel_after_select.test | 1 | 0.001599s | 0.001591s | 0.001615s | byte equality observed |
| cancel_at.test | 1 | 0.001813s | 0.001790s | 0.001829s | byte equality observed |
| cancel_at_select.test | 1 | 0.001658s | 0.001656s | 0.001656s | byte equality observed |
| co_composed.test | 10 | 0.000833s | 0.000834s | 0.000848s | byte equality observed |
| co_composed_select.test | 10 | 0.000823s | 0.000815s | 0.000841s | byte equality observed |
| compose.test | 1 | 0.001098s | 0.001128s | 0.001080s | byte equality observed |
| compose_select.test | 1 | 0.001031s | 0.001033s | 0.001012s | byte equality observed |
| composed.test | 1 | 0.001089s | 0.001091s | 0.001065s | byte equality observed |
| composed_select.test | 1 | 0.001048s | 0.001034s | 0.001046s | byte equality observed |
| config.test | 10 | 0.000882s | 0.000869s | 0.000880s | byte equality observed |
| config_select.test | 10 | 0.000923s | 0.000915s | 0.000901s | byte equality observed |
| connect_pipe.test | 10 | 0.000951s | 0.000941s | 0.000941s | byte equality observed |
| connect_pipe_select.test | 10 | 0.000943s | 0.000937s | 0.000934s | byte equality observed |
| consign.test | 10 | 0.000936s | 0.000952s | 0.000928s | byte equality observed |
| consign_select.test | 10 | 0.000882s | 0.000862s | 0.000889s | byte equality observed |
| deadline_timer.test | 1 | 0.001801s | 0.001742s | 0.001811s | byte equality observed |
| deadline_timer_select.test | 1 | 0.001610s | 0.001568s | 0.001640s | byte equality observed |
| deferred.test | 10 | 0.000798s | 0.000796s | 0.000827s | byte equality observed |
| deferred_select.test | 10 | 0.000787s | 0.000784s | 0.000819s | byte equality observed |
| disposition.test | 10 | 0.000845s | 0.000854s | 0.000876s | byte equality observed |
| disposition_select.test | 10 | 0.000842s | 0.000852s | 0.000870s | byte equality observed |
| error.test | 10 | 0.000832s | 0.000838s | 0.000837s | byte equality observed |
| error_select.test | 10 | 0.000831s | 0.000842s | 0.000845s | byte equality observed |
| inline_executor.test | 10 | 0.000830s | 0.000837s | 0.000853s | byte equality observed |
| inline_executor_select.test | 10 | 0.000836s | 0.000849s | 0.000854s | byte equality observed |
| inline_or_executor.test | 10 | 0.000851s | 0.000836s | 0.000844s | byte equality observed |
| inline_or_executor_select.test | 10 | 0.000835s | 0.000832s | 0.000845s | byte equality observed |
| io_context.test | 1 | 0.001403s | 0.001370s | 0.001473s | byte equality observed |
| io_context_select.test | 1 | 0.001330s | 0.001308s | 0.001337s | byte equality observed |
| io_context_strand.test | 1 | 0.001639s | 0.001613s | 0.001663s | byte equality observed |
| io_context_strand_select.test | 1 | 0.001343s | 0.001301s | 0.001309s | byte equality observed |
| ip_host_name.test | 10 | 0.000844s | 0.000839s | 0.000859s | byte equality observed |
| ip_host_name_select.test | 10 | 0.000804s | 0.000801s | 0.000823s | byte equality observed |
| ip_icmp.test | 10 | 0.000844s | 0.000835s | 0.000834s | byte equality observed |
| ip_icmp_select.test | 10 | 0.000836s | 0.000848s | 0.000832s | byte equality observed |
| ip_multicast.test | 10 | 0.000908s | 0.000922s | 0.000908s | byte equality observed |
| ip_multicast_select.test | 10 | 0.000894s | 0.000904s | 0.000887s | byte equality observed |
| ip_tcp.test | 10 | 0.001251s | 0.001254s | 0.001249s | byte equality observed |
| ip_tcp_select.test | 10 | 0.001201s | 0.001202s | 0.001189s | byte equality observed |
| ip_udp.test | 10 | 0.000937s | 0.000936s | 0.000920s | byte equality observed |
| ip_udp_select.test | 10 | 0.000925s | 0.000928s | 0.000909s | byte equality observed |
| ip_unicast.test | 10 | 0.000890s | 0.000893s | 0.000879s | byte equality observed |
| ip_unicast_select.test | 10 | 0.000877s | 0.000884s | 0.000872s | byte equality observed |
| ip_v6_only.test | 10 | 0.000889s | 0.000896s | 0.000893s | byte equality observed |
| ip_v6_only_select.test | 10 | 0.000878s | 0.000886s | 0.000888s | byte equality observed |
| is_read_buffered.test | 10 | 0.000819s | 0.000823s | 0.000823s | byte equality observed |
| is_read_buffered_select.test | 10 | 0.000818s | 0.000840s | 0.000821s | byte equality observed |
| is_write_buffered.test | 10 | 0.000821s | 0.000847s | 0.000825s | byte equality observed |
| is_write_buffered_select.test | 10 | 0.000819s | 0.000873s | 0.000825s | byte equality observed |
| prepend.test | 10 | 0.000873s | 0.000927s | 0.000893s | byte equality observed |
| prepend_select.test | 10 | 0.000885s | 0.000915s | 0.000884s | byte equality observed |
| read.test | 10 | 0.001253s | 0.001235s | 0.001232s | byte equality observed |
| read_at.test | 10 | 0.001444s | 0.001474s | 0.001364s | byte equality observed |
| read_at_select.test | 10 | 0.001465s | 0.001497s | 0.001342s | byte equality observed |
| read_select.test | 10 | 0.001257s | 0.001227s | 0.001208s | byte equality observed |
| read_until.test | 10 | 0.001038s | 0.001046s | 0.001015s | byte equality observed |
| read_until_select.test | 10 | 0.001058s | 0.001027s | 0.001020s | byte equality observed |
| recycling_allocator.test | 10 | 0.000863s | 0.000877s | 0.000858s | byte equality observed |
| recycling_allocator_select.test | 10 | 0.000854s | 0.000891s | 0.000884s | byte equality observed |
| signal_set.test | 10 | 0.000879s | 0.000906s | 0.000862s | byte equality observed |
| signal_set_select.test | 10 | 0.000876s | 0.000887s | 0.000886s | byte equality observed |
| socket_base.test | 10 | 0.000995s | 0.000969s | 0.000984s | byte equality observed |
| socket_base_select.test | 10 | 0.000962s | 0.000946s | 0.000947s | byte equality observed |
| spawn.test | 10 | 0.001062s | 0.001047s | 0.001059s | byte equality observed |
| spawn_select.test | 10 | 0.001060s | 0.001060s | 0.001077s | byte equality observed |
| static_thread_pool.test | 10 | 0.000870s | 0.000858s | 0.000865s | byte equality observed |
| static_thread_pool_select.test | 10 | 0.000846s | 0.000836s | 0.000866s | byte equality observed |
| strand.test | 1 | 0.001484s | 0.001394s | 0.001455s | byte equality observed |
| strand_select.test | 1 | 0.001288s | 0.001315s | 0.001326s | byte equality observed |
| streambuf.test | 10 | 0.000899s | 0.000876s | 0.000892s | byte equality observed |
| streambuf_select.test | 10 | 0.000890s | 0.000875s | 0.000886s | byte equality observed |
| thread_pool.test | 10 | 0.001814s | 0.001807s | 0.001806s | byte equality observed |
| thread_pool_select.test | 10 | 0.001821s | 0.001768s | 0.001783s | byte equality observed |
| write.test | 10 | 0.001306s | 0.001251s | 0.001262s | byte equality observed |
| write_at.test | 10 | 0.001369s | 0.001370s | 0.001327s | byte equality observed |
| write_at_select.test | 10 | 0.001351s | 0.001352s | 0.001333s | byte equality observed |
| write_select.test | 10 | 0.001284s | 0.001266s | 0.001236s | byte equality observed |

[Raw outcomes, commands and timing observations in CI artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43493409407).

[Comparison records](../results.json) · [Host and toolchains](../metadata.json)
