import subprocess, sys, os, re, shutil, time
S=os.environ.get('S', '/tmp/backend-review'); ROOT=os.environ.get('ROOT', os.getcwd())
MUT = {
 'M1-pool-order': [
   ('dev/src/native/lowering/function.cpp',
    'static const X64Register ordinary[] = {XR_R8, XR_R9, XR_RBX, XR_R12, XR_R13, XR_R14, XR_R15};\n    static const X64Register preserved[] = {XR_RBX, XR_R12, XR_R13, XR_R14, XR_R15};',
    'static const X64Register ordinary[] = {XR_R8, XR_R9, XR_R15, XR_R14, XR_R13, XR_R12, XR_RBX};\n    static const X64Register preserved[] = {XR_R15, XR_R14, XR_R13, XR_R12, XR_RBX};'),
   ('dev/src/native/allocation/location_planning.cpp',
    '{XR_R13, XR_R12, XR_RBX, XR_R14, XR_R15};',
    '{XR_R15, XR_R14, XR_RBX, XR_R12, XR_R13};'),
 ],
 'M2-size-cap-41': [
   ('dev/src/lowir/optimize/inline_o1.cpp', 'const std::size_t kOrdinarySizeCap = 40;', 'const std::size_t kOrdinarySizeCap = 41;'),
 ],
 'M4-frame-padding': [
   ('dev/src/native/frame/layout.h',
    '  frame_bytes = selection::align_up(frame_bytes, type.alignment);\n  frame_bytes += abi::frame_storage_size(type);',
    '  if(frame_bytes == 0) frame_bytes = 16;\n  frame_bytes = selection::align_up(frame_bytes, type.alignment);\n  frame_bytes += abi::frame_storage_size(type);'),
 ],
 'M3-always-rpo': [
   ('dev/src/lowir/optimize/boolean_cfg.cpp', '    if(!violated) return false;\n  }\n  std::vector<std::size_t> index(function->next_block_id, kNoBlockIndex);', '    (void)violated;\n  }\n  std::vector<std::size_t> index(function->next_block_id, kNoBlockIndex);'),
 ],
}
def run(cmd, log):
    with open(log,'a') as f: return subprocess.call(cmd, shell=True, stdout=f, stderr=subprocess.STDOUT, cwd=ROOT)
def suites(tag):
    out=[]
    for pa in ['pa29','pa37','pa38']:
        try: os.remove(f'{ROOT}/.test_counts')
        except FileNotFoundError: pass
        log=f'{S}/audit/{tag}-{pa}.log'
        open(log,'w').close()
        rc=run(f'KEEP_GOING=1 make -C {pa} test', log)
        counts=open(f'{ROOT}/.test_counts').read().strip().replace('\n',';') if os.path.exists(f'{ROOT}/.test_counts') else ''
        errs=sum(1 for l in open(log) if 'ERROR' in l or ': FAIL' in l or 'incorrectly' in l or 'did not' in l)
        out.append(f'{pa} rc={rc} counts=[{counts}] errlines={errs}')
    return out
which = sys.argv[1:] or list(MUT)
for tag in which:
    files=[]
    for path,old,new in MUT[tag]:
        p=f'{ROOT}/{path}'; s=open(p).read()
        assert s.count(old)==1, (tag,path,s.count(old))
        open(p,'w').write(s.replace(old,new)); files.append(path)
    t=time.time()
    rc=run('make -C dev -j32', f'{S}/audit/{tag}-build.log')
    res = suites(tag) if rc==0 else [f'BUILD FAILED rc={rc}']
    subprocess.call(['git','checkout','--']+files, cwd=ROOT)
    with open(f'{S}/audit/mutations.txt','a') as f:
        f.write(f'== {tag} ({int(time.time()-t)} s)\n' + '\n'.join(res) + '\n')
subprocess.call('make -C dev -j32 > /dev/null 2>&1', shell=True, cwd=ROOT)
open(f'{S}/audit/mutations.done','w').write('done\n')
