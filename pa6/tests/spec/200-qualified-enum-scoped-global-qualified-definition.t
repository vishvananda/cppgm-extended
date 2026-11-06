// N3485 7.2p4, CWG1411: a qualified enum definition may use bare global qualification.
// valid type declarations
enum class E; enum class ::E{zero};
