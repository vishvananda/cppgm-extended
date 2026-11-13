# libs/asio/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| any_completion_executor.test | RUN | 0.631811 | 3.763267 | 3.772256 | 0.002523 | 0.002614 | 0.002714 |
| any_completion_executor_select.test | RUN | 0.688928 | 3.762724 | 3.847333 | 0.002312 | 0.002498 | 0.002456 |
| any_completion_handler.test | RUN | 0.717584 | 3.414362 | 3.446893 | 0.002004 | 0.001997 | 0.002104 |
| any_completion_handler_select.test | RUN | 0.736060 | 3.533241 | 3.610634 | 0.001736 | 0.002107 | 0.002140 |
| any_io_executor.test | RUN | 0.681789 | 4.091330 | 4.066959 | 0.002542 | 0.002658 | 0.002762 |
| any_io_executor_select.test | RUN | 0.704371 | 3.970500 | 4.271397 | 0.002094 | 0.002689 | 0.002438 |
| append.test | RUN | 0.637640 | 3.245774 | 2.973835 | 0.001599 | 0.001978 | 0.002112 |
| append_select.test | RUN | 0.636328 | 3.333702 | 3.032443 | 0.001833 | 0.001740 | 0.001830 |
| as_tuple.test | RUN | 0.796265 | 3.900940 | 4.277501 | 0.002043 | 0.001808 | 0.002016 |
| as_tuple_select.test | RUN | 0.809625 | 3.922001 | 4.234215 | 0.001714 | 0.001927 | 0.001760 |
| associated_allocator.test | LINK | 0.179031 | 0.360085 | 0.458507 | — | — | — |
| associated_allocator_select.test | LINK | 0.181214 | 0.348615 | 0.482376 | — | — | — |
| associated_cancellation_slot.test | LINK | 0.191284 | 0.364259 | 0.518885 | — | — | — |
| associated_cancellation_slot_select.test | LINK | 0.190010 | 0.371199 | 0.520018 | — | — | — |
| associated_executor.test | LINK | 0.266848 | 0.628723 | 0.827943 | — | — | — |
| associated_executor_select.test | LINK | 0.268508 | 0.640051 | 0.834934 | — | — | — |
| associated_immediate_executor.test | LINK | 0.247053 | 0.596322 | 0.763731 | — | — | — |
| associated_immediate_executor_select.test | LINK | 0.256120 | 0.613317 | 0.744048 | — | — | — |
| associator.test | LINK | 0.141160 | 0.295465 | 0.405400 | — | — | — |
| associator_select.test | LINK | 0.140560 | 0.292356 | 0.389882 | — | — | — |
| awaitable.test | LINK | 0.139529 | 0.293061 | 0.391533 | — | — | — |
| awaitable_select.test | LINK | 0.142645 | 0.310824 | 0.392447 | — | — | — |
| basic_datagram_socket.test | LINK | 0.431021 | 0.952173 | 1.491238 | — | — | — |
| basic_datagram_socket_select.test | LINK | 0.432162 | 0.989160 | 1.526691 | — | — | — |
| basic_deadline_timer.test | LINK | 0.515533 | 1.124386 | 1.691379 | — | — | — |
| basic_deadline_timer_select.test | LINK | 0.516018 | 1.139469 | 1.694164 | — | — | — |
| basic_file.test | LINK | 0.140403 | 0.286816 | 0.402340 | — | — | — |
| basic_file_select.test | LINK | 0.141319 | 0.301701 | 0.402093 | — | — | — |
| basic_random_access_file.test | LINK | 0.141285 | 0.279923 | 0.390816 | — | — | — |
| basic_random_access_file_select.test | LINK | 0.150144 | 0.278992 | 0.405744 | — | — | — |
| basic_raw_socket.test | LINK | 0.435638 | 0.899400 | 1.496354 | — | — | — |
| basic_raw_socket_select.test | LINK | 0.442356 | 0.922143 | 1.525967 | — | — | — |
| basic_readable_pipe.test | LINK | 0.416901 | 0.861976 | 1.289226 | — | — | — |
| basic_readable_pipe_select.test | LINK | 0.435867 | 0.919211 | 1.577861 | — | — | — |
| basic_seq_packet_socket.test | LINK | 0.422233 | 0.899095 | 1.504732 | — | — | — |
| basic_seq_packet_socket_select.test | LINK | 0.431905 | 0.929413 | 1.526356 | — | — | — |
| basic_signal_set.test | LINK | 0.393263 | 0.856273 | 1.194707 | — | — | — |
| basic_signal_set_select.test | LINK | 0.414249 | 0.897212 | 1.441902 | — | — | — |
| basic_socket_acceptor.test | LINK | 0.428301 | 0.911623 | 1.551105 | — | — | — |
| basic_socket_acceptor_select.test | LINK | 0.438612 | 0.921967 | 1.592631 | — | — | — |
| basic_stream_file.test | LINK | 0.139607 | 0.303910 | 0.390908 | — | — | — |
| basic_stream_file_select.test | LINK | 0.146455 | 0.295255 | 0.391595 | — | — | — |
| basic_stream_socket.test | LINK | 0.468748 | 0.896333 | 1.497142 | — | — | — |
| basic_stream_socket_select.test | LINK | 0.465538 | 0.915055 | 1.524842 | — | — | — |
| basic_streambuf.test | LINK | 0.253507 | 0.429138 | 0.589222 | — | — | — |
| basic_streambuf_select.test | LINK | 0.244780 | 0.416886 | 0.597130 | — | — | — |
| basic_waitable_timer.test | LINK | 0.412970 | 0.837375 | 1.404388 | — | — | — |
| basic_waitable_timer_select.test | LINK | 0.409875 | 0.852560 | 1.468161 | — | — | — |
| basic_writable_pipe.test | LINK | 0.422352 | 0.860993 | 1.341301 | — | — | — |
| basic_writable_pipe_select.test | LINK | 0.435226 | 0.919290 | 1.590102 | — | — | — |
| bind_allocator.test | RUN | 0.660879 | 3.247833 | 3.354789 | 0.002294 | 0.002348 | 0.002443 |
| bind_allocator_select.test | RUN | 0.682870 | 3.298555 | 3.389280 | 0.001898 | 0.001946 | 0.002377 |
| bind_cancellation_slot.test | RUN | 0.676538 | 3.263599 | 3.397198 | 0.001650 | 0.001546 | 0.001633 |
| bind_cancellation_slot_select.test | RUN | 0.686325 | 3.305620 | 3.277376 | 0.001440 | 0.001548 | 0.001571 |
| bind_executor.test | RUN | 0.461952 | 2.296104 | 2.127580 | 0.002369 | 0.002142 | 0.002269 |
| bind_executor_select.test | RUN | 0.464834 | 2.301027 | 2.107482 | 0.002134 | 0.001974 | 0.002188 |
| bind_immediate_executor.test | RUN | 0.352966 | 1.870850 | 1.333990 | 0.001448 | 0.001433 | 0.001483 |
| bind_immediate_executor_select.test | RUN | 0.365330 | 1.826191 | 1.492376 | 0.001508 | 0.001422 | 0.001502 |
| buffer.test | RUN | 0.204263 | 0.786896 | 0.813574 | 0.001388 | 0.001314 | 0.001595 |
| buffer_select.test | RUN | 0.208520 | 0.779247 | 0.813620 | 0.001345 | 0.001350 | 0.001495 |
| buffer_registration.test | LINK | 0.173903 | 0.400898 | 0.478314 | — | — | — |
| buffer_registration_select.test | LINK | 0.177182 | 0.398591 | 0.476432 | — | — | — |
| buffered_read_stream.test | RUN | 0.749404 | 5.091270 | 4.905441 | 0.001941 | 0.002262 | 0.002184 |
| buffered_read_stream_select.test | RUN | 0.746572 | 5.124394 | 4.662809 | 0.001882 | 0.002166 | 0.002452 |
| buffered_stream.test | RUN | 1.173241 | 7.752574 | 8.137770 | 0.002228 | 0.002414 | 0.002539 |
| buffered_stream_select.test | RUN | 1.186715 | 7.862722 | 7.783779 | 0.002186 | 0.002446 | 0.002188 |
| buffered_write_stream.test | RUN | 1.213814 | 6.659995 | 6.706934 | 0.002288 | 0.002380 | 0.002406 |
| buffered_write_stream_select.test | RUN | 1.217670 | 6.733832 | 6.446776 | 0.002082 | 0.002260 | 0.002192 |
| buffers_iterator.test | RUN | 0.387346 | 0.633134 | 1.101974 | 0.001554 | 0.001926 | 0.001659 |
| buffers_iterator_select.test | RUN | 0.374057 | 0.630222 | 1.109356 | 0.001713 | 0.001908 | 0.001603 |
| cancel_after.test | RUN | 0.972183 | 5.233196 | 4.929674 | 0.004393 | 0.003823 | 0.003875 |
| cancel_after_select.test | RUN | 0.986146 | 5.208118 | 4.947102 | 0.003748 | 0.003395 | 0.003765 |
| cancel_at.test | RUN | 0.966725 | 4.621319 | 4.678267 | 0.003262 | 0.003429 | 0.003772 |
| cancel_at_select.test | RUN | 0.979633 | 4.559802 | 4.654379 | 0.003879 | 0.003433 | 0.003644 |
| cancellation_signal.test | LINK | 0.168187 | 0.241593 | 0.307313 | — | — | — |
| cancellation_signal_select.test | LINK | 0.180568 | 0.241300 | 0.319176 | — | — | — |
| cancellation_state.test | LINK | 0.180569 | 0.240787 | 0.373557 | — | — | — |
| cancellation_state_select.test | LINK | 0.186342 | 0.276560 | 0.459570 | — | — | — |
| cancellation_type.test | LINK | 0.147248 | 0.212842 | 0.402125 | — | — | — |
| cancellation_type_select.test | LINK | 0.136595 | 0.192927 | 0.393418 | — | — | — |
| co_composed.test | RUN | 0.144087 | 0.190917 | 0.380505 | 0.001602 | 0.001343 | 0.001727 |
| co_composed_select.test | RUN | 0.143507 | 0.190369 | 0.380940 | 0.001845 | 0.001667 | 0.001621 |
| co_spawn.test | LINK | 0.139621 | 0.235541 | 0.360023 | — | — | — |
| co_spawn_select.test | LINK | 0.144720 | 0.280460 | 0.393736 | — | — | — |
| completion_condition.test | LINK | 0.186199 | 0.422845 | 0.493128 | — | — | — |
| completion_condition_select.test | LINK | 0.182648 | 0.410781 | 0.497603 | — | — | — |
| compose.test | RUN | 0.800392 | 4.003940 | 4.200934 | 0.002414 | 0.002259 | 0.002543 |
| compose_select.test | RUN | 0.829189 | 3.944906 | 4.199871 | 0.002197 | 0.001948 | 0.002435 |
| composed.test | RUN | 0.810665 | 3.951491 | 4.085151 | 0.002068 | 0.001958 | 0.002082 |
| composed_select.test | RUN | 0.817059 | 3.914288 | 4.032437 | 0.002046 | 0.001796 | 0.002059 |
| config.test | RUN | 0.420418 | 2.541472 | 1.784127 | 0.002018 | 0.001900 | 0.001944 |
| config_select.test | RUN | 0.489620 | 2.672435 | 2.024483 | 0.001979 | 0.001791 | 0.001831 |
| connect.test | LINK | 1.284417 | 8.412581 | 8.321454 | — | — | — |
| connect_select.test | LINK | 1.307047 | 8.476961 | 8.349317 | — | — | — |
| connect_pipe.test | RUN | 0.718160 | 3.184714 | 3.084770 | 0.002004 | 0.001988 | 0.001996 |
| connect_pipe_select.test | RUN | 0.721150 | 3.339437 | 3.305595 | 0.001738 | 0.002022 | 0.001929 |
| consign.test | RUN | 0.635334 | 3.114029 | 2.730204 | 0.001971 | 0.001919 | 0.001804 |
| consign_select.test | RUN | 0.640570 | 3.221281 | 2.701531 | 0.001438 | 0.001940 | 0.001614 |
| coroutine.test | LINK | 0.142243 | 0.336115 | 0.392109 | — | — | — |
| coroutine_select.test | LINK | 0.143144 | 0.337991 | 0.386137 | — | — | — |
| deadline_timer.test | RUN | 0.881069 | 4.095187 | 4.092152 | 0.003424 | 0.002846 | 0.003160 |
| deadline_timer_select.test | RUN | 0.905568 | 4.047879 | 4.021103 | 0.003175 | 0.003167 | 0.002656 |
| deferred.test | RUN | 0.161074 | 0.312193 | 0.386944 | 0.001721 | 0.001908 | 0.001710 |
| deferred_select.test | RUN | 0.156626 | 0.311899 | 0.381974 | 0.001828 | 0.001876 | 0.001676 |
| detached.test | LINK | 0.172085 | 0.351311 | 0.436121 | — | — | — |
| detached_select.test | LINK | 0.175648 | 0.348952 | 0.438993 | — | — | — |
| disposition.test | RUN | 0.161391 | 0.803665 | 0.810860 | 0.001528 | 0.001873 | 0.001842 |
| disposition_select.test | RUN | 0.162006 | 0.785551 | 0.802696 | 0.001512 | 0.001896 | 0.001798 |
| error.test | RUN | 0.150418 | 0.772761 | 0.718713 | 0.001484 | 0.001781 | 0.001815 |
| error_select.test | RUN | 0.150359 | 0.747387 | 0.721982 | 0.001481 | 0.001843 | 0.001782 |
| file_base.test | LINK | 0.191487 | 0.635619 | 0.791239 | — | — | — |
| file_base_select.test | LINK | 0.191465 | 0.611971 | 0.680223 | — | — | — |
| fwd_first.test | LINK | 0.436496 | 0.950157 | 1.388775 | — | — | — |
| fwd_last.test | LINK | 0.433186 | 0.916360 | 1.386443 | — | — | — |
| generic_basic_endpoint.test | LINK | 0.164031 | 0.375235 | 0.510410 | — | — | — |
| generic_basic_endpoint_select.test | LINK | 0.164012 | 0.376037 | 0.504084 | — | — | — |
| generic_datagram_protocol.test | LINK | 0.647648 | 3.280853 | 2.743083 | — | — | — |
| generic_datagram_protocol_select.test | LINK | 0.645070 | 3.362229 | 2.698144 | — | — | — |
| generic_raw_protocol.test | LINK | 0.640914 | 3.221409 | 2.829187 | — | — | — |
| generic_raw_protocol_select.test | LINK | 0.638218 | 3.306524 | 2.789137 | — | — | — |
| generic_seq_packet_protocol.test | LINK | 0.558168 | 2.739200 | 2.339365 | — | — | — |
| generic_seq_packet_protocol_select.test | LINK | 0.565777 | 2.751952 | 2.316591 | — | — | — |
| generic_stream_protocol.test | LINK | 0.599903 | 2.947937 | 2.556181 | — | — | — |
| generic_stream_protocol_select.test | LINK | 0.605252 | 3.000903 | 2.540254 | — | — | — |
| high_resolution_timer.test | LINK | 0.274841 | 0.562879 | 0.781577 | — | — | — |
| high_resolution_timer_select.test | LINK | 0.282438 | 0.574245 | 0.798514 | — | — | — |
| immediate.test | LINK | 0.191379 | 0.431115 | 0.490131 | — | — | — |
| immediate_select.test | LINK | 0.190292 | 0.428882 | 0.483884 | — | — | — |
| inline_executor.test | RUN | 0.231462 | 0.670599 | 0.753016 | 0.001414 | 0.001367 | 0.001370 |
| inline_executor_select.test | RUN | 0.230062 | 0.658255 | 0.746955 | 0.001321 | 0.001394 | 0.001266 |
| inline_or_executor.test | RUN | 0.363504 | 1.914443 | 1.373781 | 0.001504 | 0.001394 | 0.001352 |
| inline_or_executor_select.test | RUN | 0.378526 | 1.955284 | 1.494299 | 0.001379 | 0.001428 | 0.001306 |
| io_context.test | RUN | 0.521354 | 2.936077 | 2.395078 | 0.002596 | 0.002274 | 0.002197 |
| io_context_select.test | RUN | 0.529938 | 2.874247 | 2.384680 | 0.002199 | 0.002147 | 0.002006 |
| io_context_strand.test | RUN | 0.524888 | 2.460135 | 2.080694 | 0.002368 | 0.002290 | 0.002223 |
| io_context_strand_select.test | RUN | 0.529792 | 2.434802 | 2.087017 | 0.002248 | 0.002199 | 0.002139 |
| ip_address.test | LINK | 0.195218 | 0.584153 | 0.613270 | — | — | — |
| ip_address_select.test | LINK | 0.195499 | 0.580426 | 0.610303 | — | — | — |
| ip_address_v4.test | LINK | 0.198338 | 0.617362 | 0.669503 | — | — | — |
| ip_address_v4_select.test | LINK | 0.198509 | 0.611058 | 0.661482 | — | — | — |
| ip_address_v6.test | LINK | 0.219939 | 0.802809 | 0.814351 | — | — | — |
| ip_address_v6_select.test | LINK | 0.220563 | 0.796581 | 0.805878 | — | — | — |
| ip_basic_endpoint.test | LINK | 0.176489 | 0.394278 | 0.524217 | — | — | — |
| ip_basic_endpoint_select.test | LINK | 0.180548 | 0.395940 | 0.526821 | — | — | — |
| ip_basic_resolver.test | LINK | 0.286409 | 0.566195 | 1.023530 | — | — | — |
| ip_basic_resolver_select.test | LINK | 0.291719 | 0.586816 | 1.035939 | — | — | — |
| ip_basic_resolver_entry.test | LINK | 0.096966 | 0.192041 | 0.406111 | — | — | — |
| ip_basic_resolver_entry_select.test | LINK | 0.097312 | 0.193229 | 0.397510 | — | — | — |
| ip_basic_resolver_iterator.test | LINK | 0.171100 | 0.386116 | 0.888393 | — | — | — |
| ip_basic_resolver_iterator_select.test | LINK | 0.173233 | 0.385006 | 0.911713 | — | — | — |
| ip_basic_resolver_query.test | LINK | 0.165014 | 0.371399 | 0.861816 | — | — | — |
| ip_basic_resolver_query_select.test | LINK | 0.163634 | 0.372491 | 0.885059 | — | — | — |
| ip_host_name.test | RUN | 0.175815 | 0.505159 | 0.959528 | 0.001487 | 0.001778 | 0.001700 |
| ip_host_name_select.test | RUN | 0.177645 | 0.549588 | 0.973756 | 0.001406 | 0.001348 | 0.001732 |
| ip_icmp.test | RUN | 0.797280 | 6.843720 | 5.848688 | 0.001527 | 0.002086 | 0.001721 |
| ip_icmp_select.test | RUN | 0.794369 | 7.023071 | 5.723349 | 0.001374 | 0.002002 | 0.001802 |
| ip_multicast.test | RUN | 0.480078 | 3.405528 | 3.331470 | 0.001692 | 0.001984 | 0.001989 |
| ip_multicast_select.test | RUN | 0.482955 | 3.445950 | 3.335753 | 0.001485 | 0.001942 | 0.001940 |
| ip_resolver_query_base.test | LINK | 0.113542 | 0.305493 | 0.402674 | — | — | — |
| ip_resolver_query_base_select.test | LINK | 0.114038 | 0.311800 | 0.402876 | — | — | — |
| ip_tcp.test | RUN | 1.179266 | 10.214804 | 8.935968 | 0.002286 | 0.002701 | 0.002276 |
| ip_tcp_select.test | RUN | 1.166139 | 10.611459 | 8.670590 | 0.001902 | 0.002195 | 0.002521 |
| ip_udp.test | RUN | 0.849105 | 7.302842 | 6.162253 | 0.001691 | 0.002086 | 0.002000 |
| ip_udp_select.test | RUN | 0.859351 | 7.466604 | 6.035762 | 0.001533 | 0.002090 | 0.001906 |
| ip_unicast.test | RUN | 0.442528 | 3.080071 | 2.899098 | 0.001652 | 0.001912 | 0.001852 |
| ip_unicast_select.test | RUN | 0.447187 | 3.117139 | 2.879982 | 0.001426 | 0.001841 | 0.001842 |
| ip_v6_only.test | RUN | 0.455937 | 3.168307 | 3.012570 | 0.001644 | 0.002026 | 0.001706 |
| ip_v6_only_select.test | RUN | 0.478882 | 3.250776 | 3.023877 | 0.001649 | 0.001978 | 0.001857 |
| is_read_buffered.test | RUN | 0.325440 | 0.985590 | 1.657050 | 0.001422 | 0.001878 | 0.001724 |
| is_read_buffered_select.test | RUN | 0.334873 | 1.003608 | 1.697795 | 0.001421 | 0.001758 | 0.001308 |
| is_write_buffered.test | RUN | 0.332306 | 0.994044 | 1.591332 | 0.001413 | 0.001677 | 0.001610 |
| is_write_buffered_select.test | RUN | 0.340836 | 1.007555 | 1.682393 | 0.001469 | 0.001627 | 0.001858 |
| local_basic_endpoint.test | LINK | 0.173220 | 0.562609 | 0.892526 | — | — | — |
| local_basic_endpoint_select.test | LINK | 0.175766 | 0.566852 | 0.885353 | — | — | — |
| local_connect_pair.test | LINK | 0.431670 | 3.130043 | 2.853005 | — | — | — |
| local_connect_pair_select.test | LINK | 0.441900 | 3.112653 | 2.767089 | — | — | — |
| local_datagram_protocol.test | LINK | 0.808046 | 4.054720 | 3.855598 | — | — | — |
| local_datagram_protocol_select.test | LINK | 0.852647 | 4.075253 | 3.762210 | — | — | — |
| local_seq_packet_protocol.test | LINK | 0.859814 | 3.897022 | 3.859206 | — | — | — |
| local_seq_packet_protocol_select.test | LINK | 0.864348 | 3.969311 | 3.852334 | — | — | — |
| local_stream_protocol.test | LINK | 0.809990 | 3.763585 | 3.762304 | — | — | — |
| local_stream_protocol_select.test | LINK | 0.820474 | 3.781659 | 3.773147 | — | — | — |
| placeholders.test | LINK | 0.160346 | 0.299133 | 0.435518 | — | — | — |
| placeholders_select.test | LINK | 0.158912 | 0.307638 | 0.420364 | — | — | — |
| posix_basic_descriptor.test | LINK | 0.416775 | 0.843589 | 1.177358 | — | — | — |
| posix_basic_descriptor_select.test | LINK | 0.431804 | 0.896427 | 1.496336 | — | — | — |
| posix_basic_stream_descriptor.test | LINK | 0.405979 | 0.843802 | 1.237526 | — | — | — |
| posix_basic_stream_descriptor_select.test | LINK | 0.419290 | 0.888159 | 1.476956 | — | — | — |
| posix_descriptor_base.test | LINK | 0.172557 | 0.325244 | 0.431253 | — | — | — |
| posix_descriptor_base_select.test | LINK | 0.169252 | 0.320584 | 0.428156 | — | — | — |
| posix_stream_descriptor.test | LINK | 0.801105 | 3.829118 | 3.623091 | — | — | — |
| posix_stream_descriptor_select.test | LINK | 0.823303 | 3.885877 | 3.705471 | — | — | — |
| prepend.test | RUN | 0.634619 | 3.128866 | 2.913352 | 0.001846 | 0.001913 | 0.001866 |
| prepend_select.test | RUN | 0.637183 | 3.199681 | 2.872034 | 0.001782 | 0.001842 | 0.001548 |
| random_access_file.test | LINK | 0.386775 | 0.779632 | 0.969542 | — | — | — |
| random_access_file_select.test | LINK | 0.400419 | 0.815324 | 1.287470 | — | — | — |
| read.test | RUN | 1.929459 | 19.315646 | 16.534294 | 0.002585 | 0.002592 | 0.002377 |
| read_select.test | RUN | 1.979892 | 19.272726 | 16.583662 | 0.002475 | 0.002398 | 0.002875 |
| read_at.test | RUN | 1.964156 | 28.295561 | 22.600050 | 0.003064 | 0.003126 | 0.002739 |
| read_at_select.test | RUN | 1.952291 | 28.353965 | 22.923137 | 0.002995 | 0.003017 | 0.003001 |
| read_until.test | RUN | 1.031459 | 7.518946 | 5.923886 | 0.002172 | 0.002110 | 0.002012 |
| read_until_select.test | RUN | 1.045494 | 7.672232 | 6.102298 | 0.001904 | 0.002134 | 0.002020 |
| readable_pipe.test | LINK | 0.660122 | 3.106484 | 2.767120 | — | — | — |
| readable_pipe_select.test | LINK | 0.681382 | 3.307535 | 3.060541 | — | — | — |
| recycling_allocator.test | RUN | 0.204822 | 0.497965 | 0.623614 | 0.001960 | 0.001913 | 0.001611 |
| recycling_allocator_select.test | RUN | 0.200749 | 0.483425 | 0.615756 | 0.001649 | 0.001844 | 0.001716 |
| redirect_disposition.test | LINK | 0.855814 | 4.485540 | 4.466203 | — | — | — |
| redirect_disposition_select.test | LINK | 0.860455 | 4.495369 | 4.431697 | — | — | — |
| redirect_error.test | LINK | 0.755444 | 3.797406 | 3.869059 | — | — | — |
| redirect_error_select.test | LINK | 0.766312 | 3.765248 | 3.839887 | — | — | — |
| registered_buffer.test | LINK | 0.210672 | 0.406146 | 0.535066 | — | — | — |
| registered_buffer_select.test | LINK | 0.216221 | 0.405568 | 0.555840 | — | — | — |
| signal_set.test | RUN | 0.636556 | 3.171801 | 2.669966 | 0.001934 | 0.001969 | 0.001524 |
| signal_set_select.test | RUN | 0.665706 | 3.300655 | 2.895838 | 0.001786 | 0.001892 | 0.001868 |
| signal_set_base.test | LINK | 0.176716 | 0.323240 | 0.414674 | — | — | — |
| signal_set_base_select.test | LINK | 0.175309 | 0.329931 | 0.414116 | — | — | — |
| socket_base.test | RUN | 0.792348 | 4.965247 | 4.113191 | 0.002202 | 0.002114 | 0.001990 |
| socket_base_select.test | RUN | 0.798725 | 4.819908 | 4.098431 | 0.002188 | 0.001716 | 0.001707 |
| spawn.test | RUN | 1.293605 | 7.638989 | 7.054105 | 0.002416 | 0.002046 | 0.002411 |
| spawn_select.test | RUN | 1.319591 | 7.560540 | 7.004472 | 0.002245 | 0.002266 | 0.002240 |
| static_thread_pool.test | RUN | 0.378457 | 0.740622 | 0.963875 | 0.001934 | 0.001761 | 0.001784 |
| static_thread_pool_select.test | RUN | 0.386302 | 0.804132 | 1.288659 | 0.001878 | 0.001874 | 0.001872 |
| steady_timer.test | LINK | 0.417097 | 0.870585 | 1.396583 | — | — | — |
| steady_timer_select.test | LINK | 0.432881 | 0.916234 | 1.353328 | — | — | — |
| strand.test | RUN | 0.847405 | 4.314925 | 4.262987 | 0.002690 | 0.002795 | 0.002452 |
| strand_select.test | RUN | 0.857032 | 4.324778 | 4.320590 | 0.002646 | 0.002868 | 0.002929 |
| stream_file.test | LINK | 0.395981 | 0.777876 | 1.041380 | — | — | — |
| stream_file_select.test | LINK | 0.409338 | 0.827015 | 1.286659 | — | — | — |
| streambuf.test | RUN | 0.219084 | 0.569265 | 0.756765 | 0.001532 | 0.001425 | 0.001720 |
| streambuf_select.test | RUN | 0.180455 | 0.457240 | 0.702715 | 0.001504 | 0.001472 | 0.001917 |
| system_timer.test | LINK | 0.532090 | 2.781264 | 2.825006 | — | — | — |
| system_timer_select.test | LINK | 0.546216 | 2.727374 | 2.447576 | — | — | — |
| system_context.test | LINK | 0.263953 | 0.500357 | 0.591198 | — | — | — |
| system_context_select.test | LINK | 0.261116 | 0.541217 | 0.748093 | — | — | — |
| system_executor.test | LINK | 0.332790 | 1.590157 | 1.134691 | — | — | — |
| system_executor_select.test | LINK | 0.345201 | 1.552426 | 1.278988 | — | — | — |
| this_coro.test | LINK | 0.100209 | 0.192507 | 0.241616 | — | — | — |
| this_coro_select.test | LINK | 0.103589 | 0.193037 | 0.237264 | — | — | — |
| thread_pool.test | RUN | 0.379934 | 2.048983 | 1.453401 | 0.002763 | 0.003021 | 0.002575 |
| thread_pool_select.test | RUN | 0.392799 | 2.163085 | 1.581798 | 0.002758 | 0.003330 | 0.002518 |
| time_traits.test | LINK | 0.222470 | 0.582706 | 0.469958 | — | — | — |
| time_traits_select.test | LINK | 0.223055 | 0.577502 | 0.475438 | — | — | — |
| ts_buffer.test | LINK | 0.213400 | 0.652836 | 0.504736 | — | — | — |
| ts_buffer_select.test | LINK | 0.212300 | 0.643384 | 0.609301 | — | — | — |
| ts_executor.test | LINK | 0.298375 | 0.890480 | 0.921075 | — | — | — |
| ts_executor_select.test | LINK | 0.319446 | 0.957056 | 1.492092 | — | — | — |
| ts_internet.test | LINK | 0.327437 | 0.988075 | 1.638405 | — | — | — |
| ts_internet_select.test | LINK | 0.337905 | 1.020570 | 1.672626 | — | — | — |
| ts_io_context.test | LINK | 0.294733 | 0.764074 | 1.018640 | — | — | — |
| ts_io_context_select.test | LINK | 0.398545 | 0.807967 | 1.240073 | — | — | — |
| ts_net.test | LINK | 0.414140 | 1.190203 | 2.024972 | — | — | — |
| ts_net_select.test | LINK | 0.411953 | 1.210461 | 2.063372 | — | — | — |
| ts_netfwd.test | LINK | 0.392418 | 1.204929 | 2.019777 | — | — | — |
| ts_netfwd_select.test | LINK | 0.407029 | 1.231227 | 2.042023 | — | — | — |
| ts_socket.test | LINK | 0.319758 | 0.939525 | 1.550103 | — | — | — |
| ts_socket_select.test | LINK | 0.319423 | 0.974701 | 1.570035 | — | — | — |
| ts_timer.test | LINK | 0.279638 | 0.837675 | 1.343085 | — | — | — |
| ts_timer_select.test | LINK | 0.283452 | 0.847895 | 1.375237 | — | — | — |
| use_awaitable.test | LINK | 0.095621 | 0.290785 | 0.362585 | — | — | — |
| use_awaitable_select.test | LINK | 0.099320 | 0.285276 | 0.356692 | — | — | — |
| wait_traits.test | LINK | 0.101406 | 0.283466 | 0.386402 | — | — | — |
| wait_traits_select.test | LINK | 0.099136 | 0.288149 | 0.364317 | — | — | — |
| windows_basic_object_handle.test | LINK | 0.098956 | 0.285713 | 0.356565 | — | — | — |
| windows_basic_object_handle_select.test | LINK | 0.095926 | 0.289887 | 0.390423 | — | — | — |
| windows_basic_overlapped_handle.test | LINK | 0.096925 | 0.290493 | 0.392191 | — | — | — |
| windows_basic_overlapped_handle_select.test | LINK | 0.098661 | 0.286630 | 0.360841 | — | — | — |
| windows_basic_random_access_handle.test | LINK | 0.098216 | 0.311864 | 0.386280 | — | — | — |
| windows_basic_random_access_handle_select.test | LINK | 0.106711 | 0.312836 | 0.354858 | — | — | — |
| windows_basic_stream_handle.test | LINK | 0.137104 | 0.317122 | 0.392917 | — | — | — |
| windows_basic_stream_handle_select.test | LINK | 0.136831 | 0.307878 | 0.358571 | — | — | — |
| windows_object_handle.test | LINK | 0.392668 | 0.763934 | 0.974127 | — | — | — |
| windows_object_handle_select.test | LINK | 0.407168 | 0.811004 | 1.276624 | — | — | — |
| windows_overlapped_ptr.test | LINK | 0.405220 | 0.813910 | 1.120310 | — | — | — |
| windows_overlapped_ptr_select.test | LINK | 0.429019 | 0.873055 | 1.362334 | — | — | — |
| windows_random_access_handle.test | LINK | 0.382753 | 0.769637 | 0.984259 | — | — | — |
| windows_random_access_handle_select.test | LINK | 0.405279 | 0.832715 | 1.269889 | — | — | — |
| windows_stream_handle.test | LINK | 0.389101 | 0.777672 | 1.023555 | — | — | — |
| windows_stream_handle_select.test | LINK | 0.414655 | 0.851078 | 1.282060 | — | — | — |
| writable_pipe.test | LINK | 0.690398 | 3.154906 | 3.005581 | — | — | — |
| writable_pipe_select.test | LINK | 0.709734 | 3.276500 | 3.186674 | — | — | — |
| write.test | RUN | 1.923835 | 16.273889 | 16.512688 | 0.002560 | 0.002398 | 0.002446 |
| write_select.test | RUN | 1.931566 | 16.381564 | 16.672593 | 0.002744 | 0.002424 | 0.002566 |
| write_at.test | RUN | 1.960702 | 23.780864 | 20.702421 | 0.002834 | 0.002626 | 0.002595 |
| write_at_select.test | RUN | 1.986098 | 23.728398 | 21.015860 | 0.002748 | 0.002616 | 0.002654 |
| any_executor.test | RUN | 0.798986 | 7.990611 | 6.853715 | 0.002334 | 0.002600 | 0.002466 |
| blocking.test | RUN | 2.032169 | 9.457600 | 13.826523 | 0.007499 | 0.006780 | 0.007488 |
| blocking_adaptation.test | RUN | 0.999906 | 4.117809 | 6.204599 | 0.003822 | 0.004245 | 0.003905 |
| context_as.test | RUN | 0.547695 | 2.884205 | 2.084329 | 0.002149 | 0.002048 | 0.001940 |
| executor.test | RUN | 0.160491 | 0.305942 | 0.388511 | 0.001954 | 0.001881 | 0.001760 |
| inline_exception_handling.test | RUN | 1.899810 | 8.593131 | 13.710057 | 0.005291 | 0.005151 | 0.005068 |
| invocable_archetype.test | RUN | 0.144481 | 0.294744 | 0.386931 | 0.001546 | 0.001909 | 0.001775 |
| mapping.test | RUN | 1.798226 | 9.207704 | 13.081302 | 0.005421 | 0.005265 | 0.005350 |
| outstanding_work.test | RUN | 0.930602 | 3.914569 | 5.986033 | 0.004026 | 0.004206 | 0.004148 |
| prefer_only.test | RUN | 0.300598 | 1.493041 | 1.357771 | 0.001939 | 0.001916 | 0.001708 |
| relationship.test | RUN | 0.732061 | 3.813717 | 5.542517 | 0.003063 | 0.004137 | 0.003949 |

