# libs/asio/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| any_completion_executor.test | RUN | 0.648749 | 3.720883 | 3.504105 | 0.002162 | 0.002606 | 0.002290 |
| any_completion_executor_select.test | RUN | 0.687526 | 3.729149 | 3.711941 | 0.002432 | 0.002280 | 0.002389 |
| any_completion_handler.test | RUN | 0.727339 | 3.372189 | 3.278331 | 0.002002 | 0.001998 | 0.001746 |
| any_completion_handler_select.test | RUN | 0.742948 | 3.468265 | 3.302648 | 0.001605 | 0.002068 | 0.001697 |
| any_io_executor.test | RUN | 0.698199 | 4.020809 | 3.774617 | 0.002137 | 0.002439 | 0.002398 |
| any_io_executor_select.test | RUN | 0.706081 | 3.914301 | 4.034490 | 0.002598 | 0.002471 | 0.002232 |
| append.test | RUN | 0.645620 | 3.142605 | 2.781199 | 0.001534 | 0.001967 | 0.001890 |
| append_select.test | RUN | 0.632978 | 3.197068 | 2.857236 | 0.001758 | 0.001930 | 0.001862 |
| as_tuple.test | RUN | 0.839573 | 3.865954 | 3.948292 | 0.001611 | 0.001963 | 0.001929 |
| as_tuple_select.test | RUN | 0.846912 | 3.831101 | 3.972396 | 0.001994 | 0.001906 | 0.001827 |
| associated_allocator.test | LINK | 0.185937 | 0.328861 | 0.476774 | — | — | — |
| associated_allocator_select.test | LINK | 0.192930 | 0.328461 | 0.479461 | — | — | — |
| associated_cancellation_slot.test | LINK | 0.197421 | 0.356699 | 0.519625 | — | — | — |
| associated_cancellation_slot_select.test | LINK | 0.199609 | 0.364742 | 0.548997 | — | — | — |
| associated_executor.test | LINK | 0.289529 | 0.609568 | 0.823774 | — | — | — |
| associated_executor_select.test | LINK | 0.286952 | 0.623934 | 0.798130 | — | — | — |
| associated_immediate_executor.test | LINK | 0.263421 | 0.575589 | 0.741139 | — | — | — |
| associated_immediate_executor_select.test | LINK | 0.262751 | 0.587301 | 0.696615 | — | — | — |
| associator.test | LINK | 0.153188 | 0.275021 | 0.396751 | — | — | — |
| associator_select.test | LINK | 0.151905 | 0.287571 | 0.379695 | — | — | — |
| awaitable.test | LINK | 0.149205 | 0.271518 | 0.367536 | — | — | — |
| awaitable_select.test | LINK | 0.149119 | 0.275241 | 0.377686 | — | — | — |
| basic_datagram_socket.test | LINK | 0.445585 | 0.886057 | 1.450386 | — | — | — |
| basic_datagram_socket_select.test | LINK | 0.455191 | 0.890579 | 1.465464 | — | — | — |
| basic_deadline_timer.test | LINK | 0.537014 | 1.023625 | 1.625713 | — | — | — |
| basic_deadline_timer_select.test | LINK | 0.546899 | 1.056865 | 1.671444 | — | — | — |
| basic_file.test | LINK | 0.149641 | 0.284581 | 0.376829 | — | — | — |
| basic_file_select.test | LINK | 0.150776 | 0.286334 | 0.377196 | — | — | — |
| basic_random_access_file.test | LINK | 0.147374 | 0.271458 | 0.371856 | — | — | — |
| basic_random_access_file_select.test | LINK | 0.149650 | 0.263687 | 0.349233 | — | — | — |
| basic_raw_socket.test | LINK | 0.449386 | 0.894168 | 1.438806 | — | — | — |
| basic_raw_socket_select.test | LINK | 0.460686 | 0.897863 | 1.464372 | — | — | — |
| basic_readable_pipe.test | LINK | 0.430488 | 0.871854 | 1.247224 | — | — | — |
| basic_readable_pipe_select.test | LINK | 0.455384 | 0.917820 | 1.473321 | — | — | — |
| basic_seq_packet_socket.test | LINK | 0.443170 | 0.874674 | 1.405940 | — | — | — |
| basic_seq_packet_socket_select.test | LINK | 0.464004 | 0.896103 | 1.460033 | — | — | — |
| basic_signal_set.test | LINK | 0.422680 | 0.822453 | 1.120584 | — | — | — |
| basic_signal_set_select.test | LINK | 0.423882 | 0.883246 | 1.375073 | — | — | — |
| basic_socket_acceptor.test | LINK | 0.450925 | 0.884846 | 1.468148 | — | — | — |
| basic_socket_acceptor_select.test | LINK | 0.467236 | 0.907812 | 1.491883 | — | — | — |
| basic_stream_file.test | LINK | 0.147027 | 0.274000 | 0.349196 | — | — | — |
| basic_stream_file_select.test | LINK | 0.151759 | 0.277311 | 0.353975 | — | — | — |
| basic_stream_socket.test | LINK | 0.450829 | 0.890750 | 1.414368 | — | — | — |
| basic_stream_socket_select.test | LINK | 0.466522 | 0.939626 | 1.462083 | — | — | — |
| basic_streambuf.test | LINK | 0.219048 | 0.398607 | 0.574363 | — | — | — |
| basic_streambuf_select.test | LINK | 0.233009 | 0.404000 | 0.571929 | — | — | — |
| basic_waitable_timer.test | LINK | 0.414047 | 0.811150 | 1.344733 | — | — | — |
| basic_waitable_timer_select.test | LINK | 0.427647 | 0.860793 | 1.350971 | — | — | — |
| basic_writable_pipe.test | LINK | 0.435083 | 0.872045 | 1.251551 | — | — | — |
| basic_writable_pipe_select.test | LINK | 0.442069 | 0.921595 | 1.509872 | — | — | — |
| bind_allocator.test | RUN | 0.682157 | 3.222552 | 3.135842 | 0.002276 | 0.002362 | 0.002190 |
| bind_allocator_select.test | RUN | 0.673196 | 3.256814 | 3.128588 | 0.002073 | 0.002034 | 0.001446 |
| bind_cancellation_slot.test | RUN | 0.702317 | 3.253917 | 3.205376 | 0.001561 | 0.001495 | 0.001445 |
| bind_cancellation_slot_select.test | RUN | 0.706213 | 3.209238 | 3.062113 | 0.001399 | 0.001472 | 0.001399 |
| bind_executor.test | RUN | 0.463985 | 2.241685 | 1.893574 | 0.002120 | 0.002196 | 0.002073 |
| bind_executor_select.test | RUN | 0.471666 | 2.268295 | 1.899964 | 0.001806 | 0.001958 | 0.001844 |
| bind_immediate_executor.test | RUN | 0.355119 | 1.858356 | 1.201432 | 0.001407 | 0.001513 | 0.001359 |
| bind_immediate_executor_select.test | RUN | 0.369173 | 1.835931 | 1.333885 | 0.001419 | 0.001378 | 0.001319 |
| buffer.test | RUN | 0.218220 | 0.787870 | 0.749315 | 0.001424 | 0.001377 | 0.001309 |
| buffer_select.test | RUN | 0.220546 | 0.787069 | 0.752059 | 0.001414 | 0.001316 | 0.001277 |
| buffer_registration.test | LINK | 0.189591 | 0.399661 | 0.441059 | — | — | — |
| buffer_registration_select.test | LINK | 0.187382 | 0.395872 | 0.445110 | — | — | — |
| buffered_read_stream.test | RUN | 0.753218 | 4.921508 | 4.540290 | 0.001754 | 0.002484 | 0.002008 |
| buffered_read_stream_select.test | RUN | 0.763670 | 5.063675 | 4.448836 | 0.001737 | 0.002452 | 0.002194 |
| buffered_stream.test | RUN | 1.093262 | 7.660277 | 7.644358 | 0.002422 | 0.002312 | 0.002336 |
| buffered_stream_select.test | RUN | 1.198132 | 7.660658 | 7.381332 | 0.002406 | 0.002404 | 0.002130 |
| buffered_write_stream.test | RUN | 1.252401 | 6.578290 | 6.557401 | 0.002113 | 0.002196 | 0.002260 |
| buffered_write_stream_select.test | RUN | 1.277734 | 6.690831 | 6.300335 | 0.001983 | 0.002346 | 0.002238 |
| buffers_iterator.test | RUN | 0.335146 | 0.623966 | 1.066194 | 0.001858 | 0.001839 | 0.001694 |
| buffers_iterator_select.test | RUN | 0.333234 | 0.657246 | 1.069401 | 0.001596 | 0.001831 | 0.001718 |
| cancel_after.test | RUN | 1.177771 | 5.176011 | 4.856126 | 0.004009 | 0.003876 | 0.004012 |
| cancel_after_select.test | RUN | 1.180864 | 5.155061 | 4.805632 | 0.004026 | 0.003907 | 0.003512 |
| cancel_at.test | RUN | 1.177749 | 4.630915 | 4.594692 | 0.003291 | 0.003552 | 0.003390 |
| cancel_at_select.test | RUN | 1.112953 | 4.465529 | 4.526296 | 0.003138 | 0.003331 | 0.003296 |
| cancellation_signal.test | LINK | 0.132913 | 0.234876 | 0.287342 | — | — | — |
| cancellation_signal_select.test | LINK | 0.189613 | 0.239187 | 0.288875 | — | — | — |
| cancellation_state.test | LINK | 0.188612 | 0.235833 | 0.402568 | — | — | — |
| cancellation_state_select.test | LINK | 0.191563 | 0.237648 | 0.397073 | — | — | — |
| cancellation_type.test | LINK | 0.148676 | 0.187102 | 0.378286 | — | — | — |
| cancellation_type_select.test | LINK | 0.150638 | 0.186758 | 0.374856 | — | — | — |
| co_composed.test | RUN | 0.146256 | 0.189957 | 0.391494 | 0.001568 | 0.001316 | 0.001474 |
| co_composed_select.test | RUN | 0.146877 | 0.199196 | 0.391090 | 0.001423 | 0.001699 | 0.001332 |
| co_spawn.test | LINK | 0.147132 | 0.211935 | 0.362596 | — | — | — |
| co_spawn_select.test | LINK | 0.152149 | 0.259641 | 0.371276 | — | — | — |
| completion_condition.test | LINK | 0.193163 | 0.427253 | 0.523116 | — | — | — |
| completion_condition_select.test | LINK | 0.202978 | 0.430548 | 0.519054 | — | — | — |
| compose.test | RUN | 0.841344 | 4.005411 | 4.013231 | 0.002079 | 0.002296 | 0.002316 |
| compose_select.test | RUN | 0.836919 | 3.959753 | 4.006280 | 0.002246 | 0.001925 | 0.001826 |
| composed.test | RUN | 0.823856 | 3.925521 | 3.846363 | 0.001888 | 0.002022 | 0.001582 |
| composed_select.test | RUN | 0.779318 | 3.877562 | 3.853458 | 0.001536 | 0.001547 | 0.001890 |
| config.test | RUN | 0.370256 | 2.493969 | 1.645578 | 0.002107 | 0.001973 | 0.001818 |
| config_select.test | RUN | 0.513242 | 2.678285 | 1.971817 | 0.001903 | 0.001918 | 0.001782 |
| connect.test | LINK | 1.391796 | 8.404554 | 8.194840 | — | — | — |
| connect_select.test | LINK | 1.346293 | 8.354753 | 8.241958 | — | — | — |
| connect_pipe.test | RUN | 0.749412 | 3.192873 | 3.067361 | 0.002046 | 0.001980 | 0.001888 |
| connect_pipe_select.test | RUN | 0.751006 | 3.330632 | 3.277372 | 0.001896 | 0.002007 | 0.001718 |
| consign.test | RUN | 0.632149 | 3.077778 | 2.756453 | 0.001846 | 0.001924 | 0.001857 |
| consign_select.test | RUN | 0.648856 | 3.137292 | 2.714342 | 0.001897 | 0.001861 | 0.001565 |
| coroutine.test | LINK | 0.156851 | 0.327985 | 0.410381 | — | — | — |
| coroutine_select.test | LINK | 0.157135 | 0.319404 | 0.411044 | — | — | — |
| deadline_timer.test | RUN | 0.943189 | 4.078018 | 4.052757 | 0.003263 | 0.003411 | 0.002744 |
| deadline_timer_select.test | RUN | 0.937455 | 4.073786 | 4.013454 | 0.002936 | 0.002684 | 0.003244 |
| deferred.test | RUN | 0.162695 | 0.290996 | 0.402102 | 0.001844 | 0.001586 | 0.001742 |
| deferred_select.test | RUN | 0.180534 | 0.305509 | 0.399069 | 0.001700 | 0.001809 | 0.001708 |
| detached.test | LINK | 0.194000 | 0.337387 | 0.440377 | — | — | — |
| detached_select.test | LINK | 0.190843 | 0.339902 | 0.447234 | — | — | — |
| disposition.test | RUN | 0.233329 | 0.752955 | 0.805391 | 0.001417 | 0.001980 | 0.001848 |
| disposition_select.test | RUN | 0.173314 | 0.797507 | 0.799713 | 0.001461 | 0.001835 | 0.001806 |
| error.test | RUN | 0.163382 | 0.762083 | 0.703465 | 0.001381 | 0.001851 | 0.001866 |
| error_select.test | RUN | 0.161475 | 0.760170 | 0.712556 | 0.001483 | 0.001824 | 0.001681 |
| file_base.test | LINK | 0.211029 | 0.633949 | 0.792349 | — | — | — |
| file_base_select.test | LINK | 0.207219 | 0.625685 | 0.641598 | — | — | — |
| fwd_first.test | LINK | 0.474061 | 1.017624 | 1.379008 | — | — | — |
| fwd_last.test | LINK | 0.514921 | 0.899468 | 1.385072 | — | — | — |
| generic_basic_endpoint.test | LINK | 0.184171 | 0.373909 | 0.502569 | — | — | — |
| generic_basic_endpoint_select.test | LINK | 0.177710 | 0.371883 | 0.504457 | — | — | — |
| generic_datagram_protocol.test | LINK | 0.645961 | 3.218058 | 2.706893 | — | — | — |
| generic_datagram_protocol_select.test | LINK | 0.650037 | 3.305460 | 2.678780 | — | — | — |
| generic_raw_protocol.test | LINK | 0.668300 | 3.220893 | 2.690688 | — | — | — |
| generic_raw_protocol_select.test | LINK | 0.665954 | 3.309221 | 2.693678 | — | — | — |
| generic_seq_packet_protocol.test | LINK | 0.571332 | 2.719167 | 2.309723 | — | — | — |
| generic_seq_packet_protocol_select.test | LINK | 0.581662 | 2.749915 | 2.316490 | — | — | — |
| generic_stream_protocol.test | LINK | 0.611671 | 2.963801 | 2.477454 | — | — | — |
| generic_stream_protocol_select.test | LINK | 0.601931 | 3.001093 | 2.496529 | — | — | — |
| high_resolution_timer.test | LINK | 0.287688 | 0.554884 | 0.784902 | — | — | — |
| high_resolution_timer_select.test | LINK | 0.290991 | 0.565850 | 0.800914 | — | — | — |
| immediate.test | LINK | 0.208199 | 0.426724 | 0.487624 | — | — | — |
| immediate_select.test | LINK | 0.206370 | 0.429570 | 0.487527 | — | — | — |
| inline_executor.test | RUN | 0.246199 | 0.651131 | 0.719290 | 0.001401 | 0.001429 | 0.001326 |
| inline_executor_select.test | RUN | 0.243033 | 0.653955 | 0.728706 | 0.001404 | 0.001402 | 0.001328 |
| inline_or_executor.test | RUN | 0.382762 | 1.897140 | 1.368471 | 0.001440 | 0.001415 | 0.001314 |
| inline_or_executor_select.test | RUN | 0.394147 | 1.957677 | 1.500405 | 0.001430 | 0.001428 | 0.001340 |
| io_context.test | RUN | 0.536420 | 2.905989 | 2.390053 | 0.002338 | 0.002312 | 0.002216 |
| io_context_select.test | RUN | 0.541191 | 2.846661 | 2.379014 | 0.002223 | 0.002150 | 0.002026 |
| io_context_strand.test | RUN | 0.503410 | 2.455254 | 2.083706 | 0.002286 | 0.002330 | 0.002218 |
| io_context_strand_select.test | RUN | 0.512567 | 2.431465 | 2.097604 | 0.002210 | 0.002073 | 0.001980 |
| ip_address.test | LINK | 0.200236 | 0.575837 | 0.606605 | — | — | — |
| ip_address_select.test | LINK | 0.204733 | 0.573239 | 0.602915 | — | — | — |
| ip_address_v4.test | LINK | 0.210681 | 0.612673 | 0.665840 | — | — | — |
| ip_address_v4_select.test | LINK | 0.209240 | 0.607935 | 0.658316 | — | — | — |
| ip_address_v6.test | LINK | 0.229799 | 0.794589 | 0.812242 | — | — | — |
| ip_address_v6_select.test | LINK | 0.229004 | 0.790749 | 0.812312 | — | — | — |
| ip_basic_endpoint.test | LINK | 0.186405 | 0.389945 | 0.522957 | — | — | — |
| ip_basic_endpoint_select.test | LINK | 0.188043 | 0.391805 | 0.524555 | — | — | — |
| ip_basic_resolver.test | LINK | 0.292084 | 0.562845 | 1.320101 | — | — | — |
| ip_basic_resolver_select.test | LINK | 0.295378 | 0.579706 | 1.091791 | — | — | — |
| ip_basic_resolver_entry.test | LINK | 0.102833 | 0.187653 | 0.386257 | — | — | — |
| ip_basic_resolver_entry_select.test | LINK | 0.102559 | 0.187469 | 0.393942 | — | — | — |
| ip_basic_resolver_iterator.test | LINK | 0.183894 | 0.382793 | 0.884405 | — | — | — |
| ip_basic_resolver_iterator_select.test | LINK | 0.183361 | 0.386291 | 0.854964 | — | — | — |
| ip_basic_resolver_query.test | LINK | 0.175496 | 0.371200 | 0.884502 | — | — | — |
| ip_basic_resolver_query_select.test | LINK | 0.178727 | 0.368594 | 0.852877 | — | — | — |
| ip_host_name.test | RUN | 0.184533 | 0.645311 | 0.957539 | 0.001347 | 0.001858 | 0.001661 |
| ip_host_name_select.test | RUN | 0.186563 | 0.497270 | 0.962867 | 0.001412 | 0.001573 | 0.001642 |
| ip_icmp.test | RUN | 0.804515 | 6.642800 | 5.725127 | 0.001402 | 0.001919 | 0.001600 |
| ip_icmp_select.test | RUN | 0.807998 | 6.827780 | 5.579773 | 0.001548 | 0.001950 | 0.001524 |
| ip_multicast.test | RUN | 0.493837 | 3.385186 | 3.368148 | 0.001520 | 0.001908 | 0.001678 |
| ip_multicast_select.test | RUN | 0.490295 | 3.350998 | 3.304137 | 0.001526 | 0.002004 | 0.002108 |
| ip_resolver_query_base.test | LINK | 0.119755 | 0.314528 | 0.376213 | — | — | — |
| ip_resolver_query_base_select.test | LINK | 0.118441 | 0.316959 | 0.385211 | — | — | — |
| ip_tcp.test | RUN | 1.239872 | 9.777286 | 8.874799 | 0.002016 | 0.002593 | 0.002565 |
| ip_tcp_select.test | RUN | 1.199753 | 10.127616 | 8.615278 | 0.001924 | 0.002670 | 0.002476 |
| ip_udp.test | RUN | 0.844074 | 7.004932 | 6.100508 | 0.001525 | 0.002048 | 0.001855 |
| ip_udp_select.test | RUN | 0.839859 | 7.126133 | 5.948779 | 0.001575 | 0.002180 | 0.002000 |
| ip_unicast.test | RUN | 0.449314 | 3.054118 | 2.866516 | 0.001485 | 0.001982 | 0.001930 |
| ip_unicast_select.test | RUN | 0.450096 | 3.116591 | 2.854168 | 0.001480 | 0.002071 | 0.001705 |
| ip_v6_only.test | RUN | 0.463221 | 3.186246 | 3.037221 | 0.001476 | 0.002041 | 0.001811 |
| ip_v6_only_select.test | RUN | 0.468360 | 3.263235 | 3.008203 | 0.001468 | 0.001895 | 0.001676 |
| is_read_buffered.test | RUN | 0.337442 | 0.991814 | 1.627226 | 0.001406 | 0.001814 | 0.001594 |
| is_read_buffered_select.test | RUN | 0.383862 | 0.988585 | 1.697959 | 0.001417 | 0.001816 | 0.001455 |
| is_write_buffered.test | RUN | 0.350892 | 0.995091 | 1.607798 | 0.001412 | 0.001848 | 0.001822 |
| is_write_buffered_select.test | RUN | 0.344234 | 1.009526 | 1.661090 | 0.001400 | 0.001698 | 0.001724 |
| local_basic_endpoint.test | LINK | 0.177424 | 0.566901 | 0.881397 | — | — | — |
| local_basic_endpoint_select.test | LINK | 0.177805 | 0.555199 | 0.888947 | — | — | — |
| local_connect_pair.test | LINK | 0.426298 | 3.095940 | 2.807086 | — | — | — |
| local_connect_pair_select.test | LINK | 0.466304 | 3.117663 | 2.788066 | — | — | — |
| local_datagram_protocol.test | LINK | 0.837482 | 4.052345 | 3.908053 | — | — | — |
| local_datagram_protocol_select.test | LINK | 0.808892 | 4.085267 | 3.818898 | — | — | — |
| local_seq_packet_protocol.test | LINK | 0.822742 | 3.924048 | 3.817806 | — | — | — |
| local_seq_packet_protocol_select.test | LINK | 0.810397 | 3.993244 | 3.817888 | — | — | — |
| local_stream_protocol.test | LINK | 0.771872 | 3.739356 | 3.633920 | — | — | — |
| local_stream_protocol_select.test | LINK | 0.794826 | 3.784084 | 3.608685 | — | — | — |
| placeholders.test | LINK | 0.163424 | 0.309080 | 0.427705 | — | — | — |
| placeholders_select.test | LINK | 0.173744 | 0.315216 | 0.434295 | — | — | — |
| posix_basic_descriptor.test | LINK | 0.441469 | 0.858331 | 1.204752 | — | — | — |
| posix_basic_descriptor_select.test | LINK | 0.469459 | 0.889578 | 1.499157 | — | — | — |
| posix_basic_stream_descriptor.test | LINK | 0.433505 | 0.846267 | 1.198906 | — | — | — |
| posix_basic_stream_descriptor_select.test | LINK | 0.443401 | 0.912578 | 1.485110 | — | — | — |
| posix_descriptor_base.test | LINK | 0.177901 | 0.306551 | 0.425232 | — | — | — |
| posix_descriptor_base_select.test | LINK | 0.183074 | 0.315727 | 0.432617 | — | — | — |
| posix_stream_descriptor.test | LINK | 0.803861 | 3.804789 | 3.594517 | — | — | — |
| posix_stream_descriptor_select.test | LINK | 0.810529 | 3.857561 | 3.723602 | — | — | — |
| prepend.test | RUN | 0.635943 | 3.132482 | 2.863052 | 0.001745 | 0.001886 | 0.001848 |
| prepend_select.test | RUN | 0.638979 | 3.173506 | 2.851982 | 0.001697 | 0.001888 | 0.001790 |
| random_access_file.test | LINK | 0.387443 | 0.745208 | 0.939904 | — | — | — |
| random_access_file_select.test | LINK | 0.411660 | 0.792016 | 1.240631 | — | — | — |
| read.test | RUN | 1.926749 | 19.140956 | 16.286518 | 0.002621 | 0.002502 | 0.002163 |
| read_select.test | RUN | 1.957872 | 19.021113 | 16.519524 | 0.002599 | 0.002330 | 0.002567 |
| read_at.test | RUN | 1.928166 | 26.977509 | 21.838669 | 0.003297 | 0.003118 | 0.002584 |
| read_at_select.test | RUN | 1.958497 | 26.472745 | 21.970352 | 0.003022 | 0.003175 | 0.002880 |
| read_until.test | RUN | 1.073349 | 7.345798 | 6.344349 | 0.001885 | 0.001976 | 0.002122 |
| read_until_select.test | RUN | 1.090896 | 7.413846 | 6.580399 | 0.001923 | 0.002068 | 0.002026 |
| readable_pipe.test | LINK | 0.663764 | 3.149958 | 2.798827 | — | — | — |
| readable_pipe_select.test | LINK | 0.680019 | 3.324388 | 3.018793 | — | — | — |
| recycling_allocator.test | RUN | 0.214187 | 0.492357 | 0.621484 | 0.001727 | 0.001765 | 0.001712 |
| recycling_allocator_select.test | RUN | 0.212480 | 0.493487 | 0.620763 | 0.001448 | 0.001846 | 0.001798 |
| redirect_disposition.test | LINK | 0.908627 | 4.462559 | 4.361216 | — | — | — |
| redirect_disposition_select.test | LINK | 0.894218 | 4.418553 | 4.356860 | — | — | — |
| redirect_error.test | LINK | 0.788474 | 3.709377 | 3.801956 | — | — | — |
| redirect_error_select.test | LINK | 0.798820 | 3.682047 | 3.727800 | — | — | — |
| registered_buffer.test | LINK | 0.209166 | 0.438686 | 0.548669 | — | — | — |
| registered_buffer_select.test | LINK | 0.213267 | 0.432814 | 0.543377 | — | — | — |
| signal_set.test | RUN | 0.617329 | 3.100351 | 2.645128 | 0.001756 | 0.001862 | 0.001804 |
| signal_set_select.test | RUN | 0.646410 | 3.204070 | 2.915181 | 0.001822 | 0.001845 | 0.001616 |
| signal_set_base.test | LINK | 0.177583 | 0.321327 | 0.415144 | — | — | — |
| signal_set_base_select.test | LINK | 0.181666 | 0.325815 | 0.414863 | — | — | — |
| socket_base.test | RUN | 0.802330 | 4.628628 | 4.088167 | 0.002024 | 0.001978 | 0.001990 |
| socket_base_select.test | RUN | 0.804108 | 4.502083 | 4.063159 | 0.001846 | 0.001926 | 0.001895 |
| spawn.test | RUN | 1.314551 | 7.355024 | 6.903987 | 0.002022 | 0.002126 | 0.002216 |
| spawn_select.test | RUN | 1.313669 | 7.392079 | 6.878724 | 0.002018 | 0.002076 | 0.002109 |
| static_thread_pool.test | RUN | 0.371249 | 0.727795 | 0.974143 | 0.001762 | 0.001860 | 0.001844 |
| static_thread_pool_select.test | RUN | 0.391487 | 0.778805 | 1.254856 | 0.001724 | 0.001822 | 0.001748 |
| steady_timer.test | LINK | 0.425473 | 0.842213 | 1.361796 | — | — | — |
| steady_timer_select.test | LINK | 0.415535 | 0.848701 | 1.406696 | — | — | — |
| strand.test | RUN | 0.861471 | 4.210371 | 4.104671 | 0.002450 | 0.002645 | 0.002612 |
| strand_select.test | RUN | 0.848674 | 4.182155 | 4.112289 | 0.002168 | 0.002433 | 0.002748 |
| stream_file.test | LINK | 0.399858 | 0.770554 | 1.015550 | — | — | — |
| stream_file_select.test | LINK | 0.420052 | 0.817202 | 1.276197 | — | — | — |
| streambuf.test | RUN | 0.198153 | 0.647256 | 0.689773 | 0.001361 | 0.001571 | 0.001433 |
| streambuf_select.test | RUN | 0.194225 | 0.543027 | 0.665707 | 0.001121 | 0.001373 | 0.001295 |
| system_timer.test | LINK | 0.540726 | 2.715864 | 2.361520 | — | — | — |
| system_timer_select.test | LINK | 0.554952 | 2.675391 | 2.298105 | — | — | — |
| system_context.test | LINK | 0.255805 | 0.492489 | 0.578015 | — | — | — |
| system_context_select.test | LINK | 0.270566 | 0.528399 | 0.725885 | — | — | — |
| system_executor.test | LINK | 0.331379 | 1.556882 | 1.097908 | — | — | — |
| system_executor_select.test | LINK | 0.339149 | 1.529314 | 1.236240 | — | — | — |
| this_coro.test | LINK | 0.104795 | 0.189238 | 0.234605 | — | — | — |
| this_coro_select.test | LINK | 0.106612 | 0.194420 | 0.228268 | — | — | — |
| thread_pool.test | RUN | 0.383767 | 1.985107 | 1.414478 | 0.002571 | 0.002491 | 0.002568 |
| thread_pool_select.test | RUN | 0.395396 | 2.012633 | 1.545894 | 0.002540 | 0.002491 | 0.002512 |
| time_traits.test | LINK | 0.236482 | 0.534278 | 0.470807 | — | — | — |
| time_traits_select.test | LINK | 0.239417 | 0.589172 | 0.467570 | — | — | — |
| ts_buffer.test | LINK | 0.220652 | 0.592574 | 0.498345 | — | — | — |
| ts_buffer_select.test | LINK | 0.220853 | 0.648511 | 0.554397 | — | — | — |
| ts_executor.test | LINK | 0.300255 | 0.892397 | 1.186347 | — | — | — |
| ts_executor_select.test | LINK | 0.318758 | 0.951639 | 1.390616 | — | — | — |
| ts_internet.test | LINK | 0.334051 | 0.969749 | 1.622082 | — | — | — |
| ts_internet_select.test | LINK | 0.336342 | 1.009427 | 1.661900 | — | — | — |
| ts_io_context.test | LINK | 0.266055 | 0.737146 | 0.979734 | — | — | — |
| ts_io_context_select.test | LINK | 0.277825 | 0.784243 | 1.259483 | — | — | — |
| ts_net.test | LINK | 0.389819 | 1.179473 | 1.997577 | — | — | — |
| ts_net_select.test | LINK | 0.403193 | 1.201073 | 2.037421 | — | — | — |
| ts_netfwd.test | LINK | 0.389893 | 1.181830 | 1.999786 | — | — | — |
| ts_netfwd_select.test | LINK | 0.402557 | 1.214568 | 2.042437 | — | — | — |
| ts_socket.test | LINK | 0.325887 | 0.920445 | 1.533103 | — | — | — |
| ts_socket_select.test | LINK | 0.336438 | 0.951149 | 1.558620 | — | — | — |
| ts_timer.test | LINK | 0.285645 | 0.829754 | 1.360780 | — | — | — |
| ts_timer_select.test | LINK | 0.293491 | 0.849438 | 1.342125 | — | — | — |
| use_awaitable.test | LINK | 0.105192 | 0.293459 | 0.402955 | — | — | — |
| use_awaitable_select.test | LINK | 0.104523 | 0.290448 | 0.381079 | — | — | — |
| wait_traits.test | LINK | 0.107172 | 0.282035 | 0.388107 | — | — | — |
| wait_traits_select.test | LINK | 0.104384 | 0.282620 | 0.366750 | — | — | — |
| windows_basic_object_handle.test | LINK | 0.107499 | 0.276814 | 0.378638 | — | — | — |
| windows_basic_object_handle_select.test | LINK | 0.104169 | 0.283045 | 0.363867 | — | — | — |
| windows_basic_overlapped_handle.test | LINK | 0.104461 | 0.282851 | 0.382450 | — | — | — |
| windows_basic_overlapped_handle_select.test | LINK | 0.104050 | 0.280583 | 0.369634 | — | — | — |
| windows_basic_random_access_handle.test | LINK | 0.104543 | 0.280196 | 0.360326 | — | — | — |
| windows_basic_random_access_handle_select.test | LINK | 0.104563 | 0.288036 | 0.378572 | — | — | — |
| windows_basic_stream_handle.test | LINK | 0.104217 | 0.280269 | 0.385081 | — | — | — |
| windows_basic_stream_handle_select.test | LINK | 0.146398 | 0.278956 | 0.368596 | — | — | — |
| windows_object_handle.test | LINK | 0.385801 | 0.755253 | 0.973696 | — | — | — |
| windows_object_handle_select.test | LINK | 0.403527 | 0.795152 | 1.277664 | — | — | — |
| windows_overlapped_ptr.test | LINK | 0.402934 | 0.812703 | 1.115780 | — | — | — |
| windows_overlapped_ptr_select.test | LINK | 0.424004 | 0.870805 | 1.371151 | — | — | — |
| windows_random_access_handle.test | LINK | 0.387726 | 0.747526 | 0.984724 | — | — | — |
| windows_random_access_handle_select.test | LINK | 0.406128 | 0.803867 | 1.254695 | — | — | — |
| windows_stream_handle.test | LINK | 0.390916 | 0.770118 | 1.024276 | — | — | — |
| windows_stream_handle_select.test | LINK | 0.408861 | 0.811844 | 1.260462 | — | — | — |
| writable_pipe.test | LINK | 0.668183 | 3.123577 | 2.965397 | — | — | — |
| writable_pipe_select.test | LINK | 0.680308 | 3.244588 | 3.141380 | — | — | — |
| write.test | RUN | 1.953828 | 15.965208 | 16.292253 | 0.002683 | 0.002371 | 0.002358 |
| write_select.test | RUN | 1.963198 | 16.033052 | 16.368538 | 0.002635 | 0.002458 | 0.002483 |
| write_at.test | RUN | 2.029536 | 22.589491 | 20.800915 | 0.002471 | 0.002520 | 0.002022 |
| write_at_select.test | RUN | 2.048720 | 22.412569 | 20.818219 | 0.002736 | 0.002350 | 0.002579 |
| any_executor.test | RUN | 0.827576 | 7.305362 | 6.202199 | 0.002224 | 0.002462 | 0.002351 |
| blocking.test | RUN | 2.032994 | 9.212349 | 13.428429 | 0.007387 | 0.006333 | 0.007406 |
| blocking_adaptation.test | RUN | 0.999516 | 4.002985 | 6.231859 | 0.003964 | 0.004151 | 0.004043 |
| context_as.test | RUN | 0.546671 | 2.844247 | 2.096755 | 0.002136 | 0.002128 | 0.001595 |
| executor.test | RUN | 0.153984 | 0.301327 | 0.417025 | 0.001716 | 0.001735 | 0.001673 |
| inline_exception_handling.test | RUN | 2.007968 | 8.403330 | 13.368326 | 0.007478 | 0.006767 | 0.005060 |
| invocable_archetype.test | RUN | 0.163838 | 0.294112 | 0.388553 | 0.001537 | 0.001788 | 0.001792 |
| mapping.test | RUN | 1.778963 | 9.197440 | 13.277232 | 0.005275 | 0.005216 | 0.005226 |
| outstanding_work.test | RUN | 0.894242 | 3.774560 | 6.001220 | 0.003865 | 0.004081 | 0.004305 |
| prefer_only.test | RUN | 0.321716 | 1.456451 | 1.361130 | 0.001766 | 0.001891 | 0.001700 |
| relationship.test | RUN | 0.666701 | 3.794770 | 4.844543 | 0.003034 | 0.002917 | 0.002888 |

