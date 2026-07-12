#pragma once

#include "GameContent_Defines.h"
#include "UIContainerObject.h"

NS_BEGIN(Client)

// 대화창: 이름 + 텍스트 3줄(Top/Middle/Bottom), 타자기 연출.
// 좌클릭 = 타이핑 중이면 전체 노출(스킵), 전부 노출됐으면 다음 대사 진행
class CLIENT_DLL CUI_Dialogue final : public CUIContainerObject
{
    GENERATED_BODY(CUI_Dialogue)

    PROPERTY(_float, m_fCharInterval, L"Char Interval", L"Dialogue")   // 글자당 초
    PROPERTY(_float, m_fCursorBlink, L"Cursor Blink", L"Dialogue")

public:
    static constexpr const _tchar* PROTOTYPE_TAG = L"Proto_UI_Dialogue";

private:
    CUI_Dialogue(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
    CUI_Dialogue(const CUI_Dialogue& Prototype);
    virtual ~CUI_Dialogue() = default;

public:
    virtual HRESULT Initialize_Prototype() override;
    virtual HRESULT Initialize(void* pArg) override;
    virtual void Priority_Update(_float fTimeDelta) override;
    virtual void Update(_float fTimeDelta) override;
    virtual void Late_Update(_float fTimeDelta) override;
    virtual void Copy_PrototypeName(ENGINE_OBJECT_DATA* pOut) override
    {
        pOut->strPrototypeTag = PROTOTYPE_TAG;
    }

protected:
    virtual HRESULT Ready_Events() override;

private:
    void Begin_Say(const _wstring& strSpeaker, const _wstring* pLines);
    void Tick_Typing(_float fTimeDelta);
    void Apply_Reveal();
    void Reveal_All();
    void Set_LineText(_uint iLine, const _wstring& strText);
    void Set_CursorActive(_bool bOn);

private:
    enum class EBOX { HIDDEN, TYPING, WAIT_INPUT, ADVANCED };
    EBOX m_eBox = { EBOX::HIDDEN };

    _wstring m_FullLines[3];
    size_t   m_iTotalChars = { 0 };
    size_t   m_iRevealed = { 0 };
    size_t   m_LineShown[3] = {}; 
    _float   m_fCharTimer = { 0.f };
    _float m_fCursorTimer = { 0.f };
    _bool  m_bCursorShown = { false };

public:
    static CUI_Dialogue* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
    virtual CGameObject* Clone(void* pArg) override;

protected:
    virtual void Free() override;
};

NS_END