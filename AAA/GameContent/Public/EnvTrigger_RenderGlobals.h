#pragma once
#include "EnvObject_Trigger.h"
#include "Loader_Prototype.h"

NS_BEGIN(Client)

class CLIENT_DLL CEnvTrigger_RenderGlobals final : public CEnvObject_Trigger
{
    GENERATED_BODY(CEnvTrigger_RenderGlobals)

    PROPERTY(_wstring, m_strPreset, L"Preset", L"Render Globals")
    PROPERTY(_float, m_fInTransitionSec, L"In Transition Sec", L"Render Globals")

public:
    static constexpr const _tchar* PROTOTYPE_TAG = L"Proto_EnvTrigger_RenderGlobals";
    static constexpr const _tchar* PRESET_FOLDER = L"../../Resources/YSH/RenderGlobals";

private:
    CEnvTrigger_RenderGlobals(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
    CEnvTrigger_RenderGlobals(const CEnvTrigger_RenderGlobals& Prototype);
    virtual ~CEnvTrigger_RenderGlobals() = default;

public:
    virtual HRESULT Initialize_Prototype() override;
    virtual HRESULT Initialize(void* pArg) override;

    void Apply_RenderGlobals();

#pragma region Editable
    const RENDERGLOBALS_TABLE* Get_PresetTable() const { return m_pPresetTable.get(); }
    void Reload_PresetTable();
#pragma endregion

protected:
    virtual void On_Deserialized() override;

private:
    virtual void OnTriggerEnter(CCollider* pOther) override;
    virtual void OnTriggerStay(CCollider* pOther) override;
    virtual void OnTriggerExit(CCollider* pOther) override;

    void Resolve_Preset();

private:
    shared_ptr<RENDERGLOBALS_TABLE>        m_pPresetTable;
    shared_ptr<const RENDERGLOBALS_VALUES> m_pPreset;
    _wstring                               m_strResolvedPreset;

public:
    static CEnvTrigger_RenderGlobals* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
    virtual CGameObject* Clone(void* pArg) override;

protected:
    virtual void Free() override;
};

NS_END