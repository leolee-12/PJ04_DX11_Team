#include "Kirby_HitBox_Model.h"

#include "GameInstance.h"

CKirby_HitBox_Model::CKirby_HitBox_Model(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CKirby_Deform_Model(pDevice, pContext)
{
}

CKirby_HitBox_Model::CKirby_HitBox_Model(const CKirby_HitBox_Model& Prototype)
    : CKirby_Deform_Model(Prototype)
{
}

HRESULT CKirby_HitBox_Model::Initialize_Prototype()
{
    return S_OK;
}

HRESULT CKirby_HitBox_Model::Initialize(void* pArg)
{
    if (FAILED(__super::Initialize(pArg)))
        return E_FAIL;

    return S_OK;
}

void CKirby_HitBox_Model::Late_Update(_float fTimeDelta)
{
    __super::Late_Update(fTimeDelta);

    if (!m_bActive || !m_pHitBox || !m_pHitBox->Is_Enabled())
        return;

    m_pHitBox->Update(XMLoadFloat4x4(&m_CombinedWorldMatrix));

#ifdef _DEBUG
    if (m_pHitBox->Is_Enabled())
        m_pGameInstance_Proxy->Add_DebugComponent(m_pHitBox);
#endif  
}

void CKirby_HitBox_Model::Set_HitBoxEnabled(_bool bOn)
{
    if (m_pHitBox)
        m_pHitBox->Set_Enabled(bOn);
}

void CKirby_HitBox_Model::Free()
{
    __super::Free();
}