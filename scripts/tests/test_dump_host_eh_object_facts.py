#!/usr/bin/env python3

import importlib.util
import pathlib
import textwrap
import unittest
from unittest import mock


REPO_ROOT = pathlib.Path(__file__).resolve().parents[2]
MODULE_PATH = REPO_ROOT / "scripts" / "dump_host_eh_object_facts.py"


def load_module():
    spec = importlib.util.spec_from_file_location("dump_host_eh_object_facts", MODULE_PATH)
    module = importlib.util.module_from_spec(spec)
    assert spec.loader is not None
    spec.loader.exec_module(module)
    return module


MODULE = load_module()


class DumpHostEhObjectFactsTests(unittest.TestCase):
    def test_host_eh_symbol_filter(self) -> None:
        self.assertTrue(MODULE.is_host_eh_symbol("__cxa_throw"))
        self.assertTrue(MODULE.is_host_eh_symbol("__gxx_personality_v0"))
        self.assertTrue(MODULE.is_host_eh_symbol("_Unwind_Resume"))
        self.assertTrue(MODULE.is_host_eh_symbol("_ZTIi"))
        self.assertTrue(MODULE.is_host_eh_symbol("_ZTS9BaseError"))
        self.assertFalse(MODULE.is_host_eh_symbol("_main"))

    def test_dump_facts_filters_to_eh_surface_macho(self) -> None:
        fake_path = pathlib.Path("/tmp/fake.o")
        with mock.patch.object(MODULE, "detect_format", return_value="mach-o"), \
             mock.patch.object(
                 MODULE,
                 "run_command",
                 side_effect=[
                     textwrap.dedent(
                         """\
                         0000000000000000 T _main
                                          U ___cxa_throw
                                          U ___gxx_personality_v0
                                          U __ZTIi
                         """
                     ),
                     textwrap.dedent(
                         """\
                         Section
                           sectname __text
                            segname __TEXT
                               addr 0x0
                         Section
                           sectname __gcc_except_tab
                            segname __TEXT
                               addr 0x40
                         Section
                           sectname __compact_unwind
                            segname __LD
                               addr 0x80
                         """
                     ),
                     textwrap.dedent(
                         """\
                         Relocation information (__TEXT,__text) 2 entries
                         address  pcrel length extern type    scattered symbolnum/value
                         00000018 True  long   True   BRANCH  False     ___cxa_throw
                         00000030 True  long   True   GOT_LD  False     __ZTIi
                         Relocation information (__TEXT,__gcc_except_tab) 1 entries
                         address  pcrel length extern type    scattered symbolnum/value
                         00000010 True  long   True   GOT     False     __ZTIi
                         Relocation information (__LD,__compact_unwind) 2 entries
                         address  pcrel length extern type    scattered symbolnum/value
                         00000030 False ?( 3)  True   UNSIGND False     ___gxx_personality_v0
                         00000038 False ?( 3)  False  UNSIGND False     2 (__TEXT,__gcc_except_tab)
                         """
                     ),
                 ],
             ):
            facts = MODULE.dump_facts(fake_path)

        self.assertEqual(
            facts,
            [
                "object_format mach-o",
                "undef _ZTIi",
                "undef __cxa_throw",
                "undef __gxx_personality_v0",
                "section compact_unwind",
                "section gcc_except_table",
                "section text",
                "reloc compact_unwind abs64 __gxx_personality_v0",
                "reloc compact_unwind abs64 section:gcc_except_table",
                "reloc gcc_except_table got _ZTIi",
                "reloc text branch32 __cxa_throw",
                "reloc text gotpcrel _ZTIi",
            ],
        )

    def test_dump_facts_filters_to_eh_surface_elf(self) -> None:
        fake_path = pathlib.Path("/tmp/fake.o")
        with mock.patch.object(MODULE, "detect_format", return_value="elf"), \
             mock.patch.object(
                 MODULE,
                 "run_command",
                 side_effect=[
                     textwrap.dedent(
                         """\
                         0000000000000000 T _main
                                          U __cxa_throw
                                          U __gxx_personality_v0
                                          U _ZTI9BaseError
                         """
                     ),
                     textwrap.dedent(
                         """\
                           [ 1] .text             PROGBITS         0000000000000000  00000040
                           [ 2] .gcc_except_table PROGBITS         0000000000000000  00000080
                           [ 3] .eh_frame         X86_64_UNWIND    0000000000000000  000000b0
                           [ 4] .symtab           SYMTAB           0000000000000000  00000100
                         """
                     ),
                     textwrap.dedent(
                         """\
                         Relocation section '.rela.text' at offset 0x1f0 contains 2 entries:
                             Offset             Info             Type               Symbol's Value  Symbol's Name + Addend
                         0000000000000018  0000000400000004 R_X86_64_PLT32         0000000000000000 __cxa_throw - 4
                         0000000000000030  000000050000002a R_X86_64_REX_GOTPCRELX 0000000000000000 _ZTI9BaseError - 4
                         Relocation section '.rela.gcc_except_table' at offset 0x220 contains 1 entry:
                             Offset             Info             Type               Symbol's Value  Symbol's Name + Addend
                         0000000000000010  0000000500000009 R_X86_64_GOTPCREL      0000000000000000 _ZTI9BaseError + 0
                         Relocation section '.rela.eh_frame' at offset 0x250 contains 2 entries:
                             Offset             Info             Type               Symbol's Value  Symbol's Name + Addend
                         0000000000000020  0000000200000002 R_X86_64_PC32          0000000000000000 .text + 0
                         0000000000000053  0000000600000009 R_X86_64_GOTPCREL      0000000000000000 __gxx_personality_v0 + 0
                         """
                     ),
                 ],
             ):
            facts = MODULE.dump_facts(fake_path)

        self.assertEqual(
            facts,
            [
                "object_format elf",
                "undef _ZTI9BaseError",
                "undef __cxa_throw",
                "undef __gxx_personality_v0",
                "section eh_frame",
                "section gcc_except_table",
                "section text",
                "reloc eh_frame gotpcrel __gxx_personality_v0",
                "reloc eh_frame pcrel32 section:text",
                "reloc gcc_except_table gotpcrel _ZTI9BaseError",
                "reloc text branch32 __cxa_throw",
                "reloc text gotpcrel _ZTI9BaseError",
            ],
        )


if __name__ == "__main__":
    unittest.main()
