#!/usr/bin/env python3
"""Private causal comparison of parser phases and compiler-pass ablations."""
from pathlib import Path
import os,json,subprocess,concurrent.futures,sys,time
import benchmark_timing as timing
import simdjson_ci as suite
ROOT=Path(__file__).resolve().parents[1];OUT=ROOT/'obj/phase-analysis';OUT.mkdir(parents=True)
S=OUT/'upstream';suite.fetch(S);suite.source_check(S)
VARIANTS={
 'gcc-default':('g++',[]),
 'gcc-gpr-limited':('g++',['-ffixed-r10','-ffixed-r11']),
 'gcc-xmm-limited':('g++',['-ffixed-xmm14','-ffixed-xmm15']),
 'gcc-all-limited':('g++',['-ffixed-r10','-ffixed-r11','-ffixed-xmm14','-ffixed-xmm15']),
 'gcc-no-scheduling':('g++',['-fno-schedule-insns','-fno-schedule-insns2']),
 'gcc-no-pre-fre':('g++',['-fno-tree-pre','-fno-tree-fre']),
 'gcc-no-sra':('g++',['-fno-tree-sra','-fno-ipa-sra']),
 'gcc-no-loop-motion':('g++',['-fno-tree-loop-im']),
 'clang-default':('clang++-21',[]),
 'clang-basic-ra':('clang++-21',['-mllvm','-regalloc=basic']),
 'clang-no-scheduling':('clang++-21',['-mllvm','-enable-misched=false']),
 'cppgm':(str(ROOT/'dev/cppgm++'),[]),
}
flags=suite.flags(S,'sse');driver=OUT/'driver.o'
subprocess.run(['g++',*flags,'-c',str(ROOT/'scripts/probe/phases.cpp'),'-o',str(driver)],check=True)
metadata=dict(cpu=Path('/proc/cpuinfo').read_text(),revision=subprocess.check_output(['git','rev-parse','HEAD'],text=True).strip(),upstream=suite.COMMIT,variants=VARIANTS,versions={c:subprocess.check_output([c,'--version'],text=True) for c in ['g++','clang++-21',str(ROOT/'dev/cppgm++')]},flags=flags)
suite.save(OUT/'metadata.json',metadata)
def build(item):
 label,(compiler,extra)=item;obj=OUT/(label+'.o');exe=OUT/('phase-'+label);cmd=[compiler,*flags,*extra,'-c',str(S/'src/simdjson.cpp'),'-o',str(obj)]
 suite.save(OUT/(label+'.command.json'),cmd)
 with (OUT/(label+'.log')).open('w') as log:subprocess.run(cmd,stdout=log,stderr=log,check=True)
 subprocess.run(['g++',str(driver),str(obj),'-pthread','-o',str(exe)],check=True)
 print('built',label,flush=True)
with concurrent.futures.ThreadPoolExecutor(max_workers=2) as pool:list(pool.map(build,VARIANTS.items()))
metadata['object_sha256']={k:timing.sha(OUT/(k+'.o')) for k in VARIANTS};suite.save(OUT/'metadata.json',metadata)
# All compilation has finished before any timing. Every phase must retain the
# same canonical document, including after repeated stage2-only calls.
for name in suite.INPUTS:
 expected=None;path=S/'jsonexamples'/name
 for label in VARIANTS:
  for phase in ['full','stage1','stage2']:
   result=subprocess.check_output([str(OUT/('phase-'+label)),str(path),phase,'16','check']);out=OUT/('check-'+label+'-'+name+'-'+phase);out.write_bytes(result)
   if expected is None:expected=result
   assert result==expected,(label,name,phase)
print('All isolated phases agree on canonical JSON',flush=True)
cpu=min(os.sched_getaffinity(0));os.sched_setaffinity(0,{cpu});metadata['cpu_id']=cpu;suite.save(OUT/'metadata.json',metadata)
results={};warnings=[]
for name in suite.INPUTS:
 path=S/'jsonexamples'/name
 for phase in ['full','stage1','stage2']:
  repeats=32
  def sample(label):
   row=json.loads(subprocess.check_output([str(OUT/('phase-'+label)),str(path),phase,str(repeats)],text=True));assert row['iterations']==repeats and row['bytes']==path.stat().st_size;return row
  while sample('gcc-default')['cpu']<.25:repeats*=2
  for label in VARIANTS:
   timing.retain(OUT/'warmups.jsonl',dict(sample(label),input=name,parser_phase=phase,label=label))
   def one(current,calibration,block,position):
    row=sample(current);timing.retain(OUT/'observations.jsonl',dict(row,input=name,parser_phase=phase,candidate=label,label=current,phase=calibration,block=block,position=position));return row
   values={}
   for calibration,a,b,blocks in [('AA-before','gcc-default','gcc-default',2),('AB','gcc-default',label,3),('AA-after','gcc-default','gcc-default',2)]:
    rows=timing.observations(one,a,b,blocks,phase=calibration);values[calibration]={key:suite.summarize(rows,key) for key in ['cpu','wall']}
   key='|'.join([name,phase,label]);results[key]=values
   for calibration in ['AA-before','AA-after']:
    for metric in ['cpu','wall']:
     if abs(values[calibration][metric]['speed']-1)>.02:warnings.append(key+' '+calibration+' '+metric)
   suite.save(OUT/'results.json',dict(comparisons=results,calibration_warnings=warnings));print(key,values['AB']['cpu']['speed'],flush=True)
lines=['# Controlled parser-phase analysis','','Higher is better: GCC default CPU time / variant CPU time. All libraries parse identical JSON. Three ABBA/BAAB blocks with A/A before and after; no observations discarded.','','| Input | Phase | Compiler / ablation | CPU speed vs GCC |','|---|---|---|---:|']
for key,values in results.items():
 name,phase,label=key.split('|');lines.append(f"| {name} | {phase} | {label} | {values['AB']['cpu']['speed']:.4f}× |")
lines+=['','Calibration warnings: '+('; '.join(warnings) if warnings else 'none')];(OUT/'details.md').write_text('\n'.join(lines)+'\n')
