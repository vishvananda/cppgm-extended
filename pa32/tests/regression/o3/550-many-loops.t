global @cell : i32 [binding=internal] = 0

function @many(%p : ptr, %n : i64, %m : i32) -> i32 [binding=strong, no_inline=yes] {
  block ^entry:
    jump ^head_0

  block ^head_0:
    %i_0 = phi i64 [^entry: 0, ^body_0: %next_0]
    %s_0 = phi i32 [^entry: 0, ^body_0: %sum_0]
    %c_0 = cmp lt i64 %i_0, %n
    branch %c_0, ^body_0, ^after_0

  block ^body_0:
    %q_0 = index i32 %p, %i_0
    %v_0 = load i32 %q_0
    %w_0 = binary mul i32 %m, 3
    %x_0 = binary add i32 %v_0, %w_0
    %sum_0 = binary add i32 %s_0, %x_0
    %next_0 = binary add nowrap i64 %i_0, 1
    jump ^head_0

  block ^after_0:
    %z_0 = phi i32 [^head_0: %s_0]
    jump ^head_1

  block ^head_1:
    %i_1 = phi i64 [^after_0: 0, ^body_1: %next_1]
    %s_1 = phi i32 [^after_0: %z_0, ^body_1: %sum_1]
    %c_1 = cmp lt i64 %i_1, %n
    branch %c_1, ^body_1, ^after_1

  block ^body_1:
    %q_1 = index i32 %p, %i_1
    %v_1 = load i32 %q_1
    %w_1 = binary mul i32 %m, 4
    %x_1 = binary add i32 %v_1, %w_1
    %sum_1 = binary add i32 %s_1, %x_1
    %next_1 = binary add nowrap i64 %i_1, 1
    jump ^head_1

  block ^after_1:
    %z_1 = binary xor i32 %s_1, 1
    %y_1 = cmp ne i32 %z_1, 0
    branch %y_1, ^through_1, ^through_1

  block ^through_1:
    jump ^head_2

  block ^head_2:
    %i_2 = phi i64 [^through_1: 0, ^body_2: %next_2]
    %s_2 = phi i32 [^through_1: %z_1, ^body_2: %sum_2]
    %c_2 = cmp lt i64 %i_2, %n
    branch %c_2, ^body_2, ^after_2

  block ^body_2:
    %q_2 = index i32 %p, %i_2
    %v_2 = load i32 %q_2
    %w_2 = binary mul i32 %m, 5
    %x_2 = binary add i32 %v_2, %w_2
    %sum_2 = binary add i32 %s_2, %x_2
    %next_2 = binary add nowrap i64 %i_2, 1
    jump ^head_2

  block ^after_2:
    %z_2 = phi i32 [^head_2: %s_2]
    jump ^head_3

  block ^head_3:
    %i_3 = phi i64 [^after_2: 0, ^body_3: %next_3]
    %s_3 = phi i32 [^after_2: %z_2, ^body_3: %sum_3]
    %c_3 = cmp lt i64 %i_3, %n
    branch %c_3, ^body_3, ^after_3

  block ^body_3:
    %q_3 = index i32 %p, %i_3
    %v_3 = load i32 %q_3
    %w_3 = binary mul i32 %m, 6
    %x_3 = binary add i32 %v_3, %w_3
    %sum_3 = binary add i32 %s_3, %x_3
    %next_3 = binary add nowrap i64 %i_3, 1
    jump ^head_3

  block ^after_3:
    %z_3 = binary xor i32 %s_3, 3
    %y_3 = cmp ne i32 %z_3, 0
    branch %y_3, ^through_3, ^through_3

  block ^through_3:
    jump ^head_4

  block ^head_4:
    %i_4 = phi i64 [^through_3: 0, ^body_4: %next_4]
    %s_4 = phi i32 [^through_3: %z_3, ^body_4: %sum_4]
    %c_4 = cmp lt i64 %i_4, %n
    branch %c_4, ^body_4, ^after_4

  block ^body_4:
    %q_4 = index i32 %p, %i_4
    %v_4 = load i32 %q_4
    %w_4 = binary mul i32 %m, 7
    %x_4 = binary add i32 %v_4, %w_4
    %sum_4 = binary add i32 %s_4, %x_4
    %next_4 = binary add nowrap i64 %i_4, 1
    jump ^head_4

  block ^after_4:
    %z_4 = phi i32 [^head_4: %s_4]
    jump ^head_5

  block ^head_5:
    %i_5 = phi i64 [^after_4: 0, ^body_5: %next_5]
    %s_5 = phi i32 [^after_4: %z_4, ^body_5: %sum_5]
    %c_5 = cmp lt i64 %i_5, %n
    branch %c_5, ^body_5, ^after_5

  block ^body_5:
    %q_5 = index i32 %p, %i_5
    %v_5 = load i32 %q_5
    %w_5 = binary mul i32 %m, 8
    %x_5 = binary add i32 %v_5, %w_5
    %sum_5 = binary add i32 %s_5, %x_5
    %next_5 = binary add nowrap i64 %i_5, 1
    jump ^head_5

  block ^after_5:
    %z_5 = binary xor i32 %s_5, 5
    %y_5 = cmp ne i32 %z_5, 0
    branch %y_5, ^through_5, ^through_5

  block ^through_5:
    jump ^head_6

  block ^head_6:
    %i_6 = phi i64 [^through_5: 0, ^body_6: %next_6]
    %s_6 = phi i32 [^through_5: %z_5, ^body_6: %sum_6]
    %c_6 = cmp lt i64 %i_6, %n
    branch %c_6, ^body_6, ^after_6

  block ^body_6:
    %q_6 = index i32 %p, %i_6
    %v_6 = load i32 %q_6
    %w_6 = binary mul i32 %m, 9
    %x_6 = binary add i32 %v_6, %w_6
    %sum_6 = binary add i32 %s_6, %x_6
    %next_6 = binary add nowrap i64 %i_6, 1
    jump ^head_6

  block ^after_6:
    %z_6 = phi i32 [^head_6: %s_6]
    jump ^head_7

  block ^head_7:
    %i_7 = phi i64 [^after_6: 0, ^body_7: %next_7]
    %s_7 = phi i32 [^after_6: %z_6, ^body_7: %sum_7]
    %c_7 = cmp lt i64 %i_7, %n
    branch %c_7, ^body_7, ^after_7

  block ^body_7:
    %q_7 = index i32 %p, %i_7
    %v_7 = load i32 %q_7
    %w_7 = binary mul i32 %m, 10
    %x_7 = binary add i32 %v_7, %w_7
    %sum_7 = binary add i32 %s_7, %x_7
    %next_7 = binary add nowrap i64 %i_7, 1
    jump ^head_7

  block ^after_7:
    %z_7 = binary xor i32 %s_7, 7
    %y_7 = cmp ne i32 %z_7, 0
    branch %y_7, ^through_7, ^through_7

  block ^through_7:
    jump ^head_8

  block ^head_8:
    %i_8 = phi i64 [^through_7: 0, ^body_8: %next_8]
    %s_8 = phi i32 [^through_7: %z_7, ^body_8: %sum_8]
    %c_8 = cmp lt i64 %i_8, %n
    branch %c_8, ^body_8, ^after_8

  block ^body_8:
    %q_8 = index i32 %p, %i_8
    %v_8 = load i32 %q_8
    %w_8 = binary mul i32 %m, 11
    %x_8 = binary add i32 %v_8, %w_8
    %sum_8 = binary add i32 %s_8, %x_8
    %next_8 = binary add nowrap i64 %i_8, 1
    jump ^head_8

  block ^after_8:
    %z_8 = phi i32 [^head_8: %s_8]
    jump ^head_9

  block ^head_9:
    %i_9 = phi i64 [^after_8: 0, ^body_9: %next_9]
    %s_9 = phi i32 [^after_8: %z_8, ^body_9: %sum_9]
    %c_9 = cmp lt i64 %i_9, %n
    branch %c_9, ^body_9, ^after_9

  block ^body_9:
    %q_9 = index i32 %p, %i_9
    %v_9 = load i32 %q_9
    %w_9 = binary mul i32 %m, 12
    %x_9 = binary add i32 %v_9, %w_9
    %sum_9 = binary add i32 %s_9, %x_9
    %next_9 = binary add nowrap i64 %i_9, 1
    jump ^head_9

  block ^after_9:
    %z_9 = binary xor i32 %s_9, 9
    %y_9 = cmp ne i32 %z_9, 0
    branch %y_9, ^through_9, ^through_9

  block ^through_9:
    jump ^head_10

  block ^head_10:
    %i_10 = phi i64 [^through_9: 0, ^body_10: %next_10]
    %s_10 = phi i32 [^through_9: %z_9, ^body_10: %sum_10]
    %c_10 = cmp lt i64 %i_10, %n
    branch %c_10, ^body_10, ^after_10

  block ^body_10:
    %q_10 = index i32 %p, %i_10
    %v_10 = load i32 %q_10
    %w_10 = binary mul i32 %m, 13
    %x_10 = binary add i32 %v_10, %w_10
    %sum_10 = binary add i32 %s_10, %x_10
    %next_10 = binary add nowrap i64 %i_10, 1
    jump ^head_10

  block ^after_10:
    %z_10 = phi i32 [^head_10: %s_10]
    jump ^head_11

  block ^head_11:
    %i_11 = phi i64 [^after_10: 0, ^body_11: %next_11]
    %s_11 = phi i32 [^after_10: %z_10, ^body_11: %sum_11]
    %c_11 = cmp lt i64 %i_11, %n
    branch %c_11, ^body_11, ^after_11

  block ^body_11:
    %q_11 = index i32 %p, %i_11
    %v_11 = load i32 %q_11
    %w_11 = binary mul i32 %m, 14
    %x_11 = binary add i32 %v_11, %w_11
    %sum_11 = binary add i32 %s_11, %x_11
    %next_11 = binary add nowrap i64 %i_11, 1
    jump ^head_11

  block ^after_11:
    %z_11 = binary xor i32 %s_11, 11
    %y_11 = cmp ne i32 %z_11, 0
    branch %y_11, ^through_11, ^through_11

  block ^through_11:
    jump ^head_12

  block ^head_12:
    %i_12 = phi i64 [^through_11: 0, ^body_12: %next_12]
    %s_12 = phi i32 [^through_11: %z_11, ^body_12: %sum_12]
    %c_12 = cmp lt i64 %i_12, %n
    branch %c_12, ^body_12, ^after_12

  block ^body_12:
    %q_12 = index i32 %p, %i_12
    %v_12 = load i32 %q_12
    %w_12 = binary mul i32 %m, 15
    %x_12 = binary add i32 %v_12, %w_12
    %sum_12 = binary add i32 %s_12, %x_12
    %next_12 = binary add nowrap i64 %i_12, 1
    jump ^head_12

  block ^after_12:
    %z_12 = phi i32 [^head_12: %s_12]
    jump ^head_13

  block ^head_13:
    %i_13 = phi i64 [^after_12: 0, ^body_13: %next_13]
    %s_13 = phi i32 [^after_12: %z_12, ^body_13: %sum_13]
    %c_13 = cmp lt i64 %i_13, %n
    branch %c_13, ^body_13, ^after_13

  block ^body_13:
    %q_13 = index i32 %p, %i_13
    %v_13 = load i32 %q_13
    %w_13 = binary mul i32 %m, 16
    %x_13 = binary add i32 %v_13, %w_13
    %sum_13 = binary add i32 %s_13, %x_13
    %next_13 = binary add nowrap i64 %i_13, 1
    jump ^head_13

  block ^after_13:
    %z_13 = binary xor i32 %s_13, 13
    %y_13 = cmp ne i32 %z_13, 0
    branch %y_13, ^through_13, ^through_13

  block ^through_13:
    jump ^head_14

  block ^head_14:
    %i_14 = phi i64 [^through_13: 0, ^body_14: %next_14]
    %s_14 = phi i32 [^through_13: %z_13, ^body_14: %sum_14]
    %c_14 = cmp lt i64 %i_14, %n
    branch %c_14, ^body_14, ^after_14

  block ^body_14:
    %q_14 = index i32 %p, %i_14
    %v_14 = load i32 %q_14
    %w_14 = binary mul i32 %m, 17
    %x_14 = binary add i32 %v_14, %w_14
    %sum_14 = binary add i32 %s_14, %x_14
    %next_14 = binary add nowrap i64 %i_14, 1
    jump ^head_14

  block ^after_14:
    %z_14 = phi i32 [^head_14: %s_14]
    jump ^head_15

  block ^head_15:
    %i_15 = phi i64 [^after_14: 0, ^body_15: %next_15]
    %s_15 = phi i32 [^after_14: %z_14, ^body_15: %sum_15]
    %c_15 = cmp lt i64 %i_15, %n
    branch %c_15, ^body_15, ^after_15

  block ^body_15:
    %q_15 = index i32 %p, %i_15
    %v_15 = load i32 %q_15
    %w_15 = binary mul i32 %m, 18
    %x_15 = binary add i32 %v_15, %w_15
    %sum_15 = binary add i32 %s_15, %x_15
    %next_15 = binary add nowrap i64 %i_15, 1
    jump ^head_15

  block ^after_15:
    %z_15 = binary xor i32 %s_15, 15
    %y_15 = cmp ne i32 %z_15, 0
    branch %y_15, ^through_15, ^through_15

  block ^through_15:
    jump ^head_16

  block ^head_16:
    %i_16 = phi i64 [^through_15: 0, ^body_16: %next_16]
    %s_16 = phi i32 [^through_15: %z_15, ^body_16: %sum_16]
    %c_16 = cmp lt i64 %i_16, %n
    branch %c_16, ^body_16, ^after_16

  block ^body_16:
    %q_16 = index i32 %p, %i_16
    %v_16 = load i32 %q_16
    %w_16 = binary mul i32 %m, 19
    %x_16 = binary add i32 %v_16, %w_16
    %sum_16 = binary add i32 %s_16, %x_16
    %next_16 = binary add nowrap i64 %i_16, 1
    jump ^head_16

  block ^after_16:
    %z_16 = phi i32 [^head_16: %s_16]
    jump ^head_17

  block ^head_17:
    %i_17 = phi i64 [^after_16: 0, ^body_17: %next_17]
    %s_17 = phi i32 [^after_16: %z_16, ^body_17: %sum_17]
    %c_17 = cmp lt i64 %i_17, %n
    branch %c_17, ^body_17, ^after_17

  block ^body_17:
    %q_17 = index i32 %p, %i_17
    %v_17 = load i32 %q_17
    %w_17 = binary mul i32 %m, 20
    %x_17 = binary add i32 %v_17, %w_17
    %sum_17 = binary add i32 %s_17, %x_17
    %next_17 = binary add nowrap i64 %i_17, 1
    jump ^head_17

  block ^after_17:
    %z_17 = binary xor i32 %s_17, 17
    %y_17 = cmp ne i32 %z_17, 0
    branch %y_17, ^through_17, ^through_17

  block ^through_17:
    jump ^head_18

  block ^head_18:
    %i_18 = phi i64 [^through_17: 0, ^body_18: %next_18]
    %s_18 = phi i32 [^through_17: %z_17, ^body_18: %sum_18]
    %c_18 = cmp lt i64 %i_18, %n
    branch %c_18, ^body_18, ^after_18

  block ^body_18:
    %q_18 = index i32 %p, %i_18
    %v_18 = load i32 %q_18
    %w_18 = binary mul i32 %m, 21
    %x_18 = binary add i32 %v_18, %w_18
    %sum_18 = binary add i32 %s_18, %x_18
    %next_18 = binary add nowrap i64 %i_18, 1
    jump ^head_18

  block ^after_18:
    %z_18 = phi i32 [^head_18: %s_18]
    jump ^head_19

  block ^head_19:
    %i_19 = phi i64 [^after_18: 0, ^body_19: %next_19]
    %s_19 = phi i32 [^after_18: %z_18, ^body_19: %sum_19]
    %c_19 = cmp lt i64 %i_19, %n
    branch %c_19, ^body_19, ^after_19

  block ^body_19:
    %q_19 = index i32 %p, %i_19
    %v_19 = load i32 %q_19
    %w_19 = binary mul i32 %m, 22
    %x_19 = binary add i32 %v_19, %w_19
    %sum_19 = binary add i32 %s_19, %x_19
    %next_19 = binary add nowrap i64 %i_19, 1
    jump ^head_19

  block ^after_19:
    %z_19 = binary xor i32 %s_19, 19
    %y_19 = cmp ne i32 %z_19, 0
    branch %y_19, ^through_19, ^through_19

  block ^through_19:
    jump ^head_20

  block ^head_20:
    %i_20 = phi i64 [^through_19: 0, ^body_20: %next_20]
    %s_20 = phi i32 [^through_19: %z_19, ^body_20: %sum_20]
    %c_20 = cmp lt i64 %i_20, %n
    branch %c_20, ^body_20, ^after_20

  block ^body_20:
    %q_20 = index i32 %p, %i_20
    %v_20 = load i32 %q_20
    %w_20 = binary mul i32 %m, 23
    %x_20 = binary add i32 %v_20, %w_20
    %sum_20 = binary add i32 %s_20, %x_20
    %next_20 = binary add nowrap i64 %i_20, 1
    jump ^head_20

  block ^after_20:
    %z_20 = phi i32 [^head_20: %s_20]
    jump ^head_21

  block ^head_21:
    %i_21 = phi i64 [^after_20: 0, ^body_21: %next_21]
    %s_21 = phi i32 [^after_20: %z_20, ^body_21: %sum_21]
    %c_21 = cmp lt i64 %i_21, %n
    branch %c_21, ^body_21, ^after_21

  block ^body_21:
    %q_21 = index i32 %p, %i_21
    %v_21 = load i32 %q_21
    %w_21 = binary mul i32 %m, 24
    %x_21 = binary add i32 %v_21, %w_21
    %sum_21 = binary add i32 %s_21, %x_21
    %next_21 = binary add nowrap i64 %i_21, 1
    jump ^head_21

  block ^after_21:
    %z_21 = binary xor i32 %s_21, 21
    %y_21 = cmp ne i32 %z_21, 0
    branch %y_21, ^through_21, ^through_21

  block ^through_21:
    jump ^head_22

  block ^head_22:
    %i_22 = phi i64 [^through_21: 0, ^body_22: %next_22]
    %s_22 = phi i32 [^through_21: %z_21, ^body_22: %sum_22]
    %c_22 = cmp lt i64 %i_22, %n
    branch %c_22, ^body_22, ^after_22

  block ^body_22:
    %q_22 = index i32 %p, %i_22
    %v_22 = load i32 %q_22
    %w_22 = binary mul i32 %m, 25
    %x_22 = binary add i32 %v_22, %w_22
    %sum_22 = binary add i32 %s_22, %x_22
    %next_22 = binary add nowrap i64 %i_22, 1
    jump ^head_22

  block ^after_22:
    %z_22 = phi i32 [^head_22: %s_22]
    jump ^head_23

  block ^head_23:
    %i_23 = phi i64 [^after_22: 0, ^body_23: %next_23]
    %s_23 = phi i32 [^after_22: %z_22, ^body_23: %sum_23]
    %c_23 = cmp lt i64 %i_23, %n
    branch %c_23, ^body_23, ^after_23

  block ^body_23:
    %q_23 = index i32 %p, %i_23
    %v_23 = load i32 %q_23
    %w_23 = binary mul i32 %m, 26
    %x_23 = binary add i32 %v_23, %w_23
    %sum_23 = binary add i32 %s_23, %x_23
    %next_23 = binary add nowrap i64 %i_23, 1
    jump ^head_23

  block ^after_23:
    %z_23 = binary xor i32 %s_23, 23
    %y_23 = cmp ne i32 %z_23, 0
    branch %y_23, ^through_23, ^through_23

  block ^through_23:
    jump ^head_24

  block ^head_24:
    %i_24 = phi i64 [^through_23: 0, ^body_24: %next_24]
    %s_24 = phi i32 [^through_23: %z_23, ^body_24: %sum_24]
    %c_24 = cmp lt i64 %i_24, %n
    branch %c_24, ^body_24, ^after_24

  block ^body_24:
    %q_24 = index i32 %p, %i_24
    %v_24 = load i32 %q_24
    %w_24 = binary mul i32 %m, 27
    %x_24 = binary add i32 %v_24, %w_24
    %sum_24 = binary add i32 %s_24, %x_24
    %next_24 = binary add nowrap i64 %i_24, 1
    jump ^head_24

  block ^after_24:
    %z_24 = phi i32 [^head_24: %s_24]
    jump ^head_25

  block ^head_25:
    %i_25 = phi i64 [^after_24: 0, ^body_25: %next_25]
    %s_25 = phi i32 [^after_24: %z_24, ^body_25: %sum_25]
    %c_25 = cmp lt i64 %i_25, %n
    branch %c_25, ^body_25, ^after_25

  block ^body_25:
    %q_25 = index i32 %p, %i_25
    %v_25 = load i32 %q_25
    %w_25 = binary mul i32 %m, 28
    %x_25 = binary add i32 %v_25, %w_25
    %sum_25 = binary add i32 %s_25, %x_25
    %next_25 = binary add nowrap i64 %i_25, 1
    jump ^head_25

  block ^after_25:
    %z_25 = binary xor i32 %s_25, 25
    %y_25 = cmp ne i32 %z_25, 0
    branch %y_25, ^through_25, ^through_25

  block ^through_25:
    jump ^head_26

  block ^head_26:
    %i_26 = phi i64 [^through_25: 0, ^body_26: %next_26]
    %s_26 = phi i32 [^through_25: %z_25, ^body_26: %sum_26]
    %c_26 = cmp lt i64 %i_26, %n
    branch %c_26, ^body_26, ^after_26

  block ^body_26:
    %q_26 = index i32 %p, %i_26
    %v_26 = load i32 %q_26
    %w_26 = binary mul i32 %m, 29
    %x_26 = binary add i32 %v_26, %w_26
    %sum_26 = binary add i32 %s_26, %x_26
    %next_26 = binary add nowrap i64 %i_26, 1
    jump ^head_26

  block ^after_26:
    %z_26 = phi i32 [^head_26: %s_26]
    jump ^head_27

  block ^head_27:
    %i_27 = phi i64 [^after_26: 0, ^body_27: %next_27]
    %s_27 = phi i32 [^after_26: %z_26, ^body_27: %sum_27]
    %c_27 = cmp lt i64 %i_27, %n
    branch %c_27, ^body_27, ^after_27

  block ^body_27:
    %q_27 = index i32 %p, %i_27
    %v_27 = load i32 %q_27
    %w_27 = binary mul i32 %m, 30
    %x_27 = binary add i32 %v_27, %w_27
    %sum_27 = binary add i32 %s_27, %x_27
    %next_27 = binary add nowrap i64 %i_27, 1
    jump ^head_27

  block ^after_27:
    %z_27 = binary xor i32 %s_27, 27
    %y_27 = cmp ne i32 %z_27, 0
    branch %y_27, ^through_27, ^through_27

  block ^through_27:
    jump ^head_28

  block ^head_28:
    %i_28 = phi i64 [^through_27: 0, ^body_28: %next_28]
    %s_28 = phi i32 [^through_27: %z_27, ^body_28: %sum_28]
    %c_28 = cmp lt i64 %i_28, %n
    branch %c_28, ^body_28, ^after_28

  block ^body_28:
    %q_28 = index i32 %p, %i_28
    %v_28 = load i32 %q_28
    %w_28 = binary mul i32 %m, 31
    %x_28 = binary add i32 %v_28, %w_28
    %sum_28 = binary add i32 %s_28, %x_28
    %next_28 = binary add nowrap i64 %i_28, 1
    jump ^head_28

  block ^after_28:
    %z_28 = phi i32 [^head_28: %s_28]
    jump ^head_29

  block ^head_29:
    %i_29 = phi i64 [^after_28: 0, ^body_29: %next_29]
    %s_29 = phi i32 [^after_28: %z_28, ^body_29: %sum_29]
    %c_29 = cmp lt i64 %i_29, %n
    branch %c_29, ^body_29, ^after_29

  block ^body_29:
    %q_29 = index i32 %p, %i_29
    %v_29 = load i32 %q_29
    %w_29 = binary mul i32 %m, 32
    %x_29 = binary add i32 %v_29, %w_29
    %sum_29 = binary add i32 %s_29, %x_29
    %next_29 = binary add nowrap i64 %i_29, 1
    jump ^head_29

  block ^after_29:
    %z_29 = binary xor i32 %s_29, 29
    %y_29 = cmp ne i32 %z_29, 0
    branch %y_29, ^through_29, ^through_29

  block ^through_29:
    jump ^head_30

  block ^head_30:
    %i_30 = phi i64 [^through_29: 0, ^body_30: %next_30]
    %s_30 = phi i32 [^through_29: %z_29, ^body_30: %sum_30]
    %c_30 = cmp lt i64 %i_30, %n
    branch %c_30, ^body_30, ^after_30

  block ^body_30:
    %q_30 = index i32 %p, %i_30
    %v_30 = load i32 %q_30
    %w_30 = binary mul i32 %m, 33
    %x_30 = binary add i32 %v_30, %w_30
    %sum_30 = binary add i32 %s_30, %x_30
    %next_30 = binary add nowrap i64 %i_30, 1
    jump ^head_30

  block ^after_30:
    %z_30 = phi i32 [^head_30: %s_30]
    jump ^head_31

  block ^head_31:
    %i_31 = phi i64 [^after_30: 0, ^body_31: %next_31]
    %s_31 = phi i32 [^after_30: %z_30, ^body_31: %sum_31]
    %c_31 = cmp lt i64 %i_31, %n
    branch %c_31, ^body_31, ^after_31

  block ^body_31:
    %q_31 = index i32 %p, %i_31
    %v_31 = load i32 %q_31
    %w_31 = binary mul i32 %m, 34
    %x_31 = binary add i32 %v_31, %w_31
    %sum_31 = binary add i32 %s_31, %x_31
    %next_31 = binary add nowrap i64 %i_31, 1
    jump ^head_31

  block ^after_31:
    %z_31 = binary xor i32 %s_31, 31
    %y_31 = cmp ne i32 %z_31, 0
    branch %y_31, ^through_31, ^through_31

  block ^through_31:
    jump ^head_32

  block ^head_32:
    %i_32 = phi i64 [^through_31: 0, ^body_32: %next_32]
    %s_32 = phi i32 [^through_31: %z_31, ^body_32: %sum_32]
    %c_32 = cmp lt i64 %i_32, %n
    branch %c_32, ^body_32, ^after_32

  block ^body_32:
    %q_32 = index i32 %p, %i_32
    %v_32 = load i32 %q_32
    %w_32 = binary mul i32 %m, 35
    %x_32 = binary add i32 %v_32, %w_32
    %sum_32 = binary add i32 %s_32, %x_32
    %next_32 = binary add nowrap i64 %i_32, 1
    jump ^head_32

  block ^after_32:
    %z_32 = phi i32 [^head_32: %s_32]
    jump ^head_33

  block ^head_33:
    %i_33 = phi i64 [^after_32: 0, ^body_33: %next_33]
    %s_33 = phi i32 [^after_32: %z_32, ^body_33: %sum_33]
    %c_33 = cmp lt i64 %i_33, %n
    branch %c_33, ^body_33, ^after_33

  block ^body_33:
    %q_33 = index i32 %p, %i_33
    %v_33 = load i32 %q_33
    %w_33 = binary mul i32 %m, 36
    %x_33 = binary add i32 %v_33, %w_33
    %sum_33 = binary add i32 %s_33, %x_33
    %next_33 = binary add nowrap i64 %i_33, 1
    jump ^head_33

  block ^after_33:
    %z_33 = binary xor i32 %s_33, 33
    %y_33 = cmp ne i32 %z_33, 0
    branch %y_33, ^through_33, ^through_33

  block ^through_33:
    jump ^head_34

  block ^head_34:
    %i_34 = phi i64 [^through_33: 0, ^body_34: %next_34]
    %s_34 = phi i32 [^through_33: %z_33, ^body_34: %sum_34]
    %c_34 = cmp lt i64 %i_34, %n
    branch %c_34, ^body_34, ^after_34

  block ^body_34:
    %q_34 = index i32 %p, %i_34
    %v_34 = load i32 %q_34
    %w_34 = binary mul i32 %m, 37
    %x_34 = binary add i32 %v_34, %w_34
    %sum_34 = binary add i32 %s_34, %x_34
    %next_34 = binary add nowrap i64 %i_34, 1
    jump ^head_34

  block ^after_34:
    %z_34 = phi i32 [^head_34: %s_34]
    jump ^head_35

  block ^head_35:
    %i_35 = phi i64 [^after_34: 0, ^body_35: %next_35]
    %s_35 = phi i32 [^after_34: %z_34, ^body_35: %sum_35]
    %c_35 = cmp lt i64 %i_35, %n
    branch %c_35, ^body_35, ^after_35

  block ^body_35:
    %q_35 = index i32 %p, %i_35
    %v_35 = load i32 %q_35
    %w_35 = binary mul i32 %m, 38
    %x_35 = binary add i32 %v_35, %w_35
    %sum_35 = binary add i32 %s_35, %x_35
    %next_35 = binary add nowrap i64 %i_35, 1
    jump ^head_35

  block ^after_35:
    %z_35 = binary xor i32 %s_35, 35
    %y_35 = cmp ne i32 %z_35, 0
    branch %y_35, ^through_35, ^through_35

  block ^through_35:
    jump ^done

  block ^done:
    return i32 %z_35
}

