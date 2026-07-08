#include "Kirby_Sword.h"

#include "GameInstance.h"

#include "Damageable.h"

#include "Kirby.h"
#include "Kirby_Body.h"

CKirby_Sword::CKirby_Sword(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CKirby_OnOffPart(pDevice, pContext)
{
}

CKirby_Sword::CKirby_Sword(const CKirby_Sword& Prototype)
    : CKirby_OnOffPart(Prototype) {
}

HRESULT CKirby_Sword::Initialize_Prototype()
{
    m_eProjType = PROJ_TYPE::PERSPEC;
    return S_OK;
}

HRESULT CKirby_Sword::Initialize(void* pArg)
{
    KIRBY_SWORD_DESC* pDesc = static_cast<KIRBY_SWORD_DESC*>(pArg);

    if (FAILED(__super::Initialize(pDesc)))
        return E_FAIL;

    if (FAILED(Ready_Components()))
        return E_FAIL;

    if (FAILED(Ready_HitBox()))
        return E_FAIL;

    m_pAnimatorCom->Play("Reset", true, true);

    return S_OK;
}

void CKirby_Sword::Late_Update(_float fTimeDelta)
{
    __super::Late_Update(fTimeDelta);

    if (!m_bOn)
        return;

    if (m_pHitBox && m_pHitBox->Is_Enabled())
    {
        m_pHitBox->Update(XMLoadFloat4x4(&m_CombinedWorldMatrix));
#ifdef _DEBUG
        m_pGameInstance_Proxy->Add_DebugComponent(m_pHitBox);
#endif
    }
}

HRESULT CKirby_Sword::Render()
{
    if (FAILED(Bind_ShaderResources()))
        return E_FAIL;

    const size_t iNumMeshes = m_pModelCom->Get_NumMeshes();

    static constexpr _uint iJewelMeshIndex = 1;

    for (_uint i = 0; i < iNumMeshes; ++i)
    {
        _uint iPassIndex{};

        if (i != iJewelMeshIndex) {

            if (FAILED(m_pModelCom->Bind_Material(m_pShaderCom, "g_DiffuseTexture", i, MTEX_TYPE::DIFFUSE, 0)))
                return E_FAIL;

            if (FAILED(m_pModelCom->Bind_Material(m_pShaderCom, "g_NormalTexture", i, MTEX_TYPE::NORMALS, 0)))
                return E_FAIL;

            if (FAILED(m_pModelCom->Bind_Material(m_pShaderCom, "g_MRATexture", i, MTEX_TYPE::METALNESS, 0)))
                return E_FAIL;

            iPassIndex = ETOUI(KIRBY_SHADER_PASS::ANIM_TEXTURED_PBR);
        }
        else
        {
            static constexpr _float4 vConstantDiffuse = { 1.f, 0.72f, 0.08f, 1.f };
            if (FAILED(m_pShaderCom->Bind_RawValue("g_vConstantDiffuse", &vConstantDiffuse, sizeof(_float4))))
                return E_FAIL;

            static constexpr _float3 vConstantMRA = { 0.25f, 0.18f, 1.f };
            if (FAILED(m_pShaderCom->Bind_RawValue("g_vConstantMRA", &vConstantMRA, sizeof(_float3))))
                return E_FAIL;

            static constexpr _float4 vConstantEmissive = { 0.05f, 0.025f, 0.f, 1.f };
            if (FAILED(m_pShaderCom->Bind_RawValue("g_vConstantEmissive", &vConstantEmissive, sizeof(_float4))))
                return E_FAIL;

            iPassIndex = ETOUI(KIRBY_SHADER_PASS::ANIM_CONSTANT_PBR);
        }

        if (FAILED(m_pModelCom->Bind_BoneMatrices(m_pShaderCom, "g_BoneMatrices", i)))
            return E_FAIL;

        if (FAILED(m_pShaderCom->Begin(iPassIndex)))
            return E_FAIL;

        if (FAILED(m_pModelCom->Render(i)))
            return E_FAIL;
    }

    return S_OK;
}

void CKirby_Sword::Put_OnBack(CKirby* pKirby, _bool bOn)
{
    if (bOn)
    {
        Set_SocketBoneMatrix(pKirby->Get_Body()->Get_BoneMatrixPtr("FloaterL"));
        m_pAnimatorCom->Play("Carry", true, true, 0.f);
    }
    else
    {
        Set_SocketBoneMatrix(pKirby->Get_Body()->Get_BoneMatrixPtr("RHaveL"));
        m_pAnimatorCom->Play("Reset", true, true, 0.f);
    }
}

void CKirby_Sword::Set_HitBox(_bool bOn)
{
    if (m_pHitBox)
        m_pHitBox->Set_Enabled(bOn);
}

HRESULT CKirby_Sword::Ready_Components()
{
    KIRBY_PART_COMPONENT_DESC°¡ tDesc{};
    tDesc.tShaderDesc = Shader_Kirby;
    tDesc.szModelProtoTag = TEXT("Prototype_Component_Model_Sword");
    tDesc.bCreateAnimator = true;

    if (FAILED(Ready_PartComponents(tDesc)))
        return E_FAIL;

    return S_OK;
}

HRESULT CKirby_Sword::Ready_HitBox()
{
    CCollider::COLLIDER_DESC desc{};
    desc.pOwner = this;
    desc.vCenter = {0.f, 0.f, -0.5f};
    desc.fRadius = {0.25f};
    desc.fHeight = {0.8f};
    desc.vRadians = { XMConvertToRadians(-90.f), 0.f, 0.f};
    m_pHitBox = Add_Component<CCollider>(Collider_Capsule.iLevelID, Collider_Capsule.szProtoTag,
        TEXT("HitBox_Com"), &desc);
    if (nullptr == m_pHitBox) return E_FAIL;

    SetUp_HitBox_Callback();
    m_pHitBox->Set_Enabled(false);
    m_pGameInstance_Proxy->Register_Collider(m_pHitBox,
        ETOUI(COLLISION_LAYER::PLAYER_HIT));

    return S_OK;
}

void CKirby_Sword::SetUp_HitBox_Callback()
{
    m_pHitBox->Set_OnEnter([this](CCollider* pOther)
        {
            CGameObject* pTarget = pOther->Get_Owner();
            if (m_HitTargets.count(pTarget))   
                return;

            IDamageable* pVictim = dynamic_cast<IDamageable*>(pTarget);
            if (nullptr == pVictim) return;

            ATTACK_INFO atk{};
            atk.fDamage = 500.f;
            atk.fKnockback = 6.f;
            atk.vAttackerPos = _float3(m_pParentMatrix->_41, m_pParentMatrix->_42, m_pParentMatrix->_43);
            atk.pAttacker = this;
            pVictim->Damaged(atk);

            m_HitTargets.insert(pTarget);   
#ifdef _DEBUG
            OutputDebugStringA("[Kirby_Sword] HIT Somthing!\n");
#endif
        });
}

CKirby_Sword* CKirby_Sword::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CKirby_Sword* pInstance = new CKirby_Sword(pDevice, pContext);

    if (FAILED(pInstance->Initialize_Prototype()))
    {
        MSG_BOX("Failed to Created: CKirby_Sword");
        Safe_Release(pInstance);
    }

    return pInstance;
}

CGameObject* CKirby_Sword::Clone(void* pArg)
{
    CKirby_Sword* pInstance = new CKirby_Sword(*this);

    if (FAILED(pInstance->Initialize(pArg)))
    {
        MSG_BOX("Failed to Cloned: CKirby_Sword");
        Safe_Release(pInstance);
    }

    return pInstance;
}

void CKirby_Sword::Free()
{
    __super::Free();
}