# libs/asio/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| any_completion_executor.test | RUN | 0.645808 | 3.745112 | 3.553376 | 0.002298 | 0.002473 | 0.002355 |
| any_completion_executor_select.test | RUN | 0.646816 | 3.720958 | 3.652948 | 0.002429 | 0.002450 | 0.002376 |
| any_completion_handler.test | RUN | 0.674785 | 3.310766 | 3.250413 | 0.001996 | 0.002012 | 0.001947 |
| any_completion_handler_select.test | RUN | 0.719032 | 3.462689 | 3.496702 | 0.002090 | 0.002020 | 0.002012 |
| any_io_executor.test | RUN | 0.655368 | 3.922751 | 3.780709 | 0.002230 | 0.002294 | 0.002367 |
| any_io_executor_select.test | RUN | 0.667235 | 3.894203 | 3.937035 | 0.002109 | 0.002293 | 0.002346 |
| append.test | RUN | 0.608013 | 3.130638 | 2.880700 | 0.001819 | 0.001977 | 0.001928 |
| append_select.test | RUN | 0.608504 | 3.229636 | 2.904855 | 0.001793 | 0.001725 | 0.001778 |
| as_tuple.test | RUN | 0.766316 | 3.835720 | 4.024585 | 0.002044 | 0.001730 | 0.001938 |
| as_tuple_select.test | RUN | 0.777813 | 3.823978 | 3.989273 | 0.001778 | 0.002054 | 0.001708 |
| associated_allocator.test | LINK | 0.167029 | 0.328162 | 0.434128 | — | — | — |
| associated_allocator_select.test | LINK | 0.169863 | 0.329815 | 0.444371 | — | — | — |
| associated_cancellation_slot.test | LINK | 0.180347 | 0.356994 | 0.479730 | — | — | — |
| associated_cancellation_slot_select.test | LINK | 0.181242 | 0.357733 | 0.480287 | — | — | — |
| associated_executor.test | LINK | 0.262881 | 0.615972 | 0.788115 | — | — | — |
| associated_executor_select.test | LINK | 0.258848 | 0.627486 | 0.793997 | — | — | — |
| associated_immediate_executor.test | LINK | 0.240650 | 0.585452 | 0.724795 | — | — | — |
| associated_immediate_executor_select.test | LINK | 0.241768 | 0.570224 | 0.727077 | — | — | — |
| associator.test | LINK | 0.135626 | 0.286104 | 0.382686 | — | — | — |
| associator_select.test | LINK | 0.135112 | 0.279213 | 0.384320 | — | — | — |
| awaitable.test | LINK | 0.137730 | 0.277864 | 0.375863 | — | — | — |
| awaitable_select.test | LINK | 0.138388 | 0.273023 | 0.369911 | — | — | — |
| basic_datagram_socket.test | LINK | 0.408443 | 0.868091 | 1.434310 | — | — | — |
| basic_datagram_socket_select.test | LINK | 0.416462 | 0.884334 | 1.469811 | — | — | — |
| basic_deadline_timer.test | LINK | 0.485030 | 1.039288 | 1.619583 | — | — | — |
| basic_deadline_timer_select.test | LINK | 0.498681 | 1.052680 | 1.675014 | — | — | — |
| basic_file.test | LINK | 0.134602 | 0.287481 | 0.381061 | — | — | — |
| basic_file_select.test | LINK | 0.138109 | 0.287700 | 0.380698 | — | — | — |
| basic_random_access_file.test | LINK | 0.137556 | 0.273713 | 0.377865 | — | — | — |
| basic_random_access_file_select.test | LINK | 0.137846 | 0.272605 | 0.385776 | — | — | — |
| basic_raw_socket.test | LINK | 0.408964 | 0.877152 | 1.421847 | — | — | — |
| basic_raw_socket_select.test | LINK | 0.427382 | 0.908322 | 1.470144 | — | — | — |
| basic_readable_pipe.test | LINK | 0.406500 | 0.855043 | 1.227704 | — | — | — |
| basic_readable_pipe_select.test | LINK | 0.417129 | 0.903657 | 1.507444 | — | — | — |
| basic_seq_packet_socket.test | LINK | 0.407468 | 0.878539 | 1.416480 | — | — | — |
| basic_seq_packet_socket_select.test | LINK | 0.422929 | 0.906414 | 1.477731 | — | — | — |
| basic_signal_set.test | LINK | 0.386476 | 0.812970 | 1.132013 | — | — | — |
| basic_signal_set_select.test | LINK | 0.395403 | 0.864861 | 1.388084 | — | — | — |
| basic_socket_acceptor.test | LINK | 0.408950 | 0.883613 | 1.463098 | — | — | — |
| basic_socket_acceptor_select.test | LINK | 0.422878 | 0.918597 | 1.484286 | — | — | — |
| basic_stream_file.test | LINK | 0.132960 | 0.278172 | 0.375544 | — | — | — |
| basic_stream_file_select.test | LINK | 0.140060 | 0.282761 | 0.376076 | — | — | — |
| basic_stream_socket.test | LINK | 0.410273 | 0.884638 | 1.442853 | — | — | — |
| basic_stream_socket_select.test | LINK | 0.421654 | 0.920903 | 1.475741 | — | — | — |
| basic_streambuf.test | LINK | 0.200286 | 0.397471 | 0.572695 | — | — | — |
| basic_streambuf_select.test | LINK | 0.202389 | 0.405525 | 0.574197 | — | — | — |
| basic_waitable_timer.test | LINK | 0.387816 | 0.820809 | 1.374465 | — | — | — |
| basic_waitable_timer_select.test | LINK | 0.396862 | 0.847504 | 1.394645 | — | — | — |
| basic_writable_pipe.test | LINK | 0.390666 | 0.843874 | 1.231758 | — | — | — |
| basic_writable_pipe_select.test | LINK | 0.416760 | 0.902473 | 1.489228 | — | — | — |
| bind_allocator.test | RUN | 0.645002 | 3.214028 | 3.146386 | 0.002153 | 0.002319 | 0.002056 |
| bind_allocator_select.test | RUN | 0.655465 | 3.269225 | 3.167191 | 0.001842 | 0.002042 | 0.001646 |
| bind_cancellation_slot.test | RUN | 0.654061 | 3.254232 | 3.185799 | 0.001468 | 0.001467 | 0.001377 |
| bind_cancellation_slot_select.test | RUN | 0.663995 | 3.242091 | 3.031081 | 0.001440 | 0.001474 | 0.001359 |
| bind_executor.test | RUN | 0.441465 | 2.258126 | 1.930983 | 0.002139 | 0.002161 | 0.002043 |
| bind_executor_select.test | RUN | 0.446409 | 2.270209 | 1.910884 | 0.001931 | 0.001879 | 0.001821 |
| bind_immediate_executor.test | RUN | 0.338079 | 1.846735 | 1.217491 | 0.001510 | 0.001440 | 0.001368 |
| bind_immediate_executor_select.test | RUN | 0.345197 | 1.838006 | 1.349440 | 0.001371 | 0.001402 | 0.001318 |
| buffer.test | RUN | 0.204348 | 0.774256 | 0.760821 | 0.001482 | 0.001449 | 0.001344 |
| buffer_select.test | RUN | 0.201641 | 0.784841 | 0.772262 | 0.001331 | 0.001368 | 0.001291 |
| buffer_registration.test | LINK | 0.168094 | 0.404814 | 0.450897 | — | — | — |
| buffer_registration_select.test | LINK | 0.168096 | 0.402454 | 0.453218 | — | — | — |
| buffered_read_stream.test | RUN | 0.707950 | 4.897901 | 4.595510 | 0.001746 | 0.002438 | 0.002304 |
| buffered_read_stream_select.test | RUN | 0.721396 | 5.062472 | 4.524218 | 0.001736 | 0.002332 | 0.002362 |
| buffered_stream.test | RUN | 1.040355 | 7.664368 | 7.692003 | 0.002226 | 0.002367 | 0.002497 |
| buffered_stream_select.test | RUN | 1.109295 | 7.723738 | 7.464352 | 0.002326 | 0.002459 | 0.002246 |
| buffered_write_stream.test | RUN | 1.165790 | 6.640799 | 6.634483 | 0.002186 | 0.002483 | 0.002256 |
| buffered_write_stream_select.test | RUN | 1.172869 | 6.736738 | 6.327180 | 0.002198 | 0.002338 | 0.002165 |
| buffers_iterator.test | RUN | 0.376160 | 0.626425 | 1.070920 | 0.001736 | 0.001760 | 0.001622 |
| buffers_iterator_select.test | RUN | 0.370199 | 0.615592 | 1.077249 | 0.001746 | 0.001835 | 0.001547 |
| cancel_after.test | RUN | 1.007567 | 5.141480 | 4.850419 | 0.003662 | 0.003767 | 0.003816 |
| cancel_after_select.test | RUN | 1.021874 | 5.081566 | 4.841144 | 0.003957 | 0.003452 | 0.003722 |
| cancel_at.test | RUN | 1.011397 | 4.657596 | 4.571471 | 0.003532 | 0.003528 | 0.003390 |
| cancel_at_select.test | RUN | 0.988533 | 4.506477 | 4.529000 | 0.003332 | 0.003261 | 0.003404 |
| cancellation_signal.test | LINK | 0.122027 | 0.239406 | 0.292126 | — | — | — |
| cancellation_signal_select.test | LINK | 0.203758 | 0.237678 | 0.287501 | — | — | — |
| cancellation_state.test | LINK | 0.201569 | 0.242743 | 0.369452 | — | — | — |
| cancellation_state_select.test | LINK | 0.188032 | 0.243044 | 0.413186 | — | — | — |
| cancellation_type.test | LINK | 0.142512 | 0.192457 | 0.378746 | — | — | — |
| cancellation_type_select.test | LINK | 0.133840 | 0.192138 | 0.356248 | — | — | — |
| co_composed.test | RUN | 0.135732 | 0.192816 | 0.386309 | 0.001642 | 0.001366 | 0.001663 |
| co_composed_select.test | RUN | 0.134926 | 0.225683 | 0.389984 | 0.001422 | 0.001913 | 0.001692 |
| co_spawn.test | LINK | 0.137644 | 0.228362 | 0.360397 | — | — | — |
| co_spawn_select.test | LINK | 0.137400 | 0.217736 | 0.378302 | — | — | — |
| completion_condition.test | LINK | 0.174083 | 0.404800 | 0.513633 | — | — | — |
| completion_condition_select.test | LINK | 0.173298 | 0.422495 | 0.511775 | — | — | — |
| compose.test | RUN | 0.777599 | 4.058240 | 3.977528 | 0.002241 | 0.002166 | 0.002106 |
| compose_select.test | RUN | 0.805757 | 3.986944 | 3.977710 | 0.002376 | 0.002186 | 0.001839 |
| composed.test | RUN | 0.750327 | 3.925706 | 3.858326 | 0.002024 | 0.001986 | 0.001960 |
| composed_select.test | RUN | 0.758043 | 3.893419 | 3.811932 | 0.001589 | 0.001886 | 0.001604 |
| config.test | RUN | 0.360506 | 2.519049 | 1.622039 | 0.001937 | 0.001979 | 0.001876 |
| config_select.test | RUN | 0.475248 | 2.625391 | 1.922708 | 0.001938 | 0.001992 | 0.001760 |
| connect.test | LINK | 1.250546 | 8.439481 | 8.289683 | — | — | — |
| connect_select.test | LINK | 1.247709 | 8.413131 | 8.313112 | — | — | — |
| connect_pipe.test | RUN | 0.701618 | 3.187658 | 3.089306 | 0.001868 | 0.001898 | 0.001882 |
| connect_pipe_select.test | RUN | 0.705009 | 3.277984 | 3.330161 | 0.001902 | 0.001924 | 0.001842 |
| consign.test | RUN | 0.611002 | 3.127342 | 2.765331 | 0.001888 | 0.001817 | 0.001774 |
| consign_select.test | RUN | 0.615370 | 3.167844 | 2.725773 | 0.001952 | 0.001928 | 0.001808 |
| coroutine.test | LINK | 0.142548 | 0.333561 | 0.412416 | — | — | — |
| coroutine_select.test | LINK | 0.140829 | 0.316347 | 0.402647 | — | — | — |
| deadline_timer.test | RUN | 0.867133 | 4.057912 | 3.990228 | 0.003235 | 0.002892 | 0.003182 |
| deadline_timer_select.test | RUN | 0.872986 | 3.959623 | 3.978844 | 0.002653 | 0.003115 | 0.002874 |
| deferred.test | RUN | 0.148303 | 0.305028 | 0.428990 | 0.001738 | 0.001828 | 0.001745 |
| deferred_select.test | RUN | 0.150327 | 0.314250 | 0.427385 | 0.001797 | 0.001855 | 0.001776 |
| detached.test | LINK | 0.165452 | 0.343366 | 0.473755 | — | — | — |
| detached_select.test | LINK | 0.175055 | 0.340731 | 0.462110 | — | — | — |
| disposition.test | RUN | 0.193218 | 0.802667 | 0.830162 | 0.001498 | 0.001947 | 0.001889 |
| disposition_select.test | RUN | 0.154909 | 0.796228 | 0.828768 | 0.001468 | 0.001867 | 0.001934 |
| error.test | RUN | 0.146733 | 0.758594 | 0.746611 | 0.001384 | 0.001848 | 0.001752 |
| error_select.test | RUN | 0.145509 | 0.754211 | 0.735561 | 0.001373 | 0.001834 | 0.001750 |
| file_base.test | LINK | 0.183776 | 0.637242 | 0.767892 | — | — | — |
| file_base_select.test | LINK | 0.183180 | 0.610890 | 0.632383 | — | — | — |
| fwd_first.test | LINK | 0.417214 | 0.942824 | 1.407800 | — | — | — |
| fwd_last.test | LINK | 0.416081 | 0.905135 | 1.397996 | — | — | — |
| generic_basic_endpoint.test | LINK | 0.161047 | 0.374453 | 0.506107 | — | — | — |
| generic_basic_endpoint_select.test | LINK | 0.158371 | 0.374188 | 0.507813 | — | — | — |
| generic_datagram_protocol.test | LINK | 0.634420 | 3.225211 | 2.732941 | — | — | — |
| generic_datagram_protocol_select.test | LINK | 0.618292 | 3.332776 | 2.704462 | — | — | — |
| generic_raw_protocol.test | LINK | 0.637604 | 3.219530 | 2.726008 | — | — | — |
| generic_raw_protocol_select.test | LINK | 0.632137 | 3.347588 | 2.692632 | — | — | — |
| generic_seq_packet_protocol.test | LINK | 0.547716 | 2.729401 | 2.344660 | — | — | — |
| generic_seq_packet_protocol_select.test | LINK | 0.561767 | 2.779861 | 2.317414 | — | — | — |
| generic_stream_protocol.test | LINK | 0.586922 | 2.946788 | 2.511453 | — | — | — |
| generic_stream_protocol_select.test | LINK | 0.593352 | 3.025108 | 2.522221 | — | — | — |
| high_resolution_timer.test | LINK | 0.273086 | 0.555779 | 0.791756 | — | — | — |
| high_resolution_timer_select.test | LINK | 0.279991 | 0.579408 | 0.802952 | — | — | — |
| immediate.test | LINK | 0.186498 | 0.423206 | 0.490794 | — | — | — |
| immediate_select.test | LINK | 0.189517 | 0.425869 | 0.488629 | — | — | — |
| inline_executor.test | RUN | 0.224175 | 0.653108 | 0.725260 | 0.001449 | 0.001443 | 0.001273 |
| inline_executor_select.test | RUN | 0.230541 | 0.660212 | 0.720929 | 0.001420 | 0.001426 | 0.001302 |
| inline_or_executor.test | RUN | 0.354818 | 1.922447 | 1.390418 | 0.001405 | 0.001435 | 0.001351 |
| inline_or_executor_select.test | RUN | 0.367442 | 1.978658 | 1.511307 | 0.001423 | 0.001435 | 0.001326 |
| io_context.test | RUN | 0.519578 | 2.924028 | 2.415664 | 0.002262 | 0.002342 | 0.002274 |
| io_context_select.test | RUN | 0.518387 | 2.887897 | 2.381317 | 0.002184 | 0.002292 | 0.002015 |
| io_context_strand.test | RUN | 0.483166 | 2.478077 | 2.101326 | 0.002311 | 0.002238 | 0.002292 |
| io_context_strand_select.test | RUN | 0.490333 | 2.433546 | 2.098734 | 0.002085 | 0.002088 | 0.002095 |
| ip_address.test | LINK | 0.183858 | 0.589105 | 0.612401 | — | — | — |
| ip_address_select.test | LINK | 0.185994 | 0.587886 | 0.609587 | — | — | — |
| ip_address_v4.test | LINK | 0.191389 | 0.615730 | 0.668280 | — | — | — |
| ip_address_v4_select.test | LINK | 0.191161 | 0.622024 | 0.671685 | — | — | — |
| ip_address_v6.test | LINK | 0.213358 | 0.800958 | 0.822300 | — | — | — |
| ip_address_v6_select.test | LINK | 0.212715 | 0.793251 | 0.825820 | — | — | — |
| ip_basic_endpoint.test | LINK | 0.166829 | 0.396016 | 0.536098 | — | — | — |
| ip_basic_endpoint_select.test | LINK | 0.167559 | 0.390760 | 0.531739 | — | — | — |
| ip_basic_resolver.test | LINK | 0.270920 | 0.568934 | 1.029961 | — | — | — |
| ip_basic_resolver_select.test | LINK | 0.271880 | 0.581294 | 1.084987 | — | — | — |
| ip_basic_resolver_entry.test | LINK | 0.093266 | 0.189697 | 0.397472 | — | — | — |
| ip_basic_resolver_entry_select.test | LINK | 0.092517 | 0.189942 | 0.397807 | — | — | — |
| ip_basic_resolver_iterator.test | LINK | 0.167238 | 0.386849 | 0.899052 | — | — | — |
| ip_basic_resolver_iterator_select.test | LINK | 0.167931 | 0.386577 | 0.867529 | — | — | — |
| ip_basic_resolver_query.test | LINK | 0.158480 | 0.371799 | 0.867570 | — | — | — |
| ip_basic_resolver_query_select.test | LINK | 0.158666 | 0.367013 | 0.876897 | — | — | — |
| ip_host_name.test | RUN | 0.168033 | 0.551363 | 0.957959 | 0.001346 | 0.001456 | 0.001661 |
| ip_host_name_select.test | RUN | 0.168564 | 0.507565 | 0.959522 | 0.001335 | 0.001685 | 0.001595 |
| ip_icmp.test | RUN | 0.762065 | 6.615657 | 5.805411 | 0.001419 | 0.001884 | 0.001484 |
| ip_icmp_select.test | RUN | 0.757402 | 6.755679 | 5.665037 | 0.001410 | 0.001948 | 0.001583 |
| ip_multicast.test | RUN | 0.460680 | 3.387744 | 3.345089 | 0.001504 | 0.001942 | 0.001375 |
| ip_multicast_select.test | RUN | 0.475469 | 3.381786 | 3.302296 | 0.001571 | 0.001848 | 0.001635 |
| ip_resolver_query_base.test | LINK | 0.111111 | 0.313724 | 0.407355 | — | — | — |
| ip_resolver_query_base_select.test | LINK | 0.107843 | 0.309216 | 0.407370 | — | — | — |
| ip_tcp.test | RUN | 1.122135 | 9.776852 | 9.009411 | 0.002006 | 0.002622 | 0.002652 |
| ip_tcp_select.test | RUN | 1.112452 | 10.191058 | 8.708472 | 0.002004 | 0.002638 | 0.002534 |
| ip_udp.test | RUN | 0.814183 | 6.933823 | 6.123500 | 0.001547 | 0.002060 | 0.002012 |
| ip_udp_select.test | RUN | 0.807454 | 7.152622 | 5.994772 | 0.001522 | 0.002070 | 0.001889 |
| ip_unicast.test | RUN | 0.428705 | 3.087021 | 2.877559 | 0.001490 | 0.001902 | 0.001797 |
| ip_unicast_select.test | RUN | 0.428733 | 3.123046 | 2.866073 | 0.001453 | 0.001850 | 0.001815 |
| ip_v6_only.test | RUN | 0.438911 | 3.173803 | 3.020870 | 0.001484 | 0.001907 | 0.001721 |
| ip_v6_only_select.test | RUN | 0.450997 | 3.273081 | 3.001399 | 0.001463 | 0.002040 | 0.001789 |
| is_read_buffered.test | RUN | 0.309980 | 0.989501 | 1.658596 | 0.001379 | 0.001722 | 0.001675 |
| is_read_buffered_select.test | RUN | 0.326686 | 0.993992 | 1.715074 | 0.001394 | 0.001774 | 0.001593 |
| is_write_buffered.test | RUN | 0.313067 | 0.996073 | 1.672384 | 0.001432 | 0.001777 | 0.001720 |
| is_write_buffered_select.test | RUN | 0.318438 | 1.021999 | 1.731327 | 0.001358 | 0.001822 | 0.001752 |
| local_basic_endpoint.test | LINK | 0.159326 | 0.565189 | 0.841856 | — | — | — |
| local_basic_endpoint_select.test | LINK | 0.157600 | 0.555370 | 0.849321 | — | — | — |
| local_connect_pair.test | LINK | 0.410675 | 3.111791 | 2.780023 | — | — | — |
| local_connect_pair_select.test | LINK | 0.409202 | 3.111194 | 2.819369 | — | — | — |
| local_datagram_protocol.test | LINK | 0.546145 | 3.997363 | 3.918110 | — | — | — |
| local_datagram_protocol_select.test | LINK | 0.550172 | 4.050119 | 3.831505 | — | — | — |
| local_seq_packet_protocol.test | LINK | 0.731057 | 3.937357 | 3.857807 | — | — | — |
| local_seq_packet_protocol_select.test | LINK | 0.789715 | 4.007824 | 3.869048 | — | — | — |
| local_stream_protocol.test | LINK | 0.752638 | 3.769023 | 3.677132 | — | — | — |
| local_stream_protocol_select.test | LINK | 0.768617 | 3.811682 | 3.639541 | — | — | — |
| placeholders.test | LINK | 0.150950 | 0.300807 | 0.436407 | — | — | — |
| placeholders_select.test | LINK | 0.146550 | 0.301866 | 0.429413 | — | — | — |
| posix_basic_descriptor.test | LINK | 0.397866 | 0.841946 | 1.206505 | — | — | — |
| posix_basic_descriptor_select.test | LINK | 0.405902 | 0.903636 | 1.485974 | — | — | — |
| posix_basic_stream_descriptor.test | LINK | 0.393311 | 0.838117 | 1.199363 | — | — | — |
| posix_basic_stream_descriptor_select.test | LINK | 0.418822 | 0.902816 | 1.479458 | — | — | — |
| posix_descriptor_base.test | LINK | 0.163868 | 0.311177 | 0.430969 | — | — | — |
| posix_descriptor_base_select.test | LINK | 0.161801 | 0.319380 | 0.425278 | — | — | — |
| posix_stream_descriptor.test | LINK | 0.774341 | 3.799357 | 3.622414 | — | — | — |
| posix_stream_descriptor_select.test | LINK | 0.786935 | 3.897456 | 3.709305 | — | — | — |
| prepend.test | RUN | 0.599078 | 3.130791 | 2.866230 | 0.001606 | 0.001942 | 0.001821 |
| prepend_select.test | RUN | 0.613050 | 3.212761 | 2.849952 | 0.001681 | 0.001816 | 0.001652 |
| random_access_file.test | LINK | 0.360867 | 0.747662 | 0.988400 | — | — | — |
| random_access_file_select.test | LINK | 0.382059 | 0.797678 | 1.245839 | — | — | — |
| read.test | RUN | 1.840571 | 19.293179 | 16.313057 | 0.002586 | 0.002406 | 0.002515 |
| read_select.test | RUN | 1.861527 | 19.227893 | 16.526781 | 0.002477 | 0.002296 | 0.002678 |
| read_at.test | RUN | 1.841699 | 26.952111 | 22.557367 | 0.003048 | 0.003053 | 0.002791 |
| read_at_select.test | RUN | 1.868556 | 26.674169 | 22.763158 | 0.002973 | 0.003091 | 0.002756 |
| read_until.test | RUN | 0.996879 | 7.591828 | 5.955553 | 0.002014 | 0.002082 | 0.002136 |
| read_until_select.test | RUN | 1.007048 | 7.633693 | 6.073845 | 0.001921 | 0.001868 | 0.001802 |
| readable_pipe.test | LINK | 0.627446 | 3.147972 | 2.688824 | — | — | — |
| readable_pipe_select.test | LINK | 0.645365 | 3.283361 | 3.092881 | — | — | — |
| recycling_allocator.test | RUN | 0.197993 | 0.489681 | 0.615541 | 0.001492 | 0.001828 | 0.001745 |
| recycling_allocator_select.test | RUN | 0.194420 | 0.482593 | 0.601280 | 0.001740 | 0.001671 | 0.001806 |
| redirect_disposition.test | LINK | 0.803059 | 4.485064 | 4.383315 | — | — | — |
| redirect_disposition_select.test | LINK | 0.810465 | 4.415635 | 4.285882 | — | — | — |
| redirect_error.test | LINK | 0.722294 | 3.673674 | 3.776411 | — | — | — |
| redirect_error_select.test | LINK | 0.725502 | 3.663073 | 3.810548 | — | — | — |
| registered_buffer.test | LINK | 0.190217 | 0.400221 | 0.585774 | — | — | — |
| registered_buffer_select.test | LINK | 0.191145 | 0.399230 | 0.497555 | — | — | — |
| signal_set.test | RUN | 0.597748 | 3.111324 | 2.640439 | 0.001645 | 0.001867 | 0.001766 |
| signal_set_select.test | RUN | 0.623893 | 3.196973 | 2.892545 | 0.001873 | 0.001734 | 0.002082 |
| signal_set_base.test | LINK | 0.164000 | 0.328078 | 0.413820 | — | — | — |
| signal_set_base_select.test | LINK | 0.160753 | 0.328434 | 0.411377 | — | — | — |
| socket_base.test | RUN | 0.762635 | 4.593112 | 4.129575 | 0.002082 | 0.002040 | 0.002089 |
| socket_base_select.test | RUN | 0.759006 | 4.457438 | 4.147929 | 0.002046 | 0.002064 | 0.001958 |
| spawn.test | RUN | 1.212120 | 7.390173 | 6.793358 | 0.001773 | 0.001904 | 0.002084 |
| spawn_select.test | RUN | 1.230836 | 7.358684 | 6.886147 | 0.002158 | 0.001519 | 0.002080 |
| static_thread_pool.test | RUN | 0.350450 | 0.735649 | 0.993418 | 0.001694 | 0.001689 | 0.001873 |
| static_thread_pool_select.test | RUN | 0.368800 | 0.787885 | 1.239554 | 0.001797 | 0.001459 | 0.001380 |
| steady_timer.test | LINK | 0.391934 | 0.824810 | 1.350374 | — | — | — |
| steady_timer_select.test | LINK | 0.400384 | 0.847770 | 1.375631 | — | — | — |
| strand.test | RUN | 0.807300 | 4.180302 | 4.192556 | 0.002392 | 0.002544 | 0.002612 |
| strand_select.test | RUN | 0.807441 | 4.201245 | 4.065693 | 0.002605 | 0.002550 | 0.002668 |
| stream_file.test | LINK | 0.354961 | 0.751913 | 1.017105 | — | — | — |
| stream_file_select.test | LINK | 0.374414 | 0.802192 | 1.299881 | — | — | — |
| streambuf.test | RUN | 0.206886 | 0.616301 | 0.696439 | 0.001400 | 0.001354 | 0.001721 |
| streambuf_select.test | RUN | 0.155469 | 0.606833 | 0.703093 | 0.001281 | 0.001319 | 0.001792 |
| system_timer.test | LINK | 0.504859 | 2.746843 | 2.686693 | — | — | — |
| system_timer_select.test | LINK | 0.505514 | 2.691228 | 2.420655 | — | — | — |
| system_context.test | LINK | 0.238388 | 0.493166 | 0.585849 | — | — | — |
| system_context_select.test | LINK | 0.245305 | 0.527998 | 0.738251 | — | — | — |
| system_executor.test | LINK | 0.313106 | 1.573114 | 1.106638 | — | — | — |
| system_executor_select.test | LINK | 0.318800 | 1.522135 | 1.250875 | — | — | — |
| this_coro.test | LINK | 0.095380 | 0.189117 | 0.228982 | — | — | — |
| this_coro_select.test | LINK | 0.094409 | 0.188440 | 0.228973 | — | — | — |
| thread_pool.test | RUN | 0.365410 | 1.998680 | 1.429827 | 0.002574 | 0.002557 | 0.002556 |
| thread_pool_select.test | RUN | 0.373386 | 2.027746 | 1.573734 | 0.002371 | 0.002459 | 0.002517 |
| time_traits.test | LINK | 0.214748 | 0.536611 | 0.469011 | — | — | — |
| time_traits_select.test | LINK | 0.213166 | 0.548099 | 0.472234 | — | — | — |
| ts_buffer.test | LINK | 0.196769 | 0.644427 | 0.501161 | — | — | — |
| ts_buffer_select.test | LINK | 0.197612 | 0.643111 | 0.499656 | — | — | — |
| ts_executor.test | LINK | 0.276604 | 0.890819 | 1.220941 | — | — | — |
| ts_executor_select.test | LINK | 0.290813 | 0.945520 | 1.589445 | — | — | — |
| ts_internet.test | LINK | 0.301279 | 0.990880 | 1.241034 | — | — | — |
| ts_internet_select.test | LINK | 0.307819 | 1.014300 | 1.621538 | — | — | — |
| ts_io_context.test | LINK | 0.244959 | 0.746062 | 1.009030 | — | — | — |
| ts_io_context_select.test | LINK | 0.253910 | 0.788068 | 1.248753 | — | — | — |
| ts_net.test | LINK | 0.365969 | 1.185798 | 2.014050 | — | — | — |
| ts_net_select.test | LINK | 0.367563 | 1.198620 | 2.020131 | — | — | — |
| ts_netfwd.test | LINK | 0.363783 | 1.175222 | 1.989590 | — | — | — |
| ts_netfwd_select.test | LINK | 0.374648 | 1.208941 | 2.060117 | — | — | — |
| ts_socket.test | LINK | 0.298398 | 0.930792 | 1.550627 | — | — | — |
| ts_socket_select.test | LINK | 0.304112 | 0.949780 | 1.571836 | — | — | — |
| ts_timer.test | LINK | 0.261952 | 0.837737 | 1.319911 | — | — | — |
| ts_timer_select.test | LINK | 0.267642 | 0.801707 | 1.391909 | — | — | — |
| use_awaitable.test | LINK | 0.094399 | 0.287478 | 0.357720 | — | — | — |
| use_awaitable_select.test | LINK | 0.092705 | 0.279706 | 0.338877 | — | — | — |
| wait_traits.test | LINK | 0.095832 | 0.281102 | 0.372038 | — | — | — |
| wait_traits_select.test | LINK | 0.097157 | 0.278372 | 0.349465 | — | — | — |
| windows_basic_object_handle.test | LINK | 0.094522 | 0.283365 | 0.387657 | — | — | — |
| windows_basic_object_handle_select.test | LINK | 0.094797 | 0.283874 | 0.393385 | — | — | — |
| windows_basic_overlapped_handle.test | LINK | 0.095526 | 0.277670 | 0.354053 | — | — | — |
| windows_basic_overlapped_handle_select.test | LINK | 0.093263 | 0.275114 | 0.372772 | — | — | — |
| windows_basic_random_access_handle.test | LINK | 0.093247 | 0.281481 | 0.371426 | — | — | — |
| windows_basic_random_access_handle_select.test | LINK | 0.093498 | 0.281511 | 0.362012 | — | — | — |
| windows_basic_stream_handle.test | LINK | 0.095112 | 0.273364 | 0.366341 | — | — | — |
| windows_basic_stream_handle_select.test | LINK | 0.093599 | 0.270529 | 0.366419 | — | — | — |
| windows_object_handle.test | LINK | 0.253868 | 0.753940 | 0.977195 | — | — | — |
| windows_object_handle_select.test | LINK | 0.274812 | 0.796787 | 1.276564 | — | — | — |
| windows_overlapped_ptr.test | LINK | 0.366276 | 0.811898 | 1.125299 | — | — | — |
| windows_overlapped_ptr_select.test | LINK | 0.385815 | 0.854699 | 1.347747 | — | — | — |
| windows_random_access_handle.test | LINK | 0.360844 | 0.763809 | 0.994372 | — | — | — |
| windows_random_access_handle_select.test | LINK | 0.391905 | 0.810070 | 1.258230 | — | — | — |
| windows_stream_handle.test | LINK | 0.367826 | 0.761916 | 1.027248 | — | — | — |
| windows_stream_handle_select.test | LINK | 0.374768 | 0.819580 | 1.266980 | — | — | — |
| writable_pipe.test | LINK | 0.642984 | 3.122690 | 2.989072 | — | — | — |
| writable_pipe_select.test | LINK | 0.674466 | 3.267468 | 3.166289 | — | — | — |
| write.test | RUN | 1.856048 | 16.197508 | 16.327495 | 0.002564 | 0.002447 | 0.002625 |
| write_select.test | RUN | 1.851269 | 16.244992 | 16.654681 | 0.002443 | 0.002293 | 0.002285 |
| write_at.test | RUN | 1.889248 | 22.294537 | 20.904846 | 0.002400 | 0.002716 | 0.002663 |
| write_at_select.test | RUN | 1.913159 | 22.419712 | 21.341800 | 0.002746 | 0.002638 | 0.002604 |
| any_executor.test | RUN | 0.779028 | 7.606987 | 6.285042 | 0.002514 | 0.002304 | 0.002388 |
| blocking.test | RUN | 1.896979 | 9.289490 | 13.529798 | 0.007412 | 0.007190 | 0.007393 |
| blocking_adaptation.test | RUN | 0.956751 | 4.066469 | 6.142689 | 0.003988 | 0.004236 | 0.004005 |
| context_as.test | RUN | 0.544100 | 2.845577 | 2.054340 | 0.001865 | 0.002048 | 0.001868 |
| executor.test | RUN | 0.142973 | 0.291698 | 0.399212 | 0.001582 | 0.001731 | 0.001763 |
| inline_exception_handling.test | RUN | 1.905585 | 8.492221 | 12.495381 | 0.005175 | 0.005101 | 0.005102 |
| invocable_archetype.test | RUN | 0.135433 | 0.278174 | 0.394281 | 0.001608 | 0.001798 | 0.001688 |
| mapping.test | RUN | 1.862510 | 9.273595 | 13.560454 | 0.005221 | 0.005255 | 0.005246 |
| outstanding_work.test | RUN | 0.843590 | 3.738729 | 6.044102 | 0.004017 | 0.004088 | 0.004558 |
| prefer_only.test | RUN | 0.269582 | 1.489932 | 1.359110 | 0.001610 | 0.001774 | 0.001751 |
| relationship.test | RUN | 0.539270 | 3.802564 | 5.702388 | 0.004028 | 0.004077 | 0.002976 |

