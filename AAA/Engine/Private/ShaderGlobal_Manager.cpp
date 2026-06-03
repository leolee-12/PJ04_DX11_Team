#include "ShaderGlobal_Manager.h"
#include "Shader.h"

CShaderGlobal_Manager::CShaderGlobal_Manager()
{
}

HRESULT CShaderGlobal_Manager::Initialize()
{
    /* 기본 전역 등록 파라미터 추가 시 여기 한 줄 + 셰이더 변수만 늘리면 에디터 UI까지 자동 */
    Register({ "g_fSSAORadius",     "SSAO Radius",     GVAL::FLOAT, { 0.5f,   0.f, 0.f, 0.f }, { 0.f,  5.f } });
    Register({ "g_fSSAOBias",       "SSAO Bias",       GVAL::FLOAT, { 0.015f, 0.f, 0.f, 0.f }, { 0.f,  0.1f } });
    Register({ "g_fSSAOPower",      "SSAO Power",      GVAL::FLOAT, { 1.8f,   0.f, 0.f, 0.f }, { 0.5f, 4.f } });
    Register({ "g_fThreshold",      "Bloom Threshold", GVAL::FLOAT, { 1.f,    0.f, 0.f, 0.f }, { 0.f,  5.f } });
    Register({ "g_fBloomIntensity", "Bloom Intensity", GVAL::FLOAT, { 1.f,    0.f, 0.f, 0.f }, { 0.f,  3.f } });
    // 향후: 포그 색(FLOAT3)/밀도, IBL 강도 등. (IBL 강도는 현재 Environment_Manager 소관이라 옮길지 결정 후)

    return S_OK;
}

void CShaderGlobal_Manager::Register(const GLOBAL_DESC& Desc)
{
    auto it = m_Index.find(Desc.strShaderName);
    if (it != m_Index.end())
    {
        m_Globals[it->second] = Desc;   // 이미 있으면 갱신
        return;
    }
    m_Index.emplace(Desc.strShaderName, static_cast<_uint>(m_Globals.size()));
    m_Globals.push_back(Desc);
}

void CShaderGlobal_Manager::Set(const string& strName, const _float4& vValue)
{
    auto it = m_Index.find(strName);
    if (it == m_Index.end())
        return;
    m_Globals[it->second].vValue = vValue;
}

const _float4* CShaderGlobal_Manager::Get(const string& strName) const
{
    auto it = m_Index.find(strName);
    if (it == m_Index.end())
        return nullptr;
    return &m_Globals[it->second].vValue;
}

HRESULT CShaderGlobal_Manager::Bind(CShader* pShader, const string& strName)
{
    if (nullptr == pShader)
        return E_FAIL;

    auto it = m_Index.find(strName);
    if (it == m_Index.end())
        return E_FAIL;   // 등록 안 된 이름 → 오타 방어(디버그에 잡힘)

    const GLOBAL_DESC& g = m_Globals[it->second];
    return pShader->Bind_RawValue(g.strShaderName.c_str(), &g.vValue, Type_Size(g.eType));
}

HRESULT CShaderGlobal_Manager::Bind(CShader* pShader, initializer_list<const _char*> Names)
{
    for (auto pName : Names)
        if (FAILED(Bind(pShader, pName)))
            return E_FAIL;
    return S_OK;
}

_uint CShaderGlobal_Manager::Type_Size(GVAL eType) const
{
    switch (eType)
    {
        case GVAL::FLOAT:  return sizeof(_float);
        case GVAL::FLOAT2: return sizeof(_float2);
        case GVAL::FLOAT3: return sizeof(_float3);
        case GVAL::FLOAT4: return sizeof(_float4);
    }
    return sizeof(_float4);
}

CShaderGlobal_Manager* CShaderGlobal_Manager::Create()
{
    CShaderGlobal_Manager* pInstance = new CShaderGlobal_Manager;
    if (FAILED(pInstance->Initialize()))
    {
        MSG_BOX("Create Failed : CShaderGlobal_Manager");
        Safe_Release(pInstance);
        return nullptr;
    }
    return pInstance;
}

void CShaderGlobal_Manager::Free()
{
}