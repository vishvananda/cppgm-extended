# libs/asio/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| any_completion_executor.test | RUN | 0.625784 | 3.257066 | 2.831181 | 0.002504 | 0.002598 | 0.002419 |
| any_completion_executor_select.test | RUN | 0.637626 | 3.297031 | 2.974280 | 0.002480 | 0.002227 | 0.002198 |
| any_completion_handler.test | RUN | 0.673458 | 3.041212 | 2.558717 | 0.001958 | 0.001997 | 0.002076 |
| any_completion_handler_select.test | RUN | 0.692561 | 3.149280 | 2.735028 | 0.001779 | 0.002100 | 0.001949 |
| any_io_executor.test | RUN | 0.657098 | 3.516067 | 3.042224 | 0.002298 | 0.002622 | 0.002647 |
| any_io_executor_select.test | RUN | 0.665624 | 3.440418 | 3.114119 | 0.002493 | 0.002678 | 0.002210 |
| append.test | RUN | 0.583137 | 2.840754 | 2.257298 | 0.001812 | 0.001938 | 0.001953 |
| append_select.test | RUN | 0.575937 | 2.936671 | 2.253069 | 0.001766 | 0.001999 | 0.001928 |
| as_tuple.test | RUN | 0.770081 | 3.391643 | 3.231145 | 0.001852 | 0.001896 | 0.001777 |
| as_tuple_select.test | RUN | 0.783974 | 3.375279 | 3.158541 | 0.002272 | 0.002074 | 0.001869 |
| associated_allocator.test | LINK | 0.187829 | 0.274918 | 0.335265 | — | — | — |
| associated_allocator_select.test | LINK | 0.181530 | 0.274203 | 0.379635 | — | — | — |
| associated_cancellation_slot.test | LINK | 0.188732 | 0.318552 | 0.366499 | — | — | — |
| associated_cancellation_slot_select.test | LINK | 0.186525 | 0.323629 | 0.388053 | — | — | — |
| associated_executor.test | LINK | 0.271580 | 0.545651 | 0.596892 | — | — | — |
| associated_executor_select.test | LINK | 0.263172 | 0.559268 | 0.605517 | — | — | — |
| associated_immediate_executor.test | LINK | 0.251531 | 0.544422 | 0.570854 | — | — | — |
| associated_immediate_executor_select.test | LINK | 0.249102 | 0.562066 | 0.541603 | — | — | — |
| associator.test | LINK | 0.146387 | 0.232075 | 0.266403 | — | — | — |
| associator_select.test | LINK | 0.143868 | 0.238020 | 0.275582 | — | — | — |
| awaitable.test | LINK | 0.133995 | 0.240547 | 0.270045 | — | — | — |
| awaitable_select.test | LINK | 0.132314 | 0.226858 | 0.274397 | — | — | — |
| basic_datagram_socket.test | LINK | 0.389315 | 0.700534 | 1.031319 | — | — | — |
| basic_datagram_socket_select.test | LINK | 0.399022 | 0.717093 | 1.061246 | — | — | — |
| basic_deadline_timer.test | LINK | 0.483884 | 0.878725 | 1.179851 | — | — | — |
| basic_deadline_timer_select.test | LINK | 0.490332 | 0.893173 | 1.189858 | — | — | — |
| basic_file.test | LINK | 0.136470 | 0.258748 | 0.271006 | — | — | — |
| basic_file_select.test | LINK | 0.136793 | 0.230657 | 0.270616 | — | — | — |
| basic_random_access_file.test | LINK | 0.145430 | 0.216886 | 0.277106 | — | — | — |
| basic_random_access_file_select.test | LINK | 0.146588 | 0.219151 | 0.290389 | — | — | — |
| basic_raw_socket.test | LINK | 0.405777 | 0.763339 | 1.013118 | — | — | — |
| basic_raw_socket_select.test | LINK | 0.422110 | 0.794686 | 1.053449 | — | — | — |
| basic_readable_pipe.test | LINK | 0.398205 | 0.707200 | 0.870181 | — | — | — |
| basic_readable_pipe_select.test | LINK | 0.414818 | 0.747445 | 1.062741 | — | — | — |
| basic_seq_packet_socket.test | LINK | 0.404663 | 0.719085 | 1.061858 | — | — | — |
| basic_seq_packet_socket_select.test | LINK | 0.406053 | 0.734092 | 1.083515 | — | — | — |
| basic_signal_set.test | LINK | 0.380815 | 0.670497 | 0.837968 | — | — | — |
| basic_signal_set_select.test | LINK | 0.394983 | 0.699214 | 1.044170 | — | — | — |
| basic_socket_acceptor.test | LINK | 0.409543 | 0.723410 | 1.110837 | — | — | — |
| basic_socket_acceptor_select.test | LINK | 0.410584 | 0.740342 | 1.102363 | — | — | — |
| basic_stream_file.test | LINK | 0.134338 | 0.232865 | 0.288755 | — | — | — |
| basic_stream_file_select.test | LINK | 0.139082 | 0.231765 | 0.320823 | — | — | — |
| basic_stream_socket.test | LINK | 0.411687 | 0.724880 | 1.059420 | — | — | — |
| basic_stream_socket_select.test | LINK | 0.419176 | 0.737589 | 1.062946 | — | — | — |
| basic_streambuf.test | LINK | 0.204709 | 0.329622 | 0.396901 | — | — | — |
| basic_streambuf_select.test | LINK | 0.203774 | 0.334624 | 0.405675 | — | — | — |
| basic_waitable_timer.test | LINK | 0.387250 | 0.726236 | 0.997002 | — | — | — |
| basic_waitable_timer_select.test | LINK | 0.397124 | 0.699043 | 1.009443 | — | — | — |
| basic_writable_pipe.test | LINK | 0.406158 | 0.689860 | 0.914330 | — | — | — |
| basic_writable_pipe_select.test | LINK | 0.428046 | 0.748152 | 1.088274 | — | — | — |
| bind_allocator.test | RUN | 0.648376 | 2.786391 | 2.379519 | 0.002548 | 0.002146 | 0.002095 |
| bind_allocator_select.test | RUN | 0.650672 | 2.838148 | 2.391131 | 0.002065 | 0.002031 | 0.001698 |
| bind_cancellation_slot.test | RUN | 0.657666 | 2.820609 | 2.440626 | 0.001695 | 0.001710 | 0.001498 |
| bind_cancellation_slot_select.test | RUN | 0.644203 | 2.892399 | 2.346995 | 0.001447 | 0.001562 | 0.001458 |
| bind_executor.test | RUN | 0.501810 | 2.049673 | 1.520144 | 0.002610 | 0.002254 | 0.002149 |
| bind_executor_select.test | RUN | 0.509799 | 2.089939 | 1.479283 | 0.002142 | 0.001908 | 0.001772 |
| bind_immediate_executor.test | RUN | 0.333137 | 1.585842 | 0.915389 | 0.001567 | 0.001551 | 0.001331 |
| bind_immediate_executor_select.test | RUN | 0.345253 | 1.593285 | 1.031834 | 0.002018 | 0.001518 | 0.001326 |
| buffer.test | RUN | 0.210133 | 0.645178 | 0.580560 | 0.001480 | 0.001406 | 0.001326 |
| buffer_select.test | RUN | 0.203778 | 0.636278 | 0.597333 | 0.001492 | 0.001399 | 0.001356 |
| buffer_registration.test | LINK | 0.171882 | 0.367745 | 0.329371 | — | — | — |
| buffer_registration_select.test | LINK | 0.173599 | 0.370568 | 0.327675 | — | — | — |
| buffered_read_stream.test | RUN | 0.685412 | 4.263486 | 3.354444 | 0.002080 | 0.002494 | 0.002442 |
| buffered_read_stream_select.test | RUN | 0.713134 | 4.286087 | 3.312313 | 0.002006 | 0.002479 | 0.002317 |
| buffered_stream.test | RUN | 0.943220 | 6.885865 | 6.302496 | 0.002508 | 0.002339 | 0.002474 |
| buffered_stream_select.test | RUN | 1.002398 | 6.936965 | 6.051908 | 0.002618 | 0.002294 | 0.002082 |
| buffered_write_stream.test | RUN | 1.135725 | 5.952681 | 5.419766 | 0.002477 | 0.002664 | 0.002190 |
| buffered_write_stream_select.test | RUN | 1.148378 | 5.993475 | 5.191577 | 0.002286 | 0.002527 | 0.001952 |
| buffers_iterator.test | RUN | 0.295876 | 0.548711 | 0.845432 | 0.001885 | 0.001852 | 0.001514 |
| buffers_iterator_select.test | RUN | 0.292315 | 0.541842 | 0.846806 | 0.001947 | 0.002096 | 0.001757 |
| cancel_after.test | RUN | 1.018486 | 4.641412 | 3.827182 | 0.004736 | 0.004376 | 0.003723 |
| cancel_after_select.test | RUN | 1.038194 | 4.617374 | 3.804350 | 0.004743 | 0.004201 | 0.003018 |
| cancel_at.test | RUN | 0.991854 | 4.244425 | 3.801886 | 0.003745 | 0.004117 | 0.003148 |
| cancel_at_select.test | RUN | 0.925787 | 4.240760 | 3.747668 | 0.003295 | 0.003580 | 0.003030 |
| cancellation_signal.test | LINK | 0.114209 | 0.201735 | 0.371736 | — | — | — |
| cancellation_signal_select.test | LINK | 0.122394 | 0.198061 | 0.370863 | — | — | — |
| cancellation_state.test | LINK | 0.136406 | 0.195052 | 0.363071 | — | — | — |
| cancellation_state_select.test | LINK | 0.171801 | 0.199634 | 0.364424 | — | — | — |
| cancellation_type.test | LINK | 0.128907 | 0.156693 | 0.270654 | — | — | — |
| cancellation_type_select.test | LINK | 0.132164 | 0.162731 | 0.271317 | — | — | — |
| co_composed.test | RUN | 0.131729 | 0.210953 | 0.261624 | 0.001530 | 0.001710 | 0.001610 |
| co_composed_select.test | RUN | 0.126055 | 0.233574 | 0.262310 | 0.001997 | 0.001724 | 0.001279 |
| co_spawn.test | LINK | 0.128563 | 0.231733 | 0.261099 | — | — | — |
| co_spawn_select.test | LINK | 0.133778 | 0.229829 | 0.258206 | — | — | — |
| completion_condition.test | LINK | 0.175719 | 0.351106 | 0.342529 | — | — | — |
| completion_condition_select.test | LINK | 0.173024 | 0.353186 | 0.342465 | — | — | — |
| compose.test | RUN | 0.747542 | 3.408562 | 3.183122 | 0.003537 | 0.002358 | 0.002271 |
| compose_select.test | RUN | 0.753402 | 3.386182 | 3.153519 | 0.003103 | 0.001880 | 0.002510 |
| composed.test | RUN | 0.757580 | 3.395745 | 3.107455 | 0.002392 | 0.002239 | 0.002024 |
| composed_select.test | RUN | 0.736277 | 3.337906 | 3.078134 | 0.002252 | 0.001936 | 0.002158 |
| config.test | RUN | 0.383250 | 2.156620 | 1.473839 | 0.001996 | 0.001934 | 0.001904 |
| config_select.test | RUN | 0.446237 | 2.245554 | 1.664347 | 0.001980 | 0.002060 | 0.002120 |
| connect.test | LINK | 1.285086 | 7.359671 | 6.710549 | — | — | — |
| connect_select.test | LINK | 1.278780 | 7.414088 | 6.808202 | — | — | — |
| connect_pipe.test | RUN | 0.658523 | 2.870902 | 2.468079 | 0.002022 | 0.001950 | 0.001958 |
| connect_pipe_select.test | RUN | 0.664711 | 2.976214 | 2.629724 | 0.002056 | 0.001977 | 0.001792 |
| consign.test | RUN | 0.597576 | 2.767496 | 2.252289 | 0.002118 | 0.001803 | 0.001857 |
| consign_select.test | RUN | 0.613136 | 2.805942 | 2.233142 | 0.001956 | 0.002061 | 0.001788 |
| coroutine.test | LINK | 0.138989 | 0.302799 | 0.309823 | — | — | — |
| coroutine_select.test | LINK | 0.142307 | 0.291044 | 0.297074 | — | — | — |
| deadline_timer.test | RUN | 0.867622 | 3.591305 | 3.151898 | 0.003814 | 0.003470 | 0.003619 |
| deadline_timer_select.test | RUN | 0.854916 | 3.544780 | 3.165137 | 0.003630 | 0.002662 | 0.003751 |
| deferred.test | RUN | 0.145213 | 0.244703 | 0.309227 | 0.001366 | 0.001857 | 0.001459 |
| deferred_select.test | RUN | 0.150489 | 0.255277 | 0.299156 | 0.001785 | 0.001903 | 0.001841 |
| detached.test | LINK | 0.169751 | 0.281365 | 0.334560 | — | — | — |
| detached_select.test | LINK | 0.177228 | 0.279581 | 0.333673 | — | — | — |
| disposition.test | RUN | 0.155401 | 0.664831 | 0.611006 | 0.001704 | 0.001916 | 0.001901 |
| disposition_select.test | RUN | 0.158285 | 0.665349 | 0.633865 | 0.001646 | 0.002008 | 0.002000 |
| error.test | RUN | 0.155121 | 0.682091 | 0.547166 | 0.001469 | 0.001886 | 0.001632 |
| error_select.test | RUN | 0.156100 | 0.631158 | 0.559418 | 0.001342 | 0.001893 | 0.001872 |
| file_base.test | LINK | 0.172998 | 0.532092 | 0.536631 | — | — | — |
| file_base_select.test | LINK | 0.185013 | 0.521811 | 0.488154 | — | — | — |
| fwd_first.test | LINK | 0.430758 | 0.816071 | 1.160768 | — | — | — |
| fwd_last.test | LINK | 0.421473 | 0.744945 | 1.171441 | — | — | — |
| generic_basic_endpoint.test | LINK | 0.171897 | 0.317772 | 0.432965 | — | — | — |
| generic_basic_endpoint_select.test | LINK | 0.173293 | 0.319912 | 0.436130 | — | — | — |
| generic_datagram_protocol.test | LINK | 0.623005 | 2.759295 | 2.402772 | — | — | — |
| generic_datagram_protocol_select.test | LINK | 0.602257 | 2.861794 | 2.399890 | — | — | — |
| generic_raw_protocol.test | LINK | 0.597386 | 2.906835 | 2.315734 | — | — | — |
| generic_raw_protocol_select.test | LINK | 0.585677 | 3.012640 | 2.296986 | — | — | — |
| generic_seq_packet_protocol.test | LINK | 0.529682 | 2.423366 | 2.053985 | — | — | — |
| generic_seq_packet_protocol_select.test | LINK | 0.530988 | 2.430722 | 2.023228 | — | — | — |
| generic_stream_protocol.test | LINK | 0.546746 | 2.608622 | 2.178937 | — | — | — |
| generic_stream_protocol_select.test | LINK | 0.535063 | 2.709991 | 2.136507 | — | — | — |
| high_resolution_timer.test | LINK | 0.270759 | 0.525588 | 0.640211 | — | — | — |
| high_resolution_timer_select.test | LINK | 0.280181 | 0.485143 | 0.647887 | — | — | — |
| immediate.test | LINK | 0.173404 | 0.356245 | 0.419053 | — | — | — |
| immediate_select.test | LINK | 0.180700 | 0.362641 | 0.425593 | — | — | — |
| inline_executor.test | RUN | 0.220668 | 0.562147 | 0.629113 | 0.001492 | 0.002062 | 0.001566 |
| inline_executor_select.test | RUN | 0.224214 | 0.563185 | 0.620338 | 0.001406 | 0.001447 | 0.001345 |
| inline_or_executor.test | RUN | 0.334453 | 1.651148 | 1.177952 | 0.001465 | 0.001426 | 0.001444 |
| inline_or_executor_select.test | RUN | 0.339506 | 1.700001 | 1.259646 | 0.001318 | 0.001464 | 0.001488 |
| io_context.test | RUN | 0.468173 | 2.567555 | 2.034536 | 0.002592 | 0.002379 | 0.002479 |
| io_context_select.test | RUN | 0.477260 | 2.527033 | 2.048667 | 0.002310 | 0.002208 | 0.002572 |
| io_context_strand.test | RUN | 0.463837 | 2.152663 | 1.855938 | 0.002616 | 0.002320 | 0.002690 |
| io_context_strand_select.test | RUN | 0.474024 | 2.120222 | 1.878357 | 0.002404 | 0.002199 | 0.002639 |
| ip_address.test | LINK | 0.195485 | 0.496470 | 0.474181 | — | — | — |
| ip_address_select.test | LINK | 0.198774 | 0.497348 | 0.488242 | — | — | — |
| ip_address_v4.test | LINK | 0.179689 | 0.504546 | 0.753498 | — | — | — |
| ip_address_v4_select.test | LINK | 0.182789 | 0.505668 | 0.862724 | — | — | — |
| ip_address_v6.test | LINK | 0.207581 | 0.659746 | 0.988282 | — | — | — |
| ip_address_v6_select.test | LINK | 0.207332 | 0.652613 | 1.068797 | — | — | — |
| ip_basic_endpoint.test | LINK | 0.170340 | 0.325874 | 0.667382 | — | — | — |
| ip_basic_endpoint_select.test | LINK | 0.175987 | 0.319670 | 0.661293 | — | — | — |
| ip_basic_resolver.test | LINK | 0.259540 | 0.643446 | 1.012370 | — | — | — |
| ip_basic_resolver_select.test | LINK | 0.264795 | 0.585265 | 1.002882 | — | — | — |
| ip_basic_resolver_entry.test | LINK | 0.090382 | 0.228074 | 0.286093 | — | — | — |
| ip_basic_resolver_entry_select.test | LINK | 0.094958 | 0.233526 | 0.290088 | — | — | — |
| ip_basic_resolver_iterator.test | LINK | 0.168835 | 0.466500 | 0.654080 | — | — | — |
| ip_basic_resolver_iterator_select.test | LINK | 0.169432 | 0.457559 | 0.625755 | — | — | — |
| ip_basic_resolver_query.test | LINK | 0.156757 | 0.446856 | 0.621535 | — | — | — |
| ip_basic_resolver_query_select.test | LINK | 0.161563 | 0.434966 | 0.596613 | — | — | — |
| ip_host_name.test | RUN | 0.165734 | 0.607249 | 0.705270 | 0.001430 | 0.001933 | 0.001979 |
| ip_host_name_select.test | RUN | 0.165299 | 0.618890 | 0.701625 | 0.001363 | 0.001884 | 0.001896 |
| ip_icmp.test | RUN | 0.737003 | 5.873604 | 4.751559 | 0.001541 | 0.001852 | 0.001638 |
| ip_icmp_select.test | RUN | 0.745504 | 6.047170 | 4.576718 | 0.001449 | 0.001924 | 0.001442 |
| ip_multicast.test | RUN | 0.440459 | 3.008275 | 2.540066 | 0.001696 | 0.002166 | 0.001777 |
| ip_multicast_select.test | RUN | 0.461476 | 2.982289 | 2.506447 | 0.001691 | 0.002026 | 0.001983 |
| ip_resolver_query_base.test | LINK | 0.115264 | 0.248056 | 0.281792 | — | — | — |
| ip_resolver_query_base_select.test | LINK | 0.111274 | 0.266901 | 0.278132 | — | — | — |
| ip_tcp.test | RUN | 1.145267 | 8.785003 | 7.432777 | 0.002164 | 0.002680 | 0.002597 |
| ip_tcp_select.test | RUN | 1.132915 | 9.157178 | 7.141844 | 0.002018 | 0.002790 | 0.002286 |
| ip_udp.test | RUN | 0.780636 | 6.337842 | 5.065942 | 0.001790 | 0.002108 | 0.001866 |
| ip_udp_select.test | RUN | 0.775744 | 6.491129 | 4.954622 | 0.001592 | 0.002268 | 0.002010 |
| ip_unicast.test | RUN | 0.421062 | 2.660931 | 2.243681 | 0.001731 | 0.002022 | 0.002038 |
| ip_unicast_select.test | RUN | 0.424775 | 2.707302 | 2.240923 | 0.001446 | 0.002098 | 0.001618 |
| ip_v6_only.test | RUN | 0.438400 | 2.821862 | 2.313630 | 0.002779 | 0.002104 | 0.001652 |
| ip_v6_only_select.test | RUN | 0.436199 | 2.926738 | 2.297091 | 0.001675 | 0.001931 | 0.001501 |
| is_read_buffered.test | RUN | 0.321499 | 0.851158 | 1.165756 | 0.001578 | 0.001866 | 0.001332 |
| is_read_buffered_select.test | RUN | 0.329560 | 0.881061 | 1.211666 | 0.001488 | 0.001876 | 0.001509 |
| is_write_buffered.test | RUN | 0.397160 | 0.836464 | 1.174751 | 0.001635 | 0.002100 | 0.001824 |
| is_write_buffered_select.test | RUN | 0.387355 | 0.859706 | 1.205153 | 0.001346 | 0.001696 | 0.001850 |
| local_basic_endpoint.test | LINK | 0.168662 | 0.457187 | 0.610075 | — | — | — |
| local_basic_endpoint_select.test | LINK | 0.168922 | 0.466815 | 0.608644 | — | — | — |
| local_connect_pair.test | LINK | 0.393437 | 2.729811 | 2.217701 | — | — | — |
| local_connect_pair_select.test | LINK | 0.397151 | 2.678257 | 2.194025 | — | — | — |
| local_datagram_protocol.test | LINK | 0.505790 | 3.578648 | 3.146406 | — | — | — |
| local_datagram_protocol_select.test | LINK | 0.519151 | 3.618055 | 3.109595 | — | — | — |
| local_seq_packet_protocol.test | LINK | 0.605734 | 3.449303 | 3.122370 | — | — | — |
| local_seq_packet_protocol_select.test | LINK | 0.648796 | 3.470984 | 3.131058 | — | — | — |
| local_stream_protocol.test | LINK | 0.759022 | 3.208031 | 2.967086 | — | — | — |
| local_stream_protocol_select.test | LINK | 0.767611 | 3.218351 | 2.902011 | — | — | — |
| placeholders.test | LINK | 0.158835 | 0.238431 | 0.310087 | — | — | — |
| placeholders_select.test | LINK | 0.178939 | 0.231613 | 0.310388 | — | — | — |
| posix_basic_descriptor.test | LINK | 0.395077 | 0.674786 | 0.921971 | — | — | — |
| posix_basic_descriptor_select.test | LINK | 0.413094 | 0.728236 | 1.135329 | — | — | — |
| posix_basic_stream_descriptor.test | LINK | 0.410664 | 0.690875 | 0.891246 | — | — | — |
| posix_basic_stream_descriptor_select.test | LINK | 0.417887 | 0.742375 | 1.063647 | — | — | — |
| posix_descriptor_base.test | LINK | 0.172534 | 0.261360 | 0.301847 | — | — | — |
| posix_descriptor_base_select.test | LINK | 0.167672 | 0.253434 | 0.298655 | — | — | — |
| posix_stream_descriptor.test | LINK | 0.715960 | 3.475637 | 2.866359 | — | — | — |
| posix_stream_descriptor_select.test | LINK | 0.735362 | 3.484992 | 2.900674 | — | — | — |
| prepend.test | RUN | 0.592074 | 2.805899 | 2.253308 | 0.002048 | 0.002074 | 0.001714 |
| prepend_select.test | RUN | 0.581216 | 2.893359 | 2.216189 | 0.002012 | 0.001954 | 0.001878 |
| random_access_file.test | LINK | 0.358470 | 0.633647 | 0.698094 | — | — | — |
| random_access_file_select.test | LINK | 0.356628 | 0.671553 | 0.910900 | — | — | — |
| read.test | RUN | 1.643329 | 16.863339 | 13.068341 | 0.002733 | 0.001958 | 0.002896 |
| read_select.test | RUN | 1.669430 | 17.051961 | 13.146459 | 0.002768 | 0.002371 | 0.002547 |
| read_at.test | RUN | 1.662737 | 25.224289 | 18.977569 | 0.003198 | 0.002593 | 0.002770 |
| read_at_select.test | RUN | 1.693415 | 25.160935 | 18.998806 | 0.003029 | 0.002731 | 0.002884 |
| read_until.test | RUN | 0.944058 | 6.158027 | 5.063383 | 0.003832 | 0.002154 | 0.002072 |
| read_until_select.test | RUN | 0.948068 | 6.248774 | 5.103666 | 0.002291 | 0.002156 | 0.002152 |
| readable_pipe.test | LINK | 0.605124 | 2.702665 | 2.337628 | — | — | — |
| readable_pipe_select.test | LINK | 0.646190 | 2.822347 | 2.528043 | — | — | — |
| recycling_allocator.test | RUN | 0.203656 | 0.384157 | 0.473661 | 0.001707 | 0.001697 | 0.001854 |
| recycling_allocator_select.test | RUN | 0.199592 | 0.399982 | 0.485150 | 0.001810 | 0.003031 | 0.001829 |
| redirect_disposition.test | LINK | 0.845396 | 3.934165 | 3.831334 | — | — | — |
| redirect_disposition_select.test | LINK | 0.852369 | 3.815712 | 3.726339 | — | — | — |
| redirect_error.test | LINK | 0.727267 | 3.217802 | 3.249355 | — | — | — |
| redirect_error_select.test | LINK | 0.735989 | 3.287206 | 3.417918 | — | — | — |
| registered_buffer.test | LINK | 0.193034 | 0.319907 | 0.445190 | — | — | — |
| registered_buffer_select.test | LINK | 0.191113 | 0.306987 | 0.427730 | — | — | — |
| signal_set.test | RUN | 0.567195 | 2.602941 | 2.277498 | 0.002020 | 0.001926 | 0.002080 |
| signal_set_select.test | RUN | 0.594436 | 2.693383 | 2.528004 | 0.001902 | 0.001822 | 0.002084 |
| signal_set_base.test | LINK | 0.154901 | 0.259646 | 0.321095 | — | — | — |
| signal_set_base_select.test | LINK | 0.157409 | 0.252238 | 0.320003 | — | — | — |
| socket_base.test | RUN | 0.776529 | 4.065301 | 3.489443 | 0.002122 | 0.002210 | 0.001944 |
| socket_base_select.test | RUN | 0.787614 | 3.880654 | 3.531558 | 0.002102 | 0.001918 | 0.002122 |
| spawn.test | RUN | 1.269760 | 6.507271 | 6.211503 | 0.002372 | 0.002051 | 0.002311 |
| spawn_select.test | RUN | 1.263006 | 6.491787 | 6.040582 | 0.002302 | 0.002165 | 0.002028 |
| static_thread_pool.test | RUN | 0.334909 | 0.632742 | 0.780679 | 0.001940 | 0.001629 | 0.001977 |
| static_thread_pool_select.test | RUN | 0.364060 | 0.684941 | 0.971703 | 0.001774 | 0.001704 | 0.001931 |
| steady_timer.test | LINK | 0.377672 | 0.711965 | 1.052972 | — | — | — |
| steady_timer_select.test | LINK | 0.395053 | 0.733373 | 1.126243 | — | — | — |
| strand.test | RUN | 0.766884 | 3.641591 | 3.613744 | 0.002803 | 0.002452 | 0.003050 |
| strand_select.test | RUN | 0.775980 | 3.629595 | 3.550578 | 0.002832 | 0.002681 | 0.002261 |
| stream_file.test | LINK | 0.371392 | 0.648896 | 0.810998 | — | — | — |
| stream_file_select.test | LINK | 0.394679 | 0.685894 | 1.032798 | — | — | — |
| streambuf.test | RUN | 0.203041 | 0.410374 | 0.574978 | 0.001354 | 0.002038 | 0.001957 |
| streambuf_select.test | RUN | 0.175964 | 0.359867 | 0.524150 | 0.001511 | 0.001444 | 0.001477 |
| system_timer.test | LINK | 0.524829 | 2.384449 | 2.236238 | — | — | — |
| system_timer_select.test | LINK | 0.515429 | 2.378234 | 2.111694 | — | — | — |
| system_context.test | LINK | 0.245500 | 0.412734 | 0.501639 | — | — | — |
| system_context_select.test | LINK | 0.252458 | 0.440319 | 0.597794 | — | — | — |
| system_executor.test | LINK | 0.323209 | 1.343753 | 0.982607 | — | — | — |
| system_executor_select.test | LINK | 0.321145 | 1.315637 | 1.102209 | — | — | — |
| this_coro.test | LINK | 0.097200 | 0.160196 | 0.193554 | — | — | — |
| this_coro_select.test | LINK | 0.096978 | 0.158932 | 0.189819 | — | — | — |
| thread_pool.test | RUN | 0.370457 | 1.778231 | 1.297977 | 0.002910 | 0.002550 | 0.002772 |
| thread_pool_select.test | RUN | 0.372412 | 1.824159 | 1.441953 | 0.002423 | 0.002481 | 0.002689 |
| time_traits.test | LINK | 0.219054 | 0.324363 | 0.400899 | — | — | — |
| time_traits_select.test | LINK | 0.220917 | 0.327823 | 0.433145 | — | — | — |
| ts_buffer.test | LINK | 0.207854 | 0.373637 | 0.419483 | — | — | — |
| ts_buffer_select.test | LINK | 0.207819 | 0.429680 | 0.416714 | — | — | — |
| ts_executor.test | LINK | 0.288274 | 0.716769 | 0.705132 | — | — | — |
| ts_executor_select.test | LINK | 0.299084 | 0.787886 | 0.983467 | — | — | — |
| ts_internet.test | LINK | 0.314551 | 0.796119 | 1.246216 | — | — | — |
| ts_internet_select.test | LINK | 0.306736 | 0.843878 | 1.262939 | — | — | — |
| ts_io_context.test | LINK | 0.246303 | 0.619867 | 0.776161 | — | — | — |
| ts_io_context_select.test | LINK | 0.254094 | 0.667296 | 0.987428 | — | — | — |
| ts_net.test | LINK | 0.365442 | 1.053327 | 1.506920 | — | — | — |
| ts_net_select.test | LINK | 0.373831 | 1.073059 | 1.561549 | — | — | — |
| ts_netfwd.test | LINK | 0.366162 | 1.036046 | 1.546772 | — | — | — |
| ts_netfwd_select.test | LINK | 0.376440 | 1.014077 | 1.548833 | — | — | — |
| ts_socket.test | LINK | 0.300912 | 0.763206 | 1.178583 | — | — | — |
| ts_socket_select.test | LINK | 0.307019 | 0.789056 | 1.181025 | — | — | — |
| ts_timer.test | LINK | 0.259350 | 0.691582 | 1.024853 | — | — | — |
| ts_timer_select.test | LINK | 0.270201 | 0.751197 | 1.040446 | — | — | — |
| use_awaitable.test | LINK | 0.091221 | 0.231265 | 0.300807 | — | — | — |
| use_awaitable_select.test | LINK | 0.091942 | 0.273158 | 0.297031 | — | — | — |
| wait_traits.test | LINK | 0.092353 | 0.227617 | 0.299253 | — | — | — |
| wait_traits_select.test | LINK | 0.095510 | 0.231664 | 0.269239 | — | — | — |
| windows_basic_object_handle.test | LINK | 0.093757 | 0.225318 | 0.280173 | — | — | — |
| windows_basic_object_handle_select.test | LINK | 0.094140 | 0.225837 | 0.277300 | — | — | — |
| windows_basic_overlapped_handle.test | LINK | 0.090730 | 0.227392 | 0.290358 | — | — | — |
| windows_basic_overlapped_handle_select.test | LINK | 0.090263 | 0.224514 | 0.304254 | — | — | — |
| windows_basic_random_access_handle.test | LINK | 0.099686 | 0.235423 | 0.287932 | — | — | — |
| windows_basic_random_access_handle_select.test | LINK | 0.097370 | 0.235737 | 0.297089 | — | — | — |
| windows_basic_stream_handle.test | LINK | 0.098120 | 0.240067 | 0.296950 | — | — | — |
| windows_basic_stream_handle_select.test | LINK | 0.099085 | 0.240677 | 0.278440 | — | — | — |
| windows_object_handle.test | LINK | 0.294784 | 0.629738 | 0.767418 | — | — | — |
| windows_object_handle_select.test | LINK | 0.308264 | 0.667387 | 1.010283 | — | — | — |
| windows_overlapped_ptr.test | LINK | 0.371179 | 0.649805 | 0.891425 | — | — | — |
| windows_overlapped_ptr_select.test | LINK | 0.386274 | 0.704627 | 1.095495 | — | — | — |
| windows_random_access_handle.test | LINK | 0.360720 | 0.604763 | 0.802300 | — | — | — |
| windows_random_access_handle_select.test | LINK | 0.377545 | 0.637698 | 0.992099 | — | — | — |
| windows_stream_handle.test | LINK | 0.367008 | 0.602903 | 0.804800 | — | — | — |
| windows_stream_handle_select.test | LINK | 0.376875 | 0.669328 | 0.990100 | — | — | — |
| writable_pipe.test | LINK | 0.625410 | 2.702614 | 2.707218 | — | — | — |
| writable_pipe_select.test | LINK | 0.666135 | 2.802546 | 2.823343 | — | — | — |
| write.test | RUN | 1.728372 | 13.794125 | 14.513698 | 0.002737 | 0.002363 | 0.003844 |
| write_select.test | RUN | 1.745510 | 13.819436 | 14.599342 | 0.002650 | 0.002415 | 0.002502 |
| write_at.test | RUN | 1.717805 | 20.690443 | 18.402615 | 0.003079 | 0.002570 | 0.002881 |
| write_at_select.test | RUN | 1.747338 | 20.567765 | 18.370822 | 0.002886 | 0.002542 | 0.002667 |
| any_executor.test | RUN | 0.684689 | 6.681008 | 5.384454 | 0.002706 | 0.002403 | 0.002468 |
| blocking.test | RUN | 1.539415 | 8.136833 | 10.448023 | 0.007637 | 0.006381 | 0.006592 |
| blocking_adaptation.test | RUN | 0.824955 | 3.359428 | 4.828382 | 0.004441 | 0.003532 | 0.003853 |
| context_as.test | RUN | 0.517435 | 2.310684 | 1.733040 | 0.002225 | 0.002085 | 0.002018 |
| executor.test | RUN | 0.156673 | 0.239926 | 0.305812 | 0.001899 | 0.001746 | 0.001941 |
| inline_exception_handling.test | RUN | 1.401613 | 7.008557 | 9.909859 | 0.006552 | 0.004617 | 0.004833 |
| invocable_archetype.test | RUN | 0.136912 | 0.231995 | 0.289269 | 0.001770 | 0.001674 | 0.001872 |
| mapping.test | RUN | 1.444819 | 7.112916 | 10.390209 | 0.004938 | 0.004785 | 0.004965 |
| outstanding_work.test | RUN | 0.688787 | 3.157554 | 4.574982 | 0.003800 | 0.003740 | 0.003838 |
| prefer_only.test | RUN | 0.284353 | 1.194624 | 1.022202 | 0.002150 | 0.002154 | 0.001871 |
| relationship.test | RUN | 0.573851 | 2.697292 | 4.594287 | 0.002720 | 0.002658 | 0.003410 |

