#include "CameraDirector.h"
#include "GameInstance.h"
#include "GameContrnt_Events.h"
#include "Camera_Cutscene.h"
#include "Camera_Boss.h"
#include "Camera_AreaCam.h"
#include "Camera_Dialogue.h"

CCameraDirector* CCameraDirector::s_pActiveDirector = nullptr;

namespace
{
    constexpr const _tchar* LAYER_CAMERA = TEXT("Layer_Camera");
    constexpr const _tchar* TAG_AREA = TEXT("CameraFollow");
    constexpr const _tchar* TAG_CUT = TEXT("CameraCutscene");
    constexpr const _tchar* TAG_BOSS = TEXT("CameraBoss");
    constexpr const _tchar* TAG_DIALOG = TEXT("CameraDialogue");

    const _tchar* Resolve_AreaCamDataPath(LEVEL eLevel)
    {
        switch (eLevel)
        {
            case LEVEL::STAGE0_STEP1:  return L"../../Resources/YSH/CameraData/Level0_Stage1_Step01_cam.json";
            case LEVEL::STAGE0_STEP2:  return L"../../Resources/YSH/CameraData/Level0_Stage1_Step02_cam.json";
            case LEVEL::TOWN_STEP1:    return L"../../Resources/YSH/CameraData/Town_Step1_cam.json";
            case LEVEL::BOSS_STAGE1:   return L"../../Resources/YSH/CameraData/Level1_Stage5_Step01_cam.json";
            case LEVEL::STAGE1_STEP1:  return L"../../Resources/YSH/CameraData/Level6_Stage4_Step01_cam.json";
            case LEVEL::STAGE1_STEP2:  return L"../../Resources/YSH/CameraData/Level6_Stage4_Step02_cam.json";
            case LEVEL::STAGE1_STEP3:  return L"../../Resources/YSH/CameraData/Level6_Stage4_Step03_cam.json";
            case LEVEL::TEST:          return L"../../Resources/YSH/CameraData/Level0_Stage1_Step01_cam.json";
            default:
                OutputDebugStringW(L"[CameraDirector] AreaCam 데이터 경로 테이블에 이 레벨이 없음\n");
                return L"../../Resources/YSH/CameraData/Level0_Stage1_Step01_cam.json";
        }
    }
}

