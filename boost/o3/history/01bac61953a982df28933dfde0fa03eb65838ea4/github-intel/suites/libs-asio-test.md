# libs/asio/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| any_completion_executor.test | RUN | 0.524049 | 3.385163 | 2.896672 | 0.002000 | 0.001800 | 0.001831 |
| any_completion_executor_select.test | RUN | 0.536754 | 3.396247 | 3.038534 | 0.001588 | 0.001663 | 0.001703 |
| any_completion_handler.test | RUN | 0.584902 | 3.080509 | 2.685361 | 0.001491 | 0.001352 | 0.001351 |
| any_completion_handler_select.test | RUN | 0.609520 | 3.190040 | 2.851873 | 0.001387 | 0.001372 | 0.001222 |
| any_io_executor.test | RUN | 0.553834 | 3.621087 | 3.108531 | 0.001789 | 0.001667 | 0.001827 |
| any_io_executor_select.test | RUN | 0.570296 | 3.549317 | 3.277743 | 0.001782 | 0.001818 | 0.001480 |
| append.test | RUN | 0.517560 | 2.929299 | 2.385091 | 0.001146 | 0.001327 | 0.001231 |
| append_select.test | RUN | 0.521119 | 2.964894 | 2.347682 | 0.001280 | 0.001469 | 0.001160 |
| as_tuple.test | RUN | 0.666033 | 3.509492 | 3.307084 | 0.001461 | 0.001433 | 0.001214 |
| as_tuple_select.test | RUN | 0.675378 | 3.474867 | 3.292898 | 0.001246 | 0.001325 | 0.001206 |
| associated_allocator.test | LINK | 0.126621 | 0.272305 | 0.350820 | — | — | — |
| associated_allocator_select.test | LINK | 0.128970 | 0.279608 | 0.350413 | — | — | — |
| associated_cancellation_slot.test | LINK | 0.139451 | 0.296390 | 0.374652 | — | — | — |
| associated_cancellation_slot_select.test | LINK | 0.142537 | 0.303769 | 0.378132 | — | — | — |
| associated_executor.test | LINK | 0.209017 | 0.530735 | 0.612803 | — | — | — |
| associated_executor_select.test | LINK | 0.202666 | 0.534054 | 0.614005 | — | — | — |
| associated_immediate_executor.test | LINK | 0.185781 | 0.495694 | 0.556585 | — | — | — |
| associated_immediate_executor_select.test | LINK | 0.188759 | 0.493215 | 0.552085 | — | — | — |
| associator.test | LINK | 0.107356 | 0.232891 | 0.288131 | — | — | — |
| associator_select.test | LINK | 0.103195 | 0.233943 | 0.292098 | — | — | — |
| awaitable.test | LINK | 0.103444 | 0.236361 | 0.286943 | — | — | — |
| awaitable_select.test | LINK | 0.106588 | 0.228933 | 0.275757 | — | — | — |
| basic_datagram_socket.test | LINK | 0.323666 | 0.748301 | 1.091505 | — | — | — |
| basic_datagram_socket_select.test | LINK | 0.330121 | 0.773224 | 1.099423 | — | — | — |
| basic_deadline_timer.test | LINK | 0.391826 | 0.880714 | 1.244614 | — | — | — |
| basic_deadline_timer_select.test | LINK | 0.402094 | 0.900857 | 1.260926 | — | — | — |
| basic_file.test | LINK | 0.102921 | 0.233619 | 0.290319 | — | — | — |
| basic_file_select.test | LINK | 0.105539 | 0.240609 | 0.284826 | — | — | — |
| basic_random_access_file.test | LINK | 0.102672 | 0.236784 | 0.286542 | — | — | — |
| basic_random_access_file_select.test | LINK | 0.105588 | 0.240450 | 0.284245 | — | — | — |
| basic_raw_socket.test | LINK | 0.330115 | 0.766365 | 1.083912 | — | — | — |
| basic_raw_socket_select.test | LINK | 0.325868 | 0.773480 | 1.135134 | — | — | — |
| basic_readable_pipe.test | LINK | 0.313865 | 0.717661 | 0.958128 | — | — | — |
| basic_readable_pipe_select.test | LINK | 0.340054 | 0.769208 | 1.144557 | — | — | — |
| basic_seq_packet_socket.test | LINK | 0.330148 | 0.758976 | 1.095681 | — | — | — |
| basic_seq_packet_socket_select.test | LINK | 0.333614 | 0.782694 | 1.118195 | — | — | — |
| basic_signal_set.test | LINK | 0.302365 | 0.697465 | 0.854685 | — | — | — |
| basic_signal_set_select.test | LINK | 0.321309 | 0.738359 | 1.068287 | — | — | — |
| basic_socket_acceptor.test | LINK | 0.324753 | 0.757652 | 1.101504 | — | — | — |
| basic_socket_acceptor_select.test | LINK | 0.338270 | 0.770669 | 1.124628 | — | — | — |
| basic_stream_file.test | LINK | 0.102794 | 0.240114 | 0.281343 | — | — | — |
| basic_stream_file_select.test | LINK | 0.104760 | 0.236798 | 0.283139 | — | — | — |
| basic_stream_socket.test | LINK | 0.334094 | 0.763380 | 1.075131 | — | — | — |
| basic_stream_socket_select.test | LINK | 0.337686 | 0.770912 | 1.109437 | — | — | — |
| basic_streambuf.test | LINK | 0.158316 | 0.350475 | 0.425726 | — | — | — |
| basic_streambuf_select.test | LINK | 0.159590 | 0.346109 | 0.424371 | — | — | — |
| basic_waitable_timer.test | LINK | 0.306061 | 0.710014 | 1.015554 | — | — | — |
| basic_waitable_timer_select.test | LINK | 0.313071 | 0.726553 | 1.045356 | — | — | — |
| basic_writable_pipe.test | LINK | 0.310047 | 0.709238 | 0.949939 | — | — | — |
| basic_writable_pipe_select.test | LINK | 0.332204 | 0.762807 | 1.156874 | — | — | — |
| bind_allocator.test | RUN | 0.546557 | 2.915032 | 2.607189 | 0.001972 | 0.001794 | 0.001547 |
| bind_allocator_select.test | RUN | 0.549917 | 2.984842 | 2.555802 | 0.001498 | 0.001700 | 0.001546 |
| bind_cancellation_slot.test | RUN | 0.557430 | 2.972274 | 2.622643 | 0.000948 | 0.000999 | 0.000875 |
| bind_cancellation_slot_select.test | RUN | 0.552953 | 2.947910 | 2.528560 | 0.000964 | 0.000933 | 0.000865 |
| bind_executor.test | RUN | 0.382940 | 2.007062 | 1.643592 | 0.001588 | 0.001627 | 0.001491 |
| bind_executor_select.test | RUN | 0.390506 | 2.043705 | 1.608011 | 0.001412 | 0.001327 | 0.001282 |
| bind_immediate_executor.test | RUN | 0.285286 | 1.676123 | 1.032435 | 0.000961 | 0.000965 | 0.000873 |
| bind_immediate_executor_select.test | RUN | 0.296144 | 1.635902 | 1.140651 | 0.000951 | 0.000920 | 0.000847 |
| buffer.test | RUN | 0.172545 | 0.685654 | 0.606054 | 0.000923 | 0.000935 | 0.000842 |
| buffer_select.test | RUN | 0.174352 | 0.685590 | 0.610390 | 0.000902 | 0.000904 | 0.000846 |
| buffer_registration.test | LINK | 0.131688 | 0.336053 | 0.351898 | — | — | — |
| buffer_registration_select.test | LINK | 0.132811 | 0.337790 | 0.351486 | — | — | — |
| buffered_read_stream.test | RUN | 0.645810 | 4.479495 | 3.670177 | 0.001223 | 0.001608 | 0.001548 |
| buffered_read_stream_select.test | RUN | 0.656016 | 4.625243 | 3.639557 | 0.001161 | 0.001570 | 0.001521 |
| buffered_stream.test | RUN | 0.826156 | 7.157862 | 6.589706 | 0.001586 | 0.001678 | 0.001572 |
| buffered_stream_select.test | RUN | 0.875682 | 7.179364 | 6.458849 | 0.001540 | 0.001491 | 0.001692 |
| buffered_write_stream.test | RUN | 1.059479 | 6.240942 | 5.670450 | 0.001598 | 0.001695 | 0.001320 |
| buffered_write_stream_select.test | RUN | 1.075271 | 6.269185 | 5.507481 | 0.001558 | 0.001563 | 0.001557 |
| buffers_iterator.test | RUN | 0.317131 | 0.539728 | 0.880969 | 0.001242 | 0.001394 | 0.001246 |
| buffers_iterator_select.test | RUN | 0.310761 | 0.539600 | 0.883243 | 0.001171 | 0.001227 | 0.001062 |
| cancel_after.test | RUN | 0.829975 | 4.821643 | 4.053333 | 0.003534 | 0.002752 | 0.002775 |
| cancel_after_select.test | RUN | 0.851226 | 4.813671 | 4.030419 | 0.002775 | 0.002715 | 0.002226 |
| cancel_at.test | RUN | 0.733886 | 4.226102 | 4.023761 | 0.002853 | 0.002347 | 0.002255 |
| cancel_at_select.test | RUN | 0.711255 | 4.209598 | 3.979256 | 0.002763 | 0.002237 | 0.002127 |
| cancellation_signal.test | LINK | 0.093984 | 0.197647 | 0.234901 | — | — | — |
| cancellation_signal_select.test | LINK | 0.094653 | 0.197097 | 0.282965 | — | — | — |
| cancellation_state.test | LINK | 0.093069 | 0.195273 | 0.367141 | — | — | — |
| cancellation_state_select.test | LINK | 0.095430 | 0.194350 | 0.366859 | — | — | — |
| cancellation_type.test | LINK | 0.079694 | 0.154229 | 0.293107 | — | — | — |
| cancellation_type_select.test | LINK | 0.107336 | 0.153976 | 0.285226 | — | — | — |
| co_composed.test | RUN | 0.101188 | 0.155138 | 0.295977 | 0.000999 | 0.000886 | 0.001128 |
| co_composed_select.test | RUN | 0.100348 | 0.152116 | 0.287307 | 0.001034 | 0.000867 | 0.001053 |
| co_spawn.test | LINK | 0.103376 | 0.202746 | 0.288715 | — | — | — |
| co_spawn_select.test | LINK | 0.104495 | 0.184222 | 0.293491 | — | — | — |
| completion_condition.test | LINK | 0.135341 | 0.366163 | 0.395190 | — | — | — |
| completion_condition_select.test | LINK | 0.136950 | 0.353602 | 0.390965 | — | — | — |
| compose.test | RUN | 0.678410 | 3.685192 | 3.291926 | 0.001666 | 0.001175 | 0.001675 |
| compose_select.test | RUN | 0.688221 | 3.604975 | 3.298674 | 0.001540 | 0.001665 | 0.001509 |
| composed.test | RUN | 0.658074 | 3.628001 | 3.174843 | 0.001386 | 0.001313 | 0.001308 |
| composed_select.test | RUN | 0.664467 | 3.541534 | 3.192348 | 0.001240 | 0.001178 | 0.001289 |
| config.test | RUN | 0.332466 | 2.345155 | 1.369340 | 0.001329 | 0.001354 | 0.001304 |
| config_select.test | RUN | 0.362864 | 2.448027 | 1.582863 | 0.001132 | 0.001318 | 0.001193 |
| connect.test | LINK | 1.110476 | 7.842882 | 6.943527 | — | — | — |
| connect_select.test | LINK | 1.131404 | 7.769062 | 6.924570 | — | — | — |
| connect_pipe.test | RUN | 0.584653 | 2.938923 | 2.584265 | 0.001326 | 0.001353 | 0.001267 |
| connect_pipe_select.test | RUN | 0.611645 | 3.078447 | 2.718308 | 0.001156 | 0.001335 | 0.001335 |
| consign.test | RUN | 0.509017 | 2.869613 | 2.404430 | 0.001432 | 0.001356 | 0.001294 |
| consign_select.test | RUN | 0.516477 | 2.946673 | 2.355011 | 0.001375 | 0.001274 | 0.001274 |
| coroutine.test | LINK | 0.106731 | 0.274489 | 0.334139 | — | — | — |
| coroutine_select.test | LINK | 0.107511 | 0.276863 | 0.327060 | — | — | — |
| deadline_timer.test | RUN | 0.740461 | 3.752924 | 3.386861 | 0.002717 | 0.002451 | 0.003086 |
| deadline_timer_select.test | RUN | 0.748446 | 3.673426 | 3.369888 | 0.002128 | 0.002788 | 0.002573 |
| deferred.test | RUN | 0.119135 | 0.254944 | 0.311947 | 0.001147 | 0.001142 | 0.001144 |
| deferred_select.test | RUN | 0.115674 | 0.255106 | 0.316313 | 0.001057 | 0.001198 | 0.001266 |
| detached.test | LINK | 0.127015 | 0.284024 | 0.351456 | — | — | — |
| detached_select.test | LINK | 0.131646 | 0.279046 | 0.350040 | — | — | — |
| disposition.test | RUN | 0.136926 | 0.702244 | 0.660004 | 0.000925 | 0.001238 | 0.001398 |
| disposition_select.test | RUN | 0.127981 | 0.687160 | 0.656595 | 0.000883 | 0.001284 | 0.001280 |
| error.test | RUN | 0.113871 | 0.667849 | 0.585066 | 0.000938 | 0.001300 | 0.001317 |
| error_select.test | RUN | 0.114150 | 0.664521 | 0.584023 | 0.000890 | 0.001215 | 0.001354 |
| file_base.test | LINK | 0.143039 | 0.539834 | 0.642150 | — | — | — |
| file_base_select.test | LINK | 0.143414 | 0.548723 | 0.578436 | — | — | — |
| fwd_first.test | LINK | 0.333743 | 0.925757 | 1.162949 | — | — | — |
| fwd_last.test | LINK | 0.333302 | 0.795723 | 1.152269 | — | — | — |
| generic_basic_endpoint.test | LINK | 0.120660 | 0.311944 | 0.418158 | — | — | — |
| generic_basic_endpoint_select.test | LINK | 0.121391 | 0.312984 | 0.414918 | — | — | — |
| generic_datagram_protocol.test | LINK | 0.560406 | 2.963696 | 2.462787 | — | — | — |
| generic_datagram_protocol_select.test | LINK | 0.559831 | 3.056469 | 2.410531 | — | — | — |
| generic_raw_protocol.test | LINK | 0.570658 | 2.939781 | 2.462578 | — | — | — |
| generic_raw_protocol_select.test | LINK | 0.562223 | 3.046179 | 2.403215 | — | — | — |
| generic_seq_packet_protocol.test | LINK | 0.482335 | 2.488312 | 2.089792 | — | — | — |
| generic_seq_packet_protocol_select.test | LINK | 0.480245 | 2.551408 | 2.056306 | — | — | — |
| generic_stream_protocol.test | LINK | 0.515687 | 2.702998 | 2.241104 | — | — | — |
| generic_stream_protocol_select.test | LINK | 0.515079 | 2.725883 | 2.209577 | — | — | — |
| high_resolution_timer.test | LINK | 0.211856 | 0.469705 | 0.644241 | — | — | — |
| high_resolution_timer_select.test | LINK | 0.215336 | 0.481567 | 0.652337 | — | — | — |
| immediate.test | LINK | 0.144007 | 0.363570 | 0.394893 | — | — | — |
| immediate_select.test | LINK | 0.142657 | 0.367268 | 0.387656 | — | — | — |
| inline_executor.test | RUN | 0.179378 | 0.576499 | 0.606830 | 0.000928 | 0.000944 | 0.000885 |
| inline_executor_select.test | RUN | 0.178987 | 0.571625 | 0.592316 | 0.000912 | 0.000938 | 0.000912 |
| inline_or_executor.test | RUN | 0.296357 | 1.745106 | 1.216908 | 0.000967 | 0.000995 | 0.000966 |
| inline_or_executor_select.test | RUN | 0.307957 | 1.768967 | 1.311513 | 0.000906 | 0.000972 | 0.000932 |
| io_context.test | RUN | 0.443302 | 2.666541 | 2.136105 | 0.001858 | 0.001919 | 0.002012 |
| io_context_select.test | RUN | 0.450221 | 2.610568 | 2.098654 | 0.001486 | 0.001729 | 0.001876 |
| io_context_strand.test | RUN | 0.423139 | 2.233159 | 1.864400 | 0.001766 | 0.001586 | 0.001698 |
| io_context_strand_select.test | RUN | 0.429114 | 2.214227 | 1.873318 | 0.001563 | 0.001646 | 0.001777 |
| ip_address.test | LINK | 0.152478 | 0.506635 | 0.507130 | — | — | — |
| ip_address_select.test | LINK | 0.147382 | 0.501803 | 0.507345 | — | — | — |
| ip_address_v4.test | LINK | 0.150446 | 0.527226 | 0.909891 | — | — | — |
| ip_address_v4_select.test | LINK | 0.148541 | 0.530561 | 0.908535 | — | — | — |
| ip_address_v6.test | LINK | 0.173641 | 0.691162 | 1.107509 | — | — | — |
| ip_address_v6_select.test | LINK | 0.171577 | 0.692067 | 1.107643 | — | — | — |
| ip_basic_endpoint.test | LINK | 0.128839 | 0.327096 | 0.704500 | — | — | — |
| ip_basic_endpoint_select.test | LINK | 0.126642 | 0.331320 | 0.696226 | — | — | — |
| ip_basic_resolver.test | LINK | 0.213003 | 0.480002 | 1.045799 | — | — | — |
| ip_basic_resolver_select.test | LINK | 0.214847 | 0.651028 | 1.071521 | — | — | — |
| ip_basic_resolver_entry.test | LINK | 0.069397 | 0.240196 | 0.293937 | — | — | — |
| ip_basic_resolver_entry_select.test | LINK | 0.068097 | 0.231590 | 0.293083 | — | — | — |
| ip_basic_resolver_iterator.test | LINK | 0.127844 | 0.471605 | 0.681921 | — | — | — |
| ip_basic_resolver_iterator_select.test | LINK | 0.125657 | 0.467936 | 0.679746 | — | — | — |
| ip_basic_resolver_query.test | LINK | 0.119958 | 0.468890 | 0.664281 | — | — | — |
| ip_basic_resolver_query_select.test | LINK | 0.121686 | 0.468903 | 0.644851 | — | — | — |
| ip_host_name.test | RUN | 0.129473 | 0.625437 | 0.750069 | 0.000914 | 0.000910 | 0.001121 |
| ip_host_name_select.test | RUN | 0.129459 | 0.628918 | 0.748084 | 0.000920 | 0.001219 | 0.001133 |
| ip_icmp.test | RUN | 0.695614 | 6.212763 | 5.013114 | 0.000960 | 0.001307 | 0.001246 |
| ip_icmp_select.test | RUN | 0.703643 | 6.322322 | 4.928415 | 0.000918 | 0.001378 | 0.001185 |
| ip_multicast.test | RUN | 0.396336 | 3.070698 | 2.801758 | 0.001115 | 0.001314 | 0.001217 |
| ip_multicast_select.test | RUN | 0.403822 | 3.024546 | 2.778287 | 0.000987 | 0.001366 | 0.001282 |
| ip_resolver_query_base.test | LINK | 0.080547 | 0.257810 | 0.311383 | — | — | — |
| ip_resolver_query_base_select.test | LINK | 0.080493 | 0.257044 | 0.310793 | — | — | — |
| ip_tcp.test | RUN | 1.050256 | 9.171319 | 7.733177 | 0.001298 | 0.001830 | 0.001777 |
| ip_tcp_select.test | RUN | 1.052810 | 9.579029 | 7.592954 | 0.001200 | 0.001867 | 0.001852 |
| ip_udp.test | RUN | 0.746876 | 6.644970 | 5.272840 | 0.001139 | 0.001422 | 0.001344 |
| ip_udp_select.test | RUN | 0.751378 | 6.742106 | 5.177914 | 0.001014 | 0.001423 | 0.001414 |
| ip_unicast.test | RUN | 0.358081 | 2.832598 | 2.483149 | 0.001075 | 0.001343 | 0.001378 |
| ip_unicast_select.test | RUN | 0.365161 | 2.869599 | 2.429654 | 0.001009 | 0.001485 | 0.001210 |
| ip_v6_only.test | RUN | 0.371126 | 2.964885 | 2.550223 | 0.000998 | 0.001286 | 0.001237 |
| ip_v6_only_select.test | RUN | 0.380231 | 3.002079 | 2.528596 | 0.000936 | 0.001349 | 0.001246 |
| is_read_buffered.test | RUN | 0.244666 | 0.863643 | 1.325940 | 0.000919 | 0.001212 | 0.001252 |
| is_read_buffered_select.test | RUN | 0.249348 | 0.878279 | 1.340013 | 0.000915 | 0.001218 | 0.001035 |
| is_write_buffered.test | RUN | 0.246311 | 0.847399 | 1.283277 | 0.000935 | 0.001247 | 0.001264 |
| is_write_buffered_select.test | RUN | 0.251651 | 0.885821 | 1.309311 | 0.000895 | 0.001204 | 0.001147 |
| local_basic_endpoint.test | LINK | 0.119716 | 0.483495 | 0.667375 | — | — | — |
| local_basic_endpoint_select.test | LINK | 0.120523 | 0.474221 | 0.669602 | — | — | — |
| local_connect_pair.test | LINK | 0.342742 | 2.890358 | 2.363812 | — | — | — |
| local_connect_pair_select.test | LINK | 0.345543 | 2.853316 | 2.326785 | — | — | — |
| local_datagram_protocol.test | LINK | 0.479751 | 3.743513 | 3.326419 | — | — | — |
| local_datagram_protocol_select.test | LINK | 0.488248 | 3.766259 | 3.274848 | — | — | — |
| local_seq_packet_protocol.test | LINK | 0.484685 | 3.676128 | 3.260699 | — | — | — |
| local_seq_packet_protocol_select.test | LINK | 0.484660 | 3.742844 | 3.228798 | — | — | — |
| local_stream_protocol.test | LINK | 0.454180 | 3.483235 | 3.064583 | — | — | — |
| local_stream_protocol_select.test | LINK | 0.464057 | 3.511707 | 3.021305 | — | — | — |
| placeholders.test | LINK | 0.080274 | 0.246641 | 0.322672 | — | — | — |
| placeholders_select.test | LINK | 0.080202 | 0.253251 | 0.320671 | — | — | — |
| posix_basic_descriptor.test | LINK | 0.214370 | 0.720863 | 0.968203 | — | — | — |
| posix_basic_descriptor_select.test | LINK | 0.224147 | 0.777722 | 1.168411 | — | — | — |
| posix_basic_stream_descriptor.test | LINK | 0.216938 | 0.721866 | 0.973679 | — | — | — |
| posix_basic_stream_descriptor_select.test | LINK | 0.220722 | 0.765285 | 1.170313 | — | — | — |
| posix_descriptor_base.test | LINK | 0.081932 | 0.265862 | 0.311878 | — | — | — |
| posix_descriptor_base_select.test | LINK | 0.086879 | 0.265016 | 0.311210 | — | — | — |
| posix_stream_descriptor.test | LINK | 0.460087 | 3.601987 | 3.070203 | — | — | — |
| posix_stream_descriptor_select.test | LINK | 0.477423 | 3.641853 | 3.143359 | — | — | — |
| prepend.test | RUN | 0.415434 | 2.911646 | 2.401176 | 0.001418 | 0.001299 | 0.001243 |
| prepend_select.test | RUN | 0.442562 | 2.942719 | 2.389422 | 0.001286 | 0.001211 | 0.001238 |
| random_access_file.test | LINK | 0.288289 | 0.657007 | 0.777971 | — | — | — |
| random_access_file_select.test | LINK | 0.305071 | 0.691407 | 0.974624 | — | — | — |
| read.test | RUN | 1.692557 | 18.364886 | 13.958228 | 0.001631 | 0.001742 | 0.001683 |
| read_select.test | RUN | 1.713647 | 18.123924 | 14.158326 | 0.001699 | 0.001525 | 0.001831 |
| read_at.test | RUN | 1.678091 | 23.750095 | 18.621868 | 0.001952 | 0.002142 | 0.001984 |
| read_at_select.test | RUN | 1.694905 | 23.654058 | 18.713034 | 0.001894 | 0.002473 | 0.001877 |
| read_until.test | RUN | 0.890801 | 7.091089 | 5.149415 | 0.001572 | 0.001420 | 0.001358 |
| read_until_select.test | RUN | 0.895077 | 7.101243 | 5.235545 | 0.001440 | 0.001340 | 0.001448 |
| readable_pipe.test | LINK | 0.526667 | 2.935193 | 2.405702 | — | — | — |
| readable_pipe_select.test | LINK | 0.563182 | 3.057034 | 2.549842 | — | — | — |
| recycling_allocator.test | RUN | 0.150068 | 0.408038 | 0.472398 | 0.001263 | 0.001397 | 0.001169 |
| recycling_allocator_select.test | RUN | 0.152400 | 0.407792 | 0.465845 | 0.001121 | 0.001088 | 0.001054 |
| redirect_disposition.test | LINK | 0.692297 | 4.026067 | 3.576107 | — | — | — |
| redirect_disposition_select.test | LINK | 0.700807 | 4.034470 | 3.542262 | — | — | — |
| redirect_error.test | LINK | 0.614516 | 3.412376 | 3.163400 | — | — | — |
| redirect_error_select.test | LINK | 0.628795 | 3.376005 | 3.154442 | — | — | — |
| registered_buffer.test | LINK | 0.147389 | 0.333636 | 0.420534 | — | — | — |
| registered_buffer_select.test | LINK | 0.145449 | 0.331148 | 0.418563 | — | — | — |
| signal_set.test | RUN | 0.517600 | 2.884934 | 2.277992 | 0.001258 | 0.001287 | 0.001170 |
| signal_set_select.test | RUN | 0.540588 | 2.994323 | 2.458146 | 0.001217 | 0.001230 | 0.001168 |
| signal_set_base.test | LINK | 0.116088 | 0.258355 | 0.305508 | — | — | — |
| signal_set_base_select.test | LINK | 0.116869 | 0.263285 | 0.301643 | — | — | — |
| socket_base.test | RUN | 0.634205 | 4.158846 | 3.448115 | 0.001409 | 0.001440 | 0.001438 |
| socket_base_select.test | RUN | 0.646257 | 4.075878 | 3.401167 | 0.001064 | 0.001482 | 0.001355 |
| spawn.test | RUN | 1.064982 | 6.930659 | 6.028526 | 0.001543 | 0.001687 | 0.001571 |
| spawn_select.test | RUN | 1.089952 | 6.996329 | 5.934525 | 0.001392 | 0.001580 | 0.001378 |
| static_thread_pool.test | RUN | 0.276489 | 0.637588 | 0.774253 | 0.001054 | 0.001395 | 0.001378 |
| static_thread_pool_select.test | RUN | 0.287215 | 0.692630 | 0.977002 | 0.001190 | 0.001182 | 0.001126 |
| steady_timer.test | LINK | 0.310674 | 0.693998 | 1.090379 | — | — | — |
| steady_timer_select.test | LINK | 0.313898 | 0.732266 | 1.101549 | — | — | — |
| strand.test | RUN | 0.693082 | 3.924998 | 3.522467 | 0.001823 | 0.002043 | 0.001688 |
| strand_select.test | RUN | 0.707093 | 3.895918 | 3.449453 | 0.002026 | 0.002078 | 0.001827 |
| stream_file.test | LINK | 0.287642 | 0.552514 | 0.819510 | — | — | — |
| stream_file_select.test | LINK | 0.299357 | 0.610518 | 0.999255 | — | — | — |
| streambuf.test | RUN | 0.164039 | 0.364434 | 0.549014 | 0.000946 | 0.000956 | 0.001213 |
| streambuf_select.test | RUN | 0.133900 | 0.365621 | 0.473335 | 0.000878 | 0.000976 | 0.001267 |
| system_timer.test | LINK | 0.444773 | 2.499640 | 2.060061 | — | — | — |
| system_timer_select.test | LINK | 0.449610 | 2.466822 | 2.010534 | — | — | — |
| system_context.test | LINK | 0.188199 | 0.418762 | 0.459901 | — | — | — |
| system_context_select.test | LINK | 0.195830 | 0.452301 | 0.590679 | — | — | — |
| system_executor.test | LINK | 0.260982 | 1.417337 | 0.948209 | — | — | — |
| system_executor_select.test | LINK | 0.270287 | 1.389854 | 1.055426 | — | — | — |
| this_coro.test | LINK | 0.070827 | 0.158469 | 0.184108 | — | — | — |
| this_coro_select.test | LINK | 0.069510 | 0.158327 | 0.183594 | — | — | — |
| thread_pool.test | RUN | 0.314668 | 1.816526 | 1.239751 | 0.001624 | 0.001589 | 0.001620 |
| thread_pool_select.test | RUN | 0.321831 | 1.843378 | 1.347840 | 0.001618 | 0.001564 | 0.001603 |
| time_traits.test | LINK | 0.160020 | 0.331612 | 0.377679 | — | — | — |
| time_traits_select.test | LINK | 0.161266 | 0.481993 | 0.375265 | — | — | — |
| ts_buffer.test | LINK | 0.153185 | 0.559285 | 0.398300 | — | — | — |
| ts_buffer_select.test | LINK | 0.154041 | 0.493198 | 0.403442 | — | — | — |
| ts_executor.test | LINK | 0.223866 | 0.774735 | 0.633872 | — | — | — |
| ts_executor_select.test | LINK | 0.230255 | 0.827338 | 0.751122 | — | — | — |
| ts_internet.test | LINK | 0.243572 | 0.851597 | 0.791250 | — | — | — |
| ts_internet_select.test | LINK | 0.245439 | 0.874608 | 1.264028 | — | — | — |
| ts_io_context.test | LINK | 0.193969 | 0.647800 | 0.770952 | — | — | — |
| ts_io_context_select.test | LINK | 0.202679 | 0.692003 | 0.969782 | — | — | — |
| ts_net.test | LINK | 0.288146 | 1.017741 | 1.532893 | — | — | — |
| ts_net_select.test | LINK | 0.295585 | 1.037409 | 1.571496 | — | — | — |
| ts_netfwd.test | LINK | 0.290257 | 1.014203 | 1.543646 | — | — | — |
| ts_netfwd_select.test | LINK | 0.298806 | 1.049679 | 1.574958 | — | — | — |
| ts_socket.test | LINK | 0.236777 | 0.796841 | 1.167100 | — | — | — |
| ts_socket_select.test | LINK | 0.243235 | 0.820578 | 1.209149 | — | — | — |
| ts_timer.test | LINK | 0.209955 | 0.719633 | 1.025034 | — | — | — |
| ts_timer_select.test | LINK | 0.214298 | 0.738415 | 1.058171 | — | — | — |
| use_awaitable.test | LINK | 0.070398 | 0.240905 | 0.294725 | — | — | — |
| use_awaitable_select.test | LINK | 0.070029 | 0.236260 | 0.289521 | — | — | — |
| wait_traits.test | LINK | 0.068833 | 0.237960 | 0.294037 | — | — | — |
| wait_traits_select.test | LINK | 0.067956 | 0.234698 | 0.291353 | — | — | — |
| windows_basic_object_handle.test | LINK | 0.069788 | 0.238224 | 0.288639 | — | — | — |
| windows_basic_object_handle_select.test | LINK | 0.069708 | 0.237265 | 0.297186 | — | — | — |
| windows_basic_overlapped_handle.test | LINK | 0.068331 | 0.236646 | 0.297353 | — | — | — |
| windows_basic_overlapped_handle_select.test | LINK | 0.072860 | 0.240215 | 0.293472 | — | — | — |
| windows_basic_random_access_handle.test | LINK | 0.071176 | 0.238493 | 0.291676 | — | — | — |
| windows_basic_random_access_handle_select.test | LINK | 0.070648 | 0.237258 | 0.296068 | — | — | — |
| windows_basic_stream_handle.test | LINK | 0.069304 | 0.237244 | 0.291052 | — | — | — |
| windows_basic_stream_handle_select.test | LINK | 0.068905 | 0.242813 | 0.293591 | — | — | — |
| windows_object_handle.test | LINK | 0.201304 | 0.654690 | 0.775041 | — | — | — |
| windows_object_handle_select.test | LINK | 0.206427 | 0.694559 | 0.979000 | — | — | — |
| windows_overlapped_ptr.test | LINK | 0.207088 | 0.692586 | 0.859808 | — | — | — |
| windows_overlapped_ptr_select.test | LINK | 0.213065 | 0.740182 | 1.055828 | — | — | — |
| windows_random_access_handle.test | LINK | 0.197155 | 0.658375 | 0.773573 | — | — | — |
| windows_random_access_handle_select.test | LINK | 0.205529 | 0.693154 | 0.972436 | — | — | — |
| windows_stream_handle.test | LINK | 0.199927 | 0.654101 | 0.785783 | — | — | — |
| windows_stream_handle_select.test | LINK | 0.204473 | 0.707759 | 0.965386 | — | — | — |
| writable_pipe.test | LINK | 0.377481 | 2.922595 | 2.486456 | — | — | — |
| writable_pipe_select.test | LINK | 0.388648 | 3.056132 | 2.627890 | — | — | — |
| write.test | RUN | 1.674454 | 15.152527 | 13.812506 | 0.001849 | 0.001618 | 0.001673 |
| write_select.test | RUN | 1.705496 | 15.184269 | 13.740777 | 0.001339 | 0.001787 | 0.001842 |
| write_at.test | RUN | 1.708720 | 20.511972 | 17.351849 | 0.001849 | 0.001872 | 0.001942 |
| write_at_select.test | RUN | 1.728703 | 20.319712 | 17.593840 | 0.001797 | 0.001949 | 0.002060 |
| any_executor.test | RUN | 0.668503 | 6.656811 | 5.657854 | 0.001700 | 0.001926 | 0.001677 |
| blocking.test | RUN | 1.464640 | 8.976122 | 11.364551 | 0.003334 | 0.003141 | 0.002884 |
| blocking_adaptation.test | RUN | 0.725104 | 3.789519 | 5.139887 | 0.001996 | 0.001932 | 0.001972 |
| context_as.test | RUN | 0.421536 | 2.650900 | 1.748145 | 0.001479 | 0.001398 | 0.001320 |
| executor.test | RUN | 0.103097 | 0.243176 | 0.309920 | 0.001188 | 0.001144 | 0.001167 |
| inline_exception_handling.test | RUN | 1.369033 | 7.827302 | 11.337325 | 0.003390 | 0.002227 | 0.002282 |
| invocable_archetype.test | RUN | 0.098955 | 0.238831 | 0.292821 | 0.001198 | 0.001141 | 0.001188 |
| mapping.test | RUN | 1.416827 | 8.525377 | 10.979523 | 0.002338 | 0.002328 | 0.002320 |
| outstanding_work.test | RUN | 0.648482 | 3.462711 | 4.918796 | 0.002114 | 0.001989 | 0.001984 |
| prefer_only.test | RUN | 0.215594 | 1.419638 | 1.082414 | 0.001236 | 0.001297 | 0.001183 |
| relationship.test | RUN | 0.490487 | 3.537554 | 4.828661 | 0.001492 | 0.002212 | 0.001972 |

