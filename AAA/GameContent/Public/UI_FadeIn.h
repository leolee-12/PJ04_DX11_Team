#pragma once
#include "UI_CurtainSequenceBase.h"

NS_BEGIN(Client)

// 화면 페이드 인: 덮은 채 시작해 진입 시 자동으로 걷힘. 완료 시 신호 발행.
class CLIENT_DLL CUI_FadeIn final : public CUI_CurtainSequenceBase
{
    GENERATED_BODY(CUI_FadeIn)
    PROPERTY(_wstring, m_strDoneEvent, L"DoneEvent", L"Sequence")

public:
    static constexpr const _tchar* PROTOTYPE_TAG = L"Proto_UI_FadeIn";

private:
    CUI_FadeIn(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
    CUI_FadeIn(const CUI_FadeIn& Prototype);
    virtual ~CUI_FadeIn() = default;

public:
    virtual void Copy_PrototypeName(ENGINE_OBJECT_DATA* pOut) override { pOut->strPrototypeTag = PROTOTYPE_TAG; }

protected:
    virtual void On_SequenceDone() override;   // 페이드인 완료 → 게임플레이 시작 신호

public:
    static CUI_FadeIn* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
    virtual CGameObject* Clone(void* pArg) override;
protected:
    virtual void Free() override;
};
NS_END