// The text view and object compile consume the same preprocessor controls.
#ifndef FIRST_FORCED_HEADER
#error first forced header was not read
#endif
#ifndef SECOND_FORCED_HEADER
#error second forced header was not read
#endif
#if __has_include(<vector>)
#error nostdinc must suppress the host standard include paths
#endif
int main() { return forced_value == 37 && second_value == 5 ? 0 : 1; }
