#include "Camera_AreaData.h"
#include "GameInstance.h"   // json (nlohmann)
#include <sstream>
#include <fstream>

USING(Client)

static _float3 RV3(const json& a) { return _float3(a[0].get<_float>(), a[1].get<_float>(), a[2].get<_float>()); }
static json    WV3(const _float3& v) { return json::array({ v.x, v.y, v.z }); }
static _float4 RV4(const json& a) {
    return _float4(a[0].get<_float>(), a[1].get<_float>(), a[2].get<_float>(),
        a[3].get<_float>());
}
static json    WV4(const _float4& v) { return json::array({ v.x, v.y, v.z, v.w }); }

static CAM_FRAME RD_Frame(const json& o)
{
    CAM_FRAME f;
    if (!o.is_object()) return f;
    f.yaw = o.value("yaw", f.yaw);
    f.pitch = o.value("pitch", f.pitch);
    f.distance = o.value("distance", f.distance);
    f.height = o.value("height", f.height);
    f.fov = o.value("fov", f.fov);
    f.aimHeight = o.value("aimHeight", f.aimHeight);
    if (o.contains("gazePoint")) f.gazePoint = RV3(o["gazePoint"]);
    f.gazeBlend = o.value("gazeBlend", f.gazeBlend);
    return f;
}
static json WR_Frame(const CAM_FRAME& f)
{
    json o;
    o["yaw"] = f.yaw; o["pitch"] = f.pitch; o["distance"] = f.distance;
    o["height"] = f.height; o["fov"] = f.fov; o["aimHeight"] = f.aimHeight;
    o["gazePoint"] = WV3(f.gazePoint); o["gazeBlend"] = f.gazeBlend;
    return o;
}

_bool CAreaCameraSolver::Load(const wstring& path)
{
    std::ifstream fin(path);
    if (!fin.is_open()) { MSG_BOX("Camera data not found"); return false; }
    std::stringstream ss; ss << fin.rdbuf();
    json j;
    try { j = json::parse(ss.str()); }
    catch (...) { MSG_BOX("Camera data parse failed"); return false; }

    Clear();

    if (j.contains("rails"))
        for (auto& r : j["rails"]) {
            CAM_RAIL rail;
            rail.uid = (_uint)r.value("uid", (int64_t)0);
            rail.close = r.value("close", false);
            if (r.contains("nodes")) for (auto& n : r["nodes"]) rail.nodes.push_back(RV3(n));
            m_rails.push_back(rail);
        }

    if (j.contains("areas"))
        for (auto& a : j["areas"]) {
            CAM_AREA A;
            if (a.contains("center")) A.center = RV3(a["center"]);
            if (a.contains("size"))   A.size = RV3(a["size"]);
            if (a.contains("rot"))    A.rot = RV4(a["rot"]);
            A.priority = a.value("priority", 0);
            A.useRail = a.value("useRail", false);
            A.railUid = (_uint)a.value("railUid", (int64_t)0);
            if (a.contains("scrollDead")) A.scrollDead = RV3(a["scrollDead"]);
            A.smoothBase = a.value("smoothBase", 0.35f);
            A.gazeMode = a.value("gazeMode", 0);
            A.gazeTag = a.value("gazeTag", string());
            A.frame = RD_Frame(a.contains("frame") ? a["frame"] : json::object());
            A.frameEnd = a.contains("frameEnd") ? RD_Frame(a["frameEnd"]) : A.frame;
            m_areas.push_back(A);
        }
    return true;
}

_bool CAreaCameraSolver::Save(const wstring& path) const
{
    json j;
    json rails = json::array();
    for (auto& R : m_rails) {
        json r; r["uid"] = R.uid; r["close"] = R.close;
        json nodes = json::array();
        for (auto& n : R.nodes) nodes.push_back(WV3(n));
        r["nodes"] = nodes;
        rails.push_back(r);
    }
    j["rails"] = rails;

    json areas = json::array();
    for (auto& A : m_areas) {
        json a;
        a["center"] = WV3(A.center); a["size"] = WV3(A.size); a["rot"] = WV4(A.rot);
        a["priority"] = A.priority;
        a["useRail"] = A.useRail; a["railUid"] = A.railUid;
        a["scrollDead"] = WV3(A.scrollDead);
        a["smoothBase"] = A.smoothBase;
        a["gazeMode"] = A.gazeMode; a["gazeTag"] = A.gazeTag;
        a["frame"] = WR_Frame(A.frame);
        a["frameEnd"] = WR_Frame(A.frameEnd);
        areas.push_back(a);
    }
    j["areas"] = areas;

    std::ofstream fout(path);
    if (!fout.is_open()) { MSG_BOX("Camera doc save failed"); return false; }
    fout << j.dump(1);
    return true;
}

