#!/usr/bin/env python3
from __future__ import annotations

import argparse
import os
import shutil
import stat
import tempfile
import xml.etree.ElementTree as ET
from collections import defaultdict
from io import BytesIO
from pathlib import Path
from typing import Iterable


# 검색에서 제외할 일반적인 생성 폴더입니다.
DEFAULT_EXCLUDED_DIRS = {
    ".git",
    ".vs",
    ".idea",
    "__pycache__",
    "bin",
    "obj",
    "out",
    "build",
}

FILTERABLE_ITEM_TYPES = {
    "ClCompile",
    "ClInclude",
    "FxCompile",
    "Image",
    "None",
    "ResourceCompile",
}

ALLOWED_FILTER_ITEM_TYPES = FILTERABLE_ITEM_TYPES | {"Filter"}


class FilterValidationError(ValueError):
    """필터 구조 또는 프로젝트 대조 오류를 모아서 전달합니다."""

    def __init__(self, messages: list[str]) -> None:
        super().__init__("filter validation failed")
        self.messages = messages


def local_name(tag: object) -> str:
    """
    XML namespace가 붙은 태그에서 실제 이름만 꺼냅니다.

    예:
        {http://schemas.microsoft.com/developer/msbuild/2003}ClCompile
        -> ClCompile

    XML Comment는 tag가 문자열이 아니므로 빈 문자열을 반환합니다.
    """
    if not isinstance(tag, str):
        return ""

    return tag.rsplit("}", 1)[-1]


def normalize_path(value: str) -> str:
    """
    정렬 비교용으로만 경로를 정규화합니다.

    실제 XML의 Include 문자열은 수정하지 않습니다.
    Windows 경로 특성을 고려하여:
    - '/'를 '\\'로 취급
    - 대소문자를 구분하지 않음
    """
    return value.replace("/", "\\").casefold()


def child_text(item: ET.Element, name: str) -> str:
    """
    항목의 특정 자식 노드 텍스트를 가져옵니다.

    예:
        <ClCompile Include="Player.cpp">
          <Filter>Source Files\\Player</Filter>
        </ClCompile>
    """
    for child in item:
        if local_name(child.tag) == name:
            return (child.text or "").strip()

    return ""


def item_sort_key(
    item: ET.Element,
) -> tuple[str, str, str, str, str]:
    """
    결정론적 정렬 기준입니다.

    1. 항목 타입: ClCompile, ClInclude, Filter 등
    2. Include 경로
    3. 자식 Filter 경로
    4. Condition
    5. 원본 Include 문자열: 최종 동률 해소
    """
    include = item.get("Include", "")

    return (
        local_name(item.tag).casefold(),
        normalize_path(include),
        normalize_path(child_text(item, "Filter")),
        item.get("Condition", "").casefold(),
        include,
    )


def is_sortable_item(element: ET.Element) -> bool:
    """
    ItemGroup의 직접 자식 중 Include 속성이 있는 항목만 정렬합니다.

    XML Comment, Property, 메타데이터 등은 제외됩니다.
    """
    return (
        isinstance(element.tag, str)
        and "Include" in element.attrib
    )


def sort_item_group(group: ET.Element) -> bool:
    """
    하나의 ItemGroup 내부를 정렬합니다.

    주석이나 Include가 없는 노드를 만나면 정렬 구간을 끊습니다.
    따라서 주석이 설명하고 있는 영역이 엉뚱한 위치로 이동하지 않습니다.
    """
    children = list(group)
    changed = False
    start = 0

    while start < len(children):
        # 정렬 가능한 항목이 시작되는 위치 탐색
        while (
            start < len(children)
            and not is_sortable_item(children[start])
        ):
            start += 1

        if start >= len(children):
            break

        # 연속된 정렬 가능 항목의 끝 탐색
        end = start

        while (
            end < len(children)
            and is_sortable_item(children[end])
        ):
            end += 1

        original = children[start:end]
        ordered = sorted(original, key=item_sort_key)

        if original != ordered:
            children[start:end] = ordered
            changed = True

        start = end

    if changed:
        group[:] = children

    return changed


