#include "Boss_Metaknight_Sword.h"
#include "GameInstance.h"

CBoss_Metaknight_Sword::CBoss_Metaknight_Sword(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CMonsterHitPart(pDevice, pContext) {
}
CBoss_Metaknight_Sword::CBoss_Metaknight_Sword(const CBoss_Metaknight_Sword& Prototype)
    : CMonsterHitPart(Prototype) {
}

HRESULT CBoss_Metaknight_Sword::Initialize(void* pArg)
{
    if (FAILED(__super::Initialize(pArg)))  return E_FAIL;
    if (FAILED(Ready_Components()))         return E_FAIL;

    CAPSULE_DESC CapsuleDesc{};
    CapsuleDesc.vCenter = { 0.f, 0.f, 0.f };
    CapsuleDesc.fHeight = 1.5f;
    CapsuleDesc.fRadius = 0.5f;
    CapsuleDesc.vRadians = { XMConvertToRadians(-90.f), 0.f, 0.f };
    if (FAILED(Ready_HitBox(CapsuleDesc)))
        return E_FAIL;


    CCollider::COLLIDER_DESC PickDesc{};
    PickDesc.pOwner = this;
    PickDesc.vCenter = { 0.f, 0.f, 0.f };
    PickDesc.fRadius = 0.6f;
    PickDesc.fHeight = 0.6f;
    PickDesc.vRadians = { XMConvertToRadians(-90.f), 0.f, 0.f };
    m_pPickBox = Add_Component<CCollider>(Collider_Capsule.iLevelID, Collider_Capsule.szProtoTag,
        TEXT("PickBox_Com"), &PickDesc);
    if (nullptr == m_pPickBox)
        return E_FAIL;

    m_pPickBox->Set_OnEnter([this](CCollider* pOther) {
        if (ETOUI(COLLISION_LAYER::PLAYER_HURT) != pOther->Get_RegisteredGroup())
            return;
        if (m_bTaken)
            return;
        m_bTaken = true;
        m_pPickBox->Set_Enabled(false);
        Set_Active(false);
        });

    m_pPickBox->Set_Enabled(false);
    m_pGameInstance_Proxy->Register_Collider(m_pPickBox, ETOUI(COLLISION_LAYER::EXCALIBUR));

    m_iShadowPassIdx = 6;

    return S_OK;
}

void CBoss_Metaknight_Sword::Late_Update(_float fTimeDelta)
{
    if (!m_bDropped)
    {
        __super::Late_Update(fTimeDelta);
        return;
    }

    if (!m_bActive)
        return;

    m_CombinedWorldMatrix = m_DropWorld;
    m_pGameInstance_Proxy->Add_RenderGroup(RENDERID::NONBLEND, this);
    m_pGameInstance_Proxy->Add_RenderGroup(RENDERID::SHADOW, this);

    if (m_pPickBox && m_pPickBox->Is_Enabled())
    {
        m_pPickBox->Update(XMLoadFloat4x4(&m_CombinedWorldMatrix));
#ifdef _DEBUG
        m_pGameInstance_Proxy->Add_DebugComponent(m_pPickBox);
#endif
    }
}

HRESULT CBoss_Metaknight_Sword::Render()
{
    if (FAILED(m_pShaderCom->Bind_Matrix("g_WorldMatrix", &m_CombinedWorldMatrix)))
        return E_FAIL;
    if (FAILED(m_pShaderCom->Bind_Matrix("g_ViewMatrix", m_pGameInstance_Proxy->Get_Matrix(D3DTS::VIEW, m_eProjType))))
        return E_FAIL;
    if (FAILED(m_pShaderCom->Bind_Matrix("g_ProjMatrix", m_pGameInstance_Proxy->Get_Matrix(D3DTS::PROJ, m_eProjType))))
        return E_FAIL;

    const _uint iNumMeshes = static_cast<_uint>(m_pModelCom->Get_NumMeshes());
    for (_uint i = 0; i < iNumMeshes; ++i)
    {
        if (FAILED(m_pModelCom->Bind_Material(m_pShaderCom, "g_DiffuseTexture", i, MTEX_TYPE::DIFFUSE, 0)))
            return E_FAIL;
        if (FAILED(m_pModelCom->Bind_Material(m_pShaderCom, "g_NormalTexture", i, MTEX_TYPE::NORMALS, 0)))
            return E_FAIL;
        if (FAILED(m_pModelCom->Bind_Material(m_pShaderCom, "g_MRATexture", i, MTEX_TYPE::METALNESS, 0)))
            return E_FAIL;
        if (FAILED(m_pModelCom->Bind_Material(m_pShaderCom, "g_CrystalTexture", i, MTEX_TYPE::UNKNOWN, 1)))
            return E_FAIL;
        if (FAILED(m_pModelCom->Bind_Material(m_pShaderCom, "g_HeightTexture", i, MTEX_TYPE::UNKNOWN, 0)))
            return E_FAIL;

        if (FAILED(m_pShaderCom->Begin(7)))
            return E_FAIL;
        if (FAILED(m_pModelCom->Render(i)))
            return E_FAIL;
    }

    return S_OK;
}

void CBoss_Metaknight_Sword::Drop_Freeze()
{
    if (m_bDropped)
        return;
    m_bDropped = true;

    m_DropWorld = m_CombinedWorldMatrix;

    if (m_pHitBox)  m_pHitBox->Set_Enabled(false);
    if (m_pPickBox) m_pPickBox->Set_Enabled(true);
}

void CBoss_Metaknight_Sword::Retire_Drop()
{
    if (m_pPickBox) m_pPickBox->Set_Enabled(false);
    Set_Active(false);
}

HRESULT CBoss_Metaknight_Sword::Ready_Components()
{
    PART_SETUP t{};
    t.tShader = Shader_Metaknight;
    t.szModelProtoTag = MODEL_PROTO_TAG;
    t.bAnimated = false;
    return Ready_MeshPart(t);
}

CBoss_Metaknight_Sword* CBoss_Metaknight_Sword::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CBoss_Metaknight_Sword* pInstance = new CBoss_Metaknight_Sword(pDevice, pContext);
    if (FAILED(pInstance->Initialize_Prototype())) {
        MSG_BOX("Failed to Created: CBoss_Metaknight_Sword");
        Safe_Release(pInstance);
    }
    return pInstance;
}

CBoss_Metaknight_Sword* CBoss_Metaknight_Sword::Clone(void* pArg)
{
    CBoss_Metaknight_Sword* pInstance = new CBoss_Metaknight_Sword(*this);
    if (FAILED(pInstance->Initialize(pArg))) {
        MSG_BOX("Failed to Cloned: CBoss_Metaknight_Sword");
        Safe_Release(pInstance);
    }
    return pInstance;
}

void CBoss_Metaknight_Sword::Free() { __super::Free(); }