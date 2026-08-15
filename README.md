# Kirby Clone Project (Jusin159 DX11 Team Project)

> DirectX 11 기반 3D 액션 플랫포머 팀 프로젝트.
> 닌텐도 스위치 게임 `별의 커비 디스커버리`를 레퍼런스로, 교육 과정에서 제공된 컴포넌트 기반 엔진 프레임워크를 확장해 제작했습니다.

| 항목 | 내용 |
|---|---|
| 개발 기간 | 2026.05.28 ~ 2026.08.02 (저장소 기준 약 10주) |
| 인원 | 4명 |
| 플랫폼 | Windows x64 |
| 그래픽 API | DirectX 11 |
| 언어 | C++17 / HLSL |

---

## 프로젝트 개요

원작의 3D 액션 플랫포머 구조를 그대로 재현하는 것을 목표로 했습니다.

- **마을 - 스테이지 - 보스전 - 엔딩**으로 이어지는 완결된 플레이 흐름
- 커비의 **흡입 / 능력 복사**(소드, 토이해머 등)와 **머금기 변형(Deform)** 시스템
- **보스 4종** (메타나이트, 고릴라, 기간트엣지, 아르마딜로)과 몬스터 AI
- 컷씬 시퀀스, 대화 연출, 화면 전환, 클리어 연출 등 게임 외적 연출까지 포함
- 레벨 배치 / 애니메이션 이벤트 / 카메라 연출을 위한 **자체 툴 4종** 병행 개발

---

## 기술 스택

| 분류 | 사용 기술 |
|---|---|
| 렌더링 | DirectX 11, HLSL, Effects11 |
| 물리 / 충돌 | NVIDIA PhysX 5 (환경 충돌, 캡슐 CCT) + 자체 콜라이더 (전투 판정) |
| 사운드 | FMOD |
| 에디터 UI | Dear ImGui, ImGuizmo |
| 데이터 | nlohmann/json (레벨, 이펙트, 애님 이벤트, 카메라 데이터) |
| 텍스처 / 모델 | DirectXTK, DirectXTex, Assimp 기반 자체 모델 포맷(`.ysh`) |
| 툴체인 | Visual Studio 2022 (v143), Python (프로젝트 파일 정렬 훅) |

---

## 저장소 구성

```
AAA/
├── Engine/          엔진 코어 DLL (렌더러, 리소스, 컴포넌트, 물리, 충돌, BT, 이펙트)
├── GameContent/     게임 로직 DLL (캐릭터, 보스, 몬스터, UI, 레벨 디자인 오브젝트)
├── Launcher/        실행 파일 (레벨 정의, 비동기 로더, 진입점)
├── Editor/          런타임 인게임 에디터
├── MapTool/         맵 배치 툴
├── AnimUITool/      애니메이션 이벤트 / UI 배치 툴
├── CameraTool/      카메라 연출 편집 툴
├── EngineSDK/       Engine 헤더 + lib 배포 대상 (빌드 산출물)
├── ContentSDK/      GameContent 헤더 + lib 배포 대상 (빌드 산출물)
└── Tools/           보조 스크립트 (프로젝트 필터 정렬, 카메라 데이터 변환)
```

`Engine`과 `GameContent`는 DLL, 나머지는 이 둘을 참조하는 EXE로 구성됩니다.
`UpdateLib.bat`이 헤더 / lib / DLL / 셰이더를 각 EXE 프로젝트로 배포합니다.

### 코드 규모 (서드파티 및 툴 사본 제외, 저장소 추적 파일 기준)

| 모듈 | 파일 수 | 라인 수 |
|---|---:|---:|
| GameContent | 1,112 | 123,829 |
| Engine | 255 | 43,161 |
| Tools (MapTool / AnimUITool / Editor / CameraTool) | 119 | 20,157 |
| Launcher | 42 | 2,380 |
| **합계** | **1,528** | **약 190,000** |

HLSL 셰이더는 별도로 43개 파일 / 13,575 라인입니다.

---

## 주요 구현

<details>
<summary><b>렌더링 파이프라인</b></summary>

교육용 프레임워크에 포함된 기본 디퍼드 렌더러를 단계적으로 확장했습니다.

**패스 구성 (총 22패스)**

```
Priority → Sky → Shadow → ShadowBlur → NonBlend → Decals → SSAO
→ Lights → VolumetricFog → Combined → SpotlightDarken → SSR → DoF
→ Effect_HDR → Distortion → DistortionApply → Bloom → NonLight
→ Blend → Occlusion → UI(BACK/MIDDLE/FRONT) → Curtain → Flash
```

