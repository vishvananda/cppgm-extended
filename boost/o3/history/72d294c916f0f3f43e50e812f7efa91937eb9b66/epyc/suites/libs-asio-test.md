# libs/asio/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| any_completion_executor.test | RUN | 0.863517 | 5.145397 | 5.247891 | 0.002795 | 0.002786 | 0.002833 |
| any_completion_executor_select.test | RUN | 0.872095 | 5.128281 | 5.744406 | 0.002552 | 0.002418 | 0.002785 |
| any_completion_handler.test | RUN | 0.944894 | 4.494668 | 4.808183 | 0.002112 | 0.002256 | 0.002420 |
| any_completion_handler_select.test | RUN | 0.971654 | 4.650190 | 5.017152 | 0.001982 | 0.002804 | 0.002383 |
| any_io_executor.test | RUN | 0.885107 | 5.438604 | 5.541112 | 0.002822 | 0.002687 | 0.002910 |
| any_io_executor_select.test | RUN | 0.899453 | 5.370970 | 6.092488 | 0.002582 | 0.002928 | 0.002840 |
| append.test | RUN | 0.808752 | 4.229641 | 4.253996 | 0.002102 | 0.002366 | 0.002223 |
| append_select.test | RUN | 0.818457 | 4.351167 | 4.405104 | 0.002088 | 0.002295 | 0.002258 |
| as_tuple.test | RUN | 1.029388 | 5.055806 | 5.875990 | 0.002274 | 0.002553 | 0.002264 |
| as_tuple_select.test | RUN | 1.045183 | 5.079863 | 5.872565 | 0.002302 | 0.001906 | 0.002267 |
| associated_allocator.test | LINK | 0.222845 | 0.496511 | 0.724916 | — | — | — |
| associated_allocator_select.test | LINK | 0.228818 | 0.482603 | 0.748755 | — | — | — |
| associated_cancellation_slot.test | LINK | 0.238311 | 0.530697 | 0.775941 | — | — | — |
| associated_cancellation_slot_select.test | LINK | 0.241880 | 0.541583 | 0.806975 | — | — | — |
| associated_executor.test | LINK | 0.329514 | 0.879665 | 1.261281 | — | — | — |
| associated_executor_select.test | LINK | 0.341448 | 0.901519 | 1.282705 | — | — | — |
| associated_immediate_executor.test | LINK | 0.318722 | 0.803450 | 1.084875 | — | — | — |
| associated_immediate_executor_select.test | LINK | 0.322552 | 0.816551 | 1.066715 | — | — | — |
| associator.test | LINK | 0.175597 | 0.386475 | 0.562956 | — | — | — |
| associator_select.test | LINK | 0.176605 | 0.389463 | 0.557701 | — | — | — |
| awaitable.test | LINK | 0.182157 | 0.383986 | 0.578172 | — | — | — |
| awaitable_select.test | LINK | 0.183699 | 0.384527 | 0.586660 | — | — | — |
| basic_datagram_socket.test | LINK | 0.546143 | 1.198924 | 2.288760 | — | — | — |
| basic_datagram_socket_select.test | LINK | 0.558559 | 1.257680 | 2.319162 | — | — | — |
| basic_deadline_timer.test | LINK | 0.648623 | 1.455650 | 2.630825 | — | — | — |
| basic_deadline_timer_select.test | LINK | 0.664369 | 1.471134 | 2.632495 | — | — | — |
| basic_file.test | LINK | 0.179430 | 0.403672 | 0.577388 | — | — | — |
| basic_file_select.test | LINK | 0.183882 | 0.377267 | 0.607734 | — | — | — |
| basic_random_access_file.test | LINK | 0.181087 | 0.378265 | 0.549681 | — | — | — |
| basic_random_access_file_select.test | LINK | 0.177480 | 0.381408 | 0.596036 | — | — | — |
| basic_raw_socket.test | LINK | 0.552723 | 1.222719 | 2.335471 | — | — | — |
| basic_raw_socket_select.test | LINK | 0.557016 | 1.279274 | 2.259107 | — | — | — |
| basic_readable_pipe.test | LINK | 0.535685 | 1.161963 | 1.924623 | — | — | — |
| basic_readable_pipe_select.test | LINK | 0.551960 | 1.222820 | 2.462574 | — | — | — |
| basic_seq_packet_socket.test | LINK | 0.554370 | 1.208219 | 2.321073 | — | — | — |
| basic_seq_packet_socket_select.test | LINK | 0.556037 | 1.250931 | 2.312818 | — | — | — |
| basic_signal_set.test | LINK | 0.518282 | 1.128837 | 1.710644 | — | — | — |
| basic_signal_set_select.test | LINK | 0.520431 | 1.205940 | 2.261862 | — | — | — |
| basic_socket_acceptor.test | LINK | 0.546608 | 1.231168 | 2.329140 | — | — | — |
| basic_socket_acceptor_select.test | LINK | 0.567040 | 1.277586 | 2.305063 | — | — | — |
| basic_stream_file.test | LINK | 0.179484 | 0.388057 | 0.610988 | — | — | — |
| basic_stream_file_select.test | LINK | 0.182572 | 0.398007 | 0.590891 | — | — | — |
| basic_stream_socket.test | LINK | 0.544824 | 1.224968 | 2.188424 | — | — | — |
| basic_stream_socket_select.test | LINK | 0.559279 | 1.268137 | 2.346707 | — | — | — |
| basic_streambuf.test | LINK | 0.271821 | 0.559535 | 0.913091 | — | — | — |
| basic_streambuf_select.test | LINK | 0.277280 | 0.544773 | 0.851074 | — | — | — |
| basic_waitable_timer.test | LINK | 0.503244 | 1.150259 | 2.155924 | — | — | — |
| basic_waitable_timer_select.test | LINK | 0.518940 | 1.160730 | 2.231495 | — | — | — |
| basic_writable_pipe.test | LINK | 0.539647 | 1.168174 | 1.920825 | — | — | — |
| basic_writable_pipe_select.test | LINK | 0.559890 | 1.249885 | 2.454286 | — | — | — |
| bind_allocator.test | RUN | 0.857499 | 4.372404 | 4.668323 | 0.002436 | 0.002632 | 0.002578 |
| bind_allocator_select.test | RUN | 0.861358 | 4.381414 | 4.684879 | 0.002142 | 0.002051 | 0.001761 |
| bind_cancellation_slot.test | RUN | 0.857525 | 4.311727 | 4.819285 | 0.001734 | 0.001764 | 0.001640 |
| bind_cancellation_slot_select.test | RUN | 0.867445 | 4.358586 | 4.372550 | 0.001439 | 0.001604 | 0.001501 |
| bind_executor.test | RUN | 0.555052 | 2.910377 | 2.889293 | 0.002350 | 0.002937 | 0.002872 |
| bind_executor_select.test | RUN | 0.556932 | 3.128763 | 3.162018 | 0.002173 | 0.002887 | 0.002754 |
| bind_immediate_executor.test | RUN | 0.413346 | 2.623280 | 2.004253 | 0.001598 | 0.001530 | 0.001854 |
| bind_immediate_executor_select.test | RUN | 0.426523 | 2.459420 | 2.326188 | 0.001630 | 0.001538 | 0.001446 |
| buffer.test | RUN | 0.253739 | 1.018359 | 1.141914 | 0.001516 | 0.001632 | 0.001442 |
| buffer_select.test | RUN | 0.249541 | 1.024940 | 1.121813 | 0.001784 | 0.001542 | 0.001424 |
| buffer_registration.test | LINK | 0.204859 | 0.525798 | 0.665931 | — | — | — |
| buffer_registration_select.test | LINK | 0.212598 | 0.525672 | 0.647792 | — | — | — |
| buffered_read_stream.test | RUN | 0.885370 | 6.725803 | 6.928341 | 0.001932 | 0.002604 | 0.002728 |
| buffered_read_stream_select.test | RUN | 0.894491 | 6.943860 | 7.136171 | 0.001874 | 0.002498 | 0.002658 |
| buffered_stream.test | RUN | 1.574983 | 10.031986 | 11.224647 | 0.002602 | 0.002729 | 0.002722 |
| buffered_stream_select.test | RUN | 1.686511 | 10.064070 | 10.941644 | 0.002376 | 0.002663 | 0.002711 |
| buffered_write_stream.test | RUN | 1.580096 | 8.867460 | 9.606539 | 0.002568 | 0.002771 | 0.002830 |
| buffered_write_stream_select.test | RUN | 1.557316 | 8.952880 | 9.281296 | 0.002317 | 0.002632 | 0.002784 |
| buffers_iterator.test | RUN | 0.485927 | 0.901591 | 1.660568 | 0.002019 | 0.002088 | 0.001963 |
| buffers_iterator_select.test | RUN | 0.488446 | 0.892892 | 1.657728 | 0.002146 | 0.002146 | 0.002108 |
| cancel_after.test | RUN | 1.327909 | 6.964593 | 7.176515 | 0.004328 | 0.004333 | 0.004816 |
| cancel_after_select.test | RUN | 1.352975 | 6.829838 | 7.103905 | 0.003607 | 0.003947 | 0.004004 |
| cancel_at.test | RUN | 1.307728 | 5.753804 | 6.280473 | 0.003706 | 0.004400 | 0.003928 |
| cancel_at_select.test | RUN | 1.204305 | 5.599467 | 6.457015 | 0.003798 | 0.003816 | 0.003636 |
| cancellation_signal.test | LINK | 0.146546 | 0.306222 | 0.445765 | — | — | — |
| cancellation_signal_select.test | LINK | 0.152386 | 0.311309 | 0.427015 | — | — | — |
| cancellation_state.test | LINK | 0.222021 | 0.309656 | 0.428074 | — | — | — |
| cancellation_state_select.test | LINK | 0.221828 | 0.349686 | 0.455922 | — | — | — |
| cancellation_type.test | LINK | 0.183680 | 0.250659 | 0.372182 | — | — | — |
| cancellation_type_select.test | LINK | 0.182924 | 0.247971 | 0.541191 | — | — | — |
| co_composed.test | RUN | 0.179788 | 0.249132 | 0.573659 | 0.001988 | 0.001466 | 0.001916 |
| co_composed_select.test | RUN | 0.178018 | 0.245447 | 0.590127 | 0.001942 | 0.001465 | 0.001784 |
| co_spawn.test | LINK | 0.185085 | 0.246886 | 0.549696 | — | — | — |
| co_spawn_select.test | LINK | 0.183363 | 0.247663 | 0.562659 | — | — | — |
| completion_condition.test | LINK | 0.228592 | 0.389043 | 0.797397 | — | — | — |
| completion_condition_select.test | LINK | 0.229125 | 0.585128 | 0.755269 | — | — | — |
| compose.test | RUN | 1.091715 | 5.419750 | 5.970024 | 0.002017 | 0.002574 | 0.002445 |
| compose_select.test | RUN | 1.090309 | 5.330816 | 5.955208 | 0.003010 | 0.001908 | 0.002212 |
| composed.test | RUN | 0.994205 | 5.385675 | 5.747247 | 0.002793 | 0.002136 | 0.002074 |
| composed_select.test | RUN | 1.001796 | 5.175818 | 5.620110 | 0.002502 | 0.002042 | 0.001906 |
| config.test | RUN | 0.470879 | 3.377254 | 2.433263 | 0.001972 | 0.002188 | 0.002245 |
| config_select.test | RUN | 0.480441 | 3.683079 | 3.329420 | 0.002073 | 0.002060 | 0.002178 |
| connect.test | LINK | 1.659956 | 11.367216 | 11.832029 | — | — | — |
| connect_select.test | LINK | 1.675750 | 11.413339 | 11.823188 | — | — | — |
| connect_pipe.test | RUN | 0.918597 | 4.250907 | 4.699869 | 0.002232 | 0.002194 | 0.002315 |
| connect_pipe_select.test | RUN | 0.950025 | 4.477041 | 5.078671 | 0.002041 | 0.002215 | 0.002210 |
| consign.test | RUN | 0.805826 | 4.153321 | 4.386203 | 0.002131 | 0.001991 | 0.002228 |
| consign_select.test | RUN | 0.813852 | 4.263469 | 4.383413 | 0.001792 | 0.002163 | 0.002202 |
| coroutine.test | LINK | 0.185333 | 0.445487 | 0.620268 | — | — | — |
| coroutine_select.test | LINK | 0.188475 | 0.460082 | 0.659551 | — | — | — |
| deadline_timer.test | RUN | 1.139402 | 5.492993 | 5.994053 | 0.003550 | 0.003860 | 0.003814 |
| deadline_timer_select.test | RUN | 1.150247 | 5.446644 | 5.913857 | 0.003325 | 0.002831 | 0.003342 |
| deferred.test | RUN | 0.196285 | 0.412306 | 0.612318 | 0.002440 | 0.002250 | 0.002027 |
| deferred_select.test | RUN | 0.195943 | 0.417428 | 0.663388 | 0.002004 | 0.002230 | 0.002108 |
| detached.test | LINK | 0.219069 | 0.469015 | 0.742918 | — | — | — |
| detached_select.test | LINK | 0.224441 | 0.489559 | 0.739815 | — | — | — |
| disposition.test | RUN | 0.191394 | 1.096791 | 1.247100 | 0.001557 | 0.002122 | 0.002203 |
| disposition_select.test | RUN | 0.196275 | 1.065221 | 1.283303 | 0.001454 | 0.002197 | 0.002199 |
| error.test | RUN | 0.174774 | 1.045651 | 1.140487 | 0.001475 | 0.002003 | 0.002112 |
| error_select.test | RUN | 0.176146 | 1.035836 | 1.092039 | 0.001424 | 0.002074 | 0.002052 |
| file_base.test | LINK | 0.222030 | 0.872483 | 1.100630 | — | — | — |
| file_base_select.test | LINK | 0.225292 | 0.847763 | 0.866796 | — | — | — |
| fwd_first.test | LINK | 0.506243 | 1.256107 | 2.056031 | — | — | — |
| fwd_last.test | LINK | 0.508551 | 1.201029 | 2.044764 | — | — | — |
| generic_basic_endpoint.test | LINK | 0.195043 | 0.488601 | 0.735637 | — | — | — |
| generic_basic_endpoint_select.test | LINK | 0.196465 | 0.489745 | 0.743828 | — | — | — |
| generic_datagram_protocol.test | LINK | 0.783080 | 4.056170 | 3.804525 | — | — | — |
| generic_datagram_protocol_select.test | LINK | 0.783271 | 4.218305 | 3.849098 | — | — | — |
| generic_raw_protocol.test | LINK | 0.782153 | 4.068172 | 3.808336 | — | — | — |
| generic_raw_protocol_select.test | LINK | 0.774677 | 4.233286 | 3.809382 | — | — | — |
| generic_seq_packet_protocol.test | LINK | 0.674979 | 3.454746 | 3.300033 | — | — | — |
| generic_seq_packet_protocol_select.test | LINK | 0.690812 | 3.531249 | 3.293103 | — | — | — |
| generic_stream_protocol.test | LINK | 0.726116 | 3.737951 | 3.486172 | — | — | — |
| generic_stream_protocol_select.test | LINK | 0.717790 | 3.849767 | 3.557233 | — | — | — |
| high_resolution_timer.test | LINK | 0.319664 | 0.730848 | 1.165628 | — | — | — |
| high_resolution_timer_select.test | LINK | 0.329530 | 0.755995 | 1.209271 | — | — | — |
| immediate.test | LINK | 0.225612 | 0.556157 | 0.696648 | — | — | — |
| immediate_select.test | LINK | 0.224571 | 0.549539 | 0.737487 | — | — | — |
| inline_executor.test | RUN | 0.276514 | 0.852122 | 1.064208 | 0.001579 | 0.001586 | 0.001484 |
| inline_executor_select.test | RUN | 0.274725 | 0.854625 | 1.083867 | 0.001483 | 0.001627 | 0.001524 |
| inline_or_executor.test | RUN | 0.435058 | 2.461418 | 1.940927 | 0.001846 | 0.001627 | 0.001505 |
| inline_or_executor_select.test | RUN | 0.453824 | 2.505923 | 2.226853 | 0.001582 | 0.001559 | 0.001445 |
| io_context.test | RUN | 0.641042 | 3.697410 | 3.409869 | 0.002507 | 0.002703 | 0.002583 |
| io_context_select.test | RUN | 0.648987 | 3.659954 | 3.444255 | 0.002652 | 0.002977 | 0.002543 |
| io_context_strand.test | RUN | 0.608134 | 4.548125 | 3.942237 | 0.002610 | 0.002452 | 0.002928 |
| io_context_strand_select.test | RUN | 0.619876 | 4.264364 | 3.676814 | 0.002970 | 0.002781 | 0.002580 |
| ip_address.test | LINK | 0.227327 | 1.204364 | 1.718342 | — | — | — |
| ip_address_select.test | LINK | 0.231153 | 1.201197 | 1.632794 | — | — | — |
| ip_address_v4.test | LINK | 0.233346 | 1.291579 | 1.318104 | — | — | — |
| ip_address_v4_select.test | LINK | 0.233111 | 1.275152 | 0.985225 | — | — | — |
| ip_address_v6.test | LINK | 0.260197 | 1.518396 | 1.233460 | — | — | — |
| ip_address_v6_select.test | LINK | 0.260423 | 1.287944 | 1.202791 | — | — | — |
| ip_basic_endpoint.test | LINK | 0.200604 | 0.516714 | 0.802149 | — | — | — |
| ip_basic_endpoint_select.test | LINK | 0.203340 | 0.523236 | 0.791658 | — | — | — |
| ip_basic_resolver.test | LINK | 0.324848 | 0.750722 | 1.195923 | — | — | — |
| ip_basic_resolver_select.test | LINK | 0.333988 | 0.764864 | 1.210134 | — | — | — |
| ip_basic_resolver_entry.test | LINK | 0.116705 | 0.250275 | 0.345488 | — | — | — |
| ip_basic_resolver_entry_select.test | LINK | 0.114256 | 0.245480 | 0.336972 | — | — | — |
| ip_basic_resolver_iterator.test | LINK | 0.204426 | 0.499406 | 0.789056 | — | — | — |
| ip_basic_resolver_iterator_select.test | LINK | 0.205583 | 0.499464 | 0.785355 | — | — | — |
| ip_basic_resolver_query.test | LINK | 0.197134 | 0.482063 | 0.728668 | — | — | — |
| ip_basic_resolver_query_select.test | LINK | 0.192128 | 0.488498 | 0.739645 | — | — | — |
| ip_host_name.test | RUN | 0.207974 | 0.641337 | 0.847807 | 0.001505 | 0.001495 | 0.001596 |
| ip_host_name_select.test | RUN | 0.211322 | 0.640258 | 1.098938 | 0.001492 | 0.001663 | 0.002074 |
| ip_icmp.test | RUN | 0.957046 | 7.271291 | 8.526415 | 0.001487 | 0.002058 | 0.002034 |
| ip_icmp_select.test | RUN | 0.949270 | 7.592735 | 8.112853 | 0.001642 | 0.001936 | 0.002104 |
| ip_multicast.test | RUN | 0.576489 | 4.495275 | 5.052484 | 0.002166 | 0.002172 | 0.002282 |
| ip_multicast_select.test | RUN | 0.591368 | 4.482514 | 5.059610 | 0.001681 | 0.002032 | 0.002096 |
| ip_resolver_query_base.test | LINK | 0.138273 | 0.424972 | 0.610261 | — | — | — |
| ip_resolver_query_base_select.test | LINK | 0.134161 | 0.413004 | 0.605973 | — | — | — |
| ip_tcp.test | RUN | 1.412143 | 12.947771 | 12.934634 | 0.002251 | 0.003276 | 0.002860 |
| ip_tcp_select.test | RUN | 1.407117 | 13.490975 | 12.414817 | 0.002138 | 0.002836 | 0.002976 |
| ip_udp.test | RUN | 1.012753 | 9.282627 | 8.955859 | 0.001648 | 0.002334 | 0.002319 |
| ip_udp_select.test | RUN | 1.011500 | 9.499583 | 8.650723 | 0.001672 | 0.002239 | 0.002132 |
| ip_unicast.test | RUN | 0.522650 | 4.056307 | 4.394317 | 0.002094 | 0.002136 | 0.002326 |
| ip_unicast_select.test | RUN | 0.543798 | 4.177046 | 4.385039 | 0.002171 | 0.002283 | 0.002454 |
| ip_v6_only.test | RUN | 0.853794 | 4.285837 | 4.550121 | 0.002156 | 0.002421 | 0.002666 |
| ip_v6_only_select.test | RUN | 0.864561 | 4.450254 | 4.493569 | 0.002004 | 0.002332 | 0.002079 |
| is_read_buffered.test | RUN | 0.607323 | 1.417006 | 2.569833 | 0.001890 | 0.002175 | 0.002040 |
| is_read_buffered_select.test | RUN | 0.629685 | 1.462116 | 2.646640 | 0.001742 | 0.001996 | 0.001965 |
| is_write_buffered.test | RUN | 0.601300 | 1.427510 | 2.562639 | 0.002040 | 0.001923 | 0.002132 |
| is_write_buffered_select.test | RUN | 0.613166 | 1.413539 | 2.586429 | 0.002011 | 0.001824 | 0.001784 |
| local_basic_endpoint.test | LINK | 0.301963 | 0.746711 | 1.386570 | — | — | — |
| local_basic_endpoint_select.test | LINK | 0.314719 | 0.753604 | 1.401729 | — | — | — |
| local_connect_pair.test | LINK | 0.800759 | 4.161763 | 4.195085 | — | — | — |
| local_connect_pair_select.test | LINK | 0.821438 | 4.159709 | 4.299494 | — | — | — |
| local_datagram_protocol.test | LINK | 1.054934 | 5.440896 | 5.819695 | — | — | — |
| local_datagram_protocol_select.test | LINK | 1.095132 | 5.520226 | 5.710862 | — | — | — |
| local_seq_packet_protocol.test | LINK | 1.064660 | 5.252864 | 5.728359 | — | — | — |
| local_seq_packet_protocol_select.test | LINK | 1.073493 | 5.339390 | 5.712823 | — | — | — |
| local_stream_protocol.test | LINK | 0.997056 | 5.127008 | 5.549508 | — | — | — |
| local_stream_protocol_select.test | LINK | 1.034956 | 5.151580 | 5.431079 | — | — | — |
| placeholders.test | LINK | 0.198397 | 0.417862 | 0.664686 | — | — | — |
| placeholders_select.test | LINK | 0.191962 | 0.428071 | 0.622749 | — | — | — |
| posix_basic_descriptor.test | LINK | 0.522141 | 1.168272 | 1.872801 | — | — | — |
| posix_basic_descriptor_select.test | LINK | 0.555095 | 1.243361 | 2.434629 | — | — | — |
| posix_basic_stream_descriptor.test | LINK | 0.535740 | 1.161305 | 1.958832 | — | — | — |
| posix_basic_stream_descriptor_select.test | LINK | 0.547658 | 1.232579 | 2.412618 | — | — | — |
| posix_descriptor_base.test | LINK | 0.219874 | 0.455085 | 0.657700 | — | — | — |
| posix_descriptor_base_select.test | LINK | 0.227643 | 0.456703 | 0.636196 | — | — | — |
| posix_stream_descriptor.test | LINK | 1.028144 | 5.104102 | 5.338802 | — | — | — |
| posix_stream_descriptor_select.test | LINK | 1.042171 | 5.203095 | 5.586085 | — | — | — |
| prepend.test | RUN | 0.805720 | 4.287596 | 4.306010 | 0.001748 | 0.002408 | 0.001838 |
| prepend_select.test | RUN | 0.802701 | 4.364571 | 4.302057 | 0.001836 | 0.002054 | 0.001753 |
| random_access_file.test | LINK | 0.486859 | 1.028543 | 1.551466 | — | — | — |
| random_access_file_select.test | LINK | 0.499732 | 1.112051 | 2.030125 | — | — | — |
| read.test | RUN | 2.486795 | 25.703844 | 23.568773 | 0.002940 | 0.002955 | 0.002587 |
| read_select.test | RUN | 2.529814 | 25.685085 | 23.886436 | 0.002744 | 0.002908 | 0.002610 |
| read_at.test | RUN | 2.513836 | 34.938872 | 31.783207 | 0.003666 | 0.003478 | 0.003414 |
| read_at_select.test | RUN | 2.535619 | 35.023931 | 32.147757 | 0.003577 | 0.003576 | 0.003625 |
| read_until.test | RUN | 1.322949 | 9.867171 | 9.058092 | 0.002403 | 0.002244 | 0.002320 |
| read_until_select.test | RUN | 1.319843 | 9.980448 | 9.334279 | 0.002140 | 0.002330 | 0.002124 |
| readable_pipe.test | LINK | 0.817954 | 4.300774 | 4.173060 | — | — | — |
| readable_pipe_select.test | LINK | 0.840463 | 4.537789 | 4.667189 | — | — | — |
| recycling_allocator.test | RUN | 0.257809 | 0.684696 | 0.940709 | 0.002024 | 0.002060 | 0.002090 |
| recycling_allocator_select.test | RUN | 0.261364 | 0.671676 | 0.952005 | 0.001882 | 0.001860 | 0.002089 |
| redirect_disposition.test | LINK | 1.082011 | 6.060107 | 6.470608 | — | — | — |
| redirect_disposition_select.test | LINK | 1.089762 | 6.050070 | 6.653309 | — | — | — |
| redirect_error.test | LINK | 0.973640 | 4.978651 | 5.685450 | — | — | — |
| redirect_error_select.test | LINK | 0.973935 | 4.950702 | 5.617825 | — | — | — |
| registered_buffer.test | LINK | 0.255954 | 0.551984 | 0.841226 | — | — | — |
| registered_buffer_select.test | LINK | 0.259655 | 0.557320 | 0.852990 | — | — | — |
| signal_set.test | RUN | 0.819508 | 4.194575 | 3.862892 | 0.002023 | 0.001861 | 0.002126 |
| signal_set_select.test | RUN | 0.844650 | 4.359284 | 4.426815 | 0.002314 | 0.002174 | 0.002086 |
| signal_set_base.test | LINK | 0.215425 | 0.456741 | 0.645796 | — | — | — |
| signal_set_base_select.test | LINK | 0.239127 | 0.453980 | 0.648642 | — | — | — |
| socket_base.test | RUN | 1.008870 | 6.303286 | 6.204688 | 0.002188 | 0.001924 | 0.002360 |
| socket_base_select.test | RUN | 1.014807 | 6.154371 | 6.148116 | 0.001703 | 0.002267 | 0.001904 |
| spawn.test | RUN | 1.654676 | 9.768768 | 10.182965 | 0.002373 | 0.002450 | 0.002551 |
| spawn_select.test | RUN | 1.655104 | 9.738185 | 10.078615 | 0.002295 | 0.002423 | 0.002496 |
| static_thread_pool.test | RUN | 0.469121 | 1.009852 | 1.521145 | 0.002035 | 0.001942 | 0.002114 |
| static_thread_pool_select.test | RUN | 0.489257 | 1.077493 | 2.030188 | 0.002051 | 0.001837 | 0.002024 |
| steady_timer.test | LINK | 0.527651 | 1.151085 | 2.155623 | — | — | — |
| steady_timer_select.test | LINK | 0.545624 | 1.196390 | 2.131708 | — | — | — |
| strand.test | RUN | 1.063699 | 5.693970 | 6.142017 | 0.002726 | 0.003123 | 0.003182 |
| strand_select.test | RUN | 1.074272 | 5.676218 | 6.326098 | 0.002988 | 0.002470 | 0.003074 |
| stream_file.test | LINK | 0.481552 | 1.065303 | 1.565183 | — | — | — |
| stream_file_select.test | LINK | 0.498114 | 1.127085 | 1.958215 | — | — | — |
| streambuf.test | RUN | 0.279549 | 0.857810 | 1.109350 | 0.001564 | 0.002065 | 0.001793 |
| streambuf_select.test | RUN | 0.221980 | 0.842572 | 0.922639 | 0.001490 | 0.001951 | 0.001501 |
| system_timer.test | LINK | 0.627239 | 3.609167 | 4.094612 | — | — | — |
| system_timer_select.test | LINK | 0.635336 | 3.463262 | 3.329752 | — | — | — |
| system_context.test | LINK | 0.296405 | 0.648777 | 0.860906 | — | — | — |
| system_context_select.test | LINK | 0.313634 | 0.701513 | 1.046754 | — | — | — |
| system_executor.test | LINK | 0.389463 | 1.998490 | 1.596905 | — | — | — |
| system_executor_select.test | LINK | 0.401116 | 1.957954 | 1.795771 | — | — | — |
| this_coro.test | LINK | 0.115107 | 0.250228 | 0.330114 | — | — | — |
| this_coro_select.test | LINK | 0.117267 | 0.248105 | 0.340904 | — | — | — |
| thread_pool.test | RUN | 0.454988 | 3.395780 | 2.707235 | 0.003243 | 0.003447 | 0.003957 |
| thread_pool_select.test | RUN | 0.476408 | 3.454185 | 2.481432 | 0.003277 | 0.003788 | 0.004070 |
| time_traits.test | LINK | 0.256532 | 0.838660 | 1.280581 | — | — | — |
| time_traits_select.test | LINK | 0.254349 | 0.830147 | 1.188858 | — | — | — |
| ts_buffer.test | LINK | 0.241012 | 0.884684 | 1.252611 | — | — | — |
| ts_buffer_select.test | LINK | 0.246300 | 0.900364 | 1.272574 | — | — | — |
| ts_executor.test | LINK | 0.342616 | 1.243046 | 1.972520 | — | — | — |
| ts_executor_select.test | LINK | 0.361268 | 1.315290 | 2.485564 | — | — | — |
| ts_internet.test | LINK | 0.374944 | 1.375192 | 2.587532 | — | — | — |
| ts_internet_select.test | LINK | 0.383652 | 1.415962 | 2.644523 | — | — | — |
| ts_io_context.test | LINK | 0.301496 | 1.031854 | 1.524684 | — | — | — |
| ts_io_context_select.test | LINK | 0.313044 | 1.109512 | 2.047259 | — | — | — |
| ts_net.test | LINK | 0.438599 | 1.636611 | 3.055699 | — | — | — |
| ts_net_select.test | LINK | 0.464450 | 1.688203 | 3.288989 | — | — | — |
| ts_netfwd.test | LINK | 0.446914 | 1.630111 | 3.209514 | — | — | — |
| ts_netfwd_select.test | LINK | 0.451697 | 1.698486 | 3.220454 | — | — | — |
| ts_socket.test | LINK | 0.373456 | 1.277321 | 2.483754 | — | — | — |
| ts_socket_select.test | LINK | 0.386980 | 1.334267 | 2.554397 | — | — | — |
| ts_timer.test | LINK | 0.325184 | 1.150668 | 2.123007 | — | — | — |
| ts_timer_select.test | LINK | 0.324150 | 1.202904 | 2.161375 | — | — | — |
| use_awaitable.test | LINK | 0.111880 | 0.375795 | 0.523983 | — | — | — |
| use_awaitable_select.test | LINK | 0.113417 | 0.390533 | 0.610586 | — | — | — |
| wait_traits.test | LINK | 0.112200 | 0.394439 | 0.562563 | — | — | — |
| wait_traits_select.test | LINK | 0.179460 | 0.395794 | 0.551609 | — | — | — |
| windows_basic_object_handle.test | LINK | 0.179991 | 0.398485 | 0.564112 | — | — | — |
| windows_basic_object_handle_select.test | LINK | 0.177406 | 0.404774 | 0.559007 | — | — | — |
| windows_basic_overlapped_handle.test | LINK | 0.179027 | 0.399020 | 0.597192 | — | — | — |
| windows_basic_overlapped_handle_select.test | LINK | 0.182868 | 0.401580 | 0.518766 | — | — | — |
| windows_basic_random_access_handle.test | LINK | 0.178429 | 0.386919 | 0.564044 | — | — | — |
| windows_basic_random_access_handle_select.test | LINK | 0.181009 | 0.400124 | 0.556866 | — | — | — |
| windows_basic_stream_handle.test | LINK | 0.182741 | 0.401572 | 0.549147 | — | — | — |
| windows_basic_stream_handle_select.test | LINK | 0.178941 | 0.406643 | 0.551690 | — | — | — |
| windows_object_handle.test | LINK | 0.491853 | 1.053115 | 1.521439 | — | — | — |
| windows_object_handle_select.test | LINK | 0.500575 | 1.124429 | 2.019665 | — | — | — |
| windows_overlapped_ptr.test | LINK | 0.497154 | 1.103142 | 1.701669 | — | — | — |
| windows_overlapped_ptr_select.test | LINK | 0.540723 | 1.182136 | 2.227206 | — | — | — |
| windows_random_access_handle.test | LINK | 0.483131 | 1.051462 | 1.533825 | — | — | — |
| windows_random_access_handle_select.test | LINK | 0.499404 | 1.127466 | 1.968436 | — | — | — |
| windows_stream_handle.test | LINK | 0.484731 | 1.054135 | 1.559085 | — | — | — |
| windows_stream_handle_select.test | LINK | 0.502993 | 1.133293 | 2.058808 | — | — | — |
| writable_pipe.test | LINK | 0.859567 | 4.260242 | 4.430300 | — | — | — |
| writable_pipe_select.test | LINK | 0.878959 | 4.463854 | 4.805144 | — | — | — |
| write.test | RUN | 2.478385 | 21.638018 | 23.121432 | 0.003046 | 0.003039 | 0.002869 |
| write_select.test | RUN | 2.483403 | 21.623542 | 23.545270 | 0.003101 | 0.002886 | 0.002620 |
| write_at.test | RUN | 2.560873 | 29.741789 | 30.890408 | 0.003270 | 0.003070 | 0.003220 |
| write_at_select.test | RUN | 2.587820 | 29.538838 | 30.642481 | 0.003036 | 0.003175 | 0.003227 |
| any_executor.test | RUN | 1.038394 | 9.848865 | 8.925174 | 0.003054 | 0.002704 | 0.002410 |
| blocking.test | RUN | 2.764306 | 12.139588 | 19.879678 | 0.008682 | 0.007733 | 0.008888 |
| blocking_adaptation.test | RUN | 1.351597 | 5.480248 | 9.093269 | 0.004637 | 0.004934 | 0.004483 |
| context_as.test | RUN | 0.707316 | 3.911102 | 3.023001 | 0.002433 | 0.002650 | 0.002206 |
| executor.test | RUN | 0.197548 | 0.403497 | 0.552149 | 0.002088 | 0.002208 | 0.001980 |
| inline_exception_handling.test | RUN | 2.683637 | 11.524927 | 18.618083 | 0.008745 | 0.005721 | 0.005693 |
| invocable_archetype.test | RUN | 0.181007 | 0.413994 | 0.591918 | 0.001778 | 0.001998 | 0.002130 |
| mapping.test | RUN | 2.446784 | 12.015961 | 19.512043 | 0.006024 | 0.006010 | 0.006026 |
| outstanding_work.test | RUN | 1.236130 | 5.032159 | 9.050669 | 0.004558 | 0.004641 | 0.004941 |
| prefer_only.test | RUN | 0.383077 | 2.137971 | 2.074411 | 0.002020 | 0.002066 | 0.002081 |
| relationship.test | RUN | 0.784658 | 5.146764 | 8.434163 | 0.003220 | 0.003368 | 0.003322 |

