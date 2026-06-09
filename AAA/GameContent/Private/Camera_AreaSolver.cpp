#include "Camera_AreaData.h"
#include "GameInstance.h"   // json (nlohmann)
#include <fstream>

USING(Client)

// 로드
_bool CAreaCameraSolver::Load(const wstring& path)
{
    std::ifstream fin(path);
    if (!fin.is_open()) { MSG_BOX("Camera data not found"); return false; }
    json j; fin >> j;

    auto V3 = [](const json& a) { return _float3(a[0].get<_float>(), a[1].get<_float>(), a[2].get<_float>()); };
    auto Q4 = [](const json& a) { return _float4(a[0].get<_float>(), a[1].get<_float>(), a[2].get<_float>(),
        a[3].get<_float>()); };
    auto RD = [&](const json& o) {
        CAM_OFFSET p;
        p.targetOffs = V3(o["targetOffs"]); p.snapCenter = V3(o["snapCenter"]);
        p.snapRate = V3(o["snapRate"]); p.snapRot = Q4(o["snapRot"]);
        p.eyeSnapCenter = V3(o["eyeSnapCenter"]); p.eyeSnapRate = V3(o["eyeSnapRate"]); p.eyeSnapRot =
            Q4(o["eyeSnapRot"]);
        p.dist = o["distOffs"]; p.tilt = o["tiltOffs"]; p.rotY = o["rotYOffs"]; p.roll = o["rollOffs"]; p.fovY =
            o["fovYOffs"];
        return p;
        };

    m_rails.clear(); m_areas.clear(); m_curArea = -1;
    for (auto& r : j["rails"]) {
        CAM_RAIL rail; rail.uid = r["uid"].get<_uint>(); rail.close = r["close"].get<bool>();
        for (auto& n : r["nodes"]) rail.nodes.push_back(V3(n));
        m_rails.push_back(rail);
    }
    for (auto& a : j["areas"]) {
        CAM_AREA A;
        A.mode = (a.value("mode", std::string("Normal")) == "FixedGazing") ? CAM_MODE::FIXED_GAZING :
            CAM_MODE::NORMAL;
        A.center = V3(a["center"]); A.size = V3(a["size"]); A.rot = Q4(a["rot"]);
        A.priority = a["priority"]; A.erpIn = a["erpIn"]; A.erpOut = a["erpOut"];
        A.useRail = a["useRail"]; A.railUid = a["railUid"].get<_uint>();
        A.eyeSnap = a["eyeSnap"]; A.targetSnap = a["targetSnap"];
        A.usePanLimit = a["usePanLimit"]; A.panCenter = a["panLimitCenter"]; A.panRange = a["panLimitRange"];
        A.base = RD(a["base"]); A.end = RD(a["end"]);
        m_areas.push_back(A);
    }
    return true;
}

// 영역/레일 헬퍼
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