## Native runtime replay

Child CPU of successful native tests, with process startup included; GDB and expected-failure executions are correctness-only. Two warmups per compiler, four A/A observations per reference and mirrored ABBA/BAAB blocks. Short tests repeat up to ten times, identically across compilers; CPU is divided by repetitions before summing tests. Executable hashes and output checks stay outside process timers. All runs must pass their original native oracle. Output hashes record equality or variation across the complete campaign; test reports may print times or addresses.

| Reference | CPU speedup ↑ | Paired blocks | A/A speedup |
|---|---:|---|---:|
| gcc | 0.984× | 0.982×, 0.986× | 1.002× |
| clang | 0.982× | 0.983×, 0.981× | 1.008× |

† A/A drift or paired-block spread exceeds 5%; small differences are unresolved. 1375 native tests in the full cohort; 1362 have byte-identical output throughout. [Raw replay evidence in CI artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43357224918).

| Test | Repeats | CPU cppgm | GCC | Clang | Output check |
|---|---:|---:|---:|---:|---|
| any_completion_executor.test | 5 | 0.001595s | 0.001557s | 0.001592s | byte equality observed |
| any_completion_executor_select.test | 5 | 0.001566s | 0.001556s | 0.001563s | byte equality observed |
| any_completion_handler.test | 6 | 0.001284s | 0.001254s | 0.001261s | byte equality observed |
| any_completion_handler_select.test | 8 | 0.001272s | 0.001237s | 0.001252s | byte equality observed |
| any_io_executor.test | 4 | 0.001602s | 0.001539s | 0.001569s | byte equality observed |
| any_io_executor_select.test | 3 | 0.001586s | 0.001572s | 0.001551s | byte equality observed |
| append.test | 9 | 0.001217s | 0.001204s | 0.001210s | byte equality observed |
| append_select.test | 9 | 0.001205s | 0.001184s | 0.001169s | byte equality observed |
| as_tuple.test | 10 | 0.001292s | 0.001276s | 0.001270s | byte equality observed |
| as_tuple_select.test | 10 | 0.001234s | 0.001217s | 0.001218s | byte equality observed |
| bind_allocator.test | 1 | 0.001672s | 0.001643s | 0.001646s | byte equality observed |
| bind_allocator_select.test | 1 | 0.001465s | 0.001392s | 0.001418s | byte equality observed |
| bind_cancellation_slot.test | 10 | 0.001139s | 0.001122s | 0.001138s | byte equality observed |
| bind_cancellation_slot_select.test | 10 | 0.001059s | 0.001064s | 0.001054s | byte equality observed |
| bind_executor.test | 1 | 0.001675s | 0.001641s | 0.001663s | byte equality observed |
| bind_executor_select.test | 1 | 0.001988s | 0.001929s | 0.001955s | byte equality observed |
| bind_immediate_executor.test | 10 | 0.001114s | 0.001111s | 0.001074s | byte equality observed |
| bind_immediate_executor_select.test | 10 | 0.001087s | 0.001082s | 0.001072s | byte equality observed |
| buffer.test | 10 | 0.001028s | 0.001031s | 0.001024s | byte equality observed |
| buffer_select.test | 10 | 0.001026s | 0.001018s | 0.001034s | byte equality observed |
| buffered_read_stream.test | 10 | 0.001355s | 0.001361s | 0.001329s | byte equality observed |
| buffered_read_stream_select.test | 10 | 0.001312s | 0.001285s | 0.001289s | byte equality observed |
| buffered_stream.test | 10 | 0.001338s | 0.001343s | 0.001286s | byte equality observed |
| buffered_stream_select.test | 10 | 0.001295s | 0.001280s | 0.001311s | byte equality observed |
| buffered_write_stream.test | 10 | 0.001308s | 0.001341s | 0.001342s | byte equality observed |
| buffered_write_stream_select.test | 10 | 0.001336s | 0.001313s | 0.001296s | byte equality observed |
| buffers_iterator.test | 10 | 0.001003s | 0.001022s | 0.000995s | byte equality observed |
| buffers_iterator_select.test | 10 | 0.000985s | 0.000990s | 0.000988s | byte equality observed |
| cancel_after.test | 1 | 0.003327s | 0.003265s | 0.003345s | byte equality observed |
| cancel_after_select.test | 1 | 0.002862s | 0.002734s | 0.002739s | byte equality observed |
| cancel_at.test | 1 | 0.003742s | 0.003571s | 0.003544s | byte equality observed |
| cancel_at_select.test | 1 | 0.003545s | 0.003456s | 0.003518s | byte equality observed |
| co_composed.test | 10 | 0.001038s | 0.001044s | 0.001032s | byte equality observed |
| co_composed_select.test | 10 | 0.001012s | 0.001024s | 0.000995s | byte equality observed |
| compose.test | 1 | 0.001678s | 0.001636s | 0.001668s | byte equality observed |
| compose_select.test | 1 | 0.001419s | 0.001370s | 0.001376s | byte equality observed |
| composed.test | 1 | 0.002066s | 0.002028s | 0.002021s | byte equality observed |
| composed_select.test | 1 | 0.001478s | 0.001432s | 0.001468s | byte equality observed |
| config.test | 10 | 0.001138s | 0.001123s | 0.001118s | byte equality observed |
| config_select.test | 10 | 0.001082s | 0.001091s | 0.001079s | byte equality observed |
| connect_pipe.test | 10 | 0.001108s | 0.001102s | 0.001121s | byte equality observed |
| connect_pipe_select.test | 10 | 0.001137s | 0.001103s | 0.001104s | byte equality observed |
| consign.test | 10 | 0.001109s | 0.001107s | 0.001048s | byte equality observed |
| consign_select.test | 10 | 0.001053s | 0.001080s | 0.001020s | byte equality observed |
| deadline_timer.test | 1 | 0.002394s | 0.002308s | 0.002321s | byte equality observed |
| deadline_timer_select.test | 1 | 0.002136s | 0.002093s | 0.002059s | byte equality observed |
| deferred.test | 10 | 0.000996s | 0.000987s | 0.000998s | byte equality observed |
| deferred_select.test | 10 | 0.000961s | 0.000938s | 0.000962s | byte equality observed |
| disposition.test | 10 | 0.001021s | 0.000994s | 0.001017s | byte equality observed |
| disposition_select.test | 10 | 0.001018s | 0.000991s | 0.001014s | byte equality observed |
| error.test | 10 | 0.000993s | 0.000968s | 0.000975s | byte equality observed |
| error_select.test | 10 | 0.000986s | 0.000984s | 0.000985s | byte equality observed |
| inline_executor.test | 10 | 0.000998s | 0.000992s | 0.000989s | byte equality observed |
| inline_executor_select.test | 10 | 0.000993s | 0.000994s | 0.000994s | byte equality observed |
| inline_or_executor.test | 10 | 0.000994s | 0.000986s | 0.000995s | byte equality observed |
| inline_or_executor_select.test | 10 | 0.001007s | 0.000983s | 0.000990s | byte equality observed |
| io_context.test | 1 | 0.001883s | 0.001885s | 0.001895s | byte equality observed |
| io_context_select.test | 1 | 0.002279s | 0.002270s | 0.002248s | byte equality observed |
| io_context_strand.test | 1 | 0.002261s | 0.002253s | 0.002227s | byte equality observed |
| io_context_strand_select.test | 1 | 0.002012s | 0.002006s | 0.002021s | byte equality observed |
| ip_host_name.test | 10 | 0.001013s | 0.001010s | 0.001011s | byte equality observed |
| ip_host_name_select.test | 10 | 0.000961s | 0.000965s | 0.000968s | byte equality observed |
| ip_icmp.test | 10 | 0.000994s | 0.001012s | 0.000991s | byte equality observed |
| ip_icmp_select.test | 10 | 0.001002s | 0.001012s | 0.000983s | byte equality observed |
| ip_multicast.test | 10 | 0.001069s | 0.001080s | 0.001073s | byte equality observed |
| ip_multicast_select.test | 10 | 0.001054s | 0.001056s | 0.001050s | byte equality observed |
| ip_tcp.test | 10 | 0.001476s | 0.001464s | 0.001445s | byte equality observed |
| ip_tcp_select.test | 10 | 0.001411s | 0.001388s | 0.001398s | byte equality observed |
| ip_udp.test | 10 | 0.001106s | 0.001092s | 0.001082s | byte equality observed |
| ip_udp_select.test | 10 | 0.001117s | 0.001082s | 0.001085s | byte equality observed |
| ip_unicast.test | 10 | 0.001056s | 0.001053s | 0.001083s | byte equality observed |
| ip_unicast_select.test | 10 | 0.001042s | 0.001020s | 0.001024s | byte equality observed |
| ip_v6_only.test | 10 | 0.001087s | 0.001086s | 0.001069s | byte equality observed |
| ip_v6_only_select.test | 10 | 0.001059s | 0.001064s | 0.001025s | byte equality observed |
| is_read_buffered.test | 10 | 0.000996s | 0.001038s | 0.000976s | byte equality observed |
| is_read_buffered_select.test | 10 | 0.000995s | 0.001014s | 0.000994s | byte equality observed |
| is_write_buffered.test | 10 | 0.000991s | 0.000997s | 0.001023s | byte equality observed |
| is_write_buffered_select.test | 10 | 0.000998s | 0.000995s | 0.001002s | byte equality observed |
| prepend.test | 10 | 0.001073s | 0.001112s | 0.001088s | byte equality observed |
| prepend_select.test | 10 | 0.001072s | 0.001066s | 0.001073s | byte equality observed |
| read.test | 10 | 0.001512s | 0.001430s | 0.001416s | byte equality observed |
| read_at.test | 10 | 0.001883s | 0.001877s | 0.001685s | byte equality observed |
| read_at_select.test | 10 | 0.001930s | 0.001804s | 0.001683s | byte equality observed |
| read_select.test | 10 | 0.001535s | 0.001431s | 0.001424s | byte equality observed |
| read_until.test | 10 | 0.001224s | 0.001197s | 0.001177s | byte equality observed |
| read_until_select.test | 10 | 0.001222s | 0.001205s | 0.001182s | byte equality observed |
| recycling_allocator.test | 10 | 0.001030s | 0.001031s | 0.001021s | byte equality observed |
| recycling_allocator_select.test | 10 | 0.001015s | 0.001041s | 0.001023s | byte equality observed |
| signal_set.test | 10 | 0.001057s | 0.001078s | 0.001011s | byte equality observed |
| signal_set_select.test | 10 | 0.001041s | 0.001052s | 0.001062s | byte equality observed |
| socket_base.test | 10 | 0.001156s | 0.001154s | 0.001167s | byte equality observed |
| socket_base_select.test | 10 | 0.001125s | 0.001110s | 0.001121s | byte equality observed |
| spawn.test | 10 | 0.001255s | 0.001225s | 0.001232s | byte equality observed |
| spawn_select.test | 10 | 0.001230s | 0.001208s | 0.001211s | byte equality observed |
| static_thread_pool.test | 10 | 0.001036s | 0.001007s | 0.001030s | byte equality observed |
| static_thread_pool_select.test | 10 | 0.000996s | 0.000997s | 0.000988s | byte equality observed |
| strand.test | 1 | 0.001927s | 0.001901s | 0.001950s | byte equality observed |
| strand_select.test | 1 | 0.001621s | 0.001634s | 0.001611s | byte equality observed |
| streambuf.test | 10 | 0.001044s | 0.001063s | 0.001063s | byte equality observed |
| streambuf_select.test | 10 | 0.001053s | 0.001027s | 0.001038s | byte equality observed |
| thread_pool.test | 10 | 0.002194s | 0.002170s | 0.002213s | byte equality observed |
| thread_pool_select.test | 10 | 0.002207s | 0.002200s | 0.002207s | byte equality observed |
| write.test | 10 | 0.001570s | 0.001517s | 0.001496s | byte equality observed |
| write_at.test | 10 | 0.001753s | 0.001599s | 0.001581s | byte equality observed |
| write_at_select.test | 10 | 0.001748s | 0.001615s | 0.001577s | byte equality observed |
| write_select.test | 10 | 0.001575s | 0.001461s | 0.001468s | byte equality observed |

[Raw outcomes, commands and timing observations in CI artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43357224918).

[Comparison records](../results.json) · [Host and toolchains](../metadata.json)
