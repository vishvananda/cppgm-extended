"""Source adapters with explicit input/output identity, shared by both compilers."""
import hashlib
import re


def project_source(source, kind):
    if kind != 'gnu-line-markers':
        raise ValueError('Unknown source projection: ' + kind)
    marker = re.compile(r'^[ \t]*#[ \t]*([0-9]+)[ \t]+("(?:[^"\\]|\\.)*")((?:[ \t]+[1-4])*)[ \t]*$')
    lines, changed = [], False
    for line in source.decode('utf-8').splitlines(keepends=True):
        match = marker.fullmatch(line.rstrip('\r\n'))
        if match:
            lines.append('#line ' + match[1] + ' ' + match[2] + ('\n' if line.endswith('\n') else ''))
            changed = True
        else:
            if re.match(r'^[ \t]*#[ \t]*[0-9]', line):
                raise ValueError('Unreviewed GNU line-marker syntax')
            lines.append(line)
    if not changed:
        raise ValueError('Source projection changed no line markers')
    return ''.join(lines).encode('utf-8')


def projection_identity(source, kind):
    return dict(kind=kind, sha256=hashlib.sha256(project_source(source, kind)).hexdigest())
