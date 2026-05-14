#!/usr/bin/env python3

import importlib.util
import pathlib
import textwrap
import unittest
from unittest import mock


REPO_ROOT = pathlib.Path(__file__).resolve().parents[2]
MODULE_PATH = REPO_ROOT / "scripts" / "dump_private_eh_object_facts.py"


def load_module():
    spec = importlib.util.spec_from_file_location("dump_private_eh_object_facts", MODULE_PATH)
    module = importlib.util.module_from_spec(spec)
    assert spec.loader is not None
    spec.loader.exec_module(module)
    return module


MODULE = load_module()


class DumpPrivateEhObjectFactsTests(unittest.TestCase):
    def test_parse_nm_output_handles_macho_and_strips_leading_underscore(self) -> None:
        text = textwrap.dedent(
            """\
            0000000000000000 T __cppgm_406d61696e
                             U _cppgm_priv_exc_top
                             U _cppgm_priv_exc_unhandled
            """
        )
        defined, undefined = MODULE.parse_nm_output(text, "mach-o")
        self.assertEqual(defined, ["_cppgm_406d61696e"])
        self.assertEqual(undefined, ["cppgm_priv_exc_top", "cppgm_priv_exc_unhandled"])

    def test_parse_macho_sections_and_relocations_normalize_output(self) -> None:
        sections_text = textwrap.dedent(
            """\
            Section
              sectname __text
               segname __TEXT
                  addr 0x0
            Section
              sectname __compact_unwind
               segname __LD
                  addr 0xe0
            """
        )
        reloc_text = textwrap.dedent(
            """\
            Relocation information (__TEXT,__text) 2 entries
            address  pcrel length extern type    scattered symbolnum/value
            00000018 True  long   True   SIGNED  False     _cppgm_priv_exc_top
            00000090 True  long   True   BRANCH  False     _cppgm_priv_exc_unhandled
            Relocation information (__LD,__compact_unwind) 1 entries
            address  pcrel length extern type    scattered symbolnum/value
            00000000 False ?( 3)  False  UNSIGND False     1 (__TEXT,__text)
            """
        )
        self.assertEqual(
            MODULE.parse_macho_sections(sections_text),
            ["compact_unwind", "text"],
        )
        self.assertEqual(
            MODULE.parse_macho_relocations(reloc_text),
            [
                ("compact_unwind", "abs64", "section:text"),
                ("text", "branch32", "cppgm_priv_exc_unhandled"),
                ("text", "pcrel32", "cppgm_priv_exc_top"),
            ],
        )

    def test_parse_elf_sections_and_relocations_normalize_output(self) -> None:
        sections_text = textwrap.dedent(
            """\
              [ 1] .text             PROGBITS         0000000000000000  00000040
              [ 2] .eh_frame         X86_64_UNWIND    0000000000000000  00000080
            """
        )
        reloc_text = textwrap.dedent(
            """\
            Relocation section '.rela.text' at offset 0x1f0 contains 2 entries:
                Offset             Info             Type               Symbol's Value  Symbol's Name + Addend
            0000000000000018  0000000400000002 R_X86_64_PC32          0000000000000000 cppgm_priv_exc_top - 4
            0000000000000090  0000000500000004 R_X86_64_PLT32         0000000000000000 cppgm_priv_exc_unhandled - 4
            Relocation section '.rela.eh_frame' at offset 0x220 contains 1 entry:
                Offset             Info             Type               Symbol's Value  Symbol's Name + Addend
            0000000000000020  0000000200000002 R_X86_64_PC32          0000000000000000 .text + 0
            """
        )
        self.assertEqual(MODULE.parse_elf_sections(sections_text), ["eh_frame", "text"])
        self.assertEqual(
            MODULE.parse_elf_relocations(reloc_text),
            [
                ("eh_frame", "pcrel32", "section:text"),
                ("text", "branch32", "cppgm_priv_exc_unhandled"),
                ("text", "pcrel32", "cppgm_priv_exc_top"),
            ],
        )

    def test_dump_facts_filters_uninteresting_sections(self) -> None:
        fake_path = pathlib.Path("/tmp/fake.o")

        with mock.patch.object(MODULE, "detect_format", return_value="elf"), \
             mock.patch.object(
                 MODULE,
                 "run_command",
                 side_effect=[
                     "0000000000000000 T __cppgm_406d61696e\n                 U cppgm_priv_exc_top\n",
                     textwrap.dedent(
                         """\
                           [ 0] NULL             NULL             0000000000000000  00000000
                           [ 1] .text            PROGBITS         0000000000000000  00000040
                           [ 2] .data            PROGBITS         0000000000000000  00000080
                           [ 3] .rela.text       RELA             0000000000000000  000000a0
                           [ 4] .symtab          SYMTAB           0000000000000000  000000c0
                         """
                     ),
                     textwrap.dedent(
                         """\
                         Relocation section '.rela.text' at offset 0x1f0 contains 1 entry:
                             Offset             Info             Type               Symbol's Value  Symbol's Name + Addend
                         0000000000000018  0000000400000002 R_X86_64_PC32          0000000000000000 cppgm_priv_exc_top - 4
                         """
                     ),
                 ],
             ):
            facts = MODULE.dump_facts(fake_path)

        self.assertEqual(
            facts,
            [
                "object_format elf",
                "define __cppgm_406d61696e",
                "undef cppgm_priv_exc_top",
                "section data",
                "section text",
                "reloc text pcrel32 cppgm_priv_exc_top",
            ],
        )


if __name__ == "__main__":
    unittest.main()