def register_namespaces(raw: bytes) -> None:
    """
    기존 XML namespace를 ElementTree에 등록합니다.

    등록하지 않으면 다음처럼 바뀔 수 있습니다.

        <Project xmlns="...">

    ->

        <ns0:Project xmlns:ns0="...">

    MSBuild 기본 namespace를 유지하기 위한 처리입니다.
    """
    seen: set[tuple[str, str]] = set()

    for _, namespace in ET.iterparse(
        BytesIO(raw),
        events=("start-ns",),
    ):
        prefix, uri = namespace
        pair = (prefix, uri)

        if pair in seen:
            continue

        seen.add(pair)

        try:
            ET.register_namespace(prefix, uri)
        except ValueError:
            # ns0처럼 ElementTree가 예약한 prefix가 있다면 무시합니다.
            pass


def parse_xml(raw: bytes) -> ET.Element:
    """
    주석과 Processing Instruction을 가능한 범위에서 보존하며
    XML을 파싱합니다.
    """
    register_namespaces(raw)

    parser = ET.XMLParser(
        target=ET.TreeBuilder(
            insert_comments=True,
            insert_pis=True,
        )
    )

    return ET.fromstring(raw, parser=parser)


def direct_item_group_children(
    root: ET.Element,
) -> Iterable[ET.Element]:
    """모든 ItemGroup의 직접 자식 요소를 순서대로 반환합니다."""

    for element in root.iter():
        if local_name(element.tag) != "ItemGroup":
            continue

        yield from element


def collect_file_items(
    root: ET.Element,
) -> dict[tuple[str, str], list[ET.Element]]:
    """필터에 배치할 수 있는 파일 항목을 타입과 경로로 묶습니다."""

    items: dict[
        tuple[str, str],
        list[ET.Element],
    ] = defaultdict(list)

    for element in direct_item_group_children(root):
        item_type = local_name(element.tag)

        if item_type not in FILTERABLE_ITEM_TYPES:
            continue

        include = (element.get("Include") or "").strip()

        if not include:
            continue

        key = (
            item_type.casefold(),
            normalize_path(include),
        )
        items[key].append(element)

    return items


def validate_filter_tree(root: ET.Element) -> list[str]:
    """필터 XML 내부의 구조, 중복, 참조 오류를 검사합니다."""

    errors: list[str] = []
    filter_definitions: dict[
        str,
        list[ET.Element],
    ] = defaultdict(list)
    guid_definitions: dict[
        str,
        list[tuple[str, str]],
    ] = defaultdict(list)

    for element in direct_item_group_children(root):
        item_type = local_name(element.tag)

        if not item_type:
            continue

        if item_type not in ALLOWED_FILTER_ITEM_TYPES:
            include = (element.get("Include") or "").strip()
            errors.append(
                "[UNEXPECTED_ITEM] "
                f"Type={item_type} "
                f"Include={include or '<none>'}"
            )
            continue

        include = (element.get("Include") or "").strip()

        if not include:
            if item_type == "Filter":
                value = (element.text or "").strip()
                errors.append(
                    "[ORPHAN_FILTER] "
                    f"Filter={value or '<empty>'}"
                )
            else:
                errors.append(
                    "[MISSING_INCLUDE] "
                    f"Type={item_type}"
                )

            continue

        if item_type != "Filter":
            continue

        filter_definitions[
            normalize_path(include)
        ].append(element)

        guid = child_text(element, "UniqueIdentifier")

        if guid:
            guid_definitions[
                guid.casefold()
            ].append((guid, include))

    for definitions in filter_definitions.values():
        if len(definitions) <= 1:
            continue

        include = (
            definitions[0].get("Include")
            or "<empty>"
        )
        errors.append(
            "[DUPLICATE_FILTER] "
            f"Filter={include} "
            f"Count={len(definitions)}"
        )

    for definitions in guid_definitions.values():
        if len(definitions) <= 1:
            continue

        guid = definitions[0][0]
        filters = ", ".join(
            include
            for _, include in definitions
        )
        errors.append(
            "[DUPLICATE_GUID] "
            f"GUID={guid} "
            f"Filters={filters}"
        )

    file_items = collect_file_items(root)

    for elements in file_items.values():
        if len(elements) <= 1:
            continue

        item_type = local_name(elements[0].tag)
        include = (
            elements[0].get("Include")
            or "<empty>"
        )
        filters = ", ".join(
            child_text(element, "Filter")
            or "<none>"
            for element in elements
        )
        errors.append(
            "[DUPLICATE_ITEM] "
            f"Type={item_type} "
            f"Include={include} "
            f"Count={len(elements)} "
            f"Filters={filters}"
        )

    known_filters = set(filter_definitions)

    for elements in file_items.values():
        for element in elements:
            filter_name = child_text(
                element,
                "Filter",
            )

            if not filter_name:
                continue

            if normalize_path(filter_name) in known_filters:
                continue

            item_type = local_name(element.tag)
            include = (
                element.get("Include")
                or "<empty>"
            )
            errors.append(
                "[UNDECLARED_FILTER] "
                f"Type={item_type} "
                f"Include={include} "
                f"Filter={filter_name}"
            )

    return errors