const CAM_RAIL* CAreaCameraSolver::Find_Rail(_uint uid) const
{
    for (auto& r : m_rails) if (r.uid == uid) return &r;
    return nullptr;
}

_bool CAreaCameraSolver::Point_In_OBB(_fvector p, const CAM_AREA& A)
{
    _vector c = XMLoadFloat3(&A.center);
    _vector q = XMLoadFloat4(&A.rot);
    _vector local = XMVector3Rotate(XMVectorSubtract(p, c), XMQuaternionInverse(q));
    _float3 l, h; XMStoreFloat3(&l, local);
    XMStoreFloat3(&h, XMVectorScale(XMLoadFloat3(&A.size), 0.5f));
    const _float pad = 1.f;
    return fabsf(l.x) <= h.x + pad && fabsf(l.y) <= h.y + pad && fabsf(l.z) <= h.z + pad;
}

_int CAreaCameraSolver::Resolve_Area(_fvector vKirby) const
{
    _int best = -1;
    for (_int i = 0; i < (_int)m_areas.size(); ++i)
        if (Point_In_OBB(vKirby, m_areas[i]) && (best < 0 || m_areas[i].priority > m_areas[best].priority))
            best = i;
    return best;
}

_float3 CAreaCameraSolver::Eval_Rail(const vector<_float3>& nodes, _float t)
{
    if (nodes.size() < 2) return nodes.empty() ? _float3{} : nodes[0];
    _uint last = (_uint)nodes.size() - 1;
    _float f = t * last; _uint i = min((_uint)f, last - 1); _float u = f - i;
    _float3 o; XMStoreFloat3(&o, XMVectorLerp(XMLoadFloat3(&nodes[i]), XMLoadFloat3(&nodes[i + 1]), u));
    return o;
}

_float CAreaCameraSolver::Project_OnRail(const vector<_float3>& nodes, _fvector vPos)
{
    if (nodes.size() < 2) return 0.f;
    _uint seg = (_uint)nodes.size() - 1; _float bestT = 0.f, bestD = FLT_MAX;
    for (_uint i = 0; i < seg; ++i) {
        _vector a = XMLoadFloat3(&nodes[i]), b = XMLoadFloat3(&nodes[i + 1]);
        _vector ab = XMVectorSubtract(b, a);
        _float len2 = XMVectorGetX(XMVector3LengthSq(ab));
        _float u = (len2 > 1e-6f) ? XMVectorGetX(XMVector3Dot(XMVectorSubtract(vPos, a), ab)) / len2 : 0.f;
        u = min(max(u, 0.f), 1.f);
        _vector pt = XMVectorAdd(a, XMVectorScale(ab, u));
        _float d = XMVectorGetX(XMVector3LengthSq(XMVectorSubtract(vPos, pt)));
        if (d < bestD) { bestD = d; bestT = (i + u) / seg; }
    }
    return bestT;
}

CAM_FRAME CAreaCameraSolver::Lerp_Frame(const CAM_FRAME& a, const CAM_FRAME& b, _float t)
{
    auto L = [&](_float x, _float y) { return x + (y - x) * t; };
    auto L3 = [&](const _float3& x, const _float3& y) { _float3 o;
    XMStoreFloat3(&o, XMVectorLerp(XMLoadFloat3(&x), XMLoadFloat3(&y), t)); return o; };

    CAM_FRAME f;
    f.yaw = L(a.yaw, b.yaw);
    f.pitch = L(a.pitch, b.pitch);
    f.distance = L(a.distance, b.distance);
    f.height = L(a.height, b.height);
    f.fov = L(a.fov, b.fov);
    f.aimHeight = L(a.aimHeight, b.aimHeight);
    f.gazePoint = L3(a.gazePoint, b.gazePoint);
    f.gazeBlend = L(a.gazeBlend, b.gazeBlend);
    return f;
}

void CAreaCameraSolver::Update(_fvector vKirby, _float /*dt*/)
{
    _int idx = Resolve_Area(vKirby);
    if (idx >= 0 && idx != m_curArea) m_scrollInit = false;   // 새 영역 -> 스크롤 앵커 스냅
    if (idx >= 0) m_curArea = idx;

    if (m_curArea < 0) {
        CAM_POSE p; XMStoreFloat3(&p.eye, XMVectorAdd(vKirby, XMVectorSet(0.f, 3.f, -14.f, 0.f)));
        p.fwd = _float3(0.f, 0.f, 1.f);
        m_lastPose = p; return;
    }
    m_lastPose = Solve_Area(m_areas[m_curArea], vKirby);
}

