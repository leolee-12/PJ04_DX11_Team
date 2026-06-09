#!/usr/bin/env python3
"""
camera_obj_converter.py
원작 커비 디스커버리 Camera_Obj.bin (HAL XBIN/YAML 컨테이너) -> JSON 변환기.

XBIN 헤더(LE, magic "XBIN") + 내부 "YAML" 바이너리 트리 파싱.
노드 타입: 1=int, 2=float, 3=bool, 4=string(풀오프셋), 5=dict, 6=array.
dict = [5][N] + N*(keyOff,valOff),  array = [6][N] + N*(elemOff).
문자열은 풀에 [u32 len][ascii], 4바이트 정렬.

사용법:
    python camera_obj_converter.py <입력경로> [-o 출력루트]
    - 입력경로가 .bin 파일이면 단일 변환
    - 디렉터리면 하위 모든 Camera_Obj.bin 재귀 변환(폴더 구조 미러링)
"""
import struct, json, sys, os, argparse

class XbinYaml:
    def __init__(self, data: bytes):
        self.f = data
        if data[:4] != b"XBIN":
            raise ValueError("XBIN magic 아님")
        # YAML 서브컨테이너는 0x14에서 시작, 루트 dict 헤더는 0x1c(type)/0x20(count)
        if data[0x14:0x18] != b"YAML":
            raise ValueError("YAML 서브컨테이너 아님")

    def u32(self, o): return struct.unpack_from("<I", self.f, o)[0]
    def i32(self, o): return struct.unpack_from("<i", self.f, o)[0]
    def f32(self, o): return struct.unpack_from("<f", self.f, o)[0]

    def getstr(self, o):
        """오프셋 o에서 [u32 len][ascii] 문자열을 직접 읽음(풀 위치 무관)."""
        ln = self.u32(o)
        if 0 <= ln <= 256 and o + 4 + ln <= len(self.f):
            s = self.f[o + 4:o + 4 + ln]
            if all(32 <= c < 127 for c in s):
                return s.decode("ascii")
        return f"<str@{o:#x}>"

    def node(self, o, depth=0):
        if depth > 64:
            return "<too-deep>"
        t = self.u32(o)
        if t == 1: return self.i32(o + 4)            # int (부호 보존)
        if t == 2: return round(self.f32(o + 4), 6)  # float
        if t == 3: return bool(self.u32(o + 4))      # bool
        if t == 4: return self.getstr(self.u32(o + 4))
        if t == 5:                                   # dict
            cnt = self.u32(o + 4); d = {}
            for i in range(cnt):
                ko = self.u32(o + 8 + i * 8)
                vo = self.u32(o + 12 + i * 8)
                d[self.getstr(ko)] = self.node(vo, depth + 1)
            return d
        if t == 6:                                   # array
            cnt = self.u32(o + 4); a = []
            for i in range(cnt):
                a.append(self.node(self.u32(o + 8 + i * 4), depth + 1))
            return a
        return f"<unknown-type{t}@{o:#x}>"

    def parse(self):
        return self.node(0x1c)


def convert_file(path):
    with open(path, "rb") as fp:
        return XbinYaml(fp.read()).parse()


# ─────────────────────────────────────────────────────────────
#  슬림 익스포트: 원작 전체 트리 → 엔진 AREA_CAMERA/RAIL 슬림 스키마
#  좌표 변환: Switch(RH, -Z forward) → 우리 엔진(LH, +Z forward) = Z 부호 반전
#  (안 맞으면 FLIP_Z를 끄거나 FLIP_X로 바꿔 캘리브레이션)
# ─────────────────────────────────────────────────────────────
FLIP_Z = True

def _v3(a):
    x, y, z = a[0], a[1], a[2]
    return [x, y, (-z if FLIP_Z else z)]

def _quat(a):
    x, y, z, w = a
    return [(-x if FLIP_Z else x), (-y if FLIP_Z else y), z, w]   # Z반전 시 best-guess

def _rate3(a):   # SnapRate 등 비율(축별 0~1) — 좌표 아님, 부호 유지
    return [a[0], a[1], a[2]]

def _ang(v):     # 각도 오프셋: Z거울변환에서 회전 sense가 반대 → 부호 반전
    return (-v if FLIP_Z else v)