## Native runtime replay

Child CPU of successful native tests, with process startup included; GDB and expected-failure executions are correctness-only. Two warmups per compiler, four A/A observations per reference and mirrored ABBA/BAAB blocks. Short tests repeat up to ten times, identically across compilers; CPU is divided by repetitions before summing tests. Executable hashes and output checks stay outside process timers. All runs must pass their original native oracle. Output hashes record equality or variation across the complete campaign; test reports may print times or addresses.

| Reference | CPU speedup ↑ | Paired blocks | A/A speedup |
|---|---:|---|---:|
| gcc | 0.995× | 0.993×, 0.996× | 0.993× |
| clang | 0.983× | 0.974×, 0.991× | 1.007× |

† A/A drift or paired-block spread exceeds 5%; small differences are unresolved. 1375 native tests in the full cohort; 1364 have byte-identical output throughout. [Raw replay evidence in CI artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43559814672).

| Test | Repeats | CPU cppgm | GCC | Clang | Output check |
|---|---:|---:|---:|---:|---|
| any_completion_executor.test | 3 | 0.001233s | 0.001202s | 0.001254s | byte equality observed |
| any_completion_executor_select.test | 5 | 0.001192s | 0.001202s | 0.001198s | byte equality observed |
| any_completion_handler.test | 10 | 0.001020s | 0.001041s | 0.001022s | byte equality observed |
| any_completion_handler_select.test | 10 | 0.001009s | 0.001035s | 0.001017s | byte equality observed |
| any_io_executor.test | 10 | 0.001218s | 0.001227s | 0.001234s | byte equality observed |
| any_io_executor_select.test | 4 | 0.001224s | 0.001240s | 0.001239s | byte equality observed |
| append.test | 10 | 0.000993s | 0.000989s | 0.000977s | byte equality observed |
| append_select.test | 8 | 0.000974s | 0.000982s | 0.000953s | byte equality observed |
| as_tuple.test | 10 | 0.001050s | 0.001052s | 0.001028s | byte equality observed |
| as_tuple_select.test | 10 | 0.000987s | 0.000982s | 0.000987s | byte equality observed |
| bind_allocator.test | 1 | 0.001460s | 0.001423s | 0.001479s | byte equality observed |
| bind_allocator_select.test | 1 | 0.001191s | 0.001130s | 0.001186s | byte equality observed |
| bind_cancellation_slot.test | 10 | 0.000893s | 0.000911s | 0.000871s | byte equality observed |
| bind_cancellation_slot_select.test | 10 | 0.000816s | 0.000807s | 0.000809s | byte equality observed |
| bind_executor.test | 1 | 0.001438s | 0.001438s | 0.001437s | byte equality observed |
| bind_executor_select.test | 1 | 0.001740s | 0.001724s | 0.001670s | byte equality observed |
| bind_immediate_executor.test | 10 | 0.000916s | 0.000942s | 0.000900s | byte equality observed |
| bind_immediate_executor_select.test | 10 | 0.000905s | 0.000901s | 0.000871s | byte equality observed |
| buffer.test | 10 | 0.000844s | 0.000867s | 0.000853s | byte equality observed |
| buffer_select.test | 10 | 0.000844s | 0.000815s | 0.000820s | byte equality observed |
| buffered_read_stream.test | 10 | 0.001039s | 0.001034s | 0.001035s | byte equality observed |
| buffered_read_stream_select.test | 8 | 0.001040s | 0.000983s | 0.000985s | byte equality observed |
| buffered_stream.test | 8 | 0.001016s | 0.001008s | 0.001026s | byte equality observed |
| buffered_stream_select.test | 10 | 0.001013s | 0.000999s | 0.001030s | byte equality observed |
| buffered_write_stream.test | 10 | 0.001009s | 0.001018s | 0.001003s | byte equality observed |
| buffered_write_stream_select.test | 10 | 0.000999s | 0.000998s | 0.000999s | byte equality observed |
| buffers_iterator.test | 10 | 0.000830s | 0.000823s | 0.000817s | byte equality observed |
| buffers_iterator_select.test | 10 | 0.000785s | 0.000771s | 0.000772s | byte equality observed |
| cancel_after.test | 1 | 0.002966s | 0.003144s | 0.002956s | byte equality observed |
| cancel_after_select.test | 1 | 0.002510s | 0.002588s | 0.002553s | byte equality observed |
| cancel_at.test | 1 | 0.003219s | 0.003240s | 0.002919s | byte equality observed |
| cancel_at_select.test | 1 | 0.003137s | 0.003182s | 0.002811s | byte equality observed |
| co_composed.test | 10 | 0.000849s | 0.000851s | 0.000837s | byte equality observed |
| co_composed_select.test | 10 | 0.000806s | 0.000779s | 0.000801s | byte equality observed |
| compose.test | 1 | 0.001481s | 0.001459s | 0.001384s | byte equality observed |
| compose_select.test | 1 | 0.001173s | 0.001186s | 0.001151s | byte equality observed |
| composed.test | 1 | 0.001716s | 0.001677s | 0.001602s | byte equality observed |
| composed_select.test | 1 | 0.001350s | 0.001380s | 0.001245s | byte equality observed |
| config.test | 10 | 0.000924s | 0.000912s | 0.000908s | byte equality observed |
| config_select.test | 10 | 0.000888s | 0.000895s | 0.000902s | byte equality observed |
| connect_pipe.test | 10 | 0.000911s | 0.000898s | 0.000908s | byte equality observed |
| connect_pipe_select.test | 10 | 0.000892s | 0.000894s | 0.000875s | byte equality observed |
| consign.test | 10 | 0.000879s | 0.000892s | 0.000869s | byte equality observed |
| consign_select.test | 10 | 0.000830s | 0.000848s | 0.000819s | byte equality observed |
| deadline_timer.test | 1 | 0.002281s | 0.002288s | 0.002234s | byte equality observed |
| deadline_timer_select.test | 1 | 0.001975s | 0.001878s | 0.001924s | byte equality observed |
| deferred.test | 10 | 0.000843s | 0.000854s | 0.000825s | byte equality observed |
| deferred_select.test | 10 | 0.000766s | 0.000763s | 0.000756s | byte equality observed |
| disposition.test | 10 | 0.000799s | 0.000799s | 0.000803s | byte equality observed |
| disposition_select.test | 10 | 0.000792s | 0.000797s | 0.000792s | byte equality observed |
| error.test | 10 | 0.000777s | 0.000781s | 0.000779s | byte equality observed |
| error_select.test | 10 | 0.000766s | 0.000775s | 0.000758s | byte equality observed |
| inline_executor.test | 10 | 0.000785s | 0.000788s | 0.000781s | byte equality observed |
| inline_executor_select.test | 10 | 0.000765s | 0.000783s | 0.000776s | byte equality observed |
| inline_or_executor.test | 10 | 0.000795s | 0.000790s | 0.000768s | byte equality observed |
| inline_or_executor_select.test | 10 | 0.000783s | 0.000788s | 0.000775s | byte equality observed |
| io_context.test | 1 | 0.001785s | 0.001749s | 0.001744s | byte equality observed |
| io_context_select.test | 1 | 0.002097s | 0.001974s | 0.001896s | byte equality observed |
| io_context_strand.test | 1 | 0.002066s | 0.002152s | 0.002215s | byte equality observed |
| io_context_strand_select.test | 1 | 0.001748s | 0.001743s | 0.001791s | byte equality observed |
| ip_host_name.test | 10 | 0.000851s | 0.000853s | 0.000837s | byte equality observed |
| ip_host_name_select.test | 10 | 0.000767s | 0.000779s | 0.000752s | byte equality observed |
| ip_icmp.test | 10 | 0.000798s | 0.000795s | 0.000770s | byte equality observed |
| ip_icmp_select.test | 10 | 0.000793s | 0.000789s | 0.000764s | byte equality observed |
| ip_multicast.test | 10 | 0.000855s | 0.000856s | 0.000831s | byte equality observed |
| ip_multicast_select.test | 10 | 0.000835s | 0.000829s | 0.000817s | byte equality observed |
| ip_tcp.test | 10 | 0.001054s | 0.001044s | 0.001035s | byte equality observed |
| ip_tcp_select.test | 10 | 0.001019s | 0.001016s | 0.000990s | byte equality observed |
| ip_udp.test | 10 | 0.000877s | 0.000866s | 0.000843s | byte equality observed |
| ip_udp_select.test | 10 | 0.000859s | 0.000854s | 0.000846s | byte equality observed |
| ip_unicast.test | 10 | 0.000820s | 0.000829s | 0.000808s | byte equality observed |
| ip_unicast_select.test | 10 | 0.000820s | 0.000817s | 0.000790s | byte equality observed |
| ip_v6_only.test | 10 | 0.000827s | 0.000822s | 0.000814s | byte equality observed |
| ip_v6_only_select.test | 10 | 0.000818s | 0.000816s | 0.000799s | byte equality observed |
| is_read_buffered.test | 10 | 0.000766s | 0.000753s | 0.000755s | byte equality observed |
| is_read_buffered_select.test | 10 | 0.000762s | 0.000750s | 0.000749s | byte equality observed |
| is_write_buffered.test | 10 | 0.000765s | 0.000729s | 0.000752s | byte equality observed |
| is_write_buffered_select.test | 10 | 0.000759s | 0.000766s | 0.000753s | byte equality observed |
| prepend.test | 10 | 0.000816s | 0.000813s | 0.000804s | byte equality observed |
| prepend_select.test | 10 | 0.000799s | 0.000804s | 0.000785s | byte equality observed |
| read.test | 10 | 0.001116s | 0.001144s | 0.001085s | byte equality observed |
| read_at.test | 10 | 0.001329s | 0.001457s | 0.001320s | byte equality observed |
| read_at_select.test | 10 | 0.001324s | 0.001453s | 0.001319s | byte equality observed |
| read_select.test | 10 | 0.001114s | 0.001169s | 0.001071s | byte equality observed |
| read_until.test | 10 | 0.000924s | 0.000952s | 0.000885s | byte equality observed |
| read_until_select.test | 10 | 0.000927s | 0.000964s | 0.000889s | byte equality observed |
| recycling_allocator.test | 10 | 0.000816s | 0.000828s | 0.000759s | byte equality observed |
| recycling_allocator_select.test | 10 | 0.000807s | 0.000803s | 0.000783s | byte equality observed |
| signal_set.test | 10 | 0.000842s | 0.000855s | 0.000778s | byte equality observed |
| signal_set_select.test | 10 | 0.000841s | 0.000841s | 0.000818s | byte equality observed |
| socket_base.test | 10 | 0.000898s | 0.000905s | 0.000912s | byte equality observed |
| socket_base_select.test | 10 | 0.000914s | 0.000873s | 0.000892s | byte equality observed |
| spawn.test | 10 | 0.001018s | 0.000980s | 0.000988s | byte equality observed |
| spawn_select.test | 10 | 0.000987s | 0.000974s | 0.000977s | byte equality observed |
| static_thread_pool.test | 10 | 0.000841s | 0.000813s | 0.000812s | byte equality observed |
| static_thread_pool_select.test | 10 | 0.000783s | 0.000758s | 0.000762s | byte equality observed |
| strand.test | 1 | 0.001762s | 0.001733s | 0.001721s | byte equality observed |
| strand_select.test | 1 | 0.001415s | 0.001459s | 0.001416s | byte equality observed |
| streambuf.test | 10 | 0.000894s | 0.000879s | 0.000880s | byte equality observed |
| streambuf_select.test | 10 | 0.000840s | 0.000857s | 0.000846s | byte equality observed |
| thread_pool.test | 10 | 0.001543s | 0.001535s | 0.001569s | byte equality observed |
| thread_pool_select.test | 10 | 0.001554s | 0.001539s | 0.001563s | byte equality observed |
| write.test | 10 | 0.001208s | 0.001193s | 0.001201s | byte equality observed |
| write_at.test | 10 | 0.001321s | 0.001316s | 0.001296s | byte equality observed |
| write_at_select.test | 10 | 0.001314s | 0.001305s | 0.001290s | byte equality observed |
| write_select.test | 10 | 0.001191s | 0.001174s | 0.001154s | byte equality observed |

[Raw outcomes, commands and timing observations in CI artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43559814672).

[Comparison records](../results.json) · [Host and toolchains](../metadata.json)
