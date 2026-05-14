#!/usr/bin/env python3

import importlib.util
import pathlib
import textwrap
import unittest
from unittest import mock


REPO_ROOT = pathlib.Path(__file__).resolve().parents[2]
MODULE_PATH = REPO_ROOT / "scripts" / "dump_host_abi_object_facts.py"


def load_module():
    spec = importlib.util.spec_from_file_location("dump_host_abi_object_facts", MODULE_PATH)
    module = importlib.util.module_from_spec(spec)
    assert spec.loader is not None
    spec.loader.exec_module(module)
    return module


MODULE = load_module()


class DumpHostAbiObjectFactsTests(unittest.TestCase):
    def test_symbol_filters(self) -> None:
        self.assertTrue(MODULE.is_host_abi_symbol("_ZTV4Poly"))
        self.assertTrue(MODULE.is_host_abi_symbol("_ZTI4Poly"))
        self.assertTrue(MODULE.is_host_abi_symbol("_ZTS4Poly"))
        self.assertTrue(MODULE.is_host_abi_symbol("_ZTv0_n24_N4Poly1fEv"))
        self.assertTrue(MODULE.is_host_abi_symbol("__dynamic_cast"))
        self.assertFalse(MODULE.is_host_abi_symbol("_main"))

    def test_dump_facts_filters_to_host_abi_surface_macho(self) -> None:
        fake_path = pathlib.Path("/tmp/fake.o")
        with mock.patch.object(MODULE, "detect_format", return_value="mach-o"), \
             mock.patch.object(
                 MODULE,
                 "run_command",
                 side_effect=[
                     textwrap.dedent(
                         """\
                         0000000000000000 T _main
                         0000000000000040 S __ZTV4Poly
                         0000000000000060 S __ZTI4Poly
                                          U __ZTVN10__cxxabiv117__class_type_infoE
                                          U ___dynamic_cast
                         """
                     ),
                     textwrap.dedent(
                         """\
                         0000000000000000 T main
                         0000000000000040 S vtable for Poly
                         0000000000000060 S typeinfo for Poly
                         00000000000000a0 T non-virtual thunk to Poly::f() const
                         """
                     ),
                     textwrap.dedent(
                         """\
                         Section
                           sectname __text
                            segname __TEXT
                               addr 0x0
                         Section
                           sectname __data
                            segname __DATA
                               addr 0x40
                         """
                     ),
                     textwrap.dedent(
                         """\
                         Relocation information (__TEXT,__text) 1 entries
                         address  pcrel length extern type    scattered symbolnum/value
                         00000018 True  long   True   BRANCH  False     ___dynamic_cast
                         Relocation information (__DATA,__data) 2 entries
                         address  pcrel length extern type    scattered symbolnum/value
                         00000000 False long   True   UNSIGND False     __ZTI4Poly
                         00000008 False long   True   UNSIGND False     __ZN4Poly1fEv
                         """
                     ),
                 ],
             ):
            facts = MODULE.dump_facts(fake_path)

        self.assertEqual(
            facts,
            [
                "object_format mach-o",
                "define _ZTI4Poly",
                "define _ZTV4Poly",
                "undef _ZTVN10__cxxabiv117__class_type_infoE",
                "undef __dynamic_cast",
                "define_thunk non-virtual thunk to Poly::f() const",
                "section data",
                "section text",
                "reloc data abs64 _ZN4Poly1fEv",
                "reloc data abs64 _ZTI4Poly",
                "reloc text branch32 __dynamic_cast",
            ],
        )

    def test_dump_facts_filters_to_host_abi_surface_elf(self) -> None:
        fake_path = pathlib.Path("/tmp/fake.o")
        with mock.patch.object(MODULE, "detect_format", return_value="elf"), \
             mock.patch.object(
                 MODULE,
                 "run_command",
                 side_effect=[
                     textwrap.dedent(
                         """\
                         0000000000000000 T _main
                         0000000000000000 V _ZTV8HostPoly
                                          U _ZTI11HostDerived
                                          U __cxa_bad_cast
                         """
                     ),
                     textwrap.dedent(
                         """\
                         0000000000000000 T main
                         0000000000000030 T covariant return thunk to HostDerived::self()
                         """
                     ),
                     textwrap.dedent(
                         """\
                           [ 1] .text             PROGBITS         0000000000000000  00000040
                           [ 2] .data.rel.ro      PROGBITS         0000000000000000  00000080
                           [ 3] .symtab           SYMTAB           0000000000000000  00000100
                         """
                     ),
                     textwrap.dedent(
                         """\
                         Relocation section '.rela.text' at offset 0x1f0 contains 1 entry:
                             Offset             Info             Type               Symbol's Value  Symbol's Name + Addend
                         0000000000000018  0000000400000004 R_X86_64_PLT32         0000000000000000 __cxa_bad_cast - 4
                         Relocation section '.rela.data.rel.ro' at offset 0x220 contains 2 entries:
                             Offset             Info             Type               Symbol's Value  Symbol's Name + Addend
                         0000000000000000  0000000500000001 R_X86_64_64            0000000000000000 _ZTI11HostDerived + 0
                         0000000000000008  0000000600000001 R_X86_64_64            0000000000000000 _ZTv0_n24_N11HostDerived4selfEv + 0
                         """
                     ),
                 ],
             ):
            facts = MODULE.dump_facts(fake_path)

        self.assertEqual(
            facts,
            [
                "object_format elf",
                "define _ZTV8HostPoly",
                "undef _ZTI11HostDerived",
                "undef __cxa_bad_cast",
                "define_thunk covariant return thunk to HostDerived::self()",
                "section data",
                "section text",
                "reloc data abs64 _ZTI11HostDerived",
                "reloc data abs64 _ZTv0_n24_N11HostDerived4selfEv",
                "reloc text branch32 __cxa_bad_cast",
            ],
        )


if __name__ == "__main__":
    unittest.main()
