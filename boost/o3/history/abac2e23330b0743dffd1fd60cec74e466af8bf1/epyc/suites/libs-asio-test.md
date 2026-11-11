# libs/asio/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| any_completion_executor.test | RUN | 0.957164 | 5.322185 | 5.502058 | 0.003136 | 0.003315 | 0.002842 |
| any_completion_executor_select.test | RUN | 0.943508 | 5.328442 | 6.021813 | 0.002946 | 0.002998 | 0.003194 |
| any_completion_handler.test | RUN | 1.065749 | 4.665403 | 5.184559 | 0.002191 | 0.002550 | 0.002440 |
| any_completion_handler_select.test | RUN | 1.068491 | 4.813101 | 5.183168 | 0.002397 | 0.002452 | 0.002547 |
| any_io_executor.test | RUN | 0.973403 | 5.720962 | 5.981856 | 0.002743 | 0.003064 | 0.003174 |
| any_io_executor_select.test | RUN | 0.974471 | 5.571445 | 6.435924 | 0.002760 | 0.003057 | 0.003317 |
| append.test | RUN | 0.928571 | 4.463237 | 4.433626 | 0.002142 | 0.002521 | 0.002176 |
| append_select.test | RUN | 0.905230 | 4.579286 | 4.681452 | 0.002094 | 0.002430 | 0.002319 |
| as_tuple.test | RUN | 1.142136 | 5.276449 | 6.332558 | 0.002640 | 0.002332 | 0.002778 |
| as_tuple_select.test | RUN | 1.216846 | 5.260456 | 6.219979 | 0.002353 | 0.002476 | 0.002184 |
| associated_allocator.test | LINK | 0.258662 | 0.504339 | 0.778945 | — | — | — |
| associated_allocator_select.test | LINK | 0.265574 | 0.511339 | 0.761800 | — | — | — |
| associated_cancellation_slot.test | LINK | 0.289152 | 0.544386 | 0.834254 | — | — | — |
| associated_cancellation_slot_select.test | LINK | 0.283195 | 0.548987 | 0.824251 | — | — | — |
| associated_executor.test | LINK | 0.424486 | 0.937204 | 1.340116 | — | — | — |
| associated_executor_select.test | LINK | 0.407799 | 0.911687 | 1.289957 | — | — | — |
| associated_immediate_executor.test | LINK | 0.386547 | 0.858735 | 1.111302 | — | — | — |
| associated_immediate_executor_select.test | LINK | 0.375492 | 0.813421 | 1.134040 | — | — | — |
| associator.test | LINK | 0.216669 | 0.400618 | 0.584942 | — | — | — |
| associator_select.test | LINK | 0.213065 | 0.407853 | 0.590144 | — | — | — |
| awaitable.test | LINK | 0.208661 | 0.409457 | 0.575317 | — | — | — |
| awaitable_select.test | LINK | 0.207771 | 0.386822 | 0.620901 | — | — | — |
| basic_datagram_socket.test | LINK | 0.633598 | 1.245533 | 2.443304 | — | — | — |
| basic_datagram_socket_select.test | LINK | 0.634989 | 1.308083 | 2.500423 | — | — | — |
| basic_deadline_timer.test | LINK | 0.755904 | 1.515448 | 2.796998 | — | — | — |
| basic_deadline_timer_select.test | LINK | 0.797756 | 1.560487 | 2.742660 | — | — | — |
| basic_file.test | LINK | 0.216289 | 0.422039 | 0.619514 | — | — | — |
| basic_file_select.test | LINK | 0.210469 | 0.426340 | 0.603811 | — | — | — |
| basic_random_access_file.test | LINK | 0.210878 | 0.395928 | 0.580965 | — | — | — |
| basic_random_access_file_select.test | LINK | 0.208919 | 0.383746 | 0.618377 | — | — | — |
| basic_raw_socket.test | LINK | 0.628654 | 1.256178 | 2.412522 | — | — | — |
| basic_raw_socket_select.test | LINK | 0.651821 | 1.306701 | 2.512497 | — | — | — |
| basic_readable_pipe.test | LINK | 0.616103 | 1.222413 | 2.002902 | — | — | — |
| basic_readable_pipe_select.test | LINK | 0.632300 | 1.293378 | 2.568917 | — | — | — |
| basic_seq_packet_socket.test | LINK | 0.625668 | 1.264879 | 2.372766 | — | — | — |
| basic_seq_packet_socket_select.test | LINK | 0.643035 | 1.286516 | 2.421072 | — | — | — |
| basic_signal_set.test | LINK | 0.589699 | 1.193444 | 1.802355 | — | — | — |
| basic_signal_set_select.test | LINK | 0.602619 | 1.248673 | 2.406088 | — | — | — |
| basic_socket_acceptor.test | LINK | 0.641123 | 1.235555 | 2.464607 | — | — | — |
| basic_socket_acceptor_select.test | LINK | 0.653558 | 1.304210 | 2.409762 | — | — | — |
| basic_stream_file.test | LINK | 0.215306 | 0.417350 | 0.627192 | — | — | — |
| basic_stream_file_select.test | LINK | 0.208042 | 0.421727 | 0.627282 | — | — | — |
| basic_stream_socket.test | LINK | 0.637059 | 1.288857 | 2.402372 | — | — | — |
| basic_stream_socket_select.test | LINK | 0.643744 | 1.282748 | 2.420625 | — | — | — |
| basic_streambuf.test | LINK | 0.313006 | 0.611646 | 0.946491 | — | — | — |
| basic_streambuf_select.test | LINK | 0.328991 | 0.599704 | 0.941836 | — | — | — |
| basic_waitable_timer.test | LINK | 0.582202 | 1.178346 | 2.281972 | — | — | — |
| basic_waitable_timer_select.test | LINK | 0.605921 | 1.221643 | 2.338623 | — | — | — |
| basic_writable_pipe.test | LINK | 0.607640 | 1.196503 | 2.037561 | — | — | — |
| basic_writable_pipe_select.test | LINK | 0.628018 | 1.297309 | 2.466196 | — | — | — |
| bind_allocator.test | RUN | 0.964836 | 4.496400 | 5.005646 | 0.002718 | 0.002955 | 0.002718 |
| bind_allocator_select.test | RUN | 0.952222 | 4.505185 | 4.935085 | 0.002165 | 0.002210 | 0.002539 |
| bind_cancellation_slot.test | RUN | 0.968236 | 4.474618 | 5.014771 | 0.001896 | 0.001832 | 0.001800 |
| bind_cancellation_slot_select.test | RUN | 0.993922 | 4.543838 | 4.748495 | 0.001804 | 0.001798 | 0.001764 |
| bind_executor.test | RUN | 0.625151 | 3.199116 | 3.095836 | 0.002528 | 0.003225 | 0.003232 |
| bind_executor_select.test | RUN | 0.622537 | 3.266542 | 3.426712 | 0.002384 | 0.002940 | 0.003165 |
| bind_immediate_executor.test | RUN | 0.476992 | 2.735197 | 2.116214 | 0.001835 | 0.001848 | 0.001775 |
| bind_immediate_executor_select.test | RUN | 0.490853 | 2.652338 | 2.621346 | 0.001728 | 0.001711 | 0.001679 |
| buffer.test | RUN | 0.294303 | 1.095045 | 1.193519 | 0.001721 | 0.001693 | 0.001663 |
| buffer_select.test | RUN | 0.291631 | 1.057594 | 1.189793 | 0.001680 | 0.001719 | 0.001648 |
| buffer_registration.test | LINK | 0.251847 | 0.543858 | 0.700394 | — | — | — |
| buffer_registration_select.test | LINK | 0.251077 | 0.539677 | 0.685818 | — | — | — |
| buffered_read_stream.test | RUN | 1.014898 | 7.135288 | 7.362576 | 0.002180 | 0.002817 | 0.003061 |
| buffered_read_stream_select.test | RUN | 0.993470 | 7.508387 | 8.091007 | 0.002180 | 0.002976 | 0.002862 |
| buffered_stream.test | RUN | 1.942689 | 10.818513 | 11.937430 | 0.002751 | 0.002971 | 0.002925 |
| buffered_stream_select.test | RUN | 1.986927 | 10.921263 | 11.723820 | 0.002709 | 0.002869 | 0.002880 |
| buffered_write_stream.test | RUN | 1.752758 | 9.268189 | 10.444967 | 0.002630 | 0.003007 | 0.002989 |
| buffered_write_stream_select.test | RUN | 1.795095 | 9.457869 | 9.646450 | 0.002789 | 0.002872 | 0.002907 |
| buffers_iterator.test | RUN | 0.452551 | 0.955597 | 1.736852 | 0.002256 | 0.002286 | 0.002228 |
| buffers_iterator_select.test | RUN | 0.446573 | 0.931627 | 1.763214 | 0.001692 | 0.002170 | 0.002212 |
| cancel_after.test | RUN | 1.637457 | 7.315326 | 7.608372 | 0.004918 | 0.004906 | 0.004992 |
| cancel_after_select.test | RUN | 1.618454 | 7.224542 | 7.731673 | 0.004360 | 0.005013 | 0.004616 |
| cancel_at.test | RUN | 1.483505 | 5.999948 | 6.879977 | 0.004616 | 0.004506 | 0.003999 |
| cancel_at_select.test | RUN | 1.511148 | 5.863883 | 6.305358 | 0.003775 | 0.004043 | 0.003883 |
| cancellation_signal.test | LINK | 0.174187 | 0.324507 | 0.443133 | — | — | — |
| cancellation_signal_select.test | LINK | 0.172723 | 0.322551 | 0.455734 | — | — | — |
| cancellation_state.test | LINK | 0.263344 | 0.324405 | 0.465001 | — | — | — |
| cancellation_state_select.test | LINK | 0.275888 | 0.324245 | 0.453648 | — | — | — |
| cancellation_type.test | LINK | 0.214822 | 0.256068 | 0.362270 | — | — | — |
| cancellation_type_select.test | LINK | 0.209315 | 0.254425 | 0.388759 | — | — | — |
| co_composed.test | RUN | 0.210145 | 0.254627 | 0.623153 | 0.002164 | 0.001533 | 0.001732 |
| co_composed_select.test | RUN | 0.216720 | 0.257914 | 0.584181 | 0.002064 | 0.001684 | 0.002156 |
| co_spawn.test | LINK | 0.207908 | 0.255341 | 0.607791 | — | — | — |
| co_spawn_select.test | LINK | 0.209915 | 0.266486 | 0.580938 | — | — | — |
| completion_condition.test | LINK | 0.268678 | 0.424736 | 0.814930 | — | — | — |
| completion_condition_select.test | LINK | 0.269104 | 0.550996 | 0.846265 | — | — | — |
| compose.test | RUN | 1.178476 | 5.653404 | 6.401192 | 0.002715 | 0.002789 | 0.002749 |
| compose_select.test | RUN | 1.185610 | 5.493677 | 6.281244 | 0.002799 | 0.002726 | 0.002889 |
| composed.test | RUN | 1.104193 | 5.569419 | 6.006283 | 0.002195 | 0.002401 | 0.002284 |
| composed_select.test | RUN | 1.081000 | 5.463009 | 5.867159 | 0.002279 | 0.002346 | 0.002349 |
| config.test | RUN | 0.534490 | 3.626041 | 2.565948 | 0.002247 | 0.002361 | 0.002483 |
| config_select.test | RUN | 0.602232 | 3.945863 | 3.111003 | 0.002352 | 0.002463 | 0.002330 |
| connect.test | LINK | 1.962076 | 11.794047 | 12.993629 | — | — | — |
| connect_select.test | LINK | 1.920211 | 11.781116 | 13.260667 | — | — | — |
| connect_pipe.test | RUN | 1.030194 | 4.514240 | 4.984811 | 0.002348 | 0.002360 | 0.002489 |
| connect_pipe_select.test | RUN | 1.053011 | 4.731195 | 5.224519 | 0.002265 | 0.002474 | 0.002354 |
| consign.test | RUN | 0.880330 | 4.454509 | 4.407759 | 0.002318 | 0.002346 | 0.002516 |
| consign_select.test | RUN | 0.907493 | 4.615547 | 4.334108 | 0.002284 | 0.002454 | 0.002004 |
| coroutine.test | LINK | 0.237134 | 0.451058 | 0.666067 | — | — | — |
| coroutine_select.test | LINK | 0.222179 | 0.463141 | 0.625660 | — | — | — |
| deadline_timer.test | RUN | 1.286540 | 5.694464 | 6.367271 | 0.003688 | 0.004432 | 0.004188 |
| deadline_timer_select.test | RUN | 1.280050 | 5.694168 | 6.452954 | 0.003332 | 0.004093 | 0.003565 |
| deferred.test | RUN | 0.240525 | 0.452801 | 0.637072 | 0.002240 | 0.002258 | 0.002215 |
| deferred_select.test | RUN | 0.257336 | 0.451413 | 0.656350 | 0.002141 | 0.002202 | 0.002370 |
| detached.test | LINK | 0.269965 | 0.503824 | 0.758760 | — | — | — |
| detached_select.test | LINK | 0.264010 | 0.504958 | 0.766673 | — | — | — |
| disposition.test | RUN | 0.246436 | 1.148038 | 1.334653 | 0.001794 | 0.002410 | 0.002298 |
| disposition_select.test | RUN | 0.225664 | 1.131760 | 1.335512 | 0.001779 | 0.002274 | 0.002468 |
| error.test | RUN | 0.211893 | 1.099323 | 1.162889 | 0.001734 | 0.002276 | 0.002374 |
| error_select.test | RUN | 0.213492 | 1.098938 | 1.169845 | 0.001754 | 0.002202 | 0.002118 |
| file_base.test | LINK | 0.271632 | 0.903267 | 1.247838 | — | — | — |
| file_base_select.test | LINK | 0.275993 | 0.684905 | 1.058969 | — | — | — |
| fwd_first.test | LINK | 0.623081 | 1.259432 | 2.198891 | — | — | — |
| fwd_last.test | LINK | 0.594315 | 1.236283 | 2.223496 | — | — | — |
| generic_basic_endpoint.test | LINK | 0.234133 | 0.518535 | 0.799785 | — | — | — |
| generic_basic_endpoint_select.test | LINK | 0.228601 | 0.515641 | 0.817741 | — | — | — |
| generic_datagram_protocol.test | LINK | 0.867454 | 4.345824 | 4.103053 | — | — | — |
| generic_datagram_protocol_select.test | LINK | 0.841057 | 4.417514 | 4.128273 | — | — | — |
| generic_raw_protocol.test | LINK | 0.872657 | 4.290244 | 4.118420 | — | — | — |
| generic_raw_protocol_select.test | LINK | 0.844705 | 4.393403 | 4.159571 | — | — | — |
| generic_seq_packet_protocol.test | LINK | 0.747432 | 3.681857 | 3.531755 | — | — | — |
| generic_seq_packet_protocol_select.test | LINK | 0.735544 | 3.660172 | 3.535868 | — | — | — |
| generic_stream_protocol.test | LINK | 0.800608 | 3.962828 | 3.765914 | — | — | — |
| generic_stream_protocol_select.test | LINK | 0.792938 | 4.029153 | 3.827781 | — | — | — |
| high_resolution_timer.test | LINK | 0.378916 | 0.781720 | 1.210643 | — | — | — |
| high_resolution_timer_select.test | LINK | 0.383326 | 0.783208 | 1.252955 | — | — | — |
| immediate.test | LINK | 0.259608 | 0.591480 | 0.765646 | — | — | — |
| immediate_select.test | LINK | 0.267871 | 0.581845 | 0.755668 | — | — | — |
| inline_executor.test | RUN | 0.314238 | 0.903792 | 1.132314 | 0.001598 | 0.001770 | 0.001666 |
| inline_executor_select.test | RUN | 0.322017 | 0.891753 | 1.159680 | 0.001590 | 0.001732 | 0.001788 |
| inline_or_executor.test | RUN | 0.500856 | 2.657644 | 2.103152 | 0.001756 | 0.001815 | 0.001722 |
| inline_or_executor_select.test | RUN | 0.533609 | 2.652030 | 2.374145 | 0.001742 | 0.001744 | 0.001670 |
| io_context.test | RUN | 0.712876 | 4.004129 | 3.692045 | 0.002992 | 0.002882 | 0.002904 |
| io_context_select.test | RUN | 0.738831 | 3.807175 | 3.704807 | 0.002596 | 0.002882 | 0.002910 |
| io_context_strand.test | RUN | 0.666246 | 3.692819 | 3.128921 | 0.002609 | 0.003046 | 0.002704 |
| io_context_strand_select.test | RUN | 0.697904 | 3.740841 | 3.146060 | 0.002637 | 0.003110 | 0.002675 |
| ip_address.test | LINK | 0.263268 | 1.186218 | 0.966481 | — | — | — |
| ip_address_select.test | LINK | 0.273060 | 1.188225 | 0.937630 | — | — | — |
| ip_address_v4.test | LINK | 0.277651 | 0.848163 | 1.061217 | — | — | — |
| ip_address_v4_select.test | LINK | 0.279808 | 0.854365 | 1.017748 | — | — | — |
| ip_address_v6.test | LINK | 0.305704 | 1.084809 | 1.295994 | — | — | — |
| ip_address_v6_select.test | LINK | 0.302114 | 1.124234 | 1.282073 | — | — | — |
| ip_basic_endpoint.test | LINK | 0.246220 | 0.537487 | 0.848576 | — | — | — |
| ip_basic_endpoint_select.test | LINK | 0.239655 | 0.549035 | 0.835060 | — | — | — |
| ip_basic_resolver.test | LINK | 0.388129 | 0.779043 | 1.223362 | — | — | — |
| ip_basic_resolver_select.test | LINK | 0.391409 | 0.811739 | 1.281481 | — | — | — |
| ip_basic_resolver_entry.test | LINK | 0.136943 | 0.256392 | 0.355041 | — | — | — |
| ip_basic_resolver_entry_select.test | LINK | 0.133497 | 0.262146 | 0.349333 | — | — | — |
| ip_basic_resolver_iterator.test | LINK | 0.246148 | 0.522736 | 0.795687 | — | — | — |
| ip_basic_resolver_iterator_select.test | LINK | 0.247284 | 0.539444 | 0.879081 | — | — | — |
| ip_basic_resolver_query.test | LINK | 0.227905 | 0.511506 | 1.448661 | — | — | — |
| ip_basic_resolver_query_select.test | LINK | 0.235600 | 0.517501 | 1.478293 | — | — | — |
| ip_host_name.test | RUN | 0.243768 | 0.677130 | 1.563672 | 0.001582 | 0.001706 | 0.002305 |
| ip_host_name_select.test | RUN | 0.239638 | 0.688130 | 1.650921 | 0.001631 | 0.001700 | 0.001921 |
| ip_icmp.test | RUN | 1.089149 | 8.599231 | 9.031031 | 0.001788 | 0.002282 | 0.002340 |
| ip_icmp_select.test | RUN | 1.063785 | 8.883391 | 8.616486 | 0.001607 | 0.002388 | 0.002307 |
| ip_multicast.test | RUN | 0.651552 | 4.900571 | 5.280672 | 0.001867 | 0.002505 | 0.002464 |
| ip_multicast_select.test | RUN | 0.640790 | 4.806085 | 5.300727 | 0.001712 | 0.002324 | 0.002284 |
| ip_resolver_query_base.test | LINK | 0.154676 | 0.444236 | 0.623837 | — | — | — |
| ip_resolver_query_base_select.test | LINK | 0.158663 | 0.448778 | 0.626227 | — | — | — |
| ip_tcp.test | RUN | 1.631051 | 13.489693 | 13.270569 | 0.002670 | 0.003458 | 0.003118 |
| ip_tcp_select.test | RUN | 1.587805 | 14.180194 | 12.984754 | 0.003112 | 0.003130 | 0.002892 |
| ip_udp.test | RUN | 1.744778 | 9.832989 | 9.347480 | 0.002670 | 0.002621 | 0.002385 |
| ip_udp_select.test | RUN | 1.737036 | 10.188050 | 9.181087 | 0.002389 | 0.002474 | 0.002366 |
| ip_unicast.test | RUN | 0.959020 | 4.386229 | 4.502697 | 0.002394 | 0.002531 | 0.002060 |
| ip_unicast_select.test | RUN | 0.958501 | 4.429303 | 4.471295 | 0.002173 | 0.002600 | 0.002132 |
| ip_v6_only.test | RUN | 0.952919 | 4.579524 | 4.761156 | 0.002394 | 0.002476 | 0.002431 |
| ip_v6_only_select.test | RUN | 0.952234 | 4.569962 | 4.876387 | 0.002448 | 0.002223 | 0.002375 |
| is_read_buffered.test | RUN | 0.713385 | 1.423505 | 2.665910 | 0.002022 | 0.002259 | 0.002212 |
| is_read_buffered_select.test | RUN | 0.732988 | 1.449229 | 2.735358 | 0.001916 | 0.002320 | 0.002363 |
| is_write_buffered.test | RUN | 0.707778 | 1.440345 | 2.747883 | 0.002053 | 0.002310 | 0.001883 |
| is_write_buffered_select.test | RUN | 0.713114 | 1.471244 | 2.701887 | 0.001747 | 0.002020 | 0.002078 |
| local_basic_endpoint.test | LINK | 0.379906 | 0.803237 | 1.502817 | — | — | — |
| local_basic_endpoint_select.test | LINK | 0.375722 | 0.792757 | 1.473991 | — | — | — |
| local_connect_pair.test | LINK | 0.892322 | 4.384008 | 4.414028 | — | — | — |
| local_connect_pair_select.test | LINK | 0.884975 | 4.425229 | 4.435129 | — | — | — |
| local_datagram_protocol.test | LINK | 1.140010 | 5.606085 | 6.044852 | — | — | — |
| local_datagram_protocol_select.test | LINK | 1.166535 | 5.635883 | 5.932256 | — | — | — |
| local_seq_packet_protocol.test | LINK | 1.148743 | 5.587516 | 5.954978 | — | — | — |
| local_seq_packet_protocol_select.test | LINK | 1.166485 | 5.632763 | 5.944821 | — | — | — |
| local_stream_protocol.test | LINK | 1.093907 | 5.298987 | 5.696680 | — | — | — |
| local_stream_protocol_select.test | LINK | 1.109803 | 5.307252 | 5.583273 | — | — | — |
| placeholders.test | LINK | 0.227550 | 0.428853 | 0.648405 | — | — | — |
| placeholders_select.test | LINK | 0.227159 | 0.430750 | 0.643896 | — | — | — |
| posix_basic_descriptor.test | LINK | 0.602445 | 1.211826 | 1.964572 | — | — | — |
| posix_basic_descriptor_select.test | LINK | 0.644502 | 1.315499 | 2.484763 | — | — | — |
| posix_basic_stream_descriptor.test | LINK | 0.604825 | 1.206181 | 2.040979 | — | — | — |
| posix_basic_stream_descriptor_select.test | LINK | 0.625287 | 1.285364 | 2.481327 | — | — | — |
| posix_descriptor_base.test | LINK | 0.252685 | 0.463990 | 0.671293 | — | — | — |
| posix_descriptor_base_select.test | LINK | 0.258315 | 0.457903 | 0.666652 | — | — | — |
| posix_stream_descriptor.test | LINK | 1.097609 | 5.449486 | 5.434791 | — | — | — |
| posix_stream_descriptor_select.test | LINK | 1.119511 | 5.462978 | 5.816256 | — | — | — |
| prepend.test | RUN | 0.891170 | 4.402388 | 4.538583 | 0.002361 | 0.002392 | 0.002262 |
| prepend_select.test | RUN | 0.916153 | 4.522404 | 4.480947 | 0.002274 | 0.002244 | 0.002505 |
| random_access_file.test | LINK | 0.549716 | 1.075215 | 1.529361 | — | — | — |
| random_access_file_select.test | LINK | 0.579139 | 1.189365 | 2.165696 | — | — | — |
| read.test | RUN | 2.729625 | 27.552183 | 24.662210 | 0.003087 | 0.002968 | 0.002746 |
| read_select.test | RUN | 2.805946 | 27.217481 | 25.399340 | 0.003262 | 0.002929 | 0.003214 |
| read_at.test | RUN | 2.736922 | 40.027109 | 34.429120 | 0.003808 | 0.004112 | 0.003738 |
| read_at_select.test | RUN | 2.778452 | 38.959371 | 34.937281 | 0.004034 | 0.003894 | 0.003384 |
| read_until.test | RUN | 1.471602 | 10.171882 | 8.933846 | 0.002584 | 0.002560 | 0.002404 |
| read_until_select.test | RUN | 1.527168 | 10.405270 | 9.308519 | 0.002428 | 0.002391 | 0.002510 |
| readable_pipe.test | LINK | 0.936400 | 4.324558 | 4.348135 | — | — | — |
| readable_pipe_select.test | LINK | 0.920349 | 4.635860 | 4.789861 | — | — | — |
| recycling_allocator.test | RUN | 0.301670 | 0.701234 | 0.975538 | 0.001981 | 0.002288 | 0.002251 |
| recycling_allocator_select.test | RUN | 0.302923 | 0.710378 | 0.916876 | 0.001878 | 0.002317 | 0.001922 |
| redirect_disposition.test | LINK | 1.254773 | 6.170694 | 6.917604 | — | — | — |
| redirect_disposition_select.test | LINK | 1.276901 | 6.126110 | 6.763689 | — | — | — |
| redirect_error.test | LINK | 1.102635 | 5.221664 | 5.946692 | — | — | — |
| redirect_error_select.test | LINK | 1.139391 | 5.316106 | 5.986780 | — | — | — |
| registered_buffer.test | LINK | 0.299259 | 0.568797 | 0.926361 | — | — | — |
| registered_buffer_select.test | LINK | 0.299644 | 0.549727 | 0.833045 | — | — | — |
| signal_set.test | RUN | 0.881155 | 4.356453 | 4.037985 | 0.002114 | 0.002320 | 0.002216 |
| signal_set_select.test | RUN | 0.909163 | 4.550760 | 4.632196 | 0.002161 | 0.002436 | 0.002127 |
| signal_set_base.test | LINK | 0.243969 | 0.445216 | 0.611492 | — | — | — |
| signal_set_base_select.test | LINK | 0.243867 | 0.451757 | 0.615308 | — | — | — |
| socket_base.test | RUN | 1.132843 | 6.677652 | 6.616321 | 0.002365 | 0.002488 | 0.002538 |
| socket_base_select.test | RUN | 1.124245 | 6.664063 | 6.469188 | 0.002322 | 0.002401 | 0.002503 |
| spawn.test | RUN | 1.807449 | 10.178893 | 10.704948 | 0.002625 | 0.002702 | 0.002764 |
| spawn_select.test | RUN | 1.817517 | 10.137868 | 10.691190 | 0.002516 | 0.002668 | 0.002688 |
| static_thread_pool.test | RUN | 0.525757 | 1.054348 | 1.586158 | 0.002151 | 0.002250 | 0.002238 |
| static_thread_pool_select.test | RUN | 0.558540 | 1.157717 | 1.984925 | 0.001943 | 0.002138 | 0.002268 |
| steady_timer.test | LINK | 0.586649 | 1.202748 | 2.236610 | — | — | — |
| steady_timer_select.test | LINK | 0.625994 | 1.250997 | 2.329161 | — | — | — |
| strand.test | RUN | 1.177030 | 5.959498 | 6.413092 | 0.003081 | 0.003158 | 0.003520 |
| strand_select.test | RUN | 1.211468 | 5.872453 | 6.368853 | 0.002771 | 0.003090 | 0.003494 |
| stream_file.test | LINK | 0.549620 | 1.064414 | 1.599544 | — | — | — |
| stream_file_select.test | LINK | 0.582804 | 1.180340 | 2.185851 | — | — | — |
| streambuf.test | RUN | 0.285525 | 0.907206 | 1.074129 | 0.001558 | 0.001715 | 0.002272 |
| streambuf_select.test | RUN | 0.225264 | 0.647170 | 1.086812 | 0.001714 | 0.001714 | 0.002354 |
| system_timer.test | LINK | 0.744046 | 3.686330 | 3.810510 | — | — | — |
| system_timer_select.test | LINK | 0.720959 | 3.597267 | 3.541951 | — | — | — |
| system_context.test | LINK | 0.349508 | 0.688342 | 0.869878 | — | — | — |
| system_context_select.test | LINK | 0.358537 | 0.728143 | 1.129539 | — | — | — |
| system_executor.test | LINK | 0.445908 | 2.116493 | 1.659618 | — | — | — |
| system_executor_select.test | LINK | 0.446404 | 2.039444 | 1.895872 | — | — | — |
| this_coro.test | LINK | 0.137467 | 0.262295 | 0.346092 | — | — | — |
| this_coro_select.test | LINK | 0.133856 | 0.266759 | 0.359449 | — | — | — |
| thread_pool.test | RUN | 0.523070 | 3.909880 | 2.657580 | 0.003459 | 0.003818 | 0.004302 |
| thread_pool_select.test | RUN | 0.522372 | 4.180006 | 3.834339 | 0.003436 | 0.004169 | 0.003734 |
| time_traits.test | LINK | 0.308167 | 0.824324 | 1.357548 | — | — | — |
| time_traits_select.test | LINK | 0.304145 | 0.853264 | 1.228434 | — | — | — |
| ts_buffer.test | LINK | 0.295642 | 0.922592 | 1.322543 | — | — | — |
| ts_buffer_select.test | LINK | 0.286653 | 0.953560 | 1.310906 | — | — | — |
| ts_executor.test | LINK | 0.402516 | 1.304880 | 2.045521 | — | — | — |
| ts_executor_select.test | LINK | 0.411938 | 1.394892 | 2.574261 | — | — | — |
| ts_internet.test | LINK | 0.438820 | 1.392536 | 2.716659 | — | — | — |
| ts_internet_select.test | LINK | 0.465174 | 1.434496 | 2.762887 | — | — | — |
| ts_io_context.test | LINK | 0.354646 | 1.111561 | 1.548605 | — | — | — |
| ts_io_context_select.test | LINK | 0.365023 | 1.149902 | 2.076968 | — | — | — |
| ts_net.test | LINK | 0.519035 | 1.672656 | 3.251770 | — | — | — |
| ts_net_select.test | LINK | 0.541916 | 1.695416 | 3.370143 | — | — | — |
| ts_netfwd.test | LINK | 0.518287 | 1.700481 | 3.327885 | — | — | — |
| ts_netfwd_select.test | LINK | 0.542796 | 1.742192 | 3.226644 | — | — | — |
| ts_socket.test | LINK | 0.458516 | 1.340320 | 2.523557 | — | — | — |
| ts_socket_select.test | LINK | 0.527962 | 1.371956 | 2.602377 | — | — | — |
| ts_timer.test | LINK | 0.613965 | 1.208308 | 2.251226 | — | — | — |
| ts_timer_select.test | LINK | 0.596768 | 1.258701 | 2.217213 | — | — | — |
| use_awaitable.test | LINK | 0.206434 | 0.413156 | 0.627798 | — | — | — |
| use_awaitable_select.test | LINK | 0.222227 | 0.410523 | 0.568641 | — | — | — |
| wait_traits.test | LINK | 0.212935 | 0.410940 | 0.582807 | — | — | — |
| wait_traits_select.test | LINK | 0.210942 | 0.399041 | 0.599662 | — | — | — |
| windows_basic_object_handle.test | LINK | 0.212221 | 0.403119 | 0.583559 | — | — | — |
| windows_basic_object_handle_select.test | LINK | 0.212119 | 0.413027 | 0.600687 | — | — | — |
| windows_basic_overlapped_handle.test | LINK | 0.209859 | 0.417875 | 0.569092 | — | — | — |
| windows_basic_overlapped_handle_select.test | LINK | 0.208012 | 0.403152 | 0.581389 | — | — | — |
| windows_basic_random_access_handle.test | LINK | 0.216715 | 0.405963 | 0.566195 | — | — | — |
| windows_basic_random_access_handle_select.test | LINK | 0.205947 | 0.414989 | 0.613760 | — | — | — |
| windows_basic_stream_handle.test | LINK | 0.209919 | 0.406848 | 0.591386 | — | — | — |
| windows_basic_stream_handle_select.test | LINK | 0.213333 | 0.393797 | 0.595566 | — | — | — |
| windows_object_handle.test | LINK | 0.560067 | 1.084761 | 1.599479 | — | — | — |
| windows_object_handle_select.test | LINK | 0.574258 | 1.162971 | 2.086700 | — | — | — |
| windows_overlapped_ptr.test | LINK | 0.570121 | 1.169913 | 1.764020 | — | — | — |
| windows_overlapped_ptr_select.test | LINK | 0.633798 | 1.255180 | 2.195373 | — | — | — |
| windows_random_access_handle.test | LINK | 0.562172 | 1.086597 | 1.598429 | — | — | — |
| windows_random_access_handle_select.test | LINK | 0.573875 | 1.157431 | 2.066940 | — | — | — |
| windows_stream_handle.test | LINK | 0.549828 | 1.107837 | 1.578738 | — | — | — |
| windows_stream_handle_select.test | LINK | 0.604819 | 1.158577 | 2.057989 | — | — | — |
| writable_pipe.test | LINK | 0.937577 | 4.432808 | 4.649668 | — | — | — |
| writable_pipe_select.test | LINK | 0.955131 | 4.581709 | 4.909039 | — | — | — |
| write.test | RUN | 2.736253 | 23.221369 | 24.253937 | 0.003319 | 0.002980 | 0.003270 |
| write_select.test | RUN | 2.825602 | 22.855668 | 24.686873 | 0.003375 | 0.003051 | 0.003196 |
| write_at.test | RUN | 2.857931 | 33.302504 | 31.393040 | 0.003396 | 0.003171 | 0.003102 |
| write_at_select.test | RUN | 2.847476 | 32.863896 | 32.136379 | 0.003608 | 0.003651 | 0.003537 |
| any_executor.test | RUN | 1.152380 | 10.830874 | 10.578690 | 0.003017 | 0.002986 | 0.002899 |
| blocking.test | RUN | 2.841900 | 12.677995 | 21.031361 | 0.009340 | 0.007401 | 0.009091 |
| blocking_adaptation.test | RUN | 1.455853 | 5.707606 | 9.695033 | 0.004902 | 0.005223 | 0.005164 |
| context_as.test | RUN | 0.774712 | 4.038957 | 3.246196 | 0.002422 | 0.002521 | 0.002412 |
| executor.test | RUN | 0.231789 | 0.441721 | 0.640748 | 0.002307 | 0.002289 | 0.002198 |
| inline_exception_handling.test | RUN | 2.682576 | 12.198479 | 21.025162 | 0.006091 | 0.005905 | 0.005994 |
| invocable_archetype.test | RUN | 0.228757 | 0.435686 | 0.619819 | 0.002338 | 0.002315 | 0.002290 |
| mapping.test | RUN | 2.703610 | 11.805778 | 19.882371 | 0.006167 | 0.006176 | 0.006230 |
| outstanding_work.test | RUN | 1.357322 | 5.405860 | 9.380355 | 0.004788 | 0.004970 | 0.005012 |
| prefer_only.test | RUN | 0.472352 | 2.177659 | 2.145468 | 0.002207 | 0.002282 | 0.002260 |
| relationship.test | RUN | 1.053563 | 5.246514 | 9.271451 | 0.003373 | 0.005147 | 0.004901 |

