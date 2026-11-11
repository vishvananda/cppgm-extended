# libs/asio/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| any_completion_executor.test | RUN | 0.687217 | 3.787802 | 3.548463 | 0.001992 | 0.002623 | 0.002296 |
| any_completion_executor_select.test | RUN | 0.703757 | 3.733720 | 3.775698 | 0.002498 | 0.002518 | 0.002054 |
| any_completion_handler.test | RUN | 0.737609 | 3.357744 | 3.255390 | 0.001594 | 0.002102 | 0.001802 |
| any_completion_handler_select.test | RUN | 0.778495 | 3.513301 | 3.444834 | 0.002040 | 0.002082 | 0.001946 |
| any_io_executor.test | RUN | 0.698635 | 3.980949 | 3.866269 | 0.002550 | 0.002392 | 0.002577 |
| any_io_executor_select.test | RUN | 0.716547 | 3.904967 | 4.142973 | 0.002488 | 0.002449 | 0.002727 |
| append.test | RUN | 0.648787 | 3.176574 | 2.975563 | 0.002062 | 0.002064 | 0.002042 |
| append_select.test | RUN | 0.665229 | 3.230854 | 2.852869 | 0.001890 | 0.002166 | 0.001950 |
| as_tuple.test | RUN | 0.837717 | 3.902204 | 4.073136 | 0.001880 | 0.001960 | 0.002108 |
| as_tuple_select.test | RUN | 0.862893 | 3.835597 | 4.018640 | 0.002048 | 0.001742 | 0.001651 |
| associated_allocator.test | LINK | 0.193251 | 0.341204 | 0.475345 | — | — | — |
| associated_allocator_select.test | LINK | 0.193979 | 0.351532 | 0.487394 | — | — | — |
| associated_cancellation_slot.test | LINK | 0.201597 | 0.387183 | 0.527682 | — | — | — |
| associated_cancellation_slot_select.test | LINK | 0.206768 | 0.389983 | 0.529182 | — | — | — |
| associated_executor.test | LINK | 0.303356 | 0.649867 | 0.829397 | — | — | — |
| associated_executor_select.test | LINK | 0.300620 | 0.650137 | 0.803990 | — | — | — |
| associated_immediate_executor.test | LINK | 0.267232 | 0.602924 | 0.744675 | — | — | — |
| associated_immediate_executor_select.test | LINK | 0.271984 | 0.603412 | 0.701590 | — | — | — |
| associator.test | LINK | 0.153517 | 0.285705 | 0.399806 | — | — | — |
| associator_select.test | LINK | 0.149469 | 0.295526 | 0.360502 | — | — | — |
| awaitable.test | LINK | 0.148309 | 0.277828 | 0.371396 | — | — | — |
| awaitable_select.test | LINK | 0.149951 | 0.283370 | 0.372580 | — | — | — |
| basic_datagram_socket.test | LINK | 0.447326 | 0.888514 | 1.433714 | — | — | — |
| basic_datagram_socket_select.test | LINK | 0.466192 | 0.917493 | 1.481669 | — | — | — |
| basic_deadline_timer.test | LINK | 0.560406 | 1.039380 | 1.656127 | — | — | — |
| basic_deadline_timer_select.test | LINK | 0.550369 | 1.077537 | 1.695498 | — | — | — |
| basic_file.test | LINK | 0.155846 | 0.294285 | 0.388620 | — | — | — |
| basic_file_select.test | LINK | 0.154750 | 0.282502 | 0.388859 | — | — | — |
| basic_random_access_file.test | LINK | 0.146306 | 0.282834 | 0.396571 | — | — | — |
| basic_random_access_file_select.test | LINK | 0.152196 | 0.274560 | 0.396719 | — | — | — |
| basic_raw_socket.test | LINK | 0.447951 | 0.890791 | 1.487024 | — | — | — |
| basic_raw_socket_select.test | LINK | 0.464573 | 0.926499 | 1.532284 | — | — | — |
| basic_readable_pipe.test | LINK | 0.437370 | 0.874809 | 1.284850 | — | — | — |
| basic_readable_pipe_select.test | LINK | 0.456186 | 0.905935 | 1.518637 | — | — | — |
| basic_seq_packet_socket.test | LINK | 0.453762 | 0.843280 | 1.447526 | — | — | — |
| basic_seq_packet_socket_select.test | LINK | 0.464040 | 0.919176 | 1.492024 | — | — | — |
| basic_signal_set.test | LINK | 0.414359 | 0.839935 | 1.092695 | — | — | — |
| basic_signal_set_select.test | LINK | 0.429947 | 0.868613 | 1.391407 | — | — | — |
| basic_socket_acceptor.test | LINK | 0.464027 | 0.911406 | 1.419397 | — | — | — |
| basic_socket_acceptor_select.test | LINK | 0.470253 | 0.926503 | 1.458943 | — | — | — |
| basic_stream_file.test | LINK | 0.157178 | 0.303211 | 0.399006 | — | — | — |
| basic_stream_file_select.test | LINK | 0.158323 | 0.295154 | 0.396131 | — | — | — |
| basic_stream_socket.test | LINK | 0.454901 | 0.901008 | 1.449749 | — | — | — |
| basic_stream_socket_select.test | LINK | 0.467785 | 0.908521 | 1.439468 | — | — | — |
| basic_streambuf.test | LINK | 0.232261 | 0.411360 | 0.583407 | — | — | — |
| basic_streambuf_select.test | LINK | 0.233759 | 0.428529 | 0.572832 | — | — | — |
| basic_waitable_timer.test | LINK | 0.430573 | 0.839480 | 1.342589 | — | — | — |
| basic_waitable_timer_select.test | LINK | 0.445244 | 0.851283 | 1.390079 | — | — | — |
| basic_writable_pipe.test | LINK | 0.449545 | 0.861124 | 1.276865 | — | — | — |
| basic_writable_pipe_select.test | LINK | 0.470451 | 0.858833 | 1.514324 | — | — | — |
| bind_allocator.test | RUN | 0.693699 | 3.301256 | 3.214334 | 0.002471 | 0.002424 | 0.002217 |
| bind_allocator_select.test | RUN | 0.692392 | 3.328291 | 3.185824 | 0.001981 | 0.002425 | 0.002206 |
| bind_cancellation_slot.test | RUN | 0.706626 | 3.319737 | 3.228631 | 0.001633 | 0.001672 | 0.001485 |
| bind_cancellation_slot_select.test | RUN | 0.710320 | 3.304682 | 3.102849 | 0.001463 | 0.001632 | 0.001487 |
| bind_executor.test | RUN | 0.474476 | 2.296906 | 1.955278 | 0.002251 | 0.002278 | 0.002258 |
| bind_executor_select.test | RUN | 0.475438 | 2.348483 | 1.934653 | 0.001831 | 0.002150 | 0.001927 |
| bind_immediate_executor.test | RUN | 0.359979 | 1.902905 | 1.228306 | 0.001468 | 0.001393 | 0.001384 |
| bind_immediate_executor_select.test | RUN | 0.377084 | 1.836656 | 1.374964 | 0.001438 | 0.001337 | 0.001342 |
| buffer.test | RUN | 0.223154 | 0.794839 | 0.763939 | 0.001506 | 0.001353 | 0.001422 |
| buffer_select.test | RUN | 0.225884 | 0.795122 | 0.772692 | 0.001390 | 0.001317 | 0.001366 |
| buffer_registration.test | LINK | 0.189643 | 0.401818 | 0.477851 | — | — | — |
| buffer_registration_select.test | LINK | 0.193096 | 0.399883 | 0.478800 | — | — | — |
| buffered_read_stream.test | RUN | 0.750653 | 5.092770 | 4.558692 | 0.001763 | 0.002594 | 0.002355 |
| buffered_read_stream_select.test | RUN | 0.767602 | 5.228553 | 4.750008 | 0.001751 | 0.002309 | 0.002464 |
| buffered_stream.test | RUN | 1.139295 | 8.013857 | 7.819615 | 0.002474 | 0.002206 | 0.002422 |
| buffered_stream_select.test | RUN | 1.247360 | 7.957343 | 7.544709 | 0.002409 | 0.002327 | 0.002016 |
| buffered_write_stream.test | RUN | 1.272317 | 6.826087 | 6.671538 | 0.002610 | 0.002691 | 0.002209 |
| buffered_write_stream_select.test | RUN | 1.288828 | 6.926075 | 6.462573 | 0.002122 | 0.002397 | 0.002356 |
| buffers_iterator.test | RUN | 0.341457 | 0.647612 | 1.095590 | 0.001859 | 0.001946 | 0.001497 |
| buffers_iterator_select.test | RUN | 0.328776 | 0.646921 | 1.067310 | 0.001679 | 0.001785 | 0.001766 |
| cancel_after.test | RUN | 1.212649 | 5.260779 | 4.888105 | 0.004162 | 0.004125 | 0.004221 |
| cancel_after_select.test | RUN | 1.203514 | 5.237694 | 4.895345 | 0.003417 | 0.003285 | 0.003428 |
| cancel_at.test | RUN | 1.197381 | 4.732328 | 4.590908 | 0.003840 | 0.003782 | 0.003548 |
| cancel_at_select.test | RUN | 1.155802 | 4.646550 | 4.559128 | 0.002861 | 0.003541 | 0.003476 |
| cancellation_signal.test | LINK | 0.133039 | 0.245494 | 0.293348 | — | — | — |
| cancellation_signal_select.test | LINK | 0.193103 | 0.240570 | 0.292232 | — | — | — |
| cancellation_state.test | LINK | 0.190971 | 0.246098 | 0.312522 | — | — | — |
| cancellation_state_select.test | LINK | 0.190384 | 0.246026 | 0.324067 | — | — | — |
| cancellation_type.test | LINK | 0.148589 | 0.192080 | 0.363026 | — | — | — |
| cancellation_type_select.test | LINK | 0.150264 | 0.192116 | 0.387550 | — | — | — |
| co_composed.test | RUN | 0.148005 | 0.197358 | 0.369470 | 0.001318 | 0.001340 | 0.001648 |
| co_composed_select.test | RUN | 0.148186 | 0.203067 | 0.389676 | 0.001528 | 0.001726 | 0.001558 |
| co_spawn.test | LINK | 0.147868 | 0.275977 | 0.346538 | — | — | — |
| co_spawn_select.test | LINK | 0.147752 | 0.282555 | 0.371712 | — | — | — |
| completion_condition.test | LINK | 0.197481 | 0.418182 | 0.493820 | — | — | — |
| completion_condition_select.test | LINK | 0.197035 | 0.424580 | 0.486478 | — | — | — |
| compose.test | RUN | 0.841189 | 4.040435 | 4.023025 | 0.002473 | 0.002407 | 0.002346 |
| compose_select.test | RUN | 0.812980 | 3.998351 | 3.982101 | 0.002165 | 0.002464 | 0.002388 |
| composed.test | RUN | 0.837681 | 3.985703 | 3.959211 | 0.002042 | 0.001969 | 0.001876 |
| composed_select.test | RUN | 0.844188 | 3.959383 | 3.912910 | 0.001824 | 0.001731 | 0.001852 |
| config.test | RUN | 0.372738 | 2.626921 | 1.766415 | 0.001930 | 0.002060 | 0.002058 |
| config_select.test | RUN | 0.490047 | 2.772854 | 2.102435 | 0.001870 | 0.002034 | 0.001912 |
| connect.test | LINK | 1.396750 | 8.531065 | 8.376555 | — | — | — |
| connect_select.test | LINK | 1.419474 | 8.520636 | 8.482985 | — | — | — |
| connect_pipe.test | RUN | 0.766670 | 3.244161 | 3.107303 | 0.002114 | 0.002058 | 0.002070 |
| connect_pipe_select.test | RUN | 0.760913 | 3.366078 | 3.331445 | 0.001885 | 0.002064 | 0.001940 |
| consign.test | RUN | 0.665796 | 3.167238 | 2.798039 | 0.001950 | 0.002156 | 0.001992 |
| consign_select.test | RUN | 0.663784 | 3.289453 | 2.739521 | 0.001920 | 0.001582 | 0.002032 |
| coroutine.test | LINK | 0.163251 | 0.334534 | 0.418556 | — | — | — |
| coroutine_select.test | LINK | 0.158492 | 0.336055 | 0.423390 | — | — | — |
| deadline_timer.test | RUN | 0.968419 | 4.150995 | 4.022231 | 0.003492 | 0.003640 | 0.003399 |
| deadline_timer_select.test | RUN | 0.942010 | 4.127525 | 4.041765 | 0.003388 | 0.002984 | 0.003324 |
| deferred.test | RUN | 0.166980 | 0.306953 | 0.426647 | 0.002096 | 0.001921 | 0.001809 |
| deferred_select.test | RUN | 0.171632 | 0.305669 | 0.436661 | 0.001690 | 0.001869 | 0.001869 |
| detached.test | LINK | 0.191296 | 0.338054 | 0.487133 | — | — | — |
| detached_select.test | LINK | 0.187592 | 0.263895 | 0.470798 | — | — | — |
| disposition.test | RUN | 0.228900 | 0.792135 | 0.850653 | 0.001444 | 0.002147 | 0.001898 |
| disposition_select.test | RUN | 0.173351 | 0.801310 | 0.840370 | 0.001506 | 0.002086 | 0.002040 |
| error.test | RUN | 0.161973 | 0.768632 | 0.768146 | 0.001409 | 0.001943 | 0.001832 |
| error_select.test | RUN | 0.167155 | 0.774198 | 0.724923 | 0.001518 | 0.001913 | 0.001887 |
| file_base.test | LINK | 0.207477 | 0.607818 | 0.772633 | — | — | — |
| file_base_select.test | LINK | 0.208288 | 0.579742 | 0.563015 | — | — | — |
| fwd_first.test | LINK | 0.465457 | 0.903659 | 1.426560 | — | — | — |
| fwd_last.test | LINK | 0.470805 | 0.935402 | 1.405440 | — | — | — |
| generic_basic_endpoint.test | LINK | 0.185181 | 0.378455 | 0.511449 | — | — | — |
| generic_basic_endpoint_select.test | LINK | 0.194138 | 0.391679 | 0.514730 | — | — | — |
| generic_datagram_protocol.test | LINK | 0.654357 | 3.371801 | 2.784991 | — | — | — |
| generic_datagram_protocol_select.test | LINK | 0.651806 | 3.508105 | 2.747540 | — | — | — |
| generic_raw_protocol.test | LINK | 0.682431 | 3.312815 | 2.762852 | — | — | — |
| generic_raw_protocol_select.test | LINK | 0.671711 | 3.456945 | 2.715143 | — | — | — |
| generic_seq_packet_protocol.test | LINK | 0.570972 | 2.777018 | 2.390265 | — | — | — |
| generic_seq_packet_protocol_select.test | LINK | 0.571713 | 2.857544 | 2.350503 | — | — | — |
| generic_stream_protocol.test | LINK | 0.609878 | 3.039849 | 2.618301 | — | — | — |
| generic_stream_protocol_select.test | LINK | 0.616648 | 3.134021 | 2.584681 | — | — | — |
| high_resolution_timer.test | LINK | 0.295235 | 0.582757 | 0.798758 | — | — | — |
| high_resolution_timer_select.test | LINK | 0.298836 | 0.582265 | 0.816957 | — | — | — |
| immediate.test | LINK | 0.206167 | 0.434032 | 0.500080 | — | — | — |
| immediate_select.test | LINK | 0.208546 | 0.432762 | 0.492779 | — | — | — |
| inline_executor.test | RUN | 0.250088 | 0.657612 | 0.728207 | 0.001452 | 0.001500 | 0.001387 |
| inline_executor_select.test | RUN | 0.245794 | 0.669995 | 0.718661 | 0.001486 | 0.001481 | 0.001415 |
| inline_or_executor.test | RUN | 0.388440 | 1.942147 | 1.382363 | 0.001525 | 0.001536 | 0.001367 |
| inline_or_executor_select.test | RUN | 0.395288 | 2.015769 | 1.530989 | 0.001534 | 0.001494 | 0.001369 |
| io_context.test | RUN | 0.553521 | 2.945777 | 2.444478 | 0.002449 | 0.002457 | 0.002274 |
| io_context_select.test | RUN | 0.552270 | 2.904412 | 2.458347 | 0.002224 | 0.002330 | 0.002108 |
| io_context_strand.test | RUN | 0.513259 | 2.558049 | 2.135176 | 0.002320 | 0.002474 | 0.002392 |
| io_context_strand_select.test | RUN | 0.525321 | 2.524511 | 2.145983 | 0.002121 | 0.002412 | 0.002308 |
| ip_address.test | LINK | 0.207697 | 0.601759 | 0.616112 | — | — | — |
| ip_address_select.test | LINK | 0.210134 | 0.592957 | 0.612463 | — | — | — |
| ip_address_v4.test | LINK | 0.214832 | 0.645615 | 0.685016 | — | — | — |
| ip_address_v4_select.test | LINK | 0.214663 | 0.645071 | 0.677340 | — | — | — |
| ip_address_v6.test | LINK | 0.236136 | 0.816502 | 0.833751 | — | — | — |
| ip_address_v6_select.test | LINK | 0.233961 | 0.810034 | 0.819578 | — | — | — |
| ip_basic_endpoint.test | LINK | 0.188780 | 0.412723 | 0.536778 | — | — | — |
| ip_basic_endpoint_select.test | LINK | 0.190832 | 0.402646 | 0.537572 | — | — | — |
| ip_basic_resolver.test | LINK | 0.299232 | 0.592717 | 0.917467 | — | — | — |
| ip_basic_resolver_select.test | LINK | 0.304874 | 0.592760 | 0.893801 | — | — | — |
| ip_basic_resolver_entry.test | LINK | 0.106806 | 0.194137 | 0.380458 | — | — | — |
| ip_basic_resolver_entry_select.test | LINK | 0.104252 | 0.196849 | 0.380782 | — | — | — |
| ip_basic_resolver_iterator.test | LINK | 0.191042 | 0.392087 | 0.895270 | — | — | — |
| ip_basic_resolver_iterator_select.test | LINK | 0.188502 | 0.394122 | 0.909873 | — | — | — |
| ip_basic_resolver_query.test | LINK | 0.179546 | 0.379317 | 0.889893 | — | — | — |
| ip_basic_resolver_query_select.test | LINK | 0.180082 | 0.379272 | 0.842649 | — | — | — |
| ip_host_name.test | RUN | 0.189594 | 0.519912 | 0.985037 | 0.001412 | 0.001350 | 0.001672 |
| ip_host_name_select.test | RUN | 0.190943 | 0.517449 | 0.989782 | 0.001439 | 0.001488 | 0.001483 |
| ip_icmp.test | RUN | 0.820414 | 6.341340 | 5.815554 | 0.001490 | 0.002038 | 0.001602 |
| ip_icmp_select.test | RUN | 0.829658 | 6.890308 | 5.719197 | 0.001462 | 0.001942 | 0.001758 |
| ip_multicast.test | RUN | 0.499121 | 3.518143 | 3.349710 | 0.001612 | 0.002225 | 0.001596 |
| ip_multicast_select.test | RUN | 0.500326 | 3.591136 | 3.313249 | 0.001505 | 0.002173 | 0.001984 |
| ip_resolver_query_base.test | LINK | 0.124332 | 0.330520 | 0.400760 | — | — | — |
| ip_resolver_query_base_select.test | LINK | 0.122244 | 0.310858 | 0.397454 | — | — | — |
| ip_tcp.test | RUN | 1.248507 | 10.154010 | 8.815905 | 0.002098 | 0.002824 | 0.002694 |
| ip_tcp_select.test | RUN | 1.226765 | 10.483026 | 8.600220 | 0.002009 | 0.002737 | 0.002354 |
| ip_udp.test | RUN | 0.866837 | 7.237834 | 6.140460 | 0.001677 | 0.002196 | 0.001662 |
| ip_udp_select.test | RUN | 0.875616 | 7.467801 | 5.985212 | 0.001618 | 0.002318 | 0.001822 |
| ip_unicast.test | RUN | 0.463503 | 3.153391 | 2.954688 | 0.001547 | 0.002224 | 0.001356 |
| ip_unicast_select.test | RUN | 0.469725 | 3.278239 | 2.944091 | 0.001592 | 0.002097 | 0.001664 |
| ip_v6_only.test | RUN | 0.472117 | 3.362809 | 3.008800 | 0.001582 | 0.002177 | 0.001858 |
| ip_v6_only_select.test | RUN | 0.482745 | 3.388086 | 2.983881 | 0.001550 | 0.001966 | 0.001643 |
| is_read_buffered.test | RUN | 0.348336 | 1.043812 | 1.657775 | 0.001502 | 0.001978 | 0.001728 |
| is_read_buffered_select.test | RUN | 0.356447 | 1.032153 | 1.698665 | 0.001368 | 0.001936 | 0.001472 |
| is_write_buffered.test | RUN | 0.344320 | 1.042688 | 1.648179 | 0.001526 | 0.001995 | 0.001744 |
| is_write_buffered_select.test | RUN | 0.360433 | 1.032243 | 1.678321 | 0.001441 | 0.001616 | 0.001752 |
| local_basic_endpoint.test | LINK | 0.182661 | 0.575151 | 0.866597 | — | — | — |
| local_basic_endpoint_select.test | LINK | 0.184460 | 0.556831 | 0.873559 | — | — | — |
| local_connect_pair.test | LINK | 0.484858 | 3.269536 | 2.792720 | — | — | — |
| local_connect_pair_select.test | LINK | 0.608562 | 3.246985 | 2.793428 | — | — | — |
| local_datagram_protocol.test | LINK | 0.837911 | 4.147780 | 3.845090 | — | — | — |
| local_datagram_protocol_select.test | LINK | 0.871156 | 4.161845 | 3.788896 | — | — | — |
| local_seq_packet_protocol.test | LINK | 0.860046 | 4.162957 | 3.821053 | — | — | — |
| local_seq_packet_protocol_select.test | LINK | 0.855809 | 4.242691 | 3.818811 | — | — | — |
| local_stream_protocol.test | LINK | 0.808112 | 3.895092 | 3.688272 | — | — | — |
| local_stream_protocol_select.test | LINK | 0.830984 | 3.908383 | 3.665277 | — | — | — |
| placeholders.test | LINK | 0.178059 | 0.309884 | 0.410501 | — | — | — |
| placeholders_select.test | LINK | 0.168364 | 0.308320 | 0.414198 | — | — | — |
| posix_basic_descriptor.test | LINK | 0.455393 | 0.862182 | 1.270425 | — | — | — |
| posix_basic_descriptor_select.test | LINK | 0.457116 | 0.912921 | 1.527615 | — | — | — |
| posix_basic_stream_descriptor.test | LINK | 0.442611 | 0.877123 | 1.277997 | — | — | — |
| posix_basic_stream_descriptor_select.test | LINK | 0.458245 | 0.922136 | 1.528806 | — | — | — |
| posix_descriptor_base.test | LINK | 0.189102 | 0.340207 | 0.380065 | — | — | — |
| posix_descriptor_base_select.test | LINK | 0.185723 | 0.326192 | 0.383286 | — | — | — |
| posix_stream_descriptor.test | LINK | 0.823097 | 3.986853 | 3.640204 | — | — | — |
| posix_stream_descriptor_select.test | LINK | 0.810185 | 4.088212 | 3.746725 | — | — | — |
| prepend.test | RUN | 0.671292 | 3.249483 | 2.897909 | 0.001949 | 0.002094 | 0.002012 |
| prepend_select.test | RUN | 0.656966 | 3.378243 | 2.876766 | 0.001918 | 0.001822 | 0.001740 |
| random_access_file.test | LINK | 0.410026 | 0.799958 | 0.979858 | — | — | — |
| random_access_file_select.test | LINK | 0.411395 | 0.848293 | 1.264087 | — | — | — |
| read.test | RUN | 1.995753 | 21.126396 | 16.406230 | 0.002721 | 0.002323 | 0.002482 |
| read_select.test | RUN | 2.019075 | 21.057183 | 16.665420 | 0.002770 | 0.002513 | 0.002619 |
| read_at.test | RUN | 1.966201 | 30.859232 | 22.181312 | 0.003539 | 0.003268 | 0.002983 |
| read_at_select.test | RUN | 1.977138 | 30.645614 | 22.588976 | 0.003240 | 0.003381 | 0.002931 |
| read_until.test | RUN | 1.088808 | 7.592069 | 6.456887 | 0.001960 | 0.002128 | 0.002178 |
| read_until_select.test | RUN | 1.096823 | 7.580202 | 6.750410 | 0.001680 | 0.001968 | 0.002118 |
| readable_pipe.test | LINK | 0.668828 | 3.210397 | 2.813636 | — | — | — |
| readable_pipe_select.test | LINK | 0.668620 | 3.308553 | 3.034038 | — | — | — |
| recycling_allocator.test | RUN | 0.221621 | 0.502249 | 0.622885 | 0.001754 | 0.001946 | 0.001502 |
| recycling_allocator_select.test | RUN | 0.216205 | 0.483889 | 0.620614 | 0.001752 | 0.001614 | 0.001924 |
| redirect_disposition.test | LINK | 0.924291 | 4.534385 | 4.487756 | — | — | — |
| redirect_disposition_select.test | LINK | 0.925350 | 4.479283 | 4.386480 | — | — | — |
| redirect_error.test | LINK | 0.798337 | 3.839795 | 3.809940 | — | — | — |
| redirect_error_select.test | LINK | 0.802321 | 3.761674 | 3.892250 | — | — | — |
| registered_buffer.test | LINK | 0.216889 | 0.419157 | 0.532626 | — | — | — |
| registered_buffer_select.test | LINK | 0.219923 | 0.410420 | 0.564927 | — | — | — |
| signal_set.test | RUN | 0.641147 | 3.178443 | 2.778097 | 0.001671 | 0.001960 | 0.001822 |
| signal_set_select.test | RUN | 0.658109 | 3.329777 | 3.034023 | 0.001696 | 0.002005 | 0.002004 |
| signal_set_base.test | LINK | 0.175322 | 0.303541 | 0.422486 | — | — | — |
| signal_set_base_select.test | LINK | 0.179398 | 0.249057 | 0.423987 | — | — | — |
| socket_base.test | RUN | 0.810472 | 4.648151 | 4.234327 | 0.002046 | 0.001938 | 0.001858 |
| socket_base_select.test | RUN | 0.810576 | 4.594251 | 4.192083 | 0.001907 | 0.001956 | 0.001927 |
| spawn.test | RUN | 1.338767 | 7.486020 | 7.088210 | 0.002110 | 0.002172 | 0.002191 |
| spawn_select.test | RUN | 1.339408 | 7.371975 | 7.032794 | 0.002040 | 0.002389 | 0.001736 |
| static_thread_pool.test | RUN | 0.381825 | 0.744372 | 0.995558 | 0.001632 | 0.001724 | 0.001936 |
| static_thread_pool_select.test | RUN | 0.398418 | 0.810795 | 1.255308 | 0.001864 | 0.001571 | 0.001636 |
| steady_timer.test | LINK | 0.428651 | 0.839371 | 1.336761 | — | — | — |
| steady_timer_select.test | LINK | 0.444641 | 0.876288 | 1.371179 | — | — | — |
| strand.test | RUN | 0.873439 | 4.255755 | 4.135765 | 0.001920 | 0.002837 | 0.002445 |
| strand_select.test | RUN | 0.858067 | 4.240639 | 4.126584 | 0.002553 | 0.002679 | 0.002874 |
| stream_file.test | LINK | 0.408438 | 0.775906 | 1.022893 | — | — | — |
| stream_file_select.test | LINK | 0.428982 | 0.831778 | 1.285828 | — | — | — |
| streambuf.test | RUN | 0.221434 | 0.470678 | 0.702260 | 0.001426 | 0.001511 | 0.001704 |
| streambuf_select.test | RUN | 0.175982 | 0.430342 | 0.553870 | 0.001357 | 0.001470 | 0.001276 |
| system_timer.test | LINK | 0.574879 | 2.759886 | 2.471410 | — | — | — |
| system_timer_select.test | LINK | 0.570690 | 2.723104 | 2.385698 | — | — | — |
| system_context.test | LINK | 0.262545 | 0.493904 | 0.606993 | — | — | — |
| system_context_select.test | LINK | 0.271231 | 0.541250 | 0.754818 | — | — | — |
| system_executor.test | LINK | 0.331444 | 1.588867 | 1.147238 | — | — | — |
| system_executor_select.test | LINK | 0.344419 | 1.582141 | 1.290964 | — | — | — |
| this_coro.test | LINK | 0.108708 | 0.189833 | 0.238339 | — | — | — |
| this_coro_select.test | LINK | 0.105384 | 0.189078 | 0.245474 | — | — | — |
| thread_pool.test | RUN | 0.390854 | 2.147047 | 1.478973 | 0.002659 | 0.003487 | 0.002726 |
| thread_pool_select.test | RUN | 0.401812 | 2.331436 | 1.624260 | 0.002590 | 0.003457 | 0.002702 |
| time_traits.test | LINK | 0.238459 | 0.605477 | 0.482069 | — | — | — |
| time_traits_select.test | LINK | 0.242727 | 0.589065 | 0.491270 | — | — | — |
| ts_buffer.test | LINK | 0.229846 | 0.654066 | 0.546475 | — | — | — |
| ts_buffer_select.test | LINK | 0.227501 | 0.662284 | 0.803674 | — | — | — |
| ts_executor.test | LINK | 0.305039 | 0.919317 | 1.349388 | — | — | — |
| ts_executor_select.test | LINK | 0.323710 | 0.982907 | 1.638716 | — | — | — |
| ts_internet.test | LINK | 0.337624 | 1.020230 | 1.659880 | — | — | — |
| ts_internet_select.test | LINK | 0.349109 | 1.027513 | 1.685081 | — | — | — |
| ts_io_context.test | LINK | 0.274470 | 0.785455 | 1.026592 | — | — | — |
| ts_io_context_select.test | LINK | 0.288890 | 0.814383 | 1.284014 | — | — | — |
| ts_net.test | LINK | 0.403596 | 1.226957 | 2.044615 | — | — | — |
| ts_net_select.test | LINK | 0.416445 | 1.248260 | 2.096600 | — | — | — |
| ts_netfwd.test | LINK | 0.405899 | 1.190901 | 2.041027 | — | — | — |
| ts_netfwd_select.test | LINK | 0.417496 | 1.245261 | 2.103551 | — | — | — |
| ts_socket.test | LINK | 0.345454 | 0.953712 | 1.552299 | — | — | — |
| ts_socket_select.test | LINK | 0.359076 | 0.968190 | 1.609740 | — | — | — |
| ts_timer.test | LINK | 0.295419 | 0.853315 | 1.414151 | — | — | — |
| ts_timer_select.test | LINK | 0.297802 | 0.903373 | 1.329916 | — | — | — |
| use_awaitable.test | LINK | 0.103288 | 0.276302 | 0.401612 | — | — | — |
| use_awaitable_select.test | LINK | 0.105682 | 0.282267 | 0.373936 | — | — | — |
| wait_traits.test | LINK | 0.104516 | 0.273999 | 0.376838 | — | — | — |
| wait_traits_select.test | LINK | 0.106704 | 0.276400 | 0.393604 | — | — | — |
| windows_basic_object_handle.test | LINK | 0.104377 | 0.283409 | 0.381722 | — | — | — |
| windows_basic_object_handle_select.test | LINK | 0.105914 | 0.285411 | 0.372622 | — | — | — |
| windows_basic_overlapped_handle.test | LINK | 0.107309 | 0.274834 | 0.392248 | — | — | — |
| windows_basic_overlapped_handle_select.test | LINK | 0.104623 | 0.283544 | 0.392398 | — | — | — |
| windows_basic_random_access_handle.test | LINK | 0.104599 | 0.286429 | 0.376191 | — | — | — |
| windows_basic_random_access_handle_select.test | LINK | 0.138808 | 0.285358 | 0.372455 | — | — | — |
| windows_basic_stream_handle.test | LINK | 0.142234 | 0.288975 | 0.396023 | — | — | — |
| windows_basic_stream_handle_select.test | LINK | 0.155314 | 0.284519 | 0.393353 | — | — | — |
| windows_object_handle.test | LINK | 0.424160 | 0.795986 | 1.003167 | — | — | — |
| windows_object_handle_select.test | LINK | 0.436980 | 0.824959 | 1.251818 | — | — | — |
| windows_overlapped_ptr.test | LINK | 0.441412 | 0.846648 | 1.114632 | — | — | — |
| windows_overlapped_ptr_select.test | LINK | 0.452090 | 0.886805 | 1.360887 | — | — | — |
| windows_random_access_handle.test | LINK | 0.409049 | 0.793810 | 0.994407 | — | — | — |
| windows_random_access_handle_select.test | LINK | 0.425707 | 0.845526 | 1.307375 | — | — | — |
| windows_stream_handle.test | LINK | 0.402963 | 0.802009 | 0.958640 | — | — | — |
| windows_stream_handle_select.test | LINK | 0.437069 | 0.837451 | 1.268779 | — | — | — |
| writable_pipe.test | LINK | 0.686384 | 3.232166 | 2.992377 | — | — | — |
| writable_pipe_select.test | LINK | 0.696906 | 3.384288 | 3.175927 | — | — | — |
| write.test | RUN | 1.997045 | 16.598974 | 16.222338 | 0.002908 | 0.002542 | 0.002514 |
| write_select.test | RUN | 2.007114 | 16.505846 | 16.538135 | 0.002807 | 0.002526 | 0.002707 |
| write_at.test | RUN | 2.073480 | 24.462968 | 20.977684 | 0.003012 | 0.005698 | 0.002455 |
| write_at_select.test | RUN | 2.106413 | 24.078025 | 21.318749 | 0.003009 | 0.004480 | 0.002713 |
| any_executor.test | RUN | 0.814969 | 8.015559 | 6.963311 | 0.002410 | 0.002526 | 0.002442 |
| blocking.test | RUN | 2.030607 | 9.415632 | 13.939423 | 0.007525 | 0.006347 | 0.007556 |
| blocking_adaptation.test | RUN | 1.011618 | 4.246774 | 6.253614 | 0.004073 | 0.004328 | 0.004032 |
| context_as.test | RUN | 0.561415 | 3.091074 | 2.030162 | 0.002115 | 0.002292 | 0.001854 |
| executor.test | RUN | 0.174198 | 0.368801 | 0.406202 | 0.001934 | 0.002942 | 0.001761 |
| inline_exception_handling.test | RUN | 1.920946 | 8.817247 | 13.574907 | 0.007355 | 0.005296 | 0.005201 |
| invocable_archetype.test | RUN | 0.152860 | 0.338094 | 0.397166 | 0.001828 | 0.002064 | 0.001796 |
| mapping.test | RUN | 1.893593 | 9.254310 | 13.667076 | 0.005419 | 0.005298 | 0.005276 |
| outstanding_work.test | RUN | 0.938000 | 3.762599 | 6.049000 | 0.004106 | 0.004128 | 0.004252 |
| prefer_only.test | RUN | 0.339384 | 1.576144 | 1.398670 | 0.001942 | 0.001943 | 0.001888 |
| relationship.test | RUN | 0.735994 | 3.825642 | 5.049055 | 0.003022 | 0.004303 | 0.003007 |

