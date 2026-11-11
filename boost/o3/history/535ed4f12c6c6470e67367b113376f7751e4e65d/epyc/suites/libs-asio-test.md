# libs/asio/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| any_completion_executor.test | RUN | 0.650082 | 3.747800 | 3.517248 | 0.002049 | 0.002368 | 0.002509 |
| any_completion_executor_select.test | RUN | 0.708444 | 3.745451 | 3.803942 | 0.002386 | 0.002594 | 0.002285 |
| any_completion_handler.test | RUN | 0.742855 | 3.371054 | 3.263956 | 0.002104 | 0.001877 | 0.001717 |
| any_completion_handler_select.test | RUN | 0.754291 | 3.497047 | 3.505138 | 0.001627 | 0.002155 | 0.001936 |
| any_io_executor.test | RUN | 0.711489 | 3.993005 | 3.922880 | 0.002312 | 0.002508 | 0.002681 |
| any_io_executor_select.test | RUN | 0.725179 | 3.952596 | 4.160123 | 0.002428 | 0.002474 | 0.002479 |
| append.test | RUN | 0.652394 | 3.169997 | 2.984057 | 0.002024 | 0.002080 | 0.002108 |
| append_select.test | RUN | 0.668957 | 3.254833 | 2.871549 | 0.001784 | 0.001832 | 0.002074 |
| as_tuple.test | RUN | 0.858130 | 3.902340 | 4.066760 | 0.001870 | 0.002112 | 0.002025 |
| as_tuple_select.test | RUN | 0.861045 | 3.877773 | 3.976481 | 0.001738 | 0.002008 | 0.001900 |
| associated_allocator.test | LINK | 0.191526 | 0.327545 | 0.483720 | — | — | — |
| associated_allocator_select.test | LINK | 0.197274 | 0.334200 | 0.483165 | — | — | — |
| associated_cancellation_slot.test | LINK | 0.204207 | 0.360232 | 0.510584 | — | — | — |
| associated_cancellation_slot_select.test | LINK | 0.205304 | 0.365909 | 0.537475 | — | — | — |
| associated_executor.test | LINK | 0.302065 | 0.620686 | 0.839336 | — | — | — |
| associated_executor_select.test | LINK | 0.294673 | 0.629872 | 0.801907 | — | — | — |
| associated_immediate_executor.test | LINK | 0.272550 | 0.577224 | 0.754839 | — | — | — |
| associated_immediate_executor_select.test | LINK | 0.264841 | 0.587579 | 0.748496 | — | — | — |
| associator.test | LINK | 0.158950 | 0.279349 | 0.403365 | — | — | — |
| associator_select.test | LINK | 0.146996 | 0.279271 | 0.371607 | — | — | — |
| awaitable.test | LINK | 0.147012 | 0.287917 | 0.393138 | — | — | — |
| awaitable_select.test | LINK | 0.151446 | 0.276671 | 0.396302 | — | — | — |
| basic_datagram_socket.test | LINK | 0.458021 | 0.902427 | 1.487158 | — | — | — |
| basic_datagram_socket_select.test | LINK | 0.472717 | 0.921651 | 1.519006 | — | — | — |
| basic_deadline_timer.test | LINK | 0.561420 | 1.035365 | 1.705828 | — | — | — |
| basic_deadline_timer_select.test | LINK | 0.556526 | 1.055186 | 1.730008 | — | — | — |
| basic_file.test | LINK | 0.157430 | 0.293433 | 0.380908 | — | — | — |
| basic_file_select.test | LINK | 0.152635 | 0.281981 | 0.381723 | — | — | — |
| basic_random_access_file.test | LINK | 0.151269 | 0.274960 | 0.391776 | — | — | — |
| basic_random_access_file_select.test | LINK | 0.149337 | 0.270826 | 0.392488 | — | — | — |
| basic_raw_socket.test | LINK | 0.452495 | 0.892384 | 1.474528 | — | — | — |
| basic_raw_socket_select.test | LINK | 0.471676 | 0.913280 | 1.503965 | — | — | — |
| basic_readable_pipe.test | LINK | 0.432336 | 0.847093 | 1.281784 | — | — | — |
| basic_readable_pipe_select.test | LINK | 0.459979 | 0.894991 | 1.525738 | — | — | — |
| basic_seq_packet_socket.test | LINK | 0.459527 | 0.883548 | 1.470885 | — | — | — |
| basic_seq_packet_socket_select.test | LINK | 0.471162 | 0.908279 | 1.510648 | — | — | — |
| basic_signal_set.test | LINK | 0.431276 | 0.830029 | 1.110801 | — | — | — |
| basic_signal_set_select.test | LINK | 0.437250 | 0.868324 | 1.389534 | — | — | — |
| basic_socket_acceptor.test | LINK | 0.462703 | 0.884769 | 1.505246 | — | — | — |
| basic_socket_acceptor_select.test | LINK | 0.476277 | 0.895288 | 1.515650 | — | — | — |
| basic_stream_file.test | LINK | 0.150041 | 0.292047 | 0.369639 | — | — | — |
| basic_stream_file_select.test | LINK | 0.150046 | 0.291253 | 0.373089 | — | — | — |
| basic_stream_socket.test | LINK | 0.452627 | 0.877606 | 1.441700 | — | — | — |
| basic_stream_socket_select.test | LINK | 0.460210 | 0.902191 | 1.486538 | — | — | — |
| basic_streambuf.test | LINK | 0.228235 | 0.416567 | 0.584991 | — | — | — |
| basic_streambuf_select.test | LINK | 0.229627 | 0.420125 | 0.581693 | — | — | — |
| basic_waitable_timer.test | LINK | 0.423468 | 0.824186 | 1.357494 | — | — | — |
| basic_waitable_timer_select.test | LINK | 0.433158 | 0.855702 | 1.411005 | — | — | — |
| basic_writable_pipe.test | LINK | 0.434040 | 0.844325 | 1.272519 | — | — | — |
| basic_writable_pipe_select.test | LINK | 0.458870 | 0.896900 | 1.548787 | — | — | — |
| bind_allocator.test | RUN | 0.691743 | 3.225752 | 3.232826 | 0.002328 | 0.002294 | 0.002250 |
| bind_allocator_select.test | RUN | 0.691213 | 3.286666 | 3.177073 | 0.001869 | 0.002266 | 0.002109 |
| bind_cancellation_slot.test | RUN | 0.705335 | 3.276822 | 3.235521 | 0.001558 | 0.001541 | 0.001517 |
| bind_cancellation_slot_select.test | RUN | 0.711623 | 3.285817 | 3.115157 | 0.001533 | 0.001544 | 0.001459 |
| bind_executor.test | RUN | 0.487794 | 2.282003 | 1.969323 | 0.002198 | 0.002393 | 0.002275 |
| bind_executor_select.test | RUN | 0.485000 | 2.318084 | 1.981127 | 0.001972 | 0.002071 | 0.002203 |
| bind_immediate_executor.test | RUN | 0.369676 | 1.880315 | 1.241481 | 0.001499 | 0.001397 | 0.001482 |
| bind_immediate_executor_select.test | RUN | 0.386576 | 1.857304 | 1.388330 | 0.001546 | 0.001449 | 0.001387 |
| buffer.test | RUN | 0.223058 | 0.836351 | 0.786616 | 0.001508 | 0.001543 | 0.001446 |
| buffer_select.test | RUN | 0.225876 | 0.842146 | 0.785557 | 0.001452 | 0.001552 | 0.001329 |
| buffer_registration.test | LINK | 0.194078 | 0.429999 | 0.459466 | — | — | — |
| buffer_registration_select.test | LINK | 0.191341 | 0.422190 | 0.455685 | — | — | — |
| buffered_read_stream.test | RUN | 0.758413 | 5.084186 | 4.537606 | 0.001926 | 0.002318 | 0.002022 |
| buffered_read_stream_select.test | RUN | 0.771098 | 5.244675 | 4.387240 | 0.001847 | 0.002286 | 0.002149 |
| buffered_stream.test | RUN | 1.156885 | 7.792077 | 7.781948 | 0.002569 | 0.002480 | 0.002412 |
| buffered_stream_select.test | RUN | 1.279923 | 7.870750 | 7.490966 | 0.002393 | 0.002416 | 0.002269 |
| buffered_write_stream.test | RUN | 1.284823 | 6.685122 | 6.738552 | 0.002236 | 0.002500 | 0.002394 |
| buffered_write_stream_select.test | RUN | 1.286735 | 6.837429 | 6.459565 | 0.002205 | 0.002420 | 0.002469 |
| buffers_iterator.test | RUN | 0.325282 | 0.652756 | 1.079799 | 0.001887 | 0.001894 | 0.001816 |
| buffers_iterator_select.test | RUN | 0.318984 | 0.648465 | 1.083703 | 0.001544 | 0.001861 | 0.001788 |
| cancel_after.test | RUN | 1.202118 | 5.230740 | 4.867573 | 0.004165 | 0.004150 | 0.003711 |
| cancel_after_select.test | RUN | 1.184990 | 5.217715 | 4.856157 | 0.003574 | 0.004042 | 0.003665 |
| cancel_at.test | RUN | 1.193233 | 4.662581 | 4.690491 | 0.003840 | 0.003632 | 0.003670 |
| cancel_at_select.test | RUN | 1.194074 | 4.474504 | 4.611711 | 0.003180 | 0.003568 | 0.003556 |
| cancellation_signal.test | LINK | 0.137148 | 0.242302 | 0.298137 | — | — | — |
| cancellation_signal_select.test | LINK | 0.186362 | 0.241483 | 0.303355 | — | — | — |
| cancellation_state.test | LINK | 0.199616 | 0.240274 | 0.356759 | — | — | — |
| cancellation_state_select.test | LINK | 0.198561 | 0.242802 | 0.365907 | — | — | — |
| cancellation_type.test | LINK | 0.150354 | 0.190875 | 0.365044 | — | — | — |
| cancellation_type_select.test | LINK | 0.151407 | 0.190959 | 0.363335 | — | — | — |
| co_composed.test | RUN | 0.151957 | 0.191672 | 0.361254 | 0.001763 | 0.001375 | 0.001512 |
| co_composed_select.test | RUN | 0.155068 | 0.191944 | 0.357523 | 0.001435 | 0.001414 | 0.001602 |
| co_spawn.test | LINK | 0.151719 | 0.199358 | 0.385009 | — | — | — |
| co_spawn_select.test | LINK | 0.155574 | 0.256092 | 0.370631 | — | — | — |
| completion_condition.test | LINK | 0.193955 | 0.368890 | 0.522881 | — | — | — |
| completion_condition_select.test | LINK | 0.193254 | 0.422902 | 0.516871 | — | — | — |
| compose.test | RUN | 0.864617 | 4.075827 | 4.020054 | 0.002422 | 0.002448 | 0.002324 |
| compose_select.test | RUN | 0.875054 | 4.002154 | 4.000380 | 0.002429 | 0.002347 | 0.002032 |
| composed.test | RUN | 0.844924 | 4.033230 | 3.975244 | 0.001710 | 0.002151 | 0.002136 |
| composed_select.test | RUN | 0.827396 | 3.968306 | 3.919791 | 0.002075 | 0.001961 | 0.001834 |
| config.test | RUN | 0.393665 | 2.551655 | 1.706830 | 0.001923 | 0.002103 | 0.002049 |
| config_select.test | RUN | 0.519721 | 2.707804 | 2.053491 | 0.001914 | 0.001996 | 0.001974 |
| connect.test | LINK | 1.455328 | 8.558740 | 8.473825 | — | — | — |
| connect_select.test | LINK | 1.458355 | 8.549663 | 8.500058 | — | — | — |
| connect_pipe.test | RUN | 0.743327 | 3.220629 | 3.155347 | 0.002066 | 0.002030 | 0.001992 |
| connect_pipe_select.test | RUN | 0.756460 | 3.363916 | 3.363227 | 0.001898 | 0.002034 | 0.002011 |
| consign.test | RUN | 0.647844 | 3.171728 | 2.820384 | 0.001983 | 0.001936 | 0.001934 |
| consign_select.test | RUN | 0.647720 | 3.212684 | 2.782807 | 0.001941 | 0.001959 | 0.001724 |
| coroutine.test | LINK | 0.161323 | 0.332910 | 0.412654 | — | — | — |
| coroutine_select.test | LINK | 0.155009 | 0.338531 | 0.427541 | — | — | — |
| deadline_timer.test | RUN | 0.969384 | 4.125595 | 4.147441 | 0.003510 | 0.003256 | 0.003107 |
| deadline_timer_select.test | RUN | 0.962191 | 4.095466 | 4.137669 | 0.002798 | 0.002943 | 0.002967 |
| deferred.test | RUN | 0.166431 | 0.298717 | 0.415662 | 0.001601 | 0.001908 | 0.001816 |
| deferred_select.test | RUN | 0.172118 | 0.314616 | 0.411089 | 0.001842 | 0.001878 | 0.001850 |
| detached.test | LINK | 0.201500 | 0.342153 | 0.455792 | — | — | — |
| detached_select.test | LINK | 0.183863 | 0.342255 | 0.476973 | — | — | — |
| disposition.test | RUN | 0.236152 | 0.808381 | 0.857301 | 0.001524 | 0.002002 | 0.001960 |
| disposition_select.test | RUN | 0.175781 | 0.794221 | 0.844714 | 0.001516 | 0.001996 | 0.001923 |
| error.test | RUN | 0.163348 | 0.770229 | 0.780139 | 0.001340 | 0.001870 | 0.001884 |
| error_select.test | RUN | 0.163806 | 0.766495 | 0.726576 | 0.001544 | 0.001960 | 0.001935 |
| file_base.test | LINK | 0.210523 | 0.642367 | 0.807184 | — | — | — |
| file_base_select.test | LINK | 0.209644 | 0.638771 | 0.612421 | — | — | — |
| fwd_first.test | LINK | 0.464961 | 1.006808 | 1.425167 | — | — | — |
| fwd_last.test | LINK | 0.467565 | 0.918557 | 1.439925 | — | — | — |
| generic_basic_endpoint.test | LINK | 0.184283 | 0.384372 | 0.513636 | — | — | — |
| generic_basic_endpoint_select.test | LINK | 0.181671 | 0.383882 | 0.522408 | — | — | — |
| generic_datagram_protocol.test | LINK | 0.664432 | 3.270258 | 2.775965 | — | — | — |
| generic_datagram_protocol_select.test | LINK | 0.656110 | 3.405260 | 2.785337 | — | — | — |
| generic_raw_protocol.test | LINK | 0.666050 | 3.294313 | 2.775811 | — | — | — |
| generic_raw_protocol_select.test | LINK | 0.654700 | 3.405453 | 2.770560 | — | — | — |
| generic_seq_packet_protocol.test | LINK | 0.575585 | 2.766413 | 2.395231 | — | — | — |
| generic_seq_packet_protocol_select.test | LINK | 0.573546 | 2.806287 | 2.386513 | — | — | — |
| generic_stream_protocol.test | LINK | 0.609935 | 3.025210 | 2.575461 | — | — | — |
| generic_stream_protocol_select.test | LINK | 0.612032 | 3.102363 | 2.566268 | — | — | — |
| high_resolution_timer.test | LINK | 0.295829 | 0.564669 | 0.816243 | — | — | — |
| high_resolution_timer_select.test | LINK | 0.303242 | 0.588736 | 0.829086 | — | — | — |
| immediate.test | LINK | 0.209098 | 0.433021 | 0.503599 | — | — | — |
| immediate_select.test | LINK | 0.206817 | 0.432707 | 0.495481 | — | — | — |
| inline_executor.test | RUN | 0.247772 | 0.666615 | 0.754500 | 0.001459 | 0.001509 | 0.001425 |
| inline_executor_select.test | RUN | 0.251110 | 0.662648 | 0.758299 | 0.001530 | 0.001508 | 0.001398 |
| inline_or_executor.test | RUN | 0.391944 | 1.946804 | 1.416669 | 0.001514 | 0.001451 | 0.001474 |
| inline_or_executor_select.test | RUN | 0.398539 | 2.019157 | 1.553438 | 0.001526 | 0.001548 | 0.001404 |
| io_context.test | RUN | 0.552432 | 2.979498 | 2.473013 | 0.002428 | 0.002440 | 0.002363 |
| io_context_select.test | RUN | 0.560511 | 2.913558 | 2.471383 | 0.002444 | 0.002362 | 0.002232 |
| io_context_strand.test | RUN | 0.516183 | 2.508073 | 2.149892 | 0.002331 | 0.002236 | 0.002514 |
| io_context_strand_select.test | RUN | 0.525600 | 2.501984 | 2.156105 | 0.002209 | 0.002312 | 0.002231 |
| ip_address.test | LINK | 0.206994 | 0.589117 | 0.615488 | — | — | — |
| ip_address_select.test | LINK | 0.207161 | 0.590155 | 0.611435 | — | — | — |
| ip_address_v4.test | LINK | 0.215881 | 0.628814 | 0.678947 | — | — | — |
| ip_address_v4_select.test | LINK | 0.214586 | 0.629872 | 0.677702 | — | — | — |
| ip_address_v6.test | LINK | 0.236327 | 0.812537 | 0.828926 | — | — | — |
| ip_address_v6_select.test | LINK | 0.238775 | 0.814533 | 0.828726 | — | — | — |
| ip_basic_endpoint.test | LINK | 0.191090 | 0.405268 | 0.537261 | — | — | — |
| ip_basic_endpoint_select.test | LINK | 0.190705 | 0.406989 | 0.537943 | — | — | — |
| ip_basic_resolver.test | LINK | 0.299585 | 0.578364 | 0.891110 | — | — | — |
| ip_basic_resolver_select.test | LINK | 0.304623 | 0.595696 | 0.868547 | — | — | — |
| ip_basic_resolver_entry.test | LINK | 0.105740 | 0.194613 | 0.358716 | — | — | — |
| ip_basic_resolver_entry_select.test | LINK | 0.104903 | 0.196905 | 0.336712 | — | — | — |
| ip_basic_resolver_iterator.test | LINK | 0.188715 | 0.392799 | 0.878743 | — | — | — |
| ip_basic_resolver_iterator_select.test | LINK | 0.186274 | 0.391196 | 0.944219 | — | — | — |
| ip_basic_resolver_query.test | LINK | 0.180016 | 0.384044 | 0.881225 | — | — | — |
| ip_basic_resolver_query_select.test | LINK | 0.178886 | 0.379223 | 0.846493 | — | — | — |
| ip_host_name.test | RUN | 0.193258 | 0.502299 | 0.973966 | 0.001408 | 0.001479 | 0.001820 |
| ip_host_name_select.test | RUN | 0.188707 | 0.501781 | 0.958032 | 0.001440 | 0.001466 | 0.001844 |
| ip_icmp.test | RUN | 0.835062 | 6.685040 | 5.989948 | 0.001513 | 0.001820 | 0.001783 |
| ip_icmp_select.test | RUN | 0.821992 | 6.872403 | 5.839079 | 0.001449 | 0.002004 | 0.001879 |
| ip_multicast.test | RUN | 0.506126 | 3.254753 | 3.376118 | 0.001597 | 0.002195 | 0.002088 |
| ip_multicast_select.test | RUN | 0.502630 | 3.407229 | 3.375194 | 0.001524 | 0.001878 | 0.001968 |
| ip_resolver_query_base.test | LINK | 0.126333 | 0.321545 | 0.390416 | — | — | — |
| ip_resolver_query_base_select.test | LINK | 0.123422 | 0.319689 | 0.407241 | — | — | — |
| ip_tcp.test | RUN | 1.271125 | 9.930536 | 8.978592 | 0.002036 | 0.002792 | 0.002612 |
| ip_tcp_select.test | RUN | 1.228263 | 10.362475 | 8.703632 | 0.002033 | 0.002803 | 0.002670 |
| ip_udp.test | RUN | 0.881505 | 7.085966 | 6.308955 | 0.001682 | 0.002142 | 0.001927 |
| ip_udp_select.test | RUN | 0.860042 | 7.247514 | 6.087690 | 0.001688 | 0.002002 | 0.001575 |
| ip_unicast.test | RUN | 0.461297 | 3.088270 | 3.036035 | 0.001548 | 0.002086 | 0.001817 |
| ip_unicast_select.test | RUN | 0.464821 | 3.133406 | 2.963008 | 0.001510 | 0.002040 | 0.002036 |
| ip_v6_only.test | RUN | 0.472450 | 3.201207 | 3.086684 | 0.001538 | 0.001984 | 0.002133 |
| ip_v6_only_select.test | RUN | 0.480078 | 3.315264 | 3.057556 | 0.001478 | 0.001754 | 0.001980 |
| is_read_buffered.test | RUN | 0.344080 | 1.021987 | 1.667005 | 0.001435 | 0.001710 | 0.001930 |
| is_read_buffered_select.test | RUN | 0.357187 | 1.023558 | 1.692043 | 0.001476 | 0.001907 | 0.001667 |
| is_write_buffered.test | RUN | 0.343047 | 1.019740 | 1.617480 | 0.001441 | 0.001962 | 0.001725 |
| is_write_buffered_select.test | RUN | 0.356943 | 1.023923 | 1.689400 | 0.001385 | 0.001959 | 0.001896 |
| local_basic_endpoint.test | LINK | 0.183910 | 0.562284 | 0.901084 | — | — | — |
| local_basic_endpoint_select.test | LINK | 0.180976 | 0.551805 | 0.894492 | — | — | — |
| local_connect_pair.test | LINK | 0.489032 | 3.176735 | 2.848894 | — | — | — |
| local_connect_pair_select.test | LINK | 0.573497 | 3.139398 | 2.789738 | — | — | — |
| local_datagram_protocol.test | LINK | 0.838248 | 4.103475 | 3.957225 | — | — | — |
| local_datagram_protocol_select.test | LINK | 0.853710 | 4.164022 | 3.878643 | — | — | — |
| local_seq_packet_protocol.test | LINK | 0.839213 | 4.028358 | 3.868056 | — | — | — |
| local_seq_packet_protocol_select.test | LINK | 0.837868 | 4.082817 | 3.840875 | — | — | — |
| local_stream_protocol.test | LINK | 0.799789 | 3.848354 | 3.707834 | — | — | — |
| local_stream_protocol_select.test | LINK | 0.792297 | 3.864140 | 3.683678 | — | — | — |
| placeholders.test | LINK | 0.171430 | 0.315251 | 0.424411 | — | — | — |
| placeholders_select.test | LINK | 0.166762 | 0.313750 | 0.434703 | — | — | — |
| posix_basic_descriptor.test | LINK | 0.437492 | 0.843025 | 1.263908 | — | — | — |
| posix_basic_descriptor_select.test | LINK | 0.451007 | 0.890828 | 1.501199 | — | — | — |
| posix_basic_stream_descriptor.test | LINK | 0.434953 | 0.859204 | 1.298870 | — | — | — |
| posix_basic_stream_descriptor_select.test | LINK | 0.449798 | 0.887625 | 1.532935 | — | — | — |
| posix_descriptor_base.test | LINK | 0.191388 | 0.325159 | 0.385094 | — | — | — |
| posix_descriptor_base_select.test | LINK | 0.183327 | 0.331725 | 0.391174 | — | — | — |
| posix_stream_descriptor.test | LINK | 0.809415 | 3.885764 | 3.692450 | — | — | — |
| posix_stream_descriptor_select.test | LINK | 0.818672 | 3.921483 | 3.804841 | — | — | — |
| prepend.test | RUN | 0.635784 | 3.177565 | 2.919798 | 0.001926 | 0.002000 | 0.001935 |
| prepend_select.test | RUN | 0.645876 | 3.238721 | 2.910212 | 0.001916 | 0.001949 | 0.001953 |
| random_access_file.test | LINK | 0.403577 | 0.768151 | 0.998211 | — | — | — |
| random_access_file_select.test | LINK | 0.414608 | 0.824559 | 1.284573 | — | — | — |
| read.test | RUN | 1.961220 | 19.740279 | 16.783749 | 0.002685 | 0.002410 | 0.002551 |
| read_select.test | RUN | 1.985670 | 19.762822 | 16.926480 | 0.002682 | 0.002485 | 0.002749 |
| read_at.test | RUN | 1.972065 | 28.237463 | 22.707892 | 0.002997 | 0.003204 | 0.002896 |
| read_at_select.test | RUN | 1.986561 | 27.939305 | 22.674087 | 0.003247 | 0.003102 | 0.003066 |
| read_until.test | RUN | 1.091242 | 7.491593 | 6.597615 | 0.002203 | 0.002173 | 0.001904 |
| read_until_select.test | RUN | 1.105447 | 7.518129 | 6.790755 | 0.001966 | 0.002248 | 0.002206 |
| readable_pipe.test | LINK | 0.672036 | 3.186336 | 2.925088 | — | — | — |
| readable_pipe_select.test | LINK | 0.672354 | 3.412463 | 3.102404 | — | — | — |
| recycling_allocator.test | RUN | 0.227109 | 0.466724 | 0.575455 | 0.001840 | 0.001910 | 0.002049 |
| recycling_allocator_select.test | RUN | 0.226439 | 0.465575 | 0.628157 | 0.001704 | 0.001898 | 0.001923 |
| redirect_disposition.test | LINK | 0.925257 | 4.449787 | 4.443708 | — | — | — |
| redirect_disposition_select.test | LINK | 0.933130 | 4.433761 | 4.393378 | — | — | — |
| redirect_error.test | LINK | 0.824856 | 3.728072 | 3.934170 | — | — | — |
| redirect_error_select.test | LINK | 0.821084 | 3.727359 | 3.877878 | — | — | — |
| registered_buffer.test | LINK | 0.216740 | 0.388834 | 0.532423 | — | — | — |
| registered_buffer_select.test | LINK | 0.214264 | 0.383517 | 0.537074 | — | — | — |
| signal_set.test | RUN | 0.644066 | 3.142338 | 2.751071 | 0.001706 | 0.002022 | 0.001833 |
| signal_set_select.test | RUN | 0.649760 | 3.242986 | 2.990372 | 0.001604 | 0.001972 | 0.001953 |
| signal_set_base.test | LINK | 0.185925 | 0.323018 | 0.370313 | — | — | — |
| signal_set_base_select.test | LINK | 0.187255 | 0.321973 | 0.388007 | — | — | — |
| socket_base.test | RUN | 0.835156 | 4.743196 | 4.156985 | 0.002094 | 0.002187 | 0.002182 |
| socket_base_select.test | RUN | 0.834081 | 4.650126 | 4.091483 | 0.001762 | 0.002074 | 0.002091 |
| spawn.test | RUN | 1.324139 | 7.546232 | 7.026410 | 0.001910 | 0.002072 | 0.002315 |
| spawn_select.test | RUN | 1.335906 | 7.533887 | 7.015159 | 0.001549 | 0.002066 | 0.002400 |
| static_thread_pool.test | RUN | 0.393803 | 0.752917 | 0.952529 | 0.001807 | 0.001730 | 0.001933 |
| static_thread_pool_select.test | RUN | 0.418566 | 0.791259 | 1.261048 | 0.001620 | 0.001925 | 0.001906 |
| steady_timer.test | LINK | 0.420366 | 0.861359 | 1.378248 | — | — | — |
| steady_timer_select.test | LINK | 0.429788 | 0.855923 | 1.405244 | — | — | — |
| strand.test | RUN | 0.864533 | 4.236000 | 4.333590 | 0.002372 | 0.002590 | 0.002712 |
| strand_select.test | RUN | 0.873631 | 4.182575 | 4.225188 | 0.002238 | 0.002756 | 0.002836 |
| stream_file.test | LINK | 0.390464 | 0.777413 | 0.959595 | — | — | — |
| stream_file_select.test | LINK | 0.412543 | 0.803925 | 1.267450 | — | — | — |
| streambuf.test | RUN | 0.173824 | 0.456470 | 0.733812 | 0.001467 | 0.001414 | 0.001670 |
| streambuf_select.test | RUN | 0.176795 | 0.502964 | 0.731522 | 0.001356 | 0.001419 | 0.001824 |
| system_timer.test | LINK | 0.563037 | 2.766284 | 2.770728 | — | — | — |
| system_timer_select.test | LINK | 0.558155 | 2.745893 | 2.454525 | — | — | — |
| system_context.test | LINK | 0.269780 | 0.502627 | 0.602322 | — | — | — |
| system_context_select.test | LINK | 0.278841 | 0.539422 | 0.740838 | — | — | — |
| system_executor.test | LINK | 0.336487 | 1.577515 | 1.134426 | — | — | — |
| system_executor_select.test | LINK | 0.346440 | 1.553759 | 1.278834 | — | — | — |
| this_coro.test | LINK | 0.106695 | 0.198198 | 0.235445 | — | — | — |
| this_coro_select.test | LINK | 0.106320 | 0.194602 | 0.239859 | — | — | — |
| thread_pool.test | RUN | 0.391749 | 2.101853 | 1.454347 | 0.002818 | 0.003331 | 0.002753 |
| thread_pool_select.test | RUN | 0.403200 | 2.093273 | 1.611376 | 0.002638 | 0.003004 | 0.002655 |
| time_traits.test | LINK | 0.241697 | 0.570426 | 0.483048 | — | — | — |
| time_traits_select.test | LINK | 0.241905 | 0.584381 | 0.481171 | — | — | — |
| ts_buffer.test | LINK | 0.223306 | 0.651902 | 0.510300 | — | — | — |
| ts_buffer_select.test | LINK | 0.221371 | 0.643853 | 0.591406 | — | — | — |
| ts_executor.test | LINK | 0.307756 | 0.909940 | 1.143294 | — | — | — |
| ts_executor_select.test | LINK | 0.319436 | 0.951971 | 1.436136 | — | — | — |
| ts_internet.test | LINK | 0.337992 | 0.996155 | 1.641475 | — | — | — |
| ts_internet_select.test | LINK | 0.351429 | 1.006825 | 1.680678 | — | — | — |
| ts_io_context.test | LINK | 0.270355 | 0.766095 | 1.012268 | — | — | — |
| ts_io_context_select.test | LINK | 0.285113 | 0.809927 | 1.270178 | — | — | — |
| ts_net.test | LINK | 0.403144 | 1.190296 | 2.028512 | — | — | — |
| ts_net_select.test | LINK | 0.420782 | 1.214694 | 2.070943 | — | — | — |
| ts_netfwd.test | LINK | 0.404834 | 1.204967 | 2.037187 | — | — | — |
| ts_netfwd_select.test | LINK | 0.409530 | 1.230151 | 2.072158 | — | — | — |
| ts_socket.test | LINK | 0.338944 | 0.953973 | 1.565754 | — | — | — |
| ts_socket_select.test | LINK | 0.346571 | 0.968139 | 1.594171 | — | — | — |
| ts_timer.test | LINK | 0.291641 | 0.842804 | 1.378089 | — | — | — |
| ts_timer_select.test | LINK | 0.298303 | 0.841450 | 1.378306 | — | — | — |
| use_awaitable.test | LINK | 0.107481 | 0.280172 | 0.401721 | — | — | — |
| use_awaitable_select.test | LINK | 0.109071 | 0.293456 | 0.361195 | — | — | — |
| wait_traits.test | LINK | 0.108260 | 0.280877 | 0.373937 | — | — | — |
| wait_traits_select.test | LINK | 0.106556 | 0.279462 | 0.368596 | — | — | — |
| windows_basic_object_handle.test | LINK | 0.107332 | 0.283929 | 0.362389 | — | — | — |
| windows_basic_object_handle_select.test | LINK | 0.106332 | 0.276281 | 0.378402 | — | — | — |
| windows_basic_overlapped_handle.test | LINK | 0.106940 | 0.285847 | 0.368986 | — | — | — |
| windows_basic_overlapped_handle_select.test | LINK | 0.105501 | 0.281402 | 0.399893 | — | — | — |
| windows_basic_random_access_handle.test | LINK | 0.107313 | 0.283729 | 0.395374 | — | — | — |
| windows_basic_random_access_handle_select.test | LINK | 0.141317 | 0.287221 | 0.385877 | — | — | — |
| windows_basic_stream_handle.test | LINK | 0.153376 | 0.278343 | 0.391031 | — | — | — |
| windows_basic_stream_handle_select.test | LINK | 0.152010 | 0.280344 | 0.402633 | — | — | — |
| windows_object_handle.test | LINK | 0.409146 | 0.764171 | 1.023838 | — | — | — |
| windows_object_handle_select.test | LINK | 0.415455 | 0.813512 | 1.263716 | — | — | — |
| windows_overlapped_ptr.test | LINK | 0.416120 | 0.798259 | 1.126139 | — | — | — |
| windows_overlapped_ptr_select.test | LINK | 0.433553 | 0.854634 | 1.388175 | — | — | — |
| windows_random_access_handle.test | LINK | 0.413890 | 0.763512 | 0.993680 | — | — | — |
| windows_random_access_handle_select.test | LINK | 0.418142 | 0.810126 | 1.306354 | — | — | — |
| windows_stream_handle.test | LINK | 0.401882 | 0.775939 | 1.017242 | — | — | — |
| windows_stream_handle_select.test | LINK | 0.414791 | 0.818746 | 1.323685 | — | — | — |
| writable_pipe.test | LINK | 0.671077 | 3.159971 | 3.065626 | — | — | — |
| writable_pipe_select.test | LINK | 0.693854 | 3.285793 | 3.242282 | — | — | — |
| write.test | RUN | 1.972325 | 16.493894 | 16.519829 | 0.002791 | 0.002490 | 0.002608 |
| write_select.test | RUN | 1.980920 | 16.322424 | 16.674132 | 0.002710 | 0.002492 | 0.002586 |
| write_at.test | RUN | 2.053787 | 23.786396 | 21.282421 | 0.002997 | 0.002824 | 0.002738 |
| write_at_select.test | RUN | 2.070568 | 23.462654 | 21.616436 | 0.002912 | 0.002494 | 0.002836 |
| any_executor.test | RUN | 0.813032 | 7.978059 | 7.214094 | 0.002216 | 0.002594 | 0.002592 |
| blocking.test | RUN | 1.995013 | 9.395988 | 14.015582 | 0.007576 | 0.007089 | 0.007556 |
| blocking_adaptation.test | RUN | 1.011560 | 4.046320 | 6.352591 | 0.004076 | 0.004282 | 0.004224 |
| context_as.test | RUN | 0.546852 | 2.894724 | 2.085882 | 0.002090 | 0.002124 | 0.002052 |
| executor.test | RUN | 0.163152 | 0.303510 | 0.410495 | 0.001848 | 0.001890 | 0.001884 |
| inline_exception_handling.test | RUN | 1.997840 | 8.918908 | 13.832218 | 0.007315 | 0.005126 | 0.005215 |
| invocable_archetype.test | RUN | 0.159414 | 0.294887 | 0.395189 | 0.001792 | 0.001835 | 0.001794 |
| mapping.test | RUN | 1.788150 | 9.102986 | 13.350013 | 0.005328 | 0.005217 | 0.005272 |
| outstanding_work.test | RUN | 0.930113 | 3.738315 | 6.098847 | 0.004346 | 0.004301 | 0.004295 |
| prefer_only.test | RUN | 0.326361 | 1.545950 | 1.404292 | 0.001887 | 0.001692 | 0.001900 |
| relationship.test | RUN | 0.807128 | 3.273034 | 5.611129 | 0.003004 | 0.002973 | 0.004110 |

