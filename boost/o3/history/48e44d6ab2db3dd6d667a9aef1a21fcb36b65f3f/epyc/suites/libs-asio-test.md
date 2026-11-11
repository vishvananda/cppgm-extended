# libs/asio/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| any_completion_executor.test | RUN | 0.864654 | 5.150298 | 5.244086 | 0.002997 | 0.002938 | 0.003419 |
| any_completion_executor_select.test | RUN | 0.896859 | 5.104329 | 5.505789 | 0.002807 | 0.003082 | 0.002395 |
| any_completion_handler.test | RUN | 1.033984 | 4.580439 | 4.891594 | 0.002120 | 0.002406 | 0.002131 |
| any_completion_handler_select.test | RUN | 1.005109 | 4.734003 | 5.260791 | 0.002047 | 0.002544 | 0.002379 |
| any_io_executor.test | RUN | 0.921566 | 5.431200 | 5.531759 | 0.002753 | 0.003082 | 0.002970 |
| any_io_executor_select.test | RUN | 0.943504 | 5.434392 | 6.056365 | 0.002654 | 0.003128 | 0.002835 |
| append.test | RUN | 0.847077 | 4.276559 | 4.214255 | 0.001936 | 0.002480 | 0.002200 |
| append_select.test | RUN | 0.870260 | 4.379488 | 4.270657 | 0.002198 | 0.002366 | 0.002155 |
| as_tuple.test | RUN | 1.117696 | 5.216411 | 6.086655 | 0.002269 | 0.002364 | 0.002598 |
| as_tuple_select.test | RUN | 1.119164 | 5.094644 | 5.948097 | 0.002216 | 0.002087 | 0.002181 |
| associated_allocator.test | LINK | 0.243035 | 0.517152 | 0.691506 | — | — | — |
| associated_allocator_select.test | LINK | 0.263322 | 0.465240 | 0.711620 | — | — | — |
| associated_cancellation_slot.test | LINK | 0.274810 | 0.502049 | 0.758906 | — | — | — |
| associated_cancellation_slot_select.test | LINK | 0.268590 | 0.503653 | 0.759011 | — | — | — |
| associated_executor.test | LINK | 0.388125 | 0.872084 | 1.208879 | — | — | — |
| associated_executor_select.test | LINK | 0.392131 | 0.877200 | 1.248424 | — | — | — |
| associated_immediate_executor.test | LINK | 0.357272 | 0.801084 | 1.133114 | — | — | — |
| associated_immediate_executor_select.test | LINK | 0.362997 | 0.818703 | 1.130367 | — | — | — |
| associator.test | LINK | 0.196120 | 0.398749 | 0.595667 | — | — | — |
| associator_select.test | LINK | 0.204422 | 0.393869 | 0.611371 | — | — | — |
| awaitable.test | LINK | 0.200919 | 0.395778 | 0.586222 | — | — | — |
| awaitable_select.test | LINK | 0.207751 | 0.390483 | 0.589888 | — | — | — |
| basic_datagram_socket.test | LINK | 0.596099 | 1.226758 | 2.329638 | — | — | — |
| basic_datagram_socket_select.test | LINK | 0.607784 | 1.244323 | 2.386415 | — | — | — |
| basic_deadline_timer.test | LINK | 0.723280 | 1.485953 | 2.614473 | — | — | — |
| basic_deadline_timer_select.test | LINK | 0.732816 | 1.513980 | 2.726622 | — | — | — |
| basic_file.test | LINK | 0.204415 | 0.405177 | 0.577746 | — | — | — |
| basic_file_select.test | LINK | 0.206158 | 0.403467 | 0.583639 | — | — | — |
| basic_random_access_file.test | LINK | 0.199342 | 0.400423 | 0.548593 | — | — | — |
| basic_random_access_file_select.test | LINK | 0.200292 | 0.407610 | 0.599083 | — | — | — |
| basic_raw_socket.test | LINK | 0.597809 | 1.244958 | 2.338715 | — | — | — |
| basic_raw_socket_select.test | LINK | 0.614331 | 1.291637 | 2.403599 | — | — | — |
| basic_readable_pipe.test | LINK | 0.587515 | 1.187155 | 1.995313 | — | — | — |
| basic_readable_pipe_select.test | LINK | 0.609242 | 1.259044 | 2.465270 | — | — | — |
| basic_seq_packet_socket.test | LINK | 0.590781 | 1.238006 | 2.346459 | — | — | — |
| basic_seq_packet_socket_select.test | LINK | 0.609524 | 1.283631 | 2.327895 | — | — | — |
| basic_signal_set.test | LINK | 0.551658 | 1.138519 | 1.747826 | — | — | — |
| basic_signal_set_select.test | LINK | 0.585510 | 1.223631 | 2.218949 | — | — | — |
| basic_socket_acceptor.test | LINK | 0.606758 | 1.246400 | 2.279744 | — | — | — |
| basic_socket_acceptor_select.test | LINK | 0.614349 | 1.293386 | 2.329426 | — | — | — |
| basic_stream_file.test | LINK | 0.200348 | 0.388300 | 0.623461 | — | — | — |
| basic_stream_file_select.test | LINK | 0.201229 | 0.392995 | 0.602364 | — | — | — |
| basic_stream_socket.test | LINK | 0.591045 | 1.230887 | 2.360241 | — | — | — |
| basic_stream_socket_select.test | LINK | 0.618094 | 1.299756 | 2.322399 | — | — | — |
| basic_streambuf.test | LINK | 0.298647 | 0.562433 | 0.915160 | — | — | — |
| basic_streambuf_select.test | LINK | 0.306255 | 0.573511 | 0.926334 | — | — | — |
| basic_waitable_timer.test | LINK | 0.568563 | 1.146592 | 2.185400 | — | — | — |
| basic_waitable_timer_select.test | LINK | 0.576265 | 1.181020 | 2.243704 | — | — | — |
| basic_writable_pipe.test | LINK | 0.573886 | 1.172408 | 2.000639 | — | — | — |
| basic_writable_pipe_select.test | LINK | 0.609377 | 1.257438 | 2.403422 | — | — | — |
| bind_allocator.test | RUN | 0.913918 | 4.370367 | 4.815478 | 0.002572 | 0.002562 | 0.002547 |
| bind_allocator_select.test | RUN | 0.922491 | 4.436583 | 4.758107 | 0.001926 | 0.002282 | 0.002382 |
| bind_cancellation_slot.test | RUN | 0.940989 | 4.404442 | 4.826094 | 0.001474 | 0.002392 | 0.001642 |
| bind_cancellation_slot_select.test | RUN | 0.951054 | 4.384095 | 4.556056 | 0.001778 | 0.001645 | 0.001595 |
| bind_executor.test | RUN | 0.586523 | 2.942423 | 2.754663 | 0.002220 | 0.003082 | 0.003046 |
| bind_executor_select.test | RUN | 0.581513 | 3.006071 | 3.051753 | 0.002268 | 0.002223 | 0.002812 |
| bind_immediate_executor.test | RUN | 0.442858 | 2.580458 | 2.032308 | 0.001590 | 0.001650 | 0.001517 |
| bind_immediate_executor_select.test | RUN | 0.460072 | 2.511519 | 2.023610 | 0.001563 | 0.001616 | 0.001289 |
| buffer.test | RUN | 0.270265 | 1.025201 | 1.108508 | 0.001520 | 0.001633 | 0.001441 |
| buffer_select.test | RUN | 0.268140 | 1.026488 | 1.134346 | 0.001571 | 0.001512 | 0.001374 |
| buffer_registration.test | LINK | 0.233936 | 0.527427 | 0.645101 | — | — | — |
| buffer_registration_select.test | LINK | 0.237084 | 0.530518 | 0.646782 | — | — | — |
| buffered_read_stream.test | RUN | 0.945005 | 7.066498 | 7.543362 | 0.001922 | 0.002818 | 0.002794 |
| buffered_read_stream_select.test | RUN | 0.945036 | 7.061359 | 6.939959 | 0.001878 | 0.002568 | 0.002699 |
| buffered_stream.test | RUN | 1.807399 | 10.309480 | 11.356342 | 0.002533 | 0.002810 | 0.002796 |
| buffered_stream_select.test | RUN | 1.870161 | 10.462490 | 10.961720 | 0.002246 | 0.002737 | 0.002733 |
| buffered_write_stream.test | RUN | 1.697717 | 9.009791 | 9.816693 | 0.002650 | 0.002848 | 0.002811 |
| buffered_write_stream_select.test | RUN | 1.688498 | 9.015609 | 9.354223 | 0.002592 | 0.002677 | 0.002672 |
| buffers_iterator.test | RUN | 0.422489 | 0.911222 | 1.707829 | 0.002174 | 0.002090 | 0.002042 |
| buffers_iterator_select.test | RUN | 0.415320 | 0.913756 | 1.666270 | 0.002204 | 0.002236 | 0.002079 |
| cancel_after.test | RUN | 1.483601 | 7.003153 | 7.152502 | 0.003954 | 0.004495 | 0.004444 |
| cancel_after_select.test | RUN | 1.485036 | 7.017620 | 7.189691 | 0.003962 | 0.004229 | 0.004353 |
| cancel_at.test | RUN | 1.368936 | 5.695226 | 6.157571 | 0.003560 | 0.003847 | 0.003952 |
| cancel_at_select.test | RUN | 1.362319 | 5.584907 | 6.177881 | 0.003570 | 0.003688 | 0.003569 |
| cancellation_signal.test | LINK | 0.167668 | 0.307364 | 0.432993 | — | — | — |
| cancellation_signal_select.test | LINK | 0.170882 | 0.318048 | 0.432025 | — | — | — |
| cancellation_state.test | LINK | 0.253803 | 0.316071 | 0.434176 | — | — | — |
| cancellation_state_select.test | LINK | 0.263862 | 0.314286 | 0.439426 | — | — | — |
| cancellation_type.test | LINK | 0.200972 | 0.245504 | 0.345854 | — | — | — |
| cancellation_type_select.test | LINK | 0.203192 | 0.247615 | 0.376959 | — | — | — |
| co_composed.test | RUN | 0.202857 | 0.248121 | 0.522342 | 0.001816 | 0.001426 | 0.001514 |
| co_composed_select.test | RUN | 0.201380 | 0.246991 | 0.564609 | 0.001809 | 0.001549 | 0.002153 |
| co_spawn.test | LINK | 0.207138 | 0.247258 | 0.566359 | — | — | — |
| co_spawn_select.test | LINK | 0.204596 | 0.248740 | 0.582635 | — | — | — |
| completion_condition.test | LINK | 0.263210 | 0.379691 | 0.800206 | — | — | — |
| completion_condition_select.test | LINK | 0.264786 | 0.560335 | 0.794643 | — | — | — |
| compose.test | RUN | 1.125399 | 5.380106 | 6.095159 | 0.002552 | 0.002503 | 0.002358 |
| compose_select.test | RUN | 1.128792 | 5.325642 | 6.145692 | 0.001840 | 0.002078 | 0.002840 |
| composed.test | RUN | 1.063876 | 5.508579 | 5.858792 | 0.002273 | 0.002186 | 0.002394 |
| composed_select.test | RUN | 1.043941 | 5.248196 | 5.821277 | 0.002197 | 0.002139 | 0.002211 |
| config.test | RUN | 0.512997 | 3.607448 | 2.528821 | 0.002112 | 0.002392 | 0.002323 |
| config_select.test | RUN | 0.528096 | 3.585071 | 3.169606 | 0.002028 | 0.002064 | 0.002251 |
| connect.test | LINK | 1.880523 | 11.406913 | 12.312419 | — | — | — |
| connect_select.test | LINK | 1.858571 | 11.310249 | 12.456615 | — | — | — |
| connect_pipe.test | RUN | 0.993583 | 4.312026 | 4.628862 | 0.002048 | 0.002217 | 0.002350 |
| connect_pipe_select.test | RUN | 1.004530 | 4.497478 | 5.062173 | 0.002004 | 0.002276 | 0.002293 |
| consign.test | RUN | 0.854896 | 4.273019 | 4.164866 | 0.002176 | 0.002418 | 0.002149 |
| consign_select.test | RUN | 0.839498 | 4.366730 | 4.114881 | 0.001428 | 0.002397 | 0.001960 |
| coroutine.test | LINK | 0.209099 | 0.447677 | 0.633718 | — | — | — |
| coroutine_select.test | LINK | 0.209095 | 0.455050 | 0.664009 | — | — | — |
| deadline_timer.test | RUN | 1.237010 | 5.549045 | 6.034512 | 0.003341 | 0.003524 | 0.003997 |
| deadline_timer_select.test | RUN | 1.224570 | 5.502588 | 6.072382 | 0.003382 | 0.003667 | 0.003220 |
| deferred.test | RUN | 0.222312 | 0.443729 | 0.679924 | 0.001927 | 0.002324 | 0.002069 |
| deferred_select.test | RUN | 0.219200 | 0.434774 | 0.673209 | 0.002138 | 0.002276 | 0.002346 |
| detached.test | LINK | 0.250448 | 0.485015 | 0.748003 | — | — | — |
| detached_select.test | LINK | 0.250555 | 0.490370 | 0.732778 | — | — | — |
| disposition.test | RUN | 0.245811 | 1.116588 | 1.289832 | 0.001411 | 0.002168 | 0.002301 |
| disposition_select.test | RUN | 0.246164 | 1.101075 | 1.281026 | 0.001599 | 0.002246 | 0.002240 |
| error.test | RUN | 0.200810 | 1.063155 | 1.145076 | 0.001565 | 0.002064 | 0.002179 |
| error_select.test | RUN | 0.201967 | 1.053739 | 1.104711 | 0.001554 | 0.002152 | 0.002233 |
| file_base.test | LINK | 0.246058 | 0.885099 | 1.127516 | — | — | — |
| file_base_select.test | LINK | 0.245994 | 0.883756 | 0.933441 | — | — | — |
| fwd_first.test | LINK | 0.566897 | 1.204350 | 2.087945 | — | — | — |
| fwd_last.test | LINK | 0.566782 | 1.198989 | 2.034611 | — | — | — |
| generic_basic_endpoint.test | LINK | 0.214865 | 0.489732 | 0.770892 | — | — | — |
| generic_basic_endpoint_select.test | LINK | 0.215356 | 0.487574 | 0.765522 | — | — | — |
| generic_datagram_protocol.test | LINK | 0.825028 | 4.082901 | 3.815215 | — | — | — |
| generic_datagram_protocol_select.test | LINK | 0.816750 | 4.251336 | 3.819947 | — | — | — |
| generic_raw_protocol.test | LINK | 0.824086 | 4.097751 | 3.874963 | — | — | — |
| generic_raw_protocol_select.test | LINK | 0.815254 | 4.255663 | 3.864155 | — | — | — |
| generic_seq_packet_protocol.test | LINK | 0.712048 | 3.490395 | 3.351945 | — | — | — |
| generic_seq_packet_protocol_select.test | LINK | 0.711239 | 3.551316 | 3.315179 | — | — | — |
| generic_stream_protocol.test | LINK | 0.757721 | 3.782504 | 3.585025 | — | — | — |
| generic_stream_protocol_select.test | LINK | 0.749985 | 3.876225 | 3.590865 | — | — | — |
| high_resolution_timer.test | LINK | 0.352266 | 0.743857 | 1.191650 | — | — | — |
| high_resolution_timer_select.test | LINK | 0.357027 | 0.756359 | 1.218731 | — | — | — |
| immediate.test | LINK | 0.247378 | 0.564727 | 0.739846 | — | — | — |
| immediate_select.test | LINK | 0.245088 | 0.566088 | 0.758252 | — | — | — |
| inline_executor.test | RUN | 0.302562 | 0.861203 | 1.072683 | 0.001323 | 0.001641 | 0.001520 |
| inline_executor_select.test | RUN | 0.297791 | 0.865860 | 1.081034 | 0.001578 | 0.001692 | 0.001528 |
| inline_or_executor.test | RUN | 0.471161 | 2.510614 | 1.975917 | 0.001530 | 0.001720 | 0.001596 |
| inline_or_executor_select.test | RUN | 0.488877 | 2.543993 | 2.222065 | 0.001474 | 0.001710 | 0.001504 |
| io_context.test | RUN | 0.678435 | 3.778464 | 3.464382 | 0.002665 | 0.002722 | 0.002771 |
| io_context_select.test | RUN | 0.684087 | 3.716661 | 3.451051 | 0.002265 | 0.002691 | 0.002740 |
| io_context_strand.test | RUN | 0.636844 | 4.357845 | 3.951814 | 0.002528 | 0.002974 | 0.003190 |
| io_context_strand_select.test | RUN | 0.638251 | 4.150269 | 3.847538 | 0.002356 | 0.002838 | 0.003039 |
| ip_address.test | LINK | 0.246814 | 1.219479 | 1.741653 | — | — | — |
| ip_address_select.test | LINK | 0.254345 | 1.206705 | 1.661708 | — | — | — |
| ip_address_v4.test | LINK | 0.256470 | 1.292578 | 1.244491 | — | — | — |
| ip_address_v4_select.test | LINK | 0.260816 | 1.254456 | 1.045831 | — | — | — |
| ip_address_v6.test | LINK | 0.293564 | 1.239126 | 1.247884 | — | — | — |
| ip_address_v6_select.test | LINK | 0.297318 | 1.052622 | 1.210369 | — | — | — |
| ip_basic_endpoint.test | LINK | 0.226751 | 0.525179 | 0.784891 | — | — | — |
| ip_basic_endpoint_select.test | LINK | 0.229444 | 0.525678 | 0.805001 | — | — | — |
| ip_basic_resolver.test | LINK | 0.360438 | 0.750041 | 1.159107 | — | — | — |
| ip_basic_resolver_select.test | LINK | 0.362603 | 0.774491 | 1.240747 | — | — | — |
| ip_basic_resolver_entry.test | LINK | 0.126696 | 0.249343 | 0.342296 | — | — | — |
| ip_basic_resolver_entry_select.test | LINK | 0.127881 | 0.248975 | 0.346437 | — | — | — |
| ip_basic_resolver_iterator.test | LINK | 0.222899 | 0.510520 | 0.791658 | — | — | — |
| ip_basic_resolver_iterator_select.test | LINK | 0.224853 | 0.515562 | 0.801900 | — | — | — |
| ip_basic_resolver_query.test | LINK | 0.214668 | 0.488290 | 0.769133 | — | — | — |
| ip_basic_resolver_query_select.test | LINK | 0.215155 | 0.490346 | 0.739584 | — | — | — |
| ip_host_name.test | RUN | 0.223156 | 0.645939 | 0.841516 | 0.001573 | 0.001583 | 0.001478 |
| ip_host_name_select.test | RUN | 0.230506 | 0.647026 | 0.916527 | 0.001456 | 0.001673 | 0.002132 |
| ip_icmp.test | RUN | 1.020245 | 7.481862 | 8.582067 | 0.001678 | 0.002113 | 0.001856 |
| ip_icmp_select.test | RUN | 1.019267 | 7.908502 | 8.415135 | 0.001476 | 0.002198 | 0.001873 |
| ip_multicast.test | RUN | 0.605881 | 4.548295 | 5.110506 | 0.001780 | 0.002064 | 0.002474 |
| ip_multicast_select.test | RUN | 0.613502 | 4.537063 | 5.120688 | 0.001570 | 0.002018 | 0.002272 |
| ip_resolver_query_base.test | LINK | 0.145814 | 0.436928 | 0.602681 | — | — | — |
| ip_resolver_query_base_select.test | LINK | 0.147837 | 0.420595 | 0.618538 | — | — | — |
| ip_tcp.test | RUN | 1.528072 | 13.125879 | 12.625266 | 0.002366 | 0.003447 | 0.002849 |
| ip_tcp_select.test | RUN | 1.525642 | 13.852245 | 12.335189 | 0.002120 | 0.003174 | 0.002749 |
| ip_udp.test | RUN | 1.136815 | 9.469747 | 9.007267 | 0.002284 | 0.002652 | 0.002262 |
| ip_udp_select.test | RUN | 1.158453 | 9.582538 | 8.809992 | 0.002242 | 0.002674 | 0.002166 |
| ip_unicast.test | RUN | 0.888793 | 4.142339 | 4.513848 | 0.002176 | 0.002076 | 0.002088 |
| ip_unicast_select.test | RUN | 0.894794 | 4.177879 | 4.397088 | 0.002094 | 0.002466 | 0.001965 |
| ip_v6_only.test | RUN | 0.910860 | 4.352783 | 4.628288 | 0.002301 | 0.002263 | 0.002030 |
| ip_v6_only_select.test | RUN | 0.929715 | 4.417304 | 4.551826 | 0.002247 | 0.002162 | 0.001840 |
| is_read_buffered.test | RUN | 0.660603 | 1.478949 | 2.699477 | 0.002056 | 0.002091 | 0.001922 |
| is_read_buffered_select.test | RUN | 0.681888 | 1.478706 | 2.751449 | 0.001955 | 0.001968 | 0.001976 |
| is_write_buffered.test | RUN | 0.689109 | 1.384555 | 2.675233 | 0.002082 | 0.002114 | 0.001844 |
| is_write_buffered_select.test | RUN | 0.688762 | 1.442022 | 2.661787 | 0.001919 | 0.002216 | 0.001771 |
| local_basic_endpoint.test | LINK | 0.364629 | 0.749518 | 1.450459 | — | — | — |
| local_basic_endpoint_select.test | LINK | 0.350766 | 0.784851 | 1.431676 | — | — | — |
| local_connect_pair.test | LINK | 0.836129 | 4.291360 | 4.284817 | — | — | — |
| local_connect_pair_select.test | LINK | 0.843548 | 4.253395 | 4.249221 | — | — | — |
| local_datagram_protocol.test | LINK | 1.121395 | 5.565498 | 5.702284 | — | — | — |
| local_datagram_protocol_select.test | LINK | 1.138414 | 5.563921 | 5.651965 | — | — | — |
| local_seq_packet_protocol.test | LINK | 1.115747 | 5.447458 | 5.596836 | — | — | — |
| local_seq_packet_protocol_select.test | LINK | 1.134611 | 5.516487 | 5.622679 | — | — | — |
| local_stream_protocol.test | LINK | 1.063812 | 5.162803 | 5.520470 | — | — | — |
| local_stream_protocol_select.test | LINK | 1.094689 | 5.255828 | 5.460530 | — | — | — |
| placeholders.test | LINK | 0.222643 | 0.428723 | 0.652827 | — | — | — |
| placeholders_select.test | LINK | 0.228247 | 0.427250 | 0.655426 | — | — | — |
| posix_basic_descriptor.test | LINK | 0.577656 | 1.169343 | 1.987439 | — | — | — |
| posix_basic_descriptor_select.test | LINK | 0.604863 | 1.241331 | 2.456196 | — | — | — |
| posix_basic_stream_descriptor.test | LINK | 0.572940 | 1.206292 | 1.965059 | — | — | — |
| posix_basic_stream_descriptor_select.test | LINK | 0.610657 | 1.244219 | 2.453980 | — | — | — |
| posix_descriptor_base.test | LINK | 0.245100 | 0.443243 | 0.607288 | — | — | — |
| posix_descriptor_base_select.test | LINK | 0.241905 | 0.468910 | 0.620207 | — | — | — |
| posix_stream_descriptor.test | LINK | 1.086289 | 5.241567 | 5.277126 | — | — | — |
| posix_stream_descriptor_select.test | LINK | 1.098102 | 5.287133 | 5.536304 | — | — | — |
| prepend.test | RUN | 0.873538 | 4.268394 | 4.435368 | 0.002030 | 0.001934 | 0.002076 |
| prepend_select.test | RUN | 0.879613 | 4.429766 | 4.361317 | 0.001887 | 0.001828 | 0.002070 |
| random_access_file.test | LINK | 0.518424 | 1.056386 | 1.511344 | — | — | — |
| random_access_file_select.test | LINK | 0.547454 | 1.116958 | 2.119895 | — | — | — |
| read.test | RUN | 2.644638 | 26.539857 | 23.846773 | 0.002986 | 0.002968 | 0.003003 |
| read_select.test | RUN | 2.657869 | 26.551078 | 23.805764 | 0.002929 | 0.003096 | 0.003036 |
| read_at.test | RUN | 2.646775 | 35.740832 | 32.310586 | 0.003788 | 0.003553 | 0.003434 |
| read_at_select.test | RUN | 2.670150 | 35.602462 | 33.109662 | 0.003656 | 0.003790 | 0.003597 |
| read_until.test | RUN | 1.444808 | 10.092930 | 8.640233 | 0.002348 | 0.002355 | 0.002348 |
| read_until_select.test | RUN | 1.463609 | 10.066102 | 8.980948 | 0.002448 | 0.002790 | 0.002182 |
| readable_pipe.test | LINK | 0.881439 | 4.308737 | 4.141448 | — | — | — |
| readable_pipe_select.test | LINK | 0.902198 | 4.574965 | 4.647908 | — | — | — |
| recycling_allocator.test | RUN | 0.292939 | 0.680989 | 0.942962 | 0.002022 | 0.002341 | 0.002128 |
| recycling_allocator_select.test | RUN | 0.294829 | 0.683319 | 0.970955 | 0.002097 | 0.002314 | 0.002156 |
| redirect_disposition.test | LINK | 1.220660 | 6.126359 | 6.619357 | — | — | — |
| redirect_disposition_select.test | LINK | 1.214477 | 6.043915 | 6.636360 | — | — | — |
| redirect_error.test | LINK | 1.067774 | 4.949108 | 5.774377 | — | — | — |
| redirect_error_select.test | LINK | 1.060848 | 5.014505 | 5.741559 | — | — | — |
| registered_buffer.test | LINK | 0.293911 | 0.575819 | 0.850231 | — | — | — |
| registered_buffer_select.test | LINK | 0.289508 | 0.563983 | 0.835838 | — | — | — |
| signal_set.test | RUN | 0.837308 | 4.228344 | 4.004624 | 0.001979 | 0.002326 | 0.002189 |
| signal_set_select.test | RUN | 0.860720 | 4.380143 | 4.528219 | 0.002144 | 0.002362 | 0.002156 |
| signal_set_base.test | LINK | 0.243207 | 0.463787 | 0.596997 | — | — | — |
| signal_set_base_select.test | LINK | 0.237873 | 0.467707 | 0.601472 | — | — | — |
| socket_base.test | RUN | 1.080021 | 6.356522 | 6.324242 | 0.002451 | 0.002126 | 0.002487 |
| socket_base_select.test | RUN | 1.076353 | 6.248464 | 6.220629 | 0.002119 | 0.002158 | 0.002362 |
| spawn.test | RUN | 1.708292 | 10.200692 | 9.908985 | 0.002484 | 0.002586 | 0.002492 |
| spawn_select.test | RUN | 1.710896 | 10.033566 | 9.938765 | 0.002392 | 0.002608 | 0.002568 |
| static_thread_pool.test | RUN | 0.506997 | 1.020582 | 1.502253 | 0.002074 | 0.002123 | 0.002174 |
| static_thread_pool_select.test | RUN | 0.528292 | 1.129887 | 1.987863 | 0.002002 | 0.002192 | 0.002066 |
| steady_timer.test | LINK | 0.563434 | 1.155791 | 2.172056 | — | — | — |
| steady_timer_select.test | LINK | 0.579887 | 1.185800 | 2.195072 | — | — | — |
| strand.test | RUN | 1.131889 | 5.732359 | 6.325729 | 0.003080 | 0.002932 | 0.003256 |
| strand_select.test | RUN | 1.144712 | 5.665993 | 6.101499 | 0.002451 | 0.002543 | 0.002458 |
| stream_file.test | LINK | 0.520542 | 1.077367 | 1.510829 | — | — | — |
| stream_file_select.test | LINK | 0.551647 | 1.135606 | 2.105543 | — | — | — |
| streambuf.test | RUN | 0.252485 | 0.852697 | 1.132804 | 0.001530 | 0.002190 | 0.002222 |
| streambuf_select.test | RUN | 0.237981 | 0.801913 | 1.115120 | 0.001416 | 0.001551 | 0.002150 |
| system_timer.test | LINK | 0.685002 | 3.585361 | 4.285600 | — | — | — |
| system_timer_select.test | LINK | 0.681106 | 3.456357 | 3.678333 | — | — | — |
| system_context.test | LINK | 0.323845 | 0.649301 | 0.844320 | — | — | — |
| system_context_select.test | LINK | 0.333977 | 0.700107 | 1.062928 | — | — | — |
| system_executor.test | LINK | 0.415833 | 2.030817 | 1.591964 | — | — | — |
| system_executor_select.test | LINK | 0.417872 | 1.993655 | 1.793465 | — | — | — |
| this_coro.test | LINK | 0.134107 | 0.252755 | 0.330348 | — | — | — |
| this_coro_select.test | LINK | 0.132078 | 0.249810 | 0.342554 | — | — | — |
| thread_pool.test | RUN | 0.476975 | 3.438642 | 2.425224 | 0.003282 | 0.003486 | 0.003219 |
| thread_pool_select.test | RUN | 0.488979 | 3.690662 | 2.627653 | 0.003169 | 0.004162 | 0.004337 |
| time_traits.test | LINK | 0.289950 | 0.860814 | 1.321423 | — | — | — |
| time_traits_select.test | LINK | 0.295170 | 0.849913 | 1.237651 | — | — | — |
| ts_buffer.test | LINK | 0.271441 | 0.911493 | 1.313171 | — | — | — |
| ts_buffer_select.test | LINK | 0.273305 | 0.902053 | 1.273197 | — | — | — |
| ts_executor.test | LINK | 0.368404 | 1.231922 | 2.018789 | — | — | — |
| ts_executor_select.test | LINK | 0.394729 | 1.332575 | 2.493805 | — | — | — |
| ts_internet.test | LINK | 0.406717 | 1.405371 | 2.622508 | — | — | — |
| ts_internet_select.test | LINK | 0.423350 | 1.402083 | 2.687425 | — | — | — |
| ts_io_context.test | LINK | 0.327991 | 1.068921 | 1.532392 | — | — | — |
| ts_io_context_select.test | LINK | 0.350777 | 1.114018 | 2.084261 | — | — | — |
| ts_net.test | LINK | 0.491030 | 1.681449 | 3.258685 | — | — | — |
| ts_net_select.test | LINK | 0.500927 | 1.668571 | 3.241178 | — | — | — |
| ts_netfwd.test | LINK | 0.488204 | 1.644785 | 3.224438 | — | — | — |
| ts_netfwd_select.test | LINK | 0.507409 | 1.720494 | 3.204297 | — | — | — |
| ts_socket.test | LINK | 0.401557 | 1.320301 | 2.549825 | — | — | — |
| ts_socket_select.test | LINK | 0.410976 | 1.317782 | 2.510771 | — | — | — |
| ts_timer.test | LINK | 0.384875 | 1.157227 | 2.198360 | — | — | — |
| ts_timer_select.test | LINK | 0.493645 | 1.183897 | 2.192373 | — | — | — |
| use_awaitable.test | LINK | 0.171349 | 0.415837 | 0.595879 | — | — | — |
| use_awaitable_select.test | LINK | 0.199240 | 0.395927 | 0.601406 | — | — | — |
| wait_traits.test | LINK | 0.199684 | 0.412495 | 0.598611 | — | — | — |
| wait_traits_select.test | LINK | 0.203563 | 0.389240 | 0.579154 | — | — | — |
| windows_basic_object_handle.test | LINK | 0.200937 | 0.395419 | 0.599790 | — | — | — |
| windows_basic_object_handle_select.test | LINK | 0.200185 | 0.398518 | 0.605484 | — | — | — |
| windows_basic_overlapped_handle.test | LINK | 0.204156 | 0.395130 | 0.568265 | — | — | — |
| windows_basic_overlapped_handle_select.test | LINK | 0.205487 | 0.405032 | 0.588115 | — | — | — |
| windows_basic_random_access_handle.test | LINK | 0.202258 | 0.379379 | 0.602846 | — | — | — |
| windows_basic_random_access_handle_select.test | LINK | 0.204134 | 0.398779 | 0.610094 | — | — | — |
| windows_basic_stream_handle.test | LINK | 0.206243 | 0.406652 | 0.581020 | — | — | — |
| windows_basic_stream_handle_select.test | LINK | 0.202297 | 0.385547 | 0.604010 | — | — | — |
| windows_object_handle.test | LINK | 0.518388 | 1.055448 | 1.563647 | — | — | — |
| windows_object_handle_select.test | LINK | 0.548334 | 1.114903 | 2.088069 | — | — | — |
| windows_overlapped_ptr.test | LINK | 0.543737 | 1.139116 | 1.704164 | — | — | — |
| windows_overlapped_ptr_select.test | LINK | 0.584833 | 1.193834 | 2.226225 | — | — | — |
| windows_random_access_handle.test | LINK | 0.514903 | 1.068436 | 1.535098 | — | — | — |
| windows_random_access_handle_select.test | LINK | 0.546114 | 1.161499 | 2.043914 | — | — | — |
| windows_stream_handle.test | LINK | 0.522487 | 1.036738 | 1.552139 | — | — | — |
| windows_stream_handle_select.test | LINK | 0.560444 | 1.126546 | 2.107151 | — | — | — |
| writable_pipe.test | LINK | 0.888640 | 4.271742 | 4.422649 | — | — | — |
| writable_pipe_select.test | LINK | 0.920946 | 4.461097 | 4.888787 | — | — | — |
| write.test | RUN | 2.671634 | 21.754802 | 23.545853 | 0.003036 | 0.002304 | 0.002692 |
| write_select.test | RUN | 2.704090 | 21.565071 | 24.226915 | 0.003072 | 0.002692 | 0.002892 |
| write_at.test | RUN | 2.736184 | 29.271965 | 29.936291 | 0.003358 | 0.003052 | 0.003139 |
| write_at_select.test | RUN | 2.795950 | 29.374819 | 31.102849 | 0.003236 | 0.003481 | 0.003310 |
| any_executor.test | RUN | 1.066640 | 10.384730 | 9.006281 | 0.002597 | 0.003014 | 0.003082 |
| blocking.test | RUN | 2.803249 | 12.491089 | 20.218329 | 0.008912 | 0.007352 | 0.008994 |
| blocking_adaptation.test | RUN | 1.404203 | 5.637557 | 9.229888 | 0.004548 | 0.004702 | 0.005117 |
| context_as.test | RUN | 0.714996 | 3.829509 | 3.123761 | 0.002356 | 0.002456 | 0.002267 |
| executor.test | RUN | 0.201955 | 0.418029 | 0.623621 | 0.002738 | 0.001892 | 0.001808 |
| inline_exception_handling.test | RUN | 2.760130 | 11.751737 | 19.337481 | 0.007257 | 0.005844 | 0.005796 |
| invocable_archetype.test | RUN | 0.217423 | 0.397243 | 0.604623 | 0.002026 | 0.002240 | 0.001529 |
| mapping.test | RUN | 2.495303 | 12.053434 | 19.677835 | 0.006041 | 0.005971 | 0.006051 |
| outstanding_work.test | RUN | 1.297550 | 5.089902 | 9.098337 | 0.004834 | 0.004873 | 0.004194 |
| prefer_only.test | RUN | 0.439582 | 2.102147 | 2.112196 | 0.002231 | 0.002154 | 0.002162 |
| relationship.test | RUN | 0.821513 | 4.204821 | 8.704597 | 0.003442 | 0.003327 | 0.003241 |

