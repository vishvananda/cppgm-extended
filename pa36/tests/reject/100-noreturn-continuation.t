assembly @stop intel none 33 0 0 0 { text "ud2" }
function @invalid() -> void {
  block ^entry:
    asm @stop()
    return void
}
