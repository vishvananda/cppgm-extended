import {WASI, File, OpenFile, ConsoleStdout, PreopenDirectory, Directory} from './shim/index.js';
import {fetchBytes} from './transport_bytes.mjs';
export {fetchBytes};

export const encoder = new TextEncoder();
const hex = bytes => Array.from(bytes, n => n.toString(16).padStart(2, '0')).join('');
export const hash = async bytes => hex(new Uint8Array(await crypto.subtle.digest('SHA-256', bytes)));
export function equal(a, b, name) {
  const end = Math.min(a.length, b.length);
  let offset = 0;
  while (offset < end && a[offset] === b[offset]) ++offset;
  if (offset !== end || a.length !== b.length)
    throw Error(`${name}: first byte difference at ${offset}, sizes ${a.length}/${b.length}`);
}
export function insert(root, path, file) {
  const parts = path.split('/');
  let dir = root;
  for (const part of parts.slice(0, -1)) {
    if (!dir.contents.has(part)) dir.contents.set(part, new Directory([]));
    dir = dir.contents.get(part);
  }
  dir.contents.set(parts.at(-1), file);
}
export function lookup(root, path) {
  let node = root;
  for (const part of path.split('/')) node = node?.contents?.get(part);
  return node;
}
export function invoke(module, root, args) {
  const log = [];
  const wasi = new WASI(['cppgm++', ...args], [], [new OpenFile(new File([])),
    ConsoleStdout.lineBuffered(s => log.push(s)), ConsoleStdout.lineBuffered(s => log.push(s)),
    new PreopenDirectory('.', root.contents)], {debug: false});
  return WebAssembly.instantiate(module, {wasi_snapshot_preview1: wasi.wasiImport}).then(instance => {
    const status = wasi.start(instance);
    return {status, log, memoryBytes: instance.exports.memory.buffer.byteLength};
  });
}

export {File, Directory};

export async function loadCompiler(prefix = '') {
    const manifestBytes = await fetchBytes(prefix + 'manifest.json');
    const expectedManifest = new TextDecoder().decode(await fetchBytes(prefix + 'manifest.sha256')).trim();
    if (await hash(manifestBytes) !== expectedManifest) throw Error('manifest hash mismatch');
    const manifest = JSON.parse(new TextDecoder().decode(manifestBytes));
    if (manifest.schema !== 1) throw Error('unsupported input manifest');
    for (const [path, expected] of Object.entries(manifest.shim_files))
      if (await hash(await fetchBytes(prefix + path)) !== expected) throw Error(`shim changed: ${path}`);
    for (const [path, expected] of Object.entries(manifest.runner_files))
      if (await hash(await fetchBytes(prefix + path)) !== expected) throw Error(`runner changed: ${path}`);
    const bundle = await fetchBytes(prefix + 'bundle.bin');
    if (await hash(bundle) !== manifest.bundle_sha256) throw Error('input bundle hash mismatch');
    const root = new Directory([]);
    for (const item of manifest.files) {
      const bytes = bundle.subarray(item.offset, item.offset + item.size);
      if (await hash(bytes) !== item.sha256) throw Error(`input hash mismatch: ${item.path}`);
      insert(root, item.path, new File(bytes, {readonly: true}));
    }
    const seedRecord = JSON.parse(new TextDecoder().decode(await fetchBytes(prefix + 'w1-complete.json')));
    if (seedRecord.manifest !== expectedManifest) throw Error('seed used a different input manifest');
    const seedBytes = await fetchBytes(prefix + 'inputs/w1/cppgm.wasm');
    if (await hash(seedBytes) !== seedRecord.commands.at(-1).sha256) throw Error('seed hash mismatch');
    const module = await WebAssembly.compile(seedBytes);
    return {manifest, expectedManifest, root, seedBytes, module};
}