## Native runtime replay

Child CPU of successful native tests, with process startup included; GDB and expected-failure executions are correctness-only. Two warmups per compiler, four A/A observations per reference and mirrored ABBA/BAAB blocks. Short tests repeat up to ten times, identically across compilers; CPU is divided by repetitions before summing tests. Executable hashes and output checks stay outside process timers. All runs must pass their original native oracle. Output hashes record equality or variation across the complete campaign; test reports may print times or addresses.

| Reference | CPU speedup ↑ | Paired blocks | A/A speedup |
|---|---:|---|---:|
| gcc | 0.989× | 0.989×, 0.990× | 1.002× |
| clang | 0.984× | 0.983×, 0.985× | 1.010× |

† A/A drift or paired-block spread exceeds 5%; small differences are unresolved. 1375 native tests in the full cohort; 1362 have byte-identical output throughout. [Manifest](https://github.com/vishvananda/cppgm-extended/actions/runs/42916136104), [paired observations](https://github.com/vishvananda/cppgm-extended/actions/runs/42916136104) and [per-test summaries](https://github.com/vishvananda/cppgm-extended/actions/runs/42916136104).

| Test | Repeats | CPU cppgm | GCC | Clang | Output check |
|---|---:|---:|---:|---:|---|
| any_completion_executor.test | 3 | 0.001646s | 0.001643s | 0.001658s | byte equality observed |
| any_completion_executor_select.test | 5 | 0.001640s | 0.001629s | 0.001628s | byte equality observed |
| any_completion_handler.test | 10 | 0.001335s | 0.001311s | 0.001308s | byte equality observed |
| any_completion_handler_select.test | 9 | 0.001324s | 0.001310s | 0.001312s | byte equality observed |
| any_io_executor.test | 4 | 0.001664s | 0.001628s | 0.001621s | byte equality observed |
| any_io_executor_select.test | 4 | 0.001628s | 0.001623s | 0.001643s | byte equality observed |
| append.test | 10 | 0.001264s | 0.001246s | 0.001250s | byte equality observed |
| append_select.test | 10 | 0.001242s | 0.001229s | 0.001224s | byte equality observed |
| as_tuple.test | 7 | 0.001351s | 0.001324s | 0.001347s | byte equality observed |
| as_tuple_select.test | 8 | 0.001292s | 0.001256s | 0.001242s | byte equality observed |
| bind_allocator.test | 1 | 0.001727s | 0.001719s | 0.001673s | byte equality observed |
| bind_allocator_select.test | 1 | 0.001470s | 0.001418s | 0.001392s | byte equality observed |
| bind_cancellation_slot.test | 10 | 0.001187s | 0.001186s | 0.001169s | byte equality observed |
| bind_cancellation_slot_select.test | 10 | 0.001106s | 0.001096s | 0.001082s | byte equality observed |
| bind_executor.test | 1 | 0.001709s | 0.001699s | 0.001724s | byte equality observed |
| bind_executor_select.test | 1 | 0.002008s | 0.002038s | 0.001996s | byte equality observed |
| bind_immediate_executor.test | 10 | 0.001117s | 0.001129s | 0.001127s | byte equality observed |
| bind_immediate_executor_select.test | 10 | 0.001096s | 0.001109s | 0.001076s | byte equality observed |
| buffer.test | 10 | 0.001053s | 0.001067s | 0.001041s | byte equality observed |
| buffer_select.test | 10 | 0.001081s | 0.001063s | 0.001059s | byte equality observed |
| buffered_read_stream.test | 10 | 0.001393s | 0.001365s | 0.001371s | byte equality observed |
| buffered_read_stream_select.test | 10 | 0.001340s | 0.001376s | 0.001364s | byte equality observed |
| buffered_stream.test | 10 | 0.001381s | 0.001381s | 0.001376s | byte equality observed |
| buffered_stream_select.test | 10 | 0.001352s | 0.001315s | 0.001325s | byte equality observed |
| buffered_write_stream.test | 10 | 0.001375s | 0.001398s | 0.001366s | byte equality observed |
| buffered_write_stream_select.test | 10 | 0.001357s | 0.001333s | 0.001319s | byte equality observed |
| buffers_iterator.test | 10 | 0.001044s | 0.001064s | 0.001051s | byte equality observed |
| buffers_iterator_select.test | 10 | 0.001031s | 0.000991s | 0.001026s | byte equality observed |
| cancel_after.test | 1 | 0.003478s | 0.003264s | 0.003399s | byte equality observed |
| cancel_after_select.test | 1 | 0.002925s | 0.002811s | 0.002798s | byte equality observed |
| cancel_at.test | 1 | 0.003620s | 0.003602s | 0.003540s | byte equality observed |
| cancel_at_select.test | 1 | 0.003701s | 0.003511s | 0.003505s | byte equality observed |
| co_composed.test | 10 | 0.001062s | 0.001039s | 0.001078s | byte equality observed |
| co_composed_select.test | 10 | 0.001013s | 0.000993s | 0.001047s | byte equality observed |
| compose.test | 1 | 0.001745s | 0.001727s | 0.001703s | byte equality observed |
| compose_select.test | 1 | 0.001463s | 0.001442s | 0.001475s | byte equality observed |
| composed.test | 1 | 0.002079s | 0.002045s | 0.002080s | byte equality observed |
| composed_select.test | 1 | 0.001597s | 0.001613s | 0.001603s | byte equality observed |
| config.test | 10 | 0.001138s | 0.001124s | 0.001123s | byte equality observed |
| config_select.test | 10 | 0.001158s | 0.001159s | 0.001129s | byte equality observed |
| connect_pipe.test | 10 | 0.001146s | 0.001173s | 0.001141s | byte equality observed |
| connect_pipe_select.test | 10 | 0.001149s | 0.001132s | 0.001110s | byte equality observed |
| consign.test | 10 | 0.001146s | 0.001176s | 0.001129s | byte equality observed |
| consign_select.test | 10 | 0.001076s | 0.001063s | 0.001068s | byte equality observed |
| deadline_timer.test | 1 | 0.002459s | 0.002381s | 0.002324s | byte equality observed |
| deadline_timer_select.test | 1 | 0.002196s | 0.002140s | 0.002200s | byte equality observed |
| deferred.test | 10 | 0.001032s | 0.001037s | 0.001032s | byte equality observed |
| deferred_select.test | 10 | 0.000986s | 0.000983s | 0.000963s | byte equality observed |
| disposition.test | 10 | 0.001052s | 0.001050s | 0.001047s | byte equality observed |
| disposition_select.test | 10 | 0.001046s | 0.001046s | 0.001041s | byte equality observed |
| error.test | 10 | 0.001024s | 0.001013s | 0.001016s | byte equality observed |
| error_select.test | 10 | 0.001020s | 0.001011s | 0.001023s | byte equality observed |
| inline_executor.test | 10 | 0.001030s | 0.001024s | 0.001010s | byte equality observed |
| inline_executor_select.test | 10 | 0.001032s | 0.001026s | 0.001026s | byte equality observed |
| inline_or_executor.test | 10 | 0.001034s | 0.001035s | 0.001018s | byte equality observed |
| inline_or_executor_select.test | 10 | 0.001042s | 0.001039s | 0.001030s | byte equality observed |
| io_context.test | 1 | 0.001953s | 0.001905s | 0.002013s | byte equality observed |
| io_context_select.test | 1 | 0.002325s | 0.002318s | 0.002303s | byte equality observed |
| io_context_strand.test | 1 | 0.002314s | 0.002312s | 0.002307s | byte equality observed |
| io_context_strand_select.test | 1 | 0.002056s | 0.002044s | 0.002062s | byte equality observed |
| ip_host_name.test | 10 | 0.001052s | 0.001045s | 0.001038s | byte equality observed |
| ip_host_name_select.test | 10 | 0.000997s | 0.000985s | 0.000997s | byte equality observed |
| ip_icmp.test | 8 | 0.001035s | 0.001043s | 0.001014s | byte equality observed |
| ip_icmp_select.test | 10 | 0.001034s | 0.001026s | 0.001014s | byte equality observed |
| ip_multicast.test | 10 | 0.001113s | 0.001100s | 0.001110s | byte equality observed |
| ip_multicast_select.test | 10 | 0.001096s | 0.001080s | 0.001086s | byte equality observed |
| ip_tcp.test | 10 | 0.001515s | 0.001483s | 0.001505s | byte equality observed |
| ip_tcp_select.test | 10 | 0.001457s | 0.001421s | 0.001433s | byte equality observed |
| ip_udp.test | 10 | 0.001139s | 0.001122s | 0.001127s | byte equality observed |
| ip_udp_select.test | 10 | 0.001131s | 0.001103s | 0.001109s | byte equality observed |
| ip_unicast.test | 10 | 0.001090s | 0.001087s | 0.001080s | byte equality observed |
| ip_unicast_select.test | 10 | 0.001073s | 0.001111s | 0.001054s | byte equality observed |
| ip_v6_only.test | 10 | 0.001081s | 0.001170s | 0.001074s | byte equality observed |
| ip_v6_only_select.test | 10 | 0.001069s | 0.001157s | 0.001056s | byte equality observed |
| is_read_buffered.test | 10 | 0.001001s | 0.001069s | 0.000991s | byte equality observed |
| is_read_buffered_select.test | 10 | 0.001007s | 0.001043s | 0.001000s | byte equality observed |
| is_write_buffered.test | 10 | 0.001003s | 0.001034s | 0.000991s | byte equality observed |
| is_write_buffered_select.test | 10 | 0.001021s | 0.001068s | 0.000990s | byte equality observed |
| prepend.test | 10 | 0.001082s | 0.001138s | 0.001115s | byte equality observed |
| prepend_select.test | 10 | 0.001060s | 0.001103s | 0.001085s | byte equality observed |
| read.test | 10 | 0.001570s | 0.001490s | 0.001469s | byte equality observed |
| read_at.test | 10 | 0.002017s | 0.001919s | 0.001764s | byte equality observed |
| read_at_select.test | 10 | 0.001981s | 0.001913s | 0.001735s | byte equality observed |
| read_select.test | 10 | 0.001595s | 0.001505s | 0.001490s | byte equality observed |
| read_until.test | 10 | 0.001252s | 0.001216s | 0.001253s | byte equality observed |
| read_until_select.test | 10 | 0.001278s | 0.001229s | 0.001276s | byte equality observed |
| recycling_allocator.test | 10 | 0.001052s | 0.001045s | 0.001044s | byte equality observed |
| recycling_allocator_select.test | 10 | 0.001061s | 0.001052s | 0.001043s | byte equality observed |
| signal_set.test | 10 | 0.001113s | 0.001105s | 0.001080s | byte equality observed |
| signal_set_select.test | 10 | 0.001066s | 0.001075s | 0.001078s | byte equality observed |
| socket_base.test | 10 | 0.001212s | 0.001193s | 0.001179s | byte equality observed |
| socket_base_select.test | 10 | 0.001170s | 0.001158s | 0.001174s | byte equality observed |
| spawn.test | 10 | 0.001286s | 0.001301s | 0.001288s | byte equality observed |
| spawn_select.test | 10 | 0.001281s | 0.001275s | 0.001288s | byte equality observed |
| static_thread_pool.test | 10 | 0.001066s | 0.001058s | 0.001064s | byte equality observed |
| static_thread_pool_select.test | 10 | 0.001028s | 0.000995s | 0.001036s | byte equality observed |
| strand.test | 1 | 0.001927s | 0.001977s | 0.002017s | byte equality observed |
| strand_select.test | 1 | 0.001679s | 0.001651s | 0.001659s | byte equality observed |
| streambuf.test | 10 | 0.001120s | 0.001080s | 0.001099s | byte equality observed |
| streambuf_select.test | 10 | 0.001081s | 0.001053s | 0.001057s | byte equality observed |
| thread_pool.test | 10 | 0.002295s | 0.002266s | 0.002226s | byte equality observed |
| thread_pool_select.test | 8 | 0.002252s | 0.002287s | 0.002268s | byte equality observed |
| write.test | 10 | 0.001616s | 0.001542s | 0.001565s | byte equality observed |
| write_at.test | 10 | 0.001766s | 0.001639s | 0.001634s | byte equality observed |
| write_at_select.test | 10 | 0.001787s | 0.001635s | 0.001656s | byte equality observed |
| write_select.test | 10 | 0.001619s | 0.001508s | 0.001521s | byte equality observed |

Compiler command and timing observations: [cppgm](https://github.com/vishvananda/cppgm-extended/actions/runs/42916136104) · [gcc](https://github.com/vishvananda/cppgm-extended/actions/runs/42916136104) · [clang](https://github.com/vishvananda/cppgm-extended/actions/runs/42916136104).

[Reviewed outcomes and per-test records](../results.json) · [Host and toolchains](../metadata.json)
