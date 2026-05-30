#include "Effect_Quad.h"

#include "GameInstance.h"

CEffect_Quad::CEffect_Quad(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CEffect_Part(pDevice, pContext)
{
}

CEffect_Quad::CEffect_Quad(const CEffect_Quad& Prototype)
    : CEffect_Part(Prototype)
{
}

HRESULT CEffect_Quad::Initialize_Prototype()
{
    return S_OK;
}

HRESULT CEffect_Quad::Initialize(void* pArg)
{
    EFFECT_PART_DESC* pDesc = static_cast<EFFECT_PART_DESC*>(pArg);

    if (FAILED(__super::Initialize(pArg)))
        return E_FAIL;

    return S_OK;
}

void CEffect_Quad::Priority_Update(_float fTimeDelta)
{

}

void CEffect_Quad::Update(_float fTimeDelta)
{

}

void CEffect_Quad::Late_Update(_float fTimeDelta)
{
}

HRESULT CEffect_Quad::Render()
{
    return S_OK;
}

void CEffect_Quad::Free()
{
    __super::Free();
}