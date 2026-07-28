#include "LD_MeteorGenerator.h"
#include "LevelDesign_Registry.h"
#include "Parsing_Utils.h"
#include "GameInstance.h"
#include "GameContent_Const.h"

#include "Projectile_Manager.h"
#include "MeteorRock_Large.h"
#include "MeteorRock_Small.h"

CLD_MeteorGenerator::CLD_MeteorGenerator(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CLevelDesignObject(pDevice, pContext)
{
}

CLD_MeteorGenerator::CLD_MeteorGenerator(const CLD_MeteorGenerator& Prototype)
    : CLevelDesignObject(Prototype)
{
}

HRESULT CLD_MeteorGenerator::Initialize_Prototype()
{
    return __super::Initialize_Prototype();
}

HRESULT CLD_MeteorGenerator::Initialize(void* pArg)
{
    if (FAILED(__super::Initialize(pArg)))
        return E_FAIL;

    if (nullptr != pArg)
        m_tMeteorDesc = *static_cast<const LD_METEOR_DESC*>(pArg);

    m_bLarge = (m_tLevelDesignDesc.strObjectName == L"VolcanoRockLarge");
    if (m_tMeteorDesc.bFindHero)
    {
        if (FAILED(Ready_Trigger()))
            return E_FAIL;
    }

    return S_OK;
}

void CLD_MeteorGenerator::Update(_float fTimeDelta)
{
    __super::Update(fTimeDelta);

    if (!m_bStopped)
    {
        const _bool bArmed = (m_tMeteorDesc.bFindHero && m_bHeroInRange) || m_bEventFired;

        if (m_tMeteorDesc.bRepeat)
        {
            if (m_tMeteorDesc.bFindHero && !m_bFired && bArmed)
            {
                m_bFired = true;
                Fire_Rock();
                m_fFireTimer = m_tMeteorDesc.fInterval;
            }
            else
            {
                m_fFireTimer -= fTimeDelta;
                if (m_fFireTimer <= 0.f)
                {
                    if (bArmed)
                        Fire_Rock();
                    m_fFireTimer = m_tMeteorDesc.fInterval;
                }
            }
        }
        else if (bArmed && !m_bFiredOnce)
        {
            Fire_Rock();
            m_bFiredOnce = true;
        }
    }
}

void CLD_MeteorGenerator::Late_Update(_float fTimeDelta)
{
    __super::Late_Update(fTimeDelta);

    if (m_pTrigger)
    {
        m_pTrigger->Update(XMLoadFloat4x4(m_pTransformCom->Get_WorldMatrixPtr()));
#ifdef _DEBUG
        if (m_pTrigger->Is_Enabled())
            m_pGameInstance_Proxy->Add_DebugComponent(m_pTrigger);
#endif
    }
}

void CLD_MeteorGenerator::On_LDEventReceived(const _wstring& strEventTag)
{
    if (m_tMeteorDesc.bFallEndOnEvent)      m_bStopped = true;   
    else                                    m_bEventFired = true;
    
    if (m_tMeteorDesc.bRepeat && m_tMeteorDesc.fInterval > 0.f)
    {
        m_fFireTimer = (static_cast<_float>(rand()) / RAND_MAX) *  m_tMeteorDesc.fInterval;
    }
}

HRESULT CLD_MeteorGenerator::Ready_Trigger()
{
    CCollider::COLLIDER_DESC Desc{};
    Desc.pOwner = this;
    Desc.vCenter = _float3{ 0.f, m_tMeteorDesc.fSearchHeightOffset, 0.f };
    Desc.fRadius = m_tMeteorDesc.fSearchRadius;

    m_pTrigger = Add_Component<CCollider>(Collider_Sphere.iLevelID, Collider_Sphere.szProtoTag,
        TEXT("Com_MeteorTrigger"), &Desc);
    if (nullptr == m_pTrigger)
        return E_FAIL;

    m_pGameInstance_Proxy->Register_Collider(m_pTrigger, ETOUI(COLLISION_LAYER::ENV_TRIGGER));

    m_pTrigger->Set_OnEnter([this](CCollider* pOther) {
        if (ETOUI(COLLISION_LAYER::PLAYER_HURT) == pOther->Get_RegisteredGroup())
        {
            m_bHeroInRange = true;
        }
        });
    m_pTrigger->Set_OnExit([this](CCollider* pOther) {
        if (ETOUI(COLLISION_LAYER::PLAYER_HURT) == pOther->Get_RegisteredGroup())
            m_bHeroInRange = false;
        });

    m_pTrigger->Update(XMLoadFloat4x4(m_pTransformCom->Get_WorldMatrixPtr()));
    return S_OK;
}

void CLD_MeteorGenerator::Fire_Rock()
{
    _vector vGen = m_pTransformCom->Get_State(STATE::POSITION);
    _vector vStart = XMLoadFloat3(&m_tMeteorDesc.vFallStart);
    _vector vDelta = vGen - vStart;

    const _float fDist = XMVectorGetX(XMVector3Length(vDelta));
    if (fDist < 1e-3f)
        return;

    const _tchar* pProtoTag = m_bLarge ? CMeteorRock_Large::PROTOTYPE_TAG : CMeteorRock_Small::PROTOTYPE_TAG;
    const _wstring strKey = m_bLarge ? L"MeteorRock_Large" : L"MeteorRock_Small";

    CProjectile* pProj = nullptr;
    CProjectile_Manager::GetInstance()->Spawn(Get_PrototypeLevelIndex(), Get_LevelIndex(), strKey, pProtoTag, &pProj);
    if (nullptr == pProj)
        return;

    const _float fLife = (m_tMeteorDesc.fFallSpeed > 0.f) ? (fDist / m_tMeteorDesc.fFallSpeed) + 0.5f : 12.f;
    _float3 vTarget{};
    XMStoreFloat3(&vTarget, vGen);

    CMeteorRock* pRock = static_cast<CMeteorRock*>(pProj);
    pRock->Configure(m_tMeteorDesc.fFallSpeed, fLife, !m_tMeteorDesc.bNoBreakEffect, m_tMeteorDesc.fImpactRadius * 2.f);
    pRock->Set_TargetPos(vTarget);

    _float3 vDir;
    XMStoreFloat3(&vDir, XMVector3Normalize(vDelta));
    pRock->Launch(m_tMeteorDesc.vFallStart, vDir);

    _float3 vScale{};
    vScale = m_tMeteorDesc.vParsedScale;
    pRock->Get_Transform()->Set_Scale(vScale.x, vScale.y, vScale.z);
}

void CLD_MeteorGenerator::Register_LevelDesignSpecs()
{
    const _wstring ObjectNames[] =
    {
        L"VolcanoRockLarge",
        L"VolcanoRockSmall",
    };

    for (const _wstring& strObjectName : ObjectNames)
    {
        LD_SPAWN_SPEC Spec{};
        Spec.strObjectName = strObjectName;
        Spec.strPrototypeTag = PROTOTYPE_TAG;
        Spec.strLayerTag = LAYER_TAG;
        Spec.eCategory = LD_CATEGORY::GIMMICK;
        Spec.pPrototypeFactory = &Create_Prototype;
        Spec.pBuildDesc = &Build_Desc;
        Spec.bUseFactoryResourceLoader = true;

        CLevelDesign_Registry::Register(Spec.strObjectName, Spec);
    }
}

_bool CLD_MeteorGenerator::Build_Desc(const LD_OBJECT_DESC& CommonDesc, const json& jEntry, const LD_SPAWN_SPEC& Spec, LD_OBJECT_ENTRY* pOutEntry)
{
    if (nullptr == pOutEntry)
        return false;

    LD_METEOR_DESC Desc{};
    static_cast<LD_OBJECT_DESC&>(Desc) = CommonDesc;
    Desc.eCategory = Spec.eCategory;

    const _string strComp = "Gimmick.VolcanoRock.Builder.MainComponent.";
    JsonUtils::Try_ReadFloat3Array(jEntry, strComp + "FallStartPosition", &Desc.vFallStart);
    JsonUtils::Try_ReadFloat(jEntry, strComp + "FallSpeed", &Desc.fFallSpeed);
    JsonUtils::Try_ReadFloat(jEntry, strComp + "Radius", &Desc.fImpactRadius);
    JsonUtils::Try_ReadFloat(jEntry, strComp + "IntervalTime", &Desc.fInterval);
    JsonUtils::Try_ReadBoolFromNumeric(jEntry, strComp + "IsInvalidToGenerateBreakEffect", &Desc.bNoBreakEffect);
    JsonUtils::Try_ReadBoolFromNumeric(jEntry, strComp + "IsFallEndOnEventReceive", &Desc.bFallEndOnEvent);

    _wstring strFallStartType;
    if (JsonUtils::Try_ReadString(jEntry, strComp + "FallStartType", &strFallStartType))
        Desc.bFindHero = (strFallStartType == L"FindHero");

    _wstring strRepeatType;
    if (JsonUtils::Try_ReadString(jEntry, strComp + "RepeatType", &strRepeatType))
        Desc.bRepeat = (strRepeatType == L"Repeat");

    const _string strSearch = "Basic.SearchHeroCylinder.";
    JsonUtils::Try_ReadFloat(jEntry, strSearch + "SearchRadiusScene", &Desc.fSearchRadius);
    JsonUtils::Try_ReadFloat(jEntry, strSearch + "SearchHeightScene", &Desc.fSearchHeight);
    JsonUtils::Try_ReadFloat(jEntry, strSearch + "SearchHeightOffsetScene", &Desc.fSearchHeightOffset);

    *pOutEntry = Desc;
    return true;
}

CGameObject* CLD_MeteorGenerator::Create_Prototype(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    return CLD_MeteorGenerator::Create(pDevice, pContext);
}

CLD_MeteorGenerator* CLD_MeteorGenerator::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CLD_MeteorGenerator* pInstance = new CLD_MeteorGenerator(pDevice, pContext);
    if (FAILED(pInstance->Initialize_Prototype()))
    {
        MSG_BOX("Failed to Created : CLD_MeteorGenerator");
        Safe_Release(pInstance);
    }
    return pInstance;
}

CGameObject* CLD_MeteorGenerator::Clone(void* pArg)
{
    CLD_MeteorGenerator* pInstance = new CLD_MeteorGenerator(*this);
    if (FAILED(pInstance->Initialize(pArg)))
    {
        MSG_BOX("Failed to Cloned : CLD_MeteorGenerator");
        Safe_Release(pInstance);
    }
    return pInstance;
}

void CLD_MeteorGenerator::Free()
{
    __super::Free();
}
