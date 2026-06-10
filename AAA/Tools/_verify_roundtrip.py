# Verifies the C++ CAreaCameraSolver Load->Save round-trip (simulated in Python)
# against the original binary full-dump. Lossless == every field preserved.
import sys, os, glob, copy
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import camera_obj_converter as C   # convert_file = binary parser (ground truth)

# ---- involution transforms (mirror C++ RV3/WV3/RQ4/WQ4) ----
def v3(a):  return [a[0], a[1], -a[2]]          # negate z (own inverse)
def q4(a):  return [-a[0], -a[1], a[2], a[3]]   # negate x,y (own inverse)
def raw(a): return [a[0], a[1], a[2]]

def rd_offset(o):
    if not isinstance(o, dict): return {}
    return dict(
        targetOffs=v3(o.get("TargetOffs", [0, 0, 0])), snapCenter=v3(o.get("SnapCenter", [0, 0, 0])),
        snapRate=raw(o.get("SnapRate", [1, 1, 1])), snapRot=q4(o.get("SnapRot", [0, 0, 0, 1])),
        eyeSnapCenter=v3(o.get("EyeSnapCenter", [0, 0, 0])), eyeSnapRate=raw(o.get("EyeSnapRate", [0, 0, 0])),
        eyeSnapRot=q4(o.get("EyeSnapRot", [0, 0, 0, 1])),
        dist=o.get("DistOffs", 0.0), tilt=-o.get("TiltOffs", 0.0), rotY=-o.get("RotYOffs", 0.0),
        roll=-o.get("RollOffs", 0.0), fovY=o.get("FovYOffs", 0.0),
        dofFarDepth=o.get("DofFarDepthOffs", 0.0), dofFarRate=o.get("DofFarRateOffs", 0.0))

def wr_offset(p, o):  # write into existing node dict (preserve unknown keys)
    o["TargetOffs"] = v3(p["targetOffs"]); o["SnapCenter"] = v3(p["snapCenter"])
    o["SnapRate"] = raw(p["snapRate"]); o["SnapRot"] = q4(p["snapRot"])
    o["EyeSnapCenter"] = v3(p["eyeSnapCenter"]); o["EyeSnapRate"] = raw(p["eyeSnapRate"]); o["EyeSnapRot"] = q4(p["eyeSnapRot"])
    o["DistOffs"] = p["dist"]; o["TiltOffs"] = -p["tilt"]; o["RotYOffs"] = -p["rotY"]; o["RollOffs"] = -p["roll"]; o["FovYOffs"] = p["fovY"]
    o["DofFarDepthOffs"] = p["dofFarDepth"]; o["DofFarRateOffs"] = p["dofFarRate"]
    return o

def load(doc):
    rails = []; areas = []
    for k, r in doc.get("Custom", {}).items():
        if not isinstance(r, dict) or r.get("Kind") != "Rail":
            continue
        rails.append(dict(srcKey=k, uid=r.get("Uid", 0), close=r.get("IsClose", False),
            nodes=[v3(n["Pos"]) for n in r.get("Node", [])], radius=r.get("Radius", 2.0),
            bezier=r.get("BezierControlLength", 5.0), centerPos=v3(r.get("CenterPos", [0, 0, 0])),
            clockwise=r.get("IsClockwise", False), kind=r.get("Kind", "Rail"),
            erpType=r.get("ErpType", "Line"), objectName=r.get("Basic.ObjectName", "Rail")))
    for k, c in doc.get("Standard", {}).items():
        mc = c.get("Gimmick.AreaCamera.MainComponent")
        if not mc: continue
        ru = c.get("Basic.RailUser", {}) or {}
        areas.append(dict(srcKey=k,
            mode=mc.get("AreaCameraMode", "Normal"), center=v3(mc["AreaCenter"]), size=raw(mc["AreaSize"]),
            rot=q4(mc.get("AreaRot", [0, 0, 0, 1])), priority=mc.get("Priority", 0),
            erpIn=mc.get("ErpFrameIn", 120), erpOut=mc.get("ErpFrameOut", 120),
            useRail=mc.get("UseRail", False), eyeSnap=mc.get("IsEyeSnap", False), targetSnap=mc.get("IsTargetSnap", False),
            usePanLimit=mc.get("UsePanLimit", False), panCenter=mc.get("PanLimitCenter", 180.0), panRange=mc.get("PanLimitRange", 360.0),
            shapeKind=mc.get("AreaShapeKind", "Cube"), kind=mc.get("Kind", "AreaCamera"),
            checkMargin=mc.get("AreaCheckMargin", 2.0), useCustomMargin=mc.get("UseCustomAreaCheckMargin", False),
            reactPreWait=mc.get("ReactPreWaitFrame", 0), initActive=mc.get("IsInitActiveForUseEvent", False),
            useDof=mc.get("UseDofParam", False), useEventTrigger=mc.get("UseEventTrigger", False),
            mcSnapCenter=v3(mc.get("SnapCenter", [0, 0, 0])), mcSnapRot=q4(mc.get("SnapRot", [0, 0, 0, 1])),
            mcEyeSnapCenter=v3(mc.get("EyeSnapCenter", [0, 0, 0])), mcEyeSnapRot=q4(mc.get("EyeSnapRot", [0, 0, 0, 1])),
            railUid=ru.get("TargetRailUid", 0), railNodeIndex=ru.get("NodeIndex", 0),
            objectName=c.get("Basic.ObjectName", "AreaCamera"),
            base=rd_offset(c.get("Gimmick.AreaCamera.CameraOffsetParamBase")),
            end=rd_offset(c.get("Gimmick.AreaCamera.CameraOffsetParamEnd"))))
    return rails, areas

