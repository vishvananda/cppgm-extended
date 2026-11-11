# libs/asio/test

All original oracles passed for all three compilers. Seconds are child CPU, one clean campaign per compiler. Test execution includes startup and GDB; small timings need repeats.

| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |
|---|---|---:|---:|---:|---:|---:|---:|
| any_completion_executor.test | RUN | 0.820875 | 5.028185 | 4.312545 | 0.002232 | 0.002529 | 0.002326 |
| any_completion_executor_select.test | RUN | 0.852914 | 5.054689 | 4.678209 | 0.002416 | 0.002503 | 0.002017 |
| any_completion_handler.test | RUN | 0.942307 | 4.533188 | 3.925745 | 0.001864 | 0.001904 | 0.001714 |
| any_completion_handler_select.test | RUN | 0.958628 | 4.587667 | 4.211574 | 0.001848 | 0.002050 | 0.001806 |
| any_io_executor.test | RUN | 0.838814 | 5.374690 | 4.732940 | 0.002252 | 0.002651 | 0.002190 |
| any_io_executor_select.test | RUN | 0.879679 | 5.356077 | 5.076790 | 0.002460 | 0.002660 | 0.002295 |
| append.test | RUN | 0.779645 | 4.245994 | 3.561127 | 0.001773 | 0.002014 | 0.001841 |
| append_select.test | RUN | 0.794965 | 4.321025 | 3.490994 | 0.001689 | 0.001844 | 0.001684 |
| as_tuple.test | RUN | 1.055038 | 5.107027 | 4.848688 | 0.001729 | 0.001912 | 0.001764 |
| as_tuple_select.test | RUN | 1.048629 | 5.107142 | 4.830986 | 0.001831 | 0.001986 | 0.001546 |
| associated_allocator.test | LINK | 0.222268 | 0.463791 | 0.546411 | — | — | — |
| associated_allocator_select.test | LINK | 0.215891 | 0.442136 | 0.548436 | — | — | — |
| associated_cancellation_slot.test | LINK | 0.239731 | 0.494118 | 0.596538 | — | — | — |
| associated_cancellation_slot_select.test | LINK | 0.238057 | 0.504583 | 0.608050 | — | — | — |
| associated_executor.test | LINK | 0.352773 | 0.845070 | 0.962253 | — | — | — |
| associated_executor_select.test | LINK | 0.348414 | 0.853816 | 0.957291 | — | — | — |
| associated_immediate_executor.test | LINK | 0.319088 | 0.786514 | 0.876617 | — | — | — |
| associated_immediate_executor_select.test | LINK | 0.320265 | 0.790410 | 0.878170 | — | — | — |
| associator.test | LINK | 0.177496 | 0.361656 | 0.458869 | — | — | — |
| associator_select.test | LINK | 0.178314 | 0.365277 | 0.440395 | — | — | — |
| awaitable.test | LINK | 0.177654 | 0.372026 | 0.439757 | — | — | — |
| awaitable_select.test | LINK | 0.179824 | 0.358910 | 0.452214 | — | — | — |
| basic_datagram_socket.test | LINK | 0.550870 | 1.146740 | 1.735430 | — | — | — |
| basic_datagram_socket_select.test | LINK | 0.547463 | 1.181060 | 1.755656 | — | — | — |
| basic_deadline_timer.test | LINK | 0.666241 | 1.367015 | 1.966446 | — | — | — |
| basic_deadline_timer_select.test | LINK | 0.675566 | 1.410243 | 2.003889 | — | — | — |
| basic_file.test | LINK | 0.176897 | 0.372412 | 0.449270 | — | — | — |
| basic_file_select.test | LINK | 0.176334 | 0.379339 | 0.444055 | — | — | — |
| basic_random_access_file.test | LINK | 0.178027 | 0.372913 | 0.446821 | — | — | — |
| basic_random_access_file_select.test | LINK | 0.177687 | 0.373448 | 0.443984 | — | — | — |
| basic_raw_socket.test | LINK | 0.541708 | 1.161817 | 1.725623 | — | — | — |
| basic_raw_socket_select.test | LINK | 0.553792 | 1.200681 | 1.772032 | — | — | — |
| basic_readable_pipe.test | LINK | 0.521162 | 1.117472 | 1.464285 | — | — | — |
| basic_readable_pipe_select.test | LINK | 0.544328 | 1.191498 | 1.822944 | — | — | — |
| basic_seq_packet_socket.test | LINK | 0.545554 | 1.164731 | 1.741661 | — | — | — |
| basic_seq_packet_socket_select.test | LINK | 0.554879 | 1.187899 | 1.750025 | — | — | — |
| basic_signal_set.test | LINK | 0.505384 | 1.071370 | 1.306157 | — | — | — |
| basic_signal_set_select.test | LINK | 0.524640 | 1.135744 | 1.669313 | — | — | — |
| basic_socket_acceptor.test | LINK | 0.545766 | 1.162128 | 1.725251 | — | — | — |
| basic_socket_acceptor_select.test | LINK | 0.560549 | 1.197053 | 1.782477 | — | — | — |
| basic_stream_file.test | LINK | 0.176249 | 0.361917 | 0.449951 | — | — | — |
| basic_stream_file_select.test | LINK | 0.175875 | 0.365918 | 0.457787 | — | — | — |
| basic_stream_socket.test | LINK | 0.552014 | 1.157519 | 1.720200 | — | — | — |
| basic_stream_socket_select.test | LINK | 0.549585 | 1.205804 | 1.780398 | — | — | — |
| basic_streambuf.test | LINK | 0.277275 | 0.537205 | 0.663328 | — | — | — |
| basic_streambuf_select.test | LINK | 0.271542 | 0.542994 | 0.676618 | — | — | — |
| basic_waitable_timer.test | LINK | 0.511467 | 1.111061 | 1.620023 | — | — | — |
| basic_waitable_timer_select.test | LINK | 0.523980 | 1.146710 | 1.659555 | — | — | — |
| basic_writable_pipe.test | LINK | 0.522704 | 1.127296 | 1.479259 | — | — | — |
| basic_writable_pipe_select.test | LINK | 0.545211 | 1.205832 | 1.837199 | — | — | — |
| bind_allocator.test | RUN | 0.850822 | 4.299317 | 3.831028 | 0.002172 | 0.002128 | 0.002117 |
| bind_allocator_select.test | RUN | 0.876085 | 4.386764 | 3.806855 | 0.001992 | 0.001694 | 0.001648 |
| bind_cancellation_slot.test | RUN | 0.888346 | 4.329615 | 3.960546 | 0.001398 | 0.001306 | 0.001312 |
| bind_cancellation_slot_select.test | RUN | 0.879640 | 4.368070 | 3.686467 | 0.001332 | 0.001371 | 0.001262 |
| bind_executor.test | RUN | 0.556649 | 2.837005 | 2.347748 | 0.001927 | 0.002212 | 0.001880 |
| bind_executor_select.test | RUN | 0.571384 | 2.926975 | 2.370411 | 0.001889 | 0.002426 | 0.001836 |
| bind_immediate_executor.test | RUN | 0.407220 | 2.390436 | 1.567240 | 0.001336 | 0.001319 | 0.001238 |
| bind_immediate_executor_select.test | RUN | 0.421848 | 2.318792 | 1.747793 | 0.001340 | 0.001289 | 0.001266 |
| buffer.test | RUN | 0.242884 | 0.978216 | 0.932901 | 0.001268 | 0.001250 | 0.001229 |
| buffer_select.test | RUN | 0.239037 | 1.000248 | 0.936142 | 0.001287 | 0.001277 | 0.001217 |
| buffer_registration.test | LINK | 0.201968 | 0.486926 | 0.540311 | — | — | — |
| buffer_registration_select.test | LINK | 0.202993 | 0.511732 | 0.539587 | — | — | — |
| buffered_read_stream.test | RUN | 0.901893 | 6.849339 | 5.712781 | 0.001702 | 0.002192 | 0.002150 |
| buffered_read_stream_select.test | RUN | 0.917010 | 6.868644 | 6.095697 | 0.001618 | 0.002127 | 0.002182 |
| buffered_stream.test | RUN | 1.555058 | 10.060047 | 9.604030 | 0.002164 | 0.002244 | 0.002214 |
| buffered_stream_select.test | RUN | 1.652377 | 10.138322 | 9.354553 | 0.002258 | 0.002257 | 0.002254 |
| buffered_write_stream.test | RUN | 1.612440 | 8.778050 | 8.387634 | 0.002358 | 0.002090 | 0.002289 |
| buffered_write_stream_select.test | RUN | 1.635149 | 8.880692 | 7.990020 | 0.002240 | 0.002172 | 0.002084 |
| buffers_iterator.test | RUN | 0.401845 | 0.834404 | 1.396161 | 0.001552 | 0.001761 | 0.001565 |
| buffers_iterator_select.test | RUN | 0.398792 | 0.841270 | 1.360117 | 0.001670 | 0.001772 | 0.001674 |
| cancel_after.test | RUN | 1.512660 | 6.893791 | 6.082723 | 0.004193 | 0.004015 | 0.003625 |
| cancel_after_select.test | RUN | 1.504929 | 6.813582 | 6.015967 | 0.003475 | 0.003604 | 0.003910 |
| cancel_at.test | RUN | 1.509626 | 5.707302 | 5.475454 | 0.003670 | 0.003457 | 0.003259 |
| cancel_at_select.test | RUN | 1.487342 | 5.607335 | 5.360345 | 0.003434 | 0.003210 | 0.003207 |
| cancellation_signal.test | LINK | 0.198488 | 0.283523 | 0.353331 | — | — | — |
| cancellation_signal_select.test | LINK | 0.233340 | 0.291161 | 0.355376 | — | — | — |
| cancellation_state.test | LINK | 0.155235 | 0.288333 | 0.353256 | — | — | — |
| cancellation_state_select.test | LINK | 0.234736 | 0.288365 | 0.355932 | — | — | — |
| cancellation_type.test | LINK | 0.176498 | 0.226098 | 0.315132 | — | — | — |
| cancellation_type_select.test | LINK | 0.181602 | 0.229112 | 0.460489 | — | — | — |
| co_composed.test | RUN | 0.168113 | 0.225922 | 0.442453 | 0.001716 | 0.001262 | 0.001466 |
| co_composed_select.test | RUN | 0.177281 | 0.226897 | 0.459034 | 0.001757 | 0.001226 | 0.001710 |
| co_spawn.test | LINK | 0.176344 | 0.229567 | 0.461312 | — | — | — |
| co_spawn_select.test | LINK | 0.176050 | 0.230835 | 0.459916 | — | — | — |
| completion_condition.test | LINK | 0.231556 | 0.352282 | 0.609114 | — | — | — |
| completion_condition_select.test | LINK | 0.229853 | 0.356554 | 0.616696 | — | — | — |
| compose.test | RUN | 1.057179 | 5.337592 | 4.962082 | 0.002308 | 0.002294 | 0.002170 |
| compose_select.test | RUN | 1.068974 | 5.356706 | 4.871921 | 0.002337 | 0.002150 | 0.002257 |
| composed.test | RUN | 1.049946 | 5.372675 | 4.827135 | 0.001856 | 0.001978 | 0.001864 |
| composed_select.test | RUN | 0.989161 | 5.338901 | 4.789128 | 0.001804 | 0.001670 | 0.001764 |
| config.test | RUN | 0.505934 | 3.488831 | 2.088599 | 0.001707 | 0.001880 | 0.001732 |
| config_select.test | RUN | 0.540630 | 3.717280 | 2.672764 | 0.001742 | 0.001820 | 0.001845 |
| connect.test | LINK | 1.800559 | 11.486944 | 10.400176 | — | — | — |
| connect_select.test | LINK | 1.787489 | 11.515621 | 10.612365 | — | — | — |
| connect_pipe.test | RUN | 0.896629 | 4.344919 | 3.796489 | 0.001930 | 0.002068 | 0.001825 |
| connect_pipe_select.test | RUN | 0.923922 | 4.532074 | 4.176631 | 0.001753 | 0.001850 | 0.001846 |
| consign.test | RUN | 0.782037 | 4.213619 | 3.472133 | 0.001886 | 0.001834 | 0.001812 |
| consign_select.test | RUN | 0.792205 | 4.297057 | 3.467865 | 0.001822 | 0.001916 | 0.001733 |
| coroutine.test | LINK | 0.182211 | 0.439927 | 0.505305 | — | — | — |
| coroutine_select.test | LINK | 0.182761 | 0.437221 | 0.504210 | — | — | — |
| deadline_timer.test | RUN | 1.163407 | 5.472445 | 4.905590 | 0.003497 | 0.003261 | 0.003428 |
| deadline_timer_select.test | RUN | 1.174415 | 5.443865 | 5.009651 | 0.003304 | 0.002990 | 0.002929 |
| deferred.test | RUN | 0.196575 | 0.406572 | 0.490630 | 0.001714 | 0.001747 | 0.001635 |
| deferred_select.test | RUN | 0.196124 | 0.407337 | 0.498553 | 0.001645 | 0.001680 | 0.001654 |
| detached.test | LINK | 0.223468 | 0.450595 | 0.556789 | — | — | — |
| detached_select.test | LINK | 0.228417 | 0.452158 | 0.540335 | — | — | — |
| disposition.test | RUN | 0.265759 | 1.045713 | 0.997036 | 0.001315 | 0.001876 | 0.001766 |
| disposition_select.test | RUN | 0.192988 | 1.041902 | 0.984246 | 0.001289 | 0.001891 | 0.001776 |
| error.test | RUN | 0.172806 | 1.024355 | 0.921218 | 0.001306 | 0.001799 | 0.001638 |
| error_select.test | RUN | 0.175130 | 1.013406 | 0.893048 | 0.001270 | 0.001790 | 0.001610 |
| file_base.test | LINK | 0.219637 | 0.843967 | 0.975538 | — | — | — |
| file_base_select.test | LINK | 0.219073 | 0.828155 | 0.825917 | — | — | — |
| fwd_first.test | LINK | 0.511698 | 1.272193 | 1.690099 | — | — | — |
| fwd_last.test | LINK | 0.501835 | 1.110725 | 1.696801 | — | — | — |
| generic_basic_endpoint.test | LINK | 0.188164 | 0.451722 | 0.621167 | — | — | — |
| generic_basic_endpoint_select.test | LINK | 0.187255 | 0.456841 | 0.621461 | — | — | — |
| generic_datagram_protocol.test | LINK | 0.929316 | 4.006692 | 3.308880 | — | — | — |
| generic_datagram_protocol_select.test | LINK | 0.925775 | 4.174905 | 3.298779 | — | — | — |
| generic_raw_protocol.test | LINK | 0.793807 | 4.056248 | 3.330460 | — | — | — |
| generic_raw_protocol_select.test | LINK | 0.788606 | 4.224732 | 3.307848 | — | — | — |
| generic_seq_packet_protocol.test | LINK | 0.675280 | 3.455360 | 2.845367 | — | — | — |
| generic_seq_packet_protocol_select.test | LINK | 0.672706 | 3.521920 | 2.849567 | — | — | — |
| generic_stream_protocol.test | LINK | 0.731803 | 3.655290 | 3.068880 | — | — | — |
| generic_stream_protocol_select.test | LINK | 0.724935 | 3.770678 | 3.077231 | — | — | — |
| high_resolution_timer.test | LINK | 0.318396 | 0.674868 | 0.963274 | — | — | — |
| high_resolution_timer_select.test | LINK | 0.323488 | 0.710544 | 0.993679 | — | — | — |
| immediate.test | LINK | 0.217965 | 0.525168 | 0.589854 | — | — | — |
| immediate_select.test | LINK | 0.219108 | 0.521332 | 0.586087 | — | — | — |
| inline_executor.test | RUN | 0.262377 | 0.812315 | 0.879347 | 0.001223 | 0.001275 | 0.001573 |
| inline_executor_select.test | RUN | 0.265047 | 0.805051 | 0.876720 | 0.001188 | 0.001333 | 0.001119 |
| inline_or_executor.test | RUN | 0.435850 | 2.415121 | 1.707334 | 0.001329 | 0.001348 | 0.001276 |
| inline_or_executor_select.test | RUN | 0.452502 | 2.451812 | 1.877390 | 0.001236 | 0.001320 | 0.001232 |
| io_context.test | RUN | 0.641532 | 3.660917 | 2.997287 | 0.002412 | 0.002258 | 0.002232 |
| io_context_select.test | RUN | 0.643183 | 3.574358 | 2.971315 | 0.002070 | 0.002114 | 0.002188 |
| io_context_strand.test | RUN | 0.603760 | 4.651727 | 2.647579 | 0.002122 | 0.002352 | 0.002551 |
| io_context_strand_select.test | RUN | 0.613605 | 4.526161 | 2.619141 | 0.002033 | 0.002505 | 0.002435 |
| ip_address.test | LINK | 0.214967 | 1.135196 | 0.875595 | — | — | — |
| ip_address_select.test | LINK | 0.215230 | 1.130712 | 0.764407 | — | — | — |
| ip_address_v4.test | LINK | 0.221971 | 1.222988 | 0.839987 | — | — | — |
| ip_address_v4_select.test | LINK | 0.221670 | 1.220323 | 0.841067 | — | — | — |
| ip_address_v6.test | LINK | 0.245590 | 1.617107 | 1.060793 | — | — | — |
| ip_address_v6_select.test | LINK | 0.245574 | 1.584559 | 1.050756 | — | — | — |
| ip_basic_endpoint.test | LINK | 0.188153 | 0.482666 | 0.679067 | — | — | — |
| ip_basic_endpoint_select.test | LINK | 0.190186 | 0.476938 | 0.664693 | — | — | — |
| ip_basic_resolver.test | LINK | 0.315444 | 0.690033 | 0.989521 | — | — | — |
| ip_basic_resolver_select.test | LINK | 0.318263 | 0.703424 | 1.010612 | — | — | — |
| ip_basic_resolver_entry.test | LINK | 0.106842 | 0.229075 | 0.276833 | — | — | — |
| ip_basic_resolver_entry_select.test | LINK | 0.106231 | 0.229113 | 0.279634 | — | — | — |
| ip_basic_resolver_iterator.test | LINK | 0.191386 | 0.468367 | 0.653485 | — | — | — |
| ip_basic_resolver_iterator_select.test | LINK | 0.191464 | 0.465830 | 0.645931 | — | — | — |
| ip_basic_resolver_query.test | LINK | 0.186191 | 0.446873 | 0.633605 | — | — | — |
| ip_basic_resolver_query_select.test | LINK | 0.194472 | 0.450991 | 0.627170 | — | — | — |
| ip_host_name.test | RUN | 0.199313 | 0.602528 | 0.707852 | 0.001246 | 0.001252 | 0.001189 |
| ip_host_name_select.test | RUN | 0.198341 | 0.603248 | 0.699974 | 0.001149 | 0.001218 | 0.001524 |
| ip_icmp.test | RUN | 1.017466 | 6.653676 | 7.121327 | 0.001321 | 0.001874 | 0.001727 |
| ip_icmp_select.test | RUN | 1.019883 | 6.901163 | 7.138156 | 0.001195 | 0.001771 | 0.001711 |
| ip_multicast.test | RUN | 0.588581 | 4.491893 | 4.143388 | 0.001432 | 0.001966 | 0.001955 |
| ip_multicast_select.test | RUN | 0.597512 | 4.498568 | 4.131063 | 0.001305 | 0.001870 | 0.001606 |
| ip_resolver_query_base.test | LINK | 0.134466 | 0.412289 | 0.477739 | — | — | — |
| ip_resolver_query_base_select.test | LINK | 0.134187 | 0.406354 | 0.492697 | — | — | — |
| ip_tcp.test | RUN | 1.605538 | 13.268753 | 11.122433 | 0.001756 | 0.002472 | 0.002448 |
| ip_tcp_select.test | RUN | 1.590802 | 13.570697 | 10.781957 | 0.001738 | 0.002314 | 0.002464 |
| ip_udp.test | RUN | 1.089554 | 9.305717 | 7.683215 | 0.001325 | 0.002002 | 0.001962 |
| ip_udp_select.test | RUN | 1.091344 | 9.624711 | 7.465829 | 0.001427 | 0.001909 | 0.001884 |
| ip_unicast.test | RUN | 0.539843 | 4.032004 | 3.611635 | 0.001654 | 0.001962 | 0.001798 |
| ip_unicast_select.test | RUN | 0.541171 | 4.154228 | 3.544734 | 0.001207 | 0.001840 | 0.001656 |
| ip_v6_only.test | RUN | 0.836331 | 4.252778 | 3.750099 | 0.001620 | 0.001994 | 0.001841 |
| ip_v6_only_select.test | RUN | 0.859014 | 4.318204 | 3.731755 | 0.001793 | 0.001966 | 0.001842 |
| is_read_buffered.test | RUN | 0.604249 | 1.324769 | 2.019828 | 0.001645 | 0.001679 | 0.001729 |
| is_read_buffered_select.test | RUN | 0.618993 | 1.364919 | 2.058336 | 0.001590 | 0.001894 | 0.001455 |
| is_write_buffered.test | RUN | 0.598391 | 1.327057 | 2.012431 | 0.001676 | 0.001838 | 0.001544 |
| is_write_buffered_select.test | RUN | 0.609841 | 1.337710 | 2.060345 | 0.001738 | 0.001783 | 0.001721 |
| local_basic_endpoint.test | LINK | 0.306984 | 0.735043 | 1.095384 | — | — | — |
| local_basic_endpoint_select.test | LINK | 0.306168 | 0.713362 | 1.087469 | — | — | — |
| local_connect_pair.test | LINK | 0.779349 | 4.144658 | 3.454470 | — | — | — |
| local_connect_pair_select.test | LINK | 0.778977 | 4.118460 | 3.397450 | — | — | — |
| local_datagram_protocol.test | LINK | 1.036639 | 5.415404 | 4.835145 | — | — | — |
| local_datagram_protocol_select.test | LINK | 1.048956 | 5.475457 | 4.744046 | — | — | — |
| local_seq_packet_protocol.test | LINK | 1.037423 | 5.246193 | 4.766978 | — | — | — |
| local_seq_packet_protocol_select.test | LINK | 1.034305 | 5.375180 | 4.757304 | — | — | — |
| local_stream_protocol.test | LINK | 0.984295 | 5.100052 | 4.576835 | — | — | — |
| local_stream_protocol_select.test | LINK | 0.990479 | 5.087588 | 4.540558 | — | — | — |
| placeholders.test | LINK | 0.198354 | 0.403891 | 0.496462 | — | — | — |
| placeholders_select.test | LINK | 0.196930 | 0.406285 | 0.494950 | — | — | — |
| posix_basic_descriptor.test | LINK | 0.526756 | 1.094836 | 1.494614 | — | — | — |
| posix_basic_descriptor_select.test | LINK | 0.545510 | 1.159198 | 1.852122 | — | — | — |
| posix_basic_stream_descriptor.test | LINK | 0.528158 | 1.114665 | 1.496600 | — | — | — |
| posix_basic_stream_descriptor_select.test | LINK | 0.548357 | 1.143109 | 1.850481 | — | — | — |
| posix_descriptor_base.test | LINK | 0.215411 | 0.424005 | 0.508891 | — | — | — |
| posix_descriptor_base_select.test | LINK | 0.210932 | 0.414849 | 0.490298 | — | — | — |
| posix_stream_descriptor.test | LINK | 1.010539 | 5.142584 | 4.452278 | — | — | — |
| posix_stream_descriptor_select.test | LINK | 1.028663 | 5.231041 | 4.594062 | — | — | — |
| prepend.test | RUN | 0.789044 | 4.146083 | 3.573168 | 0.001818 | 0.001714 | 0.001616 |
| prepend_select.test | RUN | 0.797705 | 4.234977 | 3.550722 | 0.001553 | 0.001798 | 0.001773 |
| random_access_file.test | LINK | 0.486151 | 1.001080 | 1.169846 | — | — | — |
| random_access_file_select.test | LINK | 0.497128 | 1.075324 | 1.545038 | — | — | — |
| read.test | RUN | 2.528857 | 27.406121 | 20.709906 | 0.002645 | 0.002464 | 0.002404 |
| read_select.test | RUN | 2.563127 | 27.300117 | 21.050867 | 0.002523 | 0.002302 | 0.002505 |
| read_at.test | RUN | 2.561309 | 36.090648 | 28.483051 | 0.003208 | 0.003105 | 0.002736 |
| read_at_select.test | RUN | 2.591590 | 35.891425 | 28.840307 | 0.003201 | 0.003159 | 0.002895 |
| read_until.test | RUN | 1.372165 | 10.305689 | 7.661149 | 0.001922 | 0.001910 | 0.001903 |
| read_until_select.test | RUN | 1.388533 | 10.392572 | 7.938973 | 0.001962 | 0.001963 | 0.002022 |
| readable_pipe.test | LINK | 0.817880 | 4.130190 | 3.510175 | — | — | — |
| readable_pipe_select.test | LINK | 0.846462 | 4.352605 | 3.763148 | — | — | — |
| recycling_allocator.test | RUN | 0.258720 | 0.612279 | 0.717240 | 0.001637 | 0.001454 | 0.001683 |
| recycling_allocator_select.test | RUN | 0.258275 | 0.618951 | 0.726306 | 0.001424 | 0.001660 | 0.001718 |
| redirect_disposition.test | LINK | 1.142443 | 5.882056 | 5.427026 | — | — | — |
| redirect_disposition_select.test | LINK | 1.145158 | 5.873447 | 5.406304 | — | — | — |
| redirect_error.test | LINK | 0.993247 | 4.860644 | 4.738498 | — | — | — |
| redirect_error_select.test | LINK | 0.998919 | 4.883320 | 4.720582 | — | — | — |
| registered_buffer.test | LINK | 0.257475 | 0.516479 | 0.666251 | — | — | — |
| registered_buffer_select.test | LINK | 0.255472 | 0.525110 | 0.652307 | — | — | — |
| signal_set.test | RUN | 0.792080 | 4.062246 | 3.254458 | 0.001756 | 0.001914 | 0.001693 |
| signal_set_select.test | RUN | 0.801116 | 4.249346 | 3.583235 | 0.001660 | 0.001842 | 0.001747 |
| signal_set_base.test | LINK | 0.205679 | 0.408559 | 0.482282 | — | — | — |
| signal_set_base_select.test | LINK | 0.201038 | 0.393134 | 0.478674 | — | — | — |
| socket_base.test | RUN | 1.008736 | 6.039107 | 5.086701 | 0.001881 | 0.001864 | 0.001912 |
| socket_base_select.test | RUN | 0.998975 | 5.881211 | 5.064246 | 0.001930 | 0.001911 | 0.001825 |
| spawn.test | RUN | 1.624167 | 9.787990 | 8.549583 | 0.002077 | 0.002140 | 0.002086 |
| spawn_select.test | RUN | 1.648526 | 9.813561 | 8.535611 | 0.001966 | 0.002172 | 0.002030 |
| static_thread_pool.test | RUN | 0.465997 | 0.946491 | 1.150686 | 0.001684 | 0.001733 | 0.001680 |
| static_thread_pool_select.test | RUN | 0.486680 | 1.026937 | 1.535116 | 0.001522 | 0.001730 | 0.001640 |
| steady_timer.test | LINK | 0.511817 | 1.073756 | 1.692748 | — | — | — |
| steady_timer_select.test | LINK | 0.521738 | 1.110165 | 1.715777 | — | — | — |
| strand.test | RUN | 1.063176 | 5.551792 | 5.236229 | 0.002667 | 0.002306 | 0.002676 |
| strand_select.test | RUN | 1.091815 | 5.524460 | 5.134832 | 0.002090 | 0.002410 | 0.002695 |
| stream_file.test | LINK | 0.483246 | 0.971426 | 1.226077 | — | — | — |
| stream_file_select.test | LINK | 0.507321 | 0.991230 | 1.576118 | — | — | — |
| streambuf.test | RUN | 0.283926 | 0.576088 | 0.844474 | 0.001287 | 0.001249 | 0.001714 |
| streambuf_select.test | RUN | 0.266115 | 0.512478 | 0.860375 | 0.001167 | 0.001242 | 0.001768 |
| system_timer.test | LINK | 0.656955 | 3.355859 | 3.258586 | — | — | — |
| system_timer_select.test | LINK | 0.654326 | 3.295958 | 3.058603 | — | — | — |
| system_context.test | LINK | 0.284506 | 0.579536 | 0.708607 | — | — | — |
| system_context_select.test | LINK | 0.297799 | 0.625025 | 0.914734 | — | — | — |
| system_executor.test | LINK | 0.375548 | 1.911316 | 1.381514 | — | — | — |
| system_executor_select.test | LINK | 0.388349 | 1.873398 | 1.546198 | — | — | — |
| this_coro.test | LINK | 0.109833 | 0.228897 | 0.280082 | — | — | — |
| this_coro_select.test | LINK | 0.107691 | 0.228008 | 0.280747 | — | — | — |
| thread_pool.test | RUN | 0.447172 | 3.514652 | 1.780750 | 0.002410 | 0.003179 | 0.002409 |
| thread_pool_select.test | RUN | 0.459883 | 3.523621 | 1.973712 | 0.002551 | 0.002897 | 0.002364 |
| time_traits.test | LINK | 0.249321 | 0.759279 | 0.939057 | — | — | — |
| time_traits_select.test | LINK | 0.250389 | 0.771351 | 0.965627 | — | — | — |
| ts_buffer.test | LINK | 0.239105 | 0.855658 | 0.699263 | — | — | — |
| ts_buffer_select.test | LINK | 0.235522 | 0.859193 | 0.996969 | — | — | — |
| ts_executor.test | LINK | 0.333754 | 1.168459 | 1.553450 | — | — | — |
| ts_executor_select.test | LINK | 0.350682 | 1.234039 | 1.924002 | — | — | — |
| ts_internet.test | LINK | 0.369597 | 1.289442 | 1.980242 | — | — | — |
| ts_internet_select.test | LINK | 0.376170 | 1.310841 | 2.015749 | — | — | — |
| ts_io_context.test | LINK | 0.296877 | 0.982648 | 1.182605 | — | — | — |
| ts_io_context_select.test | LINK | 0.309967 | 1.057350 | 1.525017 | — | — | — |
| ts_net.test | LINK | 0.445233 | 1.518038 | 2.407894 | — | — | — |
| ts_net_select.test | LINK | 0.451526 | 1.568712 | 2.447645 | — | — | — |
| ts_netfwd.test | LINK | 0.443318 | 1.540219 | 2.412627 | — | — | — |
| ts_netfwd_select.test | LINK | 0.451638 | 1.585067 | 2.464162 | — | — | — |
| ts_socket.test | LINK | 0.362977 | 1.194586 | 1.882418 | — | — | — |
| ts_socket_select.test | LINK | 0.363575 | 1.237444 | 1.916461 | — | — | — |
| ts_timer.test | LINK | 0.319838 | 1.072600 | 1.645623 | — | — | — |
| ts_timer_select.test | LINK | 0.325087 | 1.108385 | 1.639471 | — | — | — |
| use_awaitable.test | LINK | 0.110952 | 0.362072 | 0.456263 | — | — | — |
| use_awaitable_select.test | LINK | 0.111072 | 0.359514 | 0.454194 | — | — | — |
| wait_traits.test | LINK | 0.110035 | 0.365163 | 0.454211 | — | — | — |
| wait_traits_select.test | LINK | 0.110847 | 0.365031 | 0.453000 | — | — | — |
| windows_basic_object_handle.test | LINK | 0.109287 | 0.362638 | 0.457815 | — | — | — |
| windows_basic_object_handle_select.test | LINK | 0.172019 | 0.365832 | 0.456374 | — | — | — |
| windows_basic_overlapped_handle.test | LINK | 0.182178 | 0.358082 | 0.450847 | — | — | — |
| windows_basic_overlapped_handle_select.test | LINK | 0.182179 | 0.364694 | 0.450710 | — | — | — |
| windows_basic_random_access_handle.test | LINK | 0.177275 | 0.365618 | 0.452856 | — | — | — |
| windows_basic_random_access_handle_select.test | LINK | 0.175228 | 0.362533 | 0.444751 | — | — | — |
| windows_basic_stream_handle.test | LINK | 0.178563 | 0.361619 | 0.451974 | — | — | — |
| windows_basic_stream_handle_select.test | LINK | 0.178420 | 0.360740 | 0.449940 | — | — | — |
| windows_object_handle.test | LINK | 0.487128 | 0.978566 | 1.183586 | — | — | — |
| windows_object_handle_select.test | LINK | 0.515096 | 1.041554 | 1.564228 | — | — | — |
| windows_overlapped_ptr.test | LINK | 0.495804 | 1.035692 | 1.326681 | — | — | — |
| windows_overlapped_ptr_select.test | LINK | 0.521019 | 1.119344 | 1.664834 | — | — | — |
| windows_random_access_handle.test | LINK | 0.478548 | 0.974502 | 1.199041 | — | — | — |
| windows_random_access_handle_select.test | LINK | 0.501073 | 1.054482 | 1.569189 | — | — | — |
| windows_stream_handle.test | LINK | 0.477466 | 0.985737 | 1.215264 | — | — | — |
| windows_stream_handle_select.test | LINK | 0.499015 | 1.063547 | 1.592221 | — | — | — |
| writable_pipe.test | LINK | 0.828595 | 4.118346 | 3.710353 | — | — | — |
| writable_pipe_select.test | LINK | 0.861199 | 4.345254 | 3.927198 | — | — | — |
| write.test | RUN | 2.532083 | 21.902870 | 20.857243 | 0.002664 | 0.002473 | 0.002417 |
| write_select.test | RUN | 2.572677 | 22.340863 | 21.010111 | 0.002661 | 0.002599 | 0.002541 |
| write_at.test | RUN | 2.690227 | 30.123314 | 27.043380 | 0.003051 | 0.002758 | 0.002808 |
| write_at_select.test | RUN | 2.693448 | 30.501943 | 27.382346 | 0.002949 | 0.002807 | 0.002732 |
| any_executor.test | RUN | 1.018081 | 9.993042 | 8.528577 | 0.002332 | 0.002405 | 0.002480 |
| blocking.test | RUN | 2.430424 | 12.461207 | 17.127787 | 0.005390 | 0.005100 | 0.005277 |
| blocking_adaptation.test | RUN | 1.225323 | 5.291103 | 7.757002 | 0.003291 | 0.003253 | 0.003193 |
| context_as.test | RUN | 0.672842 | 3.719134 | 2.557430 | 0.002053 | 0.002003 | 0.001923 |
| executor.test | RUN | 0.187231 | 0.361148 | 0.475566 | 0.001724 | 0.001622 | 0.001636 |
| inline_exception_handling.test | RUN | 2.317749 | 11.302868 | 15.827699 | 0.003386 | 0.003363 | 0.003403 |
| invocable_archetype.test | RUN | 0.184451 | 0.370098 | 0.481098 | 0.001759 | 0.001772 | 0.001655 |
| mapping.test | RUN | 2.238991 | 11.589718 | 17.018193 | 0.003526 | 0.003400 | 0.003393 |
| outstanding_work.test | RUN | 1.123923 | 5.144057 | 7.498832 | 0.003307 | 0.003256 | 0.003159 |
| prefer_only.test | RUN | 0.389407 | 2.156029 | 1.706430 | 0.001742 | 0.001817 | 0.001718 |
| relationship.test | RUN | 0.953413 | 4.266673 | 7.415747 | 0.002140 | 0.002098 | 0.002102 |