## Native runtime replay

Child CPU of successful native tests, with process startup included; GDB and expected-failure executions are correctness-only. Two warmups per compiler, four A/A observations per reference and mirrored ABBA/BAAB blocks. Short tests repeat up to ten times, identically across compilers; CPU is divided by repetitions before summing tests. Executable hashes and output checks stay outside process timers. All runs must pass their original native oracle. Output hashes record equality or variation across the complete campaign; test reports may print times or addresses.

| Reference | CPU speedup ↑ | Paired blocks | A/A speedup |
|---|---:|---|---:|
| gcc | 0.993× | 1.003×, 0.982× | 0.995× |
| clang | 0.975× | 0.986×, 0.965× | 1.010× |

† A/A drift or paired-block spread exceeds 5%; small differences are unresolved. 1375 native tests in the full cohort; 1363 have byte-identical output throughout. [Raw replay evidence in CI artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43479737952).

| Test | Repeats | CPU cppgm | GCC | Clang | Output check |
|---|---:|---:|---:|---:|---|
| any_completion_executor.test | 3 | 0.001659s | 0.001628s | 0.001623s | byte equality observed |
| any_completion_executor_select.test | 4 | 0.001636s | 0.001618s | 0.001606s | byte equality observed |
| any_completion_handler.test | 10 | 0.001317s | 0.001315s | 0.001260s | byte equality observed |
| any_completion_handler_select.test | 7 | 0.001288s | 0.001301s | 0.001211s | byte equality observed |
| any_io_executor.test | 5 | 0.001623s | 0.001614s | 0.001575s | byte equality observed |
| any_io_executor_select.test | 2 | 0.001585s | 0.001603s | 0.001610s | byte equality observed |
| append.test | 5 | 0.001254s | 0.001249s | 0.001202s | byte equality observed |
| append_select.test | 10 | 0.001219s | 0.001229s | 0.001186s | byte equality observed |
| as_tuple.test | 10 | 0.001333s | 0.001313s | 0.001293s | byte equality observed |
| as_tuple_select.test | 9 | 0.001269s | 0.001245s | 0.001199s | byte equality observed |
| bind_allocator.test | 1 | 0.001700s | 0.001704s | 0.001695s | byte equality observed |
| bind_allocator_select.test | 1 | 0.001470s | 0.001474s | 0.001482s | byte equality observed |
| bind_cancellation_slot.test | 10 | 0.001242s | 0.001217s | 0.001166s | byte equality observed |
| bind_cancellation_slot_select.test | 10 | 0.001097s | 0.001121s | 0.001058s | byte equality observed |
| bind_executor.test | 1 | 0.001736s | 0.001783s | 0.001661s | byte equality observed |
| bind_executor_select.test | 1 | 0.002077s | 0.002049s | 0.001975s | byte equality observed |
| bind_immediate_executor.test | 10 | 0.001089s | 0.001083s | 0.001060s | byte equality observed |
| bind_immediate_executor_select.test | 10 | 0.001073s | 0.001105s | 0.001063s | byte equality observed |
| buffer.test | 10 | 0.001090s | 0.001078s | 0.001035s | byte equality observed |
| buffer_select.test | 10 | 0.001049s | 0.001045s | 0.001045s | byte equality observed |
| buffered_read_stream.test | 10 | 0.001365s | 0.001375s | 0.001350s | byte equality observed |
| buffered_read_stream_select.test | 7 | 0.001359s | 0.001335s | 0.001343s | byte equality observed |
| buffered_stream.test | 10 | 0.001357s | 0.001367s | 0.001330s | byte equality observed |
| buffered_stream_select.test | 10 | 0.001324s | 0.001310s | 0.001282s | byte equality observed |
| buffered_write_stream.test | 10 | 0.001374s | 0.001395s | 0.001325s | byte equality observed |
| buffered_write_stream_select.test | 10 | 0.001332s | 0.001326s | 0.001346s | byte equality observed |
| buffers_iterator.test | 10 | 0.001047s | 0.001035s | 0.001022s | byte equality observed |
| buffers_iterator_select.test | 10 | 0.001029s | 0.001038s | 0.001000s | byte equality observed |
| cancel_after.test | 1 | 0.003451s | 0.003397s | 0.003398s | byte equality observed |
| cancel_after_select.test | 1 | 0.002857s | 0.002844s | 0.002909s | byte equality observed |
| cancel_at.test | 1 | 0.003660s | 0.003518s | 0.003556s | byte equality observed |
| cancel_at_select.test | 1 | 0.003587s | 0.003446s | 0.003397s | byte equality observed |
| co_composed.test | 10 | 0.001097s | 0.001074s | 0.001024s | byte equality observed |
| co_composed_select.test | 10 | 0.001023s | 0.001022s | 0.000985s | byte equality observed |
| compose.test | 1 | 0.001775s | 0.001659s | 0.001697s | byte equality observed |
| compose_select.test | 1 | 0.001447s | 0.001417s | 0.001407s | byte equality observed |
| composed.test | 1 | 0.002086s | 0.002031s | 0.002043s | byte equality observed |
| composed_select.test | 1 | 0.001528s | 0.001598s | 0.001501s | byte equality observed |
| config.test | 10 | 0.001134s | 0.001107s | 0.001094s | byte equality observed |
| config_select.test | 10 | 0.001083s | 0.001106s | 0.001060s | byte equality observed |
| connect_pipe.test | 10 | 0.001116s | 0.001106s | 0.001103s | byte equality observed |
| connect_pipe_select.test | 10 | 0.001097s | 0.001104s | 0.001098s | byte equality observed |
| consign.test | 10 | 0.001110s | 0.001086s | 0.001073s | byte equality observed |
| consign_select.test | 10 | 0.001058s | 0.001051s | 0.001027s | byte equality observed |
| deadline_timer.test | 1 | 0.002485s | 0.002376s | 0.002444s | byte equality observed |
| deadline_timer_select.test | 1 | 0.002227s | 0.002160s | 0.002222s | byte equality observed |
| deferred.test | 10 | 0.001013s | 0.001021s | 0.001005s | byte equality observed |
| deferred_select.test | 10 | 0.000959s | 0.000983s | 0.000947s | byte equality observed |
| disposition.test | 10 | 0.001021s | 0.001067s | 0.001027s | byte equality observed |
| disposition_select.test | 10 | 0.001025s | 0.001058s | 0.000991s | byte equality observed |
| error.test | 10 | 0.000990s | 0.001017s | 0.000970s | byte equality observed |
| error_select.test | 10 | 0.000994s | 0.001030s | 0.000978s | byte equality observed |
| inline_executor.test | 10 | 0.001010s | 0.001026s | 0.001009s | byte equality observed |
| inline_executor_select.test | 10 | 0.001008s | 0.001066s | 0.001009s | byte equality observed |
| inline_or_executor.test | 10 | 0.001017s | 0.001054s | 0.000994s | byte equality observed |
| inline_or_executor_select.test | 10 | 0.001018s | 0.001032s | 0.000992s | byte equality observed |
| io_context.test | 1 | 0.001936s | 0.002012s | 0.001927s | byte equality observed |
| io_context_select.test | 1 | 0.002271s | 0.001689s | 0.002253s | byte equality observed |
| io_context_strand.test | 1 | 0.002309s | 0.002298s | 0.002319s | byte equality observed |
| io_context_strand_select.test | 1 | 0.002102s | 0.001817s | 0.002076s | byte equality observed |
| ip_host_name.test | 10 | 0.001030s | 0.001076s | 0.001018s | byte equality observed |
| ip_host_name_select.test | 10 | 0.000997s | 0.001044s | 0.000980s | byte equality observed |
| ip_icmp.test | 10 | 0.001019s | 0.001088s | 0.000979s | byte equality observed |
| ip_icmp_select.test | 10 | 0.001015s | 0.001098s | 0.000983s | byte equality observed |
| ip_multicast.test | 10 | 0.001094s | 0.001129s | 0.001070s | byte equality observed |
| ip_multicast_select.test | 10 | 0.001082s | 0.001194s | 0.001055s | byte equality observed |
| ip_tcp.test | 10 | 0.001492s | 0.001541s | 0.001475s | byte equality observed |
| ip_tcp_select.test | 10 | 0.001440s | 0.001524s | 0.001431s | byte equality observed |
| ip_udp.test | 10 | 0.001148s | 0.001197s | 0.001119s | byte equality observed |
| ip_udp_select.test | 10 | 0.001150s | 0.001168s | 0.001087s | byte equality observed |
| ip_unicast.test | 10 | 0.001110s | 0.001148s | 0.001079s | byte equality observed |
| ip_unicast_select.test | 10 | 0.001105s | 0.001140s | 0.001039s | byte equality observed |
| ip_v6_only.test | 10 | 0.001136s | 0.001154s | 0.001081s | byte equality observed |
| ip_v6_only_select.test | 10 | 0.001106s | 0.001106s | 0.001047s | byte equality observed |
| is_read_buffered.test | 10 | 0.001075s | 0.001004s | 0.000988s | byte equality observed |
| is_read_buffered_select.test | 10 | 0.001044s | 0.001018s | 0.000979s | byte equality observed |
| is_write_buffered.test | 10 | 0.001071s | 0.001007s | 0.000989s | byte equality observed |
| is_write_buffered_select.test | 10 | 0.001049s | 0.001010s | 0.000991s | byte equality observed |
| prepend.test | 10 | 0.001117s | 0.001118s | 0.001056s | byte equality observed |
| prepend_select.test | 10 | 0.001099s | 0.001081s | 0.001032s | byte equality observed |
| read.test | 10 | 0.001501s | 0.001502s | 0.001455s | byte equality observed |
| read_at.test | 10 | 0.001857s | 0.001900s | 0.001679s | byte equality observed |
| read_at_select.test | 10 | 0.001894s | 0.001911s | 0.001698s | byte equality observed |
| read_select.test | 10 | 0.001536s | 0.001486s | 0.001431s | byte equality observed |
| read_until.test | 10 | 0.001255s | 0.001191s | 0.001189s | byte equality observed |
| read_until_select.test | 10 | 0.001260s | 0.001216s | 0.001194s | byte equality observed |
| recycling_allocator.test | 10 | 0.001055s | 0.001035s | 0.001037s | byte equality observed |
| recycling_allocator_select.test | 10 | 0.001053s | 0.001041s | 0.001101s | byte equality observed |
| signal_set.test | 10 | 0.001059s | 0.001050s | 0.001060s | byte equality observed |
| signal_set_select.test | 10 | 0.001075s | 0.001046s | 0.001043s | byte equality observed |
| socket_base.test | 10 | 0.001176s | 0.001176s | 0.001177s | byte equality observed |
| socket_base_select.test | 10 | 0.001155s | 0.001125s | 0.001152s | byte equality observed |
| spawn.test | 10 | 0.001274s | 0.001254s | 0.001280s | byte equality observed |
| spawn_select.test | 10 | 0.001268s | 0.001255s | 0.001242s | byte equality observed |
| static_thread_pool.test | 10 | 0.001042s | 0.000992s | 0.001034s | byte equality observed |
| static_thread_pool_select.test | 10 | 0.000998s | 0.000965s | 0.000992s | byte equality observed |
| strand.test | 1 | 0.001956s | 0.002037s | 0.002015s | byte equality observed |
| strand_select.test | 1 | 0.001684s | 0.001616s | 0.001649s | byte equality observed |
| streambuf.test | 10 | 0.001087s | 0.001064s | 0.001046s | byte equality observed |
| streambuf_select.test | 10 | 0.001063s | 0.001042s | 0.001029s | byte equality observed |
| thread_pool.test | 10 | 0.002235s | 0.002210s | 0.002256s | byte equality observed |
| thread_pool_select.test | 10 | 0.002242s | 0.002223s | 0.002259s | byte equality observed |
| write.test | 10 | 0.001564s | 0.001469s | 0.001524s | byte equality observed |
| write_at.test | 10 | 0.001679s | 0.001618s | 0.001592s | byte equality observed |
| write_at_select.test | 10 | 0.001664s | 0.001624s | 0.001591s | byte equality observed |
| write_select.test | 10 | 0.001567s | 0.001514s | 0.001478s | byte equality observed |

[Raw outcomes, commands and timing observations in CI artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43479737952).

[Comparison records](../results.json) · [Host and toolchains](../metadata.json)
