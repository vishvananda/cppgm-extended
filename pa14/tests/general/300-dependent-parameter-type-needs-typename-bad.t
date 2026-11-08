// The fixed-alias allowance must not classify this dependent qualified type
// as a type without the required typename introducer.
template<class T> struct Reader { void read(const T::Metadata::element& value); };
