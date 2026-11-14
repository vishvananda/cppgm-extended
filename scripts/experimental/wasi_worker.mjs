import {File, Directory, encoder, hash, fetchBytes, equal, insert, lookup, invoke, loadCompiler} from './wasi_runtime.mjs';

self.onmessage = async ({data: options}) => {
  const started = performance.now();
  try {
    const {manifest, expectedManifest, root, seedBytes, module: seedModule} = await loadCompiler();
    let module = seedModule;
    postMessage({kind: 'ready', revision: manifest.revision, manifest: expectedManifest,
                 dirty: manifest.dirty, sourceSha256: manifest.source_sha256,
                 sources: manifest.sources, seedBytes: seedBytes.length,
                 seedSha256: await hash(seedBytes)});
    const records = [];
    let peakMemoryBytes = 0;
    for (const stage of [2, 3]) {
      root.contents.set(`w${stage}`, new Directory([]));
      const rows = [];
      const stageStart = performance.now();
      for (let i = 0; i <= manifest.sources.length; ++i) {
        const linking = i === manifest.sources.length;
        const name = linking ? 'cppgm.wasm' : manifest.sources[i].replaceAll('/', '__') + '.o';
        const output = `w${stage}/${name}`;
        const inputs = linking
          ? manifest.sources.map(s => `w${stage}/` + s.replaceAll('/', '__') + '.o').concat(manifest.link_flags)
          : ['-c', manifest.sources[i]];
        postMessage({kind: 'progress', stage, index: i, total: manifest.sources.length + 1,
                     name: linking ? 'link compiler' : manifest.sources[i]});
        const before = performance.now();
        const result = await invoke(module, root, [...manifest.flags, ...inputs, '-o', output]);
        peakMemoryBytes = Math.max(peakMemoryBytes, result.memoryBytes);
        if (result.status !== 0) throw Error(`${output}: exit ${result.status}\n${result.log.join('\n')}`);
        const actual = lookup(root, output)?.data;
        if (!actual) throw Error(`${output}: compiler produced no output`);
        const expected = stage === 2 ? await fetchBytes(`inputs/w1/${name}`) : lookup(root, `w2/${name}`).data;
        equal(expected, actual, output);
        rows.push({output, bytes: actual.length, sha256: await hash(actual),
                   seconds: (performance.now() - before) / 1000, memoryBytes: result.memoryBytes,
                   diagnostics: result.log});
        postMessage({kind: 'unit', stage, index: i, total: manifest.sources.length + 1,
                     name: linking ? 'link compiler' : manifest.sources[i], ...rows.at(-1)});
      }
      records.push({stage, seconds: (performance.now() - stageStart) / 1000, commands: rows});
      module = await WebAssembly.compile(lookup(root, `w${stage}/cppgm.wasm`).data);
      postMessage({kind: 'stage', stage, seconds: records.at(-1).seconds, peakMemoryBytes});
    }
    const x86Checks = [];
    for (const level of [0, 2]) {
      postMessage({kind: 'checking', name: `x86 floating corpus at -O${level}`});
      const output = `floating-O${level}.o`;
      const result = await invoke(module, root,
        [...manifest.x86_flags, `-O${level}`, '-c', 'checks/floating.cpp', '-o', output]);
      if (result.status !== 0) throw Error(`x86 corpus failed: ${result.log.join('\n')}`);
      const expected = await fetchBytes(`inputs/x86-check/checks__floating.cpp.o.O${level}.native.o`);
      const actual = lookup(root, output).data;
      equal(expected, actual, output);
      x86Checks.push({level, bytes: actual.length, sha256: await hash(actual), ...result});
      peakMemoryBytes = Math.max(peakMemoryBytes, result.memoryBytes);
    }
    // A fresh instance of the final compiler checks both diagnostics and its
    // x86 backend. This also verifies that the linked fixed point executes.
    const smoke = 'extern "C" long long answer(long long x){return x*7+3;}\n' +
                  'long double exact=0x1.0000000000000002p0L;\nlong double rounding=1.0L/3.0L;\n';
    insert(root, 'accepted.cpp', new File(encoder.encode(smoke)));
    insert(root, 'rejected.cpp', new File(encoder.encode('int broken=missing_name;\n')));
    const accepted = await invoke(module, root, ['-c', 'accepted.cpp', '-o', 'accepted.o']);
    const rejected = await invoke(module, root, ['-c', 'rejected.cpp', '-o', 'rejected.o']);
    if (accepted.status !== 0 || rejected.status === 0) throw Error('final compiler smoke failed');
    const finalModule = lookup(root, 'w3/cppgm.wasm').data;
    postMessage({kind: 'complete', manifest: expectedManifest, userAgent: navigator.userAgent,
                 seconds: (performance.now() - started) / 1000, peakMemoryBytes, stages: records,
                 accepted, rejected, x86Checks, module: finalModule}, [finalModule.buffer]);
  } catch (error) {
    postMessage({kind: 'error', error: String(error), stack: error.stack,
                 seconds: (performance.now() - started) / 1000});
  }
};