## Native runtime replay

Child CPU of successful native tests, with process startup included; GDB and expected-failure executions are correctness-only. Two warmups per compiler, four A/A observations per reference and mirrored ABBA/BAAB blocks. Short tests repeat up to ten times, identically across compilers; CPU is divided by repetitions before summing tests. Executable hashes and output checks stay outside process timers. All runs must pass their original native oracle. Output hashes record equality or variation across the complete campaign; test reports may print times or addresses.

| Reference | CPU speedup ↑ | Paired blocks | A/A speedup |
|---|---:|---|---:|
| gcc | 0.988× | 0.987×, 0.990× | 1.005× |
| clang | 0.981× | 0.979×, 0.983× | 0.998× |

† A/A drift or paired-block spread exceeds 5%; small differences are unresolved. 1375 native tests in the full cohort; 1363 have byte-identical output throughout. [Manifest](https://github.com/vishvananda/cppgm-extended/actions/runs/42735798837), [paired observations](https://github.com/vishvananda/cppgm-extended/actions/runs/42735798837) and [per-test summaries](https://github.com/vishvananda/cppgm-extended/actions/runs/42735798837).

| Test | Repeats | CPU cppgm | GCC | Clang | Output check |
|---|---:|---:|---:|---:|---|
| any_completion_executor.test | 5 | 0.001667s | 0.001654s | 0.001682s | byte equality observed |
| any_completion_executor_select.test | 3 | 0.001646s | 0.001633s | 0.001657s | byte equality observed |
| any_completion_handler.test | 6 | 0.001330s | 0.001316s | 0.001313s | byte equality observed |
| any_completion_handler_select.test | 10 | 0.001321s | 0.001310s | 0.001276s | byte equality observed |
| any_io_executor.test | 2 | 0.001633s | 0.001589s | 0.001657s | byte equality observed |
| any_io_executor_select.test | 4 | 0.001652s | 0.001641s | 0.001673s | byte equality observed |
| append.test | 9 | 0.001265s | 0.001256s | 0.001233s | byte equality observed |
| append_select.test | 10 | 0.001247s | 0.001212s | 0.001236s | byte equality observed |
| as_tuple.test | 8 | 0.001333s | 0.001322s | 0.001325s | byte equality observed |
| as_tuple_select.test | 8 | 0.001277s | 0.001257s | 0.001181s | byte equality observed |
| bind_allocator.test | 1 | 0.001721s | 0.001687s | 0.001760s | byte equality observed |
| bind_allocator_select.test | 1 | 0.001439s | 0.001375s | 0.001325s | byte equality observed |
| bind_cancellation_slot.test | 10 | 0.001177s | 0.001181s | 0.001151s | byte equality observed |
| bind_cancellation_slot_select.test | 10 | 0.001103s | 0.001082s | 0.001071s | byte equality observed |
| bind_executor.test | 1 | 0.001704s | 0.001707s | 0.001664s | byte equality observed |
| bind_executor_select.test | 1 | 0.002025s | 0.001972s | 0.001997s | byte equality observed |
| bind_immediate_executor.test | 10 | 0.001142s | 0.001128s | 0.001107s | byte equality observed |
| bind_immediate_executor_select.test | 10 | 0.001103s | 0.001102s | 0.001110s | byte equality observed |
| buffer.test | 10 | 0.001068s | 0.001058s | 0.001070s | byte equality observed |
| buffer_select.test | 10 | 0.001054s | 0.001049s | 0.001037s | byte equality observed |
| buffered_read_stream.test | 10 | 0.001389s | 0.001366s | 0.001360s | byte equality observed |
| buffered_read_stream_select.test | 10 | 0.001318s | 0.001320s | 0.001337s | byte equality observed |
| buffered_stream.test | 10 | 0.001378s | 0.001370s | 0.001396s | byte equality observed |
| buffered_stream_select.test | 10 | 0.001352s | 0.001336s | 0.001345s | byte equality observed |
| buffered_write_stream.test | 10 | 0.001368s | 0.001389s | 0.001336s | byte equality observed |
| buffered_write_stream_select.test | 10 | 0.001354s | 0.001304s | 0.001362s | byte equality observed |
| buffers_iterator.test | 10 | 0.001046s | 0.001030s | 0.001031s | byte equality observed |
| buffers_iterator_select.test | 10 | 0.001016s | 0.001037s | 0.001014s | byte equality observed |
| cancel_after.test | 1 | 0.003575s | 0.003513s | 0.003482s | byte equality observed |
| cancel_after_select.test | 1 | 0.003009s | 0.002853s | 0.002986s | byte equality observed |
| cancel_at.test | 1 | 0.003672s | 0.003659s | 0.003730s | byte equality observed |
| cancel_at_select.test | 1 | 0.003795s | 0.003421s | 0.003540s | byte equality observed |
| co_composed.test | 10 | 0.001064s | 0.001053s | 0.001044s | byte equality observed |
| co_composed_select.test | 10 | 0.001022s | 0.001002s | 0.000993s | byte equality observed |
| compose.test | 1 | 0.001744s | 0.001705s | 0.001788s | byte equality observed |
| compose_select.test | 1 | 0.001463s | 0.001418s | 0.001455s | byte equality observed |
| composed.test | 1 | 0.002113s | 0.002075s | 0.002099s | byte equality observed |
| composed_select.test | 1 | 0.001572s | 0.001531s | 0.001601s | byte equality observed |
| config.test | 10 | 0.001153s | 0.001118s | 0.001117s | byte equality observed |
| config_select.test | 10 | 0.001117s | 0.001095s | 0.001112s | byte equality observed |
| connect_pipe.test | 10 | 0.001160s | 0.001140s | 0.001149s | byte equality observed |
| connect_pipe_select.test | 10 | 0.001151s | 0.001102s | 0.001112s | byte equality observed |
| consign.test | 10 | 0.001149s | 0.001085s | 0.001093s | byte equality observed |
| consign_select.test | 10 | 0.001082s | 0.001062s | 0.001051s | byte equality observed |
| deadline_timer.test | 1 | 0.002390s | 0.002389s | 0.002443s | byte equality observed |
| deadline_timer_select.test | 1 | 0.002274s | 0.002149s | 0.002154s | byte equality observed |
| deferred.test | 10 | 0.001027s | 0.001022s | 0.001027s | byte equality observed |
| deferred_select.test | 10 | 0.000983s | 0.000979s | 0.000972s | byte equality observed |
| disposition.test | 10 | 0.001044s | 0.001040s | 0.001024s | byte equality observed |
| disposition_select.test | 10 | 0.001043s | 0.001029s | 0.001023s | byte equality observed |
| error.test | 10 | 0.001018s | 0.001011s | 0.000997s | byte equality observed |
| error_select.test | 10 | 0.001014s | 0.001017s | 0.001009s | byte equality observed |
| inline_executor.test | 10 | 0.001026s | 0.001018s | 0.001020s | byte equality observed |
| inline_executor_select.test | 10 | 0.001032s | 0.001018s | 0.001019s | byte equality observed |
| inline_or_executor.test | 10 | 0.001040s | 0.001030s | 0.001013s | byte equality observed |
| inline_or_executor_select.test | 10 | 0.001034s | 0.001028s | 0.001013s | byte equality observed |
| io_context.test | 1 | 0.001986s | 0.001891s | 0.001964s | byte equality observed |
| io_context_select.test | 1 | 0.002309s | 0.002247s | 0.002260s | byte equality observed |
| io_context_strand.test | 1 | 0.002306s | 0.002310s | 0.002275s | byte equality observed |
| io_context_strand_select.test | 1 | 0.002092s | 0.002083s | 0.002087s | byte equality observed |
| ip_host_name.test | 10 | 0.001042s | 0.001039s | 0.001030s | byte equality observed |
| ip_host_name_select.test | 10 | 0.000996s | 0.000991s | 0.000983s | byte equality observed |
| ip_icmp.test | 10 | 0.001033s | 0.001025s | 0.001005s | byte equality observed |
| ip_icmp_select.test | 10 | 0.001027s | 0.001030s | 0.001006s | byte equality observed |
| ip_multicast.test | 10 | 0.001113s | 0.001110s | 0.001099s | byte equality observed |
| ip_multicast_select.test | 10 | 0.001095s | 0.001094s | 0.001070s | byte equality observed |
| ip_tcp.test | 10 | 0.001521s | 0.001497s | 0.001488s | byte equality observed |
| ip_tcp_select.test | 10 | 0.001446s | 0.001421s | 0.001413s | byte equality observed |
| ip_udp.test | 10 | 0.001137s | 0.001129s | 0.001111s | byte equality observed |
| ip_udp_select.test | 10 | 0.001126s | 0.001111s | 0.001096s | byte equality observed |
| ip_unicast.test | 10 | 0.001085s | 0.001156s | 0.001061s | byte equality observed |
| ip_unicast_select.test | 10 | 0.001066s | 0.001180s | 0.001041s | byte equality observed |
| ip_v6_only.test | 10 | 0.001077s | 0.001136s | 0.001061s | byte equality observed |
| ip_v6_only_select.test | 10 | 0.001072s | 0.001136s | 0.001049s | byte equality observed |
| is_read_buffered.test | 10 | 0.000997s | 0.001042s | 0.000978s | byte equality observed |
| is_read_buffered_select.test | 10 | 0.001002s | 0.001072s | 0.000977s | byte equality observed |
| is_write_buffered.test | 10 | 0.001001s | 0.001084s | 0.000977s | byte equality observed |
| is_write_buffered_select.test | 10 | 0.000995s | 0.001078s | 0.000977s | byte equality observed |
| prepend.test | 10 | 0.001058s | 0.001162s | 0.001073s | byte equality observed |
| prepend_select.test | 10 | 0.001049s | 0.001101s | 0.001084s | byte equality observed |
| read.test | 10 | 0.001509s | 0.001491s | 0.001462s | byte equality observed |
| read_at.test | 10 | 0.001956s | 0.001912s | 0.001697s | byte equality observed |
| read_at_select.test | 10 | 0.001957s | 0.001887s | 0.001742s | byte equality observed |
| read_select.test | 10 | 0.001563s | 0.001484s | 0.001464s | byte equality observed |
| read_until.test | 10 | 0.001238s | 0.001218s | 0.001235s | byte equality observed |
| read_until_select.test | 10 | 0.001245s | 0.001224s | 0.001220s | byte equality observed |
| recycling_allocator.test | 10 | 0.001047s | 0.001043s | 0.001060s | byte equality observed |
| recycling_allocator_select.test | 10 | 0.001050s | 0.001039s | 0.001065s | byte equality observed |
| signal_set.test | 10 | 0.001052s | 0.001115s | 0.001084s | byte equality observed |
| signal_set_select.test | 10 | 0.001063s | 0.001069s | 0.001063s | byte equality observed |
| socket_base.test | 10 | 0.001164s | 0.001165s | 0.001184s | byte equality observed |
| socket_base_select.test | 10 | 0.001147s | 0.001138s | 0.001167s | byte equality observed |
| spawn.test | 10 | 0.001280s | 0.001253s | 0.001274s | byte equality observed |
| spawn_select.test | 10 | 0.001254s | 0.001212s | 0.001255s | byte equality observed |
| static_thread_pool.test | 10 | 0.001039s | 0.001033s | 0.001041s | byte equality observed |
| static_thread_pool_select.test | 10 | 0.001017s | 0.001026s | 0.001010s | byte equality observed |
| strand.test | 1 | 0.001993s | 0.001943s | 0.001994s | byte equality observed |
| strand_select.test | 1 | 0.001736s | 0.001673s | 0.001692s | byte equality observed |
| streambuf.test | 10 | 0.001068s | 0.001105s | 0.001087s | byte equality observed |
| streambuf_select.test | 10 | 0.001063s | 0.001061s | 0.001069s | byte equality observed |
| thread_pool.test | 10 | 0.002273s | 0.002223s | 0.002271s | byte equality observed |
| thread_pool_select.test | 10 | 0.002228s | 0.002242s | 0.002223s | byte equality observed |
| write.test | 10 | 0.001642s | 0.001540s | 0.001541s | byte equality observed |
| write_at.test | 10 | 0.001769s | 0.001695s | 0.001609s | byte equality observed |
| write_at_select.test | 9 | 0.001775s | 0.001668s | 0.001648s | byte equality observed |
| write_select.test | 10 | 0.001629s | 0.001497s | 0.001536s | byte equality observed |

Compiler command and timing observations: [cppgm](https://github.com/vishvananda/cppgm-extended/actions/runs/42735798837) · [gcc](https://github.com/vishvananda/cppgm-extended/actions/runs/42735798837) · [clang](https://github.com/vishvananda/cppgm-extended/actions/runs/42735798837).

[Reviewed outcomes and per-test records](../results.json) · [Host and toolchains](../metadata.json)
