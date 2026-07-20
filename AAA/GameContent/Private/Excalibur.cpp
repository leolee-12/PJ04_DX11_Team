#include "Excalibur.h"
#include "Excalibur_Body.h"
#include "Excalibur_GetIt.h"

CExcalibur::CExcalibur(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CContainerObject{ pDevice, pContext } {
}
CExcalibur::CExcalibur(const CExcalibur& Prototype)
    : CContainerObject(Prototype) {
}

HRESULT CExcalibur::Initialize_Prototype()
{
    return S_OK;
}

HRESULT CExcalibur::Initialize(void* pArg)
{
    if (FAILED(__super::Initialize(pArg)))
        return E_FAIL;

    if (FAILED(Ready_PartObjects()))
        return E_FAIL;

    if (FAILED(Ready_Collider()))
        return E_FAIL;

    return S_OK;
}

void CExcalibur::Update(_float fTimeDelta)
{
    if (!Is_Active())
        return;

    __super::Update(fTimeDelta);
}

void CExcalibur::Late_Update(_float fTimeDelta)
{
    if (!Is_Active())
        return;

    __super::Late_Update(fTimeDelta);

    if (m_pHurtBox)
        m_pHurtBox->Update(XMLoadFloat4x4(m_pTransformCom->Get_WorldMatrixPtr()));
#ifdef _DEBUG
    if (m_pHurtBox)
        m_pGameInstance_Proxy->Add_DebugComponent(m_pHurtBox);
#endif
}

HRESULT CExcalibur::Ready_PartObjects()
{
    CMonsterPart::MONSTERPART_DESC Desc{};
    Desc.pParentMatrix = m_pTransformCom->Get_WorldMatrixPtr();

    if (FAILED(Add_PartObject(m_iPrototypeLevel, CExcalibur_Body::PROTOTYPE_TAG,
        CExcalibur_Body::PART_TAG, &Desc)))
        return E_FAIL;
    m_pBody = static_cast<CExcalibur_Body*>(m_PartObjects.find(CExcalibur_Body::PART_TAG)->second);
    if (!m_pBody) return E_FAIL;

    if (FAILED(Add_PartObject(m_iPrototypeLevel, CExcalibur_GetIt::PROTOTYPE_TAG,
        CExcalibur_GetIt::PART_TAG, &Desc)))
        return E_FAIL;
    m_pGetIt = static_cast<CExcalibur_GetIt*>(m_PartObjects.find(CExcalibur_GetIt::PART_TAG)->second);
    if (!m_pGetIt) return E_FAIL;

    return S_OK;
}

HRESULT CExcalibur::Ready_Collider()
{
    CCollider::COLLIDER_DESC HurtDesc{};
    HurtDesc.pOwner = this;
    HurtDesc.fRadius = 0.2f;
    HurtDesc.fHeight = 1.f;

    m_pHurtBox = Add_Component<CCollider>(Collider_Capsule.iLevelID, Collider_Capsule.szProtoTag,
        TEXT("MonHurtBox_Com"), &HurtDesc);
    if (m_pHurtBox == nullptr)
        return E_FAIL;

    m_pHurtBox->Set_OnEnter([this](CCollider* pOther) {
        if (ETOUI(COLLISION_LAYER::PLAYER_HURT) != pOther->Get_RegisteredGroup())
            return;
        
        
        });

    m_pGameInstance_Proxy->Register_Collider(m_pHurtBox, ETOUI(COLLISION_LAYER::EXCALIBUR));

    return S_OK;
}

void CExcalibur::Show_GetIt(_bool bOn)
{
    if (m_pGetIt)
        m_pGetIt->Set_Active(bOn);
}

CExcalibur* CExcalibur::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CExcalibur* pInstance = new CExcalibur(pDevice, pContext);
    if (FAILED(pInstance->Initialize_Prototype()))
    {
        MSG_BOX("Failed to Created : CExcalibur");
        Safe_Release(pInstance);
    }
    return pInstance;
}

CGameObject* CExcalibur::Clone(void* pArg)
{
    CExcalibur* pInstance = new CExcalibur(*this);
    if (FAILED(pInstance->Initialize(pArg)))
    {
        MSG_BOX("Failed to Cloned : CExcalibur");
        Safe_Release(pInstance);
    }
    return pInstance;
}

void CExcalibur::Free() { __super::Free(); }
