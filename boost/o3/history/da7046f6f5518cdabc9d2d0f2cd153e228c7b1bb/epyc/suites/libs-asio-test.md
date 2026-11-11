# libs/asio/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| any_completion_executor.test | RUN | 0.555984 | 2.799001 | 2.484138 | 0.002310 | 0.002147 | 0.002361 |
| any_completion_executor_select.test | RUN | 0.604347 | 2.758436 | 2.598900 | 0.002481 | 0.002100 | 0.002345 |
| any_completion_handler.test | RUN | 0.632095 | 2.545652 | 2.288791 | 0.001995 | 0.001745 | 0.001808 |
| any_completion_handler_select.test | RUN | 0.658858 | 2.652063 | 2.494253 | 0.001780 | 0.001781 | 0.001887 |
| any_io_executor.test | RUN | 0.608312 | 2.968829 | 2.780566 | 0.002130 | 0.001990 | 0.001971 |
| any_io_executor_select.test | RUN | 0.606835 | 2.934131 | 2.771713 | 0.002480 | 0.002247 | 0.002172 |
| append.test | RUN | 0.559654 | 2.423951 | 2.071866 | 0.002008 | 0.001697 | 0.001478 |
| append_select.test | RUN | 0.558370 | 2.468633 | 2.010243 | 0.001788 | 0.001511 | 0.001589 |
| as_tuple.test | RUN | 0.731868 | 2.972797 | 2.797667 | 0.001774 | 0.001666 | 0.001618 |
| as_tuple_select.test | RUN | 0.752498 | 2.982769 | 2.759027 | 0.001968 | 0.001751 | 0.001511 |
| associated_allocator.test | LINK | 0.166539 | 0.236186 | 0.299858 | — | — | — |
| associated_allocator_select.test | LINK | 0.166807 | 0.238624 | 0.297138 | — | — | — |
| associated_cancellation_slot.test | LINK | 0.175408 | 0.254362 | 0.328178 | — | — | — |
| associated_cancellation_slot_select.test | LINK | 0.173605 | 0.261521 | 0.325921 | — | — | — |
| associated_executor.test | LINK | 0.256446 | 0.447874 | 0.569354 | — | — | — |
| associated_executor_select.test | LINK | 0.257810 | 0.459514 | 0.569319 | — | — | — |
| associated_immediate_executor.test | LINK | 0.231391 | 0.416201 | 0.488595 | — | — | — |
| associated_immediate_executor_select.test | LINK | 0.232880 | 0.428186 | 0.492560 | — | — | — |
| associator.test | LINK | 0.132800 | 0.204532 | 0.239426 | — | — | — |
| associator_select.test | LINK | 0.132249 | 0.198604 | 0.239594 | — | — | — |
| awaitable.test | LINK | 0.129630 | 0.211620 | 0.284779 | — | — | — |
| awaitable_select.test | LINK | 0.128248 | 0.202446 | 0.277817 | — | — | — |
| basic_datagram_socket.test | LINK | 0.382526 | 0.640971 | 1.201200 | — | — | — |
| basic_datagram_socket_select.test | LINK | 0.397741 | 0.651291 | 1.238346 | — | — | — |
| basic_deadline_timer.test | LINK | 0.467157 | 0.774676 | 1.186853 | — | — | — |
| basic_deadline_timer_select.test | LINK | 0.483628 | 0.780828 | 1.201509 | — | — | — |
| basic_file.test | LINK | 0.133825 | 0.201986 | 0.259983 | — | — | — |
| basic_file_select.test | LINK | 0.134195 | 0.203262 | 0.259508 | — | — | — |
| basic_random_access_file.test | LINK | 0.130724 | 0.201000 | 0.250065 | — | — | — |
| basic_random_access_file_select.test | LINK | 0.131602 | 0.207831 | 0.251630 | — | — | — |
| basic_raw_socket.test | LINK | 0.397751 | 0.686953 | 0.905536 | — | — | — |
| basic_raw_socket_select.test | LINK | 0.396380 | 0.705113 | 0.926966 | — | — | — |
| basic_readable_pipe.test | LINK | 0.374908 | 0.666118 | 0.781771 | — | — | — |
| basic_readable_pipe_select.test | LINK | 0.396775 | 0.704396 | 0.956469 | — | — | — |
| basic_seq_packet_socket.test | LINK | 0.388374 | 0.638710 | 0.907977 | — | — | — |
| basic_seq_packet_socket_select.test | LINK | 0.399129 | 0.665244 | 0.927965 | — | — | — |
| basic_signal_set.test | LINK | 0.360314 | 0.588061 | 0.722125 | — | — | — |
| basic_signal_set_select.test | LINK | 0.382414 | 0.633759 | 0.886493 | — | — | — |
| basic_socket_acceptor.test | LINK | 0.388011 | 0.647030 | 0.907916 | — | — | — |
| basic_socket_acceptor_select.test | LINK | 0.402336 | 0.668417 | 0.928789 | — | — | — |
| basic_stream_file.test | LINK | 0.128466 | 0.203651 | 0.252549 | — | — | — |
| basic_stream_file_select.test | LINK | 0.134540 | 0.206823 | 0.241461 | — | — | — |
| basic_stream_socket.test | LINK | 0.393790 | 0.640850 | 0.917171 | — | — | — |
| basic_stream_socket_select.test | LINK | 0.396742 | 0.667589 | 0.973970 | — | — | — |
| basic_streambuf.test | LINK | 0.190554 | 0.308754 | 0.386348 | — | — | — |
| basic_streambuf_select.test | LINK | 0.202666 | 0.305084 | 0.383922 | — | — | — |
| basic_waitable_timer.test | LINK | 0.365621 | 0.596702 | 0.865442 | — | — | — |
| basic_waitable_timer_select.test | LINK | 0.377034 | 0.613208 | 0.870450 | — | — | — |
| basic_writable_pipe.test | LINK | 0.370684 | 0.616407 | 0.808158 | — | — | — |
| basic_writable_pipe_select.test | LINK | 0.388547 | 0.645977 | 0.960437 | — | — | — |
| bind_allocator.test | RUN | 0.597062 | 2.493241 | 2.163577 | 0.002812 | 0.001912 | 0.001732 |
| bind_allocator_select.test | RUN | 0.609149 | 2.492125 | 2.135327 | 0.002070 | 0.001810 | 0.001534 |
| bind_cancellation_slot.test | RUN | 0.617032 | 2.515363 | 2.207822 | 0.001707 | 0.001294 | 0.001252 |
| bind_cancellation_slot_select.test | RUN | 0.603659 | 2.510991 | 2.097703 | 0.001404 | 0.001262 | 0.001215 |
| bind_executor.test | RUN | 0.429533 | 1.730436 | 1.333279 | 0.002318 | 0.001846 | 0.001870 |
| bind_executor_select.test | RUN | 0.432083 | 1.751502 | 1.335994 | 0.001976 | 0.001641 | 0.001704 |
| bind_immediate_executor.test | RUN | 0.315069 | 1.420506 | 0.835785 | 0.001503 | 0.001254 | 0.001182 |
| bind_immediate_executor_select.test | RUN | 0.332760 | 1.371071 | 0.937995 | 0.001565 | 0.001255 | 0.001147 |
| buffer.test | RUN | 0.192411 | 0.565789 | 0.489863 | 0.001491 | 0.001184 | 0.001131 |
| buffer_select.test | RUN | 0.196622 | 0.574490 | 0.502167 | 0.001464 | 0.001207 | 0.001149 |
| buffer_registration.test | LINK | 0.170223 | 0.296262 | 0.301932 | — | — | — |
| buffer_registration_select.test | LINK | 0.170111 | 0.297798 | 0.299757 | — | — | — |
| buffered_read_stream.test | RUN | 0.677230 | 3.604763 | 2.879657 | 0.001930 | 0.002028 | 0.001926 |
| buffered_read_stream_select.test | RUN | 0.690542 | 3.647862 | 2.878554 | 0.001866 | 0.002071 | 0.001945 |
| buffered_stream.test | RUN | 0.919189 | 6.008595 | 5.575414 | 0.002464 | 0.002014 | 0.001904 |
| buffered_stream_select.test | RUN | 0.989399 | 6.029816 | 5.379834 | 0.002436 | 0.001792 | 0.001983 |
| buffered_write_stream.test | RUN | 1.100170 | 5.205001 | 4.719311 | 0.002270 | 0.002145 | 0.001732 |
| buffered_write_stream_select.test | RUN | 1.096649 | 5.264844 | 4.546315 | 0.002171 | 0.002156 | 0.001808 |
| buffers_iterator.test | RUN | 0.281152 | 0.448353 | 0.718549 | 0.001880 | 0.001507 | 0.001526 |
| buffers_iterator_select.test | RUN | 0.282578 | 0.474641 | 0.711588 | 0.001840 | 0.001521 | 0.001431 |
| cancel_after.test | RUN | 1.000682 | 4.003520 | 3.513856 | 0.004502 | 0.002546 | 0.002129 |
| cancel_after_select.test | RUN | 1.014779 | 4.010016 | 3.468669 | 0.003894 | 0.002601 | 0.002288 |
| cancel_at.test | RUN | 0.908819 | 3.814525 | 3.456243 | 0.003745 | 0.002456 | 0.002271 |
| cancel_at_select.test | RUN | 0.977812 | 3.696214 | 3.419467 | 0.003930 | 0.002206 | 0.002229 |
| cancellation_signal.test | LINK | 0.117870 | 0.176608 | 0.220740 | — | — | — |
| cancellation_signal_select.test | LINK | 0.127233 | 0.177773 | 0.237427 | — | — | — |
| cancellation_state.test | LINK | 0.143648 | 0.178751 | 0.318646 | — | — | — |
| cancellation_state_select.test | LINK | 0.176821 | 0.176336 | 0.309261 | — | — | — |
| cancellation_type.test | LINK | 0.132868 | 0.166321 | 0.252070 | — | — | — |
| cancellation_type_select.test | LINK | 0.133569 | 0.175081 | 0.252958 | — | — | — |
| co_composed.test | RUN | 0.136639 | 0.190896 | 0.248674 | 0.001800 | 0.001522 | 0.001475 |
| co_composed_select.test | RUN | 0.133336 | 0.205029 | 0.249512 | 0.001639 | 0.001596 | 0.001356 |
| co_spawn.test | LINK | 0.132874 | 0.208502 | 0.237616 | — | — | — |
| co_spawn_select.test | LINK | 0.130702 | 0.205263 | 0.247455 | — | — | — |
| completion_condition.test | LINK | 0.175587 | 0.307537 | 0.334397 | — | — | — |
| completion_condition_select.test | LINK | 0.166829 | 0.345498 | 0.323085 | — | — | — |
| compose.test | RUN | 0.734684 | 3.349843 | 2.738730 | 0.002653 | 0.001761 | 0.001633 |
| compose_select.test | RUN | 0.737085 | 3.282398 | 2.718911 | 0.001976 | 0.001687 | 0.001441 |
| composed.test | RUN | 0.738575 | 3.239573 | 2.655128 | 0.002090 | 0.001756 | 0.001772 |
| composed_select.test | RUN | 0.697980 | 3.046640 | 2.628605 | 0.002058 | 0.001624 | 0.001464 |
| config.test | RUN | 0.360786 | 1.949508 | 1.121761 | 0.001935 | 0.001622 | 0.001555 |
| config_select.test | RUN | 0.398349 | 2.009213 | 1.263936 | 0.002034 | 0.001381 | 0.001568 |
| connect.test | LINK | 1.217522 | 6.487620 | 5.679757 | — | — | — |
| connect_select.test | LINK | 1.213341 | 6.500835 | 5.726443 | — | — | — |
| connect_pipe.test | RUN | 0.639730 | 2.527122 | 2.119147 | 0.001876 | 0.001782 | 0.001668 |
| connect_pipe_select.test | RUN | 0.656268 | 2.629594 | 2.222875 | 0.001723 | 0.001678 | 0.001638 |
| consign.test | RUN | 0.562842 | 2.437013 | 1.939086 | 0.002016 | 0.001644 | 0.001618 |
| consign_select.test | RUN | 0.560000 | 2.464328 | 1.905551 | 0.001893 | 0.001736 | 0.001595 |
| coroutine.test | LINK | 0.134159 | 0.243260 | 0.288329 | — | — | — |
| coroutine_select.test | LINK | 0.139616 | 0.244308 | 0.275172 | — | — | — |
| deadline_timer.test | RUN | 0.827401 | 3.180249 | 2.809557 | 0.003620 | 0.002547 | 0.002976 |
| deadline_timer_select.test | RUN | 0.834341 | 3.131949 | 2.783591 | 0.003231 | 0.002382 | 0.002638 |
| deferred.test | RUN | 0.146733 | 0.216620 | 0.269431 | 0.001798 | 0.001726 | 0.001415 |
| deferred_select.test | RUN | 0.146699 | 0.223415 | 0.266362 | 0.001613 | 0.001604 | 0.001482 |
| detached.test | LINK | 0.161405 | 0.248969 | 0.298235 | — | — | — |
| detached_select.test | LINK | 0.164484 | 0.260614 | 0.293967 | — | — | — |
| disposition.test | RUN | 0.155362 | 0.599745 | 0.550943 | 0.001531 | 0.001688 | 0.001681 |
| disposition_select.test | RUN | 0.150485 | 0.589075 | 0.549033 | 0.001436 | 0.001674 | 0.001628 |
| error.test | RUN | 0.143572 | 0.564586 | 0.556862 | 0.001524 | 0.001619 | 0.001602 |
| error_select.test | RUN | 0.142620 | 0.558991 | 0.548330 | 0.001296 | 0.001673 | 0.001570 |
| file_base.test | LINK | 0.182888 | 0.478965 | 0.508152 | — | — | — |
| file_base_select.test | LINK | 0.181263 | 0.462153 | 0.426721 | — | — | — |
| fwd_first.test | LINK | 0.413682 | 0.714400 | 0.939307 | — | — | — |
| fwd_last.test | LINK | 0.410445 | 0.657364 | 0.966418 | — | — | — |
| generic_basic_endpoint.test | LINK | 0.155007 | 0.276311 | 0.338798 | — | — | — |
| generic_basic_endpoint_select.test | LINK | 0.158895 | 0.275262 | 0.340798 | — | — | — |
| generic_datagram_protocol.test | LINK | 0.583061 | 2.541530 | 1.988952 | — | — | — |
| generic_datagram_protocol_select.test | LINK | 0.585545 | 2.617200 | 1.982256 | — | — | — |
| generic_raw_protocol.test | LINK | 0.587303 | 2.533904 | 1.965599 | — | — | — |
| generic_raw_protocol_select.test | LINK | 0.632489 | 2.593416 | 1.944111 | — | — | — |
| generic_seq_packet_protocol.test | LINK | 0.520755 | 2.093120 | 1.735918 | — | — | — |
| generic_seq_packet_protocol_select.test | LINK | 0.506563 | 2.113335 | 1.685533 | — | — | — |
| generic_stream_protocol.test | LINK | 0.552017 | 2.354747 | 1.770078 | — | — | — |
| generic_stream_protocol_select.test | LINK | 0.541547 | 2.377026 | 1.790231 | — | — | — |
| high_resolution_timer.test | LINK | 0.270824 | 0.435367 | 0.580043 | — | — | — |
| high_resolution_timer_select.test | LINK | 0.273118 | 0.434041 | 0.590238 | — | — | — |
| immediate.test | LINK | 0.190310 | 0.327392 | 0.334691 | — | — | — |
| immediate_select.test | LINK | 0.189700 | 0.321080 | 0.342597 | — | — | — |
| inline_executor.test | RUN | 0.213919 | 0.504339 | 0.497822 | 0.001436 | 0.001249 | 0.001134 |
| inline_executor_select.test | RUN | 0.219426 | 0.505001 | 0.486767 | 0.001326 | 0.001272 | 0.001141 |
| inline_or_executor.test | RUN | 0.341790 | 1.508333 | 0.983030 | 0.001428 | 0.001240 | 0.001185 |
| inline_or_executor_select.test | RUN | 0.350314 | 1.530367 | 1.095635 | 0.001317 | 0.001250 | 0.001171 |
| io_context.test | RUN | 0.487133 | 2.273324 | 1.692838 | 0.002499 | 0.001818 | 0.001758 |
| io_context_select.test | RUN | 0.490933 | 2.227362 | 1.698513 | 0.002104 | 0.001809 | 0.001882 |
| io_context_strand.test | RUN | 0.457299 | 1.934935 | 1.432957 | 0.002291 | 0.001905 | 0.001690 |
| io_context_strand_select.test | RUN | 0.457754 | 1.881437 | 1.459735 | 0.002015 | 0.001777 | 0.001770 |
| ip_address.test | LINK | 0.180223 | 0.565009 | 0.415996 | — | — | — |
| ip_address_select.test | LINK | 0.177767 | 0.539308 | 0.425244 | — | — | — |
| ip_address_v4.test | LINK | 0.184703 | 0.673954 | 0.482149 | — | — | — |
| ip_address_v4_select.test | LINK | 0.182887 | 0.683596 | 0.469119 | — | — | — |
| ip_address_v6.test | LINK | 0.200555 | 0.891890 | 0.546602 | — | — | — |
| ip_address_v6_select.test | LINK | 0.200573 | 0.892194 | 0.538781 | — | — | — |
| ip_basic_endpoint.test | LINK | 0.165197 | 0.426381 | 0.362891 | — | — | — |
| ip_basic_endpoint_select.test | LINK | 0.163813 | 0.435556 | 0.356495 | — | — | — |
| ip_basic_resolver.test | LINK | 0.258217 | 0.608732 | 0.691693 | — | — | — |
| ip_basic_resolver_select.test | LINK | 0.260680 | 0.646722 | 0.797708 | — | — | — |
| ip_basic_resolver_entry.test | LINK | 0.091327 | 0.203933 | 0.251558 | — | — | — |
| ip_basic_resolver_entry_select.test | LINK | 0.091354 | 0.204254 | 0.254629 | — | — | — |
| ip_basic_resolver_iterator.test | LINK | 0.162107 | 0.460575 | 0.562496 | — | — | — |
| ip_basic_resolver_iterator_select.test | LINK | 0.164408 | 0.461445 | 0.556856 | — | — | — |
| ip_basic_resolver_query.test | LINK | 0.155180 | 0.417512 | 0.554359 | — | — | — |
| ip_basic_resolver_query_select.test | LINK | 0.155228 | 0.406154 | 0.534412 | — | — | — |
| ip_host_name.test | RUN | 0.162867 | 0.521153 | 0.617910 | 0.001366 | 0.001272 | 0.001519 |
| ip_host_name_select.test | RUN | 0.163094 | 0.517352 | 0.605355 | 0.001366 | 0.001243 | 0.001336 |
| ip_icmp.test | RUN | 0.716652 | 5.205305 | 4.144244 | 0.001433 | 0.001644 | 0.001519 |
| ip_icmp_select.test | RUN | 0.715914 | 5.356002 | 4.058419 | 0.001425 | 0.001724 | 0.001534 |
| ip_multicast.test | RUN | 0.432197 | 2.565349 | 2.268702 | 0.001644 | 0.001567 | 0.001794 |
| ip_multicast_select.test | RUN | 0.437573 | 2.535237 | 2.262434 | 0.001452 | 0.001761 | 0.001383 |
| ip_resolver_query_base.test | LINK | 0.104871 | 0.223354 | 0.253627 | — | — | — |
| ip_resolver_query_base_select.test | LINK | 0.103837 | 0.224593 | 0.266405 | — | — | — |
| ip_tcp.test | RUN | 1.048398 | 7.609705 | 6.524051 | 0.001762 | 0.002270 | 0.002278 |
| ip_tcp_select.test | RUN | 1.039055 | 7.954479 | 6.301947 | 0.001844 | 0.002386 | 0.002164 |
| ip_udp.test | RUN | 0.701897 | 5.492523 | 4.389123 | 0.001497 | 0.001863 | 0.001727 |
| ip_udp_select.test | RUN | 0.700444 | 5.612067 | 4.225304 | 0.001358 | 0.001733 | 0.001454 |
| ip_unicast.test | RUN | 0.370490 | 2.355053 | 2.026101 | 0.001508 | 0.001803 | 0.001410 |
| ip_unicast_select.test | RUN | 0.374948 | 2.354010 | 1.974940 | 0.001282 | 0.001664 | 0.001631 |
| ip_v6_only.test | RUN | 0.378833 | 2.423631 | 2.111981 | 0.001179 | 0.001718 | 0.001688 |
| ip_v6_only_select.test | RUN | 0.372138 | 2.532965 | 2.059743 | 0.001340 | 0.001403 | 0.001609 |
| is_read_buffered.test | RUN | 0.265460 | 0.731866 | 1.055746 | 0.001248 | 0.001522 | 0.001409 |
| is_read_buffered_select.test | RUN | 0.266267 | 0.737095 | 1.074566 | 0.001253 | 0.001463 | 0.001569 |
| is_write_buffered.test | RUN | 0.262260 | 0.737851 | 1.049183 | 0.001221 | 0.001474 | 0.001381 |
| is_write_buffered_select.test | RUN | 0.269886 | 0.739850 | 1.087783 | 0.001226 | 0.001421 | 0.001449 |
| local_basic_endpoint.test | LINK | 0.137786 | 0.406715 | 0.532236 | — | — | — |
| local_basic_endpoint_select.test | LINK | 0.136168 | 0.400581 | 0.527171 | — | — | — |
| local_connect_pair.test | LINK | 0.339294 | 2.374006 | 1.900412 | — | — | — |
| local_connect_pair_select.test | LINK | 0.336838 | 2.377483 | 1.889503 | — | — | — |
| local_datagram_protocol.test | LINK | 0.439077 | 3.149155 | 3.083672 | — | — | — |
| local_datagram_protocol_select.test | LINK | 0.451720 | 3.171332 | 3.025180 | — | — | — |
| local_seq_packet_protocol.test | LINK | 0.449038 | 3.087630 | 3.198637 | — | — | — |
| local_seq_packet_protocol_select.test | LINK | 0.449966 | 3.112541 | 3.079908 | — | — | — |
| local_stream_protocol.test | LINK | 0.429604 | 2.955277 | 2.637309 | — | — | — |
| local_stream_protocol_select.test | LINK | 0.423551 | 2.954766 | 2.582146 | — | — | — |
| placeholders.test | LINK | 0.087691 | 0.228038 | 0.270051 | — | — | — |
| placeholders_select.test | LINK | 0.087697 | 0.225311 | 0.265214 | — | — | — |
| posix_basic_descriptor.test | LINK | 0.291906 | 0.611726 | 0.802582 | — | — | — |
| posix_basic_descriptor_select.test | LINK | 0.297530 | 0.651425 | 0.959390 | — | — | — |
| posix_basic_stream_descriptor.test | LINK | 0.343375 | 0.606087 | 0.783394 | — | — | — |
| posix_basic_stream_descriptor_select.test | LINK | 0.353247 | 0.639485 | 0.946371 | — | — | — |
| posix_descriptor_base.test | LINK | 0.140301 | 0.230881 | 0.258034 | — | — | — |
| posix_descriptor_base_select.test | LINK | 0.143930 | 0.238238 | 0.248121 | — | — | — |
| posix_stream_descriptor.test | LINK | 0.624919 | 2.983785 | 2.546293 | — | — | — |
| posix_stream_descriptor_select.test | LINK | 0.638940 | 2.965492 | 2.560487 | — | — | — |
| prepend.test | RUN | 0.517954 | 2.388668 | 1.995858 | 0.001292 | 0.001763 | 0.001611 |
| prepend_select.test | RUN | 0.514910 | 2.467089 | 1.998985 | 0.001595 | 0.001606 | 0.001343 |
| random_access_file.test | LINK | 0.298884 | 0.558253 | 0.629720 | — | — | — |
| random_access_file_select.test | LINK | 0.312596 | 0.612853 | 0.803221 | — | — | — |
| read.test | RUN | 1.448704 | 14.749904 | 11.236715 | 0.002262 | 0.002390 | 0.002194 |
| read_select.test | RUN | 1.462252 | 14.755679 | 11.405142 | 0.002174 | 0.002242 | 0.002232 |
| read_at.test | RUN | 1.465680 | 21.044126 | 15.426730 | 0.002809 | 0.002511 | 0.002345 |
| read_at_select.test | RUN | 1.477630 | 20.799432 | 15.519411 | 0.002511 | 0.002286 | 0.002238 |
| read_until.test | RUN | 0.828771 | 5.623297 | 4.201799 | 0.001962 | 0.001935 | 0.001824 |
| read_until_select.test | RUN | 0.844271 | 5.673518 | 4.330425 | 0.001747 | 0.001974 | 0.001774 |
| readable_pipe.test | LINK | 0.517856 | 2.397334 | 1.966346 | — | — | — |
| readable_pipe_select.test | LINK | 0.527320 | 2.501568 | 2.078452 | — | — | — |
| recycling_allocator.test | RUN | 0.169457 | 0.367029 | 0.399205 | 0.001624 | 0.001466 | 0.001533 |
| recycling_allocator_select.test | RUN | 0.166387 | 0.364052 | 0.392907 | 0.001447 | 0.001400 | 0.001545 |
| redirect_disposition.test | LINK | 0.696116 | 3.341060 | 3.008034 | — | — | — |
| redirect_disposition_select.test | LINK | 0.703167 | 3.334146 | 2.952827 | — | — | — |
| redirect_error.test | LINK | 0.626763 | 2.847689 | 2.606344 | — | — | — |
| redirect_error_select.test | LINK | 0.624092 | 2.806783 | 2.641376 | — | — | — |
| registered_buffer.test | LINK | 0.168932 | 0.292274 | 0.344414 | — | — | — |
| registered_buffer_select.test | LINK | 0.169608 | 0.289405 | 0.345723 | — | — | — |
| signal_set.test | RUN | 0.505646 | 2.367859 | 1.821651 | 0.001586 | 0.001595 | 0.001569 |
| signal_set_select.test | RUN | 0.520886 | 2.440083 | 2.018651 | 0.001471 | 0.001551 | 0.001557 |
| signal_set_base.test | LINK | 0.135762 | 0.232100 | 0.265715 | — | — | — |
| signal_set_base_select.test | LINK | 0.136728 | 0.231949 | 0.257781 | — | — | — |
| socket_base.test | RUN | 0.617803 | 3.514588 | 2.768996 | 0.001980 | 0.001608 | 0.001754 |
| socket_base_select.test | RUN | 0.628669 | 3.412748 | 2.778751 | 0.001746 | 0.001681 | 0.001850 |
| spawn.test | RUN | 1.072491 | 5.793860 | 5.038431 | 0.001911 | 0.001932 | 0.001882 |
| spawn_select.test | RUN | 1.074007 | 5.754834 | 5.080700 | 0.001726 | 0.001673 | 0.001851 |
| static_thread_pool.test | RUN | 0.285783 | 0.550869 | 0.691255 | 0.001667 | 0.001372 | 0.001554 |
| static_thread_pool_select.test | RUN | 0.298655 | 0.594015 | 0.929607 | 0.001488 | 0.001463 | 0.001645 |
| steady_timer.test | LINK | 0.323190 | 0.635155 | 0.985016 | — | — | — |
| steady_timer_select.test | LINK | 0.328169 | 0.648934 | 0.943858 | — | — | — |
| strand.test | RUN | 0.675768 | 3.235688 | 2.888869 | 0.002375 | 0.002361 | 0.002199 |
| strand_select.test | RUN | 0.687997 | 3.205996 | 2.730772 | 0.001968 | 0.002129 | 0.002157 |
| stream_file.test | LINK | 0.301327 | 0.565548 | 0.660801 | — | — | — |
| stream_file_select.test | LINK | 0.312233 | 0.596366 | 0.846155 | — | — | — |
| streambuf.test | RUN | 0.189472 | 0.319124 | 0.477978 | 0.001193 | 0.001206 | 0.001544 |
| streambuf_select.test | RUN | 0.165807 | 0.308825 | 0.438599 | 0.001268 | 0.001124 | 0.001306 |
| system_timer.test | LINK | 0.431534 | 2.113355 | 1.853165 | — | — | — |
| system_timer_select.test | LINK | 0.434183 | 2.081969 | 1.707699 | — | — | — |
| system_context.test | LINK | 0.203809 | 0.369664 | 0.403701 | — | — | — |
| system_context_select.test | LINK | 0.212338 | 0.392084 | 0.493588 | — | — | — |
| system_executor.test | LINK | 0.261075 | 1.201305 | 0.792262 | — | — | — |
| system_executor_select.test | LINK | 0.270340 | 1.168304 | 0.871740 | — | — | — |
| this_coro.test | LINK | 0.083588 | 0.139591 | 0.152769 | — | — | — |
| this_coro_select.test | LINK | 0.083527 | 0.138657 | 0.157716 | — | — | — |
| thread_pool.test | RUN | 0.310794 | 1.539141 | 1.028413 | 0.002262 | 0.002071 | 0.002188 |
| thread_pool_select.test | RUN | 0.323476 | 1.518900 | 1.129781 | 0.002239 | 0.002293 | 0.002112 |
| time_traits.test | LINK | 0.184270 | 0.289893 | 0.314803 | — | — | — |
| time_traits_select.test | LINK | 0.185804 | 0.293133 | 0.312026 | — | — | — |
| ts_buffer.test | LINK | 0.174372 | 0.324187 | 0.354459 | — | — | — |
| ts_buffer_select.test | LINK | 0.175190 | 0.322959 | 0.347540 | — | — | — |
| ts_executor.test | LINK | 0.243396 | 0.443044 | 0.559815 | — | — | — |
| ts_executor_select.test | LINK | 0.253787 | 0.473085 | 0.673415 | — | — | — |
| ts_internet.test | LINK | 0.263741 | 0.604494 | 0.691761 | — | — | — |
| ts_internet_select.test | LINK | 0.270093 | 0.666025 | 0.693194 | — | — | — |
| ts_io_context.test | LINK | 0.210261 | 0.563729 | 0.424233 | — | — | — |
| ts_io_context_select.test | LINK | 0.215697 | 0.593068 | 0.709438 | — | — | — |
| ts_net.test | LINK | 0.306755 | 0.851641 | 1.162415 | — | — | — |
| ts_net_select.test | LINK | 0.315250 | 0.868916 | 1.383495 | — | — | — |
| ts_netfwd.test | LINK | 0.309750 | 0.858679 | 1.353237 | — | — | — |
| ts_netfwd_select.test | LINK | 0.323156 | 0.882332 | 1.408508 | — | — | — |
| ts_socket.test | LINK | 0.255195 | 0.674832 | 1.015723 | — | — | — |
| ts_socket_select.test | LINK | 0.259367 | 0.686440 | 1.068490 | — | — | — |
| ts_timer.test | LINK | 0.226754 | 0.604761 | 0.893857 | — | — | — |
| ts_timer_select.test | LINK | 0.233370 | 0.632961 | 0.936504 | — | — | — |
| use_awaitable.test | LINK | 0.085370 | 0.203160 | 0.261441 | — | — | — |
| use_awaitable_select.test | LINK | 0.082070 | 0.202961 | 0.248249 | — | — | — |
| wait_traits.test | LINK | 0.080256 | 0.204696 | 0.263427 | — | — | — |
| wait_traits_select.test | LINK | 0.080065 | 0.201687 | 0.245454 | — | — | — |
| windows_basic_object_handle.test | LINK | 0.079345 | 0.201966 | 0.258712 | — | — | — |
| windows_basic_object_handle_select.test | LINK | 0.079200 | 0.198973 | 0.258647 | — | — | — |
| windows_basic_overlapped_handle.test | LINK | 0.080333 | 0.200694 | 0.273471 | — | — | — |
| windows_basic_overlapped_handle_select.test | LINK | 0.078774 | 0.205248 | 0.263281 | — | — | — |
| windows_basic_random_access_handle.test | LINK | 0.081292 | 0.202905 | 0.279150 | — | — | — |
| windows_basic_random_access_handle_select.test | LINK | 0.087904 | 0.203856 | 0.289124 | — | — | — |
| windows_basic_stream_handle.test | LINK | 0.081660 | 0.204253 | 0.299701 | — | — | — |
| windows_basic_stream_handle_select.test | LINK | 0.078941 | 0.203213 | 0.310100 | — | — | — |
| windows_object_handle.test | LINK | 0.204520 | 0.552134 | 0.713507 | — | — | — |
| windows_object_handle_select.test | LINK | 0.214314 | 0.583160 | 0.918287 | — | — | — |
| windows_overlapped_ptr.test | LINK | 0.212837 | 0.592597 | 0.804241 | — | — | — |
| windows_overlapped_ptr_select.test | LINK | 0.220611 | 0.629915 | 0.997614 | — | — | — |
| windows_random_access_handle.test | LINK | 0.207659 | 0.553893 | 0.757447 | — | — | — |
| windows_random_access_handle_select.test | LINK | 0.215000 | 0.582818 | 0.920929 | — | — | — |
| windows_stream_handle.test | LINK | 0.206612 | 0.565341 | 0.770646 | — | — | — |
| windows_stream_handle_select.test | LINK | 0.213459 | 0.598455 | 0.957673 | — | — | — |
| writable_pipe.test | LINK | 0.395532 | 2.421118 | 2.312923 | — | — | — |
| writable_pipe_select.test | LINK | 0.481766 | 2.565950 | 2.471018 | — | — | — |
| write.test | RUN | 1.471735 | 12.076469 | 12.341938 | 0.002366 | 0.002197 | 0.002262 |
| write_select.test | RUN | 1.470308 | 12.181182 | 12.583340 | 0.002192 | 0.002170 | 0.002264 |
| write_at.test | RUN | 1.516388 | 16.918482 | 15.726792 | 0.002627 | 0.002231 | 0.002388 |
| write_at_select.test | RUN | 1.527381 | 16.795832 | 15.761247 | 0.003034 | 0.002299 | 0.002554 |
| any_executor.test | RUN | 0.616525 | 5.638573 | 4.710150 | 0.002484 | 0.002172 | 0.002356 |
| blocking.test | RUN | 1.355741 | 7.317922 | 9.664562 | 0.006124 | 0.006036 | 0.006424 |
| blocking_adaptation.test | RUN | 0.693000 | 3.159721 | 4.469026 | 0.003150 | 0.003348 | 0.003456 |
| context_as.test | RUN | 0.428973 | 2.165093 | 1.502104 | 0.001360 | 0.001820 | 0.001887 |
| executor.test | RUN | 0.119571 | 0.203075 | 0.278036 | 0.001594 | 0.001598 | 0.001618 |
| inline_exception_handling.test | RUN | 1.304499 | 6.486999 | 9.708646 | 0.005178 | 0.004282 | 0.004552 |
| invocable_archetype.test | RUN | 0.111783 | 0.205824 | 0.265527 | 0.001540 | 0.001542 | 0.001738 |
| mapping.test | RUN | 1.240466 | 6.757937 | 8.917463 | 0.004715 | 0.004337 | 0.004678 |
| outstanding_work.test | RUN | 0.602817 | 2.813206 | 4.134774 | 0.003493 | 0.003450 | 0.003421 |
| prefer_only.test | RUN | 0.232206 | 1.103111 | 0.928573 | 0.001669 | 0.001642 | 0.001463 |
| relationship.test | RUN | 0.584839 | 2.392311 | 4.234043 | 0.002521 | 0.002482 | 0.003496 |

