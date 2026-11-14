# libs/asio/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| any_completion_executor.test | RUN | 0.526650 | 2.899620 | 2.376390 | 0.002237 | 0.002228 | 0.002141 |
| any_completion_executor_select.test | RUN | 0.532053 | 2.902557 | 2.508434 | 0.002111 | 0.002048 | 0.002126 |
| any_completion_handler.test | RUN | 0.626723 | 2.659160 | 2.227782 | 0.001598 | 0.001153 | 0.001684 |
| any_completion_handler_select.test | RUN | 0.616895 | 2.759379 | 2.366041 | 0.001690 | 0.001830 | 0.001616 |
| any_io_executor.test | RUN | 0.547165 | 3.048780 | 2.612275 | 0.002273 | 0.002312 | 0.001976 |
| any_io_executor_select.test | RUN | 0.546502 | 3.002304 | 2.693013 | 0.002574 | 0.002180 | 0.001720 |
| append.test | RUN | 0.509709 | 2.449940 | 1.988145 | 0.001750 | 0.001800 | 0.001481 |
| append_select.test | RUN | 0.523488 | 2.516725 | 1.966086 | 0.001469 | 0.001622 | 0.001355 |
| as_tuple.test | RUN | 0.666886 | 3.009682 | 2.863395 | 0.001654 | 0.001603 | 0.001724 |
| as_tuple_select.test | RUN | 0.680973 | 2.930060 | 2.827569 | 0.001764 | 0.001596 | 0.001601 |
| associated_allocator.test | LINK | 0.137178 | 0.256123 | 0.299213 | — | — | — |
| associated_allocator_select.test | LINK | 0.137677 | 0.253236 | 0.297825 | — | — | — |
| associated_cancellation_slot.test | LINK | 0.146943 | 0.282059 | 0.316425 | — | — | — |
| associated_cancellation_slot_select.test | LINK | 0.151731 | 0.286065 | 0.318628 | — | — | — |
| associated_executor.test | LINK | 0.224596 | 0.490758 | 0.508502 | — | — | — |
| associated_executor_select.test | LINK | 0.219738 | 0.477977 | 0.522681 | — | — | — |
| associated_immediate_executor.test | LINK | 0.199549 | 0.444526 | 0.472120 | — | — | — |
| associated_immediate_executor_select.test | LINK | 0.201008 | 0.429137 | 0.468299 | — | — | — |
| associator.test | LINK | 0.115445 | 0.205699 | 0.253327 | — | — | — |
| associator_select.test | LINK | 0.108761 | 0.206269 | 0.264844 | — | — | — |
| awaitable.test | LINK | 0.111817 | 0.208616 | 0.254930 | — | — | — |
| awaitable_select.test | LINK | 0.112055 | 0.201555 | 0.251243 | — | — | — |
| basic_datagram_socket.test | LINK | 0.340948 | 0.642961 | 0.914422 | — | — | — |
| basic_datagram_socket_select.test | LINK | 0.337028 | 0.667088 | 0.930197 | — | — | — |
| basic_deadline_timer.test | LINK | 0.398874 | 0.776335 | 1.058695 | — | — | — |
| basic_deadline_timer_select.test | LINK | 0.407707 | 0.813648 | 1.052056 | — | — | — |
| basic_file.test | LINK | 0.114085 | 0.218771 | 0.243658 | — | — | — |
| basic_file_select.test | LINK | 0.113573 | 0.210073 | 0.242864 | — | — | — |
| basic_random_access_file.test | LINK | 0.110357 | 0.212946 | 0.254201 | — | — | — |
| basic_random_access_file_select.test | LINK | 0.112438 | 0.209604 | 0.231458 | — | — | — |
| basic_raw_socket.test | LINK | 0.337338 | 0.665389 | 0.927983 | — | — | — |
| basic_raw_socket_select.test | LINK | 0.349002 | 0.684744 | 0.938341 | — | — | — |
| basic_readable_pipe.test | LINK | 0.337715 | 0.636973 | 0.781036 | — | — | — |
| basic_readable_pipe_select.test | LINK | 0.351223 | 0.685353 | 0.945979 | — | — | — |
| basic_seq_packet_socket.test | LINK | 0.345093 | 0.667810 | 0.911580 | — | — | — |
| basic_seq_packet_socket_select.test | LINK | 0.349327 | 0.685057 | 0.925533 | — | — | — |
| basic_signal_set.test | LINK | 0.318828 | 0.611352 | 0.705921 | — | — | — |
| basic_signal_set_select.test | LINK | 0.330126 | 0.669430 | 0.890615 | — | — | — |
| basic_socket_acceptor.test | LINK | 0.356671 | 0.664093 | 0.906322 | — | — | — |
| basic_socket_acceptor_select.test | LINK | 0.362684 | 0.691197 | 0.932006 | — | — | — |
| basic_stream_file.test | LINK | 0.115050 | 0.219344 | 0.251483 | — | — | — |
| basic_stream_file_select.test | LINK | 0.119804 | 0.216793 | 0.250613 | — | — | — |
| basic_stream_socket.test | LINK | 0.349636 | 0.650803 | 0.920889 | — | — | — |
| basic_stream_socket_select.test | LINK | 0.376772 | 0.669195 | 0.923186 | — | — | — |
| basic_streambuf.test | LINK | 0.177906 | 0.299568 | 0.348479 | — | — | — |
| basic_streambuf_select.test | LINK | 0.181065 | 0.299504 | 0.357162 | — | — | — |
| basic_waitable_timer.test | LINK | 0.345108 | 0.630662 | 0.847409 | — | — | — |
| basic_waitable_timer_select.test | LINK | 0.349946 | 0.632468 | 0.865550 | — | — | — |
| basic_writable_pipe.test | LINK | 0.349837 | 0.623604 | 0.783865 | — | — | — |
| basic_writable_pipe_select.test | LINK | 0.372010 | 0.672936 | 0.954652 | — | — | — |
| bind_allocator.test | RUN | 0.569716 | 2.537756 | 2.164700 | 0.002408 | 0.001867 | 0.001559 |
| bind_allocator_select.test | RUN | 0.589325 | 2.492647 | 2.127100 | 0.001782 | 0.001574 | 0.001616 |
| bind_cancellation_slot.test | RUN | 0.589614 | 2.500389 | 2.189570 | 0.001286 | 0.001326 | 0.001225 |
| bind_cancellation_slot_select.test | RUN | 0.594321 | 2.546877 | 2.145726 | 0.001517 | 0.001277 | 0.001174 |
| bind_executor.test | RUN | 0.398295 | 1.794812 | 1.345345 | 0.002234 | 0.002116 | 0.001731 |
| bind_executor_select.test | RUN | 0.398750 | 1.850545 | 1.307483 | 0.001985 | 0.001927 | 0.001614 |
| bind_immediate_executor.test | RUN | 0.310977 | 1.471809 | 0.863451 | 0.001452 | 0.001245 | 0.001121 |
| bind_immediate_executor_select.test | RUN | 0.324140 | 1.456420 | 0.938252 | 0.001350 | 0.001247 | 0.001155 |
| buffer.test | RUN | 0.180830 | 0.599754 | 0.496200 | 0.001408 | 0.001267 | 0.001097 |
| buffer_select.test | RUN | 0.185769 | 0.599918 | 0.494743 | 0.001290 | 0.001247 | 0.001115 |
| buffer_registration.test | LINK | 0.140718 | 0.308041 | 0.296642 | — | — | — |
| buffer_registration_select.test | LINK | 0.142637 | 0.308491 | 0.305368 | — | — | — |
| buffered_read_stream.test | RUN | 0.659470 | 3.723099 | 2.862868 | 0.001982 | 0.002124 | 0.001950 |
| buffered_read_stream_select.test | RUN | 0.662669 | 3.892995 | 2.898952 | 0.001986 | 0.001970 | 0.001946 |
| buffered_stream.test | RUN | 0.914006 | 6.172519 | 5.588373 | 0.002084 | 0.002209 | 0.002047 |
| buffered_stream_select.test | RUN | 0.986582 | 6.185814 | 5.365548 | 0.002445 | 0.002245 | 0.001636 |
| buffered_write_stream.test | RUN | 1.062949 | 5.360885 | 4.887403 | 0.002378 | 0.002118 | 0.002002 |
| buffered_write_stream_select.test | RUN | 1.061950 | 5.407822 | 4.694025 | 0.002319 | 0.002169 | 0.001908 |
| buffers_iterator.test | RUN | 0.322590 | 0.482361 | 0.720843 | 0.001688 | 0.001657 | 0.001312 |
| buffers_iterator_select.test | RUN | 0.322258 | 0.480631 | 0.715473 | 0.001789 | 0.001730 | 0.001354 |
| cancel_after.test | RUN | 0.862870 | 4.118427 | 3.414325 | 0.004015 | 0.003521 | 0.002492 |
| cancel_after_select.test | RUN | 0.871122 | 4.087238 | 3.404217 | 0.003563 | 0.003118 | 0.002372 |
| cancel_at.test | RUN | 0.825558 | 3.893580 | 3.389821 | 0.003784 | 0.003081 | 0.002173 |
| cancel_at_select.test | RUN | 0.816623 | 3.778830 | 3.405260 | 0.003240 | 0.003147 | 0.001798 |
| cancellation_signal.test | LINK | 0.114759 | 0.185791 | 0.228086 | — | — | — |
| cancellation_signal_select.test | LINK | 0.128039 | 0.191493 | 0.263555 | — | — | — |
| cancellation_state.test | LINK | 0.154183 | 0.189897 | 0.318108 | — | — | — |
| cancellation_state_select.test | LINK | 0.154188 | 0.186388 | 0.319680 | — | — | — |
| cancellation_type.test | LINK | 0.124639 | 0.146285 | 0.249316 | — | — | — |
| cancellation_type_select.test | LINK | 0.121315 | 0.206808 | 0.246922 | — | — | — |
| co_composed.test | RUN | 0.120982 | 0.160921 | 0.252148 | 0.001766 | 0.001402 | 0.001297 |
| co_composed_select.test | RUN | 0.118333 | 0.217960 | 0.251515 | 0.001516 | 0.001603 | 0.001252 |
| co_spawn.test | LINK | 0.123304 | 0.214773 | 0.243570 | — | — | — |
| co_spawn_select.test | LINK | 0.128298 | 0.212945 | 0.250070 | — | — | — |
| completion_condition.test | LINK | 0.153401 | 0.329826 | 0.337366 | — | — | — |
| completion_condition_select.test | LINK | 0.152557 | 0.325862 | 0.317741 | — | — | — |
| compose.test | RUN | 0.712396 | 3.306706 | 2.791684 | 0.002330 | 0.002389 | 0.001876 |
| compose_select.test | RUN | 0.721600 | 3.228945 | 2.775753 | 0.002366 | 0.002121 | 0.001836 |
| composed.test | RUN | 0.630338 | 3.266552 | 2.684302 | 0.002103 | 0.001948 | 0.001443 |
| composed_select.test | RUN | 0.649315 | 3.211201 | 2.696183 | 0.001775 | 0.001439 | 0.001610 |
| config.test | RUN | 0.299987 | 2.079176 | 1.144508 | 0.001720 | 0.001850 | 0.001591 |
| config_select.test | RUN | 0.344924 | 2.191927 | 1.353426 | 0.001864 | 0.001700 | 0.001503 |
| connect.test | LINK | 1.063118 | 6.919976 | 5.714163 | — | — | — |
| connect_select.test | LINK | 1.064427 | 6.908993 | 5.759488 | — | — | — |
| connect_pipe.test | RUN | 0.584927 | 2.649077 | 2.129202 | 0.001614 | 0.001834 | 0.001659 |
| connect_pipe_select.test | RUN | 0.604635 | 2.747969 | 2.292527 | 0.001910 | 0.001844 | 0.001640 |
| consign.test | RUN | 0.517235 | 2.603652 | 1.975674 | 0.001677 | 0.001902 | 0.001583 |
| consign_select.test | RUN | 0.531535 | 2.653986 | 1.944982 | 0.001848 | 0.001719 | 0.001405 |
| coroutine.test | LINK | 0.122075 | 0.266473 | 0.269905 | — | — | — |
| coroutine_select.test | LINK | 0.123653 | 0.261126 | 0.279541 | — | — | — |
| deadline_timer.test | RUN | 0.758575 | 3.377857 | 2.829835 | 0.003127 | 0.003389 | 0.003058 |
| deadline_timer_select.test | RUN | 0.761510 | 3.314729 | 2.829292 | 0.002706 | 0.003482 | 0.002038 |
| deferred.test | RUN | 0.130061 | 0.234212 | 0.261098 | 0.001721 | 0.001754 | 0.001557 |
| deferred_select.test | RUN | 0.127066 | 0.238448 | 0.267838 | 0.001675 | 0.001602 | 0.001469 |
| detached.test | LINK | 0.141186 | 0.259952 | 0.288109 | — | — | — |
| detached_select.test | LINK | 0.143912 | 0.277817 | 0.297722 | — | — | — |
| disposition.test | RUN | 0.136903 | 0.639083 | 0.537965 | 0.001380 | 0.001830 | 0.001616 |
| disposition_select.test | RUN | 0.135663 | 0.636911 | 0.545482 | 0.001470 | 0.001863 | 0.001588 |
| error.test | RUN | 0.125087 | 0.611802 | 0.486048 | 0.001325 | 0.001699 | 0.001571 |
| error_select.test | RUN | 0.126849 | 0.604178 | 0.491997 | 0.001417 | 0.001752 | 0.001543 |
| file_base.test | LINK | 0.158686 | 0.514200 | 0.549706 | — | — | — |
| file_base_select.test | LINK | 0.162193 | 0.461347 | 0.425531 | — | — | — |
| fwd_first.test | LINK | 0.403193 | 0.763143 | 0.926223 | — | — | — |
| fwd_last.test | LINK | 0.403533 | 0.724904 | 0.937709 | — | — | — |
| generic_basic_endpoint.test | LINK | 0.133421 | 0.300118 | 0.341461 | — | — | — |
| generic_basic_endpoint_select.test | LINK | 0.135222 | 0.300506 | 0.342934 | — | — | — |
| generic_datagram_protocol.test | LINK | 0.521389 | 2.786071 | 1.923403 | — | — | — |
| generic_datagram_protocol_select.test | LINK | 0.529284 | 2.818142 | 1.916760 | — | — | — |
| generic_raw_protocol.test | LINK | 0.543681 | 2.677095 | 1.911240 | — | — | — |
| generic_raw_protocol_select.test | LINK | 0.539212 | 2.741637 | 1.938069 | — | — | — |
| generic_seq_packet_protocol.test | LINK | 0.494042 | 2.253280 | 1.664522 | — | — | — |
| generic_seq_packet_protocol_select.test | LINK | 0.494941 | 2.281500 | 1.664006 | — | — | — |
| generic_stream_protocol.test | LINK | 0.562867 | 2.498180 | 1.760856 | — | — | — |
| generic_stream_protocol_select.test | LINK | 0.563223 | 2.544515 | 1.801112 | — | — | — |
| high_resolution_timer.test | LINK | 0.243405 | 0.454749 | 0.525993 | — | — | — |
| high_resolution_timer_select.test | LINK | 0.245183 | 0.457319 | 0.542072 | — | — | — |
| immediate.test | LINK | 0.170106 | 0.344492 | 0.334276 | — | — | — |
| immediate_select.test | LINK | 0.170984 | 0.344047 | 0.327675 | — | — | — |
| inline_executor.test | RUN | 0.207106 | 0.543802 | 0.491801 | 0.001439 | 0.001380 | 0.001166 |
| inline_executor_select.test | RUN | 0.206514 | 0.537210 | 0.502077 | 0.001459 | 0.001374 | 0.001154 |
| inline_or_executor.test | RUN | 0.331080 | 1.587658 | 0.973278 | 0.001449 | 0.001418 | 0.001131 |
| inline_or_executor_select.test | RUN | 0.339167 | 1.611703 | 1.049775 | 0.001534 | 0.001424 | 0.001157 |
| io_context.test | RUN | 0.486335 | 2.423818 | 1.660261 | 0.002371 | 0.002288 | 0.001769 |
| io_context_select.test | RUN | 0.500673 | 2.356354 | 1.681754 | 0.002241 | 0.002165 | 0.001790 |
| io_context_strand.test | RUN | 0.429303 | 2.052352 | 1.447079 | 0.002358 | 0.002446 | 0.001744 |
| io_context_strand_select.test | RUN | 0.434633 | 2.267557 | 1.473887 | 0.002186 | 0.005502 | 0.001779 |
| ip_address.test | LINK | 0.167477 | 0.480614 | 0.404791 | — | — | — |
| ip_address_select.test | LINK | 0.170573 | 0.476811 | 0.418918 | — | — | — |
| ip_address_v4.test | LINK | 0.174586 | 0.500165 | 0.454147 | — | — | — |
| ip_address_v4_select.test | LINK | 0.179376 | 0.496112 | 0.450072 | — | — | — |
| ip_address_v6.test | LINK | 0.189736 | 0.863819 | 0.547269 | — | — | — |
| ip_address_v6_select.test | LINK | 0.189216 | 0.671000 | 0.554449 | — | — | — |
| ip_basic_endpoint.test | LINK | 0.154262 | 0.478380 | 0.362474 | — | — | — |
| ip_basic_endpoint_select.test | LINK | 0.158500 | 0.462907 | 0.360204 | — | — | — |
| ip_basic_resolver.test | LINK | 0.244895 | 0.663899 | 0.539913 | — | — | — |
| ip_basic_resolver_select.test | LINK | 0.252697 | 0.667394 | 0.776723 | — | — | — |
| ip_basic_resolver_entry.test | LINK | 0.085072 | 0.216927 | 0.241354 | — | — | — |
| ip_basic_resolver_entry_select.test | LINK | 0.085909 | 0.216568 | 0.235648 | — | — | — |
| ip_basic_resolver_iterator.test | LINK | 0.153252 | 0.446692 | 0.554859 | — | — | — |
| ip_basic_resolver_iterator_select.test | LINK | 0.153656 | 0.446140 | 0.560447 | — | — | — |
| ip_basic_resolver_query.test | LINK | 0.148364 | 0.426672 | 0.540693 | — | — | — |
| ip_basic_resolver_query_select.test | LINK | 0.143392 | 0.434432 | 0.544589 | — | — | — |
| ip_host_name.test | RUN | 0.154738 | 0.584074 | 0.616262 | 0.001468 | 0.001720 | 0.001476 |
| ip_host_name_select.test | RUN | 0.153746 | 0.578500 | 0.613457 | 0.001402 | 0.001692 | 0.001446 |
| ip_icmp.test | RUN | 0.693785 | 5.549046 | 4.163472 | 0.001476 | 0.001938 | 0.001524 |
| ip_icmp_select.test | RUN | 0.698180 | 5.650579 | 4.033326 | 0.001490 | 0.001774 | 0.001590 |
| ip_multicast.test | RUN | 0.412740 | 2.756235 | 2.287347 | 0.001646 | 0.001918 | 0.001749 |
| ip_multicast_select.test | RUN | 0.428076 | 2.739087 | 2.280866 | 0.001628 | 0.001877 | 0.001703 |
| ip_resolver_query_base.test | LINK | 0.101922 | 0.238986 | 0.249360 | — | — | — |
| ip_resolver_query_base_select.test | LINK | 0.104005 | 0.239248 | 0.251309 | — | — | — |
| ip_tcp.test | RUN | 1.033742 | 8.181248 | 6.480247 | 0.001964 | 0.002416 | 0.002060 |
| ip_tcp_select.test | RUN | 1.029242 | 8.528911 | 6.316308 | 0.002102 | 0.002455 | 0.002174 |
| ip_udp.test | RUN | 0.740764 | 5.915661 | 4.371406 | 0.001685 | 0.001960 | 0.001662 |
| ip_udp_select.test | RUN | 0.743136 | 6.020275 | 4.298426 | 0.001698 | 0.001901 | 0.001601 |
| ip_unicast.test | RUN | 0.393337 | 2.482604 | 2.020845 | 0.001623 | 0.001926 | 0.001359 |
| ip_unicast_select.test | RUN | 0.393755 | 2.508068 | 1.981699 | 0.001570 | 0.001833 | 0.001482 |
| ip_v6_only.test | RUN | 0.408594 | 2.599047 | 2.063766 | 0.001526 | 0.001850 | 0.001398 |
| ip_v6_only_select.test | RUN | 0.414407 | 2.683427 | 2.038643 | 0.001598 | 0.001706 | 0.001432 |
| is_read_buffered.test | RUN | 0.279276 | 0.774428 | 1.072107 | 0.001421 | 0.001890 | 0.001288 |
| is_read_buffered_select.test | RUN | 0.284699 | 0.820309 | 1.071531 | 0.001479 | 0.001628 | 0.001175 |
| is_write_buffered.test | RUN | 0.292341 | 0.790190 | 1.052452 | 0.001466 | 0.001781 | 0.001331 |
| is_write_buffered_select.test | RUN | 0.301240 | 0.806245 | 1.083022 | 0.001516 | 0.001691 | 0.001365 |
| local_basic_endpoint.test | LINK | 0.157421 | 0.439224 | 0.553157 | — | — | — |
| local_basic_endpoint_select.test | LINK | 0.160628 | 0.429551 | 0.548426 | — | — | — |
| local_connect_pair.test | LINK | 0.390572 | 2.521830 | 1.908228 | — | — | — |
| local_connect_pair_select.test | LINK | 0.401138 | 2.479848 | 1.889910 | — | — | — |
| local_datagram_protocol.test | LINK | 0.508921 | 3.336585 | 2.699273 | — | — | — |
| local_datagram_protocol_select.test | LINK | 0.526156 | 3.389737 | 2.619293 | — | — | — |
| local_seq_packet_protocol.test | LINK | 0.531143 | 3.288258 | 2.702462 | — | — | — |
| local_seq_packet_protocol_select.test | LINK | 0.537138 | 3.375159 | 2.716139 | — | — | — |
| local_stream_protocol.test | LINK | 0.507932 | 3.208910 | 2.515793 | — | — | — |
| local_stream_protocol_select.test | LINK | 0.514308 | 3.211219 | 2.515249 | — | — | — |
| placeholders.test | LINK | 0.099694 | 0.231320 | 0.285076 | — | — | — |
| placeholders_select.test | LINK | 0.140597 | 0.233666 | 0.273574 | — | — | — |
| posix_basic_descriptor.test | LINK | 0.370211 | 0.660182 | 0.828217 | — | — | — |
| posix_basic_descriptor_select.test | LINK | 0.388228 | 0.708814 | 0.958724 | — | — | — |
| posix_basic_stream_descriptor.test | LINK | 0.379332 | 0.653968 | 0.795376 | — | — | — |
| posix_basic_stream_descriptor_select.test | LINK | 0.382988 | 0.696705 | 0.958289 | — | — | — |
| posix_descriptor_base.test | LINK | 0.157381 | 0.251218 | 0.270987 | — | — | — |
| posix_descriptor_base_select.test | LINK | 0.159193 | 0.256445 | 0.263340 | — | — | — |
| posix_stream_descriptor.test | LINK | 0.718219 | 3.169139 | 2.510009 | — | — | — |
| posix_stream_descriptor_select.test | LINK | 0.732658 | 3.203426 | 2.584418 | — | — | — |
| prepend.test | RUN | 0.576010 | 2.530172 | 1.993884 | 0.001942 | 0.001872 | 0.001534 |
| prepend_select.test | RUN | 0.581896 | 2.625959 | 1.984272 | 0.001641 | 0.001867 | 0.001596 |
| random_access_file.test | LINK | 0.337249 | 0.620955 | 0.656263 | — | — | — |
| random_access_file_select.test | LINK | 0.346883 | 0.647966 | 0.820403 | — | — | — |
| read.test | RUN | 1.624749 | 15.125931 | 11.378144 | 0.002361 | 0.002186 | 0.002108 |
| read_select.test | RUN | 1.655003 | 15.169894 | 11.397948 | 0.002176 | 0.002262 | 0.002113 |
| read_at.test | RUN | 1.613552 | 21.627017 | 15.439340 | 0.002905 | 0.002010 | 0.002328 |
| read_at_select.test | RUN | 1.632358 | 21.466678 | 15.620080 | 0.002812 | 0.002412 | 0.001898 |
| read_until.test | RUN | 0.884568 | 5.738369 | 4.258987 | 0.002070 | 0.001920 | 0.001732 |
| read_until_select.test | RUN | 0.887812 | 5.799432 | 4.343776 | 0.001804 | 0.001893 | 0.001596 |
| readable_pipe.test | LINK | 0.601138 | 2.479026 | 1.973491 | — | — | — |
| readable_pipe_select.test | LINK | 0.607997 | 2.628230 | 2.117964 | — | — | — |
| recycling_allocator.test | RUN | 0.182476 | 0.344591 | 0.394964 | 0.001766 | 0.001631 | 0.001339 |
| recycling_allocator_select.test | RUN | 0.183366 | 0.350887 | 0.388493 | 0.001619 | 0.001664 | 0.001338 |
| redirect_disposition.test | LINK | 0.741407 | 3.391559 | 2.977407 | — | — | — |
| redirect_disposition_select.test | LINK | 0.748635 | 3.367838 | 2.967615 | — | — | — |
| redirect_error.test | LINK | 0.696938 | 2.875073 | 2.625049 | — | — | — |
| redirect_error_select.test | LINK | 0.699274 | 2.914528 | 2.614581 | — | — | — |
| registered_buffer.test | LINK | 0.176903 | 0.278066 | 0.356249 | — | — | — |
| registered_buffer_select.test | LINK | 0.185230 | 0.286829 | 0.358442 | — | — | — |
| signal_set.test | RUN | 0.577165 | 2.454439 | 1.853077 | 0.001852 | 0.001583 | 0.001589 |
| signal_set_select.test | RUN | 0.603554 | 2.502006 | 2.021918 | 0.001572 | 0.004530 | 0.001500 |
| signal_set_base.test | LINK | 0.145256 | 0.227573 | 0.263450 | — | — | — |
| signal_set_base_select.test | LINK | 0.149576 | 0.232589 | 0.259293 | — | — | — |
| socket_base.test | RUN | 0.682760 | 3.625369 | 2.821343 | 0.001980 | 0.001545 | 0.001737 |
| socket_base_select.test | RUN | 0.698798 | 3.546248 | 2.730737 | 0.002054 | 0.001772 | 0.001673 |
| spawn.test | RUN | 1.164334 | 5.870173 | 4.928964 | 0.002204 | 0.001919 | 0.001919 |
| spawn_select.test | RUN | 1.185436 | 5.854091 | 4.846500 | 0.002248 | 0.001848 | 0.001610 |
| static_thread_pool.test | RUN | 0.325813 | 0.540998 | 0.629445 | 0.001933 | 0.001394 | 0.001595 |
| static_thread_pool_select.test | RUN | 0.355627 | 0.584287 | 0.791617 | 0.001912 | 0.001414 | 0.001490 |
| steady_timer.test | LINK | 0.365413 | 0.614179 | 0.928028 | — | — | — |
| steady_timer_select.test | LINK | 0.368452 | 0.645832 | 0.943168 | — | — | — |
| strand.test | RUN | 0.730819 | 3.242901 | 2.900927 | 0.002708 | 0.002418 | 0.002394 |
| strand_select.test | RUN | 0.748976 | 3.211048 | 2.863008 | 0.002496 | 0.002182 | 0.002238 |
| stream_file.test | LINK | 0.325951 | 0.583125 | 0.686599 | — | — | — |
| stream_file_select.test | LINK | 0.346039 | 0.593729 | 0.844098 | — | — | — |
| streambuf.test | RUN | 0.199407 | 0.396786 | 0.457638 | 0.001490 | 0.001198 | 0.001156 |
| streambuf_select.test | RUN | 0.149179 | 0.324301 | 0.471021 | 0.001800 | 0.001188 | 0.001062 |
| system_timer.test | LINK | 0.494145 | 2.103835 | 1.685616 | — | — | — |
| system_timer_select.test | LINK | 0.480058 | 2.108611 | 1.639004 | — | — | — |
| system_context.test | LINK | 0.225417 | 0.375972 | 0.391514 | — | — | — |
| system_context_select.test | LINK | 0.249120 | 0.395303 | 0.491753 | — | — | — |
| system_executor.test | LINK | 0.291865 | 1.208758 | 0.776829 | — | — | — |
| system_executor_select.test | LINK | 0.332529 | 1.193651 | 0.874816 | — | — | — |
| this_coro.test | LINK | 0.119768 | 0.142179 | 0.153512 | — | — | — |
| this_coro_select.test | LINK | 0.087567 | 0.145268 | 0.163237 | — | — | — |
| thread_pool.test | RUN | 0.349829 | 1.530019 | 0.996602 | 0.002688 | 0.002212 | 0.002084 |
| thread_pool_select.test | RUN | 0.363502 | 1.576383 | 1.109408 | 0.002554 | 0.002148 | 0.002116 |
| time_traits.test | LINK | 0.195670 | 0.290418 | 0.317374 | — | — | — |
| time_traits_select.test | LINK | 0.203172 | 0.297328 | 0.318268 | — | — | — |
| ts_buffer.test | LINK | 0.193503 | 0.329706 | 0.341692 | — | — | — |
| ts_buffer_select.test | LINK | 0.186099 | 0.335453 | 0.335347 | — | — | — |
| ts_executor.test | LINK | 0.265190 | 0.458728 | 0.528959 | — | — | — |
| ts_executor_select.test | LINK | 0.281626 | 0.489543 | 0.624828 | — | — | — |
| ts_internet.test | LINK | 0.291807 | 0.551881 | 0.640058 | — | — | — |
| ts_internet_select.test | LINK | 0.290300 | 0.725924 | 0.648159 | — | — | — |
| ts_io_context.test | LINK | 0.237735 | 0.549591 | 0.403029 | — | — | — |
| ts_io_context_select.test | LINK | 0.248877 | 0.592661 | 0.529175 | — | — | — |
| ts_net.test | LINK | 0.349973 | 0.853849 | 0.960313 | — | — | — |
| ts_net_select.test | LINK | 0.352461 | 0.885094 | 1.232399 | — | — | — |
| ts_netfwd.test | LINK | 0.341988 | 0.866108 | 1.287701 | — | — | — |
| ts_netfwd_select.test | LINK | 0.354748 | 0.905292 | 1.292327 | — | — | — |
| ts_socket.test | LINK | 0.281001 | 0.688327 | 0.968715 | — | — | — |
| ts_socket_select.test | LINK | 0.277877 | 0.691597 | 1.029335 | — | — | — |
| ts_timer.test | LINK | 0.250758 | 0.604582 | 0.831594 | — | — | — |
| ts_timer_select.test | LINK | 0.240295 | 0.622710 | 0.880847 | — | — | — |
| use_awaitable.test | LINK | 0.085584 | 0.212957 | 0.245675 | — | — | — |
| use_awaitable_select.test | LINK | 0.087253 | 0.193531 | 0.235882 | — | — | — |
| wait_traits.test | LINK | 0.088865 | 0.196472 | 0.253419 | — | — | — |
| wait_traits_select.test | LINK | 0.083050 | 0.201576 | 0.245115 | — | — | — |
| windows_basic_object_handle.test | LINK | 0.083162 | 0.210259 | 0.247158 | — | — | — |
| windows_basic_object_handle_select.test | LINK | 0.085468 | 0.201972 | 0.249681 | — | — | — |
| windows_basic_overlapped_handle.test | LINK | 0.084878 | 0.200186 | 0.248041 | — | — | — |
| windows_basic_overlapped_handle_select.test | LINK | 0.081584 | 0.205853 | 0.244327 | — | — | — |
| windows_basic_random_access_handle.test | LINK | 0.081756 | 0.208106 | 0.247252 | — | — | — |
| windows_basic_random_access_handle_select.test | LINK | 0.083717 | 0.198832 | 0.248596 | — | — | — |
| windows_basic_stream_handle.test | LINK | 0.083724 | 0.201719 | 0.248666 | — | — | — |
| windows_basic_stream_handle_select.test | LINK | 0.086351 | 0.205954 | 0.236807 | — | — | — |
| windows_object_handle.test | LINK | 0.228182 | 0.557029 | 0.662156 | — | — | — |
| windows_object_handle_select.test | LINK | 0.235492 | 0.598226 | 0.812491 | — | — | — |
| windows_overlapped_ptr.test | LINK | 0.237667 | 0.588553 | 0.703595 | — | — | — |
| windows_overlapped_ptr_select.test | LINK | 0.316823 | 0.625424 | 0.862163 | — | — | — |
| windows_random_access_handle.test | LINK | 0.312769 | 0.560542 | 0.655285 | — | — | — |
| windows_random_access_handle_select.test | LINK | 0.337120 | 0.590805 | 0.820496 | — | — | — |
| windows_stream_handle.test | LINK | 0.317423 | 0.561275 | 0.642885 | — | — | — |
| windows_stream_handle_select.test | LINK | 0.325448 | 0.606207 | 0.811176 | — | — | — |
| writable_pipe.test | LINK | 0.561147 | 2.440716 | 2.075130 | — | — | — |
| writable_pipe_select.test | LINK | 0.579020 | 2.565121 | 2.125310 | — | — | — |
| write.test | RUN | 1.555743 | 12.459748 | 11.273172 | 0.002468 | 0.002068 | 0.001956 |
| write_select.test | RUN | 1.588984 | 12.518325 | 11.374133 | 0.002524 | 0.002282 | 0.001998 |
| write_at.test | RUN | 1.603927 | 18.288715 | 14.527555 | 0.002598 | 0.002540 | 0.002259 |
| write_at_select.test | RUN | 1.604534 | 18.398788 | 14.651596 | 0.002551 | 0.002438 | 0.002215 |
| any_executor.test | RUN | 0.681146 | 6.469939 | 4.454465 | 0.002514 | 0.002506 | 0.001924 |
| blocking.test | RUN | 1.489615 | 7.722225 | 9.067592 | 0.006546 | 0.005758 | 0.006014 |
| blocking_adaptation.test | RUN | 0.761937 | 3.280540 | 4.194342 | 0.005293 | 0.003519 | 0.003334 |
| context_as.test | RUN | 0.454485 | 2.452435 | 1.461080 | 0.001966 | 0.001986 | 0.001597 |
| executor.test | RUN | 0.129357 | 0.226043 | 0.267588 | 0.002296 | 0.001560 | 0.001461 |
| inline_exception_handling.test | RUN | 1.402112 | 7.354715 | 8.834884 | 0.004908 | 0.004477 | 0.004225 |
| invocable_archetype.test | RUN | 0.127214 | 0.241160 | 0.251482 | 0.001802 | 0.001817 | 0.001529 |
| mapping.test | RUN | 1.348576 | 7.482843 | 8.920293 | 0.005153 | 0.004595 | 0.004332 |
| outstanding_work.test | RUN | 0.644386 | 3.214219 | 3.970054 | 0.003750 | 0.003748 | 0.003426 |
| prefer_only.test | RUN | 0.236257 | 1.304773 | 0.871005 | 0.001852 | 0.001817 | 0.001485 |
| relationship.test | RUN | 0.533578 | 2.617550 | 3.552593 | 0.002899 | 0.002719 | 0.002499 |

