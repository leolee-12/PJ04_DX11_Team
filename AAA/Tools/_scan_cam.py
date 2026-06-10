import struct, os, glob
from collections import Counter, defaultdict
import camera_obj_converter as C

ROOT = r"C:/Users/mabll/OneDrive/바탕 화면/decompressed/ubin/Step/Map"
files = glob.glob(os.path.join(ROOT, "**", "Camera_Obj.bin"), recursive=True)

mode = Counter()
shape = Counter()
erp = Counter()
panrange = Counter()
panuse = Counter()
userail = Counter()
eyesnap = Counter()
tgtsnap = Counter()
dofuse = Counter()
evttrig = Counter()
areacam_mode_examples = defaultdict(list)

# rotation non-identity tracking
def is_ident(q):
    # identity quat ~ (0,0,0,±1)
    if q is None: return True
    x,y,z,w = q
    return abs(x)<1e-4 and abs(y)<1e-4 and abs(z)<1e-4 and abs(abs(w)-1)<1e-4

snaprot_nonident = 0
eyesnaprot_nonident = 0
arearot_nonident = 0
snaprot_examples = []
eyesnaprot_examples = []
arearot_examples = []

# eyeSnapRate / snapRate non-zero axis stats
eyesnaprate_nonzero = 0
total_cams = 0

n_files = 0
n_cams = 0
n_rails = 0
for f in files:
    try:
        parsed = C.convert_file(f)
    except Exception as e:
        print("FAIL", f, e); continue
    n_files += 1
    rel = os.path.relpath(f, ROOT)
    for _, r in parsed.get("Custom", {}).items():
        n_rails += 1
        erp[r.get("ErpType","?")] += 1
    for _, c in parsed.get("Standard", {}).items():
        mc = c.get("Gimmick.AreaCamera.MainComponent")
        if not mc: continue
        n_cams += 1; total_cams += 1
        m = mc.get("AreaCameraMode","?")
        mode[m] += 1
        if len(areacam_mode_examples[m]) < 3 and m != "Normal":
            areacam_mode_examples[m].append(rel)
        shape[mc.get("AreaShapeKind","?")] += 1
        userail[mc.get("UseRail")] += 1
        eyesnap[mc.get("IsEyeSnap")] += 1
        tgtsnap[mc.get("IsTargetSnap")] += 1
        dofuse[mc.get("UseDofParam")] += 1
        evttrig[mc.get("UseEventTrigger")] += 1
        panuse[mc.get("UsePanLimit")] += 1
        pr = mc.get("PanLimitRange", 360.0)
        panrange[round(pr,1)] += 1
        # area rot
        ar = mc.get("AreaRot")
        if ar and not is_ident(ar):
            arearot_nonident += 1
            if len(arearot_examples)<5: arearot_examples.append((rel, ar))
        # offset params rot
        for key in ("Gimmick.AreaCamera.CameraOffsetParamBase","Gimmick.AreaCamera.CameraOffsetParamEnd"):
            op = c.get(key)
            if not op: continue
            sr = op.get("SnapRot")
            if sr and not is_ident(sr):
                snaprot_nonident += 1
                if len(snaprot_examples)<5: snaprot_examples.append((rel,key[-4:],sr))
            er = op.get("EyeSnapRot")
            if er and not is_ident(er):
                eyesnaprot_nonident += 1
                if len(eyesnaprot_examples)<5: eyesnaprot_examples.append((rel,key[-4:],er))
            esr = op.get("EyeSnapRate")
            if esr and any(abs(v)>1e-4 for v in esr):
                eyesnaprate_nonzero += 1

print(f"파일 {n_files}개 / 카메라 {n_cams}개 / 레일 {n_rails}개\n")
print("=== AreaCameraMode ===");        [print(f"  {k:20} {v}") for k,v in mode.most_common()]
for k,ex in areacam_mode_examples.items():
    if k!="Normal": print(f"    └ {k} 예시: {ex}")
print("\n=== AreaShapeKind ===");        [print(f"  {k:20} {v}") for k,v in shape.most_common()]
print("\n=== Rail ErpType ===");         [print(f"  {k:20} {v}") for k,v in erp.most_common()]
print("\n=== UseRail ===");              [print(f"  {k}: {v}") for k,v in userail.items()]
print("=== IsEyeSnap ===");             [print(f"  {k}: {v}") for k,v in eyesnap.items()]
print("=== IsTargetSnap ===");          [print(f"  {k}: {v}") for k,v in tgtsnap.items()]
print("=== UsePanLimit ===");           [print(f"  {k}: {v}") for k,v in panuse.items()]
print("=== UseDofParam ===");           [print(f"  {k}: {v}") for k,v in dofuse.items()]
print("=== UseEventTrigger ===");       [print(f"  {k}: {v}") for k,v in evttrig.items()]
print("\n=== PanLimitRange 분포 (상위) ===")
for k,v in sorted(panrange.items(), key=lambda x:-x[1])[:15]:
    print(f"  range={k:7}  {v}")
print(f"\n=== 회전 non-identity 카운트 ===")
print(f"  SnapRot    non-ident: {snaprot_nonident}")
print(f"  EyeSnapRot non-ident: {eyesnaprot_nonident}")
print(f"  AreaRot    non-ident: {arearot_nonident}")
print(f"  EyeSnapRate non-zero: {eyesnaprate_nonzero}")
if snaprot_examples:    print("  SnapRot 예:");    [print("    ",e) for e in snaprot_examples]
if eyesnaprot_examples: print("  EyeSnapRot 예:"); [print("    ",e) for e in eyesnaprot_examples]
if arearot_examples:    print("  AreaRot 예:");    [print("    ",e) for e in arearot_examples]
