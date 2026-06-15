#pragma once
#include "UI_CurtainAnimBase.h"

NS_BEGIN(Client)

// 별 모양으로 커튼 RT 알파를 뚫는 지우개. pass 10 AlphaErase.
class CLIENT_DLL CUI_Eraser final : public CUI_CurtainAnimBase
{
    GENERATED_BODY(CUI_Eraser)

public:
    using UI_ERASER_DESC = CUI_CurtainAnimBase::UI_CURTAINANIM_DESC;
    static constexpr const wchar_t* PROTOTYPE_TAG = L"Proto_UI_Eraser";

protected:
    CUI_Eraser(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
    CUI_Eraser(const CUI_Eraser& Prototype);
    virtual ~CUI_Eraser() = default;

public:
    virtual void    Copy_PrototypeName(ENGINE_OBJECT_DATA* pOut) override { pOut->strPrototypeTag = PROTOTYPE_TAG; }

protected:
    virtual _uint   Render_Pass() const override { return 10; }     // AlphaErase
    virtual HRESULT Bind_Material(CShader* pShader) override;

public:
    static CUI_Eraser* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
    virtual CGameObject* Clone(void* pArg) override;
protected:
    virtual void    Free() override;
};
NS_END