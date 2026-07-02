#include "Kirby_Sword.h"

#include "GameInstance.h"

#include "GameContent_const.h"

#include "Animator.h"

#include "Damageable.h"

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
    if (m_bOn == false)
        return;

    __super::Late_Update(fTimeDelta);

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

    size_t iNumMeshes = m_pModelCom->Get_NumMeshes();

    for (_uint i = 0; i < iNumMeshes; ++i)
    {
        _uint iPassIdx{};

        if (i != 1) {

            if (FAILED(m_pModelCom->Bind_Material(m_pShaderCom, "g_DiffuseTexture", i, MTEX_TYPE::DIFFUSE, 0)))
                return E_FAIL;

            if (FAILED(m_pModelCom->Bind_Material(m_pShaderCom, "g_NormalTexture", i, MTEX_TYPE::NORMALS, 0)))
                return E_FAIL;

            if (FAILED(m_pModelCom->Bind_Material(m_pShaderCom, "g_MRATexture", i, MTEX_TYPE::METALNESS, 0)))
                return E_FAIL;

            iPassIdx = 1;
        }
        else
        {
            m_pShaderCom->Bind_RawValue("g_vConstantDiffuse",   &m_vConstantDiffuse, sizeof(_float4));
            m_pShaderCom->Bind_RawValue("g_vConstantMRA",       &m_vConstantMRA, sizeof(_float3));
            m_pShaderCom->Bind_RawValue("g_vConstantEmissive",  &m_vConstantEmissive, sizeof(_float4));

            iPassIdx = 2;
        }

        if (FAILED(m_pModelCom->Bind_BoneMatrices(m_pShaderCom, "g_BoneMatrices", i)))
            return E_FAIL;

        if (FAILED(m_pShaderCom->Begin(iPassIdx)))
            return E_FAIL;

        if (FAILED(m_pModelCom->Render(i)))
            return E_FAIL;
    }

    return S_OK;
}

void CKirby_Sword::Set_HitBox(_bool bOn)
{
    if (m_pHitBox)
        m_pHitBox->Set_Enabled(bOn);
}

HRESULT CKirby_Sword::Ready_Components()
{
    PART_SETUP t{};
    t.tShader = Shader_Kirby;
    t.szModelProtoTag = TEXT("Prototype_Component_Model_Sword");
    t.bAnimated = true;
    return Ready_MeshPart(t);
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
            atk.fDamage = 50.f;
            atk.fKnockback = 4.f;
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