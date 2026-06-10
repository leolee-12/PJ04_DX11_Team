#pragma once
#include "GameContent_Defines.h"
#include "UIPartObject.h"

NS_BEGIN(Engine)
class CShader;
class CTexture;
class CVIBuffer_Rect;
NS_END

NS_BEGIN(Client)

class CLIENT_DLL CUI_Effect final : public CUIPartObject
{
	GENERATED_BODY(CUI_Effect)

    PROPERTY(_float4, m_vColor, L"Color", L"UI");
    PROPERTY(_float, m_fAlpha, L"Alpha", L"UI");
    PROPERTY(_int, m_iShaderPass, L"ShaderPass", L"Effect");

    PROPERTY(_float2, m_vEffectTiling, L"EffectTiling", L"Effect");
    PROPERTY(_float2, m_vEffectOffset, L"EffectOffset", L"Effect");
    PROPERTY(_float, m_fMaskPower, L"MaskPower", L"Effect");
    PROPERTY(_float, m_fEffectIntensity, L"EffectIntensity", L"Effect");
    PROPERTY(_float, m_fRevealProgress, L"RevealProgress", L"Effect");
    PROPERTY(_float, m_fRevealSoftness, L"RevealSoftness", L"Effect");
    PROPERTY(_int, m_iMaskChannel, L"MaskChannel", L"Effect");
    PROPERTY(_int, m_iInvertMask, L"InvertMask", L"Effect");

    PROPERTY(_int, m_iTextureLevel, L"TextureLevel", L"Texture");
    PROPERTY(_wstring, m_strTextureProtoTag, L"TextureProtoTag", L"Texture");

    PROPERTY(_int, m_iMaskTextureLevel, L"MaskTextureLevel", L"Texture");
    PROPERTY(_wstring, m_strMaskTextureProtoTag, L"MaskTextureProtoTag", L"Texture");
    PROPERTY(_int, m_iMaskTexIndex, L"MaskTextureIndex", L"Texture");

public:
    // pass 확인용 안쓰면 지워도 됨
    enum class EFFECT_PASS : _int       
    {
        MASKED_TEXTURE = 2,
        MASKED_COLOR = 3,
        MASKED_ADD = 4,
        BRUSH_REVEAL = 5,
        END
    };

    typedef struct tagUIEffectDesc : public CUIPartObject::UI_PARTOBJECT_DESC
    {
        _uint       iTextureLevel = {};
        const _tchar* szTextureProtoTag = { nullptr };

        _uint       iMaskTextureLevel = {};
        const _tchar* szMaskTextureProtoTag = { nullptr };

        _float2     vSize = { 100.f, 100.f };
        _float2     vPosition = { 0.f, 0.f };
        _uint       iRenderLayer = { 1 };

        _float4     vColor = { 1.f, 1.f, 1.f, 1.f };
        _float      fAlpha = { 1.f };

        _float2     vEffectTiling = { 1.f, 1.f };
        _float2     vEffectOffset = { 0.f, 0.f };
        _float      fMaskPower = { 1.f };
        _float      fEffectIntensity = { 1.f };
        _float      fRevealProgress = { 1.f };
        _float      fRevealSoftness = { 0.05f };
        _int        iMaskChannel = { 3 };   // 0:R, 1:G, 2:B, 3:A
        _int        iInvertMask = { 0 };
        _int        iMaskTexIndex = { 0 };
        _int        iShaderPass = { ETOI(EFFECT_PASS::MASKED_TEXTURE)};
    } UI_EFFECT_DESC;

    static constexpr const _tchar* PROTOTYPE_TAG = L"Proto_UI_Effect";

protected:
    CUI_Effect(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
    CUI_Effect(const CUI_Effect& Prototype);
    virtual ~CUI_Effect() = default;

public:
    virtual HRESULT         Initialize_Prototype() override;
    virtual HRESULT         Initialize(void* pArg) override;
    virtual void            Priority_Update(_float fTimeDelta) override;
    virtual void            Update(_float fTimeDelta) override;
    virtual void            Late_Update(_float fTimeDelta) override;
    virtual HRESULT         Render() override;

    virtual void            Copy_PrototypeName(ENGINE_OBJECT_DATA* pOut) override
    {
        pOut->strPrototypeTag = PROTOTYPE_TAG;
    }

public:
    HRESULT                 Set_Texture(_int iLevel, const _wstring& strProtoTag);
    HRESULT                 Set_MaskTexture(_int iLevel, const _wstring& strProtoTag);

public:
    static CUI_Effect*      Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
    virtual CGameObject*    Clone(void* pArg) override;

private:
    CShader*                m_pShaderCom = { nullptr };
    CTexture*               m_pEffectTextureCom = { nullptr };
    CTexture*               m_pMaskTextureCom = { nullptr };
    CVIBuffer_Rect*         m_pVIBufferCom = { nullptr };

private:
    HRESULT                 Ready_Components(UI_EFFECT_DESC* pDesc);
    HRESULT                 Bind_ShaderResources();

    virtual void            Deserialize_Internal(const json& j) override;

protected:
    virtual void            Free() override;

};

NS_END