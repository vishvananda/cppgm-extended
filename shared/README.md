# Shared Source Grammar

`source.gram` is the canonical source-language grammar for `cppgm++` frontend
assignments starting at PA5.

The local `paN/paN.gram` files for PA5-PA7, PA10-PA15, PA20-PA23, and PA25 expose
this shared grammar under each assignment's filename. README files
should use this standard wording:

> The authoritative source syntax is the shared `cppgm++` source grammar,
> exposed for this assignment as `paN.gram`. The grammar defines accepted syntax
> only; the PA-specific semantic and lowering requirements are defined by the
> assignment boundary and out-of-scope sections.

A construct being accepted by this grammar does not make its semantic analysis
or code generation required in every assignment.