- **IBL 기반 PBR** — BC6H HDR 큐브맵, roughness 대응 밉 샘플링
- **볼류메트릭 포그** — froxel 방식 컴퓨트 셰이더 (`Texture3D` + UAV, Inject / Integrate 2패스)
- **ESM 그림자** — 라이트 뎁스타겟 R32_FLOAT, 셰도우맵에 캐스터 클래스를 함께 기록해 셀프섀도우 아크네 제거
- **SSAO / SSR / DoF / Bloom / 디스토션** 전용 패스
- **투영 데칼** — 공격 예고 표시 등에 재사용되는 프로젝터 데칼
- **3D LUT 톤매핑** — 16³ LUT 기반 색보정
- **커튼 MRT 화면 전환** — 별 모양 와이프, 페이드 등 연출을 JSON 시퀀스로 정의
- **셰이더 전역 상수 레지스트리** (`CShaderGlobal_Manager`) — 에디터에서 실시간 튜닝

</details>

<details>
<summary><b>엔진 시스템</b></summary>

- **C++ 리플렉션** — `GENERATED_BODY` / `PROPERTY` 매크로로 멤버 오프셋을 수집해 런타임 리플렉션 테이블 구성. 에디터 노출과 JSON 직렬화가 동일한 메타데이터 하나로 동작 (`GENERATED_BODY` 725개 파일, `PROPERTY` 선언 278개)
- **EventBus** — 문자열 태그 기반 pub/sub. 핸들러 안에서 다시 `Publish`가 일어나는 중첩 호출에 대비해 depth 카운터 + 해제 요청 큐로 재진입 안전성 확보
- **Behavior Tree + Blackboard** — `std::function` 기반 Action / Condition / Decorator / Composite, 타입 소거 Blackboard
- **애니메이터** — 4레이어 + 본 마스크 + 크로스페이드, point / range 애님 이벤트와 사이드카 JSON
- **충돌 이중화** — 환경 충돌은 PhysX(트라이앵글 메시 쿠킹, 캡슐 CCT), 전투 판정은 자체 `CCollider`(그룹 페어, OBB 공격 볼륨, 콜라이더별 `std::function` 핸들러). 프레임 단위 결정론과 즉시 토글이 필요한 전투 판정을 물리 씬 쿼리에서 의도적으로 분리
- **커스텀 바운딩** — AABB / OBB / Sphere / Capsule에 더해 `TORUS`(도넛, 초승달, 부채꼴) 해석적 판정 추가
- **이펙트 시스템** — Container → Emitter → Particle 3계층, 8MB 아레나 bump 얼로케이터 + 종류별 휴면 재사용 풀
- **프로파일러** — RAII 스코프 CPU 프로파일러. `PROFILE_ENABLE` 0이면 매크로 전체가 컴파일 아웃
- **컬링** — 뷰별(메인 / 그림자) 프러스텀 상태 캐시, 거리 및 프러스텀 페이드로 팝핑 제거
- **병렬 맵 로딩** — 맵 프리로드를 모델 단위 잡으로 분해해 워커 풀에 분배, atomic 카운터로 진행률을 로딩 UI에 연동

</details>

<details>
<summary><b>게임플레이</b></summary>

- **커비** — 상태 머신 기반 이동 / 점프 / 흡입 / 뱉기, 사다리, 수영
- **능력 복사** — 적을 흡입해 소드, 토이해머 등으로 변신. 능력별 콤보와 전용 이펙트
- **머금기 변형(Deform)** — 자동차 등 오브젝트를 삼켜 형태와 조작이 바뀌는 변신
- **보스 AI** — Behavior Tree 기반 패턴
  - *메타나이트* — 스텝 접근, 랜덤 콤보, 저스트 회피 반격, 확산 파동, QTE 피니시
  - *고릴라* — 2페이즈 전환, 던지기 카운터, 스핀 공격
  - *기간트엣지* — 액티브 가드, 그로기 시스템
  - *아르마딜로* — 파트너 소환, 트윈 롤링, 벽 반사 이동
- **히트 리액션** — 피격 시 자기 시간만 정지하는 히트스톱 + 월드 기준 렌더 셰이크
- **연출 시스템** — JSON step(`wait` / `anim` / `warp` / `say` / `cam`)으로 정의하는 시퀀스 플레이어, 전용 대화 카메라, 보스 인트로 / 클리어 / 엔딩 컷씬
- **카메라** — 영역 기반 오빗 카메라, 스크롤 윈도우, 컷씬 카메라, 레벨별 카메라 디렉터

</details>

<details>
<summary><b>데이터 파이프라인 / 툴</b></summary>

**자체 툴 4종**

| 툴 | 역할 |
|---|---|
| **MapTool** | 맵 오브젝트 배치, 인스펙터 편집, 레벨 디자인 요소(부술 수 있는 오브젝트, 수풀, 포인트) 배치 |
| **AnimUITool** | 애니메이션 이벤트 타임라인 편집, UI 요소 배치 |
| **CameraTool** | 카메라 연출 데이터 편집 및 자체 포맷 변환 |
| **Editor** | 런타임 인게임 에디터 (ImGui Details 패널, 프로퍼티 실시간 편집) |

