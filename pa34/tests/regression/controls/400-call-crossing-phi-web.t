# Four sums run through a loop over pairs of elements, then through a loop
# over the rest, as the vectorizer leaves Shootout moments. Code after the
# loops reads the sums, some after calls. Each sum keeps one register through
# both loops: only the values the calls cross leave it.

global @calls : i64 = 0

function @touch() -> void [no_inline=yes, unwind=no] {
  block ^entry:
    %count = load typed i64 @calls
    %next = binary add i64 %count, 1
    store i64 %next, @calls
    return void
}

function @root(%x : f64) -> f64 [no_inline=yes, unwind=no] {
  block ^entry:
    %r = unary sqrt f64 %x
    return f64 %r
}

function @moments(%first : ptr, %last : ptr, %mean : f64, %out : ptr) -> f64 [no_inline=yes, unwind=no] {
  block ^entry:
    %bytes = binary sub ptr %last, %first
    %count = binary shr exact i64 %bytes, 3
    %n = convert uitofp f64 i64 %count
    %any = cmp ne ptr %first, %last
    branch %any, ^guard, ^done

  block ^guard:
    %ordered = cmp ult ptr %first, %last
    %wide = cmp uge ptr %bytes, 16
    %pairs = binary and i64 %ordered, %wide
    branch %pairs, ^lanes_pre, ^tail_pre

  block ^lanes_pre:
    %lanes_end = index i8 %last, -16
    %mean_lane = vector fromf64 %mean
    %means = vector pshufd %mean_lane, 68
    %mask_lane = vector fromi64 9223372036854775807
    %masks = vector pshufd %mask_lane, 68
    jump ^lanes

  block ^lanes:
    %q = phi f64 [^lanes_pre: 0, ^lanes: %q2]
    %c = phi f64 [^lanes_pre: 0, ^lanes: %c2]
    %s = phi f64 [^lanes_pre: 0, ^lanes: %s2]
    %a = phi f64 [^lanes_pre: 0, ^lanes: %a2]
    %p = phi ptr [^lanes_pre: %first, ^lanes: %p_next]
    %x = load v128 %p
    %d = vector subpd %x, %means
    %ad = vector pand %d, %masks
    %ad0 = vector tof64 %ad
    %a1 = binary add f64 %a, %ad0
    %ad_high = vector pshufd %ad, 238
    %ad1 = vector tof64 %ad_high
    %a2 = binary add f64 %a1, %ad1
    %d2 = vector mulpd %d, %d
    %d2_0 = vector tof64 %d2
    %s1 = binary add f64 %s, %d2_0
    %d2_high = vector pshufd %d2, 238
    %d2_1 = vector tof64 %d2_high
    %s2 = binary add f64 %s1, %d2_1
    %d3 = vector mulpd %d, %d2
    %d3_0 = vector tof64 %d3
    %c1 = binary add f64 %c, %d3_0
    %d3_high = vector pshufd %d3, 238
    %d3_1 = vector tof64 %d3_high
    %c2 = binary add f64 %c1, %d3_1
    %d4 = vector mulpd %d, %d3
    %d4_0 = vector tof64 %d4
    %q1 = binary add f64 %q, %d4_0
    %d4_high = vector pshufd %d4, 238
    %d4_1 = vector tof64 %d4_high
    %q2 = binary add f64 %q1, %d4_1
    %p_next = index i8 %p, 16
    %more = cmp ule ptr %p_next, %lanes_end
    branch %more, ^lanes, ^lanes_exit

  block ^lanes_exit:
    %left = cmp ult ptr %p_next, %last
    branch %left, ^tail_pre, ^done

  block ^tail_pre:
    %tq0 = phi f64 [^guard: 0, ^lanes_exit: %q2]
    %tc0 = phi f64 [^guard: 0, ^lanes_exit: %c2]
    %ts0 = phi f64 [^guard: 0, ^lanes_exit: %s2]
    %ta0 = phi f64 [^guard: 0, ^lanes_exit: %a2]
    %tp0 = phi ptr [^guard: %first, ^lanes_exit: %p_next]
    jump ^tail

  block ^tail:
    %tp = phi ptr [^tail: %tp_next, ^tail_pre: %tp0]
    %ta = phi f64 [^tail: %ta1, ^tail_pre: %ta0]
    %ts = phi f64 [^tail: %ts1, ^tail_pre: %ts0]
    %tc = phi f64 [^tail: %tc1, ^tail_pre: %tc0]
    %tq = phi f64 [^tail: %tq1, ^tail_pre: %tq0]
    %y = load typed f64 %tp
    %e = binary sub f64 %y, %mean
    %e_bits = copy i64 %e
    %ae_bits = binary and i64 %e_bits, 9223372036854775807
    %ae = copy f64 %ae_bits
    %ta1 = binary add f64 %ae, %ta
    %e2 = binary mul f64 %e, %e
    %ts1 = binary add f64 %e2, %ts
    %e3 = binary mul f64 %e, %e2
    %tc1 = binary add f64 %e3, %tc
    %e4 = binary mul f64 %e, %e3
    %tq1 = binary add f64 %e4, %tq
    %tp_next = index inbounds i8 %tp, 8
    %again = cmp ne ptr %tp_next, %last
    branch %again, ^tail, ^done

  block ^done:
    %abs_sum = phi f64 [^entry: 0, ^tail: %ta1, ^lanes_exit: %a2]
    %square_sum = phi f64 [^entry: 0, ^tail: %ts1, ^lanes_exit: %s2]
    %cube_sum = phi f64 [^entry: 0, ^tail: %tc1, ^lanes_exit: %c2]
    %fourth_sum = phi f64 [^entry: 0, ^tail: %tq1, ^lanes_exit: %q2]
    %count1 = binary sub i64 %count, 1
    %n1 = convert uitofp f64 i64 %count1
    %variance = binary div f64 %square_sum, %n1
    %fast_root = unary sqrt f64 %variance
    %negative = cmp lt f64 %variance, 0
    branch %negative, ^slow_root, ^rooted

  block ^rooted:
    %deviation = phi f64 [^done: %fast_root, ^slow_root: %slow]
    %spread = cmp ne f64 %variance, 0
    branch %spread, ^scaled, ^finish

  block ^scaled:
    %nv = binary mul f64 %n, %variance
    %skew_scale = binary mul f64 %deviation, %nv
    %skew_scaled = binary div f64 %cube_sum, %skew_scale
    %kurtosis_scale = binary mul f64 %variance, %nv
    %kurtosis_scaled = binary div f64 %fourth_sum, %kurtosis_scale
    %kurtosis_excess = binary sub f64 %kurtosis_scaled, 3
    jump ^finish

  block ^finish:
    %kurtosis = phi f64 [^rooted: %fourth_sum, ^scaled: %kurtosis_excess]
    %skew = phi f64 [^rooted: %cube_sum, ^scaled: %skew_scaled]
    call void @touch()
    store f64 %abs_sum, %out
    %out1 = index i8 %out, 8
    store f64 %kurtosis, %out1
    %out2 = index i8 %out, 16
    store f64 %skew, %out2
    %out3 = index i8 %out, 24
    store f64 %deviation, %out3
    return f64 %variance

  block ^slow_root:
    %slow = call f64 @root(%variance)
    jump ^rooted
}