def compare_with_project(
    filter_path: Path,
    filter_root: ET.Element,
) -> list[str]:
    """동일 이름의 vcxproj와 filters 파일 항목을 양방향 대조합니다."""

    errors: list[str] = []
    project_path = filter_path.with_suffix("")

    if not project_path.is_file():
        return [
            "[MISSING_PROJECT] "
            f"File={project_path}"
        ]

    try:
        project_root = parse_xml(
            project_path.read_bytes()
        )
    except ET.ParseError as error:
        return [
            "[INVALID_PROJECT_XML] "
            f"File={project_path} "
            f"Reason={error}"
        ]
    except OSError as error:
        return [
            "[PROJECT_READ_ERROR] "
            f"File={project_path} "
            f"Reason={error}"
        ]

    if local_name(project_root.tag) != "Project":
        return [
            "[INVALID_PROJECT_ROOT] "
            f"File={project_path}"
        ]

    project_items = collect_file_items(project_root)
    filter_items = collect_file_items(filter_root)

    for key in sorted(
        project_items.keys() - filter_items.keys(),
    ):
        element = project_items[key][0]
        item_type = local_name(element.tag)
        include = element.get("Include") or "<empty>"
        errors.append(
            "[PROJECT_ONLY] "
            f"Type={item_type} "
            f"Include={include}"
        )

    for key in sorted(
        filter_items.keys() - project_items.keys(),
    ):
        element = filter_items[key][0]
        item_type = local_name(element.tag)
        include = element.get("Include") or "<empty>"
        errors.append(
            "[FILTERS_ONLY] "
            f"Type={item_type} "
            f"Include={include}"
        )

    return errors


def serialize_xml(
    root: ET.Element,
    newline: str,
    has_bom: bool,
) -> bytes:
    """
    XML을 일정한 형태로 직렬화합니다.

    - 들여쓰기: 공백 2칸
    - encoding: UTF-8
    - 기존 CRLF/LF 유지
    - 기존 UTF-8 BOM 여부 유지
    """
    ET.indent(root, space="  ")

    body = ET.tostring(
        root,
        encoding="unicode",
        short_empty_elements=True,
    )

    text = (
        '<?xml version="1.0" encoding="utf-8"?>\n'
        + body
        + "\n"
    )

    if newline != "\n":
        text = text.replace("\n", newline)

    encoded = text.encode("utf-8")

    if has_bom:
        return b"\xef\xbb\xbf" + encoded

    return encoded


def normalize_file(path: Path) -> tuple[bool, bytes]:
    """
    파일 하나를 검증하고 정렬한 결과를 반환합니다.

    반환값:
        (변경 필요 여부, 정렬된 데이터)
    """
    raw = path.read_bytes()

    newline = "\r\n" if b"\r\n" in raw else "\n"
    has_bom = raw.startswith(b"\xef\xbb\xbf")

    root = parse_xml(raw)

    if local_name(root.tag) != "Project":
        raise ValueError("root element is not <Project>")

    validation_errors = validate_filter_tree(root)
    validation_errors.extend(
        compare_with_project(path, root)
    )

    if validation_errors:
        raise FilterValidationError(
            validation_errors
        )

    for element in root.iter():
        if local_name(element.tag) == "ItemGroup":
            sort_item_group(element)

    normalized = serialize_xml(
        root,
        newline,
        has_bom,
    )

    normalized_root = parse_xml(normalized)
    post_validation_errors = validate_filter_tree(
        normalized_root
    )

    if post_validation_errors:
        raise FilterValidationError(
            post_validation_errors
        )

    return normalized != raw, normalized