_float CAreaCameraSolver::Progress_T(const CAM_AREA& A, _fvector vKirby) const
{
    if (A.useRail) {
        const CAM_RAIL* r = Find_Rail(A.railUid); if (r && r->nodes.size() >= 2) return
            Project_OnRail(r->nodes, vKirby);
    }
    int ax = (A.size.x >= A.size.y && A.size.x >= A.size.z) ? 0 : (A.size.y >= A.size.z ? 1 : 2);
    _float3 k; XMStoreFloat3(&k, vKirby);
    const _float* kk = &k.x; const _float* cc = &A.center.x; const _float* ss = &A.size.x;
    _float hs = ss[ax] * 0.5f;
    if (hs < 1e-3f) return 0.f;
    return min(max((kk[ax] - cc[ax]) / hs * 0.5f + 0.5f, 0.f), 1.f);
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

CAM_OFFSET CAreaCameraSolver::Lerp_Offset(const CAM_AREA& A, _float t)
{
    auto L3 = [&](const _float3& a, const _float3& b) { _float3 o; XMStoreFloat3(&o, XMVectorLerp(XMLoadFloat3(&a),
        XMLoadFloat3(&b), t)); return o; };
    auto LS = [&](_float a, _float b) { return a + (b - a) * t; };
    auto LQ = [&](const _float4& a, const _float4& b) { _float4 o; XMStoreFloat4(&o,
        XMQuaternionSlerp(XMLoadFloat4(&a), XMLoadFloat4(&b), t)); return o; };
    CAM_OFFSET P;
    P.targetOffs = L3(A.base.targetOffs, A.end.targetOffs);
    P.snapCenter = L3(A.base.snapCenter, A.end.snapCenter);
    P.snapRate = L3(A.base.snapRate, A.end.snapRate);
    P.snapRot = LQ(A.base.snapRot, A.end.snapRot);
    P.eyeSnapCenter = L3(A.base.eyeSnapCenter, A.end.eyeSnapCenter);
    P.eyeSnapRate = L3(A.base.eyeSnapRate, A.end.eyeSnapRate);
    P.eyeSnapRot = LQ(A.base.eyeSnapRot, A.end.eyeSnapRot);
    P.dist = LS(A.base.dist, A.end.dist); P.tilt = LS(A.base.tilt, A.end.tilt);
    P.rotY = LS(A.base.rotY, A.end.rotY); P.roll = LS(A.base.roll, A.end.roll);
    P.fovY = LS(A.base.fovY, A.end.fovY);
    return P;
}

// 메인
CAM_POSE CAreaCameraSolver::Solve(_fvector vKirby)
{
    _int idx = Resolve_Area(vKirby);
    if (idx >= 0) m_curArea = idx;
    if (m_curArea < 0) {
        CAM_POSE p; XMStoreFloat3(&p.eye, XMVectorAdd(vKirby, XMVectorSet(0.f, 3.f, -14.f, 0.f)));
        p.fwd = _float3(0.f, 0.f, 1.f); return p;
    }
    const CAM_AREA& A = m_areas[m_curArea];
    const CAM_RAIL* rail = A.useRail ? Find_Rail(A.railUid) : nullptr;
    _float t = Progress_T(A, vKirby);
    CAM_OFFSET P = Lerp_Offset(A, t);
    return (A.mode == CAM_MODE::FIXED_GAZING) ? Solve_FixedGazing(A, P, vKirby, rail, t)
        : Solve_Normal(A, P, vKirby, rail, t);
}

// Normal: 트레일링 + lookAt(커비+up)  [RenderDoc 3캡쳐로 확정]
CAM_POSE CAreaCameraSolver::Solve_Normal(const CAM_AREA& A, const CAM_OFFSET& P,
    _fvector vKirby, const CAM_RAIL* rail, _float t) const
{
    // ★ 캡쳐로 맞춘 노브
    constexpr _float kFwdZ = 1.f;   // 진행축(월드 +Z) eyeSnapRot로 회전
    constexpr _float kBack = 14.f;  // 트레일링 거리 (캡쳐 13~15)
    constexpr _float kUpEye = 3.f;   // 눈 높이 오프셋 (캡쳐 2.7~3.4)
    constexpr _float kUpAim = 4.f;   // 조준점을 커비 위로 → 커비 하단프레이밍 (캡쳐 ~4)

    _vector q = XMLoadFloat4(&P.eyeSnapRot);
    _vector F = XMVector3Normalize(XMVector3Rotate(XMVectorSet(0.f, 0.f, kFwdZ, 0.f), q));
    _vector Fh = XMVector3Normalize(XMVectorSetY(F, 0.f));            // 수평 진행축

    // 눈: 커비 뒤로 트레일링 + 위로
    _vector eye = XMVectorAdd(
        XMVectorSubtract(vKirby, XMVectorScale(Fh, kBack + P.dist)),
        XMVectorSet(0.f, kUpEye, 0.f, 0.f));

    // 레일 있으면 '진행축 수직(측면)' 성분만 레일선에 핀
    if (rail && rail->nodes.size() >= 2) {
        _float3 rp = Eval_Rail(rail->nodes, Project_OnRail(rail->nodes, vKirby));
        _vector R = XMLoadFloat3(&rp);
        _vector side = XMVector3Normalize(XMVector3Cross(XMVectorSet(0.f, 1.f, 0.f, 0.f), Fh));
        _float dEye = XMVectorGetX(XMVector3Dot(eye, side));
        _float dRail = XMVectorGetX(XMVector3Dot(R, side));
        eye = XMVectorAdd(eye, XMVectorScale(side, dRail - dEye));
    }

    // 조준: 커비 머리 위 → 커비가 화면 하단
    _vector aim = XMVectorAdd(vKirby, XMVectorSet(0.f, kUpAim, 0.f, 0.f));

    // 저작 yaw lean (area8류) 데이터 rotY
    _float yaw = XMConvertToRadians(P.rotY);
    _vector look = XMVector3Normalize(
        XMVector3TransformNormal(XMVectorSubtract(aim, eye), XMMatrixRotationY(yaw)));

    CAM_POSE pose;
    XMStoreFloat3(&pose.eye, eye);
    XMStoreFloat3(&pose.fwd, look);
    pose.up = _float3(0.f, 1.f, 0.f);
    pose.fov = 50.f + P.fovY;
    return pose;
}

// FixedGazing: 진짜 lookAt(snapCenter)
CAM_POSE CAreaCameraSolver::Solve_FixedGazing(const CAM_AREA& A, const CAM_OFFSET& P,
    _fvector vKirby, const CAM_RAIL* rail, _float t) const
{
    CAM_POSE pose;
    _vector eye;
    if (rail && rail->nodes.size() >= 2) { _float3 rp = Eval_Rail(rail->nodes, t); eye = XMLoadFloat3(&rp); }
    else {
        _vector c = XMLoadFloat3(&P.eyeSnapCenter), r = XMLoadFloat3(&P.eyeSnapRate);
        eye = XMVectorMultiplyAdd(XMVectorSubtract(vKirby, c), r, c);
    }
    _vector sc = XMLoadFloat3(&P.snapCenter), sr = XMLoadFloat3(&P.snapRate);
    _vector aim = XMVectorAdd(XMVectorMultiplyAdd(XMVectorSubtract(vKirby, sc), sr, sc), XMLoadFloat3(&P.targetOffs));
    _vector look = XMVectorSubtract(aim, eye);
    if (XMVectorGetX(XMVector3LengthSq(look)) < 1e-4f) look = XMVectorSet(0.f, 0.f, 1.f, 0.f);
    look = XMVector3Normalize(look);
    XMStoreFloat3(&pose.eye, eye); XMStoreFloat3(&pose.fwd, look);
    pose.up = _float3(0.f, 1.f, 0.f); pose.fov = 50.f + P.fovY;
    return pose;
}