## Native runtime replay

Child CPU of successful native tests, with process startup included; GDB and expected-failure executions are correctness-only. Two warmups per compiler, four A/A observations per reference and mirrored ABBA/BAAB blocks. Short tests repeat up to ten times, identically across compilers; CPU is divided by repetitions before summing tests. Executable hashes and output checks stay outside process timers. All runs must pass their original native oracle. Output hashes record equality or variation across the complete campaign; test reports may print times or addresses.

| Reference | CPU speedup ↑ | Paired blocks | A/A speedup |
|---|---:|---|---:|
| gcc | 0.988× | 0.990×, 0.987× | 0.998× |
| clang | 0.980× | 0.985×, 0.976× | 0.997× |

† A/A drift or paired-block spread exceeds 5%; small differences are unresolved. 1375 native tests in the full cohort; 1360 have byte-identical output throughout. [Raw replay evidence in CI artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43348824563).

| Test | Repeats | CPU cppgm | GCC | Clang | Output check |
|---|---:|---:|---:|---:|---|
| any_completion_executor.test | 2 | 0.002048s | 0.001984s | 0.002003s | byte equality observed |
| any_completion_executor_select.test | 5 | 0.002035s | 0.001942s | 0.002042s | byte equality observed |
| any_completion_handler.test | 6 | 0.001565s | 0.001571s | 0.001580s | byte equality observed |
| any_completion_handler_select.test | 8 | 0.001546s | 0.001544s | 0.001530s | byte equality observed |
| any_io_executor.test | 4 | 0.002018s | 0.001948s | 0.001982s | byte equality observed |
| any_io_executor_select.test | 5 | 0.002002s | 0.001955s | 0.001931s | byte equality observed |
| append.test | 8 | 0.001515s | 0.001519s | 0.001453s | byte equality observed |
| append_select.test | 6 | 0.001486s | 0.001486s | 0.001461s | byte equality observed |
| as_tuple.test | 6 | 0.001654s | 0.001624s | 0.001687s | byte equality observed |
| as_tuple_select.test | 5 | 0.001516s | 0.001499s | 0.001529s | byte equality observed |
| bind_allocator.test | 1 | 0.001954s | 0.001888s | 0.001908s | byte equality observed |
| bind_allocator_select.test | 1 | 0.001645s | 0.001635s | 0.001673s | byte equality observed |
| bind_cancellation_slot.test | 9 | 0.001382s | 0.001354s | 0.001344s | byte equality observed |
| bind_cancellation_slot_select.test | 10 | 0.001265s | 0.001229s | 0.001248s | byte equality observed |
| bind_executor.test | 1 | 0.001901s | 0.001836s | 0.001863s | byte equality observed |
| bind_executor_select.test | 1 | 0.002170s | 0.002135s | 0.002137s | byte equality observed |
| bind_immediate_executor.test | 10 | 0.001385s | 0.001355s | 0.001318s | byte equality observed |
| bind_immediate_executor_select.test | 10 | 0.001328s | 0.001325s | 0.001314s | byte equality observed |
| buffer.test | 10 | 0.001338s | 0.001267s | 0.001312s | byte equality observed |
| buffer_select.test | 10 | 0.001286s | 0.001292s | 0.001301s | byte equality observed |
| buffered_read_stream.test | 10 | 0.001658s | 0.001650s | 0.001564s | byte equality observed |
| buffered_read_stream_select.test | 10 | 0.001604s | 0.001564s | 0.001535s | byte equality observed |
| buffered_stream.test | 10 | 0.001627s | 0.001644s | 0.001595s | byte equality observed |
| buffered_stream_select.test | 6 | 0.001670s | 0.001628s | 0.001553s | byte equality observed |
| buffered_write_stream.test | 7 | 0.001640s | 0.001604s | 0.001638s | byte equality observed |
| buffered_write_stream_select.test | 10 | 0.001606s | 0.001644s | 0.001604s | byte equality observed |
| buffers_iterator.test | 10 | 0.001262s | 0.001244s | 0.001247s | byte equality observed |
| buffers_iterator_select.test | 10 | 0.001174s | 0.001200s | 0.001167s | byte equality observed |
| cancel_after.test | 1 | 0.003744s | 0.003628s | 0.003550s | byte equality observed |
| cancel_after_select.test | 1 | 0.003310s | 0.003069s | 0.003070s | byte equality observed |
| cancel_at.test | 1 | 0.003780s | 0.003786s | 0.003721s | byte equality observed |
| cancel_at_select.test | 1 | 0.003630s | 0.003623s | 0.003481s | byte equality observed |
| co_composed.test | 10 | 0.001198s | 0.001240s | 0.001245s | byte equality observed |
| co_composed_select.test | 10 | 0.001179s | 0.001193s | 0.001174s | byte equality observed |
| compose.test | 1 | 0.001978s | 0.002049s | 0.001909s | byte equality observed |
| compose_select.test | 1 | 0.001603s | 0.001611s | 0.001589s | byte equality observed |
| composed.test | 1 | 0.002192s | 0.002109s | 0.002124s | byte equality observed |
| composed_select.test | 1 | 0.001726s | 0.001699s | 0.001712s | byte equality observed |
| config.test | 10 | 0.001292s | 0.001301s | 0.001295s | byte equality observed |
| config_select.test | 10 | 0.001317s | 0.001328s | 0.001322s | byte equality observed |
| connect_pipe.test | 10 | 0.001378s | 0.001374s | 0.001392s | byte equality observed |
| connect_pipe_select.test | 10 | 0.001366s | 0.001407s | 0.001377s | byte equality observed |
| consign.test | 9 | 0.001363s | 0.001375s | 0.001340s | byte equality observed |
| consign_select.test | 10 | 0.001262s | 0.001232s | 0.001235s | byte equality observed |
| deadline_timer.test | 1 | 0.002792s | 0.002721s | 0.002749s | byte equality observed |
| deadline_timer_select.test | 1 | 0.002512s | 0.002597s | 0.002498s | byte equality observed |
| deferred.test | 9 | 0.001174s | 0.001182s | 0.001175s | byte equality observed |
| deferred_select.test | 10 | 0.001144s | 0.001128s | 0.001107s | byte equality observed |
| disposition.test | 10 | 0.001209s | 0.001217s | 0.001192s | byte equality observed |
| disposition_select.test | 10 | 0.001205s | 0.001200s | 0.001208s | byte equality observed |
| error.test | 10 | 0.001179s | 0.001175s | 0.001137s | byte equality observed |
| error_select.test | 10 | 0.001177s | 0.001166s | 0.001155s | byte equality observed |
| inline_executor.test | 10 | 0.001196s | 0.001179s | 0.001171s | byte equality observed |
| inline_executor_select.test | 10 | 0.001195s | 0.001180s | 0.001158s | byte equality observed |
| inline_or_executor.test | 10 | 0.001199s | 0.001170s | 0.001168s | byte equality observed |
| inline_or_executor_select.test | 10 | 0.001203s | 0.001203s | 0.001180s | byte equality observed |
| io_context.test | 1 | 0.002240s | 0.002227s | 0.002215s | byte equality observed |
| io_context_select.test | 1 | 0.002568s | 0.002479s | 0.002507s | byte equality observed |
| io_context_strand.test | 1 | 0.002508s | 0.002477s | 0.002515s | byte equality observed |
| io_context_strand_select.test | 1 | 0.002283s | 0.002266s | 0.002260s | byte equality observed |
| ip_host_name.test | 10 | 0.001197s | 0.001197s | 0.001191s | byte equality observed |
| ip_host_name_select.test | 10 | 0.001161s | 0.001155s | 0.001144s | byte equality observed |
| ip_icmp.test | 10 | 0.001200s | 0.001202s | 0.001179s | byte equality observed |
| ip_icmp_select.test | 10 | 0.001200s | 0.001198s | 0.001170s | byte equality observed |
| ip_multicast.test | 10 | 0.001289s | 0.001278s | 0.001260s | byte equality observed |
| ip_multicast_select.test | 10 | 0.001254s | 0.001255s | 0.001251s | byte equality observed |
| ip_tcp.test | 10 | 0.001774s | 0.001761s | 0.001742s | byte equality observed |
| ip_tcp_select.test | 10 | 0.001701s | 0.001721s | 0.001675s | byte equality observed |
| ip_udp.test | 10 | 0.001305s | 0.001387s | 0.001277s | byte equality observed |
| ip_udp_select.test | 10 | 0.001290s | 0.001450s | 0.001277s | byte equality observed |
| ip_unicast.test | 10 | 0.001243s | 0.001332s | 0.001238s | byte equality observed |
| ip_unicast_select.test | 10 | 0.001238s | 0.001342s | 0.001202s | byte equality observed |
| ip_v6_only.test | 10 | 0.001262s | 0.001437s | 0.001222s | byte equality observed |
| ip_v6_only_select.test | 10 | 0.001238s | 0.001348s | 0.001209s | byte equality observed |
| is_read_buffered.test | 10 | 0.001183s | 0.001298s | 0.001175s | byte equality observed |
| is_read_buffered_select.test | 10 | 0.001272s | 0.001295s | 0.001268s | byte equality observed |
| is_write_buffered.test | 10 | 0.001292s | 0.001315s | 0.001271s | byte equality observed |
| is_write_buffered_select.test | 10 | 0.001263s | 0.001256s | 0.001243s | byte equality observed |
| prepend.test | 10 | 0.001393s | 0.001347s | 0.001346s | byte equality observed |
| prepend_select.test | 10 | 0.001354s | 0.001332s | 0.001304s | byte equality observed |
| read.test | 10 | 0.001878s | 0.001743s | 0.001740s | byte equality observed |
| read_at.test | 9 | 0.002270s | 0.002253s | 0.002184s | byte equality observed |
| read_at_select.test | 9 | 0.002300s | 0.002272s | 0.002173s | byte equality observed |
| read_select.test | 10 | 0.001848s | 0.001758s | 0.001789s | byte equality observed |
| read_until.test | 10 | 0.001519s | 0.001522s | 0.001424s | byte equality observed |
| read_until_select.test | 10 | 0.001507s | 0.001447s | 0.001510s | byte equality observed |
| recycling_allocator.test | 10 | 0.001277s | 0.001268s | 0.001224s | byte equality observed |
| recycling_allocator_select.test | 10 | 0.001298s | 0.001220s | 0.001260s | byte equality observed |
| signal_set.test | 10 | 0.001353s | 0.001274s | 0.001324s | byte equality observed |
| signal_set_select.test | 10 | 0.001366s | 0.001285s | 0.001257s | byte equality observed |
| socket_base.test | 10 | 0.001432s | 0.001389s | 0.001408s | byte equality observed |
| socket_base_select.test | 10 | 0.001416s | 0.001364s | 0.001402s | byte equality observed |
| spawn.test | 10 | 0.001526s | 0.001461s | 0.001520s | byte equality observed |
| spawn_select.test | 10 | 0.001556s | 0.001498s | 0.001468s | byte equality observed |
| static_thread_pool.test | 10 | 0.001253s | 0.001271s | 0.001228s | byte equality observed |
| static_thread_pool_select.test | 10 | 0.001189s | 0.001219s | 0.001178s | byte equality observed |
| strand.test | 1 | 0.002348s | 0.002340s | 0.002361s | byte equality observed |
| strand_select.test | 1 | 0.001992s | 0.001897s | 0.001969s | byte equality observed |
| streambuf.test | 10 | 0.001261s | 0.001276s | 0.001274s | byte equality observed |
| streambuf_select.test | 10 | 0.001331s | 0.001329s | 0.001283s | byte equality observed |
| thread_pool.test | 8 | 0.003013s | 0.002957s | 0.002941s | byte equality observed |
| thread_pool_select.test | 8 | 0.003004s | 0.002928s | 0.002916s | byte equality observed |
| write.test | 10 | 0.001950s | 0.001875s | 0.001751s | byte equality observed |
| write_at.test | 10 | 0.002143s | 0.001992s | 0.001908s | byte equality observed |
| write_at_select.test | 10 | 0.002083s | 0.002032s | 0.001976s | byte equality observed |
| write_select.test | 10 | 0.001922s | 0.001745s | 0.001764s | byte equality observed |

[Raw outcomes, commands and timing observations in CI artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43348824563).

[Comparison records](../results.json) · [Host and toolchains](../metadata.json)
