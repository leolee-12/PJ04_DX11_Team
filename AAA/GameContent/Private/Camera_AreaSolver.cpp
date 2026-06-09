#include "Camera_AreaData.h"
#include "GameInstance.h"   // json (nlohmann)
#include <sstream>
#include <fstream>

USING(Client)

static _float3 RV3(const json& a) { return _float3(a[0].get<_float>(), a[1].get<_float>(), -a[2].get<_float>()); }
static json    WV3(const _float3& v) { return json::array({ v.x, v.y, -v.z }); }
static _float4 RQ4(const json& a) {
    return _float4(-a[0].get<_float>(), -a[1].get<_float>(), a[2].get<_float>(),
        a[3].get<_float>());
}
static json    WQ4(const _float4& v) { return json::array({ -v.x, -v.y, v.z, v.w }); }
static _float3 RV3R(const json& a) { return _float3(a[0].get<_float>(), a[1].get<_float>(), a[2].get<_float>()); }
// raw (size/rate)
static json    WV3R(const _float3& v) { return json::array({ v.x, v.y, v.z }); }

static CAM_OFFSET RD_Offset(const json& o)
{
    CAM_OFFSET p;
    if (!o.is_object()) return p;
    if (o.contains("TargetOffs"))    p.targetOffs = RV3(o["TargetOffs"]);
    if (o.contains("SnapCenter"))    p.snapCenter = RV3(o["SnapCenter"]);
    if (o.contains("SnapRate"))      p.snapRate = RV3R(o["SnapRate"]);
    if (o.contains("SnapRot"))       p.snapRot = RQ4(o["SnapRot"]);
    if (o.contains("EyeSnapCenter")) p.eyeSnapCenter = RV3(o["EyeSnapCenter"]);
    if (o.contains("EyeSnapRate"))   p.eyeSnapRate = RV3R(o["EyeSnapRate"]);
    if (o.contains("EyeSnapRot"))    p.eyeSnapRot = RQ4(o["EyeSnapRot"]);
    p.dist = o.value("DistOffs", 0.f);
    p.tilt = -o.value("TiltOffs", 0.f);
    p.rotY = -o.value("RotYOffs", 0.f);
    p.roll = -o.value("RollOffs", 0.f);
    p.fovY = o.value("FovYOffs", 0.f);
    p.dofFarDepth = o.value("DofFarDepthOffs", 0.f);
    p.dofFarRate = o.value("DofFarRateOffs", 0.f);
    return p;
}

static void WR_Offset(const CAM_OFFSET& p, json& o)   // write into existing node (preserve unknown keys)
{
    o["TargetOffs"] = WV3(p.targetOffs);   o["SnapCenter"] = WV3(p.snapCenter);
    o["SnapRate"] = WV3R(p.snapRate);    o["SnapRot"] = WQ4(p.snapRot);
    o["EyeSnapCenter"] = WV3(p.eyeSnapCenter); o["EyeSnapRate"] = WV3R(p.eyeSnapRate);
    o["EyeSnapRot"] = WQ4(p.eyeSnapRot);
    o["DistOffs"] = p.dist;  o["TiltOffs"] = -p.tilt;  o["RotYOffs"] = -p.rotY;
    o["RollOffs"] = -p.roll; o["FovYOffs"] = p.fovY;
    o["DofFarDepthOffs"] = p.dofFarDepth; o["DofFarRateOffs"] = p.dofFarRate;
}

static string New_Key(const json& obj)   // max integer key + 1
{
    _int mx = -1;
    for (auto it = obj.begin(); it != obj.end(); ++it) { try { mx = max(mx, std::stoi(it.key())); } catch (...) {} }
    return std::to_string(mx + 1);
}

