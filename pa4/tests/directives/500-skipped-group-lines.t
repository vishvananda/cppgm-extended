// A line of a skipped group ends the group only when it is a directive.
first
#if 0
skipped // a comment that ends in a splice \
#endif
still skipped
#endif
second
#if 0
skipped /* a comment that goes on
#endif
and ends here */ # endif
still skipped
/* a comment that goes on
and ends here */ # /* another */ endif
third
#if 0
R"x(a raw string that goes on
#endif
and ends here)x" skipped
"a string /* that goes on \
#endif" skipped
#endif
fourth
#if 0
skipped
%:endif
fifth
#if 0
skipped ??/
#endif
still skipped
??=endif
sixth
#if 0
#if 1
skipped // /* no block comment
#else
#endif
skipped
	 # elif 1 // taken
seventh
#else
skipped
#endif
eighth
#if 0
skipped \
#else
skipped
#/* a comment */else
ninth
#endif
#ifdef absent
'\\' skipped '"' /* a comment */ skipped
%:\
else
tenth
#endif
