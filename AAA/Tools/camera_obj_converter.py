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


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("input", help=".bin 파일 또는 디렉터리")
    ap.add_argument("-o", "--out", default=None, help="출력 루트(디렉터리 모드)")
    args = ap.parse_args()

    if os.path.isfile(args.input):
        data = convert_file(args.input)
        out = args.out or (os.path.splitext(args.input)[0] + ".json")
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
