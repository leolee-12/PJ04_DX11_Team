#include "Effect_Part.h"
#include "GameInstance.h"

CEffect_Part::CEffect_Part(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CGameObject(pDevice, pContext)
{
}

CEffect_Part::CEffect_Part(const CEffect_Part& Prototype)
    : CGameObject(Prototype)
{
}

HRESULT CEffect_Part::Initialize_Prototype()
{
    return S_OK;
}

HRESULT CEffect_Part::Initialize(void* pArg)
{
    EFFECT_PART_DESC* pDesc = static_cast<EFFECT_PART_DESC*>(pArg);

    if (FAILED(__super::Initialize(pArg)))
        return E_FAIL;
   
    m_pTransformCom->Set_State(STATE::POSITION, XMVectorSetW(XMLoadFloat3(&pDesc->vLocalPos), 1.f));

    return S_OK;
}

void CEffect_Part::Priority_Update(_float fTimeDelta)
{

}

void CEffect_Part::Update(_float fTimeDelta)
{
    if (m_bIsPlay == false)
        return;

    m_fAccTime += fTimeDelta;

    if (m_fAccTime < m_fDelayTime)
    {
        m_bActive = false;
        return;
    }

    m_bActive = true;


}

void CEffect_Part::Late_Update(_float fTimeDelta)
{
}

HRESULT CEffect_Part::Render()
{
    return S_OK;
}

void CEffect_Part::Free()
{
    __super::Free();
}