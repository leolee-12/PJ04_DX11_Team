#pragma once
#include "UI_CurtainSequenceBase.h"

NS_BEGIN(Client)

// 로딩 연출 커튼: 진입 시 자동 재생, LoopPeriod 주기로 전체 시퀀스 반복. 완료 신호 없음.
class CLIENT_DLL CUI_LoadingCurtain final : public CUI_CurtainSequenceBase
{
    GENERATED_BODY(CUI_LoadingCurtain)
public:
    static constexpr const _tchar* PROTOTYPE_TAG = L"Proto_UI_LoadingCurtain";

private:
    CUI_LoadingCurtain(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
    CUI_LoadingCurtain(const CUI_LoadingCurtain& Prototype);
    virtual ~CUI_LoadingCurtain() = default;

public:
    virtual void Copy_PrototypeName(ENGINE_OBJECT_DATA* pOut) override { pOut->strPrototypeTag = PROTOTYPE_TAG; }
    // On_SequenceDone 미override → 완료 신호 발행 안 함(기본 no-op)

public:
    static CUI_LoadingCurtain* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
    virtual CGameObject* Clone(void* pArg) override;
protected:
    virtual void Free() override;
};
NS_END