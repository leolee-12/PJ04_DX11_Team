#include "Kirby_ToyHammer.h"

#include "Kirby.h"
#include "Kirby_Body.h"

CKirby_ToyHammer::CKirby_ToyHammer(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CKirby_OnOffPart(pDevice, pContext)
{
}

CKirby_ToyHammer::CKirby_ToyHammer(const CKirby_ToyHammer& Prototype)
    : CKirby_OnOffPart(Prototype)
{
}

HRESULT CKirby_ToyHammer::Initialize_Prototype()
{
    m_eProjType = PROJ_TYPE::PERSPEC;
    return S_OK;
}

HRESULT CKirby_ToyHammer::Initialize(void* pArg)
{
    KIRBY_TOYHAMMER_DESC* pDesc = static_cast<KIRBY_TOYHAMMER_DESC*>(pArg);

    if (FAILED(__super::Initialize(pDesc)))
        return E_FAIL;

    if (FAILED(Ready_Components()))
        return E_FAIL;

    if (FAILED(Ready_HitBox()))
        return E_FAIL;

    m_pAnimatorCom->Play("Reset", true, true);

    return S_OK;
}

void CKirby_ToyHammer::Late_Update(_float fTimeDelta)
{
    __super::Late_Update(fTimeDelta);

    if (!m_bOn)
        return;

    Cal_HammerHeadEffectSocket();

    if (m_pHitBox && m_pHitBox->Is_Enabled())
    {
        m_pHitBox->Update(XMLoadFloat4x4(&m_CombinedWorldMatrix));
#ifdef _DEBUG
        m_pGameInstance_Proxy->Add_DebugComponent(m_pHitBox);
#endif
    }
}

HRESULT CKirby_ToyHammer::Render()
{
    if (FAILED(Bind_ShaderResources()))
        return E_FAIL;

    const size_t iNumMeshes = m_pModelCom->Get_NumMeshes();

    for (_uint i = 0; i < iNumMeshes; ++i)
    {
        if (m_bBurn && (i == TOY_HAMMER_MESH::HEAD || i == TOY_HAMMER_MESH::TOP))
            continue;

        if (FAILED(m_pModelCom->Bind_Material(m_pShaderCom, "g_DiffuseTexture", i, MTEX_TYPE::DIFFUSE, 0)))
            return E_FAIL;

        if (FAILED(m_pModelCom->Bind_Material(m_pShaderCom, "g_NormalTexture", i, MTEX_TYPE::NORMALS, 0)))
            return E_FAIL;

        if (FAILED(m_pModelCom->Bind_Material(m_pShaderCom, "g_MRATexture", i, MTEX_TYPE::METALNESS, 0)))
            return E_FAIL;

        if (FAILED(m_pModelCom->Bind_BoneMatrices(m_pShaderCom, "g_BoneMatrices", i)))
            return E_FAIL;

        if (FAILED(m_pShaderCom->Begin(ETOUI(KIRBY_SHADER_PASS::ANIM_TEXTURED_PBR))))
            return E_FAIL;

        if (FAILED(m_pModelCom->Render(i)))
            return E_FAIL;
    }

    return S_OK;
}

_bool CKirby_ToyHammer::Should_RenderShadowMesh(_uint iMeshIndex)
{
    switch (iMeshIndex)
    {
        case TOY_HAMMER_MESH::HANDLE:
            return true;

        case TOY_HAMMER_MESH::HEAD:
        case TOY_HAMMER_MESH::TOP:
            return !m_bBurn;
    }

    return false;
}

void CKirby_ToyHammer::Cal_HammerHeadEffectSocket()
{
    //const _float4x4* pHammerHeadBoneMatrix = m_pModelCom->Get_BoneMatrixPtr("HammerheadJ");
    const _float4x4* pHammerHeadBoneMatrix = m_pModelCom->Get_BoneMatrixPtr("HammertopJ");

    const _matrix HammerHeadWorldMatrix = XMLoadFloat4x4(pHammerHeadBoneMatrix) * XMLoadFloat4x4(&m_CombinedWorldMatrix);

    // 크기, 회전 제거
    const _vector vHeadPosition = HammerHeadWorldMatrix.r[3];
    XMStoreFloat4x4(&m_HammerHeadWorldMatrix, XMMatrixTranslationFromVector(vHeadPosition));
}

void CKirby_ToyHammer::Set_PartMode(CKirby* pKirby, KIRBY_PART_MODE ePartMode)
{
    switch (ePartMode)
    {
        case KIRBY_PART_MODE::BACK:
        {
            Set_SocketBoneMatrix(pKirby->Get_Body()->Get_BoneMatrixPtr("FloaterL"));
            m_pAnimatorCom->Play("Carry", true, true, 0.f);
            break;
        }
        case KIRBY_PART_MODE::DEFAULT:
        default:
        {
            Set_SocketBoneMatrix(pKirby->Get_Body()->Get_BoneMatrixPtr("RHaveL"));
            m_pAnimatorCom->Play("Reset", true, true, 0.f);
            break;
        }
    }
}

void CKirby_ToyHammer::Begin_Hit(const ATTACK_INFO& tInfo, _bool bResetHitList)
{
    m_tAttackInfo = tInfo;

    if (m_tAttackInfo.pAttacker == nullptr)
        m_tAttackInfo.pAttacker = this;

    if (bResetHitList)
        Reset_DamagedList();

    Set_HitBox(true);
}

void CKirby_ToyHammer::End_Hit(_bool bResetHitList)
{
    if (bResetHitList)
        Reset_DamagedList();

    Set_HitBox(false);
}

void CKirby_ToyHammer::Set_HitBox(_bool bOn)
{
    m_pHitBox->Set_Enabled(bOn);
}

void CKirby_ToyHammer::Change_HitBox(TOY_HAMMER_HITBOX_TYPE eHitBoxType)
{
    CCollider::COLLIDER_DESC tDesc{};
    tDesc.pOwner = this;
    tDesc.vRadians = { 0.f, 0.f, XMConvertToRadians(-90.f) };

    switch (eHitBoxType)
    {
        case TOY_HAMMER_HITBOX_TYPE::HAMMER_ATTACK:
            tDesc.vCenter = { -1.105f, -0.002f, -1.843f };
            tDesc.fRadius = 1.11f;
            tDesc.fHeight = 0.f;
            break;

        case TOY_HAMMER_HITBOX_TYPE::HAMMER_ATTACK_FINAL:
            tDesc.vCenter = { -1.183f, -0.004f, -1.761f };
            tDesc.fRadius = 1.18f;
            tDesc.fHeight = 0.f;
            break;

        case TOY_HAMMER_HITBOX_TYPE::CHARGE_ATTACK_1:
            tDesc.vCenter = { -0.912f, -0.002f, -1.006f };
            tDesc.fRadius = 0.63f;
            tDesc.fHeight = 0.58f;
            break;

        case TOY_HAMMER_HITBOX_TYPE::CHARGE_ATTACK_2:
            tDesc.vCenter = { -1.131f, -0.002f, -1.497f };
            tDesc.fRadius = 0.79f;
            tDesc.fHeight = 0.68f;
            break;

        case TOY_HAMMER_HITBOX_TYPE::CHARGE_ATTACK_3:
            tDesc.vCenter = { -1.809f, -0.012f, -2.163f };
            tDesc.fRadius = 1.26f;
            tDesc.fHeight = 1.1f;
            break;

        case TOY_HAMMER_HITBOX_TYPE::CHARGE_ATTACK_4:
            tDesc.vCenter = { -0.68f, -0.001f, -0.5f };
            tDesc.fRadius = 0.63f;
            tDesc.fHeight = 0.1f;
            break;

        case TOY_HAMMER_HITBOX_TYPE::WHEELHAMMER:
            tDesc.vCenter = { -1.315f, -0.004f, -1.682f };
            tDesc.fRadius = 0.94f;
            tDesc.fHeight = 0.87f;
            break;

        case TOY_HAMMER_HITBOX_TYPE::WHEELHAMMER_FALL:
            tDesc.vCenter = { -1.31f, -0.002f, -1.618f };
            tDesc.fRadius = 0.91f;
            tDesc.fHeight = 0.8f;
            break;
        default:
            return;
    }

    m_pHitBox->Reset_Bounding(tDesc);
}

_bool CKirby_ToyHammer::Is_HammerHeadCollision(_float fNormalY, _float3* pOutPos, _float3* pOutNormal)
{
    const _float4x4* pLeftBoneMatrix = m_pModelCom->Get_BoneMatrixPtr("HammerheadLJ");
    const _float4x4* pRightBoneMatrix = m_pModelCom->Get_BoneMatrixPtr("HammerheadRJ");
    if (pLeftBoneMatrix == nullptr || pRightBoneMatrix == nullptr)
        return false;

    const _matrix CombinedWorldMatrix = XMLoadFloat4x4(&m_CombinedWorldMatrix);
    const _vector vLeftPosition = (XMLoadFloat4x4(pLeftBoneMatrix) * CombinedWorldMatrix).r[3];
    const _vector vRightPosition = (XMLoadFloat4x4(pRightBoneMatrix) * CombinedWorldMatrix).r[3];

    _float3 vSweepStart{};
    _float3 vSweepDirection{};
    XMStoreFloat3(&vSweepStart, vLeftPosition);
    XMStoreFloat3(&vSweepDirection, XMVector3Normalize(vLeftPosition - vRightPosition));

    constexpr _float fSweepRadius = 0.2f;
    constexpr _float fSweepDistance = 0.05f;

    _float3 vHitNormal{};
    _float fHitDistance{};
    if (!m_pGameInstance_Proxy->Sweep_Sphere(vSweepStart, fSweepRadius, vSweepDirection, fSweepDistance,
        &vHitNormal, &fHitDistance, true, false))
    {
        return false;
    }

    if (pOutPos != nullptr)
    {
        XMStoreFloat3(pOutPos, XMLoadFloat3(&vSweepStart) +
            XMLoadFloat3(&vSweepDirection) * fHitDistance -
            XMLoadFloat3(&vHitNormal) * fSweepRadius);
    }

    if (pOutNormal != nullptr)
        *pOutNormal = vHitNormal;

    if (vHitNormal.y <= 0.5f)
        int a = 10;

    return vHitNormal.y >= fNormalY;
}

HRESULT CKirby_ToyHammer::Ready_Components()
{
    KIRBY_PART_COMPONENT_DESC tDesc{};
    tDesc.tShaderDesc = Shader_Kirby;
    tDesc.szModelProtoTag = TEXT("Prototype_Component_Model_ToyHammer");
    tDesc.bCreateAnimator = true;

    if (FAILED(Ready_PartComponents(tDesc)))
        return E_FAIL;

    return S_OK;
}

HRESULT CKirby_ToyHammer::Ready_HitBox()
{
    CCollider::COLLIDER_DESC desc{};
    desc.pOwner = this;
    desc.vCenter = { -1.81f, -0.01f, -2.f };
    desc.fRadius = 1.1f;
    desc.fHeight = 1.1f;
    desc.vRadians = { 0.f, 0.f, XMConvertToRadians(-90.f) };

    m_pHitBox = Add_Component<CCollider>(Collider_Capsule.iLevelID, Collider_Capsule.szProtoTag, TEXT("HitBox_Com"), &desc);

    if (m_pHitBox == nullptr)
        return E_FAIL;

    SetUp_HitBox_Callback();
    m_pHitBox->Set_Enabled(false);
    m_pGameInstance_Proxy->Register_Collider(m_pHitBox, ETOUI(COLLISION_LAYER::PLAYER_HIT));

    return S_OK;
}

void CKirby_ToyHammer::SetUp_HitBox_Callback()
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
        }
    );
}

CKirby_ToyHammer* CKirby_ToyHammer::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CKirby_ToyHammer* pInstance = new CKirby_ToyHammer(pDevice, pContext);

    if (FAILED(pInstance->Initialize_Prototype()))
    {
        MSG_BOX("Failed to Created: CKirby_ToyHammer");
        Safe_Release(pInstance);
    }

    return pInstance;
}

CGameObject* CKirby_ToyHammer::Clone(void* pArg)
{
    CKirby_ToyHammer* pInstance = new CKirby_ToyHammer(*this);

    if (FAILED(pInstance->Initialize(pArg)))
    {
        MSG_BOX("Failed to Cloned: CKirby_ToyHammer");
        Safe_Release(pInstance);
    }

    return pInstance;
}

void CKirby_ToyHammer::Free()
{
    __super::Free();
}