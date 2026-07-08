#pragma once
#include "GameContent_Defines.h"
#include "UI_GenericContainer.h"

NS_BEGIN(Client)

class CLIENT_DLL CUI_MissionPanel final : public CUI_GenericContainer
{
    GENERATED_BODY(CUI_MissionPanel)

public:
    static constexpr const _tchar* PROTOTYPE_TAG = L"Proto_UI_MissionPanel";
    static constexpr const _tchar* PART_NAME = L"Name";   // 저작 텍스트 파트 태그에 맞추기
    static constexpr const _tchar* PART_STAMP = L"Stamp";  // 성공 스탬프 파트 태그

private:
    CUI_MissionPanel(ID3D11Device*, ID3D11DeviceContext*);
    CUI_MissionPanel(const CUI_MissionPanel&);
    virtual ~CUI_MissionPanel() = default;

public:
    virtual HRESULT Initialize_Prototype() override;
    virtual HRESULT Initialize(void* pArg) override;
    virtual void Copy_PrototypeName(ENGINE_OBJECT_DATA* p) override { p->strPrototypeTag = PROTOTYPE_TAG; }

    void  Set_Mission(_bool bIsMain, _bool bSucceeded, const _wstring& strName);
    void  Play_Success();
    _bool Is_Succeeded() const { return m_bSucceeded; }

private:
    _bool m_bSucceeded = { false };

public:
    static CUI_MissionPanel* Create(ID3D11Device*, ID3D11DeviceContext*);
    virtual CGameObject* Clone(void* pArg) override;
protected:
    virtual void Free() override;
};
NS_END