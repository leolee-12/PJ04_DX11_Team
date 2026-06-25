#include "CameraDirector.h"
#include "GameInstance.h"
#include "GameContrnt_Events.h"
#include "Camera_Cutscene.h"
#include "Camera_Boss.h"

CCameraDirector::CCameraDirector(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CGameObject(pDevice, pContext) {
}
CCameraDirector::CCameraDirector(const CCameraDirector& Prototype)
    : CGameObject(Prototype) {
}

HRESULT CCameraDirector::Initialize(void* pArg)
{
    return __super::Initialize(pArg);   // 내부에서 Ready_Events 호출
}

HRESULT CCameraDirector::Ready_Events()
{
    Subscribe_Event(EventTag::Cutscene_CameraChange, [this](void* p) { On_CameraChange(p); });
    return S_OK;
}

void CCameraDirector::On_CameraChange(void* p)
{
    auto d = static_cast<CUTSCENE_CAMERA_DESC*>(p);

    OutputDebugStringW((L"[CamDir] recv eCam=" + std::to_wstring((int)d->eCam)
        + L" track=" + (d->szTrack ? d->szTrack : L"(null)")
        + L" prog=" + (d->pProgress ? L"O" : L"X")
        + L" anchor=" + (d->pAnchorWorld ? L"O" : L"X") + L"\n").c_str());

    const _uint lvl = Get_LevelIndex();
    CGameObject* pArea = m_pGameInstance_Proxy->Find_GameObject(lvl, TEXT("Layer_Camera"), TEXT("CameraFollow"));
    auto         pCut = m_pGameInstance_Proxy->Find_GameObject<CCamera_Cutscene>(lvl, TEXT("Layer_Camera"),TEXT("CameraCutscene"));
    auto         pBoss = m_pGameInstance_Proxy->Find_GameObject<CCamera_Boss>(lvl, TEXT("Layer_Camera"), TEXT("CameraBoss"));

    const _bool bCut = (d->eCam == ECutsceneCam::Cutscene);
    const _bool bBoss = (d->eCam == ECutsceneCam::Boss);

    _bool bReady = false;
    if (bCut && pCut)
        bReady = pCut->Play_Track(d->szTrack, d->pProgress, d->pAnchorWorld);

    OutputDebugStringW((L"[CamDir] pCut=" + std::wstring(pCut ? L"O" : L"X")
        + L" bReady=" + std::wstring(bReady ? L"O" : L"X") + L"\n").c_str());

    OutputDebugStringW((L"[CamDir] pArea=" + std::wstring(pArea ? L"O" : L"X")
        + L" pCut=" + std::wstring(pCut ? L"O" : L"X")
        + L" bReady=" + std::wstring(bReady ? L"O" : L"X") + L"\n").c_str());

    if (pArea) pArea->Set_Active(!(bCut && bReady) && !bBoss);
    if (pCut)  pCut->Set_Active(bCut && bReady);
    if (bBoss)
    {
        if (pCut)  pCut->Set_Active(false);
        if (pArea) pArea->Set_Active(false);
        if (pBoss) { pBoss->Snap(); pBoss->Set_Active(true); } 
        return;
    }

    OutputDebugStringW((L"[CamDir] AFTER area=" + std::to_wstring(pArea ? (int)pArea->Is_Active() : -1)
        + L" cut=" + std::to_wstring(pCut ? (int)pCut->Is_Active() : -1)
        + L" boss=" + std::to_wstring(pBoss ? (int)pBoss->Is_Active() : -1) + L"\n").c_str());
}

CCameraDirector* CCameraDirector::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CCameraDirector* p = new CCameraDirector(pDevice, pContext);
    if (FAILED(p->Initialize_Prototype())) { MSG_BOX("Failed to Created : CCameraDirector"); Safe_Release(p); }
    return p;
}
CGameObject* CCameraDirector::Clone(void* pArg)
{
    CCameraDirector* p = new CCameraDirector(*this);
    if (FAILED(p->Initialize(pArg))) { MSG_BOX("Failed to Cloned : CCameraDirector"); Safe_Release(p); }
    return p;
}
void CCameraDirector::Free() { __super::Free(); }