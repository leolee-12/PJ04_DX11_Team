#include "CutsceneGorilla.h"
#include "GameInstance.h"
#include "Shader.h"
#include "Model.h"
#include "Animator.h"
#include "GameContent_AnimEvents.h"

CCutsceneGorilla::CCutsceneGorilla(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CCutsceneActor(pDevice, pContext) {
}
CCutsceneGorilla::CCutsceneGorilla(const CCutsceneGorilla& Prototype)
    : CCutsceneActor(Prototype) {
}

HRESULT CCutsceneGorilla::Ready_Components()
{
    VISUAL_SETUP t{};
    t.tShader = Shader_Gorilla;
    t.szModelProtoTag = TEXT("Prototype_Component_Model_Boss_Gorilla_Body");  // 전투 고릴라 메쉬 재사용
    t.szAnimEventFile = L""; //TEXT("../Bin/DataFiles/Cutscene/Gorilla_Intro.json");
    if (FAILED(Ready_Visual(t)))
        return E_FAIL;

    Play(CLIP_READY, true);
    m_ePhase = EPhase::ReadyWait;

    return S_OK;
}

HRESULT CCutsceneGorilla::Ready_Events()
{
    Subscribe_Event(EventTag::Cutscene_GorillaAppear, [this](void*) { Begin_Appear(); });
    return S_OK;
}

void CCutsceneGorilla::Update(_float fTimeDelta)
{
    __super::Update(fTimeDelta);
    if (!m_pAnimatorCom)
        return;

    switch (m_ePhase)
    {
        case EPhase::Appearing1:
            if (m_pAnimatorCom->Is_Finished())
            {
                if (!m_bGrabFired)     
                    Fire_Grab();
                Play(CLIP_APPEAR2, false);
                m_ePhase = EPhase::Appearing2;
            }
            break;

        case EPhase::Appearing2:
            if (m_pAnimatorCom->Is_Finished())
                Do_Handoff();
            break;

        default:
            break;
    }
}

HRESULT CCutsceneGorilla::Render()
{
    if (FAILED(Bind_ShaderResources()))
        return E_FAIL;

    const _uint iNumMeshes = static_cast<_uint>(m_pModelCom->Get_NumMeshes());
    for (_uint i = 0; i < iNumMeshes; ++i)
    {
        if (i == 4)
            m_pModelCom->Bind_Material(m_pShaderCom, "g_DiffuseTexture", i, MTEX_TYPE::UNKNOWN, 0);
        else
        {
            m_pModelCom->Bind_Material(m_pShaderCom, "g_DiffuseTexture", i, MTEX_TYPE::DIFFUSE, 0);
            m_pModelCom->Bind_Material(m_pShaderCom, "g_MRATexture", i, MTEX_TYPE::METALNESS, 0);
        }
        if (i == 2)
            m_pModelCom->Bind_Material(m_pShaderCom, "g_BodyMaskTexture", i, MTEX_TYPE::DIFFUSE, 1);

        m_pModelCom->Bind_BoneMatrices(m_pShaderCom, "g_BoneMatrices", i);

        _uint iPass = 0;
        switch (i)
        {
            case 0: iPass = 3; break;
            case 1: iPass = 2; break;
            case 2: iPass = 0; break;
            case 3: iPass = 2; break;
            case 4: iPass = 1; break;
            case 5: iPass = 0; break;
            case 6: iPass = 3; break;
        }
        if (FAILED(m_pShaderCom->Begin(iPass))) return E_FAIL;
        if (FAILED(m_pModelCom->Render(i)))     return E_FAIL;
    }
    return S_OK;
}

void CCutsceneGorilla::On_AnimEvent(const ANIM_EVENT& e, ANIM_EVENT_PHASE p)
{
    switch (static_cast<EANIM_EVENT>(e.iEventType))
    {
        case EANIM_EVENT::CamShake: /* 카메라 셰이크 */ break;
        case EANIM_EVENT::Sound:    /* m_pGameInstance_Proxy->Play_Sound(e.strParam.c_str()); */ break;
        case EANIM_EVENT::Fx:       /* 이펙트 풀 스폰 */ break;

        case EANIM_EVENT::LockMove:                // DemoAppear1 트랙의 strParam 으로 컷씬 신호
            if (e.strParam == "GrabKirby")
                Fire_Grab();
            break;

        default: break;
    }

    //Subscribe_Event(EventTag::Cutscene_GrabKirby, [this](void* pData) {
    //    auto pDesc = static_cast<CUTSCENE_GRAB_DESC*>(pData);
    //    Begin_CutsceneCapture(pDesc->pBoneMatrix, pDesc->pSourceWorld);
    //    });
    //Subscribe_Event(EventTag::Cutscene_ReleaseKirby, [this](void*) {
    //    End_CutsceneCapture(/* 내려놓을 좌표 */);
    //    });
}

void CCutsceneGorilla::Begin_Appear()
{
    if (m_ePhase != EPhase::ReadyWait)
        return;
    Play(CLIP_APPEAR1, false);
    m_ePhase = EPhase::Appearing1;
}

void CCutsceneGorilla::Fire_Grab()
{
    if (m_bGrabFired)
        return;
    m_bGrabFired = true;

    // 1) 커비 부착 (손 본 행렬 + 고릴라 월드)
    CUTSCENE_GRAB_DESC grab{};
    grab.pBoneMatrix = Get_BoneMatrixPtr(GRAB_BONE);
    grab.pSourceWorld = Get_Transform()->Get_WorldMatrixPtr();
    m_pGameInstance_Proxy->Publish(EventTag::Cutscene_GrabKirby, &grab);

    // 2) Area -> 컷씬 카메라
    CUTSCENE_CAMERA_DESC cam{ ECutsceneCam::Cutscene };
    m_pGameInstance_Proxy->Publish(EventTag::Cutscene_CameraChange, &cam);
}

void CCutsceneGorilla::Do_Handoff()
{
    m_ePhase = EPhase::Done;

    // 1) 먼저 보스전 카메라로 컷 -> 하드 스왑을 가림
    CUTSCENE_CAMERA_DESC cam{ ECutsceneCam::Boss };
    m_pGameInstance_Proxy->Publish(EventTag::Cutscene_CameraChange, &cam);

    // 2) 전투 고릴라에 인계 (보스 매니저가 받아서: 전투 고릴라 활성 + grab 진입 + 커비 소켓 교체)
    CUTSCENE_HANDOFF_DESC ho{};
    ho.pFinalWorld = Get_Transform()->Get_WorldMatrixPtr();
    m_pGameInstance_Proxy->Publish(EventTag::Cutscene_GorillaHandoff, &ho);

    // 3) 구독 해제 + 자기 비활성 (삭제 아님, 풀 반납)
    UnSubscribe_Event(EventTag::Cutscene_GorillaAppear);
    Set_Active(false);
}

CCutsceneGorilla* CCutsceneGorilla::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CCutsceneGorilla* pInstance = new CCutsceneGorilla(pDevice, pContext);
    if (FAILED(pInstance->Initialize_Prototype())) {
        MSG_BOX("Failed to Created: CCutsceneGorilla");
        Safe_Release(pInstance);
    }
    return pInstance;
}

CGameObject* CCutsceneGorilla::Clone(void* pArg)
{
    CCutsceneGorilla* pInstance = new CCutsceneGorilla(*this);
    if (FAILED(pInstance->Initialize(pArg))) {
        MSG_BOX("Failed to Cloned: CCutsceneGorilla");
        Safe_Release(pInstance);
    }
    return pInstance;
}

void CCutsceneGorilla::Free() { __super::Free(); }