#include "WalkSmoke.h"

#include "GameInstance.h"
#include "GameContent_const.h"

#include "SmokeSphereOriginal.h"
#include "SmokeLowPoly.h"
#include "SmokeTail.h"

CWalkSmoke::CWalkSmoke(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CEffect_Container(pDevice, pContext)
{
}

CWalkSmoke::CWalkSmoke(const CWalkSmoke& Prototype)
    : CEffect_Container(Prototype)
{
}

HRESULT CWalkSmoke::Initialize_Prototype()
{
    return S_OK;
}

HRESULT CWalkSmoke::Initialize(void* pArg)
{
    WALK_SMOKE_DESC* pDesc = static_cast<WALK_SMOKE_DESC*>(pArg);

    if (FAILED(__super::Initialize(pArg)))
        return E_FAIL;

    if (FAILED(Ready_EffectPartObjects()))
        return E_FAIL;

    return S_OK;
}

void CWalkSmoke::Priority_Update(_float fTimeDelta)
{
    __super::Priority_Update(fTimeDelta);
}

void CWalkSmoke::Update(_float fTimeDelta)
{
    __super::Update(fTimeDelta);
}

void CWalkSmoke::Late_Update(_float fTimeDelta)
{
    __super::Late_Update(fTimeDelta);
}

HRESULT CWalkSmoke::Render()
{
    __super::Render();

    return S_OK;
}

HRESULT CWalkSmoke::Ready_EffectPartObjects()
{
    Add_Effect_PartObject(ETOUI(LEVEL::GAMEPLAY), CSmokeSphereOriginal::PROTOTYPE_TAG, TEXT("Proto_SmokeSphereOriginal"));
    Add_Effect_PartObject(ETOUI(LEVEL::GAMEPLAY), CSmokeLowPoly::PROTOTYPE_TAG, TEXT("Prototype_Component_Model_SmokeLowPoly_1"));
    Add_Effect_PartObject(ETOUI(LEVEL::GAMEPLAY), CSmokeLowPoly::PROTOTYPE_TAG, TEXT("Prototype_Component_Model_SmokeLowPoly_2"));
    Add_Effect_PartObject(ETOUI(LEVEL::GAMEPLAY), CSmokeLowPoly::PROTOTYPE_TAG, TEXT("Prototype_Component_Model_SmokeLowPoly_3"));
    Add_Effect_PartObject(ETOUI(LEVEL::GAMEPLAY), CSmokeTail::PROTOTYPE_TAG, TEXT("Prototype_Component_Model_SmokeTail"));

    return S_OK;
}

CWalkSmoke* CWalkSmoke::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CWalkSmoke* pInstance = new CWalkSmoke(pDevice, pContext);

    if (FAILED(pInstance->Initialize_Prototype()))
    {
        MSG_BOX("Failed to Created: CWalkSmoke");
        Safe_Release(pInstance);
    }

    return pInstance;
}

CGameObject* CWalkSmoke::Clone(void* pArg)
{
    CWalkSmoke* pInstance = new CWalkSmoke(*this);

    if (FAILED(pInstance->Initialize(pArg)))
    {
        MSG_BOX("Failed to Cloned: CTestEffectQuad");
        Safe_Release(pInstance);
    }

    return pInstance;
}

void CWalkSmoke::Free()
{
    __super::Free();
}