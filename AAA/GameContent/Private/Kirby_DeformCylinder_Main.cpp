#include "Kirby_DeformCylinder_Main.h"

#include "GameInstance.h"

CKirby_DeformCylinder_Main::CKirby_DeformCylinder_Main(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CKirby_HitBox_Model(pDevice, pContext)
{
}

CKirby_DeformCylinder_Main::CKirby_DeformCylinder_Main(const CKirby_DeformCylinder_Main& Prototype)
    : CKirby_HitBox_Model(Prototype)
{
}

HRESULT CKirby_DeformCylinder_Main::Initialize_Prototype()
{
    m_eProjType = PROJ_TYPE::PERSPEC;

    return S_OK;
}

HRESULT CKirby_DeformCylinder_Main::Initialize(void* pArg)
{
    if (FAILED(__super::Initialize(pArg)))
        return E_FAIL;

    if (FAILED(Ready_Components()))
        return E_FAIL;

    m_bActive = false;

    return S_OK;
}

HRESULT CKirby_DeformCylinder_Main::Render()
{
    return S_OK;
}

HRESULT CKirby_DeformCylinder_Main::Ready_AnimEvents(CKirby* pKirby)
{
    return S_OK;
}

HRESULT CKirby_DeformCylinder_Main::Ready_Components()
{
    return S_OK;
}

CKirby_DeformCylinder_Main* CKirby_DeformCylinder_Main::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CKirby_DeformCylinder_Main* pInstance = new CKirby_DeformCylinder_Main(pDevice, pContext);

    if (FAILED(pInstance->Initialize_Prototype()))
    {
        MSG_BOX("Failed to Created: CKirby_DeformCylinder_Main");
        Safe_Release(pInstance);
    }

    return pInstance;
}

CGameObject* CKirby_DeformCylinder_Main::Clone(void* pArg)
{
    CKirby_DeformCylinder_Main* pInstance = new CKirby_DeformCylinder_Main(*this);

    if (FAILED(pInstance->Initialize(pArg)))
    {
        MSG_BOX("Failed to Cloned: CKirby_DeformCylinder_Main");
        Safe_Release(pInstance);
    }

    return pInstance;
}

void CKirby_DeformCylinder_Main::Free()
{
    __super::Free();
}
