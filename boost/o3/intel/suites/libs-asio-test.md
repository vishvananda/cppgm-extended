# libs/asio/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| any_completion_executor.test | RUN | 0.946325 | 4.752822 | 4.563984 | 0.002650 | 0.003016 | 0.002500 |
| any_completion_executor_select.test | RUN | 1.325663 | 4.833946 | 4.991384 | 0.003004 | 0.002898 | 0.002616 |
| any_completion_handler.test | RUN | 0.960683 | 4.437974 | 4.968658 | 0.002242 | 0.002553 | 0.002277 |
| any_completion_handler_select.test | RUN | 1.359250 | 4.549747 | 4.722631 | 0.002564 | 0.002456 | 0.002400 |
| any_io_executor.test | RUN | 0.909430 | 5.109484 | 4.963432 | 0.002821 | 0.002712 | 0.003262 |
| any_io_executor_select.test | RUN | 0.892582 | 5.241837 | 5.275192 | 0.003009 | 0.002649 | 0.003877 |
| append.test | RUN | 0.818543 | 4.120962 | 3.778500 | 0.002072 | 0.002160 | 0.002404 |
| append_select.test | RUN | 0.799683 | 4.242547 | 3.815490 | 0.001946 | 0.001896 | 0.002216 |
| as_tuple.test | RUN | 1.066408 | 4.904790 | 5.294839 | 0.002272 | 0.002013 | 0.001894 |
| as_tuple_select.test | RUN | 1.058174 | 5.083332 | 5.250161 | 0.002284 | 0.001972 | 0.001949 |
| associated_allocator.test | LINK | 0.213695 | 0.469352 | 0.594710 | — | — | — |
| associated_allocator_select.test | LINK | 0.207368 | 0.437150 | 0.622297 | — | — | — |
| associated_cancellation_slot.test | LINK | 0.243607 | 0.560190 | 0.655300 | — | — | — |
| associated_cancellation_slot_select.test | LINK | 0.250231 | 0.481952 | 0.640532 | — | — | — |
| associated_executor.test | LINK | 0.333383 | 0.809405 | 1.092467 | — | — | — |
| associated_executor_select.test | LINK | 0.352046 | 0.828814 | 1.218250 | — | — | — |
| associated_immediate_executor.test | LINK | 0.308435 | 0.750256 | 1.026393 | — | — | — |
| associated_immediate_executor_select.test | LINK | 0.321709 | 0.748714 | 0.995088 | — | — | — |
| associator.test | LINK | 0.168752 | 0.364480 | 0.512661 | — | — | — |
| associator_select.test | LINK | 0.174136 | 0.360635 | 0.520265 | — | — | — |
| awaitable.test | LINK | 0.166824 | 0.372656 | 0.529117 | — | — | — |
| awaitable_select.test | LINK | 0.168004 | 0.361461 | 0.518499 | — | — | — |
| basic_datagram_socket.test | LINK | 0.545835 | 1.152921 | 1.906133 | — | — | — |
| basic_datagram_socket_select.test | LINK | 0.632387 | 1.189665 | 2.001833 | — | — | — |
| basic_deadline_timer.test | LINK | 0.642094 | 1.339609 | 2.230864 | — | — | — |
| basic_deadline_timer_select.test | LINK | 0.693129 | 1.431935 | 2.289418 | — | — | — |
| basic_file.test | LINK | 0.168433 | 0.357799 | 0.517835 | — | — | — |
| basic_file_select.test | LINK | 0.170403 | 0.369047 | 0.491271 | — | — | — |
| basic_random_access_file.test | LINK | 0.166187 | 0.376502 | 0.510592 | — | — | — |
| basic_random_access_file_select.test | LINK | 0.171434 | 0.368108 | 0.494660 | — | — | — |
| basic_raw_socket.test | LINK | 0.542617 | 1.231031 | 1.959997 | — | — | — |
| basic_raw_socket_select.test | LINK | 0.629193 | 1.186358 | 2.010788 | — | — | — |
| basic_readable_pipe.test | LINK | 0.544035 | 1.080477 | 1.682910 | — | — | — |
| basic_readable_pipe_select.test | LINK | 0.600839 | 1.168578 | 2.080058 | — | — | — |
| basic_seq_packet_socket.test | LINK | 0.574616 | 1.248581 | 1.878937 | — | — | — |
| basic_seq_packet_socket_select.test | LINK | 0.536540 | 1.173185 | 1.948887 | — | — | — |
| basic_signal_set.test | LINK | 0.500695 | 1.086701 | 1.468447 | — | — | — |
| basic_signal_set_select.test | LINK | 0.516781 | 1.157344 | 1.880307 | — | — | — |
| basic_socket_acceptor.test | LINK | 0.524503 | 1.146406 | 2.272957 | — | — | — |
| basic_socket_acceptor_select.test | LINK | 0.553490 | 1.208056 | 2.081394 | — | — | — |
| basic_stream_file.test | LINK | 0.177115 | 0.381694 | 0.495379 | — | — | — |
| basic_stream_file_select.test | LINK | 0.179297 | 0.386856 | 0.490420 | — | — | — |
| basic_stream_socket.test | LINK | 0.528287 | 1.170102 | 1.872955 | — | — | — |
| basic_stream_socket_select.test | LINK | 0.812551 | 1.173882 | 1.951478 | — | — | — |
| basic_streambuf.test | LINK | 0.260347 | 0.536846 | 0.735344 | — | — | — |
| basic_streambuf_select.test | LINK | 0.259853 | 0.526106 | 0.715084 | — | — | — |
| basic_waitable_timer.test | LINK | 0.513567 | 1.086836 | 1.770426 | — | — | — |
| basic_waitable_timer_select.test | LINK | 0.504579 | 1.111569 | 1.782887 | — | — | — |
| basic_writable_pipe.test | LINK | 0.494979 | 1.090980 | 1.664058 | — | — | — |
| basic_writable_pipe_select.test | LINK | 0.522201 | 1.161365 | 2.090263 | — | — | — |
| bind_allocator.test | RUN | 0.886563 | 4.166090 | 4.302393 | 0.002586 | 0.003028 | 0.002316 |
| bind_allocator_select.test | RUN | 0.869068 | 4.502356 | 4.142792 | 0.002480 | 0.002594 | 0.002430 |
| bind_cancellation_slot.test | RUN | 0.856745 | 4.393032 | 4.084462 | 0.002026 | 0.002252 | 0.002388 |
| bind_cancellation_slot_select.test | RUN | 0.878621 | 4.249817 | 4.099413 | 0.001836 | 0.002246 | 0.002022 |
| bind_executor.test | RUN | 0.844909 | 4.172581 | 4.261285 | 0.002294 | 0.003589 | 0.002366 |
| bind_executor_select.test | RUN | 0.879236 | 4.391376 | 4.150907 | 0.002497 | 0.003231 | 0.002618 |
| bind_immediate_executor.test | RUN | 0.637892 | 3.600209 | 2.524097 | 0.001647 | 0.003046 | 0.002738 |
| bind_immediate_executor_select.test | RUN | 0.660365 | 3.462998 | 2.863855 | 0.001738 | 0.002462 | 0.002240 |
| buffer.test | RUN | 0.391890 | 1.572659 | 1.685951 | 0.001668 | 0.002098 | 0.002510 |
| buffer_select.test | RUN | 0.407402 | 1.576640 | 1.801532 | 0.001465 | 0.002604 | 0.001783 |
| buffer_registration.test | LINK | 0.434103 | 0.779750 | 0.978637 | — | — | — |
| buffer_registration_select.test | LINK | 0.321861 | 0.825312 | 1.163102 | — | — | — |
| buffered_read_stream.test | RUN | 1.438514 | 7.453196 | 6.885114 | 0.003072 | 0.002754 | 0.003129 |
| buffered_read_stream_select.test | RUN | 1.452860 | 7.510692 | 7.099273 | 0.002664 | 0.002818 | 0.002261 |
| buffered_stream.test | RUN | 1.905813 | 9.964974 | 9.408964 | 0.002046 | 0.002424 | 0.002626 |
| buffered_stream_select.test | RUN | 1.889611 | 9.783799 | 9.198740 | 0.002486 | 0.002757 | 0.002898 |
| buffered_write_stream.test | RUN | 1.632090 | 8.368656 | 9.315342 | 0.002464 | 0.003577 | 0.002509 |
| buffered_write_stream_select.test | RUN | 1.662101 | 8.783850 | 8.060651 | 0.002559 | 0.002444 | 0.002459 |
| buffers_iterator.test | RUN | 0.402778 | 0.821952 | 1.442677 | 0.001821 | 0.002364 | 0.001562 |
| buffers_iterator_select.test | RUN | 0.416946 | 0.841024 | 1.541380 | 0.001799 | 0.002426 | 0.002160 |
| cancel_after.test | RUN | 1.539647 | 6.813485 | 6.260420 | 0.005463 | 0.005833 | 0.004869 |
| cancel_after_select.test | RUN | 1.525580 | 6.655867 | 6.024744 | 0.005238 | 0.004797 | 0.005201 |
| cancel_at.test | RUN | 1.515456 | 6.722319 | 6.283578 | 0.005077 | 0.005017 | 0.004564 |
| cancel_at_select.test | RUN | 1.535540 | 6.545886 | 6.262904 | 0.004926 | 0.004085 | 0.004291 |
| cancellation_signal.test | LINK | 0.232765 | 0.502959 | 0.604421 | — | — | — |
| cancellation_signal_select.test | LINK | 0.214920 | 0.442565 | 0.680793 | — | — | — |
| cancellation_state.test | LINK | 0.221681 | 0.443321 | 0.645209 | — | — | — |
| cancellation_state_select.test | LINK | 0.220388 | 0.448778 | 0.641256 | — | — | — |
| cancellation_type.test | LINK | 0.162884 | 0.352481 | 0.519816 | — | — | — |
| cancellation_type_select.test | LINK | 0.162706 | 0.363642 | 0.509235 | — | — | — |
| co_composed.test | RUN | 0.169171 | 0.355839 | 0.495821 | 0.001910 | 0.002983 | 0.002764 |
| co_composed_select.test | RUN | 0.169241 | 0.358230 | 0.500663 | 0.001774 | 0.002380 | 0.001628 |
| co_spawn.test | LINK | 0.184902 | 0.367688 | 0.500338 | — | — | — |
| co_spawn_select.test | LINK | 0.221342 | 0.410737 | 0.523697 | — | — | — |
| completion_condition.test | LINK | 0.231055 | 0.548426 | 0.661830 | — | — | — |
| completion_condition_select.test | LINK | 0.241675 | 0.586155 | 0.693675 | — | — | — |
| compose.test | RUN | 1.085846 | 5.343180 | 5.025918 | 0.003120 | 0.002913 | 0.002539 |
| compose_select.test | RUN | 1.067381 | 5.066346 | 4.988877 | 0.003000 | 0.002657 | 0.002380 |
| composed.test | RUN | 1.076704 | 5.132571 | 4.983234 | 0.003222 | 0.003054 | 0.003364 |
| composed_select.test | RUN | 1.084405 | 5.180590 | 5.133531 | 0.003792 | 0.002931 | 0.002098 |
| config.test | RUN | 0.604118 | 3.414580 | 2.530199 | 0.002023 | 0.002848 | 0.001959 |
| config_select.test | RUN | 0.613436 | 3.565825 | 3.028360 | 0.001920 | 0.002436 | 0.002848 |
| connect.test | LINK | 1.845190 | 11.363581 | 10.515686 | — | — | — |
| connect_select.test | LINK | 1.873830 | 11.404879 | 10.676329 | — | — | — |
| connect_pipe.test | RUN | 0.893042 | 4.074302 | 3.907780 | 0.002186 | 0.002815 | 0.002238 |
| connect_pipe_select.test | RUN | 0.913550 | 4.262718 | 4.193446 | 0.001840 | 0.001821 | 0.002255 |
| consign.test | RUN | 0.773828 | 4.002257 | 3.750955 | 0.002142 | 0.001853 | 0.001922 |
| consign_select.test | RUN | 0.789428 | 4.214059 | 3.782400 | 0.002144 | 0.002134 | 0.001741 |
| coroutine.test | LINK | 0.187862 | 0.408104 | 0.562138 | — | — | — |
| coroutine_select.test | LINK | 0.181997 | 0.418993 | 0.543271 | — | — | — |
| deadline_timer.test | RUN | 1.192040 | 5.458294 | 5.223629 | 0.004587 | 0.004424 | 0.004422 |
| deadline_timer_select.test | RUN | 1.162285 | 5.280168 | 5.173015 | 0.004718 | 0.005454 | 0.005488 |
| deferred.test | RUN | 0.190242 | 0.391059 | 0.524175 | 0.002061 | 0.003304 | 0.001972 |
| deferred_select.test | RUN | 0.199303 | 0.383049 | 0.553610 | 0.001712 | 0.001966 | 0.001864 |
| detached.test | LINK | 0.208580 | 0.425718 | 0.612642 | — | — | — |
| detached_select.test | LINK | 0.222273 | 0.427360 | 0.595559 | — | — | — |
| disposition.test | RUN | 0.289393 | 1.035499 | 1.068931 | 0.003046 | 0.002406 | 0.002365 |
| disposition_select.test | RUN | 0.297679 | 1.011089 | 1.075297 | 0.001810 | 0.002083 | 0.001948 |
| error.test | RUN | 0.289452 | 0.975182 | 0.953039 | 0.001826 | 0.002278 | 0.002066 |
| error_select.test | RUN | 0.284564 | 0.963925 | 0.959296 | 0.001714 | 0.002102 | 0.001586 |
| file_base.test | LINK | 0.336225 | 0.819300 | 1.016430 | — | — | — |
| file_base_select.test | LINK | 0.351974 | 0.815834 | 1.147657 | — | — | — |
| fwd_first.test | LINK | 0.785872 | 1.774975 | 3.112985 | — | — | — |
| fwd_last.test | LINK | 0.800367 | 1.735918 | 3.042188 | — | — | — |
| generic_basic_endpoint.test | LINK | 0.287167 | 0.725817 | 1.220420 | — | — | — |
| generic_basic_endpoint_select.test | LINK | 0.292410 | 0.712453 | 1.156389 | — | — | — |
| generic_datagram_protocol.test | LINK | 1.305274 | 5.991290 | 5.793314 | — | — | — |
| generic_datagram_protocol_select.test | LINK | 1.242689 | 6.263921 | 5.555586 | — | — | — |
| generic_raw_protocol.test | LINK | 1.305484 | 6.318521 | 5.813001 | — | — | — |
| generic_raw_protocol_select.test | LINK | 1.213251 | 6.254981 | 5.800904 | — | — | — |
| generic_seq_packet_protocol.test | LINK | 1.054041 | 5.197065 | 4.957911 | — | — | — |
| generic_seq_packet_protocol_select.test | LINK | 1.070676 | 5.367457 | 4.964518 | — | — | — |
| generic_stream_protocol.test | LINK | 1.779824 | 5.758028 | 5.196961 | — | — | — |
| generic_stream_protocol_select.test | LINK | 1.140658 | 5.643347 | 5.315973 | — | — | — |
| high_resolution_timer.test | LINK | 0.521720 | 1.107123 | 2.146536 | — | — | — |
| high_resolution_timer_select.test | LINK | 0.533295 | 1.112999 | 1.850172 | — | — | — |
| immediate.test | LINK | 0.346321 | 0.825089 | 1.136199 | — | — | — |
| immediate_select.test | LINK | 0.345671 | 0.901432 | 1.074147 | — | — | — |
| inline_executor.test | RUN | 0.493066 | 1.258589 | 1.829240 | 0.002087 | 0.001994 | 0.002214 |
| inline_executor_select.test | RUN | 0.420878 | 1.256634 | 1.561083 | 0.002446 | 0.003307 | 0.001786 |
| inline_or_executor.test | RUN | 0.704755 | 3.736900 | 3.029375 | 0.002175 | 0.001910 | 0.002769 |
| inline_or_executor_select.test | RUN | 0.735077 | 3.771512 | 3.323461 | 0.001905 | 0.001874 | 0.001822 |
| io_context.test | RUN | 1.035489 | 5.695189 | 5.247746 | 0.003550 | 0.003387 | 0.003400 |
| io_context_select.test | RUN | 1.043056 | 5.603679 | 5.346558 | 0.003589 | 0.004030 | 0.003183 |
| io_context_strand.test | RUN | 0.945952 | 4.795570 | 4.655715 | 0.003090 | 0.004660 | 0.003218 |
| io_context_strand_select.test | RUN | 0.995669 | 4.697249 | 4.614512 | 0.003630 | 0.004309 | 0.003326 |
| ip_address.test | LINK | 0.351624 | 1.182770 | 1.358660 | — | — | — |
| ip_address_select.test | LINK | 0.343636 | 1.124283 | 1.372212 | — | — | — |
| ip_address_v4.test | LINK | 0.363660 | 1.166674 | 1.593073 | — | — | — |
| ip_address_v4_select.test | LINK | 0.352684 | 1.280180 | 1.676628 | — | — | — |
| ip_address_v6.test | LINK | 0.401867 | 1.551627 | 1.878396 | — | — | — |
| ip_address_v6_select.test | LINK | 0.422288 | 1.555135 | 2.232655 | — | — | — |
| ip_basic_endpoint.test | LINK | 0.314001 | 0.764975 | 1.253530 | — | — | — |
| ip_basic_endpoint_select.test | LINK | 0.319819 | 0.743293 | 1.196024 | — | — | — |
| ip_basic_resolver.test | LINK | 0.501016 | 1.090612 | 1.820368 | — | — | — |
| ip_basic_resolver_select.test | LINK | 0.515904 | 1.127951 | 1.791881 | — | — | — |
| ip_basic_resolver_entry.test | LINK | 0.175891 | 0.365700 | 0.512676 | — | — | — |
| ip_basic_resolver_entry_select.test | LINK | 0.173032 | 0.363178 | 0.509827 | — | — | — |
| ip_basic_resolver_iterator.test | LINK | 0.322715 | 0.740611 | 1.236957 | — | — | — |
| ip_basic_resolver_iterator_select.test | LINK | 0.312984 | 0.733054 | 1.202764 | — | — | — |
| ip_basic_resolver_query.test | LINK | 0.310733 | 0.708025 | 1.187407 | — | — | — |
| ip_basic_resolver_query_select.test | LINK | 0.289205 | 0.707376 | 1.135761 | — | — | — |
| ip_host_name.test | RUN | 0.309669 | 0.952578 | 1.318261 | 0.001907 | 0.001976 | 0.001947 |
| ip_host_name_select.test | RUN | 0.316923 | 0.971647 | 1.317472 | 0.001920 | 0.001763 | 0.001804 |
| ip_icmp.test | RUN | 1.583414 | 8.644124 | 7.527922 | 0.001600 | 0.002422 | 0.002086 |
| ip_icmp_select.test | RUN | 1.561152 | 8.676380 | 7.549896 | 0.001804 | 0.002399 | 0.001934 |
| ip_multicast.test | RUN | 0.908964 | 4.362689 | 4.287881 | 0.002642 | 0.002686 | 0.001714 |
| ip_multicast_select.test | RUN | 0.948219 | 4.328646 | 4.228160 | 0.002340 | 0.002302 | 0.002462 |
| ip_resolver_query_base.test | LINK | 0.201247 | 0.399258 | 0.510303 | — | — | — |
| ip_resolver_query_base_select.test | LINK | 0.195886 | 0.392340 | 0.511543 | — | — | — |
| ip_tcp.test | RUN | 2.510120 | 12.635539 | 12.572305 | 0.002719 | 0.003274 | 0.003934 |
| ip_tcp_select.test | RUN | 2.530374 | 13.190273 | 11.103176 | 0.002522 | 0.003182 | 0.002648 |
| ip_udp.test | RUN | 1.671959 | 8.981513 | 7.794004 | 0.002154 | 0.003485 | 0.002263 |
| ip_udp_select.test | RUN | 1.775170 | 9.244499 | 7.639268 | 0.002593 | 0.003168 | 0.002152 |
| ip_unicast.test | RUN | 0.888518 | 4.057307 | 3.718228 | 0.002136 | 0.002570 | 0.002232 |
| ip_unicast_select.test | RUN | 0.880175 | 4.079228 | 3.803358 | 0.002271 | 0.002389 | 0.001702 |
| ip_v6_only.test | RUN | 0.886681 | 4.032232 | 3.808943 | 0.002190 | 0.001686 | 0.001710 |
| ip_v6_only_select.test | RUN | 0.894090 | 4.257981 | 3.873302 | 0.002014 | 0.002986 | 0.001910 |
| is_read_buffered.test | RUN | 0.601905 | 1.287645 | 2.166462 | 0.001744 | 0.002986 | 0.002044 |
| is_read_buffered_select.test | RUN | 0.685329 | 1.326825 | 2.213005 | 0.001858 | 0.002311 | 0.001669 |
| is_write_buffered.test | RUN | 0.587566 | 1.259031 | 2.080765 | 0.001936 | 0.002076 | 0.001851 |
| is_write_buffered_select.test | RUN | 0.631055 | 1.355877 | 2.252742 | 0.001784 | 0.001998 | 0.002146 |
| local_basic_endpoint.test | LINK | 0.304346 | 0.737742 | 1.126851 | — | — | — |
| local_basic_endpoint_select.test | LINK | 0.303867 | 0.731354 | 1.157018 | — | — | — |
| local_connect_pair.test | LINK | 0.779176 | 4.108480 | 3.650925 | — | — | — |
| local_connect_pair_select.test | LINK | 0.767662 | 4.128021 | 3.543695 | — | — | — |
| local_datagram_protocol.test | LINK | 1.063256 | 5.468669 | 4.843421 | — | — | — |
| local_datagram_protocol_select.test | LINK | 1.056801 | 5.225510 | 4.921676 | — | — | — |
| local_seq_packet_protocol.test | LINK | 1.052126 | 5.225091 | 4.936168 | — | — | — |
| local_seq_packet_protocol_select.test | LINK | 1.062053 | 5.260835 | 4.867219 | — | — | — |
| local_stream_protocol.test | LINK | 0.993704 | 4.905429 | 4.597147 | — | — | — |
| local_stream_protocol_select.test | LINK | 0.987424 | 5.071820 | 4.539074 | — | — | — |
| placeholders.test | LINK | 0.188971 | 0.420599 | 0.545727 | — | — | — |
| placeholders_select.test | LINK | 0.193246 | 0.392765 | 0.510282 | — | — | — |
| posix_basic_descriptor.test | LINK | 0.495830 | 1.145004 | 1.641974 | — | — | — |
| posix_basic_descriptor_select.test | LINK | 0.515723 | 1.208749 | 2.074153 | — | — | — |
| posix_basic_stream_descriptor.test | LINK | 0.523206 | 1.112422 | 1.820445 | — | — | — |
| posix_basic_stream_descriptor_select.test | LINK | 0.534824 | 1.162947 | 2.012000 | — | — | — |
| posix_descriptor_base.test | LINK | 0.212333 | 0.427141 | 0.583349 | — | — | — |
| posix_descriptor_base_select.test | LINK | 0.208396 | 0.432585 | 0.557845 | — | — | — |
| posix_stream_descriptor.test | LINK | 1.009375 | 5.216042 | 4.490647 | — | — | — |
| posix_stream_descriptor_select.test | LINK | 1.013068 | 5.380034 | 4.743727 | — | — | — |
| prepend.test | RUN | 0.777273 | 4.098396 | 3.800068 | 0.003060 | 0.002462 | 0.001910 |
| prepend_select.test | RUN | 0.784500 | 4.394803 | 3.637596 | 0.002132 | 0.002230 | 0.001866 |
| random_access_file.test | LINK | 0.461858 | 1.016714 | 1.274033 | — | — | — |
| random_access_file_select.test | LINK | 0.477684 | 1.111954 | 1.643241 | — | — | — |
| read.test | RUN | 2.626287 | 25.558302 | 23.399968 | 0.002835 | 0.003358 | 0.003454 |
| read_select.test | RUN | 2.635643 | 23.940633 | 21.585483 | 0.002997 | 0.003802 | 0.003453 |
| read_at.test | RUN | 2.566336 | 35.862640 | 29.551137 | 0.003034 | 0.005251 | 0.003480 |
| read_at_select.test | RUN | 2.648576 | 35.072908 | 29.819805 | 0.003235 | 0.004565 | 0.003856 |
| read_until.test | RUN | 1.423618 | 10.177530 | 8.412977 | 0.003283 | 0.002803 | 0.002469 |
| read_until_select.test | RUN | 1.404024 | 9.948620 | 8.282209 | 0.002508 | 0.003382 | 0.002464 |
| readable_pipe.test | LINK | 0.801745 | 4.259502 | 3.697456 | — | — | — |
| readable_pipe_select.test | LINK | 0.842829 | 4.250755 | 3.962837 | — | — | — |
| recycling_allocator.test | RUN | 0.244472 | 0.621294 | 0.759473 | 0.003478 | 0.002421 | 0.002160 |
| recycling_allocator_select.test | RUN | 0.255290 | 0.610729 | 0.759223 | 0.001970 | 0.002126 | 0.002724 |
| redirect_disposition.test | LINK | 1.134339 | 5.817797 | 5.682657 | — | — | — |
| redirect_disposition_select.test | LINK | 1.160625 | 5.729552 | 5.699576 | — | — | — |
| redirect_error.test | LINK | 1.013039 | 4.876054 | 4.965143 | — | — | — |
| redirect_error_select.test | LINK | 0.995836 | 4.744917 | 4.947505 | — | — | — |
| registered_buffer.test | LINK | 0.251065 | 0.511887 | 0.694739 | — | — | — |
| registered_buffer_select.test | LINK | 0.245933 | 0.506393 | 0.703284 | — | — | — |
| signal_set.test | RUN | 0.783464 | 4.040731 | 3.354879 | 0.001842 | 0.003181 | 0.001620 |
| signal_set_select.test | RUN | 0.799568 | 4.249770 | 3.694516 | 0.001854 | 0.001742 | 0.002461 |
| signal_set_base.test | LINK | 0.191527 | 0.401577 | 0.512718 | — | — | — |
| signal_set_base_select.test | LINK | 0.196012 | 0.391558 | 0.522956 | — | — | — |
| socket_base.test | RUN | 1.034172 | 6.101693 | 5.344637 | 0.003330 | 0.002004 | 0.002246 |
| socket_base_select.test | RUN | 1.029358 | 6.052538 | 5.160046 | 0.002462 | 0.002589 | 0.001779 |
| spawn.test | RUN | 1.788188 | 9.793092 | 8.649852 | 0.002270 | 0.002586 | 0.002408 |
| spawn_select.test | RUN | 1.716443 | 9.707609 | 8.730777 | 0.002192 | 0.002700 | 0.002490 |
| static_thread_pool.test | RUN | 0.450616 | 0.985963 | 1.279991 | 0.001890 | 0.002812 | 0.002299 |
| static_thread_pool_select.test | RUN | 0.494499 | 1.040536 | 1.659863 | 0.001804 | 0.002757 | 0.002074 |
| steady_timer.test | LINK | 0.501959 | 1.072658 | 1.743120 | — | — | — |
| steady_timer_select.test | LINK | 0.551910 | 1.138492 | 1.888456 | — | — | — |
| strand.test | RUN | 1.129819 | 5.657308 | 5.252583 | 0.003795 | 0.004008 | 0.003336 |
| strand_select.test | RUN | 1.146981 | 5.481665 | 5.336261 | 0.002974 | 0.003904 | 0.003461 |
| stream_file.test | LINK | 0.475192 | 1.030623 | 1.351415 | — | — | — |
| stream_file_select.test | LINK | 0.537478 | 1.067606 | 1.725181 | — | — | — |
| streambuf.test | RUN | 0.320389 | 0.823359 | 0.879244 | 0.001892 | 0.003403 | 0.003331 |
| streambuf_select.test | RUN | 0.322955 | 0.839059 | 0.885858 | 0.001875 | 0.001726 | 0.002160 |
| system_timer.test | LINK | 1.115189 | 5.221128 | 4.767525 | — | — | — |
| system_timer_select.test | LINK | 1.168189 | 5.335033 | 4.757348 | — | — | — |
| system_context.test | LINK | 0.498504 | 0.952435 | 1.259817 | — | — | — |
| system_context_select.test | LINK | 0.484992 | 1.038861 | 1.665878 | — | — | — |
| system_executor.test | LINK | 0.617222 | 2.990467 | 2.277378 | — | — | — |
| system_executor_select.test | LINK | 0.623405 | 3.000020 | 2.767866 | — | — | — |
| this_coro.test | LINK | 0.171487 | 0.429700 | 0.484070 | — | — | — |
| this_coro_select.test | LINK | 0.221267 | 0.368436 | 0.500760 | — | — | — |
| thread_pool.test | RUN | 0.760796 | 3.812433 | 2.973894 | 0.026815 | 0.023126 | 0.022024 |
| thread_pool_select.test | RUN | 0.776280 | 4.016467 | 3.413151 | 0.024866 | 0.027294 | 0.028327 |
| time_traits.test | LINK | 0.400238 | 0.763428 | 1.012181 | — | — | — |
| time_traits_select.test | LINK | 0.405949 | 0.764992 | 1.037515 | — | — | — |
| ts_buffer.test | LINK | 0.369138 | 0.858463 | 1.116986 | — | — | — |
| ts_buffer_select.test | LINK | 0.433023 | 0.844029 | 1.093100 | — | — | — |
| ts_executor.test | LINK | 0.561611 | 1.192806 | 1.837661 | — | — | — |
| ts_executor_select.test | LINK | 0.614016 | 1.270462 | 2.125482 | — | — | — |
| ts_internet.test | LINK | 0.607166 | 1.349189 | 2.325759 | — | — | — |
| ts_internet_select.test | LINK | 0.647898 | 1.388966 | 2.279722 | — | — | — |
| ts_io_context.test | LINK | 0.486622 | 1.001185 | 1.280168 | — | — | — |
| ts_io_context_select.test | LINK | 0.480636 | 1.068173 | 1.702690 | — | — | — |
| ts_net.test | LINK | 0.684058 | 1.652164 | 2.698582 | — | — | — |
| ts_net_select.test | LINK | 0.713445 | 1.573025 | 2.652288 | — | — | — |
| ts_netfwd.test | LINK | 0.675833 | 1.525899 | 2.566751 | — | — | — |
| ts_netfwd_select.test | LINK | 0.691723 | 1.604917 | 2.591853 | — | — | — |
| ts_socket.test | LINK | 0.554655 | 1.230170 | 2.083188 | — | — | — |
| ts_socket_select.test | LINK | 0.581405 | 1.255813 | 2.006064 | — | — | — |
| ts_timer.test | LINK | 0.485866 | 1.104006 | 1.744430 | — | — | — |
| ts_timer_select.test | LINK | 0.514217 | 1.099204 | 2.071124 | — | — | — |
| use_awaitable.test | LINK | 0.166609 | 0.365381 | 0.507360 | — | — | — |
| use_awaitable_select.test | LINK | 0.173977 | 0.359636 | 0.506575 | — | — | — |
| wait_traits.test | LINK | 0.166562 | 0.364199 | 0.505151 | — | — | — |
| wait_traits_select.test | LINK | 0.173507 | 0.371412 | 0.539097 | — | — | — |
| windows_basic_object_handle.test | LINK | 0.165588 | 0.377054 | 0.483453 | — | — | — |
| windows_basic_object_handle_select.test | LINK | 0.166051 | 0.374046 | 0.516541 | — | — | — |
| windows_basic_overlapped_handle.test | LINK | 0.163629 | 0.372830 | 0.493716 | — | — | — |
| windows_basic_overlapped_handle_select.test | LINK | 0.161890 | 0.377880 | 0.517691 | — | — | — |
| windows_basic_random_access_handle.test | LINK | 0.162874 | 0.379312 | 0.506055 | — | — | — |
| windows_basic_random_access_handle_select.test | LINK | 0.168053 | 0.365813 | 0.504770 | — | — | — |
| windows_basic_stream_handle.test | LINK | 0.176138 | 0.373994 | 0.581358 | — | — | — |
| windows_basic_stream_handle_select.test | LINK | 0.173169 | 0.376735 | 0.519515 | — | — | — |
| windows_object_handle.test | LINK | 0.472400 | 1.010541 | 1.281520 | — | — | — |
| windows_object_handle_select.test | LINK | 0.475777 | 1.073358 | 1.663880 | — | — | — |
| windows_overlapped_ptr.test | LINK | 0.487611 | 1.044088 | 1.371901 | — | — | — |
| windows_overlapped_ptr_select.test | LINK | 0.506576 | 1.131165 | 1.808180 | — | — | — |
| windows_random_access_handle.test | LINK | 0.457323 | 0.983208 | 1.259990 | — | — | — |
| windows_random_access_handle_select.test | LINK | 0.484485 | 1.070370 | 1.722326 | — | — | — |
| windows_stream_handle.test | LINK | 0.450499 | 0.979214 | 1.292299 | — | — | — |
| windows_stream_handle_select.test | LINK | 0.495984 | 1.042686 | 2.040474 | — | — | — |
| writable_pipe.test | LINK | 0.811016 | 4.197982 | 3.918061 | — | — | — |
| writable_pipe_select.test | LINK | 0.836480 | 4.357022 | 3.980833 | — | — | — |
| write.test | RUN | 2.551046 | 21.005468 | 21.581427 | 0.003468 | 0.004446 | 0.002780 |
| write_select.test | RUN | 2.552329 | 21.131843 | 21.804041 | 0.003199 | 0.004028 | 0.003251 |
| write_at.test | RUN | 2.692279 | 27.706372 | 27.155012 | 0.003517 | 0.002668 | 0.003876 |
| write_at_select.test | RUN | 2.710797 | 27.611312 | 27.035929 | 0.003504 | 0.004090 | 0.003568 |
| any_executor.test | RUN | 1.009965 | 9.746459 | 8.624129 | 0.004641 | 0.003754 | 0.002976 |
| blocking.test | RUN | 2.399610 | 12.274445 | 17.908987 | 0.011336 | 0.010959 | 0.010544 |
| blocking_adaptation.test | RUN | 1.206857 | 5.426293 | 7.829447 | 0.006291 | 0.005802 | 0.005342 |
| context_as.test | RUN | 0.639134 | 3.809230 | 2.837826 | 0.001982 | 0.002656 | 0.002226 |
| executor.test | RUN | 0.168626 | 0.387020 | 0.499488 | 0.001946 | 0.002824 | 0.002082 |
| inline_exception_handling.test | RUN | 2.445813 | 12.291565 | 17.401827 | 0.011347 | 0.010836 | 0.010843 |
| invocable_archetype.test | RUN | 0.165952 | 0.387980 | 0.487704 | 0.002252 | 0.001758 | 0.002264 |
| mapping.test | RUN | 2.545920 | 13.500192 | 19.941367 | 0.011804 | 0.013196 | 0.014790 |
| outstanding_work.test | RUN | 1.114161 | 5.160220 | 7.884859 | 0.005963 | 0.006116 | 0.006349 |
| prefer_only.test | RUN | 0.389504 | 2.163999 | 1.771815 | 0.002084 | 0.002437 | 0.002160 |
| relationship.test | RUN | 1.136838 | 5.276689 | 7.622294 | 0.006524 | 0.006063 | 0.005906 |

