#include "EnvTrigger_RenderGlobals.h"

CEnvTrigger_RenderGlobals::CEnvTrigger_RenderGlobals(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CEnvObject_Trigger(pDevice, pContext)
    , m_strPreset{}
    , m_fInTransitionSec{ 0.f }
{
    m_strProtoTag = PROTOTYPE_TAG;
}

CEnvTrigger_RenderGlobals::CEnvTrigger_RenderGlobals(const CEnvTrigger_RenderGlobals& Prototype)
    : CEnvObject_Trigger(Prototype)
    , m_strPreset{ Prototype.m_strPreset }
    , m_fInTransitionSec{ Prototype.m_fInTransitionSec }
    , m_pPresetTable{ Prototype.m_pPresetTable }
{
    m_strProtoTag = PROTOTYPE_TAG;
}

HRESULT CEnvTrigger_RenderGlobals::Initialize_Prototype()
{
    if (FAILED(__super::Initialize_Prototype()))
        return E_FAIL;

    m_pPresetTable = make_shared<RENDERGLOBALS_TABLE>();
    Load_RenderGlobals_PresetFolder(PRESET_FOLDER, m_pPresetTable.get(), m_pGameInstance_Proxy);

#ifdef _DEBUG
    OutputDebugStringA(("[EnvTrigger_RenderGlobals] Preset loaded : "
        + to_string(m_pPresetTable->size()) + "\n").c_str());
#endif

    return S_OK;
}

HRESULT CEnvTrigger_RenderGlobals::Initialize(void* pArg)
{
    if (FAILED(__super::Initialize(pArg)))
        return E_FAIL;

    if (m_tDesc.tEffect.eEffectType == ENV_EFFECT_TYPE::TONE_MAPPING_AREA)
    {
        m_fInTransitionSec = m_tDesc.tEffect.fInTransitionSec > 0.f ? m_tDesc.tEffect.fInTransitionSec : m_tDesc.tEffect.fTransitionSec;
    }

    return S_OK;
}

void CEnvTrigger_RenderGlobals::On_Deserialized()
{
    __super::On_Deserialized();

    Resolve_Preset();
}

void CEnvTrigger_RenderGlobals::Resolve_Preset()
{
    m_pPreset = nullptr;
    m_strResolvedPreset = m_strPreset;

    if (nullptr == m_pPresetTable || m_strPreset.empty())
        return;

    auto iter = m_pPresetTable->find(m_strPreset);
    if (iter == m_pPresetTable->end())
    {
#ifdef _DEBUG
        OutputDebugStringW((L"[EnvTrigger_RenderGlobals] Preset not found : " + m_strPreset + L"\n").c_str());
#endif
        return;
    }

    m_pPreset = iter->second;
}

void CEnvTrigger_RenderGlobals::Apply_RenderGlobals()
{
    if (m_strResolvedPreset != m_strPreset)
        Resolve_Preset();

    if (nullptr == m_pPreset)
        return;

    for (const auto& [strKey, vValue] : *m_pPreset)
        m_pGameInstance_Proxy->Tween_ShaderGlobal(strKey, vValue, m_fInTransitionSec);
}

void CEnvTrigger_RenderGlobals::Reload_PresetTable()
{
    if (nullptr == m_pPresetTable)
        m_pPresetTable = make_shared<RENDERGLOBALS_TABLE>();

    Load_RenderGlobals_PresetFolder(PRESET_FOLDER, m_pPresetTable.get(), m_pGameInstance_Proxy);

    m_strResolvedPreset.clear();
    Resolve_Preset();
}

void CEnvTrigger_RenderGlobals::OnTriggerEnter(CCollider* pOther)
{
    if (false == Is_PlayerActivator(pOther))
        return;

    Apply_RenderGlobals();

#ifdef _DEBUG
    OutputDebugStringA("[EnvTrigger_RenderGlobals] Apply RenderGlobals\n");
#endif
}

void CEnvTrigger_RenderGlobals::OnTriggerStay(CCollider* pOther)
{
    UNREFERENCED_PARAMETER(pOther);
}

void CEnvTrigger_RenderGlobals::OnTriggerExit(CCollider* pOther)
{
    UNREFERENCED_PARAMETER(pOther);
}

CEnvTrigger_RenderGlobals* CEnvTrigger_RenderGlobals::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CEnvTrigger_RenderGlobals* pInstance = new CEnvTrigger_RenderGlobals(pDevice, pContext);

    if (FAILED(pInstance->Initialize_Prototype()))
    {
        MSG_BOX("Failed to Created : CEnvTrigger_RenderGlobals");
        Safe_Release(pInstance);
    }

    return pInstance;
}

CGameObject* CEnvTrigger_RenderGlobals::Clone(void* pArg)
{
    CEnvTrigger_RenderGlobals* pInstance = new CEnvTrigger_RenderGlobals(*this);

    if (FAILED(pInstance->Initialize(pArg)))
    {
        MSG_BOX("Failed to Cloned : CEnvTrigger_RenderGlobals");
        Safe_Release(pInstance);
    }

    return pInstance;
}

void CEnvTrigger_RenderGlobals::Free()
{
    __super::Free();
}