## Native runtime replay

Child CPU of successful native tests, with process startup included; GDB and expected-failure executions are correctness-only. Two warmups per compiler, four A/A observations per reference and mirrored ABBA/BAAB blocks. Short tests repeat up to ten times, identically across compilers; CPU is divided by repetitions before summing tests. Executable hashes and output checks stay outside process timers. All runs must pass their original native oracle. Output hashes record equality or variation across the complete campaign; test reports may print times or addresses.

| Reference | CPU speedup ↑ | Paired blocks | A/A speedup |
|---|---:|---|---:|
| gcc | 0.976× | 0.970×, 0.982× | 1.027× |
| clang | 0.963×† | 0.933×, 0.993× | 1.010× |

† A/A drift or paired-block spread exceeds 5%; small differences are unresolved. 1375 native tests in the full cohort; 1362 have byte-identical output throughout. [Manifest](https://github.com/vishvananda/cppgm-extended/actions/runs/42700386045), [paired observations](https://github.com/vishvananda/cppgm-extended/actions/runs/42700386045) and [per-test summaries](https://github.com/vishvananda/cppgm-extended/actions/runs/42700386045).

| Test | Repeats | CPU cppgm | GCC | Clang | Output check |
|---|---:|---:|---:|---:|---|
| any_completion_executor.test | 2 | 0.001610s | 0.001526s | 0.001542s | byte equality observed |
| any_completion_executor_select.test | 5 | 0.001571s | 0.001536s | 0.001527s | byte equality observed |
| any_completion_handler.test | 9 | 0.001292s | 0.001247s | 0.001223s | byte equality observed |
| any_completion_handler_select.test | 9 | 0.001277s | 0.001254s | 0.001217s | byte equality observed |
| any_io_executor.test | 6 | 0.001535s | 0.001519s | 0.001502s | byte equality observed |
| any_io_executor_select.test | 4 | 0.001549s | 0.001530s | 0.001474s | byte equality observed |
| append.test | 10 | 0.001216s | 0.001192s | 0.001204s | byte equality observed |
| append_select.test | 7 | 0.001181s | 0.001166s | 0.001158s | byte equality observed |
| as_tuple.test | 10 | 0.001281s | 0.001247s | 0.001238s | byte equality observed |
| as_tuple_select.test | 9 | 0.001241s | 0.001198s | 0.001186s | byte equality observed |
| bind_allocator.test | 1 | 0.001665s | 0.001664s | 0.001634s | byte equality observed |
| bind_allocator_select.test | 1 | 0.001413s | 0.001358s | 0.001298s | byte equality observed |
| bind_cancellation_slot.test | 10 | 0.001161s | 0.001227s | 0.001158s | byte equality observed |
| bind_cancellation_slot_select.test | 10 | 0.001035s | 0.001074s | 0.001024s | byte equality observed |
| bind_executor.test | 1 | 0.001678s | 0.001785s | 0.001600s | byte equality observed |
| bind_executor_select.test | 1 | 0.002047s | 0.002110s | 0.001942s | byte equality observed |
| bind_immediate_executor.test | 10 | 0.001079s | 0.001102s | 0.001073s | byte equality observed |
| bind_immediate_executor_select.test | 10 | 0.001040s | 0.001041s | 0.001029s | byte equality observed |
| buffer.test | 10 | 0.001002s | 0.001016s | 0.000975s | byte equality observed |
| buffer_select.test | 10 | 0.000998s | 0.001007s | 0.000972s | byte equality observed |
| buffered_read_stream.test | 10 | 0.001311s | 0.001290s | 0.001295s | byte equality observed |
| buffered_read_stream_select.test | 10 | 0.001296s | 0.001225s | 0.001282s | byte equality observed |
| buffered_stream.test | 10 | 0.001285s | 0.001283s | 0.001265s | byte equality observed |
| buffered_stream_select.test | 10 | 0.001287s | 0.001234s | 0.001255s | byte equality observed |
| buffered_write_stream.test | 10 | 0.001299s | 0.001286s | 0.001252s | byte equality observed |
| buffered_write_stream_select.test | 10 | 0.001268s | 0.001252s | 0.001248s | byte equality observed |
| buffers_iterator.test | 10 | 0.000986s | 0.000953s | 0.000975s | byte equality observed |
| buffers_iterator_select.test | 10 | 0.000970s | 0.000957s | 0.000963s | byte equality observed |
| cancel_after.test | 1 | 0.003366s | 0.003372s | 0.002828s | byte equality observed |
| cancel_after_select.test | 1 | 0.002828s | 0.002721s | 0.002355s | byte equality observed |
| cancel_at.test | 1 | 0.003910s | 0.003973s | 0.003042s | byte equality observed |
| cancel_at_select.test | 1 | 0.003598s | 0.003772s | 0.003177s | byte equality observed |
| co_composed.test | 10 | 0.001021s | 0.001004s | 0.001023s | byte equality observed |
| co_composed_select.test | 10 | 0.000975s | 0.000980s | 0.000977s | byte equality observed |
| compose.test | 1 | 0.001680s | 0.001751s | 0.001534s | byte equality observed |
| compose_select.test | 1 | 0.001382s | 0.001392s | 0.001306s | byte equality observed |
| composed.test | 1 | 0.002148s | 0.002168s | 0.001824s | byte equality observed |
| composed_select.test | 1 | 0.001482s | 0.001472s | 0.001386s | byte equality observed |
| config.test | 10 | 0.001087s | 0.001059s | 0.001038s | byte equality observed |
| config_select.test | 10 | 0.001043s | 0.001037s | 0.001012s | byte equality observed |
| connect_pipe.test | 10 | 0.001087s | 0.001059s | 0.001032s | byte equality observed |
| connect_pipe_select.test | 10 | 0.001063s | 0.001035s | 0.001017s | byte equality observed |
| consign.test | 10 | 0.001050s | 0.001051s | 0.001046s | byte equality observed |
| consign_select.test | 10 | 0.001028s | 0.001016s | 0.000999s | byte equality observed |
| deadline_timer.test | 1 | 0.002357s | 0.002351s | 0.002399s | byte equality observed |
| deadline_timer_select.test | 1 | 0.002113s | 0.002165s | 0.002110s | byte equality observed |
| deferred.test | 7 | 0.001004s | 0.000983s | 0.000990s | byte equality observed |
| deferred_select.test | 9 | 0.000930s | 0.000904s | 0.000928s | byte equality observed |
| disposition.test | 9 | 0.000991s | 0.000968s | 0.000985s | byte equality observed |
| disposition_select.test | 10 | 0.000994s | 0.000978s | 0.000964s | byte equality observed |
| error.test | 10 | 0.000972s | 0.000954s | 0.000927s | byte equality observed |
| error_select.test | 10 | 0.000965s | 0.000940s | 0.000960s | byte equality observed |
| inline_executor.test | 10 | 0.000964s | 0.000948s | 0.000950s | byte equality observed |
| inline_executor_select.test | 10 | 0.000961s | 0.000949s | 0.000950s | byte equality observed |
| inline_or_executor.test | 10 | 0.000955s | 0.000959s | 0.000958s | byte equality observed |
| inline_or_executor_select.test | 10 | 0.000956s | 0.000962s | 0.000968s | byte equality observed |
| io_context.test | 1 | 0.001874s | 0.001952s | 0.001873s | byte equality observed |
| io_context_select.test | 1 | 0.002246s | 0.002285s | 0.002246s | byte equality observed |
| io_context_strand.test | 1 | 0.002286s | 0.002341s | 0.002280s | byte equality observed |
| io_context_strand_select.test | 1 | 0.002000s | 0.002031s | 0.002022s | byte equality observed |
| ip_host_name.test | 6 | 0.000998s | 0.001005s | 0.001029s | byte equality observed |
| ip_host_name_select.test | 8 | 0.000942s | 0.000915s | 0.000945s | byte equality observed |
| ip_icmp.test | 9 | 0.000959s | 0.000953s | 0.000938s | byte equality observed |
| ip_icmp_select.test | 9 | 0.000964s | 0.000958s | 0.000945s | byte equality observed |
| ip_multicast.test | 6 | 0.001048s | 0.001038s | 0.001025s | byte equality observed |
| ip_multicast_select.test | 10 | 0.001025s | 0.001007s | 0.000994s | byte equality observed |
| ip_tcp.test | 10 | 0.001436s | 0.001396s | 0.001409s | byte equality observed |
| ip_tcp_select.test | 10 | 0.001361s | 0.001337s | 0.001342s | byte equality observed |
| ip_udp.test | 10 | 0.001065s | 0.001047s | 0.001046s | byte equality observed |
| ip_udp_select.test | 10 | 0.001051s | 0.001030s | 0.001027s | byte equality observed |
| ip_unicast.test | 10 | 0.001022s | 0.000996s | 0.001001s | byte equality observed |
| ip_unicast_select.test | 10 | 0.001009s | 0.000984s | 0.000989s | byte equality observed |
| ip_v6_only.test | 10 | 0.001020s | 0.001000s | 0.001002s | byte equality observed |
| ip_v6_only_select.test | 10 | 0.001001s | 0.000983s | 0.000986s | byte equality observed |
| is_read_buffered.test | 10 | 0.000946s | 0.000908s | 0.000929s | byte equality observed |
| is_read_buffered_select.test | 10 | 0.000945s | 0.000955s | 0.000933s | byte equality observed |
| is_write_buffered.test | 10 | 0.000944s | 0.000980s | 0.000930s | byte equality observed |
| is_write_buffered_select.test | 10 | 0.000935s | 0.000954s | 0.000932s | byte equality observed |
| prepend.test | 10 | 0.001015s | 0.001039s | 0.001008s | byte equality observed |
| prepend_select.test | 10 | 0.001000s | 0.001022s | 0.001013s | byte equality observed |
| read.test | 10 | 0.001421s | 0.001360s | 0.001378s | byte equality observed |
| read_at.test | 10 | 0.001732s | 0.001605s | 0.001511s | byte equality observed |
| read_at_select.test | 10 | 0.001751s | 0.001626s | 0.001517s | byte equality observed |
| read_select.test | 10 | 0.001464s | 0.001351s | 0.001333s | byte equality observed |
| read_until.test | 10 | 0.001200s | 0.001160s | 0.001171s | byte equality observed |
| read_until_select.test | 10 | 0.001187s | 0.001136s | 0.001179s | byte equality observed |
| recycling_allocator.test | 10 | 0.000988s | 0.000985s | 0.001010s | byte equality observed |
| recycling_allocator_select.test | 10 | 0.000973s | 0.000969s | 0.001017s | byte equality observed |
| signal_set.test | 10 | 0.001011s | 0.000983s | 0.001004s | byte equality observed |
| signal_set_select.test | 10 | 0.001022s | 0.000999s | 0.001012s | byte equality observed |
| socket_base.test | 10 | 0.001122s | 0.001104s | 0.001098s | byte equality observed |
| socket_base_select.test | 10 | 0.001103s | 0.001080s | 0.001110s | byte equality observed |
| spawn.test | 10 | 0.001230s | 0.001172s | 0.001188s | byte equality observed |
| spawn_select.test | 10 | 0.001208s | 0.001162s | 0.001198s | byte equality observed |
| static_thread_pool.test | 10 | 0.001006s | 0.000978s | 0.000998s | byte equality observed |
| static_thread_pool_select.test | 10 | 0.000964s | 0.000926s | 0.000936s | byte equality observed |
| strand.test | 1 | 0.001947s | 0.001971s | 0.001911s | byte equality observed |
| strand_select.test | 1 | 0.001626s | 0.001592s | 0.001734s | byte equality observed |
| streambuf.test | 10 | 0.001023s | 0.001056s | 0.001001s | byte equality observed |
| streambuf_select.test | 10 | 0.001011s | 0.001023s | 0.000977s | byte equality observed |
| thread_pool.test | 10 | 0.002036s | 0.002025s | 0.001997s | byte equality observed |
| thread_pool_select.test | 10 | 0.002004s | 0.002006s | 0.001988s | byte equality observed |
| write.test | 10 | 0.001508s | 0.001410s | 0.001391s | byte equality observed |
| write_at.test | 10 | 0.001637s | 0.001526s | 0.001443s | byte equality observed |
| write_at_select.test | 10 | 0.001617s | 0.001504s | 0.001460s | byte equality observed |
| write_select.test | 10 | 0.001524s | 0.001405s | 0.001364s | byte equality observed |

Compiler command and timing observations: [cppgm](https://github.com/vishvananda/cppgm-extended/actions/runs/42700386045) · [gcc](https://github.com/vishvananda/cppgm-extended/actions/runs/42700386045) · [clang](https://github.com/vishvananda/cppgm-extended/actions/runs/42700386045).

[Reviewed outcomes and per-test records](../results.json) · [Host and toolchains](../metadata.json)
