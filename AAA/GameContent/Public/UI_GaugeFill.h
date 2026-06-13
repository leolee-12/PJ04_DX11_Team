#pragma once

#include "GameContent_Defines.h"
#include "UIPartObject.h"
#include "UI_Enums.h"

NS_BEGIN(Engine)
class CShader;
class CTexture;
class CVIBuffer_Text;
NS_END

NS_BEGIN(Client)

class CLIENT_DLL CUI_GaugeFill final : public CUIPartObject
{
	GENERATED_BODY(CUI_GaugeFill)

public:
	using FILL_DIRECTION = Client::UI_GAUGE_FILL_DIRECTION;

private:

    PROPERTY(_float4, m_vColor, L"Color", L"Gauge");
    PROPERTY(_float, m_fAlpha, L"Alpha", L"Gauge");
    PROPERTY(_float, m_fFillRatio, L"FillRatio", L"Gauge");
    PROPERTY(_float, m_fFillSoftness, L"FillSoftness", L"Gauge");
    PROPERTY(_int, m_iFillDirection, L"FillDirection", L"Gauge");
    PROPERTY(_int, m_iShaderPass, L"ShaderPass", L"Gauge");

    PROPERTY(_float, m_fMaskPower, L"MaskPower", L"Mask");
    PROPERTY(_int, m_iMaskChannel, L"MaskChannel", L"Mask");
    PROPERTY(_int, m_iInvertMask, L"InvertMask", L"Mask");

    PROPERTY(_int, m_iTextureLevel, L"TextureLevel", L"Texture");
    PROPERTY(_wstring, m_strTextureProtoTag, L"TextureProtoTag", L"Texture");

public:
    typedef struct tagUIGaugeFillDesc : public CUIPartObject::UI_PARTOBJECT_DESC
    {
        _uint                   iTextureLevel = {};
        const _tchar*           szTextureProtoTag = { nullptr };

        _float2                 vSize = { 100.f, 20.f };
        _float2                 vPosition = { 0.f, 0.f };
        _uint                   iRenderLayer = { 1 };

        _float4                 vColor = { 1.f, 1.f, 1.f, 1.f };
        _float                  fAlpha = { 1.f };
        _float                  fFillRatio = { 1.f };
        _float                  fFillSoftness = { 0.f };
        _int                    iFillDirection = { ETOI(FILL_DIRECTION::LEFT_TO_RIGHT) };
        _int                    iShaderPass = { ETOI(UI_EFFECT_PASS::GAUGE_FILL_COLOR) };

        _float                  fMaskPower = { 1.f };
        _int                    iMaskChannel = { 3 };
        _int                    iInvertMask = { 0 };
    }UI_GAUGEFILL_DESC;

    static constexpr const _tchar* PROTOTYPE_TAG = L"Proto_UI_GaugeFill";
    static constexpr _uint SHADER_PASS_GAUGE_FILL_COLOR = { ETOUI(UI_EFFECT_PASS::GAUGE_FILL_COLOR) };
    static constexpr _uint SHADER_PASS_GAUGE_FILL_TEXTURE = { ETOUI(UI_EFFECT_PASS::GAUGE_FILL_TEXTURE) };

protected:
    CUI_GaugeFill(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
    CUI_GaugeFill(const CUI_GaugeFill& Prototype);
    virtual ~CUI_GaugeFill() = default;

public:
    virtual HRESULT                         Initialize_Prototype() override;
    virtual HRESULT                         Initialize(void* pArg) override;
    virtual void                            Priority_Update(_float fTimeDelta) override;
    virtual void                            Update(_float fTimeDelta) override;
    virtual void                            Late_Update(_float fTimeDelta) override;
    virtual HRESULT                         Render() override;

    virtual void                            Copy_PrototypeName(ENGINE_OBJECT_DATA* pOut) override
    {
        pOut->strPrototypeTag = PROTOTYPE_TAG;
    }

public:
    void                                    Set_FillRatio(_float fRatio);
    _float                                  Get_FillRatio() const { return m_fFillRatio; }

    void                                    Set_Color(const _float4& vColor) { m_vColor = vColor; }
    const _float4&                          Get_Color() const { return m_vColor; }

    HRESULT                                 Set_Texture(_int iLevel, const _wstring& strProtoTag);

public:
    static CUI_GaugeFill*                   Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
    virtual CGameObject*                    Clone(void* pArg) override;

private:
    CShader*                                m_pShaderCom = { nullptr };
    CTexture*                               m_pTextureCom = { nullptr };
    CVIBuffer_Rect*                         m_pVIBufferCom = { nullptr };

private:
    HRESULT                                 Ready_Components(UI_GAUGEFILL_DESC* pDesc);
    HRESULT                                 Bind_ShaderResources();

    virtual void                            Deserialize_Internal(const json& j) override;

protected:
    virtual void                            Free() override;
};

NS_END