## Native runtime replay

Child CPU of successful native tests, with process startup included; GDB and expected-failure executions are correctness-only. Two warmups per compiler, four A/A observations per reference and mirrored ABBA/BAAB blocks. Short tests repeat up to ten times, identically across compilers; CPU is divided by repetitions before summing tests. Executable hashes and output checks stay outside process timers. All runs must pass their original native oracle. Output hashes record equality or variation across the complete campaign; test reports may print times or addresses.

| Reference | CPU speedup ↑ | Paired blocks | A/A speedup |
|---|---:|---|---:|
| gcc | 0.983× | 0.981×, 0.986× | 0.999× |
| clang | 0.975× | 0.981×, 0.969× | 0.994× |

† A/A drift or paired-block spread exceeds 5%; small differences are unresolved. 1375 native tests in the full cohort; 1359 have byte-identical output throughout. [Manifest](https://github.com/vishvananda/cppgm-extended/actions/runs/42682904325), [paired observations](https://github.com/vishvananda/cppgm-extended/actions/runs/42682904325) and [per-test summaries](https://github.com/vishvananda/cppgm-extended/actions/runs/42682904325).

| Test | Repeats | CPU cppgm | GCC | Clang | Output check |
|---|---:|---:|---:|---:|---|
| any_completion_executor.test | 2 | 0.001779s | 0.001681s | 0.001755s | byte equality observed |
| any_completion_executor_select.test | 3 | 0.001724s | 0.001692s | 0.001672s | byte equality observed |
| any_completion_handler.test | 6 | 0.001405s | 0.001392s | 0.001384s | byte equality observed |
| any_completion_handler_select.test | 8 | 0.001413s | 0.001317s | 0.001370s | byte equality observed |
| any_io_executor.test | 6 | 0.001718s | 0.001627s | 0.001682s | byte equality observed |
| any_io_executor_select.test | 6 | 0.001714s | 0.001630s | 0.001654s | byte equality observed |
| append.test | 6 | 0.001354s | 0.001275s | 0.001313s | byte equality observed |
| append_select.test | 8 | 0.001321s | 0.001287s | 0.001287s | byte equality observed |
| as_tuple.test | 9 | 0.001411s | 0.001403s | 0.001372s | byte equality observed |
| as_tuple_select.test | 7 | 0.001341s | 0.001250s | 0.001308s | byte equality observed |
| bind_allocator.test | 1 | 0.001730s | 0.001664s | 0.001661s | byte equality observed |
| bind_allocator_select.test | 1 | 0.001548s | 0.001380s | 0.001475s | byte equality observed |
| bind_cancellation_slot.test | 10 | 0.001092s | 0.001070s | 0.001074s | byte equality observed |
| bind_cancellation_slot_select.test | 10 | 0.001030s | 0.000994s | 0.001012s | byte equality observed |
| bind_executor.test | 1 | 0.001636s | 0.001556s | 0.001592s | byte equality observed |
| bind_executor_select.test | 1 | 0.001765s | 0.001737s | 0.001684s | byte equality observed |
| bind_immediate_executor.test | 10 | 0.001155s | 0.001136s | 0.001145s | byte equality observed |
| bind_immediate_executor_select.test | 10 | 0.001114s | 0.001116s | 0.001134s | byte equality observed |
| buffer.test | 10 | 0.001061s | 0.001010s | 0.001055s | byte equality observed |
| buffer_select.test | 10 | 0.001054s | 0.001044s | 0.001038s | byte equality observed |
| buffered_read_stream.test | 10 | 0.001271s | 0.001309s | 0.001266s | byte equality observed |
| buffered_read_stream_select.test | 10 | 0.001333s | 0.001237s | 0.001319s | byte equality observed |
| buffered_stream.test | 10 | 0.001318s | 0.001296s | 0.001271s | byte equality observed |
| buffered_stream_select.test | 10 | 0.001268s | 0.001276s | 0.001292s | byte equality observed |
| buffered_write_stream.test | 10 | 0.001276s | 0.001238s | 0.001299s | byte equality observed |
| buffered_write_stream_select.test | 10 | 0.001309s | 0.001263s | 0.001228s | byte equality observed |
| buffers_iterator.test | 10 | 0.001008s | 0.001016s | 0.001006s | byte equality observed |
| buffers_iterator_select.test | 10 | 0.000968s | 0.000994s | 0.000989s | byte equality observed |
| cancel_after.test | 1 | 0.003313s | 0.003097s | 0.003246s | byte equality observed |
| cancel_after_select.test | 1 | 0.002981s | 0.002732s | 0.002757s | byte equality observed |
| cancel_at.test | 1 | 0.003375s | 0.003521s | 0.003292s | byte equality observed |
| cancel_at_select.test | 1 | 0.003458s | 0.003402s | 0.003195s | byte equality observed |
| co_composed.test | 10 | 0.001092s | 0.001094s | 0.001084s | byte equality observed |
| co_composed_select.test | 10 | 0.000996s | 0.000994s | 0.000971s | byte equality observed |
| compose.test | 1 | 0.001687s | 0.001566s | 0.001614s | byte equality observed |
| compose_select.test | 1 | 0.001462s | 0.001407s | 0.001370s | byte equality observed |
| composed.test | 1 | 0.002019s | 0.001940s | 0.001850s | byte equality observed |
| composed_select.test | 1 | 0.001684s | 0.001712s | 0.001599s | byte equality observed |
| config.test | 10 | 0.001152s | 0.001149s | 0.001120s | byte equality observed |
| config_select.test | 10 | 0.001155s | 0.001174s | 0.001165s | byte equality observed |
| connect_pipe.test | 10 | 0.001117s | 0.001136s | 0.001102s | byte equality observed |
| connect_pipe_select.test | 10 | 0.001103s | 0.001165s | 0.001095s | byte equality observed |
| consign.test | 10 | 0.001106s | 0.001113s | 0.001110s | byte equality observed |
| consign_select.test | 10 | 0.001098s | 0.001071s | 0.001028s | byte equality observed |
| deadline_timer.test | 1 | 0.002345s | 0.002334s | 0.002239s | byte equality observed |
| deadline_timer_select.test | 1 | 0.002184s | 0.002167s | 0.002083s | byte equality observed |
| deferred.test | 10 | 0.000972s | 0.001000s | 0.000957s | byte equality observed |
| deferred_select.test | 10 | 0.000923s | 0.000910s | 0.000905s | byte equality observed |
| disposition.test | 10 | 0.000976s | 0.000975s | 0.000970s | byte equality observed |
| disposition_select.test | 10 | 0.000974s | 0.000974s | 0.000954s | byte equality observed |
| error.test | 10 | 0.000965s | 0.000961s | 0.000937s | byte equality observed |
| error_select.test | 10 | 0.000955s | 0.000954s | 0.000948s | byte equality observed |
| inline_executor.test | 10 | 0.000961s | 0.000954s | 0.000936s | byte equality observed |
| inline_executor_select.test | 10 | 0.000966s | 0.000949s | 0.000948s | byte equality observed |
| inline_or_executor.test | 10 | 0.000972s | 0.000971s | 0.000935s | byte equality observed |
| inline_or_executor_select.test | 10 | 0.000972s | 0.000965s | 0.000937s | byte equality observed |
| io_context.test | 1 | 0.001904s | 0.001879s | 0.001881s | byte equality observed |
| io_context_select.test | 1 | 0.002264s | 0.002203s | 0.002120s | byte equality observed |
| io_context_strand.test | 1 | 0.002191s | 0.002099s | 0.002276s | byte equality observed |
| io_context_strand_select.test | 1 | 0.001876s | 0.001925s | 0.001997s | byte equality observed |
| ip_host_name.test | 10 | 0.000980s | 0.001033s | 0.000972s | byte equality observed |
| ip_host_name_select.test | 10 | 0.000928s | 0.000946s | 0.000908s | byte equality observed |
| ip_icmp.test | 10 | 0.000955s | 0.000967s | 0.000918s | byte equality observed |
| ip_icmp_select.test | 10 | 0.000951s | 0.000974s | 0.000932s | byte equality observed |
| ip_multicast.test | 10 | 0.001022s | 0.001038s | 0.001020s | byte equality observed |
| ip_multicast_select.test | 10 | 0.001013s | 0.001014s | 0.000988s | byte equality observed |
| ip_tcp.test | 10 | 0.001329s | 0.001314s | 0.001301s | byte equality observed |
| ip_tcp_select.test | 10 | 0.001287s | 0.001272s | 0.001261s | byte equality observed |
| ip_udp.test | 10 | 0.001054s | 0.001039s | 0.001020s | byte equality observed |
| ip_udp_select.test | 6 | 0.001049s | 0.001029s | 0.001020s | byte equality observed |
| ip_unicast.test | 10 | 0.001019s | 0.001011s | 0.001000s | byte equality observed |
| ip_unicast_select.test | 10 | 0.000993s | 0.000982s | 0.000967s | byte equality observed |
| ip_v6_only.test | 10 | 0.001010s | 0.001002s | 0.000995s | byte equality observed |
| ip_v6_only_select.test | 10 | 0.000993s | 0.000982s | 0.000989s | byte equality observed |
| is_read_buffered.test | 10 | 0.000930s | 0.000924s | 0.000914s | byte equality observed |
| is_read_buffered_select.test | 10 | 0.000935s | 0.000923s | 0.000917s | byte equality observed |
| is_write_buffered.test | 10 | 0.000937s | 0.000975s | 0.000907s | byte equality observed |
| is_write_buffered_select.test | 10 | 0.000933s | 0.000994s | 0.000908s | byte equality observed |
| prepend.test | 10 | 0.000999s | 0.001117s | 0.000978s | byte equality observed |
| prepend_select.test | 10 | 0.000972s | 0.001081s | 0.000941s | byte equality observed |
| read.test | 10 | 0.001407s | 0.001457s | 0.001308s | byte equality observed |
| read_at.test | 10 | 0.001787s | 0.001937s | 0.001658s | byte equality observed |
| read_at_select.test | 10 | 0.001811s | 0.001951s | 0.001668s | byte equality observed |
| read_select.test | 10 | 0.001422s | 0.001456s | 0.001344s | byte equality observed |
| read_until.test | 10 | 0.001149s | 0.001225s | 0.001220s | byte equality observed |
| read_until_select.test | 10 | 0.001272s | 0.001214s | 0.001226s | byte equality observed |
| recycling_allocator.test | 10 | 0.001021s | 0.000978s | 0.001008s | byte equality observed |
| recycling_allocator_select.test | 10 | 0.001006s | 0.001013s | 0.001058s | byte equality observed |
| signal_set.test | 10 | 0.001045s | 0.001048s | 0.001014s | byte equality observed |
| signal_set_select.test | 10 | 0.001073s | 0.001037s | 0.001049s | byte equality observed |
| socket_base.test | 10 | 0.001140s | 0.001122s | 0.001107s | byte equality observed |
| socket_base_select.test | 10 | 0.001133s | 0.001149s | 0.001138s | byte equality observed |
| spawn.test | 10 | 0.001224s | 0.001204s | 0.001242s | byte equality observed |
| spawn_select.test | 10 | 0.001257s | 0.001261s | 0.001216s | byte equality observed |
| static_thread_pool.test | 10 | 0.001043s | 0.000975s | 0.001044s | byte equality observed |
| static_thread_pool_select.test | 10 | 0.000964s | 0.001030s | 0.000990s | byte equality observed |
| strand.test | 1 | 0.001937s | 0.001952s | 0.001852s | byte equality observed |
| strand_select.test | 1 | 0.001657s | 0.001698s | 0.001732s | byte equality observed |
| streambuf.test | 10 | 0.001029s | 0.001161s | 0.001092s | byte equality observed |
| streambuf_select.test | 10 | 0.001097s | 0.001075s | 0.001081s | byte equality observed |
| thread_pool.test | 8 | 0.002339s | 0.002300s | 0.002338s | byte equality observed |
| thread_pool_select.test | 9 | 0.002295s | 0.002289s | 0.002315s | byte equality observed |
| write.test | 10 | 0.001617s | 0.001528s | 0.001511s | byte equality observed |
| write_at.test | 10 | 0.001752s | 0.001650s | 0.001679s | byte equality observed |
| write_at_select.test | 10 | 0.001789s | 0.001677s | 0.001671s | byte equality observed |
| write_select.test | 10 | 0.001621s | 0.001486s | 0.001488s | byte equality observed |

Compiler command and timing observations: [cppgm](https://github.com/vishvananda/cppgm-extended/actions/runs/42682904325) · [gcc](https://github.com/vishvananda/cppgm-extended/actions/runs/42682904325) · [clang](https://github.com/vishvananda/cppgm-extended/actions/runs/42682904325).

[Reviewed outcomes and per-test records](../results.json) · [Host and toolchains](../metadata.json)
