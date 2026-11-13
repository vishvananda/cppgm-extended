# libs/asio/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| any_completion_executor.test | RUN | 0.658096 | 3.926100 | 3.340129 | 0.002183 | 0.001844 | 0.002005 |
| any_completion_executor_select.test | RUN | 0.668926 | 3.940555 | 3.503446 | 0.002182 | 0.001909 | 0.001899 |
| any_completion_handler.test | RUN | 0.717085 | 3.577848 | 3.044832 | 0.001484 | 0.001561 | 0.001430 |
| any_completion_handler_select.test | RUN | 0.725438 | 3.686101 | 3.242794 | 0.001477 | 0.001636 | 0.001326 |
| any_io_executor.test | RUN | 0.680750 | 4.194039 | 3.556536 | 0.001914 | 0.002329 | 0.002012 |
| any_io_executor_select.test | RUN | 0.689891 | 4.167369 | 3.749492 | 0.001530 | 0.001906 | 0.001823 |
| append.test | RUN | 0.626705 | 3.403515 | 2.721416 | 0.001294 | 0.001507 | 0.001368 |
| append_select.test | RUN | 0.626984 | 3.478401 | 2.691890 | 0.001502 | 0.001478 | 0.001241 |
| as_tuple.test | RUN | 0.805038 | 4.105838 | 3.796152 | 0.001702 | 0.001622 | 0.001426 |
| as_tuple_select.test | RUN | 0.814980 | 4.045671 | 3.785665 | 0.001296 | 0.001544 | 0.001358 |
| associated_allocator.test | LINK | 0.156854 | 0.332960 | 0.407784 | — | — | — |
| associated_allocator_select.test | LINK | 0.156672 | 0.337372 | 0.409024 | — | — | — |
| associated_cancellation_slot.test | LINK | 0.172260 | 0.362863 | 0.439173 | — | — | — |
| associated_cancellation_slot_select.test | LINK | 0.174877 | 0.366191 | 0.436778 | — | — | — |
| associated_executor.test | LINK | 0.247818 | 0.638645 | 0.708420 | — | — | — |
| associated_executor_select.test | LINK | 0.247452 | 0.641904 | 0.716629 | — | — | — |
| associated_immediate_executor.test | LINK | 0.228585 | 0.546436 | 0.647163 | — | — | — |
| associated_immediate_executor_select.test | LINK | 0.232432 | 0.591010 | 0.632158 | — | — | — |
| associator.test | LINK | 0.126745 | 0.283038 | 0.334317 | — | — | — |
| associator_select.test | LINK | 0.126728 | 0.286539 | 0.332412 | — | — | — |
| awaitable.test | LINK | 0.126333 | 0.278402 | 0.328691 | — | — | — |
| awaitable_select.test | LINK | 0.128559 | 0.283833 | 0.329291 | — | — | — |
| basic_datagram_socket.test | LINK | 0.401533 | 0.891026 | 1.281075 | — | — | — |
| basic_datagram_socket_select.test | LINK | 0.409144 | 0.917702 | 1.310993 | — | — | — |
| basic_deadline_timer.test | LINK | 0.471238 | 1.075891 | 1.448254 | — | — | — |
| basic_deadline_timer_select.test | LINK | 0.485675 | 1.086014 | 1.486172 | — | — | — |
| basic_file.test | LINK | 0.127013 | 0.287291 | 0.338735 | — | — | — |
| basic_file_select.test | LINK | 0.124917 | 0.279123 | 0.325442 | — | — | — |
| basic_random_access_file.test | LINK | 0.128332 | 0.280508 | 0.333346 | — | — | — |
| basic_random_access_file_select.test | LINK | 0.132645 | 0.280269 | 0.334303 | — | — | — |
| basic_raw_socket.test | LINK | 0.415712 | 0.897977 | 1.253461 | — | — | — |
| basic_raw_socket_select.test | LINK | 0.428880 | 0.918948 | 1.281326 | — | — | — |
| basic_readable_pipe.test | LINK | 0.401659 | 0.860094 | 1.085007 | — | — | — |
| basic_readable_pipe_select.test | LINK | 0.404954 | 0.907911 | 1.309341 | — | — | — |
| basic_seq_packet_socket.test | LINK | 0.399135 | 0.893653 | 1.259232 | — | — | — |
| basic_seq_packet_socket_select.test | LINK | 0.405536 | 0.912781 | 1.303705 | — | — | — |
| basic_signal_set.test | LINK | 0.365669 | 0.829766 | 0.993441 | — | — | — |
| basic_signal_set_select.test | LINK | 0.392905 | 0.881466 | 1.270743 | — | — | — |
| basic_socket_acceptor.test | LINK | 0.409649 | 0.894895 | 1.298570 | — | — | — |
| basic_socket_acceptor_select.test | LINK | 0.410400 | 0.915592 | 1.318667 | — | — | — |
| basic_stream_file.test | LINK | 0.126695 | 0.287512 | 0.342423 | — | — | — |
| basic_stream_file_select.test | LINK | 0.128926 | 0.289739 | 0.340249 | — | — | — |
| basic_stream_socket.test | LINK | 0.404116 | 0.900291 | 1.255992 | — | — | — |
| basic_stream_socket_select.test | LINK | 0.414298 | 0.909357 | 1.294294 | — | — | — |
| basic_streambuf.test | LINK | 0.192823 | 0.416599 | 0.496794 | — | — | — |
| basic_streambuf_select.test | LINK | 0.191762 | 0.411751 | 0.500139 | — | — | — |
| basic_waitable_timer.test | LINK | 0.376897 | 0.833815 | 1.181959 | — | — | — |
| basic_waitable_timer_select.test | LINK | 0.383065 | 0.862529 | 1.193736 | — | — | — |
| basic_writable_pipe.test | LINK | 0.382292 | 0.835851 | 1.092950 | — | — | — |
| basic_writable_pipe_select.test | LINK | 0.413454 | 0.900950 | 1.329729 | — | — | — |
| bind_allocator.test | RUN | 0.684034 | 3.435331 | 2.996639 | 0.001934 | 0.001768 | 0.001723 |
| bind_allocator_select.test | RUN | 0.689496 | 3.465189 | 2.928446 | 0.001990 | 0.001662 | 0.001627 |
| bind_cancellation_slot.test | RUN | 0.686730 | 3.454852 | 2.998171 | 0.001395 | 0.001602 | 0.001032 |
| bind_cancellation_slot_select.test | RUN | 0.657164 | 3.464777 | 2.909445 | 0.001183 | 0.001133 | 0.001034 |
| bind_executor.test | RUN | 0.462061 | 2.295671 | 1.836731 | 0.001886 | 0.001777 | 0.001718 |
| bind_executor_select.test | RUN | 0.470011 | 2.284297 | 1.839357 | 0.001692 | 0.001557 | 0.001511 |
| bind_immediate_executor.test | RUN | 0.336619 | 1.912312 | 1.168037 | 0.001096 | 0.001103 | 0.001023 |
| bind_immediate_executor_select.test | RUN | 0.342949 | 1.837786 | 1.296680 | 0.001086 | 0.001021 | 0.000970 |
| buffer.test | RUN | 0.197372 | 0.755876 | 0.692941 | 0.001088 | 0.001033 | 0.001022 |
| buffer_select.test | RUN | 0.193569 | 0.762359 | 0.700393 | 0.001030 | 0.001008 | 0.001013 |
| buffer_registration.test | LINK | 0.152306 | 0.385092 | 0.404024 | — | — | — |
| buffer_registration_select.test | LINK | 0.150459 | 0.379834 | 0.407735 | — | — | — |
| buffered_read_stream.test | RUN | 0.758181 | 5.358275 | 4.414335 | 0.001946 | 0.001692 | 0.001713 |
| buffered_read_stream_select.test | RUN | 0.757284 | 5.270650 | 4.345152 | 0.001300 | 0.001766 | 0.001644 |
| buffered_stream.test | RUN | 1.134493 | 8.292512 | 7.562311 | 0.001995 | 0.001904 | 0.001684 |
| buffered_stream_select.test | RUN | 1.177968 | 8.325726 | 7.394136 | 0.001882 | 0.001761 | 0.001794 |
| buffered_write_stream.test | RUN | 1.268093 | 7.177890 | 6.461881 | 0.001698 | 0.001796 | 0.001746 |
| buffered_write_stream_select.test | RUN | 1.272435 | 7.226893 | 6.233616 | 0.001792 | 0.001787 | 0.001691 |
| buffers_iterator.test | RUN | 0.384935 | 0.646945 | 1.027779 | 0.001391 | 0.001442 | 0.001234 |
| buffers_iterator_select.test | RUN | 0.381853 | 0.647789 | 1.026452 | 0.001414 | 0.001393 | 0.001259 |
| cancel_after.test | RUN | 1.085224 | 5.598844 | 4.705373 | 0.003812 | 0.003542 | 0.002989 |
| cancel_after_select.test | RUN | 1.096827 | 5.557041 | 4.651955 | 0.003381 | 0.003380 | 0.002871 |
| cancel_at.test | RUN | 1.089273 | 4.846160 | 4.500470 | 0.003052 | 0.003071 | 0.002488 |
| cancel_at_select.test | RUN | 1.075653 | 4.768843 | 4.461672 | 0.003233 | 0.002845 | 0.002405 |
| cancellation_signal.test | LINK | 0.137983 | 0.225095 | 0.260171 | — | — | — |
| cancellation_signal_select.test | LINK | 0.147129 | 0.225339 | 0.260645 | — | — | — |
| cancellation_state.test | LINK | 0.162471 | 0.221152 | 0.314091 | — | — | — |
| cancellation_state_select.test | LINK | 0.171741 | 0.229952 | 0.425697 | — | — | — |
| cancellation_type.test | LINK | 0.128490 | 0.186947 | 0.344833 | — | — | — |
| cancellation_type_select.test | LINK | 0.130903 | 0.179144 | 0.329714 | — | — | — |
| co_composed.test | RUN | 0.127135 | 0.177672 | 0.332682 | 0.001277 | 0.001000 | 0.001190 |
| co_composed_select.test | RUN | 0.128872 | 0.181573 | 0.332160 | 0.001401 | 0.001002 | 0.001082 |
| co_spawn.test | LINK | 0.128366 | 0.175976 | 0.328509 | — | — | — |
| co_spawn_select.test | LINK | 0.130750 | 0.268943 | 0.326278 | — | — | — |
| completion_condition.test | LINK | 0.169447 | 0.429763 | 0.438780 | — | — | — |
| completion_condition_select.test | LINK | 0.168735 | 0.432005 | 0.450940 | — | — | — |
| compose.test | RUN | 0.835990 | 4.368930 | 3.756208 | 0.001982 | 0.001975 | 0.001740 |
| compose_select.test | RUN | 0.838869 | 4.292174 | 3.720337 | 0.001924 | 0.002130 | 0.001663 |
| composed.test | RUN | 0.819014 | 4.259113 | 3.660308 | 0.001609 | 0.001566 | 0.001523 |
| composed_select.test | RUN | 0.792874 | 4.215769 | 3.606789 | 0.001588 | 0.001398 | 0.001350 |
| config.test | RUN | 0.409403 | 2.655177 | 1.638504 | 0.001769 | 0.001517 | 0.001362 |
| config_select.test | RUN | 0.465264 | 2.927845 | 1.850576 | 0.001590 | 0.001844 | 0.001333 |
| connect.test | LINK | 1.411902 | 9.009809 | 7.781643 | — | — | — |
| connect_select.test | LINK | 1.400611 | 9.061155 | 7.797015 | — | — | — |
| connect_pipe.test | RUN | 0.763003 | 3.458169 | 2.912423 | 0.001559 | 0.001571 | 0.001365 |
| connect_pipe_select.test | RUN | 0.757211 | 3.591195 | 3.093573 | 0.001509 | 0.001637 | 0.001277 |
| consign.test | RUN | 0.619789 | 3.381787 | 2.635821 | 0.001511 | 0.001476 | 0.001421 |
| consign_select.test | RUN | 0.632009 | 3.457010 | 2.620762 | 0.001569 | 0.001456 | 0.001302 |
| coroutine.test | LINK | 0.134274 | 0.333985 | 0.360435 | — | — | — |
| coroutine_select.test | LINK | 0.137231 | 0.325845 | 0.374406 | — | — | — |
| deadline_timer.test | RUN | 0.909876 | 4.346394 | 3.821664 | 0.002964 | 0.003023 | 0.002685 |
| deadline_timer_select.test | RUN | 0.908106 | 4.309403 | 3.832695 | 0.002730 | 0.002483 | 0.002396 |
| deferred.test | RUN | 0.148727 | 0.309264 | 0.366973 | 0.001397 | 0.001311 | 0.001197 |
| deferred_select.test | RUN | 0.143263 | 0.299784 | 0.361510 | 0.001441 | 0.001382 | 0.001272 |
| detached.test | LINK | 0.160250 | 0.333895 | 0.400136 | — | — | — |
| detached_select.test | LINK | 0.161574 | 0.331908 | 0.404862 | — | — | — |
| disposition.test | RUN | 0.155426 | 0.811543 | 0.757296 | 0.001128 | 0.001425 | 0.001385 |
| disposition_select.test | RUN | 0.160154 | 0.804295 | 0.782550 | 0.001096 | 0.001458 | 0.001252 |
| error.test | RUN | 0.134166 | 0.783379 | 0.683864 | 0.001086 | 0.001393 | 0.001355 |
| error_select.test | RUN | 0.135348 | 0.780292 | 0.672127 | 0.001104 | 0.001423 | 0.001243 |
| file_base.test | LINK | 0.167151 | 0.642831 | 0.709570 | — | — | — |
| file_base_select.test | LINK | 0.175045 | 0.643410 | 0.599681 | — | — | — |
| fwd_first.test | LINK | 0.396770 | 0.989583 | 1.230561 | — | — | — |
| fwd_last.test | LINK | 0.401706 | 0.859183 | 1.246948 | — | — | — |
| generic_basic_endpoint.test | LINK | 0.143023 | 0.356180 | 0.461717 | — | — | — |
| generic_basic_endpoint_select.test | LINK | 0.155938 | 0.353337 | 0.466534 | — | — | — |
| generic_datagram_protocol.test | LINK | 0.695663 | 3.311940 | 2.631136 | — | — | — |
| generic_datagram_protocol_select.test | LINK | 0.670186 | 3.356508 | 2.642100 | — | — | — |
| generic_raw_protocol.test | LINK | 0.670709 | 3.312405 | 2.644615 | — | — | — |
| generic_raw_protocol_select.test | LINK | 0.655911 | 3.371377 | 2.639506 | — | — | — |
| generic_seq_packet_protocol.test | LINK | 0.576018 | 2.814237 | 2.255897 | — | — | — |
| generic_seq_packet_protocol_select.test | LINK | 0.574321 | 2.804864 | 2.273184 | — | — | — |
| generic_stream_protocol.test | LINK | 0.614158 | 3.017383 | 2.462510 | — | — | — |
| generic_stream_protocol_select.test | LINK | 0.609232 | 3.050079 | 2.477042 | — | — | — |
| high_resolution_timer.test | LINK | 0.250670 | 0.529904 | 0.710941 | — | — | — |
| high_resolution_timer_select.test | LINK | 0.256746 | 0.539302 | 0.727618 | — | — | — |
| immediate.test | LINK | 0.170839 | 0.408352 | 0.435905 | — | — | — |
| immediate_select.test | LINK | 0.167913 | 0.406122 | 0.434691 | — | — | — |
| inline_executor.test | RUN | 0.213548 | 0.638297 | 0.663966 | 0.001115 | 0.001052 | 0.001038 |
| inline_executor_select.test | RUN | 0.211781 | 0.639165 | 0.679555 | 0.001205 | 0.001055 | 0.001004 |
| inline_or_executor.test | RUN | 0.358189 | 1.938470 | 1.320561 | 0.001118 | 0.001090 | 0.000986 |
| inline_or_executor_select.test | RUN | 0.363210 | 1.964967 | 1.439185 | 0.001128 | 0.001054 | 0.000987 |
| io_context.test | RUN | 0.523969 | 2.996134 | 2.301725 | 0.002025 | 0.002044 | 0.001897 |
| io_context_select.test | RUN | 0.541406 | 2.911743 | 2.309114 | 0.001868 | 0.001923 | 0.001828 |
| io_context_strand.test | RUN | 0.508824 | 2.520906 | 2.098716 | 0.002054 | 0.001860 | 0.001797 |
| io_context_strand_select.test | RUN | 0.505765 | 2.458099 | 2.041550 | 0.001982 | 0.001795 | 0.001765 |
| ip_address.test | LINK | 0.171045 | 0.569345 | 0.573836 | — | — | — |
| ip_address_select.test | LINK | 0.175720 | 0.567473 | 0.564156 | — | — | — |
| ip_address_v4.test | LINK | 0.175235 | 0.597393 | 0.625257 | — | — | — |
| ip_address_v4_select.test | LINK | 0.173079 | 0.599188 | 0.613045 | — | — | — |
| ip_address_v6.test | LINK | 0.227462 | 0.781711 | 0.751320 | — | — | — |
| ip_address_v6_select.test | LINK | 0.198774 | 0.784705 | 0.738831 | — | — | — |
| ip_basic_endpoint.test | LINK | 0.153573 | 0.377985 | 0.810825 | — | — | — |
| ip_basic_endpoint_select.test | LINK | 0.148841 | 0.373992 | 0.696663 | — | — | — |
| ip_basic_resolver.test | LINK | 0.257874 | 0.535167 | 1.133696 | — | — | — |
| ip_basic_resolver_select.test | LINK | 0.258710 | 0.552925 | 1.213132 | — | — | — |
| ip_basic_resolver_entry.test | LINK | 0.084854 | 0.178019 | 0.334234 | — | — | — |
| ip_basic_resolver_entry_select.test | LINK | 0.083837 | 0.178084 | 0.334751 | — | — | — |
| ip_basic_resolver_iterator.test | LINK | 0.149761 | 0.367799 | 0.799377 | — | — | — |
| ip_basic_resolver_iterator_select.test | LINK | 0.150490 | 0.359725 | 0.801153 | — | — | — |
| ip_basic_resolver_query.test | LINK | 0.148510 | 0.349925 | 0.752141 | — | — | — |
| ip_basic_resolver_query_select.test | LINK | 0.142251 | 0.352054 | 0.755065 | — | — | — |
| ip_host_name.test | RUN | 0.158268 | 0.486997 | 0.872695 | 0.001090 | 0.001019 | 0.001288 |
| ip_host_name_select.test | RUN | 0.158550 | 0.478537 | 0.872309 | 0.001108 | 0.001049 | 0.001234 |
| ip_icmp.test | RUN | 0.869848 | 7.015374 | 5.715413 | 0.001109 | 0.001420 | 0.001269 |
| ip_icmp_select.test | RUN | 0.871318 | 7.215156 | 5.588924 | 0.001117 | 0.001419 | 0.001290 |
| ip_multicast.test | RUN | 0.472918 | 3.605199 | 3.196436 | 0.001230 | 0.001478 | 0.001476 |
| ip_multicast_select.test | RUN | 0.477214 | 3.566790 | 3.186620 | 0.001389 | 0.001451 | 0.001396 |
| ip_resolver_query_base.test | LINK | 0.099797 | 0.308499 | 0.358400 | — | — | — |
| ip_resolver_query_base_select.test | LINK | 0.097826 | 0.309303 | 0.349297 | — | — | — |
| ip_tcp.test | RUN | 1.293828 | 10.605489 | 8.820694 | 0.001621 | 0.001944 | 0.001990 |
| ip_tcp_select.test | RUN | 1.253841 | 11.098112 | 8.573546 | 0.001479 | 0.002020 | 0.002006 |
| ip_udp.test | RUN | 0.901502 | 7.596624 | 6.042093 | 0.001301 | 0.001561 | 0.001524 |
| ip_udp_select.test | RUN | 0.904938 | 7.759676 | 5.893737 | 0.001234 | 0.001594 | 0.001603 |
| ip_unicast.test | RUN | 0.439387 | 3.261768 | 2.811622 | 0.001219 | 0.001592 | 0.001371 |
| ip_unicast_select.test | RUN | 0.446916 | 3.340788 | 2.773929 | 0.001119 | 0.001473 | 0.001388 |
| ip_v6_only.test | RUN | 0.447183 | 3.387365 | 2.911132 | 0.001221 | 0.001580 | 0.001402 |
| ip_v6_only_select.test | RUN | 0.452993 | 3.496680 | 2.871426 | 0.001223 | 0.001463 | 0.001152 |
| is_read_buffered.test | RUN | 0.296056 | 0.999258 | 1.494573 | 0.001104 | 0.001354 | 0.001147 |
| is_read_buffered_select.test | RUN | 0.304471 | 1.029727 | 1.523877 | 0.001078 | 0.001449 | 0.001125 |
| is_write_buffered.test | RUN | 0.297990 | 0.993883 | 1.465383 | 0.001106 | 0.001393 | 0.001257 |
| is_write_buffered_select.test | RUN | 0.300277 | 1.034243 | 1.505757 | 0.001122 | 0.001483 | 0.001245 |
| local_basic_endpoint.test | LINK | 0.145093 | 0.562674 | 0.763942 | — | — | — |
| local_basic_endpoint_select.test | LINK | 0.150734 | 0.551529 | 0.763590 | — | — | — |
| local_connect_pair.test | LINK | 0.416542 | 3.324498 | 2.667745 | — | — | — |
| local_connect_pair_select.test | LINK | 0.416428 | 3.298031 | 2.635320 | — | — | — |
| local_datagram_protocol.test | LINK | 0.589682 | 4.233099 | 3.754027 | — | — | — |
| local_datagram_protocol_select.test | LINK | 0.582963 | 4.285113 | 3.662485 | — | — | — |
| local_seq_packet_protocol.test | LINK | 0.880167 | 4.250791 | 3.686519 | — | — | — |
| local_seq_packet_protocol_select.test | LINK | 0.879535 | 4.304824 | 3.669047 | — | — | — |
| local_stream_protocol.test | LINK | 0.816313 | 4.028804 | 3.494704 | — | — | — |
| local_stream_protocol_select.test | LINK | 0.823689 | 4.028489 | 3.445569 | — | — | — |
| placeholders.test | LINK | 0.138294 | 0.294781 | 0.368846 | — | — | — |
| placeholders_select.test | LINK | 0.137160 | 0.312345 | 0.368717 | — | — | — |
| posix_basic_descriptor.test | LINK | 0.388657 | 0.849383 | 1.116207 | — | — | — |
| posix_basic_descriptor_select.test | LINK | 0.411266 | 0.912386 | 1.339506 | — | — | — |
| posix_basic_stream_descriptor.test | LINK | 0.399044 | 0.848553 | 1.131054 | — | — | — |
| posix_basic_stream_descriptor_select.test | LINK | 0.404962 | 0.916865 | 1.368004 | — | — | — |
| posix_descriptor_base.test | LINK | 0.153545 | 0.328778 | 0.368373 | — | — | — |
| posix_descriptor_base_select.test | LINK | 0.154053 | 0.325163 | 0.371339 | — | — | — |
| posix_stream_descriptor.test | LINK | 0.835229 | 4.126314 | 3.578943 | — | — | — |
| posix_stream_descriptor_select.test | LINK | 0.845874 | 4.221428 | 3.626137 | — | — | — |
| prepend.test | RUN | 0.623398 | 3.339009 | 2.793813 | 0.001659 | 0.001569 | 0.001469 |
| prepend_select.test | RUN | 0.633675 | 3.400325 | 2.786316 | 0.001338 | 0.001396 | 0.001497 |
| random_access_file.test | LINK | 0.357714 | 0.786915 | 0.889333 | — | — | — |
| random_access_file_select.test | LINK | 0.363605 | 0.840342 | 1.133163 | — | — | — |
| read.test | RUN | 2.024983 | 20.348341 | 15.888998 | 0.002012 | 0.002080 | 0.001839 |
| read_select.test | RUN | 2.056758 | 20.195810 | 15.996464 | 0.002037 | 0.002326 | 0.001812 |
| read_at.test | RUN | 1.990519 | 26.285289 | 20.810718 | 0.002636 | 0.002547 | 0.002307 |
| read_at_select.test | RUN | 1.994268 | 26.260761 | 20.996708 | 0.002571 | 0.002520 | 0.002282 |
| read_until.test | RUN | 1.065835 | 8.111637 | 6.065416 | 0.001684 | 0.001590 | 0.001664 |
| read_until_select.test | RUN | 1.078965 | 8.130053 | 6.245028 | 0.001593 | 0.001717 | 0.001620 |
| readable_pipe.test | LINK | 0.657096 | 3.386763 | 2.723475 | — | — | — |
| readable_pipe_select.test | LINK | 0.674739 | 3.530644 | 2.904421 | — | — | — |
| recycling_allocator.test | RUN | 0.183680 | 0.483964 | 0.548966 | 0.001429 | 0.001392 | 0.001289 |
| recycling_allocator_select.test | RUN | 0.185634 | 0.486039 | 0.543603 | 0.001500 | 0.001351 | 0.001238 |
| redirect_disposition.test | LINK | 0.840003 | 4.686459 | 4.160836 | — | — | — |
| redirect_disposition_select.test | LINK | 0.850800 | 4.624209 | 4.113784 | — | — | — |
| redirect_error.test | LINK | 0.742491 | 3.896550 | 3.606852 | — | — | — |
| redirect_error_select.test | LINK | 0.748617 | 3.854814 | 3.592017 | — | — | — |
| registered_buffer.test | LINK | 0.184523 | 0.392807 | 0.490894 | — | — | — |
| registered_buffer_select.test | LINK | 0.185206 | 0.400606 | 0.496185 | — | — | — |
| signal_set.test | RUN | 0.623579 | 3.304075 | 2.593708 | 0.001456 | 0.001489 | 0.001338 |
| signal_set_select.test | RUN | 0.651290 | 3.422939 | 2.844741 | 0.001425 | 0.001415 | 0.001306 |
| signal_set_base.test | LINK | 0.144944 | 0.305576 | 0.379563 | — | — | — |
| signal_set_base_select.test | LINK | 0.150580 | 0.305090 | 0.361783 | — | — | — |
| socket_base.test | RUN | 0.768338 | 4.739577 | 3.849268 | 0.001493 | 0.001499 | 0.001428 |
| socket_base_select.test | RUN | 0.787358 | 4.626800 | 3.824318 | 0.001329 | 0.001308 | 0.001446 |
| spawn.test | RUN | 1.279772 | 7.889997 | 6.562367 | 0.001750 | 0.001666 | 0.001573 |
| spawn_select.test | RUN | 1.289017 | 7.939622 | 6.557479 | 0.001725 | 0.001777 | 0.001536 |
| static_thread_pool.test | RUN | 0.338119 | 0.747538 | 0.864117 | 0.001357 | 0.001019 | 0.001345 |
| static_thread_pool_select.test | RUN | 0.349218 | 0.792682 | 1.108357 | 0.001403 | 0.001265 | 0.001288 |
| steady_timer.test | LINK | 0.371336 | 0.838425 | 1.188036 | — | — | — |
| steady_timer_select.test | LINK | 0.381326 | 0.855533 | 1.215187 | — | — | — |
| strand.test | RUN | 0.825747 | 4.438729 | 3.993901 | 0.002145 | 0.002306 | 0.002059 |
| strand_select.test | RUN | 0.827281 | 4.428443 | 3.987443 | 0.001832 | 0.002203 | 0.002198 |
| stream_file.test | LINK | 0.353215 | 0.755672 | 0.886755 | — | — | — |
| stream_file_select.test | LINK | 0.366809 | 0.637674 | 1.135557 | — | — | — |
| streambuf.test | RUN | 0.216989 | 0.405491 | 0.628373 | 0.001453 | 0.001059 | 0.001350 |
| streambuf_select.test | RUN | 0.217282 | 0.411172 | 0.486721 | 0.001029 | 0.001035 | 0.000995 |
| system_timer.test | LINK | 0.505072 | 2.754473 | 2.499507 | — | — | — |
| system_timer_select.test | LINK | 0.514083 | 2.761761 | 2.289210 | — | — | — |
| system_context.test | LINK | 0.219667 | 0.501061 | 0.519789 | — | — | — |
| system_context_select.test | LINK | 0.235742 | 0.505577 | 0.666685 | — | — | — |
| system_executor.test | LINK | 0.300825 | 1.568075 | 1.098431 | — | — | — |
| system_executor_select.test | LINK | 0.313715 | 1.550177 | 1.224120 | — | — | — |
| this_coro.test | LINK | 0.079605 | 0.174389 | 0.205526 | — | — | — |
| this_coro_select.test | LINK | 0.082513 | 0.179465 | 0.206033 | — | — | — |
| thread_pool.test | RUN | 0.363216 | 2.186388 | 1.386367 | 0.002228 | 0.002979 | 0.002173 |
| thread_pool_select.test | RUN | 0.372406 | 2.449018 | 1.476864 | 0.002271 | 0.002700 | 0.001995 |
| time_traits.test | LINK | 0.184295 | 0.588990 | 0.425091 | — | — | — |
| time_traits_select.test | LINK | 0.182598 | 0.597154 | 0.435647 | — | — | — |
| ts_buffer.test | LINK | 0.180417 | 0.642067 | 0.460645 | — | — | — |
| ts_buffer_select.test | LINK | 0.178946 | 0.649617 | 0.453715 | — | — | — |
| ts_executor.test | LINK | 0.262856 | 0.924577 | 0.725705 | — | — | — |
| ts_executor_select.test | LINK | 0.267276 | 0.951915 | 1.400940 | — | — | — |
| ts_internet.test | LINK | 0.284766 | 0.996791 | 1.434409 | — | — | — |
| ts_internet_select.test | LINK | 0.283330 | 1.026009 | 1.314434 | — | — | — |
| ts_io_context.test | LINK | 0.228636 | 0.759780 | 0.871158 | — | — | — |
| ts_io_context_select.test | LINK | 0.236824 | 0.813986 | 1.104110 | — | — | — |
| ts_net.test | LINK | 0.343319 | 1.190677 | 1.813217 | — | — | — |
| ts_net_select.test | LINK | 0.343260 | 1.236528 | 1.838186 | — | — | — |
| ts_netfwd.test | LINK | 0.347306 | 1.182263 | 1.812171 | — | — | — |
| ts_netfwd_select.test | LINK | 0.351723 | 1.231110 | 1.842726 | — | — | — |
| ts_socket.test | LINK | 0.285987 | 0.938639 | 1.344262 | — | — | — |
| ts_socket_select.test | LINK | 0.289318 | 0.963389 | 1.364820 | — | — | — |
| ts_timer.test | LINK | 0.254486 | 0.833175 | 1.192117 | — | — | — |
| ts_timer_select.test | LINK | 0.258168 | 0.870474 | 1.209423 | — | — | — |
| use_awaitable.test | LINK | 0.093752 | 0.278210 | 0.329432 | — | — | — |
| use_awaitable_select.test | LINK | 0.085392 | 0.286890 | 0.336071 | — | — | — |
| wait_traits.test | LINK | 0.081740 | 0.276201 | 0.324574 | — | — | — |
| wait_traits_select.test | LINK | 0.084635 | 0.271829 | 0.329117 | — | — | — |
| windows_basic_object_handle.test | LINK | 0.083137 | 0.282935 | 0.335668 | — | — | — |
| windows_basic_object_handle_select.test | LINK | 0.083219 | 0.279788 | 0.325117 | — | — | — |
| windows_basic_overlapped_handle.test | LINK | 0.083120 | 0.278978 | 0.327290 | — | — | — |
| windows_basic_overlapped_handle_select.test | LINK | 0.083151 | 0.268175 | 0.340405 | — | — | — |
| windows_basic_random_access_handle.test | LINK | 0.083233 | 0.275642 | 0.333735 | — | — | — |
| windows_basic_random_access_handle_select.test | LINK | 0.083724 | 0.279777 | 0.339053 | — | — | — |
| windows_basic_stream_handle.test | LINK | 0.083467 | 0.275783 | 0.329049 | — | — | — |
| windows_basic_stream_handle_select.test | LINK | 0.085257 | 0.274690 | 0.335508 | — | — | — |
| windows_object_handle.test | LINK | 0.236738 | 0.767539 | 0.886797 | — | — | — |
| windows_object_handle_select.test | LINK | 0.248758 | 0.800265 | 1.106336 | — | — | — |
| windows_overlapped_ptr.test | LINK | 0.247550 | 0.799655 | 0.962697 | — | — | — |
| windows_overlapped_ptr_select.test | LINK | 0.255962 | 0.867277 | 1.205474 | — | — | — |
| windows_random_access_handle.test | LINK | 0.288659 | 0.771410 | 0.884299 | — | — | — |
| windows_random_access_handle_select.test | LINK | 0.388887 | 0.807333 | 1.111516 | — | — | — |
| windows_stream_handle.test | LINK | 0.351782 | 0.766920 | 0.888349 | — | — | — |
| windows_stream_handle_select.test | LINK | 0.384322 | 0.821139 | 1.128466 | — | — | — |
| writable_pipe.test | LINK | 0.683935 | 3.320507 | 2.871789 | — | — | — |
| writable_pipe_select.test | LINK | 0.700412 | 3.485843 | 3.006022 | — | — | — |
| write.test | RUN | 2.034345 | 17.068066 | 15.653084 | 0.002136 | 0.002039 | 0.001791 |
| write_select.test | RUN | 2.046174 | 17.123121 | 15.771361 | 0.002306 | 0.001992 | 0.002082 |
| write_at.test | RUN | 2.066615 | 22.587749 | 20.029657 | 0.002541 | 0.002231 | 0.002243 |
| write_at_select.test | RUN | 2.069866 | 22.623623 | 19.929175 | 0.002482 | 0.002278 | 0.002180 |
| any_executor.test | RUN | 0.808150 | 7.563397 | 6.345776 | 0.002215 | 0.001754 | 0.001863 |
| blocking.test | RUN | 1.827939 | 10.095003 | 13.368914 | 0.004537 | 0.004181 | 0.004322 |
| blocking_adaptation.test | RUN | 0.934433 | 4.334262 | 6.061162 | 0.002677 | 0.002557 | 0.002496 |
| context_as.test | RUN | 0.540136 | 3.096434 | 2.013410 | 0.001628 | 0.001358 | 0.001616 |
| executor.test | RUN | 0.134468 | 0.290470 | 0.368418 | 0.001459 | 0.001339 | 0.001249 |
| inline_exception_handling.test | RUN | 1.817017 | 9.311504 | 12.695363 | 0.004541 | 0.002778 | 0.002800 |
| invocable_archetype.test | RUN | 0.133225 | 0.303053 | 0.348465 | 0.001547 | 0.001400 | 0.001289 |
| mapping.test | RUN | 1.649167 | 9.452852 | 13.182342 | 0.002994 | 0.002905 | 0.002911 |
| outstanding_work.test | RUN | 0.838209 | 4.209852 | 5.758201 | 0.002778 | 0.002518 | 0.002535 |
| prefer_only.test | RUN | 0.276994 | 1.636558 | 1.283668 | 0.001538 | 0.001579 | 0.001341 |
| relationship.test | RUN | 0.752227 | 3.941482 | 5.434545 | 0.001745 | 0.002625 | 0.001775 |

