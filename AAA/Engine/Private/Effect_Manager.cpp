#include "Effect_Manager.h"
#include "GameInstance.h"
#include "GameObject.h"

CEffect_Manager::CEffect_Manager(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : m_pGameInstance_Proxy{ CGameInstance::GetProxy() }
    , m_pDevice{ pDevice }
    , m_pContext{ pContext }
{
    Safe_AddRef(m_pDevice);
    Safe_AddRef(m_pContext);
}

HRESULT CEffect_Manager::Initialize()
{
    m_p2DShader = CShader::Create(m_pDevice, m_pContext, TEXT("../Bin/ShaderFiles/Shader_Effect_VtxTex.hlsl"), VTXTEX::Elements, VTXTEX::iNumElements);
    if (m_p2DShader == nullptr)
        return E_FAIL;

    m_pMeshShader = CShader::Create(m_pDevice, m_pContext, TEXT("../Bin/ShaderFiles/Shader_Effect_Mesh.hlsl"), VTXEFFECTMESH::Elements, VTXEFFECTMESH::iNumElements);
    if (m_pMeshShader == nullptr)
        return E_FAIL;

    m_pTrailShader = CShader::Create(m_pDevice, m_pContext, TEXT("../Bin/ShaderFiles/Shader_Effect_Trail.hlsl"), VTXTRAIL::Elements, VTXTRAIL::iNumElements);
    if (m_pTrailShader == nullptr)
        return E_FAIL;

    return S_OK;
}

HRESULT CEffect_Manager::Spawn(_uint iLevel, const _wstring& strEffectKey, const _wstring& strProtoTag, const CEffect_Container::EFFECT_CONTAINER_DESC& desc, const json* pConfig, CEffect_Container** ppOut)
{
    POOL_KEY key{ iLevel, strEffectKey };                  // 풀 키 = effectId

    // 재사용 (작성값 이미 들어있음 → config 주입 안 함)
    auto it = m_Dormant.find(key);
    if (it != m_Dormant.end() && !it->second.empty())
    {
        CEffect_Container* pEffect = it->second.back();
        it->second.pop_back();
        if (ppOut) *ppOut = pEffect;
        return S_OK;
    }

    // STATIC 프로토에서 클론 + Initialize → 작성값 주입
    _wstring strObjTag = strEffectKey + L"#" + std::to_wstring(m_iSpawnCounter++);

    CGameObject* pObj = nullptr;
    HRESULT hr = m_pGameInstance_Proxy->Add_GameObject_Return(
        &pObj,
        m_iProtoLevel, strProtoTag,            // 프로토 조회 = STATIC(좌석)
        iLevel, EFFECT_LAYER_TAG, strObjTag,   // 객체 배치 = 타겟(스테이지) 레벨
        const_cast<CEffect_Container::EFFECT_CONTAINER_DESC*>(&desc));

    if (FAILED(hr) || !pObj) return E_FAIL;

    CEffect_Container* pEffect = dynamic_cast<CEffect_Container*>(pObj);
    if (!pEffect) { m_pGameInstance_Proxy->Destroy_GameObject(pObj); return E_FAIL; }

    if (pConfig) pEffect->Deserialize(*pConfig);           // 콜드 전용: Initialize 후 작성값 주입
    pEffect->Set_Pool(this, iLevel, strEffectKey);          // 반납 키 = effectId

    if (ppOut) *ppOut = pEffect;
    return S_OK;
}

CEffect_Manager* CEffect_Manager::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CEffect_Manager* pInstance = new CEffect_Manager(pDevice, pContext);

    if (FAILED(pInstance->Initialize()))
    {
        MSG_BOX("Failed to Created : CEffect_Manager");
        Safe_Release(pInstance);
        return nullptr;
    }
    return pInstance;
}

void CEffect_Manager::Free()
{
    m_Dormant.clear();             // 약참조라 Release 금지, clear만
    Safe_Release(m_p2DShader);
    Safe_Release(m_pMeshShader);
    Safe_Release(m_pTrailShader);
    Safe_Release(m_pContext);
    Safe_Release(m_pDevice);
    Safe_Release(m_pGameInstance_Proxy);
    __super::Free();
}

void CEffect_Manager::Return(_uint iLevel, const _wstring& strEffectKey, CEffect_Container* pEffect)
{
    m_Dormant[POOL_KEY{ iLevel, strEffectKey }].push_back(pEffect);
}

void CEffect_Manager::Clear_Level(_uint iLevel)
{
    // 레벨 언로드 시 그 레벨 객체는 레이어가 파괴 → 휴면 약참조 제거(dangling 방지)
    for (auto it = m_Dormant.begin(); it != m_Dormant.end(); )
        it = (it->first.iLevel == iLevel) ? m_Dormant.erase(it) : std::next(it);
}