CCameraDirector::CCameraDirector(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CGameObject(pDevice, pContext) {
}
CCameraDirector::CCameraDirector(const CCameraDirector& Prototype)
    : CGameObject(Prototype) {
}

HRESULT CCameraDirector::Initialize(void* pArg)
{
    if (FAILED(__super::Initialize(pArg)))   
        return E_FAIL;

    m_bClone = true;    
    return S_OK;
}

HRESULT CCameraDirector::Ready_Events()
{
    Subscribe_Event(EventTag::Cutscene_CameraChange, [this](void* p) { On_CameraChange(p); });
    Subscribe_Event(EventTag::Dialogue_CamBegin, [this](void* p) { On_DialogueCamBegin(p); });
    return S_OK;
}

void CCameraDirector::On_Deserialized()
{
    if (m_pGameInstance_Proxy->Is_EditMode())
        return;

    if (FAILED(Ensure_Cameras()))
    {
        UnSubscribe_Event(EventTag::Cutscene_CameraChange);
        return;
    }

    Arrange();
}

void CCameraDirector::On_CameraChange(void* p)
{
    auto d = static_cast<CUTSCENE_CAMERA_DESC*>(p);

    const _bool bCut = (d->eCam == ECutsceneCam::Cutscene);
    const _bool bBoss = (d->eCam == ECutsceneCam::Boss);
    const _bool bArea = (d->eCam == ECutsceneCam::Area);

    _bool bReady = false;
    if (bCut)
        bReady = m_pCutCam->Play_Track(d->szTrack, d->pProgress, d->pAnchorWorld);

    if (bArea)
    {
        CCamera* pFromCam = nullptr;
        if (m_pCutCam->Is_Active())       pFromCam = m_pCutCam;
        else if (m_pBossCam->Is_Active()) pFromCam = m_pBossCam;
        else if (m_pDlgCam->Is_Active())  pFromCam = m_pDlgCam;

        if (pFromCam)
        {
            CTransform* pTf = pFromCam->Get_Transform();
            _vector vEye = pTf->Get_State(STATE::POSITION);
            _vector vAt = vEye + pTf->Get_State(STATE::LOOK);
            m_pAreaCam->Begin_Handoff(vEye, vAt, pFromCam->Get_Fovy());
        }
        m_pAreaCam->Set_Active(true);
        m_pCutCam->Set_Active(false);
        m_pBossCam->Set_Active(false);
        m_pDlgCam->Set_Active(false);
        return;
    }

    m_pCutCam->Set_Active(bCut && bReady);
    if (bCut && bReady)
        m_pBossCam->Set_Active(false);

    if (bBoss)
    {
        m_pCutCam->Set_Active(false);
        m_pAreaCam->Set_Active(false);
        m_pDlgCam->Set_Active(false);
        m_pBossCam->Snap();
        m_pBossCam->Set_Active(true);
        return;
    }
}

void CCameraDirector::On_DialogueCamBegin(void* p)
{
    auto d = static_cast<DIALOGUE_CAMERA_DESC*>(p);
    if (nullptr == d || nullptr == d->pAnchorWorld)
        return;

    m_pDlgCam->Begin(*d->pAnchorWorld, XMLoadFloat3(&d->vPosA), XMLoadFloat3(&d->vPosB));
    m_pDlgCam->Set_Active(true);
    m_pAreaCam->Set_Active(false);
    m_pCutCam->Set_Active(false);
    m_pBossCam->Set_Active(false);
}

HRESULT CCameraDirector::Ensure_Cameras()
{
    const _uint iStatic = ETOUI(LEVEL::STATIC);

    // AreaCam
    if (!m_pGameInstance_Proxy->Has_Prototype(iStatic, CCamera_AreaCam::PROTOTYPE_TAG))
    {
        m_pGameInstance_Proxy->Add_Prototype(iStatic, CCamera_AreaCam::PROTOTYPE_TAG,
            CCamera_AreaCam::Create(m_pDevice, m_pContext));

        CCamera_AreaCam::AREACAM_DESC CamDesc{};
        CamDesc.vEye = _float3(-1.f, 1.f, -10.f);
        CamDesc.vAt = _float3(0.f, 0.f, 0.f);
        CamDesc.fFovy = XMConvertToRadians(50.f); CamDesc.fNear = 0.1f; CamDesc.fFar = 1000.f;
        CamDesc.strDataPath = Resolve_AreaCamDataPath(static_cast<LEVEL>(Get_LevelIndex()));
        if (FAILED(m_pGameInstance_Proxy->Add_GameObject(iStatic, CCamera_AreaCam::PROTOTYPE_TAG,
            iStatic, LAYER_CAMERA, TAG_AREA, &CamDesc)))
            return E_FAIL;
    }

    // Cutscene
    if (!m_pGameInstance_Proxy->Has_Prototype(iStatic, CCamera_Cutscene::PROTOTYPE_TAG))
    {
        m_pGameInstance_Proxy->Add_Prototype(iStatic, CCamera_Cutscene::PROTOTYPE_TAG,
            CCamera_Cutscene::Create(m_pDevice, m_pContext));

        CCamera_Cutscene::CUTSCENECAM_DESC CutDesc{};
        CutDesc.fFovy = XMConvertToRadians(50.f); CutDesc.fNear = 0.1f; CutDesc.fFar = 1000.f;
        if (FAILED(m_pGameInstance_Proxy->Add_GameObject(iStatic, CCamera_Cutscene::PROTOTYPE_TAG,
            iStatic, LAYER_CAMERA, TAG_CUT, &CutDesc)))
            return E_FAIL;
    }

    // Boss
    if (!m_pGameInstance_Proxy->Has_Prototype(iStatic, CCamera_Boss::PROTOTYPE_TAG))
    {
        m_pGameInstance_Proxy->Add_Prototype(iStatic, CCamera_Boss::PROTOTYPE_TAG,
            CCamera_Boss::Create(m_pDevice, m_pContext));

        CCamera_Boss::BOSSCAM_DESC BossDesc{};
        BossDesc.fFovy = XMConvertToRadians(50.f); BossDesc.fNear = 0.1f; BossDesc.fFar = 1000.f;
        if (FAILED(m_pGameInstance_Proxy->Add_GameObject(iStatic, CCamera_Boss::PROTOTYPE_TAG,
            iStatic, LAYER_CAMERA, TAG_BOSS, &BossDesc)))
            return E_FAIL;
    }

    // Dialogue
    if (!m_pGameInstance_Proxy->Has_Prototype(iStatic, CCamera_Dialogue::PROTOTYPE_TAG))
    {
        m_pGameInstance_Proxy->Add_Prototype(iStatic, CCamera_Dialogue::PROTOTYPE_TAG,
            CCamera_Dialogue::Create(m_pDevice, m_pContext));

        CCamera_Dialogue::DIALOGUECAM_DESC DialogDesc{};
        DialogDesc.fFovy = XMConvertToRadians(50.f); DialogDesc.fNear = 0.1f; DialogDesc.fFar = 1000.f;
        if (FAILED(m_pGameInstance_Proxy->Add_GameObject(iStatic, CCamera_Dialogue::PROTOTYPE_TAG,
            iStatic, LAYER_CAMERA, TAG_DIALOG, &DialogDesc)))
            return E_FAIL;
    }

    m_pAreaCam = m_pGameInstance_Proxy->Find_GameObject<CCamera_AreaCam>(iStatic, LAYER_CAMERA, TAG_AREA);
    m_pCutCam = m_pGameInstance_Proxy->Find_GameObject<CCamera_Cutscene>(iStatic, LAYER_CAMERA, TAG_CUT);
    m_pBossCam = m_pGameInstance_Proxy->Find_GameObject<CCamera_Boss>(iStatic, LAYER_CAMERA, TAG_BOSS);
    m_pDlgCam = m_pGameInstance_Proxy->Find_GameObject<CCamera_Dialogue>(iStatic, LAYER_CAMERA, TAG_DIALOG);

    if (nullptr == m_pAreaCam || nullptr == m_pCutCam || nullptr == m_pBossCam || nullptr == m_pDlgCam)
    {
        MSG_BOX("CameraDirector: STATIC camera cache failed - creation path broken");
        return E_FAIL;
    }

    Safe_AddRef(m_pAreaCam);
    Safe_AddRef(m_pCutCam);
    Safe_AddRef(m_pBossCam);
    Safe_AddRef(m_pDlgCam);

    return S_OK;
}

void CCameraDirector::Arrange()
{
    m_pAreaCam->Rearrange(Resolve_AreaCamDataPath(static_cast<LEVEL>(Get_LevelIndex())));
    m_pAreaCam->Set_Active(true);

    m_pCutCam->Set_Active(false);

    m_pBossCam->Clear_LevelRefs();
    m_pBossCam->Set_Active(false);

    m_pDlgCam->Set_Active(false);

    s_pActiveDirector = this;
}

void CCameraDirector::Sleep_Cameras()
{
    m_pAreaCam->Set_Active(false);
    m_pCutCam->Set_Active(false);
    m_pBossCam->Clear_LevelRefs();
    m_pBossCam->Set_Active(false);
    m_pDlgCam->Set_Active(false);
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
void CCameraDirector::Free() 
{ 
    if (m_bClone)
    {
        if (s_pActiveDirector == this)
        {
            Sleep_Cameras();
            s_pActiveDirector = nullptr;
        }

        Safe_Release(m_pAreaCam);
        Safe_Release(m_pCutCam);
        Safe_Release(m_pBossCam);
        Safe_Release(m_pDlgCam);
    }

    __super::Free();
}