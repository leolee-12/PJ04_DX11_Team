#include "EnvInteract_BreakProp.h"
#include "GameContent_const.h"
#include "GameContent_Events.h"
#include "LevelDesign_Defines.h"
#include "Effect_Loader.h"

#include "GameInstance_Proxy.h"
#include "Geometry_Utils.h"
#include "Collider.h"

NS_BEGIN(Client)

namespace
{
    static constexpr _float s_fBreakEffectHeightRatio = { 0.55f };
}

CEnvInteract_BreakProp::CEnvInteract_BreakProp(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CEnvObject_Interact(pDevice, pContext)
{
    m_strProtoTag = PROTOTYPE_TAG;
}

CEnvInteract_BreakProp::CEnvInteract_BreakProp(const CEnvInteract_BreakProp& Prototype)
    : CEnvObject_Interact(Prototype)
{
    m_strProtoTag = PROTOTYPE_TAG;
}

HRESULT CEnvInteract_BreakProp::Initialize_Prototype()
{
    return __super::Initialize_Prototype();
}

void CEnvInteract_BreakProp::Late_Update(_float fTimeDelta)
{
    if (!Is_Active())
        return;

#ifdef _DEBUG
    if (nullptr != m_pHurtBox && m_pHurtBox->Is_Enabled())
        m_pGameInstance_Proxy->Add_DebugComponent(m_pHurtBox);
#endif

    __super::Late_Update(fTimeDelta);
}

void CEnvInteract_BreakProp::Damaged(const ATTACK_INFO& tInfo)
{
    UNREFERENCED_PARAMETER(tInfo);

    if (BREAK_STATE::INTACT != m_eState)
        return;

    m_eState = BREAK_STATE::DESTROYED;

    const BoundingBox& LocalBounds = Get_LocalBounds();
    const _float3 vLocalEffectPos = {
        LocalBounds.Center.x,
        LocalBounds.Center.y - LocalBounds.Extents.y + LocalBounds.Extents.y * 2.f *
        s_fBreakEffectHeightRatio,
        LocalBounds.Center.z };

    _float3 vEffectPosition{};
    XMStoreFloat3(&vEffectPosition, XMVector3TransformCoord(
        XMLoadFloat3(&vLocalEffectPos), XMLoadFloat4x4(m_pTransformCom->Get_WorldMatrixPtr())));

    const ENV_INTERACT_PRESET& Preset = m_tDesc.tInteractPreset;

    if (!Preset.wstrBreakSoundKey.empty())
        m_pGameInstance_Proxy->Play_SFX(Preset.wstrBreakSoundKey.c_str(), 0.25f);

    if (!Preset.wstrBreakEffectKey.empty())
        CEffect_Loader::GetInstance()->Spawn(Preset.wstrBreakEffectKey, Get_LevelIndex(),
            vEffectPosition);

    Grant_Reward();

    m_pHurtBox->Set_Enabled(false);
    Release_PhysicsActor();
    Set_Active(false);
}

void CEnvInteract_BreakProp::Grant_Reward()
{
    const ENV_INTERACT_PRESET& Preset = m_tDesc.tInteractPreset;
    if (!Preset.bGrantReward || 0 >= Preset.iPointStarAmount)
        return;

    if (m_pGameInstance_Proxy->RandomInt(1, 100) > static_cast<_int>(Preset.iRewardChancePercent))
        return;

    KIRBY_POINTSTAR_GAINED_DESC Desc{};
    Desc.iAmount = static_cast<_uint>(Preset.iPointStarAmount);
    m_pGameInstance_Proxy->Publish(EventTag::Kirby_PointStarGained, &Desc);
}

HRESULT CEnvInteract_BreakProp::Ready_InteractComponents()
{
    if (ENV_INTERACT_TYPE::BREAKABLE != m_tDesc.eInteractType)
        return E_FAIL;

    if (ENV_INTERACT_TYPE::BREAKABLE != m_tDesc.tInteractPreset.eType)
        return E_FAIL;

    if (nullptr == m_pGameInstance_Proxy)
        return E_FAIL;

    if (m_pGameInstance_Proxy->Is_EditMode())
        return S_OK;

    if (FAILED(Ready_RigidStatic()))
        return E_FAIL;

    if (FAILED(Ready_HurtBox()))
        return E_FAIL;

    return S_OK;
}

HRESULT CEnvInteract_BreakProp::Ready_RigidStatic()
{
    Release_PhysicsActor();

    const BoundingBox& LocalBounds = Get_LocalBounds();
    if (!GeometryUtils::Is_ValidAABB(LocalBounds))
        return E_FAIL;

    m_pPhysicsActor = m_pGameInstance_Proxy->Create_StaticBox(
        LocalBounds.Center,
        LocalBounds.Extents,
        XMLoadFloat4x4(m_pTransformCom->Get_WorldMatrixPtr()));

    if (nullptr == m_pPhysicsActor)
        return E_FAIL;

    m_vPhysicsActorScale = m_pTransformCom->Get_Scaled();
    return S_OK;
}

HRESULT CEnvInteract_BreakProp::Ready_HurtBox()
{
    const BoundingBox& LocalBounds = Get_LocalBounds();
    if (!GeometryUtils::Is_ValidAABB(LocalBounds))
        return E_FAIL;

    CCollider::COLLIDER_DESC ColliderDesc{};
    ColliderDesc.pOwner = this;
    ColliderDesc.vCenter = LocalBounds.Center;
    ColliderDesc.fRadius = max(max(LocalBounds.Extents.x, LocalBounds.Extents.y), LocalBounds.Extents.z);

    m_pHurtBox = Add_Component<CCollider>(
        Collider_Sphere.iLevelID,
        Collider_Sphere.szProtoTag,
        TEXT("Com_HurtBox"),
        &ColliderDesc);
    if (nullptr == m_pHurtBox)
        return E_FAIL;

    m_pHurtBox->Set_OnEnter([this](CCollider* pOther)
        {
            if (nullptr == pOther || BREAK_STATE::INTACT != m_eState)
                return;

            PLAYER_QUERY PlayerQuery{};
            m_pGameInstance_Proxy->Publish(EventTag::Query_Player, &PlayerQuery);

            ATTACK_INFO AttackInfo{};
            if (!Try_ResolveContactHit(
                pOther->Get_RegisteredGroup(),
                pOther->Get_Owner(),
                PlayerQuery.pPlayer,
                &AttackInfo))
            {
                return;
            }

            Damaged(AttackInfo);
        });

    m_pHurtBox->Update(XMLoadFloat4x4(m_pTransformCom->Get_WorldMatrixPtr()));
    m_pGameInstance_Proxy->Register_Collider(m_pHurtBox, ETOUI(COLLISION_LAYER::ENV_HURT));

    return S_OK;
}

CEnvInteract_BreakProp* CEnvInteract_BreakProp::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CEnvInteract_BreakProp* pInstance = new CEnvInteract_BreakProp(pDevice, pContext);

    if (FAILED(pInstance->Initialize_Prototype()))
    {
        MSG_BOX("Failed to Created : CEnvInteract_BreakProp");
        Safe_Release(pInstance);
    }

    return pInstance;
}

CGameObject* CEnvInteract_BreakProp::Clone(void* pArg)
{
    CEnvInteract_BreakProp* pInstance = new CEnvInteract_BreakProp(*this);

    if (FAILED(pInstance->Initialize(pArg)))
    {
        MSG_BOX("Failed to Cloned : CEnvInteract_BreakProp");
        Safe_Release(pInstance);
    }

    return pInstance;
}

void CEnvInteract_BreakProp::Free()
{
    if (nullptr != m_pHurtBox)
    {
        m_pHurtBox->Set_Enabled(false);
        m_pHurtBox->Clear_Callbacks();

        if (nullptr != m_pGameInstance_Proxy)
            m_pGameInstance_Proxy->Immediate_Unregister(m_pHurtBox,
                ETOUI(COLLISION_LAYER::ENV_HURT));

        m_pHurtBox->Mark_Unregistered();
    }

    m_pHurtBox = nullptr;

    __super::Free();
}

NS_END