## Native runtime replay

Child CPU of successful native tests, with process startup included; GDB and expected-failure executions are correctness-only. Two warmups per compiler, four A/A observations per reference and mirrored ABBA/BAAB blocks. Short tests repeat up to ten times, identically across compilers; CPU is divided by repetitions before summing tests. Executable hashes and output checks stay outside process timers. All runs must pass their original native oracle. Output hashes record equality or variation across the complete campaign; test reports may print times or addresses.

| Reference | CPU speedup ↑ | Paired blocks | A/A speedup |
|---|---:|---|---:|
| gcc | 0.991× | 0.994×, 0.988× | 1.001× |
| clang | 0.987× | 0.987×, 0.987× | 1.000× |

† A/A drift or paired-block spread exceeds 5%; small differences are unresolved. 1375 native tests in the full cohort; 1359 have byte-identical output throughout. [Manifest](https://github.com/vishvananda/cppgm-extended/actions/runs/42692924255), [paired observations](https://github.com/vishvananda/cppgm-extended/actions/runs/42692924255) and [per-test summaries](https://github.com/vishvananda/cppgm-extended/actions/runs/42692924255).

| Test | Repeats | CPU cppgm | GCC | Clang | Output check |
|---|---:|---:|---:|---:|---|
| any_completion_executor.test | 3 | 0.002125s | 0.002056s | 0.002093s | byte equality observed |
| any_completion_executor_select.test | 2 | 0.002082s | 0.002086s | 0.002141s | byte equality observed |
| any_completion_handler.test | 5 | 0.001690s | 0.001644s | 0.001641s | byte equality observed |
| any_completion_handler_select.test | 8 | 0.001673s | 0.001645s | 0.001635s | byte equality observed |
| any_io_executor.test | 4 | 0.002118s | 0.002032s | 0.002073s | byte equality observed |
| any_io_executor_select.test | 5 | 0.002117s | 0.002064s | 0.002089s | byte equality observed |
| append.test | 6 | 0.001586s | 0.001561s | 0.001565s | byte equality observed |
| append_select.test | 8 | 0.001558s | 0.001494s | 0.001540s | byte equality observed |
| as_tuple.test | 7 | 0.001758s | 0.001743s | 0.001718s | byte equality observed |
| as_tuple_select.test | 8 | 0.001592s | 0.001562s | 0.001556s | byte equality observed |
| bind_allocator.test | 1 | 0.002110s | 0.002010s | 0.002077s | byte equality observed |
| bind_allocator_select.test | 1 | 0.001784s | 0.001648s | 0.001793s | byte equality observed |
| bind_cancellation_slot.test | 10 | 0.001418s | 0.001399s | 0.001425s | byte equality observed |
| bind_cancellation_slot_select.test | 10 | 0.001333s | 0.001320s | 0.001313s | byte equality observed |
| bind_executor.test | 1 | 0.002039s | 0.001922s | 0.001968s | byte equality observed |
| bind_executor_select.test | 1 | 0.002264s | 0.002216s | 0.002242s | byte equality observed |
| bind_immediate_executor.test | 9 | 0.001361s | 0.001380s | 0.001346s | byte equality observed |
| bind_immediate_executor_select.test | 10 | 0.001374s | 0.001381s | 0.001414s | byte equality observed |
| buffer.test | 10 | 0.001378s | 0.001351s | 0.001320s | byte equality observed |
| buffer_select.test | 10 | 0.001330s | 0.001333s | 0.001304s | byte equality observed |
| buffered_read_stream.test | 9 | 0.001722s | 0.001720s | 0.001744s | byte equality observed |
| buffered_read_stream_select.test | 10 | 0.001694s | 0.001650s | 0.001706s | byte equality observed |
| buffered_stream.test | 7 | 0.001695s | 0.001672s | 0.001722s | byte equality observed |
| buffered_stream_select.test | 10 | 0.001703s | 0.001714s | 0.001694s | byte equality observed |
| buffered_write_stream.test | 10 | 0.001771s | 0.001715s | 0.001716s | byte equality observed |
| buffered_write_stream_select.test | 10 | 0.001697s | 0.001717s | 0.001717s | byte equality observed |
| buffers_iterator.test | 10 | 0.001337s | 0.001282s | 0.001289s | byte equality observed |
| buffers_iterator_select.test | 10 | 0.001217s | 0.001207s | 0.001202s | byte equality observed |
| cancel_after.test | 1 | 0.004060s | 0.003876s | 0.004066s | byte equality observed |
| cancel_after_select.test | 1 | 0.003758s | 0.003572s | 0.003523s | byte equality observed |
| cancel_at.test | 1 | 0.004245s | 0.004235s | 0.004145s | byte equality observed |
| cancel_at_select.test | 1 | 0.003883s | 0.003940s | 0.004083s | byte equality observed |
| co_composed.test | 10 | 0.001264s | 0.001256s | 0.001310s | byte equality observed |
| co_composed_select.test | 10 | 0.001220s | 0.001218s | 0.001220s | byte equality observed |
| compose.test | 1 | 0.002142s | 0.002135s | 0.002092s | byte equality observed |
| compose_select.test | 1 | 0.001851s | 0.001826s | 0.001729s | byte equality observed |
| composed.test | 1 | 0.002440s | 0.002385s | 0.002364s | byte equality observed |
| composed_select.test | 1 | 0.002167s | 0.002061s | 0.002158s | byte equality observed |
| config.test | 10 | 0.001366s | 0.001345s | 0.001346s | byte equality observed |
| config_select.test | 10 | 0.001410s | 0.001382s | 0.001372s | byte equality observed |
| connect_pipe.test | 10 | 0.001456s | 0.001422s | 0.001411s | byte equality observed |
| connect_pipe_select.test | 10 | 0.001408s | 0.001451s | 0.001416s | byte equality observed |
| consign.test | 10 | 0.001439s | 0.001439s | 0.001391s | byte equality observed |
| consign_select.test | 10 | 0.001310s | 0.001300s | 0.001291s | byte equality observed |
| deadline_timer.test | 1 | 0.002993s | 0.002994s | 0.003040s | byte equality observed |
| deadline_timer_select.test | 1 | 0.002717s | 0.002775s | 0.002730s | byte equality observed |
| deferred.test | 10 | 0.001247s | 0.001246s | 0.001253s | byte equality observed |
| deferred_select.test | 10 | 0.001200s | 0.001196s | 0.001201s | byte equality observed |
| disposition.test | 10 | 0.001275s | 0.001275s | 0.001273s | byte equality observed |
| disposition_select.test | 10 | 0.001274s | 0.001279s | 0.001269s | byte equality observed |
| error.test | 10 | 0.001235s | 0.001244s | 0.001226s | byte equality observed |
| error_select.test | 10 | 0.001243s | 0.001254s | 0.001246s | byte equality observed |
| inline_executor.test | 10 | 0.001262s | 0.001245s | 0.001260s | byte equality observed |
| inline_executor_select.test | 10 | 0.001256s | 0.001239s | 0.001255s | byte equality observed |
| inline_or_executor.test | 10 | 0.001274s | 0.001278s | 0.001245s | byte equality observed |
| inline_or_executor_select.test | 10 | 0.001263s | 0.001254s | 0.001242s | byte equality observed |
| io_context.test | 1 | 0.002374s | 0.002374s | 0.002401s | byte equality observed |
| io_context_select.test | 1 | 0.002729s | 0.002735s | 0.002723s | byte equality observed |
| io_context_strand.test | 1 | 0.002738s | 0.002763s | 0.002685s | byte equality observed |
| io_context_strand_select.test | 1 | 0.002453s | 0.002380s | 0.002452s | byte equality observed |
| ip_host_name.test | 10 | 0.001259s | 0.001262s | 0.001252s | byte equality observed |
| ip_host_name_select.test | 10 | 0.001207s | 0.001222s | 0.001200s | byte equality observed |
| ip_icmp.test | 10 | 0.001258s | 0.001259s | 0.001215s | byte equality observed |
| ip_icmp_select.test | 10 | 0.001260s | 0.001264s | 0.001213s | byte equality observed |
| ip_multicast.test | 10 | 0.001343s | 0.001339s | 0.001344s | byte equality observed |
| ip_multicast_select.test | 10 | 0.001326s | 0.001325s | 0.001319s | byte equality observed |
| ip_tcp.test | 10 | 0.001874s | 0.001840s | 0.001862s | byte equality observed |
| ip_tcp_select.test | 10 | 0.001797s | 0.001891s | 0.001764s | byte equality observed |
| ip_udp.test | 10 | 0.001377s | 0.001488s | 0.001353s | byte equality observed |
| ip_udp_select.test | 10 | 0.001370s | 0.001485s | 0.001330s | byte equality observed |
| ip_unicast.test | 10 | 0.001325s | 0.001451s | 0.001288s | byte equality observed |
| ip_unicast_select.test | 10 | 0.001305s | 0.001455s | 0.001272s | byte equality observed |
| ip_v6_only.test | 10 | 0.001319s | 0.001492s | 0.001279s | byte equality observed |
| ip_v6_only_select.test | 10 | 0.001309s | 0.001413s | 0.001250s | byte equality observed |
| is_read_buffered.test | 10 | 0.001220s | 0.001364s | 0.001357s | byte equality observed |
| is_read_buffered_select.test | 10 | 0.001216s | 0.001359s | 0.001323s | byte equality observed |
| is_write_buffered.test | 10 | 0.001213s | 0.001371s | 0.001340s | byte equality observed |
| is_write_buffered_select.test | 10 | 0.001220s | 0.001278s | 0.001343s | byte equality observed |
| prepend.test | 10 | 0.001360s | 0.001387s | 0.001422s | byte equality observed |
| prepend_select.test | 10 | 0.001345s | 0.001405s | 0.001362s | byte equality observed |
| read.test | 10 | 0.001981s | 0.001836s | 0.001809s | byte equality observed |
| read_at.test | 9 | 0.002445s | 0.002374s | 0.002297s | byte equality observed |
| read_at_select.test | 8 | 0.002412s | 0.002296s | 0.002268s | byte equality observed |
| read_select.test | 10 | 0.001971s | 0.001863s | 0.001775s | byte equality observed |
| read_until.test | 10 | 0.001537s | 0.001522s | 0.001538s | byte equality observed |
| read_until_select.test | 10 | 0.001556s | 0.001540s | 0.001516s | byte equality observed |
| recycling_allocator.test | 10 | 0.001326s | 0.001269s | 0.001343s | byte equality observed |
| recycling_allocator_select.test | 10 | 0.001396s | 0.001347s | 0.001364s | byte equality observed |
| signal_set.test | 10 | 0.001373s | 0.001318s | 0.001328s | byte equality observed |
| signal_set_select.test | 10 | 0.001399s | 0.001328s | 0.001367s | byte equality observed |
| socket_base.test | 10 | 0.001491s | 0.001484s | 0.001452s | byte equality observed |
| socket_base_select.test | 10 | 0.001468s | 0.001446s | 0.001490s | byte equality observed |
| spawn.test | 10 | 0.001622s | 0.001551s | 0.001597s | byte equality observed |
| spawn_select.test | 10 | 0.001575s | 0.001536s | 0.001604s | byte equality observed |
| static_thread_pool.test | 10 | 0.001339s | 0.001297s | 0.001278s | byte equality observed |
| static_thread_pool_select.test | 10 | 0.001242s | 0.001269s | 0.001279s | byte equality observed |
| strand.test | 1 | 0.002523s | 0.002613s | 0.002455s | byte equality observed |
| strand_select.test | 1 | 0.002102s | 0.002102s | 0.002066s | byte equality observed |
| streambuf.test | 10 | 0.001325s | 0.001334s | 0.001372s | byte equality observed |
| streambuf_select.test | 10 | 0.001349s | 0.001369s | 0.001362s | byte equality observed |
| thread_pool.test | 8 | 0.003111s | 0.003003s | 0.003054s | byte equality observed |
| thread_pool_select.test | 8 | 0.003018s | 0.002925s | 0.003035s | byte equality observed |
| write.test | 9 | 0.002028s | 0.001923s | 0.001902s | byte equality observed |
| write_at.test | 9 | 0.002216s | 0.002154s | 0.002002s | byte equality observed |
| write_at_select.test | 9 | 0.002274s | 0.002045s | 0.001994s | byte equality observed |
| write_select.test | 10 | 0.002032s | 0.001955s | 0.001831s | byte equality observed |

Compiler command and timing observations: [cppgm](https://github.com/vishvananda/cppgm-extended/actions/runs/42692924255) · [gcc](https://github.com/vishvananda/cppgm-extended/actions/runs/42692924255) · [clang](https://github.com/vishvananda/cppgm-extended/actions/runs/42692924255).

[Reviewed outcomes and per-test records](../results.json) · [Host and toolchains](../metadata.json)
