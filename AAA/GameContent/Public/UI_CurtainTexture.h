#pragma once
#include "UI_CurtainAnimBase.h"

NS_BEGIN(Client)

// 회전+축소하며 색칠 텍스처를 커튼 RT에 그림(노란 별 등). pass 8 CurtainFill.
class CLIENT_DLL CUI_CurtainTexture final : public CUI_CurtainAnimBase
{
    GENERATED_BODY(CUI_CurtainTexture)

    PROPERTY(_float4, m_vColor, L"Color", L"UI")
    PROPERTY(_bool, m_bAdditive, L"Additive", L"UI")

public:
    typedef struct tagUICurtainTextureDesc : public CUI_CurtainAnimBase::UI_CURTAINANIM_DESC
    {
        _float4         vColor = { 1.f, 1.f, 1.f, 1.f };
    }UI_CURTAINTEXTURE_DESC;

    static constexpr const wchar_t* PROTOTYPE_TAG = L"Proto_UI_CurtainTexture";

protected:
    CUI_CurtainTexture(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
    CUI_CurtainTexture(const CUI_CurtainTexture& Prototype);
    virtual ~CUI_CurtainTexture() = default;

public:
    virtual HRESULT Initialize(void* pArg) override;        // vColor만 추가로 읽고 베이스로
    virtual void    Copy_PrototypeName(ENGINE_OBJECT_DATA* pOut) override { pOut->strPrototypeTag = PROTOTYPE_TAG; }

protected:
    virtual _uint   Render_Pass() const override { return m_bAdditive ? 11 : 8; }              // CurtainFill
    virtual HRESULT Bind_Material(CShader* pShader) override;

public:
    static CUI_CurtainTexture* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
    virtual CGameObject* Clone(void* pArg) override;
protected:
    virtual void    Free() override;
};
NS_END