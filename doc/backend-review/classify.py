import re, sys, os, json
NONNORM = {'Design Notes','Design Notes (Non-Normative)','Out Of Scope','Overview','Prerequisites','Starter Kit','Testing','Handoff','Stage Handoff','Build And Test Commands','What PA39 Tests','Working Through Failures','LowIR Family Context','Checkpoint Ownership','Intermediate Ladder','Single-Object Probes','Inception Targets','Build Variables','Standard Output / Error','Standard Output And Error','Command Line','Command-Line Contract','Input / Command-Line Arguments','Driver Surface','Driver Surface For This Assignment','Required Driver Surface','Using PA14 ABI Names','Host ABI Symbol Names','Hosted ABI Names'}
MECH = re.compile(r'\b(budget|cap|caps|planner|interval|pool|phi|phis|coalesc\w*|bypass\w*|thread\w*|remateriali\w*|recolor\w*|hoist\w*|forward\w*|fold\w*|reuse\w*|inlin\w*|spill\w*|frame home|callee-saved|caller-saved|scratch|worklist|dominat\w*|reverse postorder|SCC|strongly connected|dense|adjacency|table|liveness|clone|specializ\w*|group|unroll\w*|prefilter|diamond|merge|split|late wave|post-prune|single-call|leaf)\b', re.I)
NUM = re.compile(r'(?<![\w.])(\d{2,}|[2-9])(?![\w.]|\s*(?:-bit|-byte|-eightbyte|/))')
CONTRACT = re.compile(r'\b(must|shall|required|valid|preserve\w*|format|ABI|section|symbol|relocation|exit status|behavio\w*|deterministic|well-formed|metadata|operand|terminator|alignment|unwind\w*|serializ\w*|grammar|syntax)\b', re.I)
QUAL = re.compile(r'\b(at most|no larger than|no more than|within|fewer than|not exceed|smaller than|larger than)\b', re.I)
def sections(text):
    cur='(preamble)'; out={cur:[]}
    for line in text.splitlines():
        m=re.match(r'^### (.*)$', line)
        if m: cur=m.group(1).strip(); out.setdefault(cur,[]); continue
        out[cur].append(line)
    return out
def sentences(lines):
    # join bullets and paragraphs; split into sentences
    buf=[]; items=[]
    for l in lines:
        if l.startswith('```'): continue
        if re.match(r'^\s*[-*] ', l) or l.strip()=='' :
            if buf: items.append(' '.join(buf)); buf=[]
            if l.strip(): buf=[re.sub(r'^\s*[-*] ','',l).strip()]
        else:
            buf.append(l.strip())
    if buf: items.append(' '.join(buf))
    sents=[]
    for it in items:
        for s in re.split(r'(?<=[.;])\s+(?=[A-Z`(])', it):
            s=s.strip()
            if len(s)>25: sents.append(s)
    return sents
res={}
for pa in sys.argv[1:]:
    text=open(f'{pa}/README.md').read()
    secs=sections(text)
    norm=[]; nonnorm=0
    for name,lines in secs.items():
        if name in NONNORM or name.startswith('After ') or name.startswith('Design'): nonnorm+=len(sentences(lines)); continue
        for s in sentences(lines): norm.append((name,s))
    counts={'mechanism':0,'contract':0,'quality':0,'other':0}
    mech=[]
    for name,s in norm:
        if MECH.search(s) or NUM.search(s):
            counts['mechanism']+=1; mech.append((name,s))
        elif QUAL.search(s): counts['quality']+=1
        elif CONTRACT.search(s): counts['contract']+=1
        else: counts['other']+=1
    res[pa]={'normative_sentences':len(norm),'nonnormative_sentences':nonnorm,'counts':counts,'mechanism_examples':mech[:400]}
    print(f"{pa}: normative={len(norm)} nonnormative={nonnorm} mechanism={counts['mechanism']} contract={counts['contract']} quality={counts['quality']} other={counts['other']}")
json.dump(res, open(os.environ.get('S','/tmp/backend-review')+'/classify.json','w'), indent=1)