# The values 0 to 10 about their mean 5: five passes over pairs, then one.
function @main() -> i32 [role=entry, unwind=no] {
  slot $values : obj<88x8>
  slot $results : obj<32x8>
  block ^entry:
    %base = addr $values
    jump ^fill

  block ^fill:
    %i = phi i64 [^entry: 0, ^fill: %i_next]
    %value = convert sitofp f64 i64 %i
    %at = index f64 %base, %i
    store f64 %value, %at
    %i_next = binary add i64 %i, 1
    %filling = cmp lt i64 %i_next, 11
    branch %filling, ^fill, ^run

  block ^run:
    %end = index f64 %base, 11
    %out = addr $results
    %variance = call f64 @moments(%base, %end, 5, %out)
    %abs_sum = load typed f64 %out
    %out1 = index i8 %out, 8
    %kurtosis = load typed f64 %out1
    %out2 = index i8 %out, 16
    %skew = load typed f64 %out2
    %out3 = index i8 %out, 24
    %deviation = load typed f64 %out3
    %calls = load typed i64 @calls
    %kurtosis_scaled = binary div f64 1958, 1331
    %kurtosis_wanted = binary sub f64 %kurtosis_scaled, 3
    %deviation_wanted = unary sqrt f64 11
    %bad_variance = cmp ne f64 %variance, 11
    %bad_abs_sum = cmp ne f64 %abs_sum, 30
    %bad_kurtosis = cmp ne f64 %kurtosis, %kurtosis_wanted
    %bad_skew = cmp ne f64 %skew, 0
    %bad_deviation = cmp ne f64 %deviation, %deviation_wanted
    %bad_calls = cmp ne i64 %calls, 1
    %bad0 = binary or i32 %bad_variance, %bad_abs_sum
    %bad1 = binary or i32 %bad_kurtosis, %bad_skew
    %bad2 = binary or i32 %bad_deviation, %bad_calls
    %bad3 = binary or i32 %bad0, %bad1
    %bad = binary or i32 %bad3, %bad2
    return i32 %bad
}
