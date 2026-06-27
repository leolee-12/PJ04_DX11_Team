#include "Effect_Container.h"

#include "GameInstance.h"

#include "Effect_Part.h"

#include "Effect_Manager.h"


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
    if (!m_bIsPlay)
        return;

    for (auto& [tag, pPart] : m_EffestParts)
        pPart->Priority_Update(fTimeDelta);
}

void CEffect_Container::Update(_float fTimeDelta)
{
    Debug_ResetPlay();

    if (m_bIsPlay == false)
        return;

    m_fAccTime += fTimeDelta;

    if (m_fAccTime >= m_fDuration)
    {
        if (m_bLoop == true)
        {
            m_fAccTime = 0.f;
        }
        else
        {
            m_fAccTime = m_fDuration;
            m_bIsPlay = false;

            m_pParentMatrix = nullptr;

            if (m_pPool)   // 휴면되는 순간 매니저로 반납(재생당 1회)
                m_pPool->Return(m_iPoolLevel, m_strPoolKey, this);
        }
    }

    for (auto& [tag, pPart] : m_EffestParts)
    {
        pPart->Update_PlayValue(m_bIsPlay, m_bLoop, m_fDuration, m_fAccTime);
        pPart->Update(fTimeDelta);
    }
}

void CEffect_Container::Late_Update(_float fTimeDelta)
{
    if (!m_bIsPlay)
        return;

    Compute_CombinedWorldMatrix();

    for (auto& [tag, pPart] : m_EffestParts)
        pPart->Late_Update(fTimeDelta);
}

HRESULT CEffect_Container::Render()
{
    if (!m_bIsPlay) 
        return S_FALSE;

    for (auto& [tag, pPart] : m_EffestParts)
    {
        if (pPart->Is_EffectPartActive() == true)
            pPart->Render();
    }

    return S_OK;
}

void CEffect_Container::EffectContainer_Start(const _float3& vSpawnPos, const _float3& vLookDir, const _float4x4* pParentMatrix)
{
    for (auto& [tag, pPart] : m_EffestParts)
        pPart->Effect_Start();

    m_bIsPlay = true;
    m_fAccTime = 0.f;

    _vector vPos = XMVectorSetW(XMLoadFloat3(&vSpawnPos), 1.f);
    m_pTransformCom->Set_State(STATE::POSITION, vPos);

    _vector vDir = XMLoadFloat3(&vLookDir);
    
    if (XMVectorGetX(XMVector3LengthSq(vDir)) > Helper::fEpsilon)
    {
        vDir = XMVector3Normalize(vDir);
        m_pTransformCom->LookAt(vPos + vDir);
    }

    m_pParentMatrix = pParentMatrix;
}

void CEffect_Container::EffectContainer_Stop()
{
    if (m_bIsPlay == false)
        return;

    m_bIsPlay = false;
    m_fAccTime = m_fDuration;
    m_pParentMatrix = nullptr;

    for (auto& [tag, pPart] : m_EffestParts)
        pPart->Update_PlayValue(false, m_bLoop, m_fDuration, m_fAccTime);

    if (m_pPool)
        m_pPool->Return(m_iPoolLevel, m_strPoolKey, this);
}

void CEffect_Container::Set_ParentMatrix(const _float4x4* pParentMatrix)
{
    m_pParentMatrix = pParentMatrix;
}

json CEffect_Container::Serialize() const
{
    json j = __super::Serialize();

    for (auto& [tag, pPart] : m_EffestParts)
    {
        string strTag = WstrToStr(tag);
        j["EffectPartObjects"][strTag] = pPart->Serialize();
    }

    return j;
}

void CEffect_Container::Deserialize_Internal(const json& j)
{
    __super::Deserialize_Internal(j);

    if (!j.contains("EffectPartObjects")) return;

    for (auto& [tag, pPart] : m_EffestParts)
    {
        string strTag = WstrToStr(tag);
        if (j["EffectPartObjects"].contains(strTag))
            pPart->Deserialize(j["EffectPartObjects"][strTag]);
    }
}

void CEffect_Container::Debug_ResetPlay()
{
    if (m_bPreResetPlayDoubleCheck == false && m_bResetPlayDoubleCheck == true)
    {
        m_bPreResetPlayDoubleCheck = true;
        EffectContainer_Start(_float3{ 0.f, 0.f, 0.f });
    }

    if (m_bPreResetPlayDoubleCheck == true && m_bResetPlayDoubleCheck == false)
    {
        m_bPreResetPlayDoubleCheck = false;
    }
}

HRESULT CEffect_Container::Add_Effect_PartObject(_uint iPrototypeLevelIndex, const _wstring& strPrototypeTag, const _wstring& strPartTag, void* pArg)
{
    CEffect_Part* pEffectPart = dynamic_cast<CEffect_Part*>(
        m_pGameInstance_Proxy->Clone_Prototype(PROTOTYPE::GAMEOBJECT, iPrototypeLevelIndex, strPrototypeTag, pArg));

    if (pEffectPart == nullptr)
        return E_FAIL;

    pEffectPart->Set_ParentMatrix(&m_CombinedWorldMatrix);
    m_EffestParts.emplace(strPartTag, pEffectPart);

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
    m_bResetPlayDoubleCheck = { false };

    m_bIsPlay = { true };

    m_bLoop = { true };

    m_fDuration = { 1.f };
    m_fAccTime = { 0.f };
}

void CEffect_Container::Free()
{
    for (auto& [tag, pPart] : m_EffestParts)
        Safe_Release(pPart);
    m_EffestParts.clear();

    __super::Free();
}