## Native runtime replay

Child CPU of successful native tests, with process startup included; GDB and expected-failure executions are correctness-only. Two warmups per compiler, four A/A observations per reference and mirrored ABBA/BAAB blocks. Short tests repeat up to ten times, identically across compilers; CPU is divided by repetitions before summing tests. Executable hashes and output checks stay outside process timers. All runs must pass their original native oracle. Output hashes record equality or variation across the complete campaign; test reports may print times or addresses.

| Reference | CPU speedup ↑ | Paired blocks | A/A speedup |
|---|---:|---|---:|
| gcc | 0.993× | 0.991×, 0.996× | 1.007× |
| clang | 0.980× | 0.984×, 0.976× | 0.998× |

† A/A drift or paired-block spread exceeds 5%; small differences are unresolved. 1375 native tests in the full cohort; 1360 have byte-identical output throughout. [Raw replay evidence in CI artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43395345325).

| Test | Repeats | CPU cppgm | GCC | Clang | Output check |
|---|---:|---:|---:|---:|---|
| any_completion_executor.test | 3 | 0.001404s | 0.001384s | 0.001435s | byte equality observed |
| any_completion_executor_select.test | 6 | 0.001411s | 0.001376s | 0.001417s | byte equality observed |
| any_completion_handler.test | 10 | 0.001119s | 0.001129s | 0.001150s | byte equality observed |
| any_completion_handler_select.test | 10 | 0.001137s | 0.001104s | 0.001148s | byte equality observed |
| any_io_executor.test | 9 | 0.001387s | 0.001381s | 0.001387s | byte equality observed |
| any_io_executor_select.test | 5 | 0.001360s | 0.001329s | 0.001364s | byte equality observed |
| append.test | 10 | 0.001081s | 0.001078s | 0.001095s | byte equality observed |
| append_select.test | 10 | 0.001079s | 0.001055s | 0.001061s | byte equality observed |
| as_tuple.test | 8 | 0.001150s | 0.001140s | 0.001131s | byte equality observed |
| as_tuple_select.test | 8 | 0.001070s | 0.001007s | 0.001108s | byte equality observed |
| bind_allocator.test | 1 | 0.001518s | 0.001500s | 0.001517s | byte equality observed |
| bind_allocator_select.test | 1 | 0.001204s | 0.001113s | 0.001262s | byte equality observed |
| bind_cancellation_slot.test | 10 | 0.001019s | 0.000993s | 0.001001s | byte equality observed |
| bind_cancellation_slot_select.test | 10 | 0.000925s | 0.000903s | 0.000898s | byte equality observed |
| bind_executor.test | 1 | 0.001522s | 0.001508s | 0.001519s | byte equality observed |
| bind_executor_select.test | 1 | 0.001718s | 0.001687s | 0.001683s | byte equality observed |
| bind_immediate_executor.test | 10 | 0.001036s | 0.001019s | 0.001005s | byte equality observed |
| bind_immediate_executor_select.test | 10 | 0.001019s | 0.001032s | 0.001044s | byte equality observed |
| buffer.test | 10 | 0.000938s | 0.000984s | 0.000945s | byte equality observed |
| buffer_select.test | 10 | 0.000979s | 0.000963s | 0.000935s | byte equality observed |
| buffered_read_stream.test | 10 | 0.001192s | 0.001166s | 0.001149s | byte equality observed |
| buffered_read_stream_select.test | 10 | 0.001234s | 0.001146s | 0.001147s | byte equality observed |
| buffered_stream.test | 10 | 0.001204s | 0.001126s | 0.001186s | byte equality observed |
| buffered_stream_select.test | 10 | 0.001173s | 0.001142s | 0.001145s | byte equality observed |
| buffered_write_stream.test | 10 | 0.001170s | 0.001159s | 0.001161s | byte equality observed |
| buffered_write_stream_select.test | 10 | 0.001167s | 0.001150s | 0.001131s | byte equality observed |
| buffers_iterator.test | 10 | 0.000906s | 0.000916s | 0.000916s | byte equality observed |
| buffers_iterator_select.test | 10 | 0.000893s | 0.000866s | 0.000901s | byte equality observed |
| cancel_after.test | 1 | 0.003095s | 0.003042s | 0.003283s | byte equality observed |
| cancel_after_select.test | 1 | 0.002573s | 0.002434s | 0.002502s | byte equality observed |
| cancel_at.test | 1 | 0.003183s | 0.003052s | 0.003216s | byte equality observed |
| cancel_at_select.test | 1 | 0.003083s | 0.003115s | 0.003324s | byte equality observed |
| co_composed.test | 10 | 0.000956s | 0.000933s | 0.000943s | byte equality observed |
| co_composed_select.test | 10 | 0.000920s | 0.000901s | 0.000895s | byte equality observed |
| compose.test | 1 | 0.001514s | 0.001499s | 0.001570s | byte equality observed |
| compose_select.test | 1 | 0.001272s | 0.001226s | 0.001244s | byte equality observed |
| composed.test | 1 | 0.001705s | 0.001747s | 0.001759s | byte equality observed |
| composed_select.test | 1 | 0.001415s | 0.001404s | 0.001452s | byte equality observed |
| config.test | 10 | 0.001012s | 0.000998s | 0.001022s | byte equality observed |
| config_select.test | 10 | 0.000994s | 0.001033s | 0.001040s | byte equality observed |
| connect_pipe.test | 10 | 0.001017s | 0.001051s | 0.001005s | byte equality observed |
| connect_pipe_select.test | 10 | 0.000994s | 0.000988s | 0.000973s | byte equality observed |
| consign.test | 10 | 0.000989s | 0.000967s | 0.000981s | byte equality observed |
| consign_select.test | 10 | 0.000953s | 0.000963s | 0.000940s | byte equality observed |
| deadline_timer.test | 1 | 0.002164s | 0.002099s | 0.002164s | byte equality observed |
| deadline_timer_select.test | 1 | 0.001930s | 0.001953s | 0.001978s | byte equality observed |
| deferred.test | 10 | 0.000891s | 0.000889s | 0.000906s | byte equality observed |
| deferred_select.test | 10 | 0.000822s | 0.000807s | 0.000840s | byte equality observed |
| disposition.test | 10 | 0.000887s | 0.000854s | 0.000888s | byte equality observed |
| disposition_select.test | 10 | 0.000889s | 0.000862s | 0.000882s | byte equality observed |
| error.test | 10 | 0.000867s | 0.000849s | 0.000863s | byte equality observed |
| error_select.test | 10 | 0.000867s | 0.000829s | 0.000867s | byte equality observed |
| inline_executor.test | 10 | 0.000879s | 0.000828s | 0.000877s | byte equality observed |
| inline_executor_select.test | 10 | 0.000872s | 0.000856s | 0.000875s | byte equality observed |
| inline_or_executor.test | 10 | 0.000883s | 0.000869s | 0.000870s | byte equality observed |
| inline_or_executor_select.test | 10 | 0.000882s | 0.000876s | 0.000863s | byte equality observed |
| io_context.test | 1 | 0.001840s | 0.001787s | 0.001836s | byte equality observed |
| io_context_select.test | 1 | 0.002055s | 0.002001s | 0.002099s | byte equality observed |
| io_context_strand.test | 1 | 0.002039s | 0.002078s | 0.002065s | byte equality observed |
| io_context_strand_select.test | 1 | 0.001765s | 0.001731s | 0.001823s | byte equality observed |
| ip_host_name.test | 10 | 0.000911s | 0.000896s | 0.000889s | byte equality observed |
| ip_host_name_select.test | 10 | 0.000858s | 0.000835s | 0.000868s | byte equality observed |
| ip_icmp.test | 10 | 0.000886s | 0.000870s | 0.000863s | byte equality observed |
| ip_icmp_select.test | 10 | 0.000887s | 0.000878s | 0.000865s | byte equality observed |
| ip_multicast.test | 10 | 0.000957s | 0.000953s | 0.000948s | byte equality observed |
| ip_multicast_select.test | 10 | 0.000939s | 0.000925s | 0.000916s | byte equality observed |
| ip_tcp.test | 10 | 0.001212s | 0.001185s | 0.001172s | byte equality observed |
| ip_tcp_select.test | 10 | 0.001164s | 0.001135s | 0.001138s | byte equality observed |
| ip_udp.test | 10 | 0.000964s | 0.000956s | 0.000930s | byte equality observed |
| ip_udp_select.test | 10 | 0.000963s | 0.000935s | 0.000963s | byte equality observed |
| ip_unicast.test | 10 | 0.000924s | 0.000898s | 0.000910s | byte equality observed |
| ip_unicast_select.test | 10 | 0.000914s | 0.000886s | 0.000890s | byte equality observed |
| ip_v6_only.test | 10 | 0.000918s | 0.000886s | 0.000890s | byte equality observed |
| ip_v6_only_select.test | 10 | 0.000906s | 0.000885s | 0.000882s | byte equality observed |
| is_read_buffered.test | 10 | 0.000849s | 0.000808s | 0.000831s | byte equality observed |
| is_read_buffered_select.test | 10 | 0.000845s | 0.000835s | 0.000828s | byte equality observed |
| is_write_buffered.test | 10 | 0.000844s | 0.000875s | 0.000835s | byte equality observed |
| is_write_buffered_select.test | 10 | 0.000847s | 0.000901s | 0.000893s | byte equality observed |
| prepend.test | 10 | 0.000911s | 0.000986s | 0.000899s | byte equality observed |
| prepend_select.test | 10 | 0.000895s | 0.000969s | 0.000881s | byte equality observed |
| read.test | 10 | 0.001286s | 0.001306s | 0.001201s | byte equality observed |
| read_at.test | 10 | 0.001663s | 0.001803s | 0.001537s | byte equality observed |
| read_at_select.test | 10 | 0.001754s | 0.001748s | 0.001584s | byte equality observed |
| read_select.test | 10 | 0.001369s | 0.001289s | 0.001300s | byte equality observed |
| read_until.test | 10 | 0.001120s | 0.001075s | 0.001099s | byte equality observed |
| read_until_select.test | 10 | 0.001110s | 0.001060s | 0.001065s | byte equality observed |
| recycling_allocator.test | 10 | 0.000933s | 0.000952s | 0.000911s | byte equality observed |
| recycling_allocator_select.test | 10 | 0.000918s | 0.000906s | 0.000934s | byte equality observed |
| signal_set.test | 10 | 0.000976s | 0.000957s | 0.000933s | byte equality observed |
| signal_set_select.test | 10 | 0.000934s | 0.000919s | 0.000903s | byte equality observed |
| socket_base.test | 10 | 0.001018s | 0.001004s | 0.001026s | byte equality observed |
| socket_base_select.test | 10 | 0.001007s | 0.000990s | 0.001033s | byte equality observed |
| spawn.test | 10 | 0.001117s | 0.001138s | 0.001113s | byte equality observed |
| spawn_select.test | 10 | 0.001095s | 0.001109s | 0.001134s | byte equality observed |
| static_thread_pool.test | 10 | 0.000910s | 0.000902s | 0.000908s | byte equality observed |
| static_thread_pool_select.test | 10 | 0.000892s | 0.000875s | 0.000849s | byte equality observed |
| strand.test | 1 | 0.001834s | 0.001765s | 0.001964s | byte equality observed |
| strand_select.test | 1 | 0.001522s | 0.001461s | 0.001605s | byte equality observed |
| streambuf.test | 10 | 0.000975s | 0.000978s | 0.000952s | byte equality observed |
| streambuf_select.test | 10 | 0.000999s | 0.000976s | 0.000973s | byte equality observed |
| thread_pool.test | 10 | 0.002123s | 0.002173s | 0.002168s | byte equality observed |
| thread_pool_select.test | 9 | 0.002131s | 0.002127s | 0.002148s | byte equality observed |
| write.test | 8 | 0.001450s | 0.001330s | 0.001378s | byte equality observed |
| write_at.test | 10 | 0.001593s | 0.001451s | 0.001468s | byte equality observed |
| write_at_select.test | 10 | 0.001572s | 0.001452s | 0.001481s | byte equality observed |
| write_select.test | 7 | 0.001370s | 0.001312s | 0.001371s | byte equality observed |

[Raw outcomes, commands and timing observations in CI artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43395345325).

[Comparison records](../results.json) · [Host and toolchains](../metadata.json)