## Native runtime replay

Child CPU of successful native tests, with process startup included; GDB and expected-failure executions are correctness-only. Two warmups per compiler, four A/A observations per reference and mirrored ABBA/BAAB blocks. Short tests repeat up to ten times, identically across compilers; CPU is divided by repetitions before summing tests. Executable hashes and output checks stay outside process timers. All runs must pass their original native oracle. Output hashes record equality or variation across the complete campaign; test reports may print times or addresses.

| Reference | CPU speedup ↑ | Paired blocks | A/A speedup |
|---|---:|---|---:|
| gcc | 0.988× | 0.966×, 1.011× | 1.013× |
| clang | 1.002× | 1.023×, 0.981× | 1.003× |

† A/A drift or paired-block spread exceeds 5%; small differences are unresolved. 1375 native tests in the full cohort; 1361 have byte-identical output throughout. [Manifest](https://github.com/vishvananda/cppgm-extended/actions/runs/42695893613), [paired observations](https://github.com/vishvananda/cppgm-extended/actions/runs/42695893613) and [per-test summaries](https://github.com/vishvananda/cppgm-extended/actions/runs/42695893613).

| Test | Repeats | CPU cppgm | GCC | Clang | Output check |
|---|---:|---:|---:|---:|---|
| any_completion_executor.test | 4 | 0.001528s | 0.001529s | 0.001645s | byte equality observed |
| any_completion_executor_select.test | 7 | 0.001507s | 0.001470s | 0.001602s | byte equality observed |
| any_completion_handler.test | 10 | 0.001231s | 0.001207s | 0.001293s | byte equality observed |
| any_completion_handler_select.test | 9 | 0.001222s | 0.001225s | 0.001286s | byte equality observed |
| any_io_executor.test | 7 | 0.001517s | 0.001491s | 0.001618s | byte equality observed |
| any_io_executor_select.test | 5 | 0.001509s | 0.001508s | 0.001606s | byte equality observed |
| append.test | 10 | 0.001165s | 0.001160s | 0.001265s | byte equality observed |
| append_select.test | 10 | 0.001141s | 0.001130s | 0.001213s | byte equality observed |
| as_tuple.test | 10 | 0.001246s | 0.001259s | 0.001297s | byte equality observed |
| as_tuple_select.test | 9 | 0.001177s | 0.001199s | 0.001244s | byte equality observed |
| bind_allocator.test | 1 | 0.001625s | 0.001603s | 0.001729s | byte equality observed |
| bind_allocator_select.test | 1 | 0.001362s | 0.001318s | 0.001446s | byte equality observed |
| bind_cancellation_slot.test | 10 | 0.001124s | 0.001141s | 0.001263s | byte equality observed |
| bind_cancellation_slot_select.test | 10 | 0.001013s | 0.001082s | 0.001105s | byte equality observed |
| bind_executor.test | 1 | 0.001688s | 0.001674s | 0.001770s | byte equality observed |
| bind_executor_select.test | 1 | 0.001937s | 0.001956s | 0.002064s | byte equality observed |
| bind_immediate_executor.test | 10 | 0.001081s | 0.001074s | 0.001066s | byte equality observed |
| bind_immediate_executor_select.test | 10 | 0.001053s | 0.001086s | 0.001017s | byte equality observed |
| buffer.test | 10 | 0.001021s | 0.001017s | 0.000991s | byte equality observed |
| buffer_select.test | 10 | 0.001007s | 0.000998s | 0.000982s | byte equality observed |
| buffered_read_stream.test | 10 | 0.001313s | 0.001276s | 0.001305s | byte equality observed |
| buffered_read_stream_select.test | 10 | 0.001287s | 0.001283s | 0.001244s | byte equality observed |
| buffered_stream.test | 10 | 0.001308s | 0.001265s | 0.001284s | byte equality observed |
| buffered_stream_select.test | 10 | 0.001290s | 0.001271s | 0.001243s | byte equality observed |
| buffered_write_stream.test | 10 | 0.001283s | 0.001275s | 0.001292s | byte equality observed |
| buffered_write_stream_select.test | 10 | 0.001268s | 0.001266s | 0.001269s | byte equality observed |
| buffers_iterator.test | 10 | 0.000981s | 0.000984s | 0.000978s | byte equality observed |
| buffers_iterator_select.test | 10 | 0.000965s | 0.000948s | 0.000975s | byte equality observed |
| cancel_after.test | 1 | 0.003363s | 0.002845s | 0.003722s | byte equality observed |
| cancel_after_select.test | 1 | 0.002816s | 0.002266s | 0.003525s | byte equality observed |
| cancel_at.test | 1 | 0.003826s | 0.003311s | 0.004187s | byte equality observed |
| cancel_at_select.test | 1 | 0.003623s | 0.003085s | 0.003938s | byte equality observed |
| co_composed.test | 4 | 0.001045s | 0.001048s | 0.001281s | byte equality observed |
| co_composed_select.test | 6 | 0.000939s | 0.000942s | 0.001022s | byte equality observed |
| compose.test | 1 | 0.001667s | 0.001556s | 0.001846s | byte equality observed |
| compose_select.test | 1 | 0.001431s | 0.001322s | 0.001759s | byte equality observed |
| composed.test | 1 | 0.002005s | 0.001965s | 0.002192s | byte equality observed |
| composed_select.test | 1 | 0.001511s | 0.001393s | 0.001951s | byte equality observed |
| config.test | 10 | 0.001062s | 0.001077s | 0.001076s | byte equality observed |
| config_select.test | 10 | 0.001034s | 0.001031s | 0.001061s | byte equality observed |
| connect_pipe.test | 10 | 0.001044s | 0.001074s | 0.001090s | byte equality observed |
| connect_pipe_select.test | 10 | 0.001041s | 0.001070s | 0.001073s | byte equality observed |
| consign.test | 10 | 0.001060s | 0.001028s | 0.001059s | byte equality observed |
| consign_select.test | 10 | 0.001014s | 0.000999s | 0.001016s | byte equality observed |
| deadline_timer.test | 1 | 0.002221s | 0.002309s | 0.002530s | byte equality observed |
| deadline_timer_select.test | 1 | 0.002115s | 0.001985s | 0.002288s | byte equality observed |
| deferred.test | 10 | 0.000953s | 0.000964s | 0.001041s | byte equality observed |
| deferred_select.test | 10 | 0.000919s | 0.000910s | 0.000942s | byte equality observed |
| disposition.test | 10 | 0.000973s | 0.000985s | 0.000999s | byte equality observed |
| disposition_select.test | 10 | 0.000970s | 0.000972s | 0.000987s | byte equality observed |
| error.test | 10 | 0.000942s | 0.000949s | 0.000992s | byte equality observed |
| error_select.test | 10 | 0.000948s | 0.000947s | 0.000996s | byte equality observed |
| inline_executor.test | 10 | 0.000956s | 0.000958s | 0.001011s | byte equality observed |
| inline_executor_select.test | 10 | 0.000959s | 0.000956s | 0.000998s | byte equality observed |
| inline_or_executor.test | 10 | 0.000966s | 0.000937s | 0.000984s | byte equality observed |
| inline_or_executor_select.test | 10 | 0.000963s | 0.000934s | 0.000981s | byte equality observed |
| io_context.test | 1 | 0.001886s | 0.001817s | 0.002120s | byte equality observed |
| io_context_select.test | 1 | 0.002191s | 0.002052s | 0.002399s | byte equality observed |
| io_context_strand.test | 1 | 0.002320s | 0.002159s | 0.002417s | byte equality observed |
| io_context_strand_select.test | 1 | 0.001993s | 0.001733s | 0.002191s | byte equality observed |
| ip_host_name.test | 10 | 0.000977s | 0.000941s | 0.001001s | byte equality observed |
| ip_host_name_select.test | 10 | 0.000925s | 0.000899s | 0.000947s | byte equality observed |
| ip_icmp.test | 10 | 0.000958s | 0.000916s | 0.000953s | byte equality observed |
| ip_icmp_select.test | 10 | 0.000957s | 0.000901s | 0.000951s | byte equality observed |
| ip_multicast.test | 10 | 0.001029s | 0.000971s | 0.001064s | byte equality observed |
| ip_multicast_select.test | 10 | 0.001002s | 0.000947s | 0.001075s | byte equality observed |
| ip_tcp.test | 10 | 0.001412s | 0.001313s | 0.001504s | byte equality observed |
| ip_tcp_select.test | 10 | 0.001356s | 0.001261s | 0.001455s | byte equality observed |
| ip_udp.test | 10 | 0.001057s | 0.000992s | 0.001120s | byte equality observed |
| ip_udp_select.test | 10 | 0.001057s | 0.001000s | 0.001071s | byte equality observed |
| ip_unicast.test | 10 | 0.001010s | 0.000984s | 0.001033s | byte equality observed |
| ip_unicast_select.test | 10 | 0.000984s | 0.000955s | 0.001027s | byte equality observed |
| ip_v6_only.test | 10 | 0.000994s | 0.000968s | 0.001051s | byte equality observed |
| ip_v6_only_select.test | 10 | 0.000979s | 0.000987s | 0.001022s | byte equality observed |
| is_read_buffered.test | 10 | 0.000903s | 0.000928s | 0.000932s | byte equality observed |
| is_read_buffered_select.test | 10 | 0.000901s | 0.000958s | 0.000952s | byte equality observed |
| is_write_buffered.test | 10 | 0.000916s | 0.000939s | 0.001018s | byte equality observed |
| is_write_buffered_select.test | 10 | 0.000927s | 0.000943s | 0.001018s | byte equality observed |
| prepend.test | 10 | 0.000990s | 0.001000s | 0.001106s | byte equality observed |
| prepend_select.test | 10 | 0.000964s | 0.001002s | 0.001077s | byte equality observed |
| read.test | 10 | 0.001415s | 0.001333s | 0.001453s | byte equality observed |
| read_at.test | 10 | 0.001740s | 0.001602s | 0.001609s | byte equality observed |
| read_at_select.test | 10 | 0.001735s | 0.001575s | 0.001602s | byte equality observed |
| read_select.test | 10 | 0.001477s | 0.001305s | 0.001482s | byte equality observed |
| read_until.test | 10 | 0.001163s | 0.001092s | 0.001237s | byte equality observed |
| read_until_select.test | 10 | 0.001171s | 0.001110s | 0.001194s | byte equality observed |
| recycling_allocator.test | 10 | 0.000962s | 0.000924s | 0.001023s | byte equality observed |
| recycling_allocator_select.test | 10 | 0.000975s | 0.000948s | 0.001055s | byte equality observed |
| signal_set.test | 10 | 0.001010s | 0.000964s | 0.001059s | byte equality observed |
| signal_set_select.test | 10 | 0.000993s | 0.000956s | 0.001053s | byte equality observed |
| socket_base.test | 10 | 0.001113s | 0.001061s | 0.001174s | byte equality observed |
| socket_base_select.test | 10 | 0.001063s | 0.001037s | 0.001147s | byte equality observed |
| spawn.test | 10 | 0.001180s | 0.001108s | 0.001277s | byte equality observed |
| spawn_select.test | 10 | 0.001222s | 0.001130s | 0.001246s | byte equality observed |
| static_thread_pool.test | 10 | 0.000982s | 0.000917s | 0.001046s | byte equality observed |
| static_thread_pool_select.test | 10 | 0.000949s | 0.000884s | 0.000995s | byte equality observed |
| strand.test | 1 | 0.001870s | 0.001754s | 0.002083s | byte equality observed |
| strand_select.test | 1 | 0.001579s | 0.001450s | 0.001808s | byte equality observed |
| streambuf.test | 10 | 0.001015s | 0.001006s | 0.001088s | byte equality observed |
| streambuf_select.test | 10 | 0.000989s | 0.000974s | 0.001040s | byte equality observed |
| thread_pool.test | 10 | 0.002054s | 0.001963s | 0.002242s | byte equality observed |
| thread_pool_select.test | 10 | 0.002013s | 0.001957s | 0.002185s | byte equality observed |
| write.test | 10 | 0.001516s | 0.001384s | 0.001526s | byte equality observed |
| write_at.test | 10 | 0.001678s | 0.001488s | 0.001580s | byte equality observed |
| write_at_select.test | 10 | 0.001617s | 0.001488s | 0.001600s | byte equality observed |
| write_select.test | 10 | 0.001524s | 0.001393s | 0.001580s | byte equality observed |

Compiler command and timing observations: [cppgm](https://github.com/vishvananda/cppgm-extended/actions/runs/42695893613) · [gcc](https://github.com/vishvananda/cppgm-extended/actions/runs/42695893613) · [clang](https://github.com/vishvananda/cppgm-extended/actions/runs/42695893613).

[Reviewed outcomes and per-test records](../results.json) · [Host and toolchains](../metadata.json)
