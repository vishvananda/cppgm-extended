import {WASI, File, OpenFile, ConsoleStdout, PreopenDirectory, Directory} from './shim/index.js';

const encoder = new TextEncoder();
const hex = bytes => Array.from(bytes, n => n.toString(16).padStart(2, '0')).join('');
const hash = async bytes => hex(new Uint8Array(await crypto.subtle.digest('SHA-256', bytes)));
async function fetchBytes(path) {
  const response = await fetch(path, {cache: 'no-store'});
  if (!response.ok) throw Error(`${path}: HTTP ${response.status}`);
  return new Uint8Array(await response.arrayBuffer());
}
function equal(a, b, name) {
  const end = Math.min(a.length, b.length);
  let offset = 0;
  while (offset < end && a[offset] === b[offset]) ++offset;
  if (offset !== end || a.length !== b.length)
    throw Error(`${name}: first byte difference at ${offset}, sizes ${a.length}/${b.length}`);
}
function insert(root, path, file) {
  const parts = path.split('/');
  let dir = root;
  for (const part of parts.slice(0, -1)) {
    if (!dir.contents.has(part)) dir.contents.set(part, new Directory([]));
    dir = dir.contents.get(part);
  }
  dir.contents.set(parts.at(-1), file);
}
function lookup(root, path) {
  let node = root;
  for (const part of path.split('/')) node = node?.contents?.get(part);
  return node;
}
function invoke(module, root, args) {
  const log = [];
  const wasi = new WASI(['cppgm++', ...args], [], [new OpenFile(new File([])),
    ConsoleStdout.lineBuffered(s => log.push(s)), ConsoleStdout.lineBuffered(s => log.push(s)),
    new PreopenDirectory('.', root.contents)]);
  return WebAssembly.instantiate(module, {wasi_snapshot_preview1: wasi.wasiImport}).then(instance => {
    const status = wasi.start(instance);
    return {status, log, memoryBytes: instance.exports.memory.buffer.byteLength};
  });
}

self.onmessage = async ({data: options}) => {
  const started = performance.now();
  try {
    const manifestBytes = await fetchBytes('manifest.json');
    const expectedManifest = new TextDecoder().decode(await fetchBytes('manifest.sha256')).trim();
    if (await hash(manifestBytes) !== expectedManifest) throw Error('manifest hash mismatch');
    const manifest = JSON.parse(new TextDecoder().decode(manifestBytes));
    if (manifest.schema !== 1) throw Error('unsupported input manifest');
    for (const [path, expected] of Object.entries(manifest.shim_files))
      if (await hash(await fetchBytes(path)) !== expected) throw Error(`shim changed: ${path}`);
    for (const [path, expected] of Object.entries(manifest.runner_files))
      if (await hash(await fetchBytes(path)) !== expected) throw Error(`runner changed: ${path}`);
    const bundle = await fetchBytes('bundle.bin');
    if (await hash(bundle) !== manifest.bundle_sha256) throw Error('input bundle hash mismatch');
    const root = new Directory([]);
    for (const item of manifest.files) {
      const bytes = bundle.subarray(item.offset, item.offset + item.size);
      if (await hash(bytes) !== item.sha256) throw Error(`input hash mismatch: ${item.path}`);
      insert(root, item.path, new File(bytes, {readonly: true}));
    }
    const seedRecord = JSON.parse(new TextDecoder().decode(await fetchBytes('w1-complete.json')));
    if (seedRecord.manifest !== expectedManifest) throw Error('seed used a different input manifest');
    const seedBytes = await fetchBytes('inputs/w1/cppgm.wasm');
    if (await hash(seedBytes) !== seedRecord.commands.at(-1).sha256) throw Error('seed hash mismatch');
    let module = await WebAssembly.compile(seedBytes);
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
