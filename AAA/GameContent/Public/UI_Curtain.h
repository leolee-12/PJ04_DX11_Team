#pragma once
#include "GameContent_Defines.h"
#include "UI_CurtainAnimBase.h"

NS_BEGIN(Engine)
class CShader; class CTexture; class CVIBuffer_Rect;
NS_END

NS_BEGIN(Client)

// 커튼 배경(오렌지/노랑 풀스크린 등). pass 8 CurtainFill = 알파 누적 over.
class CLIENT_DLL CUI_Curtain final : public CUI_CurtainAnimBase
{
    GENERATED_BODY(CUI_Curtain)

    PROPERTY(_float4, m_vColor, L"Color", L"UI");

public:
    typedef struct tagUICurtainDesc : public CUI_CurtainAnimBase::UI_CURTAINANIM_DESC
    {
        _float4         vColor = { 1.f, 1.f, 1.f, 1.f };
    }UI_CURTAIN_DESC;

    static constexpr const wchar_t* PROTOTYPE_TAG = L"Proto_UI_Curtain";

protected:
    CUI_Curtain(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
    CUI_Curtain(const CUI_Curtain& Prototype);
    virtual ~CUI_Curtain() = default;

public:
    virtual HRESULT Initialize(void* pArg) override;
    virtual void    Copy_PrototypeName(ENGINE_OBJECT_DATA* pOut) override { pOut->strPrototypeTag = PROTOTYPE_TAG; }

protected:
    virtual _uint   Render_Pass() const override { return 8; }      // CurtainFill
    virtual HRESULT Bind_Material(CShader* pShader) override;

public:
    static CUI_Curtain* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
    virtual CGameObject* Clone(void* pArg) override;
protected:
    virtual void    Free() override;
};
NS_END