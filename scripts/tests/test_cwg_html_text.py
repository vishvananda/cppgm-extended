#!/usr/bin/env python3
"""Offline change wording must remain distinguishable from adopted wording."""
from pathlib import Path
import importlib.util
import subprocess
import sys
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[2]
SPEC = importlib.util.spec_from_file_location('cwg_html_text', ROOT / 'scripts/cwg_html_text.py')
MODULE = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(MODULE)


def render(source):
    parser = MODULE.HtmlText()
    parser.feed(source)
    parser.close()
    return parser


class ChangeMarkupTests(unittest.TestCase):
    def test_whole_deleted_normative_paragraph_is_not_live_wording(self):
        source = ('<blockquote><s>The name of a function declared in block scope and '
                  'the name of a variable declared by a block scope <code>extern</code> '
                  'declaration have linkage.</s><p>Replacement wording.</p></blockquote>')
        text = render(source).text()
        self.assertIn('[DELETE: The name of a function declared in block scope', text)
        self.assertIn('extern declaration have linkage.]', text)
        self.assertIn('\nReplacement wording.', text)
        self.assertNotIn('Replacement wording.]', text)

    def test_inline_s_u_keep_both_old_and_new_characters(self):
        self.assertEqual(render('<pre>#3: <s>ex</s><u>in</u>ternal linkage'
                                '<s>, ill-formed</s></pre>').text(),
                         '#3: [DELETE: ex][INSERT: in]ternal linkage[DELETE: , ill-formed]\n')

    def test_block_class_markers_cover_all_their_paragraphs(self):
        text = render('<div class="del"><p>Old first.</p><p>Old second.</p></div>'
                      '<div class="ins"><p>New first.</p><p>New second.</p></div>'
                      '<p>Ordinary continuation.</p>').text()
        self.assertLess(text.index('[DELETE:'), text.index('Old first.'))
        self.assertLess(text.index('Old second.'), text.index(']'))
        self.assertLess(text.index('[INSERT:'), text.index('New first.'))
        self.assertIn('New second.\n]', text)
        self.assertTrue(text.endswith('Ordinary continuation.\n'))

    def test_nested_annotations_close_at_their_own_boundaries(self):
        text = render('<div class="del"><p>Old <span>plain <b>bold</b></span>'
                      '<ins>inserted <i>nested</i></ins> tail.</p></div>'
                      '<p>Outside.</p>').text()
        self.assertIn('Old plain bold[INSERT: inserted nested] tail.', text)
        self.assertTrue(text.endswith(']\nOutside.\n') or text.endswith(']\n\nOutside.\n'))
        self.assertEqual(text.count('[DELETE:'), 1)
        self.assertEqual(text.count('[INSERT:'), 1)

    def test_existing_del_ins_and_span_classes_remain_annotated(self):
        text = render('<p><del>old</del> <ins>new</ins> '
                      '<span class="extra del">before <span>plain</span></span> '
                      '<span class="ins extra">after</span></p>').text()
        self.assertEqual(text, '[DELETE: old] [INSERT: new] '
                         '[DELETE: before plain] [INSERT: after]\n')

    def test_p0135_explicit_standard_wording_classes_are_preserved(self):
        text = render('<blockquote class="stddel"><p>Old rule.</p></blockquote>'
                      '<blockquote class="stdins"><p>Replacement rule.</p></blockquote>').text()
        self.assertIn('[DELETE:', text)
        self.assertIn('Old rule.\n]', text)
        self.assertIn('[INSERT:', text)
        self.assertIn('Replacement rule.\n]', text)

    def test_duplicate_class_and_tag_annotation_does_not_duplicate_markers(self):
        parser = render('<del class="del">old</del><ins class="ins">new</ins>')
        self.assertEqual(parser.text(), '[DELETE: old][INSERT: new]\n')
        self.assertEqual(parser.marker_counts, {'DELETE': 1, 'INSERT': 1})

    def test_conflicting_annotations_are_explicit_errors(self):
        for source in ('<s class="ins">old</s>', '<span class="del ins">bad</span>'):
            with self.subTest(source=source), self.assertRaisesRegex(ValueError, 'conflicting'):
                render(source)

    def test_omitted_paragraph_end_does_not_insert_later_ordinary_text(self):
        text = render('<p class="ins">Inserted paragraph.<p>Ordinary paragraph.').text()
        self.assertIn('[INSERT: Inserted paragraph.]', text)
        self.assertTrue(text.endswith('Ordinary paragraph.\n'))
        self.assertNotIn('Ordinary paragraph.]', text)

    def test_preformatted_indentation_blank_lines_and_long_code_are_preserved(self):
        code = '  int first;\n\n\n    int nested;\n' + '  ' + 'x' * 100 + ';\n'
        self.assertEqual(render('<pre>' + code + '</pre>').text(), code)

    def test_inserted_pre_block_keeps_code_layout_inside_markers(self):
        text = render('<pre class="ins">\n  int x;\n    return x;\n</pre>').text()
        self.assertIn('[INSERT:', text)
        self.assertIn('\n  int x;\n    return x;\n', text)
        self.assertTrue(text.endswith(']\n'))

    def test_optional_list_endings_preserve_annotation_boundaries(self):
        text = render('<ul><li class="del">Old item<li>Ordinary item</ul>').text()
        self.assertIn('[DELETE: Old item]', text)
        self.assertNotIn('Ordinary item]', text)

    def test_head_script_and_style_are_not_paper_text(self):
        text = render('<html><head><title>Ignore</title><style>.del{}</style></head>'
                      '<body><script>Ignore()</script><p>Visible &lt;word&gt;.</p>'
                      '</body></html>').text()
        self.assertEqual(text, 'Visible <word>.\n')

    def test_source_link_footnotes_are_stable_and_unique(self):
        parser = render('<p><a href="wording.html">Wording</a> '
                        '<a href="wording.html">again</a> <a href="#local">local</a></p>')
        self.assertEqual(parser.text(), 'Wording [1] again [1] local\n')
        self.assertEqual(parser.links, ['wording.html'])

    def test_eof_closes_an_open_change_region_without_discarding_text(self):
        text = render('<div class="del"><p>Historical text.').text()
        self.assertIn('[DELETE:', text)
        self.assertIn('Historical text.', text)
        self.assertTrue(text.endswith(']\n'))

    def test_cli_preserves_changes_and_resolves_source_links(self):
        with tempfile.TemporaryDirectory() as directory:
            source, output = Path(directory) / 'paper.html', Path(directory) / 'paper.txt'
            source.write_text('<p><s>old</s><u>new</u> <a href="next.html">next</a></p>')
            result = subprocess.run([sys.executable, str(ROOT / 'scripts/cwg_html_text.py'),
                                     str(source), '--output', str(output), '--base-url',
                                     'https://example.test/papers/original.html'], capture_output=True)
            self.assertEqual(result.returncode, 0, result.stderr)
            self.assertEqual(output.read_text(), '[DELETE: old][INSERT: new] next [1]\n'
                             '\nSource links:\n[1] https://example.test/papers/next.html\n')


if __name__ == '__main__':
    unittest.main()
