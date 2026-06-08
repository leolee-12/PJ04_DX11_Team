#include "Level_GamePlay.h"

#include "GameInstance.h"
#include "Camera_Free.h"
#include "Loader_Prototype.h"
#include "Map_Loader.h"
#include "Launcher_MapProfiles.h"

CLevel_GamePlay::CLevel_GamePlay(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CLevel { pDevice, pContext }
{
}

HRESULT CLevel_GamePlay::Initialize()
{
    if (FAILED(__super::Initialize()))
        return E_FAIL;

    LEVEL_MANIFEST Manifest{};
    if (FAILED(Load_LevelManifest(LAUNCHER_LEVEL_PROFILES::LEVEL_TEST, &Manifest)))
        return E_FAIL;

    MAP_LOAD_REPORT MapReport{};
    CMapStage* pMapStage = nullptr;

    if (FAILED(CMap_Loader::Spawn_Map(
        Manifest.strMapManifest,
        ETOUI(LEVEL::GAMEPLAY),
        &MapReport,
        &pMapStage)))
    {
        return E_FAIL;
    }

    if (FAILED(Load_Level(m_pGameInstance_Proxy, m_pDevice, m_pContext,
        Manifest.strObjectsFile.c_str(), ETOUI(LEVEL::GAMEPLAY))))
        return E_FAIL;

    if (FAILED(m_pGameInstance_Proxy->Add_Prototype(ETOUI(LEVEL::GAMEPLAY), TEXT("Prototype_GameObject_Camera_Free"), CCamera_Free::Create(m_pDevice, m_pContext))))
        return E_FAIL;

    CCamera_Free::CAMERA_FREE_DESC      CameraDesc{};

    CameraDesc.vEye = _float3(-130.f, 12.f, -70.f);
    CameraDesc.vAt = _float3(-130.f, 8.f, -64.f);
    CameraDesc.fFovy = XMConvertToRadians(60.f);
    CameraDesc.fNear = 0.1f;
    CameraDesc.fFar = 500.f;
    CameraDesc.fSpeedPerSec = 20.f;
    CameraDesc.fRotationPerSec = XMConvertToRadians(180.f);
    CameraDesc.fMouseSensor = 0.05f;

    if (FAILED(m_pGameInstance_Proxy->Add_GameObject(ETOUI(LEVEL::GAMEPLAY), TEXT("Prototype_GameObject_Camera_Free"),
        ETOUI(LEVEL::GAMEPLAY), TEXT("Layer_Camera"), TEXT("CameraFree"), & CameraDesc)))
        return E_FAIL;

    if (FAILED(Ready_Lights()))
        return E_FAIL;

    return S_OK;
}

void CLevel_GamePlay::Update(_float fTimeDelta)
{
}

HRESULT CLevel_GamePlay::Render()
{
#ifdef _DEBUG
    SetWindowText(g_hWnd, TEXT("게임플레이레벨입니다."));
#endif
    return S_OK;
}

HRESULT CLevel_GamePlay::Ready_Events()
{
    m_pGameInstance_Proxy->Subscribe(TEXT("Boss_Defeated"), [this](void* p) {
        auto* d = static_cast<BOSS_DEFEATED_DESC*>(p);
        Start_Ending(d ? d->pBoss : nullptr);
        });
    return S_OK;
}

HRESULT CLevel_GamePlay::Ready_Lights()
{
    LIGHT_DESC      LightDesc{};

    LightDesc.eType = LIGHT::DIRECTIONAL;
    LightDesc.vDiffuse = _float4(1.f, 1.f, 1.f, 1.f);
    LightDesc.vAmbient = _float4(0.5f, 0.5f, 0.5f, 1.f);
    LightDesc.vSpecular = _float4(1.f, 1.f, 1.f, 1.f);
    LightDesc.vDirection = _float4(0.25f, -1.f, 0.25f, 0.f);

    if (FAILED(m_pGameInstance_Proxy->Add_Light(LightDesc)))
        return E_FAIL;

    return S_OK;
}

void CLevel_GamePlay::Key_Input()
{
    POINT pt = m_pGameInstance_Proxy->Get_MousePos();
    _float fW = m_pGameInstance_Proxy->Get_WindowWidth();
    _float fH = m_pGameInstance_Proxy->Get_WindowHeight();
    _float ndcX = (pt.x / fW) * 2.f - 1.f;
    _float ndcY = 1.f - (pt.y / fH) * 2.f;

    WORLD_HOVER_PROBE hover{};
    m_pGameInstance_Proxy->Compute_PickingRay(ndcX, ndcY, &hover.vRayOrigin, &hover.vRayDir);

    m_pGameInstance_Proxy->Publish(TEXT("World_Hover_Probe"), &hover);
    m_pGameInstance_Proxy->Publish(TEXT("Hover_Result"), &hover);


    if (m_pGameInstance_Proxy->Mouse_Down(DIMB::RBUTTON))
    {
        POINT pt = m_pGameInstance_Proxy->Get_MousePos();
        _float fW = m_pGameInstance_Proxy->Get_WindowWidth();
        _float fH = m_pGameInstance_Proxy->Get_WindowHeight();
        _float2 vNDC = {
            (pt.x / fW) * 2.f - 1.f,
            1.f - (pt.y / fH) * 2.f
        };

        UI_RBTN_PROBE uiProbe = { vNDC, false };
        m_pGameInstance_Proxy->Publish(TEXT("UI_RButton_Probe"), &uiProbe);
        if (uiProbe.bConsumed) return;

        // 월드 probe
        WORLD_CLICK_PROBE probe{};
        probe.vNDC = vNDC;
        m_pGameInstance_Proxy->Compute_PickingRay(
            vNDC.x, vNDC.y, &probe.vRayOrigin, &probe.vRayDir);
        
        m_pGameInstance_Proxy->Publish(TEXT("World_Click_Probe"), &probe);

        // 결과에 따른 분기
        switch (probe.eHitType)
        {
            case EClickHitType::ANIMAL:
                m_pGameInstance_Proxy->Publish(TEXT("Picked_Animal"), &probe);
                break;
            case EClickHitType::ITEMBOX:
                m_pGameInstance_Proxy->Publish(TEXT("Picked_ItemBox"), &probe);
                break;
            case EClickHitType::FLOOR:
                m_pGameInstance_Proxy->Publish(TEXT("Picked_Floor"), &probe.vHitPos);
                break;
            default: break;
        }
    }

    if (m_pGameInstance_Proxy->Key_Down(DIK_A))
    {
        m_pGameInstance_Proxy->Publish(TEXT("Player_Attack"), nullptr);
    }

    _uint iWeapon = { 0 };
    if (m_pGameInstance_Proxy->Key_Down(DIK_1))
    {
        iWeapon = 0;
        m_pGameInstance_Proxy->Publish(TEXT("Player_Switch_Weapon"), &iWeapon);
    }
    if (m_pGameInstance_Proxy->Key_Down(DIK_2))
    {
        iWeapon = 1;
        m_pGameInstance_Proxy->Publish(TEXT("Player_Switch_Weapon"), &iWeapon);
    }
    if (m_pGameInstance_Proxy->Key_Down(DIK_3))
    {
        iWeapon = 2;
        m_pGameInstance_Proxy->Publish(TEXT("Player_Switch_Weapon"), &iWeapon);
    }

    auto MakeSkillInput = [this]() -> SKILL_INPUT {
        POINT pt = m_pGameInstance_Proxy->Get_MousePos();
        _float fW = m_pGameInstance_Proxy->Get_WindowWidth();
        _float fH = m_pGameInstance_Proxy->Get_WindowHeight();
        _float ndcX = (pt.x / fW) * 2.f - 1.f;
        _float ndcY = 1.f - (pt.y / fH) * 2.f;

        SKILL_INPUT in{};
        m_pGameInstance_Proxy->Compute_PickingRay(ndcX, ndcY, &in.vRayOrigin, &in.vRayDir);
        in.bHasRay = true;
        return in;
        };

    if (m_pGameInstance_Proxy->Key_Down(DIK_Q))
    {
        SKILL_INPUT in = MakeSkillInput();
        m_pGameInstance_Proxy->Publish(TEXT("Player_Skill1"), &in);
    }
    if (m_pGameInstance_Proxy->Key_Down(DIK_W))
    {
        SKILL_INPUT in = MakeSkillInput();
        m_pGameInstance_Proxy->Publish(TEXT("Player_Skill2"), &in);
    }
    if (m_pGameInstance_Proxy->Key_Down(DIK_E))
    {
        SKILL_INPUT in = MakeSkillInput();
        m_pGameInstance_Proxy->Publish(TEXT("Player_Skill3"), &in);
    }
    if (m_pGameInstance_Proxy->Key_Down(DIK_R))
    {
        SKILL_INPUT in = MakeSkillInput();
        m_pGameInstance_Proxy->Publish(TEXT("Player_Skill4"), &in);
    }

#ifdef _DEBUG
    if (m_pGameInstance_Proxy->Key_Down(DIK_O))
    {
        static const _uint base[16] = {
            0,  1,  2,  3,      
            22, 23, 24, 25,     
            32, 33, 34, 35,     
            54, 55, 56, 57      
        };
        static _uint dummy[16];
        for (_uint i = 0; i < 16; ++i) dummy[i] = base[i];

        SHOW_ITEMBOX_UI desc{};
        desc.iBoxIdx = 0;
        desc.pContents = dummy;
        m_pGameInstance_Proxy->Publish(TEXT("Show_ItemBox_UI"), &desc);
    }
#endif
}

void CLevel_GamePlay::Update_EndingSequence(_float fRawDelta)
{
    m_fEndingTimer += fRawDelta;

    switch (m_eEndingPhase)
    {
        case EEndingPhase::SLOW:
        {
            _float t = min(1.f, m_fEndingTimer / SLOWMO_RAMP_TIME);
            _float scale = 1.f + (SLOWMO_TARGET - 1.f) * t;  
            m_pGameInstance_Proxy->Set_TimeScale(scale);
            if (t >= 1.f) {
                m_eEndingPhase = EEndingPhase::HOLD;
                m_fEndingTimer = 0.f;
            }
            break;
        }
        case EEndingPhase::HOLD:
            if (m_fEndingTimer >= HOLD_TIME) {
                m_eEndingPhase = EEndingPhase::RECOVER;
                m_fEndingTimer = 0.f;
            }
            break;
        case EEndingPhase::RECOVER:
        {
            _float t = min(1.f, m_fEndingTimer / RECOVER_RAMP_TIME);
            _float scale = SLOWMO_TARGET + (1.f - SLOWMO_TARGET) * t; 
            m_pGameInstance_Proxy->Set_TimeScale(scale);
            if (t >= 1.f) {
                m_pGameInstance_Proxy->Set_TimeScale(1.f);  
                m_eEndingPhase = EEndingPhase::NONE;
                m_fEndingTimer = 0.f;
            }
            break;
        }
        default: break;
    }
}

void CLevel_GamePlay::Start_Ending(CGameObject* pBoss)
{
    if (m_eEndingPhase != EEndingPhase::NONE) return;   
    m_eEndingPhase = EEndingPhase::SLOW;
    m_fEndingTimer = 0.f;
}

CLevel_GamePlay* CLevel_GamePlay::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CLevel_GamePlay* pInstance = new CLevel_GamePlay(pDevice, pContext);

    if (FAILED(pInstance->Initialize()))
    {
        MSG_BOX("Failed to Created : CLevel_GamePlay");
        Safe_Release(pInstance);
    }

    return pInstance;
}

void CLevel_GamePlay::Free()
{
    if (m_pGameInstance_Proxy)
        m_pGameInstance_Proxy->Set_TimeScale(1.f);

    __super::Free();
}
