#!/usr/bin/env python3
"""Check the offline draft inventory and combined HTML links without network access."""
import hashlib
from html.parser import HTMLParser
import json
from pathlib import Path
import unittest
from urllib.parse import unquote

ROOT = Path(__file__).resolve().parents[2]


class DraftHTML(HTMLParser):
    def __init__(self):
        super().__init__(convert_charrefs=True)
        self.ids = []
        self.links = []
        self.assets = []

    def handle_starttag(self, tag, attrs):
        attrs = dict(attrs)
        if 'id' in attrs:
            self.ids.append(attrs['id'])
        if tag == 'a' and 'href' in attrs:
            self.links.append(attrs['href'])
        if 'src' in attrs:
            self.assets.append(attrs['src'])


class OfflineDraftTests(unittest.TestCase):
    def test_pinned_drafts_and_assets_match_the_export_inventory(self):
        data = json.loads((ROOT / 'doc/language-drafts.json').read_text())
        self.assertEqual([row['draft'] for row in data['references']], ['N4140', 'N4659'])
        inventory = set((ROOT / 'scripts/student_export_documents.txt').read_text().splitlines())
        for reference in data['references']:
            self.assertTrue(reference['source'].startswith('https://'))
            for name, expected in reference['files'].items():
                with self.subTest(file=name):
                    path = ROOT / 'doc' / name
                    self.assertIn('doc/' + name, inventory)
                    self.assertEqual(hashlib.sha256(path.read_bytes()).hexdigest(), expected)

    def test_combined_html_has_complete_local_links_and_images(self):
        document = DraftHTML()
        document.feed((ROOT / 'doc/n4140.html').read_text())
        ids = set(document.ids)
        self.assertEqual(len(document.ids), len(ids))
        self.assertGreater(len(ids), 10000)
        for link in document.links:
            if link.startswith('#'):
                self.assertIn(unquote(link[1:]), ids, link)
        self.assertTrue(document.assets)
        for path in document.assets:
            self.assertTrue((ROOT / 'doc' / path).is_file(), path)


if __name__ == '__main__':
    unittest.main()
