#pragma once
#include "GameContent_Defines.h"
#include "UI_GenericContainer.h"

NS_BEGIN(Client)

class CLIENT_DLL CUI_MissionPanel final : public CUI_GenericContainer
{
    GENERATED_BODY(CUI_MissionPanel)

public:
    static constexpr const _tchar* PROTOTYPE_TAG = L"Proto_UI_MissionPanel";
    static constexpr const _tchar* PART_NAME = L"Name";     
    static constexpr const _tchar* PART_STAMP = L"Stamp";   
    static constexpr const _tchar* PART_CAGE = L"IconCage";
    static constexpr const _tchar* PART_WADDLE = L"WaddleDee";

private:
    CUI_MissionPanel(ID3D11Device*, ID3D11DeviceContext*);
    CUI_MissionPanel(const CUI_MissionPanel&);
    virtual ~CUI_MissionPanel() = default;

public:
    virtual HRESULT Initialize_Prototype() override;
    virtual HRESULT Initialize(void* pArg) override;
    virtual void    Update(_float fTimeDelta) override;
    virtual void Copy_PrototypeName(ENGINE_OBJECT_DATA* p) override { p->strPrototypeTag = PROTOTYPE_TAG; }

    void  Set_Mission(_bool bIsMain, _bool bSucceeded, const _wstring& strName);
    void  Play_Success();
    _bool Is_Succeeded() const { return m_bSucceeded; }

protected:
    virtual void On_Deserialized() override;

private:
    enum class ESUCCESS { NONE, SHAKING, REVEALED };
    ESUCCESS m_eSuccess = { ESUCCESS::NONE };
    _float   m_fSuccessTimer = { 0.f };
    _float   m_fShakeDur = { 0.5f };

    _bool m_bSucceeded = { false };

public:
    static CUI_MissionPanel* Create(ID3D11Device*, ID3D11DeviceContext*);
    virtual CGameObject* Clone(void* pArg) override;
protected:
    virtual void Free() override;
};
NS_END