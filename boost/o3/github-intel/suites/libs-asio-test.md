# libs/asio/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| any_completion_executor.test | RUN | 0.807389 | 4.724270 | 4.032959 | 0.002755 | 0.002523 | 0.002227 |
| any_completion_executor_select.test | RUN | 0.816912 | 4.743355 | 4.281732 | 0.002628 | 0.002115 | 0.002433 |
| any_completion_handler.test | RUN | 0.929875 | 4.316213 | 3.712347 | 0.002048 | 0.001984 | 0.002062 |
| any_completion_handler_select.test | RUN | 0.930320 | 4.437012 | 3.919110 | 0.001931 | 0.002032 | 0.002001 |
| any_io_executor.test | RUN | 0.851466 | 5.107311 | 4.476172 | 0.002422 | 0.002608 | 0.002312 |
| any_io_executor_select.test | RUN | 0.885218 | 5.004983 | 4.607839 | 0.002303 | 0.002476 | 0.002531 |
| append.test | RUN | 0.795336 | 4.071850 | 3.328642 | 0.001991 | 0.002032 | 0.001880 |
| append_select.test | RUN | 0.794853 | 4.180755 | 3.302737 | 0.001922 | 0.001952 | 0.001790 |
| as_tuple.test | RUN | 1.060273 | 4.937966 | 4.598625 | 0.002102 | 0.002076 | 0.001984 |
| as_tuple_select.test | RUN | 1.057204 | 4.851570 | 4.564165 | 0.001712 | 0.001972 | 0.001852 |
| associated_allocator.test | LINK | 0.217266 | 0.419952 | 0.482019 | — | — | — |
| associated_allocator_select.test | LINK | 0.210307 | 0.415304 | 0.494657 | — | — | — |
| associated_cancellation_slot.test | LINK | 0.227898 | 0.449805 | 0.539022 | — | — | — |
| associated_cancellation_slot_select.test | LINK | 0.233048 | 0.451179 | 0.538803 | — | — | — |
| associated_executor.test | LINK | 0.340763 | 0.769343 | 0.864001 | — | — | — |
| associated_executor_select.test | LINK | 0.337165 | 0.768607 | 0.881465 | — | — | — |
| associated_immediate_executor.test | LINK | 0.311511 | 0.721555 | 0.824933 | — | — | — |
| associated_immediate_executor_select.test | LINK | 0.317135 | 0.723820 | 0.783581 | — | — | — |
| associator.test | LINK | 0.170378 | 0.335058 | 0.401889 | — | — | — |
| associator_select.test | LINK | 0.170067 | 0.339021 | 0.412485 | — | — | — |
| awaitable.test | LINK | 0.171419 | 0.341519 | 0.408588 | — | — | — |
| awaitable_select.test | LINK | 0.171778 | 0.329863 | 0.400238 | — | — | — |
| basic_datagram_socket.test | LINK | 0.527058 | 1.074050 | 1.522397 | — | — | — |
| basic_datagram_socket_select.test | LINK | 0.537534 | 1.089169 | 1.565376 | — | — | — |
| basic_deadline_timer.test | LINK | 0.638731 | 1.277177 | 1.735686 | — | — | — |
| basic_deadline_timer_select.test | LINK | 0.656289 | 1.307753 | 1.776675 | — | — | — |
| basic_file.test | LINK | 0.170444 | 0.339467 | 0.405544 | — | — | — |
| basic_file_select.test | LINK | 0.172166 | 0.349847 | 0.407869 | — | — | — |
| basic_random_access_file.test | LINK | 0.176268 | 0.339398 | 0.408609 | — | — | — |
| basic_random_access_file_select.test | LINK | 0.172666 | 0.344740 | 0.407213 | — | — | — |
| basic_raw_socket.test | LINK | 0.530369 | 1.084398 | 1.531565 | — | — | — |
| basic_raw_socket_select.test | LINK | 0.527818 | 1.109299 | 1.563614 | — | — | — |
| basic_readable_pipe.test | LINK | 0.506641 | 1.025690 | 1.324116 | — | — | — |
| basic_readable_pipe_select.test | LINK | 0.533039 | 1.105368 | 1.604343 | — | — | — |
| basic_seq_packet_socket.test | LINK | 0.520482 | 1.078524 | 1.515314 | — | — | — |
| basic_seq_packet_socket_select.test | LINK | 0.543486 | 1.119300 | 1.558821 | — | — | — |
| basic_signal_set.test | LINK | 0.491501 | 0.989302 | 1.192270 | — | — | — |
| basic_signal_set_select.test | LINK | 0.513777 | 1.069888 | 1.470314 | — | — | — |
| basic_socket_acceptor.test | LINK | 0.537394 | 1.081372 | 1.545018 | — | — | — |
| basic_socket_acceptor_select.test | LINK | 0.548972 | 1.105106 | 1.575066 | — | — | — |
| basic_stream_file.test | LINK | 0.170600 | 0.348574 | 0.392307 | — | — | — |
| basic_stream_file_select.test | LINK | 0.172747 | 0.354810 | 0.394594 | — | — | — |
| basic_stream_socket.test | LINK | 0.536127 | 1.086450 | 1.512897 | — | — | — |
| basic_stream_socket_select.test | LINK | 0.544075 | 1.123794 | 1.557143 | — | — | — |
| basic_streambuf.test | LINK | 0.259122 | 0.506977 | 0.596046 | — | — | — |
| basic_streambuf_select.test | LINK | 0.267210 | 0.499903 | 0.594286 | — | — | — |
| basic_waitable_timer.test | LINK | 0.492448 | 1.009028 | 1.431607 | — | — | — |
| basic_waitable_timer_select.test | LINK | 0.508555 | 1.048986 | 1.451256 | — | — | — |
| basic_writable_pipe.test | LINK | 0.505328 | 1.035720 | 1.322925 | — | — | — |
| basic_writable_pipe_select.test | LINK | 0.539156 | 1.089558 | 1.604258 | — | — | — |
| bind_allocator.test | RUN | 0.854748 | 4.092184 | 3.599436 | 0.002185 | 0.002368 | 0.002563 |
| bind_allocator_select.test | RUN | 0.862220 | 4.193110 | 3.552489 | 0.002441 | 0.002217 | 0.002434 |
| bind_cancellation_slot.test | RUN | 0.897748 | 4.174097 | 3.656931 | 0.001500 | 0.001494 | 0.002160 |
| bind_cancellation_slot_select.test | RUN | 0.871049 | 4.147040 | 3.522491 | 0.001435 | 0.001456 | 0.001375 |
| bind_executor.test | RUN | 0.583991 | 2.794929 | 2.214135 | 0.002306 | 0.002541 | 0.002073 |
| bind_executor_select.test | RUN | 0.588588 | 3.010411 | 2.204860 | 0.001971 | 0.002446 | 0.001888 |
| bind_immediate_executor.test | RUN | 0.435878 | 2.488802 | 1.426004 | 0.001453 | 0.001335 | 0.001366 |
| bind_immediate_executor_select.test | RUN | 0.447115 | 2.309199 | 1.587125 | 0.001458 | 0.001432 | 0.001331 |
| buffer.test | RUN | 0.262318 | 0.920598 | 0.848609 | 0.001341 | 0.001307 | 0.001365 |
| buffer_select.test | RUN | 0.256637 | 0.927997 | 0.849131 | 0.001433 | 0.001312 | 0.001289 |
| buffer_registration.test | LINK | 0.205019 | 0.463345 | 0.493954 | — | — | — |
| buffer_registration_select.test | LINK | 0.207014 | 0.465927 | 0.492081 | — | — | — |
| buffered_read_stream.test | RUN | 0.986631 | 6.684159 | 5.627901 | 0.001786 | 0.002140 | 0.002180 |
| buffered_read_stream_select.test | RUN | 0.997088 | 6.580084 | 5.506156 | 0.001764 | 0.002314 | 0.002130 |
| buffered_stream.test | RUN | 1.765785 | 10.041218 | 9.124476 | 0.002344 | 0.002328 | 0.002350 |
| buffered_stream_select.test | RUN | 1.873315 | 10.129341 | 8.877299 | 0.002194 | 0.002367 | 0.002261 |
| buffered_write_stream.test | RUN | 1.683236 | 8.649848 | 7.819811 | 0.002401 | 0.002092 | 0.002156 |
| buffered_write_stream_select.test | RUN | 1.692096 | 8.757722 | 7.516844 | 0.002292 | 0.002051 | 0.002234 |
| buffers_iterator.test | RUN | 0.384270 | 0.805449 | 1.232487 | 0.001754 | 0.001752 | 0.002018 |
| buffers_iterator_select.test | RUN | 0.380498 | 0.787345 | 1.228191 | 0.001926 | 0.001821 | 0.001722 |
| cancel_after.test | RUN | 1.556331 | 6.752022 | 5.769871 | 0.004435 | 0.004331 | 0.004371 |
| cancel_after_select.test | RUN | 1.571523 | 6.699837 | 5.736740 | 0.004645 | 0.004228 | 0.004416 |
| cancel_at.test | RUN | 1.535666 | 5.645041 | 5.258345 | 0.003720 | 0.003786 | 0.003754 |
| cancel_at_select.test | RUN | 1.413710 | 5.713151 | 5.151044 | 0.003977 | 0.003498 | 0.003214 |
| cancellation_signal.test | LINK | 0.141988 | 0.264214 | 0.317216 | — | — | — |
| cancellation_signal_select.test | LINK | 0.146091 | 0.267932 | 0.312820 | — | — | — |
| cancellation_state.test | LINK | 0.222918 | 0.283055 | 0.311376 | — | — | — |
| cancellation_state_select.test | LINK | 0.218150 | 0.265857 | 0.312463 | — | — | — |
| cancellation_type.test | LINK | 0.169672 | 0.210888 | 0.289035 | — | — | — |
| cancellation_type_select.test | LINK | 0.169640 | 0.214068 | 0.317210 | — | — | — |
| co_composed.test | RUN | 0.170556 | 0.213074 | 0.405791 | 0.001503 | 0.001868 | 0.001628 |
| co_composed_select.test | RUN | 0.169544 | 0.230949 | 0.398495 | 0.001748 | 0.001327 | 0.001627 |
| co_spawn.test | LINK | 0.169373 | 0.230943 | 0.405192 | — | — | — |
| co_spawn_select.test | LINK | 0.174526 | 0.227457 | 0.403184 | — | — | — |
| completion_condition.test | LINK | 0.219265 | 0.420735 | 0.523035 | — | — | — |
| completion_condition_select.test | LINK | 0.219028 | 0.517142 | 0.549556 | — | — | — |
| compose.test | RUN | 1.062714 | 5.135237 | 4.587753 | 0.002328 | 0.002149 | 0.002019 |
| compose_select.test | RUN | 1.062511 | 5.130384 | 4.558156 | 0.002314 | 0.001861 | 0.002290 |
| composed.test | RUN | 0.987917 | 5.136940 | 4.482061 | 0.002002 | 0.002080 | 0.001917 |
| composed_select.test | RUN | 0.995488 | 4.970495 | 4.437171 | 0.001940 | 0.001972 | 0.001867 |
| config.test | RUN | 0.478303 | 3.282772 | 1.952312 | 0.001921 | 0.001840 | 0.001896 |
| config_select.test | RUN | 0.509558 | 3.367076 | 2.254345 | 0.001624 | 0.001968 | 0.001820 |
| connect.test | LINK | 1.853900 | 10.856064 | 9.345274 | — | — | — |
| connect_select.test | LINK | 1.826492 | 10.840373 | 9.383751 | — | — | — |
| connect_pipe.test | RUN | 0.913402 | 4.151436 | 3.507093 | 0.002078 | 0.002083 | 0.001905 |
| connect_pipe_select.test | RUN | 0.939750 | 4.341143 | 3.743954 | 0.001539 | 0.002080 | 0.001849 |
| consign.test | RUN | 0.780870 | 4.074021 | 3.279601 | 0.002080 | 0.002074 | 0.001816 |
| consign_select.test | RUN | 0.791561 | 4.144076 | 3.271876 | 0.001854 | 0.002030 | 0.001741 |
| coroutine.test | LINK | 0.180338 | 0.405980 | 0.453590 | — | — | — |
| coroutine_select.test | LINK | 0.181341 | 0.405527 | 0.440051 | — | — | — |
| deadline_timer.test | RUN | 1.191057 | 5.317611 | 4.616327 | 0.002851 | 0.003537 | 0.003299 |
| deadline_timer_select.test | RUN | 1.169198 | 5.314865 | 4.585548 | 0.003433 | 0.003089 | 0.003451 |
| deferred.test | RUN | 0.190242 | 0.364413 | 0.422565 | 0.001631 | 0.001885 | 0.001768 |
| deferred_select.test | RUN | 0.196868 | 0.364290 | 0.422222 | 0.001732 | 0.001865 | 0.001695 |
| detached.test | LINK | 0.212149 | 0.402779 | 0.472369 | — | — | — |
| detached_select.test | LINK | 0.214076 | 0.407448 | 0.479298 | — | — | — |
| disposition.test | RUN | 0.296020 | 0.994310 | 0.900673 | 0.001301 | 0.001974 | 0.001899 |
| disposition_select.test | RUN | 0.264832 | 0.983143 | 0.897974 | 0.001357 | 0.001944 | 0.001849 |
| error.test | RUN | 0.178079 | 0.947280 | 0.799511 | 0.001306 | 0.001932 | 0.001794 |
| error_select.test | RUN | 0.177518 | 0.950319 | 0.795543 | 0.001351 | 0.001947 | 0.001770 |
| file_base.test | LINK | 0.221810 | 0.781976 | 0.864601 | — | — | — |
| file_base_select.test | LINK | 0.220797 | 0.787585 | 0.827261 | — | — | — |
| fwd_first.test | LINK | 0.516996 | 1.276922 | 1.539059 | — | — | — |
| fwd_last.test | LINK | 0.512729 | 1.160475 | 1.520490 | — | — | — |
| generic_basic_endpoint.test | LINK | 0.190521 | 0.437004 | 0.551987 | — | — | — |
| generic_basic_endpoint_select.test | LINK | 0.192573 | 0.434821 | 0.547443 | — | — | — |
| generic_datagram_protocol.test | LINK | 0.837237 | 4.057288 | 3.300947 | — | — | — |
| generic_datagram_protocol_select.test | LINK | 0.827354 | 4.216064 | 3.239507 | — | — | — |
| generic_raw_protocol.test | LINK | 0.840942 | 4.062091 | 3.296562 | — | — | — |
| generic_raw_protocol_select.test | LINK | 0.824296 | 4.233489 | 3.233886 | — | — | — |
| generic_seq_packet_protocol.test | LINK | 0.711023 | 3.409543 | 2.794374 | — | — | — |
| generic_seq_packet_protocol_select.test | LINK | 0.705439 | 3.488856 | 2.740477 | — | — | — |
| generic_stream_protocol.test | LINK | 0.754750 | 3.679809 | 2.968809 | — | — | — |
| generic_stream_protocol_select.test | LINK | 0.751576 | 3.803570 | 2.991160 | — | — | — |
| high_resolution_timer.test | LINK | 0.323361 | 0.644590 | 0.862725 | — | — | — |
| high_resolution_timer_select.test | LINK | 0.328645 | 0.674636 | 0.885112 | — | — | — |
| immediate.test | LINK | 0.221698 | 0.499137 | 0.529368 | — | — | — |
| immediate_select.test | LINK | 0.220596 | 0.499229 | 0.532933 | — | — | — |
| inline_executor.test | RUN | 0.269800 | 0.785587 | 0.812287 | 0.001416 | 0.001391 | 0.001280 |
| inline_executor_select.test | RUN | 0.268738 | 0.790798 | 0.819766 | 0.001298 | 0.001390 | 0.001329 |
| inline_or_executor.test | RUN | 0.459446 | 2.378304 | 1.634723 | 0.001422 | 0.001460 | 0.001329 |
| inline_or_executor_select.test | RUN | 0.470471 | 2.421329 | 1.762143 | 0.001334 | 0.001389 | 0.001370 |
| io_context.test | RUN | 0.674011 | 3.647534 | 2.870451 | 0.002530 | 0.002593 | 0.002477 |
| io_context_select.test | RUN | 0.673846 | 3.556831 | 2.834842 | 0.002386 | 0.002353 | 0.002286 |
| io_context_strand.test | RUN | 0.636246 | 4.427444 | 2.466769 | 0.002397 | 0.002752 | 0.002386 |
| io_context_strand_select.test | RUN | 0.638027 | 4.528584 | 2.476781 | 0.002186 | 0.002380 | 0.002281 |
| ip_address.test | LINK | 0.225555 | 1.087182 | 0.705423 | — | — | — |
| ip_address_select.test | LINK | 0.223759 | 1.096478 | 0.709908 | — | — | — |
| ip_address_v4.test | LINK | 0.230121 | 1.150262 | 0.754985 | — | — | — |
| ip_address_v4_select.test | LINK | 0.233675 | 1.144269 | 0.749538 | — | — | — |
| ip_address_v6.test | LINK | 0.258334 | 1.485395 | 0.930466 | — | — | — |
| ip_address_v6_select.test | LINK | 0.259763 | 1.465381 | 0.920998 | — | — | — |
| ip_basic_endpoint.test | LINK | 0.199158 | 0.535048 | 0.596043 | — | — | — |
| ip_basic_endpoint_select.test | LINK | 0.198688 | 0.530632 | 0.594579 | — | — | — |
| ip_basic_resolver.test | LINK | 0.326014 | 0.659476 | 0.883977 | — | — | — |
| ip_basic_resolver_select.test | LINK | 0.329014 | 0.670510 | 0.904937 | — | — | — |
| ip_basic_resolver_entry.test | LINK | 0.108760 | 0.211098 | 0.250751 | — | — | — |
| ip_basic_resolver_entry_select.test | LINK | 0.108705 | 0.209514 | 0.249782 | — | — | — |
| ip_basic_resolver_iterator.test | LINK | 0.197179 | 0.440004 | 0.577671 | — | — | — |
| ip_basic_resolver_iterator_select.test | LINK | 0.194967 | 0.438688 | 0.570259 | — | — | — |
| ip_basic_resolver_query.test | LINK | 0.188445 | 0.425141 | 0.571602 | — | — | — |
| ip_basic_resolver_query_select.test | LINK | 0.185234 | 0.426234 | 0.570364 | — | — | — |
| ip_host_name.test | RUN | 0.198396 | 0.581612 | 0.751683 | 0.001316 | 0.001318 | 0.001482 |
| ip_host_name_select.test | RUN | 0.200427 | 0.573441 | 0.722246 | 0.001219 | 0.001227 | 0.001593 |
| ip_icmp.test | RUN | 1.060188 | 6.402320 | 6.943588 | 0.001430 | 0.001852 | 0.001520 |
| ip_icmp_select.test | RUN | 1.067080 | 6.810873 | 6.732967 | 0.001344 | 0.001920 | 0.001659 |
| ip_multicast.test | RUN | 0.599939 | 4.322536 | 3.837469 | 0.001552 | 0.002168 | 0.001840 |
| ip_multicast_select.test | RUN | 0.606238 | 4.340956 | 3.830699 | 0.001502 | 0.002068 | 0.001761 |
| ip_resolver_query_base.test | LINK | 0.128055 | 0.378216 | 0.424815 | — | — | — |
| ip_resolver_query_base_select.test | LINK | 0.130069 | 0.371117 | 0.429950 | — | — | — |
| ip_tcp.test | RUN | 1.686658 | 12.872470 | 10.529651 | 0.001966 | 0.002573 | 0.002505 |
| ip_tcp_select.test | RUN | 1.669524 | 13.258197 | 10.227159 | 0.001710 | 0.002526 | 0.002277 |
| ip_udp.test | RUN | 1.116698 | 9.151104 | 7.310584 | 0.001633 | 0.002103 | 0.002010 |
| ip_udp_select.test | RUN | 1.132047 | 9.390495 | 7.148844 | 0.001544 | 0.002204 | 0.002044 |
| ip_unicast.test | RUN | 0.657203 | 3.998671 | 3.323891 | 0.002000 | 0.002008 | 0.001874 |
| ip_unicast_select.test | RUN | 0.685712 | 4.025360 | 3.309468 | 0.001978 | 0.001992 | 0.001852 |
| ip_v6_only.test | RUN | 0.854385 | 4.094314 | 3.441226 | 0.001759 | 0.001944 | 0.001589 |
| ip_v6_only_select.test | RUN | 0.860142 | 4.209138 | 3.405857 | 0.001960 | 0.002010 | 0.001926 |
| is_read_buffered.test | RUN | 0.594265 | 1.214541 | 1.763532 | 0.001774 | 0.001822 | 0.001624 |
| is_read_buffered_select.test | RUN | 0.597979 | 1.267835 | 1.809172 | 0.001838 | 0.001842 | 0.001772 |
| is_write_buffered.test | RUN | 0.582288 | 1.217165 | 1.779934 | 0.001670 | 0.001813 | 0.001531 |
| is_write_buffered_select.test | RUN | 0.590377 | 1.236182 | 1.818133 | 0.001537 | 0.001916 | 0.001812 |
| local_basic_endpoint.test | LINK | 0.293146 | 0.672774 | 0.910302 | — | — | — |
| local_basic_endpoint_select.test | LINK | 0.295329 | 0.670783 | 0.930132 | — | — | — |
| local_connect_pair.test | LINK | 0.770896 | 3.991621 | 3.193594 | — | — | — |
| local_connect_pair_select.test | LINK | 0.778088 | 4.003372 | 3.154973 | — | — | — |
| local_datagram_protocol.test | LINK | 1.067910 | 5.253176 | 4.506060 | — | — | — |
| local_datagram_protocol_select.test | LINK | 1.082018 | 5.311816 | 4.391946 | — | — | — |
| local_seq_packet_protocol.test | LINK | 1.062721 | 5.161605 | 4.458266 | — | — | — |
| local_seq_packet_protocol_select.test | LINK | 1.068627 | 5.192389 | 4.423516 | — | — | — |
| local_stream_protocol.test | LINK | 1.003401 | 4.921676 | 4.191249 | — | — | — |
| local_stream_protocol_select.test | LINK | 1.015167 | 4.959411 | 4.136779 | — | — | — |
| placeholders.test | LINK | 0.190279 | 0.368015 | 0.437966 | — | — | — |
| placeholders_select.test | LINK | 0.187432 | 0.370269 | 0.436930 | — | — | — |
| posix_basic_descriptor.test | LINK | 0.502109 | 1.021000 | 1.317272 | — | — | — |
| posix_basic_descriptor_select.test | LINK | 0.512275 | 1.080250 | 1.599102 | — | — | — |
| posix_basic_stream_descriptor.test | LINK | 0.507627 | 1.032703 | 1.323277 | — | — | — |
| posix_basic_stream_descriptor_select.test | LINK | 0.527163 | 1.071405 | 1.601361 | — | — | — |
| posix_descriptor_base.test | LINK | 0.199298 | 0.392499 | 0.428567 | — | — | — |
| posix_descriptor_base_select.test | LINK | 0.206056 | 0.394965 | 0.433733 | — | — | — |
| posix_stream_descriptor.test | LINK | 1.021808 | 5.053553 | 4.217828 | — | — | — |
| posix_stream_descriptor_select.test | LINK | 1.033921 | 5.090223 | 4.301817 | — | — | — |
| prepend.test | RUN | 0.786323 | 4.068145 | 3.325739 | 0.001877 | 0.002118 | 0.001912 |
| prepend_select.test | RUN | 0.791060 | 4.173302 | 3.322341 | 0.001837 | 0.001968 | 0.001806 |
| random_access_file.test | LINK | 0.461128 | 0.918383 | 1.079075 | — | — | — |
| random_access_file_select.test | LINK | 0.476159 | 0.993182 | 1.349760 | — | — | — |
| read.test | RUN | 2.513845 | 24.828320 | 19.305365 | 0.002664 | 0.002503 | 0.002485 |
| read_select.test | RUN | 2.560996 | 24.751631 | 19.540111 | 0.002639 | 0.002425 | 0.002502 |
| read_at.test | RUN | 2.516327 | 33.101798 | 26.193479 | 0.003138 | 0.003173 | 0.002941 |
| read_at_select.test | RUN | 2.518243 | 32.883542 | 26.333472 | 0.003562 | 0.003136 | 0.003004 |
| read_until.test | RUN | 1.377199 | 9.596718 | 7.100237 | 0.001955 | 0.002322 | 0.002030 |
| read_until_select.test | RUN | 1.399969 | 9.735919 | 7.307009 | 0.002805 | 0.002118 | 0.002059 |
| readable_pipe.test | LINK | 0.816223 | 3.993328 | 3.272158 | — | — | — |
| readable_pipe_select.test | LINK | 0.835852 | 4.214086 | 3.509044 | — | — | — |
| recycling_allocator.test | RUN | 0.331440 | 0.582727 | 0.633250 | 0.003301 | 0.001810 | 0.001758 |
| recycling_allocator_select.test | RUN | 0.338923 | 0.574691 | 0.640136 | 0.001707 | 0.001722 | 0.001790 |
| redirect_disposition.test | LINK | 1.215780 | 5.589763 | 4.949720 | — | — | — |
| redirect_disposition_select.test | LINK | 1.199506 | 5.527950 | 4.914230 | — | — | — |
| redirect_error.test | LINK | 0.975651 | 4.642509 | 4.333424 | — | — | — |
| redirect_error_select.test | LINK | 0.972619 | 4.654327 | 4.319107 | — | — | — |
| registered_buffer.test | LINK | 0.245895 | 0.470950 | 0.569105 | — | — | — |
| registered_buffer_select.test | LINK | 0.244178 | 0.469148 | 0.575847 | — | — | — |
| signal_set.test | RUN | 0.771572 | 3.905539 | 3.071247 | 0.001894 | 0.001828 | 0.001711 |
| signal_set_select.test | RUN | 0.794831 | 4.044566 | 3.369701 | 0.001738 | 0.001810 | 0.001786 |
| signal_set_base.test | LINK | 0.189635 | 0.374323 | 0.430369 | — | — | — |
| signal_set_base_select.test | LINK | 0.190315 | 0.370516 | 0.422706 | — | — | — |
| socket_base.test | RUN | 0.990903 | 5.764632 | 4.605546 | 0.001910 | 0.002226 | 0.001942 |
| socket_base_select.test | RUN | 0.999530 | 5.591908 | 4.587108 | 0.001681 | 0.002004 | 0.001955 |
| spawn.test | RUN | 1.629980 | 9.382558 | 7.890757 | 0.002453 | 0.002155 | 0.002215 |
| spawn_select.test | RUN | 1.640718 | 9.364445 | 7.915798 | 0.002008 | 0.002236 | 0.002168 |
| static_thread_pool.test | RUN | 0.429075 | 0.880387 | 1.020155 | 0.001926 | 0.001756 | 0.001752 |
| static_thread_pool_select.test | RUN | 0.448903 | 0.933899 | 1.295501 | 0.001700 | 0.001767 | 0.001672 |
| steady_timer.test | LINK | 0.476525 | 0.990193 | 1.391306 | — | — | — |
| steady_timer_select.test | LINK | 0.483292 | 1.021802 | 1.439163 | — | — | — |
| strand.test | RUN | 1.039946 | 5.304214 | 4.845066 | 0.002685 | 0.002814 | 0.002532 |
| strand_select.test | RUN | 1.059231 | 5.225862 | 4.670815 | 0.002343 | 0.002616 | 0.002219 |
| stream_file.test | LINK | 0.450337 | 0.922726 | 1.056828 | — | — | — |
| stream_file_select.test | LINK | 0.471159 | 0.969821 | 1.356396 | — | — | — |
| streambuf.test | RUN | 0.293577 | 0.609296 | 0.753953 | 0.001307 | 0.001330 | 0.001690 |
| streambuf_select.test | RUN | 0.258742 | 0.494584 | 0.765112 | 0.001204 | 0.001356 | 0.001870 |
| system_timer.test | LINK | 0.664752 | 3.328872 | 3.091979 | — | — | — |
| system_timer_select.test | LINK | 0.669700 | 3.254668 | 2.776873 | — | — | — |
| system_context.test | LINK | 0.280580 | 0.560968 | 0.627799 | — | — | — |
| system_context_select.test | LINK | 0.301314 | 0.597224 | 0.792772 | — | — | — |
| system_executor.test | LINK | 0.379298 | 1.888142 | 1.272096 | — | — | — |
| system_executor_select.test | LINK | 0.389859 | 1.834937 | 1.423279 | — | — | — |
| this_coro.test | LINK | 0.106627 | 0.209498 | 0.246571 | — | — | — |
| this_coro_select.test | LINK | 0.107963 | 0.212958 | 0.245974 | — | — | — |
| thread_pool.test | RUN | 0.461959 | 3.031583 | 1.678751 | 0.002717 | 0.003008 | 0.002508 |
| thread_pool_select.test | RUN | 0.473145 | 3.475083 | 1.840823 | 0.002504 | 0.003168 | 0.002518 |
| time_traits.test | LINK | 0.246310 | 0.699422 | 0.695255 | — | — | — |
| time_traits_select.test | LINK | 0.244106 | 0.699582 | 0.591792 | — | — | — |
| ts_buffer.test | LINK | 0.231910 | 0.773019 | 0.759372 | — | — | — |
| ts_buffer_select.test | LINK | 0.227841 | 0.771582 | 0.896454 | — | — | — |
| ts_executor.test | LINK | 0.328057 | 1.066691 | 1.384838 | — | — | — |
| ts_executor_select.test | LINK | 0.341628 | 1.123362 | 1.674757 | — | — | — |
| ts_internet.test | LINK | 0.361892 | 1.178507 | 1.715574 | — | — | — |
| ts_internet_select.test | LINK | 0.364033 | 1.206884 | 1.759560 | — | — | — |
| ts_io_context.test | LINK | 0.292364 | 0.905080 | 1.050689 | — | — | — |
| ts_io_context_select.test | LINK | 0.305200 | 0.956690 | 1.334381 | — | — | — |
| ts_net.test | LINK | 0.433905 | 1.421875 | 2.098180 | — | — | — |
| ts_net_select.test | LINK | 0.447234 | 1.445842 | 2.162399 | — | — | — |
| ts_netfwd.test | LINK | 0.437340 | 1.422693 | 2.123396 | — | — | — |
| ts_netfwd_select.test | LINK | 0.448104 | 1.453409 | 2.174713 | — | — | — |
| ts_socket.test | LINK | 0.356516 | 1.116378 | 1.625030 | — | — | — |
| ts_socket_select.test | LINK | 0.360478 | 1.135474 | 1.636862 | — | — | — |
| ts_timer.test | LINK | 0.316359 | 0.992465 | 1.414609 | — | — | — |
| ts_timer_select.test | LINK | 0.321415 | 1.036983 | 1.447703 | — | — | — |
| use_awaitable.test | LINK | 0.105868 | 0.340490 | 0.403949 | — | — | — |
| use_awaitable_select.test | LINK | 0.105774 | 0.329919 | 0.400401 | — | — | — |
| wait_traits.test | LINK | 0.107554 | 0.332082 | 0.398923 | — | — | — |
| wait_traits_select.test | LINK | 0.108997 | 0.338717 | 0.403000 | — | — | — |
| windows_basic_object_handle.test | LINK | 0.106516 | 0.335135 | 0.396653 | — | — | — |
| windows_basic_object_handle_select.test | LINK | 0.149315 | 0.327386 | 0.396439 | — | — | — |
| windows_basic_overlapped_handle.test | LINK | 0.159779 | 0.330787 | 0.394980 | — | — | — |
| windows_basic_overlapped_handle_select.test | LINK | 0.164165 | 0.339883 | 0.400010 | — | — | — |
| windows_basic_random_access_handle.test | LINK | 0.166521 | 0.329957 | 0.403969 | — | — | — |
| windows_basic_random_access_handle_select.test | LINK | 0.168720 | 0.330285 | 0.401289 | — | — | — |
| windows_basic_stream_handle.test | LINK | 0.166755 | 0.339044 | 0.408484 | — | — | — |
| windows_basic_stream_handle_select.test | LINK | 0.166267 | 0.335604 | 0.402575 | — | — | — |
| windows_object_handle.test | LINK | 0.452094 | 0.915229 | 1.092657 | — | — | — |
| windows_object_handle_select.test | LINK | 0.469621 | 0.970240 | 1.363771 | — | — | — |
| windows_overlapped_ptr.test | LINK | 0.464506 | 0.956372 | 1.184974 | — | — | — |
| windows_overlapped_ptr_select.test | LINK | 0.485638 | 1.022997 | 1.470947 | — | — | — |
| windows_random_access_handle.test | LINK | 0.447789 | 0.911351 | 1.061825 | — | — | — |
| windows_random_access_handle_select.test | LINK | 0.471205 | 0.960130 | 1.324409 | — | — | — |
| windows_stream_handle.test | LINK | 0.449028 | 0.916154 | 1.060071 | — | — | — |
| windows_stream_handle_select.test | LINK | 0.470342 | 0.979576 | 1.326423 | — | — | — |
| writable_pipe.test | LINK | 0.812465 | 4.031096 | 3.429420 | — | — | — |
| writable_pipe_select.test | LINK | 0.832953 | 4.182849 | 3.582125 | — | — | — |
| write.test | RUN | 2.465038 | 20.539388 | 18.946722 | 0.002678 | 0.002561 | 0.002579 |
| write_select.test | RUN | 2.498721 | 20.675278 | 19.084518 | 0.002485 | 0.002630 | 0.002645 |
| write_at.test | RUN | 2.570549 | 28.356038 | 23.847603 | 0.003006 | 0.002706 | 0.002466 |
| write_at_select.test | RUN | 2.593009 | 28.264027 | 24.325059 | 0.002949 | 0.002771 | 0.002664 |
| any_executor.test | RUN | 0.979486 | 9.478858 | 7.675425 | 0.002622 | 0.002770 | 0.002796 |
| blocking.test | RUN | 2.228306 | 12.106509 | 15.540415 | 0.005160 | 0.005254 | 0.005250 |
| blocking_adaptation.test | RUN | 1.117140 | 5.311663 | 7.144052 | 0.003313 | 0.003373 | 0.003298 |
| context_as.test | RUN | 0.648935 | 3.685319 | 2.398196 | 0.001935 | 0.002057 | 0.002026 |
| executor.test | RUN | 0.175285 | 0.352904 | 0.432310 | 0.001801 | 0.001780 | 0.001824 |
| inline_exception_handling.test | RUN | 2.153800 | 11.302473 | 15.727835 | 0.003490 | 0.003374 | 0.003450 |
| invocable_archetype.test | RUN | 0.171739 | 0.346810 | 0.402632 | 0.001878 | 0.001725 | 0.001827 |
| mapping.test | RUN | 2.081971 | 11.379069 | 15.080143 | 0.003672 | 0.003526 | 0.003577 |
| outstanding_work.test | RUN | 1.003208 | 4.911670 | 6.842265 | 0.003312 | 0.003309 | 0.003214 |
| prefer_only.test | RUN | 0.360965 | 1.988511 | 1.523131 | 0.001788 | 0.001851 | 0.001673 |
| relationship.test | RUN | 0.819736 | 4.214133 | 6.885078 | 0.002216 | 0.002250 | 0.003312 |