## Native runtime replay

Child CPU of successful native tests, with process startup included; GDB and expected-failure executions are correctness-only. Two warmups per compiler, four A/A observations per reference and mirrored ABBA/BAAB blocks. Short tests repeat up to ten times, identically across compilers; CPU is divided by repetitions before summing tests. Executable hashes and output checks stay outside process timers. All runs must pass their original native oracle. Output hashes record equality or variation across the complete campaign; test reports may print times or addresses.

| Reference | CPU speedup ↑ | Paired blocks | A/A speedup |
|---|---:|---|---:|
| gcc | 0.986× | 0.979×, 0.992× | 0.991× |
| clang | 0.987× | 0.998×, 0.976× | 1.006× |

† A/A drift or paired-block spread exceeds 5%; small differences are unresolved. 1375 native tests in the full cohort; 1359 have byte-identical output throughout. [Manifest](https://github.com/vishvananda/cppgm-extended/actions/runs/42643204761), [paired observations](https://github.com/vishvananda/cppgm-extended/actions/runs/42643204761) and [per-test summaries](https://github.com/vishvananda/cppgm-extended/actions/runs/42643204761).

| Test | Repeats | CPU cppgm | GCC | Clang | Output check |
|---|---:|---:|---:|---:|---|
| any_completion_executor.test | 3 | 0.002095s | 0.002047s | 0.002068s | byte equality observed |
| any_completion_executor_select.test | 2 | 0.002044s | 0.002043s | 0.002014s | byte equality observed |
| any_completion_handler.test | 5 | 0.001635s | 0.001595s | 0.001587s | byte equality observed |
| any_completion_handler_select.test | 8 | 0.001622s | 0.001601s | 0.001608s | byte equality observed |
| any_io_executor.test | 5 | 0.002036s | 0.002025s | 0.002014s | byte equality observed |
| any_io_executor_select.test | 4 | 0.002070s | 0.002040s | 0.002012s | byte equality observed |
| append.test | 9 | 0.001569s | 0.001547s | 0.001532s | byte equality observed |
| append_select.test | 8 | 0.001535s | 0.001526s | 0.001498s | byte equality observed |
| as_tuple.test | 6 | 0.001712s | 0.001675s | 0.001686s | byte equality observed |
| as_tuple_select.test | 8 | 0.001566s | 0.001560s | 0.001552s | byte equality observed |
| bind_allocator.test | 1 | 0.002005s | 0.001978s | 0.001965s | byte equality observed |
| bind_allocator_select.test | 1 | 0.001710s | 0.001703s | 0.001731s | byte equality observed |
| bind_cancellation_slot.test | 9 | 0.001403s | 0.001373s | 0.001356s | byte equality observed |
| bind_cancellation_slot_select.test | 10 | 0.001315s | 0.001295s | 0.001300s | byte equality observed |
| bind_executor.test | 1 | 0.001973s | 0.001948s | 0.001946s | byte equality observed |
| bind_executor_select.test | 1 | 0.002181s | 0.002098s | 0.002184s | byte equality observed |
| bind_immediate_executor.test | 10 | 0.001374s | 0.001353s | 0.001379s | byte equality observed |
| bind_immediate_executor_select.test | 10 | 0.001380s | 0.001345s | 0.001341s | byte equality observed |
| buffer.test | 8 | 0.001340s | 0.001321s | 0.001319s | byte equality observed |
| buffer_select.test | 10 | 0.001295s | 0.001304s | 0.001312s | byte equality observed |
| buffered_read_stream.test | 10 | 0.001709s | 0.001666s | 0.001662s | byte equality observed |
| buffered_read_stream_select.test | 10 | 0.001637s | 0.001753s | 0.001630s | byte equality observed |
| buffered_stream.test | 10 | 0.001704s | 0.001698s | 0.001654s | byte equality observed |
| buffered_stream_select.test | 10 | 0.001666s | 0.001637s | 0.001663s | byte equality observed |
| buffered_write_stream.test | 10 | 0.001735s | 0.001687s | 0.001662s | byte equality observed |
| buffered_write_stream_select.test | 10 | 0.001641s | 0.001643s | 0.001656s | byte equality observed |
| buffers_iterator.test | 10 | 0.001327s | 0.001286s | 0.001265s | byte equality observed |
| buffers_iterator_select.test | 10 | 0.001235s | 0.001268s | 0.001238s | byte equality observed |
| cancel_after.test | 1 | 0.003499s | 0.003364s | 0.003427s | byte equality observed |
| cancel_after_select.test | 1 | 0.003021s | 0.003030s | 0.003065s | byte equality observed |
| cancel_at.test | 1 | 0.003654s | 0.003595s | 0.003899s | byte equality observed |
| cancel_at_select.test | 1 | 0.003639s | 0.003381s | 0.003560s | byte equality observed |
| co_composed.test | 10 | 0.001268s | 0.001261s | 0.001246s | byte equality observed |
| co_composed_select.test | 10 | 0.001255s | 0.001246s | 0.001242s | byte equality observed |
| compose.test | 1 | 0.002040s | 0.002034s | 0.002025s | byte equality observed |
| compose_select.test | 1 | 0.001648s | 0.001609s | 0.001625s | byte equality observed |
| composed.test | 1 | 0.002217s | 0.002141s | 0.002217s | byte equality observed |
| composed_select.test | 1 | 0.001778s | 0.001688s | 0.001777s | byte equality observed |
| config.test | 8 | 0.001359s | 0.001356s | 0.001313s | byte equality observed |
| config_select.test | 10 | 0.001409s | 0.001354s | 0.001365s | byte equality observed |
| connect_pipe.test | 10 | 0.001434s | 0.001371s | 0.001379s | byte equality observed |
| connect_pipe_select.test | 10 | 0.001390s | 0.001432s | 0.001377s | byte equality observed |
| consign.test | 10 | 0.001405s | 0.001398s | 0.001392s | byte equality observed |
| consign_select.test | 10 | 0.001303s | 0.001345s | 0.001254s | byte equality observed |
| deadline_timer.test | 1 | 0.002818s | 0.002788s | 0.002817s | byte equality observed |
| deadline_timer_select.test | 1 | 0.002578s | 0.002525s | 0.002527s | byte equality observed |
| deferred.test | 10 | 0.001227s | 0.001213s | 0.001225s | byte equality observed |
| deferred_select.test | 10 | 0.001181s | 0.001160s | 0.001189s | byte equality observed |
| disposition.test | 10 | 0.001254s | 0.001230s | 0.001253s | byte equality observed |
| disposition_select.test | 10 | 0.001253s | 0.001251s | 0.001248s | byte equality observed |
| error.test | 10 | 0.001219s | 0.001223s | 0.001225s | byte equality observed |
| error_select.test | 10 | 0.001219s | 0.001214s | 0.001214s | byte equality observed |
| inline_executor.test | 10 | 0.001229s | 0.001239s | 0.001227s | byte equality observed |
| inline_executor_select.test | 10 | 0.001232s | 0.001230s | 0.001220s | byte equality observed |
| inline_or_executor.test | 10 | 0.001231s | 0.001235s | 0.001218s | byte equality observed |
| inline_or_executor_select.test | 10 | 0.001238s | 0.001236s | 0.001221s | byte equality observed |
| io_context.test | 1 | 0.002254s | 0.002246s | 0.002262s | byte equality observed |
| io_context_select.test | 1 | 0.002551s | 0.002412s | 0.002546s | byte equality observed |
| io_context_strand.test | 1 | 0.002643s | 0.002567s | 0.002612s | byte equality observed |
| io_context_strand_select.test | 1 | 0.002363s | 0.002278s | 0.002295s | byte equality observed |
| ip_host_name.test | 10 | 0.001237s | 0.001228s | 0.001223s | byte equality observed |
| ip_host_name_select.test | 10 | 0.001190s | 0.001186s | 0.001186s | byte equality observed |
| ip_icmp.test | 10 | 0.001242s | 0.001229s | 0.001207s | byte equality observed |
| ip_icmp_select.test | 10 | 0.001231s | 0.001232s | 0.001187s | byte equality observed |
| ip_multicast.test | 10 | 0.001331s | 0.001321s | 0.001308s | byte equality observed |
| ip_multicast_select.test | 10 | 0.001305s | 0.001293s | 0.001272s | byte equality observed |
| ip_tcp.test | 10 | 0.001837s | 0.001805s | 0.001816s | byte equality observed |
| ip_tcp_select.test | 10 | 0.001763s | 0.001748s | 0.001739s | byte equality observed |
| ip_udp.test | 10 | 0.001353s | 0.001463s | 0.001309s | byte equality observed |
| ip_udp_select.test | 10 | 0.001344s | 0.001450s | 0.001288s | byte equality observed |
| ip_unicast.test | 10 | 0.001282s | 0.001373s | 0.001248s | byte equality observed |
| ip_unicast_select.test | 10 | 0.001276s | 0.001441s | 0.001219s | byte equality observed |
| ip_v6_only.test | 10 | 0.001297s | 0.001391s | 0.001241s | byte equality observed |
| ip_v6_only_select.test | 10 | 0.001273s | 0.001390s | 0.001270s | byte equality observed |
| is_read_buffered.test | 10 | 0.001197s | 0.001306s | 0.001327s | byte equality observed |
| is_read_buffered_select.test | 10 | 0.001193s | 0.001275s | 0.001279s | byte equality observed |
| is_write_buffered.test | 10 | 0.001191s | 0.001303s | 0.001297s | byte equality observed |
| is_write_buffered_select.test | 10 | 0.001239s | 0.001302s | 0.001294s | byte equality observed |
| prepend.test | 10 | 0.001401s | 0.001432s | 0.001412s | byte equality observed |
| prepend_select.test | 10 | 0.001341s | 0.001374s | 0.001362s | byte equality observed |
| read.test | 10 | 0.001934s | 0.001824s | 0.001785s | byte equality observed |
| read_at.test | 9 | 0.002365s | 0.002314s | 0.002185s | byte equality observed |
| read_at_select.test | 9 | 0.002381s | 0.002286s | 0.002161s | byte equality observed |
| read_select.test | 10 | 0.001951s | 0.001797s | 0.001821s | byte equality observed |
| read_until.test | 10 | 0.001523s | 0.001528s | 0.001483s | byte equality observed |
| read_until_select.test | 10 | 0.001553s | 0.001488s | 0.001471s | byte equality observed |
| recycling_allocator.test | 10 | 0.001299s | 0.001321s | 0.001249s | byte equality observed |
| recycling_allocator_select.test | 10 | 0.001314s | 0.001299s | 0.001248s | byte equality observed |
| signal_set.test | 10 | 0.001338s | 0.001319s | 0.001335s | byte equality observed |
| signal_set_select.test | 10 | 0.001339s | 0.001295s | 0.001287s | byte equality observed |
| socket_base.test | 10 | 0.001467s | 0.001481s | 0.001465s | byte equality observed |
| socket_base_select.test | 10 | 0.001457s | 0.001417s | 0.001409s | byte equality observed |
| spawn.test | 10 | 0.001582s | 0.001567s | 0.001559s | byte equality observed |
| spawn_select.test | 10 | 0.001588s | 0.001541s | 0.001505s | byte equality observed |
| static_thread_pool.test | 10 | 0.001301s | 0.001253s | 0.001319s | byte equality observed |
| static_thread_pool_select.test | 10 | 0.001254s | 0.001276s | 0.001294s | byte equality observed |
| strand.test | 1 | 0.002361s | 0.002374s | 0.002440s | byte equality observed |
| strand_select.test | 1 | 0.002051s | 0.001977s | 0.002107s | byte equality observed |
| streambuf.test | 10 | 0.001310s | 0.001301s | 0.001328s | byte equality observed |
| streambuf_select.test | 10 | 0.001379s | 0.001272s | 0.001285s | byte equality observed |
| thread_pool.test | 8 | 0.003045s | 0.002973s | 0.002936s | byte equality observed |
| thread_pool_select.test | 8 | 0.003026s | 0.003009s | 0.002882s | byte equality observed |
| write.test | 10 | 0.002024s | 0.001822s | 0.001820s | byte equality observed |
| write_at.test | 9 | 0.002162s | 0.002052s | 0.001977s | byte equality observed |
| write_at_select.test | 9 | 0.002130s | 0.002000s | 0.001939s | byte equality observed |
| write_select.test | 10 | 0.001948s | 0.001843s | 0.001957s | byte equality observed |

Compiler command and timing observations: [cppgm](https://github.com/vishvananda/cppgm-extended/actions/runs/42643204761) · [gcc](https://github.com/vishvananda/cppgm-extended/actions/runs/42643204761) · [clang](https://github.com/vishvananda/cppgm-extended/actions/runs/42643204761).

[Reviewed outcomes and per-test records](../results.json) · [Host and toolchains](../metadata.json)
