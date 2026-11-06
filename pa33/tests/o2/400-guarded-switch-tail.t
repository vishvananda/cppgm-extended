function @dispatch(%selector : i64, %guard : i64, %seed : i64) -> i64 {
  block ^entry:
    %skip = cmp eq i64 %guard, 0
    branch %skip, ^skip, ^check

  block ^skip:
    return i64 %seed

  block ^check:
    %outside = cmp ugt i64 %selector, 5
    branch %outside, ^outside, ^choose

  block ^outside:
    %wide = copy i64 %selector
    %fallback = binary xor i64 %seed, %wide
    return i64 %fallback

  block ^choose:
    %negative = cmp lt i64 %guard, 0
    branch %negative, ^left, ^right

  block ^left:
    %a = binary add i64 %seed, 17
    jump ^table

  block ^right:
    %b = binary xor i64 %seed, 4
    jump ^table

  block ^table:
    %value = phi i64 [^left: %a, ^right: %b]
    switch %selector, ^impossible, 0:^case0, 1:^case1, 2:^case2, 3:^case3, 4:^case4, 5:^case5

  block ^case0:
    %v0 = binary add i64 %value, 1
    return i64 %v0
  block ^case1:
    %v1 = binary sub i64 %value, 7
    return i64 %v1
  block ^case2:
    %v2 = binary mul i64 %value, 3
    return i64 %v2
  block ^case3:
    %v3 = binary xor i64 %value, 19
    return i64 %v3
  block ^case4:
    %v4 = binary or i64 %value, 16
    return i64 %v4
  block ^case5:
    %v5 = binary and i64 %value, 127
    return i64 %v5
  block ^impossible:
    unreachable
}

function @main() -> i64 [role=entry] {
  block ^entry:
    %got0 = call i64 @dispatch(0, 0, 43)
    %bad0 = cmp ne i64 %got0, 43
    %all0 = binary or i64 0, %bad0
    %got1 = call i64 @dispatch(1, 0, 43)
    %bad1 = cmp ne i64 %got1, 43
    %all1 = binary or i64 %all0, %bad1
    %got2 = call i64 @dispatch(2, 0, 43)
    %bad2 = cmp ne i64 %got2, 43
    %all2 = binary or i64 %all1, %bad2
    %got3 = call i64 @dispatch(3, 0, 43)
    %bad3 = cmp ne i64 %got3, 43
    %all3 = binary or i64 %all2, %bad3
    %got4 = call i64 @dispatch(4, 0, 43)
    %bad4 = cmp ne i64 %got4, 43
    %all4 = binary or i64 %all3, %bad4
    %got5 = call i64 @dispatch(5, 0, 43)
    %bad5 = cmp ne i64 %got5, 43
    %all5 = binary or i64 %all4, %bad5
    %got6 = call i64 @dispatch(6, 0, 43)
    %bad6 = cmp ne i64 %got6, 43
    %all6 = binary or i64 %all5, %bad6
    %got7 = call i64 @dispatch(4294967296, 0, 43)
    %bad7 = cmp ne i64 %got7, 43
    %all7 = binary or i64 %all6, %bad7
    %got8 = call i64 @dispatch(18446744073709551615, 0, 43)
    %bad8 = cmp ne i64 %got8, 43
    %all8 = binary or i64 %all7, %bad8
    %got9 = call i64 @dispatch(0, -3, 43)
    %bad9 = cmp ne i64 %got9, 61
    %all9 = binary or i64 %all8, %bad9
    %got10 = call i64 @dispatch(1, -3, 43)
    %bad10 = cmp ne i64 %got10, 53
    %all10 = binary or i64 %all9, %bad10
    %got11 = call i64 @dispatch(2, -3, 43)
    %bad11 = cmp ne i64 %got11, 180
    %all11 = binary or i64 %all10, %bad11
    %got12 = call i64 @dispatch(3, -3, 43)
    %bad12 = cmp ne i64 %got12, 47
    %all12 = binary or i64 %all11, %bad12
    %got13 = call i64 @dispatch(4, -3, 43)
    %bad13 = cmp ne i64 %got13, 60
    %all13 = binary or i64 %all12, %bad13
    %got14 = call i64 @dispatch(5, -3, 43)
    %bad14 = cmp ne i64 %got14, 60
    %all14 = binary or i64 %all13, %bad14
    %got15 = call i64 @dispatch(6, -3, 43)
    %bad15 = cmp ne i64 %got15, 45
    %all15 = binary or i64 %all14, %bad15
    %got16 = call i64 @dispatch(4294967296, -3, 43)
    %bad16 = cmp ne i64 %got16, 4294967339
    %all16 = binary or i64 %all15, %bad16
    %got17 = call i64 @dispatch(18446744073709551615, -3, 43)
    %bad17 = cmp ne i64 %got17, -44
    %all17 = binary or i64 %all16, %bad17
    %got18 = call i64 @dispatch(0, 1, 43)
    %bad18 = cmp ne i64 %got18, 48
    %all18 = binary or i64 %all17, %bad18
    %got19 = call i64 @dispatch(1, 1, 43)
    %bad19 = cmp ne i64 %got19, 40
    %all19 = binary or i64 %all18, %bad19
    %got20 = call i64 @dispatch(2, 1, 43)
    %bad20 = cmp ne i64 %got20, 141
    %all20 = binary or i64 %all19, %bad20
    %got21 = call i64 @dispatch(3, 1, 43)
    %bad21 = cmp ne i64 %got21, 60
    %all21 = binary or i64 %all20, %bad21
    %got22 = call i64 @dispatch(4, 1, 43)
    %bad22 = cmp ne i64 %got22, 63
    %all22 = binary or i64 %all21, %bad22
    %got23 = call i64 @dispatch(5, 1, 43)
    %bad23 = cmp ne i64 %got23, 47
    %all23 = binary or i64 %all22, %bad23
    %got24 = call i64 @dispatch(6, 1, 43)
    %bad24 = cmp ne i64 %got24, 45
    %all24 = binary or i64 %all23, %bad24
    %got25 = call i64 @dispatch(4294967296, 1, 43)
    %bad25 = cmp ne i64 %got25, 4294967339
    %all25 = binary or i64 %all24, %bad25
    %got26 = call i64 @dispatch(18446744073709551615, 1, 43)
    %bad26 = cmp ne i64 %got26, -44
    %all26 = binary or i64 %all25, %bad26
    return i64 %all26
}