function @memory_sums(%p : ptr, %n : i64) -> void [binding=strong, no_inline=yes] {
  block ^entry:
    jump ^head_0

  block ^head_0:
    %i_0 = phi i64 [^entry: 0, ^body_0: %next_0]
    %c_0 = cmp lt i64 %i_0, %n
    branch %c_0, ^body_0, ^after_0

  block ^body_0:
    %q_0 = index i32 %p, %i_0
    store i32 7, %q_0
    %r_0 = load i32 @cell
    %t_0 = binary add i32 %r_0, 1
    store i32 %t_0, @cell
    %next_0 = binary add nowrap i64 %i_0, 1
    jump ^head_0

  block ^after_0:
    %e_0 = load volatile i32 @cell
    jump ^head_1

  block ^head_1:
    %i_1 = phi i64 [^after_0: 0, ^body_1: %next_1]
    %c_1 = cmp lt i64 %i_1, %n
    branch %c_1, ^body_1, ^after_1

  block ^body_1:
    %q_1 = index i32 %p, %i_1
    store i32 7, %q_1
    %r_1 = load i32 @cell
    %t_1 = binary add i32 %r_1, 2
    store i32 %t_1, @cell
    %next_1 = binary add nowrap i64 %i_1, 1
    jump ^head_1

  block ^after_1:
    %e_1 = load volatile i32 @cell
    jump ^head_2

  block ^head_2:
    %i_2 = phi i64 [^after_1: 0, ^body_2: %next_2]
    %c_2 = cmp lt i64 %i_2, %n
    branch %c_2, ^body_2, ^after_2

  block ^body_2:
    %q_2 = index i32 %p, %i_2
    store i32 7, %q_2
    %r_2 = load i32 @cell
    %t_2 = binary add i32 %r_2, 3
    store i32 %t_2, @cell
    %next_2 = binary add nowrap i64 %i_2, 1
    jump ^head_2

  block ^after_2:
    %e_2 = load volatile i32 @cell
    jump ^head_3

  block ^head_3:
    %i_3 = phi i64 [^after_2: 0, ^body_3: %next_3]
    %c_3 = cmp lt i64 %i_3, %n
    branch %c_3, ^body_3, ^after_3

  block ^body_3:
    %q_3 = index i32 %p, %i_3
    store i32 7, %q_3
    %r_3 = load i32 @cell
    %t_3 = binary add i32 %r_3, 4
    store i32 %t_3, @cell
    %next_3 = binary add nowrap i64 %i_3, 1
    jump ^head_3

  block ^after_3:
    %e_3 = load volatile i32 @cell
    jump ^head_4

  block ^head_4:
    %i_4 = phi i64 [^after_3: 0, ^body_4: %next_4]
    %c_4 = cmp lt i64 %i_4, %n
    branch %c_4, ^body_4, ^after_4

  block ^body_4:
    %q_4 = index i32 %p, %i_4
    store i32 7, %q_4
    %r_4 = load i32 @cell
    %t_4 = binary add i32 %r_4, 5
    store i32 %t_4, @cell
    %next_4 = binary add nowrap i64 %i_4, 1
    jump ^head_4

  block ^after_4:
    %e_4 = load volatile i32 @cell
    jump ^head_5

  block ^head_5:
    %i_5 = phi i64 [^after_4: 0, ^body_5: %next_5]
    %c_5 = cmp lt i64 %i_5, %n
    branch %c_5, ^body_5, ^after_5

  block ^body_5:
    %q_5 = index i32 %p, %i_5
    store i32 7, %q_5
    %r_5 = load i32 @cell
    %t_5 = binary add i32 %r_5, 6
    store i32 %t_5, @cell
    %next_5 = binary add nowrap i64 %i_5, 1
    jump ^head_5

  block ^after_5:
    %e_5 = load volatile i32 @cell
    jump ^head_6

  block ^head_6:
    %i_6 = phi i64 [^after_5: 0, ^body_6: %next_6]
    %c_6 = cmp lt i64 %i_6, %n
    branch %c_6, ^body_6, ^after_6

  block ^body_6:
    %q_6 = index i32 %p, %i_6
    store i32 7, %q_6
    %r_6 = load i32 @cell
    %t_6 = binary add i32 %r_6, 7
    store i32 %t_6, @cell
    %next_6 = binary add nowrap i64 %i_6, 1
    jump ^head_6

  block ^after_6:
    %e_6 = load volatile i32 @cell
    jump ^head_7

  block ^head_7:
    %i_7 = phi i64 [^after_6: 0, ^body_7: %next_7]
    %c_7 = cmp lt i64 %i_7, %n
    branch %c_7, ^body_7, ^after_7

  block ^body_7:
    %q_7 = index i32 %p, %i_7
    store i32 7, %q_7
    %r_7 = load i32 @cell
    %t_7 = binary add i32 %r_7, 8
    store i32 %t_7, @cell
    %next_7 = binary add nowrap i64 %i_7, 1
    jump ^head_7

  block ^after_7:
    %e_7 = load volatile i32 @cell
    jump ^head_8

  block ^head_8:
    %i_8 = phi i64 [^after_7: 0, ^body_8: %next_8]
    %c_8 = cmp lt i64 %i_8, %n
    branch %c_8, ^body_8, ^after_8

  block ^body_8:
    %q_8 = index i32 %p, %i_8
    store i32 7, %q_8
    %r_8 = load i32 @cell
    %t_8 = binary add i32 %r_8, 9
    store i32 %t_8, @cell
    %next_8 = binary add nowrap i64 %i_8, 1
    jump ^head_8

  block ^after_8:
    %e_8 = load volatile i32 @cell
    jump ^head_9

  block ^head_9:
    %i_9 = phi i64 [^after_8: 0, ^body_9: %next_9]
    %c_9 = cmp lt i64 %i_9, %n
    branch %c_9, ^body_9, ^after_9

  block ^body_9:
    %q_9 = index i32 %p, %i_9
    store i32 7, %q_9
    %r_9 = load i32 @cell
    %t_9 = binary add i32 %r_9, 10
    store i32 %t_9, @cell
    %next_9 = binary add nowrap i64 %i_9, 1
    jump ^head_9

  block ^after_9:
    %e_9 = load volatile i32 @cell
    jump ^head_10

  block ^head_10:
    %i_10 = phi i64 [^after_9: 0, ^body_10: %next_10]
    %c_10 = cmp lt i64 %i_10, %n
    branch %c_10, ^body_10, ^after_10

  block ^body_10:
    %q_10 = index i32 %p, %i_10
    store i32 7, %q_10
    %r_10 = load i32 @cell
    %t_10 = binary add i32 %r_10, 11
    store i32 %t_10, @cell
    %next_10 = binary add nowrap i64 %i_10, 1
    jump ^head_10

  block ^after_10:
    %e_10 = load volatile i32 @cell
    jump ^head_11

  block ^head_11:
    %i_11 = phi i64 [^after_10: 0, ^body_11: %next_11]
    %c_11 = cmp lt i64 %i_11, %n
    branch %c_11, ^body_11, ^after_11

  block ^body_11:
    %q_11 = index i32 %p, %i_11
    store i32 7, %q_11
    %r_11 = load i32 @cell
    %t_11 = binary add i32 %r_11, 12
    store i32 %t_11, @cell
    %next_11 = binary add nowrap i64 %i_11, 1
    jump ^head_11

  block ^after_11:
    %e_11 = load volatile i32 @cell
    jump ^head_12

  block ^head_12:
    %i_12 = phi i64 [^after_11: 0, ^body_12: %next_12]
    %c_12 = cmp lt i64 %i_12, %n
    branch %c_12, ^body_12, ^after_12

  block ^body_12:
    %q_12 = index i32 %p, %i_12
    store i32 7, %q_12
    %r_12 = load i32 @cell
    %t_12 = binary add i32 %r_12, 13
    store i32 %t_12, @cell
    %next_12 = binary add nowrap i64 %i_12, 1
    jump ^head_12

  block ^after_12:
    %e_12 = load volatile i32 @cell
    jump ^head_13

  block ^head_13:
    %i_13 = phi i64 [^after_12: 0, ^body_13: %next_13]
    %c_13 = cmp lt i64 %i_13, %n
    branch %c_13, ^body_13, ^after_13

  block ^body_13:
    %q_13 = index i32 %p, %i_13
    store i32 7, %q_13
    %r_13 = load i32 @cell
    %t_13 = binary add i32 %r_13, 14
    store i32 %t_13, @cell
    %next_13 = binary add nowrap i64 %i_13, 1
    jump ^head_13

  block ^after_13:
    %e_13 = load volatile i32 @cell
    jump ^head_14

  block ^head_14:
    %i_14 = phi i64 [^after_13: 0, ^body_14: %next_14]
    %c_14 = cmp lt i64 %i_14, %n
    branch %c_14, ^body_14, ^after_14

  block ^body_14:
    %q_14 = index i32 %p, %i_14
    store i32 7, %q_14
    %r_14 = load i32 @cell
    %t_14 = binary add i32 %r_14, 15
    store i32 %t_14, @cell
    %next_14 = binary add nowrap i64 %i_14, 1
    jump ^head_14

  block ^after_14:
    %e_14 = load volatile i32 @cell
    jump ^head_15

  block ^head_15:
    %i_15 = phi i64 [^after_14: 0, ^body_15: %next_15]
    %c_15 = cmp lt i64 %i_15, %n
    branch %c_15, ^body_15, ^after_15

  block ^body_15:
    %q_15 = index i32 %p, %i_15
    store i32 7, %q_15
    %r_15 = load i32 @cell
    %t_15 = binary add i32 %r_15, 16
    store i32 %t_15, @cell
    %next_15 = binary add nowrap i64 %i_15, 1
    jump ^head_15

  block ^after_15:
    %e_15 = load volatile i32 @cell
    jump ^head_16

  block ^head_16:
    %i_16 = phi i64 [^after_15: 0, ^body_16: %next_16]
    %c_16 = cmp lt i64 %i_16, %n
    branch %c_16, ^body_16, ^after_16

  block ^body_16:
    %q_16 = index i32 %p, %i_16
    store i32 7, %q_16
    %r_16 = load i32 @cell
    %t_16 = binary add i32 %r_16, 17
    store i32 %t_16, @cell
    %next_16 = binary add nowrap i64 %i_16, 1
    jump ^head_16

  block ^after_16:
    %e_16 = load volatile i32 @cell
    jump ^head_17

  block ^head_17:
    %i_17 = phi i64 [^after_16: 0, ^body_17: %next_17]
    %c_17 = cmp lt i64 %i_17, %n
    branch %c_17, ^body_17, ^after_17

  block ^body_17:
    %q_17 = index i32 %p, %i_17
    store i32 7, %q_17
    %r_17 = load i32 @cell
    %t_17 = binary add i32 %r_17, 18
    store i32 %t_17, @cell
    %next_17 = binary add nowrap i64 %i_17, 1
    jump ^head_17

  block ^after_17:
    %e_17 = load volatile i32 @cell
    jump ^done

  block ^done:
    return void
}

function @main() -> i32 [role=entry] {
  block ^entry:
    return i32 0
}