def atomic_write(
    path: Path,
    data: bytes,
    make_backup: bool,
) -> None:
    """
    임시 파일에 먼저 기록한 뒤 원본과 교체합니다.

    스크립트 실행 중 중단되더라도 원본 파일이 반쯤 기록되는 것을
    방지합니다.
    """
    original_mode = stat.S_IMODE(path.stat().st_mode)

    if make_backup:
        shutil.copy2(
            path,
            Path(f"{path}.bak"),
        )

    fd, temp_name = tempfile.mkstemp(
        prefix=f".{path.name}.",
        suffix=".tmp",
        dir=path.parent,
    )

    temp_path = Path(temp_name)

    try:
        with os.fdopen(fd, "wb") as stream:
            stream.write(data)
            stream.flush()
            os.fsync(stream.fileno())

        os.chmod(temp_path, original_mode)
        os.replace(temp_path, path)

    except Exception:
        temp_path.unlink(missing_ok=True)
        raise


def discover_files(
    paths: Iterable[Path],
) -> list[Path]:
    """
    입력된 파일 또는 디렉터리에서
    *.vcxproj.filters 파일을 재귀적으로 찾습니다.
    """
    found: set[Path] = set()

    for input_path in paths:
        path = input_path.resolve()

        if path.is_file():
            if path.name.endswith(".vcxproj.filters"):
                found.add(path)

            continue

        if not path.is_dir():
            raise FileNotFoundError(
                f"path does not exist: {input_path}"
            )

        for root, dirs, files in os.walk(
            path,
            followlinks=False,
        ):
            # os.walk가 하위 폴더에 들어가기 전에 제외합니다.
            dirs[:] = [
                directory
                for directory in dirs
                if directory.casefold()
                not in DEFAULT_EXCLUDED_DIRS
            ]

            root_path = Path(root)

            for filename in files:
                if filename.endswith(".vcxproj.filters"):
                    found.add(
                        (root_path / filename).resolve()
                    )

    return sorted(
        found,
        key=lambda value: str(value).casefold(),
    )


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description=(
            "Validate and deterministically sort "
            "Visual Studio .vcxproj.filters XML files."
        )
    )

    parser.add_argument(
        "paths",
        nargs="*",
        default=["."],
        help=(
            "Files or directories to scan recursively. "
            "Default: current directory."
        ),
    )

    parser.add_argument(
        "--check",
        action="store_true",
        help=(
            "Validate without modifying files; exit 1 "
            "if normalization is needed."
        ),
    )

    parser.add_argument(
        "--backup",
        action="store_true",
        help="Create '<file>.bak' before modifying.",
    )

    parser.add_argument(
        "--quiet",
        action="store_true",
        help="Print only errors and the final summary.",
    )

    return parser.parse_args()


def main() -> int:
    args = parse_args()

    try:
        files = discover_files(
            Path(value)
            for value in args.paths
        )
    except Exception as error:
        print(f"[ERROR] {error}")
        return 2

    if not files:
        print("No .vcxproj.filters files found.")
        return 0

    changed_count = 0
    error_count = 0

    for path in files:
        try:
            changed, normalized = normalize_file(path)

            if not changed:
                if not args.quiet:
                    print(f"[OK]    {path}")

                continue

            changed_count += 1

            if args.check:
                print(f"[NEEDS SORT] {path}")
            else:
                atomic_write(
                    path,
                    normalized,
                    args.backup,
                )
                print(f"[SORTED] {path}")

        except FilterValidationError as error:
            error_count += 1
            print(f"[INVALID] {path}")

            for message in error.messages:
                print(f"  {message}")

        except ET.ParseError as error:
            error_count += 1
            print(f"[INVALID_XML] {path}: {error}")

        except (OSError, ValueError) as error:
            error_count += 1
            print(f"[ERROR] {path}: {error}")

    print(
        f"Checked={len(files)}, "
        f"Changed={changed_count}, "
        f"Errors={error_count}"
    )

    if error_count:
        return 2

    if args.check and changed_count:
        return 1

    return 0


if __name__ == "__main__":
    raise SystemExit(main())
