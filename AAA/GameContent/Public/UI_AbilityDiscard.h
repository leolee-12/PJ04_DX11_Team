#pragma once
#include "GameContent_Defines.h"
#include "UI_GenericContainer.h"

NS_BEGIN(Client)

class CUI_GaugeFill;
class CUI_Text;

// 커비(3D 월드)를 스크린에 투영한 좌표의 살짝 아래에 직교투영으로 따라다니는 "능력 버리기" UI.
// 비주얼(아이콘/텍스트)은 에디터에서 이 컨테이너의 파트로 저작하고, 이 클래스는 위치 추종만 담당.
class CLIENT_DLL CUI_AbilityDiscard final : public CUI_GenericContainer
{
    GENERATED_BODY(CUI_AbilityDiscard)

    PROPERTY(_float, m_fWorldYOffset, L"World Y Offset", L"AbilityDiscard")   // 커비 기준 월드 높이(머리/몸 조준)
    PROPERTY(_float, m_fScreenYOffset, L"Screen Y Offset", L"AbilityDiscard")   // 스크린에서 아래로 내릴 양(디자인 px)
    PROPERTY(_float, m_fFadeOutDur, L"FadeOut Duration", L"AbilityDiscard")

public:
    static constexpr const _tchar* PROTOTYPE_TAG = L"Proto_UI_AbilityDiscard";

private:
    CUI_AbilityDiscard(ID3D11Device*, ID3D11DeviceContext*);
    CUI_AbilityDiscard(const CUI_AbilityDiscard&);
    virtual ~CUI_AbilityDiscard() = default;

public:
    virtual HRESULT Initialize(void* pArg) override;
    virtual void    Update(_float fTimeDelta) override;
    virtual void    Copy_PrototypeName(ENGINE_OBJECT_DATA* p) override { p->strPrototypeTag = PROTOTYPE_TAG; }

protected:
    virtual HRESULT Ready_Events() override;
    virtual void    On_Deserialized() override;

private:
    CGameObject*         m_pTarget = { nullptr };   
    CUI_GaugeFill*       m_pGauge = { nullptr };   
    CUI_Text*            m_pText = { nullptr };
    const _float*        m_pCoolTime = { nullptr };
    _float               m_fMaxCoolTime = { 1.f };
    _bool                m_bFadingOut = { false };



private:
    _bool Project_TargetToUI(_float2* pOutUI);
    void  Begin_FadeOut();

public:
    static CUI_AbilityDiscard* Create(ID3D11Device*, ID3D11DeviceContext*);
    virtual CGameObject* Clone(void* pArg) override;
protected:
    virtual void Free() override;
};
NS_END