def _offset(d):
    d = d or {}
    return {
        "targetOffs": _v3(d.get("TargetOffs", [0, 0, 0])),
        "snapCenter": _v3(d.get("SnapCenter", [0, 0, 0])),
        "snapRate":   _rate3(d.get("SnapRate", [1, 1, 1])),        # ★ 축별 부분추종
        "snapRot":    _quat(d.get("SnapRot", [0, 0, 0, 1])),       # ★ 룩 base 회전
        "eyeSnapCenter": _v3(d.get("EyeSnapCenter", [0, 0, 0])),
        "eyeSnapRate":   _rate3(d.get("EyeSnapRate", [1, 1, 1])),  # ★ 눈 축별 추종
        "eyeSnapRot":    _quat(d.get("EyeSnapRot", [0, 0, 0, 1])), # ★ 눈 base 회전(카메라 방향)
        "distOffs":  d.get("DistOffs", 0.0),
        "tiltOffs":  _ang(d.get("TiltOffs", 0.0)),                 # ★ 거울 → 부호 반전
        "rotYOffs":  _ang(d.get("RotYOffs", 0.0)),                 # ★
        "rollOffs":  _ang(d.get("RollOffs", 0.0)),                 # ★
        "fovYOffs":  d.get("FovYOffs", 0.0),                       # 화각은 부호 무관
    }

def to_slim(parsed):
    rails = []
    for _, r in parsed.get("Custom", {}).items():
        rails.append({
            "uid":   r.get("Uid", 0),
            "close": r.get("IsClose", False),
            "nodes": [_v3(n["Pos"]) for n in r.get("Node", [])],
        })
    areas = []
    for _, c in parsed.get("Standard", {}).items():
        mc = c.get("Gimmick.AreaCamera.MainComponent")
        if not mc:
            continue
        ru = c.get("Basic.RailUser", {}) or {}
        areas.append({
            "mode":    mc.get("AreaCameraMode", "Normal"),  # ★ Normal | FixedGazing
            "center":  _v3(mc["AreaCenter"]),
            "size":    list(mc["AreaSize"]),               # 크기(범위)는 부호 유지
            "rot":     _quat(mc.get("AreaRot", [0, 0, 0, 1])),
            "priority": mc.get("Priority", 0),
            "erpIn":   mc.get("ErpFrameIn", 120),
            "erpOut":  mc.get("ErpFrameOut", 120),
            "useRail": mc.get("UseRail", False),
            "railUid": ru.get("TargetRailUid", 0),
            "eyeSnap":    mc.get("IsEyeSnap", False),
            "targetSnap": mc.get("IsTargetSnap", False),
            "usePanLimit":    mc.get("UsePanLimit", False),
            "panLimitCenter": mc.get("PanLimitCenter", 0.0),
            "panLimitRange":  mc.get("PanLimitRange", 360.0),
            "base": _offset(c.get("Gimmick.AreaCamera.CameraOffsetParamBase")),
            "end":  _offset(c.get("Gimmick.AreaCamera.CameraOffsetParamEnd")),
        })
    return {"rails": rails, "areas": areas}


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("input", help=".bin 파일 또는 디렉터리")
    ap.add_argument("-o", "--out", default=None, help="출력 루트(디렉터리 모드)")
    ap.add_argument("--slim", action="store_true", help="엔진용 슬림 스키마로 출력")
    args = ap.parse_args()

    if os.path.isfile(args.input):
        data = convert_file(args.input)
        if args.slim:
            data = to_slim(data)
        suffix = "_cam.json" if args.slim else ".json"
        out = args.out or (os.path.splitext(args.input)[0] + suffix)
        with open(out, "w", encoding="utf-8") as fp:
            json.dump(data, fp, indent=1, ensure_ascii=False)
        print(f"OK: {out}")
        return

    # 디렉터리 재귀
    out_root = args.out or os.path.join(args.input, "_camera_json")
    ok = fail = 0
    unknown_types = set()
    for root, _, files in os.walk(args.input):
        for name in files:
            if name != "Camera_Obj.bin":
                continue
            src = os.path.join(root, name)
            rel = os.path.relpath(src, args.input)
            dst = os.path.join(out_root, os.path.splitext(rel)[0] + ".json")
            os.makedirs(os.path.dirname(dst), exist_ok=True)
            try:
                data = convert_file(src)
                txt = json.dumps(data, indent=1, ensure_ascii=False)
                if "unknown-type" in txt:
                    for tok in txt.split("<unknown-type")[1:]:
                        unknown_types.add(tok.split("@")[0])
                with open(dst, "w", encoding="utf-8") as fp:
                    fp.write(txt)
                ok += 1
            except Exception as e:
                fail += 1
                print(f"FAIL {rel}: {e}")
    print(f"\n변환 완료: {ok} 성공 / {fail} 실패 -> {out_root}")
    if unknown_types:
        print(f"미식별 타입코드 발견: {sorted(unknown_types)}")


if __name__ == "__main__":
    main()
