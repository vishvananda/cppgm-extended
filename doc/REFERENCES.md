# Offline language references

| Reference | Edition and use |
| --- | --- |
| [N3485](n3485.txt) | C++11 baseline clauses and existing course citations |
| [N4140 text](n4140.txt), [HTML](n4140.html) | C++14 draft, from Tim Song's N4140 source rendering with editorial fixes |
| [N4659 text](n4659.txt), [PDF](n4659.pdf) | C++17 working draft, 2017-03-21; use the PDF for formatting/page numbers |
| [CWG and paper index](cwg/INDEX.txt) | Individually retained defect reports and proposal wording |
| [Itanium ABI](itanium-mangling.txt) | ABI names and runtime conventions |
| [WebAssembly bundle](wasm/README.md) | PA37 core encoding, wasm32 data model and object ABI |

These are references, not a requirement to implement their full language or
library chapters. The [required profile](../LANGUAGE_PROFILE.md) and handouts
define course scope. Cite the draft number and stable clause tag, such as
N4659 `[dcl.struct.bind]`; paragraph numbers differ between editions.

The C++14 HTML/text is a reproduction of the public source rendering, not the
ISO publication or a downloaded official PDF. It retains the rendering's
editorial fixes. The C++17 PDF is the WG21-hosted working draft. Sources,
dates, source revision and file SHA-256 hashes are recorded in
[language-drafts.json](language-drafts.json). Copyright and contributor
attribution remain with ISO/IEC and the original draft contributors/renderers.
Chapter text, math images and internal chapter links work offline. Links to
the source renderer’s separate lookup/index pages remain online. Text extraction
may lose layout; consult HTML/PDF when punctuation matters.

A newer draft does not silently replace an earlier-mode rule. Adopted defect
resolutions and course extensions are stated separately, with the retained
paper/issue provenance. Compiler agreement is useful evidence, not the entire
correctness contract.