## Native runtime replay

Child CPU of successful native tests, with process startup included; GDB and expected-failure executions are correctness-only. Two warmups per compiler, four A/A observations per reference and mirrored ABBA/BAAB blocks. Short tests repeat up to ten times, identically across compilers; CPU is divided by repetitions before summing tests. Executable hashes and output checks stay outside process timers. All runs must pass their original native oracle. Output hashes record equality or variation across the complete campaign; test reports may print times or addresses.

| Reference | CPU speedup ↑ | Paired blocks | A/A speedup |
|---|---:|---|---:|
| gcc | 0.984× | 0.987×, 0.982× | 1.004× |
| clang | 0.977× | 0.979×, 0.975× | 1.005× |

† A/A drift or paired-block spread exceeds 5%; small differences are unresolved. 1375 native tests in the full cohort; 1360 have byte-identical output throughout. [Manifest](https://github.com/vishvananda/cppgm-extended/actions/runs/42770605980), [paired observations](https://github.com/vishvananda/cppgm-extended/actions/runs/42770605980) and [per-test summaries](https://github.com/vishvananda/cppgm-extended/actions/runs/42770605980).

| Test | Repeats | CPU cppgm | GCC | Clang | Output check |
|---|---:|---:|---:|---:|---|
| any_completion_executor.test | 2 | 0.001758s | 0.001721s | 0.001762s | byte equality observed |
| any_completion_executor_select.test | 4 | 0.001740s | 0.001719s | 0.001715s | byte equality observed |
| any_completion_handler.test | 5 | 0.001422s | 0.001376s | 0.001357s | byte equality observed |
| any_completion_handler_select.test | 6 | 0.001402s | 0.001390s | 0.001339s | byte equality observed |
| any_io_executor.test | 4 | 0.001717s | 0.001669s | 0.001647s | byte equality observed |
| any_io_executor_select.test | 4 | 0.001725s | 0.001653s | 0.001608s | byte equality observed |
| append.test | 10 | 0.001338s | 0.001321s | 0.001301s | byte equality observed |
| append_select.test | 9 | 0.001316s | 0.001305s | 0.001284s | byte equality observed |
| as_tuple.test | 9 | 0.001405s | 0.001376s | 0.001371s | byte equality observed |
| as_tuple_select.test | 8 | 0.001349s | 0.001284s | 0.001303s | byte equality observed |
| bind_allocator.test | 1 | 0.001785s | 0.001787s | 0.001773s | byte equality observed |
| bind_allocator_select.test | 1 | 0.001559s | 0.001443s | 0.001438s | byte equality observed |
| bind_cancellation_slot.test | 10 | 0.001180s | 0.001175s | 0.001156s | byte equality observed |
| bind_cancellation_slot_select.test | 10 | 0.001099s | 0.001078s | 0.001075s | byte equality observed |
| bind_executor.test | 1 | 0.001750s | 0.001701s | 0.001765s | byte equality observed |
| bind_executor_select.test | 1 | 0.001937s | 0.001948s | 0.001797s | byte equality observed |
| bind_immediate_executor.test | 10 | 0.001172s | 0.001199s | 0.001165s | byte equality observed |
| bind_immediate_executor_select.test | 10 | 0.001152s | 0.001193s | 0.001200s | byte equality observed |
| buffer.test | 10 | 0.001140s | 0.001118s | 0.001112s | byte equality observed |
| buffer_select.test | 10 | 0.001105s | 0.001097s | 0.001093s | byte equality observed |
| buffered_read_stream.test | 10 | 0.001369s | 0.001381s | 0.001361s | byte equality observed |
| buffered_read_stream_select.test | 10 | 0.001362s | 0.001374s | 0.001383s | byte equality observed |
| buffered_stream.test | 10 | 0.001368s | 0.001350s | 0.001305s | byte equality observed |
| buffered_stream_select.test | 8 | 0.001324s | 0.001357s | 0.001366s | byte equality observed |
| buffered_write_stream.test | 10 | 0.001346s | 0.001352s | 0.001360s | byte equality observed |
| buffered_write_stream_select.test | 10 | 0.001339s | 0.001344s | 0.001348s | byte equality observed |
| buffers_iterator.test | 10 | 0.001092s | 0.001093s | 0.001076s | byte equality observed |
| buffers_iterator_select.test | 10 | 0.001102s | 0.001061s | 0.001078s | byte equality observed |
| cancel_after.test | 1 | 0.003705s | 0.003639s | 0.003666s | byte equality observed |
| cancel_after_select.test | 1 | 0.003205s | 0.003035s | 0.003053s | byte equality observed |
| cancel_at.test | 1 | 0.003890s | 0.003689s | 0.003806s | byte equality observed |
| cancel_at_select.test | 1 | 0.003793s | 0.003574s | 0.003332s | byte equality observed |
| co_composed.test | 10 | 0.001127s | 0.001104s | 0.001122s | byte equality observed |
| co_composed_select.test | 10 | 0.001109s | 0.001067s | 0.001123s | byte equality observed |
| compose.test | 1 | 0.001822s | 0.001709s | 0.001671s | byte equality observed |
| compose_select.test | 1 | 0.001501s | 0.001419s | 0.001515s | byte equality observed |
| composed.test | 1 | 0.002013s | 0.001923s | 0.001871s | byte equality observed |
| composed_select.test | 1 | 0.001654s | 0.001659s | 0.001527s | byte equality observed |
| config.test | 10 | 0.001200s | 0.001198s | 0.001144s | byte equality observed |
| config_select.test | 10 | 0.001174s | 0.001192s | 0.001163s | byte equality observed |
| connect_pipe.test | 10 | 0.001178s | 0.001137s | 0.001174s | byte equality observed |
| connect_pipe_select.test | 10 | 0.001180s | 0.001154s | 0.001132s | byte equality observed |
| consign.test | 10 | 0.001157s | 0.001153s | 0.001143s | byte equality observed |
| consign_select.test | 10 | 0.001145s | 0.001124s | 0.001125s | byte equality observed |
| deadline_timer.test | 1 | 0.002458s | 0.002457s | 0.002450s | byte equality observed |
| deadline_timer_select.test | 1 | 0.002298s | 0.002208s | 0.002220s | byte equality observed |
| deferred.test | 10 | 0.001047s | 0.001056s | 0.001043s | byte equality observed |
| deferred_select.test | 10 | 0.000981s | 0.000968s | 0.000982s | byte equality observed |
| disposition.test | 10 | 0.001051s | 0.001039s | 0.001028s | byte equality observed |
| disposition_select.test | 10 | 0.001042s | 0.001036s | 0.001030s | byte equality observed |
| error.test | 10 | 0.001016s | 0.001003s | 0.000993s | byte equality observed |
| error_select.test | 10 | 0.001020s | 0.001012s | 0.000999s | byte equality observed |
| inline_executor.test | 10 | 0.001028s | 0.001022s | 0.001016s | byte equality observed |
| inline_executor_select.test | 10 | 0.001017s | 0.001028s | 0.001009s | byte equality observed |
| inline_or_executor.test | 10 | 0.001040s | 0.001020s | 0.001017s | byte equality observed |
| inline_or_executor_select.test | 10 | 0.001040s | 0.001036s | 0.001014s | byte equality observed |
| io_context.test | 1 | 0.002068s | 0.002011s | 0.002066s | byte equality observed |
| io_context_select.test | 1 | 0.002226s | 0.002254s | 0.002282s | byte equality observed |
| io_context_strand.test | 1 | 0.002307s | 0.002277s | 0.002255s | byte equality observed |
| io_context_strand_select.test | 1 | 0.001992s | 0.001955s | 0.002014s | byte equality observed |
| ip_host_name.test | 10 | 0.001074s | 0.001067s | 0.001040s | byte equality observed |
| ip_host_name_select.test | 10 | 0.000994s | 0.001006s | 0.000965s | byte equality observed |
| ip_icmp.test | 10 | 0.001039s | 0.001042s | 0.000990s | byte equality observed |
| ip_icmp_select.test | 10 | 0.001041s | 0.001044s | 0.000994s | byte equality observed |
| ip_multicast.test | 10 | 0.001118s | 0.001117s | 0.001076s | byte equality observed |
| ip_multicast_select.test | 10 | 0.001094s | 0.001113s | 0.001067s | byte equality observed |
| ip_tcp.test | 10 | 0.001416s | 0.001388s | 0.001384s | byte equality observed |
| ip_tcp_select.test | 10 | 0.001351s | 0.001332s | 0.001326s | byte equality observed |
| ip_udp.test | 10 | 0.001140s | 0.001113s | 0.001100s | byte equality observed |
| ip_udp_select.test | 10 | 0.001120s | 0.001119s | 0.001099s | byte equality observed |
| ip_unicast.test | 10 | 0.001079s | 0.001075s | 0.001065s | byte equality observed |
| ip_unicast_select.test | 10 | 0.001074s | 0.001062s | 0.001052s | byte equality observed |
| ip_v6_only.test | 10 | 0.001086s | 0.001078s | 0.001062s | byte equality observed |
| ip_v6_only_select.test | 10 | 0.001060s | 0.001069s | 0.001045s | byte equality observed |
| is_read_buffered.test | 10 | 0.000999s | 0.001026s | 0.000980s | byte equality observed |
| is_read_buffered_select.test | 10 | 0.000998s | 0.001114s | 0.000989s | byte equality observed |
| is_write_buffered.test | 10 | 0.000997s | 0.001045s | 0.000979s | byte equality observed |
| is_write_buffered_select.test | 10 | 0.000990s | 0.001056s | 0.000985s | byte equality observed |
| prepend.test | 10 | 0.001066s | 0.001182s | 0.001042s | byte equality observed |
| prepend_select.test | 10 | 0.001038s | 0.001163s | 0.001006s | byte equality observed |
| read.test | 10 | 0.001520s | 0.001522s | 0.001407s | byte equality observed |
| read_at.test | 10 | 0.002015s | 0.002032s | 0.001857s | byte equality observed |
| read_at_select.test | 10 | 0.002023s | 0.002039s | 0.001889s | byte equality observed |
| read_select.test | 10 | 0.001626s | 0.001537s | 0.001567s | byte equality observed |
| read_until.test | 10 | 0.001312s | 0.001286s | 0.001269s | byte equality observed |
| read_until_select.test | 10 | 0.001306s | 0.001282s | 0.001253s | byte equality observed |
| recycling_allocator.test | 10 | 0.001081s | 0.001086s | 0.001093s | byte equality observed |
| recycling_allocator_select.test | 10 | 0.001082s | 0.001122s | 0.001085s | byte equality observed |
| signal_set.test | 10 | 0.001084s | 0.001154s | 0.001054s | byte equality observed |
| signal_set_select.test | 10 | 0.001084s | 0.001127s | 0.001109s | byte equality observed |
| socket_base.test | 10 | 0.001233s | 0.001229s | 0.001210s | byte equality observed |
| socket_base_select.test | 10 | 0.001203s | 0.001239s | 0.001161s | byte equality observed |
| spawn.test | 10 | 0.001318s | 0.001299s | 0.001298s | byte equality observed |
| spawn_select.test | 10 | 0.001321s | 0.001333s | 0.001259s | byte equality observed |
| static_thread_pool.test | 10 | 0.001062s | 0.001072s | 0.001091s | byte equality observed |
| static_thread_pool_select.test | 10 | 0.001037s | 0.001056s | 0.001029s | byte equality observed |
| strand.test | 1 | 0.002140s | 0.002069s | 0.002079s | byte equality observed |
| strand_select.test | 1 | 0.001732s | 0.001756s | 0.001774s | byte equality observed |
| streambuf.test | 10 | 0.001117s | 0.001091s | 0.001103s | byte equality observed |
| streambuf_select.test | 10 | 0.001117s | 0.001139s | 0.001129s | byte equality observed |
| thread_pool.test | 9 | 0.002438s | 0.002403s | 0.002438s | byte equality observed |
| thread_pool_select.test | 9 | 0.002390s | 0.002370s | 0.002408s | byte equality observed |
| write.test | 10 | 0.001743s | 0.001563s | 0.001546s | byte equality observed |
| write_at.test | 10 | 0.001889s | 0.001749s | 0.001708s | byte equality observed |
| write_at_select.test | 10 | 0.001852s | 0.001711s | 0.001685s | byte equality observed |
| write_select.test | 10 | 0.001673s | 0.001615s | 0.001521s | byte equality observed |

Compiler command and timing observations: [cppgm](https://github.com/vishvananda/cppgm-extended/actions/runs/42770605980) · [gcc](https://github.com/vishvananda/cppgm-extended/actions/runs/42770605980) · [clang](https://github.com/vishvananda/cppgm-extended/actions/runs/42770605980).

[Reviewed outcomes and per-test records](../results.json) · [Host and toolchains](../metadata.json)