**데이터 드리븐 구성**

- 레벨 JSON을 `MAP` / `LiveObject` / `UI` / `EFFECT` 도메인별로 분리하고 런처 매니페스트로 묶음
- Parser / Loader / Spawner / ProtoRegister / PresetCatalog 로 역할 분리
- 에디터 배치 → JSON 저장 → 런타임 로드까지 왕복 가능
- 리플렉션 프로퍼티가 그대로 에디터 UI와 직렬화 스키마가 되므로, 필드 추가 시 툴 코드 수정이 필요 없음

</details>

---

## 팀 구성 및 담당

| 담당 | GitHub | 주요 작업 |
|---|---|---|
| 렌더링 / 보스 / UI / 카메라 / 엔진 코어 | [@Marb1e0817](https://github.com/Marb1e0817) | 디퍼드 파이프라인 확장, 셰이더, 리플렉션, EventBus, BT, PhysX 연동, 보스 AI, 컷씬 시퀀스 |
| 플레이어 / 이펙트 툴 | [@yoonseungeon](https://github.com/yoonseungeon) | 커비 상태 머신, 능력 복사 시스템, 변신(Deform), 이펙트 로더 |
| 레벨 / 맵 툴 | [@leolee-12](https://github.com/leolee-12) | MapTool, 맵 로더 / 스포너, 환경 오브젝트, 레벨 디자인 요소, 컬링 / 프로파일러 |
| 몬스터 / 애니메이션 / 애니매이션,UI툴 | [@ddoichaboom](https://github.com/ddoichaboom) | 몬스터 AI, 애니메이터 시스템, AnimUITool |

> 담당은 주 작성 영역 기준이며, 대부분의 기능은 공용 모듈 위에서 서로 맞물려 있습니다.

---

## 빌드

### 요구 사항

- Windows 10/11 x64
- Visual Studio 2022 (플랫폼 도구 집합 v143, C++17)
- Windows 10 SDK
- Python 3 (선택 — 프로젝트 필터 정렬 훅 사용 시)

PhysX, FMOD, DirectXTK, DirectXTex, Effects11 등 서드파티 라이브러리는 `ThirdPartyLib/` 아래에 미리 빌드된 형태로 포함되어 있습니다.

### 순서

```bat
:: 1. 솔루션 열기
AAA\Framework.sln

:: 2. 구성 선택: Debug|x64 또는 Release|x64

:: 3. Engine 빌드 → SDK 배포 → 나머지 빌드
::    (UpdateLib.bat 이 헤더/lib/DLL/셰이더를 각 EXE 프로젝트로 복사합니다)
cd AAA
UpdateLib.bat Debug

:: 4. GameContent 빌드 → UpdateLib 재실행 → Launcher 및 툴 빌드
UpdateLib.bat Debug
```

빌드 순서는 `Engine` → `GameContent` → `Launcher` / `Editor` / `MapTool` / `AnimUITool` / `CameraTool` 입니다.

### git 훅 설정 (선택, 클론당 1회)

`.vcxproj.filters` 파일이 정렬되지 않은 채 커밋되는 것을 막습니다.

```bash
git config core.hooksPath .githooks
```

정렬이 필요하면 저장소 루트에서 `.\AAA\SortFilters.bat`을 실행합니다.

---

## 알려진 제약

- **리소스가 저장소에 포함되어 있지 않습니다.** 모델, 텍스처, 사운드, 레벨 데이터 등이 들어가는 `Resources/`는 저작권 및 용량 문제로 `.gitignore` 처리되어 있어, 이 저장소만 클론해서는 게임을 실행할 수 없습니다. 이 저장소는 **코드 열람 용도**입니다.
- 애님 이벤트는 베이스 레이어에서만 발화하며, 마스크 / 오버레이 레이어 클립의 이벤트는 지원하지 않습니다.
- 애님 이벤트 시간 기준이 정규화 progress이므로 역재생 시 오작동할 수 있습니다 (현재 역재생 사용처 없음).
- 렌더링과 애니메이션 갱신이 싱글 스레드 구조라, 워커 스레드 병렬 평가로는 확장되지 않습니다.

---

## 고지

- 이 프로젝트의 **컴포넌트 기반 엔진 프레임워크 뼈대는 쥬신게임아카데미 교육 과정에서 제공된 수업 코드**이며, 위에 기술한 내용은 그 위에 팀이 설계하고 확장한 결과물입니다.
- **비상업적 학습 목적의 팬 프로젝트**입니다. `별의 커비` 및 관련 캐릭터의 모든 권리는 닌텐도 및 HAL laboratory에 있습니다. 원작 에셋은 저장소에 포함되어 있지 않으며 배포하지 않습니다.
- 서드파티 라이브러리는 각자의 라이선스를 따릅니다.
