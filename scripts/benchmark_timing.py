"""Paired timing and immutable compiler rebuilds for public benchmarks."""
import hashlib
import json
import math
from pathlib import Path
import shutil
import statistics


def sha(path):
    digest = hashlib.sha256()
    with Path(path).open('rb') as stream:
        for chunk in iter(lambda: stream.read(1024 * 1024), b''):
            digest.update(chunk)
    return digest.hexdigest()


def observations(run, reference, candidate, blocks, phase='AB'):
    rows = []
    for block in range(blocks):
        order = 'ABBA' if block % 2 == 0 else 'BAAB'
        for position, side in enumerate(order):
            label = reference if side == 'A' else candidate
            row = run(label, phase, block, position)
            if not math.isfinite(row['cpu']) or row['cpu'] < 0:
                raise ValueError('invalid CPU time')
            rows.append(dict(row, label=label, side=side, phase=phase,
                             block=block, position=position))
    return rows


def summarize(rows):
    """Use neighboring block means, never a ratio of independent minima."""
    pairs = []
    for block in sorted({row['block'] for row in rows}):
        group = [row for row in rows if row['block'] == block]
        reference = statistics.mean(row['cpu'] for row in group if row['side'] == 'A')
        candidate = statistics.mean(row['cpu'] for row in group if row['side'] == 'B')
        pairs.append(dict(reference_cpu=reference, candidate_cpu=candidate,
                          ratio=candidate / reference if reference > 0 else None))
    if not pairs:
        raise ValueError('no paired observations')
    ratios = [pair['ratio'] for pair in pairs]
    return dict(ratio=statistics.median(ratios) if all(r is not None for r in ratios) else None,
                reference_cpu=statistics.median(p['reference_cpu'] for p in pairs),
                candidate_cpu=statistics.median(p['candidate_cpu'] for p in pairs),
                blocks=pairs)


def retain(path, row):
    """Keep each observation even if a later check or measurement fails."""
    with Path(path).open('a') as stream:
        stream.write(json.dumps(row, sort_keys=True) + '\n')


class Rebuild:
    """Recompile identical PA34 inputs with two fixed compiler binaries."""
    def __init__(self, root, directory, jobs, seed, self_built, measure):
        self.root, self.directory = Path(root), Path(directory)
        self.jobs, self.measure = jobs, measure
        self.binaries = {}
        for label, source in [('seed', seed), ('self', self_built)]:
            target = self.directory / label
            shutil.copy2(source, target)
            target.chmod(0o555)
            self.binaries[label] = target
        self.hashes = {label: sha(path) for label, path in self.binaries.items()}
        self.build = self.directory / 'pa34'
        self.expected = None
        self.objects = None
        self.samples = []

    def run(self, label, phase, block, position):
        # This is a private, newly created measurement directory. Preserve its
        # generated host config and use identical paths/flags on every side.
        objects = self.build / 'selfhost'
        if objects.exists():
            shutil.rmtree(objects)
        command = ['make', '-C', 'pa34', f'-j{self.jobs}', 'cppgm++-self',
                   f'CXX={self.binaries[label]}', 'CPPGM_HOST_CXX=g++',
                   f'INCEPTION_OBJ_ROOT_BASE={self.build}',
                   'INCEPTION_COMPILER_FLAVOR=selfhost',
                   f'INCEPTION_BUILD_JOBS={self.jobs}',
                   f'INCEPTION_OBJECT_BUILD_JOBS={self.jobs}',
                   'INCEPTION_BUILD_MISSING_FIRST=0', 'INCEPTION_COMPARE_OBJECTS=0']
        log = self.directory / f'{len(self.samples):03}-{phase}-{label}.log'
        with log.open('wb') as output:
            result = self.measure(command, self.root, 900, log.with_suffix('.stderr'),
                                  stdout=output)
        row = dict(result, command=command, label=label, phase=phase,
                   block=block, position=position)
        self.samples.append(row)
        retain(self.directory / 'observations.jsonl', row)
        if result['timed_out'] or result['code']:
            raise RuntimeError(f'paired rebuild failed: {log}')
        output = self.build / 'bin/selfhost/cppgm++-self'
        digest = sha(output)
        produced = {str(p.relative_to(objects)): sha(p) for p in objects.rglob('*.o')}
        row.update(output_sha256=digest, object_sha256=produced)
        retain(self.directory / 'outputs.jsonl', row)
        if not produced:
            raise RuntimeError('paired rebuild produced no compiler objects')
        if self.expected is not None and (digest != self.expected or produced != self.objects):
            raise RuntimeError('paired rebuild changed compiler output')
        self.expected, self.objects = digest, produced
        if self.hashes != {label: sha(path) for label, path in self.binaries.items()}:
            raise RuntimeError('a measured compiler binary changed')
        # Retain hashes and receipts, without archiving repeated object trees.
        shutil.rmtree(objects)
        return row