## Native runtime replay

Child CPU of successful native tests, with process startup included; GDB and expected-failure executions are correctness-only. Two warmups per compiler, four A/A observations per reference and mirrored ABBA/BAAB blocks. Short tests repeat up to ten times, identically across compilers; CPU is divided by repetitions before summing tests. Executable hashes and output checks stay outside process timers. All runs must pass their original native oracle. Output hashes record equality or variation across the complete campaign; test reports may print times or addresses.

| Reference | CPU speedup ↑ | Paired blocks | A/A speedup |
|---|---:|---|---:|
| gcc | 0.990× | 0.991×, 0.990× | 1.010× |
| clang | 0.998× | 0.986×, 1.011× | 0.998× |

† A/A drift or paired-block spread exceeds 5%; small differences are unresolved. 1375 native tests in the full cohort; 1364 have byte-identical output throughout. [Raw replay evidence in CI artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43572026855).

| Test | Repeats | CPU cppgm | GCC | Clang | Output check |
|---|---:|---:|---:|---:|---|
| any_completion_executor.test | 2 | 0.001354s | 0.001400s | 0.001367s | byte equality observed |
| any_completion_executor_select.test | 6 | 0.001351s | 0.001353s | 0.001344s | byte equality observed |
| any_completion_handler.test | 10 | 0.001098s | 0.001079s | 0.001096s | byte equality observed |
| any_completion_handler_select.test | 9 | 0.001102s | 0.001065s | 0.001091s | byte equality observed |
| any_io_executor.test | 5 | 0.001336s | 0.001333s | 0.001336s | byte equality observed |
| any_io_executor_select.test | 5 | 0.001367s | 0.001346s | 0.001342s | byte equality observed |
| append.test | 7 | 0.001061s | 0.001036s | 0.001041s | byte equality observed |
| append_select.test | 7 | 0.001037s | 0.001024s | 0.001020s | byte equality observed |
| as_tuple.test | 9 | 0.001125s | 0.001086s | 0.001118s | byte equality observed |
| as_tuple_select.test | 10 | 0.001063s | 0.001010s | 0.001054s | byte equality observed |
| bind_allocator.test | 1 | 0.001239s | 0.001165s | 0.001324s | byte equality observed |
| bind_allocator_select.test | 1 | 0.001122s | 0.001087s | 0.001187s | byte equality observed |
| bind_cancellation_slot.test | 10 | 0.000969s | 0.000934s | 0.000962s | byte equality observed |
| bind_cancellation_slot_select.test | 10 | 0.000930s | 0.000896s | 0.000905s | byte equality observed |
| bind_executor.test | 1 | 0.001202s | 0.001148s | 0.001291s | byte equality observed |
| bind_executor_select.test | 1 | 0.001137s | 0.001093s | 0.001289s | byte equality observed |
| bind_immediate_executor.test | 10 | 0.000931s | 0.000912s | 0.000924s | byte equality observed |
| bind_immediate_executor_select.test | 10 | 0.000921s | 0.000923s | 0.000917s | byte equality observed |
| buffer.test | 10 | 0.000887s | 0.000880s | 0.000892s | byte equality observed |
| buffer_select.test | 10 | 0.000896s | 0.000889s | 0.000872s | byte equality observed |
| buffered_read_stream.test | 10 | 0.001172s | 0.001171s | 0.001180s | byte equality observed |
| buffered_read_stream_select.test | 10 | 0.001143s | 0.001101s | 0.001143s | byte equality observed |
| buffered_stream.test | 10 | 0.001147s | 0.001148s | 0.001172s | byte equality observed |
| buffered_stream_select.test | 10 | 0.001136s | 0.001117s | 0.001133s | byte equality observed |
| buffered_write_stream.test | 10 | 0.001152s | 0.001125s | 0.001146s | byte equality observed |
| buffered_write_stream_select.test | 10 | 0.001145s | 0.001105s | 0.001122s | byte equality observed |
| buffers_iterator.test | 10 | 0.000885s | 0.000864s | 0.000866s | byte equality observed |
| buffers_iterator_select.test | 10 | 0.000848s | 0.000843s | 0.000846s | byte equality observed |
| cancel_after.test | 1 | 0.001763s | 0.001743s | 0.001945s | byte equality observed |
| cancel_after_select.test | 1 | 0.001633s | 0.001613s | 0.001728s | byte equality observed |
| cancel_at.test | 1 | 0.001897s | 0.001888s | 0.002267s | byte equality observed |
| cancel_at_select.test | 1 | 0.001716s | 0.001705s | 0.002084s | byte equality observed |
| co_composed.test | 10 | 0.000852s | 0.000877s | 0.000880s | byte equality observed |
| co_composed_select.test | 10 | 0.000835s | 0.000863s | 0.000841s | byte equality observed |
| compose.test | 1 | 0.001141s | 0.001156s | 0.001240s | byte equality observed |
| compose_select.test | 1 | 0.001052s | 0.001021s | 0.001083s | byte equality observed |
| composed.test | 1 | 0.001121s | 0.001115s | 0.001375s | byte equality observed |
| composed_select.test | 1 | 0.001065s | 0.001035s | 0.001075s | byte equality observed |
| config.test | 10 | 0.000896s | 0.000882s | 0.000898s | byte equality observed |
| config_select.test | 10 | 0.000945s | 0.000928s | 0.000900s | byte equality observed |
| connect_pipe.test | 10 | 0.000971s | 0.000939s | 0.000953s | byte equality observed |
| connect_pipe_select.test | 10 | 0.000938s | 0.000957s | 0.000933s | byte equality observed |
| consign.test | 10 | 0.000954s | 0.000937s | 0.000943s | byte equality observed |
| consign_select.test | 10 | 0.000908s | 0.000880s | 0.000896s | byte equality observed |
| deadline_timer.test | 1 | 0.001892s | 0.001809s | 0.001837s | byte equality observed |
| deadline_timer_select.test | 1 | 0.001707s | 0.001585s | 0.001694s | byte equality observed |
| deferred.test | 10 | 0.000842s | 0.000817s | 0.000863s | byte equality observed |
| deferred_select.test | 10 | 0.000828s | 0.000808s | 0.000841s | byte equality observed |
| disposition.test | 10 | 0.000896s | 0.000865s | 0.000885s | byte equality observed |
| disposition_select.test | 10 | 0.000897s | 0.000867s | 0.000884s | byte equality observed |
| error.test | 10 | 0.000868s | 0.000838s | 0.000837s | byte equality observed |
| error_select.test | 10 | 0.000859s | 0.000862s | 0.000835s | byte equality observed |
| inline_executor.test | 10 | 0.000870s | 0.000864s | 0.000851s | byte equality observed |
| inline_executor_select.test | 10 | 0.000879s | 0.000863s | 0.000865s | byte equality observed |
| inline_or_executor.test | 10 | 0.000868s | 0.000866s | 0.000850s | byte equality observed |
| inline_or_executor_select.test | 10 | 0.000872s | 0.000863s | 0.000858s | byte equality observed |
| io_context.test | 1 | 0.001500s | 0.001404s | 0.001560s | byte equality observed |
| io_context_select.test | 1 | 0.001356s | 0.001318s | 0.001426s | byte equality observed |
| io_context_strand.test | 1 | 0.001778s | 0.001633s | 0.001845s | byte equality observed |
| io_context_strand_select.test | 1 | 0.001411s | 0.001343s | 0.001394s | byte equality observed |
| ip_host_name.test | 10 | 0.000892s | 0.000872s | 0.000878s | byte equality observed |
| ip_host_name_select.test | 10 | 0.000835s | 0.000853s | 0.000836s | byte equality observed |
| ip_icmp.test | 10 | 0.000872s | 0.000879s | 0.000842s | byte equality observed |
| ip_icmp_select.test | 10 | 0.000867s | 0.000879s | 0.000857s | byte equality observed |
| ip_multicast.test | 10 | 0.000938s | 0.000934s | 0.000925s | byte equality observed |
| ip_multicast_select.test | 10 | 0.000916s | 0.000915s | 0.000906s | byte equality observed |
| ip_tcp.test | 10 | 0.001279s | 0.001272s | 0.001275s | byte equality observed |
| ip_tcp_select.test | 10 | 0.001228s | 0.001216s | 0.001219s | byte equality observed |
| ip_udp.test | 10 | 0.000950s | 0.000943s | 0.000941s | byte equality observed |
| ip_udp_select.test | 10 | 0.000950s | 0.000934s | 0.000931s | byte equality observed |
| ip_unicast.test | 10 | 0.000911s | 0.000908s | 0.000890s | byte equality observed |
| ip_unicast_select.test | 10 | 0.000893s | 0.000892s | 0.000885s | byte equality observed |
| ip_v6_only.test | 10 | 0.000912s | 0.000906s | 0.000896s | byte equality observed |
| ip_v6_only_select.test | 10 | 0.000897s | 0.000904s | 0.000881s | byte equality observed |
| is_read_buffered.test | 10 | 0.000847s | 0.000852s | 0.000821s | byte equality observed |
| is_read_buffered_select.test | 10 | 0.000844s | 0.000857s | 0.000824s | byte equality observed |
| is_write_buffered.test | 10 | 0.000847s | 0.000856s | 0.000827s | byte equality observed |
| is_write_buffered_select.test | 10 | 0.000846s | 0.000858s | 0.000822s | byte equality observed |
| prepend.test | 10 | 0.000912s | 0.000937s | 0.000912s | byte equality observed |
| prepend_select.test | 10 | 0.000920s | 0.000935s | 0.000914s | byte equality observed |
| read.test | 10 | 0.001280s | 0.001235s | 0.001255s | byte equality observed |
| read_at.test | 10 | 0.001458s | 0.001503s | 0.001348s | byte equality observed |
| read_at_select.test | 10 | 0.001480s | 0.001491s | 0.001336s | byte equality observed |
| read_select.test | 10 | 0.001278s | 0.001239s | 0.001246s | byte equality observed |
| read_until.test | 10 | 0.001069s | 0.001046s | 0.001017s | byte equality observed |
| read_until_select.test | 10 | 0.001064s | 0.001038s | 0.001040s | byte equality observed |
| recycling_allocator.test | 10 | 0.000870s | 0.000880s | 0.000883s | byte equality observed |
| recycling_allocator_select.test | 10 | 0.000865s | 0.000857s | 0.000887s | byte equality observed |
| signal_set.test | 10 | 0.000913s | 0.000877s | 0.000889s | byte equality observed |
| signal_set_select.test | 10 | 0.000904s | 0.000899s | 0.000875s | byte equality observed |
| socket_base.test | 10 | 0.001014s | 0.000972s | 0.000986s | byte equality observed |
| socket_base_select.test | 10 | 0.000985s | 0.000966s | 0.000967s | byte equality observed |
| spawn.test | 10 | 0.001086s | 0.001068s | 0.001070s | byte equality observed |
| spawn_select.test | 10 | 0.001092s | 0.001059s | 0.001072s | byte equality observed |
| static_thread_pool.test | 10 | 0.000876s | 0.000865s | 0.000874s | byte equality observed |
| static_thread_pool_select.test | 10 | 0.000870s | 0.000837s | 0.000871s | byte equality observed |
| strand.test | 1 | 0.001546s | 0.001473s | 0.001580s | byte equality observed |
| strand_select.test | 1 | 0.001331s | 0.001275s | 0.001380s | byte equality observed |
| streambuf.test | 10 | 0.000920s | 0.000909s | 0.000933s | byte equality observed |
| streambuf_select.test | 10 | 0.000901s | 0.000884s | 0.000903s | byte equality observed |
| thread_pool.test | 10 | 0.001839s | 0.001831s | 0.001852s | byte equality observed |
| thread_pool_select.test | 10 | 0.001829s | 0.001817s | 0.001843s | byte equality observed |
| write.test | 10 | 0.001305s | 0.001263s | 0.001265s | byte equality observed |
| write_at.test | 10 | 0.001374s | 0.001373s | 0.001352s | byte equality observed |
| write_at_select.test | 10 | 0.001368s | 0.001366s | 0.001323s | byte equality observed |
| write_select.test | 10 | 0.001310s | 0.001264s | 0.001283s | byte equality observed |

[Raw outcomes, commands and timing observations in CI artifacts](https://github.com/vishvananda/cppgm-extended/actions/runs/43572026855).

[Comparison records](../results.json) · [Host and toolchains](../metadata.json)
