// N3485 [except.handle], [except.throw]: locals finish before active catches.
int live, drops, bad, trace;
struct Error {
  Error() { ++live; }
  Error(Error const&) { ++live; }
  ~Error() { --live; trace = trace * 10 + 9; }
};
struct Guard {
  Guard(int) {}
  ~Guard() { ++drops; if (live != 1) bad = 1; trace = trace * 10 + 1; }
};
void probe(int mode) {
  try { throw Error(); }
  catch (Error const&) {
    trace = 0;
    if (mode == 0) {
      Guard guard(1);
      try { throw 7L; } catch (int) {}
    } else if (mode == 1) {
      try { throw 1; }
      catch (...) {
        Guard guard(1);
        try { throw 7L; } catch (bool) {}
      }
    } else {
      Guard guard(1);
      try { throw; } catch (int) {}
    }
  }
}
int main() {
  for (int mode = 0; mode != 3; ++mode) {
    drops = bad = trace = 0;
    try { probe(mode); }
    catch (long value) {
      if (mode == 2 || value != 7 || live || bad || drops != 1 || trace != 19)
        return 1;
    } catch (Error const&) {
      if (mode != 2 || live != 1 || bad || drops != 1 || trace != 1) return 2;
    }
    if (live || bad || drops != 1 || trace != 19) return 3;
  }
  return 0;
}
