#pragma once
#include "UI_CurtainSequenceBase.h"

NS_BEGIN(Client)

class CLIENT_DLL CUI_FlashCurtain final : public CUI_CurtainSequenceBase
{
    GENERATED_BODY(CUI_FlashCurtain)
public:
    static constexpr const _tchar* PROTOTYPE_TAG = L"Proto_UI_FlashCurtain";

private:
    CUI_FlashCurtain(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
    CUI_FlashCurtain(const CUI_FlashCurtain& Prototype);
    virtual ~CUI_FlashCurtain() = default;

public:
    virtual void Copy_PrototypeName(ENGINE_OBJECT_DATA* pOut) override { pOut->strPrototypeTag = PROTOTYPE_TAG; }

protected:
    virtual void On_SequenceDone() override;   // ¹øÂ½ ³¡³ª¸é ´Ù½Ã ¼û±è + ¿Ï·á ½ÅÈ£

public:
    static CUI_FlashCurtain* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
    virtual CGameObject* Clone(void* pArg) override;
protected:
    virtual void Free() override;
};
NS_END