## Native runtime replay

Child CPU of successful native tests, with process startup included; GDB and expected-failure executions are correctness-only. Two warmups per compiler, four A/A observations per reference and mirrored ABBA/BAAB blocks. Short tests repeat up to ten times, identically across compilers; CPU is divided by repetitions before summing tests. Executable hashes and output checks stay outside process timers. All runs must pass their original native oracle. Output hashes record equality or variation across the complete campaign; test reports may print times or addresses.

| Reference | CPU speedup ↑ | Paired blocks | A/A speedup |
|---|---:|---|---:|
| gcc | 1.036×† | 1.094×, 0.977× | 1.003× |
| clang | 0.935×† | 0.863×, 1.006× | 1.004× |

† A/A drift or paired-block spread exceeds 5%; small differences are unresolved. 1375 native tests in the full cohort; 1353 have byte-identical output throughout. [Manifest](https://github.com/vishvananda/cppgm-extended/actions/runs/42701283202), [paired observations](https://github.com/vishvananda/cppgm-extended/actions/runs/42701283202) and [per-test summaries](https://github.com/vishvananda/cppgm-extended/actions/runs/42701283202).

| Test | Repeats | CPU cppgm | GCC | Clang | Output check |
|---|---:|---:|---:|---:|---|
| any_completion_executor.test | 6 | 0.002811s | 0.002732s | 0.002721s | byte equality observed |
| any_completion_executor_select.test | 4 | 0.002955s | 0.002706s | 0.002753s | byte equality observed |
| any_completion_handler.test | 6 | 0.002203s | 0.002035s | 0.001964s | byte equality observed |
| any_completion_handler_select.test | 5 | 0.002052s | 0.002002s | 0.001991s | byte equality observed |
| any_io_executor.test | 6 | 0.002600s | 0.002705s | 0.002830s | byte equality observed |
| any_io_executor_select.test | 5 | 0.002685s | 0.002718s | 0.002711s | byte equality observed |
| append.test | 7 | 0.001817s | 0.001822s | 0.001798s | byte equality observed |
| append_select.test | 6 | 0.001756s | 0.001793s | 0.001738s | byte equality observed |
| as_tuple.test | 6 | 0.001847s | 0.001833s | 0.001838s | byte equality observed |
| as_tuple_select.test | 7 | 0.001786s | 0.001971s | 0.001707s | byte equality observed |
| bind_allocator.test | 1 | 0.002534s | 0.002553s | 0.002638s | byte equality observed |
| bind_allocator_select.test | 1 | 0.002142s | 0.002246s | 0.002123s | byte equality observed |
| bind_cancellation_slot.test | 7 | 0.001871s | 0.001827s | 0.001841s | byte equality observed |
| bind_cancellation_slot_select.test | 7 | 0.001829s | 0.001831s | 0.001806s | byte equality observed |
| bind_executor.test | 1 | 0.002660s | 0.002469s | 0.002705s | byte equality observed |
| bind_executor_select.test | 1 | 0.002506s | 0.002690s | 0.002365s | byte equality observed |
| bind_immediate_executor.test | 7 | 0.001786s | 0.001844s | 0.001669s | byte equality observed |
| bind_immediate_executor_select.test | 8 | 0.001816s | 0.001853s | 0.001897s | byte equality observed |
| buffer.test | 7 | 0.001744s | 0.001737s | 0.001672s | byte equality observed |
| buffer_select.test | 6 | 0.001735s | 0.001755s | 0.001673s | byte equality observed |
| buffered_read_stream.test | 5 | 0.002257s | 0.002076s | 0.002202s | byte equality observed |
| buffered_read_stream_select.test | 6 | 0.002113s | 0.002043s | 0.002014s | byte equality observed |
| buffered_stream.test | 5 | 0.002232s | 0.002046s | 0.002048s | byte equality observed |
| buffered_stream_select.test | 6 | 0.002098s | 0.002073s | 0.001954s | byte equality observed |
| buffered_write_stream.test | 6 | 0.002135s | 0.002043s | 0.002010s | byte equality observed |
| buffered_write_stream_select.test | 6 | 0.002107s | 0.002118s | 0.001991s | byte equality observed |
| buffers_iterator.test | 5 | 0.001708s | 0.001639s | 0.001719s | byte equality observed |
| buffers_iterator_select.test | 5 | 0.001671s | 0.001614s | 0.001748s | byte equality observed |
| cancel_after.test | 1 | 0.004707s | 0.004961s | 0.004692s | byte equality observed |
| cancel_after_select.test | 1 | 0.004945s | 0.004606s | 0.004571s | byte equality observed |
| cancel_at.test | 1 | 0.004376s | 0.004586s | 0.004570s | byte equality observed |
| cancel_at_select.test | 1 | 0.004666s | 0.004580s | 0.004622s | byte equality observed |
| co_composed.test | 8 | 0.001677s | 0.001582s | 0.001618s | byte equality observed |
| co_composed_select.test | 7 | 0.001619s | 0.001578s | 0.001657s | byte equality observed |
| compose.test | 1 | 0.002744s | 0.002602s | 0.002574s | byte equality observed |
| compose_select.test | 1 | 0.002422s | 0.002437s | 0.002295s | byte equality observed |
| composed.test | 1 | 0.002662s | 0.002792s | 0.002435s | byte equality observed |
| composed_select.test | 1 | 0.002333s | 0.002429s | 0.002595s | byte equality observed |
| config.test | 5 | 0.001818s | 0.001683s | 0.001675s | byte equality observed |
| config_select.test | 6 | 0.001830s | 0.001669s | 0.001865s | byte equality observed |
| connect_pipe.test | 8 | 0.001844s | 0.001741s | 0.001747s | byte equality observed |
| connect_pipe_select.test | 8 | 0.001806s | 0.001747s | 0.001756s | byte equality observed |
| consign.test | 8 | 0.001822s | 0.001796s | 0.001722s | byte equality observed |
| consign_select.test | 8 | 0.001831s | 0.001728s | 0.001810s | byte equality observed |
| deadline_timer.test | 1 | 0.004239s | 0.004450s | 0.004444s | byte equality observed |
| deadline_timer_select.test | 1 | 0.004412s | 0.004739s | 0.004430s | byte equality observed |
| deferred.test | 6 | 0.001855s | 0.001779s | 0.001786s | byte equality observed |
| deferred_select.test | 7 | 0.001718s | 0.001653s | 0.001956s | byte equality observed |
| disposition.test | 6 | 0.001901s | 0.001805s | 0.002018s | byte equality observed |
| disposition_select.test | 6 | 0.001790s | 0.001778s | 0.001910s | byte equality observed |
| error.test | 6 | 0.001713s | 0.001690s | 0.001938s | byte equality observed |
| error_select.test | 7 | 0.001757s | 0.001709s | 0.001830s | byte equality observed |
| inline_executor.test | 7 | 0.001749s | 0.001734s | 0.001916s | byte equality observed |
| inline_executor_select.test | 6 | 0.001768s | 0.001790s | 0.001841s | byte equality observed |
| inline_or_executor.test | 7 | 0.001787s | 0.001729s | 0.001797s | byte equality observed |
| inline_or_executor_select.test | 6 | 0.001828s | 0.001700s | 0.001757s | byte equality observed |
| io_context.test | 1 | 0.003192s | 0.003014s | 0.003276s | byte equality observed |
| io_context_select.test | 1 | 0.003028s | 0.002971s | 0.002960s | byte equality observed |
| io_context_strand.test | 1 | 0.002978s | 0.002683s | 0.002911s | byte equality observed |
| io_context_strand_select.test | 1 | 0.002971s | 0.003126s | 0.002804s | byte equality observed |
| ip_host_name.test | 6 | 0.001851s | 0.001766s | 0.001629s | byte equality observed |
| ip_host_name_select.test | 9 | 0.001633s | 0.001675s | 0.001642s | byte equality observed |
| ip_icmp.test | 8 | 0.001724s | 0.001717s | 0.001779s | byte equality observed |
| ip_icmp_select.test | 7 | 0.001724s | 0.001753s | 0.002005s | byte equality observed |
| ip_multicast.test | 8 | 0.001951s | 0.001849s | 0.001876s | byte equality observed |
| ip_multicast_select.test | 8 | 0.001877s | 0.002037s | 0.001798s | byte equality observed |
| ip_tcp.test | 6 | 0.002692s | 0.002501s | 0.002552s | byte equality observed |
| ip_tcp_select.test | 6 | 0.002391s | 0.002166s | 0.002371s | byte equality observed |
| ip_udp.test | 7 | 0.001986s | 0.001690s | 0.001801s | byte equality observed |
| ip_udp_select.test | 7 | 0.002061s | 0.001830s | 0.001796s | byte equality observed |
| ip_unicast.test | 8 | 0.001777s | 0.001699s | 0.001725s | byte equality observed |
| ip_unicast_select.test | 6 | 0.001860s | 0.001785s | 0.001812s | byte equality observed |
| ip_v6_only.test | 6 | 0.001941s | 0.001716s | 0.001704s | byte equality observed |
| ip_v6_only_select.test | 8 | 0.001839s | 0.001837s | 0.001674s | byte equality observed |
| is_read_buffered.test | 9 | 0.001744s | 0.001693s | 0.001525s | byte equality observed |
| is_read_buffered_select.test | 8 | 0.001699s | 0.001578s | 0.001639s | byte equality observed |
| is_write_buffered.test | 9 | 0.001683s | 0.001610s | 0.001594s | byte equality observed |
| is_write_buffered_select.test | 7 | 0.001786s | 0.001675s | 0.001540s | byte equality observed |
| prepend.test | 8 | 0.001818s | 0.001751s | 0.001766s | byte equality observed |
| prepend_select.test | 7 | 0.001786s | 0.001640s | 0.001682s | byte equality observed |
| read.test | 6 | 0.002686s | 0.002509s | 0.002397s | byte equality observed |
| read_at.test | 5 | 0.003262s | 0.002965s | 0.002846s | byte equality observed |
| read_at_select.test | 5 | 0.003276s | 0.003087s | 0.002887s | byte equality observed |
| read_select.test | 4 | 0.002812s | 0.002555s | 0.002368s | byte equality observed |
| read_until.test | 6 | 0.002046s | 0.001978s | 0.001893s | byte equality observed |
| read_until_select.test | 6 | 0.002063s | 0.001963s | 0.001933s | byte equality observed |
| recycling_allocator.test | 9 | 0.001690s | 0.001571s | 0.001532s | byte equality observed |
| recycling_allocator_select.test | 9 | 0.001752s | 0.001644s | 0.001605s | byte equality observed |
| signal_set.test | 9 | 0.001748s | 0.001803s | 0.001551s | byte equality observed |
| signal_set_select.test | 8 | 0.001674s | 0.001747s | 0.001559s | byte equality observed |
| socket_base.test | 8 | 0.001888s | 0.001860s | 0.001766s | byte equality observed |
| socket_base_select.test | 8 | 0.001869s | 0.001927s | 0.001774s | byte equality observed |
| spawn.test | 7 | 0.002070s | 0.002103s | 0.001957s | byte equality observed |
| spawn_select.test | 7 | 0.002040s | 0.001963s | 0.001900s | byte equality observed |
| static_thread_pool.test | 7 | 0.001671s | 0.001616s | 0.001596s | byte equality observed |
| static_thread_pool_select.test | 8 | 0.001597s | 0.001586s | 0.001566s | byte equality observed |
| strand.test | 1 | 0.003288s | 0.003138s | 0.002766s | byte equality observed |
| strand_select.test | 1 | 0.003136s | 0.003097s | 0.002736s | byte equality observed |
| streambuf.test | 5 | 0.001958s | 0.002047s | 0.001916s | byte equality observed |
| streambuf_select.test | 7 | 0.001850s | 0.001755s | 0.001789s | byte equality observed |
| thread_pool.test | 1 | 0.026355s | 0.030843s | 0.027028s | byte equality observed |
| thread_pool_select.test | 1 | 0.025356s | 0.029530s | 0.024812s | byte equality observed |
| write.test | 5 | 0.002925s | 0.003090s | 0.002775s | byte equality observed |
| write_at.test | 3 | 0.003307s | 0.003061s | 0.003085s | byte equality observed |
| write_at_select.test | 4 | 0.003162s | 0.002914s | 0.003098s | byte equality observed |
| write_select.test | 6 | 0.002916s | 0.002688s | 0.002859s | byte equality observed |

Compiler command and timing observations: [cppgm](https://github.com/vishvananda/cppgm-extended/actions/runs/42701283202) · [gcc](https://github.com/vishvananda/cppgm-extended/actions/runs/42701283202) · [clang](https://github.com/vishvananda/cppgm-extended/actions/runs/42701283202).

[Reviewed outcomes and per-test records](../results.json) · [Host and toolchains](../metadata.json)