## Native runtime replay

Child CPU of successful native tests, with process startup included; GDB and expected-failure executions are correctness-only. Two warmups per compiler, four A/A observations per reference and mirrored ABBA/BAAB blocks. Short tests repeat up to ten times, identically across compilers; CPU is divided by repetitions before summing tests. Executable hashes and output checks stay outside process timers. All runs must pass their original native oracle. Output hashes record equality or variation across the complete campaign; test reports may print times or addresses.

| Reference | CPU speedup ↑ | Paired blocks | A/A speedup |
|---|---:|---|---:|
| gcc | 0.985× | 0.978×, 0.993× | 0.989× |
| clang | 0.979× | 0.977×, 0.981× | 1.004× |

† A/A drift or paired-block spread exceeds 5%; small differences are unresolved. 1375 native tests in the full cohort; 1361 have byte-identical output throughout. [Manifest](https://github.com/vishvananda/cppgm-extended/actions/runs/42709011712), [paired observations](https://github.com/vishvananda/cppgm-extended/actions/runs/42709011712) and [per-test summaries](https://github.com/vishvananda/cppgm-extended/actions/runs/42709011712).

| Test | Repeats | CPU cppgm | GCC | Clang | Output check |
|---|---:|---:|---:|---:|---|
| any_completion_executor.test | 3 | 0.001719s | 0.001683s | 0.001684s | byte equality observed |
| any_completion_executor_select.test | 10 | 0.001668s | 0.001633s | 0.001650s | byte equality observed |
| any_completion_handler.test | 10 | 0.001326s | 0.001309s | 0.001303s | byte equality observed |
| any_completion_handler_select.test | 7 | 0.001327s | 0.001314s | 0.001321s | byte equality observed |
| any_io_executor.test | 5 | 0.001652s | 0.001628s | 0.001637s | byte equality observed |
| any_io_executor_select.test | 6 | 0.001684s | 0.001659s | 0.001653s | byte equality observed |
| append.test | 8 | 0.001285s | 0.001261s | 0.001233s | byte equality observed |
| append_select.test | 8 | 0.001252s | 0.001223s | 0.001228s | byte equality observed |
| as_tuple.test | 10 | 0.001351s | 0.001330s | 0.001317s | byte equality observed |
| as_tuple_select.test | 8 | 0.001283s | 0.001274s | 0.001269s | byte equality observed |
| bind_allocator.test | 1 | 0.001725s | 0.001693s | 0.001682s | byte equality observed |
| bind_allocator_select.test | 1 | 0.001477s | 0.001435s | 0.001453s | byte equality observed |
| bind_cancellation_slot.test | 10 | 0.001252s | 0.001254s | 0.001223s | byte equality observed |
| bind_cancellation_slot_select.test | 10 | 0.001121s | 0.001103s | 0.001084s | byte equality observed |
| bind_executor.test | 1 | 0.001758s | 0.001765s | 0.001729s | byte equality observed |
| bind_executor_select.test | 1 | 0.002063s | 0.002045s | 0.002091s | byte equality observed |
| bind_immediate_executor.test | 10 | 0.001132s | 0.001155s | 0.001148s | byte equality observed |
| bind_immediate_executor_select.test | 10 | 0.001115s | 0.001128s | 0.001091s | byte equality observed |
| buffer.test | 10 | 0.001086s | 0.001047s | 0.001084s | byte equality observed |
| buffer_select.test | 10 | 0.001077s | 0.001062s | 0.001085s | byte equality observed |
| buffered_read_stream.test | 6 | 0.001423s | 0.001419s | 0.001455s | byte equality observed |
| buffered_read_stream_select.test | 8 | 0.001360s | 0.001361s | 0.001423s | byte equality observed |
| buffered_stream.test | 10 | 0.001403s | 0.001375s | 0.001435s | byte equality observed |
| buffered_stream_select.test | 10 | 0.001373s | 0.001353s | 0.001390s | byte equality observed |
| buffered_write_stream.test | 10 | 0.001398s | 0.001376s | 0.001372s | byte equality observed |
| buffered_write_stream_select.test | 10 | 0.001383s | 0.001374s | 0.001367s | byte equality observed |
| buffers_iterator.test | 10 | 0.001048s | 0.001039s | 0.001057s | byte equality observed |
| buffers_iterator_select.test | 10 | 0.001008s | 0.001008s | 0.001022s | byte equality observed |
| cancel_after.test | 1 | 0.003684s | 0.003518s | 0.003518s | byte equality observed |
| cancel_after_select.test | 1 | 0.003238s | 0.003040s | 0.002978s | byte equality observed |
| cancel_at.test | 1 | 0.003787s | 0.003755s | 0.003705s | byte equality observed |
| cancel_at_select.test | 1 | 0.003721s | 0.003610s | 0.003678s | byte equality observed |
| co_composed.test | 10 | 0.001059s | 0.001081s | 0.001056s | byte equality observed |
| co_composed_select.test | 10 | 0.001005s | 0.001011s | 0.001025s | byte equality observed |
| compose.test | 1 | 0.001776s | 0.001817s | 0.001767s | byte equality observed |
| compose_select.test | 1 | 0.001544s | 0.001463s | 0.001458s | byte equality observed |
| composed.test | 1 | 0.002166s | 0.002096s | 0.002092s | byte equality observed |
| composed_select.test | 1 | 0.001711s | 0.001687s | 0.001531s | byte equality observed |
| config.test | 10 | 0.001163s | 0.001144s | 0.001121s | byte equality observed |
| config_select.test | 10 | 0.001167s | 0.001111s | 0.001077s | byte equality observed |
| connect_pipe.test | 10 | 0.001169s | 0.001132s | 0.001120s | byte equality observed |
| connect_pipe_select.test | 10 | 0.001134s | 0.001172s | 0.001118s | byte equality observed |
| consign.test | 10 | 0.001152s | 0.001147s | 0.001138s | byte equality observed |
| consign_select.test | 10 | 0.001096s | 0.001115s | 0.001107s | byte equality observed |
| deadline_timer.test | 1 | 0.002537s | 0.002593s | 0.002424s | byte equality observed |
| deadline_timer_select.test | 1 | 0.002261s | 0.002229s | 0.002240s | byte equality observed |
| deferred.test | 10 | 0.001043s | 0.001034s | 0.001062s | byte equality observed |
| deferred_select.test | 10 | 0.000981s | 0.000987s | 0.001009s | byte equality observed |
| disposition.test | 10 | 0.001049s | 0.001039s | 0.001070s | byte equality observed |
| disposition_select.test | 10 | 0.001054s | 0.001035s | 0.001069s | byte equality observed |
| error.test | 10 | 0.001018s | 0.001036s | 0.001018s | byte equality observed |
| error_select.test | 10 | 0.001022s | 0.001025s | 0.001020s | byte equality observed |
| inline_executor.test | 10 | 0.001040s | 0.001049s | 0.001021s | byte equality observed |
| inline_executor_select.test | 10 | 0.001042s | 0.001047s | 0.001024s | byte equality observed |
| inline_or_executor.test | 10 | 0.001040s | 0.001059s | 0.001032s | byte equality observed |
| inline_or_executor_select.test | 10 | 0.001037s | 0.001036s | 0.001033s | byte equality observed |
| io_context.test | 1 | 0.001985s | 0.001911s | 0.002030s | byte equality observed |
| io_context_select.test | 1 | 0.002341s | 0.002276s | 0.002417s | byte equality observed |
| io_context_strand.test | 1 | 0.002391s | 0.002235s | 0.002406s | byte equality observed |
| io_context_strand_select.test | 1 | 0.002137s | 0.002096s | 0.002154s | byte equality observed |
| ip_host_name.test | 10 | 0.001055s | 0.001047s | 0.001046s | byte equality observed |
| ip_host_name_select.test | 10 | 0.001010s | 0.000997s | 0.000998s | byte equality observed |
| ip_icmp.test | 10 | 0.001048s | 0.001041s | 0.001013s | byte equality observed |
| ip_icmp_select.test | 10 | 0.001051s | 0.001059s | 0.001004s | byte equality observed |
| ip_multicast.test | 10 | 0.001157s | 0.001137s | 0.001109s | byte equality observed |
| ip_multicast_select.test | 10 | 0.001112s | 0.001137s | 0.001096s | byte equality observed |
| ip_tcp.test | 10 | 0.001580s | 0.001556s | 0.001496s | byte equality observed |
| ip_tcp_select.test | 10 | 0.001502s | 0.001484s | 0.001442s | byte equality observed |
| ip_udp.test | 10 | 0.001174s | 0.001169s | 0.001133s | byte equality observed |
| ip_udp_select.test | 10 | 0.001193s | 0.001127s | 0.001118s | byte equality observed |
| ip_unicast.test | 10 | 0.001146s | 0.001103s | 0.001109s | byte equality observed |
| ip_unicast_select.test | 10 | 0.001127s | 0.001110s | 0.001078s | byte equality observed |
| ip_v6_only.test | 10 | 0.001121s | 0.001119s | 0.001117s | byte equality observed |
| ip_v6_only_select.test | 10 | 0.001114s | 0.001125s | 0.001195s | byte equality observed |
| is_read_buffered.test | 10 | 0.001067s | 0.001033s | 0.001029s | byte equality observed |
| is_read_buffered_select.test | 10 | 0.001063s | 0.001057s | 0.001058s | byte equality observed |
| is_write_buffered.test | 10 | 0.001054s | 0.001091s | 0.001048s | byte equality observed |
| is_write_buffered_select.test | 10 | 0.001060s | 0.001053s | 0.001077s | byte equality observed |
| prepend.test | 10 | 0.001145s | 0.001119s | 0.001141s | byte equality observed |
| prepend_select.test | 10 | 0.001103s | 0.001098s | 0.001131s | byte equality observed |
| read.test | 10 | 0.001562s | 0.001464s | 0.001472s | byte equality observed |
| read_at.test | 10 | 0.001982s | 0.001888s | 0.001703s | byte equality observed |
| read_at_select.test | 10 | 0.002042s | 0.001897s | 0.001741s | byte equality observed |
| read_select.test | 10 | 0.001580s | 0.001472s | 0.001504s | byte equality observed |
| read_until.test | 10 | 0.001314s | 0.001280s | 0.001261s | byte equality observed |
| read_until_select.test | 10 | 0.001277s | 0.001230s | 0.001273s | byte equality observed |
| recycling_allocator.test | 10 | 0.001060s | 0.001071s | 0.001044s | byte equality observed |
| recycling_allocator_select.test | 10 | 0.001054s | 0.001033s | 0.001075s | byte equality observed |
| signal_set.test | 10 | 0.001088s | 0.001101s | 0.001076s | byte equality observed |
| signal_set_select.test | 10 | 0.001107s | 0.001091s | 0.001070s | byte equality observed |
| socket_base.test | 10 | 0.001203s | 0.001205s | 0.001202s | byte equality observed |
| socket_base_select.test | 10 | 0.001170s | 0.001200s | 0.001190s | byte equality observed |
| spawn.test | 10 | 0.001315s | 0.001275s | 0.001286s | byte equality observed |
| spawn_select.test | 10 | 0.001282s | 0.001273s | 0.001303s | byte equality observed |
| static_thread_pool.test | 10 | 0.001073s | 0.001060s | 0.001034s | byte equality observed |
| static_thread_pool_select.test | 10 | 0.001030s | 0.001026s | 0.001007s | byte equality observed |
| strand.test | 1 | 0.002020s | 0.002081s | 0.002070s | byte equality observed |
| strand_select.test | 1 | 0.001700s | 0.001730s | 0.001710s | byte equality observed |
| streambuf.test | 10 | 0.001106s | 0.001051s | 0.001062s | byte equality observed |
| streambuf_select.test | 10 | 0.001082s | 0.001074s | 0.001050s | byte equality observed |
| thread_pool.test | 10 | 0.002322s | 0.002273s | 0.002246s | byte equality observed |
| thread_pool_select.test | 10 | 0.002292s | 0.002183s | 0.002256s | byte equality observed |
| write.test | 10 | 0.001628s | 0.001513s | 0.001554s | byte equality observed |
| write_at.test | 10 | 0.001799s | 0.001659s | 0.001646s | byte equality observed |
| write_at_select.test | 10 | 0.001782s | 0.001637s | 0.001615s | byte equality observed |
| write_select.test | 10 | 0.001654s | 0.001511s | 0.001560s | byte equality observed |

Compiler command and timing observations: [cppgm](https://github.com/vishvananda/cppgm-extended/actions/runs/42709011712) · [gcc](https://github.com/vishvananda/cppgm-extended/actions/runs/42709011712) · [clang](https://github.com/vishvananda/cppgm-extended/actions/runs/42709011712).

[Reviewed outcomes and per-test records](../results.json) · [Host and toolchains](../metadata.json)
