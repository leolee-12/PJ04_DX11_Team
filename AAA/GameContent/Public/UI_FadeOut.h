#pragma once
#include "UI_CurtainSequenceBase.h"

NS_BEGIN(Client)

// 화면 페이드 아웃: 숨어 있다가 신호(Play)로 화면을 덮음. 완료 시 신호 발행.
class CLIENT_DLL CUI_FadeOut final : public CUI_CurtainSequenceBase
{
    GENERATED_BODY(CUI_FadeOut)
public:
    static constexpr const _tchar* PROTOTYPE_TAG = L"Proto_UI_FadeOut";

private:
    CUI_FadeOut(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
    CUI_FadeOut(const CUI_FadeOut& Prototype);
    virtual ~CUI_FadeOut() = default;

public:
    virtual void Copy_PrototypeName(ENGINE_OBJECT_DATA* pOut) override { pOut->strPrototypeTag = PROTOTYPE_TAG; }

protected:
    virtual void On_SequenceStart() override;
    virtual void On_SequenceDone() override;   // 페이드아웃 완료 신호

public:
    static CUI_FadeOut* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
    virtual CGameObject* Clone(void* pArg) override;
protected:
    virtual void Free() override;
};
NS_END