// 클린 팔로우 캠: 항상 lookAt(커비), 각도 손세팅, 스크롤 윈도우, 레일 진행도 프레임 보간
CAM_POSE CAreaCameraSolver::Solve_Area(const CAM_AREA& A, _fvector vKirby) const
{
    const CAM_RAIL* rail = A.useRail ? Find_Rail(A.railUid) : nullptr;

    // 레일 영역: 커비가 레일 0~1 범위를 벗어나면 카메라 정지(직전 포즈 유지)
    if (rail && rail->nodes.size() >= 2 && m_scrollInit && Kirby_Beyond_Rail(*rail, vKirby))
        return m_lastPose;

    // 스크롤 윈도우(월드축 데드존): 안이면 정지, 가장자리 밀면 스크롤
    if (!m_scrollInit) { XMStoreFloat3(&m_scrollPos, vKirby); m_scrollInit = true; }
    else {
        _vector anchor = XMLoadFloat3(&m_scrollPos);
        _float3 d; XMStoreFloat3(&d, XMVectorSubtract(vKirby, anchor));
        auto push = [](_float v, _float dead) { return v > dead ? v - dead : (v < -dead ? v + dead : 0.f); };
        d.x = push(d.x, A.scrollDead.x); d.y = push(d.y, A.scrollDead.y); d.z = push(d.z, A.scrollDead.z);
        XMStoreFloat3(&m_scrollPos, XMVectorAdd(anchor, XMLoadFloat3(&d)));
    }
    _vector followPt = XMLoadFloat3(&m_scrollPos);

    // 눈 기준점 + 프레이밍: 레일이면 진행도 t로 frame→frameEnd 보간, 눈은 레일점
    _vector eyeBase = followPt;
    CAM_FRAME F = A.frame;
    m_curT = 0.f;
    if (rail && rail->nodes.size() >= 2) {
        _float t = Project_OnRail(rail->nodes, followPt);
        _float3 rp = Eval_Rail(rail->nodes, t);
        eyeBase = XMLoadFloat3(&rp);
        F = Lerp_Frame(A.frame, A.frameEnd, t);
        m_curT = t;
    }

    // back = 타깃 뒤 방향: yaw 수평회전 + pitch 들어올림
    _float yawR = XMConvertToRadians(F.yaw);
    _float pitR = XMConvertToRadians(F.pitch);
    _vector horizBack = XMVector3Rotate(XMVectorSet(0.f, 0.f, -1.f, 0.f),
        XMQuaternionRotationAxis(XMVectorSet(0.f, 1.f, 0.f, 0.f), yawR));
    _vector back = XMVectorAdd(XMVectorScale(horizBack, cosf(pitR)),
        XMVectorScale(XMVectorSet(0.f, 1.f, 0.f, 0.f), sinf(pitR)));

    _vector eye = XMVectorAdd(XMVectorAdd(eyeBase, XMVectorScale(back, F.distance)),
        XMVectorSet(0.f, F.height, 0.f, 0.f));

    _vector aim = XMVectorAdd(followPt, XMVectorSet(0.f, F.aimHeight, 0.f, 0.f));
    if (A.gazeMode != 0) {
        _vector target = (A.gazeMode == 2 && m_gazeOverrideValid)
            ? XMLoadFloat3(&m_gazeOverride)     // Object: 런타임 주입 위치
            : XMLoadFloat3(&F.gazePoint);       // Point, 또는 미해석 Object -> authored point
        aim = XMVectorLerp(aim, target, F.gazeBlend);
    }

    _vector fwd = XMVectorSubtract(aim, eye);
    if (XMVectorGetX(XMVector3LengthSq(fwd)) < 1e-6f) fwd = XMVectorSet(0.f, 0.f, 1.f, 0.f);
    fwd = XMVector3Normalize(fwd);

    CAM_POSE pose;
    XMStoreFloat3(&pose.eye, eye);
    XMStoreFloat3(&pose.fwd, fwd);
    pose.up = _float3(0.f, 1.f, 0.f);
    pose.fov = F.fov;
    return pose;
}

_bool CAreaCameraSolver::Kirby_Beyond_Rail(const CAM_RAIL& R, _fvector kirby)
{
    if (R.close || R.nodes.size() < 2) return false;
    const _uint n = (_uint)R.nodes.size();
    // 끝점 너머
    _vector eEnd = XMLoadFloat3(&R.nodes[n - 1]);
    _vector dEnd = XMVector3Normalize(XMVectorSubtract(eEnd, XMLoadFloat3(&R.nodes[n - 2])));
    if (XMVectorGetX(XMVector3Dot(XMVectorSubtract(kirby, eEnd), dEnd)) > 0.f) return true;
    // 시작점 이전
    _vector eStart = XMLoadFloat3(&R.nodes[0]);
    _vector dStart = XMVector3Normalize(XMVectorSubtract(eStart, XMLoadFloat3(&R.nodes[1])));
    if (XMVectorGetX(XMVector3Dot(XMVectorSubtract(kirby, eStart), dStart)) > 0.f) return true;
    return false;
}