// 로드
_bool CAreaCameraSolver::Load(const wstring& path)
{
    std::ifstream fin(path);
    if (!fin.is_open()) { MSG_BOX("Camera data not found"); return false; }
    std::stringstream ss; ss << fin.rdbuf(); m_docText = ss.str();

    json j;
    try { j = json::parse(m_docText); }
    catch (...) { MSG_BOX("Camera data parse failed"); return false; }

    m_rails.clear(); m_areas.clear(); m_curArea = -1; m_railEngaged = false;

    if (j.contains("Custom"))
        for (auto it = j["Custom"].begin(); it != j["Custom"].end(); ++it)
        {
            const json& r = it.value();
            if (!r.is_object() || r.value("Kind", std::string()) != "Rail") continue;
            CAM_RAIL rail;
            rail.srcKey = it.key();
            rail.uid = (_uint)r.value("Uid", (int64_t)0);
            rail.close = r.value("IsClose", false);
            rail.radius = r.value("Radius", 2.f);
            rail.bezierCtrlLen = r.value("BezierControlLength", 5.f);
            if (r.contains("CenterPos")) rail.centerPos = RV3(r["CenterPos"]);
            rail.clockwise = r.value("IsClockwise", false);
            rail.kind = r.value("Kind", string("Rail"));
            rail.erpType = r.value("ErpType", string("Line"));
            rail.objectName = r.value("Basic.ObjectName", string("Rail"));
            if (r.contains("Node")) for (auto& n : r["Node"]) rail.nodes.push_back(RV3(n["Pos"]));
            m_rails.push_back(rail);
        }

    if (j.contains("Standard"))
        for (auto it = j["Standard"].begin(); it != j["Standard"].end(); ++it)
        {
            const json& c = it.value();
            if (!c.contains("Gimmick.AreaCamera.MainComponent")) continue;
            const json& mc = c["Gimmick.AreaCamera.MainComponent"];
            CAM_AREA A;
            A.srcKey = it.key();
            A.mode = (mc.value("AreaCameraMode", string("Normal")) == "FixedGazing") ? CAM_MODE::FIXED_GAZING :
                CAM_MODE::NORMAL;
            A.center = RV3(mc["AreaCenter"]); A.size = RV3R(mc["AreaSize"]);
            if (mc.contains("AreaRot")) A.rot = RQ4(mc["AreaRot"]);
            A.priority = mc.value("Priority", 0);
            A.erpIn = mc.value("ErpFrameIn", 120.f); A.erpOut = mc.value("ErpFrameOut", 120.f);
            A.useRail = mc.value("UseRail", false);
            A.eyeSnap = mc.value("IsEyeSnap", false); A.targetSnap = mc.value("IsTargetSnap", false);
            A.usePanLimit = mc.value("UsePanLimit", false);
            A.panCenter = mc.value("PanLimitCenter", 180.f); A.panRange = mc.value("PanLimitRange", 360.f);
            A.shapeKind = mc.value("AreaShapeKind", string("Cube"));
            A.kind = mc.value("Kind", string("AreaCamera"));
            A.checkMargin = mc.value("AreaCheckMargin", 2.f);
            A.useCustomMargin = mc.value("UseCustomAreaCheckMargin", false);
            A.reactPreWait = mc.value("ReactPreWaitFrame", 0);
            A.initActiveForEvent = mc.value("IsInitActiveForUseEvent", false);
            A.useDof = mc.value("UseDofParam", false);
            A.useEventTrigger = mc.value("UseEventTrigger", false);
            if (mc.contains("SnapCenter"))    A.mcSnapCenter = RV3(mc["SnapCenter"]);
            if (mc.contains("SnapRot"))       A.mcSnapRot = RQ4(mc["SnapRot"]);
            if (mc.contains("EyeSnapCenter")) A.mcEyeSnapCenter = RV3(mc["EyeSnapCenter"]);
            if (mc.contains("EyeSnapRot"))    A.mcEyeSnapRot = RQ4(mc["EyeSnapRot"]);
            if (c.contains("Basic.RailUser"))
            {
                A.railUid = (_uint)c["Basic.RailUser"].value("TargetRailUid", (int64_t)0);
                A.railNodeIndex = c["Basic.RailUser"].value("NodeIndex", 0);
            }
            A.objectName = c.value("Basic.ObjectName", string("AreaCamera"));
            if (c.contains("Gimmick.AreaCamera.CameraOffsetParamBase")) A.base =
                RD_Offset(c["Gimmick.AreaCamera.CameraOffsetParamBase"]);
            if (c.contains("Gimmick.AreaCamera.CameraOffsetParamEnd"))  A.end =
                RD_Offset(c["Gimmick.AreaCamera.CameraOffsetParamEnd"]);
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
    if (idx >= 0 && idx != m_curArea) m_railEngaged = false;   // entered new area -> drop frozen angle
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

_bool CAreaCameraSolver::Save(const wstring& path) const
{
    json j;
    try { j = m_docText.empty() ? json::object() : json::parse(m_docText); }
    catch (...) { j = json::object(); }
    if (!j.contains("Custom") || !j["Custom"].is_object())   j["Custom"] = json::object();
    if (!j.contains("Standard") || !j["Standard"].is_object()) j["Standard"] = json::object();

    const json oldCustom = j["Custom"];
    const json oldStandard = j["Standard"];

    // templates for newly added items (clone an existing one if possible)
    json railTmpl = (!oldCustom.empty()) ? oldCustom.begin().value() : json::object();
    json areaTmpl = json::object();
    for (auto it = oldStandard.begin(); it != oldStandard.end(); ++it)
        if (it.value().contains("Gimmick.AreaCamera.MainComponent")) { areaTmpl = it.value(); break; }

    // --- rails: Custom is entirely camera-owned -> rebuild from m_rails ---
    json newCustom = json::object();
    for (auto it = oldCustom.begin(); it != oldCustom.end(); ++it)
        if (!(it.value().is_object() && it.value().value("Kind", std::string()) == "Rail"))
            newCustom[it.key()] = it.value();   // preserve CameraLimitCollision etc.

    for (size_t i = 0; i < m_rails.size(); ++i)
    {
        const CAM_RAIL& R = m_rails[i];
        string key = R.srcKey.empty() ? New_Key(newCustom) : R.srcKey;
        json node = oldCustom.contains(key) ? oldCustom[key] : railTmpl;
        node["Uid"] = R.uid; node["IsClose"] = R.close; node["Radius"] = R.radius;
        node["BezierControlLength"] = R.bezierCtrlLen; node["CenterPos"] = WV3(R.centerPos);
        node["IsClockwise"] = R.clockwise; node["Kind"] = R.kind; node["ErpType"] = R.erpType;
        node["Basic.ObjectName"] = R.objectName; node["NodeCount"] = (int)R.nodes.size();
        const json oldNodes = (oldCustom.contains(key) && oldCustom[key].contains("Node"))
            ? oldCustom[key]["Node"] : json::array();
        json arr = json::array();
        for (size_t ni = 0; ni < R.nodes.size(); ++ni)
        {
            json nn = (ni < oldNodes.size()) ? oldNodes[ni] : json::object();
            nn["Pos"] = WV3(R.nodes[ni]);
            arr.push_back(nn);
        }
        node["Node"] = arr;
        newCustom[key] = node;
    }
    j["Custom"] = newCustom;

    // --- areas: keep non-camera Standard entries, rebuild camera ones from m_areas ---
    json newStandard = json::object();
    for (auto it = oldStandard.begin(); it != oldStandard.end(); ++it)
        if (!it.value().contains("Gimmick.AreaCamera.MainComponent"))
            newStandard[it.key()] = it.value();   // preserve passthrough entries

    for (size_t i = 0; i < m_areas.size(); ++i)
    {
        const CAM_AREA& A = m_areas[i];
        string key = A.srcKey.empty() ? New_Key(newStandard) : A.srcKey;
        json node = oldStandard.contains(key) ? oldStandard[key] : areaTmpl;

        json& mc = node["Gimmick.AreaCamera.MainComponent"];
        mc["AreaCameraMode"] = (A.mode == CAM_MODE::FIXED_GAZING) ? "FixedGazing" : "Normal";
        mc["AreaCenter"] = WV3(A.center); mc["AreaSize"] = WV3R(A.size); mc["AreaRot"] = WQ4(A.rot);
        mc["Priority"] = A.priority; mc["ErpFrameIn"] = A.erpIn; mc["ErpFrameOut"] = A.erpOut;
        mc["UseRail"] = A.useRail; mc["IsEyeSnap"] = A.eyeSnap; mc["IsTargetSnap"] = A.targetSnap;
        mc["UsePanLimit"] = A.usePanLimit; mc["PanLimitCenter"] = A.panCenter; mc["PanLimitRange"] = A.panRange;
        mc["AreaShapeKind"] = A.shapeKind; mc["Kind"] = A.kind; mc["AreaCheckMargin"] = A.checkMargin;
        mc["UseCustomAreaCheckMargin"] = A.useCustomMargin; mc["ReactPreWaitFrame"] = A.reactPreWait;
        mc["IsInitActiveForUseEvent"] = A.initActiveForEvent; mc["UseDofParam"] = A.useDof;
        mc["UseEventTrigger"] = A.useEventTrigger;
        mc["SnapCenter"] = WV3(A.mcSnapCenter); mc["SnapRot"] = WQ4(A.mcSnapRot);
        mc["EyeSnapCenter"] = WV3(A.mcEyeSnapCenter); mc["EyeSnapRot"] = WQ4(A.mcEyeSnapRot);

        if (!node.contains("Basic.RailUser") || !node["Basic.RailUser"].is_object()) node["Basic.RailUser"] =
            json::object();
        node["Basic.RailUser"]["TargetRailUid"] = A.railUid;
        node["Basic.RailUser"]["NodeIndex"] = A.railNodeIndex;
        node["Basic.ObjectName"] = A.objectName;

        if (!node.contains("Gimmick.AreaCamera.CameraOffsetParamBase"))
            node["Gimmick.AreaCamera.CameraOffsetParamBase"] = json::object();
        if (!node.contains("Gimmick.AreaCamera.CameraOffsetParamEnd"))
            node["Gimmick.AreaCamera.CameraOffsetParamEnd"] = json::object();
        WR_Offset(A.base, node["Gimmick.AreaCamera.CameraOffsetParamBase"]);
        WR_Offset(A.end, node["Gimmick.AreaCamera.CameraOffsetParamEnd"]);

        newStandard[key] = node;
    }
    j["Standard"] = newStandard;

    std::ofstream fout(path);
    if (!fout.is_open()) { MSG_BOX("Camera doc save failed"); return false; }
    fout << j.dump(1);
    return true;
}

// Normal: 트레일링 + lookAt(커비+up)  [RenderDoc 3캡쳐로 확정]
CAM_POSE CAreaCameraSolver::Solve_Normal(const CAM_AREA& A, const CAM_OFFSET& P,
    _fvector vKirby, const CAM_RAIL* rail, _float t) const
{
    constexpr _float kFwdZ = 1.f;
    constexpr _float kBack = 14.f;
    constexpr _float kUpEye = 3.f;
    constexpr _float kUpAim = 4.f;
    constexpr _float kRailNear = 6.f;    // <= this dist from rail -> full rail
    constexpr _float kRailFar = 18.f;   // >= this dist -> free / frozen
    constexpr _float kEngage = 0.5f;   // w above this = "on rail" -> capture angle

    _vector q = XMLoadFloat4(&P.eyeSnapRot);
    _vector F = XMVector3Normalize(XMVector3Rotate(XMVectorSet(0.f, 0.f, kFwdZ, 0.f), q));
    _vector Fh = XMVector3Normalize(XMVectorSetY(F, 0.f));

    // base eye: trailing behind kirby
    _vector eye = XMVectorAdd(
        XMVectorSubtract(vKirby, XMVectorScale(Fh, kBack + P.dist)),
        XMVectorSet(0.f, kUpEye, 0.f, 0.f));

    // rail proximity weight (1 near rail, 0 far) + lateral pin scaled by it
    _float w = 0.f;
    if (rail && rail->nodes.size() >= 2)
    {
        _float3 rp = Eval_Rail(rail->nodes, Project_OnRail(rail->nodes, vKirby));
        _vector R = XMLoadFloat3(&rp);

        _float3 kf; XMStoreFloat3(&kf, vKirby);
        _float dx = kf.x - rp.x, dz = kf.z - rp.z;
        _float distXZ = sqrtf(dx * dx + dz * dz);
        _float u = (distXZ - kRailNear) / (kRailFar - kRailNear);
        u = (u < 0.f) ? 0.f : (u > 1.f ? 1.f : u);
        w = 1.f - (u * u * (3.f - 2.f * u));   // 1 near -> 0 far

        _vector side = XMVector3Normalize(XMVector3Cross(XMVectorSet(0.f, 1.f, 0.f, 0.f), Fh));
        _float dEye = XMVectorGetX(XMVector3Dot(eye, side));
        _float dRail = XMVectorGetX(XMVector3Dot(R, side));
        eye = XMVectorAdd(eye, XMVectorScale(side, (dRail - dEye) * w));
    }

    // live look: re-aims at kirby (+ optional yaw lean)
    _vector aim = XMVectorAdd(vKirby, XMVectorSet(0.f, kUpAim, 0.f, 0.f));
    _float  yaw = XMConvertToRadians(P.rotY);
    _vector liveLook = XMVector3Normalize(
        XMVector3TransformNormal(XMVectorSubtract(aim, eye), XMMatrixRotationY(yaw)));

    _vector finalEye = eye, finalLook = liveLook;

    if (rail && rail->nodes.size() >= 2)
    {
        // on rail: remember the current angle + eye-offset relative to kirby
        if (w >= kEngage)
        {
            XMStoreFloat3(&m_lastRailFwd, liveLook);
            XMStoreFloat3(&m_lastRailEyeOffset, XMVectorSubtract(eye, vKirby));
            m_railEngaged = true;
        }
        // off rail (w small): hold the frozen angle, dolly rigidly with kirby
        if (m_railEngaged)
        {
            _vector frozenEye = XMVectorAdd(vKirby, XMLoadFloat3(&m_lastRailEyeOffset));
            _vector frozenFwd = XMLoadFloat3(&m_lastRailFwd);
            finalEye = XMVectorLerp(frozenEye, eye, w);
            finalLook = XMVector3Normalize(XMVectorLerp(frozenFwd, liveLook, w));
        }
    }

    CAM_POSE pose;
    XMStoreFloat3(&pose.eye, finalEye);
    XMStoreFloat3(&pose.fwd, finalLook);
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