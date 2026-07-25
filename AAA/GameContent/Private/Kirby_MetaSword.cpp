#include "Kirby_MetaSword.h"

#include "GameInstance.h"

#include "Kirby.h"
#include "Kirby_Body.h"

CKirby_MetaSword::CKirby_MetaSword(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CKirby_OnOffPart(pDevice, pContext)
{
}

CKirby_MetaSword::CKirby_MetaSword(const CKirby_MetaSword& Prototype)
    : CKirby_OnOffPart(Prototype)
{
}

HRESULT CKirby_MetaSword::Initialize_Prototype()
{
    m_eProjType = PROJ_TYPE::PERSPEC;
    return S_OK;
}

HRESULT CKirby_MetaSword::Initialize(void* pArg)
{
    KIRBY_METASWORD_DESC* pDesc = static_cast<KIRBY_METASWORD_DESC*>(pArg);

    if (FAILED(__super::Initialize(pDesc)))
        return E_FAIL;

    if (FAILED(Ready_Components()))
        return E_FAIL;

    if (FAILED(Ready_HitBox()))
        return E_FAIL;

    m_pAnimatorCom->Play("Reset", true, true);

    return S_OK;
}

void CKirby_MetaSword::Late_Update(_float fTimeDelta)
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

HRESULT CKirby_MetaSword::Render()
{
    if (FAILED(Bind_ShaderResources()))
        return E_FAIL;

    const size_t iNumMeshes = m_pModelCom->Get_NumMeshes();

    static constexpr _uint iJewelMeshIndex = 1;

    for (_uint i = 0; i < iNumMeshes; ++i)
    {
        _uint iPassIndex{};

        if (i != iJewelMeshIndex)
        {
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
            if (FAILED(m_pModelCom->Bind_Material(m_pShaderCom, "g_DiffuseTexture", i, MTEX_TYPE::UNKNOWN, 1)))
                return E_FAIL;

            iPassIndex = ETOUI(KIRBY_SHADER_PASS::META_SWORD);
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

void CKirby_MetaSword::Begin_Hit(const ATTACK_INFO& tInfo, _bool bResetHitList)
{
    m_tAttackInfo = tInfo;

    if (m_tAttackInfo.pAttacker == nullptr)
        m_tAttackInfo.pAttacker = this;

    if (bResetHitList)
        Reset_DamagedList();

    Set_HitBox(true);
}

void CKirby_MetaSword::End_Hit(_bool bResetHitList)
{
    if (bResetHitList)
        Reset_DamagedList();

    Set_HitBox(false);
}

void CKirby_MetaSword::Set_HitBox(_bool bOn)
{
    m_pHitBox->Set_Enabled(bOn);
}

HRESULT CKirby_MetaSword::Ready_Components()
{
    KIRBY_PART_COMPONENT_DESC tDesc{};
    tDesc.tShaderDesc = Shader_Kirby;
    tDesc.szModelProtoTag = TEXT("Prototype_Component_Model_MetaSword");
    tDesc.bCreateAnimator = true;

    if (FAILED(Ready_PartComponents(tDesc)))
        return E_FAIL;

    return S_OK;
}

HRESULT CKirby_MetaSword::Ready_HitBox()
{
    CCollider::COLLIDER_DESC desc{};
    desc.pOwner = this;
    desc.vCenter = { 0.f, 0.f, 0.f };
    desc.fRadius = { 1.f };
    desc.fHeight = { 1.f };
    desc.vRadians = { XMConvertToRadians(-90.f), 0.f, 0.f };
    m_pHitBox = Add_Component<CCollider>(
        Collider_Capsule.iLevelID, Collider_Capsule.szProtoTag, TEXT("HitBox_Com"), &desc);

    if (m_pHitBox == nullptr)
        return E_FAIL;

    SetUp_HitBox_Callback();
    m_pHitBox->Set_Enabled(false);

    m_pGameInstance_Proxy->Register_Collider(m_pHitBox, ETOUI(COLLISION_LAYER::PLAYER_HIT));

    return S_OK;
}

void CKirby_MetaSword::SetUp_HitBox_Callback()
{
    m_pHitBox->Set_OnEnter([this](CCollider* pOther)
        {
            CGameObject* pTarget = pOther->Get_Owner();
            if (pTarget == nullptr)
                return;

            if (m_DamagedTargets.count(pTarget) > 0)
                return;

            IDamageable* pDamageable = dynamic_cast<IDamageable*>(pTarget);
            if (pDamageable == nullptr)
                return;

            ATTACK_INFO tDesc = m_tAttackInfo;
            tDesc.pAttacker = this;
            tDesc.vAttackerPos = {
                m_pParentMatrix->_41,
                m_pParentMatrix->_42,
                m_pParentMatrix->_43
            };

            pDamageable->Damaged(tDesc);

            m_DamagedTargets.insert(pTarget);
#ifdef _DEBUG
            OutputDebugStringA("[Kirby_MetaSword] HIT Something!\n");
#endif
        }
    );
}

CKirby_MetaSword* CKirby_MetaSword::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CKirby_MetaSword* pInstance = new CKirby_MetaSword(pDevice, pContext);

    if (FAILED(pInstance->Initialize_Prototype()))
    {
        MSG_BOX("Failed to Created: CKirby_MetaSword");
        Safe_Release(pInstance);
    }

    return pInstance;
}

CGameObject* CKirby_MetaSword::Clone(void* pArg)
{
    CKirby_MetaSword* pInstance = new CKirby_MetaSword(*this);

    if (FAILED(pInstance->Initialize(pArg)))
    {
        MSG_BOX("Failed to Cloned: CKirby_MetaSword");
        Safe_Release(pInstance);
    }

    return pInstance;
}

void CKirby_MetaSword::Free()
{
    __super::Free();
}