// CWG 1490/1467 and 12.2p5: aggregate reference members keep their
// initialized character-array temporaries alive. expect exit 0
// CXX11-021.
struct Text { const char (&value)[5]; };
Text global_text = {{"abc"}};

int main()
{
    Text local_text = {{"xy"}};
    return global_text.value[0] != 'a' || global_text.value[4] != 0 ||
           local_text.value[0] != 'x' || local_text.value[1] != 'y' || local_text.value[4] != 0;
}
