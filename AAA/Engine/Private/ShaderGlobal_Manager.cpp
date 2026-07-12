#include "ShaderGlobal_Manager.h"
#include "Shader.h"

CShaderGlobal_Manager::CShaderGlobal_Manager()
{
}

HRESULT CShaderGlobal_Manager::Initialize()
{
    /* 기본 전역 등록 파라미터 추가 시 여기 한 줄 + 셰이더 변수만 늘리면 에디터 UI까지 자동 */
    Register({ "g_fSSAORadius",     "SSAO Radius",     GVAL::FLOAT, { 5.f,   0.f, 0.f, 0.f }, { 0.f,  5.f } });
    Register({ "g_fSSAOBias",       "SSAO Bias",       GVAL::FLOAT, { 0.1f, 0.f, 0.f, 0.f }, { 0.f,  0.1f } });
    Register({ "g_fSSAOPower",      "SSAO Power",      GVAL::FLOAT, { 2.4f,   0.f, 0.f, 0.f }, { 0.5f, 4.f } });
    Register({ "g_fThreshold",      "Bloom Threshold", GVAL::FLOAT, { 1.f,    0.f, 0.f, 0.f }, { 0.f,  5.f } });
    Register({ "g_fBloomIntensity", "Bloom Intensity", GVAL::FLOAT, { 1.f,    0.f, 0.f, 0.f }, { 0.f,  3.f } });

    Register({ "g_fAmbientSaturation", "Ambient Saturation", GVAL::FLOAT,  { 0.6f, 0.f, 0.f, 0.f }, { 0.f, 1.f } });

    Register({ "g_fSSRIntensity",   "SSR Intensity",   GVAL::FLOAT, { 1.0f,  0.f, 0.f, 0.f }, { 0.f,  3.f } });
    Register({ "g_fSSRMaxDistance", "SSR MaxDistance", GVAL::FLOAT, { 12.0f, 0.f, 0.f, 0.f }, { 1.f, 100.f } });
    Register({ "g_fSSRThickness",   "SSR Thickness",   GVAL::FLOAT, { 0.5f,  0.f, 0.f, 0.f }, { 0.05f, 5.f } });

    Register({ "g_fFogDensity",        "Fog Density",        GVAL::FLOAT,  { 0.04f, 0.f, 0.f, 0.f }, { 0.f, 0.3f } });
    Register({ "g_vFogColor",          "Fog Color",          GVAL::FLOAT3, { 0.70f, 0.80f, 1.00f, 0.f }, { 0.f, 1.f } });
    Register({ "g_fFogFar",            "Fog Far",            GVAL::FLOAT,  { 80.0f, 0.f, 0.f, 0.f }, { 5.f, 200.f } });
    Register({ "g_fFogStart",          "Fog Start",          GVAL::FLOAT,  { 30.0f,  0.f, 0.f, 0.f }, { 0.f, 100.f } });
    Register({ "g_fFogStartRange",     "Fog StartRange",     GVAL::FLOAT,  { 70.0f, 0.f, 0.f, 0.f }, { 0.5f, 100.f } });
    Register({ "g_fFogHeightFalloff",  "Fog HeightFalloff",  GVAL::FLOAT,  { 0.04f, 0.f, 0.f, 0.f }, { 0.f, 0.5f } });
    Register({ "g_fFogBaseHeight",     "Fog BaseHeight",     GVAL::FLOAT,  { 0.00f, 0.f, 0.f, 0.f }, { -50.f, 50.f } });
    Register({ "g_fFogAnisotropy",     "Fog Anisotropy",     GVAL::FLOAT,  { 0.70f, 0.f, 0.f, 0.f }, { -0.9f, 0.9f } });
    Register({ "g_fFogAmbient",        "Fog Ambient",        GVAL::FLOAT,  { 0.02f, 0.f, 0.f, 0.f }, { 0.f, 0.5f } });
    Register({ "g_fFogShadowStrength", "Fog ShadowStrength", GVAL::FLOAT,  { 0.0f, 0.f, 0.f, 0.f }, { 0.f, 1.f } });
    Register({ "g_fFogLightIntensity", "Fog LightIntensity", GVAL::FLOAT, { 3.f, 0.f, 0.f, 0.f }, { 0.f, 15.f } });

    Register({ "g_fFogEnable",          "Fog Enable",        GVAL::BOOL,   { 1.f, 0.f, 0.f, 0.f }, { 0.f, 1.f } });

    Register({ "g_fDoFEnable",     "DoF Enable",       GVAL::BOOL,  { 1.f,  0.f, 0.f, 0.f }, { 0.f, 1.f } });
    Register({ "g_fDoFNear",       "DoF Near",         GVAL::FLOAT, { 6.f,  0.f, 0.f, 0.f }, { 0.1f, 100.f } });
    Register({ "g_fDoFFar",        "DoF Far",          GVAL::FLOAT, { 100.f, 0.f, 0.f, 0.f }, { 0.5f, 200.f } });
    Register({ "g_fDoFFarFalloff", "DoF Far Falloff",  GVAL::FLOAT, { 100.f, 0.f, 0.f, 0.f }, { 0.5f, 100.f } });
    Register({ "g_fDoFCamNear",    "DoF NearRampFrom", GVAL::FLOAT, { 0.1f, 0.f, 0.f, 0.f }, { 0.01f, 10.f } });
    Register({ "g_fAperture",      "DoF Strength",     GVAL::FLOAT, { 0.5f,  0.f, 0.f, 0.f }, { 0.f, 1.f } });
    Register({ "g_fDoFMaxCoC",     "DoF MaxBlur",      GVAL::FLOAT, { 1.f, 0.f, 0.f, 0.f }, { 1.f, 40.f } });

    Register({ "g_fExposure",    "Exposure",          GVAL::FLOAT, { 1.f, 0.f, 0.f, 0.f }, { 0.f, 4.f } });
    Register({ "g_fToneMapMode", "ToneMap 0R/1A/2E",  GVAL::FLOAT, { 0.0f, 0.f, 0.f, 0.f }, { 0.f, 2.f } });

    Register({ "g_vAtmosColor",    "Atmos Color",     GVAL::FLOAT3, { 0.70f, 0.75f, 0.82f, 0.f }, { 0.f, 1.f } });
    Register({ "g_fAtmosStart",    "Atmos Start",     GVAL::FLOAT,  { 50.f,  0.f, 0.f, 0.f }, { 0.f, 500.f } });
    Register({ "g_fAtmosEnd",      "Atmos End",       GVAL::FLOAT,  { 300.f, 0.f, 0.f, 0.f }, { 0.f, 1000.f } });
    Register({ "g_fAtmosStrength", "Atmos Strength",  GVAL::FLOAT,  { 0.5f,  0.f, 0.f, 0.f }, { 0.f, 1.f } });

    Register({ "g_fSpotlightDarken", "Spotlight Darken",  GVAL::FLOAT,  { 0.f,  0.f, 0.f, 0.f }, { 0.f, 1.f } });

    

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

void CShaderGlobal_Manager::Tween(const string& strName, const _float4& vTarget, _float fDuration)
{
    auto it = m_Index.find(strName);
    if (it == m_Index.end())
        return;

    _uint iIdx = it->second;

    // 즉시 세팅: 진행 중이던 보간도 정리
    if (fDuration <= 0.f)
    {
        m_Globals[iIdx].vValue = vTarget;
        m_Tweens.erase(iIdx);
        return;
    }

    TWEEN tw;
    tw.vStart = m_Globals[iIdx].vValue;   // 현재값에서 시작
    tw.vTarget = vTarget;
    tw.fElapsed = 0.f;
    tw.fDuration = fDuration;
    m_Tweens[iIdx] = tw;
}

void CShaderGlobal_Manager::Stop_Tween(const string& strName)
{
    auto it = m_Index.find(strName);
    if (it == m_Index.end())
        return;
    m_Tweens.erase(it->second);
}

_bool CShaderGlobal_Manager::Is_Tweening(const string& strName) const
{
    auto it = m_Index.find(strName);
    if (it == m_Index.end())
        return false;
    return m_Tweens.find(it->second) != m_Tweens.end();
}

void CShaderGlobal_Manager::Tick(_float fTimeDelta)
{
    if (m_Tweens.empty())
        return;

    for (auto it = m_Tweens.begin(); it != m_Tweens.end(); )
    {
        TWEEN& tw = it->second;
        tw.fElapsed += fTimeDelta;

        _float t = tw.fElapsed / tw.fDuration;
        _bool  bDone = (t >= 1.f);
        if (bDone) t = 1.f;

        _float4& v = m_Globals[it->first].vValue;
        XMStoreFloat4(&v,
            XMVectorLerp(XMLoadFloat4(&tw.vStart), XMLoadFloat4(&tw.vTarget), t));

        if (bDone)
            it = m_Tweens.erase(it);
        else
            ++it;
    }
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
        case GVAL::BOOL:   return sizeof(_float);
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