def save(orig, rails, areas):
    j = copy.deepcopy(orig)
    oldC = j.get("Custom", {}); oldS = j.get("Standard", {})
    newC = {}
    for k, c in oldC.items():
        if not (isinstance(c, dict) and c.get("Kind") == "Rail"):
            newC[k] = c   # preserve non-rail Custom (CameraLimitCollision etc.)
    for R in rails:
        key = R["srcKey"]; node = copy.deepcopy(oldC.get(key, {}))
        node.update(Uid=R["uid"], IsClose=R["close"], Radius=R["radius"],
            BezierControlLength=R["bezier"], CenterPos=v3(R["centerPos"]),
            IsClockwise=R["clockwise"], Kind=R["kind"], ErpType=R["erpType"])
        node["Basic.ObjectName"] = R["objectName"]; node["NodeCount"] = len(R["nodes"])
        oldNodes = oldC.get(key, {}).get("Node", []) if isinstance(oldC.get(key), dict) else []
        arr = []
        for i, n in enumerate(R["nodes"]):
            nd = copy.deepcopy(oldNodes[i]) if i < len(oldNodes) else {}
            nd["Pos"] = v3(n)
            arr.append(nd)
        node["Node"] = arr
        newC[key] = node
    j["Custom"] = newC
    newS = {}
    for k, c in oldS.items():
        if not c.get("Gimmick.AreaCamera.MainComponent"): newS[k] = c
    for A in areas:
        key = A["srcKey"]; node = copy.deepcopy(oldS.get(key, {}))
        mc = node.setdefault("Gimmick.AreaCamera.MainComponent", {})
        mc.update(AreaCameraMode=A["mode"], AreaCenter=v3(A["center"]), AreaSize=raw(A["size"]), AreaRot=q4(A["rot"]),
            Priority=A["priority"], ErpFrameIn=A["erpIn"], ErpFrameOut=A["erpOut"],
            UseRail=A["useRail"], IsEyeSnap=A["eyeSnap"], IsTargetSnap=A["targetSnap"],
            UsePanLimit=A["usePanLimit"], PanLimitCenter=A["panCenter"], PanLimitRange=A["panRange"],
            AreaShapeKind=A["shapeKind"], Kind=A["kind"], AreaCheckMargin=A["checkMargin"],
            UseCustomAreaCheckMargin=A["useCustomMargin"], ReactPreWaitFrame=A["reactPreWait"],
            IsInitActiveForUseEvent=A["initActive"], UseDofParam=A["useDof"], UseEventTrigger=A["useEventTrigger"],
            SnapCenter=v3(A["mcSnapCenter"]), SnapRot=q4(A["mcSnapRot"]),
            EyeSnapCenter=v3(A["mcEyeSnapCenter"]), EyeSnapRot=q4(A["mcEyeSnapRot"]))
        ru = node.setdefault("Basic.RailUser", {}); ru["TargetRailUid"] = A["railUid"]; ru["NodeIndex"] = A["railNodeIndex"]
        node["Basic.ObjectName"] = A["objectName"]
        wr_offset(A["base"], node.setdefault("Gimmick.AreaCamera.CameraOffsetParamBase", {}))
        wr_offset(A["end"], node.setdefault("Gimmick.AreaCamera.CameraOffsetParamEnd", {}))
        newS[key] = node
    j["Standard"] = newS
    return j

def diff(a, b, path="", out=None):
    if out is None: out = []
    if isinstance(a, dict) and isinstance(b, dict):
        for k in set(a) | set(b):
            if k not in a: out.append(path + "." + str(k) + ": MISSING in roundtrip")
            elif k not in b: out.append(path + "." + str(k) + ": EXTRA in roundtrip")
            else: diff(a[k], b[k], path + "." + str(k), out)
    elif isinstance(a, list) and isinstance(b, list):
        if len(a) != len(b): out.append(path + ": len " + str(len(a)) + "!=" + str(len(b)))
        else:
            for i, (x, y) in enumerate(zip(a, b)): diff(x, y, path + "[" + str(i) + "]", out)
    else:
        if isinstance(a, (int, float)) and isinstance(b, (int, float)):
            if abs(float(a) - float(b)) > 1e-4: out.append(path + ": " + str(a) + " != " + str(b))
        elif a != b: out.append(path + ": " + repr(a) + " != " + repr(b))
    return out

root = r"C:\Users\mabll\OneDrive\\바탕 화면\decompressed\ubin\Step\Map"
bins = glob.glob(os.path.join(root, "**", "Camera_Obj.bin"), recursive=True)
ok = 0; bad = 0; shown = 0; biggest = (0, "")
for b in bins:
    try:
        orig = C.convert_file(b)
        rails, areas = load(orig)
        rt = save(orig, rails, areas)
        d = diff(orig, rt, os.path.relpath(b, root))
        nr = len(orig.get("Custom", {}))
        na = len([1 for c in orig.get("Standard", {}).values() if c.get("Gimmick.AreaCamera.MainComponent")])
        if nr + na > biggest[0]: biggest = (nr + na, os.path.relpath(b, root))
        if d:
            bad += 1
            if shown < 8:
                print("\n[DIFF] " + os.path.relpath(b, root) + " (rails " + str(nr) + ", areas " + str(na) + ")")
                for line in d[:12]: print("   ", line)
                shown += 1
        else:
            ok += 1
    except Exception as e:
        bad += 1
        if shown < 8:
            print("[ERR] " + os.path.relpath(b, root) + ": " + str(e)); shown += 1
print("\n==== " + str(ok) + " lossless / " + str(bad) + " diff  (of " + str(len(bins)) + ") ====")
print("biggest step:", biggest)
