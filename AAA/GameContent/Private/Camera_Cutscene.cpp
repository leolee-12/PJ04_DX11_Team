#include "Camera_Cutscene.h"
#include "GameInstance.h"
#include "Animator.h"
#include <fstream>

// ---- CAM_TRACK ----
HRESULT CAM_TRACK::Load(const wstring& path)
{
    ifstream in(path);
    if (!in.is_open()) return E_FAIL;

    json j; in >> j;
    frameCount = j.value("frameCount", 0);
    fovY = j.value("fov", 0.66f);
    aim = j.value("aim", true);

    eye.clear(); at.clear();
    for (auto& fr : j["frames"])
    {
        auto& e = fr["eye"]; eye.push_back(_float3(e[0], e[1], e[2]));
        auto& a = fr["aim"]; at.push_back(_float3(a[0], a[1], a[2]));
    }
    if (frameCount == 0) frameCount = (_int)eye.size();
    return eye.empty() ? E_FAIL : S_OK;
}

void CAM_TRACK::Sample(_float progress, _float3& outEye, _float3& outAt) const
{
    if (eye.empty()) return;
    _float f = progress * (eye.size() - 1);
    if (f < 0.f) f = 0.f;
    _int i = (_int)f;
    if (i >= (_int)eye.size() - 1) { outEye = eye.back(); outAt = at.back(); return; }
    _float a = f - i;
    XMStoreFloat3(&outEye, XMVectorLerp(XMLoadFloat3(&eye[i]), XMLoadFloat3(&eye[i + 1]), a));
    XMStoreFloat3(&outAt, XMVectorLerp(XMLoadFloat3(&at[i]), XMLoadFloat3(&at[i + 1]), a));
}

// ---- CCamera_Cutscene ----
CCamera_Cutscene::CCamera_Cutscene(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CCamera(pDevice, pContext) {
}
CCamera_Cutscene::CCamera_Cutscene(const CCamera_Cutscene& Prototype)
    : CCamera(Prototype) {
}

HRESULT CCamera_Cutscene::Initialize(void* pArg)
{
    if (FAILED(__super::Initialize(pArg))) return E_FAIL;
    m_bActive = false;     // 대기(비활성)로 시작
    return S_OK;
}

_bool CCamera_Cutscene::Play_Track(const _tchar* szTrack, CAnimator* pProgress, const _float4x4* pAnchorWorld)
{
    if (!szTrack) return false;
    wstring name = szTrack;

    auto it = m_Tracks.find(name);
    if (it == m_Tracks.end())
    {
        CAM_TRACK t{};
        if (FAILED(t.Load(m_strDir + name + L".json")))
        {
            MSG_BOX("CCamera_Cutscene: track load fail");
            return false;
        }
        it = m_Tracks.emplace(name, move(t)).first;
    }
    m_pCur = &it->second;
    m_pProgress = pProgress;
    m_pAnchor = pAnchorWorld;
    return true;
}

void CCamera_Cutscene::Priority_Update(_float fTimeDelta)
{
    if (!m_bActive) return;

    if (m_pCur && m_pProgress && m_pAnchor)
    {
        _float prog = m_pProgress->Get_Progress();
        _float3 eyeL, atL;
        m_pCur->Sample(prog, eyeL, atL);
        eyeL.x = -eyeL.x; 
        atL.x = -atL.x;

        _matrix W = XMLoadFloat4x4(m_pAnchor);
        _vector eyeW = XMVector3TransformCoord(XMLoadFloat3(&eyeL), W);
        _vector atW = XMVector3TransformCoord(XMLoadFloat3(&atL), W);

        _vector vLook = XMVector3Normalize(XMVectorSubtract(atW, eyeW));
        _vector vRight = XMVector3Normalize(XMVector3Cross(XMVectorSet(0.f, 1.f, 0.f, 0.f), vLook));
        _vector vUp = XMVector3Cross(vLook, vRight);

        _matrix CamWorld;
        CamWorld.r[0] = XMVectorSetW(vRight, 0.f);
        CamWorld.r[1] = XMVectorSetW(vUp, 0.f);
        CamWorld.r[2] = XMVectorSetW(vLook, 0.f);
        CamWorld.r[3] = XMVectorSetW(eyeW, 1.f);
        m_pTransformCom->Set_WorldMatrix(CamWorld);

        m_fFovy = m_pCur->fovY;
        Recalculate_ProjMatrix();
    }

    __super::Priority_Update(fTimeDelta);   // 파이프라인 기록 + 섀도우핏
}

CCamera_Cutscene* CCamera_Cutscene::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CCamera_Cutscene* p = new CCamera_Cutscene(pDevice, pContext);
    if (FAILED(p->Initialize_Prototype())) { MSG_BOX("Failed to Created : CCamera_Cutscene"); Safe_Release(p); }
    return p;
}
CGameObject* CCamera_Cutscene::Clone(void* pArg)
{
    CCamera_Cutscene* p = new CCamera_Cutscene(*this);
    if (FAILED(p->Initialize(pArg))) { MSG_BOX("Failed to Cloned : CCamera_Cutscene"); Safe_Release(p); }
    return p;
}
void CCamera_Cutscene::Free() { __super::Free(); }