#include "Boss_Cage.h"
#include "Boss_Cage_Body.h"

#include "GameInstance.h"

CBoss_Cage::CBoss_Cage(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CContainerObject{ pDevice, pContext }
{
}

CBoss_Cage::CBoss_Cage(const CBoss_Cage& Prototype)
    : CContainerObject(Prototype)
{
}

HRESULT CBoss_Cage::Initialize_Prototype()
{
    return S_OK;
}

HRESULT CBoss_Cage::Initialize(void* pArg)
{
    if (FAILED(__super::Initialize(pArg)))
        return E_FAIL;

    XMStoreFloat4x4(&m_AttachOffset, XMMatrixIdentity());

    if (FAILED(Ready_PartObjects()))
        return E_FAIL;

    return S_OK;
}

void CBoss_Cage::Priority_Update(_float fTimeDelta)
{
    if (!Is_Active())
        return;

    __super::Priority_Update(fTimeDelta);
}

void CBoss_Cage::Update(_float fTimeDelta)
{
    if (!Is_Active())
        return;

    __super::Update(fTimeDelta);
}

void CBoss_Cage::Late_Update(_float fTimeDelta)
{
    if (!Is_Active())
        return;

    // Late_Update에서 합성해야 본 주인(고릴라)의 애니메이터 갱신(Update 단계)이 끝난 뒤가 보장됨
    if (m_pAttachBone && m_pAttachOwnerWorld)
    {
        _matrix World = XMLoadFloat4x4(&m_AttachOffset)
            * XMLoadFloat4x4(m_pAttachBone)
            * XMLoadFloat4x4(m_pAttachOwnerWorld);
        m_pTransformCom->Set_WorldMatrix(World);
    }

    __super::Late_Update(fTimeDelta);   // 파트들이 위 월드를 pParentMatrix로 합성 + 렌더그룹 등록
}

HRESULT CBoss_Cage::Ready_PartObjects()
{
    CMonsterPart::MONSTERPART_DESC Desc{};
    Desc.pParentMatrix = m_pTransformCom->Get_WorldMatrixPtr();   // 파트가 이 월드로 합성

    if (FAILED(Add_PartObject(m_iPrototypeLevel, CBoss_Cage_Body::PROTOTYPE_TAG,
        CBoss_Cage_Body::PART_TAG, &Desc)))
        return E_FAIL;

    return S_OK;
}

void CBoss_Cage::Attach_To_Bone(const _float4x4* pBoneMatrix, const _float4x4* pOwnerWorld,
    _fmatrix OffsetMatrix)
{
    m_pAttachBone = pBoneMatrix;
    m_pAttachOwnerWorld = pOwnerWorld;
    XMStoreFloat4x4(&m_AttachOffset, OffsetMatrix);
}

void CBoss_Cage::Detach()
{
    m_pAttachBone = nullptr;
    m_pAttachOwnerWorld = nullptr;
}

CBoss_Cage* CBoss_Cage::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CBoss_Cage* pInstance = new CBoss_Cage(pDevice, pContext);
    if (FAILED(pInstance->Initialize_Prototype()))
    {
        MSG_BOX("Failed to Created: CBoss_Cage");
        Safe_Release(pInstance);
    }
    return pInstance;
}

CGameObject* CBoss_Cage::Clone(void* pArg)
{
    CBoss_Cage* pInstance = new CBoss_Cage(*this);
    if (FAILED(pInstance->Initialize(pArg)))
    {
        MSG_BOX("Failed to Cloned: CBoss_Cage");
        Safe_Release(pInstance);
    }
    return pInstance;
}

void CBoss_Cage::Free()
{
    __super::Free();
}