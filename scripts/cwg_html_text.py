#!/usr/bin/env python3
"""Render offline WG21 HTML as text, preserving historical change markup."""
import argparse
from html.parser import HTMLParser
from pathlib import Path
import re
import textwrap
from urllib.parse import urljoin


class HtmlText(HTMLParser):
    """Text and numbered source links; annotations never apply a proposed edit."""

    BLOCKS = {'p', 'h1', 'h2', 'h3', 'h4', 'h5', 'blockquote', 'ul', 'ol',
              'dl', 'table', 'tr', 'div'}
    VOID = {'area', 'base', 'br', 'col', 'embed', 'hr', 'img', 'input', 'link',
            'meta', 'param', 'source', 'track', 'wbr'}
    PRE_START, PRE_END = '\x00PRE_START\x00', '\x00PRE_END\x00'

    def __init__(self, width=88):
        super().__init__(convert_charrefs=True)
        self.width = width
        self.out, self.frames, self.lists, self.links = [], [], [], []
        self.body, self.skip, self.pre = True, 0, 0
        self.marker_counts = {'DELETE': 0, 'INSERT': 0}

    def newline(self):
        self.out.append('\n')

    def finish_frame(self, frame):
        tag, marker, href = frame
        if tag == 'a' and href and not href.startswith('#'):
            href = href.strip()
            if href not in self.links:
                self.links.append(href)
            self.out.append(' [' + str(self.links.index(href) + 1) + ']')
        if tag == 'pre':
            self.pre -= 1
            self.out.append(self.PRE_END)
        if marker:
            self.out.append(']')
        if tag in {'ul', 'ol'}:
            self.lists.pop()
        if tag in self.BLOCKS | {'li', 'dt', 'dd', 'pre'}:
            self.newline()

    def close_through(self, index):
        while len(self.frames) > index:
            self.finish_frame(self.frames.pop())

    def implicit_end(self, tag):
        # HTML permits omitted p/li/dt/dd endings. In particular, an inserted
        # paragraph must end before the following ordinary paragraph begins.
        if tag in self.BLOCKS | {'pre', 'hr'}:
            for i in range(len(self.frames) - 1, -1, -1):
                if self.frames[i][0] == 'p':
                    self.close_through(i)
                    break
        scopes = {'li': ({'li'}, {'ul', 'ol'}),
                  'dt': ({'dt', 'dd'}, {'dl'}),
                  'dd': ({'dt', 'dd'}, {'dl'}),
                  'tr': ({'tr'}, {'table'}),
                  'td': ({'td', 'th'}, {'tr'}),
                  'th': ({'td', 'th'}, {'tr'})}
        if tag in scopes:
            endings, barriers = scopes[tag]
            for i in range(len(self.frames) - 1, -1, -1):
                previous = self.frames[i][0]
                if previous in barriers:
                    break
                if previous in endings:
                    self.close_through(i)
                    break

    @staticmethod
    def annotation(tag, attributes):
        # P0135R1 explicitly defines these blockquote classes with the same
        # underline/line-through semantics as ins/del; do not infer arbitrary CSS.
        aliases = {'stddel': 'del', 'stdins': 'ins'}
        kinds = {aliases.get(c, c) for c in attributes.get('class', '').lower().split()}
        kinds &= {'del', 'ins'}
        if tag in {'del', 's'}:
            kinds.add('del')
        if tag in {'ins', 'u'}:
            kinds.add('ins')
        if len(kinds) > 1:
            raise ValueError('conflicting deletion/insertion annotation on ' + tag)
        return 'DELETE' if 'del' in kinds else 'INSERT' if 'ins' in kinds else None

    def handle_starttag(self, tag, attrs):
        attributes = dict(attrs)
        if tag == 'body':
            self.body, self.skip = True, 0
        if not self.body:
            return
        if tag in {'head', 'script', 'style'}:
            self.skip += 1
            return
        if self.skip:
            return
        self.implicit_end(tag)
        if tag in self.BLOCKS:
            self.newline()
            self.newline()
        if tag in {'ul', 'ol'}:
            self.lists.append(0 if tag == 'ol' else None)
        if tag in {'li', 'dt', 'dd'}:
            self.newline()
            prefix = '  '
            if tag == 'li':
                if self.lists and self.lists[-1] is not None:
                    self.lists[-1] += 1
                    prefix += str(self.lists[-1]) + '. '
                else:
                    prefix += '- '
            self.out.append(prefix)
        if tag in {'br', 'hr', 'pre'}:
            self.newline()
        if tag == 'td':
            self.out.append(' | ')
        marker = self.annotation(tag, attributes)
        if marker:
            self.out.append('[' + marker + ': ')
            self.marker_counts[marker] += 1
        if tag == 'pre':
            self.out.append(self.PRE_START)
            self.pre += 1
        if tag not in self.VOID:
            self.frames.append((tag, marker, attributes.get('href') if tag == 'a' else None))
        elif marker:
            self.out.append(']')

    def handle_endtag(self, tag):
        if tag in {'head', 'style', 'script'} and self.skip:
            self.skip -= 1
            return
        if not self.body or self.skip:
            return
        for i in range(len(self.frames) - 1, -1, -1):
            if self.frames[i][0] == tag:
                self.close_through(i)
                break
        if tag == 'body':
            self.body = False

    def handle_data(self, data):
        if self.body and not self.skip:
            self.out.append(data if self.pre else re.sub(r'\s+', ' ', data))

    def close(self):
        super().close()
        self.close_through(0)

    def text(self):
        parts = re.split('(' + self.PRE_START + '|' + self.PRE_END + ')', ''.join(self.out))
        out, inside = [], False
        for part in parts:
            if part == self.PRE_START:
                inside = True
            elif part == self.PRE_END:
                inside = False
            elif inside:
                out.append(part.replace('\xa0', ' '))
            else:
                part = re.sub(r' *\n *\n(?: *\n)+', '\n\n', part.replace('\xa0', ' '))
                out.append('\n'.join(textwrap.fill(line.strip(), width=self.width,
                                                  break_long_words=False, break_on_hyphens=False)
                                     if line.strip() else '' for line in part.split('\n')))
        return ''.join(out).strip('\n') + '\n'


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('source', type=Path)
    parser.add_argument('--output', type=Path, required=True)
    parser.add_argument('--base-url', default='')
    args = parser.parse_args()
    renderer = HtmlText()
    renderer.feed(args.source.read_text())
    renderer.close()
    text = renderer.text()
    if renderer.links:
        text += '\nSource links:\n' + ''.join(
            f'[{i}] {urljoin(args.base_url, href)}\n' for i, href in enumerate(renderer.links, 1))
    args.output.write_text(text)


if __name__ == '__main__':
    main()
