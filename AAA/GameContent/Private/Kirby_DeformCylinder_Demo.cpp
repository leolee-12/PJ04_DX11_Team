#include "Kirby_DeformCylinder_Demo.h"

#include "GameInstance.h"

CKirby_DeformCylinder_Demo::CKirby_DeformCylinder_Demo(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CKirby_Deform_Model(pDevice, pContext)
{
}

CKirby_DeformCylinder_Demo::CKirby_DeformCylinder_Demo(const CKirby_DeformCylinder_Demo& Prototype)
    : CKirby_Deform_Model(Prototype)
{
}

HRESULT CKirby_DeformCylinder_Demo::Initialize_Prototype()
{
    m_eProjType = PROJ_TYPE::PERSPEC;

    return S_OK;
}

HRESULT CKirby_DeformCylinder_Demo::Initialize(void* pArg)
{
    if (FAILED(__super::Initialize(pArg)))
        return E_FAIL;

    if (FAILED(Ready_Components()))
        return E_FAIL;

    m_bActive = false;

    return S_OK;
}

HRESULT CKirby_DeformCylinder_Demo::Render()
{
    return S_OK;
}

HRESULT CKirby_DeformCylinder_Demo::Ready_AnimEvents(CKirby* pKirby)
{
    return S_OK;
}

HRESULT CKirby_DeformCylinder_Demo::Ready_Components()
{
    return S_OK;
}

CKirby_DeformCylinder_Demo* CKirby_DeformCylinder_Demo::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CKirby_DeformCylinder_Demo* pInstance = new CKirby_DeformCylinder_Demo(pDevice, pContext);

    if (FAILED(pInstance->Initialize_Prototype()))
    {
        MSG_BOX("Failed to Created: CKirby_DeformCylinder_Demo");
        Safe_Release(pInstance);
    }

    return pInstance;
}

CGameObject* CKirby_DeformCylinder_Demo::Clone(void* pArg)
{
    CKirby_DeformCylinder_Demo* pInstance = new CKirby_DeformCylinder_Demo(*this);

    if (FAILED(pInstance->Initialize(pArg)))
    {
        MSG_BOX("Failed to Cloned: CKirby_DeformCylinder_Demo");
        Safe_Release(pInstance);
    }

    return pInstance;
}

void CKirby_DeformCylinder_Demo::Free()
{
    __super::Free();
}
