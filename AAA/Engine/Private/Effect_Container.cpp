#include "Effect_Container.h"

#include "GameInstance.h"

#include "Effect_Part.h"


CEffect_Container::CEffect_Container(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CGameObject(pDevice, pContext)
{
    Init_PropetyValue();
}

CEffect_Container::CEffect_Container(const CEffect_Container& Prototype)
    : CGameObject(Prototype)
{
    Init_PropetyValue();
}

HRESULT CEffect_Container::Initialize_Prototype()
{
    return S_OK;
}

HRESULT CEffect_Container::Initialize(void* pArg)
{
    XMStoreFloat4x4(&m_CombinedWorldMatrix, XMMatrixIdentity());    

    EFFECT_CONTAINER_DESC* pDesc = static_cast<EFFECT_CONTAINER_DESC*>(pArg);

    if (FAILED(__super::Initialize(pArg)))
        return E_FAIL;

    return S_OK;
}

void CEffect_Container::Priority_Update(_float fTimeDelta)
{
    for (auto& pEffectPart : m_EffestParts)
        pEffectPart->Priority_Update(fTimeDelta);
}

void CEffect_Container::Update(_float fTimeDelta)
{
    if (m_bIsPlay == false)
        return;

    m_fAccTime += fTimeDelta;

    if (m_fAccTime >= m_fDuration)
    {
        if (m_bLoop == true)
        {
            m_fAccTime -= m_fDuration;
        }
        else
        {
            m_fAccTime = m_fDuration;
            m_bIsPlay = false;
        }
    }

    for (auto& pEffectPart : m_EffestParts)    
        pEffectPart->Update_EffectByContainer(fTimeDelta, m_fAccTime);
}

void CEffect_Container::Late_Update(_float fTimeDelta)
{
    for (auto& pEffectPart : m_EffestParts)
        pEffectPart->Late_Update(fTimeDelta);

    if (m_bIsPlay == false)
        return;

    Compute_CombinedWorldMatrix();
}

HRESULT CEffect_Container::Render()
{
    for (auto& pEffectPart : m_EffestParts)
        pEffectPart->Render();

    return S_OK;
}

void CEffect_Container::EffectContainer_Start()
{
    for (auto& pEffectPart : m_EffestParts)
        pEffectPart->Effect_Start();
}

HRESULT CEffect_Container::Add_Effect_PartObject(_uint iPrototypeLevelIndex, const _wstring& strPrototypeTag, void* pArg)
{
    CEffect_Part* pEffectPart = dynamic_cast<CEffect_Part*>(
        m_pGameInstance_Proxy->Clone_Prototype(PROTOTYPE::GAMEOBJECT, iPrototypeLevelIndex, strPrototypeTag, pArg));

    if (pEffectPart == nullptr)
        return E_FAIL;

    m_EffestParts.push_back(pEffectPart);

    return S_OK;
}

void CEffect_Container::Compute_CombinedWorldMatrix()
{
    if(m_pParentMatrix != nullptr)
    {
        XMStoreFloat4x4(&m_CombinedWorldMatrix,
            XMLoadFloat4x4(m_pTransformCom->Get_WorldMatrixPtr()) * XMLoadFloat4x4(m_pParentMatrix));
    }
    else
    {
        XMStoreFloat4x4(&m_CombinedWorldMatrix, XMLoadFloat4x4(m_pTransformCom->Get_WorldMatrixPtr()));
    }
}

void CEffect_Container::Init_PropetyValue()
{
    m_bIsPlay = { true };

    m_bLoop = { true };

    m_fDuration = { 5.f };
    m_fAccTime = { 0.f };
}

void CEffect_Container::Free()
{
    for (auto& pEffectPart : m_EffestParts)
        Safe_Release(pEffectPart);
    m_EffestParts.clear();

    __super::Free();
}