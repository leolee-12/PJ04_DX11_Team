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
#  엔진 익스포트(--slim): 원작 전체 트리 → 우리 커스텀 플랫 스키마
#    {rails:[{uid,close,nodes}], areas:[{center,size,rot,priority,erpIn,erpOut,
#     useRail,railUid,scrollDead, base/end:{yaw,pitch,distance,height,fov,
#     aimOffset,followRate}}]}
#  GameContent CAreaCameraSolver::Load 가 그대로 읽음(좌표 추가변환 없음).
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

def to_engine(parsed):
    # 원작에선 영역(OBB)+레일만 가져오고, 카메라 프레이밍은 우리 기본값으로 깔아둠.
    # (에디터에서 영역별로 손세팅: yaw/pitch/distance/height/fov/aimHeight)
    rails = []
    for _, r in parsed.get("Custom", {}).items():
        if r.get("Kind") != "Rail":              # CameraLimitCollision 등 비-레일 제외
            continue
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
        use_rail = mc.get("UseRail", False)
        frame = {
            "yaw":       0.0,
            "pitch":     20.0,
            "distance":  0.0 if use_rail else 16.0,      # 레일=눈이 레일 위, 비레일=뒤로 16
            "height":    3.0,
            "fov":       50.0,
            "aimHeight": 4.0,
            "gazePoint": [0.0, 0.0, 0.0],                # Point/Object 응시 좌표
            "gazeBlend": 1.0,                            # 0=커비, 1=응시대상
        }
        areas.append({
            "center":   _v3(mc["AreaCenter"]),
            "size":     list(mc["AreaSize"]),            # 크기(범위)는 부호 유지
            "rot":      _quat(mc.get("AreaRot", [0, 0, 0, 1])),
            "priority": mc.get("Priority", 0),
            "useRail":  use_rail,
            "railUid":  ru.get("TargetRailUid", 0),
            "scrollDead": [0.0, 0.0, 0.0],
            "gazeMode": 0,                               # 0=Kirby, 1=Point, 2=Object
            "gazeTag": "",                              # Object 모드 런타임 해석 태그
            "frame":    dict(frame),                     # 비레일 프레이밍 / 레일 start
            "frameEnd": dict(frame),                     # 레일 end (레일 진행도 t로 frame→frameEnd 보간)
        })
    return {"rails": rails, "areas": areas}


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("input", help=".bin 파일 또는 디렉터리")
    ap.add_argument("-o", "--out", default=None, help="출력 루트(디렉터리 모드)")
    ap.add_argument("--slim", action="store_true", help="엔진용 커스텀 플랫 스키마로 출력")
    args = ap.parse_args()

    if os.path.isfile(args.input):
        data = convert_file(args.input)
        if args.slim:
            data = to_engine(data)
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
